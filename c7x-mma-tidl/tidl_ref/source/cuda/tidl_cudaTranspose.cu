/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaTranspose.cu
 @brief   This file contains CUDA implementation of transpose layer.
 @version 0.1 (Sep 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

// Transpose kernel
template <class Tin, class Tout>
__global__ void TransposeKernel(
    const Tin* __restrict__ inData,
    Tout* __restrict__ outData,
    int inBatches, int inDIM1, int inDIM2, int inChannels, int inHeight, int inWidth,
    int inBatchPitch, int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int outDIM1, int outDIM2, int outChannels, int outHeight, int outWidth,
    int pp, int lp, int cp, int d1p, int d2p, int bp)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = inBatches * inDIM1 * inDIM2 * inChannels * inHeight * inWidth;
    
    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (inDIM1 * inDIM2 * inChannels * inHeight * inWidth);
    int d1 = (idx / (inDIM2 * inChannels * inHeight * inWidth)) % inDIM1;
    int d2 = (idx / (inChannels * inHeight * inWidth)) % inDIM2;
    int c = (idx / (inHeight * inWidth)) % inChannels;
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;

    // Calculate input offset using pitch values
    int32_t inOffset = (b * inBatchPitch) + (d1 * inDIM1Pitch) + (d2 * inDIM2Pitch) + (c * inChPitch) + (h * inPitch) + w;

    // Calculate target index using the provided stride values
    int targetIndex = (b * bp) + (d1 * d1p) + (d2 * d2p) + (c * cp) + (h * lp) + (w * pp);

    int bi = targetIndex / (outDIM1 * outDIM2 * outChannels * outHeight * outWidth);
    int d1i = (targetIndex / (outDIM2 * outChannels * outHeight * outWidth)) % outDIM1;
    int d2i = (targetIndex / (outChannels * outHeight * outWidth)) % outDIM2;
    int ci = (targetIndex / (outHeight * outWidth)) % outChannels;
    int hi = (targetIndex / outWidth) % outHeight;
    int wi = targetIndex % outWidth;
    
    // Calculate output offset using pitch values
    int32_t outOffset = (bi * outBatchPitch) + (d1i * outDIM1Pitch) + (d2i * outDIM2Pitch) + (ci * outChPitch) + (hi * outPitch) + wi;

    // Copy data
    outData[outOffset] = inData[inOffset];
}

// CUDA wrapper for Transpose
template <class Tin, class Tout>
int TIDL_cudaTranspose(
    const Tin* input,
    Tout* output,
    int inBatches, int outBatches, int inDIM1, int inDIM2, int inChannels, int inHeight, int inWidth,
    int inBatchPitch, int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int outDIM1, int outDIM2, int outChannels, int outHeight, int outWidth,
    int pp, int lp, int cp, int d1p, int d2p, int bp)
{
    // Calculate memory sizes
    size_t input_size = inBatches * inBatchPitch * sizeof(Tin);
    size_t output_size = outBatches * outBatchPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Transpose: Unable to find input pointer\n");
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Transpose: Unable to find output pointer\n");
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    // Launch kernel on stream
    int total_elements = inBatches * inDIM1 * inDIM2 * inChannels * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    TransposeKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_input, d_output,
        inBatches, inDIM1, inDIM2, inChannels, inHeight, inWidth,
        inBatchPitch, inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch,
        outBatchPitch, outDIM1Pitch, outDIM2Pitch, outChPitch, outPitch,
        outDIM1, outDIM2, outChannels, outHeight, outWidth,
        pp, lp, cp, d1p, d2p, bp);

    // Async D2H
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));
    
    return IALG_EOK;
}

template int TIDL_cudaTranspose<signed char, signed char>(signed char const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaTranspose<short, short>(short const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaTranspose<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaTranspose<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaTranspose<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaTranspose<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaTranspose<signed int ,signed int>(signed int const*, signed int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
