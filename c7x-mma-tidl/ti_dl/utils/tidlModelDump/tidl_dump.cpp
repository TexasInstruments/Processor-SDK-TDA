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
@file      tidl_dump.cpp
@brief     This file implements a class used to render the TIDL network representation
@brief     in human-readbale format
*/

#include "itidl_ti.h"
#include "tidl_dump.h"
#include "tidl_strings.h"
#include "indent_ostream.h"
#include "gc.h"
#include "gc_helper.h"
#include "common.h"

#include <stdio.h>
#include <stdarg.h>
#include <ostream>
#include <string>

using namespace TIDL_Strings;
#define TIDL_MATMUL_DISABLED 0
#define TIDL_MATMUL_ENABLED 1

// Default options that control the dump behavior.
// Any options passed to the contructor override these defaults.
const TIDL_dump::options_t TIDL_dump::default_options =
{
   { "perfsim", false },         // include dataflow info from perfsim
   { "full_weights", false },    // print all weights even if a lot
};

// Constructor for TIDL_dump object.
TIDL_dump::TIDL_dump(std::ostream& os,
                     const sTIDL_Network_t* net,
	             const sTIDL_IOBufDesc_t* io /*=nullptr*/,
	             const layerNames_t* layerNames /*=nullptr*/,
	             options_t user_options  /*={}*/) :
      os(indent_ostream(os)), pNet(net), pIODesc(io),
      layerNames(layerNames), options(default_options)
{
   // overide default options with any that were passed in
   for (auto it : user_options)
      options[it.first] = it.second;
}

// Main API to dump the network
void TIDL_dump::dumpNet()
{
  if (pNet == nullptr)
     return;

  int32_t i, j;
  os << format("TIDL Network------------------------------------------------------\n");
  os.push();
  os << format("netVersion=0x%x ", pNet->netVersion);
  os << format("deviceName=%d\n", pNet->deviceName);
  os << format("numLayers=%d\n", pNet->numLayers);
  os << format("weightsElementSize=%d ", pNet->weightsElementSize);
  os << format("slopeElementSize=%d  ", pNet->slopeElementSize);
  os << format("biasElementSize=%d ", pNet->biasElementSize);
  os << format("dataElementSize=%d ", pNet->dataElementSize);
  os << format("interElementSize=%d\n", pNet->interElementSize);
  os << format("quantizationStyle=%s",
               quantStyleString(pNet->quantizationStyle).c_str());
  os << format("calibrationOption=%d\n", pNet->calibrationOption);
  if (pNet->calibrationOption != 0)
  {
     os << format("calibrationParams:\n");
     os.push();
     dumpCalibParams(&pNet->calibrationParams);
     os.pop();
  }
  os << format("isQuantStatsAvailable=%d ", pNet->isQuantStatsAvailable);
  os << format("dataFlowInfo=0x%x\n", pNet->dataFlowInfo);
  os.pop();
  for (i = 0; i < pNet->numLayers; i++)
  {
     os << format("-------------------------------------------------------\n");
     dumpLayer(i);
  }
}

void TIDL_dump::dumpLayer(int layerNum)
{
  const sTIDL_Layer_t *layer = &pNet->TIDLLayers[layerNum];
  int layerType = layer->layerType;
  int j;
  const std::string layerTypeName = layerTypeString(layerType);
  const std::string layerName = layerNames ? layerNames->at(layerNum)
                                           : std::string();
  if (layerType < 0 || layerType > TIDL_UnsupportedLayer)
    layerType = TIDL_UnsupportedLayer;
  os << format("Layer %d: %s \"%.*s\"\n",
               layerNum, layerTypeName.c_str(), 128, layerName.c_str());
  os.push();
  os << format("weightsElementSizeInBits=%d\n",
               layer->weightsElementSizeInBits);
  os << format("kernelType=%d\n",
              layer->layerKernelType);
  os << format("multiCoreMode=%s\n", multiCoreString(layer->multiCoreMode).c_str());

  os << format("strideOffsetMethod=%s\n",
               strideOffsetMethodString(layer->strideOffsetMethod).c_str());
  switch(layerType)
  {
     case TIDL_DataLayer:
       dumpDataLayerParams(layer);
       break;
     case TIDL_ConvolutionLayer:
       dumpConvolutionLayerParams(layer);
       break;
     case TIDL_PoolingLayer:
       dumpPoolingLayerParams(layer);
       break;
     case TIDL_ReLULayer:
       break;
     case TIDL_PReLULayer:
       break;
     case TIDL_EltWiseLayer:
       dumpEltWiseLayerParams(layer);
       break;
     case TIDL_InnerProductLayer:
       dumpInnerProductLayerParams(layer);
       break;
     case TIDL_SoftMaxLayer:
       dumpSoftMaxLayerParams(layer);
       break;
     case TIDL_BatchNormLayer:
       dumpBatchNormLayerParams(layer);
       break;
     case TIDL_BiasLayer:
       dumpBiasLayerParams(layer);
       break;
     case TIDL_ScaleLayer:
       break;
     case TIDL_Deconv2DLayer:
      dumpDeconvolutionLayerParams(layer);
       break;
     case TIDL_ConcatLayer:
       dumpConcatLayerParams(layer);
       break;
     case TIDL_SplitLayer:
       break;
     case TIDL_SliceLayer:
       dumpSliceLayerParams(layer);
       break;
     case TIDL_CropLayer:
       dumpCropLayerParams(layer);
       break;
     case TIDL_FlattenLayer:
       break;
     case TIDL_DropOutLayer:
       break;
     case TIDL_ArgOpLayer:
       dumpArgOpLayerParams(layer);
       break;
     case TIDL_DetectionOutputLayer:
       dumpDetectionOutputLayerParams(layer);
       break;
     case TIDL_ShuffleChannelLayer:
       dumpShuffleChannelLayerParams(layer);
       break;
     case TIDL_ResizeLayer:
       dumpResizeLayerParams(layer);
       break;
     case TIDL_RoiPoolingLayer:
       dumpRoiPoolingLayerParams(layer);
       break;
     case TIDL_DepthToSpaceLayer:
       dumpDepthToSpaceLayerParams(layer);
       break;
     case TIDL_OdPostProcessingLayer:
       break;
     case TIDL_OdOutputReformatLayer:
       dumpOdOutputReformatLayerParams(layer);
       break;
     case TIDL_CustomLayer:
       dumpCustomLayerParams(layer);
       break;
     case TIDL_PadLayer:
       dumpPadLayerParams(layer);
       break;
    case TIDL_LayerNormLayer:
       break;
    case TIDL_DataConvertLayer:
       dumpDataConvertParams(layer);
       break;
    case TIDL_TransposeLayer:
       dumpTransposeParams (layer);
       break;
    case TIDL_DeformableConvLayer:
       dumpDeformConvParams (layer);
       break;
    case TIDL_GridSampleLayer:
       dumpGridSampleParams (layer);
    case TIDL_TopKLayer:
       dumpTopKParams (layer);
       break;
    case TIDL_TileLayer:
      dumpTileParams (layer);
      break;
    case TIDL_LogicalOpLayer:
      dumpLogicalOpLayerParams (layer);
    case TIDL_ShapeLayer:
      dumpShapeParams (layer);
      break;
    case TIDL_SizeLayer:
      break;
    case TIDL_RMSNormalizationLayer:
      dumpRMSNormalizationParams (layer);
      break;
    case TIDL_LSTMLayer:
      dumpLSTMParams(layer);
      break;
    case TIDL_GRULayer:
      dumpGRUParams(layer);
      break;
    case TIDL_RNNLayer:
      dumpRNNParams(layer);
      break;
    case TIDL_GatherNDLayer:
      dumpGatherNDParams(layer);
      break;
    case TIDL_GatherElementsLayer:
      dumpGatherElementsParams(layer);
      break;
    case TIDL_NonZeroLayer:
      dumpNonZeroParams(layer);
      break;
    case TIDL_UnsupportedLayer:
       break;
  }

  dumpActParams(&layer->actParams);
  dumpClipParams(&layer->clipParams);
  if (hasOption("perfsim"))
    dumpPerfSimInfo(layerNum);

  if (layer->numOutBufs > 0)
  {
    os << format("Outputs:\n");
    os.push();
    for (j = 0; j < layer->numOutBufs; j++)
      dumpTensorParams(&layer->outData);
    os.pop();
  }

   if (layer->numInBufs > 0)
   {
      os << format("Input IDs: ");
      for (j = 0; j < layer->numInBufs; j++)
      {
	      os << format("%d", layer->inData[j]);
        if (j != layer->numInBufs - 1)
          os << format(",");
      }
      os << format("\n");
   }

  if (layer->numOutBufs > 0)
  {
    os << format("Output ID: ");
    for (j = 0; j < layer->numOutBufs; j++)
    {
      os << format("%d", layer->outData.dataId);
      if (j != layer->numOutBufs - 1)
        os << format(",");
    }
    os << format("\n");
  }

   os.pop();
}

void TIDL_dump::dumpActParams(const sTIDL_ActParams_t *pActParams)
{
   os << format("actParams:\n");
   os.push();
   os << format("actType=%s slopeScale=%f\n",
                activationTypeString(pActParams->actType).c_str(),
                pActParams->slopeScale);
   os.pop();
}

void TIDL_dump::dumpClipParams(const sTIDL_ClipParams_t *pClipParams)
{
   os << format("clipParams:\n");
   os.push();
   os << format("isClipEnabled=%d clipMin=%f clipMax=%f\n",
                pClipParams->isClipEnabled, pClipParams->clipMin, pClipParams->clipMax);
   os.pop();
}

void TIDL_dump::dumpCalibParams(const sTIDL_CalibParams_t *pCalibParams)
{
   os << format("activationRangeMethod=%s\n",
          activationRangeMethodString(pCalibParams->activationRangeMethod).c_str());
   os << format("weightRangeMethod=%s\n",
          weightRangeMethodString(pCalibParams->weightRangeMethod).c_str());
   if (pCalibParams->activationRangeMethod == TIDL_ActivationRangeMethodHistogram)
   {
      os.push();
      os << format("percentileActRangeShrink=%f ",
                   pCalibParams->percentileActRangeShrink);
      os << format("percentileWtRangeShrink=%f\n",
                   pCalibParams->percentileWtRangeShrink);
      os.pop();
   }
   os << format("biasCalibrationFactor=%f ", pCalibParams->biasCalibrationFactor);
   os << format("biasCalibrationIterations=%d\n", pCalibParams->biasCalibrationIterations);
}

void TIDL_dump::dumpTensorParams(const sTIDL_DataParams_t *pData)
{
   int i;
   int nDim = pData->numDim;
   os << format("[%d] ", pData->dataId);
   os << format("numDim=%d ", nDim);
   nDim = TIDL_DIM_MAX;
   os << format("dims=[");
   for (i = 0; i < nDim; ++i)
   {
      os << format("%d", pData->dimValues[i]);
      if (i != nDim-1) os << format(",");
   }
   os << format("]  ");
   os << format("dynDimMask=[");
   for (i = 0; i < nDim; ++i)
   {
      os << format("%d", (pData->dynDimMask >> i) & 1);
      if (i != nDim-1) os << format(",");
   }
   os << format("]  ");

   os << format("elementType=%s\n",
                elementTypeString(pData->elementType).c_str());
   os.push();
   os << format("padH/W=[%d,%d] ", pData->padH, pData->padW);

   os << format("batchPadH/W=[%d,%d] ", pData->batchPadH, pData->batchPadW);
   os << format("numBatchH/W=[%d,%d] ", pData->numBatchH, pData->numBatchW);


   os << format("pitch=[");
   for (i = 0; i < nDim-1; ++i)
   {
      os << format("%d", pData->pitch[i]);
      if (i != nDim-2) os << format(",");
   }
   os << format("]\n");

   os << format("dataQ=%d roundBits=%d\n", pData->dataQ, pData->roundBits);
   os << format("min/maxValue=(%d,%d) min/maxTensorValue=(%f,%f)\n",
            pData->minValue, pData->maxValue,
	    pData->minTensorValue, pData->maxTensorValue);
   os << format("tensorScale=%f\n", pData->tensorScale);
   os << format("tensorZeroPoint=%d\n",pData->tensorZeroPoint);
   os.pop();
}

void TIDL_dump::dumpDataLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_DataLayerParams_t *pData = &pLayer->layerParams.dataLayerParams;
  os << format("numChannels=%d dataQ=%d\n", pData->numChannels, pData->dataQ);
}

// Helper to dump per-channel weightScales and biasScales
void TIDL_dump::dumpAsymQuantArrays(const sTIDL_Layer_t *pLayer,
                                    int32_t numWeightScaleChannels,
                                    int32_t numBiasChannels,
                                    int32_t weightScalesOffset,
                                    int32_t biasOffset)
{
   if (pLayer->layerKernelType != TIDL_HighPrecisionKernel)
      return;
   if (pNet == nullptr)
      return;

   const int8_t *base = (const int8_t *)pNet;

   if (weightScalesOffset != 0 && numWeightScaleChannels > 0)
   {
      const float *p = (const float *)(base + weightScalesOffset);
      os << format("weightScales[%d]:", numWeightScaleChannels);
      for (int i = 0; i < numWeightScaleChannels; ++i)
         os << format("%s%f", (i == 0 ? "" : ","), p[i]);
      os << format("\n");
   }
   if (biasOffset != 0 && numBiasChannels > 0)
   {
      os << format("bias[%d]:", numBiasChannels);
      if (pLayer->weightsElementSizeInBits == 8)
      {
        const int32_t *p = (const int32_t *)(base + biasOffset);
        for (int i = 0; i < numBiasChannels; ++i)
            os << format("%s%d", (i == 0 ? "" : ","), p[i]);
      }
      else if (pLayer->weightsElementSizeInBits == 16)
      {
        const int64_t *p = (const int64_t *)(base + biasOffset);
        for (int i = 0; i < numBiasChannels; ++i)
            os << format("%s%ld", (i == 0 ? "" : ","), (long)p[i]);
      }
      os << format("\n");
   }
}

void TIDL_dump::dumpConvolutionLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_ConvParams_t *pConv = &pLayer->layerParams.convParams;
  os << format("convolutionType=%d ", pConv->convolutionType);
  os << format("numInChannels=%d ",  pConv->numInChannels);
  os << format("numOutChannels=%d ", pConv->numOutChannels);
  os << format("numGroups=%d\n", pConv->numGroups);
  os << format("kernelH/W=[%d,%d] ", pConv->kernelH, pConv->kernelW);
  os << format("strideH/W=[%d,%d] ", pConv->strideH, pConv->strideW);
  os << format("padH/W=[%d,%d] ", pConv->padH, pConv->padW);
  os << format("dilationH/W=[%d,%d]\n", pConv->dilationH, pConv->dilationW);

  if (pLayer->layerKernelType != TIDL_HighPrecisionKernel)
    os << format("weightScale=%f biasScale=%f\n",pConv->weightScale, pConv->biasScale);

  dumpAsymQuantArrays(pLayer, pConv->numOutChannels, pConv->numOutChannels,
                      pConv->weightScales, pConv->bias);
}

void TIDL_dump::dumpDeconvolutionLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_ConvParams_t *pConv = &pLayer->layerParams.convParams;
  os << format("convolutionType=%d ", pConv->convolutionType);
  os << format("numInChannels=%d ",  pConv->numInChannels);
  os << format("numOutChannels=%d ", pConv->numOutChannels);
  os << format("numGroups=%d\n", pConv->numGroups);
  os << format("kernelH/W=[%d,%d] ", pConv->kernelH, pConv->kernelW);
  os << format("strideH/W=[%d,%d] ", pConv->strideH, pConv->strideW);
  os << format("padH/W=[%d,%d] ", pConv->padH, pConv->padW);
  os << format("dilationH/W=[%d,%d]\n", pConv->dilationH, pConv->dilationW);

  if (pLayer->layerKernelType != TIDL_HighPrecisionKernel)
    os << format("weightScale=%f biasScale=%f\n",pConv->weightScale, pConv->biasScale);

  dumpAsymQuantArrays(pLayer, pConv->numOutChannels, pConv->numOutChannels,
                      pConv->weightScales, pConv->bias);
}

void TIDL_dump::dumpPoolingLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_PoolingParams_t *pPool = &pLayer->layerParams.poolParams;
  dumpPoolingLayerParams(pPool);
}

void TIDL_dump::dumpPoolingLayerParams(const sTIDL_PoolingParams_t *pPool)
{
  os << format("poolingtype=%s ", poolingTypeString(pPool->poolingType).c_str());
  os << format("numChannels=%d\n", pPool->numChannels);
  os << format("inDataQ=%d ", pPool->inDataQ);
  os << format("outDataQ=%d ", pPool->outDataQ);
  os << format("useCeil=%d\n", pPool->useCeil);

  os << format("kernelH/W=[%d,%d] ", pPool->kernelH, pPool->kernelW);
  os << format("strideH/W=[%d,%d] ", pPool->strideH, pPool->strideW);
  os << format("padH/W=[%d,%d]\n", pPool->padH, pPool->padW);
}

void TIDL_dump::dumpEltWiseLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_EltWiseParams_t *pEW = &pLayer->layerParams.eltWiseParams;
  os << format("numChannels=%d eltWiseType=%s numInData=%d\n",
         pEW->numChannels,
	 eltwiseOpTypeString(pEW->eltWiseType).c_str(),
	 pEW->numInData);

  if (pEW->numInData)
  {
     os << format("inDataQ:");
     for (int i = 0; i < pEW->numInData; ++i)
     {
	      os << format("%d", pEW->inDataQ[i]);
	      if (i != pEW->numInData-1)
        {
          os << format(",");
        }
     }
     os << format("\n");
  }
  os << format("outDataQ=%d\n", pEW->outDataQ);
  os << format("biasQ=%d ", pEW->biasQ);
  os << format("bias:0x%x\n", pEW->bias);
}

void TIDL_dump::dumpLogicalOpLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_LogicalOpLayerParams_t *pLL = &pLayer->layerParams.logicalOpLayerParams;
  os << format("logicalOpLayerType=%s\n",
	 logicalOpTypeString(pLL->operatorType).c_str()
	 );

}

void TIDL_dump::dumpInnerProductLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_InnerProductParams_t *pIP = &pLayer->layerParams.innerProductParams;
  int32_t isMatmulEnabled = pLayer->numInBufs == 1 ? TIDL_MATMUL_DISABLED : TIDL_MATMUL_ENABLED;
  os << format("activationType=%d numInRows=%d numInCols=%d numOutCols=%d transA=%d numBChannels=%d\n",
         pIP->activationType, pIP->numInRows, pIP->numInCols, pIP->numOutCols, pIP->inputATranspose, pIP->numBChannels);
  if (pLayer->layerKernelType != TIDL_HighPrecisionKernel)
  {
    os << format("weightScale=%f biasScale=%f\n", 
          pIP->weightScale, pIP->biasScale);
    os << format("biasQ=%d inDataQ=%d outDataQ=%d interDataQ=%d\n",
         pIP->biasQ, pIP->inDataQ, pIP->outDataQ, pIP->interDataQ);
    os << format("biasB=%d\n", pIP->biasB);
  }
  os << format("inputBTranspose=%d\n", pIP->inputBTranspose);
  os << format("isMatmulEnabled=%d\n", isMatmulEnabled);
  // Per-channel asym arrays are only valid for constIdx!=0
  // (column/B-const quantization). Row quantization (constIdx==0) does
  // not produce per-output-column scales/biases that match the dump layout.
  if (pIP->constIdx != 0 && pIP->constIdx != -1)
  {
    int32_t ipBCh = pIP->numBChannels > 0 ? (int32_t)pIP->numBChannels : 1;
    int32_t ipChannels = (int32_t)pIP->numOutCols * ipBCh;
    dumpAsymQuantArrays(pLayer, ipChannels, ipChannels,
                        pIP->weightScales, pIP->bias);
  }
}

void TIDL_dump::dumpSoftMaxLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_SoftMaxParams_t *pSoftMax = &pLayer->layerParams.softMaxParams;
  os << format("numChannels=%d ", pSoftMax->numChannels);
  os << format("inDataQ=%d ", pSoftMax->inDataQ);
  os << format("outDataQ=%d\n", pSoftMax->outDataQ);
  os << format("axis=%s\n",
       getDimensionString(pSoftMax->axis).c_str());
  os << format("outTranspose=%d\n", pSoftMax->outTranspose);
}

void TIDL_dump::dumpBatchNormLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_BatchNormParams_t *pBN = &pLayer->layerParams.batchNormParams;
  void * weightPtr = ((int8_t *)(pNet) + pBN->weights);
  void * biasPtr   = ((int8_t *)(pNet) + pBN->bias);
  int numChannels = 0;
  numChannels = pLayer->outData.dimValues[TIDL_DIM_NUMCH];
  os << format("numChannels=%d ", pBN->numChannels);
  os << format("mmaV2Bias=%d inDataQ=%d outDataQ=%d weightsQ=%d\n",
           pBN->mmaV2Bias, pBN->inDataQ, pBN->outDataQ, pBN->weightsQ);
  if(pLayer->actParams.actType == TIDL_NoAct)
  {
    os << format("weightScale=%f biasScale=%f\n",
            pBN->weightScale, pBN->biasScale);
  }

  if (pLayer->weightsElementSizeInBits == 8)
  {
    os << format("weights:");
    printItems((uint8_t*)weightPtr, numChannels);
  }
  else if (pLayer->weightsElementSizeInBits == 16)
  {
    os << format("weights:");
    printItems((uint16_t*)weightPtr, numChannels);
  }
  os << format("\nbias:");
  printItems((uint16_t*)biasPtr, numChannels);

  os << format("\n");
}

void TIDL_dump::dumpBiasLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_BiasParams_t *pBias = &pLayer->layerParams.biasParams;
  os << format("numChannels=%d ", pBias->numChannels);
  os << format("biasQ=%d ", pBias->biasQ);
  os << format("inDataQ=%d ", pBias->inDataQ);
  os << format("outDataQ=%d\n", pBias->outDataQ);
  os << format("bias=0x%x ", pBias->bias);
}

void TIDL_dump::dumpConcatLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_ConcatParams_t *pConcat = &pLayer->layerParams.concatParams;
  os << format("axis=%d ", pConcat->axis);
  os << format("outDataQ=%d\n", pConcat->outDataQ);
}

void TIDL_dump::dumpSliceLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_SliceLayerParams_t *pSlice = &pLayer->layerParams.sliceParams;
  os << format("slicePoints=");
  for (int i = 0; i < TIDL_NUM_OUT_BUFS+1; ++i)
  {
     os << format("%d", pSlice->slicePoints[i]);
     if (i != TIDL_NUM_OUT_BUFS) os << format(",");
  }
  os << format("\n");
  os << format("axis=%d stride=%d\n, ", pSlice->axis, pSlice->stride);
}

void TIDL_dump::dumpCropLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_CropParams_t *pCrop = &pLayer->layerParams.cropParams;
  os << format("numChannels=%d ", pCrop->numChannels);
  os << format("inDataQ=%d ", pCrop->inDataQ);
  os << format("outDataQ=%d ", pCrop->outDataQ);
  os << format("offsetW=%d ", pCrop->offsetW);
  os << format("offsetH=%d\n", pCrop->offsetH);
  os << format("CropLayerMode=%s\n", multiCoreString(pCrop->multiCoreMode).c_str());
}

void TIDL_dump::dumpGridSampleParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_GridSampleParams_t *pGridSample = &pLayer->layerParams.gridSampleParams;
  os << format("mode=%s\n", gridSampleModeString(pGridSample->mode).c_str());
  os << format("padding_mode=%s\n", gridSamplePadModeString(pGridSample->padding_mode).c_str());
  os << format("align_corners=%d\n", pGridSample->align_corners);
}

void TIDL_dump::dumpDeformConvParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_DeformConvParams_t *pDeformConv = &pLayer->layerParams.deformConvParams;
  /*To be updated*/
}

void TIDL_dump::dumpTopKParams (const sTIDL_Layer_t *pLayer)
{
  const sTIDL_TopKParams_t *pTopK = &pLayer->layerParams.topKParams;
  os << format("K=%d, axis=%d\n", pTopK->K, pTopK->axis);
  os << format("largest=%d, sorted=%d\n", pTopK->largest, pTopK->sorted);
  os << format("\n");  
}

void TIDL_dump::dumpGatherNDParams (const sTIDL_Layer_t *pLayer)
{
  const sTIDL_GatherNDLayerParams_t *pGatherND = &pLayer->layerParams.gatherNDParams;
  os << format("batchDims=%d\n", pGatherND->batchDims);
  os << format("\n");  
}


void TIDL_dump::dumpGatherElementsParams (const sTIDL_Layer_t *pLayer)
{
  const sTIDL_GatherElementsParams_t *pGatherElements = &pLayer->layerParams.gatherElementsParams;
  os << format("axis=%d\n", pGatherElements->axis);
  os << format("\n");  
}

void TIDL_dump::dumpNonZeroParams (const sTIDL_Layer_t *pLayer)
{
  const sTIDL_NonZeroParams_t *pNonZero = &pLayer->layerParams.nonZeroParams;
  os << format("numValidInputDims=%d\n", pNonZero->numValidInputDims);
  os << format("\n");  
}

void TIDL_dump::dumpTileParams (const sTIDL_Layer_t *pLayer)
{
  const sTIDL_TileParams_t *pTile = &pLayer->layerParams.tileParams;
  os << format("repeats=");
  for (int i = 0; i < TIDL_DIM_MAX; ++i)
  {
     os << format("%d", pTile->repeats[i]);
     if (i != (TIDL_DIM_MAX - 1)) os << format(",");
  }
  os << format("\n");  
}

void TIDL_dump::dumpRMSNormalizationParams (const sTIDL_Layer_t *pLayer)
{
  const sTIDL_RMSNormParams_t *pRMSNorm = &pLayer->layerParams.rmsNormParams;
  os << format("axis=%d, epsilon=%f, stash_type=%d\n", pRMSNorm->axis, pRMSNorm->epsilon, pRMSNorm->stashType);
}

void TIDL_dump::dumpShapeParams (const sTIDL_Layer_t *pLayer)
{
  const sTIDL_ShapeParams_t *pShape = &pLayer->layerParams.shapeParams;
  os << format("start=%d end=%d\n", pShape->start, pShape->end);
}

void TIDL_dump::dumpTransposeParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_TransposeParams_t *pTranspose = &pLayer->layerParams.transposeParams;
  os << format("perm=");
  for (int i = 0; i < TIDL_DIM_MAX; i++)
  {
    if (i == TIDL_DIM_MAX - 1)
    {
      os << format("%d", pTranspose->perm[i]);
    }
    else
    {
      os << format("%d,", pTranspose->perm[i]);
    }
  }
  os << format("\n");
}

void TIDL_dump::dumpDataConvertParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_dataConvertParams_t *pDataConvert = &pLayer->layerParams.dataConvertParams;
  os << format("dataConvertType=%s\n",
       dataConvertTypeString(pDataConvert->type).c_str());
  os << format("inLayout=%s\n",
       dataConvertLayoutString(pDataConvert->layout).c_str());
  os << format("outLayout=%s\n",
       dataConvertLayoutString(pDataConvert->outLayout).c_str());
  os << format("inZeroPoint=%d ", pDataConvert->inZeroPoint);
  os << format("outZeroPoint=%d\n", pDataConvert->outZeroPoint);
}

void TIDL_dump::dumpArgOpLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_ArgOpParams_t *pArgOp = &pLayer->layerParams.argOpParams;
  os << format("numChannels=%d ", pArgOp->numChannels);
  os << format("inDataQ=%d ", pArgOp->inDataQ);
  os << format("selectLastIndex=%d\n", pArgOp->selectLastIndex);
} 

void TIDL_dump::dumpDetectionOutputLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_DetectOutputParams_t *pOD = &pLayer->layerParams.detectOutParams;
  os << format("processingType=%s, ",
               outputDetectionTypeString(pOD->processingType).c_str());
  os << format("priorBox=%d, ", pOD->priorBox);
  os << format("priorBoxSize=%d ", pOD->priorBoxSize);
  os << format("numClasses=%d\n", pOD->numClasses);
  os << format("backgroundLabelId=%d ", pOD->backgroundLabelId);
  os << format("codeType=%d ", pOD->codeType);
  os << format("confThreshold=%f ", pOD->confThreshold);
  os << format("nmsThreshold=%f ", pOD->nmsThreshold);
  os << format("eta=%f\n", pOD->eta);
  os << format("topK=%d ", pOD->topK);
  os << format("keepTopK=%d ", pOD->keepTopK);
  os << format("shareLocation=%d ", pOD->shareLocation);
  os << format("varianceEncoded=%d ", pOD->varianceEncoded);
  os << format("numKeypoints=%d ", pOD->numKeypoints);
  os << format("numHeads=%d\n", pOD->numHeads);
  os << format("imWidth/Height=[%d,%d] ", pOD->imWidth, pOD->imHeight);
  os << format("metaArchType=%s\n",
               metaArchTypeString(pOD->metaArchType).c_str());
}

void TIDL_dump::dumpShuffleChannelLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_ShuffleLayerParams_t *pShuf = &pLayer->layerParams.shuffleLayerParams;
  os << format("numGroups=%d ", pShuf->numGroups);
  os << format("resvd=%d\n",  pShuf->resvd);
}

void TIDL_dump::dumpResizeLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_ResizeLayerParams_t *pResize = &pLayer->layerParams.resizeParams;
  os << format("mode=%s ", resizeModeString(pResize->mode).c_str());
  os << format("resizeRatio=[");
  for (int i = 0; i < TIDL_DIM_MAX; ++i)
  {
      os << format("%f", pResize->resizeRatio[i]);
      if (i != TIDL_DIM_MAX-1) os << format(",");
  }
   os << format("]\n");
}

void TIDL_dump::dumpRoiPoolingLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_RoiPoolingLayerParams_t *pRoiPool = &pLayer->layerParams.roiPoolingParams;
  os << format("type=%s ", roiPoolingTypeString(pRoiPool->poolingType)).c_str();
  os << format("imHeight/Width=[%d,%d]\n", pRoiPool->imHeight, pRoiPool->imWidth);
}

void TIDL_dump::dumpDepthToSpaceLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_DepthToSpaceParams_t *pDTS = &pLayer->layerParams.depthToSpaceParams;
  os << format("blockSize=%d\n", pDTS->blockSize);
}

void TIDL_dump::dumpPadLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_PadLayerParams_t *pPad = &pLayer->layerParams.padLayerParams;
  os << format("padT/B/L/R=[%d,%d,%d,%d]\n",
       pPad->padT, pPad->padB, pPad->padL, pPad->padR);
  os << format("padConstValue=%d padType=%s\n",
       pPad->padConstValue, padTypeString(pPad->padType).c_str());
  os << format("perChannelPadConstTensorOffset=0x%x\n",
       pPad->perChannelPadConstTensorOffset);
}

void TIDL_dump::dumpLSTMParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_LSTMParams_t *pLSTM = &pLayer->layerParams.lstmParams;
  int32_t num_directions = 1;
  if(pLSTM->direction == TIDL_RecurrentBidirectional)
  {
    num_directions = 2;
  }
  os << format("activations=");
  for(int32_t dirIdx = 0; dirIdx < num_directions; dirIdx++)
  {
    os << format("%s,", actTypeShort(pLSTM->activations[dirIdx * 3 + 0]).c_str());
    os << format("%s,", actTypeShort(pLSTM->activations[dirIdx * 3 + 1]).c_str());
    if(dirIdx < num_directions - 1)
    {
      os << format("%s,", actTypeShort(pLSTM->activations[dirIdx * 3 + 2]).c_str());
    }
    else
    {
      os << format("%s\n", actTypeShort(pLSTM->activations[dirIdx * 3 + 2]).c_str());
    }
  }

  if(pLSTM->isClipSet == 1)
  {
    os << format("clip=%f\n", pLSTM->clip);
  }
  os << format("direction=%s\n", recurrentDirectionString(pLSTM->direction).c_str());
  os << format("hidden_size=%d\n", pLSTM->hidden_size);
  os << format("input_forget=%d\n", pLSTM->input_forget);
  os << format("layout=%d\n", pLSTM->layout);
}

void TIDL_dump::dumpGRUParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_GRUParams_t *pGRU = &pLayer->layerParams.gruParams;
  int32_t num_directions = 1;
  if(pGRU->direction == TIDL_RecurrentBidirectional)
  {
    num_directions = 2;
  }
  os << format("activations=");
  for(int32_t dirIdx = 0; dirIdx < num_directions; dirIdx++)
  {
    os << format("%s,", actTypeShort(pGRU->activations[dirIdx * 2 + 0]).c_str());
    if(dirIdx < num_directions - 1)
    {
      os << format("%s,", actTypeShort(pGRU->activations[dirIdx * 2 + 1]).c_str());
    }
    else
    {
      os << format("%s\n", actTypeShort(pGRU->activations[dirIdx * 2 + 1]).c_str());
    }
  }

  if(pGRU->isClipSet == 1)
  {
    os << format("clip=%f\n", pGRU->clip);
  }
  os << format("direction=%s\n", recurrentDirectionString(pGRU->direction).c_str());
  os << format("hidden_size=%d\n", pGRU->hidden_size);
  os << format("layout=%d\n", pGRU->layout);
  os << format("linear_before_reset=%d\n", pGRU->linear_before_reset);
}

void TIDL_dump::dumpRNNParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_RNNParams_t *pRNN = &pLayer->layerParams.rnnParams;
  int32_t num_directions = 1;
  if(pRNN->direction == TIDL_RecurrentBidirectional)
  {
    num_directions = 2;
  }
  os << format("activations=");
  for(int32_t dirIdx = 0; dirIdx < num_directions; dirIdx++)
  {
    if(dirIdx < num_directions - 1)
    {
      os << format("%s,", actTypeShort(pRNN->activations[dirIdx]).c_str());
    }
    else
    {
      os << format("%s\n", actTypeShort(pRNN->activations[dirIdx]).c_str());
    }
  }

  if(pRNN->isClipSet == 1)
  {
    os << format("clip=%f\n", pRNN->clip);
  }
  os << format("direction=%s\n", recurrentDirectionString(pRNN->direction).c_str());
  os << format("hidden_size=%d\n", pRNN->hidden_size);
  os << format("layout=%d\n", pRNN->layout);
}

void TIDL_dump::dumpOdOutputReformatLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_odOutputReformatLayerParams_t *pOdOutput = &pLayer->layerParams.odOutputReformatLayerParams;
  os << format("odOutputType=%s\n", odOutputTypeString(pOdOutput->layerType).c_str());
}

void TIDL_dump::dumpCustomLayerParams(const sTIDL_Layer_t *pLayer)
{
  const sTIDL_CustomParams_t *pCustom = &pLayer->layerParams.customParams;
  os << format("customLayerType=%d ", pCustom->customLayerType);
  os << format("padW/H=[%d,%d] ", pCustom->padW, pCustom->padH);
  os << format("memOverlapType=%d\n", pCustom->memOverlapType);
  os << format("forceInPlace=%d\n", pCustom->forceInPlace);
  os << format("doesLayerChangePadding=%d ", pCustom->doesLayerChangePadding);
  os << format("doesLayerFillOutXPadding=%d\n", pCustom->doesLayerFillOutXPadding);
  os << format("rsvdPassThrough=%d\n", pCustom->rsvdPassThrough);
  os << format("rsvdPassThroughSize=%d\n", pCustom->rsvdPassThroughSize);
}

int32_t TIDL_isOutDataBuff(const sTIDL_Network_t *pTIDLNetStructure, int32_t dataId,
  int32_t layersGroupId)
{
  int32_t i, j;
  for (i = 0; i < pTIDLNetStructure->numLayers; i++)
  {
    for (j = 0; j < pTIDLNetStructure->TIDLLayers[i].numInBufs; j++)
    {
      if ((pTIDLNetStructure->TIDLLayers[i].layersGroupId != layersGroupId) &&
        (pTIDLNetStructure->TIDLLayers[i].inData[j] == dataId))
      {
        return 1;
      }
    }
  }
  return 0;
}

sBufParams_t * TIDL_getOutBufParams(const sTIDL_Network_t *net, int32_t dataId, int32_t currLayersGroupId)
{
  sBufParams_t *bufParams = NULL;
  sBufDataBase_t bufDBase;
  sBufDataBase_t *bufDB = &bufDBase;
  sGraphCompilerOutArgs_t *gcOutArgs = NULL;
  sGCCommonDataBase_t *commonDatabase = NULL;

  if ( net->isGCInfoAvailable != 0 )
  {
    gcOutArgs  = (sGraphCompilerOutArgs_t *)get_int8_t_pointer((int8_t *)(net), net->netBufInfo[TIDL_NET_GC_BUF].offset);
    commonDatabase = (sGCCommonDataBase_t *)getGCDataBase(gcOutArgs,DATABASE_COMMON);
    bufDB->pBufList = (sBufParams_t*) &commonDatabase->dbPayLoad[commonDatabase->dbParams[DB_COMMON_BUF].startOffset];
    bufDB->count = gcOutArgs->commonDatabaseInfo.bufDB.count;

    for(int32_t i = 0; i < bufDB->count; i++ )
    {
      bufParams = &bufDB->pBufList[i];
      sMetaDataID_t pMetaDataID;
      getMetaDataID(bufParams->dataId, &pMetaDataID);
      int32_t dataLayerId = pMetaDataID.layerId;

      if(dataId == dataLayerId && (TIDL_isOutDataBuff(net, dataLayerId, currLayersGroupId) == 1) && (pMetaDataID.type == BUF_FM_FULL))
      {
        break;
      }
    }
  }
  return bufParams;
}

// Return a pointer to the Graph Compiler info if the network has it
const sGraphCompilerOutArgs_t *TIDL_dump::pSimInfo()
{
  if (pNet->isGCInfoAvailable == 0)
     return nullptr;
  return  (sGraphCompilerOutArgs_t *)((int8_t *)(pNet) + pNet->netBufInfo[TIDL_NET_GC_BUF].offset);
}

void TIDL_dump::dumpPerfSimInfo(int layerNum)
{
  char memSpaceString[SUPERMEMTYPE_TOTAL][64] = {"L2", "MSMC", "DDR","DDR_PERSIST"};
   const sGraphCompilerOutArgs_t *gcOutArgs = pSimInfo();
   if (!gcOutArgs)
      return;

   os << format("Graph compiler Info:\n");
   os.push();
   sBufParams_t * outBufParams = TIDL_getOutBufParams(pNet, layerNum, -1);
   int32_t elemSize = getByteCnt(outBufParams->dataType);
   os << format(" space:%s", (char *)&memSpaceString[outBufParams->space]);
   os << format(" baseMem(bytes):%d-%d", outBufParams->baseMem, outBufParams->baseMem + (outBufParams->bufSize * elemSize));
   os << format(" size(elem):%d\n", outBufParams->bufSize);
   os << format(" bufferWidth(elem):%d", outBufParams->bufWidth);
   os << format(" activeBufferWidth(elem):%d", outBufParams->activeBufWidth);
   os << format(" bufferHeight(elem):%d\n", outBufParams->bufHeight);
   os << format(" padRows:%d", outBufParams->padParams.rows);
   os << format(" padCols:%d\n", outBufParams->padParams.cols);
   os.pop();
}

// Dump the IOBufDesc information that describes TIDL network inputs and outputs
void TIDL_dump::dumpIODesc()
{
  if (pIODesc == nullptr)
     return;
  int i;
  os << format("TIDL IO Buffers --------------------------------------------------\n");
  #if 0
  os << format("l1Mem: 0x%x  l2Mem: 0x%x  l3Mem: 0x%x\n",
     pIODesc->l1MemSize,
     pIODesc->l2MemSize,
     pIODesc->l3MemSize);
  #endif
  for (i = 0; i < pIODesc->numInputBuf; ++i)
  {
     os.push();
     os << format("Input[%d]:\n", i);
     os.push();
     os << format("inDataFormat=%s Size(CHW)=[%d,%d,%d] inChannelPitch=%d\n",
        inDataFormatString(pIODesc->inDataFormat[i]).c_str(),
        pIODesc->inNumChannels[i],
        pIODesc->inHeight[i],
        pIODesc->inWidth[i],
        pIODesc->inChannelPitch[i]);

     os << format("Resize H/W=[%d,%d] inResizeType=%s\n",
        pIODesc->resizeHeight[i],
        pIODesc->resizeWidth[i],
        inResizeTypeString(pIODesc->inResizeType[i]).c_str());

     os << format("inPad T/B/L/R=[%d,%d,%d,%d] inPadCh=%d\n",
        pIODesc->inPadT[i],
        pIODesc->inPadB[i],
        pIODesc->inPadL[i],
        pIODesc->inPadR[i],
        pIODesc->inPadCh[i]);
     os << format("inElementType=%s rawDataInElementType=%s\n",
        elementTypeString(pIODesc->inElementType[i]).c_str(),
        elementTypeString(pIODesc->rawDataInElementType[i]).c_str());
     os << format("inDataId=%d inDataName=%s inTensorScale=%f\n",
        pIODesc->inDataId[i],
        pIODesc->inDataName[i],
        pIODesc->inTensorScale[i]);
     os.pop();
     os.pop();
  }
  for (i = 0; i < pIODesc->numOutputBuf; ++i)
  {
     os.push();
     os << format("Output[%d]:\n", i);
     os.push();
     os << format("Size(CHW)=[%d,%d,%d] outChannelPitch=%d\n",
        pIODesc->outNumChannels[i],
        pIODesc->outHeight[i],
        pIODesc->outWidth[i],
        pIODesc->outChannelPitch[i]);
     os << format("outPad T/B/L/R=[%d,%d,%d,%d] outPadCh=%d\n",
        pIODesc->outPadT[i],
        pIODesc->outPadB[i],
        pIODesc->outPadL[i],
        pIODesc->outPadR[i],
        pIODesc->outPadCh[i]);
     os << format("outElementType=%s\noutDataId=%d outDataName=%s\n",
        elementTypeString(pIODesc->outElementType[i]).c_str(),
        pIODesc->outDataId[i],
        pIODesc->outDataName[i]);
     os.pop();
     os.pop();
  }
}

//-------------------------------------------------------------------------------
// Helper function to print values of a given type
template <typename Tw>
void TIDL_dump::printItems(Tw *items, int n)
{
   int i;
   int minVal = (n < 10)? n: 10;
   // only print first few items unless option given
   int nToPrint = hasOption("full_weights") ? n : minVal;
   for (i = 0; i < nToPrint; ++i)
   {
    if (i == nToPrint - 1)
    {
      os << format("%d", items[i]);
    }
    else
    {
      os << format("%d,", items[i]);
    }
   }
   if (n != nToPrint)
   {
      os << "...(" << n << ")";
   }
}

// Helper function to create C++ string from printf-style format spec.
// The cout<< style is cumbersome and stateful (e.g. std::hex persists
// to next call - ugh).
std::string TIDL_dump::format(const char *s, ...) const
{
   // dummy call to get result size
   va_list args, args_copy;
   va_start(args, s);
   va_copy(args_copy, args);
   auto sz = std::vsnprintf(nullptr, 0, s, args_copy);
   va_end(args_copy);

   std::string str;
   str.resize(sz+1);
   std::vsnprintf(&str.front(), sz+1, s, args);
   va_end(args);
   str.resize(sz);

   return str;
}
