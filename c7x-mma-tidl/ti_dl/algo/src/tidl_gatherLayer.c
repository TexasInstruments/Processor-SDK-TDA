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
 *  \file tidl_gatherLayer.c
 *
 *  \brief This file defines kernel functions for Gather layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_gatherLayer.h"
#include <math.h>
#include <limits>
#include "tidl_gatherLayer_ref.h"

/**
 * @brief Gather layer Process function
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param inPtrs : Pointer to input buffers to be processed
 * @param outPtrs : Pointer to output buffers to be processed
 * @param sysMems : sysMem Handle
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_gatherLayerProcessNew(TIDL_NetworkCommonParams *commonParams,
                                   sTIDL_AlgLayer_t *algLayer,
                                   sTIDL_Layer_t *tidlLayer,
                                   void *inPtrs[],
                                   void *outPtrs[],
                                   int32_t layerIdx)
{

  int32_t status = TIDL_SUCCESS;

#ifdef HOST_EMULATION
  uint32_t flowCtrl = commonParams->createParams->flowCtrl; //  L3
  if (((uint32_t)flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
  {
    int32_t dataIdx = 0;
    int32_t indicesIdx = 1;
    uint8_t (*data)[] = (uint8_t (*)[])(inPtrs[dataIdx]);
    uint8_t (*indices)[] = (uint8_t (*)[])(inPtrs[indicesIdx]);
    int8_t (*outPtr)[] = (int8_t (*)[])(outPtrs[0]);

    sTIDL_GatherLayerParams_t *params = &tidlLayer->layerParams.gatherParams;
    sTIDL_DataParams_t *inDataParams;
    sTIDL_DataParams_t *inIndicesParams;

    inDataParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
    inIndicesParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[1]].outData;

    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;
    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

    status = TIDL_gatherRefProcess(&intAlgObj,
                                   algLayer,
                                   tidlLayer,
                                   params,
                                   data,
                                   indices,
                                   outPtr,
                                   inDataParams,
                                   inIndicesParams,
                                   &tidlLayer->outData);
    if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      int32_t LayerIdx;
      LayerIdx = algLayer->layerIdx;

      TIDL_UpdateScaleFactors(&intAlgObj, LayerIdx, 0, 0, 1);
    }
  }
  else
#endif
  {
    status = TIDL_deviceUtilsCommonProcess(commonParams,
                                           algLayer,
                                           tidlLayer,
                                           inPtrs,
                                           outPtrs,
                                           layerIdx);
  }
  return status;
}

int32_t TIDL_gatherLayerAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
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
    status = TIDL_deviceUtilsCommonAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }

  return status;
}

int32_t TIDL_gatherLayerInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
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
    status = TIDL_deviceUtilsCommonInit(layerSpecificParams,
                                        commonParams,
                                        algLayer,
                                        layerIdx,
                                        memory,
                                        memorySize,
                                        outPtr);
  }

  return status;
}
