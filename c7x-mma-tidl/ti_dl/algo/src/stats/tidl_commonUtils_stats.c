/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
@file    tidl_commonUtils_stats.c
@brief   This file contains TILD common stats utilities
@version 0.1 (May 2025) : Initial version [ADK]
----------------------------------------------------------------------------
*/


#include "tidl_commonUtils_stats.h"
#include "tidl_device_utils.h"
#include "tidl_device_utils_nc.h"
#include <stdint.h>
#include <limits>
#include <math.h>
#include <float.h>
#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
#endif
#include "tidl_activation_optimizations.h"

#define TIDL_HISTOGRAM_ACTIVATION_RANGEFACTOR (4.0)

#ifdef HOST_EMULATION

int32_t TIDL_getTensorSize(const int32_t* dimValues, const int32_t* pitch)
{
  int32_t tensorSize = 1 + (dimValues[TIDL_DIM_BATCH]-1)*pitch[TIDL_ROI_PITCH] + (dimValues[TIDL_DIM_DIM1]-1)*pitch[TIDL_DIM_DIM1] + (dimValues[TIDL_DIM_DIM2]-1)*pitch[TIDL_DIM_DIM2] \
                        + (dimValues[TIDL_DIM_NUMCH]-1)*pitch[TIDL_DIM_NUMCH] + (dimValues[TIDL_DIM_HEIGHT]-1)*pitch[TIDL_DIM_HEIGHT] + (dimValues[TIDL_DIM_WIDTH]-1);

  return tensorSize;
}

/**
 * @brief To find min and max in the tensor
 *
 * @param ptr : pointer to tensor data
 * @param dataPrms : parametes of the data buffer
 * @param min : to store the min value
 * @param max : to store the max value
 */
template <class Tsrc, class TminMax>
void TIDL_TensorMinMaxStats(const Tsrc * ptr, const sTIDL_DataParams_t * dataPrms, TminMax *min, TminMax * max, sTIDL_Layer_t *layer, sTIDL_DataParams_t *inDataPrms)
{
  int32_t padOffset = dataPrms->padW + (dataPrms->padH*dataPrms->pitch[TIDL_LINE_PITCH]);
  int32_t i0, i1, i2, i3, i4, i5;
  TminMax val;
  if(layer != NULL && layer->layerType == TIDL_TopKLayer) 
  {
    int32_t dimValues[TIDL_DIM_MAX] = {1,1,1,1,1,1};
    for(int32_t i = 0; i < TIDL_DIM_MAX; i++) {
      dimValues[i] = inDataPrms->dimValues[i];
    }
    dimValues[layer->layerParams.topKParams.axis] = layer->layerParams.topKParams.K;

    for (i0 = 0; i0 < dimValues[TIDL_DIM_BATCH]; i0++)
    {
      for (i1 = 0; i1 < dimValues[TIDL_DIM_DIM1]; i1++)
      {
        for (i2 = 0; i2 < dimValues[TIDL_DIM_DIM2]; i2++)
        {
          for (i3 = 0; i3 < dimValues[TIDL_DIM_NUMCH]; i3++)
          {
            for (i4 = 0; i4 < dimValues[TIDL_DIM_HEIGHT]; i4++)
            {
              for (i5 = 0; i5 < dimValues[TIDL_DIM_WIDTH]; i5++)
              {
                val = (TminMax)ptr[padOffset + (i0*dataPrms->pitch[TIDL_ROI_PITCH]) + (i1*dataPrms->pitch[TIDL_DIM_DIM1]) + (i2*dataPrms->pitch[TIDL_DIM_DIM2]) + (i3*dataPrms->pitch[TIDL_DIM_NUMCH]) + (i4*dataPrms->pitch[TIDL_DIM_HEIGHT]) + i5];
                *min = (val < *min) ? val : *min;
                *max = (val > *max) ? val : *max;
              }
            }
          }
        }
      }
    }
  
  }

  else
  {
    #ifdef BUILD_WITH_CUDA_STATS
    if(TIDL_isLayerSupportedOnGPU())
    {
      // call cuda min max wrapper
      int32_t dataSize = TIDL_getTensorSize(dataPrms->dimValues, dataPrms->pitch);
      TIDL_cudaMinMax<Tsrc, TminMax>(ptr + padOffset, min, max, dataSize);
    }
    else
    #endif
    {
      for (i0 = 0; i0 < dataPrms->dimValues[TIDL_DIM_BATCH]; i0++)
      {
        for (i1 = 0; i1 < dataPrms->dimValues[TIDL_DIM_DIM1]; i1++)
        {
          for (i2 = 0; i2 < dataPrms->dimValues[TIDL_DIM_DIM2]; i2++)
          {
            for (i3 = 0; i3 < dataPrms->dimValues[TIDL_DIM_NUMCH]; i3++)
            {
              for (i4 = 0; i4 < dataPrms->dimValues[TIDL_DIM_HEIGHT]; i4++)
              {
                for (i5 = 0; i5 < dataPrms->dimValues[TIDL_DIM_WIDTH]; i5++)
                {
                  val = (TminMax)ptr[padOffset + (i0*dataPrms->pitch[TIDL_ROI_PITCH]) + (i1*dataPrms->pitch[TIDL_DIM_DIM1]) + (i2*dataPrms->pitch[TIDL_DIM_DIM2]) + (i3*dataPrms->pitch[TIDL_DIM_NUMCH]) + (i4*dataPrms->pitch[TIDL_DIM_HEIGHT]) + i5];
                  *min = (val < *min) ? val : *min;
                  *max = (val > *max) ? val : *max;
                }
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @brief This function is to find Current Offset For PerChannelMean
 *
 * @param net : tidl network structure
 * @param currLayerNum : layer no of the current layer
 * @param totalMemReq : to store total memory requiremet
 * @return int32_t : returns the current ch offset
 */
static int32_t TIDL_findCurrentOffsetForPerChannelMeanStats(const sTIDL_Network_t * net, int32_t currLayerNum, int32_t * totalMemReq)
{
  int32_t layerIdx = 0;
  int32_t totNumOutChannels = 0;
  int32_t currNumOutChannels = 0;
  for (layerIdx = 0; layerIdx < net->numLayers; layerIdx++)
  {

    if ( layerIdx == (currLayerNum) )
    {
      currNumOutChannels = totNumOutChannels;
    }
    
    if (net->TIDLLayers[layerIdx].layerType == TIDL_InnerProductLayer)
    {
      if(net->TIDLLayers[layerIdx].layerParams.innerProductParams.constIdx == 1)
      {
        /*Bias is along the column axis*/
        totNumOutChannels += net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_WIDTH] * net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
      }
      else
      {
        totNumOutChannels += net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
      }
    }
    else if ( (net->TIDLLayers[layerIdx].layerType != TIDL_DataLayer) && ((net->TIDLLayers[layerIdx].layerType != TIDL_ConstDataLayer)))
    {
      //:TODO: This can eventually be done only for the layers where bias is applicable
      totNumOutChannels += net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH];
    }
    else
    {
      /*do nothing*/
    }
  }

  *totalMemReq = (uint32_t)totNumOutChannels * sizeof(float32_tidl);
  return ((uint32_t)currNumOutChannels * sizeof(float32_tidl));
}

/**
 * @brief  To find histogram min and max values of the tensor
 *
 * @param ptr  : pointer to the tensor data
 * @param dataPrms : parametes of the tensor data
 * @param numBins : no of bins used for histogram
 * @param percentileActRangeShrink : active range percentage
 * @param histogramPtr : Pointer to the histogram buffer
 * @param min : to store the min value
 * @param max : to store the max value
 */
template <class Tsrc, class TminMax>
int32_t TIDL_TensorMinMaxHistStats(const Tsrc * ptr,
                                      const sTIDL_DataParams_t * dataPrms,
                                      int32_t numBins,
                                      float32_tidl percentileActRangeShrink,
                                      int32_t * histogramPtr,
                                      TminMax* min,
                                      TminMax* max)
{
  int32_t status = TIDL_SUCCESS;
  int32_t padOffset = dataPrms->padW + (dataPrms->padH*dataPrms->pitch[TIDL_LINE_PITCH]);
  int32_t i0, i1, i2, i3, i4, i;
  TminMax val;
  float32_tidl valFloat, valNorm;
  int32_t numVals;
  int32_t binIdx, minBinIdx = 0, maxBinIdx = (numBins - 1);
  float32_tidl minValue = (float32_tidl)(*min - dataPrms->tensorZeroPoint) / dataPrms->tensorScale;
  float32_tidl maxValue = (float32_tidl)(*max - dataPrms->tensorZeroPoint) / dataPrms->tensorScale;
  float32_tidl minFloatOut;
  float32_tidl maxFloatOut;

  if((!isinf(maxValue ) && !isinf(minValue)) && ((maxValue - minValue) != 0.0)) /*not all values in tensor constant */
  {
    #ifdef BUILD_WITH_CUDA_STATS
    if(TIDL_isLayerSupportedOnGPU())
    {
      // call cuda histogram wrapper
      TIDL_cudaGetHistogram<Tsrc, TminMax>(ptr, histogramPtr, padOffset, minValue, maxValue, numBins, dataPrms->tensorZeroPoint, dataPrms->tensorScale,
                                            dataPrms->dimValues[TIDL_DIM_BATCH], dataPrms->dimValues[TIDL_DIM_NUMCH], dataPrms->dimValues[TIDL_DIM_HEIGHT], dataPrms->dimValues[TIDL_DIM_WIDTH],
                                            dataPrms->pitch[TIDL_ROI_PITCH], dataPrms->pitch[TIDL_CHANNEL_PITCH], dataPrms->pitch[TIDL_LINE_PITCH]);
    }
    else
    #endif
    {
      for (i0 = 0; i0 < dataPrms->dimValues[TIDL_DIM_BATCH]; i0++)
      {
        for (i1 = 0; i1 < dataPrms->dimValues[TIDL_DIM_NUMCH]; i1++)
        {
          for (i2 = 0; i2 < dataPrms->dimValues[TIDL_DIM_HEIGHT]; i2++)
          {
            for (i3 = 0; i3 < dataPrms->dimValues[TIDL_DIM_WIDTH]; i3++)
            {
              val = (TminMax)ptr[padOffset + (i0*dataPrms->pitch[TIDL_ROI_PITCH]) + (i1*dataPrms->pitch[TIDL_CHANNEL_PITCH]) + (i2*dataPrms->pitch[TIDL_LINE_PITCH]) + i3];
              valFloat = (float32_tidl)(val - dataPrms->tensorZeroPoint) / dataPrms->tensorScale;
              valNorm = (valFloat - minValue)/(maxValue - minValue) * ((float32_tidl)numBins-1.0f);

              binIdx = (valNorm + 0.5);/* Round to nearest integer */

              if ( binIdx > (numBins-1) )
              {
                binIdx = (numBins-1);
              }
              histogramPtr[binIdx]++;
            }
          }
        }
      }
    }

    numVals = 0;
    for (i = 0; i < numBins; i++)
    {
      numVals += histogramPtr[i];
    }

    int32_t pct_freq = ((percentileActRangeShrink / 100.0) * (float32_tidl)numVals);
    int32_t count = 0;
    if (*min < 0)  /* minimum to be shrinked only for signed */
    {
      for(i4 = 0; i4 < numBins; i4++)
      {
        count += histogramPtr[i4];
        if(count >= pct_freq)
        {
          minBinIdx = i4;
          break;
        }
      }
    }
    else
    {
      minBinIdx = 0;
    }
    count = 0;
    for(i4 = numBins - 1; i4 >= 0; i4--)
    {
      count += histogramPtr[i4];
      if(count >= pct_freq)
      {
        maxBinIdx = i4;
        break;
      }
    }
    minFloatOut = minValue + (((float32_tidl)minBinIdx / ((float32_tidl)numBins-1.0f)) * (maxValue - minValue));
    maxFloatOut = minValue + (((float32_tidl)maxBinIdx / ((float32_tidl)numBins-1.0f)) * (maxValue - minValue));
    maxFloatOut = (maxFloatOut > maxValue) ? maxValue : maxFloatOut; /*to ensure back calculated bin value not greater than original max */
    minFloatOut = (minFloatOut < minValue) ? minValue : minFloatOut;
    *min = (TminMax)((minFloatOut * dataPrms->tensorScale) + (float32_tidl)dataPrms->tensorZeroPoint);
    *max = (TminMax)((maxFloatOut * dataPrms->tensorScale) + (float32_tidl)dataPrms->tensorZeroPoint);
  }

  return status;
}



bool TIDL_isQuantizationPassThroughLayer(sTIDL_Layer_t *layer) {
  if(layer->layerType == TIDL_CropLayer ||
      layer->layerType == TIDL_ReshapeLayer ||
      layer->layerType == TIDL_TransposeLayer ||
      layer->layerType == TIDL_DataConvertLayer ||
      layer->layerType == TIDL_DetectionOutputLayer ||
      // layer->layerType == TIDL_GridSampleLayer || /*Passthrough logic is only checking idx '0' input which is gridsample's input, so we can use it as a passthrough layer*/
      layer->layerType == TIDL_BatchReshapeLayer ||
      layer->layerType == TIDL_GatherLayer ||
      layer->layerType == TIDL_ReduceLayer ||
      layer->layerType == TIDL_SliceLayer ||
      layer->layerType == TIDL_TopKLayer
  ) {
    return true;
  }
  return false;
}

float32_tidl TIDL_clamp(float32_tidl value, float32_tidl minVal, float32_tidl maxVal)
{
  if(value < minVal) return minVal;
  if(value > maxVal) return maxVal;
  return value;
}

/**
 * @brief Function which finds the first producer layer
 *
 * @param pLayer : Pointer to the current layer
 * @return int32_t : returns updated required or not
 */

bool TIDL_isProducerLayer(sTIDL_Layer_t *pLayer)
{
  bool isComputeLayer = false;
  if(pLayer->layerType == TIDL_ConvolutionLayer ||
     pLayer->layerType == TIDL_Deconv2DLayer ||
     pLayer->layerType == TIDL_EltWiseLayer  ||
     pLayer->layerType == TIDL_InnerProductLayer ||
     pLayer->layerType == TIDL_DeformableConvLayer
     )
  {
    isComputeLayer = true;
  }
  else if(pLayer->layerType == TIDL_DataConvertLayer && pLayer->layerParams.dataConvertParams.type == TIDL_DC_TYPE_INPUT)
  {
    isComputeLayer = true;
  }
  return isComputeLayer;  
}

int32_t TIDL_getNumOutputs(sTIDL_Network_t *net, int32_t layerIndex, int32_t dataId)
{
  int32_t numOutputs = 0;
  for (int32_t i1 = 0; i1 < layerIndex; i1++)
  {
    for (int32_t i2 = 0; i2 < net->TIDLLayers[i1].numInBufs; i2++)
    {
      if (net->TIDLLayers[i1].inData[i2] == dataId)
      {
        numOutputs++;
      }
    }
  }
  return numOutputs;
}

int32_t TIDL_getOutLayer(sTIDL_Network_t *net, int32_t layerIndex, int32_t dataId)
{
  for (int32_t i1 = 0; i1 < layerIndex; i1++)
  {
    for (int32_t i2 = 0; i2 < net->TIDLLayers[i1].numInBufs; i2++)
    {
      if (net->TIDLLayers[i1].inData[i2] == dataId)
      {
        return (i1);
      }
    }
  }
  return -1;
}

/* When the consumer of the layer is a range optimized activation such as Sigmoid, Tanh, the 
  per channel mean of layer in float pass should be computed based on the activation range constraints 
  instead of using the output ptr statistics. 

  Implementation: For every range optimized activation layer, we will find the corresponding input layer 
  and compute the per channel mean based on the activation range constraints of the consumer layer. 
*/
void TIDL_isLayerConsumerRangeOptimizedActivation(sTIDL_Network_t *net, int32_t layerIdx) 
{
  if(!TIDL_isProducerLayer(&(net->TIDLLayers[layerIdx])) || TIDL_getNumOutputs(net, net->numLayers, net->TIDLLayers[layerIdx].outData.dataId) > 1) {
    return;
  }

  int32_t consumerLayerIdx = TIDL_getOutLayer(net, net->numLayers, net->TIDLLayers[layerIdx].outData.dataId);
  bool consumerIsRangeOptimizedActivation = false;
  while((consumerLayerIdx != -1) && (consumerLayerIdx < net->numLayers) && !TIDL_isProducerLayer(&net->TIDLLayers[consumerLayerIdx])) 
  {
    if(TIDL_isLayerRangeOptimizedActivation(&net->TIDLLayers[consumerLayerIdx])) {
      consumerIsRangeOptimizedActivation = true;
      break;
    }
    else {
      if(TIDL_getNumOutputs(net, net->numLayers, net->TIDLLayers[consumerLayerIdx].outData.dataId) > 1) {
        break;
      }
      consumerLayerIdx = TIDL_getOutLayer(net, net->numLayers, net->TIDLLayers[consumerLayerIdx].outData.dataId);
    }
  }

  if(!consumerIsRangeOptimizedActivation) {
    return;
  }

  float32_tidl minVal, maxVal;
  TIDL_getActivationOptimizedInputRange(&net->TIDLLayers[consumerLayerIdx], &minVal, &maxVal);

  sTIDL_Layer_t *producerLayer = &net->TIDLLayers[layerIdx];
  if(producerLayer->numOutBufs == 1) {
    if(producerLayer->clipParams.isClipEnabled == 1) {
      producerLayer->clipParams.clipMin = TIDL_clamp(producerLayer->clipParams.clipMin, minVal, maxVal);
      producerLayer->clipParams.clipMax = TIDL_clamp(producerLayer->clipParams.clipMax, minVal, maxVal);
    } else if(producerLayer->actParams.actType == TIDL_RelU) {
      producerLayer->outData.minTensorValue = 0.0f;
      producerLayer->outData.maxTensorValue = TIDL_clamp(producerLayer->outData.maxTensorValue, 0.0f, maxVal);
      producerLayer->clipParams.clipMin = producerLayer->outData.minTensorValue;
      producerLayer->clipParams.clipMax = producerLayer->outData.maxTensorValue;
    } else {
      producerLayer->outData.minTensorValue = TIDL_clamp(producerLayer->outData.minTensorValue, minVal, maxVal);
      producerLayer->outData.maxTensorValue = TIDL_clamp(producerLayer->outData.maxTensorValue, minVal, maxVal);
      producerLayer->clipParams.clipMin = producerLayer->outData.minTensorValue;
      producerLayer->clipParams.clipMax = producerLayer->outData.maxTensorValue;
    }
    producerLayer->actParams.actType += TIDL_TOTAL_NONLINEAR_ACT_OPS + 1; 
  }
}
/**
 * @brief To find tensor perChannel mean
 *
 * @param net : tidl network structure
 * @param currLayerNum : current layer number
 * @param ptr : pointer to the tensor data
 * @param dataPrms : parameters of the data buffer
 * @param scratchBuf : pointer to the scratch memory
 * @param scratchBufSize : size of the scratch buffer
 * @param currIterationCount : count for the current iteration
 * @return int32_t : returns tensor perchannelmean
 */
template <class Tsrc>
int32_t  TIDL_TensorPerChannelMeanStats(sTIDL_Network_t * net,
                                                      int32_t currLayerNum,
                                                      const Tsrc * ptr,
                                                      const sTIDL_DataParams_t * dataPrms,
                                                      void * scratchBuf,
                                                      int32_t scratchBufSize,
                                                      int32_t currIterationCount
                                                      )
{
  int32_t status = TIDL_SUCCESS;
  int32_t padOffset = dataPrms->padW + (dataPrms->padH*dataPrms->pitch[TIDL_LINE_PITCH]);
  int32_t i0, i1, i2, i3, i4, i5;
  float32_tidl * meanPtr;
  int32_t totalMemReqInBytes;
  int8_t * traceDumpName = (int8_t *)(scratchBuf);
  int32_t currOffsetInBytes = 0;
  int32_t currOffsetInfloats = 0;
  float32_tidl currChannelMean = 0.0;
  float32_tidl runningChannelMean = 0.0;
  float32_tidl sum;
  float32_tidl updateFactor;
  Tsrc val;

  if(tidl_getTraceBaseName() != NULL)
  {
    sprintf((char *)traceDumpName, "%s_LayerPerChannelMean.bin", (char *)tidl_getTraceBaseName());
  }
  else
  {
    (void)strcpy((char *)traceDumpName, (char *)"");
  }

  /* Find the total memory required and current offset */
  currOffsetInBytes = TIDL_findCurrentOffsetForPerChannelMeanStats(net,
                                                              currLayerNum,
                                                              &totalMemReqInBytes);

  currOffsetInfloats = ( (uint32_t)currOffsetInBytes / sizeof(float32_tidl) );

  if (totalMemReqInBytes < (scratchBufSize -(int32_t)TRACE_STRING_SIZE) )
  {
    meanPtr = (float32_tidl*)((int8_t *)scratchBuf + TRACE_STRING_SIZE);
  }
  else
  {
    tidl_printf(0,"TIDL_TensorPerChannelMeanStats : Not enough memory to allocate for per channel mean \n");
    status = TIDL_ERR_FAILURE;
  }

  if ( status == TIDL_SUCCESS )
  {
    if ( (currIterationCount == 0) && (currOffsetInBytes == 0 ) )
    {
      (void)memset(meanPtr, 0, totalMemReqInBytes);
    }
    else
    {
      (void)tidl_getReadBinFromFile()((const char *)traceDumpName, meanPtr, totalMemReqInBytes, 1 );
    }

    updateFactor = 1.0/((float32_tidl)currIterationCount + 1.0);
      //:TODO: Add minimum value so that when num images are we don't loose information from end images
    // printf("Upper bound\n");
    #ifdef BUILD_WITH_CUDA_STATS
    if(TIDL_isLayerSupportedOnGPU())
    {
      // call cuda per channel mean wrapper
      TIDL_cudaPerChannelMeans<Tsrc>(&ptr[padOffset], &meanPtr[currOffsetInfloats], net->TIDLLayers[currLayerNum].layerType, net->TIDLLayers[currLayerNum].layerParams.innerProductParams.constIdx,
                  dataPrms->dimValues[TIDL_DIM_BATCH], dataPrms->dimValues[TIDL_DIM_DIM1], dataPrms->dimValues[TIDL_DIM_DIM2], dataPrms->dimValues[TIDL_DIM_NUMCH], dataPrms->dimValues[TIDL_DIM_HEIGHT], dataPrms->dimValues[TIDL_DIM_WIDTH],
                  dataPrms->pitch[TIDL_ROI_PITCH], dataPrms->pitch[TIDL_DIM1_PITCH], dataPrms->pitch[TIDL_DIM2_PITCH], dataPrms->pitch[TIDL_CHANNEL_PITCH], dataPrms->pitch[TIDL_LINE_PITCH],
                  updateFactor, dataPrms->tensorZeroPoint, dataPrms->tensorScale);
    }
    else
    #endif
    {
      if((net->TIDLLayers[currLayerNum].layerType == TIDL_InnerProductLayer) && (net->TIDLLayers[currLayerNum].layerParams.innerProductParams.constIdx == 1))
      {
        for (i3 = 0; i3 < dataPrms->dimValues[TIDL_DIM_NUMCH]; i3++)
        {
          for (i4 = 0; i4 < dataPrms->dimValues[TIDL_DIM_WIDTH]; i4++)
          {
            sum = 0;
            for (i0 = 0; i0 < dataPrms->dimValues[TIDL_DIM_BATCH]; i0++)
            {
              for (i1 = 0; i1 < dataPrms->dimValues[TIDL_DIM_DIM1]; i1++)
              {
                for (i2 = 0; i2 < dataPrms->dimValues[TIDL_DIM_DIM2]; i2++)
                {
                  for (i5 = 0; i5 < dataPrms->dimValues[TIDL_DIM_HEIGHT]; i5++)
                  {
                    /*Need to cross check if it makes sense to have channels averaged*/
                    {
                      val = (Tsrc)ptr[padOffset + (i0*dataPrms->pitch[TIDL_ROI_PITCH]) + (i1*dataPrms->pitch[TIDL_DIM1_PITCH]) + (i2*dataPrms->pitch[TIDL_DIM2_PITCH])
                      + (i3*dataPrms->pitch[TIDL_CHANNEL_PITCH]) + (i5*dataPrms->pitch[TIDL_LINE_PITCH]) + i4];
                      if((net->TIDLLayers[currLayerNum].outData.elementType == TIDL_SinglePrecFloat) && (net->TIDLLayers[currLayerNum].actParams.actType > TIDL_TOTAL_NONLINEAR_ACT_OPS)) 
                      {
                        val = val < net->TIDLLayers[currLayerNum].clipParams.clipMin ? net->TIDLLayers[currLayerNum].clipParams.clipMin : val;
                        val = val > net->TIDLLayers[currLayerNum].clipParams.clipMax ? net->TIDLLayers[currLayerNum].clipParams.clipMax : val;
                       }
                      sum += (float32_tidl)val;
                    }
                  }
                }
              }
            }

            currChannelMean = sum / (((float32_tidl)dataPrms->dimValues[TIDL_DIM_HEIGHT]) * dataPrms->dimValues[TIDL_DIM_DIM2] *
                                                    dataPrms->dimValues[TIDL_DIM_DIM1] * dataPrms->dimValues[TIDL_DIM_BATCH]);
            currChannelMean = (currChannelMean - (float32_tidl)dataPrms->tensorZeroPoint) / dataPrms->tensorScale;
            runningChannelMean = meanPtr[currOffsetInfloats + i3 * dataPrms->dimValues[TIDL_DIM_WIDTH] + i4];
            meanPtr[currOffsetInfloats + i3 * dataPrms->dimValues[TIDL_DIM_WIDTH] + i4] = (runningChannelMean * (1.0 - updateFactor)) +
                                                                                          (currChannelMean * (updateFactor));
          }
        }
      }
      else
      {
        for (i0 = 0; i0 < dataPrms->dimValues[TIDL_DIM_BATCH]; i0++)
        {
          for (i1 = 0; i1 < dataPrms->dimValues[TIDL_DIM_NUMCH]; i1++)
          {
            sum = 0;
            for (i2 = 0; i2 < dataPrms->dimValues[TIDL_DIM_HEIGHT]; i2++)
            {
              for (i3 = 0; i3 < dataPrms->dimValues[TIDL_DIM_WIDTH]; i3++)
              {
                val = (Tsrc)ptr[padOffset + (i0*dataPrms->pitch[TIDL_ROI_PITCH]) + (i1*dataPrms->pitch[TIDL_CHANNEL_PITCH]) + (i2*dataPrms->pitch[TIDL_LINE_PITCH]) + i3];
                if((net->TIDLLayers[currLayerNum].outData.elementType == TIDL_SinglePrecFloat) && (net->TIDLLayers[currLayerNum].actParams.actType > TIDL_TOTAL_NONLINEAR_ACT_OPS)) 
                {
                  val = val < net->TIDLLayers[currLayerNum].clipParams.clipMin ? net->TIDLLayers[currLayerNum].clipParams.clipMin : val;
                  val = val > net->TIDLLayers[currLayerNum].clipParams.clipMax ? net->TIDLLayers[currLayerNum].clipParams.clipMax : val;
                }
                sum += (float32_tidl)val;
              }
            }
            currChannelMean = sum / ((float32_tidl)dataPrms->dimValues[TIDL_DIM_WIDTH] *
                                                      (float32_tidl)dataPrms->dimValues[TIDL_DIM_HEIGHT]);
            currChannelMean = (currChannelMean - (float32_tidl)dataPrms->tensorZeroPoint) / dataPrms->tensorScale;
            runningChannelMean = meanPtr[currOffsetInfloats + i1];
            meanPtr[currOffsetInfloats + i1] = (runningChannelMean * (1.0f - updateFactor)) +
                                                            (currChannelMean * (updateFactor));
          }
        }
      }
    }

    (void)tidl_getWriteBinToFileStats()((const char *)traceDumpName, meanPtr, totalMemReqInBytes, 1);
  }
  return status;
}

/**
 * @brief This function is to update the tesnor range
 *
 * @param intAlgHandle : tidl algorithm handle
 * @param layerIdx : index of the current layer
 * @param outDataIdx : index of the output data buffer
 * @param ptr : pointer to the tensor data
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_UpdateTensorRangeStats(TIDL_Handle intAlgHandle, int32_t layerIdx, const void * ptr)
{
  float32_tidl min = FLT_MAX;
  float32_tidl max = -1.0 * FLT_MAX;
  int32_t temp;
  int32_t status = TIDL_SUCCESS;
  sTIDL_Network_t * net = intAlgHandle->createParams->net;
  int32_t numBins = (int32_t)TIDL_NUM_ACTIVATION_HISTOGRAM_BINS;
  int32_t * histogramPtr = NULL;
  sTIDL_DataParams_t dataBuffParamOrig = net->TIDLLayers[layerIdx].outData;
  /* inDataPrms is utilized in the case of argmax and topK, so we always consider the first input */
  sTIDL_DataParams_t *inDataPrms = TIDL_getDataParams(net, net->TIDLLayers[layerIdx].inData[0]);
  /* Update the dim and pitch values as expected by NHWC format and layer change it back to original */
  if ( net->TIDLLayers[layerIdx].layerType == TIDL_DataConvertLayer )
  {
    if ( net->TIDLLayers[layerIdx].layerParams.dataConvertParams.layout == (int32_t)TIDL_LT_NHWC )
    {
      if ( net->TIDLLayers[layerIdx].layerParams.dataConvertParams.type == (int32_t)TIDL_DC_TYPE_OUTPUT)
      {
        sTIDL_DataParams_t * dataBuffParam = &net->TIDLLayers[layerIdx].outData;
        dataBuffParam->dimValues[TIDL_DIM_WIDTH]  = dataBuffParamOrig.dimValues[TIDL_DIM_NUMCH];
        dataBuffParam->dimValues[TIDL_DIM_HEIGHT] = dataBuffParamOrig.dimValues[TIDL_DIM_WIDTH];
        dataBuffParam->dimValues[TIDL_DIM_NUMCH]  = dataBuffParamOrig.dimValues[TIDL_DIM_HEIGHT];
        dataBuffParam->dimValues[TIDL_DIM_DIM2]  = dataBuffParamOrig.dimValues[TIDL_DIM_DIM2];
        dataBuffParam->dimValues[TIDL_DIM_DIM1]  = dataBuffParamOrig.dimValues[TIDL_DIM_DIM1];
        dataBuffParam->dimValues[TIDL_DIM_BATCH]  = dataBuffParamOrig.dimValues[TIDL_DIM_BATCH];

        dataBuffParam->pitch[TIDL_LINE_PITCH]    = dataBuffParam->dimValues[TIDL_DIM_WIDTH];
        dataBuffParam->pitch[TIDL_CHANNEL_PITCH] = dataBuffParam->pitch[TIDL_LINE_PITCH] * dataBuffParam->dimValues[TIDL_DIM_HEIGHT];
        dataBuffParam->pitch[TIDL_DIM2_PITCH]     = dataBuffParam->pitch[TIDL_CHANNEL_PITCH] * dataBuffParam->dimValues[TIDL_DIM_NUMCH];
        dataBuffParam->pitch[TIDL_DIM1_PITCH]     = dataBuffParam->pitch[TIDL_DIM2_PITCH] * dataBuffParam->dimValues[TIDL_DIM_DIM2];
        dataBuffParam->pitch[TIDL_ROI_PITCH]     = dataBuffParam->pitch[TIDL_DIM1_PITCH] * dataBuffParam->dimValues[TIDL_DIM_DIM1];
      }
    }
  }


  if (TIDL_getDatElementSize(net->TIDLLayers[layerIdx].outData.elementType) <= 2)
  {
    int32_t minTemp, maxTemp;
    int32_t minActHist;
    int32_t maxActHist;
	minTemp  = (int32_t)((((uint32_t)1) << ((sizeof(int32_t) * 8U) - 1U)) - 1U);
    minActHist = minTemp;
    temp = (int32_t)((((uint32_t)1) << ((sizeof(int32_t) * 8U) - 1U)));
	maxTemp = -1 * temp;
    maxActHist = maxTemp;

    if (intAlgHandle->activationHistPtr != NULL )
    {
      histogramPtr = &intAlgHandle->activationHistPtr[(uint32_t)layerIdx * TIDL_NUM_ACTIVATION_HISTOGRAM_BINS];
      /* activationRangePtr being NULL indicates that we want to update histogram for each frame */
      if ( intAlgHandle->activationRangePtr == NULL )
      {
        (void)memset(histogramPtr, 0 , TIDL_NUM_ACTIVATION_HISTOGRAM_BINS * sizeof(int32_t));
      }
      else
      {
        /* THis indicates that user wants to use global histogram across all the
        frames. So read the activation ranges from the previous iteration and multiply
        it by a factor to account for any variation in stats across iterations. These
        min and max value will be use to find the histogram across all the frames */
        minActHist = (intAlgHandle->activationRangePtr[2* layerIdx] *
                    TIDL_HISTOGRAM_ACTIVATION_RANGEFACTOR *
                    net->TIDLLayers[layerIdx].outData.tensorScale);
        maxActHist = (intAlgHandle->activationRangePtr[(2* layerIdx) + 1] *
                    TIDL_HISTOGRAM_ACTIVATION_RANGEFACTOR *
                    net->TIDLLayers[layerIdx].outData.tensorScale);
      }
    }

    if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedChar)
    {
      TIDL_TensorMinMaxStats((int8_t*)ptr, &net->TIDLLayers[layerIdx].outData, (int32_t *)&minTemp, (int32_t *)&maxTemp, &net->TIDLLayers[layerIdx], inDataPrms);

      if ( histogramPtr != NULL )
      {
        /* For global histogram always pick the range from histogram from previous stats collection
        iteration */
        if ( net->calibrationParams.activationRangeMethod == TIDL_ActivationRangeMethodGlobalHistogram )
        {
          minTemp = minActHist;
          maxTemp = maxActHist;
        }
        status = TIDL_TensorMinMaxHistStats((int8_t *)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          numBins,
                                          net->calibrationParams.percentileActRangeShrink,
                                          histogramPtr,
                                          (int32_t*)&minTemp, (int32_t*)&maxTemp);

      }
    }
    else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_UnsignedChar)
    {
      TIDL_TensorMinMaxStats((uint8_t*)ptr, &net->TIDLLayers[layerIdx].outData, (int32_t *)&minTemp, (int32_t *)&maxTemp, &net->TIDLLayers[layerIdx], inDataPrms);

      if ( histogramPtr != NULL )
      {
        /* For global histogram always pick the range from histogram from previous stats collection
        iteration */
        if ( net->calibrationParams.activationRangeMethod == TIDL_ActivationRangeMethodGlobalHistogram )
        {
          minTemp = minActHist;
          maxTemp = maxActHist;
        }
        status = TIDL_TensorMinMaxHistStats((uint8_t *)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          numBins,
                                          net->calibrationParams.percentileActRangeShrink,
                                          histogramPtr,
                                          (int32_t*)&minTemp, (int32_t*)&maxTemp);

      }
    }
    else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedShort)
    {
      TIDL_TensorMinMaxStats((int16_t*)ptr, &net->TIDLLayers[layerIdx].outData, (int32_t *)&minTemp, (int32_t *)&maxTemp, &net->TIDLLayers[layerIdx], inDataPrms);

      if ( histogramPtr != NULL )
      {
        /* For global histogram always pick the range from histogram from previous stats collection
        iteration */
        if ( net->calibrationParams.activationRangeMethod == TIDL_ActivationRangeMethodGlobalHistogram )
        {
          minTemp = minActHist;
          maxTemp = maxActHist;
        }
        status = TIDL_TensorMinMaxHistStats((int16_t *)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          numBins,
                                          net->calibrationParams.percentileActRangeShrink,
                                          histogramPtr,
                                          (int32_t*)&minTemp, (int32_t*)&maxTemp);

      }
    }
    else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_UnsignedShort)
    {
      TIDL_TensorMinMaxStats((uint16_t*)ptr, &net->TIDLLayers[layerIdx].outData, (int32_t *)&minTemp, (int32_t *)&maxTemp, &net->TIDLLayers[layerIdx], inDataPrms);

      if ( histogramPtr != NULL )
      {
        /* For global histogram always pick the range from histogram from previous stats collection
        iteration */
        if ( net->calibrationParams.activationRangeMethod == TIDL_ActivationRangeMethodGlobalHistogram )
        {
          minTemp = minActHist;
          maxTemp = maxActHist;
        }
        status = TIDL_TensorMinMaxHistStats((uint16_t *)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          numBins,
                                          net->calibrationParams.percentileActRangeShrink,
                                          histogramPtr,
                                          (int32_t*)&minTemp, (int32_t*)&maxTemp);

      }
    }
    else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_Bool)
    {
      minTemp = 0;
      maxTemp = 1;
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
    min = ( float32_tidl) minTemp;
    max = ( float32_tidl) maxTemp;
  }
  else
  {
    TIDL_TensorMinMaxStats((float32_tidl *)ptr, &net->TIDLLayers[layerIdx].outData, (float32_tidl *)&min, (float32_tidl *)&max, &net->TIDLLayers[layerIdx], inDataPrms);
    net->TIDLLayers[layerIdx].outData.tensorScale = 1.0;

#if 0
    if ( ( net->calibrationOption & TIDL_CalibOptionActivationRange ) == TIDL_CalibOptionActivationRange )
    {
      if ( net->calibrationParams.activationRangeMethod == TIDL_ActivationRangeMethodHistogram )
      {
        status = TIDL_TensorMinMaxHistStats((float32_tidl *)ptr,
                                           &net->TIDLLayers[layerIdx].outData,
                                           numBins,
                                           net->calibrationParams.percentileActRangeShrink,
                                           intAlgHandle->refScratchBuf,
                                           intAlgHandle->refScratchBufSize,
                                           (float32_tidl*)&min, (float32_tidl*)&max);
      }
    }
#endif
  }

  if(status == TIDL_SUCCESS)
  {
    if(net->TIDLLayers[layerIdx].layerType == TIDL_DataConvertLayer && net->TIDLLayers[layerIdx].layerParams.dataConvertParams.type == (int32_t)TIDL_DC_TYPE_INPUT)
    {
      float32_tidl curMin = (1.0f * ((float32_tidl)min - (float32_tidl)net->TIDLLayers[layerIdx].outData.tensorZeroPoint)) / net->TIDLLayers[layerIdx].outData.tensorScale;
      float32_tidl curMax = (1.0f * ((float32_tidl)max - (float32_tidl)net->TIDLLayers[layerIdx].outData.tensorZeroPoint)) / net->TIDLLayers[layerIdx].outData.tensorScale;
      if ((intAlgHandle->procCallCounter == 0U) && (intAlgHandle->createParams->quantRangeUpdateFactor != 0.0 ) )
      {
        net->TIDLLayers[layerIdx].outData.minTensorValue = curMin;
        net->TIDLLayers[layerIdx].outData.maxTensorValue = curMax;
      }
      else {
        net->TIDLLayers[layerIdx].outData.minTensorValue =  (net->TIDLLayers[layerIdx].outData.minTensorValue < curMin) ?  net->TIDLLayers[layerIdx].outData.minTensorValue : curMin;
        net->TIDLLayers[layerIdx].outData.maxTensorValue = (net->TIDLLayers[layerIdx].outData.maxTensorValue > curMax) ? net->TIDLLayers[layerIdx].outData.maxTensorValue : curMax;
      }
    }
    else if (net->TIDLLayers[layerIdx].outData.tensorScale != 0.0f)
    {
      float32_tidl curMin = (1.0f * ((float32_tidl)min - (float32_tidl)net->TIDLLayers[layerIdx].outData.tensorZeroPoint)) / net->TIDLLayers[layerIdx].outData.tensorScale;
      float32_tidl curMax = (1.0f * ((float32_tidl)max - (float32_tidl)net->TIDLLayers[layerIdx].outData.tensorZeroPoint)) / net->TIDLLayers[layerIdx].outData.tensorScale;
      if(intAlgHandle->createParams->quantRangeExpansionFactor != 1.0f)
      {
        curMin = curMin * intAlgHandle->createParams->quantRangeExpansionFactor;
        curMax = curMax * intAlgHandle->createParams->quantRangeExpansionFactor;
      }

      if ((intAlgHandle->procCallCounter == 0U) && (intAlgHandle->createParams->quantRangeUpdateFactor != 0.0 ) )
      {
        net->TIDLLayers[layerIdx].outData.minTensorValue = curMin;
        net->TIDLLayers[layerIdx].outData.maxTensorValue = curMax;
      }
      else if (((uint32_t)net->calibrationOption & (uint32_t)TIDL_CalibNoOutlier) !=  0U)
      {
        net->TIDLLayers[layerIdx].outData.minTensorValue =  (net->TIDLLayers[layerIdx].outData.minTensorValue < curMin) ?  net->TIDLLayers[layerIdx].outData.minTensorValue : curMin;
        net->TIDLLayers[layerIdx].outData.maxTensorValue = (net->TIDLLayers[layerIdx].outData.maxTensorValue > curMax) ? net->TIDLLayers[layerIdx].outData.maxTensorValue : curMax;
      }
      else
      {
        float32_tidl beta = intAlgHandle->createParams->quantRangeUpdateFactor;
        float32_tidl alpha = 1.0f - beta;

        if(intAlgHandle->createParams->quantRangeUpdateFactor == -1.0f)
        {
          beta = 1.0f / ((float32_tidl)intAlgHandle->procCallCounter + 1.0f);
          alpha = 1.0f - beta;
        }

        net->TIDLLayers[layerIdx].outData.minTensorValue = (net->TIDLLayers[layerIdx].outData.minTensorValue *alpha) + (curMin*beta);
        net->TIDLLayers[layerIdx].outData.maxTensorValue = (net->TIDLLayers[layerIdx].outData.maxTensorValue *alpha) + (curMax*beta);
      }

      /*In high precision mode, expand range by 2.5% in appropriate direction if saturation is detected for any calibration frame*/      
      if((TIDL_isKernelHighPrecision(net->TIDLLayers[layerIdx].layerKernelType) != (int32_t)FALSE) &&
         (net->TIDLLayers[layerIdx].outData.elementType < TIDL_UnsignedWord)) 
      {
        int32_t satMin = 0, satMax = 0;
        TIDL_getSaturationLimits(net->TIDLLayers[layerIdx].outData.elementType, &satMin, &satMax);
        if(net->TIDLLayers[layerIdx].layerType != TIDL_DataConvertLayer) {
          if(max == (float32_tidl)satMax)
          {
            if(net->TIDLLayers[layerIdx].outData.maxTensorValue < 0) {
              net->TIDLLayers[layerIdx].outData.maxTensorValue /= 1.025;
            }
            else {
              net->TIDLLayers[layerIdx].outData.maxTensorValue *= 1.025;
            }
          }
          else if(min == (float32_tidl)satMin)
          {
            if(net->TIDLLayers[layerIdx].outData.minTensorValue > 0) {
              net->TIDLLayers[layerIdx].outData.minTensorValue /= 1.025;
            }
            else {
              net->TIDLLayers[layerIdx].outData.minTensorValue *= 1.025;
            }
          }
        }
		    else
		    {
		    	/*do nothing*/
		    }
      }
    }


    if (net->TIDLLayers[layerIdx].clipParams.isClipEnabled == 1)
    {
      net->TIDLLayers[layerIdx].outData.minTensorValue = net->TIDLLayers[layerIdx].clipParams.clipMin;
      net->TIDLLayers[layerIdx].outData.maxTensorValue = net->TIDLLayers[layerIdx].clipParams.clipMax;
    }

    if (net->TIDLLayers[layerIdx].actParams.actType == TIDL_RelU6)
    {
      if ( net->TIDLLayers[layerIdx].outData.maxTensorValue > 6.0f)
      {
        net->TIDLLayers[layerIdx].outData.maxTensorValue = 6.0f;
      }
    }

    /* For ArgOp layer max is decided based on number of input channels*/
    if ( net->TIDLLayers[layerIdx].layerType == TIDL_ArgOpLayer )
    {
      net->TIDLLayers[layerIdx].outData.minTensorValue = 0.0;
      net->TIDLLayers[layerIdx].outData.maxTensorValue = (inDataPrms->dimValues[TIDL_DIM_NUMCH] -1);
    }

  }

  /* Restore the original buffer parameters */
  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_WIDTH]  = dataBuffParamOrig.dimValues[TIDL_DIM_WIDTH];
  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_HEIGHT] = dataBuffParamOrig.dimValues[TIDL_DIM_HEIGHT];
  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH]  = dataBuffParamOrig.dimValues[TIDL_DIM_NUMCH];
  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_DIM2]  = dataBuffParamOrig.dimValues[TIDL_DIM_DIM2];
  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_DIM1]  = dataBuffParamOrig.dimValues[TIDL_DIM_DIM1];
  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_BATCH]  = dataBuffParamOrig.dimValues[TIDL_DIM_BATCH];

  net->TIDLLayers[layerIdx].outData.pitch[TIDL_LINE_PITCH]    = dataBuffParamOrig.pitch[TIDL_LINE_PITCH];
  net->TIDLLayers[layerIdx].outData.pitch[TIDL_CHANNEL_PITCH] = dataBuffParamOrig.pitch[TIDL_CHANNEL_PITCH];
  net->TIDLLayers[layerIdx].outData.pitch[TIDL_ROI_PITCH]     = dataBuffParamOrig.pitch[TIDL_ROI_PITCH];
  net->TIDLLayers[layerIdx].outData.pitch[TIDL_DIM1_PITCH]     = dataBuffParamOrig.pitch[TIDL_DIM1_PITCH];
  net->TIDLLayers[layerIdx].outData.pitch[TIDL_DIM2_PITCH]     = dataBuffParamOrig.pitch[TIDL_DIM2_PITCH];
  return status;
}

/**
 * @brief This function is to Update Tensor PerChannelMean
 *
 * @param intAlgHandle : tidl algorithm handle
 * @param layerIdx : index of the current layer
 * @param outDataIdx : index of the output data buffer
 * @param ptr : pointer to the tensor data
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_UpdateTensorPerChannelMeanStats(TIDL_Handle intAlgHandle,
                                        int32_t layerIdx,
                                        int32_t outDataIdx,
                                        void * ptr)
{
  int32_t status = TIDL_SUCCESS;
  sTIDL_Network_t * net = intAlgHandle->createParams->net;

  if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedChar)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                            layerIdx,
                                            (int8_t*)ptr,
                                            &net->TIDLLayers[layerIdx].outData,
                                            intAlgHandle->refScratchBuf,
                                            intAlgHandle->refScratchBufSize,
                                            (int32_t)intAlgHandle->procCallCounter);
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_UnsignedChar)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                            layerIdx,
                                            (uint8_t*)ptr,
                                            &net->TIDLLayers[layerIdx].outData,
                                            intAlgHandle->refScratchBuf,
                                            intAlgHandle->refScratchBufSize,
                                            (int32_t)intAlgHandle->procCallCounter);
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedShort)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                            layerIdx,
                                            (int16_t*)ptr,
                                            &net->TIDLLayers[layerIdx].outData,
                                            intAlgHandle->refScratchBuf,
                                            intAlgHandle->refScratchBufSize,
                                            (int32_t)intAlgHandle->procCallCounter);
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_UnsignedShort)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                            layerIdx,
                                            (uint16_t*)ptr,
                                            &net->TIDLLayers[layerIdx].outData,
                                            intAlgHandle->refScratchBuf,
                                            intAlgHandle->refScratchBufSize,
                                            (int32_t)intAlgHandle->procCallCounter);
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_Bool)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                            layerIdx,
                                            (bool*)ptr,
                                            &net->TIDLLayers[layerIdx].outData,
                                            intAlgHandle->refScratchBuf,
                                            intAlgHandle->refScratchBufSize,
                                            (int32_t)intAlgHandle->procCallCounter);
  }
  else if(net->TIDLLayers[layerIdx].outData.elementType == TIDL_SinglePrecFloat)
  {
    TIDL_isLayerConsumerRangeOptimizedActivation(net, layerIdx);
    status = TIDL_TensorPerChannelMeanStats(net,
                                          layerIdx,
                                          (float32_tidl*)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          intAlgHandle->refScratchBuf,
                                          intAlgHandle->refScratchBufSize,
                                          (int32_t)intAlgHandle->procCallCounter);
    if(net->TIDLLayers[layerIdx].actParams.actType > TIDL_TOTAL_NONLINEAR_ACT_OPS)
    {
      net->TIDLLayers[layerIdx].actParams.actType -= (TIDL_TOTAL_NONLINEAR_ACT_OPS + 1);
    }
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedWord)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                          layerIdx,
                                          (int32_t*)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          intAlgHandle->refScratchBuf,
                                          intAlgHandle->refScratchBufSize,
                                          (int32_t)intAlgHandle->procCallCounter);
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_UnsignedWord)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                          layerIdx,
                                          (uint32_t*)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          intAlgHandle->refScratchBuf,
                                          intAlgHandle->refScratchBufSize,
                                          (int32_t)intAlgHandle->procCallCounter);
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedDoubleWord)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                          layerIdx,
                                          (int64_t*)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          intAlgHandle->refScratchBuf,
                                          intAlgHandle->refScratchBufSize,
                                          (int32_t)intAlgHandle->procCallCounter);
  }
  else if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_UnsignedDoubleWord)
  {
    status = TIDL_TensorPerChannelMeanStats(net,
                                          layerIdx,
                                          (uint64_t*)ptr,
                                          &net->TIDLLayers[layerIdx].outData,
                                          intAlgHandle->refScratchBuf,
                                          intAlgHandle->refScratchBufSize,
                                          (int32_t)intAlgHandle->procCallCounter);
  }
  else
  {
    status = TIDL_ERR_FAILURE;
  }

  return status;
}


/**
 * @brief This function Updates TensorStats for all the layers
 *
 * @param intAlgHandle : Algorithm Instance handle
 * @param TIDLLayer : Pointer to common layer parameters
 * @param outPtrs : output buffer pointers
 * @param i : layer Index
 * @return int32_t : returns the status
 */
int32_t TIDL_updateAllLayersTensorStats(
    TIDL_Handle intAlgHandle,
    sTIDL_Layer_t *TIDLLayer,
    void *outPtrs[],
    int32_t i)
{
  int32_t status = TIDL_SUCCESS;

  // PKS-STATS-1: This condition can be at caller place before calling it
#ifdef HOST_EMULATION
  int32_t j = 0;
  for (j = 0; j < TIDLLayer->numOutBufs; j++)
  {
    status = TIDL_UpdateTensorRangeStats(intAlgHandle, i, outPtrs[j]);
    if (status != TIDL_SUCCESS)
    {
      break;
    }
    if (((uint32_t)intAlgHandle->createParams->net->calibrationOption & (uint32_t)TIDL_CalibOptionBiasCalibration) == (uint32_t)TIDL_CalibOptionBiasCalibration)
    {
      status = TIDL_UpdateTensorPerChannelMeanStats(intAlgHandle,
                                                    i,
                                                    j,
                                                    outPtrs[j]);
      if (status != TIDL_SUCCESS)
      {
        break;
      }
    }
  }
#endif

  return status;
}
#endif
