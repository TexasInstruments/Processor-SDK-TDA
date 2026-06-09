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
@file    tidl_roiPooling_ref.c
@brief   This file contains basic functions required by roiPooling Layer
@version 0.1 (Oct 2016) : Initial version [ADK]
@version 0.5 (Jan 2017) : Cleaned up [ADK]
----------------------------------------------------------------------------
*/
#if defined TIDL_COVERAGE_DEAD_CODE
#include "tidl_alg_int.h"
#include "tidl_roiPooling.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include "tidl_resize.h"
#include "tidl_roiPooling_ref.h"

/**
 * @brief This function checks if the input is power of 2
 *
 * @param n  : Input to the function
 * @return int32_t : returns ture or false
 */
static inline int32_t isPowerOf2(uint32_t n)
{
  if ((n == 1))
  {
    return 1;
  }

  if (n == 0)
  {
    return 0;
  }
  if ((n % 2) == 0)
  {
    return isPowerOf2(n / 2);
  }
  else
  {
    return 0;
  }
}

/**
 * @brief  This function does the resize operation
 *
 * @param pIn : Pointer to the inout data
 * @param pOut : Pointer to the output data
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : parameters of the resize layer
 * @param inDataParams : parameters of the input data buffer
 */
template<class Tin> void TIDL_refResize_1(
    Tin *pIn,
    Tin *pOut,
    sTIDL_Layer_t *tidlLayer,
    sTIDL_ResizeLayerParams_t *params,
    sTIDL_DataParams_t *inDataParams)
{
#if 1
  uint32_t numInChannels = (uint16_t)inDataParams->dimValues[TIDL_DIM_NUMCH];
  uint32_t inWidth = (uint16_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
  uint32_t inHeight = (uint16_t)inDataParams->dimValues[TIDL_DIM_HEIGHT];
  uint32_t inPitch = (uint16_t)inDataParams->pitch[TIDL_LINE_PITCH];
  uint32_t inChPitch = (uint32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
  uint32_t outPitch = (uint16_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  uint32_t outChPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  uint32_t outWidth = (uint16_t)tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
  uint32_t outHeight = (uint16_t)tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];

  float wRatio;
  float hRatio;

  int32_t widthResizeRatio = params->resizeRatio[TIDL_DIM_WIDTH];
  int32_t heightResizeRatio = params->resizeRatio[TIDL_DIM_HEIGHT];

  if (params->resizeRatio[TIDL_DIM_WIDTH] > 0)
  {
    wRatio = 1 / params->resizeRatio[TIDL_DIM_WIDTH];
  }
  else
  {
    wRatio = 0;
  }
  if (params->resizeRatio[TIDL_DIM_HEIGHT] > 0)
  {
    hRatio = 1 / params->resizeRatio[TIDL_DIM_HEIGHT];
  }
  else
  {
    hRatio = 0;
  }

#if 0
  if (params->mode == TIDL_ResizeNearest)
  {
    int i = 0;
    int j = 0;
    int k = 0;
    float hLoc, wLoc;
    int hIdx, wIdx;
    for (i = 0; i < outHeight; i++)
    {
      hLoc = hRatio * (i + 0.5) - 0.5;
      hLoc = (hLoc < 0) ? 0 : hLoc;
      hIdx = hLoc+0.5;
      hIdx = (hIdx < outHeight) ? hIdx : (outHeight-1);
      for (j = 0; j < outWidth; j++)
      {
        wLoc = wRatio * (j + 0.5) - 0.5;
        wLoc = (wLoc < 0) ? 0 : wLoc;
        wIdx = wLoc+0.5;
        wIdx = (wIdx < outWidth) ? wIdx : outWidth-1;
        int32_t inputOffset = inPitch  * hIdx + wIdx;
        int32_t outputOffset = outPitch * i + j;
        for (k = 0; k < numInChannels; k++)
        {
          *(pOut + k*outChPitch + outputOffset) = *(pIn + k*inChPitch + inputOffset);
        }
      }
    }
  }
#endif
  if (params->mode == TIDL_ResizeBilinear)
  {
    int i = 0;
    int j = 0;
    int k = 0;
    float hLoc, wLoc;
    int hIdx, wIdx, hNext, wNext;
    float w00, w01, w10, w11;
    for (i = 0; i < outHeight; i++)
    {
      hLoc = (hRatio * (i + 0.5)) - 0.5;
      hLoc = (hLoc < 0) ? 0 : hLoc;
      hIdx = hLoc;
      hNext = (hIdx < (inHeight - 1)) ? 1 : 0;
      w11 = hLoc - hIdx;
      w10 = 1 - w11;
      for (j = 0; j < outWidth; j++)
      {
        wLoc = (wRatio * (j + 0.5)) - 0.5;
        wLoc = (wLoc < 0) ? 0 : wLoc;
        wIdx = wLoc;
        w01 = wLoc - wIdx;
        w00 = 1 - w01;
        int32_t inputOffset = (inPitch * hIdx) + wIdx;
        int32_t outputOffset = (outPitch * (i)) + (j);
        wNext = (wIdx < (inWidth - 1)) ? 1 : 0;

        for (k = 0; k < numInChannels; k++)
        {
          Tin i00 = *(pIn + (k * inChPitch) + inputOffset);
          Tin i01 = *(pIn + (k * inChPitch) + inputOffset + wNext);
          Tin i10 = *(pIn + (k * inChPitch) + inputOffset + (hNext * inPitch));
          Tin i11 = *(pIn + (k * inChPitch) + inputOffset + (hNext * inPitch) + wNext);

          Tin inter1, inter2;

          uint8_t w00Int, w01Int;
          uint8_t w10Int, w11Int;

          if (isPowerOf2(widthResizeRatio) && isPowerOf2(heightResizeRatio))
          {
            w00Int = w00 * (1 << widthResizeRatio);
            w01Int = (1 << widthResizeRatio) - w00Int;
            w10Int = w10 * (1 << heightResizeRatio);
            w11Int = (1 << heightResizeRatio) - w10Int;
            inter1 = ((i00 * w00Int) + (i01 * w01Int)) >> widthResizeRatio;
            inter2 = ((i10 * w00Int) + (i11 * w01Int)) >> widthResizeRatio;
            *(pOut + (k * outChPitch) + outputOffset) = ((w10Int * inter1) + (w11Int * inter2)) >> heightResizeRatio;
          }
          else
          {
            *(pOut + (k * outChPitch) + outputOffset) = ((w10 * ((i00 * w00) + (i01 * w01))) + (w11 * ((i10 * w00) + (i11 * w01))));
          }
        }
      }
    }
  }
  else
  {
    tidl_printf(0, "params->modeis  Not suported !!!\n ");
    assert(0);
  }
#endif
}

/**
 * @brief  This function clips the input between zero and offset
 *
 * @param input : Input value for clippling
 * @param max : Max value for clip opreation
 * @return int32_t : returns clipped output
 */
int32_t TIDL_roiPoolingClipoffset(int32_t input, int32_t max)
{
  int32_t output;
  output = (input >= max) ? (max - 1) : input;
  output = (output < 0) ? 0 : output;
  return output;
}

/**
 * @brief roiPooling layer reference function
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param boxInDataParams : parameters of the boxData buffer
 * @param featInDataParams : parameters of the featData buffer
 * @param outDataParams : parameters of the outputData buffer
 * @param detectionOutputPtr : pointer to the detection output buffer
 * @param inFeatPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_roiPoolingRefProcess(
    TIDL_Handle intAlgHandle,
    sTIDL_AlgLayer_t *algLayer,
    sTIDL_Layer_t *tidlLayer,
    sTIDL_DataParams_t *boxInDataParams,
    sTIDL_DataParams_t *featInDataParams,
    sTIDL_DataParams_t *outDataParams,
    uint8_t *detectionOutputPtr,
    uint8_t *inFeatPtr,
    uint8_t *outPtr)
{
  int32_t boxIdx;
  int32_t numBoxes;
  sTIDL_Layer_t tidlResizeLayer = *tidlLayer;
  sTIDL_ResizeLayerParams_t resizeParams;
  sTIDL_DataParams_t resizeInDataParams;
  int32_t inputWidth;
  int32_t inputHeight;
  int32_t numInChannels;
  uint8_t *resizeInPtr;
  uint8_t *resizeOutPtr;
  int32_t boxTopX, boxTopY, boxBottomX, boxBottomY;
  int32_t boxWidth, boxHeight;
  TIDL_ODLayerHeaderInfo *objDetHeader = (TIDL_ODLayerHeaderInfo *)detectionOutputPtr;
  TIDL_ODLayerObjInfo *bboxPtr = (TIDL_ODLayerObjInfo *)(((char *)detectionOutputPtr) + (int32_t)objDetHeader->objInfoOffset);
  int32_t pitch = objDetHeader->objInfoSize;

  inputWidth = featInDataParams->dimValues[TIDL_DIM_WIDTH];
  inputHeight = featInDataParams->dimValues[TIDL_DIM_HEIGHT];
  numInChannels = featInDataParams->dimValues[TIDL_DIM_NUMCH];

  numBoxes = objDetHeader->numDetObjects;
  sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(intAlgHandle->createParams->net, tidlLayer->inData[0]);

  for (boxIdx = 0; boxIdx < numBoxes; boxIdx++)
  {
    boxTopX = TIDL_roiPoolingClipoffset((bboxPtr->xmin * inputWidth), inputWidth);
    boxTopY = TIDL_roiPoolingClipoffset((bboxPtr->ymin * inputHeight), inputHeight);

    boxBottomX = TIDL_roiPoolingClipoffset((bboxPtr->xmax * inputWidth), inputWidth);
    boxBottomY = TIDL_roiPoolingClipoffset((bboxPtr->ymax * inputHeight), inputHeight);

    boxWidth = (((boxBottomX - boxTopX) == 0) ? 1 : (boxBottomX - boxTopX));
    boxHeight = (((boxBottomY - boxTopY) == 0) ? 1 : (boxBottomY - boxTopY));

    resizeParams.mode = TIDL_ResizeBilinear;
    resizeParams.resizeRatio[TIDL_DIM_WIDTH] = (float32_tidl)outDataParams->dimValues[TIDL_DIM_WIDTH] / (float32_tidl)boxWidth;
    resizeParams.resizeRatio[TIDL_DIM_HEIGHT] = (float32_tidl)outDataParams->dimValues[TIDL_DIM_HEIGHT] / (float32_tidl)boxHeight;
    resizeParams.resizeRatio[TIDL_DIM_NUMCH] = 1.0;
    resizeParams.resizeRatio[TIDL_DIM_BATCH] = 1.0;

    inDataParams->dimValues[TIDL_DIM_WIDTH] = boxWidth;
    inDataParams->dimValues[TIDL_DIM_HEIGHT] = boxHeight;
    inDataParams->dimValues[TIDL_DIM_NUMCH] = numInChannels;
    inDataParams->dimValues[TIDL_DIM_BATCH] = 1;

    inDataParams->pitch[TIDL_LINE_PITCH] = featInDataParams->pitch[TIDL_LINE_PITCH];
    inDataParams->pitch[TIDL_CHANNEL_PITCH] = featInDataParams->pitch[TIDL_CHANNEL_PITCH];

    tidlResizeLayer.outData.dimValues[TIDL_DIM_WIDTH] = outDataParams->dimValues[TIDL_DIM_WIDTH];
    tidlResizeLayer.outData.dimValues[TIDL_DIM_HEIGHT] = outDataParams->dimValues[TIDL_DIM_HEIGHT];
    tidlResizeLayer.outData.dimValues[TIDL_DIM_NUMCH] = outDataParams->dimValues[TIDL_DIM_NUMCH];
    tidlResizeLayer.outData.dimValues[TIDL_DIM_BATCH] = 1;

    tidlResizeLayer.outData.pitch[TIDL_LINE_PITCH] = outDataParams->pitch[TIDL_LINE_PITCH];
    tidlResizeLayer.outData.pitch[TIDL_CHANNEL_PITCH] = outDataParams->pitch[TIDL_CHANNEL_PITCH];
    tidlResizeLayer.outData.pitch[TIDL_ROI_PITCH] = outDataParams->pitch[TIDL_ROI_PITCH];

    resizeInDataParams.pitch[TIDL_LINE_PITCH] = inDataParams->pitch[TIDL_LINE_PITCH];
    resizeInDataParams.pitch[TIDL_CHANNEL_PITCH] = inDataParams->pitch[TIDL_CHANNEL_PITCH];

    resizeInPtr = inFeatPtr + (boxTopX + (boxTopY * resizeInDataParams.pitch[TIDL_LINE_PITCH]));
    resizeOutPtr = outPtr + (boxIdx * outDataParams->dimValues[TIDL_DIM_NUMCH] * outDataParams->pitch[TIDL_CHANNEL_PITCH]);
    TIDL_refResize_1(resizeInPtr,
                     resizeOutPtr,
                     &tidlResizeLayer,
                     &resizeParams,
                     &resizeInDataParams);
    bboxPtr = (TIDL_ODLayerObjInfo *)(((uint8_t *)bboxPtr) + pitch);
  }
  TIDL_L1DandL2CacheWbInv();
  return IALG_EOK;
}

#endif
