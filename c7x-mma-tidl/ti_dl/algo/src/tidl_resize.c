/*
 *
 * Copyright (c) {2015 - 2024} Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
----------------------------------------------------------------------------
@file    tidl_resize.c
@brief   This file defines private functions for resize layer.
@version 0.1 (Aug 2019) : Initial version
----------------------------------------------------------------------------
*/

#include <math.h>
#include <float.h>
#include "tidl_commonUtils.h"
#include "tidl_priv_algo.h"
#include "tidl_device_functions.h"
#include "gc_helper.h"
#include "tidl_resize_ref.h"
#include "tidl_forceNegativeTest.h"

/**
 * @brief This is reference implementation of Resize layer
 *
 * @tparam Tin : template for input data buffers
 * @param pIn : Pointer to input memory
 * @param pOut : Pointer to output memory
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : Copy of Resize layer parameters
 * @param inDataParams : parameters of the input data buffer
 */

void getResizeInfo(sTIDL_ResizeInfo_t *resizeInfo, const sTIDL_AlgLayer_t *algLayer, const sTIDL_Layer_t *tidlLayer, const TIDL_CreateParams *createParams, int32_t resizeInHeight)
{
  uint32_t copyLineInfo = TIDL_COPY_NO_LINE;
  int32_t inputHeight = resizeInHeight;

  uint32_t tileType = algLayer->wlPadParams.tileType;
  uint32_t isFirstTile = algLayer->wlPadParams.isFirstTile;
  uint32_t isLastTile = algLayer->wlPadParams.isLastTile;
  int32_t wlRepeatIter = algLayer->wlRepeatIter;
  
  int32_t multiCoreMode = (algLayer->workloadUnit != NULL)
                            ? algLayer->workloadUnit->multiCoreMode
                            : TIDL_NOT_MULTI_CORE;
                          
  int32_t notMultiCoreMode = ((uint32_t)multiCoreMode == TIDL_NOT_MULTI_CORE);
  int32_t isFirstTileType = ((tileType != NO_TILE) && (isFirstTile == 1U) && (wlRepeatIter == 0));
  int32_t isNoTileType = (tileType == NO_TILE);
  int32_t isLastTileType = ((tileType != NO_TILE) && (isLastTile == 1U) && (wlRepeatIter == 0));

#if TIDL_DEVICE_MULTICORE
  int32_t coreId = createParams->coreId;
  int32_t coreStartIdx = createParams->coreStartIdx;
  int32_t inferenceMode = createParams->net->inferenceMode;
  int32_t numCores = createParams->net->numCores;
  int32_t spatialMultiCoreMode = ((multiCoreMode & (int32_t)TIDL_MULTI_CORE_SPATIAL) == (int32_t)TIDL_MULTI_CORE_SPATIAL);
  int32_t isMultiCoreFirstTileType = ((tileType != NO_TILE) && (coreId == coreStartIdx) && (isFirstTile == 1U) && (wlRepeatIter == 0));
  int32_t isMultiCoreNoTileStart = ((tileType == NO_TILE) && (coreId == coreStartIdx));
  int32_t isMultiCoreLastTileType = ((tileType != NO_TILE) && (isLastTile == 1U) && (wlRepeatIter == 0) && (coreId == (GET_NUMCORES_FOR_STARTCOREIDX(numCores, coreStartIdx) - 1)));
  int32_t isMultiCoreNoTileEnd = ((tileType == NO_TILE) && (coreId == (GET_NUMCORES_FOR_STARTCOREIDX(numCores, coreStartIdx) - 1)));
#endif

  if ((tileType != NO_TILE) /*super Tiling*/
#if TIDL_DEVICE_MULTICORE
      || ((inferenceMode == (int32_t)TIDL_inferenceModeLowLatency) && spatialMultiCoreMode) /*Multi-Core*/
#endif
  )
  {
    /*
    *-------------------------Copy Top Line:---------------------------*
    1. layer processed in single core
      i) ST -> first tile ii) Non-ST -> always
    2. Layer Processed across multiple cores
      ii) ST -> First Tile + first core ii) Non-ST -> First Core
    *-------------------------Copy Bottom Line:-------------------------*
    1. layer processed in single core
      i) ST -> Last Tile tile ii) Non-ST -> always
    2. Layer Processed across multiple cores
      i) ST -> Last Tile + Last core ii) Non-ST -> Last Core
    */
    if ((/*1*/ notMultiCoreMode &&
         (/* i*/ isFirstTileType ||
          /*ii*/ isNoTileType))
#if TIDL_DEVICE_MULTICORE
        || (/*2*/ spatialMultiCoreMode &&
            (/* i*/ isMultiCoreFirstTileType ||
             /*ii*/ isMultiCoreNoTileStart))
#endif
    )
    {
      copyLineInfo |= TIDL_COPY_TOP_LINE;
    }
    /*Relook Copy_Bottom_Line condition*/
    if ((/*1*/ notMultiCoreMode &&
         (/* i */ isLastTileType ||
          /*ii*/ isNoTileType))
#if TIDL_DEVICE_MULTICORE
        || ((/*2*/ spatialMultiCoreMode &&
             (/*i*/ isMultiCoreLastTileType ||
              /*ii*/ isMultiCoreNoTileEnd)))
#endif
    )

    {
      copyLineInfo |= TIDL_COPY_BOTTOM_LINE;
    }
    else
    {
      /*do nothing*/
    }
  }
  else
  {
    copyLineInfo = TIDL_COPY_TOP_LINE | TIDL_COPY_BOTTOM_LINE;
  }

  resizeInfo->copyLineInfo = copyLineInfo;
  resizeInfo->inputHeight = inputHeight;

  return;
}

static void TIDL_resizeUpdateWorkload(const sWorkloadUnit_t *workloadUnit,
                                      const sGCHelperHandle *gcHelperHandle,
                                      const sTIDL_Layer_t *tidlLayer)
{
  const sTIDL_ResizeLayerParams_t *params = &tidlLayer->layerParams.resizeParams;
  int32_t numSplit = getNumSplits((const void *)workloadUnit);
  int32_t outPitch = tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  int32_t outPad = 1;
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];

  if (params->resizeRatio[TIDL_DIM_WIDTH] == 4.0)
  {
    outPad = 2;
  }
  /* Resize kernel overwrites the pad in the output buffer and expects the joint to point to the padded region*/
  for (int32_t linkIdx = 0; linkIdx < workloadUnit->numLinks; linkIdx++)
  {
    getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
    sLink_t *link = linkPtrList[0];
    const sBufParams_t *buf = getBufParamsFromBufIndex(gcHelperHandle, link->sink.bufDBindex);

    /* In resize LFMP case, there exists overlap in the output as well. This is because resize expects 'padded'
      region for every split and we provide last few rows of the previous split to satisfy the kernel requirement.
      This overlapping region in the input creates the overlapping region in the output between split feature maps.
      To optimize things further, resize layer understands that it is processing the overlap region and skips
      processing the overlapping input in every split. However, it leaves space in the output assuming that it was
      overwriting the overlap output that the previous split generated.
      When the kernel writes directly to output buffer, gc can handle by manipulating flowDims. However, in
      case of output DMA we:
      1) Write only the valid region that the kernel generates for the split (basically (kernel expected output) - (overlap output))
        This is handled by gc
      2) Offset the read from staging buffer by amount equal to the output overlap size.
        This is handled by TIDL in the following code.
      */
    if ((link->subType == (int32_t)LINK_X_FM_OUT_PART_TO_FULL) && ((numSplit > 1) || workloadUnit->multiCoreMode == TIDL_MULTI_CORE_SPATIAL_WITH_JOIN))
    {
      link->src[0].offset = link->src[0].offset + ((outPad * outPitch) + outPad);
    }
    if ((buf->type == (int32_t)BUF_FM_FULL))
    {
      /* LFMP case we overwrite padded region only in case there is no output DMA*/
      if ((link->type == (int32_t)LINK_PROC) && (numSplit > 1))
      {
        link->sink.offset = link->sink.offset - ((outPad * outPitch) + outPad);
      }
      /* SFMP case we overwrite padded region of the final buffer by links responsible for generating output info */
      // Condition modified to don't reset the offset when the mode is TIDL_ResizePadZero , resizePadZeroOffset is 1 for resize ratio 2.
      else if (((numSplit == 1) && (workloadUnit->multiCoreMode != TIDL_MULTI_CORE_SPATIAL_WITH_JOIN)) && ((((link->subType == (int32_t)LINK_P) && ((params->mode != TIDL_ResizePadZero) || ((params->resizePadZeroOffset == 0) || (params->resizeRatio[TIDL_DIM_WIDTH] == 4) )))) || (link->subType == (int32_t)LINK_X_FM_OUT_PART_TO_FULL)))
      {
        link->sink.offset = link->sink.offset - ((outPad * outPitch) + outPad);
      }
      else
      {
        /*do nothing*/
      }
    }
  }
}

static int32_t TIDL_resizeGetPermuteBufSize(sTIDL_Network_t *net, int32_t layerIdx)
{
  sTIDL_Layer_t *tidlLayer = &net->TIDLLayers[layerIdx];
  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(net, tidlLayer->inData[0]);
  uint32_t resizeInWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];
  uint32_t resizeOutWidth = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
  uint32_t simdWidth = (TIDL_SIMD_WIDTH / 2U) - 2U;

  /*scartch size for permute_control and control variables in the kernel. 128 bytes for 2 control patterns*/
  int32_t scratchSize = (((resizeInWidth + 2U + simdWidth - 1U) / simdWidth) * (resizeOutWidth / resizeInWidth) * TIDL_SIMD_WIDTH) + (TIDL_SIMD_WIDTH * 2U);

  return scratchSize;
}

static inline int32_t TIDL_resizeAllocPermuteBufMem(const TIDL_NetworkCommonParams *commonParams,
                                                    int32_t layerIdx,
                                                    sTIDL_AlgLayer_t *algLayer,
                                                    int32_t permuteBufSize,
                                                    int32_t *paramMemTabOffset,
                                                    uint8_t *memory[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = TIDL_SUCCESS;
  sTIDL_ALgResizeLayer_t *algResizeLayerParams = &algLayer->layerParams.resizeParams;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                   paramMemTabOffset,
                   permuteBufSize,
                   TIDL_ALIGNMENT_SIZE,
                   &algResizeLayerParams->permutePatternBuf);

  algResizeLayerParams->permutePatternBufInt = (int8_t *)TIDL_getMemoryChunkFromSysmem(commonParams->tidlCommonParams->sysMems,
                                                                                       permuteBufSize,
                                                                                       128U,
                                                                                       TIDL_SYSMEM_L3_SCRATCH,
                                                                                       IALG_SCRATCH);

  if (algResizeLayerParams->permutePatternBufInt == NULL)
  {
    tidl_printf(1, "Can't fit resize permute buffer in MSMC, Trying DDR\n");

    algResizeLayerParams->permutePatternBufInt = (int8_t *)TIDL_getMemoryChunkFromSysmem(commonParams->tidlCommonParams->sysMems,
                                                                                         permuteBufSize,
                                                                                         128U,
                                                                                         TIDL_SYSMEM_DDR_SCRATCH,
                                                                                         IALG_SCRATCH);

    if (algResizeLayerParams->permutePatternBufInt == NULL)
    {
      tidl_printf(0, "DDR alloc failed for resize: %d\n", permuteBufSize);
      status = TIDL_ERR_FAILURE;
    }
  }

  return status;
}

/**
*******************************************************************************
*  \brief      Function to supply memory requirement for Resize layer to user
*
*
*  \param [in]  specificParams :  Pointer to specific parameter required for all the layers
*
*  \param [in]  commonParams :  Pointer to common parameter required for all the layers
*
*  \param [in]  layerIdx           : Layer Id of the layer
*
*  \param [out]  memorySize           : Output memory requirement of the layer
*
*  \return     Error:  IALG_EOK ( Successful) else failure
*
*  \remarks
*******************************************************************************
*/
int32_t TIDL_resizeAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            int32_t layerIdx,
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;
  sTIDL_Layer_t *tidlLayer = &commonParams->net->TIDLLayers[layerIdx];
  sTIDL_ResizeLayerParams_t *resizeLayerParams = &tidlLayer->layerParams.resizeParams;

  uint32_t isOptFlowAvailable = isOptimizedResizeAvailable(resizeLayerParams);
  uint32_t flowCtrl;
  if (isOptFlowAvailable == 1U)
  {
    flowCtrl = commonParams->createParams->flowCtrl;
  }
  else
  {
    flowCtrl = (int32_t)TIDL_FLOW_CTRL_REF_ONLY;
  }

  if ((flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  else
  {
    status = TIDL_deviceUtilsCommonAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += TIDL_resizeGetPermuteBufSize(commonParams->net, layerIdx) + 128;
  }

  return status;
}

/**
*******************************************************************************
*  \brief      Function to set memory needed by Resize layer
*
*
*  \param [in]  specificParams :  Pointer to specific parameter required for all the layers
*
*  \param [in]  commonParams :  Pointer to common parameter required for all the layers
*
*  \param [in]  algLayer :  Pointer to layer specific internal parameters
*
*  \param [in]  layerIdx           : Layer Id of the layer
*
*  \param [in]  memory           : Allocated memory pointers
*
*  \param [out]  memorySize           : Output memory requirement of the layer
*
*  \param [out]  outPtr           : Output memory pointer. Applicable only for reference flow
*
*  \return     Error:  IALG_EOK ( Successful) else failure
*
*  \remarks
*******************************************************************************
*/
int32_t TIDL_resizeInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                           const TIDL_NetworkCommonParams *commonParams,
                           sTIDL_AlgLayer_t *algLayer,
                           int32_t layerIdx,
                           uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                           int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                           void **outPtr)
{
  int32_t status = IALG_EOK;
  sTIDL_Layer_t *tidlLayer = &commonParams->net->TIDLLayers[layerIdx];
  sTIDL_ResizeLayerParams_t *resizeLayerParams = &tidlLayer->layerParams.resizeParams;
  sTIDL_ALgResizeLayer_t *algResizeLayerParams = &algLayer->layerParams.resizeParams;

  uint32_t isOptFlowAvailable = isOptimizedResizeAvailable(resizeLayerParams);
  uint32_t flowCtrl = commonParams->createParams->flowCtrl;

  algResizeLayerParams->isOptResizeAvail = isOptFlowAvailable;

  if ((isOptFlowAvailable == 0U) && ((flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U))
  {
    tidl_printf(0, " WARNING : Running unoptimized code (Natural C code) for the resize layer number  %d !!!\n ", algLayer->layerIdx);
    flowCtrl = TIDL_FLOW_CTRL_REF_ONLY;
  }

  /** Resize layer duplicates the top and bottom rows into the input buffer's padded region
   * If resize is the only consumer of the input layer, then there is no issue
   * However, if the input layer has any other consumer layer, then padded region should be restored with zeroes
   * Below code checks for this and populates "inputPadRefillZeros" to be used in resize code
   */
  algResizeLayerParams->inputPadRefillZeros = 0;
  int32_t resizeInDataId = commonParams->net->TIDLLayers[layerIdx].inData[0];
  int32_t numConsumersResizeId = TIDL_getNumConsumersForDataId(commonParams->net, resizeInDataId);
  if ((numConsumersResizeId > 1) && (resizeLayerParams->mode == TIDL_ResizeBilinear))
  {
    /* Top and bottom row duplication is required only for bilinear resize, so refilling it with zero padding also will be required only for bilinear resize */
    algResizeLayerParams->inputPadRefillZeros = 1;
  }

  if ((flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_deviceCommonRefInit(layerSpecificParams,
                                      commonParams,
                                      algLayer,
                                      layerIdx,
                                      memory,
                                      memorySize,
                                      outPtr);
  }
  else
  {

    int32_t currOffset = 0;
    algResizeLayerParams->scratchSize = TIDL_resizeGetPermuteBufSize(commonParams->net, layerIdx);

    TIDL_resizeUpdateWorkload((sWorkloadUnit_t *)algLayer->workloadUnit, algLayer->gcHelperHandle, tidlLayer);
    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_L3_SCRATCHSIZE_MAX)
    {
      /* Forcing L3 level memory scratchSize to max value to make permutePatternBufInt as NULL */
      algResizeLayerParams->scratchSize = (int32_t)TIDL_DEFAULT_L3_MEM_SIZE + 1;
    }
    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_DDR_SCRATCHSIZE_MAX)
    {
      /* Forcing DDR level memory scratchSize to max value to make permutePatternBufInt as NULL */
      algResizeLayerParams->scratchSize = (int32_t)TIDL_WT_STAGING_MEM * ALG_SCRATCH_DDR_MEM_MEMREC;
    }
    /* Initialization of this buffer will be taken care of by the device init routine */
    status = TIDL_resizeAllocPermuteBufMem(commonParams,
                                           layerIdx,
                                           algLayer,
                                           algResizeLayerParams->scratchSize,
                                           &currOffset,
                                           memory);

    /* APIs calculate offset with regard to base ptr. In this situation, the device utils API would not
     * know that the memory has already been utilized. Thus we add an offset equal to the size of memory
     * space already allocated and remove the offset after the API has run so the caller function sees the
     * same memory.
     **/
    memory[TIDL_LAYER_MEMORY_PERSISTENT] += TIDL_ALIGN_CEIL(currOffset, TIDL_ALIGNMENT_SIZE);

    if (status == TIDL_SUCCESS)
    {
      status = TIDL_deviceUtilsCommonInit(layerSpecificParams,
                                          commonParams,
                                          algLayer,
                                          layerIdx,
                                          memory,
                                          memorySize,
                                          outPtr);
    }

    memory[TIDL_LAYER_MEMORY_PERSISTENT] -= TIDL_ALIGN_CEIL(currOffset, TIDL_ALIGNMENT_SIZE);
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += TIDL_ALIGN_CEIL(currOffset, TIDL_ALIGNMENT_SIZE);
  }
  return status;
}

static inline void TIDL_resizeDmaPadOverwrite(const void *dst,
                                              const void *src,
                                              int32_t icnt0,
                                              int32_t icnt1,
                                              int32_t dim1,
                                              int32_t dim2,
                                              void *dmaUtilsContext,
                                              uint8_t *trMem)
{
  (void)TIDL_prePareMemcpyTr(trMem); //: TODO: Remove this
  (void)TIDL_memcpy2D(dst, src, icnt0, icnt1, dim1, dim2, dmaUtilsContext, trMem);
}

static int32_t TIDL_resizeDspProcessNew(TIDL_NetworkCommonParams *commonParams,
                                        sTIDL_AlgLayer_t *algLayer,
                                        sTIDL_Layer_t *tidlLayer,
                                        void *inPtrs[],
                                        void *outPtrs[],
                                        int32_t layerIdx)
{
  int32_t status = TIDL_SUCCESS;
  uint8_t trMem[64] = {0};
  uint8_t *inPtrOrig;
  uint8_t *inPtr;

  sTIDL_DataParams_t *inDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
  sTIDL_ResizeLayerParams_t *params = &tidlLayer->layerParams.resizeParams;
  sTIDL_ALgResizeLayer_t *resizeParams = &algLayer->layerParams.resizeParams;

  int32_t inElmtSize = TIDL_getDatElementSize(inDataParams->elementType);
  int32_t inPitch = inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inPitchBytes = inPitch * inElmtSize;
  int32_t resizeNumChannels = 0, resizeInChPitch, resizeInHeight = 0, resizeInWidth, resizeInWidthBytes = 0, resizeInChPitchBytes = 0;
  int32_t bPtrOffset = 0;

  int32_t leftPadResize = 1;
  int32_t check_flag = 0;

  sLink_t *linkPtrList[MAX_LINKS_PER_WL];

  /* Might affect performance since variables are in DDR */
  for (int32_t linkIdx = 0; (linkIdx < algLayer->workloadUnit->numLinks) && (check_flag == 0); linkIdx++)
  {
    getLinkPtrs(algLayer->workloadUnit, NOT_VALID, linkIdx, linkPtrList);
    const sLink_t *link = linkPtrList[0];
    /* Resize layer will always have one input */
    sBufParams_t *bufParams = getBufParamsFromBufIndex(commonParams->gcHelperHandle, link->src[0].bufDBindex);
    if (bufParams->type == (int32_t)BUF_FM_FULL)
    {
      resizeNumChannels = bufParams->metaParams.featMetaParams.channel; /* TODO: Handle correctly for channel split */
      resizeInChPitch = bufParams->bufWidth;
      resizeInWidth = bufParams->metaParams.featMetaParams.width;
      resizeInChPitchBytes = resizeInChPitch * inElmtSize;
      resizeInWidthBytes = resizeInWidth * inElmtSize;

      bPtrOffset = link->src[0].offset * inElmtSize;
      check_flag = 1;
    }
    else
    {
      status = TIDL_ERR_FAILURE;
      break;
    }
  }

  if (status == TIDL_SUCCESS)
  {
    (void)TIDL_prePareMemcpyTr(trMem);
    /* permutePatternBufInt is not part of handle, so generic handle wb does not cover this, hence separate writeback required */
    (void)TIDL_memcpy2DCacheWbInv((uint8_t *)resizeParams->permutePatternBufInt,
                                  (uint8_t *)resizeParams->permutePatternBuf,
                                  resizeParams->scratchSize,
                                  1,
                                  0,
                                  0,
                                  commonParams->tidlCommonParams->dmaUtilsContext,
                                  trMem,
                                  commonParams->tidlCommonParams->sysMems);
    /*for non otf cases we required 1 pad in the left but not for otf*/
    #ifdef __C7504__
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> NOT_IN_SCPOE: This code is not supported for AM62A and is maintained for other devices.
    <justification end> */
    #endif
    if (TIDL_isPadOTF(commonParams->net->deviceName) != 0U)
    #ifdef __C7504__
    /* LDRA_JUSTIFY_END */
    #endif
    {
      leftPadResize = 0;
    }

    /* Here inPtrs[0] is expected to be A pointer and hence, we must derive B offset */
    inPtrOrig = ((uint8_t *)inPtrs[0]) + bPtrOffset;
    inPtr = ((uint8_t *)inPtrOrig) + (((1 * inPitch) + leftPadResize) * inElmtSize);

    sTIDL_ResizeInfo_t resizeInfo = {0, 0};
    if (params->mode == TIDL_ResizeBilinear)
    {
      WorkloadUnitExec_CommonParams commonParamsLocalCpy = *commonParams->tidlCommonParams;
      sWLMetaDataID_t wlMetaData = {0, 0, 0, 0, 0, 0, 0};
      getWLMetaDataID(algLayer->workloadUnit->metadata, &wlMetaData);
      resizeInHeight = wlMetaData.inTileHeight;

      getResizeInfo(&resizeInfo, algLayer, tidlLayer, commonParams->createParams, resizeInHeight);

      resizeInHeight = resizeInfo.inputHeight;

      /* Define arrays for source and destination pointer offsets */
      const int32_t lineOffsets[2] = {0, (resizeInHeight + 1) * inPitchBytes};
      const int32_t srcOffsets[2] = {0, (resizeInHeight - 1) * inPitchBytes};

      /* Define the line types to process */
      const uint32_t lineTypes[2] = {TIDL_COPY_TOP_LINE, TIDL_COPY_BOTTOM_LINE};

      /* Loop through both line types */
      for (int32_t i = 0; i < 2; i++)
      {
        uint8_t *dstPtr = inPtrOrig + (inElmtSize * leftPadResize) + lineOffsets[i];
        uint8_t *srcPtr = inPtr + srcOffsets[i];

        /* Perform the operation if the line type is set in copyLineInfo */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if ((resizeInfo.copyLineInfo & lineTypes[i]) == lineTypes[i])
        /* LDRA_JUSTIFY_END */
        {
          TIDL_resizeDmaPadOverwrite(dstPtr,
                                     srcPtr,
                                     resizeInWidthBytes,
                                     resizeNumChannels,
                                     resizeInChPitchBytes,
                                     resizeInChPitchBytes,
                                     commonParamsLocalCpy.dmaUtilsContext,
                                     trMem);
        }
      }
    }

    status = TIDL_deviceUtilsCommonProcess(commonParams,
                                           algLayer,
                                           tidlLayer,
                                           inPtrs,
                                           outPtrs,
                                           layerIdx);

    if (resizeParams->inputPadRefillZeros == 1)
    {
      WorkloadUnitExec_CommonParams commonParamsLocalCpy = *commonParams->tidlCommonParams;
      int32_t remainingTransferSize = resizeInWidthBytes;

      /* Replace the padding region duplicated with top line back to zeroes */
      while (remainingTransferSize > 0)
      {
        int32_t transferSize = remainingTransferSize;

        if ((uint32_t)transferSize > TIDL_1K_ZERO_VECTOR)
        {
          transferSize = TIDL_1K_ZERO_VECTOR;
        }

        /* Define arrays for destination pointer offsets */
        const int32_t lineOffsets[2] = {0, (resizeInHeight + 1) * inPitchBytes};

        /* Define the line types to process */
        const uint32_t lineTypes[2] = {TIDL_COPY_TOP_LINE, TIDL_COPY_BOTTOM_LINE};

        /* Loop through both line types */
        for (int32_t i = 0; i < 2; i++)
        {
          uint8_t *dstPtr = inPtrOrig + (inElmtSize * leftPadResize) + lineOffsets[i];

          /* Perform the operation if the line type is set in copyLineInfo */
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          if ((resizeInfo.copyLineInfo & lineTypes[i]) == lineTypes[i])
          /* LDRA_JUSTIFY_END */
          {
            TIDL_resizeDmaPadOverwrite(dstPtr,
                                       commonParams->tidlCommonParams->zeroVector1k,
                                       transferSize,
                                       resizeNumChannels,
                                       resizeInChPitchBytes,
                                       0,
                                       commonParamsLocalCpy.dmaUtilsContext,
                                       trMem);
          }
        }
        remainingTransferSize -= transferSize;
      }
    }
  }
  return status;
}

int32_t TIDL_resizeProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t *algLayer,
                              sTIDL_Layer_t *tidlLayer,
                              void *inPtrs[],
                              void *outPtrs[],
                              int32_t layerIdx)
{
  int32_t status = IALG_EOK;
  uint32_t flowCtrl = commonParams->createParams->flowCtrl;

  if (algLayer->layerParams.resizeParams.isOptResizeAvail == 0)
  {
    flowCtrl = flowCtrl | TIDL_FLOW_CTRL_REF_ONLY;
  }

  /* Setup various pointers needed by convolution layers */
  if ((flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
#ifdef HOST_EMULATION
    if ((commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_PROCESSSP_WIDTHRATIO_ZERO) || (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_RESIZE_FORCE_PROCESSSP_HEIGHTRATIO_ZERO))
    {
      commonParams->createParams->net->weightsElementSize = 4;
    }
#endif
    status = TIDL_resizeRefProcess(commonParams->createParams,
                                   algLayer,
                                   tidlLayer,
                                   inPtrs,
                                   outPtrs,
                                   layerIdx);
  }
  else
  {
    status = TIDL_resizeDspProcessNew(commonParams,
                                      algLayer,
                                      tidlLayer,
                                      inPtrs,
                                      outPtrs,
                                      layerIdx);
  }

  return status;
}
