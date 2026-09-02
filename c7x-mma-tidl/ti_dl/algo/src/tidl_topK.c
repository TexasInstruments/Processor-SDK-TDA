/*
 *
 * Copyright (c) {2022 - 2023} Texas Instruments Incorporated
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
@file    tidl_topK.c
@brief   This file defines private functions for topK layer.
@version 0.1 (June 2023) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_topK.h"
#include <limits>
#include "tidl_commonUtils.h"
#include "tidl_forceNegativeTest.h"

#ifdef HOST_EMULATION
/* Function Prototypes */
template <class Tin, class Tout>
int32_t TIDL_findTopK(const Tin* pIn, Tout* pOut, int32_t* indPtr, int32_t K, int32_t N, int32_t largest, int32_t inPitch, int32_t outPitch);
template <class Tin, class Tout>
int32_t TIDL_refTopK(const Tin* pIn, Tout* pOut, const sTIDL_DataParams_t* inDataParams, const sTIDL_Layer_t * tidlLayer);

template<class Tin, class Tout>
    int32_t TIDL_findTopK(
        const Tin *pIn,
        Tout *pOut,
        int32_t *indPtr,
        int32_t K,
        int32_t N,
        int32_t largest,
        int32_t inPitch,
        int32_t outPitch)
{
  int32_t status = IALG_EOK;
  int32_t i1 = 0, i2 = 0, i3 = 0;
  Tin inDataVal;
  if (N < K)
  {
    tidl_printf(0, "TopK: K cannot be greater than the axis dimension\n");
    status = IALG_EFAIL;
  }

  if (status == IALG_EOK)
  {
    for (i1 = 0; i1 < N; i1++)
    {
      inDataVal = pIn[i1 * inPitch];
      for (i2 = 0; i2 < K; i2++)
      {
        if ((tidl_compareTwoValues(inDataVal, (pOut[i2 * outPitch]), largest) != (bool)0))
        {
          if(inDataVal == (pOut[i2 * outPitch]) && indPtr[i2 * outPitch] >= 0)
          {
            /* If the values are same, then retain the old index */
            continue;
          }

          for(i3 = K - 1; i3 >= (i2 + 1); i3--)
          {
            pOut[i3 * outPitch] = pOut[(i3 - 1) * outPitch];
            indPtr[i3 * outPitch] = indPtr[(i3 - 1) * outPitch];
          }
          pOut[i2 * outPitch] = (Tout)inDataVal;
          indPtr[i2 * outPitch] = (int32_t)i1;

          break;
        }
      }
    }
  }

  return status;
}

template<class Tin, class Tout>
    int32_t TIDL_refTopK(
        const Tin *pIn,
        Tout *pOut,
        const sTIDL_DataParams_t *inDataParams,
        const sTIDL_Layer_t *tidlLayer)
{
  int32_t status = IALG_EOK;
  const Tin *inPtr = pIn;
  Tout *outPtr = pOut;
  Tout *outPtrTemp = pOut;
  int32_t *indPtr = NULL;
  int32_t dim[TIDL_PITCH_MAX], icnt[TIDL_DIM_MAX], indicesPitch = 0;
  int32_t i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, i6 = 0, outOffset = 0, inPitch = 0, outPitch = 0;
  Tout valueLimit = (Tout)0;
  const sTIDL_TopKParams_t *topKParams = &tidlLayer->layerParams.topKParams;

  if (topKParams->largest == 1)
  {
    valueLimit = (Tout)std::numeric_limits<Tout>::lowest();
  }
  else
  {
    valueLimit = (Tout)std::numeric_limits<Tout>::max();
  }

  for (i1 = 0; i1 < (tidlLayer->outData.dimValues[TIDL_DIM_BATCH] * tidlLayer->outData.pitch[TIDL_ROI_PITCH]); i1++)
  {
    pOut[i1] = valueLimit;
  }
  for (i1 = 0; i1 < (int32_t)TIDL_DIM_MAX; i1++)
  {
    icnt[i1] = inDataParams->dimValues[i1];
  }
  for (i1 = 0; i1 < (int32_t)TIDL_PITCH_MAX; i1++)
  {
    dim[i1] = inDataParams->pitch[i1];
  }

  icnt[topKParams->axis] = 1;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
  Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
  This does not impact functional correctness or safety.
  <justification end> */
  switch (topKParams->axis)
  {
    /* LDRA_JUSTIFY_END */
  case TIDL_DIM_WIDTH:
  {
    inPitch = 1;
    outPitch = 1;
    break;
  }
  case TIDL_DIM_HEIGHT:
  {
    inPitch = inDataParams->pitch[TIDL_LINE_PITCH];
    outPitch = tidlLayer->outData.pitch[TIDL_LINE_PITCH];
    break;
  }
#ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST
  case TIDL_DIM_NUMCH:
  {
    inPitch = inDataParams->pitch[TIDL_CHANNEL_PITCH];
    outPitch = tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
    break;
  }
  case TIDL_DIM_DIM2:
  {
    inPitch = inDataParams->pitch[TIDL_DIM2_PITCH];
    outPitch = tidlLayer->outData.pitch[TIDL_DIM2_PITCH];
    break;
  }
  case TIDL_DIM_DIM1:
  {
    inPitch = inDataParams->pitch[TIDL_DIM1_PITCH];
    outPitch = tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
    break;
  }
  case TIDL_DIM_BATCH:
  {
    inPitch = inDataParams->pitch[TIDL_ROI_PITCH];
    outPitch = tidlLayer->outData.pitch[TIDL_ROI_PITCH];
    break;
  }
#endif
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
  Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
  This does not impact functional correctness or safety.
  <justification end> */
  default:
  {
    break;
  }
    /* LDRA_JUSTIFY_END */
  }

  int32_t axisForIncrement = topKParams->incrementAxis;

  if (axisForIncrement != topKParams->axis)
  {
    indicesPitch = tidlLayer->outData.pitch[axisForIncrement] * inDataParams->dimValues[axisForIncrement];
  }
  else
  {
    /* Used for topKParams->axis <= TIDL_DIM_NUMCH*/
    indicesPitch = tidlLayer->outData.pitch[axisForIncrement] * topKParams->K;
  }
  for (i1 = 0; i1 < icnt[TIDL_DIM_BATCH]; i1++)
  {
    outPtrTemp = pOut + i1 * (tidlLayer->outData.pitch[TIDL_ROI_PITCH]);
    int32_t* indicesPtr = (int32_t*)(outPtrTemp + indicesPitch);
    for (i2 = 0; i2 < icnt[TIDL_DIM_DIM1]; i2++)
    {
      for (i3 = 0; i3 < icnt[TIDL_DIM_DIM2]; i3++)
      {
        for (i4 = 0; i4 < icnt[TIDL_DIM_NUMCH]; i4++)
        {
          for (i5 = 0; i5 < icnt[TIDL_DIM_HEIGHT]; i5++)
          {
            for (i6 = 0; i6 < icnt[TIDL_DIM_WIDTH]; i6++)
            {
              inPtr = pIn + ((i1 * dim[TIDL_ROI_PITCH]) + (i2 * dim[TIDL_DIM1_PITCH]) + (i3 * dim[TIDL_DIM2_PITCH]) +
                             (i4 * dim[TIDL_CHANNEL_PITCH]) + (i5 * dim[TIDL_LINE_PITCH]) + i6);
              outOffset = (i2 * tidlLayer->outData.pitch[TIDL_DIM1_PITCH]) + (i3 * tidlLayer->outData.pitch[TIDL_DIM2_PITCH])
                        + (i4 * tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH]) + (i5 * tidlLayer->outData.pitch[TIDL_LINE_PITCH]) + i6;

              outPtr = outPtrTemp + outOffset;
              indPtr = indicesPtr + outOffset;        
             
              status = TIDL_findTopK (inPtr, outPtr, indPtr, topKParams->K, inDataParams->dimValues[topKParams->axis], topKParams->largest, inPitch, outPitch);                            
            }
          }
        }
      }
    }
  }

  TIDL_L1DandL2CacheWbInv();

  return status;
}
#endif

/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_TopK
 @fn         TIDL_topKProcess
 @brief      TopK Layer process function

 @param      intAlgHandle : tidl algorothm handle
 @param      algLayer : Ptr to alg layer parameter used in topK layer
 @param      tidlLayer: Ptr to tidl layer parameter used in topK layer
 @param      inPtrs: Ptrs to input buffers to be processed
 @param      outPtrs: Ptrs to output buffers to be processed
 @param      sysMems: Ptr to memory related buffers used in crop layer
 @remarks    None
 @return     IALG_EOK   - Successful
             IALG_EFAIL - Unspecified error
 ----------------------------------------------------------------------------
*/
int32_t TIDL_topKProcess(TIDL_NetworkCommonParams *commonParams,
                         sTIDL_AlgLayer_t *algLayer,
                         sTIDL_Layer_t *tidlLayer,
                         void *inPtrs[],
                         void *outPtrs[],
                         int32_t layerIdx)
{
  sTIDL_DataParams_t *inDataParams   = &commonParams->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;

  int32_t status = IALG_EOK;

  TIDL_CreateParams createParams;
  (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));

  // sTIDL_sysMemHandle_t *sysMems = commonParams->tidlCommonParams->sysMems;

  if (createParams.forceNegativeTest == TIDL_SAFETY_FLAG_TOPK_FORCE_LARGE_K)
  {
    tidlLayer->layerParams.topKParams.K = inDataParams->dimValues[tidlLayer->layerParams.topKParams.axis] + 1;
  }

#ifdef HOST_EMULATION
  TIDL_Obj intAlgObj;
  intAlgObj.createParams = (TIDL_CreateParams *)&createParams;
  if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
  {
    if (inDataParams->elementType == TIDL_SinglePrecFloat)
    {
      status = TIDL_refTopK<float32_tidl, float32_tidl>((float32_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inDataParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_BFloat16)
    {
       status = TIDL_refTopK<bfloat16_tidl, bfloat16_tidl>((bfloat16_tidl *)inPtrs[0], (bfloat16_tidl *)outPtrs[0], inDataParams, tidlLayer);
    }
    else if (inDataParams->elementType == TIDL_UnsignedChar)
    {
#ifdef TIDL_COVERAGE_DEAD_CODE
      if (tidlLayer->outData.elementType == TIDL_UnsignedShort)
      {
        status = TIDL_refTopK<uint8_t, uint16_t>((uint8_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
      }
      else
      {
#endif
        status = TIDL_refTopK<uint8_t, uint8_t>((uint8_t *)inPtrs[0], (uint8_t *)outPtrs[0], inDataParams, tidlLayer);
#ifdef TIDL_COVERAGE_DEAD_CODE
      }
#endif
    }
    else if (inDataParams->elementType == TIDL_SignedChar)
    {
#ifdef TIDL_COVERAGE_DEAD_CODE
      if (tidlLayer->outData.elementType == TIDL_SignedShort)
      {
        status = TIDL_refTopK<int8_t, int16_t>((int8_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
      }
      else
      {
#endif
        status = TIDL_refTopK<int8_t, int8_t>((int8_t *)inPtrs[0], (int8_t *)outPtrs[0], inDataParams, tidlLayer);
#ifdef TIDL_COVERAGE_DEAD_CODE
      }
#endif
    }
    else if (inDataParams->elementType == TIDL_UnsignedShort)
    {
      status = TIDL_refTopK<uint16_t, uint16_t>((uint16_t *)inPtrs[0], (uint16_t *)outPtrs[0], inDataParams, tidlLayer);
    }
    else
    {
      status = TIDL_refTopK<int16_t, int16_t>((int16_t *)inPtrs[0], (int16_t *)outPtrs[0], inDataParams, tidlLayer);
    }

    if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      TIDL_UpdateScaleFactors(&intAlgObj, algLayer->layerIdx, 0, 0, 1);
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

#ifdef TIDL_COVERAGE_DEAD_CODE
int32_t TIDL_topKAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                       const TIDL_NetworkCommonParams *commonParams,
                       int32_t layerIdx,
                       int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

#ifdef HOST_EMULATION
  if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
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

int32_t TIDL_topKInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                      const TIDL_NetworkCommonParams *commonParams,
                      sTIDL_AlgLayer_t *algLayer,
                      int32_t layerIdx,
                      uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                      int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                      void **outPtr)
{
  int32_t status = IALG_EOK;

  return status;
}
#endif
