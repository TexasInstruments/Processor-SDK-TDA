/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaLogical.cu
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
#include <chrono>


// Logical kernel

template <class Tin, class Tout>
__global__ void LogicalOpKernel(
    const Tin* __restrict__ inDataA,
    const Tin* __restrict__ inDataB,
    Tout* __restrict__ output,
    int32_t numOutBatches,  int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels, int32_t outHeight, int32_t outWidth, 
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA, int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t inbatchPitchB, int32_t inDIM1PitchB, int32_t inDIM2PitchB, int32_t inchPitchB, int32_t inheightPitchB,int32_t inwidthPitchB,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch, int32_t outheightPitch, int32_t outwidthPitch, int32_t logicalOperatorType)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    int b = idx / (numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth);
    int d1 = (idx / (numOutDIM2 * numOutChannels * outHeight * outWidth)) % numOutDIM1;
    int d2 = (idx / (numOutChannels * outHeight * outWidth)) % numOutDIM2;
    int c = (idx / (outHeight * outWidth)) % numOutChannels;
    int h = (idx / (outWidth)) % outHeight;
    int w = idx % outWidth;

    // Calculate offsets - exact same as reference
    int32_t inOffsetA = (b * inbatchPitchA) + (d1 * inDIM1PitchA) + (d2 * inDIM2PitchA) + (c * inchPitchA) + (h * inheightPitchA) + (w * inwidthPitchA);
    int32_t inOffsetB = (b * inbatchPitchB) + (d1 * inDIM1PitchB) + (d2 * inDIM2PitchB) + (c * inchPitchB) + (h * inheightPitchB) + (w * inwidthPitchB);
    int32_t outOffset = (b * outbatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outchPitch) + (h * outheightPitch) + (w * outwidthPitch);
    
    if (logicalOperatorType == TIDL_And)
    {
        output[outOffset] = (inDataA[inOffsetA] && inDataB[inOffsetB]);
    }
    else if (logicalOperatorType == TIDL_Or)
    {
        output[outOffset] = (inDataA[inOffsetA] || inDataB[inOffsetB]);
    }
    else if (logicalOperatorType == TIDL_Xor)
    {
        output[outOffset] = (inDataA[inOffsetA] ^ inDataB[inOffsetB]);
    }
    
}

template <class Tin, class Tout>
__global__ void ComparisonOPKernel(
    const Tin* __restrict__ inDataA,
    const Tin* __restrict__ inDataB,
    Tout* __restrict__ output,
    int32_t numOutBatches,  int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels, int32_t outHeight, int32_t outWidth, 
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA, int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t inbatchPitchB, int32_t inDIM1PitchB, int32_t inDIM2PitchB, int32_t inchPitchB, int32_t inheightPitchB,int32_t inwidthPitchB,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch, int32_t outheightPitch, int32_t outwidthPitch, int32_t comparisonOperatorType)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    int b = idx / (numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth);
    int d1 = (idx / (numOutDIM2 * numOutChannels * outHeight * outWidth)) % numOutDIM1;
    int d2 = (idx / (numOutChannels * outHeight * outWidth)) % numOutDIM2;
    int c = (idx / (outHeight * outWidth)) % numOutChannels;
    int h = (idx / (outWidth)) % outHeight;
    int w = idx % outWidth;

    // Calculate offsets - exact same as reference
    int32_t inOffsetA = (b * inbatchPitchA) + (d1 * inDIM1PitchA) + (d2 * inDIM2PitchA) + (c * inchPitchA) + (h * inheightPitchA) + (w * inwidthPitchA);
    int32_t inOffsetB = (b * inbatchPitchB) + (d1 * inDIM1PitchB) + (d2 * inDIM2PitchB) + (c * inchPitchB) + (h * inheightPitchB) + (w * inwidthPitchB);
    int32_t outOffset = (b * outbatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outchPitch) + (h * outheightPitch) + (w * outwidthPitch);
    
    if (comparisonOperatorType == TIDL_Equal)
    {
        output[outOffset] = (Tout)(inDataA[inOffsetA] == inDataB[inOffsetB]);
    }
    else if (comparisonOperatorType == TIDL_Greater)
    {
        output[outOffset] = (Tout)(inDataA[inOffsetA] > inDataB[inOffsetB]);
    }
    else if (comparisonOperatorType == TIDL_GreaterOrEqual)
    {
        output[outOffset] = (Tout)(inDataA[inOffsetA] >= inDataB[inOffsetB]);
    }
    else if (comparisonOperatorType == TIDL_Less)
    {
        output[outOffset] = (Tout)(inDataA[inOffsetA] < inDataB[inOffsetB]);
    }
    else if (comparisonOperatorType == TIDL_LessOrEqual)
    {
        output[outOffset] = (Tout)(inDataA[inOffsetA] <= inDataB[inOffsetB]);
    }
    
}

// Logical And kernel - matches reference loop structure exactly (no branching)
template <class Tin>
__global__ void WhereOpKernel(
    const Tin* __restrict__ inDataA,
    const Tin* __restrict__ inDataB,
    const bool* __restrict__ inPtrCondition,
    Tin* __restrict__ output,
    int32_t numOutBatches,  int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels, int32_t outHeight, int32_t outWidth, 
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA, int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t inbatchPitchB, int32_t inDIM1PitchB, int32_t inDIM2PitchB, int32_t inchPitchB, int32_t inheightPitchB,int32_t inwidthPitchB,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch, int32_t outheightPitch, int32_t outwidthPitch,
    int32_t inbatchPitchCondition, int32_t inDIM1PitchCondition, int32_t inDIM2PitchCondition, int32_t inchPitchCondition, int32_t inheightPitchCondition, int32_t inwidthPitchCondition)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (dim1, dim2, channel, height, width)
    int b = idx / (numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth);
    int d1 = (idx / (numOutDIM2 * numOutChannels * outHeight * outWidth)) % numOutDIM1;
    int d2 = (idx / (numOutChannels * outHeight * outWidth)) % numOutDIM2;
    int c = (idx / (outHeight * outWidth)) % numOutChannels;
    int h = (idx / (outWidth)) % outHeight;
    int w = idx % outWidth;

    // Calculate offsets - exact same as reference
    int32_t inOffsetCondition = (b * inbatchPitchCondition) + (d1 * inDIM1PitchCondition) + (d2 * inDIM2PitchCondition) + (c * inchPitchCondition) + (h * inheightPitchCondition) + (w * inwidthPitchCondition);
    int32_t inOffsetA = (b * inbatchPitchA) + (d1 * inDIM1PitchA) + (d2 * inDIM2PitchA) + (c * inchPitchA) + (h * inheightPitchA) + (w * inwidthPitchA);
    int32_t inOffsetB = (b * inbatchPitchB) + (d1 * inDIM1PitchB) + (d2 * inDIM2PitchB) + (c * inchPitchB) + (h * inheightPitchB) + (w * inwidthPitchB);
    int32_t outOffset = (b * outbatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (c * outchPitch) + (h * outheightPitch) + (w * outwidthPitch);
    
    output[outOffset] = (inPtrCondition[inOffsetCondition]) ? inDataA[inOffsetA] : inDataB[inOffsetB];
   
}

// Unified CUDA wrapper for EltWise operations with stream optimization
template <class Tin, class Tout>
int TIDL_cudaLogicalOp(
    const Tin* inputA,
    const Tin* inputB,
    Tout* output,
    int32_t numOutBatches,  int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels, int32_t outHeight, int32_t outWidth,
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA, int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t inbatchPitchB, int32_t inDIM1PitchB, int32_t inDIM2PitchB, int32_t inchPitchB, int32_t inheightPitchB,int32_t inwidthPitchB,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch, int32_t outheightPitch, int32_t outwidthPitch,
    int32_t logicalOperatorType, int32_t inputA_size, int32_t inputB_size)
{
    Tin* d_inputA = NULL;
    Tin* d_inputB = NULL;
    Tout* d_output = NULL;

    size_t output_size = (1 + (numOutBatches  - 1) * outbatchPitch
                            + (numOutDIM1     - 1) * outDIM1Pitch
                            + (numOutDIM2     - 1) * outDIM2Pitch
                            + (numOutChannels - 1) * outchPitch
                            + (outHeight      - 1) * outheightPitch
                            + (outWidth       - 1) * outwidthPitch) * sizeof(Tout);

    // Get GPU pointers after synchronization
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
        {
            printf("Logical: Unable to find inputA pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inputB, (void**)&d_inputB, inputB_size * sizeof(Tin)) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inputB, (void**)&d_inputB, inputB_size * sizeof(Tin)) != IALG_EOK)
        {
            printf("Logical: Unable to find inputB pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Logical: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_inputA, inputA, inputA_size * sizeof(Tin), cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(d_inputB, inputB, inputB_size * sizeof(Tin), cudaMemcpyHostToDevice, stream));
    
    // Launch appropriate kernel based on operation type
    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    LogicalOpKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
    d_inputA, d_inputB, d_output,
    numOutBatches,  numOutDIM1, numOutDIM2, numOutChannels, outHeight, outWidth,
    inbatchPitchA, inDIM1PitchA, inDIM2PitchA, inchPitchA, inheightPitchA,inwidthPitchA,
    inbatchPitchB, inDIM1PitchB, inDIM2PitchB, inchPitchB, inheightPitchB,inwidthPitchB,
    outbatchPitch, outDIM1Pitch, outDIM2Pitch, outchPitch, outheightPitch,outwidthPitch, logicalOperatorType);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

// Stream-optimized comparison operator function
template <class Tin, class Tout>
int TIDL_cudaComparisonOperator(
    const Tin* inputA,
    const Tin* inputB,
    Tout* output,
    int32_t numOutBatches,  int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels, int32_t outHeight, int32_t outWidth,
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA, int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t inbatchPitchB, int32_t inDIM1PitchB, int32_t inDIM2PitchB, int32_t inchPitchB, int32_t inheightPitchB,int32_t inwidthPitchB,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch, int32_t outheightPitch, int32_t outwidthPitch,
    int32_t comparisonOperatorType, int32_t inputA_size, int32_t inputB_size)
{
    Tin* d_inputA = NULL;
    Tin* d_inputB = NULL;
    Tout* d_output = NULL;

    size_t output_size = (1 + (numOutBatches  - 1) * outbatchPitch
                            + (numOutDIM1     - 1) * outDIM1Pitch
                            + (numOutDIM2     - 1) * outDIM2Pitch
                            + (numOutChannels - 1) * outchPitch
                            + (outHeight      - 1) * outheightPitch
                            + (outWidth       - 1) * outwidthPitch) * sizeof(Tout);

    // Get GPU pointers after synchronization
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
        {
            printf("Logical: Unable to find inputA pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inputB, (void**)&d_inputB, inputB_size * sizeof(Tin)) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inputB, (void**)&d_inputB, inputB_size * sizeof(Tin)) != IALG_EOK)
        {
            printf("Logical: Unable to find inputB pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Logical: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_inputA, inputA, inputA_size * sizeof(Tin), cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(d_inputB, inputB, inputB_size * sizeof(Tin), cudaMemcpyHostToDevice, stream));

    // Launch appropriate kernel based on operation type
    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    
    ComparisonOPKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
    d_inputA, d_inputB, d_output,
    numOutBatches,  numOutDIM1, numOutDIM2, numOutChannels, outHeight, outWidth,
    inbatchPitchA, inDIM1PitchA, inDIM2PitchA, inchPitchA, inheightPitchA,inwidthPitchA,
    inbatchPitchB, inDIM1PitchB, inDIM2PitchB, inchPitchB, inheightPitchB,inwidthPitchB,
    outbatchPitch, outDIM1Pitch, outDIM2Pitch, outchPitch, outheightPitch,outwidthPitch, comparisonOperatorType);


    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));
  
    // Results stay in GPU accumulator for quantize wrapper to use
    return IALG_EOK;
}

// Unified CUDA wrapper for EltWise operations with stream optimization
template <class Tin>
int TIDL_cudaWhereOperator(
    const Tin* inputA,
    const Tin* inputB,
    bool* inPtrCondition,
    Tin* output,
    int32_t numOutBatches,  int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels, int32_t outHeight, int32_t outWidth, 
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA, int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t inbatchPitchB, int32_t inDIM1PitchB, int32_t inDIM2PitchB, int32_t inchPitchB, int32_t inheightPitchB,int32_t inwidthPitchB,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch, int32_t outheightPitch, int32_t outwidthPitch, 
    int32_t inputA_size, int32_t inputB_size, int32_t inPtrCondition_size,
    int32_t inbatchPitchCondition, int32_t inDIM1PitchCondition, int32_t inDIM2PitchCondition, int32_t inchPitchCondition, int32_t inheightPitchCondition, int32_t inwidthPitchCondition)
{
    Tin* d_inputA = NULL;
    Tin* d_inputB = NULL;
    bool* d_inPtrCondition = NULL;
    Tin* d_output = NULL;

    size_t output_size = (1 + (numOutBatches  - 1) * outbatchPitch
                            + (numOutDIM1     - 1) * outDIM1Pitch
                            + (numOutDIM2     - 1) * outDIM2Pitch
                            + (numOutChannels - 1) * outchPitch
                            + (outHeight      - 1) * outheightPitch
                            + (outWidth       - 1) * outwidthPitch) * sizeof(Tin);

    // Get GPU pointers after synchronization
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
        {
            printf("Logical: Unable to find inputA pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inputB, (void**)&d_inputB, inputB_size * sizeof(Tin)) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inputB, (void**)&d_inputB, inputB_size * sizeof(Tin)) != IALG_EOK)
        {
            printf("Logical: Unable to find inputB pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inPtrCondition, (void**)&d_inPtrCondition, inPtrCondition_size * sizeof(bool)) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inPtrCondition, (void**)&d_inPtrCondition, inPtrCondition_size * sizeof(bool)) != IALG_EOK)
        {
            printf("Logical: Unable to find inPtrCondition pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Logical: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_inputA, inputA, inputA_size * sizeof(Tin), cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(d_inputB, inputB, inputB_size * sizeof(Tin), cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(d_inPtrCondition, inPtrCondition, inPtrCondition_size * sizeof(bool), cudaMemcpyHostToDevice, stream));

    // Launch appropriate kernel based on operation type
    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    WhereOpKernel<Tin><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
    d_inputA, d_inputB, d_inPtrCondition, d_output,
    numOutBatches,  numOutDIM1, numOutDIM2, numOutChannels, outHeight, outWidth,
    inbatchPitchA, inDIM1PitchA, inDIM2PitchA, inchPitchA, inheightPitchA,inwidthPitchA,
    inbatchPitchB, inDIM1PitchB, inDIM2PitchB, inchPitchB, inheightPitchB,inwidthPitchB,
    outbatchPitch, outDIM1Pitch, outDIM2Pitch, outchPitch, outheightPitch,outwidthPitch,
    inbatchPitchCondition, inDIM1PitchCondition, inDIM2PitchCondition, inchPitchCondition, inheightPitchCondition, inwidthPitchCondition);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}


// Not / IsInf / IsNaN kernel — single input, bool output
template <class Tin>
__global__ void NotIsInfIsNanKernel(
    const Tin* __restrict__ inDataA,
    bool* __restrict__ output,
    int32_t numOutBatches, int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels,
    int32_t outHeight, int32_t outWidth,
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA,
    int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch,
    int32_t outheightPitch, int32_t outwidthPitch,
    int32_t operatorType, int32_t detect_negative, int32_t detect_positive)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;

    if (idx >= total_elements) return;

    int b  = idx / (numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth);
    int d1 = (idx / (numOutDIM2 * numOutChannels * outHeight * outWidth)) % numOutDIM1;
    int d2 = (idx / (numOutChannels * outHeight * outWidth)) % numOutDIM2;
    int c  = (idx / (outHeight * outWidth)) % numOutChannels;
    int h  = (idx / outWidth) % outHeight;
    int w  = idx % outWidth;

    int32_t inOffsetA = (b * inbatchPitchA) + (d1 * inDIM1PitchA) + (d2 * inDIM2PitchA)
                      + (c * inchPitchA)    + (h * inheightPitchA) + (w * inwidthPitchA);
    int32_t outOffset = (b * outbatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch)
                      + (c * outchPitch)    + (h * outheightPitch) + (w * outwidthPitch);

    if (operatorType == TIDL_Not)
    {
        output[outOffset] = !inDataA[inOffsetA];
    }
    else if (operatorType == TIDL_IsInf)
    {
        float val = (float)inDataA[inOffsetA];
        if (detect_negative && detect_positive)
            output[outOffset] = isinf(val);
        else if (!detect_negative && detect_positive)
            output[outOffset] = isinf(val) && (val > 0.0f);
        else if (detect_negative && !detect_positive)
            output[outOffset] = isinf(val) && (val < 0.0f);
        else
            output[outOffset] = false;
    }
    else if (operatorType == TIDL_IsNaN)
    {
        output[outOffset] = isnan((float)inDataA[inOffsetA]);
    }
}

// CUDA wrapper for Not / IsInf / IsNaN
template <class Tin>
int TIDL_cudaNotIsInfIsNan(
    const Tin* inputA,
    bool* output,
    int32_t numOutBatches, int32_t numOutDIM1, int32_t numOutDIM2, int32_t numOutChannels,
    int32_t outHeight, int32_t outWidth,
    int32_t inbatchPitchA, int32_t inDIM1PitchA, int32_t inDIM2PitchA, int32_t inchPitchA,
    int32_t inheightPitchA, int32_t inwidthPitchA,
    int32_t outbatchPitch, int32_t outDIM1Pitch, int32_t outDIM2Pitch, int32_t outchPitch,
    int32_t outheightPitch, int32_t outwidthPitch,
    int32_t operatorType, int32_t detect_negative, int32_t detect_positive,
    int32_t inputA_size)
{
    Tin*  d_inputA = NULL;
    bool* d_output = NULL;

    size_t output_size = (size_t)(1 + (numOutBatches  - 1) * outbatchPitch
                                    + (numOutDIM1     - 1) * outDIM1Pitch
                                    + (numOutDIM2     - 1) * outDIM2Pitch
                                    + (numOutChannels - 1) * outchPitch
                                    + (outHeight      - 1) * outheightPitch
                                    + (outWidth       - 1) * outwidthPitch) * sizeof(bool);

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inputA, (void**)&d_inputA, inputA_size * sizeof(Tin)) != IALG_EOK)
        {
            printf("NotIsInfIsNan: Unable to find inputA pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("NotIsInfIsNan: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_inputA, inputA, inputA_size * sizeof(Tin), cudaMemcpyHostToDevice, stream));

    int total_elements = numOutBatches * numOutDIM1 * numOutDIM2 * numOutChannels * outHeight * outWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    NotIsInfIsNanKernel<Tin><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_inputA, d_output,
        numOutBatches, numOutDIM1, numOutDIM2, numOutChannels, outHeight, outWidth,
        inbatchPitchA, inDIM1PitchA, inDIM2PitchA, inchPitchA, inheightPitchA, inwidthPitchA,
        outbatchPitch, outDIM1Pitch, outDIM2Pitch, outchPitch, outheightPitch, outwidthPitch,
        operatorType, detect_negative, detect_positive);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_cudaLogicalOp<bool, bool>(bool const*, bool const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<unsigned short, bool>(unsigned short const*, unsigned short const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<unsigned char, bool>(unsigned char const*, unsigned char const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<signed char, bool>(signed char const*, signed char const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<signed short, bool>(signed short const*, signed short const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<signed int, bool>(signed int const*, signed int const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<unsigned int, bool>(unsigned int const*, unsigned int const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<float, bool>(float const*, float const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaComparisonOperator<bool, bool>(bool const*, bool const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

template int TIDL_cudaWhereOperator<unsigned short>(unsigned short const*, unsigned short const*, bool*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaWhereOperator<unsigned char>(unsigned char const*, unsigned char const*, bool*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaWhereOperator<signed char>(signed char const*, signed char const*, bool*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaWhereOperator<signed short>(signed short const*, signed short const*, bool*, signed short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaWhereOperator<signed int>(signed int const*, signed int const*, bool*, signed int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaWhereOperator<unsigned int>(unsigned int const*, unsigned int const*, bool*, unsigned int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaWhereOperator<float>(float const*, float const*, bool*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaWhereOperator<bool>(bool const*, bool const*, bool*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

template int TIDL_cudaNotIsInfIsNan<bool>(bool const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNotIsInfIsNan<float>(float const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
