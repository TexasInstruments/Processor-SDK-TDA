/*
*
* Copyright (c) {2015 - 2026} Texas Instruments Incorporated
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
 *  \file tidl_gru.c
 *
 *  \brief GRU process calls & ref implementation
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_gru.h"
#include "tidl_gru_ref.h"
#include <math.h>
#include <limits>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

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

/**
 * @brief Allocation function for GRU layer
 *
 * @param layerSpecificParams : Layer specific parameters
 * @param commonParams : Common parameters for the network
 * @param layerIdx : Layer index
 * @param memorySize : Array to store memory requirements
 * @return int32_t : Status of the operation
 */
int32_t TIDL_gruAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                      const TIDL_NetworkCommonParams *commonParams,
                      int32_t layerIdx,
                      int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_gruRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  else
  #endif
  {
    status = TIDL_deviceUtilsCommonAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }

  return status;
}


/**
 * @brief Initialization function for GRU reference implementation
 *
 * @param layerSpecificParams : Layer specific parameters
 * @param commonParams : Common parameters for the network
 * @param algLayer : Algorithm layer structure
 * @param layerIdx : Layer index
 * @param memory : Array of memory pointers
 * @param memorySize : Array of memory sizes
 * @param outPtr : Output pointer
 * @return int32_t : Status of the operation
 */
int32_t TIDL_gruInit(const TIDL_LayerSpecificParams *layerSpecificParams,
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
    status = TIDL_gruRefInit(layerSpecificParams,
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

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

int32_t TIDL_gruProcess(TIDL_NetworkCommonParams   * commonParams,
                        sTIDL_AlgLayer_t           * algLayer,
                        sTIDL_Layer_t              * tidlLayer,
                        void                       * inPtrs[],
                        void                       * outPtrs[],
                        int32_t                    layerIdx)
{
  int32_t status = TIDL_SUCCESS;

  #ifdef HOST_EMULATION
  TIDL_CreateParams createParams;
  (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));

  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    sTIDL_GRUParams_t *params = &tidlLayer->layerParams.gruParams;

    uint8_t (*inPtr)[]        = (uint8_t (*)[])(inPtrs[TIDL_RecurrentInputX]);
    uint8_t (*WPtr)[]         = (uint8_t (*)[])(inPtrs[TIDL_RecurrentInputW]);
    uint8_t (*RPtr)[]         = (uint8_t (*)[])(inPtrs[TIDL_RecurrentInputR]);
    uint8_t (*biasPtr)[]      = NULL;
    uint8_t (*initial_hPtr)[] = NULL;

    sTIDL_DataParams_t *inDataParams    = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[TIDL_RecurrentInputX]].outData;
    sTIDL_DataParams_t *WParams         = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[TIDL_RecurrentInputW]].outData;
    sTIDL_DataParams_t *RParams         = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[TIDL_RecurrentInputR]].outData;
    sTIDL_DataParams_t *biasParams      = NULL;
    sTIDL_DataParams_t *initial_hParams = NULL;

    int32_t inIdx = TIDL_RecurrentInputB;
    if(params->isBiasPresent == 1)
    {
      biasPtr = (uint8_t (*)[])(inPtrs[inIdx]);
      biasParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[inIdx]].outData;
      inIdx++;
    }

    if(params->isInitialHPresent == 1)
    {
      initial_hPtr = (uint8_t (*)[])(inPtrs[inIdx]);
      initial_hParams = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[inIdx]].outData;
      inIdx++;
    }

    int8_t (*outPtr)[] = (int8_t (*)[])(outPtrs[0]);

    TIDL_Obj intAlgObj;
    intAlgObj.createParams = (TIDL_CreateParams *) &createParams;
    int8_t useTaylor = 1;
    if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      useTaylor = 0;
    }

    status = TIDL_gruRefProcess(&intAlgObj,
                                algLayer,
                                tidlLayer,
                                params,
                                inPtr,
                                WPtr,
                                RPtr,
                                biasPtr,
                                initial_hPtr,
                                outPtr,
                                inDataParams,
                                WParams,
                                RParams,
                                biasParams,
                                initial_hParams,
                                &tidlLayer->outData,
                                useTaylor);

  }
  else /* if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY) */
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
