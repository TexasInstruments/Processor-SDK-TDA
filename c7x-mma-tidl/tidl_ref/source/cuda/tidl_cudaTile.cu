/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaTile.cu
 @brief   This file contains CUDA implementation of Tile layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

/**
 * Tile kernel: each thread handles one output element.
 * Uses modulo wrapping on each dimension to map output coords to input coords.
 * Works at byte level (elementSize bytes per element) for type-agnostic tiling.
 */
__global__ void TileKernel(
    const uint8_t* __restrict__ inData,
    uint8_t* __restrict__ outData,
    int32_t elementSize,
    int32_t outB,  int32_t outD1, int32_t outD2, int32_t outC,  int32_t outH,  int32_t outW,
    int32_t inBP,  int32_t inD1P, int32_t inD2P,  int32_t inCP,  int32_t inLP,
    int32_t outBP, int32_t outD1P, int32_t outD2P, int32_t outCP, int32_t outLP,
    int32_t inB,   int32_t inD1,   int32_t inD2,   int32_t inC,   int32_t inH,  int32_t inW)
{
    int64_t idx = (int64_t)blockIdx.x * blockDim.x + threadIdx.x;
    int64_t total = (int64_t)outB * outD1 * outD2 * outC * outH * outW;
    if (idx >= total) return;

    int64_t tmp = idx;
    int32_t iw  = (int32_t)(tmp % outW);  tmp /= outW;
    int32_t ih  = (int32_t)(tmp % outH);  tmp /= outH;
    int32_t ic  = (int32_t)(tmp % outC);  tmp /= outC;
    int32_t id2 = (int32_t)(tmp % outD2); tmp /= outD2;
    int32_t id1 = (int32_t)(tmp % outD1); tmp /= outD1;
    int32_t ib  = (int32_t)tmp;

    /* Input offset: wrap each dimension into the input range */
    int64_t inOff  = (int64_t)(ib  % inB)  * inBP  + (id1 % inD1) * inD1P
                   + (int64_t)(id2 % inD2)  * inD2P  + (ic  % inC)  * inCP
                   + (int64_t)(ih  % inH)   * inLP   +  (iw  % inW);

    int64_t outOff = (int64_t)ib * outBP + id1 * outD1P
                   + (int64_t)id2 * outD2P + ic * outCP
                   + (int64_t)ih * outLP   + iw;

    for (int32_t b = 0; b < elementSize; b++)
    {
        outData[outOff * elementSize + b] = inData[inOff * elementSize + b];
    }
}

/**
 * CUDA wrapper for Tile.
 * elementSize: bytes per logical element (1, 2, 4, or 8).
 * All pitches are in element units (not byte units).
 */
int TIDL_cudaTile(
    const void*  input,
    void*        output,
    int32_t      elementSize,
    int32_t      inBatch,     int32_t inDim1,  int32_t inDim2,  int32_t inCh,  int32_t inHeight,  int32_t inWidth,
    int32_t      inBatchPitch, int32_t inDim1Pitch, int32_t inDim2Pitch, int32_t inChPitch, int32_t inLinePitch,
    int32_t      outBatch,    int32_t outDim1, int32_t outDim2, int32_t outCh, int32_t outHeight, int32_t outWidth,
    int32_t      outBatchPitch, int32_t outDim1Pitch, int32_t outDim2Pitch, int32_t outChPitch, int32_t outLinePitch)
{
    size_t inSize  = (size_t)inBatch  * inBatchPitch  * elementSize;
    size_t outSize = (size_t)outBatch * outBatchPitch * elementSize;

    uint8_t* d_input  = NULL;
    uint8_t* d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)input, (void**)&d_input, inSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)input, (void**)&d_input, inSize) != IALG_EOK)
        {
            printf("Tile: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, outSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, outSize) != IALG_EOK)
        {
            printf("Tile: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, inSize, cudaMemcpyHostToDevice, stream));

    int64_t total = (int64_t)outBatch * outDim1 * outDim2 * outCh * outHeight * outWidth;
    int64_t gridSize = (total + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    TileKernel<<<(int32_t)gridSize, THREADS_PER_BLOCK, 0, stream>>>(
        (const uint8_t*)d_input, (uint8_t*)d_output,
        elementSize,
        outBatch,  outDim1,  outDim2,  outCh,  outHeight,  outWidth,
        inBatchPitch, inDim1Pitch, inDim2Pitch, inChPitch, inLinePitch,
        outBatchPitch, outDim1Pitch, outDim2Pitch, outChPitch, outLinePitch,
        inBatch,   inDim1,   inDim2,   inCh,   inHeight,  inWidth);

    checkCudaErr(cudaMemcpyAsync(output, d_output, outSize, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}
