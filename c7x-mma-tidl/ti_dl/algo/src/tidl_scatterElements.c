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
 *  \file tidl_scatterElements.c
 *
 *  \brief This file defines kernel functions for Batch Norm layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_scatterElements.h"
#include <math.h>
#include <limits>
#include "tidl_scatterElements_ref.h"
#include "tidl_forceNegativeTest.h"

// #define TIDL_REF_BATCH_NORM_DEBUG
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TIDL_SCATTERND_DST_TO_SRC_TABLE_SIZE_DDR (200U * 1024U)

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

int32_t TIDL_scatterElementsAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                     const TIDL_NetworkCommonParams *commonParams,
                                     int32_t layerIdx,
                                     int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    int32_t scratchDataSize = 0;
    int32_t outDataSize;
    int32_t paramMemSize = 0;

    outDataSize = TIDL_refGetOutDataSize(commonParams, layerSpecificParams, layerIdx);

    sTIDL_Layer_t *tidlLayer = &commonParams->createParams->net->TIDLLayers[layerIdx];
    int32_t outChPitch = (int32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
    int32_t outChs = (int32_t)tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
    size_t size = TIDL_getDatElementSize(tidlLayer->outData.elementType);
    int32_t accSize = 2U * sizeof(size);
    scratchDataSize = outChs * outChPitch * accSize;

    memorySize[TIDL_LAYER_MEMORY_SCRATCH] += (scratchDataSize + TIDL_ALIGNMENT_SIZE);
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += (paramMemSize + TIDL_ALIGNMENT_SIZE);
    memorySize[TIDL_LAYER_MEMORY_OUTPUT] += (outDataSize + TIDL_ALIGNMENT_SIZE);
  }
  else
#endif
  {
    int32_t handleSize = 0;
    int32_t scratchMemSize = TIDL_SCATTERND_DST_TO_SRC_TABLE_SIZE_DDR; // TODO: Protect for ScatterND sum.
    handleSize = TIDL_getScatterElementsKernelHandleSize();

    memorySize[TIDL_LAYER_MEMORY_SCRATCH] += scratchMemSize;
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += handleSize;
    memorySize[TIDL_LAYER_MEMORY_OUTPUT] += 0;
  }

  return status;
}

int32_t TIDL_scatterElementsInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                    const TIDL_NetworkCommonParams *commonParams,
                                    sTIDL_AlgLayer_t *algLayer,
                                    int32_t layerIdx,
                                    uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                    int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                    void **outPtr)
{
  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_SCATTER_FORCE_OUTPTR_NULL) /* Forcing outPtr to NULL to check the null pointer exception case */
    {
      outPtr = NULL;
    }
    status = TIDL_scatterElementsRefInit(layerSpecificParams,
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
    int32_t handleSize = 0;
    void *handle;
    int32_t currOffset = 0;
    sTIDL_Layer_t *tidlLayer = &commonParams->createParams->net->TIDLLayers[layerIdx];

    handleSize = TIDL_getScatterElementsKernelHandleSize();

    TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                     &currOffset,
                     handleSize,
                     TIDL_ALIGNMENT_SIZE,
                     &handle);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
    <justification end> */
    if (handle != NULL)
    {
      /* LDRA_JUSTIFY_END */
      algLayer->workloadHandle = handle;

      if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_SCATTERND_AXIS_AND_DATATYPE_ERROR)
      {
        tidlLayer->layerParams.scatterElementsParams.axis = 0;
      }

      status = TIDL_scatterElementsKernelInitNew(commonParams->createParams,
                                                 algLayer,
                                                 tidlLayer,
                                                 handle);

      memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += handleSize;
      memorySize[TIDL_LAYER_MEMORY_OUTPUT] += 0;
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
  }

  return status;
}

int32_t TIDL_scatterElementsProcessNew(TIDL_NetworkCommonParams *commonParams,
                                       sTIDL_AlgLayer_t *algLayer,
                                       sTIDL_Layer_t *tidlLayer,
                                       void *inPtrs[],
                                       void *outPtrs[],
                                       int32_t layerIdx)
{
  int32_t status = IALG_EOK;
  /*It is import tool duty to provide in data in following order
  data    = 0;
  indices = 1;
  update  = 2;
  this order is same as onnx operator
  update is treated as feature, indices are treated as weights. data is treated as initializer.
  */

  sTIDL_sysMemHandle_t *sysMems = commonParams->tidlCommonParams->sysMems;

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    int32_t dataIdx = 0;
    int32_t indicesIdx = 1;
    int32_t voxelIdx = 2;
    uint8_t (*data)[] = (uint8_t (*)[])(inPtrs[dataIdx]);
    uint8_t (*indices)[] = (uint8_t (*)[])(inPtrs[indicesIdx]);
    uint8_t (*update)[] = (uint8_t (*)[])(inPtrs[voxelIdx]);

    int8_t (*outPtr)[] = (int8_t (*)[])(outPtrs[0]);
    sTIDL_DataParams_t *inDataParams;
    sTIDL_DataParams_t *inIndicesParams;
    sTIDL_DataParams_t *inUpdateParams;

    inDataParams   = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
    inIndicesParams= &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[1]].outData;
    inUpdateParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[2]].outData;

    sTIDL_ScatterElementsParams_t *params = &tidlLayer->layerParams.scatterElementsParams;

    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;
    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_SCATTER_FORCE_32B_DATATYPE) /* forcing elementType to TIDL_UnsignedWord to check for negative case */
    {
      inDataParams->elementType = TIDL_UnsignedWord;
    }

    status = TIDL_scatterElementsRefProcess(&intAlgObj,
                                            algLayer,
                                            tidlLayer,
                                            params,
                                            data,
                                            indices,
                                            update,
                                            outPtr,
                                            inDataParams,
                                            inIndicesParams,
                                            inUpdateParams,
                                            &tidlLayer->outData);

    if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) != TIDL_FLOW_CTRL_REF_STAT)
    {
      int32_t LayerIdx = algLayer->layerIdx;
      TIDL_UpdateScaleFactors(&intAlgObj, LayerIdx, 0, 0, 1);
    }
  } // (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  else
#endif
  {

    status = TIDL_scatterElementsDspProcessNew(commonParams,
                                               algLayer,
                                               tidlLayer,
                                               inPtrs,
                                               outPtrs,
                                               sysMems);
    /* This layer does not use standard WL flow, so need to handle layer level cache writeback explicitly in this layer */
    if (status == TIDL_SUCCESS)
    {
      TIDL_L1DCacheWbInv();
    }
  }

  if (status == TIDL_SUCCESS)
  {
    status = WorkloadUnitExec_padBufferExec(outPtrs[0], algLayer->wlRepeatIter, tidlLayer, commonParams->tidlCommonParams,
                                            &algLayer->wlPadParams, algLayer->workloadUnit, algLayer->gcHelperHandle,
                                            tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH] * TIDL_getDatElementSize(tidlLayer->outData.elementType),
                                            tidlLayer->outData.pitch[TIDL_ROI_PITCH] * TIDL_getDatElementSize(tidlLayer->outData.elementType));
  }
  return status;
}
