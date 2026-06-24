/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaEltwise.cu
 @brief   This file contains CUDA implementation of Eltwise layer.
 @version 0.1 (Oct 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include <type_traits>
#include "tidl_cuda_mem_manager.h"

// EltWise Sum kernel - matches reference loop structure exactly (no branching)
template <class Tin, class Tacc>
__global__ void EltWiseSumKernel(
    const Tin* __restrict__ inData,
    Tacc* __restrict__ accumulator,
    int32_t scale,
    int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch, int pixelPitch,
    int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numDIM1 * numDIM2 * numChannels * inHeight * inWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    int d1 = idx / (numDIM2 * numChannels * inHeight * inWidth);
    int d2 = (idx / (numChannels * inHeight * inWidth)) % numDIM2;
    int c = (idx / (inHeight * inWidth)) % numChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    // Calculate offsets - exact same as reference
    uint32_t inOffset = (d1 * inDIM1Pitch) + (d2 * inDIM2Pitch) + (c * inChPitch) + (h * inPitch) + (w * pixelPitch);
    uint32_t outOffset = (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;

    // Sum operation - always accumulate (no branching needed)
    accumulator[outOffset] += ((Tacc)inData[inOffset])*scale;
}

// EltWise Product kernel - matches reference loop structure exactly
template <class Tin, class Tacc>
__global__ void EltWiseProductKernel(
    const Tin* __restrict__ inData,
    Tacc* __restrict__ accumulator,
    int32_t scale,
    int32_t zeropoint,
    int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch, int pixelPitch,
    int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int callno)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numDIM1 * numDIM2 * numChannels * inHeight * inWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    int d1 = idx / (numDIM2 * numChannels * inHeight * inWidth);
    int d2 = (idx / (numChannels * inHeight * inWidth)) % numDIM2;
    int c = (idx / (inHeight * inWidth)) % numChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    // Calculate offsets - exact same as reference
    uint32_t inOffset = (d1 * inDIM1Pitch) + (d2 * inDIM2Pitch) + (c * inChPitch) + (h * inPitch) + (w * pixelPitch);
    uint32_t outOffset = (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;

    if (callno == 0)
    {
        // First call - initialize accumulator
        accumulator[outOffset] = ((inData[inOffset] * scale) - zeropoint);
    } 
    else
    {
        // Subsequent calls - multiply
        accumulator[outOffset] *= ((inData[inOffset] * scale) - zeropoint);
    }
}

// EltWise Mod kernel - matches reference loop structure exactly
template <class Tin, class Tacc>
__global__ void EltWiseModKernel(
    const Tin* __restrict__ inData,
    Tacc* __restrict__ accumulator,
    int32_t scale,
    int32_t zeropoint,
    int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch, int pixelPitch,
    int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch, int fModValue,
    int callno)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numDIM1 * numDIM2 * numChannels * inHeight * inWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    int d1 = idx / (numDIM2 * numChannels * inHeight * inWidth);
    int d2 = (idx / (numChannels * inHeight * inWidth)) % numDIM2;
    int c = (idx / (inHeight * inWidth)) % numChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    // Calculate offsets - exact same as reference
    uint32_t inOffset = (d1 * inDIM1Pitch) + (d2 * inDIM2Pitch) + (c * inChPitch) + (h * inPitch) + (w * pixelPitch);
    uint32_t outOffset = (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;

    if (callno == 0)
    {
        // First call - initialize accumulator
        accumulator[outOffset] = inData[inOffset];
    } 
    else
    {
        // Subsequent calls
        if(fModValue == 1)
        {
            float inAVal = (float)((accumulator[outOffset] - zeropoint) * (1.0 / scale));
            float inBVal = (float)((inData[inOffset] - zeropoint) * (1.0 / scale));
            float outVal = fmod(inAVal, inBVal);
            accumulator[outOffset] = (Tacc)(outVal * scale + zeropoint);
        }
        else
        {
            int32_t inAVal = (int32_t)accumulator[outOffset];
            int32_t inBVal = (int32_t)inData[inOffset];
            if(inBVal != 0)
            {
                int32_t outVal = ((inAVal % inBVal) + inBVal) % inBVal;
                accumulator[outOffset] = (Tacc)(outVal);
            }
        }
    }
}

// EltWise Max kernel - matches reference loop structure exactly
template <class Tin, class Tacc>
__global__ void EltWiseMinMaxKernel(
    const Tin* __restrict__ inData,
    Tacc* __restrict__ accumulator,
    int32_t scale,
    int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch, int pixelPitch,
    int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int callno, int eltWiseType)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numDIM1 * numDIM2 * numChannels * inHeight * inWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    // Note: reference uses different loop order for Max (i4, i5, i1, i2, i3)
    int d1 = idx / (numDIM2 * numChannels * inHeight * inWidth);
    int d2 = (idx / (numChannels * inHeight * inWidth)) % numDIM2;
    int c = (idx / (inHeight * inWidth)) % numChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    // Calculate offsets - exact same as reference
    uint32_t inOffset = (d1 * inDIM1Pitch) + (d2 * inDIM2Pitch) + (c * inChPitch) + (h * inPitch) + (w * pixelPitch);
    uint32_t outOffset = (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;

    if (callno == 0)
    {
        // First call - initialize accumulator
        accumulator[outOffset] = inData[inOffset] * scale;
    }
    else
    {
        // Subsequent calls - take maximum
        Tacc currentVal = accumulator[outOffset];
        Tacc newVal = inData[inOffset] * scale;
        if(eltWiseType == TIDL_EltWiseMin)
        {
            accumulator[outOffset] = min(currentVal, newVal);
        }
        else
        {
            accumulator[outOffset] = max(currentVal, newVal);
    }
}
}

// EltWise MMAv2 Quantization kernel - matches reference loop structure exactly
template <class Tacc, class Tout>
__global__ void TIDL_CudaEltWiseMMAv2QuantizeKernel(
    const Tacc* __restrict__ accumulator,
    Tout* __restrict__ output,
    uint8_t mmaScale,
    uint8_t mmaShift,
    int32_t biasTerm,
    int numBatches, int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numDIM1 * numDIM2 * numChannels * inHeight * inWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (numDIM1 * numDIM2 * numChannels * inHeight * inWidth);
    int d1 = (idx / (numDIM2 * numChannels * inHeight * inWidth)) % numDIM1;
    int d2 = (idx / (numChannels * inHeight * inWidth)) % numDIM2;
    int c = (idx / (inHeight * inWidth)) % numChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    uint32_t outOffset = (b * outBatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;

    if (!std::is_floating_point<Tout>::value)
    {
        Tacc mmaAcc = accumulator[outOffset] + biasTerm;
        Tacc tempAcc = mmaAcc * mmaScale;
        output[outOffset] = (Tout)cuda_roundSat(tempAcc, mmaShift, cuda::std::numeric_limits<Tout>::lowest(), cuda::std::numeric_limits<Tout>::max());
    }
    else
    {
        // Passthrough for float
        output[outOffset] = accumulator[outOffset];
    }
}

// EltWise Quantization kernel - matches reference loop structure exactly
template <class Tacc, class Tout>
__global__ void EltWiseQuantizeKernel(
    const Tacc* __restrict__ accumulator,
    Tout* __restrict__ output,
    int numBatches, int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int roundBits, int32_t satLow, int32_t satHigh, int mixedPrecision,
    float floatSatHigh, float floatSatLow)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numDIM1 * numDIM2 * numChannels * inHeight * inWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (numDIM1 * numDIM2 * numChannels * inHeight * inWidth);
    int d1 = (idx / (numDIM2 * numChannels * inHeight * inWidth)) % numDIM1;
    int d2 = (idx / (numChannels * inHeight * inWidth)) % numDIM2;
    int c = (idx / (inHeight * inWidth)) % numChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    uint32_t outOffset = (b * outBatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;

    Tacc outAcc = accumulator[outOffset];

    if (std::is_floating_point<Tout>::value)
    {
        outAcc = cuda_floatSat(outAcc, floatSatHigh, floatSatLow);
    }
    else
    {
        outAcc = (Tacc)cuda_roundSat((int64_t)outAcc, roundBits, satLow, satHigh);
        if (mixedPrecision == 1)
        {
            outAcc = (int64_t)outAcc >> 8;
    }
    }

    output[outOffset] = outAcc;
}


// Unified CUDA wrapper for EltWise operations
template <class Tin, class Tacc>
int TIDL_cudaEltWiseOp(
    const Tin* input,
    Tacc* accumulator,
    int32_t scale,
    int32_t zeropoint,
    int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch, int pixelPitch,
    int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int callno, int batchno, int eltWiseType, int fModValue, int outBatchPitch)
{
    // Calculate input memory size (can vary per input due to broadcasting)
    size_t input_size = sizeof(Tin)*(1 + (numDIM1-1)*inDIM1Pitch + (numDIM2-1)*inDIM2Pitch + (numChannels-1)*inChPitch + (inHeight-1)*inPitch + (inWidth-1)*pixelPitch);
    size_t accumulator_size = sizeof(Tacc)*(1 + (numDIM1-1)*outDIM1Pitch + (numDIM2-1)*outDIM2Pitch + (numChannels-1)*outChPitch + (inHeight-1)*outPitch + (inWidth-1));

    Tin* d_input = NULL;
    Tacc* d_accumulator_roi = NULL; // accumulator pointer offset to the current batch

    // Get GPU pointers after synchronization
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        /*If it's the first layer - it is possible that the input doesn't have a GPU mirror pointer, allocate a new buffer*/
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            /*Log Error*/
            printf("Eltwise: Unable to find input pointer\n");
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator_roi, accumulator_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator_roi, accumulator_size) != IALG_EOK)
        {
            /*Log Error*/
            printf("Eltwise: Unable to allocate accumulator pointer\n");
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D copy of full input buffer
    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size,
                                  cudaMemcpyHostToDevice, stream));

    // Zero accumulator on first input (callno == 0), ordered after H2D by stream
    if(callno == 0)
        checkCudaErr(cudaMemsetAsync(d_accumulator_roi, 0, accumulator_size, stream));

    int total_elements = numDIM1 * numDIM2 * numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    if (eltWiseType == TIDL_EltWiseSum)
    {
        EltWiseSumKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
            d_input, d_accumulator_roi, scale,
            numDIM1, numDIM2, numChannels, inHeight, inWidth,
            inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch, pixelPitch,
            outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch);
    }
    else if (eltWiseType == TIDL_EltWiseProduct) 
    {
        EltWiseProductKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
            d_input, d_accumulator_roi, scale, zeropoint,
            numDIM1, numDIM2, numChannels, inHeight, inWidth,
            inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch, pixelPitch,
            outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch,
            callno);
    }
    else if ((eltWiseType == TIDL_EltWiseMax) || (eltWiseType == TIDL_EltWiseMin)) 
    {
        EltWiseMinMaxKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
            d_input, d_accumulator_roi, scale,
            numDIM1, numDIM2, numChannels, inHeight, inWidth,
            inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch, pixelPitch,
            outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch,
            callno, eltWiseType);
    }
    else if(eltWiseType == TIDL_EltWiseMod)
    {
        EltWiseModKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
            d_input, d_accumulator_roi, scale, zeropoint,
            numDIM1, numDIM2, numChannels, inHeight, inWidth,
            inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch, pixelPitch,
            outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch, fModValue,
            callno);
    }
    checkCudaErr(cudaStreamSynchronize(stream));
    checkCudaErr(cudaGetLastError());
    

    // Results stay in GPU accumulator for the quantize wrapper to consume
    return IALG_EOK;
}

// CUDA wrapper for EltWise MMAv2 Quantization
template <class Tacc, class Tout>
int TIDL_cudaEltWiseMMAv2Quantize(
    const Tacc* accumulator,
    Tout* output,
    uint8_t mmaScale,
    uint8_t mmaShift,
    int32_t biasTerm,
    int numBatches, int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch, int tensorZeroPoint, int outPadOffset)
{
    // Calculate output memory size
    size_t output_size = sizeof(Tout)*(1 + (numBatches-1)*outBatchPitch + (numDIM1-1)*outDIM1Pitch + (numDIM2-1)*outDIM2Pitch + (numChannels-1)*outChPitch + (inHeight-1)*outPitch + (inWidth-1));
    size_t accum_size = sizeof(Tacc)*(1 + (numBatches-1)*outBatchPitch + (numDIM1-1)*outDIM1Pitch + (numDIM2-1)*outDIM2Pitch + (numChannels-1)*outChPitch + (inHeight-1)*outPitch + (inWidth-1));
    size_t outputTransferSize = sizeof(Tout)*(1 + (numBatches-1)*outBatchPitch + (numDIM1-1)*outDIM1Pitch + (numDIM2-1)*outDIM2Pitch + (numChannels-1)*outChPitch + (inHeight-1)*outPitch + (inWidth-1) - outPadOffset);

    Tacc* d_accumulator = NULL;
    Tout* d_output = NULL;

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
    {
        /*Log Error*/
        printf("Eltwise: Unable to find accumulator pointer\n");
    }

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            /*Log Error*/
            printf("Eltwise: Unable to find output pointer\n");
        }
    }

    // Launch kernel
    int total_elements = numBatches * numDIM1 * numDIM2 * numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // H2D output buffer
    checkCudaErr(cudaMemcpyAsync(d_output, output, output_size,
                                  cudaMemcpyHostToDevice, stream));

    // Fill zero-point (ordered after H2D by stream)
    TIDL_fillZeroPoint<Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_output, (Tout)tensorZeroPoint,
        numBatches, numDIM1, numDIM2, numChannels, inHeight, inWidth,
        outBatchPitch, outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch);

    TIDL_CudaEltWiseMMAv2QuantizeKernel<Tacc, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_accumulator, d_output, mmaScale, mmaShift, biasTerm,
        numBatches, numDIM1, numDIM2, numChannels, inHeight, inWidth,
        outBatchPitch, outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch);

    checkCudaErr(cudaGetLastError());

    // Async D2H (ordered after kernel by stream)
    checkCudaErr(cudaMemcpyAsync(output, d_output, outputTransferSize,
                                  cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));


    return IALG_EOK;
}

// CUDA wrapper for EltWise Quantization
template <class Tacc, class Tout>
int TIDL_cudaEltWiseQuantize(
    const Tacc* accumulator,
    Tout* output,
    int numBatches, int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int roundBits, int32_t satLow, int32_t satHigh, int mixedPrecision,
    float floatSatLow, float floatSatHigh)
{
    // Calculate output memory size
    size_t output_size = sizeof(Tout)*(1 + (numBatches-1)*outBatchPitch + (numDIM1-1)*outDIM1Pitch + (numDIM2-1)*outDIM2Pitch + (numChannels-1)*outChPitch + (inHeight-1)*outPitch + (inWidth-1));
    size_t accum_size = sizeof(Tacc)*(1 + (numBatches-1)*outBatchPitch + (numDIM1-1)*outDIM1Pitch + (numDIM2-1)*outDIM2Pitch + (numChannels-1)*outChPitch + (inHeight-1)*outPitch + (inWidth-1));

    Tacc* d_accumulator = NULL;
    Tout* d_output = NULL;

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
    {
        /*Log Error*/
        printf("Eltwise: Unable to find accumulator pointer\n");
    }

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            /*Log Error*/
            printf("Eltwise: Unable to find output pointer\n");
        }
    }

    // Launch kernel
    int total_elements = numBatches * numDIM1 * numDIM2 * numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // H2D output buffer
    checkCudaErr(cudaMemcpyAsync(d_output, output, output_size,
                                  cudaMemcpyHostToDevice, stream));

    // Fill zero-point (ordered after H2D by stream)
    TIDL_fillZeroPoint<Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_output, (Tout)0,
        numBatches, numDIM1, numDIM2, numChannels, inHeight, inWidth,
        outBatchPitch, outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch);

    // Quantize kernel (ordered after fill by stream)
    EltWiseQuantizeKernel<Tacc, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_accumulator, d_output,
        numBatches, numDIM1, numDIM2, numChannels, inHeight, inWidth,
        outBatchPitch, outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch,
        roundBits, satLow, satHigh, mixedPrecision,
        floatSatHigh, floatSatLow);

    checkCudaErr(cudaGetLastError());

    // Async D2H (ordered after kernel by stream)
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size,
                                  cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));


    return IALG_EOK;
}

template int TIDL_cudaEltWiseOp<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseOp<int, long>(int const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseOp<short, long>(short const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseOp<unsigned short, long>(unsigned short const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseOp<unsigned char, int>(unsigned char const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseOp<signed char, int>(signed char const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseMMAv2Quantize<int, unsigned char>(int const*, unsigned char*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseQuantize<long, int>(long const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseMMAv2Quantize<long, unsigned short>(long const*, unsigned short*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseQuantize<int, signed char>(int const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<int, unsigned char>(int const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<long, unsigned char>(long const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseMMAv2Quantize<int, signed char>(int const*, signed char*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseQuantize<long, signed char>(long const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<int, short>(int const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<int, unsigned short>(int const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<long, unsigned short>(long const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<long, short>(long const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseMMAv2Quantize<long, short>(long const*, short*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseMMAv2Quantize<float, float>(float const*, float*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int, int, int);