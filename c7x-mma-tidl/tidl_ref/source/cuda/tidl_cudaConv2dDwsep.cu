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

/* External declaration of the global memory manager pointer */
extern TIDL_CudaMemManager* g_cudaMemManager;

// Define saturation type enum
enum SaturationType {
    FIXED_POINT_SAT = 0,
    FLOAT_SAT = 1,
    FIXED_POINT_ASYM_SAT = 2
};

typedef struct
{
  int  isInit;
  void *dpIn;
  void *dpBias;
  void *dpCoeffs;
  void *dpAcc;
  void *devPtrOf;
  void *dRoundBits;
  void *dDerivedScales;
  void *dDerivedShifts;
}TIDL_cudaCV;
static TIDL_cudaCV CUDACV[MEM_BUFF_ARRAY_LEN] = {0};


/**
----------------------------------------------------------------------------
@fn         TIDL_cudaFreeConvCudaPtrs
@brief      Function frees device pointers aswell as resets the initalisation
flag

@remarks    None
----------------------------------------------------------------------------
*/
void TIDL_cudaFreeConvCudaPtrs()
{
  for(int i = 0; i < MEM_BUFF_ARRAY_LEN; i++)
  {
     if(CUDACV[i].dpIn) cudaFree (CUDACV[i].dpIn);
     if(CUDACV[i].dpBias) cudaFree (CUDACV[i].dpBias);
     if(CUDACV[i].dpCoeffs) cudaFree (CUDACV[i].dpCoeffs);
     if(CUDACV[i].dpAcc) cudaFree (CUDACV[i].dpAcc);
     if(CUDACV[i].devPtrOf) cudaFree (CUDACV[i].devPtrOf);
     if(CUDACV[i].dRoundBits) cudaFree (CUDACV[i].dRoundBits);
     if(CUDACV[i].dDerivedScales) cudaFree (CUDACV[i].dDerivedScales);
     if(CUDACV[i].dDerivedShifts) cudaFree (CUDACV[i].dDerivedShifts);
     
    CUDACV[i].devPtrOf = 0; 
    CUDACV[i].dpIn = 0;
    CUDACV[i].dpBias = 0;
    CUDACV[i].dpCoeffs = 0;
    CUDACV[i].dpAcc = 0;
    CUDACV[i].dRoundBits = 0;
    CUDACV[i].dDerivedShifts = 0;
    CUDACV[i].dDerivedScales = 0;
    CUDACV[i].isInit = 0 ;
  }
  cudaDeviceSynchronize();
}

//CUDA KERNELS:

template <class Tout>
__global__ void TIDL_convFillZeroPoint(Tout *Y, int32_t bufferSize, int32_t padFillValue)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if(i < (bufferSize))
  {
    Y[i] = (Tout)padFillValue;
  }
}


/**
----------------------------------------------------------------------------
@fn         TIDL_convSaturateCuda
@brief      Cuda kernel for performing saturation (FIXED POINT)

@param      X : Device pointer to a stream of floats
@param      Y : Device pointer to a stream of input integers
@param      N : Number of batches
@param      C : Number of output channels
@param      H : Output Height
@param      W : Output Width
@param      outChPitch : Output channel pitch
@param      outImPitch : Output image pitch
@param      outRoundBits : Number of rounding bits
@param      satLow : Saturation lower limit
@param      satHigh : Saturation upper limit

@remarks    None
----------------------------------------------------------------------------
*/
template <class Tacc,class Tout>
__global__ void TIDL_convSaturateCuda(Tacc *X, Tout *Y, int N, int C, int H, int W, int outChPitch, int outImPitch, uint8_t *outRoundBits, int32_t satLow, int32_t satHigh, int32_t precisionAdjustmentShift)
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
    outAcc = X[nc*(C*outChPitch)+cc*(outChPitch)+hc*(outImPitch)+wc];
    if(outRoundBits[cc] != 0)
    {
      outAcc += (1 << (outRoundBits[cc] - 1));
      outAcc >>= outRoundBits[cc];
    }
    outAcc >>= precisionAdjustmentShift;
    outAcc = outAcc < satLow ? satLow : outAcc;
    outAcc = outAcc > satHigh ? satHigh : outAcc;
    Y[nc*(C*outChPitch)+cc*(outChPitch)+hc*(outImPitch)+wc] = outAcc;
  }
}


template <class Tacc,class Tout, class Tin>
__global__ void TIDL_convSaturateCudaV2(Tacc *X, Tout *Y, int N, int C, int H, int W, int outChPitch, int outImPitch, int32_t satLow, int32_t satHigh, uint8_t* pDerivedScales, uint8_t* pDerivedShifts)
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
    outAcc = X[nc*(C*outChPitch)+cc*(outChPitch)+hc*(outImPitch)+wc];
    outAcc *= pDerivedScales[cc];
    if(pDerivedShifts[cc] != 0)
    {
      outAcc += (1 << (pDerivedShifts[cc] - 1));
      outAcc >>= pDerivedShifts[cc];
    } 
    //No overflow detection.. 
    //if(typeid(Tin) == typeid(int8_t) || typeid(Tin) == typeid(uint8_t))
    {
     // outAcc = ((int64_t)outAcc & (int64_t)0xFFFFFFFFFF);
    }
    outAcc = outAcc < satLow ? satLow : outAcc;
    outAcc = outAcc > satHigh ? satHigh : outAcc;
    Y[nc*(C*outChPitch)+cc*(outChPitch)+hc*(outImPitch)+wc] = outAcc;
  }
}


template <class Tacc,class Tout>
__global__ void TIDL_convSaturateCudaFloat(Tacc *X, Tout *Y, int N, int C, int H, int W, int outChPitch, int outImPitch, int outRoundBits, Tacc satLow, Tacc satHigh)
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int nc, cc, hc, wc, k;
  Tacc outAcc;
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
    outAcc = X[nc*(C*outChPitch)+cc*(outChPitch)+hc*(outImPitch)+wc];
    outAcc = outAcc < satLow ? satLow : outAcc;
    outAcc = outAcc > satHigh ? satHigh : outAcc;
    Y[nc*(C*outChPitch)+cc*(outChPitch)+hc*(outImPitch)+wc] = outAcc;
  }
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
  int32_t inWidth
  )
{
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  int32_t   i0, i2, i3, i4, i5, i6, i7, i8, k;
  int32_t   coeffOffset, indataOffset, outdataOffset;
  int32_t validPosXMin,validPosXMax,validPosYMin,validPosYMax;
  int32_t spatialOffsetY, spatialOffsetX;
  int32_t isBorderPixel;
  validPosXMin = leftPad;
  validPosXMax = leftPad + inWidth;
  validPosYMin = topPad;
  validPosYMax = topPad + inHeight;
  Tacc      outAcc;
  Tin       inData;
  Tw        coefData;
  Tb        biasData;
  if(i <= numTotRoi*numGroups*numOutChannels*width*height)
  {
    k = i;
    i8 = k / (numGroups*numOutChannels*width*height);
    k = k % (numGroups*numOutChannels*width*height);
    i7 = k / (numOutChannels*width*height);
    k = k % (numOutChannels*width*height);
    i6 = k / (width*height);
    k = k % (width*height);
    i2 = k / (width);
    i3 = k % (width);
    indataOffset = i7*numInChannels*inChPitch + i8*numGroups*numInChannels*inChPitch;
    outdataOffset = i7*numOutChannels*outChPitch + i8*numGroups*numOutChannels*outChPitch;
    if(enableBias)
    {
      biasData = pBias[i7*numOutChannels + i6];
    }
    else
    {
      biasData = 0;
    }

    outAcc = biasData; 
    for (i0 = 0; i0 < numInChannels; i0++)
    {
      coeffOffset = i7*numInChannels * coeffsHeight * coeffsWidth *numOutChannels + \
      i6* numInChannels * coeffsHeight * coeffsWidth + i0 * coeffsHeight * coeffsWidth;
      for (i4 = 0; i4 < coeffsHeight; i4++)
      {
        for (i5 = 0; i5 < coeffsWidth; i5++)
        {
          spatialOffsetY = (i2 * strideHeight) + (i4 * dilationHeight);
          spatialOffsetX = (i3 * strideWidth) + (i5 * dilationWidth);
          isBorderPixel  =  (((spatialOffsetY < validPosYMin)||(spatialOffsetY >= validPosYMax))) || (((spatialOffsetX < validPosXMin)||(spatialOffsetX >= validPosXMax)));
          if(isOTFpad & isBorderPixel)
          {
             inData = padVal;          
          }
          else
          {
            inData = pInChannel[indataOffset + i0* inChPitch + ((i2*strideHeight)* inImPitch) + i3*strideWidth +
                             (i4 * inImPitch*dilationHeight) + i5*dilationWidth];
          }
          coefData = pCoeffs[coeffOffset + (i4 * coeffsWidth) + i5];
          outAcc += (inData * coefData);
        }
      }
    }
    accPtr[outdataOffset + i6 * outChPitch + ((i2 ) * outImPitch) + (i3 )] = outAcc;
  }
}

/**
----------------------------------------------------------------------------
@fn         TIDL_cudaSaturateV1
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

@remarks    None
----------------------------------------------------------------------------
*/
template <class Tacc, class Tout>
int TIDL_cudaSaturateV1(Tout **devPtrOf, int N, int C, int H, int W, int outChPitch, int outImPitch, int outRoundBits, Tacc satLow, Tacc satHigh)
{
  int sizeOstream = N * C * outChPitch;
  if(CUDACV[CUDNNLC].isInit == 0)
  {
    cudaMalloc((void**)&CUDACV[CUDNNLC].devPtrOf, sizeOstream * sizeof(Tout));
    cudaMemset (CUDACV[CUDNNLC].devPtrOf, 0, sizeof(Tout) * sizeOstream);
    CUDACV[CUDNNLC].isInit = 1;
  }
  *devPtrOf = (Tout*)CUDACV[CUDNNLC].devPtrOf;
  
  //Float Always for this function
  TIDL_convSaturateCudaFloat<<<((sizeOstream) / THREADS_PER_BLOCK) + 1,THREADS_PER_BLOCK>>>((Tacc*)CUDACV[CUDNNLC].dpAcc, *devPtrOf, N, C, H, W, outChPitch, outImPitch, outRoundBits, satLow, satHigh);

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
int TIDL_cudaSaturateFixedPoint(Tout **devPtrOf, int N, int C, int H, int W, int outChPitch, int outImPitch, uint8_t *outRoundBits, int32_t satLow, int32_t satHigh, int32_t enablePerChannelShift, int32_t precisionAdjustmentShift)
{
  int sizeOstream = N * C * outChPitch;
  int sizeRoundBits = N * C;
  if(CUDACV[CUDNNLC].isInit == 0)
  {
    cudaMalloc((void**)&CUDACV[CUDNNLC].devPtrOf, sizeOstream * sizeof(Tout) * 2);
    cudaMemset (CUDACV[CUDNNLC].devPtrOf, 0, sizeof(Tout) * sizeOstream);
    cudaMalloc((void**)&CUDACV[CUDNNLC].dRoundBits, sizeRoundBits * sizeof(uint8_t) * 2);
    CUDACV[CUDNNLC].isInit = 1;
  }
  if (enablePerChannelShift)
  {
    checkCudaErr(cudaMemcpy (CUDACV[CUDNNLC].dRoundBits, outRoundBits, sizeof(uint8_t) * sizeRoundBits, cudaMemcpyHostToDevice));
  }
  else
  {
    cudaMemset (CUDACV[CUDNNLC].dRoundBits, outRoundBits[0], sizeof(uint8_t) * sizeRoundBits);
  }
  *devPtrOf = (Tout*)CUDACV[CUDNNLC].devPtrOf;
  

  TIDL_convSaturateCuda<Tacc,Tout><<<((sizeOstream) / THREADS_PER_BLOCK) + 1,THREADS_PER_BLOCK>>>((Tacc*)CUDACV[CUDNNLC].dpAcc, *devPtrOf, N, C, H, W, outChPitch, outImPitch, (uint8_t *)CUDACV[CUDNNLC].dRoundBits, satLow, satHigh, precisionAdjustmentShift);
  
  return 0;
}


#if 1
template <class Tacc, class Tout, class Tin>
int TIDL_cudaSaturateFixedPointAsym(Tout **devPtrOf, int N, int C, int H, int W, int outChPitch, int outImPitch, int32_t satLow, int32_t satHigh, uint8_t* pDerivedScales, uint8_t* pDerivedShifts, int32_t padFillValue)
{
  int sizeOstream = N * C * outChPitch;
  int sizeRoundBits = N * C;
  if(CUDACV[CUDNNLC].isInit == 0)
  {
    cudaMalloc((void**)&CUDACV[CUDNNLC].devPtrOf, sizeOstream * sizeof(Tout) * 2);
    cudaMemset (CUDACV[CUDNNLC].devPtrOf, 0, sizeof(Tout) * sizeOstream);
    //cudaMalloc((void**)&CUDACV[CUDNNLC].dRoundBits, sizeRoundBits * sizeof(uint8_t) * 2);
    cudaMalloc((void**)&CUDACV[CUDNNLC].dDerivedScales, sizeRoundBits * sizeof(uint8_t) * 2);
    cudaMalloc((void**)&CUDACV[CUDNNLC].dDerivedShifts, sizeRoundBits * sizeof(uint8_t) * 2);
    CUDACV[CUDNNLC].isInit = 1;
  }

  checkCudaErr(cudaMemcpy (CUDACV[CUDNNLC].dDerivedScales, pDerivedScales, sizeof(uint8_t) * sizeRoundBits, cudaMemcpyHostToDevice));
  checkCudaErr(cudaMemcpy (CUDACV[CUDNNLC].dDerivedShifts, pDerivedShifts, sizeof(uint8_t) * sizeRoundBits, cudaMemcpyHostToDevice));

  *devPtrOf = (Tout*)CUDACV[CUDNNLC].devPtrOf;
  
  TIDL_convFillZeroPoint<Tout><<<((sizeOstream) / THREADS_PER_BLOCK) + 1,THREADS_PER_BLOCK>>>(*devPtrOf, sizeOstream, padFillValue);
  TIDL_convSaturateCudaV2<Tacc,Tout,Tin><<<((sizeOstream) / THREADS_PER_BLOCK) + 1,THREADS_PER_BLOCK>>>((Tacc*)CUDACV[CUDNNLC].dpAcc, *devPtrOf, N, C, H, W, outChPitch, outImPitch, satLow, satHigh, (uint8_t*)CUDACV[CUDNNLC].dDerivedScales, (uint8_t*)CUDACV[CUDNNLC].dDerivedShifts);
  //TIDL_convSaturateCuda<Tacc,Tout><<<((sizeOstream) / THREADS_PER_BLOCK) + 1,THREADS_PER_BLOCK>>>((Tacc*)CUDACV[CUDNNLC].dpAcc, *devPtrOf, N, C, H, W, outChPitch, outImPitch, (uint8_t *)CUDACV[CUDNNLC].dRoundBits, satLow, satHigh, precisionAdjustmentShift);
  
  return 0;
}
#endif

template <>
int TIDL_cudaSaturateFixedPoint<float, float>(float **devPtrOf, int N, int C, int H, int W, int outChPitch, int outImPitch, uint8_t *outRoundBits, int32_t satLow, int32_t satHigh, int32_t enablePerChannelShift, int32_t precisionAdjustmentShift)
{

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
  long int  launchSize;
  Tin      *dpIn;
  Tw       *dpCoeffs;
  Tb       *dpBias;
  Tacc     *dpAcc;
  int inSize = numTotRoi * numGroups * numInChannels * inChPitch;
  int inTxSize = inSize;
  int inputOffset = 0;
  /*For OTF, the buffer is actually larger by a factor of topPad*/
  if(isOTFpad)
  {
    inSize += numTotRoi * numGroups * numInChannels * (topPad * inWidth);
    pInChannel = pInChannel - (topPad * inImPitch + leftPad);
    inputOffset = topPad * inImPitch + leftPad;
  }

  int outSize = numTotRoi * numGroups * numOutChannels * outChPitch;
  int filterSize = numOutChannels * numGroups * numInChannels * coeffsHeight * coeffsWidth;
  int biasSize = numTotRoi * numGroups * numOutChannels;

  if(!CUDACV[CUDNNLC].isInit)
  {
    checkCudaErr(cudaMalloc((void**)&(CUDACV[CUDNNLC].dpIn), (inSize) * sizeof(Tin) * 2));
    checkCudaErr(cudaMalloc((void**)&(CUDACV[CUDNNLC].dpAcc), (outSize) * sizeof(Tacc) * 2));
    cudaMemset(CUDACV[CUDNNLC].dpAcc, 0, sizeof(Tacc) * (outSize)); //Needs to be done always.. for min-max?
    checkCudaErr(cudaMalloc((void**)&(CUDACV[CUDNNLC].dpCoeffs), (filterSize) * sizeof(Tw) * 2));
    checkCudaErr(cudaMalloc((void**)&(CUDACV[CUDNNLC].dpBias), (((biasSize + 63)/64) * 64) * sizeof(Tb) * 2));
  }
  /*Grouped convolution handling in WL repeats layers, so forcing memcpy each frame..*/
  checkCudaErr(cudaMemcpy(CUDACV[CUDNNLC].dpCoeffs, pCoeffs, sizeof(Tw) * filterSize, cudaMemcpyHostToDevice));
  dpIn = (Tin*)CUDACV[CUDNNLC].dpIn;
  dpCoeffs = (Tw*)CUDACV[CUDNNLC].dpCoeffs;
  dpAcc = (Tacc*)CUDACV[CUDNNLC].dpAcc;
  dpBias = (Tb*)CUDACV[CUDNNLC].dpBias;
  checkCudaErr(cudaMemcpy((dpIn + inputOffset), (pInChannel + inputOffset), sizeof(Tin) * inTxSize, cudaMemcpyHostToDevice));
  if(biasSize > 0)
  {
    checkCudaErr(cudaMemcpy(dpBias, pBias, sizeof(Tb) * biasSize, cudaMemcpyHostToDevice));
  }
  launchSize = numTotRoi * numGroups * numOutChannels * outWidth * outHeight;

TIDL_cudaConvKernel<<<(launchSize / THREADS_PER_BLOCK) + 1,THREADS_PER_BLOCK>>>(
                                                                          dpIn,
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
  checkCudaErr(cudaDeviceSynchronize());
  checkCudaErr(cudaMemcpy(accPtr, dpAcc, sizeof(Tacc) * outSize, cudaMemcpyDeviceToHost));
  minmax_Thrust(outSize, dpAcc, min, max);
  return 0;
}

void TIDL_cudaSetInitFlag(int32_t layerIdx)
{
  CUDACV[layerIdx].isInit = 1;
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
template int TIDL_cudaSaturateV1<int, unsigned short>(unsigned short**, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSaturateV1<int, short>(short**, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSaturateV1<float, short>(short**, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaSaturateV1<float, float>(float**, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaSaturateV1<float, unsigned short>(unsigned short**, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaSaturateV1<int, unsigned char>(unsigned char**, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSaturateV1<int, signed char>(signed char**, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSaturateV1<long, unsigned char>(unsigned char**, int, int, int, int, int, int, int, long, long);
template int TIDL_cudaSaturateV1<long, signed char>(signed char**, int, int, int, int, int, int, int, long, long);
template int TIDL_cudaSaturateV1<int, float>(float**, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSaturateV1<float, unsigned char>(unsigned char**, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaSaturateV1<float, signed char>(signed char**, int, int, int, int, int, int, int, float, float);

template int TIDL_cudaSaturateFixedPoint<int, short>(short**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<int, unsigned char>(unsigned char**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<int, signed char>(signed char**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<long, short>(short**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<int, unsigned short>(unsigned short**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<long, signed char>(signed char**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<long, unsigned short>(unsigned short**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<long, unsigned char>(unsigned char**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPoint<float, float>(float**, int, int, int, int, int, int, unsigned char *, int, int, int, int);
template int TIDL_cudaSaturateFixedPointAsym<int, signed char, signed char>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned char, unsigned short>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, signed char, short>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned short, unsigned short>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, short, unsigned short>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, signed char, unsigned short>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned char, signed char>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, short, signed char>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned short, signed char>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned char, unsigned char>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<float, float, float>(float**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, signed char, unsigned char>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, short, unsigned char>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned short, unsigned char>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, signed char, short>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned char, short>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned short, short>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, short, short>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, signed char, unsigned short>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned char, unsigned short>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, short, unsigned short>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<int, unsigned short, unsigned short>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, signed char, signed char>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned char, signed char>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, short, signed char>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned short, signed char>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, short, unsigned char>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned short, unsigned char>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned short, short>(unsigned short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, signed char, unsigned char>(signed char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned char, unsigned char>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, unsigned char, short>(unsigned char**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
template int TIDL_cudaSaturateFixedPointAsym<long, short, short>(short**, int, int, int, int, int, int, int, int, unsigned char*, unsigned char*,int);
