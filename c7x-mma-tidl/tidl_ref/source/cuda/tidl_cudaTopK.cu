/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaTopK.cu
 @brief   This file contains CUDA implementation of TopK layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <float.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

/**
 * TopK kernel: one thread per independent slice (all non-axis positions).
 * Performs insertion sort to find top-K elements along the given axis,
 * matching the reference implementation exactly.
 *
 * outData layout (per batch): values at [0..indicesOffElems), indices at [indicesOffElems..)
 * indicesOffElems: offset in Tout units from the batch base to where int32 indices begin.
 */
template <class Tin, class Tout>
__global__ void TopKKernel(
    const Tin*   __restrict__ inData,
    Tout*        outData,
    int64_t      indicesOffElems,
    int32_t      K,
    int32_t      axisN,
    int32_t      largest,
    Tout         limitVal,
    /* outer dimensions (axis dim replaced by 1) */
    int32_t outerB, int32_t outerD1, int32_t outerD2,
    int32_t outerC, int32_t outerH,  int32_t outerW,
    /* input pitches */
    int32_t inBP, int32_t inD1P, int32_t inD2P, int32_t inCP, int32_t inLP,
    /* output pitches */
    int32_t outBP, int32_t outD1P, int32_t outD2P, int32_t outCP, int32_t outLP,
    /* axis stride in input and output */
    int32_t inAxisStride, int32_t outAxisStride)
{
    int32_t sliceIdx = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t totalSlices = outerB * outerD1 * outerD2 * outerC * outerH * outerW;
    if (sliceIdx >= totalSlices) return;

    /* Decompose slice index into outer coordinates */
    int32_t tmp = sliceIdx;
    int32_t ow  = tmp % outerW;  tmp /= outerW;
    int32_t oh  = tmp % outerH;  tmp /= outerH;
    int32_t oc  = tmp % outerC;  tmp /= outerC;
    int32_t od2 = tmp % outerD2; tmp /= outerD2;
    int32_t od1 = tmp % outerD1; tmp /= outerD1;
    int32_t ob  = tmp;

    /* Base offsets in input and output */
    int64_t baseIn  = (int64_t)ob * inBP  + od1 * inD1P  + od2 * inD2P  + oc * inCP  + oh * inLP  + ow;
    int64_t baseOut = (int64_t)ob * outBP + od1 * outD1P + od2 * outD2P + oc * outCP + oh * outLP + ow;

    /* Pointer to indices for this batch (int32 overlay on output buffer) */
    int32_t* indBase = (int32_t*)(outData + (int64_t)ob * outBP + indicesOffElems);
    int64_t  indOff  = (int64_t)od1 * outD1P + od2 * outD2P + oc * outCP + oh * outLP + ow;

    /* Initialize K output slots */
    for (int32_t k = 0; k < K; k++)
    {
        outData[baseOut + (int64_t)k * outAxisStride] = limitVal;
        indBase[indOff  + (int64_t)k * outAxisStride] = -1;
    }

    /* Insertion sort: find top-K over the axis dimension */
    for (int32_t i = 0; i < axisN; i++)
    {
        Tin inVal = inData[baseIn + (int64_t)i * inAxisStride];

        for (int32_t j = 0; j < K; j++)
        {
            Tout curVal = outData[baseOut + (int64_t)j * outAxisStride];
            bool insert = largest ? ((float)inVal > (float)curVal)
                                  : ((float)inVal < (float)curVal);
            if (!insert) continue;

            /* Skip duplicate value with valid index (matches ref behaviour) */
            if ((float)inVal == (float)curVal &&
                indBase[indOff + (int64_t)j * outAxisStride] >= 0)
            {
                continue;
            }

            /* Shift existing entries one position down */
            for (int32_t s = K - 1; s >= j + 1; s--)
            {
                outData[baseOut + (int64_t)s * outAxisStride] =
                    outData[baseOut + (int64_t)(s - 1) * outAxisStride];
                indBase[indOff  + (int64_t)s * outAxisStride] =
                    indBase[indOff  + (int64_t)(s - 1) * outAxisStride];
            }
            outData[baseOut + (int64_t)j * outAxisStride] = (Tout)inVal;
            indBase[indOff  + (int64_t)j * outAxisStride] = i;
            break;
        }
    }
}

/**
 * CUDA wrapper for TopK.
 *
 * outData: combined output buffer containing values followed by int32 indices
 *          (same layout as the reference: indices at outData + indicesPitch per batch).
 * indicesPitch: offset in Tout-element units from the batch base to where indices begin.
 */
template <class Tin, class Tout>
int TIDL_cudaTopK(
    const Tin*   input,
    Tout*        outData,
    int64_t      indicesPitch,
    int32_t      batches, int32_t dim1, int32_t dim2, int32_t channels, int32_t height, int32_t width,
    int32_t      inBatchPitch, int32_t inDim1Pitch, int32_t inDim2Pitch, int32_t inChPitch, int32_t inLinePitch,
    int32_t      outBatchPitch, int32_t outDim1Pitch, int32_t outDim2Pitch, int32_t outChPitch, int32_t outLinePitch,
    int32_t      K, int32_t axis, int32_t largest)
{
    size_t inSize  = (size_t)batches * inBatchPitch  * sizeof(Tin);
    size_t outSize = (size_t)batches * outBatchPitch * sizeof(Tout);

    Tin*   d_input  = NULL;
    Tout*  d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), (void*)input, (void**)&d_input, inSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), (void*)input, (void**)&d_input, inSize) != IALG_EOK)
        {
            printf("TopK: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), outData, (void**)&d_output, outSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), outData, (void**)&d_output, outSize) != IALG_EOK)
        {
            printf("TopK: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, inSize, cudaMemcpyHostToDevice, stream));

    /* Outer dimensions: set the axis dimension to 1 so outer loops skip it */
    int32_t outerB  = batches;
    int32_t outerD1 = (axis == TIDL_DIM_DIM1)  ? 1 : dim1;
    int32_t outerD2 = (axis == TIDL_DIM_DIM2)  ? 1 : dim2;
    int32_t outerC  = (axis == TIDL_DIM_NUMCH) ? 1 : channels;
    int32_t outerH  = (axis == TIDL_DIM_HEIGHT)? 1 : height;
    int32_t outerW  = (axis == TIDL_DIM_WIDTH) ? 1 : width;

    /* axisN: size of the dimension being reduced */
    int32_t axisN = 0;
    switch (axis)
    {
        case TIDL_DIM_DIM1:   axisN = dim1;     break;
        case TIDL_DIM_DIM2:   axisN = dim2;     break;
        case TIDL_DIM_NUMCH:  axisN = channels; break;
        case TIDL_DIM_HEIGHT: axisN = height;   break;
        case TIDL_DIM_WIDTH:  axisN = width;    break;
        default:              axisN = width;    break;
    }

    /* Stride along the axis in input and output (in element units) */
    int32_t inAxisStride  = 1;
    int32_t outAxisStride = 1;
    switch (axis)
    {
        case TIDL_DIM_DIM1:   inAxisStride = inDim1Pitch;  outAxisStride = outDim1Pitch;  break;
        case TIDL_DIM_DIM2:   inAxisStride = inDim2Pitch;  outAxisStride = outDim2Pitch;  break;
        case TIDL_DIM_NUMCH:  inAxisStride = inChPitch;    outAxisStride = outChPitch;    break;
        case TIDL_DIM_HEIGHT: inAxisStride = inLinePitch;  outAxisStride = outLinePitch;  break;
        case TIDL_DIM_WIDTH:  inAxisStride = 1;            outAxisStride = 1;             break;
        default:              inAxisStride = 1;            outAxisStride = 1;             break;
    }

    /* Limit value for output initialisation */
    Tout limitVal;
    if (largest)
        limitVal = cuda::std::numeric_limits<Tout>::lowest();
    else
        limitVal = cuda::std::numeric_limits<Tout>::max();

    int32_t totalSlices = outerB * outerD1 * outerD2 * outerC * outerH * outerW;
    int32_t gridSize    = (totalSlices + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    TopKKernel<Tin, Tout><<<gridSize, THREADS_PER_BLOCK, 0, stream>>>(
        (const Tin*)d_input, (Tout*)d_output, indicesPitch,
        K, axisN, largest, limitVal,
        outerB, outerD1, outerD2, outerC, outerH, outerW,
        inBatchPitch, inDim1Pitch, inDim2Pitch, inChPitch, inLinePitch,
        outBatchPitch, outDim1Pitch, outDim2Pitch, outChPitch, outLinePitch,
        inAxisStride, outAxisStride);

    checkCudaErr(cudaMemcpyAsync(outData, d_output, outSize, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

/* Explicit instantiations */
template int TIDL_cudaTopK<float,          float         >(const float*,          float*,         int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaTopK<bfloat16_tidl,  bfloat16_tidl >(const bfloat16_tidl*,  bfloat16_tidl*, int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaTopK<uint8_t,        uint8_t       >(const uint8_t*,        uint8_t*,       int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaTopK<int8_t,         int8_t        >(const int8_t*,         int8_t*,        int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaTopK<uint16_t,       uint16_t      >(const uint16_t*,       uint16_t*,      int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaTopK<int16_t,        int16_t       >(const int16_t*,        int16_t*,       int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaTopK<uint8_t,        uint16_t      >(const uint8_t*,        uint16_t*,      int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
template int TIDL_cudaTopK<int8_t,         int16_t       >(const int8_t*,         int16_t*,       int64_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
