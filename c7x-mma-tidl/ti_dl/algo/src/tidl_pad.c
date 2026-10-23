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
@file    tidl_pad.c
@brief   This file defines private functions for pad layer.
@version 0.1 (June 2019) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_pad.h"
#include "tidl_commonUtils.h"
#include "tidl_pad_ref.h"

int32_t TIDL_padAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                         const TIDL_NetworkCommonParams *commonParams,
                         int32_t layerIdx,
                         int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  int32_t outDataSize = TIDL_updateDataParamsPitch(&commonParams->net->TIDLLayers[layerIdx].outData);
  sTIDL_PadLayerParams_t *layerParams = &commonParams->net->TIDLLayers[layerIdx].layerParams.padLayerParams;
  int32_t paramMemSize = layerParams->perChannelPadConstTensorSize;

  memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] = (uint32_t)memorySize[TIDL_LAYER_MEMORY_PERSISTENT] + ((uint32_t)paramMemSize + 128U);
  memorySize[TIDL_LAYER_MEMORY_OUTPUT] = (uint32_t)memorySize[TIDL_LAYER_MEMORY_OUTPUT] + ((uint32_t)outDataSize + 128U);

  return status;
}

int32_t TIDL_padInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                        const TIDL_NetworkCommonParams *commonParams,
                        sTIDL_AlgLayer_t *algLayer,
                        int32_t layerIdx,
                        uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                        int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                        void **outPtr)
{
  int32_t status = IALG_EOK;

  int32_t outDataSize = TIDL_updateDataParamsPitch(&commonParams->net->TIDLLayers[layerIdx].outData);
  int32_t currOffset = 0;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_OUTPUT],
                   &currOffset,
                   outDataSize,
                   TIDL_ALIGNMENT_SIZE,
                   outPtr);

  sTIDL_PadLayerParams_t *layerParams = &commonParams->net->TIDLLayers[layerIdx].layerParams.padLayerParams;
  int32_t paramMemSize = layerParams->perChannelPadConstTensorSize;
  int32_t paramOffset = layerParams->perChannelPadConstTensorOffset;
  void *dst;
  currOffset = 0;

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                   &currOffset,
                   paramMemSize,
                   TIDL_ALIGNMENT_SIZE,
                   &dst);
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  if (dst != NULL)
  {
    /* LDRA_JUSTIFY_END */
    sTIDL_AlgPadParams_t *algPadParams = &algLayer->layerParams.padParams;
    algPadParams->perChannelPadConstTensorPtr = dst;
    algPadParams->perChannelPadConstTensorSize = paramMemSize;

    memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] = (uint32_t)memorySize[TIDL_LAYER_MEMORY_PERSISTENT] + ((uint32_t)paramMemSize + 128U);
    memorySize[TIDL_LAYER_MEMORY_OUTPUT] = (uint32_t)memorySize[TIDL_LAYER_MEMORY_OUTPUT] + ((uint32_t)outDataSize + 128U);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if ((paramMemSize != 0) && (paramOffset != 0))
    {
      /* LDRA_JUSTIFY_END */
      void *src = ((int8_t *)(commonParams->createParams->net) + paramOffset);
      (void)memcpy(dst, src, paramMemSize);
    }
  }
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  else
  {
    status = IALG_EFAIL;
  }
  /* LDRA_JUSTIFY_END */
  return status;
}

int32_t TIDL_padProcessNew(TIDL_NetworkCommonParams *commonParams,
                           sTIDL_AlgLayer_t *algLayer,
                           sTIDL_Layer_t *tidlLayer,
                           void *inPtrs[],
                           void *outPtrs[],
                           int32_t layerIdx)
{
  int32_t status = IALG_EOK;

  #ifdef HOST_EMULATION
  const sTIDL_DataParams_t *inDataParams   = &commonParams->createParams->net->TIDLLayers[tidlLayer->inData[0]].outData;

  sTIDL_AlgPadParams_t* algPadParams = &algLayer->layerParams.padParams;
  sTIDL_PadLayerParams_t *params = &tidlLayer->layerParams.padLayerParams;
  int32_t padConstValue = 0;
#ifdef PERF_MODELLING
  double perfDmaDatawl[1]= { 0.0 };
  uint64_t perfDmaBWDatawl[6]= { 0 };
  uint64_t *profilePoint = (uint64_t *)&algLayer->metaData.profilePoint[0];
  DmaUtilsAutoInc3d_resetPerfData();
  DmaUtilsAutoInc3d_resetDmaMemBwData();
#endif
  if (params->padType == (int32_t)TIDL_PadZero)
  {
    padConstValue = 0;
  }
#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
  else if (params->padType == (int32_t)TIDL_PadNonZeroConstant)
  {
    padConstValue = params->padConstValue;
  }
#endif
  else
  {
    /*DO Nothing*/
  }

  int32_t inPtrOffset = 0;
  int32_t outPtrOffset = 0;

  float32_tidl *perChannelPadValPtr = (float32_tidl *)((int8_t *)algPadParams->perChannelPadConstTensorPtr);

  if (((uint32_t)commonParams->createParams->flowCtrl & (uint32_t)TIDL_FLOW_CTRL_REF_STAT) == (uint32_t)TIDL_FLOW_CTRL_REF_STAT)
  {
    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;
    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

    TIDL_UpdateScaleFactors(&intAlgObj, layerIdx, 0, 0, 1);
  }

  /* Non zero padding for NATC flow done using reference code, zero padding done using DMA */
  if ((((uint32_t)commonParams->createParams->flowCtrl & (uint32_t)TIDL_FLOW_CTRL_DSP_NATC) == (uint32_t)TIDL_FLOW_CTRL_DSP_NATC) && (params->padType != (int32_t)TIDL_PadZero))
  {
    /* TIDL_LDRA_TAG_PAD_PRIOR_CHECK_001 */
#ifdef TIDL_COVERAGE_DEAD_CODE
    if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
      TIDL_padNATCProcess((float32_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else
#endif
        if ((inDataParams->elementType == TIDL_SignedChar))
    {
      TIDL_padNATCProcess((int8_t *)inPtrs[0], (int8_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_UnsignedChar))
    {
      TIDL_padNATCProcess((uint8_t *)inPtrs[0], (uint8_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_SignedShort))
    {
      TIDL_padNATCProcess((int16_t *)inPtrs[0], (int16_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_UnsignedShort))
    {
      TIDL_padNATCProcess((uint16_t *)inPtrs[0], (uint16_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else
    {
      /*Do Nothing*/
    }
  }
#endif
#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & (uint32_t)TIDL_FLOW_CTRL_REF_ONLY) != (uint32_t)0U)
  {
    if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
      TIDL_padRefProcess((float32_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_BFloat16))
    {
      TIDL_padRefProcess((bfloat16_tidl *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_SignedChar))
    {
      TIDL_padRefProcess((int8_t *)inPtrs[0], (int8_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_UnsignedChar))
    {
      TIDL_padRefProcess((uint8_t *)inPtrs[0], (uint8_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_SignedShort))
    {
      TIDL_padRefProcess((int16_t *)inPtrs[0], (int16_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else if ((inDataParams->elementType == TIDL_UnsignedShort))
    {
      TIDL_padRefProcess((uint16_t *)inPtrs[0], (uint16_t *)outPtrs[0], inPtrOffset, outPtrOffset, inDataParams, &tidlLayer->outData, params, padConstValue, perChannelPadValPtr);
    }
    else
    {
      /*Do Nothing*/
    }
  }
  else
#endif
  {
    /* TIDL_LDRA_TAG_DEVICEUTILS_PRIOR_CHECK_001 */
    status = TIDL_padDspProcessNew(commonParams, algLayer, tidlLayer, inPtrs, outPtrs, commonParams->tidlCommonParams->sysMems);
    /* This layer does not use standard WL flow, so need to handle layer level cache writeback explicitly in this layer */
    TIDL_L1DCacheWbInv();
  }
#ifdef PERF_MODELLING
  DmaUtilsAutoInc3d_getDmaPerformance(&perfDmaDatawl[0]);
  DmaUtilsAutoInc3d_getDmaMemBwData(&perfDmaBWDatawl[0]);

  /* --- Pad layer overhead constants (derived from J784S4 HW measurements) ---
   * PAD_OH_DMA_SETUP          : DMA API setup: main-copy + row-fill type7/type8 calls = 5189 cy
   * PAD_OH_COLFILL_WRAP_PERCH : WrapAround scalar loop cost per output channel = 167 cy/ch
   *                             Inner loop per channel: (padT+1+padB+1)×(padL+padR) scalar stores.
   *                             Measured lyr=152 (padT=padB=padL=padR=1, Ni=128):
   *                             params=21434 cy / 128 ch = 167.4 cy/ch
   *                             Only applies when padType == TIDL_PadWrapAround.
   * PAD_OH_COLFILL_TMPL       : SA0 template setup always run after WrapAround block:
   *                             tmpl(166) + sa0Open(2) + sa0Close(2)             = 170 cy
   * PAD_OH_SYNC_WB            : spad + dmaWait + l1dWbInv                       = 1066 cy
   *
   * SA0 loop model:
   *   cycles/iter = TIDL_getWriteAccessCycles(basePtr, 64) x 2
   *   The x2 is for the predicated VSTORE: every iteration hits DECDIM1 (numColPad < simdW)
   *   → partial vector store → hardware RMW on the destination cache line.
   *   TIDL_getWriteAccessCycles adds an additional alignment penalty (+1 or +2 blocks)
   *   for DDR when the store address is not 64B-aligned.
   */
  const double PAD_OH_DMA_SETUP          = 5189.0;
  const double PAD_OH_COLFILL_WRAP_PERCH = 167.0;
  const double PAD_OH_COLFILL_TMPL       = 170.0;
  const double PAD_OH_SYNC_WB            = 1066.0;

  /* FillPaddedCols SA0 loop cycles: ceil(numColPad/simdW) x Ni x fillH x cyclesPerStore */
  int32_t padElemSz  = TIDL_getDatElementSize(tidlLayer->outData.elementType);
  int32_t simdWidth  = 64 / padElemSz;
  int32_t numColPad  = params->padL + params->padR + (int32_t)tidlLayer->outData.padW;
  int32_t Ni_pad     = algLayer->wlPadParams.outChannels;
  int32_t fillH      = inDataParams->dimValues[TIDL_DIM_HEIGHT] + 1;
  int32_t numPadVecs = (numColPad > 0) ? ((numColPad + simdWidth - 1) / simdWidth) : 0;

  /* Compute SA0 basePtr address: same offset as TIDL_DspFillPaddedColsPadLayer computes
   * ptrOffset = linePitch*(padT + outDataPadH + 1) - padR (in elements), then × elemSz */
  int32_t linePitch  = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  int32_t ptrOff     = (linePitch * (params->padT + (int32_t)tidlLayer->outData.padH + 1))
                       - params->padR;
  const void *basePtr = (const uint8_t *)outPtrs[0] + (ptrOff * padElemSz);

  /* cyclesPerStore = getWriteCycles(64B, with alignment RMW) x 2 (predicated VSTORE RMW) */
  double cyclesPerStore = TIDL_getWriteAccessCycles(basePtr, 64U) * 2.0;

  double colFillLoop = (double)(numPadVecs * Ni_pad * fillH) * cyclesPerStore;

  /* WrapAround scalar loop: runs once per output channel regardless of numColPad */
  double wrapOH    = ((uint32_t)params->padType == (uint32_t)TIDL_PadWrapAround) ?
                     (PAD_OH_COLFILL_WRAP_PERCH * (double)Ni_pad) : 0.0;
  /* SA0 template+open+close always execute inside TIDL_DspFillPaddedColsPadLayer */
  double colFillOH = wrapOH + PAD_OH_COLFILL_TMPL;

  double totalOverhead = PAD_OH_DMA_SETUP + colFillOH + colFillLoop + PAD_OH_SYNC_WB;

  TIDL_updateprofileData(profilePoint, TIDL_PROFILE_CORE_LOOP,    0,
                         perfDmaDatawl[0] + totalOverhead);
  TIDL_updateprofileData(profilePoint, TIDL_PROFILE_DDR_BW_READ,  0,
                         perfDmaBWDatawl[DMA_DDR_READ_IDX]);
  TIDL_updateprofileData(profilePoint, TIDL_PROFILE_DDR_BW_WRITE, 0,
                         perfDmaBWDatawl[DMA_DDR_WRITE_IDX]);
#endif

  return status;
}
