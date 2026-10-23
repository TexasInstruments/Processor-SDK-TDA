/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaNonZero.cu
 @brief   This file contains CUDA implementation of NonZero layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"
#include <thrust/scan.h>
#include <thrust/device_ptr.h>

// Marks which elements (in the same batch/dim1/dim2/ch/height/width iteration
// order as the reference implementation) are non-zero.
template <class Tin>
__global__ void NonZeroMaskKernel(
    const Tin* __restrict__ inData,
    int32_t* __restrict__ mask,
    int batches, int dim1, int dim2, int channels, int height, int width,
    int inRoiPitch, int inDim1Pitch, int inDim2Pitch, int inChPitch, int inLinePitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = batches * dim1 * dim2 * channels * height * width;

    if (idx >= total_elements) return;

    int b  = idx / (dim1 * dim2 * channels * height * width);
    int d1 = (idx / (dim2 * channels * height * width)) % dim1;
    int d2 = (idx / (channels * height * width)) % dim2;
    int c  = (idx / (height * width)) % channels;
    int h  = (idx / width) % height;
    int w  = idx % width;

    int32_t inOffset = (b * inRoiPitch) + (d1 * inDim1Pitch) + (d2 * inDim2Pitch) + (c * inChPitch) + (h * inLinePitch) + w;

    mask[idx] = (inData[inOffset] != static_cast<Tin>(0)) ? 1 : 0;
}

// Scatters the coordinates of non-zero elements into the output buffer using
// the exclusive prefix sum of the mask as the compacted position, which
// preserves the same relative ordering as the sequential reference loop.
__global__ void NonZeroScatterKernel(
    const int32_t* __restrict__ mask,
    const int32_t* __restrict__ prefix,
    int32_t* __restrict__ outData,
    int batches, int dim1, int dim2, int channels, int height, int width,
    int outLinePitch, int numValidInputDims, int outHeightDim)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = batches * dim1 * dim2 * channels * height * width;

    if (idx >= total_elements) return;
    if (mask[idx] == 0) return;

    int b  = idx / (dim1 * dim2 * channels * height * width);
    int d1 = (idx / (dim2 * channels * height * width)) % dim1;
    int d2 = (idx / (channels * height * width)) % dim2;
    int c  = (idx / (height * width)) % channels;
    int h  = (idx / width) % height;
    int w  = idx % width;

    int32_t nonZeroElementIdx[TIDL_DIM_MAX] = { b, d1, d2, c, h, w };
    int32_t pos = prefix[idx];

    for (int32_t dim = 0; dim < numValidInputDims; dim++)
    {
        int32_t outOffset = (outLinePitch * dim) + pos;
        outData[outOffset] = nonZeroElementIdx[TIDL_DIM_MAX - outHeightDim + dim];
    }
}

// CUDA wrapper for NonZero
template <class Tin>
int TIDL_cudaNonZero(
    const Tin* input,
    int32_t* output,
    int32_t* numNonZeroElements,
    int batches, int dim1, int dim2, int channels, int height, int width,
    int inRoiPitch, int inDim1Pitch, int inDim2Pitch, int inChPitch, int inLinePitch,
    int outLinePitch, int numValidInputDims, int outHeightDim)
{
    int total_elements = batches * dim1 * dim2 * channels * height * width;

    size_t input_size = (size_t)batches * inRoiPitch * sizeof(Tin);
    size_t output_size = (size_t)outLinePitch * numValidInputDims * sizeof(int32_t);

    Tin* d_input = NULL;
    int32_t* d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("NonZero: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("NonZero: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    int32_t* d_mask = NULL;
    int32_t* d_prefix = NULL;
    checkCudaErr(cudaMalloc((void**)&d_mask, (size_t)total_elements * sizeof(int32_t)));
    checkCudaErr(cudaMalloc((void**)&d_prefix, (size_t)total_elements * sizeof(int32_t)));

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    NonZeroMaskKernel<Tin><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_input, d_mask,
        batches, dim1, dim2, channels, height, width,
        inRoiPitch, inDim1Pitch, inDim2Pitch, inChPitch, inLinePitch);

    checkCudaErr(cudaStreamSynchronize(stream));

    // mask is fully populated on the host side at this point (stream was
    // synchronized above), so a plain (default-stream) thrust call is safe.
    thrust::device_ptr<int32_t> maskPtr(d_mask);
    thrust::device_ptr<int32_t> prefixPtr(d_prefix);
    thrust::exclusive_scan(maskPtr, maskPtr + total_elements, prefixPtr);
    checkCudaErr(cudaDeviceSynchronize());

    int32_t lastMask = 0, lastPrefix = 0;
    checkCudaErr(cudaMemcpyAsync(&lastMask, d_mask + (total_elements - 1), sizeof(int32_t), cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaMemcpyAsync(&lastPrefix, d_prefix + (total_elements - 1), sizeof(int32_t), cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));
    *numNonZeroElements = lastPrefix + lastMask;

    NonZeroScatterKernel<<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_mask, d_prefix, d_output,
        batches, dim1, dim2, channels, height, width,
        outLinePitch, numValidInputDims, outHeightDim);

    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    checkCudaErr(cudaFree(d_mask));
    checkCudaErr(cudaFree(d_prefix));

    return IALG_EOK;
}

template int TIDL_cudaNonZero<uint8_t>(const uint8_t*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonZero<int8_t>(const int8_t*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonZero<uint16_t>(const uint16_t*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonZero<int16_t>(const int16_t*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonZero<uint32_t>(const uint32_t*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonZero<int32_t>(const int32_t*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonZero<float>(const float*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaNonZero<bool>(const bool*, int32_t*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
