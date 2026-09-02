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
 *  \file tidl_eltWise_ref.c
 *
 *  \brief This file defines kernel functions for Element Wise layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"
#include <limits>
#include "tidl_alg_utils.h"
#include <math.h>
#include "tidl_eltWise_ref.h"
#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
#endif
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 @struct  TIDL_EltWiseBuffParams_t

 @brief   This structure contains buffer parameters used for transferring input
          from external memory to internal memory for computation and for
          transferring output from internal memory to external memory after
          computation.

 @params  inWidth
          Width of input data pointer in external memory.

 @params  inHeight
          Height of input data pointer in external memory.

 @params  inPitch
          Pitch of input data in external memory.

 @params  outPitch
          Pitch of output data in external memory.

 @params  inChPitch
          Channel Pitch of input data in external memory. It contains the size
          of one complete channel data.

 @params  outChPitch
          Channel Pitch of output data in external memory. It contains the size
          of one complete channel output data.

 @params  numBatches
          Number of Batches

 @params  inBatchPitch
          Batch Pitch of input data in external memory. It contains the size
          of one complete channel data.

 @params  outBatchPitch
          Batch Pitch of output data in external memory. It contains the size
          of one complete channel output data.

 @params  outDataQ
          Round bits for output data after EltWise layer operations.

 @params  eltWiseType
          Type of element wise operation.

 @params  numInData
          Number of Input Data.

 @params  numChannels
          Number of channels.
*/
typedef struct
{
  uint32_t inWidth;
  uint32_t inHeight;
  uint16_t pixelPitch;
  uint32_t inPitch;
  uint32_t outPitch;
  uint32_t inChPitch;
  uint32_t outChPitch;
  uint32_t numBatches;
  uint32_t inDIM1Pitch;
  uint32_t outDIM1Pitch;
  uint32_t inDIM2Pitch;
  uint32_t outDIM2Pitch;
  uint32_t inBatchPitch;
  uint32_t outBatchPitch;
  int32_t eltWiseType;
  uint16_t numInData;
  uint32_t numChannels;
  uint32_t numDIM1;
  uint32_t numDIM2;
  int32_t *derivedBias;
  uint8_t *derivedScales;
  uint8_t *derivedShifts;
  int32_t outPadOffset;
} TIDL_EltWiseBuffParams_t;

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
/*                       Static Function Definitions                          */
/* ========================================================================== */

/**
 * @brief Eltwise layer Sum operation implementation
 *
 * @param pIn : Pointer to input buffer
 * @param pAcc : Pointer to accumulator buffer
 * @param scale : Scale to be applied on the input
 * @param eltWiseBuffParams : parameters of eltwise buffer
 * @param callno : Variable to check if it is first call
 * @param isSigned : to check if one of the input is signed
 */
template <class Tin, class Tacc>
static void TIDL_refEltWiseSum(const Tin *pIn,
                               Tacc *pAcc,
                               int32_t scale,
                               const TIDL_EltWiseBuffParams_t *eltWiseBuffParams,
                               int32_t callno,
                               int32_t batchno)
{
  uint32_t i1, i2, i3, i4, i5;
  // OPENACC(data copyin(pIn[:1+((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->inChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->inPitch) + eltWiseBuffParams->inWidth - 1])
  //             present(pAcc[:1+((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1]))

  // OPENACC(parallel loop collapse(5))
  for (i1 = 0U; i1 < eltWiseBuffParams->numDIM1; i1++)
  {
    for (i2 = 0U; i2 < eltWiseBuffParams->numDIM2; i2++)
    {
      for (i3 = 0U; i3 < eltWiseBuffParams->numChannels; i3++)
      {
        for (i4 = 0U; i4 < eltWiseBuffParams->inHeight; i4++)
        {
          for (i5 = 0U; i5 < eltWiseBuffParams->inWidth; i5++)
          {
            uint32_t inOffset = (i1 * eltWiseBuffParams->inDIM1Pitch) + (i2 * eltWiseBuffParams->inDIM2Pitch) + (i3 * eltWiseBuffParams->inChPitch) + (i4 * eltWiseBuffParams->inPitch) + (i5 * eltWiseBuffParams->pixelPitch);
            uint32_t outOffset = (i1 * eltWiseBuffParams->outDIM1Pitch) + (i2 * eltWiseBuffParams->outDIM2Pitch) + (i3 * eltWiseBuffParams->outChPitch) + (i4 * eltWiseBuffParams->outPitch) + i5;
            // OPENACC(atomic update)
            pAcc[outOffset] += ((Tacc)pIn[inOffset]) * scale;
          }
        }
      }
    }
  }
}

/**
 * @brief Eltwise layer Product operation implementation
 *
 * @param pIn : Pointer to input buffer
 * @param pAcc : Pointer to accumulator buffer
 * @param scale : Scale to be applied on the input
 * @param eltWiseBuffParams : parameters of eltwise buffer
 * @param callno : Variable to check if it is first call
 */
template<class Tin, class Tacc> static void TIDL_refEltWiseProduct(const Tin *pIn,
                                                                   Tacc *pAcc,
                                                                   int32_t scale,
                                                                   int32_t zeropoint,
                                                                   const TIDL_EltWiseBuffParams_t *eltWiseBuffParams,
                                                                   int32_t callno)
{
  uint32_t i1, i2, i3, i4, i5;

  /* OPENACC(data copyin(pIn[:1+((eltWiseBuffParams->numChannels-1)* eltWiseBuffParams->inChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->inPitch) + eltWiseBuffParams->inWidth - 1]) \
               present(pAcc[:1+((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1])) */

  if (callno == 0)
  {
    // OPENACC(parallel loop collapse(5))
    for (i1 = 0U; i1 < eltWiseBuffParams->numDIM1; i1++)
    {
      for (i2 = 0U; i2 < eltWiseBuffParams->numDIM2; i2++)
      {
        for (i3 = 0U; i3 < eltWiseBuffParams->numChannels; i3++)
        {
          for (i4 = 0U; i4 < eltWiseBuffParams->inHeight; i4++)
          {
            for (i5 = 0U; i5 < eltWiseBuffParams->inWidth; i5++)
            {
              uint32_t inOffset = (i1 * eltWiseBuffParams->inDIM1Pitch) + (i2 * eltWiseBuffParams->inDIM2Pitch) + (i3 * eltWiseBuffParams->inChPitch) + (i4 * eltWiseBuffParams->inPitch) + (i5 * eltWiseBuffParams->pixelPitch);
              uint32_t outOffset = (i1 * eltWiseBuffParams->outDIM1Pitch) + (i2 * eltWiseBuffParams->outDIM2Pitch) + (i3 * eltWiseBuffParams->outChPitch) + (i4 * eltWiseBuffParams->outPitch) + i5;
              pAcc[outOffset] = (((Tacc)pIn[inOffset] * scale) - zeropoint);
            }
          }
        }
      }
    }
  }
  else
  {
    // OPENACC(parallel loop collapse(5))
    for (i1 = 0U; i1 < eltWiseBuffParams->numDIM1; i1++)
    {
      for (i2 = 0U; i2 < eltWiseBuffParams->numDIM2; i2++)
      {
        for (i3 = 0U; i3 < eltWiseBuffParams->numChannels; i3++)
        {
          for (i4 = 0U; i4 < eltWiseBuffParams->inHeight; i4++)
          {
            for (i5 = 0U; i5 < eltWiseBuffParams->inWidth; i5++)
            {
              uint32_t inOffset = (i1 * eltWiseBuffParams->inDIM1Pitch) + (i2 * eltWiseBuffParams->inDIM2Pitch) + (i3 * eltWiseBuffParams->inChPitch) + (i4 * eltWiseBuffParams->inPitch) + (i5 * eltWiseBuffParams->pixelPitch);
              uint32_t outOffset = (i1 * eltWiseBuffParams->outDIM1Pitch) + (i2 * eltWiseBuffParams->outDIM2Pitch) + (i3 * eltWiseBuffParams->outChPitch) + (i4 * eltWiseBuffParams->outPitch) + i5;
              // OPENACC(atomic update)
              pAcc[outOffset] *= (((Tacc)pIn[inOffset] * scale) - zeropoint);
            }
          }
        }
      }
    }
  }
}

/**
 * @brief Eltwise layer Max operation implementation
 *
 * @param pIn : Pointer to input buffer
 * @param pAcc : Pointer to accumulator buffer
 * @param scale : Scale to be applied on the input
 * @param eltWiseBuffParams : parameters of eltwise buffer
 * @param callno : Variable to check if it is first call
 */
template <class Tin, class Tacc>
static void TIDL_refEltWiseMinMax(const Tin* pIn,
                               Tacc*   pAcc,
                               int32_t scale,
                               const TIDL_EltWiseBuffParams_t *eltWiseBuffParams,
                               int32_t callno)
{
  uint32_t i1, i2, i3, i4, i5;

  // OPENACC(data copyin(pIn[:1+((eltWiseBuffParams->numChannels-1)* eltWiseBuffParams->inChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->inPitch) + eltWiseBuffParams->inWidth - 1])
  //            present(pAcc[:1+((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1]))

  if (callno == 0)
  {
    // OPENACC(parallel loop collapse(3))
    for (i4 = 0U; i4 < eltWiseBuffParams->numDIM1; i4++)
    {
      for (i5 = 0U; i5 < eltWiseBuffParams->numDIM2; i5++)
      {
        for (i1 = 0U; i1 < eltWiseBuffParams->numChannels; i1++)
        {
          for (i2 = 0U; i2 < eltWiseBuffParams->inHeight; i2++)
          {
            for (i3 = 0U; i3 < eltWiseBuffParams->inWidth; i3++)
            {
              uint32_t inOffset = (i4 * eltWiseBuffParams->inDIM1Pitch) + (i5 * eltWiseBuffParams->inDIM2Pitch) + (i1 * eltWiseBuffParams->inChPitch) + (i2 * eltWiseBuffParams->inPitch) + (i3 * eltWiseBuffParams->pixelPitch);
              uint32_t outOffset = (i4 * eltWiseBuffParams->outDIM1Pitch) + (i5 * eltWiseBuffParams->outDIM2Pitch) + (i1 * eltWiseBuffParams->outChPitch) + (i2 * eltWiseBuffParams->outPitch) + i3;
              pAcc[outOffset] = pIn[inOffset] * scale;
            }
          }
        }
      }
    }
  }
  else
  {
    // OPENACC(parallel loop collapse(3))
    for (i4 = 0U; i4 < eltWiseBuffParams->numDIM1; i4++)
    {
      for (i5 = 0U; i5 < eltWiseBuffParams->numDIM2; i5++)
      {
        for (i1 = 0U; i1 < eltWiseBuffParams->numChannels; i1++)
        {
          for (i2 = 0U; i2 < eltWiseBuffParams->inHeight; i2++)
          {
            for (i3 = 0U; i3 < eltWiseBuffParams->inWidth; i3++)
            {
              uint32_t inOffset  = (i4 * eltWiseBuffParams->inDIM1Pitch) + (i5 * eltWiseBuffParams->inDIM2Pitch)+(i1 * eltWiseBuffParams->inChPitch) + (i2 * eltWiseBuffParams->inPitch) + (i3 * eltWiseBuffParams->pixelPitch);
              uint32_t outOffset = (i4 * eltWiseBuffParams->outDIM1Pitch) + (i5 * eltWiseBuffParams->outDIM2Pitch)+(i1 * eltWiseBuffParams->outChPitch) + (i2 * eltWiseBuffParams->outPitch) + i3;
              if(eltWiseBuffParams->eltWiseType == TIDL_EltWiseMax)
              {
                pAcc[outOffset] = ((pIn[inOffset] * scale) > pAcc[outOffset]) ? (pIn[inOffset] * scale) : pAcc[outOffset];
              }
              else
              {
                pAcc[outOffset] = ((pIn[inOffset] * scale) < pAcc[outOffset]) ? (pIn[inOffset] * scale) : pAcc[outOffset];
              }              
            }
          }
        }
      }
    }
  }
}

/**
 * @brief Eltwise layer Mod operation implementation
 *
 * @param pIn : Pointer to input buffer
 * @param pAcc : Pointer to accumulator buffer
 * @param scale : Scale to be applied on the input
 * @param zeropoint : zeropoint to be used while quantize/dequantize
 * @param eltWiseBuffParams : parameters of eltwise buffer
 * @param fmodValue : fmod attribute of Mod operator
 * @param callno : Variable to check if it is first call
 */
template <class Tin, class Tacc>
static void TIDL_refEltWiseMod(const Tin* pIn,
                               Tacc*   pAcc,
                               int32_t scale,
                               int32_t zeropoint,
                               const TIDL_EltWiseBuffParams_t *eltWiseBuffParams,
                               int32_t fmodValue,
                               int32_t callno)
{
  uint32_t i1, i2, i3, i4, i5;

  if (callno == 0)
  {
    for (i4 = 0U; i4 < eltWiseBuffParams->numDIM1; i4++)
    {
      for (i5 = 0U; i5 < eltWiseBuffParams->numDIM2; i5++)
      {
        for (i1 = 0U; i1 < eltWiseBuffParams->numChannels; i1++)
        {
          for (i2 = 0U; i2 < eltWiseBuffParams->inHeight; i2++)
          {
            for (i3 = 0U; i3 < eltWiseBuffParams->inWidth; i3++)
            {
              uint32_t inOffset = (i4 * eltWiseBuffParams->inDIM1Pitch) + (i5 * eltWiseBuffParams->inDIM2Pitch) + (i1 * eltWiseBuffParams->inChPitch) + (i2 * eltWiseBuffParams->inPitch) + (i3 * eltWiseBuffParams->pixelPitch);
              uint32_t outOffset = (i4 * eltWiseBuffParams->outDIM1Pitch) + (i5 * eltWiseBuffParams->outDIM2Pitch) + (i1 * eltWiseBuffParams->outChPitch) + (i2 * eltWiseBuffParams->outPitch) + i3;
              pAcc[outOffset] = pIn[inOffset];
            }
          }
        }
      }
    }
  }
  else
  {
    for (i4 = 0U; i4 < eltWiseBuffParams->numDIM1; i4++)
    {
      for (i5 = 0U; i5 < eltWiseBuffParams->numDIM2; i5++)
      {
        for (i1 = 0U; i1 < eltWiseBuffParams->numChannels; i1++)
        {
          for (i2 = 0U; i2 < eltWiseBuffParams->inHeight; i2++)
          {
            for (i3 = 0U; i3 < eltWiseBuffParams->inWidth; i3++)
            {
              uint32_t inOffset  = (i4 * eltWiseBuffParams->inDIM1Pitch) + (i5 * eltWiseBuffParams->inDIM2Pitch)+(i1 * eltWiseBuffParams->inChPitch) + (i2 * eltWiseBuffParams->inPitch) + (i3 * eltWiseBuffParams->pixelPitch);
              uint32_t outOffset = (i4 * eltWiseBuffParams->outDIM1Pitch) + (i5 * eltWiseBuffParams->outDIM2Pitch)+(i1 * eltWiseBuffParams->outChPitch) + (i2 * eltWiseBuffParams->outPitch) + i3;

              if(fmodValue == 1)
              {
                float32_tidl inAVal = (float32_tidl)((pAcc[outOffset] - zeropoint) * (1.0 / scale));
                float32_tidl inBVal = (float32_tidl)((pIn[inOffset] - zeropoint) * (1.0 / scale));
                float32_tidl outVal = fmod(inAVal, inBVal);
                pAcc[outOffset] = (Tacc)(outVal * scale + zeropoint);
              }
              else
              {
                /*
                 * fmod attribute is set to 0 i.e. inputs are constrained to integer data types hence % should be used
                 * According to onnx spec, mimicking the python % behavior
                 */
                int32_t inAVal = (int32_t)pAcc[outOffset];
                int32_t inBVal = (int32_t)pIn[inOffset];
                if(inBVal != 0)
                {
                  int32_t outVal = ((inAVal % inBVal) + inBVal) % inBVal;
                  pAcc[outOffset] = (Tacc)(outVal);
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
 * @brief Eltwise layer differnt operations implementation
 *
 * @param pIn : Pointer to input buffer
 * @param pAcc : Pointer to accumulator buffer
 * @param scale : Scale to be applied on the input
 * @param eltWiseBuffParams : parameters of eltwise buffer
 * @param params : Eltwise layerParams
 * @param callno : Variable to check if it is first call
 * @param isSigned : to check if one of the input is signed
 */
template <class Tin, class Tacc>
static void TIDL_refEltWiseOp(const Tin *pIn,
                              Tacc *pAcc,
                              int32_t scale,
                              int32_t zeropoint,
                              const TIDL_EltWiseBuffParams_t *eltWiseBuffParams,
                              const sTIDL_EltWiseParams_t *params,
                              int32_t callno,
                              int32_t batchno)
{
  #ifdef BUILD_WITH_CUDA_ELTWISE
  TIDL_cudaEltWiseOp<Tin, Tacc>(pIn, pAcc, scale, zeropoint,
                                eltWiseBuffParams->numDIM1, eltWiseBuffParams->numDIM2, 
                                eltWiseBuffParams->numChannels, eltWiseBuffParams->inHeight, eltWiseBuffParams->inWidth,
                                eltWiseBuffParams->inDIM1Pitch, eltWiseBuffParams->inDIM2Pitch, 
                                eltWiseBuffParams->inChPitch, eltWiseBuffParams->inPitch, eltWiseBuffParams->pixelPitch,
                                eltWiseBuffParams->outDIM1Pitch, eltWiseBuffParams->outDIM2Pitch, 
                                eltWiseBuffParams->outChPitch, eltWiseBuffParams->outPitch,
                                callno, batchno, eltWiseBuffParams->eltWiseType, params->fmodValue, eltWiseBuffParams->outBatchPitch);
  #else
  if(eltWiseBuffParams->eltWiseType == TIDL_EltWiseSum)
  {
    TIDL_refEltWiseSum(pIn, pAcc, scale, eltWiseBuffParams, callno, batchno);
  }
  else if (eltWiseBuffParams->eltWiseType == TIDL_EltWiseProduct)
  {
    TIDL_refEltWiseProduct(pIn, pAcc, scale, zeropoint, eltWiseBuffParams, callno);
  }
  else if((eltWiseBuffParams->eltWiseType == TIDL_EltWiseMax) || (eltWiseBuffParams->eltWiseType == TIDL_EltWiseMin))
  {
    TIDL_refEltWiseMinMax(pIn, pAcc, scale, eltWiseBuffParams, callno);
  }
  else if(eltWiseBuffParams->eltWiseType == TIDL_EltWiseMod)
  {
    TIDL_refEltWiseMod(pIn, pAcc, scale, zeropoint, eltWiseBuffParams, params->fmodValue, callno);
  }
  else
  {
    tidl_printf(0, "TIDL_EltWise Type %d is  Not supported !!!\n ", eltWiseBuffParams->eltWiseType);
  }
  #endif
}

template<class Tacc, class Tout> static void TIDL_refEltWiseMMAv2Quantize(TIDL_Handle intAlgHandle,
                                                                          int32_t layerIdx,
                                                                          Tacc *pAcc,
                                                                          Tout *pout,
                                                                          uint8_t mmaScale,
                                                                          uint8_t mmaShift,
                                                                          int32_t biasTerm,
                                                                          const TIDL_EltWiseBuffParams_t *eltWiseBuffParams,
                                                                          int32_t outElemType,
                                                                          int32_t tensorZeroPoint)
{

  #ifdef BUILD_WITH_CUDA_ELTWISE
  // call CUDA MMAV2 quantize wrapper
  TIDL_cudaEltWiseMMAv2Quantize<Tacc, Tout>(pAcc, pout, mmaScale, mmaShift, biasTerm,
                                            eltWiseBuffParams->numBatches, eltWiseBuffParams->numDIM1, eltWiseBuffParams->numDIM2,
                                            eltWiseBuffParams->numChannels, eltWiseBuffParams->inHeight, eltWiseBuffParams->inWidth,
                                            eltWiseBuffParams->outBatchPitch, eltWiseBuffParams->outDIM1Pitch, eltWiseBuffParams->outDIM2Pitch,
                                            eltWiseBuffParams->outChPitch, eltWiseBuffParams->outPitch, tensorZeroPoint, eltWiseBuffParams->outPadOffset);
  #else
  uint32_t i0, i1, i2, i3, i4, i5;
  /* Accumulator which stores the product after multiplication with mmaScale has to be 64bit in both 8/16 bit
     Tacc - 32bit for 8bit
     Tacc - 64bit for 16bit
     (Tacc * mmaScale) needs 40bit & 64bit in 8 & 16 bit, hence we keep the tempAcc in 64bit for both
  **/
  Tacc tempAcc;
  Tacc mmaAcc;

  // OPENACC(data present(pAcc[:1+((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1])
  //            copy(pout[:1+((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1]))

  // OPENACC(parallel loop collapse(5))
  for (i0 = 0U; i0 < eltWiseBuffParams->numBatches; i0++)
  {
    for (i1 = 0U; i1 < eltWiseBuffParams->numDIM1; i1++)
    {
      for (i2 = 0U; i2 < eltWiseBuffParams->numDIM2; i2++)
      {
        for (i3 = 0U; i3 < eltWiseBuffParams->numChannels; i3++)
        {
          for (i4 = 0U; i4 < eltWiseBuffParams->inHeight; i4++)
          {
            for (i5 = 0U; i5 < eltWiseBuffParams->inWidth; i5++)
            {
              uint32_t outOffset = (i0 * eltWiseBuffParams->outBatchPitch + i1 * eltWiseBuffParams->outDIM1Pitch) + (i2 * eltWiseBuffParams->outDIM2Pitch) + (i3 * eltWiseBuffParams->outChPitch) + (i4 * eltWiseBuffParams->outPitch) + i5;
              /* LDRA_JUSTIFY_START
               <metric start> branch <metric end>
               <justification start>
               Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
               Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
               This does not affect runtime behavior or safety.
               <justification end> */
              if (outElemType != TIDL_SinglePrecFloat)
              {
                /* LDRA_JUSTIFY_END */
                mmaAcc = pAcc[outOffset] + biasTerm;
                tempAcc = mmaAcc * mmaScale;
                pout[outOffset] = (Tout)TIDL_roundSat(tempAcc, mmaShift, std::numeric_limits<Tout>::lowest(), std::numeric_limits<Tout>::max());
              }
              /* LDRA_JUSTIFY_START
               <metric start> statement branch <metric end>
               <justification start>
               Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
               Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
               This does not affect runtime behavior or safety.
               <justification end> */
              else
              {
                /*Passthrough for float:*/
                pout[outOffset] = pAcc[outOffset];
              }
              /* LDRA_JUSTIFY_END */
            }
          }
        }
      }
    }
  }
  #endif
}
/**
 * @brief Eltwise layer quantization function
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx : Index of the current layer
 * @param pAcc : Pointer to accumulator buffer
 * @param pout : Pointer to ouptut buffer
 * @param eltWiseBuffParams : parameters of eltwise buffer
 * @param satLow : Min value of the saturation
 * @param satHigh : Max value of the saturation
 */
template<class Tacc, class Tout> static void TIDL_refEltWiseQuantize(TIDL_Handle intAlgHandle,
                                                                     int32_t layerIdx,
                                                                     const Tacc *pAcc,
                                                                     Tout *pout,
                                                                     const TIDL_EltWiseBuffParams_t *eltWiseBuffParams)
{
  uint32_t i0, i1, i2, i3, i4, i5;
  int32_t roundBits;
  Tacc outAcc;
  sTIDL_Network_t *net = intAlgHandle->createParams->net;
  sTIDL_Layer_t *tidlLayer = &intAlgHandle->createParams->net->TIDLLayers[layerIdx];
  Tacc min = 0, max = 0;

  min = std::numeric_limits<Tacc>::max();
  max = std::numeric_limits<Tacc>::lowest();

  // OPENACC(data present(pAcc[:1+((eltWiseBuffParams->numBatches-1) * eltWiseBuffParams->outBatchPitch) + ((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1]))

  //OPENACC(parallel loop collapse(5) firstprivate(min, max) reduction(min:min) reduction(max:max))

  // min max computed here is passed but unused in TIDL_UpdateScaleFactors() call and subsequent code in TIDL_refEltWiseQuantize
  // for(i0=0U; i0<eltWiseBuffParams->numBatches; i0++)
  // {
  // for (i1=0U; i1<eltWiseBuffParams->numDIM1; i1++)
  // {
  //   for (i2=0U; i2<eltWiseBuffParams->numDIM2; i2++)
  //   {
  //     for(i3=0U; i3<eltWiseBuffParams->numChannels; i3++)
  //     {
  //       for(i4=0U; i4<eltWiseBuffParams->inHeight; i4++)
  //       {
  //         for(i5=0U; i5<eltWiseBuffParams->inWidth; i5++)
  //         {
  //           uint32_t outOffset = (i0 * eltWiseBuffParams->outBatchPitch) + (i1* eltWiseBuffParams->outDIM1Pitch) + (i2* eltWiseBuffParams->outDIM2Pitch) + (i3* eltWiseBuffParams->outChPitch) + (i4 * eltWiseBuffParams->outPitch) + i5;
  //           outAcc = pAcc[outOffset];
  //           min = (outAcc<min) ? outAcc : min;
  //           max = (outAcc>max) ? outAcc : max;
  //         }
  //       }
  //     }
  //   }
  // }
  // }

  if ((intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
  {
    if (TIDL_getDatElementSign(net->TIDLLayers[layerIdx].outData.elementType) == 1)
    {
      TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, (int64_t)min, (int64_t)max);
    }
    else
    {
      TIDL_UpdateScaleFactors(intAlgHandle, layerIdx, 1, 0, (int64_t)max);
    }
  }

  roundBits = net->TIDLLayers[layerIdx].outData.roundBits;
  int32_t satLow;
  int32_t satHigh;
  int32_t mixedPrecision = 0;
  int32_t procElemSize;
  Tout temp = Tout();
  procElemSize = TIDL_getProcessingElementSizeInBytes(tidlLayer);

  temp = std::numeric_limits<Tout>::lowest();
  satLow = (int32_t)temp;
  temp = std::numeric_limits<Tout>::max();
  satHigh = (int32_t)(temp);

  if (sizeof(Tout) != (uint32_t)procElemSize)
  {
    mixedPrecision = 1;
    /* This is to match with target implementation wherin the saturations
    happens after applying outRoundBits - 8 shift and then the right shift
    of 8 is applied */
    roundBits = roundBits - 8;
    if (net->TIDLLayers[layerIdx].outData.elementType == TIDL_SignedChar)
    {
      satLow = std::numeric_limits<int16_t>::lowest();
      satHigh = std::numeric_limits<int16_t>::max();
    }
    else
    {
      satLow = std::numeric_limits<uint16_t>::lowest();
      satHigh = std::numeric_limits<uint16_t>::max();
    }
  }

  // OPENACC(data present(pAcc[:1+((eltWiseBuffParams->numBatches-1) * eltWiseBuffParams->outBatchPitch) + ((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1])
  //     copy(pout[:1+((eltWiseBuffParams->numBatches-1) * eltWiseBuffParams->outBatchPitch) + ((eltWiseBuffParams->numChannels-1) * eltWiseBuffParams->outChPitch) + ((eltWiseBuffParams->inHeight-1) * eltWiseBuffParams->outPitch) + eltWiseBuffParams->inWidth - 1]))

  //OPENACC(parallel loop collapse(5))
  #ifdef BUILD_WITH_CUDA_ELTWISE
  // call CUDA elementwise quantize wrapper
  float floatSatLow, floatSatHigh;
  TIDL_getSaturationFloat(&net->TIDLLayers[layerIdx], &floatSatLow, &floatSatHigh);
  TIDL_cudaEltWiseQuantize<Tacc, Tout>(pAcc, pout,
                                       eltWiseBuffParams->numBatches, eltWiseBuffParams->numDIM1, eltWiseBuffParams->numDIM2,
                                       eltWiseBuffParams->numChannels, eltWiseBuffParams->inHeight, eltWiseBuffParams->inWidth,
                                       eltWiseBuffParams->outBatchPitch, eltWiseBuffParams->outDIM1Pitch, eltWiseBuffParams->outDIM2Pitch,
                                       eltWiseBuffParams->outChPitch, eltWiseBuffParams->outPitch,
                                       roundBits, satLow, satHigh, mixedPrecision, floatSatLow, floatSatHigh);
  #else
  for(i0=0U; i0<eltWiseBuffParams->numBatches; i0++)
  {
    for (i1 = 0U; i1 < eltWiseBuffParams->numDIM1; i1++)
    {
      for (i2 = 0U; i2 < eltWiseBuffParams->numDIM2; i2++)
      {
        for (i3 = 0U; i3 < eltWiseBuffParams->numChannels; i3++)
        {
          for (i4 = 0U; i4 < eltWiseBuffParams->inHeight; i4++)
          {
            for (i5 = 0U; i5 < eltWiseBuffParams->inWidth; i5++)
            {
              uint32_t outOffset = (i0 * eltWiseBuffParams->outBatchPitch) + (i1 * eltWiseBuffParams->outDIM1Pitch) + (i2 * eltWiseBuffParams->outDIM2Pitch) + (i3 * eltWiseBuffParams->outChPitch) + (i4 * eltWiseBuffParams->outPitch) + i5;
              outAcc = pAcc[outOffset];
              if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat)
              {
                // OPENACC(routine(TIDL_floatSat))
                outAcc = TIDL_floatSat(outAcc, tidlLayer);
              }
               else if (tidlLayer->outData.elementType == TIDL_BFloat16)
              {
                outAcc = TIDL_BF16Sat(outAcc, tidlLayer);
              }
              else
              {
                // OPENACC(routine(TIDL_roundSat))
                outAcc = (Tacc)TIDL_roundSat((int64_t)outAcc, roundBits, satLow, satHigh);
                if (mixedPrecision == 1)
                {
                  outAcc = (int64_t)outAcc >> 8;
                }
              }
              pout[outOffset] = outAcc;
            }
          }
        }
      }
    }
  }
  #endif
}
/**
 * @brief Eltwise layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of eltwise layer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param numBatches : no of total Batches to be processed
 * @param eltWiseBuffParams : parameters of the eltwise buffer
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
static int32_t TIDL_refEltWiseProcess(TIDL_Handle intAlgHandle,
                                      const sTIDL_AlgLayer_t *algLayer,
                                      const sTIDL_Layer_t *tidlLayer,
                                      const sTIDL_EltWiseParams_t *params,
                                      void *inPtrs[],
                                      uint8_t *outPtr,
                                      uint32_t numBatches,
                                      TIDL_EltWiseBuffParams_t *eltWiseBuffParams)
{
  int32_t status = TIDL_SUCCESS;
  int32_t i4;
  uint32_t i;
  int32_t j;
  int32_t inDataScale[TIDL_NUM_IN_BUFS];
  float32_tidl accScale;
  int32_t *refAccPtr;
  int32_t layerIdx = algLayer->layerIdx;
  uint32_t accMemSize;
  int32_t accBitDepth = TIDL_SignedWord;
  int32_t outElementSize = TIDL_getDatElementSize(tidlLayer->outData.elementType); 
  int32_t procElemSize = TIDL_getProcessingElementSizeInBytes(tidlLayer);
  int32_t quantizationStyle = intAlgHandle->createParams->net->quantizationStyle;


  for (i = 0; i < TIDL_NUM_IN_BUFS; i++)
  {
    inDataScale[i] = 1;
  }

  for (i = 0; i < eltWiseBuffParams->numInData; i++)
  {
    sTIDL_DataParams_t *inDataParams;
    inDataParams = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[i]].outData;
    if ((inDataParams->elementType == TIDL_SignedShort) || (inDataParams->elementType == TIDL_UnsignedShort) ||
        (eltWiseBuffParams->eltWiseType == TIDL_EltWiseMod && (inDataParams->elementType == TIDL_SignedWord || inDataParams->elementType == TIDL_UnsignedWord)))
    {
      /* TIDL_LDRA_TAG_ELTWISE_PRIOR_CHECK_001 */
      accBitDepth = TIDL_SignedDoubleWord;
    }
  }

  uint8_t *outPtrLocal;
  outPtrLocal = (uint8_t *)outPtr;

  if (accBitDepth == TIDL_SignedDoubleWord)
  {
    accMemSize = numBatches * eltWiseBuffParams->outBatchPitch * ((uint32_t)sizeof(int64_t));
  }
  else // Regular situation (FP32/INT32 accumulator)
  {
    accMemSize = numBatches * eltWiseBuffParams->outBatchPitch * ((uint32_t)sizeof(int32_t));
  }

  if (algLayer->scratchSize >= (int32_t)accMemSize)
  {
    refAccPtr = (int32_t *)algLayer->scratchMem;
  }
  else
  {
    tidl_printf(0, "Memory for  TIDL_refEltWiseProcess accumulator is not sufficient exiting...\n    ");
    status = TIDL_ERR_FAILURE;
  }

  if (TIDL_SUCCESS == status)
  {

//#ifndef BUILD_WITH_OPENACC // Not needed if running on GPU
    if(accBitDepth == TIDL_SignedDoubleWord)
    {
      (void)memset(refAccPtr, 0, sizeof(int64_t) * numBatches * eltWiseBuffParams->outBatchPitch);
    }
    else
    {
      (void)memset(refAccPtr, 0, sizeof(int32_t) * numBatches * eltWiseBuffParams->outBatchPitch);
    }
    // #endif

    if (params->eltWiseType == TIDL_EltWiseProduct)
    {
      for (j = 0; j < tidlLayer->numInBufs; j++)
      {
        inDataScale[j] = 1;
      }
    }
    else if (params->eltWiseType == TIDL_EltWiseMod)
    {
      for (j = 0; j < tidlLayer->numInBufs; j++)
      {
        sTIDL_DataParams_t *inDataParams;
        inDataParams = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].outData;
        inDataScale[j] = inDataParams->tensorScale;
      }
    }
    else
    {
      uint32_t tempRoundBits = 0U;
      tempRoundBits = (((uint32_t)1) << (uint32_t)tidlLayer->outData.roundBits);
      accScale = tidlLayer->outData.tensorScale * (float32_tidl)tempRoundBits;
      for (j = 0; j < tidlLayer->numInBufs; j++)
      {
        sTIDL_DataParams_t *indata = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].outData;
        float32_tidl temp2 = accScale / indata->tensorScale;
        inDataScale[j] = (int32_t)temp2;
      }
    }
    uint8_t commonScale = 1;
    uint8_t mmaShift = 0;
    int64_t biasTerm = 0;
    if (TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE)
    {
      if (params->eltWiseType == TIDL_EltWiseSum)
      {
        // Currently for 2 tensor eltwise only.. fix!
        sTIDL_DataParams_t *inDataParams1;
        sTIDL_DataParams_t *inDataParams2;
        inDataParams1 = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
        inDataParams2 = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[1]].outData;
        float32_tidl scale1 = inDataParams1->tensorScale / tidlLayer->outData.tensorScale;
        float32_tidl scale2 = inDataParams2->tensorScale / tidlLayer->outData.tensorScale;
        uint8_t iScale1, iScale2;
        TIDL_getMMAv2_EltwiseScalesAndShift(scale1, scale2, &iScale1, &iScale2, &commonScale, &mmaShift);
        inDataScale[0] = iScale1;
        inDataScale[1] = iScale2;
        float32_tidl floatBias = tidlLayer->outData.tensorZeroPoint;
        floatBias -= (((inDataParams1->tensorScale * (float32_tidl)inDataParams1->tensorZeroPoint) + (inDataParams2->tensorScale * (float32_tidl)inDataParams2->tensorZeroPoint)) / tidlLayer->outData.tensorScale);
        // This term needs to be inverted by the mmaScale to be added to the accumulator:
        float32_tidl mmaFloatScale = ((float32_tidl)commonScale) / pow(2, (mmaShift));
        floatBias *= (1.0 / mmaFloatScale);
        biasTerm = round(floatBias);
      }
      else if (params->eltWiseType == TIDL_EltWiseProduct)
      {
        sTIDL_DataParams_t *inDataParams1;
        sTIDL_DataParams_t *inDataParams2;
        inDataParams1 = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
        inDataParams2 = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[1]].outData;
        float32_tidl scale1 = inDataParams1->tensorScale;
        float32_tidl scale2 = inDataParams2->tensorScale;
        float32_tidl scale = tidlLayer->outData.tensorScale;
        scale = (scale1 * scale2) / scale;
        TIDL_getMMAv2_ScaleAndShift(scale, &commonScale, &mmaShift);
        inDataScale[0] = 1;
        inDataScale[1] = 1;
        float32_tidl floatBias = ((float32_tidl)tidlLayer->outData.tensorZeroPoint * tidlLayer->outData.tensorScale) / (inDataParams1->tensorScale * inDataParams2->tensorScale);
        biasTerm = round(floatBias);
      }
      else
      {
        /*do nothing*/
      }
    }

    /*Asym support*/
    if ((TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)TRUE) &&
        (tidlLayer->outData.elementType != TIDL_SinglePrecFloat) &&
        (tidlLayer->outData.elementType != TIDL_BFloat16))
    {
      commonScale = params->mmaScale;
      mmaShift = params->mmaShift;
      biasTerm = params->biasTerm;

      if (eltWiseBuffParams->eltWiseType == TIDL_EltWiseSum)
      {
        for (j = 0; j < tidlLayer->numInBufs; j++)
        {
          if (tidlLayer->outData.elementType == TIDL_SignedShort || tidlLayer->outData.elementType == TIDL_UnsignedShort)
          {
            inDataScale[j] = *((int16_t *)eltWiseBuffParams->derivedScales + j);
          }
          else
          {
            inDataScale[j] = *((int8_t *)eltWiseBuffParams->derivedScales + j);
            inDataScale[j] = *((int8_t *)eltWiseBuffParams->derivedScales + j);
          }
        }
      }
      else if (eltWiseBuffParams->eltWiseType == TIDL_EltWiseProduct)
      {
        for (j = 0; j < tidlLayer->numInBufs; j++)
        {
          inDataScale[j] = 1;
        }
      }
      else
      {
        /* do nothing */
      }
    }
    // OPENACC(data present(refAccPtr[:numBatches*eltWiseBuffParams->outBatchPitch]))
    {
#ifdef BUILD_WITH_OPENACC
      // Need to zero out the accumulator
      // OPENACC(parallel loop)
      for (int i = 0; i < numBatches * eltWiseBuffParams->outBatchPitch; i++)
        refAccPtr[i] = 0;
#endif

      for (j = 0; j < (int32_t)numBatches; j++)
      {
        int32_t *refAccPtrRoi = refAccPtr + (j * eltWiseBuffParams->outBatchPitch);
        if (accBitDepth == TIDL_SignedDoubleWord)
        {
          refAccPtrRoi = (int32_t *)(((int64_t *)refAccPtr) + (j * eltWiseBuffParams->outBatchPitch));
        }
        for (i4 = 0; i4 < (int32_t)eltWiseBuffParams->numInData; i4++)
        {
          sTIDL_DataParams_t *inDataParams;
          inDataParams = &intAlgHandle->createParams->net->TIDLLayers[algLayer->inLayerIdx[i4]].outData;

          /**
           * Indirect broadcast: repeating pointer by changing the value of pitch
           * If input if only [W]: linePitch = 0, ChannelPitch = 0, BatchPitch = 0
           * If input if only [H,W]: ChannelPitch = 0, BatchPitch = 0
           * If input if only [C,H,W]: BatchPitch = 0
           *
           * Similar extension for DIM1 and DIM2
           */

          int32_t inBatchPitch;

          /**
           * Check if broadcast needed in DIM_1
           */
          if (inDataParams->dimValues[TIDL_DIM_DIM1] == 1)
          {
            eltWiseBuffParams->inDIM1Pitch = 0;
          }
          else
          {
            eltWiseBuffParams->inDIM1Pitch = (uint32_t)inDataParams->pitch[TIDL_DIM1_PITCH];
          }

          /**
           * Check if broadcast needed in DIM_2
           */
          if (inDataParams->dimValues[TIDL_DIM_DIM2] == 1)
          {
            eltWiseBuffParams->inDIM2Pitch = 0;
          }
          else
          {
            eltWiseBuffParams->inDIM2Pitch = (uint32_t)inDataParams->pitch[TIDL_DIM2_PITCH];
          }

          /**
           * Check if broadcast needed in batch dimension
           */
          if (inDataParams->dimValues[TIDL_DIM_BATCH] == 1)
          {
            inBatchPitch = 0;
          }
          else
          {
            inBatchPitch = (int32_t)inDataParams->pitch[TIDL_ROI_PITCH];
          }

          /**
           *  Check if broadcast needed in channel dimension
           */
          if (inDataParams->dimValues[TIDL_DIM_NUMCH] == 1)
          {
            eltWiseBuffParams->inChPitch = 0;
          }
          else
          {
            eltWiseBuffParams->inChPitch = (uint32_t)inDataParams->pitch[TIDL_CHANNEL_PITCH];
          }
          /**
           * Check if broadcast needed in height dimension
           */
          if (inDataParams->dimValues[TIDL_DIM_HEIGHT] == 1)
          {
            eltWiseBuffParams->inPitch = 0;
          }
          else
          {
            eltWiseBuffParams->inPitch = (uint32_t)inDataParams->pitch[TIDL_LINE_PITCH];
          }

        /**
         * Check if broadcast needed in width dimensino
        */
        if (inDataParams->dimValues[TIDL_DIM_WIDTH] == 1)
        {
          eltWiseBuffParams->pixelPitch = 0;
        }
        else
        {
          eltWiseBuffParams->pixelPitch = 1;
        }

          int32_t inZeroPoint = inDataParams->tensorZeroPoint;
          int32_t inElementSize = TIDL_getDatElementSize(inDataParams->elementType);
          void *inPtr = (uint8_t *)inPtrs[i4] +
                        (((j * inBatchPitch)) * inElementSize);

        if(inDataParams->elementType == TIDL_SignedChar)
        {
          TIDL_refEltWiseOp((int8_t*)inPtr,
                            refAccPtrRoi,
                            inDataScale[i4],
                            inZeroPoint,
                            eltWiseBuffParams,
                            params,
                            i4,
                            j);
        }
        else if(inDataParams->elementType == TIDL_UnsignedChar)
        {
          TIDL_refEltWiseOp((uint8_t*)inPtr,
                            refAccPtrRoi,
                            inDataScale[i4],
                            inZeroPoint,
                            eltWiseBuffParams,
                            params,
                            i4,
                            j);
        }
        else if(inDataParams->elementType == TIDL_SignedShort)
        {
          TIDL_refEltWiseOp((int16_t*)inPtr,
                            (int64_t*)refAccPtrRoi,
                            inDataScale[i4],
                            inZeroPoint,
                            eltWiseBuffParams,
                            params,
                            i4,
                            j);
        }
        else if(inDataParams->elementType == TIDL_UnsignedShort)
        {
          TIDL_refEltWiseOp((uint16_t*)inPtr,
                            (int64_t*)refAccPtrRoi,
                            inDataScale[i4],
                            inZeroPoint,
                            eltWiseBuffParams,
                            params,
                            i4,
                            j);
        }
        else if(inDataParams->elementType == TIDL_SignedWord)
        {
          TIDL_refEltWiseOp((int32_t *)inPtr,
                            (int64_t *)refAccPtrRoi,
                            1.0,
                            0,
                            eltWiseBuffParams,
                            params,
                            i4,
                            j);
        }
        else if(inDataParams->elementType == TIDL_SinglePrecFloat)
        {
          TIDL_refEltWiseOp((float32_tidl *)inPtr,
                            (float32_tidl *)refAccPtrRoi,
                            1.0,
                            0,
                            eltWiseBuffParams,
                            params,
                            i4,
                            j);
        }
        else if(inDataParams->elementType == TIDL_BFloat16)
        {
          TIDL_refEltWiseOp((bfloat16_tidl *)inPtr,
                            (float32_tidl *)refAccPtrRoi,
                            1.0,
                            0,
                            eltWiseBuffParams,
                            params,
                            i4,
                            j);
        }
        else
        {
          tidl_printf(0,"TIDL_EltWise in elementType is  Not supported !!!\n ");
        }
      }
    }

      if (((TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE) ||
           (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)TRUE)) &&
          (tidlLayer->outData.elementType != TIDL_SinglePrecFloat) &&
          (tidlLayer->outData.elementType != TIDL_BFloat16))
      {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
        Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
        This does not affect runtime behavior or safety.
        <justification end> */
        if (tidlLayer->outData.elementType != TIDL_SinglePrecFloat)
        /* LDRA_JUSTIFY_END */
        {
          if (accBitDepth == TIDL_SignedDoubleWord)
          {
            if (tidlLayer->outData.elementType == TIDL_SignedShort)
            {
              TIDL_refEltWiseMMAv2Quantize(intAlgHandle,
                                           layerIdx,
                                           (int64_t *)refAccPtr,
                                           (int16_t *)outPtrLocal,
                                           commonScale,
                                           mmaShift,
                                           biasTerm,
                                           eltWiseBuffParams,
                                           tidlLayer->outData.elementType,
                                           tidlLayer->outData.tensorZeroPoint);
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
            This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
            TIDL_LDRA_TAG : TIDL_LDRA_TAG_ELTWISE_PRIOR_CHECK_001
            Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
            This does not affect runtime behavior or safety.
            <justification end> */
            else if (tidlLayer->outData.elementType == TIDL_UnsignedShort)
            /* LDRA_JUSTIFY_END */
            {
              TIDL_refEltWiseMMAv2Quantize(intAlgHandle,
                                           layerIdx,
                                           (int64_t *)refAccPtr,
                                           (uint16_t *)outPtrLocal,
                                           commonScale,
                                           mmaShift,
                                           biasTerm,
                                           eltWiseBuffParams,
                                           tidlLayer->outData.elementType,
                                           tidlLayer->outData.tensorZeroPoint);
            }
          }
          else
          {
            if (tidlLayer->outData.elementType == TIDL_SignedChar)
            {
              TIDL_refEltWiseMMAv2Quantize(intAlgHandle,
                                           layerIdx,
                                           (int32_t *)refAccPtr,
                                           (int8_t *)outPtrLocal,
                                           commonScale,
                                           mmaShift,
                                           biasTerm,
                                           eltWiseBuffParams,
                                           tidlLayer->outData.elementType,
                                           tidlLayer->outData.tensorZeroPoint);
            }
            else if (tidlLayer->outData.elementType == TIDL_UnsignedChar)
            {
              TIDL_refEltWiseMMAv2Quantize(intAlgHandle,
                                           layerIdx,
                                           (int32_t *)refAccPtr,
                                           (uint8_t *)outPtrLocal,
                                           commonScale,
                                           mmaShift,
                                           biasTerm,
                                           eltWiseBuffParams,
                                           tidlLayer->outData.elementType,
                                           tidlLayer->outData.tensorZeroPoint);
            }
          }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
        Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
        This does not affect runtime behavior or safety.
        <justification end> */
        else
        {
          /*Float pass*/
          TIDL_refEltWiseMMAv2Quantize(intAlgHandle,
                                       layerIdx,
                                       (float32_tidl *)refAccPtr,
                                       (float32_tidl *)outPtrLocal,
                                       1,
                                       0,
                                       0,
                                       eltWiseBuffParams,
                                       tidlLayer->outData.elementType,
                                       tidlLayer->outData.tensorZeroPoint);
        }
        /* LDRA_JUSTIFY_END */
      }
      else
      {
        if (accBitDepth == TIDL_SignedDoubleWord)
        {
          /*8-bit output is possible in mixed precision scenario*/
          if (tidlLayer->outData.elementType == TIDL_SignedChar)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    (int64_t *)refAccPtr,
                                    (int8_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          else if (tidlLayer->outData.elementType == TIDL_UnsignedChar)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    (int64_t *)refAccPtr,
                                    (uint8_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          else if (tidlLayer->outData.elementType == TIDL_SignedShort)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    (int64_t *)refAccPtr,
                                    (int16_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start> 
          Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
          Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
          This does not affect runtime behavior or safety.
          <justification end> */
          else if (tidlLayer->outData.elementType == TIDL_UnsignedShort)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    (int64_t *)refAccPtr,
                                    (uint16_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          /* LDRA_JUSTIFY_END */
          else if (tidlLayer->outData.elementType == TIDL_SignedWord)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    (int64_t *)refAccPtr,
                                    (int32_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start> 
          Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
          Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
          This does not affect runtime behavior or safety.
          <justification end> */
          else
          {
            tidl_printf(0, "TIDL_EltWiseProduct out elementType is  Not supported !!!\n ");
          }
          /* LDRA_JUSTIFY_END */
        }
        else
        {
          if (tidlLayer->outData.elementType == TIDL_SignedChar)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    refAccPtr,
                                    (int8_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          else if (tidlLayer->outData.elementType == TIDL_UnsignedChar)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    refAccPtr,
                                    (uint8_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          else if (tidlLayer->outData.elementType == TIDL_SignedShort)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    refAccPtr,
                                    (int16_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          else if (tidlLayer->outData.elementType == TIDL_UnsignedShort)
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    refAccPtr,
                                    (uint16_t *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          else if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat) /* Use only to saturate at max/min */
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    (float32_tidl *)refAccPtr,
                                    (float32_tidl *)outPtrLocal,
                                    eltWiseBuffParams); /* Last 2 parameters not used for float */
          }
          else if (tidlLayer->outData.elementType == TIDL_BFloat16) /* Use only to saturate at max/min */
          {
            TIDL_refEltWiseQuantize(intAlgHandle,
                                    layerIdx,
                                    (float32_tidl *)refAccPtr,
                                    (bfloat16_tidl *)outPtrLocal,
                                    eltWiseBuffParams);
          }
          else
          {
            tidl_printf(0, "TIDL_EltWiseProduct out elementType is  Not supported !!!\n ");
          }
        }
      }
    }
  }
  TIDL_L1DandL2CacheWbInv();
  return status;
}

int32_t TIDL_eltWiseRefProcess(TIDL_NetworkCommonParams *commonParams,
                               sTIDL_AlgLayer_t *algLayer,
                               sTIDL_Layer_t *tidlLayer,
                               void *inPtrs[],
                               void *outPtrs[],
                               int32_t layerIdx)
{
  int32_t status = IALG_EOK;
  uint8_t (*outPtr)[] = (uint8_t (*)[])(outPtrs[0]);

  sTIDL_EltWiseParams_t *params = &tidlLayer->layerParams.eltWiseParams;
  sTIDL_DataParams_t *outDataParams;
  outDataParams = &commonParams->net->TIDLLayers[algLayer->layerIdx].outData;

  TIDL_EltWiseBuffParams_t eltWiseBuffParams;

  /**
   * All of the input dimension and pitch should be same as output
   * As these are used for loop limits
   * Final broadcasting is handled in loop - not necessary to handle here
   */

  uint32_t numBatches = (uint32_t)outDataParams->dimValues[TIDL_DIM_BATCH];
  eltWiseBuffParams.numChannels = (uint32_t)outDataParams->dimValues[TIDL_DIM_NUMCH];
  eltWiseBuffParams.numDIM1 = (uint32_t)outDataParams->dimValues[TIDL_DIM_DIM1];
  eltWiseBuffParams.inDIM1Pitch = (uint32_t)outDataParams->pitch[TIDL_DIM1_PITCH];
  eltWiseBuffParams.inDIM2Pitch = (uint32_t)outDataParams->pitch[TIDL_DIM2_PITCH];
  eltWiseBuffParams.numDIM2 = (uint32_t)outDataParams->dimValues[TIDL_DIM_DIM2];
  eltWiseBuffParams.numBatches = (uint32_t)outDataParams->dimValues[TIDL_DIM_BATCH];
  eltWiseBuffParams.inWidth = (uint32_t)outDataParams->dimValues[TIDL_DIM_WIDTH];
  eltWiseBuffParams.inHeight = (uint32_t)outDataParams->dimValues[TIDL_DIM_HEIGHT];
  eltWiseBuffParams.inPitch = (uint32_t)outDataParams->pitch[TIDL_LINE_PITCH];
  eltWiseBuffParams.inChPitch = (uint32_t)outDataParams->pitch[TIDL_CHANNEL_PITCH];
  eltWiseBuffParams.inBatchPitch = (uint32_t)outDataParams->pitch[TIDL_ROI_PITCH];
  eltWiseBuffParams.outPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  eltWiseBuffParams.outChPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  eltWiseBuffParams.outBatchPitch = (uint32_t)tidlLayer->outData.pitch[TIDL_ROI_PITCH];
  eltWiseBuffParams.outDIM1Pitch = (uint32_t)tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
  eltWiseBuffParams.outDIM2Pitch = (uint32_t)tidlLayer->outData.pitch[TIDL_DIM2_PITCH];
  eltWiseBuffParams.numInData = (uint16_t)tidlLayer->numInBufs;
  eltWiseBuffParams.eltWiseType = params->eltWiseType;
  #if defined(BUILD_WITH_CUDA_ELTWISE)
  /*Temporarily disabled - but ideally still needed because of offsets*/
  eltWiseBuffParams.outPadOffset = 0;// ((tidlLayer->outData.padH) * tidlLayer->outData.pitch[TIDL_LINE_PITCH]) + (tidlLayer->outData.padW);
  #else
  eltWiseBuffParams.outPadOffset = 0;
  #endif
  if (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)TRUE)
  {
    uint8_t *derivedScales = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), params->derivedScales);
    uint8_t *derivedShifts = (uint8_t *)get_int8_t_pointer((int8_t *)(commonParams->net), params->derivedShifts);
    int32_t *derivedBias = (int32_t *)get_int8_t_pointer((int8_t *)(commonParams->net), params->derivedBias);
    /*Update concat derived pointers*/
    eltWiseBuffParams.derivedScales = derivedScales;
    eltWiseBuffParams.derivedShifts = derivedShifts;
    eltWiseBuffParams.derivedBias = derivedBias;
  }
  else
  {
    eltWiseBuffParams.derivedScales = NULL;
    eltWiseBuffParams.derivedShifts = NULL;
    eltWiseBuffParams.derivedBias = NULL;
  }

  TIDL_Obj intAlgObj;
  TIDL_CreateParams createParams;

  (void)memcpy(&createParams, commonParams->createParams, sizeof(TIDL_CreateParams));
  intAlgObj.createParams = (TIDL_CreateParams *)&createParams;

  status = TIDL_refEltWiseProcess(&intAlgObj,
                                  algLayer,
                                  tidlLayer,
                                  params,
                                  inPtrs,
                                  (uint8_t *)outPtr,
                                  numBatches,
                                  &eltWiseBuffParams);

  return status;
}