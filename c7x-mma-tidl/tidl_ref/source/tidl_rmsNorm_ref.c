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
 *  \file tidl_rmsNorm_ref.c
 *
 *  \brief RMS norm process calls & ref implementation
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include <math.h>
#include <limits>
#include "tidl_alg_utils.h"
#include "tidl_rmsNorm.h"
#include "tidl_rmsNorm_ref.h"
#include "tidl_forceNegativeTest.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define SCALE_PRECISION_BITS 8
/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static void TIDL_updateRmsNormMemorySizes(const TIDL_LayerSpecificParams *layerSpecificParams,
                                         const TIDL_NetworkCommonParams *commonParams,
                                         int32_t layerIdx,
                                         int32_t *scratchDataSizeOut,
                                         int32_t *outDataSizeOut,
                                         int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t outBatchPitch;
  int32_t scratchDataSize;
  int32_t paramMemSize = 0;
  int32_t outDataSize = TIDL_refGetOutDataSize(commonParams, layerSpecificParams, layerIdx);
  sTIDL_DataParams_t *dataParams = TIDL_getDataParams(commonParams->net,
                                                      commonParams->net->TIDLLayers[layerIdx].inData[0]);

  outBatchPitch = commonParams->net->TIDLLayers[layerIdx].outData.pitch[TIDL_ROI_PITCH];
  scratchDataSize = commonParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_BATCH] * outBatchPitch * sizeof(float32_tidl);

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

int32_t TIDL_rmsNormRefAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            int32_t layerIdx,
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;

  TIDL_updateRmsNormMemorySizes(layerSpecificParams, commonParams, layerIdx,
                               NULL, NULL, memorySize);

  return status;
}

int32_t TIDL_rmsNormRefInit(const TIDL_LayerSpecificParams *layerSpecificParams,
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

  TIDL_updateRmsNormMemorySizes(layerSpecificParams, commonParams, layerIdx,
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


/**
 * @brief This is main function perform RMS normalization on different elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @tparam typeExEx2 : template for accumulator
 * @param inPtr : Input pointer on which rmsNorm is applied
 * @param outPtr : Output pointer after rmsNorm operation
 * @param intAlgHandle : tidl algorithm handle
 * @param layerIdx : index of the current layer
 * @param params : copy of rmsNorm parameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @return  TIDL_SUCCESS - Successful
 *          TIDL_ERR_FAILURE - Unspecified error
 */
template<class Tin, class Tout, class typeExEx2> static int32_t TIDL_refRmsNormCore(Tin *inPtr,
                                                                                   Tout *outPtr,
                                                                                   TIDL_Handle intAlgHandle,
                                                                                   int32_t layerIdx,
                                                                                   sTIDL_RMSNormParams_t *params,
                                                                                   sTIDL_AlgLayer_t *algLayer,
                                                                                   const sTIDL_DataParams_t *inDataParams,
                                                                                   const sTIDL_DataParams_t *outDataParams)
{

  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t icnt[TIDL_DIM_MAX] = {0};
  int32_t dim[TIDL_DIM_MAX] = {0};
  int32_t outDim[TIDL_DIM_MAX] = {0};
  int32_t inVolume = 1;
  int32_t meanOffset = 0;
  int32_t inOffset = 0;
  int32_t outOffset = 0;
  int32_t outZeroPoint = outDataParams->tensorZeroPoint;
  float32_tidl inScale = inDataParams->tensorScale;
  float32_tidl outScale = outDataParams->tensorScale;
  float32_tidl denom, inp_float, ep, inp_sqrt;
  float32_tidl epsilon = params->epsilon;
  uint8_t scaleAvg, shiftAvg;
  int32_t shiftDenom, scaleDenom, absShift;

  for (i0 = 0; i0 < (TIDL_DIM_MAX - 1); i0++)
  {
    icnt[i0] = inDataParams->dimValues[i0];
    inVolume *= icnt[i0];
    dim[i0] = inDataParams->pitch[i0];
    outDim[i0] = outDataParams->pitch[i0];
  }
  icnt[TIDL_DIM_MAX - 1] = inDataParams->dimValues[TIDL_DIM_MAX - 1];
  inVolume *= icnt[TIDL_DIM_MAX - 1];

  Tin *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;
  int32_t width = icnt[5];
  int32_t height = icnt[4];
  int32_t channel = icnt[3];
  int32_t dim2 = icnt[2];
  int32_t dim1 = icnt[1];
  int32_t batch = icnt[0];

  int32_t axis = params->axis;

  /*Scratch required for E(x^2)*/
  int32_t scratchSizeRequired = 0;

  /* Calculate scratch size based on the normalization axis */
  switch (axis)
  {
    case TIDL_DIM_DIM1:   /* i1 - dim1 */
      scratchSizeRequired = icnt[0] * icnt[2] * icnt[3] * icnt[4] * icnt[5] * (int32_t)sizeof(typeExEx2);
      break;
    case TIDL_DIM_DIM2:   /* i2 - dim2 */
      scratchSizeRequired = icnt[0] * icnt[1] * icnt[3] * icnt[4] * icnt[5] * (int32_t)sizeof(typeExEx2);
      break;
    case TIDL_DIM_NUMCH:   /* i3 - numch (alias for channel) */
      scratchSizeRequired = icnt[0] * icnt[1] * icnt[2] * icnt[4] * icnt[5] * (int32_t)sizeof(typeExEx2);
      break;
    case TIDL_DIM_HEIGHT:  /* i4 - height */
      scratchSizeRequired = icnt[0] * icnt[1] * icnt[2] * icnt[3] * icnt[5] * (int32_t)sizeof(typeExEx2);
      break;
    case TIDL_DIM_WIDTH:   /* i5 - width */
      scratchSizeRequired = icnt[4] * (int32_t)sizeof(typeExEx2);
      break;
  }

  int32_t minValueOutput = 0;
  minValueOutput = std::numeric_limits<Tout>::lowest();
  int32_t maxValueOutput = 0;
  maxValueOutput = std::numeric_limits<Tout>::max();
  int32_t minValueAcc = 0;
  minValueAcc = std::numeric_limits<typeExEx2>::lowest();
  int32_t maxValueAcc = 0;
  maxValueAcc = std::numeric_limits<typeExEx2>::max();

  typeExEx2 *eX2;
  int64_t acceX2, inp;
  int64_t tempAcc;
  int32_t tempAccOut;

  if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_LAYERNORM_SCRATCH_SIZE_ERROR)
  {
    algLayer->scratchSize = 0;
  }
  if (algLayer->scratchSize >= scratchSizeRequired)
  {
    eX2 = (typeExEx2 *)algLayer->scratchMem;
  }
  else
  {
    status = TIDL_ERROR_RMSNORM_INSUFFICIENT_REF_SCRATCH;
  }

  if (status == TIDL_SUCCESS)
  {
    if (axis == TIDL_DIM_DIM1)
    {
      /*Compute Average:*/
      float32_tidl avg = 1.0 / ((float)width * (float)height * (float)channel * (float)dim2 * (float)dim1);
      TIDL_getMMAv2_ScaleAndShift(avg, &scaleAvg, &shiftAvg);

      for(i0 = 0; i0 < icnt[0]; i0++)
      {
        meanOffset = i0;
        eX2[meanOffset] = 0;
        acceX2 = 0;
        /*Calculate mean of x^2 */
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  inp = inData[inOffset];
                  acceX2 += (inp * inp);
                }
              }
            }
          }
        }
        if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
          TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
          TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
        ))
        {
          eX2[meanOffset] = acceX2;
          inp = (width * height * channel * dim2 * dim1 * eX2[meanOffset]);
          inp_float = (int64_t)inp;
          ep = epsilon * (inScale * inScale * width * width * height * height * channel * channel * dim2 * dim2 * dim1 * dim1);
          inp_sqrt = inp_float + ep;
          /*Calculate denominator & produce output:*/
          denom = (float32_tidl)( __recip_sqrt(inp_sqrt));
          if((TIDL_UnsignedShort == ((int32_t)inDataParams->elementType)) || TIDL_SignedShort == ((int32_t)inDataParams->elementType))
          {
            /* Refine the reciprocal sqrt using Newton-Raphson method for better accuracy */
            float32_tidl y_approx = denom;
            denom = y_approx * (1.5f - 0.5f * inp_sqrt * y_approx * y_approx);
          }
          denom = outScale * denom;
        }
        else
        {
          if ((intAlgHandle->createParams->net->deviceName == TIDL_TDA4VM) ||
              (inDataParams->elementType == TIDL_UnsignedChar) ||
              (inDataParams->elementType == TIDL_UnsignedShort) ||
              ((outDataParams->tensorScale / sqrt(epsilon * inScale * inScale)) > (float)(std::numeric_limits<uint8_t>::max())))
          {
            eX2[meanOffset] = (typeExEx2)(((acceX2) / (icnt[5] * icnt[4] * icnt[3] * icnt[2] * icnt[1])));
          }
          else
          {
            tempAcc = ((int64_t)acceX2) * (int64_t)scaleAvg;
            eX2[meanOffset] = (typeExEx2)TIDL_roundSat((int64_t)tempAcc, shiftAvg, minValueAcc, maxValueAcc);
          }
          inp = eX2[meanOffset];
          if (inp <= 0)
          {
            inp = 0;
          }
          /*Calculate denominator & produce output:*/
          denom = (float32_tidl)(__recip_sqrt(inp + (epsilon * (inScale * inScale))));
          denom = outScale * denom;
        }
        TIDL_convertFloatToScaleAndShift(denom, &scaleDenom, &shiftDenom, SCALE_PRECISION_BITS);

        if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
          TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
          TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
        ))
        {
          for (i1 = 0; i1 < icnt[1]; i1++)
          {
            for (i2 = 0; i2 < icnt[2]; i2++)
            {
              for (i3 = 0; i3 < icnt[3]; i3++)
              {
                for (i4 = 0; i4 < icnt[4]; i4++)
                {
                  for (i5 = 0; i5 < icnt[5]; i5++)
                  {
                    inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                    outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                    meanOffset = i0;
                    float inp_f = __int_to_float(inData[inOffset]);
                    inp_f = ((float)width * (float)height * (float)channel * (float)dim2 * (float)dim1) * inp_f;
                    inp_f = inp_f * denom;
                    tempAccOut = __float_to_int(inp_f);
                    tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                    tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                    outData[outOffset] = (Tout)tempAccOut;
                  }
                }
              }
            }
          }
        }
        else
        {
          if (shiftDenom >= 0)
          {
            for (i1 = 0; i1 < icnt[1]; i1++)
            {
              for (i2 = 0; i2 < icnt[2]; i2++)
              {
                for (i3 = 0; i3 < icnt[3]; i3++)
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0;
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = (tempAccOut << shiftDenom);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
            }
          }
          else
          {
            for (i1 = 0; i1 < icnt[1]; i1++)
            {
              for (i2 = 0; i2 < icnt[2]; i2++)
              {
                for (i3 = 0; i3 < icnt[3]; i3++)
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      absShift = shiftDenom * -1;
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0;
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = ((tempAccOut + (1 << (absShift - 1))) >> absShift);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    
    else if (axis == TIDL_DIM_DIM2)
    {
      /*Compute Average:*/
      float32_tidl avg = 1.0 / ((float)width * (float)height * (float)channel * (float)dim2);
      TIDL_getMMAv2_ScaleAndShift(avg, &scaleAvg, &shiftAvg);

      for(i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          meanOffset = i0 + (i1 * icnt[0]);
          eX2[meanOffset] = 0;
          acceX2 = 0;
          /*Calculate mean of x^2 */
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  inp = inData[inOffset];
                  acceX2 += (inp * inp);
                }
              }
            }
          }
          if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
            TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
            TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
          ))
          {
            eX2[meanOffset] = acceX2;
            inp = (width * height * channel * dim2 * eX2[meanOffset]);
            inp_float = (int64_t)inp;
            ep = epsilon * (inScale * inScale * width * width * height * height * channel * channel * dim2 * dim2);
            inp_sqrt = inp_float + ep;
            /*Calculate denominator & produce output:*/
            denom = (float32_tidl)( __recip_sqrt(inp_sqrt));
            if((TIDL_UnsignedShort == ((int32_t)inDataParams->elementType)) || TIDL_SignedShort == ((int32_t)inDataParams->elementType))
            {
              /* Refine the reciprocal sqrt using Newton-Raphson method for better accuracy */
              float32_tidl y_approx = denom;
              denom = y_approx * (1.5f - 0.5f * inp_sqrt * y_approx * y_approx);
            }
            denom = outScale * denom;
          }
          else
          {
            if ((intAlgHandle->createParams->net->deviceName == TIDL_TDA4VM) ||
                (inDataParams->elementType == TIDL_UnsignedChar) ||
                (inDataParams->elementType == TIDL_UnsignedShort) ||
                ((outDataParams->tensorScale / sqrt(epsilon * inScale * inScale)) > (float)(std::numeric_limits<uint8_t>::max())))
            {
              eX2[meanOffset] = (typeExEx2)(((acceX2) / (icnt[5] * icnt[4] * icnt[3] * icnt[2])));
            }
            else
            {
              tempAcc = ((int64_t)acceX2) * (int64_t)scaleAvg;
              eX2[meanOffset] = (typeExEx2)TIDL_roundSat((int64_t)tempAcc, shiftAvg, minValueAcc, maxValueAcc);
            }
            inp = eX2[meanOffset];
            if (inp <= 0)
            {
              inp = 0;
            }
            /*Calculate denominator & produce output:*/
            denom = (float32_tidl)(__recip_sqrt(inp + (epsilon * (inScale * inScale))));
            denom = outScale * denom;
          }
          TIDL_convertFloatToScaleAndShift(denom, &scaleDenom, &shiftDenom, SCALE_PRECISION_BITS);

          if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
            TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
            TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
          ))
          {
            for (i2 = 0; i2 < icnt[2]; i2++)
            {
              for (i3 = 0; i3 < icnt[3]; i3++)
              {
                for (i4 = 0; i4 < icnt[4]; i4++)
                {
                  for (i5 = 0; i5 < icnt[5]; i5++)
                  {
                    inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                    outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                    meanOffset = i0 + (i1 * icnt[0]);
                    float inp_f = __int_to_float(inData[inOffset]);
                    inp_f = ((float)width * (float)height * (float)channel * (float)dim2) * inp_f;
                    inp_f = inp_f * denom;
                    tempAccOut = __float_to_int(inp_f);
                    tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                    tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                    outData[outOffset] = (Tout)tempAccOut;
                  }
                }
              }
            }
          }
          else
          {
            if (shiftDenom >= 0)
            {
              for (i2 = 0; i2 < icnt[2]; i2++)
              {
                for (i3 = 0; i3 < icnt[3]; i3++)
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0 + (i1 * icnt[0]);
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = (tempAccOut << shiftDenom);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
            }
            else
            {
              for (i2 = 0; i2 < icnt[2]; i2++)
              {
                for (i3 = 0; i3 < icnt[3]; i3++)
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      absShift = shiftDenom * -1;
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0 + (i1 * icnt[0]);
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = ((tempAccOut + (1 << (absShift - 1))) >> absShift);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    
    else if (axis == TIDL_DIM_NUMCH)
    {
      /*Compute Average:*/
      float32_tidl avg = 1.0 / ((float)width * (float)height * (float)channel);
      TIDL_getMMAv2_ScaleAndShift(avg, &scaleAvg, &shiftAvg);

      for(i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]);
            eX2[meanOffset] = 0;
            acceX2 = 0;
            /*Calculate mean of x^2 */
            for(i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  inp = inData[inOffset];
                  acceX2 += (inp * inp);
                }
              }
            }
            if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
              TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
              TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
            ))
            {
              eX2[meanOffset] = acceX2;
              inp = (width * height * channel * eX2[meanOffset]);
              inp_float = (int64_t)inp;
              ep = epsilon * (inScale * inScale * width * width * height * height * channel * channel);
              inp_sqrt = inp_float + ep;
              /*Calculate denominator & produce output:*/
              denom = (float32_tidl)( __recip_sqrt(inp_sqrt));
              if((TIDL_UnsignedShort == ((int32_t)inDataParams->elementType)) || TIDL_SignedShort == ((int32_t)inDataParams->elementType))
              {
                /* Refine the reciprocal sqrt using Newton-Raphson method for better accuracy */
                float32_tidl y_approx = denom;
                denom = y_approx * (1.5f - 0.5f * inp_sqrt * y_approx * y_approx);
              }
              denom = outScale * denom;
            }
            else
            {
              if ((intAlgHandle->createParams->net->deviceName == TIDL_TDA4VM) ||
                  (inDataParams->elementType == TIDL_UnsignedChar) ||
                  (inDataParams->elementType == TIDL_UnsignedShort) ||
                  ((outDataParams->tensorScale / sqrt(epsilon * inScale * inScale)) > (float)(std::numeric_limits<uint8_t>::max())))
              {
                eX2[meanOffset] = (typeExEx2)(((acceX2) / (icnt[5] * icnt[4] * icnt[3])));
              }
              else
              {
                tempAcc = ((int64_t)acceX2) * (int64_t)scaleAvg;
                eX2[meanOffset] = (typeExEx2)TIDL_roundSat((int64_t)tempAcc, shiftAvg, minValueAcc, maxValueAcc);
              }
              inp = eX2[meanOffset];
              if (inp <= 0)
              {
                inp = 0;
              }
              /*Calculate denominator & produce output:*/
              denom = (float32_tidl)(__recip_sqrt(inp + (epsilon * (inScale * inScale))));
              denom = outScale * denom;
            }
            TIDL_convertFloatToScaleAndShift(denom, &scaleDenom, &shiftDenom, SCALE_PRECISION_BITS);

            if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
              TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
              TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
            ))
            {
              for (i3 = 0; i3 < icnt[3]; i3++)
              {
                for (i4 = 0; i4 < icnt[4]; i4++)
                {
                  for (i5 = 0; i5 < icnt[5]; i5++)
                  {
                    inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                    outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                    meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]);
                    float inp_f = __int_to_float(inData[inOffset]);
                    inp_f = ((float)width * (float)height * (float)channel) * inp_f;
                    inp_f = inp_f * denom;
                    tempAccOut = __float_to_int(inp_f);
                    tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                    tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                    outData[outOffset] = (Tout)tempAccOut;
                  }
                }
              }
            }
            else
            {
              if (shiftDenom >= 0)
              {
                for (i3 = 0; i3 < icnt[3]; i3++)
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]);
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = (tempAccOut << shiftDenom);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
              else
              {
                for (i3 = 0; i3 < icnt[3]; i3++)
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      absShift = shiftDenom * -1;
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]);
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = ((tempAccOut + (1 << (absShift - 1))) >> absShift);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    else if (axis == TIDL_DIM_HEIGHT)
    {
      /*Compute Average:*/
      float32_tidl avg = 1.0 / ((float)width * (float)height);
      TIDL_getMMAv2_ScaleAndShift(avg, &scaleAvg, &shiftAvg);

      for(i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]) + (i3 * icnt[0] * icnt[1] * icnt[2]);
              eX2[meanOffset] = 0;
              acceX2 = 0;
              /*Calculate mean of x^2 */
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  inp = inData[inOffset];
                  acceX2 += (inp * inp);
                }
              }
              if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
                TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
                TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
              ))
              {
                eX2[meanOffset] = acceX2;
                inp = (width * height * eX2[meanOffset]);
                inp_float = (int64_t)inp;
                ep = epsilon * (inScale * inScale * width * width * height * height);
                inp_sqrt = inp_float + ep;
                /*Calculate denominator & produce output:*/
                denom = (float32_tidl)( __recip_sqrt(inp_sqrt));
                if((TIDL_UnsignedShort == ((int32_t)inDataParams->elementType)) || TIDL_SignedShort == ((int32_t)inDataParams->elementType))
                {
                  /* Refine the reciprocal sqrt using Newton-Raphson method for better accuracy */
                  float32_tidl y_approx = denom;
                  denom = y_approx * (1.5f - 0.5f * inp_sqrt * y_approx * y_approx);
                }
                denom = outScale * denom;
              }
              else
              {
                if ((intAlgHandle->createParams->net->deviceName == TIDL_TDA4VM) ||
                    (inDataParams->elementType == TIDL_UnsignedChar) ||
                    (inDataParams->elementType == TIDL_UnsignedShort) ||
                    ((outDataParams->tensorScale / sqrt(epsilon * inScale * inScale)) > (float)(std::numeric_limits<uint8_t>::max())))
                {
                  eX2[meanOffset] = (typeExEx2)(((acceX2) / (icnt[5] * icnt[4])));
                }
                else
                {
                  tempAcc = ((int64_t)acceX2) * (int64_t)scaleAvg;
                  eX2[meanOffset] = (typeExEx2)TIDL_roundSat((int64_t)tempAcc, shiftAvg, minValueAcc, maxValueAcc);
                }
                inp = eX2[meanOffset];
                if (inp <= 0)
                {
                  inp = 0;
                }
                /*Calculate denominator & produce output:*/
                denom = (float32_tidl)(__recip_sqrt(inp + (epsilon * (inScale * inScale))));
                denom = outScale * denom;
              }
              TIDL_convertFloatToScaleAndShift(denom, &scaleDenom, &shiftDenom, SCALE_PRECISION_BITS);

              if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
                TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
                TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
              ))
              {
                for (i4 = 0; i4 < icnt[4]; i4++)
                {
                  for (i5 = 0; i5 < icnt[5]; i5++)
                  {
                    inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                    outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                    meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]) + (i3 * icnt[0] * icnt[1] * icnt[2]);
                    float inp_f = __int_to_float(inData[inOffset]);
                    inp_f = ((float)width * (float)height) * inp_f;
                    inp_f = inp_f * denom;
                    tempAccOut = __float_to_int(inp_f);
                    tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                    tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                    outData[outOffset] = (Tout)tempAccOut;
                  }
                }
              }
              else
              {
                if (shiftDenom >= 0)
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]) + (i3 * icnt[0] * icnt[1] * icnt[2]);
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = (tempAccOut << shiftDenom);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
                else
                {
                  for (i4 = 0; i4 < icnt[4]; i4++)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      absShift = shiftDenom * -1;
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]) + (i3 * icnt[0] * icnt[1] * icnt[2]);
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = ((tempAccOut + (1 << (absShift - 1))) >> absShift);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    else if (axis == TIDL_DIM_WIDTH)
    {
      /*Compute Average:*/
      float32_tidl avg = 1.0 / (float)width;
      TIDL_getMMAv2_ScaleAndShift(avg, &scaleAvg, &shiftAvg);
      for(i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              /*Calculate mean of x^2 */
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                meanOffset = i4;
                eX2[meanOffset] = 0;
                acceX2 = 0;
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  inp = inData[inOffset];
                  acceX2 += (inp * inp);
                }
                if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
                  TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
                  TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
                ))
                {
                  eX2[meanOffset] = acceX2;
                  inp = (width * eX2[meanOffset]);
                  inp_float = (int64_t)inp;
                  ep = epsilon * (inScale * inScale * width * width);
                  inp_sqrt = inp_float + ep;
                  /*Calculate denominator & produce output:*/
                  denom = (float32_tidl)( __recip_sqrt(inp_sqrt));
                  if((TIDL_UnsignedShort == ((int32_t)inDataParams->elementType)) || TIDL_SignedShort == ((int32_t)inDataParams->elementType))
                  {
                    /* Refine the reciprocal sqrt using Newton-Raphson method for better accuracy */
                    float32_tidl y_approx = denom;
                    denom = y_approx * (1.5f - 0.5f * inp_sqrt * y_approx * y_approx);
                  }
                  denom = outScale * denom;
                }
                else
                {
                  if ((intAlgHandle->createParams->net->deviceName == TIDL_TDA4VM) ||
                      (inDataParams->elementType == TIDL_UnsignedChar) ||
                      (inDataParams->elementType == TIDL_UnsignedShort) ||
                      ((outDataParams->tensorScale / sqrt(epsilon * inScale * inScale)) > (float)(std::numeric_limits<uint8_t>::max())))
                  {
                    eX2[meanOffset] = (typeExEx2)(((acceX2) / icnt[5]));
                  }
                  else
                  {
                    tempAcc = ((int64_t)acceX2) * (int64_t)scaleAvg;
                    eX2[meanOffset] = (typeExEx2)TIDL_roundSat((int64_t)tempAcc, shiftAvg, minValueAcc, maxValueAcc);
                  }
                  inp = eX2[meanOffset];
                  if (inp <= 0)
                  {
                    inp = 0;
                  }
                  /*Calculate denominator & produce output:*/
                  denom = (float32_tidl)(__recip_sqrt(inp + (epsilon * (inScale * inScale))));
                  denom = outScale * denom;
                }
                TIDL_convertFloatToScaleAndShift(denom, &scaleDenom, &shiftDenom, SCALE_PRECISION_BITS);

                if(intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (
                  TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType)) ||
                  TIDL_SignedShort == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
                ))
                {
                  for (i5 = 0; i5 < icnt[5]; i5++)
                  {
                    inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                    outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                    meanOffset = i4;
                    float inp_f = __int_to_float(inData[inOffset]);
                    inp_f = ((float)width) * inp_f;
                    inp_f = inp_f * denom;
                    tempAccOut = __float_to_int(inp_f);
                    tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                    tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                    outData[outOffset] = (Tout)tempAccOut;
                  }
                }
                else
                {
                  if (shiftDenom >= 0)
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i4;
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = (tempAccOut << shiftDenom);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                  else
                  {
                    for (i5 = 0; i5 < icnt[5]; i5++)
                    {
                      absShift = shiftDenom * -1;
                      inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                      outOffset = (i0 * outDim[0]) + (i1 * outDim[1]) + (i2 * outDim[2]) + (i3 * outDim[3]) + (i4 * outDim[4]) + i5;
                      meanOffset = i4;
                      tempAccOut = inData[inOffset] * scaleDenom;
                      tempAccOut = ((tempAccOut + (1 << (absShift - 1))) >> absShift);
                      tempAccOut = ((tempAccOut <= minValueOutput) ? minValueOutput : tempAccOut);
                      tempAccOut = ((tempAccOut >= maxValueOutput) ? maxValueOutput : tempAccOut);
                      outData[outOffset] = (Tout)tempAccOut;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return status;
}

/**
 * @brief This is main function perform RMS normalization on float elementTypes
 *
 * @tparam Tin  : template for input
 * @tparam Tout : template for output
 * @param inPtr : Input pointer on which rmsNorm is applied
 * @param outPtr : Output pointer after rmsNorm operation
 * @param intAlgHandle : tidl algorithm handle
 * @param layerIdx : index of the current layer
 * @param params : copy of rmsNorm parameters
 * @param algLayer : Pointer to the layer specific parameters
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @return  TIDL_SUCCESS - Successful
 *          TIDL_ERR_FAILURE - Unspecified error
 */
template<class Tin, class Tout> static int32_t TIDL_refRmsNormCoreFloat(Tin *inPtr,
                                                                       Tout *outPtr,
                                                                       TIDL_Handle intAlgHandle,
                                                                       int32_t layerIdx,
                                                                       sTIDL_RMSNormParams_t *params,
                                                                       sTIDL_AlgLayer_t *algLayer,
                                                                       const sTIDL_DataParams_t *inDataParams,
                                                                       const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t i0, i1, i2, i3, i4, i5;
  int32_t icnt[TIDL_DIM_MAX] = {0};
  int32_t dim[TIDL_DIM_MAX] = {0};
  int32_t inVolume = 1;
  int32_t meanOffset = 0;
  int32_t inOffset = 0;
  float32_tidl epsilon = params->epsilon;
  int32_t axis = params->axis;
  int32_t scratchSizeRequired = 0;

  for (i0 = 0; i0 < (TIDL_DIM_MAX - 1); i0++)
  {
    icnt[i0] = inDataParams->dimValues[i0];
    inVolume *= icnt[i0];
    dim[i0] = inDataParams->pitch[i0];
  }
  icnt[TIDL_DIM_MAX - 1] = inDataParams->dimValues[TIDL_DIM_MAX - 1];
  inVolume *= icnt[TIDL_DIM_MAX - 1];

  Tin *inData = (Tin *)inPtr;
  Tout *outData = (Tout *)outPtr;

  /* Calculate scratch size based on the normalization axis */
  switch (axis)
  {
    case TIDL_DIM_DIM1:   /* i1 - dim1 */
      scratchSizeRequired = icnt[0] * icnt[2] * icnt[3] * icnt[4] * icnt[5] * (int32_t)sizeof(float32_tidl);
      break;
    case TIDL_DIM_DIM2:   /* i2 - dim2 */
      scratchSizeRequired = icnt[0] * icnt[1] * icnt[3] * icnt[4] * icnt[5] * (int32_t)sizeof(float32_tidl);
      break;
    case TIDL_DIM_NUMCH:   /* i3 - numch (alias for channel) */
      scratchSizeRequired = icnt[0] * icnt[1] * icnt[2] * icnt[4] * icnt[5] * (int32_t)sizeof(float32_tidl);
      break;
    case TIDL_DIM_HEIGHT:  /* i4 - height */
      scratchSizeRequired = icnt[0] * icnt[1] * icnt[2] * icnt[3] * icnt[5] * (int32_t)sizeof(float32_tidl);
      break;
    case TIDL_DIM_WIDTH:   /* i5 - width */
      scratchSizeRequired = icnt[4] * (int32_t)sizeof(float32_tidl);
      break;
  }

  float32_tidl *eX2;

  if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_LAYERNORM_SCRATCH_SIZE_ERROR)
  {
    algLayer->scratchSize = 0;
  }

  if (algLayer->scratchSize >= scratchSizeRequired)
  {
    eX2 = (float32_tidl *)algLayer->scratchMem;
  }
  else
  {
    status = TIDL_ERROR_RMSNORM_INSUFFICIENT_REF_SCRATCH;
  }

  if (status == TIDL_SUCCESS)
  {
    /* Initialize scratch memory to zero */
    memset(eX2, 0, scratchSizeRequired);  
    if(axis == TIDL_DIM_DIM1)
    {
      /* Normalize along dim1 dimension (i1) */
      for (i0 = 0; i0 < icnt[0]; i0++)
      {
        meanOffset = i0;
        
        /* Calculate sum of squares along dim1 dimension */
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  eX2[meanOffset] += (float32_tidl)pow(inData[inOffset], 2);
                }
              }
            }
          }
        }
                
        /* Calculate RMS */
        eX2[meanOffset] /= ((float)icnt[1] * (float)icnt[2] * (float)icnt[3] * (float)icnt[4] * (float)icnt[5]);
        eX2[meanOffset] += epsilon;
        eX2[meanOffset] = (float32_tidl)sqrt(eX2[meanOffset]);
                
        /* Apply normalization */
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  outData[inOffset] = (Tout)(inData[inOffset] / eX2[meanOffset]);
                }
              }
            }
          }
        }
      }
    }
        
    else if(axis == TIDL_DIM_DIM2)
    {
      /* Normalize along dim2 dimension (i2) */
      for (i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          meanOffset = i0 + (i1 * icnt[0]);
          
          /* Calculate sum of squares along dim2 dimension */
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  eX2[meanOffset] += (float32_tidl)pow(inData[inOffset], 2);
                }
              }
            }
          }
                
          /* Calculate RMS */
          eX2[meanOffset] /= ((float)icnt[2] * (float)icnt[3] * (float)icnt[4] * (float)icnt[5]);
          eX2[meanOffset] += epsilon;
          eX2[meanOffset] = (float32_tidl)sqrt(eX2[meanOffset]);
                
          /* Apply normalization */
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  outData[inOffset] = (Tout)(inData[inOffset] / eX2[meanOffset]);
                }
              }
            }
          }
        }
      }
    }

    else if(axis == TIDL_DIM_NUMCH)
    {
      /* Normalize along channel dimension (i3) */
      for (i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]);
            /* Calculate sum of squares along channel dimension */
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  eX2[meanOffset] += (float32_tidl)pow(inData[inOffset], 2);
                }
              }
            }
            
            /* Calculate RMS */
            eX2[meanOffset] /= ((float)icnt[3] * (float)icnt[4] * (float)icnt[5]);
            eX2[meanOffset] += epsilon;
            eX2[meanOffset] = (float32_tidl)sqrt(eX2[meanOffset]);
                
            /* Apply normalization */
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  outData[inOffset] = (Tout)(inData[inOffset] / eX2[meanOffset]);
                }
              }
            }
          }
        }
      }
    }

    else if(axis == TIDL_DIM_HEIGHT)
    {
      /* Normalize along height dimension (i4) */
      for (i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              meanOffset = i0 + (i1 * icnt[0]) + (i2 * icnt[0] * icnt[1]) + 
                          (i3 * icnt[0] * icnt[1] * icnt[2]);
              
              /* Calculate sum of squares along height dimension */
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  eX2[meanOffset] += (float32_tidl)pow(inData[inOffset], 2);
                }
              }
                
              /* Calculate RMS */
              eX2[meanOffset] /= ((float)icnt[4] * (float)icnt[5]);
              eX2[meanOffset] += epsilon;
              eX2[meanOffset] = (float32_tidl)sqrt(eX2[meanOffset]);
                
              /* Apply normalization */
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + 
                            (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  outData[inOffset] = (Tout)(inData[inOffset] / eX2[meanOffset]);
                }
              }
            }
          }
        }
      }
    }
    
    else if(axis == TIDL_DIM_WIDTH)
    {
      /* Normalize along width dimension (i5) */
      for(i0 = 0; i0 < icnt[0]; i0++)
      {
        for (i1 = 0; i1 < icnt[1]; i1++)
        {
          for (i2 = 0; i2 < icnt[2]; i2++)
          {
            for (i3 = 0; i3 < icnt[3]; i3++)
            {
              /*Calculate mean of x^2:*/
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                meanOffset = i4;
                eX2[meanOffset] = 0.0;
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  eX2[meanOffset] += (float32_tidl)pow(inData[inOffset], 2);
                }
                eX2[meanOffset] /= (float)icnt[5];
                eX2[meanOffset] += epsilon;
                eX2[meanOffset] = (float32_tidl)sqrt(eX2[meanOffset]);
              }
              
              /*Calculate final output:*/
              for (i4 = 0; i4 < icnt[4]; i4++)
              {
                for (i5 = 0; i5 < icnt[5]; i5++)
                {
                  inOffset = (i0 * dim[0]) + (i1 * dim[1]) + (i2 * dim[2]) + (i3 * dim[3]) + (i4 * dim[4]) + i5;
                  meanOffset = i4;
                  outData[inOffset] = (Tout)(inData[inOffset] / eX2[meanOffset]);
                }
              }
            }
          }
        }
      }
    }
  }
  return status;
}

/**
 * @brief RMS normalization layer reference implementation
 *
 * @param intAlgHandle : tidl algorithm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of rmsNorm layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inDataParams : pointer to input data parameters
 * @param outDataParams : pointer to output data parameters
 * @return  TIDL_SUCCESS - Successful
 *          TIDL_ERR_FAILURE - Unspecified error
 */
int32_t TIDL_rmsNormRefProcess(TIDL_Handle intAlgHandle,
                              sTIDL_AlgLayer_t *algLayer,
                              const sTIDL_Layer_t *tidlLayer,
                              sTIDL_RMSNormParams_t *params,
                              void *inPtr,
                              void *outPtr,
                              const sTIDL_DataParams_t *inDataParams,
                              const sTIDL_DataParams_t *outDataParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t layerIdx = algLayer->layerIdx;

  if (intAlgHandle->createParams->net->netVersion >= TIDL_NET_VERSION_FW_11_00_00_00 && (TIDL_SignedChar == ((int32_t)inDataParams->elementType) || (TIDL_UnsignedChar == ((int32_t)inDataParams->elementType))))
  {
    if (TIDL_SignedChar == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refRmsNormCore<int8_t, int8_t, int64_t>((int8_t *)inPtr,
                                      (int8_t *)outPtr,
                                      intAlgHandle,
                                      layerIdx,
                                      params,
                                      algLayer,
                                      inDataParams,
                                      outDataParams);
    }
    else
    {
      status = TIDL_refRmsNormCore<uint8_t, int8_t, uint64_t>((uint8_t *)inPtr,
                                        (int8_t *)outPtr,
                                        intAlgHandle,
                                        layerIdx,
                                        params,
                                        algLayer,
                                        inDataParams,
                                        outDataParams);
    }
  }
  else
  {
    if (TIDL_SignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refRmsNormCore<int16_t, int16_t, int64_t>((int16_t *)inPtr,
                                      (int16_t *)outPtr,
                                      intAlgHandle,
                                      layerIdx,
                                      params,
                                      algLayer,
                                      inDataParams,
                                      outDataParams);
    }
    else if (TIDL_UnsignedShort == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refRmsNormCore<uint16_t, int16_t, uint64_t>((uint16_t *)inPtr,
                                      (int16_t *)outPtr,
                                      intAlgHandle,
                                      layerIdx,
                                      params,
                                      algLayer,
                                      inDataParams,
                                      outDataParams);
    }
    else if (TIDL_SinglePrecFloat == ((int32_t)inDataParams->elementType))
    {
      status = TIDL_refRmsNormCoreFloat((float32_tidl *)inPtr,
                                        (float32_tidl *)outPtr,
                                        intAlgHandle,
                                        layerIdx,
                                        params,
                                        algLayer,
                                        inDataParams,
                                        outDataParams);
    }
    else
    {
      status = TIDL_ERR_FAILURE;
    }
  }

  TIDL_L1DandL2CacheWbInv();
  return status;
}