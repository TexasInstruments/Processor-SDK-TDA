/*
 *
 * Copyright (c) {2022 - 2024} Texas Instruments Incorporated
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
@file    tidl_transpose_ref.c
@brief   This file defines private functions for transpose layer.
@version 0.1 (June 2023) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_transpose.h"
#include "tidl_commonUtils.h"
#define TRANSPOSE_DEBUG 0
#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
static int CUDA_TRANSPOSE_COUNTER = 0;
#endif

/**
 * @brief This is reference implementation of Transpose layer
 *
 * @tparam Tin : template for input data buffers
 * @tparam Tout : template for output data buffers
 * @param  pIn : Pointer to input memory
 * @param  pOut : Pointer to output memory
 * @param inPtrOffset  : offset for the input buffer for transpose
 * @param outPtrOffset : offset for the input buffer for transpose
 * @param outWidth : width of the output buffer
 * @param outHeight : height of the output buffer
 * @param numChs : no of channels
 * @param numROIs : no of ROIs
 * @param inLinePitch : pitch of the input buffer
 * @param outLinePitch : pitch of the output buffer
 * @param inChPitch : Chnnel pitch of the input buffer
 * @param outChPitch : Chnnel pitch of the output buffer
 * @param inROIPitch : ROI pitch of the input buffer
 * @param outROIPitch : ROI pitch of the output buffer
 */
template<class Tin, class Tout> void TIDL_transposeRefProcess(
    const Tin *pIn,
    Tout *pOut,
    sTIDL_DataParams_t *inDataParams,
    const sTIDL_Layer_t *tidlLayer)
{
  int32_t inBatches = inDataParams->dimValues[TIDL_DIM_BATCH];
  int32_t inDIM1 = inDataParams->dimValues[TIDL_DIM_DIM1];
  int32_t inDIM2 = inDataParams->dimValues[TIDL_DIM_DIM2];
  int32_t inChannels = inDataParams->dimValues[TIDL_DIM_NUMCH];
  int32_t inHeight = inDataParams->dimValues[TIDL_DIM_HEIGHT];
  int32_t inWidth = inDataParams->dimValues[TIDL_DIM_WIDTH];

#if TRANSPOSE_DEBUG
  int32_t size = inBatches * inDIM1 * inDIM2 * inChannels * inHeight * inWidth;
  int32_t max = INT32_MIN, min = INT32_MAX;
  for (int32_t t = 0; t < size; t++)
  {
    if (pIn[t] > max)
    {
      max = pIn[t];
    }
    if (pIn[t] < min)
    {
      min = pIn[t];
    }
  }
  printf("Input: min - %d, max - %d\n", min, max);
#endif

  int32_t inLinePitch = inDataParams->pitch[TIDL_LINE_PITCH];
  int32_t inChannelPitch = inDataParams->pitch[TIDL_CHANNEL_PITCH];
  int32_t inDIM2Pitch = inDataParams->pitch[TIDL_DIM2_PITCH];
  int32_t inDIM1Pitch = inDataParams->pitch[TIDL_DIM1_PITCH];
  int32_t inBatchPitch = inDataParams->pitch[TIDL_ROI_PITCH];

  int32_t outDIM1 = tidlLayer->outData.dimValues[TIDL_DIM_DIM1];
  int32_t outDIM2 = tidlLayer->outData.dimValues[TIDL_DIM_DIM2];
  int32_t outChannels = tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
  int32_t outHeight = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  int32_t outWidth = tidlLayer->outData.dimValues[TIDL_DIM_WIDTH];

  int32_t outLinePitch = tidlLayer->outData.pitch[TIDL_LINE_PITCH];
  int32_t outChannelPitch = tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
  int32_t outDIM2Pitch = tidlLayer->outData.pitch[TIDL_DIM2_PITCH];
  int32_t outDIM1Pitch = tidlLayer->outData.pitch[TIDL_DIM1_PITCH];
  int32_t outBatchPitch = tidlLayer->outData.pitch[TIDL_ROI_PITCH];

  const int32_t *perm = tidlLayer->layerParams.transposeParams.perm;
  int32_t targetStrides[TIDL_DIM_MAX];

  targetStrides[perm[TIDL_DIM_WIDTH]] = 1;
  targetStrides[perm[TIDL_DIM_HEIGHT]] = inDataParams->dimValues[perm[TIDL_DIM_WIDTH]];
  targetStrides[perm[TIDL_DIM_NUMCH]] = inDataParams->dimValues[perm[TIDL_DIM_HEIGHT]] * targetStrides[perm[TIDL_DIM_HEIGHT]];
  targetStrides[perm[TIDL_DIM_DIM2]] = inDataParams->dimValues[perm[TIDL_DIM_NUMCH]] * targetStrides[perm[TIDL_DIM_NUMCH]];
  targetStrides[perm[TIDL_DIM_DIM1]] = inDataParams->dimValues[perm[TIDL_DIM_DIM2]] * targetStrides[perm[TIDL_DIM_DIM2]];
  targetStrides[perm[TIDL_DIM_BATCH]] = inDataParams->dimValues[perm[TIDL_DIM_DIM1]] * targetStrides[perm[TIDL_DIM_DIM1]];

  int32_t pp = targetStrides[TIDL_DIM_WIDTH];
  int32_t lp = targetStrides[TIDL_DIM_HEIGHT];
  int32_t cp = targetStrides[TIDL_DIM_NUMCH];
  int32_t d1p = targetStrides[TIDL_DIM_DIM1];
  int32_t d2p = targetStrides[TIDL_DIM_DIM2];
  int32_t bp = targetStrides[TIDL_DIM_BATCH];

  #ifdef BUILD_WITH_CUDA
  int32_t outBatches = inDataParams->dimValues[perm[TIDL_DIM_BATCH]];
  TIDL_cudaTranspose(
    pIn, pOut,
    inBatches,  outBatches, inDIM1,  inDIM2,  inChannels,  inHeight,  inWidth,
    inBatchPitch,  inDIM1Pitch,  inDIM2Pitch,  inChannelPitch,  inLinePitch,
    outBatchPitch,  outDIM1Pitch,  outDIM2Pitch,  outChannelPitch,  outLinePitch,
    outDIM1,  outDIM2,  outChannels,  outHeight,  outWidth,
    pp, lp, cp, d1p, d2p, bp);
  #else
  int32_t targetIndex=0, targetIndexPad=0, bi, d1i, d2i, ci, hi, wi;
  Tout temp;

  int32_t i0, i1, i2, i3, i4, i5;
  for (i0 = 0; i0 < inBatches; i0++)
  {
    for (i1 = 0; i1 < inDIM1; i1++)
    {
      for (i2 = 0; i2 < inDIM2; i2++)
      {
        for (i3 = 0; i3 < inChannels; i3++)
        {
          for (i4 = 0; i4 < inHeight; i4++)
          {
            for (i5 = 0; i5 < inWidth; i5++)
            {
              temp = pIn[(i0 * inBatchPitch) + (i1 * inDIM1Pitch) + (i2 * inDIM2Pitch) + (i3 * inChannelPitch) + (i4 * inLinePitch) + i5];
              targetIndex = (i0 * bp) + (i1 * d1p) + (i2 * d2p) + (i3 * cp) + (i4 * lp) + (i5 * pp);

              bi = targetIndex / (outDIM1 * outDIM2 * outChannels * outHeight * outWidth);
              targetIndex = targetIndex % (outDIM1 * outDIM2 * outChannels * outHeight * outWidth);

              d1i = targetIndex / (outDIM2 * outChannels * outHeight * outWidth);
              targetIndex = targetIndex % (outDIM2 * outChannels * outHeight * outWidth);

              d2i = targetIndex / (outChannels * outHeight * outWidth);
              targetIndex = targetIndex % (outChannels * outHeight * outWidth);

              ci = targetIndex / (outHeight * outWidth);
              targetIndex = targetIndex % (outHeight * outWidth);

              hi = targetIndex / outWidth;
              wi = targetIndex % outWidth;

              targetIndexPad = (bi * outBatchPitch) + (d1i * outDIM1Pitch) + (d2i * outDIM2Pitch) + (ci * outChannelPitch) + (hi * outLinePitch) + wi;
              pOut[targetIndexPad] = temp;
            }
          }
        }
      }
    }
  }
#if TRANSPOSE_DEBUG
  max = INT32_MIN, min = INT32_MAX;
  for (int32_t t = 0; t < size; t++)
  {
    if (pOut[t] > max)
    {
      max = pOut[t];
    }
    if (pOut[t] < min)
    {
      min = pOut[t];
    }
  }
  printf("Output: min - %d, max - %d\n", min, max);
  #endif
  #endif

  #ifdef BUILD_WITH_CUDA
  /*Mark init as completed to prevent re-allocation of buffers for subsequent frames:*/
  TIDL_cudaSetTransposeInitFlag(CUDNNLC);
  #endif
  TIDL_L1DandL2CacheWbInv();
}

template void TIDL_transposeRefProcess(const float32_tidl *pIn, float32_tidl *pOut,
                                       sTIDL_DataParams_t *inDataParams, const sTIDL_Layer_t *tidlLayer);

template void TIDL_transposeRefProcess(const uint8_t *pIn, uint8_t *pOut,
                                       sTIDL_DataParams_t *inDataParams, const sTIDL_Layer_t *tidlLayer);

template void TIDL_transposeRefProcess(const int8_t *pIn, int8_t *pOut,
                                       sTIDL_DataParams_t *inDataParams, const sTIDL_Layer_t *tidlLayer);

template void TIDL_transposeRefProcess(const uint16_t *pIn, uint16_t *pOut,
                                       sTIDL_DataParams_t *inDataParams, const sTIDL_Layer_t *tidlLayer);

template void TIDL_transposeRefProcess(const int16_t *pIn, int16_t *pOut,
                                       sTIDL_DataParams_t *inDataParams, const sTIDL_Layer_t *tidlLayer);
