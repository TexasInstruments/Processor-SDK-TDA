/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaResize.cu
 @brief   This file contains CUDA implementation of Resize layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include <type_traits>
#include "tidl_cuda_mem_manager.h"


template <class T>
__global__ void ResizePadZeroKernel(
    const T* __restrict__ pIn,
    T* __restrict__ pOut,
    int numBatches, int numInChannels, int inHeight, int inWidth,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    float resizeRatioH, float resizeRatioW, int resizePadZeroOffset, int padH, int padW)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numInChannels * inHeight * inWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 4D coordinates (batch, channel, height, width)
    int i4 = idx % inWidth;                                           // width
    int i3 = (idx / inWidth) % inHeight;                              // height
    int i2 = (idx / (inWidth * inHeight)) % numInChannels;            // channel
    int i1 = idx / (inWidth * inHeight * numInChannels);              // batch

    pIn += (padH * inPitch) + padW;

    int32_t inputOffset = (i1 * inBatchPitch) + (i2 * inChPitch) + (i3 * inPitch) + i4;
    int32_t outputOffset = (i1 * outBatchPitch) + (i2 * outChPitch) + (i3 * resizeRatioH + resizePadZeroOffset) * outPitch + (i4 * resizeRatioW + resizePadZeroOffset);

    pOut[outputOffset] = pIn[inputOffset];
}

template <class T>
__global__ void ResizeNearestKernel(
    const T* __restrict__ pIn,
    T* __restrict__ pOut,
    int numBatches, int numInChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    float hRatio, float wRatio, int inOffset, int outOffset
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numInChannels * outHeight * outWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 4D coordinates (batch, channel, outHeight, outWidth)
    int j = idx % outWidth;                                           // output width
    int i = (idx / outWidth) % outHeight;                             // output height
    int k = (idx / (outWidth * outHeight)) % numInChannels;           // channel
    int l = idx / (outWidth * outHeight * numInChannels);             // batch

    // Compute source coordinates - exactly matching reference implementation
    float hLoc = (hRatio * ((float)i + 0.5f)) - 0.5f;
    hLoc = (hLoc < 0.0f) ? 0.0f : hLoc;
    int hIdx = (int)(hLoc + 0.5f);
    hIdx = (hIdx < inHeight) ? hIdx : (inHeight - 1);

    float wLoc = (wRatio * ((float)j + 0.5f)) - 0.5f;
    wLoc = (wLoc < 0.0f) ? 0.0f : wLoc;
    int wIdx = (int)(wLoc + 0.5f);
    wIdx = (wIdx < inWidth) ? wIdx : (inWidth - 1);

    int inputOffset = inOffset + (inPitch * hIdx) + wIdx;
    int outputOffset = outOffset + (outPitch * i) + j;

    pOut[(l * outBatchPitch) + (k * outChPitch) + outputOffset] = pIn[(l * inBatchPitch) + (k * inChPitch) + inputOffset];
}

template <class T>
__global__ void ResizeBilinearKernel(
    const T* __restrict__ pIn,
    T* __restrict__ pOut,
    int numBatches, int numInChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    float hRatio, float wRatio, int inOffset, int outOffset)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numInChannels * outHeight * outWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 4D coordinates (batch, channel, outHeight, outWidth)
    int j = idx % outWidth;                                           // output width
    int i = (idx / outWidth) % outHeight;                             // output height
    int k = (idx / (outWidth * outHeight)) % numInChannels;           // channel
    int l = idx / (outWidth * outHeight * numInChannels);             // batch

    // Compute bilinear interpolation - exactly matching reference
    float hLoc = (hRatio * ((float)i + 0.5f)) - 0.5f;
    int hIdx;
    if (hLoc < 0.0f)
    {
        hIdx = (int)floorf(hLoc);
    }
    else
    {
        hIdx = (int)hLoc;
    }
    int hNext = 1;
    float w11 = hLoc - (float)hIdx;
    float w10 = 1.0f - w11;

    float wLoc = (wRatio * ((float)j + 0.5f)) - 0.5f;
    wLoc = (wLoc < 0.0f) ? 0.0f : wLoc;
    int wIdx = (int)wLoc;
    float w01 = wLoc - (float)wIdx;
    float w00 = 1.0f - w01;

    int inputOffset = (inPitch * hIdx) + wIdx;
    int inputOffset2 = inputOffset;
    int outputOffset = outOffset + (outPitch * i) + j;

    int wNext = (wIdx < (inWidth - 1)) ? 1 : 0;
    
    // Handle negative inputOffset (top padding region)
    if (inputOffset < 0)
    {
        inputOffset2 = wIdx;
        hNext = 0;
    }

    int baseOffset = inOffset + (l * inBatchPitch) + (k * inChPitch);
    
    T i00 = pIn[baseOffset + inputOffset];
    T i01 = pIn[baseOffset + inputOffset + wNext];
    T i10 = pIn[baseOffset + inputOffset2 + (hNext * inPitch)];
    T i11 = pIn[baseOffset + inputOffset2 + (hNext * inPitch) + wNext];

#ifdef HOST_EMULATION
    uint32_t widthScale, heightScale, shift;
    uint8_t w00Int, w01Int;
    uint8_t w10Int, w11Int;
    if((hRatio == wRatio) && ((hRatio == 0.5f) || (hRatio == 0.25f)))
    {
        if(hRatio == 0.25f)
        {
            widthScale = 8;
            heightScale = 8;
            shift = 6;
        }
        else
        {
            widthScale = 4;
            heightScale = 4;
            shift = 4;
        }
        w00Int = w00 * widthScale;
        w01Int = widthScale - w00Int;
        w10Int = w10 * heightScale;
        w11Int = heightScale - w10Int;
        T result = (w10Int * (uint32_t)((i00 * w00Int) + (i01 * w01Int)) + w11Int * (uint32_t)((i10 * w00Int) + (i11 * w01Int))) >> shift;
        pOut[(l * outBatchPitch) + (k * outChPitch) + outputOffset] = result;
    }
    else
#endif
    {
        // Bilinear interpolation formula
        T result = (T)((w10 * ((float)i00 * w00 + (float)i01 * w01)) + (w11 * ((float)i10 * w00 + (float)i11 * w01)));
        pOut[(l * outBatchPitch) + (k * outChPitch) + outputOffset] = result;
    }
}

template <class T>
__global__ void ResizeBilinearFloatKernel(
    const T* __restrict__ pIn,
    T* __restrict__ pOut,
    int numBatches, int numInChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    float hRatio, float wRatio, int inOffset, int outOffset)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numInChannels * outHeight * outWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 4D coordinates (batch, channel, outHeight, outWidth)
    int j = idx % outWidth;                                           // output width
    int i = (idx / outWidth) % outHeight;                             // output height
    int k = (idx / (outWidth * outHeight)) % numInChannels;           // channel
    int l = idx / (outWidth * outHeight * numInChannels);  

    float hLoc = (hRatio * ((float)i + 0.5f)) - 0.5f;
    hLoc = (hLoc < 0.0f) ? 0.0f : hLoc;
    int hIdx = hLoc;
    int hNext = (hIdx < (inHeight-1U)) ? 1U : 0U;
    T w11 = (T)(hLoc - (float32_tidl)hIdx);
    T w10 = (T)1.0f - w11;

    float wLoc = (wRatio * ((float)j + 0.5f)) - 0.5f;
    wLoc = (wLoc < 0.0f) ? 0.0f : wLoc;
    int wIdx = wLoc;
    T w01 = (T)(wLoc - (float32_tidl)wIdx);
    T w00 = (T)1.0f - w01;
    int wNext = (wIdx < (inWidth-1U)) ? 1U : 0U;

    int32_t inputOffset = inOffset + (inPitch  * hIdx) + wIdx;
    int32_t outputOffset = outOffset + (outPitch * (i)) + (j);

    float i00 = *(pIn + (l*inBatchPitch) + (k*inChPitch) + inputOffset);
    float i01 = *(pIn + (l*inBatchPitch) + (k*inChPitch) + inputOffset + wNext);
    float i10 = *(pIn + (l*inBatchPitch) + (k*inChPitch) + inputOffset + (hNext*inPitch));
    float i11 = *(pIn + (l*inBatchPitch) + (k*inChPitch) + inputOffset + (hNext*inPitch) + wNext);

    pOut[(l*outBatchPitch) + (k*outChPitch) + (uint32_t)outputOffset] = (((float)w10*((i00* (float)w00) +  (i01* (float)w01))) + 
                                                                         ((float)w11*((i10* (float)w00) +  (i11* (float)w01))));
}

template<class T>
int TIDL_cudaResize(
    const T* input,
    T* output,
    int numBatches, int numInChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    float hRatio, float wRatio, int resizePadZeroOffset, int mode, int leftPadResize, int padHeight, int padWidth, int inOffset, int outOffset, int tensorZeroPoint
)
{
    int32_t status = IALG_EOK;
    T* d_input = NULL;
    T* d_output = NULL;

    size_t input_size = (size_t)numBatches * inBatchPitch * sizeof(T);
    size_t output_size = (size_t)numBatches * outBatchPitch * sizeof(T);

    size_t inElementSize = sizeof(T);
    size_t input_size_with_pad = 1 + (numBatches-1)*inBatchPitch + (numInChannels-1)*inChPitch*inElementSize + (inHeight + 1)*inPitch*inElementSize + (inWidth*inElementSize-1) + leftPadResize*inElementSize;
    int32_t offset = (inPitch + leftPadResize);
    uint8_t* input_base = (uint8_t*)input - offset*inElementSize;

    if(mode == TIDL_ResizeBilinear && !std::is_floating_point<T>::value && !std::is_same<T, bfloat16_tidl>::value)
    {
        input = (T*)input_base;
        input_size = input_size_with_pad;
        inOffset = inOffset + offset;
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("TIDL_cudaResize: Failed to allocate input memory\n");
            return IALG_EFAIL;
        }
    }
    
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("TIDL_cudaResize: Failed to allocate output memory\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    TIDL_convFillZeroPoint<T><<<GRID_SIZE(output_size/sizeof(T), THREADS_PER_BLOCK),THREADS_PER_BLOCK, 0, stream>>>(d_output, output_size/sizeof(T), (T)tensorZeroPoint);

    if(mode == TIDL_ResizePadZero)
    {
        int total_elements = numBatches * numInChannels * inHeight * inWidth;
        int grid_size = GRID_SIZE(total_elements, THREADS_PER_BLOCK);
        ResizePadZeroKernel<T><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(d_input, d_output, numBatches, numInChannels, inHeight, inWidth,
                    inBatchPitch, inChPitch, inPitch, outBatchPitch, outChPitch, outPitch, hRatio, wRatio, resizePadZeroOffset, padHeight, padWidth);
    }
    else if(mode == TIDL_ResizeNearest)
    {
        int total_elements = numBatches * numInChannels * outHeight * outWidth;
        int grid_size = GRID_SIZE(total_elements, THREADS_PER_BLOCK);
        ResizeNearestKernel<T><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(d_input, d_output, numBatches, numInChannels, inHeight, inWidth, outHeight, outWidth,
                    inBatchPitch, inChPitch, inPitch, outBatchPitch, outChPitch, outPitch, 1/hRatio, 1/wRatio, inOffset, outOffset);
    }
    else if(mode == TIDL_ResizeBilinear)
    {
        int total_elements = numBatches * numInChannels * outHeight * outWidth;
        int grid_size = GRID_SIZE(total_elements, THREADS_PER_BLOCK);
        if(std::is_floating_point<T>::value || std::is_same<T, bfloat16_tidl>::value)
        {
            ResizeBilinearFloatKernel<T><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(d_input, d_output, numBatches, numInChannels, inHeight, inWidth, outHeight, outWidth,
                    inBatchPitch, inChPitch, inPitch, outBatchPitch, outChPitch, outPitch, 1/hRatio, 1/wRatio, inOffset, outOffset);
        }
        else
        {
            ResizeBilinearKernel<T><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(d_input, d_output, numBatches, numInChannels, inHeight, inWidth, outHeight, outWidth,
                    inBatchPitch, inChPitch, inPitch, outBatchPitch, outChPitch, outPitch, 1/hRatio, 1/wRatio, inOffset, outOffset);
        }
    }
    else
    {
        printf("params->mode is  Not supported !!!\n ");
        status = IALG_EFAIL;
    }

    // Async D2H – ordered after kernel on same stream
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));
    checkCudaErr(cudaGetLastError());
    return status;

}
 

template int TIDL_cudaResize<short>(short const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, float, float, int, int, int, int, int, int, int, int);
template int TIDL_cudaResize<float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, float, float, int, int, int, int, int, int, int, int);
template int TIDL_cudaResize<unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, float, float, int, int, int, int, int, int, int, int);
template int TIDL_cudaResize<signed char>(signed char const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, float, float, int, int, int, int, int, int, int, int);
template int TIDL_cudaResize<unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, float, float, int, int, int, int, int, int, int, int);
template int TIDL_cudaResize<bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, float, float, int, int, int, int, int, int, int, int);