/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaShape.cu
 @brief   This file contains CUDA implementation of shape layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

// Shape kernel: copies the requested slice of dimValues into the output buffer
template <class Tout>
__global__ void ShapeKernel(
    const int32_t* __restrict__ dimVals,
    Tout* __restrict__ outData,
    int n)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= n) return;

    outData[idx] = (Tout)dimVals[idx];
}

// CUDA wrapper for Shape
template <class Tout>
int TIDL_cudaShape(
    const int32_t* dimValues,
    Tout* output,
    int32_t n)
{
    size_t in_size = (size_t)n * sizeof(int32_t);
    size_t out_size = (size_t)n * sizeof(Tout);

    int32_t* d_dimVals = NULL;
    Tout* d_output = NULL;

    checkCudaErr(cudaMalloc((void**)&d_dimVals, in_size));

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, out_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, out_size) != IALG_EOK)
        {
            printf("Shape: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_dimVals, dimValues, in_size, cudaMemcpyHostToDevice, stream));

    int grid_size = (n + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    ShapeKernel<Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(d_dimVals, d_output, n);

    checkCudaErr(cudaMemcpyAsync(output, d_output, out_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    checkCudaErr(cudaFree(d_dimVals));

    return IALG_EOK;
}

template int TIDL_cudaShape<uint8_t>(const int32_t*, uint8_t*, int32_t);
template int TIDL_cudaShape<int8_t>(const int32_t*, int8_t*, int32_t);
template int TIDL_cudaShape<uint16_t>(const int32_t*, uint16_t*, int32_t);
template int TIDL_cudaShape<int16_t>(const int32_t*, int16_t*, int32_t);
template int TIDL_cudaShape<uint32_t>(const int32_t*, uint32_t*, int32_t);
template int TIDL_cudaShape<int32_t>(const int32_t*, int32_t*, int32_t);
template int TIDL_cudaShape<uint64_t>(const int32_t*, uint64_t*, int32_t);
template int TIDL_cudaShape<int64_t>(const int32_t*, int64_t*, int32_t);
template int TIDL_cudaShape<float>(const int32_t*, float*, int32_t);
