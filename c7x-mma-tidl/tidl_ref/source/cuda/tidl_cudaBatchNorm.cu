/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaBatchNorm.cu
 @brief   This file contains CUDA implementation of BatchNorm layer.
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


// CUDA version of TIDL_refBatchNormCoreRoundSlope
__device__ int32_t cuda_roundSlope(
    int32_t out,
    int32_t slopeFact,
    int32_t slopeQBits)
{
    return (((int64_t)out * slopeFact) >> (uint64_t)slopeQBits);
}

// Main BatchNorm computation kernel
template <class Tin, class Tw, class Tb, class Tacc>
__global__ void BatchNormComputeKernel(
    const Tin* __restrict__ input,
    const Tw* __restrict__ weights,
    const Tw* __restrict__ slopes,
    const Tb* __restrict__ bias,
    Tacc* __restrict__ accumulator,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int numDIM1, int numDIM2,
    int inBatchPitch, int inChPitch, int inDIM1Pitch, int inDIM2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch,
    int actType, float slopeScale)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * numDIM1 * numDIM2 * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (numDIM1 * numDIM2 * numChannels * imHeight * imWidth);
    int d1 = (idx / (numDIM2 * numChannels * imHeight * imWidth)) % numDIM1;
    int d2 = (idx / (numChannels * imHeight * imWidth)) % numDIM2;
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    // Get input value
    Tin inDataVal = (Tin)input[(b * inBatchPitch) + (d1 * inDIM1Pitch) + (d2 * inDIM2Pitch) + (c * inChPitch) + (h * inPitch) + w];
    
    // Get weight and bias for this channel
    Tw weightVal = weights[c];
    Tb biasVal = bias[c];
    
    // Main computation
    Tacc out = ((inDataVal*weightVal) + biasVal);
    
    Tw preluScale = 1;
    int32_t slopeFact = 1;
    int32_t slopeQBits = 0;
    
    if (actType == TIDL_PRelU) {
        preluScale = slopes[c];
        slopeQBits = 8;
        int32_t tempSlopeFact = preluScale * (((int32_t)1) << slopeQBits);
        float floatSlopeFact = (float)(tempSlopeFact) / slopeScale;
        slopeFact = (int32_t)(floatSlopeFact);
        
        // Special handling for 16-bit weights (TIDL-1332)
        if (sizeof(Tw) == sizeof(int16_t)) 
        {
            slopeFact = (int32_t)(floatSlopeFact * 256);
            slopeQBits += 8;
        }
    }
    
    // Handle PRelU activation if needed (innermost loop logic)
    if (out < 0) 
    {
        if (std::is_floating_point<Tacc>::value) 
        {
            out = out * preluScale;
        } 
        else 
        {
            out = (Tacc)cuda_roundSlope((int32_t)out, slopeFact, slopeQBits);
        }
    }
    
    // Store result in accumulator
    accumulator[(b * outBatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w] = out;
}

// Saturation kernel
template <class Tacc, class Tout>
__global__ void TIDL_CudaBatchNormSaturateKernel(
    const Tacc* __restrict__ accumulator,
    Tout* __restrict__ output,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int numDIM1, int numDIM2,
    int outBatchPitch, int outChPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch,
    int outRoundBits, int satLow, int satHigh, int mixedPrecision, float floatSatLow, float floatSatHigh)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * numDIM1 * numDIM2 * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (numDIM1 * numDIM2 * numChannels * imHeight * imWidth);
    int d1 = (idx / (numDIM2 * numChannels * imHeight * imWidth)) % numDIM1;
    int d2 = (idx / (numChannels * imHeight * imWidth)) % numDIM2;
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    // Get accumulator value
    int acc_idx = (b * outBatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outChPitch) + (h * outPitch) + w;
    Tacc out = accumulator[acc_idx];
    
    if (std::is_floating_point<Tacc>::value) 
    {
        // Float saturation - use float parameters like TIDL_floatSat
        out = cuda_floatSat(out, floatSatHigh, floatSatLow);
    } 
    else 
    {
        // Fixed-point saturation
        out = cuda_roundSat((int64_t)out, outRoundBits, (int32_t)satLow, (int32_t)satHigh);
        
        // Handle mixed precision
        if (mixedPrecision == 1) {
            out = (uint64_t)out >> 8U;
        }
    }
    
    // Store final result
    output[acc_idx] = (Tout)out;
}

// CUDA wrapper for BatchNorm computation
template <class Tin, class Tw, class Tb, class Tacc>
int TIDL_cudaBatchNorm(
    const Tin* input,
    const Tw* weights,
    const Tw* slopes,
    const Tb* bias,
    Tacc* accumulator,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int numDIM1, int numDIM2,
    int inBatchPitch, int inChPitch, int inDIM1Pitch, int inDIM2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch,
    int actType, float slopeScale)
{
    // Calculate memory sizes
    // size_t input_size = numTotBatches * inBatchPitch * sizeof(Tin);
    size_t input_size = sizeof(Tin) * (1 + (numTotBatches-1)*inBatchPitch + (numDIM1-1)*inDIM1Pitch + (numDIM2-1)*inDIM2Pitch + (numChannels-1)*inChPitch + (imHeight-1)*inPitch + (imWidth-1));
    size_t weights_size = numChannels * sizeof(Tw);
    size_t slopes_size = numChannels * sizeof(Tw);
    size_t bias_size = numChannels * sizeof(Tb);
    size_t acc_size = numTotBatches * outBatchPitch * sizeof(Tacc);

    Tin* dpInput = NULL;
    Tw* d_weights = NULL;
    Tw* d_slopes = NULL;
    Tb* d_bias = NULL;
    Tacc* dpAccumulator = NULL;

    // void* inPtrs[1] = {(void*)input};
    // uint32_t inDataSizes[1] = {(uint32_t)input_size};
    // TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, 1, inDataSizes);

    // Get GPU pointers after synchronization
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size) != IALG_EOK)
    {
        /*If it's the first layer - it is possible that the input doesn't have a GPU mirror pointer, allocate a new buffer*/
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size) != IALG_EOK)
        {
            /*Log Error*/
            printf("BatchNorm: Unable to find input pointer\n");
        }
    }

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), weights, (void**)&d_weights, weights_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), weights, (void**)&d_weights, weights_size) != IALG_EOK)
        {
            printf("BatchNorm: Unable to find weights pointer\n");
        }
        /* Will be copied async on stream below */
    }

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), slopes, (void**)&d_slopes, slopes_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), slopes, (void**)&d_slopes, slopes_size) != IALG_EOK)
        {
            printf("BatchNorm: Unable to find slopes pointer\n");
        }
        /* Will be copied async on stream below */
    }

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), bias, (void**)&d_bias, bias_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), bias, (void**)&d_bias, bias_size) != IALG_EOK)
        {
            printf("BatchNorm: Unable to find bias pointer\n");
        }
    }

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&dpAccumulator, acc_size) != IALG_EOK)
    {
        printf("BatchNorm: Unable to find accumulator pointer\n");
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D: weights and slopes (first-use only), input and bias in same stream
    if (d_weights != NULL) checkCudaErr(cudaMemcpyAsync(d_weights, weights, weights_size, cudaMemcpyHostToDevice, stream));
    if (d_slopes  != NULL) checkCudaErr(cudaMemcpyAsync(d_slopes,  slopes,  slopes_size,  cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(dpInput, input, input_size, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(d_bias,  bias,  bias_size,  cudaMemcpyHostToDevice, stream));


    int total_elements = numTotBatches * numDIM1 * numDIM2 * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    BatchNormComputeKernel<Tin, Tw, Tb, Tacc><<<gridSize, THREADS_PER_BLOCK, 0, stream>>>(
        dpInput, d_weights, d_slopes, d_bias, dpAccumulator,
        numTotBatches, numChannels, imWidth, imHeight, numDIM1, numDIM2,
        inBatchPitch, inChPitch, inDIM1Pitch, inDIM2Pitch, inPitch,
        outBatchPitch, outChPitch, outDIM1Pitch, outDIM2Pitch, outPitch,
        actType, slopeScale);

    checkCudaErr(cudaStreamSynchronize(stream));
    checkCudaErr(cudaGetLastError());


    return IALG_EOK;
}

// CUDA wrapper for BatchNorm saturation
template <class Tacc, class Tout>
int TIDL_cudaBatchNormSaturation(
    const Tacc* accumulator,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int numDIM1, int numDIM2,
    int outBatchPitch, int outChPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch,
    int outRoundBits, int satLow, int satHigh, int mixedPrecision, float floatSatLow, float floatSatHigh)
{
    // Calculate memory sizes
    size_t acc_size = numTotBatches * outBatchPitch * sizeof(Tacc);
    size_t output_size = numTotBatches * outBatchPitch * sizeof(Tout);

    Tacc* dpAccumulator = NULL;
    Tout* dpOutput = NULL;

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&dpAccumulator, acc_size) != IALG_EOK)
    {
        printf("BatchNorm: Unable to find accumulator pointer\n");
    }

    if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&dpOutput, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&dpOutput, output_size) != IALG_EOK)
        {
            /*Log Error*/
            printf("BatchNorm: Unable to find output pointer\n");
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    int total_elements = numTotBatches * numDIM1 * numDIM2 * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    if (sizeof(Tout) != 1)
    {
        TIDL_convFillZeroPoint<Tout><<<GRID_SIZE(output_size/sizeof(Tout), THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(dpOutput, output_size/sizeof(Tout), (Tout)0);
    }
    else
    {
        checkCudaErr(cudaMemsetAsync(dpOutput, (Tout)0, sizeof(Tout) * output_size/sizeof(Tout), stream));
    }

    TIDL_CudaBatchNormSaturateKernel<Tacc, Tout><<<gridSize, THREADS_PER_BLOCK, 0, stream>>>(
        dpAccumulator, dpOutput,
        numTotBatches, numChannels, imWidth, imHeight, numDIM1, numDIM2,
        outBatchPitch, outChPitch, outDIM1Pitch, outDIM2Pitch, outPitch,
        outRoundBits, satLow, satHigh, mixedPrecision, floatSatLow, floatSatHigh);

    /* Async D2H ordered after saturation kernel on same stream */
    checkCudaErr(cudaMemcpyAsync(output, dpOutput, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));
    checkCudaErr(cudaGetLastError());
    
    // void* outPtrs[1] = {(void*)output};
    // uint32_t outDataSizes[1] = {(uint32_t)output_size};
    // TIDL_cudaMemManagerPostLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), outPtrs, 1, outDataSizes);

    return IALG_EOK;
}


// CUDA device functions for High Accuracy Sigmoid - matching reference implementation exactly
__device__ float cuda_div_sp(float x, float y) {
    // Newton-Raphson division matching reference div_sp implementation
    float recp_y = __frcp_rn(y);  // CUDA reciprocal approximation
    float TWO = 2.0f;
    float result;
    
    // Iterative refinement step
    recp_y = recp_y * (TWO - (y * recp_y));
    result = x * recp_y;
    
    return result;
}

__device__ float cuda_exp_taylor_sigmoid(float x) {
    // Taylor series implementation matching reference exp_taylor_sigmoid exactly
    float twoPwF, ePwX;
    
    float ln2 = 0.693147180559945f;
    float oneBy6 = 0.1666667f;
    int32_t yI = (int32_t)x;
    float yf = x - (float)yI;
    float pkdOneBy65356 = 0.0000152587890625f;  // 1/65536
    float floatRes = yf * ln2;
    
    float floatRes2 = floatRes * floatRes;
    float floatRes3 = floatRes2 * floatRes;
    
    // Taylor series expansion
    twoPwF = 1.0f + floatRes + (floatRes2 * 0.5f);
    twoPwF = twoPwF + (floatRes3 * oneBy6);
    
    // Bit-shifting logic adapted for CUDA
    bool vp = (yI > 0);
    int32_t shift_val = min(14, abs(yI));
    int32_t tempShiftL = (1 << 16) << shift_val;
    int32_t tempShiftR = (1 << 16) >> shift_val;
    
    int32_t tempShift = vp ? tempShiftL : tempShiftR;
    ePwX = twoPwF * (float)(tempShift);
    
    ePwX = ePwX * pkdOneBy65356;
    
    return ePwX;
}

// High Accuracy Sigmoid computation kernel - matches reference loop structure exactly
template <class Tin, class Tout>
__global__ void HighAccuracySigmoidKernel(
    const Tin* __restrict__ inData,
    Tout* __restrict__ outData,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    float inputScale, float outputScale)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    // Mapping: b=i5, d1=i4, d2=i3, c=i2, h=i0, w=i1
    int b = idx / (inDim1 * inDim2 * numChannels * imHeight * imWidth);
    int d1 = (idx / (inDim2 * numChannels * imHeight * imWidth)) % inDim1;
    int d2 = (idx / (numChannels * imHeight * imWidth)) % inDim2;
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    // Get input value - exact same indexing as reference
    Tin inDataVal = inData[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + (h*inPitch) + w];
    
    // Calculate inDataScaleFact as per reference implementation
    float inDataScaleFact = ((-1.0f) * 1.44269504090f) / inputScale;
    
    // High accuracy sigmoid computation - exact same as reference
    float inValF = inDataVal * inDataScaleFact;
    float outValF = cuda_div_sp(1.0f, (cuda_exp_taylor_sigmoid(inValF) + 1.0f));
    outValF = outValF * outputScale;
    
    // Apply saturation using proper CUDA numeric limits (upper bound only) - optimized ternary
    Tout satHigh = cuda::std::numeric_limits<Tout>::max();
    Tout out = (outValF > (float)satHigh) ? satHigh : (Tout)outValF;
    
    // Store result - exact same indexing as reference
    outData[(b*outBatchPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + (c*outChPitch) + (h*outPitch) + w] = out;
}

// CUDA wrapper for High Accuracy Sigmoid using persistent memory
template <class Tin, class Tout>
int TIDL_cudaHighAccuracySigmoid(
    const Tin* input,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    float inputScale, float outputScale)
{
    // Calculate memory sizes
    size_t input_size = numTotBatches * inBatchPitch * sizeof(Tin);
    size_t output_size = numTotBatches * outBatchPitch * sizeof(Tout);

    Tin* dpInput = NULL;
    Tout* dpOutput = NULL;

    void* inPtrs[1] = {(void*)input};
    uint32_t inDataSizes[1] = {(uint32_t)input_size};
    TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, 1, inDataSizes);

    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&dpOutput, output_size);

    // Launch kernel
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    HighAccuracySigmoidKernel<Tin, Tout><<<gridSize, THREADS_PER_BLOCK>>>(
        dpInput, dpOutput,
        numTotBatches, numChannels, imWidth, imHeight, inDim1, inDim2,
        inBatchPitch, inChPitch, inDim1Pitch, inDim2Pitch, inPitch,
        outBatchPitch, outChPitch, outDim1Pitch, outDim2Pitch, outPitch,
        inputScale, outputScale);

    checkCudaErr(cudaGetLastError());
    
    void* outPtrs[1] = {(void*)output};
    uint32_t outDataSizes[1] = {(uint32_t)output_size};
    TIDL_cudaMemManagerPostLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), outPtrs, 1, outDataSizes);

    return IALG_EOK;
}

// 4-Point Approximation Sigmoid computation kernel
template <class Tin, class Tout, class Tacc>
__global__ void TIDL_CudaSigmoidKernel(
    const Tin* __restrict__ inData,
    Tacc* __restrict__ accumulator,
    int numTotBatches, int numChannels, int imWidth, int imHeight,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    uint32_t threshold0, uint32_t threshold1, uint32_t threshold2, uint16_t inDataScale,
    Tout* slope, Tout* offset, Tout offsetScale)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 4D coordinates (batch, channel, height, width)
    int b = idx / (numChannels * imHeight * imWidth);
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    // Get input value
    Tin inDataVal = inData[(b*inBatchPitch) + (c*inChPitch) + (h*inPitch) + w];
    
    // 4-point approximation sigmoid computation
    int32_t inDataValAbs = abs(inDataVal);
    uint32_t inDataValShl8 = inDataValAbs * inDataScale;
    
    int segment = (inDataValShl8 < threshold0) + (inDataValShl8 < threshold1) + (inDataValShl8 < threshold2);
    Tacc outVal = (slope[segment] * inDataValAbs) + (offset[segment] * offsetScale);

    outVal = (inDataVal < 0) ? ((offset[0] * offsetScale) - outVal) : outVal;
    
    // Store result
    accumulator[(b*outBatchPitch) + (c*outChPitch) + (h*outPitch) + w] = outVal;
}

// CUDA wrapper for 4-Point Approximation Sigmoid using persistent memory
template <class Tin, class Tout, class Tacc>
int TIDL_cudaSigmoid(
    const Tin* input,
    Tacc* accumulator,
    int numTotBatches, int numChannels, int imWidth, int imHeight,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    uint32_t threshold0, uint32_t threshold1, uint32_t threshold2, uint16_t inDataScale,
    Tout* slope, Tout* offset, Tout offsetScale)
{
    // Calculate memory sizes
    size_t input_size = numTotBatches * inBatchPitch * sizeof(Tin);
    size_t acc_size = numTotBatches * outBatchPitch * sizeof(Tacc);
    size_t slope_size = 4 * sizeof(Tout);
    size_t offset_size = 4 * sizeof(Tout);

    Tin* dpInput = NULL;
    Tacc* dpAccumulator = NULL;
    Tout* dpSlope = NULL;
    Tout* dpOffset = NULL;

    void* inPtrs[1] = {(void*)input};
    uint32_t inDataSizes[1] = {(uint32_t)input_size};
    TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, 1, inDataSizes);

    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&dpAccumulator, acc_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), slope, (void**)&dpSlope, slope_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), offset, (void**)&dpOffset, offset_size);

    // Launch kernel
    int total_elements = numTotBatches * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    TIDL_CudaSigmoidKernel<Tin, Tout, Tacc><<<gridSize, THREADS_PER_BLOCK>>>(
        dpInput, dpAccumulator,
        numTotBatches, numChannels, imWidth, imHeight,
        inBatchPitch, inChPitch, inPitch,
        outBatchPitch, outChPitch, outPitch,
        threshold0, threshold1, threshold2, inDataScale,
        dpSlope, dpOffset, offsetScale);

    checkCudaErr(cudaGetLastError());

    return IALG_EOK;
}

// Sigmoid Saturation kernel - matches reference loop structure exactly
template <class Tacc, class Tout>
__global__ void SigmoidSaturateKernel(
    const Tacc* __restrict__ accumulator,
    Tout* __restrict__ output,
    int numTotBatches, int numChannels, int imWidth, int imHeight,
    int outBatchPitch, int outChPitch, int outPitch,
    int outRoundBits, Tout satLow, Tout satHigh)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 4D coordinates (batch, channel, height, width)
    int b = idx / (numChannels * imHeight * imWidth);
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    // Get accumulator value - exact same indexing as reference
    int acc_idx = (b*outBatchPitch) + (c*outChPitch) + (h*outPitch) + w;
    Tacc outVal = accumulator[acc_idx];
    
    // Apply saturation - exact same as reference
    Tout out = (Tout)cuda_roundSat(outVal, outRoundBits, satLow, satHigh);
    
    // Store final result - exact same indexing as reference
    output[acc_idx] = out;
}

// CUDA wrapper for Sigmoid Saturation using persistent memory
template <class Tacc, class Tout>
int TIDL_cudaSigmoidSaturation(
    const Tacc* accumulator,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight,
    int outBatchPitch, int outChPitch, int outPitch,
    int outRoundBits, Tout satLow, Tout satHigh)
{
    // Calculate memory sizes
    size_t acc_size = numTotBatches * outBatchPitch * sizeof(Tacc);
    size_t output_size = numTotBatches * outBatchPitch * sizeof(Tout);

    Tacc* dpAccumulator = NULL;
    Tout* dpOutput = NULL;

    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&dpAccumulator, acc_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&dpOutput, output_size);

    // Launch kernel
    int total_elements = numTotBatches * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    SigmoidSaturateKernel<Tacc, Tout><<<gridSize, THREADS_PER_BLOCK>>>(
        dpAccumulator, dpOutput,
        numTotBatches, numChannels, imWidth, imHeight,
        outBatchPitch, outChPitch, outPitch,
        outRoundBits, satLow, satHigh);

    checkCudaErr(cudaGetLastError());
    
    void* outPtrs[1] = {(void*)output};
    uint32_t outDataSizes[1] = {(uint32_t)output_size};
    TIDL_cudaMemManagerPostLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), outPtrs, 1, outDataSizes);

    return IALG_EOK;
}

// Float Sigmoid computation kernel - matches reference loop structure exactly
template <class Tin, class Tout>
__global__ void FloatSigmoidKernel(
    const Tin* __restrict__ inData,
    Tout* __restrict__ outData,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    // Mapping: b=i5, d1=i4, d2=i3, c=i2, h=i0, w=i1 (matching reference implementation)
    int b = idx / (inDim1 * inDim2 * numChannels * imHeight * imWidth);
    int d1 = (idx / (inDim2 * numChannels * imHeight * imWidth)) % inDim1;
    int d2 = (idx / (numChannels * imHeight * imWidth)) % inDim2;
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    // Get input value - exact same indexing as reference
    Tin inDataVal = inData[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + (h*inPitch) + w];
    
    // Standard sigmoid computation: 1.0/(1.0+exp(-inDataVal))
    Tout out = (Tout)(1.0f / (1.0f + expf(-(float)inDataVal)));
    
    // Store result - exact same indexing as reference
    outData[(b*outBatchPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + (c*outChPitch) + (h*outPitch) + w] = out;
}

// CUDA wrapper for Float Sigmoid using persistent memory
template <class Tin, class Tout>
int TIDL_cudaFloatSigmoid(
    const Tin* input,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch)
{
    // Calculate memory sizes
    size_t input_size = numTotBatches * inBatchPitch * sizeof(Tin);
    size_t output_size = numTotBatches * outBatchPitch * sizeof(Tout);

    Tin* dpInput = NULL;
    Tout* dpOutput = NULL;

    void* inPtrs[1] = {(void*)input};
    uint32_t inDataSizes[1] = {(uint32_t)input_size};
    TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, 1, inDataSizes);

    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&dpOutput, output_size);

    // Launch kernel
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    FloatSigmoidKernel<Tin, Tout><<<gridSize, THREADS_PER_BLOCK>>>(
        dpInput, dpOutput,
        numTotBatches, numChannels, imWidth, imHeight, inDim1, inDim2,
        inBatchPitch, inChPitch, inDim1Pitch, inDim2Pitch, inPitch,
        outBatchPitch, outChPitch, outDim1Pitch, outDim2Pitch, outPitch);

    checkCudaErr(cudaGetLastError());
    
    void* outPtrs[1] = {(void*)output};
    uint32_t outDataSizes[1] = {(uint32_t)output_size};
    TIDL_cudaMemManagerPostLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), outPtrs, 1, outDataSizes);
    
    return IALG_EOK;
}

// Non-Linear LUT computation kernel - matches reference loop structure exactly
template <class Tin, class Tout>
__global__ void NonLinearLUTKernel(
    const Tin* __restrict__ inData,
    Tout* __restrict__ outData,
    const Tout* __restrict__ lutTable,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    int readOffsetLUT)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (inDim1 * inDim2 * numChannels * imHeight * imWidth);
    int d1 = (idx / (inDim2 * numChannels * imHeight * imWidth)) % inDim1;
    int d2 = (idx / (numChannels * imHeight * imWidth)) % inDim2;
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    Tin inDataVal = inData[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + (h*inPitch) + w];
    
    // LUT lookup with offset
    Tout out = lutTable[inDataVal + readOffsetLUT];
    
    // Store result - exact same indexing as reference
    outData[(b*outBatchPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + (c*outChPitch) + (h*outPitch) + w] = out;
}

// CUDA wrapper for Non-Linear LUT using persistent memory
template <class Tin, class Tout>
int TIDL_cudaNonLinearLUT(
    const Tin* input,
    Tout* output,
    const Tout* lutTable,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    int readOffsetLUT)
{
    // Calculate memory sizes
    size_t input_size = numTotBatches * inBatchPitch * sizeof(Tin);
    size_t output_size = numTotBatches * outBatchPitch * sizeof(Tout);

    Tin* dpInput = NULL;
    Tout* dpOutput = NULL;
    Tout* d_lutTable = NULL;

    

    void* inPtrs[1] = {(void*)input};
    uint32_t inDataSizes[1] = {(uint32_t)input_size};
    TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, 1, inDataSizes);

    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&dpOutput, output_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), lutTable, (void**)&d_lutTable, NULL);

    // Launch kernel
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    NonLinearLUTKernel<Tin, Tout><<<gridSize, THREADS_PER_BLOCK>>>(
        dpInput, dpOutput, d_lutTable,
        numTotBatches, numChannels, imWidth, imHeight, inDim1, inDim2,
        inBatchPitch, inChPitch, inDim1Pitch, inDim2Pitch, inPitch,
        outBatchPitch, outChPitch, outDim1Pitch, outDim2Pitch, outPitch,
        readOffsetLUT);

    checkCudaErr(cudaGetLastError());
    
    void* outPtrs[1] = {(void*)output};
    uint32_t outDataSizes[1] = {(uint32_t)output_size};
    TIDL_cudaMemManagerPostLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), outPtrs, 1, outDataSizes);

    return IALG_EOK;
}

// CUDA version of TIDL_functionWithLookup - using template type traits for saturation
template <class Tin, class Tout>
__device__ Tout cuda_functionWithLookup(int32_t f1, int32_t f2, Tin quant_input, 
                                        Tout* new_quant_lut, int32_t inoffset)
{
    uint16_t quant_int = quant_input + inoffset;
    uint8_t quant_int_shifted = quant_int >> 8;
    uint8_t quant_int_frac = quant_int & 0xFF;

    int16_t b = (int16_t)new_quant_lut[quant_int_shifted * 2];
    Tout c = new_quant_lut[(quant_int_shifted * 2) + 1];
    uint16_t x = quant_int_frac;

    int32_t b_x = b * x;
    
    // Handle f1 scaling - exact same logic as reference
    if (f1 > 0) 
    {
        b_x = b_x << ((f1 <= 8) ? f1 : 8);
    }
    else
    {
        b_x = b_x >> ((f1 >= -24) ? (-f1) : 24);
    }

    int32_t interp_val = (b_x + c);
    
    // Handle f2 scaling - exact same logic as reference
    if (f2 > 0) 
    {
        interp_val = interp_val << ((f2 <= 15) ? f2 : 15);
    } 
    else 
    {
        interp_val = interp_val >> ((f2 > -17) ? (-f2) : 17);
    }

    // Apply saturation using template type traits
    int32_t sat_min = (int32_t)cuda::std::numeric_limits<Tout>::lowest();
    int32_t sat_max = (int32_t)cuda::std::numeric_limits<Tout>::max();
    interp_val = max(sat_min, min(sat_max, interp_val));
    
    return (Tout)interp_val;
}

// Non-Linear Interpolation LUT computation kernel - matches reference loop structure exactly
template <class Tin, class Tout>
__global__ void NonLinearInterpolLUTKernel(
    const Tin* __restrict__ inData,
    Tout* __restrict__ outData,
    const Tout* __restrict__ lutTable,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    int32_t f1, int32_t f2, int32_t inoffset)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    // Mapping: b=i5, d1=i4, d2=i3, c=i2, h=i0, w=i1 (matching reference implementation)
    int b = idx / (inDim1 * inDim2 * numChannels * imHeight * imWidth);
    int d1 = (idx / (inDim2 * numChannels * imHeight * imWidth)) % inDim1;
    int d2 = (idx / (numChannels * imHeight * imWidth)) % inDim2;
    int c = (idx / (imHeight * imWidth)) % numChannels;
    int h = (idx / imWidth) % imHeight;
    int w = idx % imWidth;

    // Get input value - exact same indexing as reference
    Tin inDataVal = inData[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + (h*inPitch) + w];
    
    // Perform interpolation lookup using CUDA device function
    Tout out = cuda_functionWithLookup<Tin, Tout>(f1, f2, inDataVal, (Tout*)lutTable, inoffset);
    
    // Store result - exact same indexing as reference
    outData[(b*outBatchPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + (c*outChPitch) + (h*outPitch) + w] = out;
}

// CUDA wrapper for Non-Linear Interpolation LUT using persistent memory
template <class Tin, class Tout>
int TIDL_cudaNonLinearInterpolLUT(
    const Tin* input,
    Tout* output,
    const Tout* lutTable,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    int32_t f1, int32_t f2, int32_t inoffset)
{

    // Calculate memory sizes
    size_t input_size = numTotBatches * inBatchPitch * sizeof(Tin);
    size_t output_size = numTotBatches * outBatchPitch * sizeof(Tout);

    Tin* dpInput = NULL;
    Tout* dpOutput = NULL;
    Tout* d_lutTable = NULL;

    void* inPtrs[1] = {(void*)input};
    uint32_t inDataSizes[1] = {(uint32_t)input_size};
    TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, 1, inDataSizes);

    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&dpInput, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&dpOutput, output_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), lutTable, (void**)&d_lutTable, NULL);

    // Launch kernel
    int total_elements = numTotBatches * inDim1 * inDim2 * numChannels * imHeight * imWidth;
    int gridSize = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    NonLinearInterpolLUTKernel<Tin, Tout><<<gridSize, THREADS_PER_BLOCK>>>(
        dpInput, dpOutput, d_lutTable,
        numTotBatches, numChannels, imWidth, imHeight, inDim1, inDim2,
        inBatchPitch, inChPitch, inDim1Pitch, inDim2Pitch, inPitch,
        outBatchPitch, outChPitch, outDim1Pitch, outDim2Pitch, outPitch,
        f1, f2, inoffset);

    checkCudaErr(cudaGetLastError());
    
    void* outPtrs[1] = {(void*)output};
    uint32_t outDataSizes[1] = {(uint32_t)output_size};
    TIDL_cudaMemManagerPostLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), outPtrs, 1, outDataSizes);

    return IALG_EOK;
}


template int TIDL_cudaBatchNormSaturation<int, signed char>(int const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNormSaturation<float, unsigned char>(float const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNorm<unsigned char, short, int, long>(unsigned char const*, short const*, short const*, int const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNormSaturation<int, unsigned char>(int const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNormSaturation<int, unsigned short>(int const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNorm<unsigned char, float, float, float>(unsigned char const*, float const*, float const*, float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNormSaturation<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNorm<unsigned short, signed char, short, int>(unsigned short const*, signed char const*, signed char const*, short const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNormSaturation<long, float>(long const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNormSaturation<float, signed char>(float const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNormSaturation<int, short>(int const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNorm<signed char, signed char, short, int>(signed char const*, signed char const*, signed char const*, short const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNormSaturation<long, unsigned short>(long const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNormSaturation<int, float>(int const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNormSaturation<long, unsigned char>(long const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNorm<short, float, float, float>(short const*, float const*, float const*, float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<unsigned char, signed char, short, int>(unsigned char const*, signed char const*, signed char const*, short const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<signed char, float, float, float>(signed char const*, float const*, float const*, float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<unsigned short, float, float, float>(unsigned short const*, float const*, float const*, float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNormSaturation<long, signed char>(long const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNorm<short, signed char, short, int>(short const*, signed char const*, signed char const*, short const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<unsigned short, short, int, long>(unsigned short const*, short const*, short const*, int const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNormSaturation<float, short>(float const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNorm<signed char, short, int, long>(signed char const*, short const*, short const*, int const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<short, short, int, long>(short const*, short const*, short const*, int const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<float, signed char, short, int>(float const*, signed char const*, signed char const*, short const*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<float, short, int, long>(float const*, short const*, short const*, int const*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNorm<float, float, float, float>(float const*, float const*, float const*, float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaBatchNormSaturation<long, short>(long const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaBatchNormSaturation<float, unsigned short>(float const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);


template int TIDL_cudaHighAccuracySigmoid<short, unsigned short>(short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaHighAccuracySigmoid<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaHighAccuracySigmoid<signed char, unsigned char>(signed char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaHighAccuracySigmoid<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float);

template int TIDL_cudaFloatSigmoid<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

template int TIDL_cudaNonLinearLUT<unsigned short, unsigned short>(unsigned short const*, unsigned short*, unsigned short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearLUT<unsigned char, signed char>(unsigned char const*, signed char*, signed char const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearLUT<unsigned short, short>(unsigned short const*, short*, short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearLUT<unsigned char, unsigned char>(unsigned char const*, unsigned char*, unsigned char const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearLUT<signed char, unsigned char>(signed char const*, unsigned char*, unsigned char const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearLUT<short, unsigned short>(short const*, unsigned short*, unsigned short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearLUT<short, short>(short const*, short*, short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearLUT<signed char, signed char>(signed char const*, signed char*, signed char const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

template int TIDL_cudaNonLinearInterpolLUT<short, unsigned short>(short const*, unsigned short*, unsigned short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearInterpolLUT<unsigned short, short>(unsigned short const*, short*, short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearInterpolLUT<short, short>(short const*, short*, short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonLinearInterpolLUT<unsigned short, unsigned short>(unsigned short const*, unsigned short*, unsigned short const*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);