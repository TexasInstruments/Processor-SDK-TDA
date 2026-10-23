/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaRmsNorm.cu
 @brief   This file contains CUDA implementation of RMSNorm layer.
 @version 0.1 (Feb 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include <type_traits>
#include "tidl_cuda_mem_manager.h"

using namespace floating_point::bf16_c7x;

/**
 * General RMSNorm kernel for float and bfloat16.
 *
 * The axis parameter determines what is normalised:
 *   axis=WIDTH   (5): each (b,d1,d2,c,h) row of width W
 *   axis=HEIGHT  (4): each (b,d1,d2,c) slice of height*width H*W
 *   axis=NUMCH   (3): each (b,d1,d2) volume of channel*H*W
 *   axis=DIM2    (2): each (b,d1) block of dim2*channel*H*W
 *   axis=DIM1    (1): each (b) tensor of dim1*dim2*channel*H*W
 *
 * Thread block: 2D (THREADS_PER_BLOCK_X × THREADS_PER_BLOCK_Y).
 * tx  = which normalization group this warp-row handles.
 * ty  = which subset of inner elements this thread handles (reduced via shared memory).
 */
template <class Tin, class Tout>
__global__ void RmsNormFloatKernel(
    const Tin* __restrict__ input,
    Tout* __restrict__ output,
    int32_t B, int32_t D1, int32_t D2, int32_t C, int32_t H, int32_t W,
    int32_t bP, int32_t d1P, int32_t d2P, int32_t cP, int32_t lP,
    int32_t axis,
    float   epsilon)
{
    int32_t idx = blockIdx.x * blockDim.x + threadIdx.x;  /* group index */
    int32_t tx  = threadIdx.x;
    int32_t ty  = threadIdx.y;

    /* ---- compute outer (group) and inner (reduce) sizes based on axis ---- */
    int32_t outer_d1 = (axis <= TIDL_DIM_DIM1)  ? 1 : D1;
    int32_t outer_d2 = (axis <= TIDL_DIM_DIM2)  ? 1 : D2;
    int32_t outer_c  = (axis <= TIDL_DIM_NUMCH) ? 1 : C;
    int32_t outer_h  = (axis <= TIDL_DIM_HEIGHT)? 1 : H;
    /* WIDTH is always in the inner set */

    int32_t outer_size = B * outer_d1 * outer_d2 * outer_c * outer_h;
    if (idx >= outer_size) return;

    int32_t inner_d1 = (axis <= TIDL_DIM_DIM1)  ? D1 : 1;
    int32_t inner_d2 = (axis <= TIDL_DIM_DIM2)  ? D2 : 1;
    int32_t inner_c  = (axis <= TIDL_DIM_NUMCH) ? C  : 1;
    int32_t inner_h  = (axis <= TIDL_DIM_HEIGHT)? H  : 1;
    int32_t inner_w  = W;
    int32_t inner_size = inner_d1 * inner_d2 * inner_c * inner_h * inner_w;

    /* ---- decompose group index into outer coordinates ---- */
    int32_t tmp = idx;
    int32_t oh  = tmp % outer_h;  tmp /= outer_h;
    int32_t oc  = tmp % outer_c;  tmp /= outer_c;
    int32_t od2 = tmp % outer_d2; tmp /= outer_d2;
    int32_t od1 = tmp % outer_d1; tmp /= outer_d1;
    int32_t ob  = tmp;

    int64_t base = (int64_t)ob * bP + od1 * d1P + od2 * d2P + oc * cP + oh * lP;

    /* ---- Phase 1: accumulate sum(x^2) over inner elements ---- */
    __shared__ float reduction[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    float acc = 0.0f;

    for (int32_t i = ty; i < inner_size; i += blockDim.y)
    {
        int32_t t   = i;
        int32_t iw  = t % inner_w;  t /= inner_w;
        int32_t ih  = t % inner_h;  t /= inner_h;
        int32_t ic  = t % inner_c;  t /= inner_c;
        int32_t id2 = t % inner_d2; t /= inner_d2;
        int32_t id1 = t;

        int64_t off = base + id1 * d1P + id2 * d2P + ic * cP + ih * lP + iw;
        float   x   = (float)input[off];
        acc += x * x;
    }

    reduction[tx][ty] = acc;

    for (int32_t stride = blockDim.y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if (ty < stride) reduction[tx][ty] += reduction[tx][ty + stride];
    }
    __syncthreads();

    float rms = sqrtf(reduction[tx][0] / (float)inner_size + epsilon);

    /* ---- Phase 2: normalize ---- */
    for (int32_t i = ty; i < inner_size; i += blockDim.y)
    {
        int32_t t   = i;
        int32_t iw  = t % inner_w;  t /= inner_w;
        int32_t ih  = t % inner_h;  t /= inner_h;
        int32_t ic  = t % inner_c;  t /= inner_c;
        int32_t id2 = t % inner_d2; t /= inner_d2;
        int32_t id1 = t;

        int64_t off = base + id1 * d1P + id2 * d2P + ic * cP + ih * lP + iw;
        float   x   = (float)input[off];

        if (std::is_same<Tout, bfloat16_tidl>::value)
            output[off] = (Tout)cuda_bf16Sat(x / rms, -FLT_MAX, FLT_MAX);
        else
            output[off] = (Tout)(x / rms);
    }
}

/**
 * CUDA wrapper for RMSNorm (float / bfloat16 paths).
 */
template <class Tin, class Tout>
int TIDL_cudaRmsNormFloat(
    Tin*    input,
    Tout*   output,
    int32_t axis,
    int32_t batches, int32_t dim1, int32_t dim2, int32_t channels, int32_t height, int32_t width,
    int32_t batchPitch, int32_t dim1Pitch, int32_t dim2Pitch, int32_t chPitch, int32_t linePitch,
    float   epsilon)
{
    size_t bufSize = (size_t)batches * batchPitch * sizeof(Tin);

    Tin*  d_input  = NULL;
    Tout* d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, bufSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, bufSize) != IALG_EOK)
        {
            printf("RmsNorm: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    size_t outBufSize = (size_t)batches * batchPitch * sizeof(Tout);
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, outBufSize) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, outBufSize) != IALG_EOK)
        {
            printf("RmsNorm: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, bufSize, cudaMemcpyHostToDevice, stream));

    /* outer_size = number of independent normalization groups */
    int32_t outer_d1 = (axis <= TIDL_DIM_DIM1)  ? 1 : dim1;
    int32_t outer_d2 = (axis <= TIDL_DIM_DIM2)  ? 1 : dim2;
    int32_t outer_c  = (axis <= TIDL_DIM_NUMCH) ? 1 : channels;
    int32_t outer_h  = (axis <= TIDL_DIM_HEIGHT)? 1 : height;
    int32_t outer_size = batches * outer_d1 * outer_d2 * outer_c * outer_h;

    dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(outer_size, THREADS_PER_BLOCK_X));

    RmsNormFloatKernel<Tin, Tout><<<gridDim, blockDim, 0, stream>>>(
        d_input, d_output,
        batches, dim1, dim2, channels, height, width,
        batchPitch, dim1Pitch, dim2Pitch, chPitch, linePitch,
        axis, epsilon);

    checkCudaErr(cudaMemcpyAsync(output, d_output, outBufSize, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

/* Explicit instantiations */
template int TIDL_cudaRmsNormFloat<float,         float        >(float*,         float*,         int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, float);
template int TIDL_cudaRmsNormFloat<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl*, bfloat16_tidl*, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, float);
