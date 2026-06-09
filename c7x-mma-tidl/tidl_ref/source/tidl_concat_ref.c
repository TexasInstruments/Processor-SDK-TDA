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
@file    tidl_concate_ref.c
@brief   This file defines private functions for conact layer.
@version 0.1 (May 2017) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_concat.h"
#include "tidl_alg_utils.h"
#include "tidl_concat_ref.h"
#include "tidl_forceNegativeTest.h"

typedef struct
{
  int32_t inWidth;
  int32_t inHeight;
  int32_t outWidth;
  int32_t outHeight;
  int32_t inPitch;
  int32_t outPitch;
  int32_t inChPitch;
  int32_t outChPitch;
  int32_t numInData;
  int32_t numInChannels;
  int32_t numOutChannels;
  void *derivedBias;
  uint8_t *derivedScales;
  uint8_t *derivedShifts;
  int32_t quantizationStyle;
  int32_t outElemType;
} tidlConcatBuffParams_t;

/**
 * @brief This function will apply scale on the input data
 *
 * @tparam Tin  : template for the input
 * @tparam Tacc  : template for the accumulator
 * @param pin : Pointer to the Input buffer
 * @param pAcc : pointer to the accumulator
 * @param scale : amount of scale to be applied on the input
 * @param concatBuffParams : copy of concat parameters
 */
template<class Tin, class Tacc> void TIDL_refConcat(
    TIDL_Handle intAlgHandle,
    Tin *pIn,
    Tacc *pAcc,
    int32_t scale,
    tidlConcatBuffParams_t *concatBuffParams,
    int32_t tensorIdx,
    int32_t isKernelHighPrecision)
{
  int32_t i1, i2, i3;
  uint8_t *mmaScale = concatBuffParams->derivedScales;
  uint8_t *mmaShift = concatBuffParams->derivedShifts;
  void *mmaBias = concatBuffParams->derivedBias;
  int64_t tempAcc = 0;
  int32_t outElemSize = TIDL_getDatElementSize(concatBuffParams->outElemType);
  int32_t outElemType = concatBuffParams->outElemType;
  int32_t min = std::numeric_limits<float32_tidl>::min();
  int32_t max = std::numeric_limits<float32_tidl>::max();
  if (outElemType == TIDL_SignedChar)
  {
    min = std::numeric_limits<int8_t>::min();
    max = std::numeric_limits<int8_t>::max();
  }
  else if (outElemType == TIDL_UnsignedChar)
  {
    min = std::numeric_limits<uint8_t>::min();
    max = std::numeric_limits<uint8_t>::max();
  }
  else if (outElemType == TIDL_SignedShort)
  {
    min = std::numeric_limits<int16_t>::min();
    max = std::numeric_limits<int16_t>::max();
  }
  else if (outElemType == TIDL_UnsignedShort)
  {
    min = std::numeric_limits<uint16_t>::min();
    max = std::numeric_limits<uint16_t>::max();
  }

  // OPENACC(data copyin(pIn[:1 + ((concatBuffParams->numInChannels-1)* concatBuffParams->inChPitch) + ((concatBuffParams->inHeight-1) * concatBuffParams->inPitch) + (concatBuffParams->inWidth-1)])
  // present(pAcc[:1 + ((concatBuffParams->numInChannels-1)* concatBuffParams->outChPitch) + ((concatBuffParams->inHeight-1) * concatBuffParams->outPitch) + (concatBuffParams->inWidth-1)]))

  // OPENACC(parallel loop collapse(3))
  for (i1 = 0; i1 < concatBuffParams->numInChannels; i1++)
  {
    for (i2 = 0; i2 < (concatBuffParams->inHeight); i2++)
    {
      for (i3 = 0; i3 < (concatBuffParams->inWidth); i3++)
      {
        int32_t inOffset = (i1 * concatBuffParams->inChPitch) + (i2 * concatBuffParams->inPitch) + i3;
        int32_t outOffset = (i1 * concatBuffParams->outChPitch) + (i2 * concatBuffParams->outPitch) + i3;
        pAcc[outOffset] = pIn[inOffset] * scale;
        if (TRUE == isKernelHighPrecision && outElemType != TIDL_SinglePrecFloat)
        {
          if (outElemType == TIDL_SignedChar || outElemType == TIDL_UnsignedChar)
          {
            tempAcc = pAcc[outOffset] + *((int32_t *)mmaBias + tensorIdx);
          }
          else
          {
            tempAcc = pAcc[outOffset] + *((int64_t *)mmaBias + tensorIdx);
          }
          tempAcc = (int64_t)tempAcc * mmaScale[tensorIdx];
          /*Update the limits correctly:*/
          pAcc[outOffset] = (Tacc)TIDL_roundSatMMA(tempAcc, mmaShift[tensorIdx], min, max);
        }
      }
    }
  }
}
/**
 * @brief This function will do quantization on the accumulator data
 *
 * @tparam Tacc  : template for the accumulator
 * @tparam Tout  : template for the output
 * @param intAlgHandle : tidl algo hanlde
 * @param layerIdx : idx of the current layer
 * @param TIDL_asymRangeToScalepAcc : pointer to the accumulator
 * @param pout : Pointer to the putput buffer
 * @param roundBits : amount of bits for rounding
 * @param concatBuffParams : copy of concat parameters
 * @param satLow : min value for saturation
 * @param satHigh : : max value for saturation
 */
template<class Tacc, class Tout> void TIDL_refConcatQuantize(
    TIDL_Handle intAlgHandle,
    int32_t layerIdx,
    Tacc *pAcc,
    Tout *pout,
    int32_t roundBits,
    tidlConcatBuffParams_t *concatBuffParams,
    int32_t satLow,
    int32_t satHigh,
    sTIDL_DataParams_t *inParams,
    sTIDL_DataParams_t *outParams,
    int32_t isKernelHighPrecision)
{
  int32_t j, k, l;
  int32_t i1, i2, i3;
  int32_t temp_roundBits;
  Tacc outAcc;
  sTIDL_Network_t *net = intAlgHandle->createParams->net;
  sTIDL_Layer_t *tidlLayer = &net->TIDLLayers[layerIdx];

  int32_t numTotRoi = outParams->dimValues[TIDL_DIM_BATCH];
  int32_t numTotDim1 = outParams->dimValues[TIDL_DIM_DIM1];
  int32_t numTotDim2 = outParams->dimValues[TIDL_DIM_DIM2];

  Tacc min, max;
  int64_t temp;
  min = (Tacc)((((uint64_t)1) << ((sizeof(Tacc) * (uint64_t)8) - (uint64_t)1)) - (uint64_t)1);
  temp = (int64_t)(((uint64_t)1) << ((sizeof(Tacc) * (uint64_t)8) - (uint64_t)1));

  max = (Tacc)temp * -1;

  // OPENACC(data present(pAcc[:1+((concatBuffParams->numOutChannels-1) * concatBuffParams->outChPitch) + ((concatBuffParams->outHeight-1) * concatBuffParams->outPitch) + (concatBuffParams->outWidth-1)]))

  //OPENACC(parallel loop collapse(3) reduction(min:min) reduction(max:max))

  for (i1 = 0; i1 < concatBuffParams->numInChannels; i1++)
    {
      for (i2 = 0; i2 < (concatBuffParams->inHeight); i2++)
      {
        for (i3 = 0; i3 < (concatBuffParams->inWidth); i3++)
        {
          int32_t outOffset = (i1* concatBuffParams->outChPitch) + (i2 * concatBuffParams->outPitch) + i3;

          outAcc = pAcc[outOffset];
          min = ((outAcc < min) ? outAcc : min);
          max = ((outAcc > max) ? outAcc : max);
        }
      }
    }

  if (((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
  {
    if (TIDL_getDatElementSign(net->TIDLLayers[layerIdx].outData.elementType) == 1)
    {
      TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, min, max);
    }
    else
    {
      TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, 0, max);
    }
  }
  temp_roundBits = net->TIDLLayers[layerIdx].outData.roundBits;

  float elementtype = tidlLayer->outData.elementType;
  int32_t outElemSize = TIDL_getDatElementSize(tidlLayer->outData.elementType);

  // OPENACC(data present(pAcc[:1+((concatBuffParams->numOutChannels-1) * concatBuffParams->outChPitch) + ((concatBuffParams->outHeight-1) * concatBuffParams->outPitch) + (concatBuffParams->outWidth-1)])
  // copy(pout[:1+((concatBuffParams->numOutChannels-1) * concatBuffParams->outChPitch) + ((concatBuffParams->outHeight-1) * concatBuffParams->outPitch) + (concatBuffParams->outWidth-1)]))

  //OPENACC(parallel loop collapse(3))

  for (i1 = 0; i1 < concatBuffParams->numInChannels; i1++)
  {
    for (i2 = 0; i2 < (concatBuffParams->inHeight); i2++)
    {
      for (i3 = 0; i3 < (concatBuffParams->inWidth); i3++)
      {
        int32_t outOffset = (i1* concatBuffParams->outChPitch) + (i2 * concatBuffParams->outPitch) + i3;

        outAcc = pAcc[outOffset];
        //printf("Outoffset = %d\n",outOffset);
        if (elementtype == TIDL_SinglePrecFloat)
        {
          //OPENACC(routine(TIDL_floatSat))
          outAcc = TIDL_floatSat(outAcc, &net->TIDLLayers[layerIdx]);
        }
        else if((isKernelHighPrecision == TRUE))
        {
          //Saturate only
          outAcc  = (Tacc)TIDL_roundSatMMA(outAcc, 0, satLow, satHigh);
        }
        else
        {
          //OPENACC(routine(TIDL_roundSat))
          outAcc = (Tacc)TIDL_roundSat((int64_t)outAcc, temp_roundBits, satLow, satHigh);
        }
        pout[outOffset] = outAcc;
      }
    }
  }
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_CONCAT
@fn         TIDL_refConcatProcess
@brief      Concat layer operation currently supports ad
            This function does the C-Natural implementation of block level
@param      intAlgHandle : tidl algorothm handle
@param      algLayer : Ptr Common Layer parameters and other layer parameters.
@param      inPtrs: Ptr array to input buffers  in external memory.
@param      outPtr: Ptr to output buffer in external memory.
@param      numTotRoi: Indicates the no of ROI
@param      concatBuffParams: Ptr to buffer params used for in/out transfers
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

int32_t TIDL_refConcatProcess(
    TIDL_Handle intAlgHandle,
    sTIDL_AlgLayer_t *algLayer,
    sTIDL_Layer_t *tidlLayer,
    void *inPtrs[],
    uint8_t *outPtr,
    sTIDL_DataParams_t *inParams,
    sTIDL_DataParams_t *outParams,
    tidlConcatBuffParams_t *concatBuffParams,
    int32_t isKernelHighPrecision)
{
  int32_t status = IALG_EOK;
  int32_t i4;
  int32_t j;
  int32_t k;
  int32_t l;
  int32_t inDataScale[TIDL_NUM_IN_BUFS] = {1};
  float32_tidl accScale;
  int32_t * refAccPtr;
  int32_t * refAccPtrTemp;
  uint8_t * outPtrtemp;
  uint32_t BytesPerElement = 1;
  int32_t layerIdx = algLayer->layerIdx;
  int32_t accMemSize;

  uint16_t numTotRoi = inParams->dimValues[TIDL_DIM_BATCH];
  uint16_t numTotDim1 = inParams->dimValues[TIDL_DIM_DIM1];
  uint16_t numTotDim2 = inParams->dimValues[TIDL_DIM_DIM2];

  accMemSize = (int32_t)sizeof(int32_t)*((int32_t)numTotRoi*numTotDim1*numTotDim2*concatBuffParams->numOutChannels*concatBuffParams->outChPitch);
  outPtrtemp = (uint8_t *)outPtr;
  if (algLayer->scratchSize >= accMemSize)
  {
    refAccPtr = (int32_t *)algLayer->scratchMem;
  }
  else
  {
    tidl_printf(0, "Memory for  TIDL_refConcatProcess accumulator is not sufficient exiting...\n    ");
    status = TIDL_ERR_FAILURE;
  }
  if (status == IALG_EOK)
  {
    uint32_t tempRoundBit = ((uint32_t)(1) << (uint32_t)tidlLayer->outData.roundBits);
    accScale = tidlLayer->outData.tensorScale * (float32_tidl)tempRoundBit;
    int32_t outElemSize = TIDL_getDatElementSize(concatBuffParams->outElemType);
    for (j = 0; j < tidlLayer->numInBufs; j++)
    {
      sTIDL_DataParams_t *indata = TIDL_getDataParams(intAlgHandle->createParams->net, tidlLayer->inData[j]);
      inDataScale[j] = (accScale / indata->tensorScale);
      if (isKernelHighPrecision == TRUE)
      {
        inDataScale[j] = 1; /*If needed specify separately*/
      }
    }
    for (j = 0; j < (int32_t)numTotRoi; j++)
    {
      for (k = 0; k < (int32_t)numTotDim1; k++)
      {
        for (l = 0; l < (int32_t)numTotDim2; l++)
        {
          refAccPtrTemp = refAccPtr + (l * outParams->pitch[TIDL_DIM2_PITCH]) + (k * outParams->pitch[TIDL_DIM1_PITCH]) + (j * outParams->pitch[TIDL_ROI_PITCH]);
          outPtrtemp = outPtr + ((l * outParams->pitch[TIDL_DIM2_PITCH]) + (k * outParams->pitch[TIDL_DIM1_PITCH]) + (j * outParams->pitch[TIDL_ROI_PITCH])) * BytesPerElement;

          for (i4 = 0; i4 < (concatBuffParams->numInData); i4++)
          {
            /*
              suppose concat layer has 6 inputs.
              NC is creating 3 workloads. 
              1st WL has dma and processing link for input_1 and input_2
              2nd WL has dma and processing link for input_2 and input_4
              3rd WL has dma and processing link for input_5 and input_5

              Now we are calling ref kernel 3 times and each time inPtrs array will look like :
              1st call : inPtrs[0] = addr_0
                         inPtrs[1] = addr_1
                         inPtrs[2] = 0x0
                         inPtrs[3] = 0x0
                         inPtrs[4] = 0x0
                         inPtrs[5] = 0x0

              2nd call : inPtrs[0] = 0x0
                         inPtrs[1] = 0x0
                         inPtrs[2] = addr_2
                         inPtrs[3] = addr_3
                         inPtrs[4] = 0x0
                         inPtrs[5] = 0x0

              3rd call : inPtrs[0] = 0x0
                         inPtrs[1] = 0x0
                         inPtrs[2] = 0x0
                         inPtrs[3] = 0x0
                         inPtrs[4] = addr_4
                         inPtrs[5] = addr_5
            */
            int8_t * tempInPtr = ((int8_t *)inPtrs[i4]);
            if(tempInPtr == NULL){
              continue;
            }
            sTIDL_DataParams_t *inDataParams;
            inDataParams = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[i4]].outData;
            concatBuffParams->inPitch = inDataParams->pitch[TIDL_LINE_PITCH];
            concatBuffParams->inChPitch = inDataParams->pitch[TIDL_CHANNEL_PITCH];
            concatBuffParams->numInChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
            concatBuffParams->inWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];
            concatBuffParams->inHeight = inDataParams->dimValues[TIDL_DIM_HEIGHT];
            BytesPerElement = TIDL_getDatElementSize(inDataParams->elementType);

            uint32_t inPtrOffset = (((l * inDataParams->pitch[TIDL_DIM2_PITCH]) + (k * inDataParams->pitch[TIDL_DIM1_PITCH]) + (j * inDataParams->pitch[TIDL_ROI_PITCH])));

            if (inDataParams->elementType == TIDL_SignedChar)
            {
              int8_t * inPtr = ((int8_t *)inPtrs[i4] + inPtrOffset);
              TIDL_refConcat(intAlgHandle, (int8_t*)inPtr, refAccPtrTemp, inDataScale[i4], concatBuffParams, i4, isKernelHighPrecision);
            }
            else if (inDataParams->elementType == TIDL_UnsignedChar)
            {
              uint8_t * inPtr = ((uint8_t *)inPtrs[i4] + inPtrOffset);
              TIDL_refConcat(intAlgHandle, (uint8_t*)inPtr, refAccPtrTemp, inDataScale[i4], concatBuffParams, i4, isKernelHighPrecision);
            }
            else if (inDataParams->elementType == TIDL_SignedShort)
            {
              int16_t * inPtr = ((int16_t *)inPtrs[i4] + inPtrOffset);
              TIDL_refConcat(intAlgHandle, (int16_t*)inPtr, refAccPtrTemp, inDataScale[i4], concatBuffParams, i4, isKernelHighPrecision);
            }
            else if (inDataParams->elementType == TIDL_UnsignedShort)
            {
              uint16_t * inPtr = ((uint16_t *)inPtrs[i4] + inPtrOffset);
              TIDL_refConcat(intAlgHandle, (uint16_t*)inPtr, refAccPtrTemp, inDataScale[i4], concatBuffParams, i4, isKernelHighPrecision);
            }
            else if (inDataParams->elementType == TIDL_SinglePrecFloat)
            {
              float32_tidl * inPtr = ((float32_tidl *)inPtrs[i4] + inPtrOffset);
              TIDL_refConcat(intAlgHandle, (float32_tidl*)inPtr, (float32_tidl*)refAccPtrTemp, 1.0, concatBuffParams, i4, isKernelHighPrecision);
            }
            else
            {
              tidl_printf(0,"TIDL_Concatlayer in elementType is  Not suported !!!\n ");
              status = TIDL_ERR_FAILURE;
            }
            
            if (tidlLayer->outData.elementType == TIDL_SignedChar)
            {
              TIDL_refConcatQuantize(intAlgHandle, layerIdx, refAccPtrTemp, (int8_t*)outPtrtemp,
              tidlLayer->outData.roundBits, concatBuffParams, TIDL_SAT_LO_INT8, TIDL_SAT_HI_INT8, inParams, outParams,isKernelHighPrecision);
            }
            else if (tidlLayer->outData.elementType == TIDL_UnsignedChar)
            {
              TIDL_refConcatQuantize(intAlgHandle, layerIdx, refAccPtrTemp, (uint8_t*)outPtrtemp,
              tidlLayer->outData.roundBits, concatBuffParams, TIDL_SAT_LO_UINT8, TIDL_SAT_HI_UINT8, inParams, outParams, isKernelHighPrecision);
            }
            else if (tidlLayer->outData.elementType == TIDL_SignedShort)
            {
              TIDL_refConcatQuantize(intAlgHandle, layerIdx, refAccPtrTemp, (int16_t*)outPtrtemp,
              tidlLayer->outData.roundBits, concatBuffParams, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16, inParams, outParams,isKernelHighPrecision);
            }
            else if (tidlLayer->outData.elementType == TIDL_UnsignedShort)
            {
              TIDL_refConcatQuantize(intAlgHandle, layerIdx, refAccPtrTemp, (uint16_t*)outPtrtemp,
              tidlLayer->outData.roundBits, concatBuffParams, TIDL_SAT_LO_UINT16, TIDL_SAT_HI_UINT16, inParams, outParams,isKernelHighPrecision);
            }
            else if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat)
            {
              TIDL_refConcatQuantize(intAlgHandle, layerIdx, (float32_tidl *)refAccPtrTemp, (float32_tidl *)outPtrtemp,
              tidlLayer->outData.roundBits, concatBuffParams, TIDL_SAT_LO_UINT16, TIDL_SAT_HI_UINT16, inParams, outParams,isKernelHighPrecision);
            }
            else
            {
              tidl_printf(0,"TIDL_Concatlayer out elementType is  Not suported !!!\n ");
              status = TIDL_ERR_FAILURE;
            }
            
            if (tidlLayer->layerParams.concatParams.axis == TIDL_DIM_NUMCH)
            {
              /* Channel wise feature concate */
              refAccPtrTemp += (uint32_t)concatBuffParams->numInChannels*(uint32_t)concatBuffParams->outChPitch;
              outPtrtemp += (uint32_t)concatBuffParams->numInChannels*(uint32_t)concatBuffParams->outChPitch * BytesPerElement;
            }
            else if (tidlLayer->layerParams.concatParams.axis == TIDL_DIM_WIDTH)
            {
              /* Width wise feature concate */
              refAccPtrTemp += (uint32_t)inDataParams->dimValues[TIDL_DIM_WIDTH];
              outPtrtemp += (uint32_t)inDataParams->dimValues[TIDL_DIM_WIDTH] * BytesPerElement;
            }
            else if (tidlLayer->layerParams.concatParams.axis == TIDL_DIM_HEIGHT)
            {
              /* Height wise feature concate */
              refAccPtrTemp += (uint32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT]*(uint32_t)concatBuffParams->outPitch;
              outPtrtemp += (uint32_t)inDataParams->dimValues[TIDL_DIM_HEIGHT]*(uint32_t)concatBuffParams->outPitch * BytesPerElement;
            }
            else
            {
              tidl_printf(0, "TIDL_Concatlayer Un suported layerParams.concatParams.axis  !!!\n ");
            }

            TIDL_L1DandL2CacheWbInv();
          }
        }
      }
    }
  }
  return status;
}

static void TIDL_updateConcatMemorySizes(const TIDL_LayerSpecificParams *layerSpecificParams,
                                         const TIDL_NetworkCommonParams *commonParams,
                                         int32_t layerIdx,
                                         int32_t *scratchDataSizeOut,
                                         int32_t *outDataSizeOut,
                                         int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t paramMemSize = 0;
  int32_t outDataSize = TIDL_refGetOutDataSize(commonParams, layerSpecificParams, layerIdx);
  int32_t scratchDataSize = TIDL_updateDataParamsPitch(&commonParams->net->TIDLLayers[layerIdx].outData) * ((int32_t)sizeof(int32_t));

  if (scratchDataSizeOut != NULL)
  {
    *scratchDataSizeOut = scratchDataSize;
  }

  if (outDataSizeOut != NULL)
  {
    *outDataSizeOut = outDataSize;
  }

  memorySize[TIDL_LAYER_MEMORY_SCRATCH] = scratchDataSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] = paramMemSize + TIDL_ALIGNMENT_SIZE;
  memorySize[TIDL_LAYER_MEMORY_OUTPUT] = outDataSize + TIDL_ALIGNMENT_SIZE;
}

int32_t TIDL_concatRefAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            int32_t layerIdx,
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  TIDL_updateConcatMemorySizes(layerSpecificParams, commonParams, layerIdx,
                               NULL, NULL, memorySize);

  return status;
}

int32_t TIDL_concatRefInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                           const TIDL_NetworkCommonParams *commonParams,
                           sTIDL_AlgLayer_t *algLayer,
                           int32_t layerIdx,
                           uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                           int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                           void **outPtr)
{
  int32_t status = IALG_EOK;
  int32_t currOffset = 0;
  int32_t scratchDataSize;
  int32_t outDataSize;

  TIDL_updateConcatMemorySizes(layerSpecificParams, commonParams, layerIdx,
                               &scratchDataSize, &outDataSize, memorySize);
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  <justification end> */
  if (outPtr != NULL )
  {
    /* Point to NULL if output of network */
    if (TIDL_isOutDataBuff(commonParams->net,
                           commonParams->net->TIDLLayers[layerIdx].outData.dataId,
                           commonParams->createParams->currLayersGroupId) == 1)
    {
      *outPtr = NULL;
    }
    else if (outDataSize != 0) /* Not output of network and outDataSize > 0 */
    {
      TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_OUTPUT],
                       &currOffset,
                       outDataSize,
                       128,
                       outPtr);
    }
    else /* OutDataSize > 0 */
    {
      *outPtr = NULL;
    }
  }
  /* LDRA_JUSTIFY_END */
  currOffset = 0;
  TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_SCRATCH],
                   &currOffset,
                   scratchDataSize,
                   128,
                   &algLayer->scratchMem);
  algLayer->scratchSize = scratchDataSize;

  return status;
}

int32_t TIDL_concatRefProcess(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t *algLayer,
                              sTIDL_Layer_t *tidlLayer,
                              void *inPtrs[],
                              void *outPtrs[],
                              int32_t layerIdx)
{
  int32_t status = IALG_EOK;
  uint8_t *outPtr = (uint8_t *)outPtrs[0];
  int32_t isKernelHighPrecision = TIDL_isKernelHighPrecision(tidlLayer->layerKernelType);
  sTIDL_DataParams_t *inDataParams = &commonParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
  sTIDL_DataParams_t *outDataParams = &tidlLayer->outData;
  int32_t outElemSize = TIDL_getDatElementSize(outDataParams->elementType);
  tidlConcatBuffParams_t concatBuffParams;

  sTIDL_ConcatParams_t *concatParams = &commonParams->net->TIDLLayers[layerIdx].layerParams.concatParams;

  if (TRUE == isKernelHighPrecision)
  {
    uint8_t *derivedScales = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), concatParams->derivedScales);
    uint8_t *derivedShifts = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), concatParams->derivedShifts);
    void *derivedBias = get_int8_t_pointer((int8_t *)(commonParams->net), concatParams->derivedBias);
    /*Update concat derived pointers*/
    concatBuffParams.derivedScales = derivedScales;
    concatBuffParams.derivedShifts = derivedShifts;
    concatBuffParams.derivedBias = derivedBias;
  }
  else
  {
    concatBuffParams.derivedScales = NULL;
    concatBuffParams.derivedShifts = NULL;
    concatBuffParams.derivedBias = NULL;
  }

  concatBuffParams.outWidth = outDataParams->dimValues[TIDL_DIM_WIDTH];
  concatBuffParams.outHeight = outDataParams->dimValues[TIDL_DIM_HEIGHT];
  concatBuffParams.outPitch = outDataParams->pitch[TIDL_LINE_PITCH];
  concatBuffParams.outChPitch = outDataParams->pitch[TIDL_CHANNEL_PITCH];
  concatBuffParams.numOutChannels = outDataParams->dimValues[TIDL_DIM_NUMCH];
  concatBuffParams.numInData = tidlLayer->numInBufs;
  concatBuffParams.quantizationStyle = commonParams->net->quantizationStyle;
  concatBuffParams.outElemType = tidlLayer->outData.elementType;
  TIDL_Obj intAlgObj;
  TIDL_CreateParams createParams;

  (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
  intAlgObj.createParams = (TIDL_CreateParams *)&createParams;
  if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONCAT_FORCE_SCRATCH_SIZE)
  {
    algLayer->scratchSize = 0;
  }
  status = TIDL_refConcatProcess(&intAlgObj,
                                 algLayer,
                                 tidlLayer,
                                 inPtrs,
                                 outPtr,
                                 inDataParams,
                                 outDataParams,
                                 &concatBuffParams,
                                 isKernelHighPrecision);

  return status;
}
