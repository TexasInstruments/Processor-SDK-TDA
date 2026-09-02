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
******************************************************************************
*  @file     tidl_detectionOutput_score.c
*
*  @brief    File to have natural C/C++ implementation of DetectionOutputScore
*
*  @version  0.1 - Jan 2020 : Initial Version
*
*******************************************************************************
*/
#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_detectionOutput.h"
#include "tidl_detectionOutput.h"
#include <math.h>
template int32_t TIDL_findValidLocation_cn<int8_t>(const sTIDL_DetectOutputParams_t *params,
                                                   sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                                   float32_tidl *priorData);

template int32_t TIDL_sparseDetScoreCalc_cn<int8_t>(const sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams);

template int32_t TIDL_findValidLocation_cn<int16_t>(const sTIDL_DetectOutputParams_t *params,
                                                    sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                                    float32_tidl *priorData);

template int32_t TIDL_sparseDetScoreCalc_cn<int16_t>(const sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams);

template int32_t TIDL_findValidLocation_cn<float32_tidl>(const sTIDL_DetectOutputParams_t *params,
                                                  sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                                  float32_tidl *priorData);

template int32_t TIDL_sparseDetScoreCalc_cn<float32_tidl>(const sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams);

template int32_t TIDL_findValidLocation_cn<bfloat16_tidl>(const sTIDL_DetectOutputParams_t *params,
                                                   sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                                   float32_tidl *priorData);

template int32_t TIDL_sparseDetScoreCalc_cn<bfloat16_tidl>(const sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams);

/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_detectionOutput
 @fn         TIDL_findValidLocation_cn
 @brief      This function performs softMax on Conf data and returns the
             scores which are more than the threshold

 @param      params : Parameters of the detection output layer
 @param      algDetLyrParams : Layer specific parameters
 @param      priorData : Pointer to priorData buffer
 @remarks    None
 ----------------------------------------------------------------------------
*/
template <class Tconf>
int32_t TIDL_findValidLocation_cn(const sTIDL_DetectOutputParams_t *params,
                                  sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                  float32_tidl *priorData)
{
  int32_t i, i3, i4;
  int32_t numPriors;
  Tconf maxConfObj, minConfObj, bckVal, inVal;
  float32_tidl onebyqFact, zeroPoint;
  float32_tidl upperBoundThSoftMax = (1.0f / params->confThreshold) - 1.0f;
  float32_tidl upperBoundThSigmoid = -logf((1.0f / params->confThreshold) - 1.0f);
  int32_t curIndex;
  int32_t linePitch;
  int32_t chPitch;
  Tconf *curPlaneConf;
  int32_t curClassValidCnt;
  uint32_t curPartIndex;
  uint32_t temp_val = 0;
  int32_t topMLocal = algDetLyrParams->topMAllClasses;
  int32_t totalCnt = 0;

  sTIDL_AnchorBoxParams_t *anchorBox = (sTIDL_AnchorBoxParams_t *)priorData;
  int32_t maxInit = -32768;
  float32_tidl maxFloat = -FLT_MAX;

  if (algDetLyrParams->elementType == TIDL_SignedChar)
  {
    maxInit = -128;
  }
  else if (algDetLyrParams->elementType == TIDL_SignedShort)
  {
    maxInit = -32768;
  }
  else if (algDetLyrParams->elementType == TIDL_BFloat16)
  {
    maxFloat = -(float32_tidl)std::numeric_limits<bfloat16_tidl>::max();
  }
  else // if (algDetLyrParams->elementType == TIDL_SinglePrecFloat)
  {
    maxFloat = -FLT_MAX;
  }
  #ifdef TIDL_COVERAGE_DEAD_CODE
  else
  {
      /*Do Nothing*/
  }
  #endif

  for (i = 0; i < params->numClasses; i++)
  {
    algDetLyrParams->countMList[i] = 0;
  }

  totalCnt = 0;

  if (params->scoreConverter == TIDL_scoreConverterSIGMOID)
  {
    for (i = 0; i < (params->numHeads*params->numSubHeads); i++)
    {
      onebyqFact = algDetLyrParams->inConfdataQList[i];
      zeroPoint = algDetLyrParams->inConfdataZPList[i];
      numPriors = anchorBox[i].numAnchors;
      linePitch = algDetLyrParams->confHeadPitchList[i][TIDL_LINE_PITCH];
      chPitch = algDetLyrParams->confHeadPitchList[i][TIDL_CHANNEL_PITCH];

      for (i3 = 0; i3 < numPriors; i3++) // loop for each anchor
      {
        for (i4 = 0; i4 < params->numClasses; i4++)
        {
          if (params->backgroundLabelId == i4)
          {
            continue;
          }
          curClassValidCnt = algDetLyrParams->countMList[i4];
          curPartIndex = ((uint32_t)i << 28U) | ((uint32_t)i3 << 24U) | ((uint32_t)i4 << 16U);

          if (params->metaArchType != (int32_t)TIDL_metaArchTIDLYolo)
          {
            #if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
            if((params->metaArchType == (int32_t)TIDL_metaArchTIDLCenterPoint) && (params->reserve1 != - 1)){
                int32_t numInCodeElements = 5 + params->numClasses + (algDetLyrParams->numInElementPerKeypoint * params->numKeypoints) + algDetLyrParams->numSubCodeElements;
                curPlaneConf = &((Tconf *)(algDetLyrParams->inConfDataList[i]))[chPitch * ((i3 * numInCodeElements) + i4 + 1)];
                Tconf *curPlaneObjConf = &((Tconf *)(algDetLyrParams->inConfDataList[i]))[chPitch * ((i3 * numInCodeElements) + 0)];
                Tconf objectThr;
                for (int32_t cury = 0; cury < anchorBox[i].headHeight; cury++)
                {
                  for (int32_t curx = 0; curx < anchorBox[i].headWidth; curx++)
                  {
                    objectThr = curPlaneObjConf[(cury * linePitch) + curx]; // objectness threshold at
                    inVal = curPlaneConf[(cury * linePitch) + curx];        // acctual sigmoid score

                    // if any class score is above threshold then that box will be decoded.
                    if (((((float32_tidl)inVal - zeroPoint) * onebyqFact) >= upperBoundThSigmoid) && ((((float32_tidl)objectThr - zeroPoint) * onebyqFact) >= upperBoundThSigmoid))
                    {
                      if (totalCnt < topMLocal)
                      {
                        temp_val = ((cury * linePitch) + curx);
                        curIndex = curPartIndex | temp_val;
                        algDetLyrParams->topMIndices[totalCnt] = curIndex;
                        curClassValidCnt++;
                        totalCnt++;
                      }
                    }
                  }
                }
            }else{
            #endif
              curPlaneConf = &((Tconf *)(algDetLyrParams->inConfDataList[i]))[chPitch * ((i3 * params->numClasses) + i4)];
              for (int32_t cury = 0; cury < anchorBox[i].headHeight; cury++)
              {
                for (int32_t curx = 0; curx < anchorBox[i].headWidth; curx++)
                {
                  inVal = curPlaneConf[(cury * linePitch) + curx];
                  // if any class score is above threshold then that box will be decoded.
                  if ((((float32_tidl)inVal - zeroPoint) * onebyqFact) >= upperBoundThSigmoid)
                  {
                    if (totalCnt < topMLocal)
                    {
                      temp_val = ((cury * linePitch) + curx);
                      curIndex = curPartIndex | temp_val;
                      algDetLyrParams->topMIndices[totalCnt] = curIndex;
                      curClassValidCnt++;
                      totalCnt++;
                    }
                  }
                }
              }
            #if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST  
            }
            #endif
          }
          else
          {
            // Each keypoint is defined by (x,y,conf) or (x,y) based on the flag keypointConfidence. Ech keypoint is defined by either two or three parameters
            int32_t numInCodeElements = 5 + params->numClasses + (algDetLyrParams->numInElementPerKeypoint * params->numKeypoints) + algDetLyrParams->numSubCodeElements;
            curPlaneConf = &((Tconf *)(algDetLyrParams->inConfDataList[i]))[chPitch * ((i3 * numInCodeElements) + i4 + 1)];
            Tconf *curPlaneObjConf = &((Tconf *)(algDetLyrParams->inConfDataList[i]))[chPitch * ((i3 * numInCodeElements) + 0)];
            Tconf objectThr;
            for (int32_t cury = 0; cury < anchorBox[i].headHeight; cury++)
            {
              for (int32_t curx = 0; curx < anchorBox[i].headWidth; curx++)
              {
                objectThr = curPlaneObjConf[(cury * linePitch) + curx]; // objectness threshold at
                inVal = curPlaneConf[(cury * linePitch) + curx];        // acctual sigmoid score

                // if any class score is above threshold then that box will be decoded.
                if (((((float32_tidl)inVal - zeroPoint) * onebyqFact) >= upperBoundThSigmoid) && ((((float32_tidl)objectThr - zeroPoint) * onebyqFact) >= upperBoundThSigmoid))
                {
                  if (totalCnt < topMLocal)
                  {
                    uint32_t temp_curIndex = ((cury * linePitch) + curx);
                    curIndex = curPartIndex | temp_curIndex;
                    algDetLyrParams->topMIndices[totalCnt] = curIndex;
                    curClassValidCnt++;
                    totalCnt++;
                  }
                }
              }
            }
          }

          algDetLyrParams->countMList[i4] = curClassValidCnt;
        }
      }
    }
  }
  if (params->scoreConverter == (int32_t)TIDL_scoreConverterSOFTMAX)
  {
    for (i = 0; i < (params->numHeads*params->numSubHeads); i++)
    {
      int32_t classStride;
      int32_t anchorStride;
      onebyqFact = algDetLyrParams->inConfdataQList[i];
      zeroPoint = algDetLyrParams->inConfdataZPList[i];
      
      numPriors = anchorBox[i].numAnchors;
      linePitch = algDetLyrParams->confHeadPitchList[i][TIDL_LINE_PITCH];
      if(params->dataLayout == 0)
      {
        classStride = 1;
        anchorStride = params->numClasses;
      }
      else /* (params->dataLayout == 1) */
      {
        classStride = numPriors;
        anchorStride = 1;
      }
      for (i3 = 0; i3 < numPriors; i3++) // loop for each anchor
      {
        curPartIndex = ((uint32_t)i << 28U) | ((uint32_t)i3 << 24U);

        for (int32_t cury = 0; cury < anchorBox[i].headHeight; cury++)
        {
          for (int32_t curx = 0; curx < anchorBox[i].headWidth; curx++)
          {
            if (algDetLyrParams->elementType == TIDL_SinglePrecFloat || algDetLyrParams->elementType == TIDL_BFloat16)
            {
              maxConfObj = maxFloat;
              minConfObj = -(maxFloat + 1);
            }
            else
            {
              maxConfObj = maxInit;        // -128
              minConfObj = -(maxInit + 1); //127
            }
            bckVal = ((Tconf *)(algDetLyrParams->inConfDataList[i]))[((cury * linePitch) +
                                                                     curx + (algDetLyrParams->confHeadPitchList[i][TIDL_CHANNEL_PITCH] * (i3*anchorStride)))];
            for (i4 = 0; i4 < params->numClasses; i4++)
            {
              inVal = ((Tconf *)(algDetLyrParams->inConfDataList[i]))[((cury * linePitch) +
                                                                       curx + (algDetLyrParams->confHeadPitchList[i][TIDL_CHANNEL_PITCH] * ((i3*anchorStride) + (i4*classStride))))];
              if (i4 != params->backgroundLabelId)
              {
                maxConfObj = (inVal > maxConfObj) ? inVal : maxConfObj;
                minConfObj = (inVal < minConfObj) ? inVal : minConfObj;
              }
              else
              {
                bckVal = inVal;
              }
            }

            float32_tidl bckVal_f = ((float32_tidl)bckVal - zeroPoint) * onebyqFact;
            float32_tidl max_obj_f = ((float32_tidl)maxConfObj - zeroPoint) * onebyqFact;
            float32_tidl min_obj_f = ((float32_tidl)minConfObj - zeroPoint) * onebyqFact;
            float32_tidl e_bck_minus_max = exp_taylor(bckVal_f - max_obj_f);
            float32_tidl e_min_minus_max = exp_taylor(min_obj_f - max_obj_f);
            float32_tidl score_min = e_bck_minus_max + (((float32_tidl)params->numClasses - 2.0f) * e_min_minus_max);

            if (score_min <= upperBoundThSoftMax)
            {
              if (totalCnt < topMLocal)
              {
                uint32_t temp_curIndex = ((cury * linePitch) + curx);
                curIndex = curPartIndex | temp_curIndex;
                algDetLyrParams->topMIndicesSorted[totalCnt] = curIndex;
                algDetLyrParams->countMList[0]++; // for softmax all classes are valid
                totalCnt++;
              }
            }
          }
        }
      }
    }
  }
  if((params->reserve3 == TIDL_SAFETY_FLAG_OD_FORCE_TOTCNT_INVALID) ||(params->reserve3 == TIDL_SAFETY_FLAG_OD_FORCE_ELEMENTTYPE_16BIT) || (params->reserve3 == TIDL_SAFETY_FLAG_OD_FORCE_TOTCNTFLT_INVALID))
  {
    totalCnt = algDetLyrParams->topMAllClasses + 1;
  }
  if(params->reserve3 == TIDL_SAFETY_FLAG_OD_FORCE_TOTCNTSHORT_INVALID)
  {
    totalCnt = algDetLyrParams->topMAllClasses + 1;
  }
  return totalCnt;
}

/**
 * @brief To find top M Score index from conf buffer
 * 
 @param      params : Parameters of the detection output layer
 @param      algDetLyrParams : Layer specific parameters
 */
template <class Tconf>
int32_t TIDL_sparseDetScoreCalc_cn(const sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams)
{
  int32_t i4;
  int32_t temp;
  Tconf max, inVal;
  float32_tidl numer, denom, onebydenom;
  float32_tidl ftemp, onebyqFact, zeroPoint, output;
  uint32_t curIndex;

  int32_t classCnt;
  int32_t curClassCountM;
  int32_t totalCnt;
  int32_t maxInit = -32768;
  int32_t totValidCnt = 0;
  float32_tidl maxFloat = -FLT_MAX;

  if (algDetLyrParams->elementType == TIDL_SignedChar)
  {
    maxInit = -128;
  }
  else if (algDetLyrParams->elementType == TIDL_SignedShort)
  {
    maxInit = -32768;
  }
  else if (algDetLyrParams->elementType == TIDL_BFloat16)
  {
    maxFloat = -(float32_tidl)std::numeric_limits<bfloat16_tidl>::max();
  }
  else // if (algDetLyrParams->elementType == TIDL_SinglePrecFloat)
  {
    maxFloat = -FLT_MAX;
  }
  #ifdef TIDL_COVERAGE_DEAD_CODE
  else
  {
      /*Do Nothing*/
  }
  #endif

  if (params->scoreConverter == (int32_t)TIDL_scoreConverterSOFTMAX)
  {
    classCnt = 1;
  }
  else
  {
    totalCnt = 0;
    for (int32_t i = 0; i < params->numClasses; i++)
    {
      if (i == 0)
      {
        algDetLyrParams->countMListAcc[i] = 0;
      }
      else
      {
        algDetLyrParams->countMListAcc[i] = algDetLyrParams->countMListAcc[i - 1] + algDetLyrParams->countMList[i - 1];
        algDetLyrParams->countMList[i - 1] = 0;
      }
      totalCnt += algDetLyrParams->countMList[i];
    }

    algDetLyrParams->countMList[params->numClasses - 1] = 0;

    for (int32_t i = 0; i < totalCnt; i++)
    {
      curIndex = algDetLyrParams->topMIndices[i];
      int32_t cls = (curIndex >> 16U) & 0xffU;

      algDetLyrParams->topMIndicesSorted[algDetLyrParams->countMListAcc[cls] + algDetLyrParams->countMList[cls]] = curIndex;
      algDetLyrParams->countMList[cls]++;
    }
    classCnt = params->numClasses;
  }

  if (params->scoreConverter == TIDL_scoreConverterSIGMOID)
  {
    if (params->metaArchType != (int32_t)TIDL_metaArchTIDLYolo)
    {
      #if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
      if((params->metaArchType == (int32_t)TIDL_metaArchTIDLCenterPoint) && (params->reserve1 != -1))
      {
        totValidCnt = 0;
        for (int32_t cls = 0; cls < classCnt; cls++) // here data for all but background class is coming
        {
          curClassCountM = algDetLyrParams->countMList[cls];
          algDetLyrParams->countMList[cls] = 0;
          for (int32_t cnt = 0; cnt < curClassCountM; cnt++)
          {
            curIndex = algDetLyrParams->topMIndicesSorted[algDetLyrParams->countMListAcc[cls] + cnt];

            int32_t head = (curIndex >> 28U) & 0xFU;
            int32_t anchor = (curIndex >> 24U) & 0xFU;
            int32_t classId = (curIndex >> 16U) & 0xFFU; // this will be eqaul to cls
            int32_t curLoc = curIndex & 0xFFFFU;
            int32_t chPitch = algDetLyrParams->confHeadPitchList[head][TIDL_CHANNEL_PITCH];
            int32_t curCh;

            curCh = (anchor * (params->numClasses + 5)) + classId + 1;

            onebyqFact = algDetLyrParams->inConfdataQList[head];
            zeroPoint = algDetLyrParams->inConfdataZPList[head];

            inVal = ((Tconf *)(algDetLyrParams->inConfDataList[head]))[(curLoc + (chPitch * curCh))];
            ftemp = (inVal - zeroPoint) * onebyqFact;
            output = divsp_i(1.0f, (exp_taylor(-ftemp) + 1.0f)); // to match with ci code, otherwise floating point dfference will come
            
            inVal = ((Tconf *)(algDetLyrParams->inConfDataList[head]))[(curLoc + (chPitch * anchor * (params->numClasses + 5)))];
            ftemp = (inVal - zeroPoint) * onebyqFact;
            ftemp = divsp_i(1.0f, (exp_taylor(-ftemp) + 1.0f)); // to match with ci code, otherwise floating point dfference will come
            output = output * ftemp;

            if (output >= params->confThreshold)
            {
              ftemp = (output * (float32_tidl)SSD_CONF_DATA_Q);
              algDetLyrParams->topMScore[totValidCnt] = (uint16_t)ftemp;
              algDetLyrParams->topMIndices[totValidCnt] = curIndex;
              algDetLyrParams->countMList[cls]++;
              totValidCnt++;
            }
          }
        }
        for (int32_t i = 0; i < params->numClasses; i++)
        {
          if (i == 0)
          {
            algDetLyrParams->countMListAcc[i] = 0;
          }
          else
          {
            algDetLyrParams->countMListAcc[i] = algDetLyrParams->countMListAcc[i - 1] + algDetLyrParams->countMList[i - 1];
          }
        }
        algDetLyrParams->topMIndicesSorted = algDetLyrParams->topMIndices;
        algDetLyrParams->topMScoreSorted   = algDetLyrParams->topMScore;

      }
      else
      {
      #endif
        for (int32_t cls = 0; cls < classCnt; cls++) // here data for all but background class is coming
        {
          if(cls == params->backgroundLabelId)
          {
            continue;
          }

          curClassCountM = algDetLyrParams->countMList[cls];

          for (int32_t cnt = 0; cnt < curClassCountM; cnt++)
          {
            curIndex = algDetLyrParams->topMIndicesSorted[algDetLyrParams->countMListAcc[cls] + cnt];

          int32_t head = (curIndex >> 28U) & 0xFU;
          int32_t anchor = (curIndex >> 24U) & 0xFU;
          int32_t classId = (curIndex >> 16U) & 0xFFU; // this will be eqaul to cls
          int32_t curLoc = curIndex & 0xFFFFU;
          int32_t chPitch = algDetLyrParams->confHeadPitchList[head][TIDL_CHANNEL_PITCH];
          int32_t curCh;

          curCh = (anchor * params->numClasses) + classId;

            onebyqFact = algDetLyrParams->inConfdataQList[head];
            zeroPoint = algDetLyrParams->inConfdataZPList[head];

            inVal = ((Tconf *)(algDetLyrParams->inConfDataList[head]))[(curLoc + (chPitch * curCh))];
            ftemp = (inVal - zeroPoint) * onebyqFact;
            output = divsp_i(1.0f, (exp_taylor(-ftemp) + 1.0f)); // to match with ci code, otherwise floating point dfference will come
            ftemp = (output * (float32_tidl)SSD_CONF_DATA_Q);
            algDetLyrParams->topMScore[algDetLyrParams->countMListAcc[cls] + cnt] = (uint16_t)ftemp;
          }
        }
        algDetLyrParams->topMScoreSorted = algDetLyrParams->topMScore;
      #if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
      }
      #endif
    }
    else
    {
      totValidCnt = 0;
      for (int32_t cls = 0; cls < classCnt; cls++) // here data for all but background class is coming
      {
        curClassCountM = algDetLyrParams->countMList[cls];
        algDetLyrParams->countMList[cls] = 0;
        for (int32_t cnt = 0; cnt < curClassCountM; cnt++)
        {
          curIndex = algDetLyrParams->topMIndicesSorted[algDetLyrParams->countMListAcc[cls] + cnt];

          int32_t head = (curIndex >> 28U) & 0xFU;
          int32_t anchor = (curIndex >> 24U) & 0xFU;
          int32_t classId = (curIndex >> 16U) & 0xFFU; // this will be eqaul to cls
          int32_t curLoc = curIndex & 0xFFFFU;
          int32_t chPitch = algDetLyrParams->confHeadPitchList[head][TIDL_CHANNEL_PITCH];
          int32_t curCh;

          curCh = (anchor * (params->numClasses + 5)) + classId + 1;

          onebyqFact = algDetLyrParams->inConfdataQList[head];
          zeroPoint = algDetLyrParams->inConfdataZPList[head];

          inVal = ((Tconf *)(algDetLyrParams->inConfDataList[head]))[(curLoc + (chPitch * curCh))];
          ftemp = (inVal - zeroPoint) * onebyqFact;
          output = divsp_i(1.0f, (exp_taylor(-ftemp) + 1.0f)); // to match with ci code, otherwise floating point dfference will come
          
          inVal = ((Tconf *)(algDetLyrParams->inConfDataList[head]))[(curLoc + (chPitch * anchor * (params->numClasses + 5)))];
          ftemp = (inVal - zeroPoint) * onebyqFact;
          ftemp = divsp_i(1.0f, (exp_taylor(-ftemp) + 1.0f)); // to match with ci code, otherwise floating point dfference will come
          output = output * ftemp;

          if (output >= params->confThreshold)
          {
            ftemp = (output * (float32_tidl)SSD_CONF_DATA_Q);
            algDetLyrParams->topMScore[totValidCnt] = (uint16_t)ftemp;
            algDetLyrParams->topMIndices[totValidCnt] = curIndex;
            algDetLyrParams->countMList[cls]++;
            totValidCnt++;
          }
        }
      }  
      for (int32_t i = 0; i < params->numClasses; i++)
      {
        if (i == 0)
        {
          algDetLyrParams->countMListAcc[i] = 0;
        }
        else
        {
          algDetLyrParams->countMListAcc[i] = algDetLyrParams->countMListAcc[i - 1] + algDetLyrParams->countMList[i - 1];
        }
      }
      algDetLyrParams->topMIndicesSorted = algDetLyrParams->topMIndices;
      algDetLyrParams->topMScoreSorted   = algDetLyrParams->topMScore;
    }
  }
  else // if (params->scoreConverter == (int32_t)TIDL_scoreConverterSOFTMAX)
  {
    totValidCnt = 0;
    curClassCountM = algDetLyrParams->countMList[0];

    algDetLyrParams->countMList[0] = 0;

    for (int32_t cnt = 0; cnt < curClassCountM; cnt++)
    {
      curIndex = algDetLyrParams->topMIndicesSorted[cnt];

      int32_t head = (curIndex >> 28U) & 0xFU;
      int32_t anchor = (curIndex >> 24U) & 0xFU;
      int32_t curLoc = curIndex & 0xFFFFU;
      int32_t classStride;
      int32_t anchorStride;

      onebyqFact = algDetLyrParams->inConfdataQList[head];
      zeroPoint = algDetLyrParams->inConfdataZPList[head];
      
      if (algDetLyrParams->elementType == TIDL_SinglePrecFloat || algDetLyrParams->elementType == TIDL_BFloat16)
      {
        max = maxFloat;
      }
      else
      {
        max = maxInit;        // -128
      }
      denom = 0.0;
      if(params->dataLayout == 0)
      {
        classStride = 1;
        anchorStride = params->numClasses;
      }
      else /* (params->dataLayout == 1) */
      {
        classStride = algDetLyrParams->numAnchors[head];
        anchorStride = 1;
      }

      /* Apply softMax and store score along with Index */
      for (i4 = 0; i4 < params->numClasses; i4++)
      {
        inVal = ((Tconf *)(algDetLyrParams->inConfDataList[head]))[(curLoc + (algDetLyrParams->confHeadPitchList[head][TIDL_CHANNEL_PITCH] * ((anchor*anchorStride) + (i4*classStride))))];
        max = (max > inVal) ? max : inVal;
      }

      for (i4 = 0; i4 < params->numClasses; i4++)
      {
        inVal = ((Tconf *)(algDetLyrParams->inConfDataList[head]))[(curLoc + (algDetLyrParams->confHeadPitchList[head][TIDL_CHANNEL_PITCH] * ((anchor*anchorStride) + (i4*classStride))))];

        if (typeid(Tconf) == typeid(float32_tidl) || typeid(Tconf) == typeid(bfloat16_tidl))
        {
          ftemp = inVal - max; // in float32_tidl or bfloat16_tidl, else portion will not work, as data is typecasted as int32_t
        }
        else
        {
          temp = ((int32_t)(inVal) - (int32_t)max); // will not work in float32_tidl version
          ftemp = ((float32_tidl)(temp - zeroPoint) * onebyqFact);
        }

        numer = exp_taylor(ftemp);
        algDetLyrParams->tempScore[i4] = numer;
        denom += numer;
      }

      onebydenom = divsp_i((float32_tidl)1.0, denom); // to match with ci code, otherwise floating point dfference will come
      curIndex = curIndex & (~(0xFFU << 16U));

      for (i4 = 0; i4 < params->numClasses; i4++)
      {
        if (i4 == params->backgroundLabelId)
        {
          continue;
        }

        numer = algDetLyrParams->tempScore[i4];
        output = (numer * onebydenom);
        if (output >= params->confThreshold)
        {
          ftemp = (output * (float32_tidl)SSD_CONF_DATA_Q);
          algDetLyrParams->topMScore[totValidCnt] = (uint16_t)ftemp;
          algDetLyrParams->topMIndices[totValidCnt] = curIndex | ((uint32_t)i4 << 16U);
          algDetLyrParams->countMList[i4]++;
          totValidCnt++;
        }
      }
    }

    for (int32_t i = 0; i < params->numClasses; i++)
    {
      if (i == 0)
      {
        algDetLyrParams->countMListAcc[i] = 0;
      }
      else
      {
        algDetLyrParams->countMListAcc[i] = algDetLyrParams->countMListAcc[i - 1] + algDetLyrParams->countMList[i - 1];
        algDetLyrParams->countMList[i - 1] = 0;
      }
    }

    algDetLyrParams->countMList[params->numClasses - 1] = 0;

    for (int32_t i = 0; i < totValidCnt; i++)
    {
      curIndex = algDetLyrParams->topMIndices[i];
      uint16_t score = algDetLyrParams->topMScore[i];
      int32_t cls = (curIndex >> 16U) & 0xffU;

      algDetLyrParams->topMIndicesSorted[algDetLyrParams->countMListAcc[cls] + algDetLyrParams->countMList[cls]] = curIndex;
      algDetLyrParams->topMScoreSorted[algDetLyrParams->countMListAcc[cls] + algDetLyrParams->countMList[cls]] = score;
      algDetLyrParams->countMList[cls]++;
    }
  }
  #ifdef TIDL_COVERAGE_DEAD_CODE
  else
  {
      /*Do Nothing*/
  }
  #endif

  return totValidCnt;
}

