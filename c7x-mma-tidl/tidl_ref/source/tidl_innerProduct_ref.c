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
 @file    tidl_innerProduct_ref.c
 @brief   This file contains TILD InnerProduct Layer implementation
 @version 0.1 (Jan 2017) : initial version [EPR]
 ----------------------------------------------------------------------------
*/

#include "tidl_innerProduct.h"
#include "tidl_innerProduct_int.h"
#include <math.h>
#include <float.h>
#include "tidl_alg_utils.h"
#include "tidl_commonUtils.h"
#include "tidl_innerProduct_ref.h"

#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
static int CUDA_INNER_PRODUCT_COUNTER = 0;
#endif

/* Function Prototypes */
template<class Tin, class TBin, class Tw, class Tb, class Tout, class Tacc>
    int32_t TIDL_refInnerProduct(
        TIDL_Handle intAlgHandle,
        int32_t layerIdx,
        Tacc *accPtr,
        Tin *inPtr,
        const TBin *inBPtr,
        Tout *outPtr,
        Tb *biasPtr,
        const Tw *weightsPtr,
        tidlInnerProductBuffParams_t *buffParams,
        int32_t satLow,
        int32_t satHigh);

template<class Tw, class Tb, class Tacc>
    int32_t TIDL_refInnerProductParamBitDepth(
        TIDL_Handle intAlgHandle,
        int32_t layerIdx,
        void *inPtr,
        const void *inBPtr,
        void *refPtr,
        Tb *biasPtr,
        const Tw *weightsPtr,
        Tacc *accPtr,
        int32_t outElementType,
        tidlInnerProductBuffParams_t *buffParams);

/**
 * @brief  This funciton is the core kernel of reference innerProduct
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param accPtr : pointer to the accumulator buffer
 * @param inPtr : Input pointer on which innerProduct is applied
 * @param outPtr : Output pointer after innerProduct opreation
 * @param biasPtr  : Pointer to the Bias values
 * @param weightsPtr : Pointer to weights buffer
 * @param buffParams : parameters of the innerProduct buffer
 * @param satLow : Min value for the saturation
 * @param satHigh : Max value for the saturation
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tin, class TBin, class Tw, class Tb, class Tout, class Tacc>
    int32_t TIDL_refInnerProduct(
        TIDL_Handle intAlgHandle,
        int32_t layerIdx,
        Tacc *accPtr,
        Tin *inPtr,
        const TBin *inBPtr,
        Tout *outPtr,
        Tb *biasPtr,
        const Tw *weightsPtr,
        tidlInnerProductBuffParams_t *buffParams,
        int32_t satLow,
        int32_t satHigh)
{
  int32_t status = TIDL_SUCCESS;
  sTIDL_Network_t *net = intAlgHandle->createParams->net;
  int32_t quantizationStyle = intAlgHandle->createParams->net->quantizationStyle;
  int32_t mmaScale = 1;
  sTIDL_Layer_t *tidlLayer = &net->TIDLLayers[layerIdx];

  sTIDL_DataParams_t *inADataParams = TIDL_getDataParams(net, tidlLayer->inData[0]);
  sTIDL_DataParams_t *inBDataParams = TIDL_getDataParams(net, tidlLayer->inData[1]);
  sTIDL_DataParams_t *outDataParams = TIDL_getDataParams(net, tidlLayer->outData.dataId);

  Tb *pBiasData = NULL;
  int32_t roundBits = 0;
  sTIDL_InnerProductParams_t IPparams = tidlLayer->layerParams.innerProductParams;

#if defined TIDL_COVERAGE_DEAD_CODE || defined BUILD_WITH_OPENACC
  int32_t inputATranspose = IPparams.inputATranspose;
#endif
  int32_t inputBTranspose = IPparams.inputBTranspose;

  uint32_t  numInCols       =  (uint32_t)IPparams.numInCols;
  uint32_t  numOutCols      =  (uint32_t)IPparams.numOutCols;
  uint32_t  numInRows       =  (uint32_t)IPparams.numInRows;
  uint32_t  numOutRows      =  numInRows;

  uint32_t outChPitch = (uint32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  uint32_t outLinePitch = (uint32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  uint32_t outDIM1Pitch = (uint32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  uint32_t outDIM2Pitch = (uint32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  uint32_t outBatchPitch = (uint32_t)outDataParams->pitch[TIDL_ROI_PITCH];

  uint32_t inAChPitch = (uint32_t)inADataParams->pitch[TIDL_CHANNEL_PITCH];
  uint32_t inALinePitch = (uint32_t)inADataParams->pitch[TIDL_LINE_PITCH];
  uint32_t inADIM1Pitch = (uint32_t)inADataParams->pitch[TIDL_DIM1_PITCH];
  uint32_t inADIM2Pitch = (uint32_t)inADataParams->pitch[TIDL_DIM2_PITCH];
  uint32_t inABatchPitch = (uint32_t)inADataParams->pitch[TIDL_ROI_PITCH];

  uint32_t  inBChPitch = 0;
  uint32_t  inBLinePitch = 0;
  uint32_t  inBDIM1Pitch = 0;
  uint32_t  inBDIM2Pitch = 0;
  uint32_t  inBBatchPitch = 0;

  uint32_t numBChannels = 1;

  if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_INVALID_SCRATCHSIZE) /* forcing buffParams->scratchSize to -1 to check error case */
  {
    buffParams->scratchSize = -1;
  }

  if (inBPtr == NULL)
  {
    if (inputBTranspose != 0)
    {
      inBLinePitch = numInCols;
    }
    else
    {
      inBLinePitch = numOutCols;
    }
    inBChPitch = numInCols * (uint32_t)numOutCols;
  }
  else
  {
    inBLinePitch = (uint32_t)inBDataParams->pitch[TIDL_LINE_PITCH];
    inBChPitch = (uint32_t)inBDataParams->pitch[TIDL_CHANNEL_PITCH];
    inBDIM1Pitch = (uint32_t)inBDataParams->pitch[TIDL_DIM1_PITCH];
    inBDIM2Pitch = (uint32_t)inBDataParams->pitch[TIDL_DIM2_PITCH];
    inBBatchPitch = (uint32_t)inBDataParams->pitch[TIDL_ROI_PITCH];
    numBChannels = inBDataParams->dimValues[TIDL_DIM_NUMCH];
  }

  /* only from channel to higher possible (as need at least 2 dim for inner product)*/
  if (inBPtr != NULL)
  {
    for (int32_t i1 = TIDL_DIM_NUMCH; i1 >= TIDL_DIM_BATCH; i1--)
    {
      if((inBDataParams->dimValues[i1] != inADataParams->dimValues[i1]) && // unequal dim
          (inBDataParams->dimValues[i1] == 1)                               // dim-wise broadcast possible
      )
      {
        if (i1 == TIDL_DIM_NUMCH)
        {
          inBChPitch = 0;
        }
        if (i1 == TIDL_DIM_DIM1)
        {
          inBDIM1Pitch = 0;
        }
        if (i1 == TIDL_DIM_DIM2)
        {
          inBDIM2Pitch = 0;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        For each batch in the input, the memory planner produces a seperate workload (Ref: ti_dl/utils/perfsim/src/workloadunit.c, line no: 3366) and
        the ref code process the inputs one workload entry at a time. Therefore this condition will always fails since input A, B batch dimensions are always 1.
        <justification end> */
        if (i1 == TIDL_DIM_BATCH)
        {
          inBBatchPitch = 0;
        }
        /* LDRA_JUSTIFY_END */
      }
    }
  }
  if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_INPTR_NULL)
  {
    inPtr = NULL;
  }
  if (inPtr != NULL)
  {
    for (int32_t i1 = TIDL_DIM_NUMCH; i1 >= TIDL_DIM_BATCH; i1--)
    {
      if(inBPtr != NULL)
      {
        if( (inBDataParams->dimValues[i1] != inADataParams->dimValues[i1]) && // unequal dim
            (inADataParams->dimValues[i1] == 1)                               // dim-wise broadcast possible
          )
        {
          if (i1 == TIDL_DIM_NUMCH)
          {
            inAChPitch = 0;
          }
          if (i1 == TIDL_DIM_DIM1)
          {
            inADIM1Pitch = 0;
          }
          if (i1 == TIDL_DIM_DIM2)
          {
            inADIM2Pitch = 0;
          }
          /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> LDRA_JUSTIFICATION_INNERPRODUCT_PRIOR_CHECK : singleton batch in input A and non-singleton batch in input B is not supported
            <justification end>
          */
          if (i1 == TIDL_DIM_BATCH)
          {
            inABatchPitch = 0;
          }
          /* LDRA_JUSTIFY_END */
        }
      }
    }
  }
  else
  {
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_INPTR_NULL);
    status = TIDL_ERR_FAILURE;
  }

  if (status == TIDL_SUCCESS)
  {
    int32_t accMemSize = outDataParams->dimValues[TIDL_DIM_BATCH] * outDataParams->dimValues[TIDL_DIM_DIM1] * outDataParams->dimValues[TIDL_DIM_DIM2] *
                         outDataParams->dimValues[TIDL_DIM_NUMCH] * numOutRows * outLinePitch * (uint32_t)sizeof(Tacc);

    if (biasPtr != NULL)
    {
      pBiasData = (Tb *)biasPtr;
    }

    if (buffParams->scratchSize >= accMemSize)
    {
      accPtr = (Tacc *)buffParams->scratchMem;
    }
    else
    {
      tidl_printf(0, "Memory for  TIDL_refInnerProduct accumulator is not sufficient exiting... %d %d \n", buffParams->scratchSize, accMemSize);
      status = TIDL_ERR_FAILURE;
    }
  }
  if (status == TIDL_SUCCESS)
  {
    int64_t temp;
    Tacc min, max;
    min = (Tacc)((((uint64_t)1) << ((sizeof(Tacc) * 8U) - 1U)) - 1U);
    temp = (int64_t)(((uint64_t)1) << ((sizeof(Tacc) * 8U) - 1U));

    max = (Tacc)temp * -1;

    int32_t batches = 0, dim1 = 0, dim2 = 0, channels = 0;
    if (tidlLayer->numInBufs == 2)
    {
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      batches = MAX(inADataParams->dimValues[TIDL_DIM_BATCH], inBDataParams->dimValues[TIDL_DIM_BATCH]);
      /* LDRA_JUSTIFY_END */
      dim1 = MAX(inADataParams->dimValues[TIDL_DIM_DIM1], inBDataParams->dimValues[TIDL_DIM_DIM1]);
      dim2 = MAX(inADataParams->dimValues[TIDL_DIM_DIM2], inBDataParams->dimValues[TIDL_DIM_DIM2]);
      channels = MAX(inADataParams->dimValues[TIDL_DIM_NUMCH], inBDataParams->dimValues[TIDL_DIM_NUMCH]);
    }
    else
    {
      batches = inADataParams->dimValues[TIDL_DIM_BATCH];
      dim1 = inADataParams->dimValues[TIDL_DIM_DIM1];
      dim2 = inADataParams->dimValues[TIDL_DIM_DIM2];
      channels = inADataParams->dimValues[TIDL_DIM_NUMCH];
    }

    if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_INPTR_INFER_NULL)
    {
      inPtr = NULL;
    }
#ifdef BUILD_WITH_OPENACC
    int32_t inAPtr_sz, inBPtr_sz, weightsPtr_sz, pBiasData_sz, accPtr_sz;
    if ((IPparams.isBias) && (IPparams.constIdx == -1))
    {
      pBiasData_sz = 1 + (numOutRows - 1) * numOutCols + (numOutCols - 1);
    }
    else if ((IPparams.isBias) && (IPparams.constIdx == 0))
    {
      pBiasData_sz = numOutRows;
    }
    else if ((IPparams.isBias) && (IPparams.constIdx == 1))
    {
      pBiasData_sz = numBChannels * numOutCols;
    }
    else
    {
      pBiasData_sz = 0;
    }

    if (inBPtr != NULL)
    {
      if (inputATranspose != 0) /* transpose needed for inputA */
      {
        inAPtr_sz = 1 + inABatchPitch * (batches - 1) + inADIM1Pitch * (dim1 - 1) + inADIM2Pitch * (dim2 - 1) + inAChPitch * (channels - 1) + inALinePitch * (numInCols - 1) + (numOutCols - 1);
        inBPtr_sz = 1 + inBBatchPitch * (batches - 1) + inBDIM1Pitch * (dim1 - 1) + inBDIM2Pitch * (dim2 - 1) + inBChPitch * (channels - 1) + inBLinePitch * (numInCols - 1) + (numOutCols - 1);
        weightsPtr_sz = 0;
      }
      else if (inputBTranspose != 0) /* transpose needed for inputB */
      {
        inAPtr_sz = 1 + inABatchPitch * (batches - 1) + inADIM1Pitch * (dim1 - 1) + inADIM2Pitch * (dim2 - 1) + inAChPitch * (channels - 1) + inALinePitch * (numOutRows - 1) + (numInCols - 1);
        inBPtr_sz = 1 + inBBatchPitch * (batches - 1) + inBDIM1Pitch * (dim1 - 1) + inBDIM2Pitch * (dim2 - 1) + inBChPitch * (channels - 1) + inBLinePitch * (numOutCols - 1) + (numInCols - 1);
        weightsPtr_sz = 0;
      }
      else /* no transpose needed */
      {
        inAPtr_sz = 1 + inABatchPitch * (batches - 1) + inADIM1Pitch * (dim1 - 1) + inADIM2Pitch * (dim2 - 1) + inAChPitch * (channels - 1) + inALinePitch * (numOutRows - 1) + (numInCols - 1);
        inBPtr_sz = 1 + inBBatchPitch * (batches - 1) + inBDIM1Pitch * (dim1 - 1) + inBDIM2Pitch * (dim2 - 1) + inBChPitch * (channels - 1) + inBLinePitch * (numInCols - 1) + (numOutCols - 1);
        weightsPtr_sz = 0;
      }
    }
    else
    {
      if (inputATranspose != 0) /* transpose needed for inputA */
      {
        inAPtr_sz = 1 + inABatchPitch * (batches - 1) + inADIM1Pitch * (dim1 - 1) + inADIM2Pitch * (dim2 - 1) + inAChPitch * (channels - 1) + inALinePitch * (numInCols - 1) + (numOutCols - 1);
        weightsPtr_sz = 1 + inBBatchPitch * (batches - 1) + inBDIM1Pitch * (dim1 - 1) + inBDIM2Pitch * (dim2 - 1) + inBChPitch * (channels - 1) + inBLinePitch * (numInCols - 1) + (numOutCols - 1);
        inBPtr_sz = 0;
      }
      else if (inputBTranspose != 0) /* transpose needed for inputB */
      {
        inAPtr_sz = 1 + inABatchPitch * (batches - 1) + inADIM1Pitch * (dim1 - 1) + inADIM2Pitch * (dim2 - 1) + inAChPitch * (channels - 1) + inALinePitch * (numOutRows - 1) + (numInCols - 1);
        weightsPtr_sz = 1 + inBBatchPitch * (batches - 1) + inBDIM1Pitch * (dim1 - 1) + inBDIM2Pitch * (dim2 - 1) + inBChPitch * (channels - 1) + inBLinePitch * (numOutCols - 1) + (numInCols - 1);
        inBPtr_sz = 0;
      }
      else /* no transpose needed */
      {
        inAPtr_sz = 1 + inABatchPitch * (batches - 1) + inADIM1Pitch * (dim1 - 1) + inADIM2Pitch * (dim2 - 1) + inAChPitch * (channels - 1) + inALinePitch * (numOutRows - 1) + (numInCols - 1);
        weightsPtr_sz = 1 + inBBatchPitch * (batches - 1) + inBDIM1Pitch * (dim1 - 1) + inBDIM2Pitch * (dim2 - 1) + inBChPitch * (channels - 1) + inBLinePitch * (numInCols - 1) + (numOutCols - 1);
        inBPtr_sz = 0;
      }
    }

    accPtr_sz = 1 + outBatchPitch*(batches-1) + outDIM1Pitch*(dim1-1) + outDIM2Pitch*(dim2-1) + outChPitch*(channels-1) + outLinePitch*(numOutRows-1) + (numOutCols-1);
  OPENACC(data copyin(inPtr[0:inAPtr_sz], \
                        inBPtr[0:inBPtr_sz], \
                        weightsPtr[0:weightsPtr_sz], \
                        pBiasData[0:pBiasData_sz]) \
            copyout(accPtr[0:accPtr_sz]))
  {
  OPENACC(parallel loop collapse(6))
  for (int32_t batchIdx = 0; batchIdx < batches; batchIdx++)
  {
    for (int32_t dim1Idx = 0; dim1Idx < dim1; dim1Idx++)
    {
      for (int32_t dim2Idx = 0; dim2Idx < dim2; dim2Idx++)
      {
        for (int32_t chIdx = 0; chIdx < channels; chIdx++)
        {
          for (uint32_t outRwIdx = 0; outRwIdx < numOutRows; outRwIdx++)
          {
            for (uint32_t outColIdx = 0; outColIdx < numOutCols; outColIdx++)
            {
              Tacc  sum;
              if((IPparams.isBias) && (IPparams.constIdx == -1))
              {
                sum = pBiasData[(outRwIdx*numOutCols) + outColIdx];
              }
              else if((IPparams.isBias) && (IPparams.constIdx == 0))
              {
                sum = pBiasData[outRwIdx];
              }
              else if((IPparams.isBias) && (IPparams.constIdx == 1))
              {
                if(numBChannels > 1)
                {
                  sum = pBiasData[(chIdx * numOutCols) + outColIdx];
                }
                else
                {
                  /*Broadcast situation*/
                  sum = pBiasData[outColIdx];
                }
              }
              else
              {
                sum = 0;
              }
              if(&inPtr[inABatchPitch*batchIdx + inADIM1Pitch*dim1Idx + inADIM2Pitch*dim2Idx + (chIdx * inAChPitch)] != NULL)
              {
                OPENACC(loop seq)
                for (int32_t inColIdx = 0; inColIdx < numInCols; inColIdx++)
                {
                  /* Multiply inputA nodes with input B nodes and accumulate */
                  if(inBPtr != NULL)
                  {
                    if(inputATranspose != 0)/* transpose needed for inputA */
                    {
                      sum += inPtr[inABatchPitch*batchIdx + inADIM1Pitch*dim1Idx + inADIM2Pitch*dim2Idx + (chIdx * inAChPitch) + (inColIdx * inALinePitch) + outColIdx ] * inBPtr[batchIdx * inBBatchPitch + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch) + ((inColIdx * inBLinePitch) + outColIdx)];/* Not tested*/
                    }
                    else if (inputBTranspose != 0) /* transpose needed for inputB */
                    {
                      sum += inPtr[inABatchPitch*batchIdx + inADIM1Pitch*dim1Idx + inADIM2Pitch*dim2Idx + (chIdx * inAChPitch) + (inALinePitch * outRwIdx) + inColIdx] * inBPtr[batchIdx * inBBatchPitch + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch) + (outColIdx * inBLinePitch) + inColIdx];
                    }
                    else/* no transpose needed */
                    {
                      sum += inPtr[ inABatchPitch*batchIdx + inADIM1Pitch*dim1Idx + inADIM2Pitch*dim2Idx + (chIdx * inAChPitch) + (inALinePitch * outRwIdx) + inColIdx] * inBPtr[batchIdx * inBBatchPitch + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch) + ((inColIdx * inBLinePitch) + outColIdx)];
                    }
                  }
                  else
                  {
                    if(inputATranspose != 0)/* transpose needed for inputA */
                    {
                      sum += inPtr[inABatchPitch*batchIdx + inADIM1Pitch*dim1Idx + inADIM2Pitch*dim2Idx + (chIdx * inAChPitch) + (inColIdx * inALinePitch) + outColIdx ] * weightsPtr[batchIdx * inBBatchPitch + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch) + ((inColIdx * inBLinePitch) + outColIdx)];/* Not tested*/
                    }
                    else if (inputBTranspose != 0) /* transpose needed for inputB */
                    {
                      sum += inPtr[inABatchPitch*batchIdx + inADIM1Pitch*dim1Idx + inADIM2Pitch*dim2Idx + (chIdx * inAChPitch) + (inALinePitch * outRwIdx) + inColIdx] * weightsPtr[batchIdx * inBBatchPitch + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch) + (outColIdx * inBLinePitch) + inColIdx];
                    }
                    else/* no transpose needed */
                    {
                      sum += inPtr[inABatchPitch*batchIdx + inADIM1Pitch*dim1Idx + inADIM2Pitch*dim2Idx + (chIdx * inAChPitch) + (inALinePitch * outRwIdx) + inColIdx] * weightsPtr[batchIdx * inBBatchPitch + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch) + ((inColIdx * inBLinePitch) + outColIdx)];
                    }
                  }
                }
                min = (sum < min) ? sum : min;
                max = (sum > max) ? sum : max;
                accPtr[(batchIdx*outBatchPitch) + (dim1Idx*outDIM1Pitch) + (dim2Idx*outDIM2Pitch) + (chIdx * outChPitch) + (outRwIdx * outLinePitch) + outColIdx] = sum;
              }
            }
          }
        }
      }
    }
  }
  }
  #else
  #ifdef BUILD_WITH_CUDA
  status = TIDL_cudaInnerProduct<Tin, TBin, Tw, Tb, Tout, Tacc>(
    inPtr, inBPtr, accPtr, pBiasData, weightsPtr,
    batches, dim1, dim2, channels,
    numInCols, numOutCols, numInRows, numOutRows,
    inAChPitch, inALinePitch, inADIM1Pitch, inADIM2Pitch, inABatchPitch,
    inBChPitch, inBLinePitch, inBDIM1Pitch, inBDIM2Pitch, inBBatchPitch,
    outChPitch, outLinePitch, outDIM1Pitch, outDIM2Pitch, outBatchPitch,
    inputBTranspose, IPparams.isBias, IPparams.constIdx
  );
  #else
  for (uint32_t batchIdx = 0; batchIdx < (uint32_t)batches; batchIdx++)
  {
    for (uint32_t dim1Idx = 0; dim1Idx < (uint32_t)dim1; dim1Idx++)
    {
      for (uint32_t dim2Idx = 0; dim2Idx < (uint32_t)dim2; dim2Idx++)
      {
        for (uint32_t chIdx = 0; chIdx < (uint32_t)channels; chIdx++)
        {
          for (uint32_t outRwIdx = 0; outRwIdx < numOutRows; outRwIdx++)
          {
            for (uint32_t outColIdx = 0; outColIdx < numOutCols; outColIdx++)
            {
              if(status == TIDL_SUCCESS){
              uint32_t inOffset = (inABatchPitch*batchIdx) + (inADIM1Pitch*dim1Idx) + (inADIM2Pitch*dim2Idx) + (chIdx * inAChPitch);
              uint32_t biasOffset = (batchIdx * inBBatchPitch) + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch);
              Tacc sum = 0;
              if ((pBiasData != NULL))
              {
                if((IPparams.isBias != 0) && (IPparams.constIdx == -1))
                {
                  sum = pBiasData[(outRwIdx*numOutCols) + outColIdx];
                }
                else if((IPparams.isBias != 0) && (IPparams.constIdx == 0))
                {
                  sum = pBiasData[outRwIdx];
                }
                else if((IPparams.isBias != 0) && (IPparams.constIdx == 1))
                {
                  if(numBChannels > 1)
                  {
                    sum = pBiasData[(chIdx * numOutCols) + outColIdx];
                  }
                  else
                  {
                    /*Broadcast situation*/
                    sum = pBiasData[outColIdx];
                  }
                }
                else
                {
                  sum = 0;
                }
              }
              if(inPtr != NULL) {
                /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  <justification end>
                */
                if(&inPtr[inOffset] != NULL)
                /* LDRA_JUSTIFY_END */
                {
                  /* Multiply inputA nodes with input B nodes and accumulate */
                  if (inBPtr != NULL)
                  {
                    #ifdef HOST_EMULATION
                    #if defined TIDL_COVERAGE_DEAD_CODE
                    if (inputATranspose != 0) /* transpose needed for inputA */
                    {
                      inOffset = inOffset + outColIdx;
                      biasOffset = biasOffset + outColIdx;
                      for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++)
                      {
                        sum += inPtr[inOffset + (inColIdx * inALinePitch)] * inBPtr[biasOffset + ((inColIdx * inBLinePitch))]; /* Not tested*/
                      }
                    }
                    else if (inputBTranspose != 0) /* transpose needed for inputB */
                    #else
                    if (inputBTranspose != 0) /* transpose needed for inputB */
                    #endif
                    {
                      inOffset = inOffset + (inALinePitch * outRwIdx);
                      biasOffset = biasOffset + (outColIdx * inBLinePitch);
                      for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++)
                      {
                        sum += inPtr[inOffset + inColIdx] * inBPtr[biasOffset + inColIdx];
                      }
                    }
                    else /* no transpose needed */
                    #endif
                    {
                      inOffset = inOffset + (inALinePitch * outRwIdx);
                      biasOffset = biasOffset + outColIdx;
                      for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++)
                      {
                        sum += inPtr[inOffset + inColIdx] * inBPtr[biasOffset + (inColIdx * inBLinePitch)];
                      }
                    }
                  }
                  else
                  {
                    #if defined TIDL_COVERAGE_DEAD_CODE
                    if (inputATranspose != 0) /* transpose needed for inputA */
                    {
                      inOffset = inOffset + outColIdx;
                      biasOffset = biasOffset + outColIdx;
                      for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++)
                      {
                        sum += inPtr[inOffset + (inColIdx * inALinePitch)] * weightsPtr[biasOffset + (inColIdx * inBLinePitch)]; /* Not tested*/
                      }
                    }
                    else if (inputBTranspose != 0) /* transpose needed for inputB */
                    #else
                    if (inputBTranspose != 0) /* transpose needed for inputB */
                    #endif
                    {
                      inOffset = inOffset + (inALinePitch * outRwIdx);
                      biasOffset = biasOffset + (outColIdx * inBLinePitch);
                      for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++)
                      {
                        sum += inPtr[inOffset + inColIdx] * weightsPtr[biasOffset + inColIdx];
                      }
                    }
                    else /* no transpose needed */
                    {
                      inOffset = inOffset + (inALinePitch * outRwIdx);
                      biasOffset = biasOffset + outColIdx;
                      for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++)
                      {
                        sum += inPtr[inOffset + inColIdx] * weightsPtr[biasOffset + (inColIdx * inBLinePitch)];
                      }
                    }
                  }
                  min = (sum < min) ? sum : min;
                  max = (sum > max) ? sum : max;
                  accPtr[(batchIdx * outBatchPitch) + (dim1Idx * outDIM1Pitch) + (dim2Idx * outDIM2Pitch) + (chIdx * outChPitch) + (outRwIdx * outLinePitch) + outColIdx] = sum;
                }
              }
              else {
                TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_INPTR_NULL);
                status = TIDL_ERR_FAILURE;
              }
              }
            }
          }
        }
      }
    }
  }
  #endif
#endif
    if (status == TIDL_SUCCESS)
    {
      if ((TIDL_isAsymQuantEnabledTFL(quantizationStyle) == FALSE) &&
          (TIDL_isKernelHighPrecision(net->TIDLLayers[layerIdx].layerKernelType) == (int32_t)FALSE))
      {
#ifdef HOST_EMULATION
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
#endif
        roundBits = net->TIDLLayers[layerIdx].outData.roundBits;
        mmaScale = 1;
      }
      else if (TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE)
      {
        roundBits = buffParams->mmaShift;
        mmaScale = buffParams->mmaScale;
      }
      else
      {
        /*do nothing*/
      }

      int32_t isHighPrecision = TIDL_isKernelHighPrecision(net->TIDLLayers[layerIdx].layerKernelType) == (int32_t)TRUE;
      uint8_t *mmav2_Scales = (uint8_t *)get_int8_t_pointer((int8_t *)(net), net->TIDLLayers[layerIdx].layerParams.innerProductParams.derivedScales);
      uint8_t *mmav2_Shifts = (uint8_t *)get_int8_t_pointer((int8_t *)(net), net->TIDLLayers[layerIdx].layerParams.innerProductParams.derivedShifts);
      int64_t tempAcc;
      uint32_t chIdx1, outIdx;
#ifdef HOST_EMULATION
      if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_ROUNDSAT_MMA_ZERO_SHIFT_BITS)
      {
        mmav2_Shifts[0] = 0;
      }
#endif

  #ifdef BUILD_WITH_OPENACC
  int32_t mmav2_Scales_sz, mmav2_Shifts_sz;
  if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat)
  {
    mmav2_Scales_sz=0;
    mmav2_Shifts_sz=0;
  }
  else if (isHighPrecision)
  {
    mmav2_Scales_sz = numBChannels * numOutCols;
    mmav2_Shifts_sz = numBChannels * numOutCols;
  }
  else
  {
    mmav2_Scales_sz=0;
    mmav2_Shifts_sz=0;
  }
  OPENACC(data copyin(accPtr[0: 1 + outBatchPitch*(batches-1) + outDIM1Pitch*(dim1-1) + outDIM2Pitch*(dim2-1) + outChPitch*(channels-1) + (numInRows * numOutCols - 1)], \
                           mmav2_Scales[0:mmav2_Scales_sz], \
                           mmav2_Shifts[0:mmav2_Shifts_sz], \
                           tidlLayer[0:1]) \
                copyout(outPtr[0: 1 + outBatchPitch*(batches-1) + outDIM1Pitch*(dim1-1) + outDIM2Pitch*(dim2-1) + outChPitch*(channels-1) + (numInRows * numOutCols - 1)]))
  {
  OPENACC(parallel loop collapse(5))
  for (int32_t batchIdx = 0; batchIdx < batches; batchIdx++)
    {
      for (int32_t dim1Idx = 0; dim1Idx < dim1; dim1Idx++)
      {
        for (int32_t dim2Idx = 0; dim2Idx < dim2; dim2Idx++)
        {
          for (chIdx1 = 0; chIdx1 < channels; chIdx1++)
          {
            for (outIdx = 0; outIdx < (numInRows * numOutCols); outIdx++)
            {
              Tacc  sum;
              sum = accPtr[(batchIdx*outBatchPitch) + (dim1Idx*outDIM1Pitch) + (dim2Idx*outDIM2Pitch) + (chIdx1 * outChPitch) + outIdx];
              if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat)
              {
                OPENACC(routine(TIDL_floatSat))
                sum = TIDL_floatSat(sum, tidlLayer);
              }
              else if (isHighPrecision != 0)
              {
                int32_t i6 = outIdx % numOutCols + chIdx1 * numOutCols;
                if(numBChannels == 1)
                {
                  i6 = outIdx % numOutCols;
                }
                tempAcc = (int64_t)sum * mmav2_Scales[i6];
                OPENACC(routine(TIDL_roundSatMMA))
                sum  = (Tacc)TIDL_roundSatMMA(tempAcc, mmav2_Shifts[i6], satLow, satHigh);
              }
              else
              {
                tempAcc = sum * mmaScale;
                OPENACC(routine(TIDL_roundSat))
                sum = (Tacc)TIDL_roundSat((int64_t)tempAcc, roundBits, satLow, satHigh);
              }
              outPtr[(batchIdx*outBatchPitch) + (dim1Idx*outDIM1Pitch) + (dim2Idx*outDIM2Pitch) + (chIdx1 * outChPitch) + outIdx] = sum;
            }
          }
        }
      }
    }
  }
  #else
  #ifdef BUILD_WITH_CUDA
  status = TIDL_cudaInnerProductSaturation<Tout, Tacc>(
    accPtr, outPtr,
    batches, dim1, dim2, channels, numOutRows, numOutCols,
    outChPitch, outLinePitch, outDIM1Pitch, outDIM2Pitch, outBatchPitch,
    tidlLayer -> outData.elementType == TIDL_SinglePrecFloat, isHighPrecision,
    roundBits, mmaScale,
    satLow, satHigh,
    mmav2_Scales, mmav2_Shifts
  );
  #else
   for (uint32_t batchIdx1 = 0; batchIdx1 < (uint32_t)batches; batchIdx1++)
    {
      for (uint32_t dim1Idx = 0; dim1Idx < (uint32_t)dim1; dim1Idx++)
      {
        for (uint32_t dim2Idx = 0; dim2Idx < (uint32_t)dim2; dim2Idx++)
        {
          for (chIdx1 = 0; chIdx1 < (uint32_t)channels; chIdx1++)
          {
            uint32_t offset = (batchIdx1*outBatchPitch) + (dim1Idx*outDIM1Pitch) + (dim2Idx*outDIM2Pitch) + (chIdx1 * outChPitch) ;
            Tacc sum;
            #ifdef HOST_EMULATION
            if (tidlLayer -> outData.elementType == TIDL_SinglePrecFloat)
            {
              OPENACC(routine(TIDL_floatSat))
              for (outIdx = 0; outIdx < (numInRows * numOutCols); outIdx++)
              {
                  sum = accPtr[offset + outIdx];
                  sum = TIDL_floatSat(sum, tidlLayer);
                  outPtr[offset + outIdx] = sum;
              }
            }
            else 
            #endif
            if (isHighPrecision != 0)
            {
              OPENACC(routine(TIDL_roundSatMMA))
              for (outIdx = 0; outIdx < (numInRows * numOutCols); outIdx++)
              {
                  int32_t i6 = outIdx % numOutCols + chIdx1 * numOutCols;
                  if(numBChannels == 1)
                  {
                    i6 = outIdx % numOutCols;
                  }
                  sum = accPtr[offset + outIdx];
                  tempAcc = (int64_t)sum * (int64_t)mmav2_Scales[i6];
                  sum = (Tacc)TIDL_roundSatMMA(tempAcc, mmav2_Shifts[i6], satLow, satHigh);
                  outPtr[offset + outIdx] = sum;
                }
              }
              else
              {
                OPENACC(routine(TIDL_roundSat))
                for (outIdx = 0; outIdx < (numInRows * numOutCols); outIdx++)
                {
                  sum = accPtr[offset + outIdx];
                  tempAcc = sum * mmaScale;
                  sum = (Tacc)TIDL_roundSat((int64_t)tempAcc, roundBits, satLow, satHigh);
                  outPtr[offset + outIdx] = sum;
              }
            }
          }
        }
      }
    }
    #endif
 #endif
  }
  }
  return status;
}
/**
 * @brief  This funciton handles differnt input bit-depths for reference innerProduct
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param inPtr : Input pointer on which innerProduct is applied
 * @param refPtr : Output pointer after innerProduct opreation
 * @param biasPtr  : Pointer to the Bias values
 * @param weightsPtr : Pointer to weights buffer
 * @param accPtr : pointer to the accumulator buffer
 * @param outElementType : elementType of the output
 * @param buffParams : parameters of the innerProduct buffer
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tw, class Tb, class Tacc>
    int32_t TIDL_refInnerProductParamBitDepth(
        TIDL_Handle intAlgHandle,
        int32_t layerIdx,
        void *inPtr,
        const void *inBPtr,
        void *refPtr,
        Tb *biasPtr,
        const Tw *weightsPtr,
        Tacc *accPtr,
        int32_t outElementType,
        tidlInnerProductBuffParams_t *buffParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t satLow, sathigh;
  int32_t quantizationStyle = intAlgHandle->createParams->net->quantizationStyle;
#ifdef HOST_EMULATION
  if (outElementType == TIDL_SinglePrecFloat)
  {
    sTIDL_Layer_t *tidlLayer = &intAlgHandle->createParams->net->TIDLLayers[layerIdx];
    float32_tidl minFloat, maxFloat;
    TIDL_getSaturationFloat(tidlLayer, &minFloat, &maxFloat);
    status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (float32_tidl *)inPtr, (float32_tidl *)inBPtr, (float32_tidl *)refPtr, biasPtr, weightsPtr, buffParams, minFloat, maxFloat);
  }
  else
#endif
  {
    if (inBPtr == NULL)
    {
      if (sizeof(Tw) == 1U)
      {
        buffParams->inBElementType = TIDL_SignedChar;
      }
      else /*if(sizeof(Tw) == 2U)*/
      {
        buffParams->inBElementType = TIDL_SignedShort;
      }
    }

    if ((TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE))
    {
      satLow = TIDL_SAT_LO_INT8;
      sathigh = TIDL_SAT_HI_INT8;
      status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (int8_t *)inBPtr, (int8_t *)refPtr,
                                    biasPtr, weightsPtr, buffParams, satLow, sathigh);
    }

    if ((TIDL_isAsymQuantEnabledTFL(quantizationStyle) == FALSE) && (status == TIDL_SUCCESS))
    {
      if ((outElementType == TIDL_SignedChar) ||
          (outElementType == TIDL_SignedShort))
      {
        if (TIDL_getDatElementSize(outElementType) == 1)
        {
          satLow = TIDL_SAT_LO_INT8;
          sathigh = TIDL_SAT_HI_INT8;
        }
        else
        {
          satLow = TIDL_SAT_LO_INT16;
          sathigh = TIDL_SAT_HI_INT16;
        }
        if ((buffParams->inElementType == TIDL_SignedChar) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (int8_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (uint8_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedChar) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (int8_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (uint8_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
#if INNER_PRODUCT_MIXED_PRECISION
        else if ((buffParams->inElementType == TIDL_SignedChar) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (int8_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (uint8_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedChar) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (int8_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (uint8_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_SignedShort) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (int16_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (uint16_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedShort) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (int16_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (uint16_t *)inBPtr, (int8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
#endif /* End of "INNER_PRODUCT_MIXED_PRECISION". */
        else if ((buffParams->inElementType == TIDL_SignedShort) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (int16_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (uint16_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /*tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    ");*/
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedShort) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (int16_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (uint16_t *)inBPtr, (int16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /*tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    ");*/
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else
        {
          TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
          status = IALG_EFAIL;
        }
      }
      else /* Unsigned output */
      {

        if (TIDL_getDatElementSize(outElementType) == 1)
        {
          satLow = 0;
          sathigh = TIDL_SAT_HI_UINT8;
        }
        else
        {
          satLow = 0;
          sathigh = TIDL_SAT_HI_UINT16;
        }

        if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_INELEMENTTYPE_WORD)
        {
          buffParams->inElementType = TIDL_UnsignedWord;
        }

        if ((buffParams->inElementType == TIDL_SignedChar) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (int8_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (uint8_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedChar) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (int8_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (uint8_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
#if INNER_PRODUCT_MIXED_PRECISION
        else if ((buffParams->inElementType == TIDL_SignedChar) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (int8_t *)inBPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int8_t *)inPtr, (uint8_t *)inBPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedChar) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          if (buffParams->inBElementType == TIDL_SignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (int8_t *)inBPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedChar)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint8_t *)inPtr, (uint8_t *)inPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
#endif
        else if ((buffParams->inElementType == TIDL_SignedShort) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (int16_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (uint16_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedShort) && (TIDL_getDatElementSize(outElementType) == 1))
        {
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (int16_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (uint16_t *)inBPtr, (uint8_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_SignedShort) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          /* Singed Inputs and Unsigned output is not supported */
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (int16_t *)inBPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (int16_t *)inPtr, (int16_t *)inBPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else if ((buffParams->inElementType == TIDL_UnsignedShort) && (TIDL_getDatElementSize(outElementType) == 2))
        {
          if (buffParams->inBElementType == TIDL_SignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (int16_t *)inBPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else if (buffParams->inBElementType == TIDL_UnsignedShort)
          {
            status = TIDL_refInnerProduct(intAlgHandle, layerIdx, accPtr, (uint16_t *)inPtr, (uint16_t *)inBPtr, (uint16_t *)refPtr,
                                          biasPtr, weightsPtr, buffParams, satLow, sathigh);
          }
          else
          {
            /* tidl_printf(0,"Unsupported data type for inBData for IP layer exiting...\n    "); */
            TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
            status = IALG_EFAIL;
          }
        }
        else
        {
          TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE);
          status = IALG_EFAIL;
        }
      }
    }
  }
  return status;
}
/**
 * @brief InnerProduct layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of innerProduct layer parameters
 * @param buffParams : parameters of the innerProduct buffer
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_innerProductRefProcess(
    TIDL_Handle intAlgHandle,
    sTIDL_AlgLayer_t *algLayer,
    sTIDL_Layer_t *tidlLayer,
    sTIDL_InnerProductParams_t *params,
    tidlInnerProductBuffParams_t *buffParams,
    void *inPtr,
    void *inBPtr,
    void *outPtr)
{
  int32_t i0, status = IALG_EOK;

  sTIDL_Network_t *net = intAlgHandle->createParams->net;
  int32_t quantizationStyle = intAlgHandle->createParams->net->quantizationStyle;
  int32_t layerIdx = algLayer->layerIdx;
  void *accPtr = NULL;
  void *weightPtr = ((int8_t *)(intAlgHandle->createParams->net) + params->weights);
  void *orgBiasPtr = ((int8_t *)(intAlgHandle->createParams->net) + params->bias);
  void *biasPtr = NULL;

  if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_DATATYPE_64B) /* Forcing tidlLayer->weightsElementSizeInBits = 64 to check of invalid dataType */
  {
    tidlLayer->weightsElementSizeInBits = 64;
  }
  if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_BIASPTR_NULL) /* Forcing biasptr to zero for invalid case to hit */
  {
    algLayer->layerParams.innerProductParams.biasParamSize = 0;
  }
  if (TIDL_forceInnerProductRef(intAlgHandle->createParams, tidlLayer) != 0)
  {
    // Forced Refrence case. in that case the space to hold weights is from algLayer
    weightPtr = algLayer->layerParams.innerProductParams.wtTranformMem;
    orgBiasPtr = (void *)get_int8_t_pointer((int8_t *)(weightPtr), params->bias - params->weights);
    if (tidlLayer->numInBufs == 1)
    {
      inBPtr = NULL;
    }
  }
#ifdef HOST_EMULATION
  float32_tidl *orgBiasPtrFloat = (float32_tidl *)orgBiasPtr;
#endif
  int32_t biasSize = 0;
  int32_t scaleSize = 0;
  int32_t shiftSize = 0;
  TIDL_innerProductGetBiasScaleSize(&tidlLayer->outData,
                                   &tidlLayer->layerParams.innerProductParams, 
                                   tidlLayer->layerKernelType,
                                   &biasSize, &scaleSize, &shiftSize);
  if(params->isBias != 0)
  {
    if (algLayer->layerParams.innerProductParams.biasParamSize >= (biasSize))
    {
      biasPtr = (void *)algLayer->layerParams.innerProductParams.biasParamMem;
    }
    else
    {
      /* tidl_printf(0,"Memory for  biasptr  is not sufficient exiting...\n    "); */
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_INNERPROD, TIDL_ERROR_INNERPROD_INSUFFICIENT_MEM_BIAS);
      status = IALG_EFAIL;
    }
  }

  if (status == IALG_EOK)
  {
    #ifdef BUILD_WITH_CUDA
    CUDNNLC = CUDA_INNER_PRODUCT_COUNTER++;
    #endif
    if ((TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE))
    {
      int32_t *fBiasPtr = (int32_t *)algLayer->layerParams.innerProductParams.biasParamMem; // Txed bias
      status = TIDL_refInnerProductParamBitDepth(intAlgHandle, layerIdx, inPtr, inBPtr, outPtr, (int32_t *)fBiasPtr, (int8_t *)weightPtr, (int32_t *)accPtr, tidlLayer->outData.elementType, buffParams);
    }
    else
    {
      if ((tidlLayer->weightsElementSizeInBits <= 8))
      {
        if (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE)
        {
          /*Set bias ptr:*/
          int32_t *fBiasPtr = (int32_t *)get_int8_t_pointer((int8_t *)(net), net->TIDLLayers[layerIdx].layerParams.innerProductParams.derivedBias);
          status = TIDL_refInnerProductParamBitDepth(intAlgHandle, layerIdx, inPtr, inBPtr, outPtr, (int32_t *)fBiasPtr, (int8_t *)weightPtr, (int32_t *)accPtr, tidlLayer->outData.elementType, buffParams);
        }
        else
        {
          if ((params->isBias != 0) && (biasPtr != NULL))
          {
#if defined TIDL_COVERAGE_DEAD_CODE
            /* biasScale or scalePtr != -1 refer to tidl_import_quantize.cpp:3892 & TIDL_importQuantLayerParams function */
            if (params->biasScale != -1.0f)
            {
              TIDL_conv2dBiasSplit((int16_t *)orgBiasPtr, (int16_t *)biasPtr, &buffParams->biasB, scaleSize, buffParams->inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16, 0xFF, TIDL_getDatElementSign(buffParams->inElementType));
            }
#endif
            TIDL_conv2dBiasSplit((int16_t *)orgBiasPtr, (int16_t *)biasPtr, &buffParams->biasB, scaleSize, buffParams->inScaleFactor, TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16, 0xFF, TIDL_getDatElementSign(buffParams->inElementType));
            for (i0 = 0; i0 < scaleSize; i0++)
            {
              ((int16_t *)biasPtr)[i0] = ((int16_t *)biasPtr)[i0] * buffParams->biasB;
            }
          }
          status = TIDL_refInnerProductParamBitDepth(intAlgHandle, layerIdx, inPtr, inBPtr, outPtr, (int16_t *)biasPtr, (int8_t *)weightPtr, (int32_t *)accPtr, tidlLayer->outData.elementType, buffParams);
        }
      }
      else if ((tidlLayer->weightsElementSizeInBits <= 16))
      {
        if (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE)
        {
          /*Set bias ptr:*/
          int64_t *fBiasPtr = (int64_t *)get_int8_t_pointer((int8_t *)(net), net->TIDLLayers[layerIdx].layerParams.innerProductParams.derivedBias);
          status = TIDL_refInnerProductParamBitDepth(intAlgHandle, layerIdx, inPtr, inBPtr, outPtr, (int64_t *)fBiasPtr, (int16_t *)weightPtr, (int64_t *)accPtr, tidlLayer->outData.elementType, buffParams);
        }
        else
        {
          if ((params->isBias != 0) && (biasPtr != NULL))
          {
#if defined TIDL_COVERAGE_DEAD_CODE
            /* biasScale or scalePtr != -1 refer to tidl_import_quantize.cpp:3892 & TIDL_importQuantLayerParams function */
            if (params->biasScale != -1.0f)
            {
              TIDL_conv2dBiasSplit((int16_t *)orgBiasPtr, (int32_t *)biasPtr, &buffParams->biasB, scaleSize, buffParams->inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32, 0xFFFF, TIDL_getDatElementSign(buffParams->inElementType));
            }
#endif
            TIDL_conv2dBiasSplit((int16_t *)orgBiasPtr, (int32_t *)biasPtr, &buffParams->biasB, scaleSize, buffParams->inScaleFactor, TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32, 0xFFFF, TIDL_getDatElementSign(buffParams->inElementType));
            for (i0 = 0; i0 < scaleSize; i0++)
            {
              ((int32_t *)biasPtr)[i0] = ((int32_t *)biasPtr)[i0] * buffParams->biasB;
            }
          }

          if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_INNERPRODUCT_FORCE_INVALID_INBDATA_DATATYPE) /* forcing inBdata data type as invalid value (32-bit) */
          {
            buffParams->inBElementType = 5;
          }
          status = TIDL_refInnerProductParamBitDepth(intAlgHandle, layerIdx, inPtr, inBPtr, outPtr, (int32_t *)biasPtr, (int16_t *)weightPtr, (int64_t *)accPtr, tidlLayer->outData.elementType, buffParams);
        }
      }
#ifdef HOST_EMULATION
      else if (tidlLayer->weightsElementSizeInBits == 32)
      {
        status = TIDL_refInnerProductParamBitDepth(intAlgHandle, layerIdx, inPtr, inBPtr, outPtr, orgBiasPtrFloat, (float32_tidl *)weightPtr, (float32_tidl *)accPtr, tidlLayer->outData.elementType, buffParams);
      }
#endif
      else
      {
        status = TIDL_ERR_INVALID_TYPE;
      }
    }
  #ifdef BUILD_WITH_CUDA
  /*Mark init as completed to prevent re-allocation of buffers for subsequent frames:*/
  TIDL_cudaSetInnerProductInitFlag(CUDNNLC);
  #endif
  }
  TIDL_L1DandL2CacheWbInv();
  return status;
}
