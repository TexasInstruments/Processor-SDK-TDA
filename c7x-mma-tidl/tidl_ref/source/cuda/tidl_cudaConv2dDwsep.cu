/**
----------------------------------------------------------------------------
@file    tidl_conv2d_cuda.cu
@brief   This file contains Cuda based GPU functions required by 
tidl_conv2d_base.c  used for depthwise convolution.
----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <cuda_runtime.h>
#include <assert.h>
#include "tidl_cuda.h"
#include <cuda.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/extrema.h>
#include <iostream>
#include "tidl_cudaUtilities.cu"
#include "itidl_ti.h"
#include "tidl_cuda_mem_manager.h"

using namespace floating_point::bf16_mma;

//CUDA KERNELS:

template <class Tacc,class Tout>
__global__ void TIDL_convSaturateCudaFixedV1(Tacc *X, Tout *Y, int N, int C, int H, int W, int outBatchPitch, int outChPitch, int outImPitch, int outRoundBits, Tacc satLow, Tacc satHigh, int mixedPrecision)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int nc, cc, hc, wc, k;
  int64_t outAcc;
  if(i < (N * C * H * W))
  {
    k = i;
    nc = k / (C * H * W);
    k = k%(C * H * W);
    cc = k / (H * W);
    k = k % (H * W);
    hc = k / W;
    k = k % W;
    wc = k;
    outAcc = X[ nc * (outBatchPitch) + cc * (outChPitch) + hc * (outImPitch) + wc];
    outAcc = cuda_roundSatMMA(outAcc, outRoundBits, satLow, satHigh);
    if(mixedPrecision == 1)
    {
      outAcc = (uint64_t)outAcc >> 8U;
    }
    // if(outRoundBits != 0)
    // {
    //   outAcc += (1 << (outRoundBits - 1));
    //   outAcc >>= outRoundBits;
    // } 
    // outAcc = outAcc < satLow ? satLow : outAcc;
    // outAcc = outAcc > satHigh ? satHigh : outAcc;
    Y[ nc * (outBatchPitch) + cc * (outChPitch) + hc * (outImPitch) + wc] = (Tacc)outAcc;
  }
}


template <class Tacc,class Tout>
__global__ void TIDL_convSaturateCudaFixedV2(Tacc *X, Tout *Y, int N, int C, int H, int W, int outBatchPitch, int outChPitch, int outImPitch, int outRoundBits, Tacc satLow, Tacc satHigh, uint8_t* dpDerivedScales, uint8_t* dpDerivedShifts)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int nc, cc, hc, wc, k;
  int64_t outAcc;
  if(i < (N * C * H * W))
  {
    k = i;
    nc = k / (C * H * W);
    k = k%(C * H * W);
    cc = k / (H * W);
    k = k % (H * W);
    hc = k / W;
    k = k % W;
    wc = k;
    outAcc = X[ nc * (outBatchPitch) + cc * (outChPitch) + hc * (outImPitch) + wc];
    outAcc *= dpDerivedScales[cc];
    outAcc = cuda_roundSatMMA(outAcc, dpDerivedShifts[cc], satLow, satHigh);
    // if(dpDerivedShifts[cc] != 0)
    // {
    //   outAcc += (1 << (dpDerivedShifts[cc] - 1));
    //   outAcc >>= dpDerivedShifts[cc];
    // } 
    // outAcc = outAcc < satLow ? satLow : outAcc;
    // outAcc = outAcc > satHigh ? satHigh : outAcc;
    /*There is no overflow detection for 8-bit!*/
    //outAcc = ((int64_t)outAcc & (int64_t)0xFFFFFFFFFFU);
    Y[ nc * (outBatchPitch) + cc * (outChPitch) + hc * (outImPitch) + wc] = (Tacc)outAcc;
  }
}


template <class Tacc,class Tout>
__global__ void TIDL_convSaturateCudaFloat(Tacc *X, Tout *Y, int N, int C, int H, int W, int outBatchPitch, int outChPitch, int outImPitch, int outRoundBits, float satLow, float satHigh)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int nc, cc, hc, wc, k;
  Tacc outAcc;
  if(i < (N * C * H * W))
  {
    k = i;
    nc = k / (C * H * W);
    k = k % (C * H * W);
    cc = k / (H * W);
    k = k % (H * W);
    hc = k / W;
    k = k % W;
    wc = k;
    outAcc = X[ nc*(outBatchPitch) + cc * (outChPitch) + hc * (outImPitch) + wc];
    outAcc = outAcc < satLow ? satLow : outAcc;
    outAcc = outAcc > satHigh ? satHigh : outAcc;
    Y[ nc * (outBatchPitch) + cc * (outChPitch) + hc * (outImPitch) + wc] = outAcc;
  }
}

template <class Tacc, class Tout>
__global__ void TIDL_convSaturateCudaBFloat16(
    Tacc  *X,
    Tout  *Y,
    int N, int C, int H, int W,
    int outBatchPitch, int outChPitch, int outImPitch,
    float satLow, float satHigh)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= N * C * H * W) return;

  int k  = i;
  int nc = k / (C * H * W); k = k % (C * H * W);
  int cc = k / (H * W);     k = k % (H * W);
  int hc = k / W;
  int wc = k % W;

  int idx = nc * outBatchPitch + cc * outChPitch + hc * outImPitch + wc;
  float val = (float)X[idx];
  // cuda_bf16Sat signature: (val, actMin, actMax)
  Tout res = cuda_bf16Sat(val, satLow, satHigh);
  Y[idx] = res;
}


/**
----------------------------------------------------------------------------
@fn         TIDL_cudaConvKernel
@brief      Cuda kernel for performing convolution

@param      pInchanneli : Host input stream
@param      pCoeffsi : Host coeffecient stream
@param      pBias : Host bias stream
@param      accPtri : Host accumulator stream
@param      min : Pointer to min passed from the calling function
@param      max : Pointer to max passed from the calling function
@param      numTotRoi : Number of batches
@param      numGroups : Number of groups
@param      numInChannels : Number of input channels
@param      numOutChannels : Number of output channels
@param      inChPitch : Input channel pitch
@param      outChPitch : Output channel pitch
@param      width : Input Width (*stride width)
@param      height : Input Height (*stride height)
@param      inImPitch : Input image pitch
@param      outImPitch : Output image pitch
@param      coeffsWidth: Filter width
@param      coeffsHeight : Filter height
@param      dilationWidth: Dilation along width
@param      dilationHeight : Dilation along height
@param      strideWidth: Convolutional stride along the width
@param      strideHeight : Convolutional stride along the height

@remarks    None
----------------------------------------------------------------------------
*/
template <class Tin, class Tw, class Tb, class Tacc>
__global__ void TIDL_cudaConvKernel(
  Tin     * __restrict__ pInChannel,
  Tw      * __restrict__ pCoeffs,
  Tb      * __restrict__ pBias,
  Tacc    * __restrict__ accPtr,
  Tacc    *min,
  Tacc    *max,
  const int32_t  numTotRoi,
  const int32_t  numGroups,
  const int32_t  numInChannels,
  const int32_t  numOutChannels,
  const int32_t  inChPitch,
  const int32_t  outChPitch,
  const int32_t  width,
  const int32_t  height,
  const int32_t  inImPitch,
  const int32_t  outImPitch,
  const int32_t  coeffsWidth,
  const int32_t  coeffsHeight,
  const int32_t  dilationWidth,
  const int32_t  dilationHeight,
  const int32_t  strideWidth,
  const int32_t  strideHeight,
  const int32_t enableBias,
  const int32_t isOTFpad,
  const int32_t leftPad,
  const int32_t topPad,
  const int32_t padVal,
  const int32_t startRowNumberInTensor,
  const int32_t inHeight,
  const int32_t inWidth
  )
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  
  // Pre-compute total size for bounds check
  const int32_t totalSize = numTotRoi * numGroups * numOutChannels * width * height;
  if(i >= totalSize) return;  // Early exit for out-of-bounds threads

  // Pre-compute valid region bounds (hoisted out of loops)
  const int32_t validPosXMin = leftPad;
  const int32_t validPosXMax = leftPad + inWidth;
  const int32_t validPosYMin = topPad;
  const int32_t validPosYMax = topPad + inHeight;

  // Index decomposition
  int32_t k = i;
  const int32_t i8 = k / (numGroups * numOutChannels * width * height);  // batch
  k = k % (numGroups * numOutChannels * width * height);
  const int32_t i7 = k / (numOutChannels * width * height);  // group
  k = k % (numOutChannels * width * height);
  const int32_t i6 = k / (width * height);  // output channel
  k = k % (width * height);
  const int32_t i2 = k / width;  // output Y
  const int32_t i3 = k % width;  // output X

  // Pre-compute offsets (hoisted out of inner loops)
  const int32_t indataOffset = i7 * numInChannels * inChPitch + i8 * numGroups * numInChannels * inChPitch;
  const int32_t outdataOffset = i7 * numOutChannels * outChPitch + i8 * numGroups * numOutChannels * outChPitch;
  
  // Pre-compute base spatial positions (hoisted - these are constant within this thread)
  const int32_t baseInY = i2 * strideHeight;
  const int32_t baseInX = i3 * strideWidth;
  
  // Pre-compute coefficient base offset for this output channel
  const int32_t coeffBaseOffset = i7 * numInChannels * coeffsHeight * coeffsWidth * numOutChannels
                                + i6 * numInChannels * coeffsHeight * coeffsWidth;

  // Initialize accumulator with bias
  Tacc outAcc;
  if(enableBias)
  {
    outAcc = pBias[i7 * numOutChannels + i6];
  }
  else
  {
    outAcc = 0;
  }

  // Main convolution loop
  #pragma unroll 4
  for (int32_t i0 = 0; i0 < numInChannels; i0++)
  {
    const int32_t inChannelOffset = indataOffset + i0 * inChPitch;
    const int32_t coeffChannelOffset = coeffBaseOffset + i0 * coeffsHeight * coeffsWidth;

    #pragma unroll
    for (int32_t i4 = 0; i4 < coeffsHeight; i4++)
    {
      const int32_t spatialOffsetY = baseInY + (i4 * dilationHeight);
      const bool validY = (spatialOffsetY >= validPosYMin) && (spatialOffsetY < validPosYMax);
      const int32_t inYOffset = spatialOffsetY * inImPitch;

      #pragma unroll
      for (int32_t i5 = 0; i5 < coeffsWidth; i5++)
      {
        const int32_t spatialOffsetX = baseInX + (i5 * dilationWidth);
        const bool validX = (spatialOffsetX >= validPosXMin) && (spatialOffsetX < validPosXMax);

        Tin inData;
        if(isOTFpad && !(validY && validX))
        {
          inData = (Tin)padVal;
        }
        else
        {
          inData = pInChannel[inChannelOffset + inYOffset + spatialOffsetX];
        }

        const Tw coefData = pCoeffs[coeffChannelOffset + (i4 * coeffsWidth) + i5];
        outAcc += ((Tacc)inData * (Tacc)coefData);
      }
    }
  }

  // Write output
  accPtr[outdataOffset + i6 * outChPitch + (i2 * outImPitch) + i3] = outAcc;
}


/**
----------------------------------------------------------------------------
@fn         TIDL_cudaConvKernel1x1
@brief      Optimized CUDA kernel for 1x1 convolution (no filter loops needed)
            Maintains bit-exact compatibility with reference by preserving 
            accumulation order across input channels.

@remarks    Specialized for coeffsWidth=1, coeffsHeight=1 (pointwise convolution)
----------------------------------------------------------------------------
*/
template <class Tin, class Tw, class Tb, class Tacc>
__global__ void TIDL_cudaConvKernel1x1(
  Tin     * __restrict__ pInChannel,
  Tw      * __restrict__ pCoeffs,
  Tb      * __restrict__ pBias,
  Tacc    * __restrict__ accPtr,
  Tacc    *min,
  Tacc    *max,
  const int32_t  numTotRoi,
  const int32_t  numGroups,
  const int32_t  numInChannels,
  const int32_t  numOutChannels,
  const int32_t  inChPitch,
  const int32_t  outChPitch,
  const int32_t  width,
  const int32_t  height,
  const int32_t  inImPitch,
  const int32_t  outImPitch,
  const int32_t  strideWidth,
  const int32_t  strideHeight,
  const int32_t  enableBias,
  const int32_t  isOTFpad,
  const int32_t  leftPad,
  const int32_t  topPad,
  const int32_t  padVal,
  const int32_t  inHeight,
  const int32_t  inWidth
  )
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  
  const int32_t totalSize = numTotRoi * numGroups * numOutChannels * width * height;
  if(i >= totalSize) return;

  // Pre-compute valid region bounds for OTF padding
  const int32_t validPosXMin = leftPad;
  const int32_t validPosXMax = leftPad + inWidth;
  const int32_t validPosYMin = topPad;
  const int32_t validPosYMax = topPad + inHeight;

  // Index decomposition
  int32_t k = i;
  const int32_t i8 = k / (numGroups * numOutChannels * width * height);  // batch
  k = k % (numGroups * numOutChannels * width * height);
  const int32_t i7 = k / (numOutChannels * width * height);  // group
  k = k % (numOutChannels * width * height);
  const int32_t i6 = k / (width * height);  // output channel
  k = k % (width * height);
  const int32_t i2 = k / width;  // output Y
  const int32_t i3 = k % width;  // output X

  // Pre-compute offsets
  const int32_t indataOffset = i7 * numInChannels * inChPitch + i8 * numGroups * numInChannels * inChPitch;
  const int32_t outdataOffset = i7 * numOutChannels * outChPitch + i8 * numGroups * numOutChannels * outChPitch;

  // For 1x1 conv: spatial position is simply stride-scaled output position
  const int32_t spatialY = i2 * strideHeight;
  const int32_t spatialX = i3 * strideWidth;

  // Check if within valid region (for OTF padding)
  const bool validPos = (spatialX >= validPosXMin) && (spatialX < validPosXMax) &&
                        (spatialY >= validPosYMin) && (spatialY < validPosYMax);

  // Coefficient base offset for this output channel (1x1 filter: 1 coeff per input channel)
  const int32_t coeffBaseOffset = i7 * numInChannels * numOutChannels + i6 * numInChannels;

  // Initialize accumulator with bias
  Tacc outAcc;
  if(enableBias)
  {
    outAcc = pBias[i7 * numOutChannels + i6];
  }
  else
  {
    outAcc = 0;
  }

  // Pre-compute input spatial offset (same for all channels in 1x1)
  const int32_t inSpatialOffset = spatialY * inImPitch + spatialX;
  
  // Main convolution loop - only over input channels (no filter spatial loops)
  #pragma unroll 8
  for (int32_t i0 = 0; i0 < numInChannels; i0++)
  {
    Tin inData;
    if(isOTFpad && !validPos)
    {
      inData = (Tin)padVal;
    }
    else
    {
      inData = pInChannel[indataOffset + i0 * inChPitch + inSpatialOffset];
    }

    const Tw coefData = pCoeffs[coeffBaseOffset + i0];
    outAcc += ((Tacc)inData * (Tacc)coefData);
  }

  // Write output
  accPtr[outdataOffset + i6 * outChPitch + (i2 * outImPitch) + i3] = outAcc;
}


/**
----------------------------------------------------------------------------
@fn         TIDL_cudaConvKernel3x3
@brief      Optimized CUDA kernel for 3x3 convolution with fully unrolled 
            filter loops. Maintains bit-exact compatibility with reference
            by preserving accumulation order.

@remarks    Specialized for coeffsWidth=3, coeffsHeight=3
            Uses fully unrolled loops and register caching for coefficients
----------------------------------------------------------------------------
*/
template <class Tin, class Tw, class Tb, class Tacc>
__global__ void TIDL_cudaConvKernel3x3(
  Tin     * __restrict__ pInChannel,
  Tw      * __restrict__ pCoeffs,
  Tb      * __restrict__ pBias,
  Tacc    * __restrict__ accPtr,
  Tacc    *min,
  Tacc    *max,
  const int32_t  numTotRoi,
  const int32_t  numGroups,
  const int32_t  numInChannels,
  const int32_t  numOutChannels,
  const int32_t  inChPitch,
  const int32_t  outChPitch,
  const int32_t  width,
  const int32_t  height,
  const int32_t  inImPitch,
  const int32_t  outImPitch,
  const int32_t  dilationWidth,
  const int32_t  dilationHeight,
  const int32_t  strideWidth,
  const int32_t  strideHeight,
  const int32_t  enableBias,
  const int32_t  isOTFpad,
  const int32_t  leftPad,
  const int32_t  topPad,
  const int32_t  padVal,
  const int32_t  inHeight,
  const int32_t  inWidth
  )
{
  const int i = blockIdx.x * blockDim.x + threadIdx.x;
  const int32_t totalSize = numTotRoi * numGroups * numOutChannels * width * height;
  if(i >= totalSize) return;

  // Pre-compute valid region bounds
  const int32_t validPosXMin = leftPad;
  const int32_t validPosXMax = leftPad + inWidth;
  const int32_t validPosYMin = topPad;
  const int32_t validPosYMax = topPad + inHeight;

  // Index decomposition
  int32_t k = i;
  const int32_t i8 = k / (numGroups * numOutChannels * width * height);  // batch
  k = k % (numGroups * numOutChannels * width * height);
  const int32_t i7 = k / (numOutChannels * width * height);  // group
  k = k % (numOutChannels * width * height);
  const int32_t i6 = k / (width * height);  // output channel
  k = k % (width * height);
  const int32_t i2 = k / width;  // output Y
  const int32_t i3 = k % width;  // output X

  // Pre-compute offsets
  const int32_t indataOffset = i7 * numInChannels * inChPitch + i8 * numGroups * numInChannels * inChPitch;
  const int32_t outdataOffset = i7 * numOutChannels * outChPitch + i8 * numGroups * numOutChannels * outChPitch;
  
  // Base spatial positions
  const int32_t baseInY = i2 * strideHeight;
  const int32_t baseInX = i3 * strideWidth;
  
  // Coefficient base offset for this output channel (3x3 = 9 coeffs per input channel)
  const int32_t coeffBaseOffset = i7 * numInChannels * 9 * numOutChannels + i6 * numInChannels * 9;

  // Initialize accumulator with bias
  Tacc outAcc;
  if(enableBias)
  {
    outAcc = pBias[i7 * numOutChannels + i6];
  }
  else
  {
    outAcc = 0;
  }

  // Pre-compute spatial offsets for 3x3 filter positions (Y direction)
  const int32_t spatialY0 = baseInY + (0 * dilationHeight);
  const int32_t spatialY1 = baseInY + (1 * dilationHeight);
  const int32_t spatialY2 = baseInY + (2 * dilationHeight);
  
  // Pre-compute Y validity
  const bool validY0 = (spatialY0 >= validPosYMin) && (spatialY0 < validPosYMax);
  const bool validY1 = (spatialY1 >= validPosYMin) && (spatialY1 < validPosYMax);
  const bool validY2 = (spatialY2 >= validPosYMin) && (spatialY2 < validPosYMax);
  
  // Pre-compute Y offsets
  const int32_t inYOffset0 = spatialY0 * inImPitch;
  const int32_t inYOffset1 = spatialY1 * inImPitch;
  const int32_t inYOffset2 = spatialY2 * inImPitch;

  // Pre-compute spatial offsets for 3x3 filter positions (X direction)
  const int32_t spatialX0 = baseInX + (0 * dilationWidth);
  const int32_t spatialX1 = baseInX + (1 * dilationWidth);
  const int32_t spatialX2 = baseInX + (2 * dilationWidth);
  
  // Pre-compute X validity
  const bool validX0 = (spatialX0 >= validPosXMin) && (spatialX0 < validPosXMax);
  const bool validX1 = (spatialX1 >= validPosXMin) && (spatialX1 < validPosXMax);
  const bool validX2 = (spatialX2 >= validPosXMin) && (spatialX2 < validPosXMax);

  // Main convolution loop - preserve accumulation order for bit-matching
  #pragma unroll 4
  for (int32_t i0 = 0; i0 < numInChannels; i0++)
  {
    const int32_t inChannelOffset = indataOffset + i0 * inChPitch;
    const int32_t coeffChannelOffset = coeffBaseOffset + i0 * 9;

    // Load 9 coefficients into registers for this input channel
    const Tw c0 = pCoeffs[coeffChannelOffset + 0];
    const Tw c1 = pCoeffs[coeffChannelOffset + 1];
    const Tw c2 = pCoeffs[coeffChannelOffset + 2];
    const Tw c3 = pCoeffs[coeffChannelOffset + 3];
    const Tw c4 = pCoeffs[coeffChannelOffset + 4];
    const Tw c5 = pCoeffs[coeffChannelOffset + 5];
    const Tw c6 = pCoeffs[coeffChannelOffset + 6];
    const Tw c7 = pCoeffs[coeffChannelOffset + 7];
    const Tw c8 = pCoeffs[coeffChannelOffset + 8];

    // Row 0 (filter positions 0,1,2) - preserve accumulation order: row by row, col by col
    {
      const Tin in0 = (isOTFpad && !(validY0 && validX0)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset0 + spatialX0];
      outAcc += ((Tacc)in0 * (Tacc)c0);
      const Tin in1 = (isOTFpad && !(validY0 && validX1)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset0 + spatialX1];
      outAcc += ((Tacc)in1 * (Tacc)c1);
      const Tin in2 = (isOTFpad && !(validY0 && validX2)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset0 + spatialX2];
      outAcc += ((Tacc)in2 * (Tacc)c2);
    }

    // Row 1 (filter positions 3,4,5)
    {
      const Tin in3 = (isOTFpad && !(validY1 && validX0)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset1 + spatialX0];
      outAcc += ((Tacc)in3 * (Tacc)c3);
      const Tin in4 = (isOTFpad && !(validY1 && validX1)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset1 + spatialX1];
      outAcc += ((Tacc)in4 * (Tacc)c4);
      const Tin in5 = (isOTFpad && !(validY1 && validX2)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset1 + spatialX2];
      outAcc += ((Tacc)in5 * (Tacc)c5);
    }

    // Row 2 (filter positions 6,7,8)
    {
      const Tin in6 = (isOTFpad && !(validY2 && validX0)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset2 + spatialX0];
      outAcc += ((Tacc)in6 * (Tacc)c6);
      const Tin in7 = (isOTFpad && !(validY2 && validX1)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset2 + spatialX1];
      outAcc += ((Tacc)in7 * (Tacc)c7);
      const Tin in8 = (isOTFpad && !(validY2 && validX2)) ? (Tin)padVal : pInChannel[inChannelOffset + inYOffset2 + spatialX2];
      outAcc += ((Tacc)in8 * (Tacc)c8);
    }
  }

  // Write output
  accPtr[outdataOffset + i6 * outChPitch + (i2 * outImPitch) + i3] = outAcc;
}


void TIDL_cudaSaturationFloat(
    sTIDL_Layer_t *tidlLayer,
    float32_tidl *min,
    float32_tidl *max)
{
  if (((tidlLayer->actParams.actType == TIDL_NoAct) && (tidlLayer->clipParams.isClipEnabled == 0)) ||
      (tidlLayer->actParams.actType == TIDL_PRelU) ||
      (tidlLayer->actParams.actType == TIDL_GELU) ||
      (tidlLayer->actParams.actType == TIDL_LeakyReLU))
  {
    *max = FLT_MAX;
    *min = -FLT_MAX;
  }
  else if (tidlLayer->actParams.actType == TIDL_RelU)
  {
    *max = FLT_MAX;
    *min = 0.0;
  }
  else if (tidlLayer->actParams.actType == TIDL_RelU6)
  {
    *max = 6.0;
    *min = 0.0;
  }
  else if (tidlLayer->clipParams.isClipEnabled == 1)
  {
    *max = tidlLayer->clipParams.clipMax;
    *min = tidlLayer->clipParams.clipMin;
  }
}

/**
----------------------------------------------------------------------------
@fn         TIDL_cudaConv2DSaturateFloat
@brief      Function performs saturation and places the result in devPtrOf

@param      devPtrOf : Device pointer to a pointer to a stream of input integers
@param      N : Number of batches
@param      C : Number of output channels
@param      H : Output Height
@param      W : Output Width
@param      outChPitch : Output channel pitch
@param      outImPitch : Output image pitch
@param      outRoundBits : Number of rounding bits
@param      satLow : Saturation lower limit
@param      satHigh : Saturation upper limit
@param      enablePerChannelShift : to set per channel shift
@param      precisionAdjustmentShift : For mixed precision when output is 8 bit to match target 
            implementation shift is adjusted. This value is expcted to be 8 when output is 8 bit
            processing is in 16 bit
@remarks    None
----------------------------------------------------------------------------
*/
template <class Tacc, class Tout>
int TIDL_cudaConv2DSaturateFloat(
                                 Tacc *accPtr,
                                 Tout *outPtr,
                                 int32_t numBatches,
                                 int32_t numOutChannels,
                                 int32_t height,
                                 int32_t width,
                                 int32_t strideHeight,
                                 int32_t strideWidth,
                                 int32_t outBatchPitch,
                                 int32_t outChPitch,
                                 int32_t outImPitch,
                                 float padVal,
                                 int32_t outHeight,
                                 int32_t outWidth,
                                 int32_t outPadOffset,
                                 sTIDL_Layer_t *pTIDLNet
                                )
{
  /*Pointers:*/
  Tacc     *dpAcc = NULL;
  Tout     *dpOut = NULL;
 
  /*Initializations:*/
  float satLow, satHigh;
  /*Stream launch size:*/
  int sizeOutstream = numBatches * numOutChannels * outChPitch;


  /*Translate accPtr to GPU pointer */
  if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accPtr, (void**)&dpAcc, sizeof(Tacc) * sizeOutstream) != IALG_EOK)
  {
    printf("Convolve2D: Unable to find accumulator pointer\n");
  }

  /*Translate outPtr to GPU pointer */
  if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * sizeOutstream) != IALG_EOK)
  {
    if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * sizeOutstream) != IALG_EOK)
    {
      printf("Convolve2D: Unable to find output pointer\n");
    }
  }

  // ---- CUDA Streams Optimization ----------------------------------------
  cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

  TIDL_convFillZeroPoint<Tout><<<GRID_SIZE(sizeOutstream, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
      dpOut, sizeOutstream, (float)0);

  if(pTIDLNet->outData.elementType == TIDL_SinglePrecFloat)
  {
    TIDL_cudaSaturationFloat(pTIDLNet, &satLow, &satHigh);
    TIDL_convSaturateCudaFloat<float, float><<<GRID_SIZE(sizeOutstream, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
        reinterpret_cast<float *>(dpAcc),
        reinterpret_cast<float *>(dpOut),
        numBatches, numOutChannels, outHeight, outWidth,
        outBatchPitch, outChPitch, outImPitch,
        0, satLow, satHigh);
  }
  else if(pTIDLNet->outData.elementType == TIDL_BFloat16)
  {
    TIDL_cudaSaturationFloat(pTIDLNet, &satLow, &satHigh);
    TIDL_convSaturateCudaBFloat16<float, bfloat16_tidl><<<GRID_SIZE(sizeOutstream, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
        reinterpret_cast<float *>(dpAcc),
        reinterpret_cast<bfloat16_tidl *>(dpOut),
        numBatches, numOutChannels, outHeight, outWidth,
        outBatchPitch, outChPitch, outImPitch,
        satLow, satHigh);
  }

  /* Async D2H – ordered after kernels on same stream */
  checkCudaErr(cudaMemcpyAsync(outPtr, dpOut,
                                sizeof(Tout) * (sizeOutstream - outPadOffset),
                                cudaMemcpyDeviceToHost, stream));
  checkCudaErr(cudaStreamSynchronize(stream));

  return 0;
}

/**
----------------------------------------------------------------------------
@fn         TIDL_cudaSaturateFixedPoint
@brief      Function performs saturation and places the result in devPtrOf

@param      devPtrOf : Device pointer to a pointer to a stream of input integers
@param      N : Number of batches
@param      C : Number of output channels
@param      H : Output Height
@param      W : Output Width
@param      outChPitch : Output channel pitch
@param      outImPitch : Output image pitch
@param      outRoundBits : Number of rounding bits
@param      satLow : Saturation lower limit
@param      satHigh : Saturation upper limit
@param      enablePerChannelShift : to set per channel shift
@param      precisionAdjustmentShift : For mixed precision when output is 8 bit to match target 
            implementation shift is adjusted. This value is expcted to be 8 when output is 8 bit
            processing is in 16 bit
@remarks    None
----------------------------------------------------------------------------
*/
template <class Tacc, class Tout>
int TIDL_cudaSaturateFixedPoint(
                                 Tacc *accPtr,
                                 Tout *outPtr,
                                 int32_t numBatches,
                                 int32_t numOutChannels,
                                 int32_t height,
                                 int32_t width,
                                 int32_t strideHeight,
                                 int32_t strideWidth,
                                 int32_t outBatchPitch,
                                 int32_t outChPitch,
                                 int32_t outImPitch,
                                 uint8_t *mmav2_Scales,
                                 uint8_t *mmav2_Shifts,
                                 int32_t outRoundBits, /*Required by V1 flow*/
                                 int32_t minSat,
                                 int32_t maxSat,
                                 int32_t padVal,
                                 int32_t outHeight,
                                 int32_t outWidth,
                                 int32_t outPadOffset,
                                 int32_t mixedPrecision,
                                 sTIDL_Layer_t *pTIDLNet
                                )
{
  int sizeOutstream = numBatches * numOutChannels * outChPitch;
  int sizeRoundBits = numOutChannels;

  /*Initializations*/
  Tacc *dpAcc;
  Tout *dpOut;
  uint8_t *dpScales;
  uint8_t *dpShifts;
  /*Get Accumulator, Output, Scale & Shift pointers:*/

  /*Translate accPtr to GPU pointer */
  if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accPtr, (void**)&dpAcc, sizeof(Tacc) * sizeOutstream) != IALG_EOK)
  {
    printf("Convolve2D: Unable to find accumulator pointer\n");
  }

  /*Translate outPtr to GPU pointer */
  if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * sizeOutstream) != IALG_EOK)
  {
    if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * sizeOutstream) != IALG_EOK)
    {
      printf("Convolve2D: Unable to find output pointer\n");
    }
  }

  // ---- CUDA Streams Optimization ----------------------------------------
  cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

  /* --- stream: async H2D copy of output buffer ----------------------- */
  checkCudaErr(cudaMemcpyAsync(dpOut, outPtr,
                                sizeof(Tout) * (sizeOutstream - outPadOffset),
                                cudaMemcpyHostToDevice, stream));

  /* Fill zero-point (ordered after H2D on stream) */
  TIDL_fillZeroPoint<Tout><<<GRID_SIZE(sizeOutstream, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
      dpOut, (Tout)padVal,
      numBatches, 1, 1, numOutChannels, outHeight, outWidth,
      outBatchPitch, 1, 1, outChPitch, outImPitch);

  if(mmav2_Scales != NULL && mmav2_Shifts != NULL)
  {
    /*Asymmetric/MMAv2 Flow*/
    /*Translate scales & shift pointers:*/
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), mmav2_Scales, (void**)&dpScales, sizeof(uint8_t) * sizeRoundBits) != IALG_EOK)
    {
      printf("Convolve2D: Unable to find scales pointer\n");
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), mmav2_Shifts, (void**)&dpShifts, sizeof(uint8_t) * sizeRoundBits) != IALG_EOK)
    {
      printf("Convolve2D: Unable to find shifts pointer\n");
    }

    checkCudaErr(cudaMemcpyAsync(dpScales, mmav2_Scales,
                                  sizeof(uint8_t) * sizeRoundBits,
                                  cudaMemcpyHostToDevice, stream));

    checkCudaErr(cudaMemcpyAsync(dpShifts, mmav2_Shifts,
                                  sizeof(uint8_t) * sizeRoundBits,
                                  cudaMemcpyHostToDevice, stream));


    TIDL_convSaturateCudaFixedV2<Tacc, Tout><<<GRID_SIZE(sizeOutstream, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
        (Tacc*)dpAcc, (Tout*)dpOut,
        numBatches, numOutChannels, outHeight, outWidth,
        outBatchPitch, outChPitch, outImPitch,
        0, minSat, maxSat, dpScales, dpShifts);
  }
  else
  {

    TIDL_convSaturateCudaFixedV1<Tacc, Tout><<<GRID_SIZE(sizeOutstream, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
        (Tacc*)dpAcc, (Tout*)dpOut,
        numBatches, numOutChannels, outHeight, outWidth,
        outBatchPitch, outChPitch, outImPitch,
        outRoundBits, minSat, maxSat, mixedPrecision);
  }

  /* Async D2H – ordered after saturation kernel on stream */
  checkCudaErr(cudaMemcpyAsync(outPtr, dpOut,
                                sizeof(Tout) * (sizeOutstream - outPadOffset),
                                cudaMemcpyDeviceToHost, stream));
  checkCudaErr(cudaStreamSynchronize(stream));


  return 0;
}




//END OF CUDA KERNELS

/**
----------------------------------------------------------------------------
@fn         TIDL_cudaConvolve2d
@brief      Function performs convolution on the GPU based on the parameters 
passed to it (cuda_VERSION)

@param      pInchanneli : Host input stream
@param      pCoeffsi : Host coeffecient stream
@param      pBias : Host bias stream
@param      accPtri : Host accumulator stream
@param      min : Pointer to min passed from the calling function
@param      max : Pointer to max passed from the calling function
@param      numTotRoi : Number of batches
@param      numGroups : Number of groups
@param      numInChannels : Number of input channels
@param      numOutChannels : Number of output channels
@param      inChPitch : Input channel pitch
@param      outChPitch : Output channel pitch
@param      width : Input Width
@param      height : Input Height
@param      inImPitch : Input image pitch
@param      outImPitch : Output image pitch
@param      coeffsWidth: Filter width
@param      coeffsHeight : Filter height
@param      dilationWidth: Dilation along width
@param      dilationHeight : Dilation along height
@param      strideWidth: Convolutional stride along the width
@param      strideHeight : Convolutional stride along the height

@remarks    None
@return     Zero on success
----------------------------------------------------------------------------
*/
template <class Tin, class Tw, class Tb, class Tacc>
int TIDL_cudaConvolve2d(
  Tin     *pInChannel,
  Tw      *pCoeffs,
  Tb      *pBias,
  Tacc    *accPtr,
  Tacc    *min,
  Tacc    *max,
  int32_t  numTotRoi,
  int32_t  numGroups,
  int32_t  numInChannels,
  int32_t  numOutChannels,
  int32_t  inChPitch,
  int32_t  outChPitch,
  int32_t  width,
  int32_t  height,
  int32_t  inImPitch,
  int32_t  outImPitch,
  int32_t  coeffsWidth,
  int32_t  coeffsHeight,
  int32_t  dilationWidth,
  int32_t  dilationHeight,
  int32_t  strideWidth,
  int32_t  strideHeight,
  int32_t enableBias,
  int32_t isOTFpad,
  int32_t leftPad,
  int32_t topPad,
  int32_t padVal,
  int32_t startRowNumberInTensor,
  int32_t inHeight,
  int32_t inWidth,
  int32_t outHeight,
  int32_t outWidth
  )
{
  // Check if memory manager is initialized
  if (TIDL_cudaGetThreadManager() == NULL || !TIDL_cudaGetThreadManager()->isInitialized)
  {
    printf("ERROR: CUDA Memory Manager not initialized in TIDL_cudaConvolve2d\n");
    return -1;
  }

  long int  launchSize;
  Tin      *dpIn = NULL;
  Tw       *dpCoeffs = NULL;
  Tb       *dpBias = NULL;
  Tacc     *dpAcc = NULL;

  // Calculate buffer sizes
  int inSize = numTotRoi * numGroups * numInChannels * inChPitch;
  int inTxSize = inSize;
  int inputOffset = 0;

  Tin *pInChannelOriginal = pInChannel;
  /*For OTF, the buffer is actually larger by a factor of topPad*/
  if(isOTFpad)
  {
    inSize += numTotRoi * numGroups * numInChannels * (topPad * inWidth);
    //pInChannel = pInChannel - (topPad * inImPitch + leftPad);
    inputOffset = topPad * inImPitch + leftPad;
  }

  int outSize = numTotRoi * numGroups * numOutChannels * outChPitch;
  int filterSize = numOutChannels * numGroups * numInChannels * coeffsHeight * coeffsWidth;
  int biasSize = numTotRoi * numGroups * numOutChannels;

    // Use memory manager to get or allocate GPU pointers
    // Input buffer
  if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pInChannel, (void**)&dpIn, sizeof(Tin) * inTxSize) != IALG_EOK)
  {
      /*If it's the first layer - it is possible that the input doesn't have a GPU mirror pointer, allocate a new buffer*/
      if( TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pInChannel, (void**)&dpIn, sizeof(Tin) * inTxSize) != IALG_EOK)
      {
        /*Log Error*/
      printf("Convolve2D: Unable to find input pointer\n");
      }
  }

  /* ---- Coefficients buffer --------------------------------------------- */
  /* Track whether a fresh H2D upload is needed (first-use allocation only).
   * Once weights are resident on the GPU they are not re-uploaded across
   * inference calls for the same layer. */
  bool needsCopyCoeffs = false;
  if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pCoeffs, (void**)&dpCoeffs, sizeof(Tin) * filterSize) != IALG_EOK)
  {
    if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pCoeffs, (void**)&dpCoeffs, filterSize * sizeof(Tw)) != IALG_EOK)
    {
      printf("Convolve2D: Unable to find coefficient pointer\n");
    }
    needsCopyCoeffs = true;
  }

  // Accumulator (output) buffer
  if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accPtr, (void**)&dpAcc, sizeof(Tacc) * outSize) != IALG_EOK)
  {
    printf("Convolve2D: Unable to find accumulator pointer\n");
  }

  if(enableBias && biasSize > 0)
  {
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pBias, (void**)&dpBias, sizeof(Tb) * biasSize) != IALG_EOK)
    {
      if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pBias, (void**)&dpBias, sizeof(Tb) * biasSize) != IALG_EOK)
      {
        printf("Convolve2D: Unable to find bias pointer\n");
      }
    }
  }

  // ---- CUDA Streams Optimization ----------------------------------------
  cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

  checkCudaErr(cudaMemsetAsync(dpAcc, 0, sizeof(Tacc) * outSize, stream));

  checkCudaErr(cudaMemcpyAsync(dpIn, pInChannelOriginal,
                                sizeof(Tin) * inTxSize,
                                cudaMemcpyHostToDevice, stream));

  if(needsCopyCoeffs)
    checkCudaErr(cudaMemcpyAsync(dpCoeffs, pCoeffs,
                                  filterSize * sizeof(Tw),
                                  cudaMemcpyHostToDevice, stream));

  if(enableBias && biasSize > 0 && dpBias != NULL)
    checkCudaErr(cudaMemcpyAsync(dpBias, pBias,
                                  sizeof(Tb) * biasSize,
                                  cudaMemcpyHostToDevice, stream));


  launchSize = numTotRoi * numGroups * numOutChannels * outWidth * outHeight;

  if(coeffsWidth == 1 && coeffsHeight == 1)
  {
    // Optimized 1x1 pointwise convolution kernel
    TIDL_cudaConvKernel1x1<<<GRID_SIZE(launchSize, THREADS_PER_BLOCK),THREADS_PER_BLOCK, 0, stream>>>(
                                                                          dpIn - inputOffset,
                                                                          dpCoeffs,
                                                                          dpBias,
                                                                          dpAcc,
                                                                          min,
                                                                          max,
                                                                          numTotRoi,
                                                                          numGroups,
                                                                          numInChannels,
                                                                          numOutChannels,
                                                                          inChPitch,
                                                                          outChPitch,
                                                                          outWidth,
                                                                          outHeight,
                                                                          inImPitch,
                                                                          outImPitch,
                                                                          strideWidth,
                                                                          strideHeight,
                                                                          enableBias,
                                                                          isOTFpad,
                                                                          leftPad,
                                                                          topPad,
                                                                          padVal,
                                                                          inHeight,
                                                                          inWidth
                                                                         );
  }
  else if(coeffsWidth == 3 && coeffsHeight == 3)
  {
    // Optimized 3x3 convolution kernel with shared memory for coefficients
    TIDL_cudaConvKernel3x3<<<GRID_SIZE(launchSize, THREADS_PER_BLOCK),THREADS_PER_BLOCK, 0, stream>>>(
                                                                          dpIn - inputOffset,
                                                                          dpCoeffs,
                                                                          dpBias,
                                                                          dpAcc,
                                                                          min,
                                                                          max,
                                                                          numTotRoi,
                                                                          numGroups,
                                                                          numInChannels,
                                                                          numOutChannels,
                                                                          inChPitch,
                                                                          outChPitch,
                                                                          outWidth,
                                                                          outHeight,
                                                                          inImPitch,
                                                                          outImPitch,
                                                                          dilationWidth,
                                                                          dilationHeight,
                                                                          strideWidth,
                                                                          strideHeight,
                                                                          enableBias,
                                                                          isOTFpad,
                                                                          leftPad,
                                                                          topPad,
                                                                          padVal,
                                                                          inHeight,
                                                                          inWidth
                                                                         );
  }
  else
  {
    // General kernel for arbitrary filter sizes (fallback)
    TIDL_cudaConvKernel<<<GRID_SIZE(launchSize, THREADS_PER_BLOCK),THREADS_PER_BLOCK, 0, stream>>>(
                                                                          dpIn - inputOffset,
                                                                          dpCoeffs,
                                                                          dpBias,
                                                                          dpAcc,
                                                                          min,
                                                                          max,
                                                                          numTotRoi,
                                                                          numGroups,
                                                                          numInChannels,
                                                                          numOutChannels,
                                                                          inChPitch,
                                                                          outChPitch,
                                                                          outWidth,
                                                                          outHeight,
                                                                          inImPitch,
                                                                          outImPitch,
                                                                          coeffsWidth,
                                                                          coeffsHeight,
                                                                          dilationWidth,
                                                                          dilationHeight,
                                                                          strideWidth,
                                                                          strideHeight,
                                                                          enableBias,
                                                                          isOTFpad,
                                                                          leftPad,
                                                                          topPad,
                                                                          padVal,
                                                                          startRowNumberInTensor,
                                                                          inHeight,
                                                                          inWidth
                                                                         );
  }

  /* --- Per-stream sync – does not stall unrelated GPU work ---------------- */
  checkCudaErr(cudaStreamSynchronize(stream));
  /* --- Cleanup (only when handles were created locally) ------------------- */

  return 0;
}


//Instantiations:

template int TIDL_cudaConvolve2d<signed char, signed char, int, int>(signed char*, signed char*, int*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned char, signed char, int, int>(unsigned char*, signed char*, int*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned char, short, int, int>(unsigned char*, short*, int*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<signed char, short, int, int>(signed char*, short*, int*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<signed char, signed char, signed short, int>(signed char*, signed char*, signed short*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned char, signed char, signed short, int>(unsigned char*, signed char*, signed short*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned char, short, signed short, int>(unsigned char*, short*, signed short*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<signed char, short, signed short, int>(signed char*, short*, signed short*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned char, short, int, long>(unsigned char*, short*, int*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<float, float, float, float>(float*, float*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<signed char, short, int, long>(signed char*, short*, int*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned short, signed char, short, int>(unsigned short*, signed char*, short*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned short, short, int, long>(unsigned short*, short*, int*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned short, signed char, int, int>(unsigned short*, signed char*, int*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<short, signed char, int, int>(short*, signed char*, int*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<short, short, int, long>(short*, short*, int*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<short, signed char, short, int>(short*, signed char*, short*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<signed char, short, long, long>(signed char*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned short, short, long, long>(unsigned short*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<short, short, long, long>(short*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned char, short, long, long>(unsigned char*, short*, long*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<bfloat16_tidl, bfloat16_tidl, float, float>(bfloat16_tidl*, bfloat16_tidl*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned short, bfloat16_tidl, float, float>(unsigned short*, bfloat16_tidl*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<short, bfloat16_tidl, float, float>(short*, bfloat16_tidl*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<unsigned char, bfloat16_tidl, float, float>(unsigned char*, bfloat16_tidl*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaConvolve2d<signed char, bfloat16_tidl, float, float>(signed char*, bfloat16_tidl*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);


template int TIDL_cudaSaturateFixedPoint<float, float>(float*, float*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int,int,  int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<float, short>(float*, short*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int,int,  int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<float, unsigned short>(float*, unsigned short*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int,int,  int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<float, signed char>(float*, signed char*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int,int,  int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<float, unsigned char>(float*, unsigned char*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int,int,  int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<int, unsigned short>(int*, unsigned short*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<long, unsigned short>(long*, unsigned short*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<long, signed char>(long*, signed char*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<int, unsigned char>(int*, unsigned char*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<long, short>(long*, short*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<int, short>(int*, short*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<int, signed char>(int*, signed char*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<long, unsigned char>(long*, unsigned char*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);
template int TIDL_cudaSaturateFixedPoint<float, bfloat16_tidl>(float*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*, int, int, int, int, int, int, int, int, sTIDL_Layer_t*);

template int TIDL_cudaConv2DSaturateFloat<float, float>(float*, float*, int, int, int, int, int, int, int, int, int, float , int, int, int, sTIDL_Layer_t *);
template int TIDL_cudaConv2DSaturateFloat<float, bfloat16_tidl>(float *, bfloat16_tidl *, int, int, int, int, int, int, int, int, int, float , int, int, int, sTIDL_Layer_t *);