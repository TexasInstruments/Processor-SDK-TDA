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
 @file    tidl_odOutputReformat.c
 @brief   This file defines private functions for OD output reformat layer.
 @version 0.1 (Jan 2021) : Intial Layer
 @version 0.2 (June 2021) : Common output reformatting layer for tflite and onnx runtimes
 ----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_odOutputReformat.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include "tidl_alg_utils_ref.h"

/* This function is used to avoid empty file compilation error and unused function warning */
void TIDL_odOutputReformat_dummyFunc(void)
{
  /* This function is not used in any other place */
  return;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void TIDL_odOutputReformat.* <function end>
<justification start>
Rationale - NOT_IN_SCOPE: This condition implements the OD output reformat layer which is common for both tflite and onnx runtimes
and cannot be exercised from tidl-runtime(OSRT_SCOPE). Hence this file is justified not to include in tidl-runtime build.
Effect on this UNIT - Code prevents the software executing code which is not in scope and maintain system predictability and integrity.
<justification end> */
/**
 * @brief Implementation of OD output reformat layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param pIn : Pointer to input memory
 * @param pOut : Pointer to output memory
 * @param inPtrOffset : offset input buffer pointer
 * @param outPtrOffset : offset output buffer pointer
 * @param outWidth : width of the output buffer
 * @param outHeight : height of the output buffer
 * @param outLinePitch : pitch of the output buffer
 * @param odOutputType : type of OD output (boxes, labels, scores, etc)
 */
template<class Tin, class Tout> void TIDL_odOutputReformat(
    const Tin *pIn,
    Tout *pOut,
    int32_t inPtrOffset,
    int32_t outPtrOffset,
    int32_t outWidth,
    int32_t outHeight,
    int32_t outLinePitch,
    int32_t odOutputType,
    int32_t inWidthOdNetwork,
    int32_t inHeightOdNetwork,
    int32_t numKeypoints,
    int32_t keypointConfidence,
    int32_t subCodeType,
    int32_t codeType)
{
  int32_t i2, i3, i4, i5;
  int32_t inOffset, outOffset;
  int32_t dimSize;
  int32_t numOutElementPer_2dBox = (sizeof(TIDL_ODLayerObjInfo) - (sizeof(TIDL_ODLayerKeyPoint) * (uint32_t)TIDL_OD_MAX_KEY_POINTS)) / sizeof(float32_tidl);
  int32_t numOutElementPer_3dBox = sizeof(TIDL_3DODLayerObjInfo) / sizeof(float32_tidl);
  if (codeType == 8) /* for pointPillars 3d OD, output does not includes pitch and roll angles*/
  {
    numOutElementPer_3dBox = ((int32_t)sizeof(TIDL_3DODLayerObjInfo) / (int32_t)sizeof(float32_tidl)) - 2;
  }
  int32_t numOutElementPerKeypoint = ((sizeof(TIDL_ODLayerKeyPoint) / sizeof(float32_tidl)) - 1U + (uint32_t)keypointConfidence);
  int32_t numInElementPerKeypoint = sizeof(TIDL_ODLayerKeyPoint) / sizeof(float32_tidl);
  int32_t numSubCodeElements;
  if (subCodeType == (int32_t)TIDL_ObjectPose)
  {
    numSubCodeElements = sizeof(TIDL_ODLayerObjectPose) / sizeof(float32_tidl);
  }
  else
  {
    numSubCodeElements = 0;
  }

  int32_t numValidDetections = pIn[inPtrOffset];

  int32_t baseOffset = ((int32_t)(sizeof(TIDL_ODLayerHeaderInfo) / sizeof(float32_tidl))) + inPtrOffset; // actual data - number of valid detections, class, score and box start from 5th index

  if (odOutputType == (int32_t)TIDL_NumDetectionsTflite)
  {
    pOut[outPtrOffset] = pIn[inPtrOffset]; // first index of detection layer output gives numDetections
  }
  else if (odOutputType == (int32_t)TIDL_DetectionClassesTflite)
  {
    for (i2 = 0; i2 < outWidth; i2++)
    {
      inOffset = baseOffset + (i2 * numOutElementPer_2dBox) + 1;
      if (i2 < numValidDetections)
      {
        pOut[outPtrOffset + i2] = pIn[inOffset] - 1; // tflite runtime expects output without background class, we take care of that in python post processing
      }
      else
      {
        pOut[outPtrOffset + i2] = -1;
      }
    }
  }
  else if ((odOutputType == (int32_t)TIDL_DetectionScoresTflite) || (odOutputType == (int32_t)TIDL_DetectionScoresSsd))
  {
    for (i2 = 0; i2 < outWidth; i2++)
    {
      inOffset = baseOffset + (i2 * numOutElementPer_2dBox) + 2;
      if (i2 < numValidDetections)
      {
        pOut[outPtrOffset + i2] = pIn[inOffset];
      }
      else
      {
        pOut[outPtrOffset + i2] = 0;
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionBoxesTflite)
  {
    for (i2 = 0; i2 < outHeight; i2++) // outHeight = max_detection_classes
    {
      for (i3 = 0; i3 < outWidth; i3++) // outWidth = 4
      {
        inOffset = baseOffset + (i2 * numOutElementPer_2dBox) + 3 + i3;
        // in the tflite runtime expected bbox, x and y are interchanged compared to TIDL --- we keep dims compatible to tflite runtime
        if (i3 == 0)
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 1;
        }
        else if (i3 == 1)
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 0;
        }
        else if (i3 == 2)
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 3;
        }
        else if (i3 == 3)
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 2;
        }
        else
        {
          /*DO Nothing*/
        }

        if (i2 < numValidDetections)
        {
          pOut[outOffset] = pIn[inOffset];
        }
        else
        {
          pOut[outOffset] = -1;
        }
      }
    }
  }
  else if ((odOutputType == (int32_t)TIDL_DetectionClassesSsd) || (odOutputType == (int32_t)TIDL_DetectionClassesMmdetection))
  {
    for (i2 = 0; i2 < outWidth; i2++)
    {
      inOffset = baseOffset + (i2 * numOutElementPer_2dBox) + 1;
      if (i2 < numValidDetections)
      {
        pOut[outPtrOffset + i2] = pIn[inOffset];
      }
      else
      {
        pOut[outPtrOffset + i2] = -1;
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionBoxesSsd)
  {
    for (i2 = 0; i2 < outHeight; i2++) // outHeight = max_detection_classes
    {
      for (i3 = 0; i3 < outWidth; i3++) // outWidth = 4
      {
        inOffset = baseOffset + (i2 * numOutElementPer_2dBox) + 3 + i3;
        outOffset = outPtrOffset + (i2 * outLinePitch) + i3;
        if (i2 < numValidDetections)
        {
          pOut[outOffset] = pIn[inOffset];
        }
        else
        {
          pOut[outOffset] = -1;
        }
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionBoxesAndScoresMmdetection)
  {
    for (i2 = 0; i2 < outHeight; i2++) // outHeight = max_detection_classes
    {
      for (i3 = 0; i3 < 4; i3++) // 4 coordinates for boxes
      {
        inOffset = baseOffset + (i2 * numOutElementPer_2dBox) + 3 + i3;
        outOffset = outPtrOffset + (i2 * outLinePitch) + i3;
        dimSize = ((i3 % 2) == 0) ? inWidthOdNetwork : inHeightOdNetwork; // xmin * source_img.width, ymin * source_img.height, xmax * source_img.width, ymax * source_img.height
        if (i2 < numValidDetections)
        {
          pOut[outOffset] = pIn[inOffset] * dimSize; // Need to multiply coordinate ratio with actual width/height
        }
        else
        {
          pOut[outOffset] = -1;
        }
      }
      if (i2 < numValidDetections)
      {
        pOut[outPtrOffset + (i2 * outLinePitch) + 4] = pIn[baseOffset + (i2 * numOutElementPer_2dBox) + 2]; // Write score as the 5th output index
      }
      else
      {
        pOut[outPtrOffset + (i2 * outLinePitch) + 4] = 0;
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionBoxesScoresLabelsYoloV5)
  {
    for (i2 = 0; i2 < outHeight; i2++) // outHeight = max_detection_classes
    {
      for (i3 = 0; i3 < 4; i3++) // 4 coordinates for boxes
      {
        inOffset = baseOffset + (i2 * (numOutElementPer_2dBox + (numKeypoints * 3) + numSubCodeElements)) + 3 + i3;
        outOffset = outPtrOffset + (i2 * outLinePitch) + i3;
        dimSize = ((i3 % 2) == 0) ? inWidthOdNetwork : inHeightOdNetwork; // xmin * source_img.width, ymin * source_img.height, xmax * source_img.width, ymax * source_img.height
        if (i2 < numValidDetections)
        {
          pOut[outOffset] = pIn[inOffset] * dimSize; // Need to multiply coordinate ratio with actual width/height
        }
        else
        {
          pOut[outOffset] = -1;
        }
      }
      if (i2 < numValidDetections)
      {
        pOut[outPtrOffset + (i2 * outLinePitch) + 4] = pIn[baseOffset + (i2 * (numOutElementPer_2dBox + (numKeypoints * numInElementPerKeypoint) + numSubCodeElements)) + 2]; // Write score as the 5th output index
        pOut[outPtrOffset + (i2 * outLinePitch) + 5] = pIn[baseOffset + (i2 * (numOutElementPer_2dBox + (numKeypoints * numInElementPerKeypoint) + numSubCodeElements)) + 1]; // Write class as the 6th output index
      }
      else
      {
        pOut[outPtrOffset + (i2 * outLinePitch) + 4] = 0;
        pOut[outPtrOffset + (i2 * outLinePitch) + 5] = -1;
      }
      for (i4 = 0; i4 < numKeypoints; i4++) // Write keypoint locations(X,Y) and confidence if available based on the flag keypointConfidence.
      {
        if (i2 < numValidDetections)
        {
          pOut[outPtrOffset + (i2 * outLinePitch) + (i4 * numOutElementPerKeypoint) + 6] = pIn[baseOffset + (i2 * (numOutElementPer_2dBox + (numKeypoints * numInElementPerKeypoint) + numSubCodeElements)) + ((i4 * numInElementPerKeypoint) + 7)] * inWidthOdNetwork;
          pOut[outPtrOffset + (i2 * outLinePitch) + (i4 * numOutElementPerKeypoint) + 7] = pIn[baseOffset + (i2 * (numOutElementPer_2dBox + (numKeypoints * numInElementPerKeypoint) + numSubCodeElements)) + (i4 * numInElementPerKeypoint) + 8] * inHeightOdNetwork;
          if (keypointConfidence != 0)
          {
            pOut[outPtrOffset + (i2 * outLinePitch) + (i4 * (2 + keypointConfidence)) + 8] = pIn[baseOffset + (i2 * (numOutElementPer_2dBox + (numKeypoints * numInElementPerKeypoint) + numSubCodeElements)) + (i4 * numInElementPerKeypoint) + 9];
          }
        }
        else
        {
          pOut[outPtrOffset + (i2 * outLinePitch) + 6 + (i4 * numOutElementPerKeypoint)] = -1;
          pOut[outPtrOffset + (i2 * outLinePitch) + 7 + (i4 * numOutElementPerKeypoint)] = -1;
          pOut[outPtrOffset + (i2 * outLinePitch) + 8 + (i4 * numOutElementPerKeypoint)] = -1;
        }
      }
      if (subCodeType == (int32_t)TIDL_ObjectPose)
      {
        for (i5 = 0; i5 < 9; i5++) // There are nine parameters for object pose. Six for rotation and three for translation
        {
          if (i2 < numValidDetections)
          {
            pOut[outPtrOffset + (i2 * outLinePitch) + 6 + i5] = pIn[baseOffset + (i2 * (numOutElementPer_2dBox + (numKeypoints * numInElementPerKeypoint) + numSubCodeElements)) + i5 + (numKeypoints * numInElementPerKeypoint) + 7];
          }
          else
          {
            pOut[outPtrOffset + (i2 * outLinePitch) + 6 + i5] = -1;
          }
        }
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionBoxesScoresLabelsTfliteEfficientdet)
  {
    for (i2 = 0; i2 < outHeight; i2++) // outHeight = max_detection_classes
    {
      pOut[outPtrOffset + (i2 * outLinePitch)] = 0;
      for (i3 = 0; i3 < 4; i3++) // 4 coordinates for boxes
      {
        inOffset = baseOffset + (i2 * numOutElementPer_2dBox) + 3 + i3;
        // in the tflite runtime expected bbox, x and y are interchanged compared to TIDL --- we keep dims compatible to tflite runtime
        if (i3 == 0) // xmin * source_img.width
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 1 + 1;
          dimSize = inWidthOdNetwork;
        }
        else if (i3 == 1) // ymin * source_img.height
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 1 + 0;
          dimSize = inHeightOdNetwork;
        }
        else if (i3 == 2) // xmax * source_img.width
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 1 + 3;
          dimSize = inWidthOdNetwork;
        }
        else if (i3 == 3) // ymax * source_img.height
        {
          outOffset = outPtrOffset + (i2 * outLinePitch) + 1 + 2;
          dimSize = inHeightOdNetwork;
        }
        else
        {
          /*DO Nothing*/
        }
        if (i2 < numValidDetections)
        {
          pOut[outOffset] = pIn[inOffset] * dimSize;
        }
        else
        {
          pOut[outOffset] = -1;
        }
      }
      if (i2 < numValidDetections)
      {
        pOut[outPtrOffset + (i2 * outLinePitch) + 5] = pIn[baseOffset + (i2 * numOutElementPer_2dBox) + 2];     // Write score as the 5th output index
        pOut[outPtrOffset + (i2 * outLinePitch) + 6] = pIn[baseOffset + (i2 * numOutElementPer_2dBox) + 1] + 1; // Write class as the 6th output index - retinaNet doesn't count background class, but efficientdet lite needs it
      }
      else
      {
        pOut[outPtrOffset + (i2 * outLinePitch) + 5] = 0;
        pOut[outPtrOffset + (i2 * outLinePitch) + 6] = -1;
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_Detection3dOd)
  {
    for (i2 = 0; i2 < outHeight; i2++) // outHeight = keep_top_k
    {
      for (i3 = 0; i3 < outWidth; i3++) // outWidth = 9
      {
        inOffset = baseOffset + (i2 * numOutElementPer_3dBox) + i3 + 1; // detection output layer gives 10 dim output, remove detection no.(first index)
        outOffset = outPtrOffset + (i2 * outLinePitch) + i3;
        if (i2 < numValidDetections)
        {
          pOut[outOffset] = pIn[inOffset];
        }
        else
        {
          pOut[outOffset] = -1;
        }
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionBoxes3dOd)
  {
    for (i2 = 0; i2 < outHeight; i2++) // outHeight = keep_top_k
    {
      for (i3 = 0; i3 < outWidth; i3++) // outWidth = 9
      {
        inOffset = baseOffset + (i2 * numOutElementPer_3dBox) + i3 + 3; // detection output layer gives 12 dim output, remove 3 detection parameters (ObjId, label, score)
        outOffset = outPtrOffset + (i2 * outLinePitch) + i3;
        if (i2 < numValidDetections)
        {
          pOut[outOffset] = pIn[inOffset];
        }
        else
        {
          pOut[outOffset] = -1;
        }
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionClasses3dOd)
  {
    for (i2 = 0; i2 < outWidth; i2++) // outWidth = keep_top_k
    {
      inOffset = baseOffset + (i2 * numOutElementPer_3dBox) + 1; // detection output layer gives 12 dim output, remove 1 detection parameters (ObjId)
      outOffset = outPtrOffset + i2;
      if (i2 < numValidDetections)
      {
        pOut[outOffset] = pIn[inOffset];
      }
      else
      {
        pOut[outOffset] = -1;
      }
    }
  }
  else if (odOutputType == (int32_t)TIDL_DetectionScores3dOd)
  {
    for (i2 = 0; i2 < outWidth; i2++) // outWidth = keep_top_k
    {
      inOffset = baseOffset + (i2 * numOutElementPer_3dBox) + 2; // detection output layer gives 12 dim output, remove 2 detection parameters (ObjId, label)
      outOffset = outPtrOffset + i2;
      if (i2 < numValidDetections)
      {
        pOut[outOffset] = pIn[inOffset];
      }
      else
      {
        pOut[outOffset] = -1;
      }
    }
  }
  else
  {
    /*DO Nothing*/
  }
  TIDL_L1DandL2CacheWbInv();
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_odOutputReformatAlloc.* <function end>
<justification start>
Rationale - NOT_IN_SCOPE: This condition implements the OD output reformat layer which is common for both tflite and onnx runtimes
and cannot be exercised from tidl-runtime(OSRT_SCOPE). Hence this file is justified not to include in tidl-runtime build.
Effect on this UNIT - Code prevents the software executing code which is not in scope and maintain system predictability and integrity.
<justification end> */
/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_OdOutputReformatLayer
 @fn         TIDL_odOutputReformatAlloc
 @brief      OD Output Reformatting Layer
 ----------------------------------------------------------------------------
*/
int32_t TIDL_OdOutputReformatAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                                  const TIDL_NetworkCommonParams *commonParams,
                                  int32_t layerIdx,
                                  int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  const TIDL_CreateParams *params = commonParams->createParams;
  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
#ifdef HOST_EMULATION
    if (((uint32_t)params->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
    {
      outDataSize = TIDL_findFreeOutBuff(params, layerIdx, outDataSize, commonParams->TIDLLayersBufPtr);
    }
#endif
    commonParams->TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }

  if ( !(TIDL_isOutDataBuff(params->net,
                          params->net->TIDLLayers[layerIdx].outData.dataId,
                          params->currLayersGroupId) == 0
        ) 
        || !(params->net->dataFlowInfo == NULL)
      )
  {
    outDataSize = 0;
  }
 
  memorySize[TIDL_LAYER_MEMORY_SCRATCH] += 0;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] += ((uint32_t)outDataSize);
  memorySize[TIDL_LAYER_MEMORY_OUTPUT] += 0;

  return status;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_odOutputReformatInit.* <function end>
<justification start>
Rationale - NOT_IN_SCOPE: This condition implements the OD output reformat layer which is common for both tflite and onnx runtimes
and cannot be exercised from tidl-runtime(OSRT_SCOPE). Hence this file is justified not to include in tidl-runtime build.
Effect on this UNIT - Code prevents the software executing code which is not in scope and maintain system predictability and integrity.
<justification end> */
/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_OdOutputReformatLayer
 @fn         TIDL_odOutputReformatInit
 @brief      OD Output Reformatting Layer
 ----------------------------------------------------------------------------
*/
int32_t TIDL_OdOutputReformatInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                                 const TIDL_NetworkCommonParams *commonParams,
                                 sTIDL_AlgLayer_t *algLayer,
                                 int32_t layerIdx,
                                 uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                 int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                 void **outPtr)
{
  int32_t status = IALG_EOK;
  // TODO
  const TIDL_CreateParams *params = commonParams->createParams;

  int32_t outDataSize = TIDL_updateDataParamsPitch(&params->net->TIDLLayers[layerIdx].outData);

  if (params->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
#ifdef HOST_EMULATION
    if (((uint32_t)params->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) != 0U)
    {
      outDataSize = TIDL_findFreeOutBuff(params, layerIdx, outDataSize, commonParams->TIDLLayersBufPtr);
    }
#endif
    commonParams->TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }

  if (status == IALG_EOK)
  {
    if ((TIDL_isOutDataBuff(params->net,
                          params->net->TIDLLayers[layerIdx].outData.dataId,
                          params->currLayersGroupId) == 1)
#ifdef HOST_EMULATION
        || (params->net->dataFlowInfo != NULL)
#endif
    )
    {
      *outPtr = NULL;
    }
#ifdef HOST_EMULATION
    else
    {
      /* outDataSize is 0 means, not allocate the new memory for output buffer */
      if (outDataSize == 0)
      {
        *outPtr = NULL;
      }
      else
      {
        TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_OUTPUT],
                        0,
                        outDataSize,
                        128,
                        outPtr);
      }
    }
#endif

    algLayer->scratchSize = 0;
    algLayer->metaData.totalOps =
        (params->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH] *
         params->net->TIDLLayers[layerIdx].layerParams.eltWiseParams.numInData);
    algLayer->metaData.actualOps = algLayer->metaData.totalOps;
  }

  return status;
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_odOutputReformatProcess.* <function end>
<justification start>
Rationale - NOT_IN_SCOPE: This condition implements the OD output reformat layer which is common for both tflite and onnx runtimes
and cannot be exercised from tidl-runtime(OSRT_SCOPE). Hence this file is justified not to include in tidl-runtime build.
Effect on this UNIT - Code prevents the software executing code which is not in scope and maintain system predictability and integrity.
<justification end> */
/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_OdOutputReformatLayer
 @fn         TIDL_odOutputReformatProcess
 @brief      OD Output Reformatting Layer
 ----------------------------------------------------------------------------
*/
int32_t TIDL_OdOutputReformatProcess(TIDL_NetworkCommonParams *commonParams,
                                    sTIDL_AlgLayer_t *algLayer,
                                    sTIDL_Layer_t *tidlLayer,
                                    void *inPtrs[],
                                    void *outPtrs[],
                                    int32_t layerIdx)
{
  int32_t status = IALG_EOK;

  int32_t outWidth = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];
  int32_t outHeight = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];

  int32_t outPitch = (int32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];

  int32_t numKeypoints = tidlLayer->layerParams.detectOutParams.numKeypoints;
  int32_t keypointConfidence = tidlLayer->layerParams.detectOutParams.keypointConfidence;
  int32_t subCodeType = tidlLayer->layerParams.detectOutParams.subCodeType;
  int32_t codeType = tidlLayer->layerParams.detectOutParams.codeType;
  sTIDL_odOutputReformatLayerParams_t *params = &tidlLayer->layerParams.odOutputReformatLayerParams;
  int32_t layerType = params->layerType;
  int32_t inWidthOdNetwork = params->inWidthOdNetwork;
  int32_t inHeightOdNetwork = params->inHeightOdNetwork;

  int32_t inPtrOffset = 0;
  int32_t outPtrOffset = 0;

  if (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
  {
    int32_t layerIdx;
    layerIdx = algLayer->layerIdx;

    TIDL_Obj intAlgObj;
    TIDL_CreateParams createParams;
    (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
    intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

    TIDL_UpdateScaleFactors(&intAlgObj, layerIdx, 0, 0, 1);
  }

  TIDL_odOutputReformat((float32_tidl *)inPtrs[0], (float32_tidl *)outPtrs[0], inPtrOffset, outPtrOffset, outWidth, outHeight, outPitch,
                        layerType, inWidthOdNetwork, inHeightOdNetwork, numKeypoints, keypointConfidence, subCodeType, codeType);
  return status;
}
