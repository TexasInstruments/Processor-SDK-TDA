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
extern TIDL_CudaMemManager* g_cudaMemManager;

// Persistent memory structure for Slice
typedef struct {
    int isInit;
    void *dpInput;
    void *dpOutput;
} TIDL_cudaSC;

static TIDL_cudaSC CUDASC[MEM_BUFF_ARRAY_LEN] = {0};

// Function to free device pointers
void TIDL_cudaFreeSliceCudaPtrs()
{
    for(int i = 0; i < MEM_BUFF_ARRAY_LEN; i++)
    {
        if(CUDASC[i].dpInput) cudaFree(CUDASC[i].dpInput);
        if(CUDASC[i].dpOutput) cudaFree(CUDASC[i].dpOutput);
        
        CUDASC[i].dpInput = 0;
        CUDASC[i].dpOutput = 0;
        CUDASC[i].isInit = 0;
    }
    cudaDeviceSynchronize();
}

void TIDL_cudaSetSliceInitFlag(int32_t layerIdx)
{
    if(layerIdx >= 0 && layerIdx < MEM_BUFF_ARRAY_LEN) {
        CUDASC[layerIdx].isInit = 1;
    }
}

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

    if(!CUDASC[CUDNNLC].isInit) {
        checkCudaErr(cudaMalloc((void**)&CUDASC[CUDNNLC].dpInput, input_size));
        checkCudaErr(cudaMalloc((void**)&CUDASC[CUDNNLC].dpOutput, output_size));
        CUDASC[CUDNNLC].isInit = 1;
    }

    Tin* d_input = (Tin*)CUDASC[CUDNNLC].dpInput;
    Tout* d_output = (Tout*)CUDASC[CUDNNLC].dpOutput;

    checkCudaErr(cudaMemcpy(d_input, pIn, input_size, cudaMemcpyHostToDevice));
    
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
    
    // Copy results back to CPU
    checkCudaErr(cudaMemcpy(pOut, d_output, output_size, cudaMemcpyDeviceToHost));

    return IALG_EOK;
}

template int TIDL_refCudaSlice<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaSlice<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaSlice<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_refCudaSlice<unsigned int, unsigned int>(unsigned int const*, unsigned int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);