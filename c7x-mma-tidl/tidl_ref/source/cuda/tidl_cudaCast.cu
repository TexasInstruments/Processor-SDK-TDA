/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaCast.cu
 @brief   This file contains CUDA implementation of cast layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

// Cast kernel
template <class Tin, class Tout>
__global__ void CastKernel(
    const Tin* __restrict__ inData,
    Tout* __restrict__ outData,
    int batches, int dim1, int dim2, int channels, int height, int width,
    int inRoiPitch, int inDim1Pitch, int inDim2Pitch, int inChPitch, int inLinePitch,
    int outRoiPitch, int outDim1Pitch, int outDim2Pitch, int outChPitch, int outLinePitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = batches * dim1 * dim2 * channels * height * width;

    if (idx >= total_elements) return;

    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b  = idx / (dim1 * dim2 * channels * height * width);
    int d1 = (idx / (dim2 * channels * height * width)) % dim1;
    int d2 = (idx / (channels * height * width)) % dim2;
    int c  = (idx / (height * width)) % channels;
    int h  = (idx / width) % height;
    int w  = idx % width;

    int32_t inOffset  = (b * inRoiPitch) + (d1 * inDim1Pitch) + (d2 * inDim2Pitch) + (c * inChPitch) + (h * inLinePitch) + w;
    int32_t outOffset = (b * outRoiPitch) + (d1 * outDim1Pitch) + (d2 * outDim2Pitch) + (c * outChPitch) + (h * outLinePitch) + w;

    outData[outOffset] = static_cast<Tout>(inData[inOffset]);
}

// CUDA wrapper for Cast
template <class Tin, class Tout>
int TIDL_cudaCast(
    const Tin* input,
    Tout* output,
    int batches, int dim1, int dim2, int channels, int height, int width,
    int inRoiPitch, int inDim1Pitch, int inDim2Pitch, int inChPitch, int inLinePitch,
    int outRoiPitch, int outDim1Pitch, int outDim2Pitch, int outChPitch, int outLinePitch)
{
    // Calculate memory sizes
    size_t input_size = (size_t)batches * inRoiPitch * sizeof(Tin);
    size_t output_size = (size_t)batches * outRoiPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Cast: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Cast: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    // Launch kernel on stream
    int total_elements = batches * dim1 * dim2 * channels * height * width;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    CastKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_input, d_output,
        batches, dim1, dim2, channels, height, width,
        inRoiPitch, inDim1Pitch, inDim2Pitch, inChPitch, inLinePitch,
        outRoiPitch, outDim1Pitch, outDim2Pitch, outChPitch, outLinePitch);

    // Async D2H
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_cudaCast<signed char, signed char>(signed char const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, unsigned char>(signed char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, short>(signed char const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, unsigned short>(signed char const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, float>(signed char const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, int32_t>(signed char const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, uint32_t>(signed char const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, int64_t>(signed char const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, uint64_t>(signed char const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, bool>(signed char const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<signed char, bfloat16_tidl>(signed char const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, signed char>(unsigned char const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, short>(unsigned char const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, unsigned short>(unsigned char const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, float>(unsigned char const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, int32_t>(unsigned char const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, uint32_t>(unsigned char const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, int64_t>(unsigned char const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, uint64_t>(unsigned char const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, bool>(unsigned char const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned char, bfloat16_tidl>(unsigned char const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, signed char>(short const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, unsigned char>(short const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, short>(short const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, unsigned short>(short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, float>(short const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, int32_t>(short const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, uint32_t>(short const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, int64_t>(short const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, uint64_t>(short const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, bool>(short const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<short, bfloat16_tidl>(short const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, signed char>(unsigned short const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, unsigned char>(unsigned short const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, short>(unsigned short const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, float>(unsigned short const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, int32_t>(unsigned short const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, uint32_t>(unsigned short const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, int64_t>(unsigned short const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, uint64_t>(unsigned short const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, bool>(unsigned short const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<unsigned short, bfloat16_tidl>(unsigned short const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, signed char>(float const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, unsigned char>(float const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, short>(float const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, unsigned short>(float const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, int32_t>(float const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, uint32_t>(float const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, int64_t>(float const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, uint64_t>(float const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, bool>(float const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<float, bfloat16_tidl>(float const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, signed char>(int32_t const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, unsigned char>(int32_t const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, short>(int32_t const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, unsigned short>(int32_t const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, float>(int32_t const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, int32_t>(int32_t const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, uint32_t>(int32_t const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, int64_t>(int32_t const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, uint64_t>(int32_t const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, bool>(int32_t const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int32_t, bfloat16_tidl>(int32_t const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, signed char>(uint32_t const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, unsigned char>(uint32_t const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, short>(uint32_t const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, unsigned short>(uint32_t const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, float>(uint32_t const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, int32_t>(uint32_t const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, uint32_t>(uint32_t const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, int64_t>(uint32_t const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, uint64_t>(uint32_t const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, bool>(uint32_t const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint32_t, bfloat16_tidl>(uint32_t const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, signed char>(int64_t const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, unsigned char>(int64_t const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, short>(int64_t const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, unsigned short>(int64_t const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, float>(int64_t const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, int32_t>(int64_t const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, uint32_t>(int64_t const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, int64_t>(int64_t const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, uint64_t>(int64_t const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, bool>(int64_t const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<int64_t, bfloat16_tidl>(int64_t const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, signed char>(uint64_t const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, unsigned char>(uint64_t const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, short>(uint64_t const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, unsigned short>(uint64_t const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, float>(uint64_t const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, int32_t>(uint64_t const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, uint32_t>(uint64_t const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, int64_t>(uint64_t const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, uint64_t>(uint64_t const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, bool>(uint64_t const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<uint64_t, bfloat16_tidl>(uint64_t const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, signed char>(bool const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, unsigned char>(bool const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, short>(bool const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, unsigned short>(bool const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, float>(bool const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, int32_t>(bool const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, uint32_t>(bool const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, int64_t>(bool const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, uint64_t>(bool const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, bool>(bool const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bool, bfloat16_tidl>(bool const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, signed char>(bfloat16_tidl const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, unsigned char>(bfloat16_tidl const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, short>(bfloat16_tidl const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, unsigned short>(bfloat16_tidl const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, float>(bfloat16_tidl const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, int32_t>(bfloat16_tidl const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, uint32_t>(bfloat16_tidl const*, uint32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, int64_t>(bfloat16_tidl const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, uint64_t>(bfloat16_tidl const*, uint64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, bool>(bfloat16_tidl const*, bool*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaCast<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
