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
@file    tidl_conv2d_base_ref.c
@brief   This file contains basic functions required by Convolution Layer
@version 0.1 (Oct 2016) : Initial version [ADK]
@version 0.5 (Jan 2017) : Cleaned up [ADK]
----------------------------------------------------------------------------
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <limits>

#include "tidl_alg_int.h"
#include "tidl_deconv2d.h"
#include "tidl_deconv2d_ref.h"
#include "tidl_conv2d_ref.h"
#include "tidl_conv2d_spatialPooling.h"
#include "tidl_commonUtils.h"
#include "tidl_types.h"
#include "tidl_depthToSpace_ref.h"
#include "tidl_forceNegativeTest.h"
#include "tidl_alg_utils_ref.h"

#include <math.h>
#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
#endif
#if defined(_HOST_BUILD)
#define _POSIX_C_SOURCE 200112L
#include "tidl_avx.h"
#include <cmath>
#endif
#include <stdlib.h>

#define DUMP_MMA_TEST_CASE (0)

#define USE_16BIT_BIAS_FOR_8BIT_MODE (1)

#if DUMP_MMA_TEST_CASE
void TIDL_WriteMemAsUint8Array(FILE *fp, uint8_t *ptr, int32_t size, const char arrayName[], const char secName[])
{
  int32_t ctr;
  // Write luma data
  fprintf(fp, "#pragma DATA_SECTION(%s, \".%s\")\n", arrayName, secName);
  fprintf(fp, "const uint8_t  %s[] = {\n", arrayName);

  for (ctr = 0; ctr < (size - 1); ctr++)
    fprintf(fp, "%4d,", ptr[ctr]);

  fprintf(fp, "%4d};\n\n", ptr[ctr]);
}
#endif

/**
 * @brief  This function is to find min max for quant
 *
 * @param numBatches  : no of total Batches
 * @param numGroups : total no of groups
 * @param numInChannels : no of input channles
 * @param numOutChannels :  no of output channles
 * @param inChPitch : Channel pitch of the input buffer
 * @param outChPitch : Channel pitch of the output buffer
 * @param outImPitch : line pitch of the output buffer
 * @param width : Width of the input buffer
 * @param height : Height of the input buffer
 * @param strideWidth : Stride in width dimension
 * @param strideHeight: Stride in height dimension
 * @param perChannelWeightScalePtr : pointer to scale buffer
 * @param tensorScale : scale of the input tensor
 * @param min : pointer to store minimum value
 * @param max : pointer to store maximum value
 */
template<class Tacc>
    float32_tidl TIDL_findMinMaxForChQuant(
        Tacc *accPtr,
        int32_t numBatches,     // N
        int32_t numGroups,      // G
        int32_t numInChannels,  // Ci
        int32_t numOutChannels, // Co
        int32_t outBatchPitch,
        int32_t outChPitch,
        int32_t outImPitch,
        int32_t width,  // Wi
        int32_t height, // Hi
        int32_t strideHeight,
        int32_t strideWidth,
        float32_tidl *perChannelWeightScalePtr,
        float32_tidl tensorScale,
        float32_tidl *min,
        float32_tidl *max)
{
  Tacc inDataVal;
  float32_tidl inDataFloat;
  float32_tidl absMax = 0;
  float32_tidl accScale;
  int32_t outdataOffset, maxChIdx = 0;
  int32_t i2, i3, i6, i7, i8;
  *min = FLT_MAX;
  *max = -FLT_MAX;

  OPENACC(data copyin(perChannelWeightScalePtr [0:numGroups])
              present(accPtr [0:1 + ((numGroups - 1) * numOutChannels * outChPitch + (numBatches - 1) * outBatchPitch + (((height - strideHeight + (height % strideHeight)) / strideHeight) * outImPitch) + ((width - strideWidth + (width % strideWidth)) / strideWidth))]))
  OPENACC(parallel loop collapse(2))
  for (i8 = 0; i8 < numBatches; i8++)
  {
    for (i7 = 0; i7 < numGroups; i7++) // for depthwise channels in TIDL_refConv2dKernel, numInChannels = numOutChannels = 1, main iteration on numGroups
    {
      OPENACC(loop seq)
      for (i6 = 0; i6 < numOutChannels; i6++) // numOutChannels = params->numOutChannels / params->numGroups
      {
        OPENACC(loop seq)
        for (i2 = 0; i2 < (height); i2 += strideHeight)
        {
          OPENACC(loop seq)
          for (i3 = 0; i3 < (width); i3 += strideWidth)
          {
            outdataOffset = (i7 * numOutChannels * outChPitch) + (i8 * outBatchPitch);
            accScale = perChannelWeightScalePtr[i7] * tensorScale;

            inDataVal = accPtr[outdataOffset + (i6 * outChPitch) + ((i2 / strideHeight) * outImPitch) + (i3 / strideWidth)];
            inDataFloat = inDataVal / accScale;
            *max = (inDataFloat > *max) ? inDataFloat : *max;
            *min = (inDataFloat < *min) ? inDataFloat : *min;
          }
        }
      }

      if (fabs(*max) > absMax)
      {
        absMax = fabs(*max);
        maxChIdx = i7;
      }
      if (fabs(*min) > absMax)
      {
        absMax = fabs(*min);
        maxChIdx = i7;
      }
    }
  }
  return perChannelWeightScalePtr[maxChIdx];
}

/**
 * @brief Check for border pixel
 *
 * @param spatialOffsetY : Offset value in Height dimension
 * @param spatialOffsetX : Offset value in Width dimension
 * @param validPosXMin : Minimum width value to be a valid pixel
 * @param validPosXMax : Maximum width value to be a valid pixel
 * @param validPosYMin : Minimum height value to be a valid pixel
 * @param validPosYMax : Maximum height value to be a valid pixel
 * @return isBorderPixel : Returning true for border pixels
 */
inline static int32_t TIDL_checkPixelInPadRegion_(int32_t spatialOffsetY,
                                                  int32_t spatialOffsetX,
                                                  int32_t validPosXMin,
                                                  int32_t validPosXMax,
                                                  int32_t validPosYMin,
                                                  int32_t validPosYMax)
{
  int32_t isBorderPixel = 0;

  if ((spatialOffsetY < validPosYMin) || (spatialOffsetY >= validPosYMax))
  {
    isBorderPixel = 1;
  }
  if ((spatialOffsetX < validPosXMin) || (spatialOffsetX >= validPosXMax))
  {
    isBorderPixel = 1;
  }

  return isBorderPixel;
}

/**
 * @brief  This is the reference conv2d core Kernel implementation
 *
 * @param numBatches  : no of total batches
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
template<class Tin, class Tw, class Tb, class Tacc> void TIDL_refConv2dKernel(
    const Tin *restrict pInChannel,
    const Tw *restrict pCoeffs,
    const Tb *restrict pBias,
    Tacc *restrict accPtr,
    Tacc *restrict min,
    Tacc *restrict max,
    int32_t numBatches,     // N
    int32_t numGroups,      // G
    int32_t numInChannels,  // Ci
    int32_t numOutChannels, // Co
    int32_t inChPitch,      // CiStride
    int32_t outChPitch,     // CoStide
    uint32_t inBatchPitch,  // BatchStride
    uint32_t outBatchPitch, // BatchStide
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
    int32_t enableBias,
    uint32_t isOTFpad,
    int32_t leftPad,
    int32_t topPad,
    int32_t padVal,
    int32_t startRowNumberInTensor,
    int32_t inHeight,
    int32_t inWidth)
{
#ifdef BUILD_WITH_OPENACC
  Tacc _min, _max;
#endif

  OPENACC(data present(pInChannel [0:1 + (numGroups - 1) * numInChannels * inChPitch + (numBatches - 1) * inBatchPitch + (numInChannels - 1) * inChPitch + ((height - strideHeight + (height % strideHeight)) * inImPitch) + (width - strideWidth + (width % strideWidth)) + ((coeffsHeight - 1) * inImPitch * dilationHeight) + (coeffsWidth - 1) * dilationWidth],
                       pCoeffs [0:1 + (numGroups - 1) * numInChannels * coeffsHeight * coeffsWidth * numOutChannels + (numOutChannels - 1) * numInChannels * coeffsHeight * coeffsWidth + (numInChannels - 1) * coeffsHeight * coeffsWidth + ((coeffsHeight - 1) * coeffsWidth) + (coeffsWidth - 1)],
                       pBias [0:1 + ((numGroups - 1) * numOutChannels) + (numOutChannels - 1)],
                       accPtr [0:1 + (numGroups - 1) * numOutChannels * outChPitch + (numBatches - 1) * outBatchPitch + (numOutChannels - 1) * outChPitch + (((height - strideHeight + (height % strideHeight)) / strideHeight) * outImPitch) + ((width - strideWidth + (width % strideWidth)) / strideWidth)]))
  OPENACC(parallel firstprivate(_min, _max))
  {

    if (isOTFpad == 1)
    {
      pInChannel = pInChannel - ((topPad * inImPitch) + leftPad);
    }

    int32_t i0, i2, i3, i4, i5, i6, i7, i8;
    int32_t coeffOffset, indataOffset, outdataOffset;
    Tacc outAcc = 0;
    Tin inData;
    Tw coefData;
    Tb biasData;
    uint32_t isBorderPixel;
    int32_t validPosXMin, validPosXMax, validPosYMin, validPosYMax;
    int32_t spatialOffsetY, spatialOffsetX;
    // Setting the parameters marking for pixels on borders
    validPosXMin = leftPad;
    validPosXMax = leftPad + inWidth;
    validPosYMin = topPad;
    validPosYMax = topPad + inHeight;

    OPENACC(loop collapse(5) reduction(min : _min) reduction(max : _max))
    for (i8 = 0; i8 < numBatches; i8++)
    {
      for (i7 = 0; i7 < numGroups; i7++)
      {
        for (i6 = 0; i6 < numOutChannels; i6++)
        {
          for (i2 = 0; i2 < (height); i2 += strideHeight)
          {
            for (i3 = 0; i3 < (width); i3 += strideWidth)
            {
              if (enableBias != 0)
              {
                biasData = pBias[(i7 * numOutChannels) + i6];
              }
              else
              {
                biasData = 0;
              }

              OPENACC(loop seq)
              {
                indataOffset = (i7 * numInChannels * inChPitch) + (i8 * inBatchPitch);
                outAcc = biasData;
                for (i0 = 0; i0 < numInChannels; i0++)
                {
                  coeffOffset = ((i7 * numInChannels) * coeffsHeight * coeffsWidth * numOutChannels) +
                                (i6 * numInChannels * coeffsHeight * coeffsWidth) + (i0 * coeffsHeight * coeffsWidth);
                  OPENACC(loop seq)
                  for (i4 = 0; i4 < coeffsHeight; i4++)
                  {
                    OPENACC(loop seq)
                    for (i5 = 0; i5 < coeffsWidth; i5++)
                    {
                      if (isOTFpad != 0)
                      {
                        spatialOffsetY = i2 + (i4 * dilationHeight);
                        spatialOffsetX = i3 + (i5 * dilationWidth);
                        isBorderPixel = TIDL_checkPixelInPadRegion_(spatialOffsetY + startRowNumberInTensor, spatialOffsetX, validPosXMin, validPosXMax, validPosYMin, validPosYMax);
                      }
                      if (isOTFpad && isBorderPixel)
                      {
                        inData = padVal;
                      }
                      else
                      {
                        inData = pInChannel[indataOffset + (i0 * inChPitch) + (i2 * inImPitch) + i3 +
                                            (i4 * inImPitch * dilationHeight) + (i5 * dilationWidth)];
                      }
                      coefData = pCoeffs[coeffOffset + (i4 * coeffsWidth) + i5];
                      outAcc += (inData * coefData);
                    }
                  }
                }
#ifdef BUILD_WITH_OPENACC
                _min = outAcc;
                _max = outAcc;
#else
                *min = (outAcc < *min) ? outAcc : *min;
                *max = (outAcc > *max) ? outAcc : *max;
#endif

                outdataOffset = (i7 * numOutChannels * outChPitch) + (i8 * outBatchPitch);
                accPtr[outdataOffset + (i6 * outChPitch) + ((i2 / strideHeight) * outImPitch) + (i3 / strideWidth)] = outAcc;
              }
            } // Seq loop end
          }
        }
      }
    }
  } // OPENACC end
#ifdef BUILD_WITH_OPENACC
  *max = _max > *max ? _max : *max;
  *min = _min < *min ? _min : *min;
#endif
}

/**
 * @brief  This is the reference conv2d core Kernel implementation
 *
 * @param numBatches  : no of total Batches
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
template<int Ksize, class Tin, class Tw, class Tb, class Tacc> void TIDL_refConv2dKernelFast(
    Tin *pInChannel,
    Tw *pCoeffs,
    Tb *pBias,
    Tacc *accPtr,
    Tacc *min,
    Tacc *max,
    int32_t numBatches,     // N
    int32_t numGroups,      // G
    int32_t numInChannels,  // Ci
    int32_t numOutChannels, // Co
    int32_t inChPitch,      // CiStride
    int32_t outChPitch,     // CoStide
    int32_t inBatchPitch,   // BatchStride
    int32_t outBatchPitch,  // BatchStide
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
    int32_t enableBias,
    uint32_t isOTFpad,
    int32_t leftPad,
    int32_t topPad,
    int32_t padVal,
    int32_t startRowNumberInTensor,
    int32_t inHeight,
    int32_t inWidth)
{
#ifdef BUILD_WITH_OPENACC
  Tacc _min, _max;
#endif

  OPENACC(data present(pInChannel [0:1 + (numGroups - 1) * numInChannels * inChPitch + (numBatches - 1) * inBatchPitch + (numInChannels - 1) * inChPitch + ((height - strideHeight + (height % strideHeight)) * inImPitch) + (width - strideWidth + (width % strideWidth)) + (Ksize - 1) * inImPitch * dilationHeight + (Ksize - 1) * dilationWidth],
                       pCoeffs [0:1 + ((numGroups - 1) * numInChannels * coeffsHeight * coeffsWidth * numOutChannels + (numOutChannels - 1) * numInChannels * coeffsHeight * coeffsWidth) + (numInChannels - 1) * (coeffsHeight * coeffsWidth) + ((Ksize - 1) * coeffsWidth) + (Ksize - 1)],
                       pBias [0:1 + ((numGroups - 1) * numOutChannels) + (numOutChannels - 1)],
                       accPtr [0:1 + (numGroups - 1) * numOutChannels * outChPitch + (numBatches - 1) * outBatchPitch + (numOutChannels - 1) * outChPitch + ((height - strideHeight + (height % strideHeight)) * outImPitch) + ((width - strideWidth + (width % strideWidth)) / strideWidth)]))

  OPENACC(parallel firstprivate(_min, _max))
  {

    if (isOTFpad == 1)
    {
      pInChannel = pInChannel - ((topPad * inImPitch) + leftPad);
    }

    int32_t i0, i2, i3, i4, i5, i6, i7, i8;
    int32_t coeffOffset, indataOffset, outdataOffset;
    Tacc outAcc = 0;
    Tin inData;
    Tw coefData;
    Tb biasData;
    uint32_t isBorderPixel;
    int32_t validPosXMin, validPosXMax, validPosYMin, validPosYMax;
    int32_t spatialOffsetY, spatialOffsetX;
    // Setting the parameters marking for pixels on borders
    validPosXMin = leftPad;
    validPosXMax = leftPad + inWidth;
    validPosYMin = topPad;
    validPosYMax = topPad + inHeight;

    OPENACC(loop collapse(5) reduction(min : _min) reduction(max : _max))
    for (i8 = 0; i8 < numBatches; i8++)
    {
      for (i7 = 0; i7 < numGroups; i7++)
      {
        for (i6 = 0; i6 < numOutChannels; i6++)
        {
          for (i2 = 0; i2 < (height); i2 += strideHeight)
          {
            for (i3 = 0; i3 < (width); i3 += strideWidth)
            {

              if (enableBias != 0)
              {
                biasData = pBias[(i7 * numOutChannels) + i6];
              }
              else
              {
                biasData = 0;
              }

              OPENACC(loop seq)
              {
                indataOffset = (i7 * numInChannels * inChPitch) + (i8 * inBatchPitch);
                outAcc = biasData;
                for (i0 = 0; i0 < numInChannels; i0++)
                {
                  coeffOffset = (i7 * numInChannels * coeffsHeight * coeffsWidth * numOutChannels) +
                                (i6 * numInChannels * coeffsHeight * coeffsWidth) + (i0 * coeffsHeight * coeffsWidth);
                  OPENACC(loop seq)
                  for (i4 = 0; i4 < Ksize; i4++)
                  {
                    OPENACC(loop seq)
                    for (i5 = 0; i5 < Ksize; i5++)
                    {
                      if (isOTFpad != 0)
                      {
                        spatialOffsetY = i2 + (i4 * dilationHeight);
                        spatialOffsetX = i3 + (i5 * dilationWidth);
                        isBorderPixel = TIDL_checkPixelInPadRegion_(spatialOffsetY + startRowNumberInTensor, spatialOffsetX, validPosXMin, validPosXMax, validPosYMin, validPosYMax);
                      }
                      if (isOTFpad && isBorderPixel != 0U)
                      {
                        inData = padVal;
                      }
                      else
                      {
                        inData = pInChannel[indataOffset + (i0 * inChPitch) + ((i2)*inImPitch) + i3 + (i4 * inImPitch * dilationHeight) + (i5 * dilationWidth)];
                      }
                      coefData = pCoeffs[coeffOffset + (i4 * coeffsWidth) + i5];
                      outAcc += (inData * coefData);
                    }
                  }
                }
#ifdef BUILD_WITH_OPENACC
                _min = outAcc;
                _max = outAcc;
#else
                *min = (outAcc < *min) ? outAcc : *min;
                *max = (outAcc > *max) ? outAcc : *max;
#endif

                outdataOffset = (i7 * numOutChannels * outChPitch) + (i8 * outBatchPitch);
                accPtr[outdataOffset + (i6 * outChPitch) + ((i2 / strideHeight) * outImPitch) + (i3 / strideWidth)] = outAcc;
              }
            } // Seq loop end
          }
        }
      }
    }
  } // OPENACC end
#ifdef BUILD_WITH_OPENACC
  *max = _max > *max ? _max : *max;
  *min = _min < *min ? _min : *min;
#endif
}

#ifdef BUILD_WITH_CUDA
int CUDNNLC;
#endif
template<class Tin, class Tw>
    int32_t isAVXSupported(uint32_t flowCtrl, int32_t strideWidth, int32_t strideHeight, int32_t dilationWidth, int32_t dilationHeight, int32_t otf, int32_t inImPitch, int32_t outImPitch)
{
  int32_t avx = 1;
  if ((flowCtrl & TIDL_FLOW_CTRL_AVX_REF) != TIDL_FLOW_CTRL_AVX_REF)
  {
    avx = 0;
  }
  if ((strideWidth != 1) || (strideHeight != 1))
  {
    avx = 0;
  }
  if ((dilationWidth != 1) || (dilationWidth != 1))
  {
    avx = 0;
  }
  if ((typeid(Tw) == typeid(float32_tidl)) && ((flowCtrl & TIDL_FLOW_CTRL_AVX_FLOAT) != TIDL_FLOW_CTRL_AVX_FLOAT))
  {
    avx = 0;
  }
  if ((sizeof(Tw) == 2U) || (sizeof(Tin) == 2U))
  {
    avx = 0;
  }
  if (inImPitch != outImPitch)
  {
    avx = 0;
  }
  if ((otf == 1) && ((flowCtrl & TIDL_FLOW_CTRL_REF_STAT) != TIDL_FLOW_CTRL_REF_STAT))
  {
    avx = 0;
  }
  return avx;
}

/**
 * @brief  This funciton does the quant and saturation of the accumulator data
 *
 * @param pInChannel : Input pointer on which conv is applied
 * @param pCoeffs : Pointer to weights buffer
 * @param pBias  : Pointer to the Bias values
 * @param pOutChanne : Pointer to output after Conv opreation
 * @param accPtr : Pointer to store intermidiate accumulator values
 * @param intAlgHandle : tidl algorothm handle
 * @param layerIdx :index of the current layer
 * @param params : copy of conv parameters
 * @param buffParams : copy of conv buffer parameters
 * @param satLow : min value for the saturation
 * @param satHigh : max value for the saturation
 */
template<class Tin, class Tw, class Tb, class Tout, class Tacc>
    int32_t TIDL_refConv2d(
        Tin *pInChannel,
        Tw *pCoeffs,
        Tb *pBias,
        Tout *pOutChanne,
        Tacc *accPtr,
        const TIDL_CreateParams *createParams,
        int32_t layerIdx,
        sTIDL_ConvParams_t *params,
        tidlConv2dBuffParams_t *buffParams,
        sTIDL_AlgLayer_t *algLayer)
{
  int32_t status = IALG_EOK;
  sTIDL_Network_t *net = createParams->net;
  int32_t quantizationStyle = createParams->net->quantizationStyle;
  sTIDL_Layer_t *tidlLayer = &net->TIDLLayers[layerIdx];
  uint16_t numInChannels = params->numInChannels / params->numGroups;
  uint16_t numOutChannels = params->numOutChannels / params->numGroups;
  uint16_t coeffsWidth = params->kernelW;
  uint16_t coeffsHeight = params->kernelH;
  uint16_t strideWidth = params->strideW;
  uint16_t strideHeight = params->strideH;
  uint16_t dilationWidth = params->dilationW;
  uint16_t dilationHeight = params->dilationH;
  uint16_t p_w = (params->padL < params->padR) ? params->padR : params->padL;
  uint16_t p_h = (params->padT < params->padB) ? params->padB : params->padT;
  uint16_t width = ((((uint32_t)buffParams->inWidth + (p_w * 2U) - (((coeffsWidth - 1U) * dilationWidth) + 1U)) / strideWidth) + 1U) * strideWidth;
  uint16_t height = ((((uint32_t)buffParams->inHeight + (p_h * 2U) - (((coeffsHeight - 1U) * dilationHeight) + 1U)) / strideHeight) + 1U) * strideHeight;
  uint16_t inImPitch = buffParams->inPitch;
  uint32_t inChPitch = buffParams->inChPitch;
  uint32_t inBatchPitch = buffParams->inBatchPitch;
  uint16_t outImPitch = buffParams->outPitch;
  uint32_t outChPitch = buffParams->outChPitch;
  uint32_t outBatchPitch = buffParams->outBatchPitch;
  uint8_t outRoundBits;
  int32_t numBatches = buffParams->numTotRoi;
  uint16_t numGroups = params->numGroups;
  float32_tidl *perChannelWeightScalePtr = (float32_tidl *)((int8_t *)(net) + params->perChannelWeightScaleOffset) + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
  sTIDL_DataParams_t *indata = TIDL_getDataParams(net, net->TIDLLayers[layerIdx].inData[0]);
  float32_tidl tensorScale = indata->tensorScale;

  int32_t i2, i3, i6, i8;
  Tacc outAcc;
  int32_t outdataOffset;
  Tacc min, max;
  int64_t temp;
  int32_t accMemSize = ((int32_t)outBatchPitch) * numBatches * ((int32_t)sizeof(Tacc));

  int32_t padVal = 0;
  int32_t leftPad = 0, topPad = 0;
  if (TIDL_isPadOTF(net->deviceName) != 0U)
  {
    leftPad = algLayer->layerParams.convParams.virtualPad.padL;
    topPad = algLayer->layerParams.convParams.virtualPad.padT;
    width = buffParams->inWidth + (algLayer->layerParams.convParams.virtualPad.padL + algLayer->layerParams.convParams.virtualPad.padR) - ((((((int16_t)coeffsWidth - 1) * (int16_t)dilationWidth) + 1) / 2) * 2);
    height = buffParams->inHeight + (algLayer->layerParams.convParams.virtualPad.padT + algLayer->layerParams.convParams.virtualPad.padB) - ((((((int16_t)coeffsHeight - 1) * (int16_t)dilationHeight) + 1) / 2) * 2);
  }

  /*Deprecate TFL*/
  if (TIDL_QuantStyleAsymNP2_TFL == quantizationStyle)
  {
    params->enableBias = 1; // Always need bias for asymmetric quant
    padVal = indata->tensorZeroPoint;
  }

  if (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE)
  {
    padVal = indata->tensorZeroPoint;
  }

  if (((buffParams->quantizationStyle == TIDL_QuantStyleNP2Fixed) ||
       (buffParams->quantizationStyle == TIDL_QuantStyleP2Dynamic) ||
       (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)FALSE)) &&
      (buffParams->quantizationStyle != TIDL_QuantStyleAsymNP2_TFL))
  {
    outRoundBits = buffParams->secondRoundBits;
  }
  min = (Tacc)((((uint64_t)1) << ((sizeof(Tacc) * 8LU) - 1LU)) - 1LU);
  temp = (int64_t)(((uint64_t)1) << ((sizeof(Tacc) * 8LU) - 1LU));
  max = (Tacc)temp * -1;
  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_FORCE_SCRATCH_SIZE)
  {
    buffParams->scratchSize = 0;
  }
  if (buffParams->scratchSize >= accMemSize)
  {
    accPtr = (Tacc *)buffParams->scratchMem;
    accPtr += (indata->padH * buffParams->outPitch) + indata->padW;
  }
  else
  {
    tidl_printf(0, "Memory for  TIDL_refConv2d accumulator is not sufficient exiting...\n    ");
    status = IALG_EFAIL;
  }
  if (status == IALG_EOK)
  {
#if 0
   //:TODO: Not very sure what was the change but if some change was done here then we need to do that in GC
  /*Modulated by pad values for OTF*/
  if (net->TIDLLayers[layerIdx].strideOffsetMethod != TIDL_StrideOffsetTopLeft && (!TIDL_isPadOTF( net->deviceName)))
  {
    /* Line 94 : /tensorflow/core/kernels/quantized_conv_ops.cc*/
    int32_t effectiveKw = ((coeffsWidth - 1)*dilationWidth) + 1;
    int32_t effectiveKh = ((coeffsHeight - 1)*dilationHeight) + 1;
    int32_t filter_left_offset =
      ((net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_WIDTH] - 1) * strideWidth + effectiveKw - buffParams->inWidth) / 2;
    int32_t filter_top_offset =
      ((net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_HEIGHT] - 1) * strideHeight + effectiveKh - buffParams->inHeight) / 2;

    pInChannel = pInChannel + ((effectiveKh / 2)*inImPitch + (effectiveKw / 2));

    pInChannel = pInChannel - (filter_top_offset*inImPitch + filter_left_offset);
  }
#endif

    OPENACC(data copyin(pInChannel [0:1 + (numGroups - 1) * numInChannels * inChPitch + (numBatches - 1) * inBatchPitch + (numInChannels - 1) * inChPitch + ((height - strideHeight + (height % strideHeight)) * inImPitch) + (width - strideWidth + (width % strideWidth)) + ((coeffsHeight - 1) * inImPitch * dilationHeight) + (coeffsWidth - 1) * dilationWidth],
                        pBias [0:1 + ((numGroups - 1) * numOutChannels) + (numOutChannels - 1)])
                present(pCoeffs [0:1 + (numGroups - 1) * numInChannels * coeffsHeight * coeffsWidth * numOutChannels + (numOutChannels - 1) * numInChannels * coeffsHeight * coeffsWidth + (numInChannels - 1) * coeffsHeight * coeffsWidth + ((coeffsHeight - 1) * coeffsWidth) + (coeffsWidth - 1)],
                        accPtr[:outBatchPitch * numBatches]))
    {

#ifdef BUILD_WITH_CUDA
      CUDNNLC = layerIdx;
      /*Temporarily block CUDA for Calibration..*/
      // if((createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) != TIDL_FLOW_CTRL_REF_STAT)
      if (TRUE)
      {
        TIDL_cudaConvolve2d(pInChannel, pCoeffs, pBias, accPtr, &min, &max, numBatches, numGroups, numInChannels,
                            numOutChannels, inChPitch, outChPitch, width, height, inImPitch, outImPitch,
                            coeffsWidth, coeffsHeight, dilationWidth, dilationHeight, strideWidth, strideHeight, params->enableBias, TIDL_isPadOTF(net->deviceName), leftPad, topPad, padVal, algLayer->layerParams.convParams.startRowNumberInTensor, buffParams->inHeight, buffParams->inWidth, buffParams->outHeight, buffParams->outWidth);
      }
      else
#endif /*BUILD_WITH_CUDA*/
#if defined(_HOST_BUILD) && !defined(_OPENACC)
          if ((isAVXSupported<Tin, Tw>(createParams->flowCtrl, strideWidth, strideHeight, dilationWidth, dilationHeight, TIDL_isPadOTF(net->deviceName), inImPitch, outImPitch) && (numBatches == 1U)))
      {
        TIDL_refConv2dKernelAvxProc(pInChannel, pCoeffs, pBias, accPtr, &min, &max, numGroups, numInChannels,
                                    numOutChannels, inChPitch, outChPitch, width, height, inImPitch, coeffsWidth, coeffsHeight, params->enableBias);
      }
      else
#endif
          if ((coeffsWidth == 1U) && (coeffsHeight == 1U) && (numBatches == 1U))
      {
        TIDL_refConv2dKernelFast<1, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, &min, &max, numBatches, numGroups, numInChannels,
                                                       numOutChannels, inChPitch, outChPitch, inBatchPitch, outBatchPitch, width, height, inImPitch, outImPitch,
                                                       coeffsWidth, coeffsHeight, dilationWidth, dilationHeight, strideWidth, strideHeight, params->enableBias, TIDL_isPadOTF(net->deviceName), leftPad, topPad, padVal, algLayer->layerParams.convParams.startRowNumberInTensor, buffParams->inHeight, buffParams->inWidth);
      }
      else if ((coeffsWidth == 3U) && (coeffsHeight == 3U) && (numBatches == 1U))
      {
        TIDL_refConv2dKernelFast<3, Tin, Tw, Tb, Tacc>(pInChannel, pCoeffs, pBias, accPtr, &min, &max, numBatches, numGroups, numInChannels,
                                                       numOutChannels, inChPitch, outChPitch, inBatchPitch, outBatchPitch, width, height, inImPitch, outImPitch,
                                                       coeffsWidth, coeffsHeight, dilationWidth, dilationHeight, strideWidth, strideHeight, params->enableBias, TIDL_isPadOTF(net->deviceName), leftPad, topPad, padVal, algLayer->layerParams.convParams.startRowNumberInTensor, buffParams->inHeight, buffParams->inWidth);
      }
      else
      {
        TIDL_refConv2dKernel(pInChannel, pCoeffs, pBias, accPtr, &min, &max, numBatches, numGroups, numInChannels,
                             numOutChannels, inChPitch, outChPitch, inBatchPitch, outBatchPitch, width, height, inImPitch, outImPitch,
                             coeffsWidth, coeffsHeight, dilationWidth, dilationHeight, strideWidth, strideHeight, params->enableBias, TIDL_isPadOTF(net->deviceName), leftPad, topPad, padVal, algLayer->layerParams.convParams.startRowNumberInTensor, buffParams->inHeight, buffParams->inWidth);
      }

      if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
      {
        if (isPerChannelQuantizationEnabled(params, net->calibrationOption) == 1)
        {
          float32_tidl accMinFloat = FLT_MAX;
          float32_tidl accMaxFloat = -FLT_MAX;
          /* Find accMin and accMax in float and the weight scale of the channel with absolute max value */
          float32_tidl accMaxWeightScale = TIDL_findMinMaxForChQuant(accPtr, numBatches, numGroups, numInChannels, numOutChannels, outBatchPitch, outChPitch, outImPitch, width, height,
                                                                     strideHeight, strideWidth, (float32_tidl *)perChannelWeightScalePtr, tensorScale, &accMinFloat, &accMaxFloat);
          /* convert accMinFloat and accMaxFloat to the scale of channel with the absolute max value */
          min = (Tacc)(accMinFloat * (accMaxWeightScale * tensorScale));
          max = (Tacc)(accMaxFloat * (accMaxWeightScale * tensorScale));
          net->TIDLLayers[layerIdx].layerParams.convParams.weightScale = accMaxWeightScale;
        }
      }

      outRoundBits = net->TIDLLayers[layerIdx].outData.roundBits;
      if (typeid(Tin) == typeid(float32_tidl))
      {
        outRoundBits = 0;
      }

      uint8_t *roundBitsPtr = (uint8_t *)(&outRoundBits);
      int32_t enablePerChannelShift = 0;
      if (isPerChannelQuantizationEnabled(params, net->calibrationOption) == 1)
      {
        int32_t chIdx;
        float32_tidl perChannelAccScale;
        float32_tidl tempRoundBits = 0.0f;
        roundBitsPtr = (uint8_t *)(buffParams->outRoundBitsPtr);
        /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          <justification end> */
        if (indata->tensorScale != 0)
        {
          /* LDRA_JUSTIFY_END */
          for (chIdx = 0; chIdx < params->numInChannels; chIdx++)
          {
            perChannelAccScale = perChannelWeightScalePtr[chIdx] * indata->tensorScale;

            tempRoundBits = (log((float32_tidl)perChannelAccScale /
                                 net->TIDLLayers[layerIdx].outData.tensorScale) /
                             log(2.0));
            roundBitsPtr[chIdx] = (int8_t)tempRoundBits;
          }
        }
        enablePerChannelShift = 1;
      }

      int32_t satLow;
      int32_t satHigh;
      int32_t mixedPrecision = 0;

      TIDL_getSaturationLimits(buffParams->outElementType, &satLow, &satHigh);

      /* If size of weights is not same as size of output then it indicates
      mixed precision */
      if (sizeof(*pCoeffs) != sizeof(*pOutChanne))
      {
        mixedPrecision = 1;
        /* This is to match with target implementation wherin the saturations
        happens after applying outRoundBits - 8 shift and then the right shift
        of 8 is applied */
        outRoundBits = outRoundBits - 8U;
        if (buffParams->outElementType == TIDL_SignedChar)
        {
          TIDL_getSaturationLimits(TIDL_SignedShort, &satLow, &satHigh);
        }
        else
        {
          TIDL_getSaturationLimits(TIDL_UnsignedShort, &satLow, &satHigh);
        }
      }

#ifdef BUILD_WITH_CUDA
/*Disable cuda kernel*/
#if 0
if((createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) != TIDL_FLOW_CTRL_REF_STAT)
{
  int32_t sizeOstream = numBatches*outBatchPitch;
  Tout *devPtrOf;
  int32_t precisionAdjustmentShift = 0;
  if (mixedPrecision == 1)
  {
    precisionAdjustmentShift = 8;
  }

  if(typeid(Tin) == typeid(float32_tidl))
  {
    float32_tidl fmin, fmax;
    TIDL_getSaturationFloat(&net->TIDLLayers[layerIdx],&fmin,&fmax);
    TIDL_cudaSaturateV1(&devPtrOf, numBatches, params->numOutChannels, height / strideHeight, width / strideWidth, outChPitch, outImPitch, outRoundBits, fmin, fmax);//Performing Saturation for float
  }
  else
  {
    if(TIDL_isAsymQuantEnabledTFL(quantizationStyle) || (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)TRUE))
    {
      uint8_t* mmav2_Scales = (uint8_t*) algLayer->layerParams.convParams.mmaScalesPtr + algLayer->layerParams.convParams.biasRefExtraOffset;
      uint8_t* mmav2_Shifts = (uint8_t*) algLayer->layerParams.convParams.mmaShiftsPtr + algLayer->layerParams.convParams.biasRefExtraOffset;
      int32_t i0 = 0;
      if(mixedPrecision == 1)
      {
        for(i0 = 0; i0 < params->numOutChannels; i0++)
        {
          mmav2_Shifts[i0] += 8U;
        }
      }
      TIDL_cudaSaturateFixedPointAsym<Tacc,Tout,Tin>(&devPtrOf, numBatches, params->numOutChannels, buffParams->outHeight, buffParams->outWidth, outChPitch, outImPitch, buffParams->mmaPSATMin, buffParams->mmaPSATMax, mmav2_Scales, mmav2_Shifts, padVal);
    }
    else
    {
      TIDL_cudaSaturateFixedPoint<Tacc,Tout>(&devPtrOf, numBatches, params->numOutChannels, buffParams->outHeight, buffParams->outWidth, outChPitch, outImPitch, roundBitsPtr, satLow, satHigh, enablePerChannelShift, precisionAdjustmentShift);//Performing Saturation for Fixed Point
    }
  }

  TIDL_cudaOutputTx( devPtrOf, sizeOstream, numBatches, params->numOutChannels, height, width, outChPitch, outImPitch, strideHeight, strideWidth, pOutChanne); //Performing Tx of output stream
}
else
#endif
      {
        /*Mark init as completed to prevent re-allocation of buffers for subsequent frames:*/
        TIDL_cudaSetInitFlag(CUDNNLC);
#endif
        int64_t tempAcc;
        /* Extra offset to handle split workload with grouped conv. Need to review this change */
        uint8_t *mmav2_Scales = (uint8_t *)algLayer->layerParams.convParams.mmaScalesPtr + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
        uint8_t *mmav2_Shifts = (uint8_t *)algLayer->layerParams.convParams.mmaShiftsPtr + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;

        int32_t numOutChannels = params->numOutChannels;
        int32_t mmaPSATMin = buffParams->mmaPSATMin;
        int32_t mmaPSATMax = buffParams->mmaPSATMax;

        OPENACC(data present(accPtr[:1 + (numBatches - 1) * outBatchPitch + (numOutChannels - 1) * outChPitch + (((height - strideHeight + (height % strideHeight)) / strideHeight) * outImPitch) + ((width - strideWidth + (width % strideWidth)) / strideWidth)])
                    copy(pOutChanne[:1 + (numBatches - 1) * outBatchPitch + (numOutChannels - 1) * outChPitch + (((height - strideHeight + (height % strideHeight)) / strideHeight) * outImPitch) + ((width - strideWidth + (width % strideWidth)) / strideWidth)]))

        if ((TIDL_QuantStyleAsymNP2_TFL == quantizationStyle) || (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE))
        {
          sTIDL_Layer_t *pTIDLNet = &net->TIDLLayers[layerIdx];

          OPENACC(parallel loop collapse(4))
          for (i8 = 0; i8 < numBatches; i8++)
          {
            for (i6 = 0; i6 < numOutChannels; i6++)
            {
              for (i2 = 0; i2 < ((int32_t)height); i2 += (int32_t)strideHeight)
              {
                for (i3 = 0; i3 < ((int32_t)width); i3 += (int32_t)strideWidth)
                {
                  outdataOffset = i8 * outBatchPitch;
#if defined TIDL_COVERAGE_DEAD_CODE
                  /* PerChannel shift is enabled for only depth wise convolution. If depth wise convolution and perChannel calibration is enabled, HighPrecisionKernel is not enabled */
                  if (enablePerChannelShift == 1)
                  {
                    outRoundBits = roundBitsPtr[i6];
                  }
#endif
                  outAcc = accPtr[outdataOffset + (i6 * (int32_t)outChPitch) + ((i2 / (int32_t)strideHeight) * (int32_t)outImPitch) + (i3 / (int32_t)strideWidth)];
                  outAcc = outAcc;
                  if (tidlLayer->outData.elementType == TIDL_SinglePrecFloat)
                  {
                    OPENACC(routine(TIDL_floatSat))
                    outAcc = TIDL_floatSat(outAcc, pTIDLNet);
                  }
                  else
                  {
                    tempAcc = (int64_t)outAcc * (int64_t)mmav2_Scales[i6];
                    OPENACC(routine(TIDL_roundSatMMA))
                    outAcc = (Tacc)TIDL_roundSatMMA(tempAcc, mmav2_Shifts[i6], mmaPSATMin, mmaPSATMax);
                    outAcc = ((int64_t)outAcc & (int64_t)0xFFFFFFFFFFU); // Only 40 bits are valid in the accumulator..

                    /* The below condition will be activated when asymmetric support is enabled for 16bit convolution layer. */
                    if (std::is_same<Tin, int8_t>::value || std::is_same<Tin, uint8_t>::value)
                    {
                      outAcc = ((int64_t)outAcc & (int64_t)0xFFFFFFFFFFU); // Only 40 bits are valid in the accumulator..
                    }
                    else
                    {
                      outAcc = ((int64_t)outAcc);
                    }
#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
                    if (mixedPrecision == 1 && (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) == (int32_t)FALSE)) // Invalid for Asymmetric currently
                    {
                      outAcc = (uint64_t)outAcc >> 8U;
                    }
#endif
                  }
                  pOutChanne[outdataOffset + (i6 * (int32_t)outChPitch) + ((i2 / (int32_t)strideHeight) * (int32_t)outImPitch) + (i3 / (int32_t)strideWidth)] = outAcc;
                }
              }
            }
          }
        }
        else
        {
          sTIDL_Layer_t *pTIDLNet = &net->TIDLLayers[layerIdx];
          int32_t outElementType = tidlLayer->outData.elementType;

          OPENACC(parallel loop collapse(4))
          for (i8 = 0; i8 < numBatches; i8++)
          {
            for (i6 = 0; i6 < numOutChannels; i6++)
            {
              for (i2 = 0; i2 < (height); i2 += strideHeight)
              {
                for (i3 = 0; i3 < ((int32_t)width); i3 += (int32_t)strideWidth)
                {
                  outdataOffset = i8 * outBatchPitch;
                  outAcc = accPtr[outdataOffset + (i6 * outChPitch) + ((i2 / strideHeight) * outImPitch) + (i3 / strideWidth)];
                  outAcc = outAcc;
                  if (outElementType == TIDL_SinglePrecFloat)
                  {
                    OPENACC(routine(TIDL_floatSat))
                    outAcc = TIDL_floatSat(outAcc, pTIDLNet);
                  }
                  else
                  {
                    if (enablePerChannelShift == 1)
                    {
                      outRoundBits = roundBitsPtr[i6];
                    }
                    OPENACC(routine(TIDL_roundSatMMA))
                    outAcc = (Tacc)TIDL_roundSatMMA((int64_t)outAcc, outRoundBits, satLow, satHigh);

                    if (mixedPrecision == 1)
                    {
                      outAcc = (uint64_t)outAcc >> 8U;
                    }
                  }
                  pOutChanne[outdataOffset + (i6 * (int32_t)outChPitch) + ((i2 / (int32_t)strideHeight) * (int32_t)outImPitch) + (i3 / (int32_t)strideWidth)] = outAcc;
                }
              }
            }
          }
        }
#ifdef BUILD_WITH_CUDA
      }
#endif /*BUILD_WITH_CUDA*/
    }
  }
  return status;
}

/**
 * @brief  This funciton handles differnt input bit-depths for Conv reference
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
 * @return  None
 */
template<class Tw, class Tb, class Tacc>
    int32_t TIDL_refConv2dBitDepth(
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
        int32_t outDataOffset,
        sTIDL_AlgLayer_t *algLayer)
{
  int32_t status = IALG_EOK;
  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_FORCE_OUTDATATYPE)
  {
    buffParams->outElementType = 4;
  }
  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_FORCE_REF_INVALID_INDATATYPE)
  {
    buffParams->inElementType = 4;
  }
  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_FORCE_REF_INVALID_OUTDATATYPE)
  {
    buffParams->inElementType = 1;
    buffParams->outElementType = 4;
  }
  if (inElementType == TIDL_SinglePrecFloat)
  {
    status = TIDL_refConv2d(
        ((float32_tidl *)inPtr + inDataOffset),
        (float32_tidl *)wgtPtr,
        (float32_tidl *)biasPtr,
        (((float32_tidl *)refPtr) + outDataOffset),
        (float32_tidl *)accPtr,
        createParams, layerIdx, params, buffParams, algLayer);
  }
  else if (buffParams->inElementType == TIDL_SignedChar)
  {
    if (buffParams->outElementType == TIDL_SignedChar)
    {
      status = TIDL_refConv2d(
          ((int8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedChar)
    {
      status = TIDL_refConv2d(
          ((int8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_SignedShort)
    {
      status = TIDL_refConv2d(
          ((int8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedShort)
    {
      status = TIDL_refConv2d(
          ((int8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else
    {
      status = IALG_EFAIL;
    }
  }
  else if (buffParams->inElementType == TIDL_UnsignedChar)
  {
    if (buffParams->outElementType == TIDL_SignedChar)
    {
      status = TIDL_refConv2d(
          ((uint8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedChar)
    {
      status = TIDL_refConv2d(
          ((uint8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_SignedShort)
    {
      status = TIDL_refConv2d(
          ((uint8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedShort)
    {
      status = TIDL_refConv2d(
          ((uint8_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else
    {
      status = IALG_EFAIL;
    }
  }
  else if (buffParams->inElementType == TIDL_SignedShort)
  {
    if (buffParams->outElementType == TIDL_SignedChar)
    {
      status = TIDL_refConv2d(
          ((int16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedChar)
    {
      status = TIDL_refConv2d(
          ((int16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_SignedShort)
    {
      status = TIDL_refConv2d(
          ((int16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedShort)
    {
      status = TIDL_refConv2d(
          ((int16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else
    {
      status = IALG_EFAIL;
    }
  }
  else if (buffParams->inElementType == TIDL_UnsignedShort)
  {
    if (buffParams->outElementType == TIDL_SignedChar)
    {
      status = TIDL_refConv2d(
          ((uint16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedChar)
    {
      status = TIDL_refConv2d(
          ((uint16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint8_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_SignedShort)
    {
      status = TIDL_refConv2d(
          ((uint16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((int16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else if (buffParams->outElementType == TIDL_UnsignedShort)
    {
      status = TIDL_refConv2d(
          ((uint16_t *)inPtr + inDataOffset),
          wgtPtr,
          biasPtr,
          ((uint16_t *)refPtr + outDataOffset),
          accPtr,
          createParams, layerIdx, params, buffParams, algLayer);
    }
    else
    {
      status = IALG_EFAIL;
    }
  }
  /* All the supported inElementTypes are covered by the previous conditions */
  else
  {
    status = IALG_EFAIL;
  }

  return status;
}

/**
 * @brief Convolution layer reference implementation
 *
 * @param intAlgHandle : tidl algorothm handle
 * @param algLayer : Pointer to the layer specific parameters
 * @param tidlLayer : Pointer to the common layer parameters
 * @param params : copy of conv layer parameters
 * @param buffParams : copy of conv buffer parameters
 * @param inPtr : Pointer to input buffers to be processed
 * @param outPtr : Pointer to output buffers to be processed
 * @param inElementType : ElementType of the input
 * @param numBatches : total no of batches
 * @param sysMems : sysMem Handle
 * @return  IALG_EOK   - Successful
 *          IALG_EFAIL - Unspecified error
 */
int32_t TIDL_refConv2dProcess(
    const TIDL_CreateParams *createParams,
    sTIDL_AlgLayer_t *algLayer,
    sTIDL_Layer_t *tidlLayer,
    sTIDL_ConvParams_t *params,
    tidlConv2dBuffParams_t *buffParams,
    void *inPtr,
    void *outPtr)
{
  const sTIDL_Network_t *net = createParams->net;

  uint16_t numOutChs = params->numOutChannels / params->numGroups;
  uint16_t k_w = params->kernelW;
  uint16_t k_h = params->kernelH;
  uint16_t s_w = params->strideW;
  uint16_t s_h = params->strideH;
  uint16_t dilation_w = params->dilationW;
  uint16_t dilation_h = params->dilationH;
  uint16_t p_w = (params->padL < params->padR) ? params->padR : params->padL;
  uint16_t p_h = (params->padT < params->padB) ? params->padB : params->padT;
  uint16_t imWidth = ((((uint16_t)buffParams->inWidth + (p_w * 2U) - (((k_w - 1U) * dilation_w) + 1U)) / s_w) + 1U) * s_w;
  uint16_t imHeight = ((((uint16_t)buffParams->inHeight + (p_h * 2U) - (((k_h - 1U) * dilation_h) + 1U)) / s_h) + 1U) * s_h;
  if (TIDL_isPadOTF(net->deviceName) != 0U)
  {
    imWidth = buffParams->inWidth;
    imHeight = buffParams->inHeight;
    p_w = algLayer->layerParams.convParams.virtualPad.padL; //(algLayer->layerParams.convParams.virtualPad.padL <  algLayer->layerParams.convParams.virtualPad.padR) ? algLayer->layerParams.convParams.virtualPad.padR : algLayer->layerParams.convParams.virtualPad.padL;
    p_h = algLayer->layerParams.convParams.virtualPad.padT; //(algLayer->layerParams.convParams.virtualPad.padT <  algLayer->layerParams.convParams.virtualPad.padB) ? algLayer->layerParams.convParams.virtualPad.padT : algLayer->layerParams.convParams.virtualPad.padB;
  }
  uint16_t outPitchPooling;
  int16_t outRefPitch = 0;
  int32_t status = IALG_EOK;
  uint16_t avgPoolingRoundBits;
  int32_t inChPitchPooling, outChPitchPooling;
  int32_t j;
  int32_t outWidth, outHeight;
  uint8_t *refPtr;
  uint8_t *refPtrMerged;
  int32_t inDataOffset;
  int32_t outDataOffset;
  int32_t layerIdx;
  void *biasptr;
  int16_t *orgbiasptr;
  float32_tidl *orgbiasptrfloat;
  void *accPtr = NULL;
  int32_t outRefChPitch;
  int32_t padH, padW;
  int32_t upscaleFactor = 1;

  int32_t quantizationStyle = net->quantizationStyle;
  void *weightPtr = ((int8_t *)(net) + params->weights + algLayer->layerParams.convParams.coeffRefExtraOffset + algLayer->layerParams.convParams.coeffMultiCoreExtraOffset);
  void *biasPtr = ((int8_t *)(net) + params->bias);
  float32_tidl *perChannelWeightScalePtr =
      (float32_tidl *)get_int8_t_pointer((int8_t *)(net), params->perChannelWeightScaleOffset) + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;

  layerIdx = algLayer->layerIdx;

  outWidth = (s_w > 1U) ? ((imWidth + 1U) / s_w) : imWidth;
  outHeight = (s_h > 1U) ? ((imHeight + 1U) / s_h) : imHeight; // Review

  padW = net->TIDLLayers[layerIdx].outData.padW;
  padH = net->TIDLLayers[layerIdx].outData.padH;

  outPitchPooling = buffParams->outPitchPooling;
  outChPitchPooling = buffParams->outChPitch;
#if defined TIDL_COVERAGE_DEAD_CODE
  if (params->enablePooling == 1)
  {
    refPtrMerged = (uint8_t *)outPtr;
    refPtr = (uint8_t *)algLayer->scratchMem;
    outRefPitch = outWidth + padW;
    outRefChPitch = outRefPitch * ((outHeight + (2 * padH)) + 1);
    buffParams->outPitch = outRefPitch;
    buffParams->outChPitch = outRefChPitch;
  }
  else if (params->enableDepthToSpace == 1)
  {
    refPtrMerged = (uint8_t *)outPtr;
    refPtr = (uint8_t *)algLayer->scratchMem;
    upscaleFactor = params->upscaleFactor;
    outRefPitch = (outWidth) + padW;
    outRefChPitch = outRefPitch * ((outHeight + (2 * padH)) + 1);
    buffParams->outPitch = outRefPitch;
    buffParams->outChPitch = outRefChPitch;
  }
  else
#endif
  {
    outRefPitch = buffParams->outPitch;
    outRefChPitch = buffParams->outChPitch;
    refPtrMerged = (uint8_t *)outPtr;
    refPtr = (uint8_t *)outPtr;
  }

  biasPtr = (int16_t *)biasPtr + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
  orgbiasptr = (int16_t *)biasPtr;
  orgbiasptrfloat = (float32_tidl *)biasPtr;

  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_FORCE_INVALID_BIASPTR)
  {
    params->numOutChannels = 50;
  }

  if (!((TIDL_isAsymQuantEnabledTFL(quantizationStyle) != FALSE) ||
        ((TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE) && (tidlLayer->weightsElementSizeInBits < 32))))
  {
#if USE_16BIT_BIAS_FOR_8BIT_MODE
    if (tidlLayer->weightsElementSizeInBits <= 8)
    {
      if (algLayer->layerParams.convParams.biasParamSize >= (params->numOutChannels * (int32_t)sizeof(int16_t)))
      {
        biasptr = (void *)algLayer->layerParams.convParams.biasParamMem;
      }
      else
      {
        tidl_printf(0, "Memory for  biasptr  is not sufficient exiting...\n    ");
        exit(-1);
      }

      if (params->enableBias != 0)
      {
        if (isPerChannelQuantizationEnabled(params, net->calibrationOption) == 1)
        {
          const sTIDL_DataParams_t *inDataParams =
              &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
          int32_t chIdx;
          float32_tidl inScaleFactor;

          int32_t biasB;
          for (chIdx = 0; chIdx < params->numOutChannels; chIdx++)
          {
            inScaleFactor = (perChannelWeightScalePtr[chIdx] * inDataParams->tensorScale) / params->biasScale;
            TIDL_conv2dBiasSplit(orgbiasptr + chIdx,
                                 (int16_t *)biasptr + chIdx,
                                 &biasB,
                                 1,
                                 inScaleFactor,
                                 TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16,
                                 0xFF,
                                 TIDL_getDatElementSign(buffParams->inElementType));
            ((int16_t *)biasptr)[chIdx] = ((int16_t *)biasptr)[chIdx] * biasB;
          }
        }
        else
        {
          TIDL_conv2dBiasSplit(orgbiasptr, (int16_t *)biasptr, &params->biasB, params->numOutChannels, buffParams->inScaleFactor,
                               TIDL_SAT_LO_INT16, TIDL_SAT_HI_INT16, 0xFF, TIDL_getDatElementSign(buffParams->inElementType));
          for (j = 0; j < params->numOutChannels; j++)
          {
            ((int16_t *)biasptr)[j] = ((int16_t *)biasptr)[j] * params->biasB;
          }
        }
      }
      else
      {
        params->biasB = 0;
        for (j = 0; j < params->numOutChannels; j++)
        {
          ((int16_t *)biasptr)[j] = ((int16_t *)biasptr)[j] * params->biasB;
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
        exit(-1);
      }

      if (params->enableBias != 0)
      {
        if (isPerChannelQuantizationEnabled(params, net->calibrationOption) == 1)
        {
          int chIdx;
          float32_tidl inScaleFactor;
          const sTIDL_DataParams_t *inDataParams =
              &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;

          int32_t biasB;
          for (chIdx = 0; chIdx < params->numOutChannels; chIdx++)
          {
            inScaleFactor = (perChannelWeightScalePtr[chIdx] * inDataParams->tensorScale) / params->biasScale;
            TIDL_conv2dBiasSplit(orgbiasptr + chIdx,
                                 (int32_t *)biasptr + chIdx,
                                 &biasB,
                                 1,
                                 inScaleFactor,
                                 TIDL_SAT_LO_INT32, TIDL_SAT_HI_INT32,
                                 0xFFFF,
                                 TIDL_getDatElementSign(buffParams->inElementType));
            ((int32_t *)biasptr)[chIdx] = ((int32_t *)biasptr)[chIdx] * biasB;
          }
        }
        else
        {
          int32_t maxBiasB = 0xFFFF;
          TIDL_conv2dBiasSplit(orgbiasptr,
                               (int32_t *)biasptr,
                               &params->biasB,
                               params->numOutChannels,
                               buffParams->inScaleFactor,
                               TIDL_SAT_LO_INT32,
                               TIDL_SAT_HI_INT32,
                               maxBiasB,
                               TIDL_getDatElementSign(buffParams->inElementType));
          for (j = 0; j < params->numOutChannels; j++)
          {
            ((int32_t *)biasptr)[j] = ((int32_t *)biasptr)[j] * params->biasB;
          }
        }
      }
      else
      {
        params->biasB = 0;
        for (j = 0; j < params->numOutChannels; j++)
        {
          ((int32_t *)biasptr)[j] = ((int32_t *)biasptr)[j] * params->biasB;
        }
        /* When bias is not enabled for a conv layer then the
        original bias pointer in the net would not be pointing to zeros
          refernce kenrls Implemenation expects zero for bias disabled case */
        orgbiasptrfloat = (float32_tidl *)biasptr;
      }
    }
  }
  else if (quantizationStyle == TIDL_QuantStyleAsymNP2_TFL) // Deprecate
  {
    biasptr = (int32_t *)algLayer->layerParams.convParams.biasParamMem;
    biasPtr = (int32_t *)biasPtr + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
  }
  else // HP
  {
#if defined TIDL_COVERAGE_DEAD_CODE_NO_TEST
    /* This branch will be activated with 16bit asym support for convolution */
    if (tidlLayer->weightsElementSizeInBits <= 8)
    {
      biasptr = ((int32_t *)algLayer->layerParams.convParams.biasParamMem) + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
    }
    else
    {
      biasptr = ((int64_t *)algLayer->layerParams.convParams.biasParamMem) + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
    }
#else
    biasptr = ((int32_t *)algLayer->layerParams.convParams.biasParamMem) + algLayer->layerParams.convParams.biasRefExtraOffset + algLayer->layerParams.convParams.biasMultiCoreExtraOffset;
#endif
  }

  avgPoolingRoundBits = 2;

  inDataOffset = 0;
  outDataOffset = 0;

#if 0
  //:TODO: If some change is done here then it needs to be taken care in GC
  inDataOffset = (inDataParams->padH - p_h)*inPitch + (inDataParams->padW - p_w); //Virtual translation (Move to kernel)
  outDataOffset = (padH)*outRefPitch + (padW);
#endif
  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_FORCE_INVALID_DATATYPE)
  {
    tidlLayer->weightsElementSizeInBits = 64;
  }

  if (tidlLayer->weightsElementSizeInBits <= 8)
  {
    if (buffParams->inElementType < 2)
    {
      if (TIDL_QuantStyleAsymNP2_TFL == quantizationStyle)
      {
        buffParams->inElementType = 1;
        status = TIDL_refConv2dBitDepth(inPtr, refPtr, ((int8_t *)weightPtr), ((int32_t *)biasptr), ((int32_t *)accPtr), createParams, layerIdx, params, buffParams, buffParams->inElementType, inDataOffset, outDataOffset, algLayer);
      }
      else if (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE)
      {
        status = TIDL_refConv2dBitDepth(inPtr, refPtr, ((int8_t *)weightPtr), ((int32_t *)biasptr), ((int32_t *)accPtr), createParams, layerIdx, params, buffParams, buffParams->inElementType, inDataOffset, outDataOffset, algLayer);
      }
      else
      {
#if USE_16BIT_BIAS_FOR_8BIT_MODE
        status = TIDL_refConv2dBitDepth(inPtr, refPtr, ((int8_t *)weightPtr), ((int16_t *)biasptr), ((int32_t *)accPtr), createParams, layerIdx, params, buffParams, buffParams->inElementType, inDataOffset, outDataOffset, algLayer);
#else
        status = TIDL_refConv2dBitDepth(inPtr, refPtr, ((int8_t *)weightPtr), ((int32_t *)biasptr), ((int32_t *)accPtr), createParams, layerIdx, params, buffParams, buffParams->inElementType, inDataOffset, outDataOffset, algLayer);
#endif
      }
    }
    else
    {
      /* tidl_printf(0,"Element type > 1 is supported only if  weightsElementSizeInBits > 8 \n   "); */
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_CONV, TIDL_ERROR_CONV_UNSUPPORTED_DATA_TYPE);
      status = TIDL_ERR_FAILURE;
    }
  }
  else if (tidlLayer->weightsElementSizeInBits <= 16)
  {
    /* The following branches will get activated with 16bit asym enabled for convolution layers */
    if (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE)
    {
      status = TIDL_refConv2dBitDepth(inPtr, refPtr, ((int16_t *)weightPtr), ((int64_t *)biasptr), ((int64_t *)accPtr), createParams, layerIdx, params, buffParams, buffParams->inElementType, inDataOffset, outDataOffset, algLayer);
    }
    else
    {
      status = TIDL_refConv2dBitDepth(inPtr, refPtr, ((int16_t *)weightPtr), ((int32_t *)biasptr), ((int64_t *)accPtr), createParams, layerIdx, params, buffParams, buffParams->inElementType, inDataOffset, outDataOffset, algLayer);
    }
  }
  else if (tidlLayer->weightsElementSizeInBits == 32)
  {
    status = TIDL_refConv2dBitDepth(inPtr, refPtr, (int8_t *)weightPtr, (int32_t *)orgbiasptrfloat, (int32_t *)accPtr, createParams, layerIdx, params, buffParams, buffParams->inElementType, inDataOffset, outDataOffset, algLayer);
  }
  else
  {
    status = IALG_EFAIL;
  }

  if (status == IALG_EOK)
  {
#if defined TIDL_COVERAGE_DEAD_CODE
    if (((uint32_t)params->enablePooling) == TRUE)
    {
      const sTIDL_DataParams_t *inDataParams = &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
      tidlConv2dPoolingParams_t poolParams;

      inChPitchPooling = outRefChPitch;

      buffParams->outPitch = outPitchPooling;
      buffParams->outChPitch = outChPitchPooling;

      poolParams.width = outWidth;
      poolParams.height = outHeight;
      poolParams.inPitch = outRefPitch;
      poolParams.outPitch = outPitchPooling;
      poolParams.numOutChannels = (int32_t)numOutChs * params->numGroups;
      poolParams.roundBits = avgPoolingRoundBits;
      poolParams.inChPitch = inChPitchPooling;
      poolParams.outChPitch = outChPitchPooling;

      for (j = 0; j < inDataParams->dimValues[TIDL_DIM_BATCH]; j++)
      {

        inDataOffset = j * params->numGroups * (int32_t)numOutChs * outRefPitch * inChPitchPooling;
        outDataOffset = j * params->numGroups * (int32_t)numOutChs * (int32_t)outPitchPooling * outChPitchPooling;

        if ((buffParams->outElementType == TIDL_UnsignedChar) ||
            (buffParams->outElementType == TIDL_UnsignedShort))
        {
          if (params->poolParams.poolingType == TIDL_MaxPooling)
          {
            TIDL_refMaxPooling_2x2_skip2_cn(
                (refPtr + inDataOffset),
                (refPtrMerged + outDataOffset),
                &poolParams);
          }
          else if (params->poolParams.poolingType == TIDL_AveragePooling)
          {
            TIDL_refAvgPooling_2x2_skip2_cn(
                (refPtr + inDataOffset),
                (refPtrMerged + outDataOffset),
                &poolParams,
                (uint8_t)0,
                (uint8_t)255);
          }
          else
          {
            /*Do Nothing*/
          }
        }
        else
        {
          if (params->poolParams.poolingType == TIDL_MaxPooling)
          {
            TIDL_refMaxPooling_2x2_skip2_cn(
                (((int8_t *)refPtr) + inDataOffset),
                (((int8_t *)refPtrMerged) + outDataOffset),
                &poolParams);
          }
          else if (params->poolParams.poolingType == TIDL_AveragePooling)
          {
            TIDL_refAvgPooling_2x2_skip2_cn(
                (((int8_t *)refPtr) + inDataOffset),
                (((int8_t *)refPtrMerged) + outDataOffset),
                &poolParams,
                (int8_t)-128,
                (int8_t)127);
          }
          else
          {
            /*Do Nothing*/
          }
        }
      }

      outWidth = outWidth / 2;
      outHeight = outHeight / 2;
    }
    else if (((uint32_t)params->enableDepthToSpace) == TRUE)
    {
      inDataOffset = ((padH)*outRefPitch) + (padW);
      outDataOffset = ((padH)*net->TIDLLayers[layerIdx].outData.pitch[TIDL_LINE_PITCH]) + (padW);

      TIDL_depthToSpaceRefProcess((void *)refPtr, (void *)refPtrMerged, inDataOffset, outDataOffset,
                                  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_WIDTH],
                                  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_HEIGHT],
                                  params->numOutChannels,
                                  net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_BATCH],
                                  outRefPitch,
                                  net->TIDLLayers[layerIdx].outData.pitch[TIDL_LINE_PITCH],
                                  outRefChPitch,
                                  net->TIDLLayers[layerIdx].outData.pitch[TIDL_CHANNEL_PITCH],
                                  outRefChPitch * params->numOutChannels,
                                  net->TIDLLayers[layerIdx].outData.pitch[TIDL_CHANNEL_PITCH] * net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_NUMCH],
                                  upscaleFactor,
                                  net->TIDLLayers[layerIdx].outData.elementType);
    }
    else
    {
      /*Do Nothing*/
    }
#endif
    TIDL_L1DandL2CacheWbInv();
  }
  return status;
}

static int32_t TIDL_conv2dRefGetOutDataSize(const TIDL_NetworkCommonParams *commonParams,
                                            const TIDL_LayerSpecificParams *layerSpecificParams,
                                            int32_t layerIdx)
{
  int32_t outDataSize;
  outDataSize = TIDL_updateDataParamsPitch(&commonParams->net->TIDLLayers[layerIdx].outData);

  if (commonParams->createParams->optimiseExtMem != TIDL_OptimiseExtMemL0)
  {
    outDataSize = TIDL_findFreeOutBuff(commonParams->createParams, layerIdx,
                                       outDataSize, commonParams->TIDLLayersBufPtr);
    commonParams->TIDLLayersBufPtr->outDataSize[layerIdx] = outDataSize;
  }

  /* If NC information is available then no output data required to be allocated */
  if (layerSpecificParams->workloadUnit != NULL) //: TODO: Once we move to workload this should be based on workload
  {
    outDataSize = 0;
  }

  return outDataSize;
}

static int32_t TIDL_conv2dRefGetScratchDataSize(const TIDL_NetworkCommonParams *commonParams,
                                                int32_t layerIdx)
{
  int32_t outBatchPitch;
  int32_t scratchDataSize;
  sTIDL_DataParams_t *dataParams = TIDL_getDataParams(commonParams->net,
                                                      commonParams->net->TIDLLayers[layerIdx].inData[0]);

  outBatchPitch = commonParams->net->TIDLLayers[layerIdx].outData.pitch[TIDL_ROI_PITCH];
  scratchDataSize = commonParams->net->TIDLLayers[layerIdx].outData.dimValues[TIDL_DIM_BATCH] * outBatchPitch *
                    TIDL_getDatElementSize(dataParams->elementType);

  scratchDataSize = (int32_t)((float32_tidl)scratchDataSize * 1.5f);

#ifdef TIDL_COVERAGE_DEAD_CODE
  if (commonParams->net->TIDLLayers[layerIdx].layerType == TIDL_ConvolutionLayer)
  {
    scratchDataSize = (int32_t)((float32_tidl)scratchDataSize * 1.5f);
  }
  else
  {
    scratchDataSize = (int32_t)((float32_tidl)scratchDataSize * 2.0f);
  }
#endif

  if (TIDL_getDatElementSize(commonParams->net->TIDLLayers[layerIdx].outData.elementType) <= 2)
  {
    scratchDataSize *= ((commonParams->net->TIDLLayers[layerIdx].weightsElementSizeInBits + 7) / 8) * ((int32_t)(sizeof(int32_t)));
  }

  return scratchDataSize;
}

void TIDL_conv2dSetupBuffParams(const sTIDL_ConvParams_t *conv2dparams,
                                const sTIDL_Network_t *net,
                                sTIDL_AlgLayer_t *algLayer,
                                const sTIDL_Layer_t *tidlLayer,
                                int32_t layerIdx,
                                tidlConv2dBuffParams_t *buffParams)
{
  const sTIDL_DataParams_t *inDataParams;
  const sTIDL_DataParams_t *outDataParams;
  int32_t outRoundBits = 0;

  /* Read inDataParams by finding which layer produces it i.e. which layers output is input to this layer*/
  inDataParams = &net->TIDLLayers[algLayer->inLayerIdx[0]].outData;
  outDataParams = &tidlLayer->outData;

  buffParams->inElementType = inDataParams->elementType;
  buffParams->outElementType = outDataParams->elementType;
  buffParams->inWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];
  buffParams->inHeight = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  buffParams->outWidth = outDataParams->dimValues[TIDL_DIM_WIDTH];
  buffParams->outHeight = outDataParams->dimValues[TIDL_DIM_HEIGHT];
  //: TODO: Not sure why it was computed instead of using from dimValues
#if 0
  buffParams->outWidth = (((buffParams->inWidth + (params->padW * 2U)) - (((params->kernelW - 1U)* params->dilationW) + 1U)) / strideW) + 1U;
  buffParams->outHeight = (((buffParams->inHeight + (params->padH * 2U)) - (((params->kernelH - 1U)* params->dilationH) + 1U)) / strideH) + 1U;
#endif
  buffParams->inPitch = inDataParams->pitch[TIDL_LINE_PITCH];
  buffParams->inChPitch = inDataParams->pitch[TIDL_CHANNEL_PITCH];
  buffParams->outPitch = outDataParams->pitch[TIDL_LINE_PITCH];
  buffParams->outChPitch = outDataParams->pitch[TIDL_CHANNEL_PITCH];
  buffParams->outPitchPooling = outDataParams->pitch[TIDL_LINE_PITCH];
  buffParams->outChPitchPooling = outDataParams->pitch[TIDL_CHANNEL_PITCH];
  buffParams->actType = tidlLayer->actParams.actType;
  buffParams->outRoundBitsPtr = algLayer->layerParams.convParams.outRoundBitsPtr;
  buffParams->inScaleFactor = 1;

  if (((net->quantizationStyle == TIDL_QuantStyleNP2Fixed) || (net->quantizationStyle == TIDL_QuantStyleP2Dynamic) ||
       (TIDL_isKernelHighPrecision(net->TIDLLayers[layerIdx].layerKernelType) == (int32_t)FALSE)) &&
      (net->quantizationStyle != TIDL_QuantStyleAsymNP2_TFL))
  {
    if (conv2dparams->enableBias != 0)
    {
      buffParams->inScaleFactor = (conv2dparams->weightScale * inDataParams->tensorScale) / conv2dparams->biasScale;
    }

    outRoundBits = outDataParams->roundBits;
  }

  if ((TIDL_QuantStyleAsymNP2_TFL == net->quantizationStyle) || (TIDL_isKernelHighPrecision(tidlLayer->layerKernelType) != (int32_t)FALSE))
  {
    buffParams->mmaPSATMin = conv2dparams->minPSAT;
    buffParams->mmaPSATMax = conv2dparams->maxPSAT;
  }

  buffParams->secondRoundBits = outRoundBits;
  buffParams->avgPoolingRoundBits = 2;
  buffParams->biasQFact = 0; //: TODO: Not used
  buffParams->quantizationStyle = net->quantizationStyle;
  buffParams->quantFactor = 1;     //: TODO: Not used
  buffParams->zeroWeightValue = 0; //: TODO: Not used
  buffParams->max = 0;
  buffParams->min = (int32_t)0xFFFFFFFFU;
  buffParams->kernelType = conv2dparams->kernelType;
  buffParams->numTotRoi = inDataParams->dimValues[TIDL_DIM_BATCH];
  buffParams->inBatchPitch = inDataParams->pitch[TIDL_ROI_PITCH];
  buffParams->outBatchPitch = outDataParams->pitch[TIDL_ROI_PITCH];
  buffParams->scratchSize = algLayer->scratchSize;
  buffParams->scratchMem = algLayer->scratchMem;
  buffParams->memcpyTr = (uint8_t *)algLayer->memcpyTr;

  return;
}

int32_t TIDL_conv2dRefAlloc(const sTIDL_ConvParams_t *conv2dparams,
                            const TIDL_LayerSpecificParams *layerSpecificParams,
                            const TIDL_NetworkCommonParams *commonParams,
                            int32_t layerIdx,
                            int32_t memorySize[TIDL_LAYER_MEMORY_MAX])
{
  int32_t status = IALG_EOK;
  int32_t scratchDataSize;
  int32_t outDataSize;
  int32_t paramMemSize = 0;

  (void)memset(memorySize, 0, sizeof(memorySize));

  paramMemSize = TIDL_conv2dPersistentMemAllocateAndComputeSize(NULL,
                                                                conv2dparams,
                                                                layerSpecificParams,
                                                                commonParams,
                                                                layerIdx,
                                                                NULL);

  outDataSize = TIDL_conv2dRefGetOutDataSize(commonParams, layerSpecificParams, layerIdx);

  if (commonParams->net->TIDLLayers[layerIdx].layerType == TIDL_Deconv2DLayer)
  {
    scratchDataSize = TIDL_deconv2DGetScratchDataSize(commonParams->createParams, layerIdx);
  }
  else
  {
    scratchDataSize = TIDL_conv2dRefGetScratchDataSize(commonParams, layerIdx);
  }

  memorySize[TIDL_LAYER_MEMORY_SCRATCH] = scratchDataSize + 128;
  memorySize[TIDL_LAYER_MEMORY_PERSISTENT] = paramMemSize + 128;
  memorySize[TIDL_LAYER_MEMORY_OUTPUT] = outDataSize + 128;

  return status;
}

int32_t TIDL_conv2dRefInit(sTIDL_ConvParams_t *conv2dparams,
                           const TIDL_LayerSpecificParams *layerSpecificParams,
                           const TIDL_NetworkCommonParams *commonParams,
                           sTIDL_AlgLayer_t *algLayer,
                           int32_t layerIdx,
                           uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                           int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                           void **outPtr)
{
  int32_t status = IALG_EOK;
  int32_t scratchDataSize;
  int32_t outDataSize;
  int32_t paramMemSize = 0;
  int32_t currOffset = 0;
  int32_t layerType = commonParams->net->TIDLLayers[layerIdx].layerType;

  paramMemSize = TIDL_conv2dPersistentMemAllocateAndComputeSize(memory[TIDL_LAYER_MEMORY_PERSISTENT],
                                                                conv2dparams,
                                                                layerSpecificParams,
                                                                commonParams,
                                                                layerIdx,
                                                                algLayer);

  if (layerType == TIDL_ConvolutionLayer) /* Channelwise quantization only supports convolution*/
  {
    status = TIDL_conv2dFillPerChannelParamters(conv2dparams,
                                                commonParams,
                                                algLayer,
                                                layerIdx);
  }
  if (status == IALG_EOK)
  {
    /* Fill buffers after bias split */
    TIDL_conv2dSetupBiasPointers(conv2dparams,
                                 layerSpecificParams,
                                 commonParams,
                                 algLayer,
                                 layerIdx);

    outDataSize = TIDL_conv2dRefGetOutDataSize(commonParams, layerSpecificParams, layerIdx);

    if (commonParams->net->TIDLLayers[layerIdx].layerType == TIDL_Deconv2DLayer)
    {
      scratchDataSize = TIDL_deconv2DGetScratchDataSize(commonParams->createParams, layerIdx);
    }
    else
    {
      scratchDataSize = TIDL_conv2dRefGetScratchDataSize(commonParams, layerIdx);
    }
    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_FORCE_OUTPTR_NULL)
    {
      outPtr = NULL;
    }
    if (outPtr != NULL)
    {
      if (TIDL_isOutDataBuff(commonParams->net,
                             commonParams->net->TIDLLayers[layerIdx].outData.dataId,
                             commonParams->createParams->currLayersGroupId) == 1)
      {
        *outPtr = NULL;
      }
      else if (outDataSize != 0)
      {
        TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_OUTPUT],
                         &currOffset,
                         outDataSize,
                         128,
                         outPtr);
      }
      else
      {
        *outPtr = NULL;
      }
    }

    currOffset = 0;
    TIDL_AllocatePtr((intptr_t)memory[TIDL_LAYER_MEMORY_SCRATCH],
                     &currOffset,
                     scratchDataSize,
                     128,
                     &algLayer->scratchMem);
    algLayer->scratchSize = scratchDataSize;

    memorySize[TIDL_LAYER_MEMORY_SCRATCH] = scratchDataSize + 128;
    memorySize[TIDL_LAYER_MEMORY_PERSISTENT] = paramMemSize + 128;
    memorySize[TIDL_LAYER_MEMORY_OUTPUT] = outDataSize + 128;
  }
  return status;
}

int32_t TIDL_conv2dRefProcess(sTIDL_ConvParams_t *conv2dparams,
                              const TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t *algLayer,
                              sTIDL_Layer_t *tidlLayer,
                              void *inPtr,
                              void *outPtr,
                              int32_t layerIdx)
{
  int32_t status = IALG_EOK;
  tidlConv2dBuffParams_t buffParams;

  TIDL_conv2dSetupBuffParams(conv2dparams,
                             commonParams->net,
                             algLayer,
                             tidlLayer,
                             layerIdx,
                             &buffParams);

  if (tidlLayer->layerType == TIDL_ConvolutionLayer)
  {
    status = TIDL_refConv2dProcess(commonParams->createParams,
                                   algLayer,
                                   tidlLayer,
                                   conv2dparams,
                                   &buffParams,
                                   (int8_t *)inPtr,
                                   (void *)outPtr);
  }
  else /*tidlLayer->layerType == TIDL_Deconv2DLayer*/
  {
    if ((commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DECONV_FORCE_8B_BIASPARAMSIZE) || (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DECONV_FORCE_16B_BIASPARAMSIZE))
    {
      algLayer->layerParams.convParams.biasParamSize = 2;
    }
    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DECONV_FORCE_BUFSCRATCHSIZE)
    {
      /* Intensionally setting invalid scratchSize value to cover error situation */
      buffParams.scratchSize = -1;
    }
    if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DECONV_FORCE_64B_WEIGHTSELEMENTSIZEINBITS)
    {
      /* Forcefully setting invalid weightsElementSizeInBits value to cover error situation */
      tidlLayer->weightsElementSizeInBits = 64;
    }
    status = TIDL_deconv2dRefProcess(commonParams->createParams,
                                     algLayer,
                                     tidlLayer,
                                     conv2dparams,
                                     &buffParams,
                                     (int8_t *)inPtr,
                                     (void *)outPtr);
  }

  return status;
}
