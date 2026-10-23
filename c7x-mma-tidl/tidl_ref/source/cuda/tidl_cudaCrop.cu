/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaCrop.cu
 @brief   This file contains CUDA implementation of Crop layer.
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
__global__ void CropKernel(
  const Tin*    __restrict__ input,
  Tout*   __restrict__ output,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t outWidth, int32_t outHeight, int32_t numChs,
  int32_t numDim2, int32_t numDim1, int32_t numROIs,
  int32_t inLinePitch, int32_t outLinePitch, int32_t inChPitch,
  int32_t inDim2Pitch, int32_t inDim1Pitch, int32_t inROIPitch,
  int32_t outChPitch, int32_t outDim2Pitch, int32_t outDim1Pitch,
  int32_t outROIPitch)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numROIs * numDim1 * numDim2 * numChs * outHeight * outWidth;
    
    if (idx >= total_elements) return;
    
    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (numDim1 * numDim2 * numChs * outHeight * outWidth);
    int d1 = (idx / (numDim2 * numChs * outHeight * outWidth)) % numDim1;
    int d2 = (idx / (numChs * outHeight * outWidth)) % numDim2;
    int c = (idx / (outHeight * outWidth)) % numChs;
    int h = (idx / outWidth) % outHeight;
    int w = idx % outWidth;

    int32_t inOffset  = inPtrOffset + (b*inROIPitch)   + (d1* inDim1Pitch) + (d2*inDim2Pitch)   +(c* inChPitch)   + (h * inLinePitch)   + w;
    int32_t outOffset = outPtrOffset + (b*outROIPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + (c* outChPitch)  + (h * outLinePitch)  + w;
    output[outOffset] = input[inOffset];
}

template <class Tin, class Tout>
int TIDL_refCudaCrop(
  const Tin* pIn,
  Tout*  pOut,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t outWidth, int32_t outHeight, int32_t numChs,
  int32_t numDim2, int32_t numDim1, int32_t numROIs,
  int32_t inLinePitch, int32_t outLinePitch, int32_t inChPitch,
  int32_t inDim2Pitch, int32_t inDim1Pitch, int32_t inROIPitch,
  int32_t outChPitch, int32_t outDim2Pitch, int32_t outDim1Pitch,
  int32_t outROIPitch)
{
    size_t input_elements = 1 + inPtrOffset +
                           ((numROIs - 1) * inROIPitch) +
                           ((numDim1 - 1) * inDim1Pitch) +
                           ((numDim2 - 1) * inDim2Pitch) +
                           ((numChs - 1) * inChPitch) +
                           ((outHeight - 1) * inLinePitch) +
                           (outWidth - 1);
    size_t output_elements = 1 + outPtrOffset +
                            ((numROIs - 1) * outROIPitch) +
                            ((numDim1 - 1) * outDim1Pitch) +
                            ((numDim2 - 1) * outDim2Pitch) +
                            ((numChs - 1) * outChPitch) +
                            ((outHeight - 1) * outLinePitch) +
                            (outWidth - 1);
    size_t input_size = input_elements * sizeof(Tin);
    size_t output_size = output_elements * sizeof(Tout);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Crop: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Crop: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_input, pIn, input_size, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(d_output, pOut, output_size, cudaMemcpyHostToDevice, stream));
    
    int total_elements = numROIs * numDim1 * numDim2 * numChs * outHeight * outWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    CropKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
    d_input, d_output,
    inPtrOffset,  outPtrOffset,
    outWidth,  outHeight, numChs,
    numDim2, numDim1, numROIs,
    inLinePitch, outLinePitch,  inChPitch,
    inDim2Pitch, inDim1Pitch, inROIPitch,
    outChPitch, outDim2Pitch, outDim1Pitch,
    outROIPitch);

    checkCudaErr(cudaGetLastError());
    
    // Copy results back to CPU
    checkCudaErr(cudaMemcpyAsync(pOut, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));
    return IALG_EOK;
}

template int TIDL_refCudaCrop<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaCrop<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaCrop<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaCrop<floating_point::bfloat16_t, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, floating_point::bfloat16_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
