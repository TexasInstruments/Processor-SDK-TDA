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
@file    tidl_conv2d_base.c
@brief   This file contains basic functions required by Convolution Layer
@version 0.1 (Oct 2016) : Initial version [ADK]
@version 0.5 (Jan 2017) : Cleaned up [ADK]
----------------------------------------------------------------------------
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <limits>

#include "tidl_alg_int.h"
#include "tidl_conv2d.h"
#include "tidl_commonUtils.h"
#include "tidl_types.h"
#include "tidl_device_functions.h"

#include <math.h>
#include <stdlib.h>
#include "tidl_conv2d_mma_i.h"
#include "tidl_deviceInfo.h"
#include "gc_helper.h"
#include "tidl_conv2d_ref.h"

#define USE_16BIT_BIAS_FOR_8BIT_MODE (1)


/**
 * @brief Function to check if perChannelQuantizationEnabled
 *
 * @param convParams : conv layer parameters
 * @param calibrationOption : indicates calibration Options
 * @return : PerChannelQuantization Enabled or not
 */
int32_t isPerChannelQuantizationEnabled(const sTIDL_ConvParams_t * convParams,
                                                 int32_t calibrationOption)
{
  int32_t isEnabled = 0;

  if((((convParams->numInChannels/convParams->numGroups) == 1) &&
    ((convParams->numOutChannels/convParams->numGroups) == 1)) &&
    (( (uint32_t)calibrationOption & (uint32_t)TIDL_CalibOptionPerChannelWeightQuantization) ==
       (uint32_t)TIDL_CalibOptionPerChannelWeightQuantization ))
  {
    isEnabled = 1;
  }
  return isEnabled;
}

#define TIDL_CONV2D_ALIGNMENT_SIZE (128)

/* If basePtr is NULL, then this function will return the size without allocating, otherwise
it will allocate the size and return */
int32_t TIDL_conv2dPersistentMemAllocateAndComputeSize(void   *basePtr,
                                                                       const sTIDL_ConvParams_t *conv2dparams,
                                                                       const TIDL_LayerSpecificParams *layerSpecificParams,
                                                                       const TIDL_NetworkCommonParams *commonParams,
                                                                       int32_t layerIdx,
                                                                       sTIDL_AlgLayer_t *algLayer)
{
  int32_t currOffset = 0;
  int32_t layerType = commonParams->net->TIDLLayers[layerIdx].layerType;
  int32_t layerKernelType = commonParams->net->TIDLLayers[layerIdx].layerKernelType;
  void * ptr;
  int32_t biasParamSize;
  void *temp_basePtr = NULL;

  /* Algin the base pointer, extra memory for this is already requested */
  if ( basePtr != NULL )
  {
    temp_basePtr = (void *)TIDL_ALIGN_CEIL((intptr_t)basePtr, TIDL_CONV2D_ALIGNMENT_SIZE);
  }



  /* For target flow compute and set handle required for processing */
  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY ) == 0U)
  #endif
  {
    currOffset = TIDL_conv2dAllocateHandleAndComputeSize(temp_basePtr,
                                                        conv2dparams,
                                                        layerSpecificParams,
                                                        commonParams,
                                                        layerIdx,
                                                        algLayer);
  }
  
  biasParamSize = TIDL_getBiasParamSize(commonParams->net->deviceName, conv2dparams->numOutChannels);

  //TEMP : [TIDL-3356] handling case where GC doesn't give bias buffer where required. Expected to be reomved when GC handles
  // this case.
  /*********************************************************************************************/
  biasParamSize += (conv2dparams->numOutChannels * ((int32_t)( ((int32_t)(sizeof(int32_t))))));
  if (TIDL_getProcessingElementSizeInBytes(&commonParams->net->TIDLLayers[layerIdx]) == 2)
  {
    biasParamSize += (conv2dparams->numOutChannels * ((int32_t)( ((int32_t)(sizeof(int32_t))))));
  }
  #if defined (__C7120__) || defined (__C7604__)
  if(TIDL_isBiasHighPrecision(commonParams->net->deviceName) == TRUE)
  {
    biasParamSize += 2*(conv2dparams->numOutChannels * ((int32_t)( ((int32_t)(sizeof(int8_t))))));
  }
  #endif
  #if defined (__C7504__) || defined (__C7524__)
  biasParamSize += 2*(conv2dparams->numOutChannels * ((int32_t)( ((int32_t)(sizeof(int8_t))))));
  #endif
  /*********************************************************************************************/

  TIDL_AllocatePtr((intptr_t)temp_basePtr,
                   &currOffset,
                   biasParamSize,
                   TIDL_CONV2D_ALIGNMENT_SIZE,
                   (void **)&ptr);

  if (( ptr != NULL ) && (algLayer != NULL))
  {
    algLayer->layerParams.convParams.biasParamMem  = ptr;
    algLayer->layerParams.convParams.biasParamSize = biasParamSize;
  }

  if ((isPerChannelQuantizationEnabled(conv2dparams, commonParams->net->calibrationOption) == 1) &&
      (layerType == TIDL_ConvolutionLayer))
  {
    int32_t outRoundBitsSize, biasBSize;
    outRoundBitsSize = conv2dparams->numOutChannels * (int32_t)sizeof(uint8_t);
    biasBSize        = conv2dparams->numOutChannels * (int32_t)sizeof(int32_t);
    TIDL_AllocatePtr((intptr_t)temp_basePtr,
                 &currOffset,
                 outRoundBitsSize,
                 TIDL_CONV2D_ALIGNMENT_SIZE,
                 (void **)&ptr);
    if (( ptr != NULL ) && (algLayer != NULL))
    {
      algLayer->layerParams.convParams.outRoundBitsPtr = ptr;
    }

    TIDL_AllocatePtr((intptr_t)temp_basePtr,
                 &currOffset,
                 biasBSize,
                 TIDL_CONV2D_ALIGNMENT_SIZE,
                 (void **)&ptr);
    if (( ptr != NULL ) && (algLayer != NULL))
    {
      algLayer->layerParams.convParams.biasBPtr = ptr;
    }
  }

  /*Deprecate TFL:*/
  if( (TIDL_QuantStyleAsymNP2_TFL == commonParams->net->quantizationStyle) ||
      (TIDL_isKernelHighPrecision(layerKernelType) != (int32_t)FALSE) )
  {
    int32_t mmaScaleShiftBufSize = conv2dparams->numOutChannels * (int32_t)sizeof(uint8_t);

    TIDL_AllocatePtr((intptr_t)temp_basePtr,
                 &currOffset,
                 mmaScaleShiftBufSize,
                 TIDL_CONV2D_ALIGNMENT_SIZE,
                 (void **)&ptr);

    if (( ptr != NULL ) && (algLayer != NULL))
    {
      algLayer->layerParams.convParams.mmaShiftsPtr = ptr;
    }

    TIDL_AllocatePtr((intptr_t)temp_basePtr,
                 &currOffset,
                 mmaScaleShiftBufSize,
                 TIDL_CONV2D_ALIGNMENT_SIZE,
                 (void **)&ptr);

    if (( ptr != NULL ) && (algLayer != NULL))
    {
      algLayer->layerParams.convParams.mmaScalesPtr = ptr;
    }
  }

  currOffset += TIDL_CONV2D_ALIGNMENT_SIZE; /*Request extra memory to align the base pointers */

  return currOffset;

}


int32_t TIDL_conv2dFillPerChannelParamters(const sTIDL_ConvParams_t *conv2dparams,
                                                     const TIDL_NetworkCommonParams *commonParams,
                                                     const sTIDL_AlgLayer_t               *algLayer,
                                                     int32_t layerIdx)
{
  int32_t status = TIDL_SUCCESS;
  #ifdef HOST_EMULATION
  if(((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == 0U)
  #endif
  {
    /* REF for staisPerChannelQuantizationEnabledts flow this will be happening with the convlution later ref process call */
    if (isPerChannelQuantizationEnabled(conv2dparams, commonParams->net->calibrationOption) == 1 )
    {

      sTIDL_DataParams_t * dataParams = TIDL_getDataParams(commonParams->net,
                                                           commonParams->net->TIDLLayers[layerIdx].inData[0]);
      int32_t chIdx;
      float32_tidl perChannelAccScale;
      float32_tidl tempRoundBits = 0.0f;
      float32_tidl * perChannelWeightScalePtr =
              (float32_tidl *)get_int8_t_pointer((int8_t *)(commonParams->net),
                                                conv2dparams->perChannelWeightScaleOffset);
      uint8_t * roundBitsPtr = (uint8_t *)(algLayer->layerParams.convParams.outRoundBitsPtr);

      if ( dataParams->tensorScale != 0.0f )
      {
        for(chIdx = 0; chIdx < conv2dparams->numInChannels; chIdx++)
        {
          perChannelAccScale = perChannelWeightScalePtr[chIdx] * dataParams->tensorScale;
          tempRoundBits = (log((float64_tidl)perChannelAccScale /
            commonParams->net->TIDLLayers[layerIdx].outData.tensorScale)/log(2.0));
          roundBitsPtr[chIdx] = (uint8_t)tempRoundBits;
          //printf("Layer Idx %d, roundBitsPtr %d \n", layerIdx, chIdx);
        }
      }
      else
      {
        status = TIDL_ERR_NOT_IMPLEMENTED;
      }
    }
  }
  return status;
}


void TIDL_conv2dSetupBiasPointers(sTIDL_ConvParams_t *conv2dparams,
                                   const TIDL_LayerSpecificParams *specificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   sTIDL_AlgLayer_t               *algLayer,
                                   int32_t layerIdx)
{
  int32_t layerType = commonParams->net->TIDLLayers[layerIdx].layerType;
  int32_t layerKernelType = commonParams->net->TIDLLayers[layerIdx].layerKernelType;
  uint32_t targetDevice = commonParams->net->deviceName;
  int32_t flowType = 0;
  float32_tidl inScaleFactor;

  if(specificParams->workloadUnit != NULL)
  {
    flowType = TIDL_getDataFlowType(specificParams->workloadUnit, commonParams->gcHelperHandle);
  }
  
  if ((TIDL_isPadOTF(targetDevice) != 0U) && (layerType != TIDL_ConvolutionLayer))
  {
    /* Layers other than convolution that have bias splitting (deconv, depth2Space) need to use non-OTF bias split
      as they do not really support OTF in the first place */
    targetDevice = TIDL_disablePadOTF(targetDevice);
  }

  sTIDL_DataParams_t * inDataParams = TIDL_getDataParams(commonParams->net,
    commonParams->net->TIDLLayers[layerIdx].inData[0]);

  //Deprecate TFL:
  if(TIDL_QuantStyleAsymNP2_TFL == commonParams->net->quantizationStyle)
  {
    //Populate scale and shift for MMA for convolution:
    //Scale ratio
    float32_tidl scaleRatio = 1;
    float32_tidl* weightScalePtr = (float32_tidl *)get_int8_t_pointer((int8_t *)(commonParams->net), conv2dparams->weightScales);
    float32_tidl scale_x = inDataParams->tensorScale;
    float32_tidl scale_y = commonParams->net->TIDLLayers[layerIdx].outData.tensorScale;
    int32_t axisCtr;
    uint8_t* scaleBuffer = (uint8_t*)algLayer->layerParams.convParams.mmaScalesPtr;
    uint8_t* shiftBuffer = (uint8_t*)algLayer->layerParams.convParams.mmaShiftsPtr;
    //Assuming per axis: (should be single iteration for non per ch) !Replace with axis length
    //Temp logic:
    for(axisCtr = 0; axisCtr < conv2dparams->numOutChannels; axisCtr++)
    {
      scaleRatio = (scale_x * weightScalePtr[axisCtr])/scale_y;
      TIDL_getMMAv2_ScaleAndShift(scaleRatio, &scaleBuffer[axisCtr],&shiftBuffer[axisCtr]);
    }
    int32_t * orgbiasptr = (int32_t *)get_int8_t_pointer((int8_t *)(commonParams->net), conv2dparams->bias);
    orgbiasptr = orgbiasptr + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
    int32_t * biasPtr  = (int32_t *)algLayer->layerParams.convParams.biasParamMem;
    //Simple copy over of bias from model to local memory.
    for(axisCtr = 0; axisCtr < conv2dparams->numOutChannels; axisCtr++)
    {
      biasPtr[axisCtr] = orgbiasptr[axisCtr];
    }
  }
  else if(TIDL_isKernelHighPrecision(layerKernelType) != (int32_t)FALSE)
  {
    int32_t axisCtr;
    if(commonParams->net->TIDLLayers[layerIdx].weightsElementSizeInBits <= 8)
    {
      /*8-bit & 32-bit bias*/
      int32_t * orgBiasptr = (int32_t *)get_int8_t_pointer((int8_t *)(commonParams->net), conv2dparams->derivedBias);
      orgBiasptr = orgBiasptr + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
      int32_t * biasPtr  = (int32_t *)algLayer->layerParams.convParams.biasParamMem;
      for(axisCtr = 0; axisCtr < conv2dparams->numOutChannels; axisCtr++)
      {
        biasPtr[axisCtr] = orgBiasptr[axisCtr];
      }
    }
    else
    {
      /*16-bit & 64-bit bias*/
      int64_t * orgBiasptr = (int64_t *)get_int8_t_pointer((int8_t *)(commonParams->net), conv2dparams->derivedBias);
      orgBiasptr = orgBiasptr + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
      int64_t * biasPtr  = (int64_t *)algLayer->layerParams.convParams.biasParamMem;
      for(axisCtr = 0; axisCtr < conv2dparams->numOutChannels; axisCtr++)
      {
        biasPtr[axisCtr] = orgBiasptr[axisCtr];
      }
    }
    /*Copy over derived scales & shifts*/
    uint8_t* derivedScales = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), conv2dparams->derivedScales + algLayer->layerParams.convParams.biasMultiCoreExtraOffset);
    uint8_t* derivedShifts = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), conv2dparams->derivedShifts + algLayer->layerParams.convParams.biasMultiCoreExtraOffset);
    uint8_t* scaleBuffer = (uint8_t*)algLayer->layerParams.convParams.mmaScalesPtr + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
    uint8_t* shiftBuffer = (uint8_t*)algLayer->layerParams.convParams.mmaShiftsPtr + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
    for(axisCtr = 0; axisCtr < conv2dparams->numOutChannels; axisCtr++)
    {
      scaleBuffer[axisCtr] = derivedScales[axisCtr];
      shiftBuffer[axisCtr] = derivedShifts[axisCtr];
    }
  }
  else
  {
    int16_t * orgbiasptr = (int16_t *)get_int8_t_pointer((int8_t *)(commonParams->net), conv2dparams->bias);
    orgbiasptr = orgbiasptr + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
    int8_t * biasPtr  = (int8_t *)algLayer->layerParams.convParams.biasParamMem;
    if(conv2dparams->enableBias == 1)
    {
      inScaleFactor = (conv2dparams->weightScale * inDataParams->tensorScale) / conv2dparams->biasScale;
    }
    else if (layerType != TIDL_ConvolutionLayer)
    {
      inScaleFactor = 1.0f;
    }
    else
    {
      inScaleFactor = 0.0f;
    }
  #if USE_16BIT_BIAS_FOR_8BIT_MODE
    if(commonParams->net->TIDLLayers[layerIdx].weightsElementSizeInBits <= 8)
    {
      if (isPerChannelQuantizationEnabled(conv2dparams, commonParams->net->calibrationOption) == 1 )
      {
        int32_t chIdx;
        float32_tidl * perChannelWeightScalePtr =((float32_tidl *) ((int8_t *)(commonParams->net) +
                                                  conv2dparams->perChannelWeightScaleOffset) )+ algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
        inDataParams  =
          &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
        int32_t * biasBPtr = (int32_t *)(algLayer->layerParams.convParams.biasBPtr);

        int32_t biasB;
        for ( chIdx = 0; chIdx < conv2dparams->numOutChannels; chIdx++ )
        {
          inScaleFactor = (perChannelWeightScalePtr[chIdx] * inDataParams->tensorScale) / conv2dparams->biasScale;
          TIDL_conv2dBiasSplit(targetDevice,
                              flowType,
                              algLayer->layerParams.convParams.procElemSize,
                              orgbiasptr + chIdx,
                              (int8_t*)biasPtr + chIdx,
                              &biasB,
                              1,
                              inScaleFactor,
                              TIDL_SAT_LO_INT16,
                              TIDL_SAT_HI_INT16,
                              0xFF,
                              TIDL_getDatElementSign(inDataParams->elementType));
          biasBPtr[chIdx] = biasB;

        }
      }
      else
      {
        //Wrapper function for the basic split logic (This is an overloaded function, deviceName is the point of overload)
        TIDL_conv2dBiasSplit(targetDevice,
                            flowType,
                            algLayer->layerParams.convParams.procElemSize,
                            orgbiasptr,
                            (int8_t *)biasPtr,
                            &conv2dparams->biasB,
                            conv2dparams->numOutChannels,
                            inScaleFactor,
                            TIDL_SAT_LO_INT16,
                            TIDL_SAT_HI_INT16,
                            0xFF,
                            TIDL_getDatElementSign(inDataParams->elementType));
      }
    }
    else
  #endif
    {
    /* 16 bit weights*/
      if ((isPerChannelQuantizationEnabled(conv2dparams, commonParams->net->calibrationOption) == 1) &&
          (layerType == TIDL_ConvolutionLayer))   /*No channelwise quantization for deconvolution */
      {
        int32_t chIdx;
        float32_tidl * perChannelWeightScalePtr =((float32_tidl *) ((int8_t *)(commonParams->net) +
                                                  conv2dparams->perChannelWeightScaleOffset) ) + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
        inDataParams  =
          &commonParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData;
        int32_t * biasBPtr = (int32_t *)(algLayer->layerParams.convParams.biasBPtr);

        int32_t biasB;
        for ( chIdx = 0; chIdx < conv2dparams->numOutChannels; chIdx++ )
        {
          inScaleFactor = (perChannelWeightScalePtr[chIdx] * inDataParams->tensorScale) / conv2dparams->biasScale;
          TIDL_conv2dBiasSplit(targetDevice,
                              flowType,
                              algLayer->layerParams.convParams.procElemSize,
                              orgbiasptr + chIdx,
                              (int16_t*)biasPtr + chIdx,
                              &biasB,
                              1,
                              inScaleFactor,
                              TIDL_SAT_LO_INT32,
                              TIDL_SAT_HI_INT32,
                              0xFFFF,
                              TIDL_getDatElementSign(inDataParams->elementType));
          biasBPtr[chIdx] = biasB;
        }
      }
      else
      {
        int32_t maxBiasB = 0xFFFF;
        TIDL_conv2dBiasSplit(targetDevice,
                            flowType,
                            algLayer->layerParams.convParams.procElemSize,
                            orgbiasptr,
                            (int16_t*)biasPtr,
                            &conv2dparams->biasB,
                            conv2dparams->numOutChannels,
                            inScaleFactor,
                            TIDL_SAT_LO_INT32,
                            TIDL_SAT_HI_INT32,
                            maxBiasB,
                            TIDL_getDatElementSign(inDataParams->elementType));
      }
    }
  }
  return;
}


int32_t TIDL_conv2dIsSplitGroupedWl(const sWorkloadUnit_t *workloadUnit,
                                const sGCHelperHandle * gcHelperHandle,
                                const sTIDL_ConvParams_t *conv2dparams)
{
  int32_t isGroupSplit = 0;

  for (int32_t linkIdx = 0; linkIdx < workloadUnit->numLinks; linkIdx++)
  {
    sLink_t *linkPtrList[MAX_LINKS_PER_WL];
    getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
    const sLink_t *link = linkPtrList[0];
    if (link->type == (int32_t)LINK_PROC)
    {
      const sJoint_t *coeffJoint;
      if (( getBufParamsFromBufIndex(gcHelperHandle, link->src[0].bufDBindex)->type == (int32_t)BUF_FM_FULL) ||
          ( getBufParamsFromBufIndex(gcHelperHandle, link->src[0].bufDBindex)->type == (int32_t)BUF_FM_IN_PART) )
      {
        coeffJoint = &link->src[1];
      }
      else
      {
        #ifdef TIDL_COVERAGE_DEAD_CODE_NO_TEST
        coeffJoint = &link->src[0];
        #endif
        isGroupSplit = NOT_VALID;
      }
      if(isGroupSplit != NOT_VALID)
      {
        isGroupSplit = TIDL_conv2dRowFlowIsGrpOptAvailable(coeffJoint, conv2dparams, TIDLCONV_TYPE_CONV);
        break;
      }
    }
  }

  return isGroupSplit;
}

/**
*******************************************************************************
*  \brief      Function to supply memory requirement for convolution layer to user
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
int32_t TIDL_conv2dAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx,
                                   int32_t memorySize[TIDL_LAYER_MEMORY_MAX])

{
  int32_t status = IALG_EOK;
  const sTIDL_ConvParams_t *conv2dparams = &commonParams->net->TIDLLayers[layerIdx].layerParams.convParams;

  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_conv2dRefAlloc(conv2dparams, layerSpecificParams, commonParams, layerIdx, memorySize);
  }
  else
  #endif
  {
    status = TIDL_conv2dDspAllocNew(conv2dparams, layerSpecificParams, commonParams, layerIdx, memorySize);
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
int32_t TIDL_conv2DInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr)

{
  int32_t status = IALG_EOK;
  sTIDL_ConvParams_t *conv2dparams = &commonParams->net->TIDLLayers[layerIdx].layerParams.convParams;

  // ((TIDL_LayerSpecificParams *)layerSpecificParams)->isMixedPrecEnabled = 1U;
  algLayer->layerParams.convParams.procElemSize =
          TIDL_getProcessingElementSizeInBytes(&commonParams->net->TIDLLayers[layerIdx]);
  algLayer->layerParams.convParams.startRowNumberInTensor = 0; /* Need to take care of ST */

  if ((algLayer->gcHelperHandle != NULL) && (commonParams->net->TIDLLayers[layerIdx].layerType == TIDL_ConvolutionLayer))
  {
    int32_t isColFlow = ( ((conv2dparams->numInChannels / conv2dparams->numGroups) == 1) &&
                          ((conv2dparams->numOutChannels / conv2dparams->numGroups) == 1) );
    status = TIDL_conv2dIsSplitGroupedWl(algLayer->workloadUnit, algLayer->gcHelperHandle, conv2dparams);
    if(status != NOT_VALID)
    {
      if ( ( status != 1) && (isColFlow != 1))
      {
        /* Assumption is that workload will get split across groups in case full weight map cannot fit in L2
        * As a result, we will need to do padding only for one group and not the full buffer
        */
        algLayer->wlPadParams.outChannels =  conv2dparams->numOutChannels / conv2dparams->numGroups;
      }
    }
  }
  if(status != NOT_VALID)
  {
    /*Deprecate TFL*/
    /*Pre-populated for TIDL Asym PTQ*/
    if(TIDL_QuantStyleAsymNP2_TFL == commonParams->net->quantizationStyle)
    {
      int32_t satLow, satHigh;
      conv2dparams->minPSAT = round((float64_tidl)commonParams->net->TIDLLayers[layerIdx].actParams.clipMin/commonParams->net->TIDLLayers[layerIdx].outData.tensorScale) + (float32_tidl)commonParams->net->TIDLLayers[layerIdx].outData.tensorZeroPoint;
      conv2dparams->maxPSAT = round((float64_tidl)commonParams->net->TIDLLayers[layerIdx].actParams.clipMax/commonParams->net->TIDLLayers[layerIdx].outData.tensorScale) + (float32_tidl)commonParams->net->TIDLLayers[layerIdx].outData.tensorZeroPoint;
      TIDL_getSaturationLimits(commonParams->net->TIDLLayers[layerIdx].outData.elementType, &satLow, &satHigh);
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      Here conv2dparams->minPSAT never less than satLow and conv2dparams->maxPSAT never greater than satHigh.
      <justification end> */
      conv2dparams->minPSAT = (conv2dparams->minPSAT < satLow)? satLow : conv2dparams->minPSAT;
      conv2dparams->maxPSAT = (conv2dparams->maxPSAT > satHigh)? satHigh : conv2dparams->maxPSAT;
      /* LDRA_JUSTIFY_END */
    }


    (void)memset(&algLayer->layerParams.convParams.virtualPad, 0, sizeof(algLayer->layerParams.convParams.virtualPad));

    if((TIDL_isPadOTF(commonParams->net->deviceName) == TRUE) && (algLayer->workloadUnit != NULL))
    {
      /*Set the default tensor padding:*/
      algLayer->layerParams.convParams.virtualPad.padT = algLayer->workloadUnit->procParam.padVirtual[TOP_DIR ];
      algLayer->layerParams.convParams.virtualPad.padL = algLayer->workloadUnit->procParam.padVirtual[LEFT_DIR];
      algLayer->layerParams.convParams.virtualPad.padR = algLayer->workloadUnit->procParam.padVirtual[RIGHT_DIR ];
      algLayer->layerParams.convParams.virtualPad.padB = algLayer->workloadUnit->procParam.padVirtual[BOTTOM_DIR];
    }

    /* Setup various pointers needed by convolution layers */
    #ifdef HOST_EMULATION
    if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
    {
      status = TIDL_conv2dRefInit(conv2dparams,
                                  layerSpecificParams,
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
      status = TIDL_conv2dDspInitNew(conv2dparams,
                                    layerSpecificParams,
                                    commonParams,
                                    algLayer,
                                    layerIdx,
                                    memory,
                                    memorySize);
    }
  }

  return status;
}

int32_t TIDL_conv2dProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx)
{
  int32_t status = IALG_EOK;
  sTIDL_ConvParams_t *conv2dparams = &commonParams->net->TIDLLayers[layerIdx].layerParams.convParams;

  /* Setup various pointers needed by convolution layers */
  #ifdef HOST_EMULATION
  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
  {
    status = TIDL_conv2dRefProcess(conv2dparams,
                                   commonParams,
                                   algLayer,
                                   tidlLayer,
                                   inPtrs[0],
                                   outPtrs[0],
                                   layerIdx);
  }
  else
  #endif
  {
    if(commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_BASE_FORCE_ALGLAYER_WL_NULL)
    {
      algLayer->workloadUnit = NULL;
    }

    status = TIDL_conv2dDspProcess(conv2dparams,
                                   commonParams,
                                   algLayer,
                                   tidlLayer,
                                   inPtrs[0],
                                   outPtrs[0],
                                   layerIdx);

  }

  return status;
}
