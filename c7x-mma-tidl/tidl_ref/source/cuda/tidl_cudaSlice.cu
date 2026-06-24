/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaSlice.cu
 @brief   This file contains CUDA implementation of Slice layer.
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
/* External declaration of the global memory manager pointer */
extern TIDL_CudaMemManager* TIDL_cudaGetThreadManager();

template <class Tin, class Tout>
__global__ void SliceKernel(
  const Tin*    __restrict__ input,
  Tout*   __restrict__ output,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t outWidth, int32_t outHeight,int32_t numChs,
  int32_t numDim1,int32_t numDim2,int32_t numROIs,
  int32_t inLinePitch,int32_t outLinePitch, int32_t inChPitch,
  int32_t outChPitch, int32_t inDim1Pitch, int32_t outDim1Pitch, int32_t inDim2Pitch,
  int32_t outDim2Pitch, int32_t inROIPitch, int32_t outROIPitch)
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
int TIDL_refCudaSlice(
  const Tin* pIn,
  Tout*  pOut,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t outWidth, int32_t outHeight,int32_t numChs,
  int32_t numDim1,int32_t numDim2,int32_t numROIs,
  int32_t inLinePitch,int32_t outLinePitch, int32_t inChPitch,
  int32_t outChPitch, int32_t inDim1Pitch, int32_t outDim1Pitch, int32_t inDim2Pitch,
  int32_t outDim2Pitch, int32_t inROIPitch, int32_t outROIPitch)
{
    size_t input_size = numROIs * inROIPitch * sizeof(Tin);
    size_t output_size = numROIs * outROIPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    void* inPtrs[1] = {(void*)pIn};
    uint32_t inDataSizes[1] = {(uint32_t)input_size};
    TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, 1, inDataSizes);

    // Get GPU pointers after synchronization
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pOut,  (void**)&d_output, output_size);
    
    int total_elements = numROIs * numDim1 * numDim2 * numChs * outHeight * outWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    SliceKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK>>>(
    d_input, d_output,
    inPtrOffset,  outPtrOffset,
    outWidth,  outHeight, numChs,
    numDim1, numDim2, numROIs,
    inLinePitch, outLinePitch,  inChPitch,
    outChPitch,  inDim1Pitch,  outDim1Pitch,  inDim2Pitch,
    outDim2Pitch,  inROIPitch,  outROIPitch);

    checkCudaErr(cudaGetLastError());
    
    void* outPtrs[1] = {(void*)pOut};
    uint32_t outDataSizes[1] = {(uint32_t)output_size};
    TIDL_cudaMemManagerPostLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), outPtrs, 1, outDataSizes);

    return IALG_EOK;
}

template int TIDL_refCudaSlice<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaSlice<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaSlice<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaSlice<unsigned int, unsigned int>(unsigned int const*, unsigned int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);