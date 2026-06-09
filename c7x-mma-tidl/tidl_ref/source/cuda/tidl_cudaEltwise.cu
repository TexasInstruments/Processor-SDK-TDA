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
/* External declaration of the global memory manager pointer */
extern TIDL_CudaMemManager* g_cudaMemManager;

// Persistent memory structure for EltWise
typedef struct {
    int isInit;
    void *dpInput[TIDL_NUM_IN_BUFS];  // Multiple inputs for EltWise operations
    void *dpAccumulator;
    void *dpOutput;
} TIDL_cudaEWS;

static TIDL_cudaEWS CUDAEWS[MEM_BUFF_ARRAY_LEN] = {0};

// Function to allocate and initialize accumulator memory once for all batches
template <class Tacc>
int TIDL_cudaEltWiseAllocateAccumulator(int numBatches, int outBatchPitch)
{
    size_t acc_size = numBatches * outBatchPitch * sizeof(Tacc);
    if (!CUDAEWS[CUDNNLC].isInit) {
        // Allocate accumulator for all batches (stays constant)
        checkCudaErr(cudaMalloc((void**)&CUDAEWS[CUDNNLC].dpAccumulator, acc_size));
        
        // Initialize accumulator to zero (critical for Sum operations)
        checkCudaErr(cudaMemset(CUDAEWS[CUDNNLC].dpAccumulator, 0, acc_size));
    }
    
    return IALG_EOK;
}

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

// EltWise Max kernel - matches reference loop structure exactly
template <class Tin, class Tacc>
__global__ void EltWiseMaxKernel(
    const Tin* __restrict__ inData,
    Tacc* __restrict__ accumulator,
    int32_t scale,
    int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch, int pixelPitch,
    int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int callno)
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
        accumulator[outOffset] = max(currentVal, newVal);
    }
}

// EltWise MMAv2 Quantization kernel - matches reference loop structure exactly
template <class Tacc, class Tout>
__global__ void EltWiseMMAv2QuantizeKernel(
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
        output[outOffset] = (Tout)cuda_roundSat((int64_t)tempAcc, mmaShift, (int64_t)cuda::std::numeric_limits<Tout>::lowest(), (int64_t)cuda::std::numeric_limits<Tout>::max());
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
    int callno, int batchno, int eltWiseType, int outBatchPitch)
{
    // Calculate input memory size (can vary per input due to broadcasting)
    size_t input_size = sizeof(Tin);
    // Only multiply by dimensions that actually contribute to memory layout
    if (inDIM1Pitch > 0) input_size *= numDIM1;
    if (inDIM2Pitch > 0) input_size *= numDIM2; 
    if (inChPitch > 0) input_size *= numChannels;
    if (inPitch > 0) input_size *= inHeight;
    if (pixelPitch > 0) input_size *= inWidth;

    // Batch-optimized input allocation: only allocate for first batch (batchno == 0)
    if (batchno == 0) {
        if (CUDAEWS[CUDNNLC].dpInput[callno] == NULL) {
            checkCudaErr(cudaMalloc((void**)&CUDAEWS[CUDNNLC].dpInput[callno], input_size));
        }
    }

    // Get persistent pointers
    Tin* d_input = (Tin*)CUDAEWS[CUDNNLC].dpInput[callno];
    Tacc* d_accumulator_base = (Tacc*)CUDAEWS[CUDNNLC].dpAccumulator;

    // Calculate batch-specific accumulator pointer (matches reference logic)
    Tacc* d_accumulator_roi;
    if (sizeof(Tacc) == sizeof(int64_t)) {
        // 64-bit accumulator (TIDL_SignedDoubleWord)
        d_accumulator_roi = (Tacc*)(((int64_t*)d_accumulator_base) + (batchno * outBatchPitch));
    } else {
        // 32-bit accumulator (regular case)
        d_accumulator_roi = d_accumulator_base + (batchno * outBatchPitch);
    }

    // Copy input data to GPU (reusing same buffer for all batches)
    checkCudaErr(cudaMemcpy(d_input, input, input_size, cudaMemcpyHostToDevice));

    // Launch appropriate kernel based on operation type
    int total_elements = numDIM1 * numDIM2 * numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    if (eltWiseType == TIDL_EltWiseSum)
    {
        EltWiseSumKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_input, d_accumulator_roi, scale,
            numDIM1, numDIM2, numChannels, inHeight, inWidth,
            inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch, pixelPitch,
            outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch);
    }
    else if (eltWiseType == TIDL_EltWiseProduct) 
    {
        EltWiseProductKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_input, d_accumulator_roi, scale, zeropoint,
            numDIM1, numDIM2, numChannels, inHeight, inWidth,
            inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch, pixelPitch,
            outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch,
            callno);
    } 
    else if (eltWiseType == TIDL_EltWiseMax) 
    {
        EltWiseMaxKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_input, d_accumulator_roi, scale,
            numDIM1, numDIM2, numChannels, inHeight, inWidth,
            inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch, pixelPitch,
            outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch,
            callno);
    }

    checkCudaErr(cudaGetLastError());
    
    // Results stay in GPU accumulator for quantize wrapper to use
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
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch)
{
    // Calculate output memory size
    size_t output_size = numBatches * outBatchPitch * sizeof(Tout);

    // Allocate output memory if not already initialized
    if (!CUDAEWS[CUDNNLC].isInit) {
        checkCudaErr(cudaMalloc((void**)&CUDAEWS[CUDNNLC].dpOutput, output_size));
        CUDAEWS[CUDNNLC].isInit = 1;
    }

    // Get persistent pointers (accumulator already allocated by TIDL_cudaEltWiseAllocateAccumulator)
    Tacc* d_accumulator = (Tacc*)CUDAEWS[CUDNNLC].dpAccumulator;
    Tout* d_output = (Tout*)CUDAEWS[CUDNNLC].dpOutput;

    // Launch kernel
    int total_elements = numBatches * numDIM1 * numDIM2 * numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    EltWiseMMAv2QuantizeKernel<Tacc, Tout><<<grid_size, THREADS_PER_BLOCK>>>(
        d_accumulator, d_output, mmaScale, mmaShift, biasTerm,
        numBatches, numDIM1, numDIM2, numChannels, inHeight, inWidth,
        outBatchPitch, outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch);

    checkCudaErr(cudaGetLastError());
    
    // Copy result back to CPU
    checkCudaErr(cudaMemcpy(output, d_output, output_size, cudaMemcpyDeviceToHost));

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
    size_t output_size = numBatches * outBatchPitch * sizeof(Tout);

    // Allocate output memory if not already initialized
    if (!CUDAEWS[CUDNNLC].isInit) {
        checkCudaErr(cudaMalloc((void**)&CUDAEWS[CUDNNLC].dpOutput, output_size));
        CUDAEWS[CUDNNLC].isInit = 1;
    }

    // Get persistent pointers (accumulator already allocated by TIDL_cudaEltWiseAllocateAccumulator)
    Tacc* d_accumulator = (Tacc*)CUDAEWS[CUDNNLC].dpAccumulator;
    Tout* d_output = (Tout*)CUDAEWS[CUDNNLC].dpOutput;

    // Launch kernel
    int total_elements = numBatches * numDIM1 * numDIM2 * numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    EltWiseQuantizeKernel<Tacc, Tout><<<grid_size, THREADS_PER_BLOCK>>>(
        d_accumulator, d_output,
        numBatches, numDIM1, numDIM2, numChannels, inHeight, inWidth,
        outBatchPitch, outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch,
        roundBits, satLow, satHigh, mixedPrecision,
        floatSatHigh, floatSatLow);

    checkCudaErr(cudaGetLastError());
    
    // Copy result back to CPU
    checkCudaErr(cudaMemcpy(output, d_output, output_size, cudaMemcpyDeviceToHost));

    return IALG_EOK;
}

// Function to set initialization flag
void TIDL_cudaSetEltwiseInitFlag(int32_t layerIdx)
{
    if (layerIdx >= 0 && layerIdx < MEM_BUFF_ARRAY_LEN) {
        CUDAEWS[layerIdx].isInit = 1;
    }
}

// Function to free CUDA memory
void TIDL_cudaFreeEltwiseCudaPtrs()
{
    for (int i = 0; i < MEM_BUFF_ARRAY_LEN; i++) {
        for (int j = 0; j < TIDL_NUM_IN_BUFS; j++) {
            if (CUDAEWS[i].dpInput[j]) cudaFree(CUDAEWS[i].dpInput[j]);
            CUDAEWS[i].dpInput[j] = 0;
        }
        if (CUDAEWS[i].dpAccumulator) cudaFree(CUDAEWS[i].dpAccumulator);
        if (CUDAEWS[i].dpOutput) cudaFree(CUDAEWS[i].dpOutput);
        
        CUDAEWS[i].dpAccumulator = 0;
        CUDAEWS[i].dpOutput = 0;
        CUDAEWS[i].isInit = 0;
    }
    cudaDeviceSynchronize();
}

template int TIDL_cudaEltWiseOp<short, long>(short const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseOp<unsigned short, long>(unsigned short const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseQuantize<int, unsigned char>(int const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<long, unsigned char>(long const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseOp<unsigned char, int>(unsigned char const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseQuantize<int, signed char>(int const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseOp<signed char, int>(signed char const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseQuantize<long, short>(long const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<int, unsigned short>(int const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<int, short>(int const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseMMAv2Quantize<int, unsigned char>(int const*, unsigned char*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseAllocateAccumulator<long>(int, int);
template int TIDL_cudaEltWiseQuantize<long, signed char>(long const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseQuantize<long, unsigned short>(long const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseMMAv2Quantize<long, unsigned short>(long const*, unsigned short*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseQuantize<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaEltWiseOp<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseMMAv2Quantize<int, signed char>(int const*, signed char*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseMMAv2Quantize<long, short>(long const*, short*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaEltWiseAllocateAccumulator<int>(int, int);
template int TIDL_cudaEltWiseMMAv2Quantize<float, float>(float const*, float*, unsigned char, unsigned char, int, int, int, int, int, int, int, int, int, int, int, int);
