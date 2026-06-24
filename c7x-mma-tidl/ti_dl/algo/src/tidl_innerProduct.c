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
 @file    tidl_innerProduct.c
 @brief   This file contains TILD InnerProduct Layer implementation
 @version 0.1 (Jan 2017) : initial version [EPR]
 ----------------------------------------------------------------------------
*/

#include <math.h>
#include <float.h>
#include "tidl_alg_utils.h"
#include "tidl_device_functions.h"
#include "tidl_common_utils_infer_import.h"
#include "tidl_commonUtils.h"
#include "tidl_innerProduct_mma.h"
#include "tidl_matMul_mma.h"

#include "tidl_innerProduct_ref.h"
#include "tidl_forceNegativeTest.h"
#include "tidl_innerProduct.h"

#define USE_16BIT_BIAS_FOR_8BIT_MODE (1)

int32_t TIDL_forceInnerProductRef(
    const TIDL_CreateParams *createParamsPtr,
    const sTIDL_Layer_t *tidlLayer)
{
  int32_t forceRef;
  sTIDL_Network_t *pNet = createParamsPtr->net;
  sTIDL_DataParams_t *inDataParams;
  uint16_t numInNodes, numOutNodes;
  Bool isMatmul = (tidlLayer->numInBufs > 1) ? TRUE : FALSE;

  inDataParams = &pNet->TIDLLayers[tidlLayer->inData[0]].outData;
  numOutNodes = (uint16_t)tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
  numInNodes = (uint16_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  forceRef = (TIDL_referencFlow(createParamsPtr) == 0) ? 1 : 0;
  if (forceRef == 1)
  {
    /* LDRA_JUSTIFY_END */
    if (((createParamsPtr->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) &&
        ((createParamsPtr->reservedCtrl & TIDL_REF_CODE_FOR_IP) == 0U))
    {
      if (isMatmul == TRUE)
      {
        forceRef = 0;
      }
      else if (((numInNodes <= 2048U) && (numOutNodes <= 1024U)) && (numOutNodes >= 16U)) /*InnerProduct*/
      {
        forceRef = 0;
      }
      else
      {
        /*do nothing*/
      }
    }
  }
  return forceRef;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_innerProductAlloc
@brief      Function to Request Memory required by the Innerproduct layer

@param      specificParams : Pointer to specific parameter required for all the layers
@param      commonParams   : Pointer to common parameter required for all the layers
@param      layerIdx       : Layer Id of the layer
@param      memorySize     : Output memory requirement of the layer
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

int32_t TIDL_innerProductAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                  const TIDL_NetworkCommonParams *commonParams,
                                  int32_t layerIdx,
                                  int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;
  int32_t forceRef = TIDL_forceInnerProductRef(commonParams->createParams, &commonParams->net->TIDLLayers[layerIdx]);

  int32_t biasParamSize = 0;
  int32_t tranformSize = TIDL_getDatElementSize(commonParams->net->TIDLLayers[layerIdx].outData.elementType) *
                         TIDL_innerProductCoeffMemRequired(&commonParams->net->TIDLLayers[layerIdx]);

  if (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.isBias != 0)
  {
    int32_t biasSize = 0;
    int32_t scaleSize = 0;
    int32_t shiftSize = 0;
    TIDL_innerProductGetBiasScaleSize(&commonParams->net->TIDLLayers[layerIdx].outData,
                                      &commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams, 
                                      commonParams->net->TIDLLayers[layerIdx].layerKernelType,
                                      &biasSize, &scaleSize, &shiftSize);
    biasParamSize = biasSize;
  }

  if ((((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) && (forceRef == 0))
  {
    if (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.weights == 0) /* Matmul */
    {
      status = TIDL_deviceUtilsCommonAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
      if (status == IALG_EOK)
      {
        if (TIDL_isKernelHighPrecision(commonParams->net->TIDLLayers[layerIdx].layerKernelType) == (int32_t)TRUE)
        {
          int32_t biasSize = 0;
          int32_t scaleSize = 0;
          int32_t shiftSize = 0;
          TIDL_innerProductGetBiasScaleSize(&commonParams->net->TIDLLayers[layerIdx].outData,
                                            &commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams,
                                            commonParams->net->TIDLLayers[layerIdx].layerKernelType,
                                            &biasSize, &scaleSize, &shiftSize);
          memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += (biasSize + scaleSize + shiftSize + TIDL_ALIGNMENT_SIZE);
        }
      }
    }
    else /* Inner Product */
    {
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += (tranformSize + TIDL_ALIGNMENT_SIZE);
    }
  }
  else
  {
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
/* Have addtional space for bias during forced reference for inner product */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if (forceRef == 1)
    {
      /* LDRA_JUSTIFY_END */
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += (tranformSize + biasParamSize + TIDL_ALIGNMENT_SIZE);
    }
  }
  if (status == IALG_EOK)
  {
#ifdef HOST_EMULATION
    if ((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U)
#endif
    {
      memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += tranformSize + TIDL_ALIGNMENT_SIZE;
    }

    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += biasParamSize + TIDL_ALIGNMENT_SIZE;
  }
  return status;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_innerProductInit
@brief      Function to Initialize the InnerProduct layer specific params

@param      specificParams :  Pointer to specific parameter required for all the layers
@param      commonParams   :  Pointer to common parameter required for all the layers
@param      algLayer       :  Pointer to layer specific internal parameters
@param      layerIdx       :  Layer Id of the layer
@param      memory         :  Allocated memory pointers
@param      memorySize     :  Output memory requirement of the layer
@param      outPtr         :  Output memory pointer. Applicable only for reference flow
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

int32_t TIDL_innerProductInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                 const TIDL_NetworkCommonParams *commonParams,
                                 sTIDL_AlgLayer_t *algLayer,
                                 int32_t layerIdx,
                                 uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                 int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                 void **outPtr)

{
  int32_t status = IALG_EOK;
  int32_t forceRef = TIDL_forceInnerProductRef(commonParams->createParams, &commonParams->net->TIDLLayers[layerIdx]);
  int32_t tranformSize;
  int32_t paramMemTabOffset = 0;
  int32_t quantizationStyle = commonParams->net->quantizationStyle;

  tranformSize = TIDL_getDatElementSize(commonParams->net->TIDLLayers[layerIdx].outData.elementType) *
                 TIDL_innerProductCoeffMemRequired(&commonParams->net->TIDLLayers[layerIdx]);

  sTIDL_InnerProductParams_t *innerProductParams = &commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams;

  int32_t outBatch = commonParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_BATCH];
  int32_t outDim1 = commonParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_DIM1];
  int32_t outDim2 = commonParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_DIM2];
  int32_t outCh   = commonParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
  int32_t biasSize = 0;
  int32_t scaleSize = 0;
  int32_t shiftSize = 0;

  algLayer->layerParams.innerProductParams.accMem = NULL;
  algLayer->layerParams.innerProductParams.accMemSize = 0;

  algLayer->layerParams.innerProductParams.wtTranformMem = NULL;
  algLayer->layerParams.innerProductParams.wtTranformSize = 0;

  algLayer->layerParams.innerProductParams.biasParamMem = NULL;
  algLayer->layerParams.innerProductParams.biasParamSize = 0;

  algLayer->layerParams.innerProductParams.matmulScaleMem = NULL;
  algLayer->layerParams.innerProductParams.matmulScaleMemSize = 0;

  algLayer->layerParams.innerProductParams.matmulShiftMem = NULL;
  algLayer->layerParams.innerProductParams.matmulShiftMemSize = 0;

  algLayer->layerParams.innerProductParams.maxWorkloadLoopIter = 1;

  if (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.isBias != 0)
  {
    TIDL_innerProductGetBiasScaleSize(&commonParams->net->TIDLLayers[layerIdx].outData,
                                    &commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams, 
                                    commonParams->net->TIDLLayers[layerIdx].layerKernelType,
                                    &biasSize, &scaleSize, &shiftSize);
    algLayer->layerParams.innerProductParams.biasParamSize = biasSize;

    TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                     &paramMemTabOffset,
                     algLayer->layerParams.innerProductParams.biasParamSize,
                     TIDL_ALIGNMENT_SIZE,
                     &algLayer->layerParams.innerProductParams.biasParamMem);
  }
  else
  {
    algLayer->layerParams.innerProductParams.biasParamMem = NULL;
  }

  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                   &paramMemTabOffset,
                   algLayer->layerParams.innerProductParams.accMemSize,
                   TIDL_ALIGNMENT_SIZE,
                   &algLayer->layerParams.innerProductParams.accMem);

  // algLayer->layerParams.innerProductParams.accMem =
  //   get_uint8_t_pointer((uint8_t*)(void*)memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].base, *paramMemTabOffset);
  // *paramMemTabOffset += ALIGN_SIZE((int32_t)scratchDataSize, 128);

  algLayer->layerParams.innerProductParams.wtTranformMem = NULL;
  algLayer->layerParams.innerProductParams.wtTranformSize = 0;

  // Have addtional space for bias during forced reference for inner product
  if (forceRef == 1)
  {
    tranformSize += algLayer->layerParams.innerProductParams.biasParamSize;
  }
  if ((((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) &&
      (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.weights != 0))
  {
    algLayer->layerParams.innerProductParams.wtTranformSize = tranformSize;

    TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                     &paramMemTabOffset,
                     algLayer->layerParams.innerProductParams.wtTranformSize,
                     TIDL_ALIGNMENT_SIZE,
                     &algLayer->layerParams.innerProductParams.wtTranformMem);

    // algLayer->layerParams.innerProductParams.wtTranformMem =
    //   get_uint8_t_pointer((uint8_t*)(void*)memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].base, *paramMemTabOffset);

    (void)memset(algLayer->layerParams.innerProductParams.wtTranformMem, 0, (size_t)algLayer->layerParams.innerProductParams.wtTranformSize);
  }

  int8_t *weightPtr = get_int8_t_pointer((int8_t *)(commonParams->net), innerProductParams->weights);
  void *orgBiasPtr = NULL;
  void *biasPtr = NULL;
  float32_tidl inScaleFactor;
  sTIDL_DataParams_t * inDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
  int32_t layerType = commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].layerType;
  if (layerType == TIDL_ConstDataLayer)
  {
    inDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[1]].outData;
  }

  if (TIDL_QuantStyleAsymNP2_TFL != quantizationStyle)
  {
    orgBiasPtr = (int16_t *)get_int8_t_pointer((int8_t *)(commonParams->net), innerProductParams->bias);
    biasPtr = (int16_t *)(algLayer->layerParams.innerProductParams.biasParamMem);
    if (innerProductParams->weightScale != 0.0f)
    {
      inScaleFactor = (innerProductParams->weightScale * inDataParams->tensorScale) / innerProductParams->biasScale;
    }
    else
    {
      sTIDL_DataParams_t *inDataParams2;
      inDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
      inDataParams2 = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[1]].outData;
      inScaleFactor = (inDataParams->tensorScale * inDataParams2->tensorScale) / innerProductParams->biasScale;
    }
#ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST
    if (innerProductParams->biasScale == -1.0f)
    {
      inScaleFactor = 0.0f;
    }
#endif
  }

  int32_t *orgbiasptr;
  if (TIDL_QuantStyleAsymNP2_TFL == quantizationStyle)
  {
    // Populate scale and shift for MMA for convolution:
    // Scale ratio
    float32_tidl scaleRatio = 1;
    float32_tidl weightScalePtr = innerProductParams->weightScale;
    float32_tidl scale_x = inDataParams->tensorScale;
    float32_tidl scale_y = commonParams->net->TIDLLayers[layerIdx].outData.tensorScale;
    // Assuming per axis: (should be single iteration for non per ch) !Replace with axis length
    // Temp logic:
    scaleRatio = (scale_x * weightScalePtr) / scale_y;
    TIDL_getMMAv2_ScaleAndShift(scaleRatio, &algLayer->layerParams.innerProductParams.mmaScale, &algLayer->layerParams.innerProductParams.mmaShift);
    int32_t axisCtr;
    orgbiasptr = (int32_t *)get_int8_t_pointer((int8_t *)(commonParams->net), innerProductParams->bias);
    biasPtr = (int32_t *)algLayer->layerParams.innerProductParams.biasParamMem;
    // Simple copy over of bias from model to local memory.
    for (axisCtr = 0; axisCtr < scaleSize; axisCtr++)
    {
      ((int32_t *)biasPtr)[axisCtr] = orgbiasptr[axisCtr];
    }
  }

#if USE_16BIT_BIAS_FOR_8BIT_MODE
  if ((innerProductParams->constIdx == -1))
  {
    if (commonParams->net->TIDLLayers[layerIdx].weightsElementSizeInBits <= 8)
    {
      if (TIDL_QuantStyleAsymNP2_TFL != quantizationStyle) // No Splitting
      {
        if (innerProductParams->isBias != 0)
        {
          #if defined (__C7120__) || defined (__C7604__)
          if (TIDL_isBiasHighPrecision(commonParams->net->deviceName) == TRUE)
          {
            TIDL_AM_conv2dBiasSplit((int16_t*)orgBiasPtr, (int32_t*)biasPtr, &innerProductParams->biasB, scaleSize, inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16,0xFF, TIDL_getDatElementSign(inDataParams->elementType));
          }
          else
          {
            TIDL_conv2dBiasSplit((int16_t *)orgBiasPtr, (int16_t *)biasPtr, &innerProductParams->biasB,
                                 scaleSize, inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16, 0xFF, TIDL_getDatElementSign(inDataParams->elementType));
          }
          #endif
          #if defined (__C7504__) || defined (__C7524__)
          TIDL_AM_conv2dBiasSplit((int16_t*)orgBiasPtr, (int32_t*)biasPtr, &innerProductParams->biasB, scaleSize, inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16,0xFF, TIDL_getDatElementSign(inDataParams->elementType));
          #endif
          #ifdef __C7100__
          TIDL_conv2dBiasSplit((int16_t *)orgBiasPtr, (int16_t *)biasPtr, &innerProductParams->biasB,
                                 scaleSize, inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16, 0xFF, TIDL_getDatElementSign(inDataParams->elementType));
          #endif
        }

        if ((((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) &&
            (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.weights != 0))
        {

          int8_t *tranformPtr = (int8_t*)algLayer->layerParams.innerProductParams.wtTranformMem;
          algLayer->layerParams.innerProductParams.wtTransformBatchPitch=
          TIDL_innerProductTransform(weightPtr, tranformPtr, (int16_t*)biasPtr,
            innerProductParams->numInCols, innerProductParams->numOutCols, (outBatch * outDim1 * outDim2 * outCh),
            innerProductParams->isBias, innerProductParams->inputBTranspose, commonParams->net->deviceName);
          algLayer->layerParams.innerProductParams.biasBatchPitch = innerProductParams->numOutCols;
        }
      }
    }
    else
#endif
    {
      if (TIDL_QuantStyleAsymNP2_TFL != quantizationStyle) // No Splitting
      {
        if (innerProductParams->isBias != 0)
        {
          #if defined (__C7120__) || defined (__C7604__)
          if ((TIDL_isBiasHighPrecision(commonParams->net->deviceName) == TRUE))
          {
            TIDL_AM_conv2dBiasSplit((int16_t*)orgBiasPtr, (int64_t*)biasPtr, &innerProductParams->biasB, scaleSize, inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32, 0xFFFF, TIDL_getDatElementSign(inDataParams->elementType));
          }
          else
          {
            TIDL_conv2dBiasSplit((int16_t*)orgBiasPtr, (int16_t*)biasPtr,  &innerProductParams->biasB,
            scaleSize, inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32,0xFFFF,TIDL_getDatElementSign(inDataParams->elementType));
          }
          #endif
          #if defined (__C7504__) || defined (__C7524__)
          TIDL_AM_conv2dBiasSplit((int16_t*)orgBiasPtr, (int64_t*)biasPtr, &innerProductParams->biasB, scaleSize, inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32, 0xFFFF, TIDL_getDatElementSign(inDataParams->elementType));
          #endif
          #ifdef __C7100__
          TIDL_conv2dBiasSplit((int16_t*)orgBiasPtr, (int16_t*)biasPtr,  &innerProductParams->biasB,
            scaleSize, inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32,0xFFFF,TIDL_getDatElementSign(inDataParams->elementType));
          #endif
        }

        if ((((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) &&
            (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.weights != 0))
        {
          int16_t *tranformPtr = (int16_t*)algLayer->layerParams.innerProductParams.wtTranformMem;
          algLayer->layerParams.innerProductParams.wtTransformBatchPitch=
          TIDL_innerProductTransform((int16_t*)weightPtr, tranformPtr, (int16_t*)biasPtr,
            innerProductParams->numInCols, innerProductParams->numOutCols, (outBatch * outDim1 * outDim2 * outCh),
            innerProductParams->isBias, innerProductParams->inputBTranspose, commonParams->net->deviceName);
          algLayer->layerParams.innerProductParams.biasBatchPitch = innerProductParams->numOutCols;
        }
      }
    }

    if (TIDL_QuantStyleAsymNP2_TFL == quantizationStyle)
    {
#ifdef HOST_EMULATION
      if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U)
#endif
      {
        int8_t *tranformPtr = (int8_t*)algLayer->layerParams.innerProductParams.wtTranformMem;
        algLayer->layerParams.innerProductParams.wtTransformBatchPitch=
        TIDL_innerProductTransform(weightPtr, tranformPtr, (int32_t*)biasPtr,
          innerProductParams->numInCols, innerProductParams->numOutCols, (outBatch * outDim1 * outDim2 * outCh),
          innerProductParams->isBias, innerProductParams->inputBTranspose, commonParams->net->deviceName);
        algLayer->layerParams.innerProductParams.biasBatchPitch = innerProductParams->numOutCols;
      }
    }
  }

  if (((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) &&
      (TIDL_isKernelHighPrecision(commonParams->net->TIDLLayers[layerIdx].layerKernelType) == (int32_t)TRUE) &&
      (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.weights == 0))
  {
    void *biasScaleShiftPtr = NULL;
    uint8_t *mmav2_bias = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.derivedBias);
    uint8_t *mmav2_Scales = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.derivedScales);
    uint8_t *mmav2_Shifts = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.derivedShifts);
    TIDL_innerProductGetBiasScaleSize(&commonParams->net->TIDLLayers[layerIdx].outData,
                                  &commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams,
                                  commonParams->net->TIDLLayers[layerIdx].layerKernelType,
                                  &algLayer->layerParams.innerProductParams.biasParamSize, 
                                  &algLayer->layerParams.innerProductParams.matmulScaleMemSize,
                                  &algLayer->layerParams.innerProductParams.matmulShiftMemSize);
    
    TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_PERSISTENT],
                     &paramMemTabOffset,
                     (algLayer->layerParams.innerProductParams.biasParamSize +
                      algLayer->layerParams.innerProductParams.matmulScaleMemSize +
                      algLayer->layerParams.innerProductParams.matmulShiftMemSize),
                     TIDL_ALIGNMENT_SIZE,
                     &biasScaleShiftPtr);

    algLayer->layerParams.innerProductParams.biasParamMem = biasScaleShiftPtr;
    algLayer->layerParams.innerProductParams.matmulScaleMem = (void *)((uint8_t *)algLayer->layerParams.innerProductParams.biasParamMem + algLayer->layerParams.innerProductParams.biasParamSize);
    algLayer->layerParams.innerProductParams.matmulShiftMem = (void *)((uint8_t *)algLayer->layerParams.innerProductParams.matmulScaleMem + algLayer->layerParams.innerProductParams.matmulScaleMemSize);

    (void)memcpy((void *)algLayer->layerParams.innerProductParams.biasParamMem, (const void *)mmav2_bias, algLayer->layerParams.innerProductParams.biasParamSize);
    (void)memcpy((void *)algLayer->layerParams.innerProductParams.matmulScaleMem, (const void *)mmav2_Scales, algLayer->layerParams.innerProductParams.matmulScaleMemSize);
    (void)memcpy((void *)algLayer->layerParams.innerProductParams.matmulShiftMem, (const void *)mmav2_Shifts, algLayer->layerParams.innerProductParams.matmulShiftMemSize);
  }

  {
    if (forceRef != 0)
    {
      // copy the original weights and bias in alg layer buffer space
      void *src = get_int8_t_pointer((int8_t *)(commonParams->net), innerProductParams->weights);
      void *dst = algLayer->layerParams.innerProductParams.wtTranformMem;
      int32_t size = algLayer->layerParams.innerProductParams.wtTranformSize;
      if ((size != 0) && (dst != NULL) && (src != NULL))
      {
        (void)memcpy(dst, src, size);
      }
    }
  }

  /* APIs calculate offset with regard to base ptr. In this situation, the device utils API would not
   * know that the memory has already been utilized. Thus we add an offset equal to the size of memory
   * space already allocated and remove the offset after the API has run so the caller function sees the
   * same memory.
   **/
  memory[TIDL_LAYER_MEMORY_PERSISTENT] += TIDL_ALIGN_CEIL(paramMemTabOffset, TIDL_ALIGNMENT_SIZE);

  if ((((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) && (forceRef == 0))
  {
    if (commonParams->net->TIDLLayers[layerIdx].layerParams.innerProductParams.weights == 0)
    {
      status = TIDL_deviceUtilsCommonInit(layerSpecificParams,
                                          commonParams,
                                          algLayer,
                                          layerIdx,
                                          memory,
                                          memorySize,
                                          outPtr);

      algLayer->layerParams.innerProductParams.maxWorkloadLoopIter = TIDL_matmulDeviceGetWlLoopMaxIterWrap(layerSpecificParams,
                                                                                                           &commonParams->net->TIDLLayers[layerIdx],
                                                                                                           commonParams);
    }
  }
  else
  {
    status = TIDL_deviceCommonRefInit(layerSpecificParams,
                                      commonParams,
                                      algLayer,
                                      layerIdx,
                                      memory,
                                      memorySize,
                                      outPtr);

    // TODO: Deprecate accMem in innerProduct since we have scratchData that performs same function
    algLayer->layerParams.innerProductParams.accMem = algLayer->scratchMem;
    algLayer->layerParams.innerProductParams.accMemSize = algLayer->scratchSize;
  }

  memory[TIDL_LAYER_MEMORY_PERSISTENT] -= TIDL_ALIGN_CEIL(paramMemTabOffset, TIDL_ALIGNMENT_SIZE);
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += TIDL_ALIGN_CEIL(paramMemTabOffset, TIDL_ALIGNMENT_SIZE);

  return status;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION_SUPPORT
@fn         TIDL_innerProductProcessNew
@brief InnerProduct layer Process function

@param      commonParams   :  Pointer to common parameter required for all the layers
@param      algLayer       :  Pointer to layer specific internal parameters
@param      layerIdx       :  Layer Id of the layer
@param      memory         :  Allocated memory pointers
@param      memorySize     :  Output memory requirement of the layer
@param      outPtr         :  Output memory pointer. Applicable only for reference flow
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_innerProductProcessNew(TIDL_NetworkCommonParams *commonParams,
                                    sTIDL_AlgLayer_t *algLayer,
                                    sTIDL_Layer_t *tidlLayer,
                                    void *inPtrs[],
                                    void *outPtrs[],
                                    int32_t layerIdx)

{
  sTIDL_InnerProductParams_t *params =
      &tidlLayer->layerParams.innerProductParams;
  tidlInnerProductBuffParams_t buffParams;
  sTIDL_DataParams_t *inDataParams;         /*Points to parameters of first non-const tensor*/
  sTIDL_DataParams_t *inBDataParams = NULL; /*Points to parameters of second (B) tensor*/
  inDataParams    = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;

  if (tidlLayer->numInBufs > 1)
  {
    inBDataParams   = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[1]].outData;
  }

  int32_t quantizationStyle = commonParams->net->quantizationStyle;
  uint16_t outPitch = (uint16_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  uint8_t *inPtr = (uint8_t *)inPtrs[0];
  uint8_t *outPtr = (uint8_t *)outPtrs[0];

  int32_t status = 1;
  int32_t outSize = TIDL_getDatElementSize(tidlLayer->outData.elementType);
  int32_t forceRef = 0;

  if (TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE)
  {
    buffParams.mmaShift = algLayer->layerParams.innerProductParams.mmaShift;
    buffParams.mmaScale = algLayer->layerParams.innerProductParams.mmaScale;
  }

  buffParams.inElementType = inDataParams->elementType;

  if (inBDataParams != NULL)
  {
    buffParams.inBElementType = inBDataParams->elementType;
  }
  else
  {
    if (tidlLayer->weightsElementSizeInBits <= 8)
    {
      buffParams.inBElementType = TIDL_SignedChar;
    }
    else if (tidlLayer->weightsElementSizeInBits <= 16)
    {
      buffParams.inBElementType = TIDL_SignedShort;
    }
    else // if (tidlLayer->weightsElementSizeInBits == 32)
    {
      buffParams.inBElementType = TIDL_SinglePrecFloat;
    }
  }

  buffParams.scratchSize = algLayer->layerParams.innerProductParams.accMemSize;
  buffParams.scratchMem = algLayer->layerParams.innerProductParams.accMem;

  if (((commonParams->net->quantizationStyle == TIDL_QuantStyleNP2Fixed) ||
       (commonParams->net->quantizationStyle == TIDL_QuantStyleP2Dynamic) ||
       (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)FALSE)) &&
      (commonParams->net->quantizationStyle != TIDL_QuantStyleAsymNP2_TFL))
  {
    if (params->weightScale != 0.0f)
    {
      buffParams.inScaleFactor = (params->weightScale * inDataParams->tensorScale) / params->biasScale;
    }
    else
    {
      inDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
      inBDataParams = &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[1]].outData;
      buffParams.inScaleFactor = (inDataParams->tensorScale * inBDataParams->tensorScale) / params->biasScale;
    }
    buffParams.outRoundBits = tidlLayer->outData.roundBits;
#ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST
    if (params->biasScale == -1.0f)
    {
      buffParams.inScaleFactor = 0;
    }
#endif
  }

  outPtr = outPtr + (((tidlLayer->outData.padH * (int32_t)outPitch) + tidlLayer->outData.padW) * outSize);

  forceRef = TIDL_forceInnerProductRef(commonParams->createParams, tidlLayer);
  if ((forceRef != 0) || (TIDL_referencFlow(commonParams->createParams) != 0))
  {
    uint8_t *inBPtr = (uint8_t *)inPtrs[1];
    if (algLayer->layerParams.innerProductParams.wtTranformSize != 0)
    {
      /* NULL or not NULL to just indicate 1 or 2 tensor scenario */
      inBPtr = NULL;
    }

    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;

    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));

    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;
    tidl_printf(1, "Executing reference flow for inner product layer \n");
    /*Refrence code is common if there is two input or one input.
      In real projection kind of scenario, there are two inputs in all models except in caffe
      it is only one tensor input in projection kind of scenarion for caffe
    */
    status = TIDL_innerProductRefProcess(&intAlgObj,
                                         algLayer,
                                         tidlLayer,
                                         params,
                                         &buffParams,
                                         (void *)inPtr,
                                         (void *)inBPtr,
                                         (void *)outPtr);
  }
  else
  {
    if (algLayer->layerParams.innerProductParams.wtTranformMem != NULL)
    {
      status = TIDL_innerProductMMAProcess(commonParams,
                                           algLayer,
                                           tidlLayer,
                                           (void *)(inPtr),
                                           (void *)(algLayer->layerParams.innerProductParams.wtTranformMem),
                                           (void *)(algLayer->layerParams.innerProductParams.biasParamMem),
                                           (void *)(outPtr),
                                           &buffParams);

      /* This call does not use standard WL flow, so need to handle layer level cache writeback explicitly in this layer */
      TIDL_L1DCacheWbInv();
    }
    else
    {
      if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_MATMUL_FORCE_WORKLOADUNIT_NULL)
      {
        algLayer->workloadUnit = NULL;
      }
      status = TIDL_innerProductDeviceProcess(commonParams,
                                              algLayer,
                                              tidlLayer,
                                              inPtrs,
                                              outPtrs);
    }
  }
  return status;
}
