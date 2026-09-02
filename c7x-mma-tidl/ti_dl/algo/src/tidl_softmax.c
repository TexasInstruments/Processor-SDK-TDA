/*
 *
 * Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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
@file    tidl_softmax.c
@brief   This file defines private functions for Softmax layer.
@version 0.1 (Feb 2017) : Initial version [ADK]
----------------------------------------------------------------------------
 */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_device_utils.h"
#include "tidl_softmax.h"
#include <math.h>
#include <float.h>
#include "tidl_softmax_ref.h"
// #include "tidl_softmax_ixX_oxX_priv.h"
#include "tidl_forceNegativeTest.h"
#if defined TIDL_COVERAGE_DEAD_CODE
/**
 * @brief This function is reference implementation of softmax layer
 *
 * @param tidlLayer : Pointer to the common layer parameters
 * @param inPtrs : Pointer to input memory
 * @param outPtr : Pointer to output memory
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param basePrmPtr : Copy of softmax layer parameters
 */

int32_t TIDL_softmaxtProcessSP(
    sTIDL_Layer_t *tidlLayer,
    void *inPtrs,
    void *outPtr,
    sTIDL_DataParams_t *inDataParams[],
    sTIDL_DataParams_t *outDataParams,
    uint8_t *basePrmPtr)
{
  int32_t i, maxIndex = 0;
  float32_tidl *in = (float32_tidl *)inPtrs;
  float32_tidl *out = (float32_tidl *)outPtr;

  int32_t imWidth = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];

  int32_t inOffset = 0;
  int32_t outOffset = 0;

  float32_tidl max = -FLT_MAX;
  float32_tidl denom = (float32_tidl)0;
  float32_tidl maxValue = (float32_tidl)0;
  float32_tidl inDataVal, output;
  float32_tidl *inPtr = &in[inOffset];
  float32_tidl temp, numer;

  // OPENACC(parallel loop)
  for (i = 0; i < imWidth; i++)
  {
    inDataVal = inPtr[i];
    max = (max > inDataVal) ? max : inDataVal;
  }
  // OPENACC(parallel loop)
  for (i = 0; i < imWidth; i++)
  {
    inDataVal = inPtr[i];
    temp = (inDataVal - max);
    denom += (float32_tidl)exp((float64_tidl)(temp));
  }
  // OPENACC(parallel loop)
  for (i = 0; i < imWidth; i++)
  {
    inDataVal = inPtr[i];
    temp = (inDataVal - max);
    numer = (float32_tidl)exp((float64_tidl)(temp));
    output = (numer / denom);
    if (maxValue <= output)
    {
      maxValue = output;
      maxIndex = i;
    }
    out[outOffset + i] = (output);
  }
  tidl_printf(1, "-------Max Index %4d : %7.2f -------\n", maxIndex, out[outOffset + maxIndex]);
  return IALG_EOK;
}
#endif
#if (defined(__C7100__)) || (defined(__C7120__))
static int32_t TIDL_DeviceSupportsLUT(uint32_t targetDeviceName)
{

/* except am62a all devices support LUT */
#if defined(__C7504__)
  /* iLUT is supported for AEN only and not for AM62A.*/
  int32_t doesSupport = 0;
#else
  int32_t doesSupport = 1;
#endif

  return doesSupport;
};

static int32_t TIDL_HwLUTSupportsDatatype(int32_t inputDataType, int32_t outputDataType)
{

  int32_t doesSupport = 0;
  if (((inputDataType == TIDL_SignedChar) || (inputDataType == TIDL_UnsignedChar)) &&
      ((outputDataType == TIDL_SignedChar) || (outputDataType == TIDL_UnsignedChar)))
  {
    doesSupport = 1;
  }

  return doesSupport;
};
#endif
#ifdef HOST_EMULATION
static inline int32_t TIDL_softmaxForceRef(const TIDL_NetworkCommonParams *commonParams,
                                           int32_t layerIdx)
{
  int32_t forceRef = 0;
  const sTIDL_Layer_t *tidlLayer = &commonParams->net->TIDLLayers[layerIdx];
  if (((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_DSP_NATC) == TIDL_FLOW_CTRL_DSP_NATC) &&
      (tidlLayer->outData.elementType == TIDL_SinglePrecFloat))
  {
    /* NatC not currently supported for float output */
    forceRef = 1;
  }
  return forceRef;
}
#endif
#if (defined(__C7100__)) || (defined(__C7120__))
static int32_t TIDL_softmaxInitLUTParams(sTIDL_Network_t *net,
                                         int32_t layerIdx,
                                         sTIDL_AlgLayer_t *algLayer,
                                         sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX],
                                         int32_t forceNegativeTest)
{
  int32_t status = IALG_EOK;

  sTIDL_Layer_t *tidlLayer = &net->TIDLLayers[layerIdx];
  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(net, tidlLayer->inData[0]);
  sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;

  if ((TIDL_HwLUTSupportsDatatype(inDataParams->elementType, outDataParams->elementType) != 0) && (TIDL_DeviceSupportsLUT(net->deviceName) != 0))
  {
    void *memPtr = NULL;
    /* LUT implementation requires that the L1D memory be reserved fully for the kernel which it will use for the L1D during process call.
      By allocating all the L1D scratch memory upfront, we prevent any following part of the init code from using the L1D mem as scratch
      space. */
    memPtr = (void *)TIDL_getMemoryChunkFromSysmem(sysMems,
                                                   sysMems[TIDL_SYSMEM_L1_SCRATCH].size,
                                                   TIDL_ALIGNMENT_SIZE,
                                                   (uint32_t)TIDL_SYSMEM_L1_SCRATCH,
                                                   (uint32_t)IALG_SCRATCH);
    if (forceNegativeTest == TIDL_SAFETY_FLAG_SOFTMAX_FORCE_MEMPTR_NULL)
    {
      memPtr = NULL;
    }
    if (memPtr == NULL)
    {
      tidl_printf(0, "LUT implementation of batchNorm expects L1 mem to be reserved entirely for the LUT. Please make sure L1 memory is not used for this layer \n");
      status = TIDL_ERR_FAILURE;
    }
  }

  return status;
}
#endif
/**
*******************************************************************************
*  \brief      Function to supply memory requirement for Softmax layer to user
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
int32_t TIDL_softmaxAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;
  #ifdef HOST_EMULATION
  int32_t forceRef = TIDL_softmaxForceRef(commonParams, layerIdx);
  if (((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != TIDL_FLOW_CTRL_REF_ONLY) && (forceRef != 1))
  #endif
  {
    status = TIDL_deviceUtilsCommonAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  #ifdef HOST_EMULATION
  else
  {
    status = TIDL_deviceCommonRefAlloc(layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  #endif
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
int32_t TIDL_softmaxInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            sTIDL_AlgLayer_t *algLayer,
                            int32_t layerIdx,
                            uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                            void **outPtr)
{
  int32_t status = IALG_EOK;
  #ifdef HOST_EMULATION
  int32_t forceRef = TIDL_softmaxForceRef(commonParams, layerIdx);

  if (((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != TIDL_FLOW_CTRL_REF_ONLY) && (forceRef != 1))
  #endif
  {
    /* presently, L1D would be allocated for the LUT table in the kernel code from x0000.
    We are here blocking L1D here for the LUT table and the layer handle would be stored in MSMC probably.
    LUT will be initialized from the beginning of the L1D.
    Degradation on configuration 64x197x197 softamx is of 0.23 ms.
    Suggestion: This degration can be handled by allocating/initializing LUT at a place mutually excluse with the handles in L1D
    i.e x2000 or x3000 instead of x0000 in case of ahp(16K L1D)*/
    #if (defined(__C7100__)) || (defined(__C7120__))
    status = TIDL_softmaxInitLUTParams(commonParams->net,
                                       layerIdx,
                                       algLayer,
                                       commonParams->tidlCommonParams->sysMems,
                                       commonParams->createParams->forceNegativeTest);

    if (status == IALG_EOK)
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
  }
  #ifdef HOST_EMULATION
  else
  {
    status = TIDL_deviceCommonRefInit(layerSpecificParams,
                                      commonParams,
                                      algLayer,
                                      layerIdx,
                                      memory,
                                      memorySize,
                                      outPtr);
  }
  #endif

  return status;
}

/* API to be used in import code for decicions between Widthwise and Heighwise and check if transpose is supported or not
int32_t TIDL_getSoftmaxAxis(int32_t numBatches,
                              int32_t numChannels,
                              int32_t height,
                              int32_t width,
                              int32_t dataType,
                              int32_t inAxis)
{
  int32_t computeVal;

  if(dataType == TIDL_INT8 || dataType == TIDL_UINT8)
  {
    if(inAxis == TIDL_DIM_WIDTH)
    {
      computeVal = ( ( TIDL_CEIL_DIV((1*width+21)*height,TIDL_SIMD_WIDTH)
                      + (TIDL_SIMD_WIDTH*height + 5)
                      + TIDL_CEIL_DIV((4*width + 31)*height,TIDL_SIMD_WIDTH)
                      + ((height)*30)
                      + TIDL_CEIL_DIV((2*width + 24)*height,(TIDL_SIMD_WIDTH/4)) )*numChannels + 34*numChannels) * numBatches;
    }
    else if(inAxis == TIDL_DIM_HEIGHT)
    {
      computeVal = ( ( TIDL_CEIL_DIV((1*height+1)*width,TIDL_SIMD_WIDTH)
                      + TIDL_CEIL_DIV((4*height + 31)*width,TIDL_SIMD_WIDTH)
                      + ((width/(TIDL_SIMD_WIDTH/4))*30)
                      + TIDL_CEIL_DIV((1*height + 26)*width,TIDL_SIMD_WIDTH) )*numChannels + 540*numChannels) * numBatches;
    }
  }
  else if(dataType == TIDL_INT16 || dataType == TIDL_UINT16)
  {
    if(inAxis == TIDL_DIM_WIDTH)
    {
      computeVal = ( ( TIDL_CEIL_DIV((1*width+21)*height,TIDL_SIMD_WIDTH)
                      + (TIDL_SIMD_WIDTH*height + 5)
                      + TIDL_CEIL_DIV((8*width + 65)*height,(TIDL_SIMD_WIDTH/4))
                      + ((height)*30)
                      + TIDL_CEIL_DIV((2*width + 24)*height,(TIDL_SIMD_WIDTH/4)) )*numChannels + 34*numChannels) * numBatches;
    }
    else if(inAxis == TIDL_DIM_HEIGHT)
    {
      computeVal = ( ( TIDL_CEIL_DIV((1*height+1)*width,TIDL_SIMD_WIDTH)
                    + TIDL_CEIL_DIV((8*height + 65)*width,(TIDL_SIMD_WIDTH/4))
                    + ((width/(TIDL_SIMD_WIDTH/4))*30)
                    + TIDL_CEIL_DIV((1*height + 26)*width,TIDL_SIMD_WIDTH) )*numChannels + 540*numChannels) * numBatches;

    }
  }

  return computeVal;
}

int32_t isTransposeSupportedSoftmax(int32_t inAxis,
                                      int32_t outTensorScale,
                                      int32_t targetDeviceName)
{
  int32_t isTransposeSupported = 0;

  if(inAxis == TIDL_DIM_HEIGHT)
  {
    if( ((outTensorScale/inAxis) > SAT_HI_UINT8) || (targetDeviceName == TIDL_TDA4VM))
    {
      isTransposeSupported = 0;
    }
    else
    {
      isTransposeSupported = 1;
    }
  }
  else if(inAxis == TIDL_DIM_WIDTH)
  {
    isTransposeSupported = 0;
  }

  return isTransposeSupported;
}*/

int32_t TIDL_softmaxProcessNew(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
  int32_t status = IALG_EOK;

  #ifdef HOST_EMULATION
  int32_t forceRef = TIDL_softmaxForceRef(commonParams, layerIdx);
  if ((((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != TIDL_FLOW_CTRL_REF_ONLY) && (forceRef != 1))
  #endif
  {
    status = TIDL_deviceUtilsCommonProcess(commonParams,
                                           algLayer,
                                           tidlLayer,
                                           inPtrs,
                                           outPtrs,
                                           layerIdx);
  }
  #ifdef HOST_EMULATION
  else
  {
    sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(commonParams->createParams->net, tidlLayer->inData[0]);

    int32_t imWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];
    int32_t inPitch = (int32_t)inDataParams->pitch[TIDL_LINE_PITCH];
    int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
    int32_t inBatchPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
    int32_t numBatches = (int32_t)inDataParams->dimValues[TIDL_DIM_BATCH]; /*Batches  */
    int32_t outBatchPitch = tidlLayer->outData.pitch[TIDL_ROI_PITCH];
    int32_t outElementType = tidlLayer->outData.elementType;
    int32_t axis = tidlLayer->layerParams.softMaxParams.axis;
    int32_t outTranspose = tidlLayer->layerParams.softMaxParams.outTranspose;

    if (commonParams->createParams->net->weightsElementSize == 4)
    {
      status = TIDL_softmaxRefProcessFloat(tidlLayer, (void *)((int8_t *)inPtrs[0]), (void *)((int8_t *)outPtrs[0]), &inDataParams, &tidlLayer->outData, (uint8_t *)commonParams->createParams->net, outTranspose);
    }
    else if (outElementType == TIDL_SinglePrecFloat)
    {
      if (inDataParams->elementType == TIDL_SignedChar)
      {

        TIDL_softmaxRefProcess(
            (int8_t *)(inPtrs[0]),
            (float32_tidl *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else if (inDataParams->elementType == TIDL_UnsignedChar)
      {

        TIDL_softmaxRefProcess(
            (uint8_t *)(inPtrs[0]),
            (float32_tidl *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else if (inDataParams->elementType == TIDL_SignedShort)
      {

        TIDL_softmaxRefProcess(
            (int16_t *)(inPtrs[0]),
            (float32_tidl *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else if (inDataParams->elementType == TIDL_UnsignedShort)
      {

        TIDL_softmaxRefProcess(
            (uint16_t *)(inPtrs[0]),
            (float32_tidl *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else if (inDataParams->elementType == TIDL_BFloat16)
      {
        TIDL_softmaxRefProcessBFloat16<bfloat16_tidl, float32_tidl>(
              tidlLayer, 
              (bfloat16_tidl *)inPtrs[0], 
              (float32_tidl *)outPtrs[0], 
              &inDataParams, 
              &tidlLayer->outData, 
              (uint8_t *)commonParams->createParams->net, 
              outTranspose);
      }
      else
      {
        /* TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_002 */
        status = TIDL_ERR_INVALID_TYPE;
      }
    }
    else if (outElementType == TIDL_SignedChar)
    {
      if (inDataParams->elementType == TIDL_SignedChar)
      {

        TIDL_softmaxRefProcess(
            (int8_t *)(inPtrs[0]),
            (int8_t *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else if (inDataParams->elementType == TIDL_UnsignedChar)
      {

        TIDL_softmaxRefProcess(
            (uint8_t *)(inPtrs[0]),
            (int8_t *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else
      {
        /* TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_003 */
        status = TIDL_ERR_INVALID_TYPE;
      }
    }
    else if (outElementType == TIDL_UnsignedChar)
    {
      if (inDataParams->elementType == TIDL_SignedChar)
      {

        TIDL_softmaxRefProcess(
            (int8_t *)(inPtrs[0]),
            (uint8_t *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else if (inDataParams->elementType == TIDL_UnsignedChar)
      {

        TIDL_softmaxRefProcess(
            (uint8_t *)(inPtrs[0]),
            (uint8_t *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else
      {
        /* TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_003 */
        status = TIDL_ERR_INVALID_TYPE;
      }
    }
    // 16-bit:
    else if (outElementType == TIDL_UnsignedShort)
    {
      if (inDataParams->elementType == TIDL_SignedShort)
      {

        TIDL_softmaxRefProcess(
            (int16_t *)(inPtrs[0]),
            (uint16_t *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else if (inDataParams->elementType == TIDL_UnsignedShort)
      {

        TIDL_softmaxRefProcess(
            (uint16_t *)(inPtrs[0]),
            (uint16_t *)(outPtrs[0]),
            inDataParams->tensorScale,
            imWidth,
            inPitch,
            outPitch,
            numBatches,
            inBatchPitch,
            outBatchPitch,
            inDataParams,
            &tidlLayer->outData,
            axis,
            outTranspose);
      }
      else
      {
        /* TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_003 */
        status = TIDL_ERR_INVALID_TYPE;
      }
    }
    else if (outElementType == TIDL_BFloat16)
    {
      if (inDataParams->elementType == TIDL_BFloat16)
      {
        TIDL_softmaxRefProcessBFloat16<bfloat16_tidl, bfloat16_tidl>(
              tidlLayer, 
              (bfloat16_tidl *)inPtrs[0], 
              (bfloat16_tidl *)outPtrs[0], 
              &inDataParams, 
              &tidlLayer->outData, 
              (uint8_t *)commonParams->createParams->net, 
              outTranspose);
      }
      else
      {
        /* TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_002 */
        status = TIDL_ERR_INVALID_TYPE;
      }
    }
    else
    {
      tidl_printf(0, "Unsupported combination!\n");
      status = TIDL_ERR_INVALID_TYPE;
    }
  }
  #endif
  return status;
}
