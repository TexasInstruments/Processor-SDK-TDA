/*
 *
 * Copyright (c) {2022 - 2024} Texas Instruments Incorporated
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
@file    tidl_transpose.c
@brief   This file defines private functions for transpose layer.
@version 0.1 (June 2023) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_transpose.h"
#include "tidl_commonUtils.h"
#include "tidl_transpose_ref.h"
#include "tidl_forceNegativeTest.h"
/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_Transpose
 @fn         TIDL_transposeProcess
 @brief      Transpose Layer process function

 @param      intAlgHandle : tidl algorothm handle
 @param      algLayer : Ptr to alg layer parameter used in transpose layer
 @param      tidlLayer: Ptr to tidl layer parameter used in transpose layer
 @param      inPtrs: Ptrs to input buffers to be processed
 @param      outPtrs: Ptrs to output buffers to be processed
 @param      sysMems: Ptr to memory related buffers used in crop layer
 @remarks    None
 @return     IALG_EOK   - Successful
             IALG_EFAIL - Unspecified error
 ----------------------------------------------------------------------------
*/
int32_t TIDL_transposeProcessNew(TIDL_NetworkCommonParams *commonParams,
                                 sTIDL_AlgLayer_t *algLayer,
                                 sTIDL_Layer_t *tidlLayer,
                                 void *inPtrs[],
                                 void *outPtrs[],
                                 int32_t layerIdx)
{
  int32_t status = IALG_EOK;

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_TRANSPOSE_PROCESS_FORCE_NETVERSION)
  {
    commonParams->net->netVersion = 0x20241120;
  }
#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
  { 
    sTIDL_DataParams_t *inDataParams   = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;
    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

    if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
      TIDL_transposeRefProcess<float32_tidl, float32_tidl>((float32_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_UnsignedChar)
    {
      TIDL_transposeRefProcess<uint8_t, uint8_t>((uint8_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_SignedChar)
    {
      TIDL_transposeRefProcess<int8_t, int8_t>((int8_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_UnsignedShort)
    {
      TIDL_transposeRefProcess<uint16_t, uint16_t>((uint16_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_SignedShort)
    {
      TIDL_transposeRefProcess<int16_t, int16_t>((int16_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_SignedWord)
    {
      TIDL_transposeRefProcess<int32_t, int32_t>((int32_t *)inPtrs[0], (int32_t *)outPtrs[0], inDataParams, tidlLayer);
    }
    else
    {
      tidl_printf(0,"TIDL_TransposeLayer inElementType is  not supported !!!\n ");
      status = TIDL_ERR_FAILURE;
    }

    if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      TIDL_UpdateScaleFactors(&intAlgObj, algLayer->layerIdx, 0, 0, 1);
    }
  }
  else
#endif
  {

    if (commonParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
    {
      status = TIDL_deviceUtilsCommonProcess(commonParams,
                                             algLayer,
                                             tidlLayer,
                                             inPtrs,
                                             outPtrs,
                                             layerIdx);
    }
    else
    {
#if TIDL_NET_VERSION_FW_PASSIVE_CODE
      sTIDL_sysMemHandle_t *sysMems = commonParams->tidlCommonParams->sysMems;
      status = TIDL_transposeDspProcessNew(commonParams, algLayer, tidlLayer, inPtrs, outPtrs, sysMems);
      /* This layer does not use standard WL flow, so need to handle layer level cache writeback explicitly in this layer */
      if (status == IALG_EOK)
      {
        TIDL_L1DandL2CacheWbInv();
      }
#else
      status = IALG_EFAIL;
#endif
    }
  }

  return status;
}

int32_t TIDL_transposeAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                               const TIDL_NetworkCommonParams *commonParams,
                               int32_t layerIdx,
                               int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_TRANSPOSE_ALLOC_FORCE_NETVERSION)
  {
    commonParams->net->netVersion = 0x20241120;
  }
#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  else
#endif
  {
    if (commonParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
    {
      status = TIDL_deviceUtilsCommonAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
    }
    else
    {
#if TIDL_NET_VERSION_FW_PASSIVE_CODE
      int32_t handleSize = 0;
      sTIDL_Layer_t *tidlLayer = &commonParams->createParams->net->TIDLLayers[layerIdx];
      handleSize = TIDL_getTransposeKernelHandleSize(tidlLayer);

      memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += handleSize;
      memorySize[TIDL_LAYER_MEMORY_OUTPUT] += 0;
#else
      status = IALG_EFAIL;
#endif
    }
  }

  return status;
}

int32_t TIDL_transposeInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                              const TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t *algLayer,
                              int32_t layerIdx,
                              uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                              int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                              void **outPtr)
{
  int32_t status = IALG_EOK;

  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_TRANSPOSE_INIT_FORCE_NETVERSION)
  {
    commonParams->net->netVersion = 0x20241120;
  }

#ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
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
#endif
  {
    if (commonParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00)
    {
      status = TIDL_deviceUtilsCommonInit(layerSpecificParams,
                                          commonParams,
                                          algLayer,
                                          layerIdx,
                                          memory,
                                          memorySize,
                                          outPtr);
    }
    else
    {
#if TIDL_NET_VERSION_FW_PASSIVE_CODE
      int32_t handleSize = 0;
      void *handle;
      int32_t currOffset = 0;
      sTIDL_Layer_t *tidlLayer = &commonParams->createParams->net->TIDLLayers[layerIdx];

      handleSize = TIDL_getTransposeKernelHandleSize(tidlLayer);

      TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                       &currOffset,
                       handleSize,
                       TIDL_ALIGNMENT_SIZE,
                       &handle);

      algLayer->workloadHandle = handle;

      status = TIDL_transposeKernelInitNew(commonParams->createParams,
                                           algLayer,
                                           tidlLayer,
                                           handle);

      memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += handleSize;
      memorySize[TIDL_LAYER_MEMORY_OUTPUT] += 0;
#else
      status = IALG_EFAIL;
#endif
    }
  }
  return status;
}
