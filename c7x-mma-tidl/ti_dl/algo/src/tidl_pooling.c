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
 *  \file tidl_pooling.c
 *
 *  \brief This file defines kernel functions for pooling layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_pooling.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include <limits>
#include <math.h>
#include "tidl_device_functions.h"
#include "tidl_pooling_ref.h"
#include "tidl_forceNegativeTest.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* Function that sets the startRowNumberInTensor member of algLayer needed by the maxPool kernel */
static inline void TIDL_poolingSetPrivAlgArgs(sTIDL_AlgLayer_t *algLayer, const TIDL_CreateParams *createParams, int32_t layerIdx)
{
  algLayer->layerParams.poolParams.startRowNumberInTensor = 0;

#if TIDL_DEVICE_MULTICORE
  uint32_t spatialMultiCoreMode, curCoreisMiddleCore, curCoreisLastCore;
  uint32_t coreId = createParams->coreId;
  uint32_t coreStartIdx = createParams->coreStartIdx;
  uint32_t coreEndIdx = coreStartIdx + createParams->net->numCores - 1U;
  uint32_t multiCoreMode = createParams->net->TIDLLayers[layerIdx].multiCoreMode;

  spatialMultiCoreMode = ( (multiCoreMode & (int32_t)TIDL_MULTI_CORE_SPATIAL)  == (int32_t)TIDL_MULTI_CORE_SPATIAL);
  curCoreisMiddleCore = ((coreId != coreStartIdx) && (coreId != coreEndIdx));
  curCoreisLastCore = (coreId == coreEndIdx);
#endif
  if (algLayer->workloadUnit != NULL)
  {
    /* If normal tile (except for normal tile with no preceeding first tile or last tile */
    if ((((algLayer->wlPadParams.tileType == NO_TILE) || (algLayer->wlPadParams.isFirstTile == 0U)) && (algLayer->wlPadParams.isLastTile == 0U))
#if TIDL_DEVICE_MULTICORE
        || (spatialMultiCoreMode && curCoreisMiddleCore) /*middle Cores*/
#endif
    )
    {
      /* Ideally this variable would be assigned the value of the prev tile height. In workload flow
        this value is inaccesible */
      algLayer->layerParams.poolParams.startRowNumberInTensor = 1;
    }
    else if (((algLayer->wlPadParams.tileType != NO_TILE) && (algLayer->wlPadParams.isLastTile != 0U))
#if TIDL_DEVICE_MULTICORE
             || (spatialMultiCoreMode && curCoreisLastCore) /*Last Core*/
#endif
    )
    {
      sWLMetaDataID_t wlMetaData = {0, 0, 0, 0, 0, 0, 0};
      getWLMetaDataID(algLayer->workloadUnit->metadata, &wlMetaData);
      algLayer->layerParams.poolParams.startRowNumberInTensor = algLayer->layerParams.poolParams.orgInTensorHeight -
                                                                wlMetaData.inTileHeight;
    }
    else
    {
      /*do nothing */
    }
  }
}

/**
*******************************************************************************
*  \brief      Function to supply memory requirement for Pooling layer to user
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
int32_t TIDL_poolingAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX])

{
  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  else
#endif
  {
    status = TIDL_poolingDspAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }

  return status;
}

/**
*******************************************************************************
*  \brief      Function to set memory needed by convolution layer
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
int32_t TIDL_poolingInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            sTIDL_AlgLayer_t *algLayer,
                            int32_t layerIdx,
                            uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                            void **outPtr)

{
  int32_t status = IALG_EOK;
  sTIDL_Layer_t *tidlLayer = &commonParams->net->TIDLLayers[layerIdx];
  sTIDL_PoolingParams_t *poolParams = &tidlLayer->layerParams.poolParams;
  int32_t procElemSize = TIDL_getProcessingElementSizeInBytes(tidlLayer);

  TIDL_poolingSetPrivAlgArgs(algLayer, commonParams->createParams, layerIdx);

  /* The lines below must match the same lines in function TIDL_UpdateScaleFactors()
   * Local average pooling 2x2, 3x3, with stride 2 implement an optimized kernel that assumes the scale factors bit-depth are
   * the same as the input data type, which is either 8-bits or 16-bits.
   * The remaining local average pooling assume 16-bits or 32-bits scale factors.
   * NOTE: This has been edited to match workload flow which handles st differently*/

  if ((poolParams->poolingType == TIDL_AveragePooling) &&
      ((poolParams->strideW == 2) && (poolParams->strideH == 2) &&
       (((poolParams->kernelW == 2) && (poolParams->kernelH == 2)) ||
        ((poolParams->kernelW == 3) && (poolParams->kernelH == 3)))))
  {
    /* Local average pooling 2x2, 3x3, with stride 2 implement an optimized kernel that assumes the scale factors bit-depth are
     * the same as the input data type, which is either 8-bits or 16-bits.*/
    if (procElemSize == 1)
    {
      algLayer->layerParams.poolParams.internalPoolingWeightQ = (int32_t)TIDL_INTERNAL_POOLING_WEIGHT_Q_U8;
    }
    else
    {
      algLayer->layerParams.poolParams.internalPoolingWeightQ = (int32_t)TIDL_INTERNAL_POOLING_WEIGHT_Q_U16; /* To avoid overflow, we set it 12-bits instead fo 16-bits */
    }
  }
  else
  {
    /** The remaining local average pooling assume 16-bits or 32-bits scale factors.*/
    algLayer->layerParams.poolParams.internalPoolingWeightQ = (int32_t)TIDL_INTERNAL_POOLING_WEIGHT_Q_U16;
  }

  (void)memset(&algLayer->layerParams.poolParams.virtualPad, 0, sizeof(algLayer->layerParams.poolParams.virtualPad));

  //: TODO: Need to handle ST
  if ((TIDL_isPadOTF(commonParams->net->deviceName) == TRUE) && (algLayer->workloadUnit != NULL))
  {
    algLayer->layerParams.poolParams.virtualPad.padT = algLayer->workloadUnit->procParam.padVirtual[TOP_DIR];
    algLayer->layerParams.poolParams.virtualPad.padL = algLayer->workloadUnit->procParam.padVirtual[LEFT_DIR];
    algLayer->layerParams.poolParams.virtualPad.padR = algLayer->workloadUnit->procParam.padVirtual[RIGHT_DIR];
    algLayer->layerParams.poolParams.virtualPad.padB = algLayer->workloadUnit->procParam.padVirtual[BOTTOM_DIR];
  }

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DEVICE_UTILS_FORCE_OUTPRT_NULL)
    {
      outPtr = NULL; /* Forcing outPtr to NULL to check NULL pointer exception error */
    }

    status = TIDL_deviceCommonRefInit(layerSpecificParams,
                                      commonParams,
                                      algLayer,
                                      layerIdx,
                                      memory,
                                      memorySize,
                                      outPtr);
  }
  else
#endif
  {
    status = TIDL_poolingDspInit(layerSpecificParams,
                                 commonParams,
                                 algLayer,
                                 layerIdx,
                                 memory,
                                 memorySize,
                                 outPtr);
  }

  return status;
}

int32_t TIDL_poolingProcessNew(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)

{
  int32_t status = IALG_EOK;

/* For low latency mode set start row number to zero for first core for other
core value is set inside TIDL_poolingSetPrivAlgArgs function */
#if TIDL_DEVICE_MULTICORE
  if (commonParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
  {
    //: TODO: Need to handle ST with Low latency
    if (commonParams->createParams->coreId == (commonParams->createParams->coreStartIdx))
    {
      algLayer->layerParams.poolParams.startRowNumberInTensor = 0;
    }
  }
  /* If normal tile (except for normal tile with no preceeding first tile or last tile */
  else
#endif
      if ((algLayer->wlPadParams.isFirstTile == 1U) && (algLayer->wlRepeatIter == 0))
  {
    algLayer->layerParams.poolParams.startRowNumberInTensor = 0;
  }
  else
  {
    /*DO Nothing*/
  }

/* Setup various pointers needed by convolution layers */
#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_poolingRefProcess(commonParams,
                                    algLayer,
                                    tidlLayer,
                                    inPtrs,
                                    outPtrs,
                                    layerIdx);
  }
  else
#endif
  {
    if ((int32_t)commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_POOLING_FORCE_WORKLOADUNIT_NULL)
    {
      algLayer->workloadUnit = NULL;
    }
    status = TIDL_poolingDspProcessNew(commonParams,
                                       algLayer,
                                       tidlLayer,
                                       inPtrs,
                                       outPtrs,
                                       layerIdx);
  }

#if TIDL_DEVICE_MULTICORE
  if (commonParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
  {
    //: TODO: Need to handle this when low latency mode is supported with super tiling
  }
  else
  {
#endif
    algLayer->layerParams.poolParams.startRowNumberInTensor += tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
#if TIDL_DEVICE_MULTICORE
  }
#endif

  return status;
}
