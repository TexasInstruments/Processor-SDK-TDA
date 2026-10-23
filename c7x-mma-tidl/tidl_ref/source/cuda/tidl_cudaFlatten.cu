/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaFlatten.cu
 @brief   This file contains CUDA implementation of Flatten layer.
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
__global__ void FlattenKernel(
  const Tin*    __restrict__ input,
  Tout*   __restrict__ output,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t width, int32_t height,int32_t numChs, int32_t numROIs,
  int32_t inLinePitch, int32_t inChPitch, int32_t inROIPitch, int32_t outROIPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numROIs * numChs * height * width;
    
    if (idx >= total_elements) return;
    
    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (numChs * height * width);
    int c = (idx / (height * width)) % numChs;
    int h = (idx / width) % height;
    int w = idx % width;

    int32_t inOffset  = inPtrOffset + (b*inROIPitch) +(c* inChPitch)   + (h * inLinePitch)   + w;
    int32_t outOffset = outPtrOffset + (b*outROIPitch) + c * height * width + h * width + w;
    output[outOffset] = input[inOffset];
}

template <class Tin, class Tout>
int TIDL_refCudaFlatten(
  const Tin* pIn,
  Tout*  pOut,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t width, int32_t height, int32_t numChs, int32_t numROIs,
  int32_t inLinePitch, int32_t inChPitch, int32_t inROIPitch, int32_t outROIPitch)
{
    size_t input_size = numROIs * inROIPitch * sizeof(Tin);
    size_t output_size = numROIs * outROIPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Flatten: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Flatten: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_input, pIn, input_size, cudaMemcpyHostToDevice, stream));

    int total_elements = numROIs * numChs * height * width;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    FlattenKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
    d_input, d_output,
    inPtrOffset,  outPtrOffset,
    width,  height, numChs, numROIs,
    inLinePitch,  inChPitch, inROIPitch,  outROIPitch);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpyAsync(pOut, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_refCudaFlatten<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaFlatten<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaFlatten<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaFlatten<unsigned int, unsigned int>(unsigned int const*, unsigned int*, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaFlatten<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int);
