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
@file    tidl_avx.c
@brief   This file contains AVX based keenels for Reference flow
@version 0.1 (Jan 2020) : Initial version [ADK]
----------------------------------------------------------------------------
*/



#include "tidl_avx.h"
#include <stdio.h>
#include <immintrin.h>
#include <iostream>
#include <typeinfo>
#include <float.h>

template <int Ksize>
void TIDL_refConv2dKernelAvxProcSP(
  float *   pInChannel,
  float *   pCoeffs,
  float *   pBias,
  float *   accPtr,
  float *   min,
  float *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 8;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  float inData;
  float acc;
  float *inDataPtr;
  float *aPtr;
  float lmin = *min;
  float lmax = *max;
  uint32_t mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  float coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;
  __m256 bias_sp8;
  float biasData;
  __m256 acc_sp8;
  __m256 inData_sp8;
  int32_t coeffOffset;
  __m256 coeff_sp8;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }

      bias_sp8 = _mm256_set1_ps(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        acc_sp8 = bias_sp8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                               i6 * numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
          for (i4 = 0; i4 < Ksize; i4++)
          {
            for (i5 = 0; i5 < Ksize; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
              coeff_sp8 = _mm256_set1_ps(coefData);
              inData_sp8 = _mm256_loadu_ps((const float *)inDataPtr);

              acc_sp8 = _mm256_fmadd_ps (coeff_sp8,inData_sp8, acc_sp8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_ps((float *)aPtr, acc_sp8);
      }
    }
  }
  if(remSize > 0)
  {
  for(j=0;j<remSize;j++)
  {
    mask[j]=1<<31;
  }
  __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
      bias_sp8 = _mm256_set1_ps(biasData);
      for (i3 = 0; i3 < 1; i3 += vecSize)
      {
        acc_sp8 = bias_sp8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                               i6 * numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
          for (i4 = 0; i4 < Ksize; i4++)
          {
            for (i5 = 0; i5 < Ksize; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 + numVecs +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
              coeff_sp8 = _mm256_set1_ps(coefData);
              inData_sp8 = _mm256_loadu_ps((const float *)inDataPtr);

              acc_sp8 = _mm256_fmadd_ps (coeff_sp8,inData_sp8, acc_sp8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3 + numVecs];
        _mm256_maskstore_ps((float *)aPtr, mask_0, acc_sp8);
      }
    }
  }
  }

  *min = -FLT_MAX;
  *max =  FLT_MAX;
}

void TIDL_refConv2dKernelAvxProcSPGen(
  float *   pInChannel,
  float *   pCoeffs,
  float *   pBias,
  float *   accPtr,
  float *   min,
  float *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  kw,
  int32_t  kh, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 8;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  float inData;
  float acc;
  float *inDataPtr;
  float *aPtr;
  float lmin = *min;
  float lmax = *max;
  uint32_t mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  float coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;
  __m256 bias_sp8;
  float biasData;
  __m256 acc_sp8;
  __m256 inData_sp8;
  int32_t coeffOffset;
  __m256 coeff_sp8;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }

      bias_sp8 = _mm256_set1_ps(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        acc_sp8 = bias_sp8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                               i6 * numInChannels * kw * kh + i0 * kw * kh;
          for (i4 = 0; i4 < kh; i4++)
          {
            for (i5 = 0; i5 < kw; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
              coeff_sp8 = _mm256_set1_ps(coefData);
              inData_sp8 = _mm256_loadu_ps((const float *)inDataPtr);

              acc_sp8 = _mm256_fmadd_ps (coeff_sp8,inData_sp8, acc_sp8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_ps((float *)aPtr, acc_sp8);
      }
    }
  }
  if(remSize > 0)
  {
  for(j=0;j<remSize;j++)
  {
    mask[j]=1<<31;
  }
  __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
      bias_sp8 = _mm256_set1_ps(biasData);
      for (i3 = 0; i3 < 1; i3 += vecSize)
      {
        acc_sp8 = bias_sp8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                               i6 * numInChannels * kw * kh + i0 * kw * kh;
          for (i4 = 0; i4 < kh; i4++)
          {
            for (i5 = 0; i5 < kw; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 + numVecs +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
              coeff_sp8 = _mm256_set1_ps(coefData);
              inData_sp8 = _mm256_loadu_ps((const float *)inDataPtr);

              acc_sp8 = _mm256_fmadd_ps (coeff_sp8,inData_sp8, acc_sp8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3 + numVecs];
        _mm256_maskstore_ps((float *)aPtr, mask_0, acc_sp8);
      }
    }
  }
  }

  *min = -FLT_MAX;
  *max =  FLT_MAX;
}



template <int Ksize, class Tin, class Tw, class Tb, class Tacc>
void TIDL_refConv2dKernelAvxProc8bit(
  Tin*     pInChannel,
  Tw*      pCoeffs,
  Tb*      pBias,
  Tacc *   accPtr,
  Tacc *   min,
  Tacc *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 16;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  Tin inData;
  Tacc acc;
  Tin *inDataPtr;
  Tacc *aPtr;
  Tacc lmin = *min;
  Tacc lmax = *max;
  Tacc mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  int16_t coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }

      __m256i bias_int8 = _mm256_set1_epi32(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        __m256i acc_int8_0 = bias_int8;
        __m256i acc_int8_1 = bias_int8;
        __m256i inData_short16;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                               i6 * numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
          for (i4 = 0; i4 < Ksize; i4++)
          {
            for (i5 = 0; i5 < Ksize; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
              __m256i coeff_short16 = _mm256_set1_epi16(coefData);
              if (std::is_same<Tin, int8_t>::value)
              {
                inData_short16 = _mm256_cvtepi8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if (std::is_same<Tin, uint8_t>::value)
              {
                inData_short16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi16(coeff_short16, inData_short16);
              __m256i out0_int8 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(out3));
              __m256i out1_int8 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(out3, 1));

              acc_int8_0 = _mm256_add_epi32(acc_int8_0, out0_int8);
              acc_int8_1 = _mm256_add_epi32(acc_int8_1, out1_int8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_si256((__m256i *)aPtr, acc_int8_0);
        _mm256_storeu_si256((__m256i *)(aPtr + 8), acc_int8_1);
      }
    }
  }
  if(remSize > 0)
  {
  for(j=0;j<remSize;j++)
  {
    mask[j]=1<<31;
  }
  __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);
  __m256i mask_1 = _mm256_loadu_si256((__m256i *)&mask[8]);

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;
    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
      __m256i bias_int8  = _mm256_set1_epi32(biasData);
      for (i3 = 0; i3 < 1; i3 ++)
      {
        __m256i acc_int8_0 = bias_int8;
        __m256i acc_int8_1 = bias_int8;
        __m256i inData_short16;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                                i6* numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
          for (i4 = 0; i4 < Ksize; i4++)
          {
            for (i5 = 0; i5 < Ksize; i5++)
            {
              inDataPtr = &pInChannel[indataOffset + i0* inChPitch + i3 + numVecs +
                (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
              __m256i coeff_short16  = _mm256_set1_epi16(coefData);
              if(std::is_same<Tin,int8_t>::value)
              {
                inData_short16 = _mm256_cvtepi8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if(std::is_same<Tin,uint8_t>::value)
              {
                inData_short16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi16(coeff_short16, inData_short16);
              __m256i out0_int8 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(out3));
              __m256i out1_int8 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(out3, 1));

              acc_int8_0 = _mm256_add_epi32(acc_int8_0, out0_int8);
              acc_int8_1 = _mm256_add_epi32(acc_int8_1, out1_int8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch +i3 + numVecs];
        _mm256_maskstore_epi32((int *)aPtr, mask_0, acc_int8_0);
        _mm256_maskstore_epi32((int *)(aPtr+8), mask_1, acc_int8_1);
      }
    }
  }
  }

  for (i6 = 0; i6 < numGroups*numOutChannels; i6++)
  {
    for (i3 = 0; i3 < height; i3 ++)
    {
      for (i4 = 0; i4 < width; i4 ++)
      {
        acc = accPtr[i6 * outChPitch + i3*inImPitch + i4];
        lmin = (acc < lmin) ? acc : lmin;
        lmax = (acc > lmax) ? acc : lmax;
      }
    }
  }
  *min = lmin;
  *max = lmax;
}
template <class Tin, class Tw, class Tb, class Tacc>
void TIDL_refConv2dKernelAvxProc8bitGen(
  Tin*     pInChannel,
  Tw*      pCoeffs,
  Tb*      pBias,
  Tacc *   accPtr,
  Tacc *   min,
  Tacc *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  kw,
  int32_t  kh, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 16;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  Tin inData;
  Tacc acc;
  Tin *inDataPtr;
  Tacc *aPtr;
  Tacc lmin = *min;
  Tacc lmax = *max;
  Tacc mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  int16_t coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }

      __m256i bias_int8 = _mm256_set1_epi32(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        __m256i acc_int8_0 = bias_int8;
        __m256i acc_int8_1 = bias_int8;
        __m256i inData_short16;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                               i6 * numInChannels * kw * kh + i0 * kw * kh;
          for (i4 = 0; i4 < kh; i4++)
          {
            for (i5 = 0; i5 < kw; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
              __m256i coeff_short16 = _mm256_set1_epi16(coefData);
              if (std::is_same<Tin, int8_t>::value)
              {
                inData_short16 = _mm256_cvtepi8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if (std::is_same<Tin, uint8_t>::value)
              {
                inData_short16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi16(coeff_short16, inData_short16);
              __m256i out0_int8 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(out3));
              __m256i out1_int8 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(out3, 1));

              acc_int8_0 = _mm256_add_epi32(acc_int8_0, out0_int8);
              acc_int8_1 = _mm256_add_epi32(acc_int8_1, out1_int8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_si256((__m256i *)aPtr, acc_int8_0);
        _mm256_storeu_si256((__m256i *)(aPtr + 8), acc_int8_1);
      }
    }
  }
  if(remSize > 0)
  {
  for(j=0;j<remSize;j++)
  {
    mask[j]=1<<31;
  }
  __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);
  __m256i mask_1 = _mm256_loadu_si256((__m256i *)&mask[8]);

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;
    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
      __m256i bias_int8  = _mm256_set1_epi32(biasData);
      for (i3 = 0; i3 < 1; i3 ++)
      {
        __m256i acc_int8_0 = bias_int8;
        __m256i acc_int8_1 = bias_int8;
        __m256i inData_short16;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                                i6* numInChannels * kw * kh + i0 * kw * kh;
          for (i4 = 0; i4 < kh; i4++)
          {
            for (i5 = 0; i5 < kw; i5++)
            {
              inDataPtr = &pInChannel[indataOffset + i0* inChPitch + i3 + numVecs +
                (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
              __m256i coeff_short16  = _mm256_set1_epi16(coefData);
              if(std::is_same<Tin,int8_t>::value)
              {
                inData_short16 = _mm256_cvtepi8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if(std::is_same<Tin,uint8_t>::value)
              {
                inData_short16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi16(coeff_short16, inData_short16);
              __m256i out0_int8 = _mm256_cvtepi16_epi32(_mm256_castsi256_si128(out3));
              __m256i out1_int8 = _mm256_cvtepi16_epi32(_mm256_extracti128_si256(out3, 1));

              acc_int8_0 = _mm256_add_epi32(acc_int8_0, out0_int8);
              acc_int8_1 = _mm256_add_epi32(acc_int8_1, out1_int8);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch +i3 + numVecs];
        _mm256_maskstore_epi32((int *)aPtr, mask_0, acc_int8_0);
        _mm256_maskstore_epi32((int *)(aPtr+8), mask_1, acc_int8_1);
      }
    }
  }
  }

  for (i6 = 0; i6 < numGroups*numOutChannels; i6++)
  {
    for (i3 = 0; i3 < height; i3 ++)
    {
      for (i4 = 0; i4 < width; i4 ++)
      {
        acc = accPtr[i6 * outChPitch + i3*inImPitch + i4];
        lmin = (acc < lmin) ? acc : lmin;
        lmax = (acc > lmax) ? acc : lmax;
      }
    }
  }
  *min = lmin;
  *max = lmax;
}

template <int Ksize, class Tin, class Tw, class Tb, class Tacc>
void TIDL_refConv2dKernelAvxProc16bit(
  Tin*     pInChannel,
  Tw*      pCoeffs,
  Tb*      pBias,
  Tacc *   accPtr,
  Tacc *   min,
  Tacc *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 8;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  Tin inData;
  Tacc acc;
  Tin *inDataPtr;
  Tacc *aPtr;
  Tacc lmin = *min;
  Tacc lmax = *max;
  Tacc mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  int16_t coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
     __m256i bias_long4 = _mm256_set1_epi64x(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        __m256i acc_long4_0 = bias_long4;
        __m256i acc_long4_1 = bias_long4;
        __m256i inData_int8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                               i6 * numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
          for (i4 = 0; i4 < Ksize; i4++)
          {
            for (i5 = 0; i5 < Ksize; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
              __m256i coeff_int8 = _mm256_set1_epi32(coefData);
              if (std::is_same<Tin, int16_t>::value)
              {
                inData_int8 = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if (std::is_same<Tin, uint16_t>::value)
              {
                inData_int8 = _mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
              __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
              __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

              acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
              acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_si256((__m256i *)aPtr, acc_long4_0);
        _mm256_storeu_si256((__m256i *)(aPtr + 4), acc_long4_1);
      }
    }
  }
  if(remSize > 0)
  {
    for(j=0;j<remSize;j++)
    {
      mask[j]=((Tacc)1)<<63;
    }
    __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);
    __m256i mask_1 = _mm256_loadu_si256((__m256i *)&mask[4]);

    for (i7 = 0; i7 < numGroups; i7++)
    {
      indataOffset  = i7 * numInChannels * inChPitch;
      outdataOffset = i7 * numOutChannels * outChPitch;

      for (i6 = 0; i6 < numOutChannels; i6++)
      {
        int32_t biasData = pBias[i7*numOutChannels + i6];
        if(!enableBias)
        {
            biasData = 0;
        }
        __m256i bias_long4 = _mm256_set1_epi64x(biasData);
        for (i3 = 0; i3 < 1; i3 ++)
        {
          __m256i acc_long4_0 = bias_long4;
          __m256i acc_long4_1 = bias_long4;
          __m256i inData_int8;
          for (i0 = 0; i0 < numInChannels; i0++)
          {
            int32_t coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                                i6 * numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
            for (i4 = 0; i4 < Ksize; i4++)
            {
              for (i5 = 0; i5 < Ksize; i5++)
              {
                inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 + numVecs +
                                        (i4 * inImPitch) + i5];
                coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
                __m256i coeff_int8 = _mm256_set1_epi32(coefData);
                if (std::is_same<Tin, int16_t>::value)
                {
                  inData_int8 = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                else if (std::is_same<Tin, uint16_t>::value)
                {
                  inData_int8 = _mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
                __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
                __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

                acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
                acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
              }
            }
          }
          aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3 + numVecs];
        _mm256_maskstore_epi64((long long int *)aPtr, mask_0, acc_long4_0);
        _mm256_maskstore_epi64((long long int *)(aPtr+4), mask_1, acc_long4_1);
        }
      }
    }
  }


  for (i6 = 0; i6 < numGroups*numOutChannels; i6++)
  {
    for (i3 = 0; i3 < height; i3 ++)
    {
      for (i4 = 0; i4 < width; i4 ++)
      {
        acc = accPtr[i6 * outChPitch + i3*inImPitch + i4];
        lmin = (acc < lmin) ? acc : lmin;
        lmax = (acc > lmax) ? acc : lmax;
      }
    }
  }
  *min = lmin;
  *max = lmax;
}


template <class Tin, class Tw, class Tb, class Tacc>
void TIDL_refConv2dKernelAvxProc16bitGen(
  Tin*     pInChannel,
  Tw*      pCoeffs,
  Tb*      pBias,
  Tacc *   accPtr,
  Tacc *   min,
  Tacc *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  kw,
  int32_t  kh, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 8;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  Tin inData;
  Tacc acc;
  Tin *inDataPtr;
  Tacc *aPtr;
  Tacc lmin = *min;
  Tacc lmax = *max;
  Tacc mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  int16_t coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
     __m256i bias_long4 = _mm256_set1_epi64x(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        __m256i acc_long4_0 = bias_long4;
        __m256i acc_long4_1 = bias_long4;
        __m256i inData_int8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                               i6 * numInChannels * kw * kh + i0 * kw * kh;
          for (i4 = 0; i4 < kw; i4++)
          {
            for (i5 = 0; i5 < kw; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
              __m256i coeff_int8 = _mm256_set1_epi32(coefData);
              if (std::is_same<Tin, int16_t>::value)
              {
                inData_int8 = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if (std::is_same<Tin, uint16_t>::value)
              {
                inData_int8 = _mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
              __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
              __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

              acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
              acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_si256((__m256i *)aPtr, acc_long4_0);
        _mm256_storeu_si256((__m256i *)(aPtr + 4), acc_long4_1);
      }
    }
  }
  if(remSize > 0)
  {
    for(j=0;j<remSize;j++)
    {
      mask[j]=((Tacc)1)<<63;
    }
    __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);
    __m256i mask_1 = _mm256_loadu_si256((__m256i *)&mask[4]);

    for (i7 = 0; i7 < numGroups; i7++)
    {
      indataOffset  = i7 * numInChannels * inChPitch;
      outdataOffset = i7 * numOutChannels * outChPitch;

      for (i6 = 0; i6 < numOutChannels; i6++)
      {
        int32_t biasData = pBias[i7*numOutChannels + i6];
        if(!enableBias)
        {
            biasData = 0;
        }
        __m256i bias_long4 = _mm256_set1_epi64x(biasData);
        for (i3 = 0; i3 < 1; i3 ++)
        {
          __m256i acc_long4_0 = bias_long4;
          __m256i acc_long4_1 = bias_long4;
          __m256i inData_int8;
          for (i0 = 0; i0 < numInChannels; i0++)
          {
            int32_t coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                                i6 * numInChannels * kw * kh + i0 * kw * kh;
            for (i4 = 0; i4 < kh; i4++)
            {
              for (i5 = 0; i5 < kw; i5++)
              {
                inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 + numVecs +
                                        (i4 * inImPitch) + i5];
                coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
                __m256i coeff_int8 = _mm256_set1_epi32(coefData);
                if (std::is_same<Tin, int16_t>::value)
                {
                  inData_int8 = _mm256_cvtepi16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                else if (std::is_same<Tin, uint16_t>::value)
                {
                  inData_int8 = _mm256_cvtepu16_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
                __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
                __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

                acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
                acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
              }
            }
          }
          aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3 + numVecs];
        _mm256_maskstore_epi64((long long int *)aPtr, mask_0, acc_long4_0);
        _mm256_maskstore_epi64((long long int *)(aPtr+4), mask_1, acc_long4_1);
        }
      }
    }
  }


  for (i6 = 0; i6 < numGroups*numOutChannels; i6++)
  {
    for (i3 = 0; i3 < height; i3 ++)
    {
      for (i4 = 0; i4 < width; i4 ++)
      {
        acc = accPtr[i6 * outChPitch + i3*inImPitch + i4];
        lmin = (acc < lmin) ? acc : lmin;
        lmax = (acc > lmax) ? acc : lmax;
      }
    }
  }
  *min = lmin;
  *max = lmax;
}


template <int Ksize, class Tin, class Tw, class Tb, class Tacc>
void TIDL_refConv2dKernelAvxIn8bitProc16bit(
  Tin*     pInChannel,
  Tw*      pCoeffs,
  Tb*      pBias,
  Tacc *   accPtr,
  Tacc *   min,
  Tacc *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 8;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  Tin inData;
  Tacc acc;
  Tin *inDataPtr;
  Tacc *aPtr;
  Tacc lmin = *min;
  Tacc lmax = *max;
  Tacc mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  int16_t coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
     __m256i bias_long4 = _mm256_set1_epi64x(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        __m256i acc_long4_0 = bias_long4;
        __m256i acc_long4_1 = bias_long4;
        __m256i inData_int8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                               i6 * numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
          for (i4 = 0; i4 < Ksize; i4++)
          {
            for (i5 = 0; i5 < Ksize; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
              __m256i coeff_int8 = _mm256_set1_epi32(coefData);
              if (std::is_same<Tin, int8_t>::value)
              {
                inData_int8 = _mm256_cvtepi8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if (std::is_same<Tin, uint8_t>::value)
              {
                inData_int8 = _mm256_cvtepu8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
              __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
              __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

              acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
              acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_si256((__m256i *)aPtr, acc_long4_0);
        _mm256_storeu_si256((__m256i *)(aPtr + 4), acc_long4_1);
      }
    }
  }
  if(remSize > 0)
  {
    for(j=0;j<remSize;j++)
    {
      mask[j]=((Tacc)1)<<63;
    }
    __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);
    __m256i mask_1 = _mm256_loadu_si256((__m256i *)&mask[4]);

    for (i7 = 0; i7 < numGroups; i7++)
    {
      indataOffset  = i7 * numInChannels * inChPitch;
      outdataOffset = i7 * numOutChannels * outChPitch;

      for (i6 = 0; i6 < numOutChannels; i6++)
      {
        int32_t biasData = pBias[i7*numOutChannels + i6];
        if(!enableBias)
        {
            biasData = 0;
        }
        __m256i bias_long4 = _mm256_set1_epi64x(biasData);
        for (i3 = 0; i3 < 1; i3 ++)
        {
          __m256i acc_long4_0 = bias_long4;
          __m256i acc_long4_1 = bias_long4;
          __m256i inData_int8;
          for (i0 = 0; i0 < numInChannels; i0++)
          {
            int32_t coeffOffset = i7*numInChannels * Ksize * Ksize *numOutChannels + \
                                i6 * numInChannels * Ksize * Ksize + i0 * Ksize * Ksize;
            for (i4 = 0; i4 < Ksize; i4++)
            {
              for (i5 = 0; i5 < Ksize; i5++)
              {
                inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 + numVecs +
                                        (i4 * inImPitch) + i5];
                coefData = pCoeffs[coeffOffset + (i4 * Ksize) + i5];
                __m256i coeff_int8 = _mm256_set1_epi32(coefData);
                if (std::is_same<Tin, int8_t>::value)
                {
                  inData_int8 = _mm256_cvtepi8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                else if (std::is_same<Tin, uint8_t>::value)
                {
                  inData_int8 = _mm256_cvtepu8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
                __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
                __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

                acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
                acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
              }
            }
          }
          aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3 + numVecs];
        _mm256_maskstore_epi64((long long int *)aPtr, mask_0, acc_long4_0);
        _mm256_maskstore_epi64((long long int *)(aPtr+4), mask_1, acc_long4_1);
        }
      }
    }
  }


  for (i6 = 0; i6 < numGroups*numOutChannels; i6++)
  {
    for (i3 = 0; i3 < height; i3 ++)
    {
      for (i4 = 0; i4 < width; i4 ++)
      {
        acc = accPtr[i6 * outChPitch + i3*inImPitch + i4];
        lmin = (acc < lmin) ? acc : lmin;
        lmax = (acc > lmax) ? acc : lmax;
      }
    }
  }
  *min = lmin;
  *max = lmax;
}


template <class Tin, class Tw, class Tb, class Tacc>
void TIDL_refConv2dKernelAvxIn8bitProc16bitGen(
  Tin*     pInChannel,
  Tw*      pCoeffs,
  Tb*      pBias,
  Tacc *   accPtr,
  Tacc *   min,
  Tacc *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  kw,
  int32_t  kh, // Wi
  int32_t  inImPitch,
  int32_t  enableBias
  )
{
  int32_t   i0, i2, i3, i4, i5, i6, i7, j;
  int32_t size = height * inImPitch;
  int32_t vecSize = 8;
  int32_t numVecs = 0;
  int32_t remSize = 0;
  Tin inData;
  Tacc acc;
  Tin *inDataPtr;
  Tacc *aPtr;
  Tacc lmin = *min;
  Tacc lmax = *max;
  Tacc mask[32] = {0};
  int32_t indataOffset;
  int32_t outdataOffset;

  int16_t coefData;
  numVecs = ((size / vecSize) * vecSize);
  remSize = size - numVecs;

  for (i7 = 0; i7 < numGroups; i7++)
  {
    indataOffset  = i7 * numInChannels * inChPitch;
    outdataOffset = i7 * numOutChannels * outChPitch;

    for (i6 = 0; i6 < numOutChannels; i6++)
    {
      int32_t biasData = pBias[i7*numOutChannels + i6];
      if(!enableBias)
      {
          biasData = 0;
      }
     __m256i bias_long4 = _mm256_set1_epi64x(biasData);
      for (i3 = 0; i3 < numVecs; i3 += vecSize)
      {
        __m256i acc_long4_0 = bias_long4;
        __m256i acc_long4_1 = bias_long4;
        __m256i inData_int8;
        for (i0 = 0; i0 < numInChannels; i0++)
        {
          int32_t coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                               i6 * numInChannels * kw * kh + i0 * kw * kh;
          for (i4 = 0; i4 < kw; i4++)
          {
            for (i5 = 0; i5 < kw; i5++)
            {
              inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 +
                                      (i4 * inImPitch) + i5];
              coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
              __m256i coeff_int8 = _mm256_set1_epi32(coefData);
              if (std::is_same<Tin, int8_t>::value)
              {
                inData_int8 = _mm256_cvtepi8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              else if (std::is_same<Tin, uint8_t>::value)
              {
                inData_int8 = _mm256_cvtepu8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
              }
              __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
              __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
              __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

              acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
              acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
            }
          }
        }
        aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3];
        _mm256_storeu_si256((__m256i *)aPtr, acc_long4_0);
        _mm256_storeu_si256((__m256i *)(aPtr + 4), acc_long4_1);
      }
    }
  }
  if(remSize > 0)
  {
    for(j=0;j<remSize;j++)
    {
      mask[j]=((Tacc)1)<<63;
    }
    __m256i mask_0 = _mm256_loadu_si256((__m256i *)&mask[0]);
    __m256i mask_1 = _mm256_loadu_si256((__m256i *)&mask[4]);

    for (i7 = 0; i7 < numGroups; i7++)
    {
      indataOffset  = i7 * numInChannels * inChPitch;
      outdataOffset = i7 * numOutChannels * outChPitch;

      for (i6 = 0; i6 < numOutChannels; i6++)
      {
        int32_t biasData = pBias[i7*numOutChannels + i6];
        if(!enableBias)
        {
            biasData = 0;
        }
        __m256i bias_long4 = _mm256_set1_epi64x(biasData);
        for (i3 = 0; i3 < 1; i3 ++)
        {
          __m256i acc_long4_0 = bias_long4;
          __m256i acc_long4_1 = bias_long4;
          __m256i inData_int8;
          for (i0 = 0; i0 < numInChannels; i0++)
          {
            int32_t coeffOffset = i7*numInChannels * kw * kh *numOutChannels + \
                                i6 * numInChannels * kw * kh + i0 * kw * kh;
            for (i4 = 0; i4 < kh; i4++)
            {
              for (i5 = 0; i5 < kw; i5++)
              {
                inDataPtr = &pInChannel[indataOffset+ i0 * inChPitch + i3 + numVecs +
                                        (i4 * inImPitch) + i5];
                coefData = pCoeffs[coeffOffset + (i4 * kw) + i5];
                __m256i coeff_int8 = _mm256_set1_epi32(coefData);
                if (std::is_same<Tin, int8_t>::value)
                {
                  inData_int8 = _mm256_cvtepi8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                else if (std::is_same<Tin, uint8_t>::value)
                {
                  inData_int8 = _mm256_cvtepu8_epi32(_mm_loadu_si128((__m128i *)inDataPtr));
                }
                __m256i out3 = _mm256_mullo_epi32(coeff_int8, inData_int8);
                __m256i out0_long4 = _mm256_cvtepi32_epi64(_mm256_castsi256_si128(out3));
                __m256i out1_long4 = _mm256_cvtepi32_epi64(_mm256_extracti128_si256(out3, 1));

                acc_long4_0 = _mm256_add_epi64(acc_long4_0, out0_long4);
                acc_long4_1 = _mm256_add_epi64(acc_long4_1, out1_long4);
              }
            }
          }
          aPtr = &accPtr[outdataOffset + i6 * outChPitch + i3 + numVecs];
        _mm256_maskstore_epi64((long long int *)aPtr, mask_0, acc_long4_0);
        _mm256_maskstore_epi64((long long int *)(aPtr+4), mask_1, acc_long4_1);
        }
      }
    }
  }


  for (i6 = 0; i6 < numGroups*numOutChannels; i6++)
  {
    for (i3 = 0; i3 < height; i3 ++)
    {
      for (i4 = 0; i4 < width; i4 ++)
      {
        acc = accPtr[i6 * outChPitch + i3*inImPitch + i4];
        lmin = (acc < lmin) ? acc : lmin;
        lmax = (acc > lmax) ? acc : lmax;
      }
    }
  }
  *min = lmin;
  *max = lmax;
}

/**
----------------------------------------------------------------------------
@fn         TIDL_avxSum
@brief      Function performs summation using AVX intrinsics

@param      pIn : Input stream
@param      pAcc : Accumulator stream
@param      scale : Scale Factor
@param      isSigned : Signed/Unsigned flag
@param      numChannels : Number of channels
@param      inHeight : Input Height
@param      inWidth : Number of batches
@param      inChPitch : Number of groups
@param      inPitch : Number of input channels
@param      outChPitch : Number of output channels
@param      outPitch : Input channel pitch

@remarks    None
@return     Zero on success
----------------------------------------------------------------------------
*/
template <class Tin, class Tacc>
void TIDL_avxSum(Tin *pIn, Tacc *pAcc, int scale, int isSigned, int numChannels, int inHeight, int  inWidth, int inChPitch, int inPitch, int outChPitch, int outPitch)
{
  int i1,i2,i3;
  if(!isSigned)
  {
    for (i1 = 0; i1 < numChannels; i1++)
    {
      for (i2 = 0; i2 < (inHeight); i2++)
      {
        int32_t inOffset = (i1* inChPitch) + (i2 * inPitch);
        int32_t outOffset = (i1* outChPitch) + (i2 * outPitch);
        short int a[10],j;
        for(j=0;j<10;j++)a[j]=-1;
        int n8b = (inWidth)/8; //How many perfect multiples of 8
        int rem = (inWidth)%8; //Leftovers
        for(j=0;j<n8b;j++)
        {
          __m256i mask =  _mm256_set_epi32 (a[0],a[1], a[2],a[3], a[4], a[5], a[6], a[7]);
          __m256i scale1=  _mm256_set_epi32 (scale,scale,scale,scale,scale,scale,scale,scale);
          __m256i loaded = _mm256_loadu_si256((__m256i*)(pIn+inOffset+j*8));
          __m256i loaded1 = _mm256_loadu_si256((__m256i*)(pAcc+outOffset+j*8));  //Loading 256 bit int
          __m128i int1 = _mm256_castsi256_si128 (loaded); //Cast 256 bit to 128 bit
          __m256i int2 = _mm256_cvtepu8_epi32 (int1); //8 * 8 bit chars to 32 bit ints
          __m256i intermediate = _mm256_mullo_epi32 (int2,scale1);
          __m256i yx = _mm256_add_epi32 (intermediate, loaded1);
          _mm256_maskstore_epi32 (pAcc+outOffset+j*8,mask,yx);
        }
        int ecount=j;
        if(rem>0)
        {
          for(j=0;j<(8-rem);j++)
          {
            a[j]=+1;
          }
          __m256i mask =  _mm256_set_epi32 (a[0],a[1], a[2],a[3], a[4], a[5], a[6], a[7]);
          __m256i scale1=  _mm256_set_epi32 (scale,scale,scale,scale,scale,scale,scale,scale);
          __m256i loaded = _mm256_loadu_si256((__m256i*)(pIn+inOffset+ecount*8));
          __m256i loaded1 = _mm256_loadu_si256((__m256i*)(pAcc+outOffset+ecount*8));  //Loading 256 bit int
          __m128i int1 = _mm256_castsi256_si128 (loaded); //Cast 256 bit to 128 bit
          __m256i int2 = _mm256_cvtepu8_epi32 (int1); //8 * 8 bit chars to 32 bit ints
          __m256i intermediate = _mm256_mullo_epi32 (int2,scale1);
          __m256i yx = _mm256_add_epi32 (intermediate, loaded1);
          _mm256_maskstore_epi32 (pAcc+outOffset+ecount*8,mask,yx);
        }
      }
    }
  }
  else
  {
    for (i1 = 0; i1 < numChannels; i1++)
    {
      for (i2 = 0; i2 < (inHeight); i2++)
      {
        int32_t inOffset = (i1* inChPitch) + (i2 * inPitch);
        int32_t outOffset = (i1* outChPitch) + (i2 * outPitch);
        short int a[10],j;
        for(j=0;j<10;j++)a[j]=-1;
        int n8b = (inWidth)/8; //How many perfect multiples of 8
        int rem = (inWidth)%8; //Leftovers
        for(j=0;j<n8b;j++)
        {
          __m256i mask =  _mm256_set_epi32 (a[0],a[1], a[2],a[3], a[4], a[5], a[6], a[7]);
          __m256i scale1=  _mm256_set_epi32 (scale,scale,scale,scale,scale,scale,scale,scale);
          __m256i loaded = _mm256_loadu_si256((__m256i*)(pIn+inOffset+j*8));
          __m256i loaded1 = _mm256_loadu_si256((__m256i*)(pAcc+outOffset+j*8));  //Loading 256 bit int
          __m128i int1 = _mm256_castsi256_si128 (loaded); //Cast 256 bit to 128 bit
          __m256i int2 = _mm256_cvtepi8_epi32 (int1); //8 * 8 bit chars to 32 bit ints
          __m256i intermediate = _mm256_mullo_epi32 (int2,scale1);
          __m256i yx = _mm256_add_epi32 (intermediate, loaded1);
         _mm256_maskstore_epi32 (pAcc+outOffset+j*8,mask,yx);
        }
        int ecount=j;
        if(rem>0)
        {
          for(j=0;j<(8-rem);j++)
          {
            a[j]=+1;
          }
          __m256i mask =  _mm256_set_epi32 (a[0],a[1], a[2],a[3], a[4], a[5], a[6], a[7]);
          __m256i scale1=  _mm256_set_epi32 (scale,scale,scale,scale,scale,scale,scale,scale);
          __m256i loaded = _mm256_loadu_si256((__m256i*)(pIn+inOffset+ecount*8));
          __m256i loaded1 = _mm256_loadu_si256((__m256i*)(pAcc+outOffset+ecount*8));  //Loading 256 bit int
          __m128i int1 = _mm256_castsi256_si128 (loaded); //Cast 256 bit to 128 bit
          __m256i int2 = _mm256_cvtepi8_epi32 (int1); //8 * 8 bit chars to 32 bit ints
          __m256i intermediate = _mm256_mullo_epi32 (int2,scale1);
          __m256i yx = _mm256_add_epi32 (intermediate, loaded1);
          _mm256_maskstore_epi32 (pAcc+outOffset+ecount*8,mask,yx);
        }
      }
    }
  }
}

/**
----------------------------------------------------------------------------
@fn         TIDL_avxSaturate
@brief      Function performs saturation using AVX intrinsics

@param      pIn : Input stream
@param      pAcc : Accumulator stream
@param      scale : Scale Factor
@param      isSigned : Signed/Unsigned flag
@param      numChannels : Number of channels
@param      inHeight : Input Height
@param      inWidth : Number of batches
@param      inChPitch : Number of groups
@param      inPitch : Number of input channels
@param      outChPitch : Number of output channels
@param      outPitch : Input channel pitch

@remarks    None
@return     Zero on success
----------------------------------------------------------------------------
*/
template <class Tout, class Tacc>
void TIDL_avxSaturate(Tout *pout, Tacc *pAcc, int numChannels, int inHeight, int inWidth,int outChPitch, int outPitch, int satLow, int satHigh, int roundBits )//Tout* , Tacc*
{
  int outOffset,outAcc;
  int i1,i2,i3,ik;
  int saturationTerm;
  if(roundBits!=0)
  {
    saturationTerm=1<<(roundBits-1);
  }
          __m256i mask =  _mm256_set_epi32 (-1,-1,-1,-1,-1,-1,-1,-1);
          __m256i sLow =  _mm256_set_epi32 (satLow,satLow,satLow,satLow,satLow,satLow,satLow,satLow);
          __m256i sHigh =  _mm256_set_epi32 (satHigh,satHigh,satHigh,satHigh,satHigh,satHigh,satHigh,satHigh);
          __m256i sat1 =  _mm256_set_epi32(saturationTerm,saturationTerm,saturationTerm,saturationTerm,saturationTerm,saturationTerm,saturationTerm,saturationTerm);
  if(typeid(Tout)==typeid(unsigned char)||1)
  {
    __m256i r2;
    for (i1 = 0; i1 < numChannels; i1++)
    {
      for (i2 = 0; i2 < (inHeight); i2++)
      {
        int32_t outOffset = (i1* outChPitch) + (i2 * outPitch);
        int32_t j;
        int n8b = (inWidth)/8 +1; //How many perfect multiples of 8
        int rem = (inWidth)%8; //Leftovers
        for(j=0;j<n8b;j++)
        {
          __m256i loaded = _mm256_maskload_epi32 ((pAcc+outOffset+j*8), mask);
          if(roundBits!=0)
          {
            __m256i rn = _mm256_add_epi32 (loaded, sat1);
            r2 = _mm256_srai_epi32(rn, roundBits);
          }
          else
          {
            r2=loaded;
          }
          __m256i r3 = _mm256_cmpgt_epi32 (sLow,r2);
          __m256i r4 = _mm256_blendv_epi8 (r2, sLow, r3);
          __m256i r5 = _mm256_cmpgt_epi32(r4,sHigh);
          __m256i r6 = _mm256_blendv_epi8(r4,sHigh,r5);
          //Copying out:
          int32_t *x = (int32_t*)&(r6);
          int k;
          for(k=0;k<8;k++)
          {
            pout[outOffset+j*8+k]=(unsigned  char)x[k];
          }
        }
      }
    }
  }
}

template <>
void TIDL_refConv2dKernelAvxProc <float,  float, float, float>(
  float *     pInChannel,
  float *      pCoeffs,
  float *      pBias,
  float *   accPtr,
  float *   min,
  float *   max,
  int32_t  numGroups,
  int32_t  numInChannels, //Ci
  int32_t  numOutChannels, //Co
  int32_t  inChPitch, //CiStride
  int32_t  outChPitch, //CoStide
  int32_t  width,
  int32_t  height, // Wi
  int32_t  inImPitch,
  int32_t coeffsWidth,
  int32_t coeffsHeight,
  int32_t  enableBias)
  {
      if ((coeffsWidth == 1) && (coeffsHeight == 1))
      {
        TIDL_refConv2dKernelAvxProcSP<1>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
        numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
      }
      else if ((coeffsWidth == 3) && (coeffsHeight == 3))
      {
        TIDL_refConv2dKernelAvxProcSP<3>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
        numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
      }
      else
      {
        TIDL_refConv2dKernelAvxProcSPGen(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
        numOutChannels, inChPitch, outChPitch, width, height, coeffsWidth, coeffsHeight, inImPitch,enableBias);
      }
  }

  template <class Tin, class Tw, class Tb, class Tacc>
  void TIDL_refConv2dKernelAvxProc(
      Tin *pInChannel,
      Tw *pCoeffs,
      Tb *pBias,
      Tacc *accPtr,
      Tacc *min,
      Tacc *max,
      int32_t numGroups,
      int32_t numInChannels,  //Ci
      int32_t numOutChannels, //Co
      int32_t inChPitch,      //CiStride
      int32_t outChPitch,     //CoStide
      int32_t width,
      int32_t height, // Wi
      int32_t inImPitch,
      int32_t coeffsWidth,
      int32_t coeffsHeight,
     int32_t  enableBias)
  {
      if ((coeffsWidth == 1) && (coeffsHeight == 1))
      {
        if ((std::is_same<Tw, int8_t>::value) && ((std::is_same<Tin, int8_t>::value) || (std::is_same<Tin, uint8_t>::value)))
        {
          TIDL_refConv2dKernelAvxProc8bit<1, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
        }
        else if ((std::is_same<Tw, int16_t>::value) && ((std::is_same<Tin, int16_t>::value) || (std::is_same<Tin, uint16_t>::value)))
        {
          TIDL_refConv2dKernelAvxProc16bit<1, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
        }
        else if ((std::is_same<Tw, int16_t>::value) && ((std::is_same<Tin, int8_t>::value) || (std::is_same<Tin, uint8_t>::value)))
        {
          TIDL_refConv2dKernelAvxIn8bitProc16bit<1, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
        }
      }
      else if ((coeffsWidth == 3) && (coeffsHeight == 3))
      {
        if ((std::is_same<Tw, int8_t>::value) && ((std::is_same<Tin, int8_t>::value) || (std::is_same<Tin, uint8_t>::value)))
        {
          TIDL_refConv2dKernelAvxProc8bit<3, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
        }
        else if ((std::is_same<Tw, int16_t>::value) && ((std::is_same<Tin, int16_t>::value) || (std::is_same<Tin, uint16_t>::value)))
        {
          TIDL_refConv2dKernelAvxProc16bit<3, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
        }
        else if ((std::is_same<Tw, int16_t>::value) && ((std::is_same<Tin, int8_t>::value) || (std::is_same<Tin, uint8_t>::value)))
        {
          TIDL_refConv2dKernelAvxIn8bitProc16bit<3, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, inImPitch,enableBias);
        }

      }
      else
      {
        if ((std::is_same<Tw, int8_t>::value) && ((std::is_same<Tin, int8_t>::value) || (std::is_same<Tin, uint8_t>::value)))
        {
          TIDL_refConv2dKernelAvxProc8bitGen<Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, coeffsWidth, coeffsHeight, inImPitch,enableBias);
        }
        else if ((std::is_same<Tw, int16_t>::value) && ((std::is_same<Tin, int16_t>::value) || (std::is_same<Tin, uint16_t>::value)))
        {
          TIDL_refConv2dKernelAvxProc16bitGen<Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, coeffsWidth, coeffsHeight, inImPitch,enableBias);
        }
        else if ((std::is_same<Tw, int16_t>::value) && ((std::is_same<Tin, int8_t>::value) || (std::is_same<Tin, uint8_t>::value)))
        {
          TIDL_refConv2dKernelAvxIn8bitProc16bitGen<Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, min, max, numGroups, numInChannels,
          numOutChannels, inChPitch, outChPitch, width, height, coeffsWidth, coeffsHeight, inImPitch,enableBias);
        }

      }
  }



template void TIDL_avxSaturate<unsigned char, int>(unsigned char*, int*, int, int, int, int, int, int, int, int);
template void TIDL_avxSaturate<signed char, int>(signed char*, int*, int, int, int, int, int, int, int ,int );
template void TIDL_avxSum<unsigned char,int>(unsigned char *pIn, int *pAcc, int scale, int isSigned, int numChannels, int inHeight, int  inWidth, int inChPitch, int inPitch, int outChPitch, int outPitch);
template void TIDL_avxSum<signed char,int>(signed char *pIn, int *pAcc, int scale, int isSigned, int numChannels, int inHeight, int  inWidth, int inChPitch, int inPitch, int outChPitch, int outPitch);


template void TIDL_refConv2dKernelAvxProc <float, float, float, float> (
    float *pInChannel,
    float *pCoeffs,
    float *pBias,
    float *accPtr,
    float *min,
    float *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);

template void TIDL_refConv2dKernelAvxProc <uint8_t, int8_t, int16_t, int32_t> (
    uint8_t *pInChannel,
    int8_t *pCoeffs,
    int16_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);
  template void TIDL_refConv2dKernelAvxProc <int8_t, int8_t, int16_t, int32_t> (
    int8_t *pInChannel,
    int8_t *pCoeffs,
    int16_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);
  template void TIDL_refConv2dKernelAvxProc <uint16_t, int16_t, int32_t, int64_t> (
    uint16_t *pInChannel,
    int16_t *pCoeffs,
    int32_t *pBias,
    int64_t *accPtr,
    int64_t *min,
    int64_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);
    template void TIDL_refConv2dKernelAvxProc <int16_t, int16_t, int32_t, int64_t> (
    int16_t *pInChannel,
    int16_t *pCoeffs,
    int32_t *pBias,
    int64_t *accPtr,
    int64_t *min,
    int64_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);


    template void TIDL_refConv2dKernelAvxProc <uint8_t, int16_t, int32_t, int64_t> (
    uint8_t *pInChannel,
    int16_t *pCoeffs,
    int32_t *pBias,
    int64_t *accPtr,
    int64_t *min,
    int64_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);
  template void TIDL_refConv2dKernelAvxProc <int8_t, int16_t, int32_t, int64_t> (
    int8_t *pInChannel,
    int16_t *pCoeffs,
    int32_t *pBias,
    int64_t *accPtr,
    int64_t *min,
    int64_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);
  template void TIDL_refConv2dKernelAvxProc <uint16_t, int8_t, int16_t, int32_t> (
    uint16_t *pInChannel,
    int8_t *pCoeffs,
    int16_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);
    template void TIDL_refConv2dKernelAvxProc <int16_t, int8_t, int16_t, int32_t> (
    int16_t *pInChannel,
    int8_t *pCoeffs,
    int16_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);


    template void TIDL_refConv2dKernelAvxProc <int8_t, int8_t, int32_t, int32_t> (
    int8_t *pInChannel,
    int8_t *pCoeffs,
    int32_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);

    template void TIDL_refConv2dKernelAvxProc <uint8_t, int8_t, int32_t, int32_t> (
    uint8_t *pInChannel,
    int8_t *pCoeffs,
    int32_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);

    template void TIDL_refConv2dKernelAvxProc <uint16_t, int8_t, int32_t, int32_t> (
    uint16_t *pInChannel,
    int8_t *pCoeffs,
    int32_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);

    template void TIDL_refConv2dKernelAvxProc <int16_t, int8_t, int32_t, int32_t> (
    int16_t *pInChannel,
    int8_t *pCoeffs,
    int32_t *pBias,
    int32_t *accPtr,
    int32_t *min,
    int32_t *max,
    int32_t numGroups,
    int32_t numInChannels,  //Ci
    int32_t numOutChannels, //Co
    int32_t inChPitch,      //CiStride
    int32_t outChPitch,     //CoStide
    int32_t width,
    int32_t height, // Wi
    int32_t inImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t  enableBias);

template void TIDL_refConv2dKernelAvxProc<signed char, short, long, long>(signed char*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int);
template void TIDL_refConv2dKernelAvxProc<unsigned char, short, long, long>(unsigned char*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int);
template void TIDL_refConv2dKernelAvxProc<short, short, long, long>(short*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int);
template void TIDL_refConv2dKernelAvxProc<unsigned short, short, long, long>(unsigned short*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int);
