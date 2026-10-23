/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaColorConvert.cu
 @brief   This file contains CUDA implementation of Color Conversion layer.
 @version 0.1 (Nov 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

template <class Tin, class Tout>
__global__ void UV420ConvKernel(
  const Tin*    __restrict__ input,
  Tout*   __restrict__ output,
  int32_t inWidth, int32_t inHeight,
  int32_t inLinePitch, int32_t outLinePitch, int32_t outChPitch,
  int32_t numTotBatches, uint32_t inBatchPitch, uint32_t outBatchPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numTotBatches * inHeight * inWidth;
    
    if (idx >= total_elements) return;
    
    // Convert linear index to 3D coordinates (batch, height, width)
    int b = idx / (inHeight * inWidth);
    int h = (idx / inWidth) % inHeight;
    int w = idx % inWidth;
    
    int32_t inOffset = (b * inBatchPitch) + (h * inLinePitch) + w;
    int32_t outChOffset = (0 == (inOffset % 2)) ? 0 : outChPitch;
    int32_t outOffset1 = outChOffset + (b * outBatchPitch) + (outLinePitch * (2 * h)) + (2 * (w / 2));
    int32_t outOffset2 = outChOffset + (b * outBatchPitch) + (outLinePitch * (2 * h)) + (2 * (w / 2)) + 1;
    int32_t outOffset3 = outChOffset + (b * outBatchPitch) + (outLinePitch * ((2 * h) + 1)) + (2 * (w / 2));
    int32_t outOffset4 = outChOffset + (b * outBatchPitch) + (outLinePitch * ((2 * h) + 1)) + (2 * (w / 2)) + 1;

    output[outOffset1] = input[inOffset];
    output[outOffset2] = input[inOffset];
    output[outOffset3] = input[inOffset];
    output[outOffset4] = input[inOffset];
}

template <class Tin, class Tout>
int TIDL_refCudaUV420Conv(
  const Tin* pIn,
  Tout*  pOut,
  int32_t inWidth, int32_t inHeight,
  int32_t inLinePitch, int32_t outLinePitch, int32_t outChPitch,
  int32_t numTotBatches, uint32_t inBatchPitch, uint32_t outBatchPitch)
{
    size_t input_size = numTotBatches * inBatchPitch * sizeof(Tin);
    size_t output_size = numTotBatches * outBatchPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("ColorConversion: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("ColorConversion: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_input, pIn, input_size, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(d_output, pOut, output_size, cudaMemcpyHostToDevice, stream));

    int total_elements = numTotBatches * inHeight * inWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    UV420ConvKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_input, d_output,
        inWidth, inHeight,
        inLinePitch, outLinePitch, outChPitch,
        numTotBatches, inBatchPitch, outBatchPitch);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(pOut, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_refCudaUV420Conv<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, unsigned int, unsigned int);
template int TIDL_refCudaUV420Conv<float, float>(float const*, float*, int, int, int, int, int, int, unsigned int, unsigned int);
template int TIDL_refCudaUV420Conv<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, unsigned int, unsigned int);
template int TIDL_refCudaUV420Conv<floating_point::bfloat16_t, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, floating_point::bfloat16_t*, int, int, int, int, int, int, unsigned int, unsigned int);
