/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaSize.cu
 @brief   This file contains CUDA implementation of size layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

// Size kernel: writes the (host-computed) total element count into the output scalar
template <class Tout>
__global__ void SizeKernel(
    Tout* __restrict__ outData,
    int64_t totalElements)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx == 0)
    {
        outData[0] = (Tout)totalElements;
    }
}

// CUDA wrapper for Size
template <class Tout>
int TIDL_cudaSize(
    Tout* output,
    int64_t totalElements)
{
    size_t out_size = sizeof(Tout);

    Tout* d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, out_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, out_size) != IALG_EOK)
        {
            printf("Size: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    SizeKernel<Tout><<<1, 1, 0, stream>>>(d_output, totalElements);

    checkCudaErr(cudaMemcpyAsync(output, d_output, out_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_cudaSize<uint8_t>(uint8_t*, int64_t);
template int TIDL_cudaSize<int8_t>(int8_t*, int64_t);
template int TIDL_cudaSize<uint16_t>(uint16_t*, int64_t);
template int TIDL_cudaSize<int16_t>(int16_t*, int64_t);
template int TIDL_cudaSize<uint32_t>(uint32_t*, int64_t);
template int TIDL_cudaSize<int32_t>(int32_t*, int64_t);
template int TIDL_cudaSize<uint64_t>(uint64_t*, int64_t);
template int TIDL_cudaSize<int64_t>(int64_t*, int64_t);
template int TIDL_cudaSize<float>(float*, int64_t);
