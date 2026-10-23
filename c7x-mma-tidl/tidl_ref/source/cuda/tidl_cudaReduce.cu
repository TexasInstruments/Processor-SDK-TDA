/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaReduce.cu
 @brief   This file contains CUDA implementation of Reduce layer.
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

template <class Tin, class Tout>
__global__ void ReduceKernelMin(
  const Tin*    __restrict__ inData,
  Tout*   __restrict__ outData,
  int32_t outWidth, int32_t outHeight, int32_t numChs, int32_t numDim2, int32_t numDim1, int32_t numROIs,
  int32_t inPitch, int32_t inChPitch, int32_t inDim2Pitch, int32_t inDim1Pitch, int32_t inROIPitch,
  int32_t outChPitch, int32_t outDim2Pitch, int32_t outDim1Pitch, int32_t outROIPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numROIs * numDim1 * numDim2 * numChs * outWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (batch, dim1, dim2, channel, width)
    int b = idx / (numDim1 * numDim2 * numChs * outWidth);
    int d1 = (idx / (numDim2 * numChs * outWidth)) % numDim1;
    int d2 = (idx / (numChs * outWidth)) % numDim2;
    int c = (idx / outWidth) % numChs;
    int w = idx % outWidth;

    __shared__ Tin reduction_min[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    Tin minVal = cuda::std::numeric_limits<Tin>::max();

    for (int h = ty; h < outHeight; h += THREADS_PER_BLOCK_Y)
    {
        Tin inputVal = inData[(b * inROIPitch) + (d1 * inDim1Pitch) + (d2 * inDim2Pitch) + (c * inChPitch) + (h * inPitch) + w];
        minVal = min(minVal, inputVal);
    }
    reduction_min[tx][ty] = minVal;

    for (int stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if (ty < stride)
            reduction_min[tx][ty] = min(reduction_min[tx][ty], reduction_min[tx][ty + stride]);
    }
    __syncthreads();

    if (ty == 0)
    {
        int outOffset = (b * outROIPitch) + (d1 * outDim1Pitch) + (d2 * outDim2Pitch) + (c * outChPitch) + w;
        outData[outOffset] = reduction_min[tx][0];
    }
}

template <class Tin, class Tout>
__global__ void ReduceKernelMax(
  const Tin*    __restrict__ inData,
  Tout*   __restrict__ outData,
  int32_t outWidth, int32_t outHeight, int32_t numChs, int32_t numDim2, int32_t numDim1, int32_t numROIs,
  int32_t inPitch, int32_t inChPitch, int32_t inDim2Pitch, int32_t inDim1Pitch, int32_t inROIPitch,
  int32_t outChPitch, int32_t outDim2Pitch, int32_t outDim1Pitch, int32_t outROIPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numROIs * numDim1 * numDim2 * numChs * outWidth;

    if (idx >= total_elements) return;

    // Convert linear index to 5D coordinates (batch, dim1, dim2, channel, width)
    int b = idx / (numDim1 * numDim2 * numChs * outWidth);
    int d1 = (idx / (numDim2 * numChs * outWidth)) % numDim1;
    int d2 = (idx / (numChs * outWidth)) % numDim2;
    int c = (idx / outWidth) % numChs;
    int w = idx % outWidth;

    __shared__ Tin reduction_max[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    Tin maxVal = cuda::std::numeric_limits<Tin>::lowest();

    for (int h = ty; h < outHeight; h += THREADS_PER_BLOCK_Y)
    {
        Tin inputVal = inData[(b * inROIPitch) + (d1 * inDim1Pitch) + (d2 * inDim2Pitch) + (c * inChPitch) + (h * inPitch) + w];
        maxVal = max(maxVal, inputVal);
    }
    reduction_max[tx][ty] = maxVal;

    for (int stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if (ty < stride)
            reduction_max[tx][ty] = max(reduction_max[tx][ty], reduction_max[tx][ty + stride]);
    }
    __syncthreads();

    if (ty == 0)
    {
        int outOffset = (b * outROIPitch) + (d1 * outDim1Pitch) + (d2 * outDim2Pitch) + (c * outChPitch) + w;
        outData[outOffset] = reduction_max[tx][0];
    }
}


template <class Tin, class Tout>
int TIDL_refCudaReduce(
    const Tin* pIn,
    Tout*  pOut,
    int32_t outWidth, int32_t outHeight, int32_t numChs, int32_t numDim2, int32_t numDim1, int32_t numROIs,
    int32_t inPitch, int32_t inChPitch, int32_t inDim2Pitch, int32_t inDim1Pitch, int32_t inBatchPitch,
    int32_t outChPitch, int32_t outDim2Pitch, int32_t outDim1Pitch, int32_t outBatchPitch, int32_t reduceOp)
{
    size_t input_size = numROIs * inBatchPitch * sizeof(Tin);
    size_t output_size = numROIs * outBatchPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Transpose: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Transpose: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_input, pIn, input_size, cudaMemcpyHostToDevice, stream));

    int total_elements = numROIs * numDim1 * numDim2 * numChs * outWidth;
    dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    if(reduceOp == (int32_t)TIDL_inReduceOpMin)
    {
        ReduceKernelMin<Tin, Tout><<<gridDim, blockDim, 0 , stream>>>(
        d_input, d_output,
        outWidth, outHeight, numChs, numDim2, numDim1, numROIs,
        inPitch, inChPitch, inDim2Pitch, inDim1Pitch, inBatchPitch,
        outChPitch, outDim2Pitch, outDim1Pitch, outBatchPitch);
    }
    else
    {
        ReduceKernelMax<Tin, Tout><<<gridDim, blockDim, 0 , stream>>>(
        d_input, d_output,
        outWidth, outHeight, numChs, numDim2, numDim1, numROIs,
        inPitch, inChPitch, inDim2Pitch, inDim1Pitch, inBatchPitch,
        outChPitch, outDim2Pitch, outDim1Pitch, outBatchPitch);
    }

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(pOut, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_refCudaReduce<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaReduce<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaReduce<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaReduce<unsigned int, unsigned int>(unsigned int const*, unsigned int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaReduce<signed char, signed char>(signed char const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaReduce<short, short>(short const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaReduce<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
