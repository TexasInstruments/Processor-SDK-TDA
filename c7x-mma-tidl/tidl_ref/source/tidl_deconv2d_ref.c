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
@file    tidl_deconv2d_base.c
@brief   This file contains basic functions required by Convolution Layer
@version 0.1 (Oct 2016) : Initial version [ADK]
@version 0.5 (Jan 2017) : Cleaned up [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_deconv2d_ref.h"
#include "tidl_commonUtils.h"
#include <math.h>

#define BIAS_BITDEPTH_32BITS (0)
#define TEMP_DISABLE_DECONV (1)

#define COL_FLOW_COEFF_DUP_COPY_FACTOR (10U)

/**
 * @brief  This function is used by TIDL_deconv2DAlloc(), TIDL_deconv2DInit() in tidl_alg.c,
 *         and TIDL_refDeconv2d() in tidl_deconv2d.c,to get the size of the scratch buffer
 *         for the deconvolution layer.
 *
 * @param params  : tidl parameters at the create time
 * @param layerIdx : index of the current layer
 * @return int32_t : Returns scratchDataSize for Deconv layer
 */
int32_t TIDL_deconv2DGetScratchDataSize(const TIDL_CreateParams *params, int32_t layerIdx)
{
    int32_t scratchDataSize;
    const sTIDL_ConvParams_t * conv2dparams =
      &params->net->TIDLLayers[layerIdx].layerParams.convParams;

  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(params->net,
                                                        params->net->TIDLLayers[layerIdx].inData[0]);

  uint16_t width = inDataParams->dimValues[TIDL_DIM_WIDTH];
  uint16_t height = inDataParams->dimValues[TIDL_DIM_HEIGHT] + (conv2dparams->padH * 2);
  uint16_t coeffsWidth = conv2dparams->kernelW;
  uint16_t coeffsHeight = conv2dparams->kernelH;
  uint16_t dilationWidth = conv2dparams->dilationW;
  uint16_t dilationHeight = conv2dparams->dilationH;
  uint32_t actualKernelWidth = (((coeffsWidth - (uint32_t)1) * dilationWidth) + (uint32_t)1);
  uint32_t actualKernelHeight = (((coeffsHeight - (uint32_t)1) * dilationHeight) + (uint32_t)1);
  uint32_t strideWidth = conv2dparams->strideW;
  uint32_t strideHeight = conv2dparams->strideH;

  int32_t accWidth;
  int32_t accHeight;
  accWidth = actualKernelWidth + ((strideWidth) * ((width - 1U)));
  accHeight = actualKernelHeight + ((strideHeight) * ((height - 1U)));
  int32_t accChPitch = accWidth * (accHeight + 1);

    if (TIDL_getDatElementSize(params->net->TIDLLayers[layerIdx].outData.elementType) == 2)
    {
      scratchDataSize = params->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH]*accChPitch*inDataParams->dimValues[TIDL_DIM_BATCH]*(int32_t)sizeof(int64_t);
    }
    else
    {
      scratchDataSize = params->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH]*accChPitch*inDataParams->dimValues[TIDL_DIM_BATCH]*(int32_t)sizeof(int32_t);
    }

  return scratchDataSize;
}

/**
 * @brief  This is the reference Deconv2d core Kernel implementation
 *
 *
 * @param numTotRoi  : no of total ROI
 * @param numGroups : total no of groups
 * @param numInChannels : no of input channles
 * @param numOutChannels :  no of output channles
 * @param inChPitch : Channel pitch of the input buffer
 * @param outChPitch : Channel pitch of the output buffer
 * @param width : Width of the input buffer
 * @param height : Height of the input buffer
 * @param inImPitch : line pitch of the input buffer
 * @param outImPitch : line pitch of the output buffer
 * @param coeffsWidth : width of the coeff buffer
 * @param coeffsHeight : height of the coeff buffer
 * @param dilationWidth : Dilation in the width dimension
 * @param dilationHeight: Dilation in the height dimension
 * @param strideWidth : Stride in width dimension
 * @param strideHeight: Stride in height dimension
 * @param enableBias : Flag to indicate if bias in enabled
 */
template<class Tin, class Tw, class Tb, class Tacc> void TIDL_refDeconv2dKernel(
    Tin *pInChannel,
    Tw *pCoeffs,
    Tb *pBias,
    Tacc *accPtr,
    Tacc *min,
    Tacc *max,
    int32_t numTotRoi,      // N
    int32_t numGroups,      // G
    int32_t numInChannels,  // Ci
    int32_t numOutChannels, // Co
    int32_t inChPitch,      // CiStride
    int32_t outChPitch,     // CoStide
    int32_t width,          // Wi
    int32_t height,         // Hi
    int32_t inImPitch,      // HiStride
    int32_t outImPitch,     // HoStride
    int32_t coeffsWidth,    // KW
    int32_t coeffsHeight,   // KH
    int32_t dilationWidth,  // DW
    int32_t dilationHeight, // DH
    int32_t strideWidth,    // SW S[0]
    int32_t strideHeight,   // SH S[1]
    int32_t enableBias)
{

  int32_t i0, i2, i3, i4, i5, i6, i7, i8;
  int32_t coeffOffset, indataOffset, outdataOffset;
  Tacc outAcc;
  Tin inData;
  Tw coefData;
  Tb biasData;

  /* OPENACC(data present(pBias[0:1+(numGroups-1)*numOutChannels + (numOutChannels-1)], \
                       accPtr[0:1+(numGroups-1)*numOutChannels*outChPitch + (numTotRoi-1)*numGroups*numOutChannels*outChPitch + (numOutChannels-1)*outChPitch + (outChPitch-1)])) */
  // OPENACC(parallel loop collapse(4))
  for (i8 = 0; i8 < numTotRoi; i8++)
  {
    for (i7 = 0; i7 < numGroups; i7++)
    {
      for (i6 = 0; i6 < numOutChannels; i6++)
      {
        for (i5 = 0; i5 < outChPitch; i5++)
        {
          if (enableBias != 0)
          {
            biasData = pBias[(i7 * numOutChannels) + i6];
          }
          else
          {
            biasData = 0;
          }

          outdataOffset = (i7 * numOutChannels * outChPitch) + (i8 * numGroups * numOutChannels * outChPitch);
          accPtr[outdataOffset + (i6 * outChPitch) + i5] = biasData;
        }
      }
    }
  }

  /* OPENACC(data copyin(pCoeffs[0:1+(numGroups-1)*numInChannels * coeffsHeight * coeffsWidth *numOutChannels + \
                (numOutChannels-1)* numInChannels * coeffsHeight * coeffsWidth + (numInChannels-1) * coeffsHeight * coeffsWidth + ((coeffsHeight-1) * coeffsWidth) + (coeffsWidth-1)], \
               pInChannel[0:1+(numGroups-1)*numInChannels*inChPitch + (numTotRoi-1)*numGroups*numInChannels*inChPitch + (numInChannels-1)* inChPitch + (((height)-1)* inImPitch) + (width-1)]) \
               present(accPtr[0:1+(numGroups-1)*numOutChannels*outChPitch + (numTotRoi-1)*numGroups*numOutChannels*outChPitch + (numOutChannels-1)*outChPitch + ((height-1) * outImPitch * strideHeight) + ((width-1)*strideWidth) + \
                    ((coeffsHeight-1) * dilationHeight * outImPitch) + ((coeffsWidth-1)*dilationWidth)])) */
  // OPENACC(parallel loop collapse(5))
  for (i8 = 0; i8 < numTotRoi; i8++)
  {
    for (i7 = 0; i7 < numGroups; i7++)
    {
      for (i6 = 0; i6 < numOutChannels; i6++)
      {
        for (i2 = 0; i2 < (height); i2++)
        {
          for (i3 = 0; i3 < (width); i3++)
          {
            // OPENACC(loop seq)
            for (i0 = 0; i0 < numInChannels; i0++)
            {
              indataOffset = (i7 * numInChannels * inChPitch) + (i8 * numGroups * numInChannels * inChPitch);
              outdataOffset = (i7 * numOutChannels * outChPitch) + (i8 * numGroups * numOutChannels * outChPitch);
              inData = pInChannel[indataOffset + (i0 * inChPitch) + ((i2)*inImPitch) + i3];
              coeffOffset = (i7 * numInChannels * coeffsHeight * coeffsWidth * numOutChannels) +
                            (i6 * numInChannels * coeffsHeight * coeffsWidth) + (i0 * coeffsHeight * coeffsWidth);
              // OPENACC(loop seq)
              for (i4 = 0; i4 < coeffsHeight; i4++)
              {
                // OPENACC(loop seq)
                for (i5 = 0; i5 < coeffsWidth; i5++)
                {
                  coefData = pCoeffs[coeffOffset + (i4 * coeffsWidth) + i5];
                  accPtr[outdataOffset + (i6 * outChPitch) + (i2 * outImPitch * strideHeight) + (i3 * strideWidth) +
                         (i4 * dilationHeight * outImPitch) + (i5 * dilationWidth)] += (inData * coefData);
                }
              }
            }
          }
        }
      }
    }
  }

  // OPENACC(data present(accPtr[0:1+(numGroups-1)*numOutChannels*outChPitch + (numTotRoi-1)*numGroups*numOutChannels*outChPitch + (numOutChannels-1)*outChPitch + (outChPitch-1)]))
  // OPENACC(parallel loop collapse(4))
  for (i8 = 0; i8 < numTotRoi; i8++)
  {
    for (i7 = 0; i7 < numGroups; i7++)
    {
      for (i6 = 0; i6 < numOutChannels; i6++)
      {
        for (i5 = 0; i5 < outChPitch; i5++)
        {
          outdataOffset = (i7 * numOutChannels * outChPitch) + (i8 * numGroups * numOutChannels * outChPitch);
          outAcc = accPtr[outdataOffset + (i6 * outChPitch) + i5];
          *min = (outAcc < *min) ? outAcc : *min;
          *max = (outAcc > *max) ? outAcc : *max;
        }
      }
    }
  }
}

/**
 * @brief  This funciton does the quant and saturation of the accumulator data
 *
 * @param pInChannel : Input pointer on which Deconv is applied
 * @param pCoeffs : Pointer to weights buffer
 * @param pBias  : Pointer to the Bias values
 * @param pOutChanne : Pointer to output after DeConv opreation
 * @param accPtr : Pointer to store intermidiate accumulator values
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param params : copy of Deconv parameters
 * @param buffParams : copy of Deconv buffer parameters
 * @param satLow : min value for the saturation
 * @param satHigh : max value for the saturation
 */
template<class Tin, class Tw, class Tb, class Tout, class Tacc>
    int32_t TIDL_refDeconv2d(
        Tin *pInChannel,
        Tw *pCoeffs,
        Tb *pBias,
        Tout *pOutChanne,
        Tacc *accPtr,
        const TIDL_CreateParams *createParams,
        int32_t layerIdx,
        sTIDL_ConvParams_t *params,
        tidlConv2dBuffParams_t *buffParams,
        int32_t satLow,
        int32_t satHigh)
{
  int32_t status = IALG_EOK;
  sTIDL_Network_t *net = createParams->net;
  sTIDL_Layer_t *tidlLayer = &net->TIDLLayers[layerIdx];
  uint16_t numInChannels = params->numInChannels / params->numGroups;
  uint16_t numOutChannels = params->numOutChannels / params->numGroups;
  uint16_t coeffsWidth = params->kernelW;
  uint16_t coeffsHeight = params->kernelH;
  uint16_t strideWidth = params->strideW;
  uint16_t strideHeight = params->strideH;
  uint16_t dilationWidth = params->dilationW;
  uint16_t dilationHeight = params->dilationH;
  uint16_t p_w = params->padW;
  uint16_t p_h = params->padH;
  uint16_t width = buffParams->inWidth;
  uint16_t height = buffParams->inHeight + (2 * (int16_t)p_h);
  uint16_t inImPitch = buffParams->inPitch;
  uint32_t inChPitch = buffParams->inChPitch;
  uint16_t outWidth;
  uint16_t outHeight;
  uint16_t outImPitch = buffParams->outPitch;
  uint32_t outChPitch = buffParams->outChPitch;
  uint16_t outRoundBits;
  int32_t numTotRoi = buffParams->numTotRoi;
  uint16_t numGroups = params->numGroups;
  int32_t outOffsetW;
  int32_t outOffsetH;
  uint32_t actualKernelWidth;
  uint32_t actualKernelHeight;
  int32_t accWidth;
  int32_t accHeight;
  int32_t accChPitch;

  /*
  https://github.com/BVLC/caffe/blob/master/src/caffe/layers/deconv_layer.cpp

  const int kernel_extent = dilation_data[i] * (kernel_shape_data[i] - 1) + 1;
  const int output_dim = stride_data[i] * (input_dim - 1)
  + kernel_extent - 2 * pad_data[i
  */
  actualKernelWidth = (((coeffsWidth - (uint32_t)1) * dilationWidth) + (uint32_t)1);
  actualKernelHeight = (((coeffsHeight - (uint32_t)1) * dilationHeight) + (uint32_t)1);
  outWidth = buffParams->outWidth;   //(width - 1)*strideWidth + actualKernelWidth - p_w * 2;
  outHeight = buffParams->outHeight; //(height - 1)*strideHeight + actualKernelHeight - p_h * 2;
  outOffsetW = p_w;
  outOffsetH = (p_h * strideHeight) + p_h;
  accWidth = actualKernelWidth + ((strideWidth) * ((width - 1U)));
  accHeight = actualKernelHeight + ((strideHeight) * ((height - 1U)));
  accChPitch = accWidth * (accHeight);
  int32_t i2, i3, i6, i8;
  Tacc outAcc;
  int32_t outdataOffset;
  Tacc min, max;
  int64_t temp;
  int32_t accMemSize;

#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
  /* TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == FALSE is failing safety check because this statement is always true so no negative test case */
  /* TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == FALSE will pass safety check when asymmetric support for deconv2d layer is enabled */
  if ((net->quantizationStyle != TIDL_QuantStyleAsymNP2_TFL) && ((buffParams->quantizationStyle == TIDL_QuantStyleNP2Fixed) || (buffParams->quantizationStyle == TIDL_QuantStyleP2Dynamic) || (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)FALSE)))
#else
  if ((net->quantizationStyle != TIDL_QuantStyleAsymNP2_TFL) && ((buffParams->quantizationStyle == TIDL_QuantStyleNP2Fixed) || (buffParams->quantizationStyle == TIDL_QuantStyleP2Dynamic)))
#endif
  {
    outRoundBits = buffParams->secondRoundBits;
  }
  min = (Tacc)((((uint64_t)1) << ((sizeof(Tacc) * 8LU) - 1LU)) - 1LU);
  temp = (int64_t)(((uint64_t)1) << ((sizeof(Tacc) * 8LU) - 1LU));
  max = (Tacc)temp * -1;

  accMemSize = TIDL_deconv2DGetScratchDataSize(createParams, layerIdx);

  if (buffParams->scratchSize >= accMemSize)
  {
    accPtr = (Tacc *)buffParams->scratchMem;
    accPtr += buffParams->outPitch;
  }
  else
  {
    tidl_printf(0, "Memory for  TIDL_refDeconv2d accumulator is not sufficient exiting...\n    ");
    status = IALG_EFAIL;
  }
  if (status == IALG_EOK)
  {

    int32_t numTotOutChannels = params->numOutChannels;
    /* OPENACC(data copyin(pBias[0:1+(numGroups-1)*numOutChannels + (numOutChannels-1)]) \
                 copy(pOutChanne[0 :1+(numTotRoi-1)*numTotOutChannels*outChPitch + (numTotOutChannels -1) * outChPitch + ((outHeight-1)* outImPitch) + (outWidth-1)])) */
    {

      TIDL_refDeconv2dKernel(pInChannel, pCoeffs, pBias, accPtr, &min, &max, numTotRoi, numGroups, numInChannels,
                             numOutChannels, inChPitch, accChPitch, width, height, inImPitch, accWidth,
                             coeffsWidth, coeffsHeight, dilationWidth, dilationHeight, strideWidth, strideHeight, params->enableBias);

      if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
      {
        TIDL_Obj intAlgObj;
        TIDL_CreateParams createParamsCpy;

        (void)memcpy(&createParamsCpy, createParams, sizeof(TIDL_CreateParams));
        intAlgObj.createParams = (TIDL_CreateParams *)&createParamsCpy;

        if (TIDL_getDatElementSign(net->TIDLLayers[layerIdx].outData.elementType) == 1)
        {
          TIDL_UpdateScaleFactors(&intAlgObj, layerIdx, 1, min, max);
        }
        else
        {
          TIDL_UpdateScaleFactors(&intAlgObj, layerIdx, 1, 0, max);
        }
      }
      outRoundBits = net->TIDLLayers[layerIdx].outData.roundBits;

      if (typeid(Tin) == typeid(float32_tidl))
      {
        outRoundBits = 0;
      }

      sTIDL_Layer_t *pTIDLNet = &net->TIDLLayers[layerIdx];

      // OPENACC(data present(accPtr[0:(numTotRoi-1)*numTotOutChannels*accChPitch + (numTotOutChannels -1) * accChPitch + (((outHeight-1) + outOffsetH) * accWidth) + ((outWidth-1) + outOffsetW)]))
      // OPENACC(parallel loop collapse(4))
      for (i8 = 0; i8 < numTotRoi; i8++)
      {
        for (i6 = 0; i6 < numTotOutChannels; i6++)
        {
          for (i2 = 0; i2 < ((int32_t)outHeight); i2++)
          {
            for (i3 = 0; i3 < ((int32_t)outWidth); i3++)
            {
              outAcc = accPtr[(i8 * numTotOutChannels * accChPitch) + (i6 * accChPitch) + ((i2 + outOffsetH) * accWidth) + (i3 + outOffsetW)];
              if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat)
              {
                // OPENACC(routine(TIDL_floatSat))
                outAcc = TIDL_floatSat(outAcc, pTIDLNet);
              }
              else
              {
                // OPENACC(routine(TIDL_roundSat))
                outAcc = (Tacc)TIDL_roundSat((int64_t)outAcc, outRoundBits, satLow, satHigh);
              }
              outdataOffset = i8 * numTotOutChannels * outChPitch;
              pOutChanne[outdataOffset + (i6 * (int32_t)outChPitch) + ((i2) * (int32_t)outImPitch) + (i3)] = outAcc;
            }
          }
        }
      }
    }
  }
  return status;
}

/**
 * @brief  This funciton handles differnt input bit-depths for DeConv reference
 *
 * @param inPtr : Input pointer on which Deconv is applied
 * @param refPtr : Pointer to output after DeConv opreation
 * @param wgtPtr : Pointer to weights buffer
 * @param biasPtr  : Pointer to the Bias values
 * @param accPtr : Pointer to store intermidiate accumulator values
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param params : copy of Deconv parameters
 * @param buffParams : copy of Deconv buffer parameters
 * @param inElementType : ElementType of the input
 * @param inDataOffset : initial offest the input data buffer
 * @param outDataOffset : initial offset of the output data buffer
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tw, class Tb, class Tacc>
    int32_t TIDL_refDeconv2dBitDepth(
        void *inPtr,
        void *refPtr,
        Tw *wgtPtr,
        Tb *biasPtr,
        Tacc *accPtr,
        const TIDL_CreateParams *createParams,
        int32_t layerIdx,
        sTIDL_ConvParams_t *params,
        tidlConv2dBuffParams_t *buffParams,
        int32_t inElementType,
        int32_t inDataOffset,
        int32_t outDataOffset)
{
  int32_t status = IALG_EOK;
  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DECONV2D_FORCE_REF_INVALID_OUTDATATYPE)
  {
    inElementType = 4;
    buffParams->outElementType = 4;
  }
  if (inElementType == TIDL_SinglePrecFloat)
  {
    status = TIDL_refDeconv2d(
        ((float32_tidl *)inPtr + inDataOffset),
        wgtPtr,
        biasPtr,
        (((float32_tidl *)refPtr) + outDataOffset),
        accPtr,
        createParams, layerIdx, params, buffParams,
        1, 1); /* last 2 arguments not used for float */
  }
  else if ((buffParams->outElementType == TIDL_UnsignedChar) ||
           (buffParams->outElementType == TIDL_UnsignedShort))
  {
    if (inElementType == TIDL_SignedChar)
    {
      int8_t *input = (int8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((int8_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_UINT8, TIDL_SAT_HI_UINT8);
    }
    else if (inElementType == TIDL_UnsignedChar)
    {
      uint8_t *input = (uint8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((uint8_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_UINT8, TIDL_SAT_HI_UINT8);
    }
    else if (inElementType == TIDL_SignedShort)
    {
      int8_t *input = (int8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((int16_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_UINT16, TIDL_SAT_HI_UINT16);
    }
    else if (inElementType == TIDL_UnsignedShort)
    {
      uint8_t *input = (uint8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((uint16_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_UINT16, TIDL_SAT_HI_UINT16);
    }
    else
    {
      status = IALG_EFAIL;
    }
  }
  else /*  Un signed output */
  {
    if (inElementType == TIDL_SignedChar)
    {
      int8_t *input = (int8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((int8_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          (((int8_t *)refPtr) + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_INT8, TIDL_SAT_HI_INT8);
    }
    else if (inElementType == TIDL_UnsignedChar)
    {
      uint8_t *input = (uint8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((uint8_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          (((int8_t *)refPtr) + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_INT8, TIDL_SAT_HI_INT8);
    }
    else if (inElementType == TIDL_SignedShort)
    {
      int8_t *input = (int8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((int16_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          (((int16_t *)refPtr) + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16);
    }
    else if (inElementType == TIDL_UnsignedShort)
    {
      uint8_t *input = (uint8_t *)inPtr;
      status = TIDL_refDeconv2d(
          ((uint16_t *)input + inDataOffset),
          wgtPtr,
          biasPtr,
          (((int16_t *)refPtr) + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams,
          TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16);
    }
    else
    {
      status = IALG_EFAIL;
    }
  }
  return status;
}

/**
 * @brief DeConvolution layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of Deconv layer parameters
 * @param buffParams : copy of Deconv buffer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inElementType : ElementType of the input
 * @param numTotRoi : total no of ROI
 * @param sysMems : sysMem Handle
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_deconv2dRefProcess(
    const TIDL_CreateParams *createParams,
    sTIDL_AlgLayer_t *algLayer,
    sTIDL_Layer_t *tidlLayer,
    sTIDL_ConvParams_t *params,
    tidlConv2dBuffParams_t *buffParams,
    void *inPtr,
    void *outPtr)
{
  int32_t status = IALG_EOK;
  int32_t inElementType = buffParams->inElementType;
  // uint16_t numTotRoi = buffParams->numTotRoi;
  uint16_t outRoundBits = 0, inRoundBits = 0;
  int32_t outPutShift;
  int32_t j;
  uint8_t *refPtr;
  int32_t inDataOffset;
  int32_t outDataOffset;
  int32_t layerIdx;
  void *biasptr;
  int16_t *orgbiasptr;
  float32_tidl *orgbiasptr_float;
  void *accPtr = NULL;

  void *weightPtr = ((int8_t *)(createParams->net) + params->weights);
  void *biasPtr = ((int8_t *)(createParams->net) + params->bias);

  layerIdx = algLayer->layerIdx;

  if (createParams->net->quantizationStyle == TIDL_QuantStyleFixed)
  {
    outPutShift = (params->inDataQ + params->weightsQ - params->outDataQ);
    outRoundBits = 0;
    inRoundBits = (uint32_t)outPutShift - outRoundBits;
  }

  inRoundBits = outRoundBits + inRoundBits;
  outRoundBits = 0;

  refPtr = (uint8_t *)outPtr;

  orgbiasptr = (int16_t *)biasPtr;
  orgbiasptr_float = (float32_tidl *)biasPtr;

#if (BIAS_BITDEPTH_32BITS == 0)
  if (tidlLayer->weightsElementSizeInBits <= 8)
  {
    if (algLayer->layerParams.convParams.biasParamSize >= (params->numOutChannels * (int32_t)sizeof(int16_t)))
    {
      biasptr = (void *)algLayer->layerParams.convParams.biasParamMem;
    }
    else
    {
      tidl_printf(0, "Memory for  biasptr  is not sufficient exiting...\n    ");
      status = IALG_EFAIL;
    }
    if (status == IALG_EOK)
    {
      if (params->enableBias == 1)
      {
        TIDL_conv2dBiasSplit(orgbiasptr, (int16_t *)biasptr, &params->biasB, params->numOutChannels, buffParams->inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16, 0xFF, TIDL_getDatElementSign(buffParams->inElementType));
        for (j = 0; j < params->numOutChannels; j++)
        {
          ((int16_t *)biasptr)[j] = ((int16_t *)biasptr)[j] * params->biasB;
        }
      }
      else
      {
        params->biasB = 0;
        for (j = 0; j < params->numOutChannels; j++)
        {
          ((int16_t *)biasptr)[j] = 0;
        }
      }
    }
  }
  else
#endif
  {
    if (algLayer->layerParams.convParams.biasParamSize >= (params->numOutChannels * (int32_t)sizeof(int32_t)))
    {
      biasptr = (void *)algLayer->layerParams.convParams.biasParamMem;
    }
    else
    {
      tidl_printf(0, "Memory for  biasptr  is not sufficient exiting...\n    ");
      status = IALG_EFAIL;
    }
    if (status == IALG_EOK)
    {
      if (params->enableBias == 1)
      {
        TIDL_conv2dBiasSplit(orgbiasptr, (int32_t *)(int16_t *)biasptr, &params->biasB, params->numOutChannels, buffParams->inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32, 0xFFFF, TIDL_getDatElementSign(buffParams->inElementType));
        for (j = 0; j < params->numOutChannels; j++)
        {
          ((int32_t *)biasptr)[j] = ((int32_t *)biasptr)[j] * params->biasB;
        }
      }
      else
      {
        params->biasB = 0;
        for (j = 0; j < params->numOutChannels; j++)
        {
          ((int32_t *)biasptr)[j] = 0;
        }
      }
    }
  }
  if (status == IALG_EOK)
  {
    // inDataOffset = (inDataParams->padH - params->padH)*inPitch + (inDataParams->padW);
    inDataOffset = params->padW;
    outDataOffset = 0;
    if (tidlLayer->weightsElementSizeInBits <= 8)
    {
#if BIAS_BITDEPTH_32BITS
      status = TIDL_refDeconv2dBitDepth(inPtr, refPtr, ((int8_t *)weightPtr), ((int32_t *)biasptr), ((int32_t *)accPtr), createParams, layerIdx, params, buffParams, inElementType, inDataOffset, outDataOffset);
#else
      status = TIDL_refDeconv2dBitDepth(inPtr, refPtr, ((int8_t *)weightPtr), ((int16_t *)biasptr), ((int32_t *)accPtr), createParams, layerIdx, params, buffParams, inElementType, inDataOffset, outDataOffset);
#endif
    }
    else if (tidlLayer->weightsElementSizeInBits <= 16)
    {
      status = TIDL_refDeconv2dBitDepth(inPtr, refPtr, ((int16_t *)weightPtr), ((int32_t *)biasptr), ((int64_t *)accPtr), createParams, layerIdx, params, buffParams, inElementType, inDataOffset, outDataOffset);
    }
    else if (tidlLayer->weightsElementSizeInBits == 32)
    {
      status = TIDL_refDeconv2dBitDepth(inPtr, refPtr, ((float32_tidl *)weightPtr), orgbiasptr_float, ((float32_tidl *)accPtr), createParams, layerIdx, params, buffParams, inElementType, inDataOffset, outDataOffset);
    }
    else
    {
      status = IALG_EFAIL;
    }
    if (status == IALG_EOK)
    {
      TIDL_L1DandL2CacheWbInv();
    }
  }
  return status;
}
