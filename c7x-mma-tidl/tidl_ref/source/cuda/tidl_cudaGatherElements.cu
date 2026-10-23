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

#include <stdio.h>
#include <stdint.h>
#include <cuda_runtime.h>
#include "tidl_cuda.h"
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

template<class Tin, class Tindx, class Tout>
__global__ void TIDL_cudaGatherElements_kernel(
    const Tin*   data,
    const Tindx* indices,
    Tout*        output,
    /* indices tensor dimensions */
    int32_t inIndicesBatches,
    int32_t inIndicesDim1,
    int32_t inIndicesDim2,
    int32_t inIndicesNumChs,
    int32_t inIndicesNumRows,
    int32_t inIndicesNumCols,
    /* indices tensor pitches */
    int32_t inIndicesRoiPitch,
    int32_t inIndicesDim1Pitch,
    int32_t inIndicesDim2Pitch,
    int32_t inIndicesChPitch,
    int32_t inIndicesPitch,
    /* data tensor pitches */
    int32_t inDataRoiPitch,
    int32_t inDataDim1Pitch,
    int32_t inDataDim2Pitch,
    int32_t inDataChPitch,
    int32_t inDataPitch,
    /* data tensor dim sizes (for negative index wrap-around) */
    int32_t inDataDimBatch,
    int32_t inDataDimDim1,
    int32_t inDataDimDim2,
    int32_t inDataDimCh,
    int32_t inDataDimH,
    int32_t inDataDimW,
    /* output tensor pitches */
    int32_t outRoiPitch,
    int32_t outDim1Pitch,
    int32_t outDim2Pitch,
    int32_t outChPitch,
    int32_t outPitch,
    /* gather axis */
    int32_t axis)
{
    const int tid = blockIdx.x * blockDim.x + threadIdx.x;
    const int totalOut = inIndicesBatches * inIndicesDim1 * inIndicesDim2 *
                         inIndicesNumChs  * inIndicesNumRows * inIndicesNumCols;
    if (tid >= totalOut) return;

    /* Decompose flat thread index into 6D indices tensor position */
    int k  = tid;
    const int i0 = k / (inIndicesDim1 * inIndicesDim2 * inIndicesNumChs * inIndicesNumRows * inIndicesNumCols);
    k = k % (inIndicesDim1 * inIndicesDim2 * inIndicesNumChs * inIndicesNumRows * inIndicesNumCols);
    const int i1 = k / (inIndicesDim2 * inIndicesNumChs * inIndicesNumRows * inIndicesNumCols);
    k = k % (inIndicesDim2 * inIndicesNumChs * inIndicesNumRows * inIndicesNumCols);
    const int i2 = k / (inIndicesNumChs * inIndicesNumRows * inIndicesNumCols);
    k = k % (inIndicesNumChs * inIndicesNumRows * inIndicesNumCols);
    const int i3 = k / (inIndicesNumRows * inIndicesNumCols);
    k = k % (inIndicesNumRows * inIndicesNumCols);
    const int i4 = k / inIndicesNumCols;
    const int i5 = k % inIndicesNumCols;

    int32_t index = (int32_t)indices[i0 * inIndicesRoiPitch  +
                                     i1 * inIndicesDim1Pitch  +
                                     i2 * inIndicesDim2Pitch  +
                                     i3 * inIndicesChPitch    +
                                     i4 * inIndicesPitch      + i5];

    const int outIdx = i0 * outRoiPitch  +
                       i1 * outDim1Pitch +
                       i2 * outDim2Pitch +
                       i3 * outChPitch   +
                       i4 * outPitch     + i5;

    int32_t srcIdx = 0;

    if (axis == 0)   /* TIDL_DIM_BATCH */
    {
        if (index < 0) index += inDataDimBatch;
        srcIdx = index * inDataRoiPitch  +
                 i1    * inDataDim1Pitch +
                 i2    * inDataDim2Pitch +
                 i3    * inDataChPitch   +
                 i4    * inDataPitch     + i5;
    }
    else if (axis == 1)   /* TIDL_DIM_DIM1 */
    {
        if (index < 0) index += inDataDimDim1;
        srcIdx = i0    * inDataRoiPitch  +
                 index * inDataDim1Pitch +
                 i2    * inDataDim2Pitch +
                 i3    * inDataChPitch   +
                 i4    * inDataPitch     + i5;
    }
    else if (axis == 2)   /* TIDL_DIM_DIM2 */
    {
        if (index < 0) index += inDataDimDim2;
        srcIdx = i0    * inDataRoiPitch  +
                 i1    * inDataDim1Pitch +
                 index * inDataDim2Pitch +
                 i3    * inDataChPitch   +
                 i4    * inDataPitch     + i5;
    }
    else if (axis == 3)   /* TIDL_DIM_NUMCH */
    {
        if (index < 0) index += inDataDimCh;
        srcIdx = i0    * inDataRoiPitch  +
                 i1    * inDataDim1Pitch +
                 i2    * inDataDim2Pitch +
                 index * inDataChPitch   +
                 i4    * inDataPitch     + i5;
    }
    else if (axis == 4)   /* TIDL_DIM_HEIGHT */
    {
        if (index < 0) index += inDataDimH;
        srcIdx = i0    * inDataRoiPitch  +
                 i1    * inDataDim1Pitch +
                 i2    * inDataDim2Pitch +
                 i3    * inDataChPitch   +
                 index * inDataPitch     + i5;
    }
    else   /* axis == 5, TIDL_DIM_WIDTH */
    {
        if (index < 0) index += inDataDimW;
        srcIdx = i0 * inDataRoiPitch  +
                 i1 * inDataDim1Pitch +
                 i2 * inDataDim2Pitch +
                 i3 * inDataChPitch   +
                 i4 * inDataPitch     + index;
    }

    output[outIdx] = (Tout)data[srcIdx];
}


template<class Tin, class Tindx, class Tout>
int TIDL_cudaGatherElements(
    const Tin*   data,
    const Tindx* indices,
    Tout*        outPtr,
    int32_t inIndicesBatches,
    int32_t inIndicesDim1,
    int32_t inIndicesDim2,
    int32_t inIndicesNumChs,
    int32_t inIndicesNumRows,
    int32_t inIndicesNumCols,
    int32_t inIndicesRoiPitch,
    int32_t inIndicesDim1Pitch,
    int32_t inIndicesDim2Pitch,
    int32_t inIndicesChPitch,
    int32_t inIndicesPitch,
    int32_t inDataRoiPitch,
    int32_t inDataDim1Pitch,
    int32_t inDataDim2Pitch,
    int32_t inDataChPitch,
    int32_t inDataPitch,
    int32_t inDataDimBatch,
    int32_t inDataDimDim1,
    int32_t inDataDimDim2,
    int32_t inDataDimCh,
    int32_t inDataDimH,
    int32_t inDataDimW,
    int32_t outRoiPitch,
    int32_t outDim1Pitch,
    int32_t outDim2Pitch,
    int32_t outChPitch,
    int32_t outPitch,
    int32_t axis)
{
    const int dataSize = inDataDimBatch * inDataRoiPitch;
    const int indSize  = inIndicesBatches * inIndicesRoiPitch;
    const int outSize  = inIndicesBatches * outRoiPitch;

    Tin*   dpData    = NULL;
    Tindx* dpIndices = NULL;
    Tout*  dpOut     = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)data, (void**)&dpData, sizeof(Tin) * dataSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)data, (void**)&dpData, sizeof(Tin) * dataSize) != IALG_EOK)
        {
            printf("GatherElements: Unable to allocate data buffer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)indices, (void**)&dpIndices, sizeof(Tindx) * indSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)indices, (void**)&dpIndices, sizeof(Tindx) * indSize) != IALG_EOK)
        {
            printf("GatherElements: Unable to allocate indices buffer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)outPtr, (void**)&dpOut, sizeof(Tout) * outSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)outPtr, (void**)&dpOut, sizeof(Tout) * outSize) != IALG_EOK)
        {
            printf("GatherElements: Unable to allocate output buffer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(dpData, data, sizeof(Tin) * dataSize, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(dpIndices, indices, sizeof(Tindx) * indSize, cudaMemcpyHostToDevice, stream));

    const long launchSize = (long)inIndicesBatches * inIndicesDim1 * inIndicesDim2 * inIndicesNumChs * inIndicesNumRows * inIndicesNumCols;

    TIDL_cudaGatherElements_kernel<Tin, Tindx, Tout>
        <<<GRID_SIZE(launchSize, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
            dpData, dpIndices, dpOut,
            inIndicesBatches, inIndicesDim1, inIndicesDim2,
            inIndicesNumChs, inIndicesNumRows, inIndicesNumCols,
            inIndicesRoiPitch, inIndicesDim1Pitch, inIndicesDim2Pitch,
            inIndicesChPitch, inIndicesPitch,
            inDataRoiPitch, inDataDim1Pitch, inDataDim2Pitch,
            inDataChPitch, inDataPitch,
            inDataDimBatch, inDataDimDim1, inDataDimDim2,
            inDataDimCh, inDataDimH, inDataDimW,
            outRoiPitch, outDim1Pitch, outDim2Pitch,
            outChPitch, outPitch, axis);

    checkCudaErr(cudaMemcpyAsync(outPtr, dpOut, sizeof(Tout) * outSize, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return 0;
}

/* ======================================================================== */
/*  Explicit template instantiations                                        */
/* ======================================================================== */

template int TIDL_cudaGatherElements<int8_t,   int32_t, int8_t> (const int8_t*,  const int32_t*, int8_t*,  int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGatherElements<uint8_t,  int32_t, uint8_t>(const uint8_t*, const int32_t*, uint8_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGatherElements<int16_t,  int32_t, int16_t>(const int16_t*, const int32_t*, int16_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGatherElements<uint16_t, int32_t, uint16_t>(const uint16_t*,const int32_t*, uint16_t*,int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGatherElements<int32_t, int32_t, int32_t>(const int32_t*, const int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGatherElements<uint32_t,int32_t, uint32_t>(const uint32_t*,const int32_t*, uint32_t*,int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGatherElements<float,   int32_t, float>  (const float*,   const int32_t*, float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaGatherElements<float,   float,   float>  (const float*,   const float*,   float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);

