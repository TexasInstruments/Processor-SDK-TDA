/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaArgOp.cu
 @brief   This file contains CUDA implementation of ArgOp layer.
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

using namespace floating_point::bf16_mma;

// ArgOp Max kernel
template <class Tin, class Tout>
__global__ void ArgOpMaxKernel(
    const Tin* __restrict__ input,
    Tout* __restrict__ output,
    int numTotRoi, int inDIM1, int inDIM2, int numChannels, int inHeight, int inWidth,
    int inBatchPitch, int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch, int selectLastIndex)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numTotRoi * inDIM1 * inDIM2 * inHeight * inWidth;

    if (idx >= total_elements) return;

    int b  = idx / (inDIM1 * inDIM2 * inHeight * inWidth);
    int d1 = (idx / (inDIM2 * inHeight * inWidth)) % inDIM1;
    int d2 = (idx / (inHeight * inWidth)) % inDIM2;
    int h  = (idx / inWidth) % inHeight;
    int w  = idx % inWidth;

    // Shared tile: THREADS_PER_BLOCK_Y channels loaded per round
    __shared__ Tin channel_tile[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    Tin  maxVal;
    int  maxIdx = 0;
    bool first  = true;

    // Process channels in tiles of THREADS_PER_BLOCK_Y
    for (int cBase = 0; cBase < numChannels; cBase += THREADS_PER_BLOCK_Y)
    {
        // All ty threads cooperatively load one tile
        int c = cBase + ty;
        if (c < numChannels)
        {
            channel_tile[tx][ty] = input[(b * inBatchPitch) + (d1 * inDIM1Pitch) +
                                         (d2 * inDIM2Pitch) + (c * inChPitch) +
                                         (h * inPitch) + w];
        }
        __syncthreads();

        // Only ty==0 scans the tile sequentially — identical to C ref loop
        if (ty == 0)
        {
            int tileLen = min(THREADS_PER_BLOCK_Y, numChannels - cBase);
            for (int t = 0; t < tileLen; t++)
            {
                Tin val = channel_tile[tx][t];
                int c_global = cBase + t;
                if (first)
                {
                    maxVal = val;
                    maxIdx = c_global;
                    first  = false;
                }
                else if (val > maxVal)
                {
                    maxVal = val;
                    maxIdx = c_global;
                }
                else if (val == maxVal && selectLastIndex == 1)
                {
                    maxIdx = c_global;
                }
            }
        }
        __syncthreads();
    }

    if (ty == 0)
    {
        int outOffset = (b * outBatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (h * outPitch) + w;
        output[outOffset] = (Tout)maxIdx;
    }
}

// ArgOp Min kernel
template <class Tin, class Tout>
__global__ void ArgOpMinKernel(
    const Tin* __restrict__ input,
    Tout* __restrict__ output,
    int numTotRoi, int inDIM1, int inDIM2, int numChannels, int inHeight, int inWidth,
    int inBatchPitch, int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch, int selectLastIndex)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numTotRoi * inDIM1 * inDIM2 * inHeight * inWidth;

    if (idx >= total_elements) return;

    int b  = idx / (inDIM1 * inDIM2 * inHeight * inWidth);
    int d1 = (idx / (inDIM2 * inHeight * inWidth)) % inDIM1;
    int d2 = (idx / (inHeight * inWidth)) % inDIM2;
    int h  = (idx / inWidth) % inHeight;
    int w  = idx % inWidth;

    // Shared tile: THREADS_PER_BLOCK_Y channels loaded per round
    __shared__ Tin channel_tile[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    Tin  minVal;
    int  minIdx = 0;
    bool first  = true;

    // Process channels in tiles of THREADS_PER_BLOCK_Y
    for (int cBase = 0; cBase < numChannels; cBase += THREADS_PER_BLOCK_Y)
    {
        // All ty threads cooperatively load one tile
        int c = cBase + ty;
        if (c < numChannels)
        {
            channel_tile[tx][ty] = input[(b * inBatchPitch) + (d1 * inDIM1Pitch) +
                                         (d2 * inDIM2Pitch) + (c * inChPitch) +
                                         (h * inPitch) + w];
        }
        __syncthreads();

        // Only ty==0 scans the tile sequentially — identical to C ref loop
        if (ty == 0)
        {
            int tileLen = min(THREADS_PER_BLOCK_Y, numChannels - cBase);
            for (int t = 0; t < tileLen; t++)
            {
                Tin val = channel_tile[tx][t];
                int c_global = cBase + t;
                if (first)
                {
                    minVal = val;
                    minIdx = c_global;
                    first  = false;
                }
                else if (val < minVal)
                {
                    minVal = val;
                    minIdx = c_global;
                }
                else if (val == minVal && selectLastIndex == 1)
                {
                    minIdx = c_global;
                }
            }
        }
        __syncthreads();
    }

    if (ty == 0)
    {
        int outOffset = (b * outBatchPitch) + (d1 * outDIM1Pitch) + (d2 * outDIM2Pitch) + (h * outPitch) + w;
        output[outOffset] = (Tout)minIdx;
    }
}

// Unified CUDA wrapper for ArgOP Max operations
template <class Tin, class Tout>
int TIDL_cudaArgOpMax(
    const Tin* input,
    Tout* output,
    uint32_t numTotRoi, uint32_t inDIM1, uint32_t inDIM2, uint32_t numChannels, uint32_t inHeight, uint32_t inWidth,
    uint32_t inBatchPitch, uint32_t inDIM1Pitch, uint32_t inDIM2Pitch, uint32_t inChPitch, uint32_t inPitch,
    uint32_t outBatchPitch, uint32_t outDIM1Pitch, uint32_t outChPitch, uint32_t outPitch, int32_t selectLastIndex)
{
    size_t input_size  = sizeof(Tin)  * numTotRoi * inBatchPitch;
    size_t output_size = sizeof(Tout) * numTotRoi * outBatchPitch;

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("ArgOp: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("ArgOp: Unable to allocate output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    // total_elements = output shape (no channel dimension)
    int total_elements = numTotRoi * inDIM1 * inDIM2 * inHeight * inWidth;
    dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    // outChPitch passed from caller is outDIM2Pitch in ref (DIM2 is the axis after DIM1)
    ArgOpMaxKernel<Tin, Tout><<<gridDim, blockDim, 0, stream>>>(
        d_input, d_output,
        numTotRoi, inDIM1, inDIM2, numChannels, inHeight, inWidth,
        inBatchPitch, inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch,
        outBatchPitch, outDIM1Pitch, outChPitch, outPitch, selectLastIndex);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

// Unified CUDA wrapper for ArgOP Min operations
template <class Tin, class Tout>
int TIDL_cudaArgOpMin(
    const Tin* input,
    Tout* output,
    uint32_t numTotRoi, uint32_t inDIM1, uint32_t inDIM2, uint32_t numChannels, uint32_t inHeight, uint32_t inWidth,
    uint32_t inBatchPitch, uint32_t inDIM1Pitch, uint32_t inDIM2Pitch, uint32_t inChPitch, uint32_t inPitch,
    uint32_t outBatchPitch, uint32_t outDIM1Pitch, uint32_t outChPitch, uint32_t outPitch, int32_t selectLastIndex)
{
    size_t input_size  = sizeof(Tin)  * numTotRoi * inBatchPitch;
    size_t output_size = sizeof(Tout) * numTotRoi * outBatchPitch;

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("ArgOp: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("ArgOp: Unable to allocate output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    // total_elements = output shape (no channel dimension)
    int total_elements = numTotRoi * inDIM1 * inDIM2 * inHeight * inWidth;
    dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    // outChPitch passed from caller is outDIM2Pitch in ref (DIM2 is the axis after DIM1)
    ArgOpMinKernel<Tin, Tout><<<gridDim, blockDim, 0, stream>>>(
        d_input, d_output,
        numTotRoi, inDIM1, inDIM2, numChannels, inHeight, inWidth,
        inBatchPitch, inDIM1Pitch, inDIM2Pitch, inChPitch, inPitch,
        outBatchPitch, outDIM1Pitch, outChPitch, outPitch, selectLastIndex);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_cudaArgOpMax<signed char, unsigned char>(signed char const*, unsigned char*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMax<short, unsigned short>(short const*, unsigned short*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMax<float, float>(float const*, float*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMax<unsigned char, unsigned char>(unsigned char const*, unsigned char*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMax<unsigned short, unsigned short>(unsigned short const*, unsigned short*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMax<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);


template int TIDL_cudaArgOpMin<signed char, unsigned char>(signed char const*, unsigned char*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMin<short, unsigned short>(short const*, unsigned short*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMin<float, float>(float const*, float*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMin<unsigned char, unsigned char>(unsigned char const*, unsigned char*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMin<unsigned short, unsigned short>(unsigned short const*, unsigned short*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
template int TIDL_cudaArgOpMin<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
