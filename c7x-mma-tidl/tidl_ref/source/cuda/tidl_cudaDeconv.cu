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

template<class Tin, class Tw, class Tb, class Tacc>
__global__ void TIDL_cudaDeconv2d_kernel(
    const Tin*  pInChannel,
    const Tw*   pCoeffs,
    const Tb*   pBias,
    Tacc*       accPtr,
    int32_t numTotRoi,
    int32_t numGroups,
    int32_t numInChannels,
    int32_t numOutChannels,
    int32_t inChPitch,
    int32_t outChPitch,
    int32_t width,          /* input spatial width */
    int32_t height,         /* input spatial height */
    int32_t inImPitch,      /* elements per input row */
    int32_t outImPitch,     /* elements per accumulator row */
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t dilationWidth,
    int32_t dilationHeight,
    int32_t strideWidth,
    int32_t strideHeight,
    int32_t enableBias)
{
    const int tid = blockIdx.x * blockDim.x + threadIdx.x;
    const int totalElems = numTotRoi * numGroups * numOutChannels * outChPitch;
    if (tid >= totalElems) return;

    /* Decompose flat index into (batch, group, outCh, spatial_flat) */
    int k = tid;
    const int n = k / (numGroups * numOutChannels * outChPitch);
    k = k % (numGroups * numOutChannels * outChPitch);
    const int g = k / (numOutChannels * outChPitch);
    k = k % (numOutChannels * outChPitch);
    const int co = k / outChPitch;
    const int sp = k % outChPitch;   /* flat spatial index within one channel */

    /* acc_line and acc_x correspond to the ref indices:
     *   acc_line = i2 * strideH + i4 * dilH
     *   acc_x    = i3 * strideW + i5 * dilW
     */
    const int acc_line = sp / outImPitch;
    const int acc_x    = sp % outImPitch;

    /* Initialise with bias or zero */
    Tacc val = enableBias ? (Tacc)pBias[g * numOutChannels + co] : (Tacc)0;

    /* Base offsets into input and coefficient arrays */
    const int inBase    = n * numGroups * numInChannels * inChPitch + g * numInChannels * inChPitch;
    const int coeffBase = g  * numInChannels * coeffsHeight * coeffsWidth * numOutChannels + co * numInChannels * coeffsHeight * coeffsWidth;

    /* Iterate over filter positions (ky, kx) to find contributing inputs */
    for (int ky = 0; ky < coeffsHeight; ky++)
    {
        const int rem_line = acc_line - ky * dilationHeight;
        if (rem_line < 0 || (rem_line % strideHeight) != 0) continue;
        const int in_y = rem_line / strideHeight;
        if (in_y >= height) continue;

        for (int kx = 0; kx < coeffsWidth; kx++)
        {
            const int rem_x = acc_x - kx * dilationWidth;
            if (rem_x < 0 || (rem_x % strideWidth) != 0) continue;
            const int in_x = rem_x / strideWidth;
            if (in_x >= width) continue;

            /* Accumulate over all input channels */
            for (int ci = 0; ci < numInChannels; ci++)
            {
                const Tin inV = pInChannel[inBase + ci * inChPitch + in_y * inImPitch + in_x];
                const Tw  wV  = pCoeffs[coeffBase + ci * coeffsHeight * coeffsWidth + ky * coeffsWidth + kx];
                val += (Tacc)inV * (Tacc)wV;
            }
        }
    }

    /* Write result to accumulator */
    const int accIdx = n  * numGroups * numOutChannels * outChPitch + g  * numOutChannels * outChPitch + co * outChPitch + sp;
    accPtr[accIdx] = val;
}

template<class Tin, class Tw, class Tb, class Tacc>
int TIDL_cudaDeconv2d(
    Tin*    pInChannel,
    Tw*     pCoeffs,
    Tb*     pBias,
    Tacc*   accPtr,
    Tacc*   minPtr,
    Tacc*   maxPtr,
    int32_t numTotRoi,
    int32_t numGroups,
    int32_t numInChannels,
    int32_t numOutChannels,
    int32_t inChPitch,
    int32_t outChPitch,
    int32_t width,
    int32_t height,
    int32_t inImPitch,
    int32_t outImPitch,
    int32_t coeffsWidth,
    int32_t coeffsHeight,
    int32_t dilationWidth,
    int32_t dilationHeight,
    int32_t strideWidth,
    int32_t strideHeight,
    int32_t enableBias)
{
    const int inSize     = numTotRoi * numGroups * numInChannels * inChPitch;
    const int coeffSize  = numGroups * numOutChannels * numInChannels *
                           coeffsHeight * coeffsWidth;
    const int biasSize   = numGroups * numOutChannels;
    const int accSize    = numTotRoi * numGroups * numOutChannels * outChPitch;

    Tin*   dpIn     = NULL;
    Tw*    dpCoeffs = NULL;
    Tb*    dpBias   = NULL;
    Tacc*  dpAcc    = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pInChannel, (void**)&dpIn, sizeof(Tin) * inSize) != IALG_EOK)
    {
        /*If it's the first layer - it is possible that the input doesn't have a GPU mirror pointer, allocate a new buffer*/
        if( TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pInChannel, (void**)&dpIn, sizeof(Tin) * inSize) != IALG_EOK)
        {
            /*Log Error*/
            printf("DeConv: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pCoeffs, (void**)&dpCoeffs, sizeof(Tw) * coeffSize) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pCoeffs, (void**)&dpCoeffs, sizeof(Tw) * coeffSize) != IALG_EOK)
        {
            printf("DeConv: Unable to find coefficient pointer\n");
            return IALG_EFAIL;
        }
    }

    // Accumulator (output) buffer
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accPtr, (void**)&dpAcc, sizeof(Tacc) * accSize) != IALG_EOK)
    {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), accPtr, (void**)&dpAcc, sizeof(Tacc) * accSize) != IALG_EOK)
        {
            printf("DeConv: Unable to find accumulation pointer\n");
            return IALG_EFAIL;
        }
    }

    if(enableBias && biasSize > 0)
    {
        if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pBias, (void**)&dpBias, sizeof(Tb) * biasSize) != IALG_EOK)
        {
        if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pBias, (void**)&dpBias, sizeof(Tb) * biasSize) != IALG_EOK)
        {
            printf("DeConv: Unable to find bias pointer\n");
            return IALG_EFAIL;
        }
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    /* H2D: input always; weights/bias only when not already on device */
    checkCudaErr(cudaMemcpyAsync(dpIn, pInChannel, sizeof(Tin) * inSize, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemcpyAsync(dpCoeffs, pCoeffs, sizeof(Tw) * coeffSize, cudaMemcpyHostToDevice, stream));
    if (enableBias && dpBias != NULL)
        checkCudaErr(cudaMemcpyAsync(dpBias, pBias, sizeof(Tb) * biasSize, cudaMemcpyHostToDevice, stream));

    const long launchSize = (long)numTotRoi * numGroups * numOutChannels * outChPitch;
    TIDL_cudaDeconv2d_kernel<Tin, Tw, Tb, Tacc>
        <<<GRID_SIZE(launchSize, THREADS_PER_BLOCK), THREADS_PER_BLOCK, 0, stream>>>(
            dpIn, dpCoeffs, dpBias, dpAcc,
            numTotRoi, numGroups, numInChannels, numOutChannels,
            inChPitch, outChPitch,
            width, height, inImPitch, outImPitch,
            coeffsWidth, coeffsHeight,
            dilationWidth, dilationHeight,
            strideWidth, strideHeight,
            enableBias);

    /* D2H: bring accPtr back so the caller's min/max and saturation code works */
    checkCudaErr(cudaMemcpyAsync(accPtr, dpAcc, sizeof(Tacc) * accSize, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    /* Update min/max from the D2H'd accPtr (mirrors the ref loop) */
    if (minPtr != NULL && maxPtr != NULL)
    {
        for (int n = 0; n < numTotRoi; n++)
        for (int g = 0; g < numGroups; g++)
        for (int co = 0; co < numOutChannels; co++)
        for (int s = 0; s < outChPitch; s++)
        {
            Tacc v = accPtr[n * numGroups * numOutChannels * outChPitch +
                            g * numOutChannels * outChPitch +
                            co * outChPitch + s];
            if (v < *minPtr) *minPtr = v;
            if (v > *maxPtr) *maxPtr = v;
        }
    }

    return 0;
}

/* ======================================================================== */
/*  Explicit template instantiations                                        */
/* ======================================================================== */

template int TIDL_cudaDeconv2d<int8_t,   int8_t,  uint8_t,  int32_t>(int8_t*,   int8_t*,  uint8_t*,  int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint8_t,  int8_t,  uint8_t,  int32_t>(uint8_t*,  int8_t*,  uint8_t*,  int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int8_t,   int16_t, uint16_t, int32_t>(int8_t*,   int16_t*, uint16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint8_t,  int16_t, uint16_t, int32_t>(uint8_t*,  int16_t*, uint16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int16_t,  int16_t, uint16_t, int32_t>(int16_t*,  int16_t*, uint16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint16_t, int16_t, uint16_t, int32_t>(uint16_t*, int16_t*, uint16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int8_t,   int8_t,  int8_t,   int32_t>(int8_t*,   int8_t*,  int8_t*,   int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int16_t,  int16_t, int16_t,  int32_t>(int16_t*,  int16_t*, int16_t*,  int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<float,    float,   float,    float>(float*,    float*,   float*,   float*,   float*,   float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<floating_point::bfloat16_t, signed char, short, int>(floating_point::bfloat16_t*, signed char*, short*, int*, int*, int*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<float, floating_point::bfloat16_t, float, float>(float*, floating_point::bfloat16_t*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<floating_point::bfloat16_t, floating_point::bfloat16_t, float, float>(floating_point::bfloat16_t*, floating_point::bfloat16_t*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<signed char, floating_point::bfloat16_t, float, float>(signed char*, floating_point::bfloat16_t*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<unsigned char, floating_point::bfloat16_t, float, float>(unsigned char*, floating_point::bfloat16_t*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<short, floating_point::bfloat16_t, float, float>(short*, floating_point::bfloat16_t*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<unsigned short, floating_point::bfloat16_t, float, float>(unsigned short*, floating_point::bfloat16_t*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<floating_point::bfloat16_t, short, int, long>(floating_point::bfloat16_t*, short*, int*, long*, long*, long*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaDeconv2d<floating_point::bfloat16_t, float, float, float>(floating_point::bfloat16_t*, float*, float*, float*, float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

/* Tw=int8_t, Tb=int16_t, Tacc=int32_t */
template int TIDL_cudaDeconv2d<float,    int8_t,  int16_t,  int32_t>(float*,    int8_t*,  int16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int8_t,   int8_t,  int16_t,  int32_t>(int8_t*,   int8_t*,  int16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint8_t,  int8_t,  int16_t,  int32_t>(uint8_t*,  int8_t*,  int16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int16_t,  int8_t,  int16_t,  int32_t>(int16_t*,  int8_t*,  int16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint16_t, int8_t,  int16_t,  int32_t>(uint16_t*, int8_t*,  int16_t*, int32_t*, int32_t*, int32_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);

/* Tw=int16_t, Tb=int32_t, Tacc=int64_t */
template int TIDL_cudaDeconv2d<float,    int16_t, int32_t,  int64_t>(float*,    int16_t*, int32_t*, int64_t*, int64_t*, int64_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int8_t,   int16_t, int32_t,  int64_t>(int8_t*,   int16_t*, int32_t*, int64_t*, int64_t*, int64_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint8_t,  int16_t, int32_t,  int64_t>(uint8_t*,  int16_t*, int32_t*, int64_t*, int64_t*, int64_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int16_t,  int16_t, int32_t,  int64_t>(int16_t*,  int16_t*, int32_t*, int64_t*, int64_t*, int64_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint16_t, int16_t, int32_t,  int64_t>(uint16_t*, int16_t*, int32_t*, int64_t*, int64_t*, int64_t*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);

/* Tw=float, Tb=float, Tacc=float with non-float input */
template int TIDL_cudaDeconv2d<int8_t,   float,   float,    float>(int8_t*,   float*,   float*,   float*,   float*,   float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint8_t,  float,   float,    float>(uint8_t*,  float*,   float*,   float*,   float*,   float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<int16_t,  float,   float,    float>(int16_t*,  float*,   float*,   float*,   float*,   float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaDeconv2d<uint16_t, float,   float,    float>(uint16_t*, float*,   float*,   float*,   float*,   float*,   int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
