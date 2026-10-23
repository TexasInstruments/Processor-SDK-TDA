/*
 * Copyright (c) {2022 - 2026} Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 * Limited License.
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.
 *
 * DISCLAIMER.
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 */

/**
 *  \file tidl_cudaGather.cu
 *  \brief CUDA kernel for TIDL Gather layer.
 *
 *  Parallelises the row/column gather operation: one GPU thread per
 *  output element, replacing the two CPU loops in TIDL_refGather.
 */

#include <stdio.h>
#include <stdint.h>
#include <cuda_runtime.h>
#include "tidl_cuda.h"
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"


template<class Tin, class Tindx, class Tout>
__global__ void TIDL_cudaGather_kernel(
    const Tin*   data,
    const Tindx* indices,
    Tout*        output,
    int32_t numIndices,
    int32_t inDataNumRows,
    int32_t inDataNumCols,
    int32_t inDataPitch,
    int32_t tensorZeroPoint,
    int32_t axisIsWidth)   /* 1 = WIDTH axis, 0 = HEIGHT axis */
{
    const int tid = blockIdx.x * blockDim.x + threadIdx.x;

    if (axisIsWidth == 0)
    {
        /* Row selection: totalOut = numIndices * inDataNumCols */
        const int totalOut = numIndices * inDataNumCols;
        if (tid >= totalOut) return;

        const int i0 = tid / inDataNumCols;   /* index position */
        const int i1 = tid % inDataNumCols;   /* column within selected row */

        int32_t index = (int32_t)indices[i0];

        /* Out-of-bounds: fill with zero-point */
        if (index >= inDataNumRows || index < -inDataNumRows)
        {
            output[tid] = (Tout)tensorZeroPoint;
            return;
        }
        if (index < 0) index += inDataNumRows;

        output[i0 * inDataNumCols + i1] = (Tout)data[index * inDataPitch + i1];
    }
    else
    {
        /* Column selection: totalOut = inDataNumRows * numIndices */
        const int totalOut = inDataNumRows * numIndices;
        if (tid >= totalOut) return;

        const int i1 = tid / numIndices;   /* data row */
        const int i0 = tid % numIndices;   /* index position */

        int32_t index = (int32_t)indices[i0];

        /* Out-of-bounds: fill with zero-point */
        if (index >= inDataNumCols || index < -inDataNumCols)
        {
            output[i1 * numIndices + i0] = (Tout)tensorZeroPoint;
            return;
        }
        if (index < 0) index += inDataNumCols;

        output[i1 * numIndices + i0] = (Tout)data[i1 * inDataNumCols + index];
    }
}

template<class Tin, class Tindx, class Tout>
int TIDL_cudaGather(
    const Tin*   data,
    const Tindx* indices,
    Tout*        outPtr,
    int32_t numIndices,
    int32_t inDataNumRows,
    int32_t inDataNumCols,
    int32_t inDataPitch,
    int32_t tensorZeroPoint,
    int32_t axis)   /* TIDL_DIM_WIDTH = 5 for column, anything else for row */
{
    const int axisIsWidth = (axis == TIDL_DIM_WIDTH) ? 1 : 0;
    /* Buffer sizes */
    const int dataSize    = inDataNumRows * inDataPitch;
    const int indicesSize = numIndices;
    const int outSize     = axisIsWidth ? (inDataNumRows * numIndices) : (numIndices * inDataNumCols);

    Tin*   dpData    = NULL;
    Tindx* dpIndices = NULL;
    Tout*  dpOut     = NULL;

    /* Translate or allocate GPU buffers */
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)data, (void**)&dpData, sizeof(Tin) * dataSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)data, (void**)&dpData, sizeof(Tin) * dataSize) != IALG_EOK)
        {
            printf("Gather: Unable to allocate data buffer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)indices, (void**)&dpIndices, sizeof(Tindx) * indicesSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)indices, (void**)&dpIndices, sizeof(Tindx) * indicesSize) != IALG_EOK)
        { 
            printf("Gather: Unable to allocate indices buffer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)outPtr, (void**)&dpOut, sizeof(Tout) * outSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)outPtr, (void**)&dpOut, sizeof(Tout) * outSize) != IALG_EOK)
        {
            printf("Gather: Unable to allocate output buffer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    /* H2D copies */
    checkCudaErr(cudaMemcpyAsync(dpData, data, sizeof(Tin) * dataSize, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(dpIndices, indices, sizeof(Tindx) * indicesSize, cudaMemcpyHostToDevice, stream));

    /* Launch kernel */
    const long launchSize = (long)outSize;
    TIDL_cudaGather_kernel<Tin, Tindx, Tout>
        <<<GRID_SIZE(launchSize, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
            dpData, dpIndices, dpOut,
            numIndices, inDataNumRows, inDataNumCols,
            inDataPitch, tensorZeroPoint, axisIsWidth);

    /* D2H: bring output back to CPU */
    checkCudaErr(cudaMemcpyAsync(outPtr, dpOut, sizeof(Tout) * outSize, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return 0;
}

/* ======================================================================== */
/*  Explicit template instantiations                                        */
/* ======================================================================== */

template int TIDL_cudaGather<int8_t,  int32_t, int8_t> (const int8_t*,  const int32_t*, int8_t*,  int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<uint8_t, int32_t, uint8_t>(const uint8_t*, const int32_t*, uint8_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<int16_t, int32_t, int16_t>(const int16_t*, const int32_t*, int16_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<uint16_t,int32_t, uint16_t>(const uint16_t*,const int32_t*, uint16_t*,int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<int32_t, int32_t, int32_t>(const int32_t*, const int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<uint32_t,int32_t, uint32_t>(const uint32_t*,const int32_t*, uint32_t*,int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<float,   int32_t, float>  (const float*,   const int32_t*, float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<float,   float,   float>  (const float*,   const float*,   float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGather<floating_point::bfloat16_t, int, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, int const*, floating_point::bfloat16_t*, int, int, int, int, int, int);
template int TIDL_cudaGather<floating_point::bfloat16_t, floating_point::bfloat16_t, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, floating_point::bfloat16_t const*, floating_point::bfloat16_t*, int, int, int, int, int, int);
