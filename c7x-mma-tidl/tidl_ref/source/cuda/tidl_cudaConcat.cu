/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaConcat.cu
 @brief   This file contains CUDA implementation of Concat layer.
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


/**
 * @brief CUDA kernel for concat operation - matches TIDL_refConcat exactly
 * 
 * Thread mapping: linearize 3D (channel, height, width) to 1D thread index
 * Each thread processes one output element
 */
template <class Tin, class Tacc>
__global__ void TIDL_CudaConcatKernel(
    const Tin* __restrict__ inData,
    Tacc* __restrict__ accumulator,
    int32_t scale,
    int numChannels, int inHeight, int inWidth,
    int inChPitch, int outChPitch, int inPitch, int outPitch,
    int isKernelHighPrecision, uint8_t mmaScale, uint8_t mmaShift,
    int32_t biasTerm, int outElemType, int32_t satLow, int32_t satHigh)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numChannels * inHeight * inWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 3D coordinates (channel, height, width)
    // Matches reference loop order: for(i1) for(i2) for(i3)
    int i1 = idx / (inHeight * inWidth);           // channel
    int i2 = (idx / inWidth) % inHeight;           // height
    int i3 = idx % inWidth;                         // width

    // Calculate offsets - exact same as reference
    int32_t inOffset = (i1 * inChPitch) + (i2 * inPitch) + i3;
    int32_t outOffset = (i1 * outChPitch) + (i2 * outPitch) + i3;

    // Core operation: pAcc[outOffset] = pIn[inOffset] * scale
    Tacc tempVal = (Tacc)inData[inOffset] * scale;
    
    // Optional MMA quantization for high precision kernel
    if (isKernelHighPrecision && outElemType != TIDL_SinglePrecFloat && outElemType != TIDL_BFloat16)
    {
        int64_t tempAcc = (int64_t)tempVal + biasTerm;
        tempAcc = tempAcc * mmaScale;
        // Use cuda_roundSatMMA for bit-exact match with reference TIDL_roundSatMMA
        accumulator[outOffset] = (Tacc)cuda_roundSatMMA(tempAcc, mmaShift, satLow, satHigh);
    }
    else
    {
        accumulator[outOffset] = tempVal;
    }
}

/**
 * @brief CUDA kernel for concat quantization - matches TIDL_refConcatQuantize exactly
 * 
 * Performs final quantization/saturation on accumulator values
 */
template <class Tacc, class Tout>
__global__ void TIDL_CudaConcatQuantizeKernel(
    const Tacc* __restrict__ accumulator,
    Tout* __restrict__ output,
    int numChannels, int inHeight, int inWidth,
    int outChPitch, int outPitch,
    int roundBits, int32_t satLow, int32_t satHigh,
    int isKernelHighPrecision, int outElemType,
    float floatSatLow, float floatSatHigh)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numChannels * inHeight * inWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 3D coordinates
    int i1 = idx / (inHeight * inWidth);           // channel
    int i2 = (idx / inWidth) % inHeight;           // height
    int i3 = idx % inWidth;                         // width

    int32_t outOffset = (i1 * outChPitch) + (i2 * outPitch) + i3;
    
    Tacc outAcc = accumulator[outOffset];

    if (std::is_floating_point<Tout>::value)
    {
        // Float path: apply float saturation
        outAcc = cuda_floatSat((float)outAcc, floatSatHigh, floatSatLow);
    }
    else if(std::is_same<Tout, bfloat16_tidl>::value)
    {
        outAcc = cuda_bf16Sat((float)outAcc, floatSatLow, floatSatHigh);
    }
    else if (isKernelHighPrecision)
    {
        // High precision path: saturate only (no rounding shift)
        outAcc = (Tacc)cuda_roundSatMMA((int64_t)outAcc, 0, satLow, satHigh);
    }
    else
    {
        // Standard fixed-point path: apply rounding and saturation
        outAcc = (Tacc)cuda_roundSat((int64_t)outAcc, roundBits, satLow, satHigh);
    }
    
    output[outOffset] = (Tout)outAcc;
}

/**
 * @brief CUDA wrapper for concat operation
 * 
 * Handles memory management and kernel launch for concat data copy with scale
 */
template <class Tin, class Tacc>
int TIDL_cudaConcatOp(
    const Tin* input,
    Tacc* accumulator,
    int32_t scale,
    int numChannels, int inHeight, int inWidth,
    int inChPitch, int outChPitch, int inPitch, int outPitch,
    int tensorIdx, int isKernelHighPrecision,
    uint8_t mmaScale, uint8_t mmaShift, int32_t biasTerm,
    int outElemType, int32_t satLow, int32_t satHigh)
{
    if (outElemType == TIDL_SignedChar)
    {
        satLow = cuda::std::numeric_limits<int8_t>::lowest();
        satHigh = cuda::std::numeric_limits<int8_t>::max();
    }
    else if (outElemType == TIDL_UnsignedChar)
    {
        satLow = cuda::std::numeric_limits<uint8_t>::lowest();
        satHigh = cuda::std::numeric_limits<uint8_t>::max();
    }
    else if (outElemType == TIDL_SignedShort)
    {
        satLow = cuda::std::numeric_limits<int16_t>::lowest();
        satHigh = cuda::std::numeric_limits<int16_t>::max();
    }
    else if (outElemType == TIDL_UnsignedShort)
    {
        satLow = cuda::std::numeric_limits<uint16_t>::lowest();
        satHigh = cuda::std::numeric_limits<uint16_t>::max();
    }


    // Calculate input memory size
    size_t input_size = sizeof(Tin) * (1 + (numChannels - 1) * inChPitch + 
                                       (inHeight - 1) * inPitch + (inWidth - 1));
    
    size_t accumulation_size = sizeof(Tacc) * (1 + (numChannels - 1) * outChPitch + 
                                         (inHeight - 1) * outPitch + (inWidth - 1));

    Tin* dpInput = NULL;
    Tacc* dpAccumulator = NULL;

    // Get GPU pointer for input or allocate if needed
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size) != IALG_EOK)
        {
            printf("Concat: Unable to allocate input pointer\n");
            return IALG_EFAIL;
        }
    }

    // Get GPU pointer for accumulator
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&dpAccumulator, accumulation_size) != IALG_EOK)
    {
        printf("Concat: Unable to allocate accumulator pointer\n");
        return IALG_EFAIL;
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(dpInput, input, input_size, cudaMemcpyHostToDevice, stream));

    int total_elements = numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    TIDL_CudaConcatKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        dpInput, dpAccumulator, scale,
        numChannels, inHeight, inWidth,
        inChPitch, outChPitch, inPitch, outPitch,
        isKernelHighPrecision, mmaScale, mmaShift, biasTerm,
        outElemType, satLow, satHigh);


    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaStreamSynchronize(stream));

    // Results stay in GPU accumulator for quantize wrapper to use
    return IALG_EOK;
}

/**
 * @brief CUDA wrapper for concat quantization
 * 
 * Performs final quantization and copies results back to CPU
 */
template <class Tacc, class Tout>
int TIDL_cudaConcatQuantize(
    const Tacc* accumulator,
    Tout* output,
    int numChannels, int inHeight, int inWidth,
    int outChPitch, int outPitch,
    int roundBits, int32_t satLow, int32_t satHigh,
    int isKernelHighPrecision, int outElemType, int32_t outPadOffset,
    float floatSatLow, float floatSatHigh, int32_t tensorZeroPoint)
{
    // Calculate output memory size
    size_t output_size = sizeof(Tout) * (1 + (numChannels - 1) * outChPitch + 
                                         (inHeight - 1) * outPitch + (inWidth - 1));
    
    size_t accumulation_size = sizeof(Tacc) * (1 + (numChannels - 1) * outChPitch + 
                                         (inHeight - 1) * outPitch + (inWidth - 1));

    Tacc* dpAccumulator = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&dpAccumulator, accumulation_size) != IALG_EOK)
    {
        printf("Concat: Unable to allocate accumulator pointer\n");
        return IALG_EFAIL;
    }
    
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Concat: Unable to allocate output pointer\n");
            return IALG_EFAIL;
        }
    }

    int total_elements = numChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

   cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D for output buffer + zero-point fill on same stream
    checkCudaErr(cudaMemcpyAsync(d_output, output, output_size, cudaMemcpyHostToDevice, stream));
    TIDL_fillZeroPoint<Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(d_output, (Tout)tensorZeroPoint,
                                                    1, 1, 1, numChannels, inHeight, inWidth,
                                                    1, 1, 1, outChPitch, outPitch);

    TIDL_CudaConcatQuantizeKernel<Tacc, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        dpAccumulator, d_output,
        numChannels, inHeight, inWidth,
        outChPitch, outPitch,
        roundBits, satLow, satHigh,
        isKernelHighPrecision, outElemType,
        floatSatLow, floatSatHigh);

    // Async D2H – enqueued on stream before sync so D2H overlaps with host cleanup
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));
    checkCudaErr(cudaGetLastError());

    return IALG_EOK;
}

// Template instantiations for TIDL_cudaConcatOp
template int TIDL_cudaConcatOp<int8_t, int32_t>(int8_t const*, int32_t*, int32_t, int, int, int, int, int, int, int, int, int, unsigned char, unsigned char, int32_t, int, int32_t, int32_t);
template int TIDL_cudaConcatOp<uint8_t, int32_t>(uint8_t const*, int32_t*, int32_t, int, int, int, int, int, int, int, int, int, unsigned char, unsigned char, int32_t, int, int32_t, int32_t);
template int TIDL_cudaConcatOp<int16_t, int32_t>(int16_t const*, int32_t*, int32_t, int, int, int, int, int, int, int, int, int, unsigned char, unsigned char, int32_t, int, int32_t, int32_t);
template int TIDL_cudaConcatOp<uint16_t, int32_t>(uint16_t const*, int32_t*, int32_t, int, int, int, int, int, int, int, int, int, unsigned char, unsigned char, int32_t, int, int32_t, int32_t);
template int TIDL_cudaConcatOp<float, float>(float const*, float*, int32_t, int, int, int, int, int, int, int, int, int, unsigned char, unsigned char, int32_t, int, int32_t, int32_t);
template int TIDL_cudaConcatOp<bfloat16_tidl, float>(bfloat16_tidl const*, float*, int32_t, int, int, int, int, int, int, int, int, int, unsigned char, unsigned char, int32_t, int, int32_t, int32_t);

// Template instantiations for TIDL_cudaConcatQuantize
template int TIDL_cudaConcatQuantize<int32_t, int8_t>(int32_t const*, int8_t*, int, int, int, int, int, int, int32_t, int32_t, int, int, int, float, float,int);
template int TIDL_cudaConcatQuantize<int32_t, uint8_t>(int32_t const*, uint8_t*, int, int, int, int, int, int, int32_t, int32_t, int, int, int, float, float,int);
template int TIDL_cudaConcatQuantize<int32_t, int16_t>(int32_t const*, int16_t*, int, int, int, int, int, int, int32_t, int32_t, int, int, int, float, float,int);
template int TIDL_cudaConcatQuantize<int32_t, uint16_t>(int32_t const*, uint16_t*, int, int, int, int, int, int, int32_t, int32_t, int, int, int, float, float,int);
template int TIDL_cudaConcatQuantize<float, float>(float const*, float*, int, int, int, int, int, int, int32_t, int32_t, int, int, int, float, float,int);
template int TIDL_cudaConcatQuantize<float, bfloat16_tidl>(float const*, bfloat16_tidl*, int, int, int, int, int, int, int32_t, int32_t, int, int, int, float, float,int);
