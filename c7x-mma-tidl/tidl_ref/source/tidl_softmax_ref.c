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
@file    tidl_softmax_ref.c
@brief   This file defines private functions for Softmax layer.
@version 0.1 (Feb 2017) : Initial version [ADK]
----------------------------------------------------------------------------
 */
#ifdef HOST_EMULATION
#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_lut_module.h"
#include <math.h>
#include <float.h>
#include "tidl_softmax_ref.h"

// #define lutTensorScale (1.0/255.0) /* This is an intermediate variable set by user */
#define lutTensorZP (0.0)                               /* This is an intermediate variable set by user */
#define SCALE_PRECISION_BITS 8                          /* This parameter specifies the precision to which the scale of float should be calculated while converting float to scale and shift */
#define FLOAT_SIMD_WIDTH ((uint32_t)(VEC_NELEM(float))) // This macro defines the max number of float elements that can be present in a full c7x vector. Since the softmax kernel uses float vectors a lot, it is useful to have this macro defined.

template<class Tin, class Tout, class Tacc>
    int32_t TIDL_refSoftmaxProcessInt(
        Tin *inData,
        Tout *outData,
        float32_tidl tensorScale,
        uint32_t imWidth,
        int32_t inPitch,
        int32_t outPitch,
        int32_t numBatches,
        int32_t inBatchPitch,
        int32_t outBatchPitch,
        const sTIDL_DataParams_t *inDataParams,
        const sTIDL_DataParams_t *outDataParams,
        int32_t axis,
        int32_t outTranspose)
{
  Tin *inPtr = inData;
  Tout *outPtr = outData;
  Tin outputTensorZP = outDataParams->tensorZeroPoint;
  float32_tidl outputTensorScale = outDataParams->tensorScale;
  float32_tidl inputTensorScale = inDataParams->tensorScale;
  float32_tidl inputTensorScaleInv = ((float32_tidl)1) / inputTensorScale;
#if defined(__C7100__) || defined(__C7120__)
  float32_tidl lutTensorScale = 1.0 / 255.0;
#endif
  float32_tidl quantScale = 255.0f;
  if ((typeid(Tin) == typeid(int16_t)) || (typeid(Tin) == typeid(uint16_t)))
  {
#if defined(__C7100__) || defined(__C7120__)
    lutTensorScale = 1.0 / 65535.0;
#endif
    quantScale = 65535.0f;
  }
  int32_t icnt[TIDL_DIM_MAX] = {0};
  int32_t dim[TIDL_DIM_MAX] = {0};
  int32_t ddim[TIDL_DIM_MAX] = {0};
  /*Set dims & counts:*/
  /*DIM0*/
  icnt[0] = inDataParams->dimValues[TIDL_DIM_WIDTH];
  dim[0] = 1U;
  /*DIM1*/
  icnt[1] = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  dim[1] = inDataParams->pitch[TIDL_LINE_PITCH];

  if (outTranspose != 0)
  {
    /*DIM0*/
    ddim[0] = outDataParams->pitch[TIDL_LINE_PITCH];
    /*DIM1*/
    ddim[1] = 1U;
  }
  else
  {
    /*DIM0*/
    ddim[0] = 1U;
    /*DIM1*/
    ddim[1] = outDataParams->pitch[TIDL_LINE_PITCH];
  }
  /*DIM2*/
  icnt[2] = inDataParams->dimValues[TIDL_DIM_NUMCH];
  dim[2] = inDataParams->pitch[TIDL_CHANNEL_PITCH];
  ddim[2] = outDataParams->pitch[TIDL_CHANNEL_PITCH];
  /*DIM3*/
  icnt[3] = inDataParams->dimValues[TIDL_DIM_DIM2];
  dim[3] = inDataParams->pitch[TIDL_DIM2_PITCH];
  ddim[3] = outDataParams->pitch[TIDL_DIM2_PITCH];
  /*DIM4*/
  icnt[4] = inDataParams->dimValues[TIDL_DIM_DIM1];
  dim[4] = inDataParams->pitch[TIDL_DIM1_PITCH];
  ddim[4] = outDataParams->pitch[TIDL_DIM1_PITCH];
  /*DIM5*/
  icnt[5] = inDataParams->dimValues[TIDL_DIM_BATCH];
  dim[5] = inDataParams->pitch[TIDL_ROI_PITCH];
  ddim[5] = outDataParams->pitch[TIDL_ROI_PITCH];

  /*Use softmax's "axis" parameter to swap the innermost dimension*/
  int32_t icnt_temp, dim_temp, ddim_temp;
  icnt_temp = icnt[0];
  dim_temp = dim[0];
  ddim_temp = ddim[0];
  /* axis dim -> dim 0*/
  icnt[0] = icnt[(int32_t)TIDL_DIM_MAX - 1 - axis];
  dim[0] = dim[(int32_t)TIDL_DIM_MAX - 1 - axis];
  ddim[0] = ddim[(int32_t)TIDL_DIM_MAX - 1 - axis];
  /* axis dim <- dim 0*/
  icnt[(int32_t)TIDL_DIM_MAX - 1 - axis] = icnt_temp;
  dim[(int32_t)TIDL_DIM_MAX - 1 - axis] = dim_temp;
  ddim[(int32_t)TIDL_DIM_MAX - 1 - axis] = ddim_temp;

  int32_t i5, i4, i3, i2, i1, i0;
  /*Compute:*/
  /* OPENACC(data copyin(inPtr[: 1+ (icnt[5]-1) * dim[5] + (icnt[4]-1) * dim[4] + (icnt[3]-1) * dim[3] + (icnt[2]-1) * dim[2] + (icnt[1]-1) * dim[1] + (icnt[0]-1) * dim[0]]) \
               copy(outPtr[: 1+ (icnt[5]-1) * ddim[5] + (icnt[4]-1) * ddim[4] + (icnt[3]-1) * ddim[3] + (icnt[2]-1) * ddim[2] + (icnt[1]-1) * ddim[1] + (icnt[0]-1) * ddim[0]]))
  OPENACC(parallel loop collapse(5)) */
  for (i5 = 0; i5 < icnt[5]; i5++)
  {
    for (i4 = 0; i4 < icnt[4]; i4++)
    {
      for (i3 = 0; i3 < icnt[3]; i3++)
      {
        for (i2 = 0; i2 < icnt[2]; i2++)
        {
          for (i1 = 0; i1 < icnt[1]; i1++)
          {
            Tin xMax = 0;
            xMax = std::numeric_limits<Tin>::lowest();
            ;
            Tin inDataVal;
            int32_t numTiles = imWidth / TIDL_SIMD_WIDTH;
            if ((imWidth % TIDL_SIMD_WIDTH) != 0U)
            {
              numTiles++;
            }
            Tacc denomSum = 0;

            /*Find Max:*/
            // OPENACC(loop)
            for (i0 = 0; i0 < icnt[0]; i0++)
            {
              inDataVal = inPtr[(i5 * dim[5]) + (i4 * dim[4]) + (i3 * dim[3]) + (i2 * dim[2]) + (i1 * dim[1]) + (i0 * dim[0])];
              xMax = (xMax > inDataVal) ? xMax : inDataVal;
            }

            // OPENACC(loop)
            for (i0 = 0; i0 < icnt[0]; i0++)
            {
              Tout expOut;
              if ((typeid(Tin) == typeid(int16_t)) || (typeid(Tout) == typeid(uint16_t)))
              {
                int32_t inputLUT = (int32_t)(inPtr[(i5 * dim[5]) + (i4 * dim[4]) + (i3 * dim[3]) + (i2 * dim[2]) + (i1 * dim[1]) + (i0 * dim[0])] - xMax);
                expOut = float_to_int_c7x((exp_taylor(inputLUT * inputTensorScaleInv) * (quantScale))); /*LUT output calculation, out = (exp(-5∗sx)∗(1/sp))+zp */
              }
              else
#if defined(__C7504__) || defined(__C7524__) || defined(__C7604__) 
              {
                int32_t inputLUT = (int32_t)(inPtr[i5 * dim[5] + i4 * dim[4] + i3 * dim[3] + i2 * dim[2] + i1 * dim[1] + i0 * dim[0]] - xMax);
                expOut = round((exp_taylor(inputLUT * inputTensorScaleInv) * (quantScale))); /*LUT output calculation, out = (exp(-5∗sx)∗(1/sp))+zp */
              }
#else
              {
                uint8_t inputLUT = (uint8_t)(xMax - inPtr[(i5 * dim[5]) + (i4 * dim[4]) + (i3 * dim[3]) + (i2 * dim[2]) + (i1 * dim[1]) + (i0 * dim[0])]);
                expOut = round((exp(-1.0 * (float)inputLUT / inputTensorScale) * (1.0 / lutTensorScale)) + lutTensorZP); /*LUT output calculation, out = (exp(-5∗sx)∗(1/sp))+zp */
              }
#endif
              denomSum += expOut;
              outPtr[(i5 * ddim[5]) + (i4 * ddim[4]) + (i3 * ddim[3]) + (i2 * ddim[2]) + (i1 * ddim[1]) + (i0 * ddim[0])] = (Tout)(expOut);
            }

            float32_tidl mulFactor = __recip(__convert_float(denomSum)) * (outputTensorScale);

            int32_t OutScale, OutShift;
            // OPENACC(routine(TIDL_convertFloatToScaleAndShift))
            TIDL_convertFloatToScaleAndShift(mulFactor, &OutScale, &OutShift, SCALE_PRECISION_BITS);
            uint8_t scaleFactor = (uint8_t)OutScale;
            uint32_t absShift = abs(OutShift);

            if (OutShift >= 0)
            {
              // OPENACC(loop)
              for (i0 = 0; i0 < icnt[0]; i0++)
              {
                Tout expOut = (Tout)outPtr[(i5 * ddim[5]) + (i4 * ddim[4]) + (i3 * ddim[3]) + (i2 * ddim[2]) + (i1 * ddim[1]) + (i0 * ddim[0])];
                Tacc prod = (expOut * scaleFactor);
                Tacc shiftAcc = prod << (uint64_t)OutShift;
                shiftAcc = shiftAcc + outputTensorZP;
                /* LDRA_JUSTIFY_START
                  <metric start> statement branch <metric end>
                  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  <justification end> */
                Tout softmaxOut = (shiftAcc > std::numeric_limits<Tout>::max()) ? std::numeric_limits<Tout>::max() : shiftAcc;
                /* LDRA_JUSTIFY_END */
                outPtr[(i5 * ddim[5]) + (i4 * ddim[4]) + (i3 * ddim[3]) + (i2 * ddim[2]) + (i1 * ddim[1]) + (i0 * ddim[0])] = (Tout)(softmaxOut);
              }
            }
            else
            {
              // OPENACC(loop)
              for (i0 = 0; i0 < icnt[0]; i0++)
              {
                Tacc expOut = (Tout)outPtr[(i5 * ddim[5]) + (i4 * ddim[4]) + (i3 * ddim[3]) + (i2 * ddim[2]) + (i1 * ddim[1]) + (i0 * ddim[0])];
                Tacc prod = (expOut * scaleFactor);
                Tacc shiftAcc = (prod + (1U << (absShift - 1))) >> absShift;
                shiftAcc = shiftAcc + outputTensorZP;
                Tout softmaxOut = (shiftAcc > std::numeric_limits<Tout>::max()) ? std::numeric_limits<Tout>::max() : shiftAcc;
                outPtr[(i5 * ddim[5]) + (i4 * ddim[4]) + (i3 * ddim[3]) + (i2 * ddim[2]) + (i1 * ddim[1]) + (i0 * ddim[0])] = (Tout)(softmaxOut);
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

/**
 * @brief This function is reference implementation of softmax layer
 *
 * @param inData : Pointer to input memory
 * @param outData : Pointer to output memory
 * @param tensorScale : scale for the input tensor
 * @param imWidth : width of the input buffer
 * @param inPitch : pitch of the input buffer
 * @param outPitch : pitch of the output buffer
 * @param numBatches : no of total Batches
 * @param inBatchPitch : Batch pitch of the input buffer
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
template<class Tin, class Tout> void TIDL_softmaxRefProcess(
    Tin *inData,
    Tout *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose)
{
  float32_tidl maxValue = 0;
  int32_t maxIndex = 0;
  int32_t i, j;
  float32_tidl temp;
  float32_tidl denom;
  float32_tidl numer;
  float32_tidl output;
  int32_t max, min = TIDL_SAT_LO_INT8;
  Tin *inPtr = inData;
  Tout *outPtr = outData;

  if (outDataParams->elementType == TIDL_SinglePrecFloat)
  {
    /* OPENACC(data copyin(inPtr[:1+((numBatches-1)*inBatchPitch) + (imWidth-1)]) \
                 copy(outPtr[:1+((numBatches-1)*outBatchPitch) + (imWidth-1)])) */
    // OPENACC(parallel loop reduction(max:max))
    for (j = 0; j < numBatches; j++)
    {
      int32_t numTiles = imWidth / FLOAT_SIMD_WIDTH;
      if (inDataParams->elementType == TIDL_SignedChar)
      {
        min = TIDL_SAT_LO_INT8;
      }
      else if (inDataParams->elementType == TIDL_UnsignedChar)
      {
        min = TIDL_SAT_LO_UINT8;
      }
      else if (inDataParams->elementType == TIDL_SignedShort)
      {
        min = TIDL_SAT_LO_INT16;
      }
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_002
      <justification end> */
      else if (inDataParams->elementType == TIDL_UnsignedShort)
      {
        /* LDRA_JUSTIFY_END */
        min = TIDL_SAT_LO_UINT16;
      }
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_002
      <justification end> */
      else
      {
        /*do nothing*/
      }
      /* LDRA_JUSTIFY_END */
      max = min;
      denom = 0.0;
      maxValue = 0;
      float32_tidl inDataVal;
      float32_tidl log2baseE = (float32_tidl)0.693147180559945;
      float32_tidl logEbase2 = (float32_tidl)1 / (float32_tidl)0.693147180559945;
      float32_tidl oneBy6 = ((float32_tidl)1 / (float32_tidl)6);
      float32_tidl oneBy24 = (0.25) * ((float32_tidl)1 / (float32_tidl)6);
      // OPENACC(loop)
      for (i = 0; i < imWidth; i++)
      {
        inDataVal = inPtr[(j * inBatchPitch) + i];
        max = ((float32_tidl)max > inDataVal) ? (float32_tidl)max : inDataVal;
      }

      if (((uint32_t)imWidth % FLOAT_SIMD_WIDTH) != 0U)
      {
        numTiles++;
      }

      float32_tidl vecSim[32] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
      for (int32_t k = 0; k < (int32_t)FLOAT_SIMD_WIDTH; k++)
      {
        vecSim[k] = 0.0f;
      }

      // OPENACC(loop seq collapse(2))
      for (i = 0; i < numTiles; i++)
      {
        for (int32_t k = 0; (k < (int32_t)FLOAT_SIMD_WIDTH) && (k < imWidth); k++)
        {

          // Imitating decdim.
          if (((i * (int32_t)FLOAT_SIMD_WIDTH) + k) >= imWidth)
          {
            inDataVal = min;
          }
          else
          {
            inDataVal = inPtr[((uint32_t)j * (uint32_t)inBatchPitch) + (((uint32_t)i * FLOAT_SIMD_WIDTH) + (uint32_t)k)];
          }

          temp = (inDataVal - (float32_tidl)max) * (1.0f / tensorScale);

#if defined TIDL_COVERAGE_DEAD_CODE
          /* For float input TIDL_softmaxRefProcessFloat function is called */
          if (inDataParams->elementType == TIDL_SinglePrecFloat)
          {
            numer = exp(temp);
          }
          else
          {
          }
#endif
          temp = logEbase2 * temp;
          int32_t floorTemp = (int32_t)temp;
          float32_tidl floatRes = temp - (float32_tidl)floorTemp;
          floatRes = floatRes * log2baseE;

          float32_tidl floatRes2 = floatRes * floatRes;
          float32_tidl floatRes3 = floatRes2 * floatRes;
          float32_tidl floatRes4 = floatRes2 * floatRes2;
          numer = 1.0f + floatRes + (floatRes2 * 0.5);
          numer = numer + (floatRes3 * oneBy6);
          numer = numer + (floatRes4 * oneBy24);
#if defined TIDL_COVERAGE_DEAD_CODE
          /* floorTemp will always <= 0 Ref: Line No. 390 */
          if (floorTemp > 0)
          {
            numer = numer * ((1U << 16U) << (uint32_t)(floorTemp));
          }
          else
          {
            numer = numer * ((1U << 16U) >> (-floorTemp));
          }
#else
          numer = numer * ((1U << 16U) >> (-floorTemp));
#endif

          numer = numer * (1.0f / ((float32_tidl)(1U << 16)));
          if (floorTemp < -16)
          {
            numer = 0;
          }

          output = numer;

          vecSim[k] += numer;

          if (maxValue <= output)
          {
            maxValue = output;
            maxIndex = (i * (int32_t)FLOAT_SIMD_WIDTH) + k;
          }
          /*tidl_printf(0,"%4d : %6.2f",i, output*100);*/
          if (((i * (int32_t)FLOAT_SIMD_WIDTH) + k) < imWidth)
          {
            outPtr[((uint32_t)j * (uint32_t)outBatchPitch) + (((uint32_t)i * FLOAT_SIMD_WIDTH) + (uint32_t)k)] = (Tout)(output);
          }
        }
      }

      for (int32_t k = 0; k < (int32_t)FLOAT_SIMD_WIDTH; k++)
      {
        denom += vecSim[k];
      }
      float32_tidl denomInv = 0.0f;
#if defined TIDL_COVERAGE_DEAD_CODE
      /* Denominator in softmax can never be equal to zero hence this condition is redundant */
      if (denom != 0.0f)
      {
        denomInv = (1.0f / denom);
      }
#endif

      denomInv = (1.0f / denom);
      for (i = 0; i < imWidth; i++)
      {
        outPtr[((j * outBatchPitch) + i)] = outPtr[((j * outBatchPitch) + i)] * denomInv;
        output = outPtr[((j * outBatchPitch) + i)];
      }
#ifndef BUILD_WITH_OPENACC
      tidl_printf(2, "-------Max Index %4d : %7.2f -------\n", maxIndex, outPtr[(j * outBatchPitch) + maxIndex]);
#endif
    }
  }
  else // if((inDataParams->elementType == TIDL_SignedChar || inDataParams->elementType == TIDL_UnsignedChar) && (outDataParams->elementType == TIDL_SignedChar || outDataParams->elementType == TIDL_UnsignedChar))
  {
    if ((inDataParams->elementType == TIDL_SignedChar) || (inDataParams->elementType == TIDL_UnsignedChar))
    {
      int32_t rval = TIDL_refSoftmaxProcessInt<Tin, Tout, uint32_t>(
          inData,
          outData,
          tensorScale,
          imWidth,
          inPitch,
          outPitch,
          numBatches,
          inBatchPitch,
          outBatchPitch,
          inDataParams,
          outDataParams,
          axis,
          outTranspose);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_003
    <justification end> */
    else if ((inDataParams->elementType == TIDL_SignedShort) || (inDataParams->elementType == TIDL_UnsignedShort))
    {
      /* LDRA_JUSTIFY_END */
      int32_t rval = TIDL_refSoftmaxProcessInt<Tin, Tout, uint32_t>(
          inData,
          outData,
          tensorScale,
          imWidth,
          inPitch,
          outPitch,
          numBatches,
          inBatchPitch,
          outBatchPitch,
          inDataParams,
          outDataParams,
          axis,
          outTranspose);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_SOFTMAX_PRIOR_CHECK_003
    <justification end> */
    else
    {
      tidl_printf(0, "Unsupported softmax combination!\n");
    }
    /* LDRA_JUSTIFY_END */
  }
  TIDL_L1DandL2CacheWbInv();
}

/**
 * @brief This function is the floating point reference implementation of softmax layer
 *
 * @param tidlLayer : Pointer to the common layer parameters
 * @param inPtrs : Pointer to input memory
 * @param outPtr : Pointer to output memory
 * @param inDataParams : parameters of the input data buffer
 * @param outDataParams : parameters of the output data buffer
 * @param basePrmPtr : Copy of softmax layer parameters
 */
int32_t TIDL_softmaxRefProcessFloat(const sTIDL_Layer_t *tidlLayer,
                                    void *inPtrs,
                                    void *outPtr,
                                    sTIDL_DataParams_t *inDataParams[],
                                    const sTIDL_DataParams_t *outDataParams,
                                    uint8_t *basePrmPtr,
                                    int32_t outTranspose)
{
  float32_tidl *in = (float32_tidl *)inPtrs;
  float32_tidl *out = (float32_tidl *)outPtr;

  int32_t inOffset = 0;

  int32_t axis = tidlLayer->layerParams.softMaxParams.axis;
  int32_t icnt[TIDL_DIM_MAX] = {0};
  int32_t dim[TIDL_DIM_MAX] = {0};
  int32_t ddim[TIDL_DIM_MAX] = {0};

  float32_tidl max = -FLT_MAX;
  float32_tidl denom = 1.0;
  float32_tidl inDataVal, output = 0.0f;
  float32_tidl *inPtr = &in[inOffset];
  float32_tidl temp, numer;

  /*Set dims & counts:*/
  if (outTranspose != 0)
  {
    /*DIM0*/
    icnt[0] = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
    dim[0] = 1U;
    ddim[0] = outDataParams->pitch[TIDL_LINE_PITCH];
    /*DIM1*/
    icnt[1] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    dim[1] = inDataParams[0]->pitch[TIDL_LINE_PITCH];
    ddim[1] = 1U;
  }
  else
  {
    /*DIM0*/
    icnt[0] = inDataParams[0]->dimValues[TIDL_DIM_WIDTH];
    dim[0] = 1U;
    ddim[0] = 1U;
    /*DIM1*/
    icnt[1] = inDataParams[0]->dimValues[TIDL_DIM_HEIGHT];
    dim[1] = inDataParams[0]->pitch[TIDL_LINE_PITCH];
    ddim[1] = outDataParams->pitch[TIDL_LINE_PITCH];
  }
  /*DIM2*/
  icnt[2] = inDataParams[0]->dimValues[TIDL_DIM_NUMCH];
  dim[2] = inDataParams[0]->pitch[TIDL_CHANNEL_PITCH];
  ddim[2] = outDataParams->pitch[TIDL_CHANNEL_PITCH];
  /*DIM3*/
  icnt[3] = inDataParams[0]->dimValues[TIDL_DIM_DIM2];
  dim[3] = inDataParams[0]->pitch[TIDL_DIM2_PITCH];
  ddim[3] = outDataParams->pitch[TIDL_DIM2_PITCH];
  /*DIM4*/
  icnt[4] = inDataParams[0]->dimValues[TIDL_DIM_DIM1];
  dim[4] = inDataParams[0]->pitch[TIDL_DIM1_PITCH];
  ddim[4] = outDataParams->pitch[TIDL_DIM1_PITCH];
  /*DIM5*/
  icnt[5] = inDataParams[0]->dimValues[TIDL_DIM_BATCH];
  dim[5] = inDataParams[0]->pitch[TIDL_ROI_PITCH];
  ddim[5] = outDataParams->pitch[TIDL_ROI_PITCH];

  /*Use softmax's "axis" parameter to swap the innermost dimension*/
  int32_t icnt_temp, dim_temp, ddim_temp;
  icnt_temp = icnt[0];
  dim_temp = dim[0];
  ddim_temp = ddim[0];
  /* axis dim -> dim 0*/
  icnt[0] = icnt[(int32_t)TIDL_DIM_MAX - 1 - axis];
  dim[0] = dim[(int32_t)TIDL_DIM_MAX - 1 - axis];
  ddim[0] = ddim[(int32_t)TIDL_DIM_MAX - 1 - axis];
  /* axis dim <- dim 0*/
  icnt[(int32_t)TIDL_DIM_MAX - 1 - axis] = icnt_temp;
  dim[(int32_t)TIDL_DIM_MAX - 1 - axis] = dim_temp;
  ddim[(int32_t)TIDL_DIM_MAX - 1 - axis] = ddim_temp;

  int32_t i0, i1, i2, i3, i4, i5;

  /* OPENACC(data copyin(inPtr[: 1+ (icnt[5]-1) * dim[5] + (icnt[4]-1) * dim[4] + (icnt[3]-1) * dim[3] + (icnt[2]-1) * dim[2] + (icnt[1]-1) * dim[1] + (icnt[0]-1) * dim[0]]) \
               copy(out[: 1+ (icnt[5]-1) * ddim[5] + (icnt[4]-1) * ddim[4] + (icnt[3]-1) * ddim[3] + (icnt[2]-1) * ddim[2] + (icnt[1]-1) * ddim[1] + (icnt[0]-1) * ddim[0]])) */
  // OPENACC(parallel loop collapse(5))
  for (i5 = 0; i5 < icnt[5]; i5++)
  {
    for (i4 = 0; i4 < icnt[4]; i4++)
    {
      for (i3 = 0; i3 < icnt[3]; i3++)
      {
        for (i2 = 0; i2 < icnt[2]; i2++)
        {
          for (i1 = 0; i1 < icnt[1]; i1++)
          {
            max = -FLT_MAX;
            denom = 0.0;
            // OPENACC(loop)
            /*Find Max:*/
            for (i0 = 0; i0 < icnt[0]; i0++)
            {
              inDataVal = inPtr[(i5 * dim[5]) + (i4 * dim[4]) + (i3 * dim[3]) + (i2 * dim[2]) + (i1 * dim[1]) + (i0 * dim[0])];
              max = (max > inDataVal) ? max : inDataVal;
            }
            // OPENACC(loop)
            /*Find Denominator*/
            for (i0 = 0; i0 < icnt[0]; i0++)
            {
              inDataVal = inPtr[(i5 * dim[5]) + (i4 * dim[4]) + (i3 * dim[3]) + (i2 * dim[2]) + (i1 * dim[1]) + (i0 * dim[0])];
              /*Subtract "max" for mathematical stability*/
              temp = (inDataVal - max);
              denom += (float32_tidl)exp((float64_tidl)(temp));
            }
            // OPENACC(loop)
            /*Calculate softmax at a per element granularity*/
            for (i0 = 0; i0 < icnt[0]; i0++)
            {
              inDataVal = inPtr[(i5 * dim[5]) + (i4 * dim[4]) + (i3 * dim[3]) + (i2 * dim[2]) + (i1 * dim[1]) + (i0 * dim[0])];
              temp = (inDataVal - max);
              numer = (float32_tidl)exp((float64_tidl)(temp));
#if defined TIDL_COVERAGE_DEAD_CODE
              /* Denominator in softmax can never be equal to zero hence this condition is redundant */
              if (denom != 0.0f)
              {
                output = (numer / denom);
              }
#endif
              output = (numer / denom);
              out[(i5 * ddim[5]) + (i4 * ddim[4]) + (i3 * ddim[3]) + (i2 * ddim[2]) + (i1 * ddim[1]) + (i0 * ddim[0])] = output;
            }
          }
        }
      }
    }
  }
  return IALG_EOK;
}

template void TIDL_softmaxRefProcess(
    uint8_t *inData,
    uint8_t *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    uint16_t *inData,
    uint16_t *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    uint8_t *inData,
    int8_t *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    int16_t *inData,
    uint16_t *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    uint8_t *inData,
    float32_tidl *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    int8_t *inData,
    float32_tidl *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    int8_t *inData,
    int8_t *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    int8_t *inData,
    uint8_t *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    int16_t *inData,
    float32_tidl *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);

template void TIDL_softmaxRefProcess(
    uint16_t *inData,
    float32_tidl *outData,
    float32_tidl tensorScale,
    int32_t imWidth,
    int32_t inPitch,
    int32_t outPitch,
    int32_t numBatches,
    int32_t inBatchPitch,
    int32_t outBatchPitch,
    const sTIDL_DataParams_t *inDataParams,
    const sTIDL_DataParams_t *outDataParams,
    int32_t axis,
    int32_t outTranspose);
#endif
