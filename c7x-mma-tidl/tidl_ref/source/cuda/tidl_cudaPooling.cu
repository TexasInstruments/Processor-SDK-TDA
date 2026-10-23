/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaPooling.cu
 @brief   This file contains CUDA implementation of MaxPool and AvgPool layers.
          Covers both spatial (windowed) and global pooling variants.
 @version 0.1 (Jun 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include <type_traits>
#include "tidl_cuda_mem_manager.h"

__device__ __forceinline__ int32_t cuda_checkBorderPixel(
    int32_t posY, int32_t posX,
    int32_t validPosXMin, int32_t validPosXMax,
    int32_t validPosYMin, int32_t validPosYMax)
{
    return ((posY  <  validPosYMin) || (posY  >= validPosYMax) ||
            (posX  <  validPosXMin) || (posX  >= validPosXMax)) ? 1 : 0;
}

template <class Tin>
__global__ void SpatialMaxPoolKernel(
    const Tin* __restrict__ input,
    Tin*       __restrict__ output,
    int32_t numBatches, int32_t numChannels,
    int32_t outHeight,  int32_t outWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch, int32_t outPitch,
    int32_t kernelH, int32_t kernelW,
    int32_t strideH,  int32_t strideW,
    int32_t startRowNumberInTensor,
    int32_t validPosXMin, int32_t validPosXMax,
    int32_t validPosYMin, int32_t validPosYMax)
{
    int32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t total_elements = numBatches * numChannels * outHeight * outWidth;

    if (idx >= total_elements) return;

    int32_t b  = idx / (numChannels * outHeight * outWidth);
    int32_t c  = (idx / (outHeight * outWidth)) % numChannels;
    int32_t oh = (idx / outWidth) % outHeight;
    int32_t ow = idx % outWidth;

    Tin maxVal = cuda::std::numeric_limits<Tin>::lowest();

    for (int32_t kh = 0; kh < kernelH; kh++)
    {
        for (int32_t kw = 0; kw < kernelW; kw++)
        {
            int32_t spatialOffsetY = (oh * strideH) + kh;
            int32_t spatialOffsetX = (ow * strideW) + kw;

            int32_t isBorderPixel = cuda_checkBorderPixel(
                spatialOffsetY + startRowNumberInTensor, spatialOffsetX,
                validPosXMin, validPosXMax, validPosYMin, validPosYMax);

            Tin inVal = isBorderPixel
                            ? cuda::std::numeric_limits<Tin>::lowest()
                            : input[(b * inBatchPitch) + (c * inChPitch) +
                                    (spatialOffsetY * inPitch) + spatialOffsetX];
            maxVal = max(maxVal, inVal);
        }
    }

    output[(b * outBatchPitch) + (c * outChPitch) + (oh * outPitch) + ow] = maxVal;
}

template <class Tin, class Tacc>
__global__ void SpatialAvgPoolComputeKernel(
    const Tin*  __restrict__ input,
    Tacc*       __restrict__ accumulator,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,  int32_t inWidth,
    int32_t outHeight, int32_t outWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch, int32_t outPitch,
    int32_t kernelH, int32_t kernelW,
    int32_t strideH,  int32_t strideW,
    int32_t padH,     int32_t padW,
    int32_t roundVal, int32_t isOTFpad,
    int32_t countIncludePad,
    int32_t startRowNumberInTensor,
    int32_t validPosXMin, int32_t validPosXMax,
    int32_t validPosYMin, int32_t validPosYMax)
{
    int32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t total_elements = numBatches * numChannels * outHeight * outWidth;

    if (idx >= total_elements) return;

    int32_t b  = idx / (numChannels * outHeight * outWidth);
    int32_t c  = (idx / (outHeight * outWidth)) % numChannels;
    int32_t oh = (idx / outWidth) % outHeight;
    int32_t ow = idx % outWidth;

    int32_t startH = oh * strideH - padH;
    int32_t startW = ow * strideW - padW;
    int32_t endH   = startH + kernelH;
    int32_t endW   = startW + kernelW;

    int32_t clampedStartH = max(startH, 0);
    int32_t clampedEndH   = min(endH, inHeight);
    int32_t clampedStartW = max(startW, 0);
    int32_t clampedEndW   = min(endW, inWidth);

    int32_t poolSize;
    if (countIncludePad)
    {
        poolSize = kernelH * kernelW;
    }
    else
    {
        poolSize = (clampedEndH - clampedStartH) * (clampedEndW - clampedStartW);
    }

    Tacc sumBlock = 0;
    for (int32_t ih = clampedStartH; ih < clampedEndH; ih++)
    {
        for (int32_t iw = clampedStartW; iw < clampedEndW; iw++)
        {
            int32_t isBorderPixel = cuda_checkBorderPixel(
                ih + startRowNumberInTensor, iw,
                validPosXMin, validPosXMax, validPosYMin, validPosYMax);

            Tacc inVal = isBorderPixel
                             ? (Tacc)0
                             : (Tacc)input[(b * inBatchPitch) + (c * inChPitch) +
                                           (ih * inPitch) + iw];
            sumBlock = sumBlock + inVal;
        }
    }

    Tacc result;
    if (std::is_floating_point<Tacc>::value || std::is_same<Tacc, bfloat16_tidl>::value)
    {
        result = sumBlock * (Tacc)(1.0f / (float)poolSize);
    }
    else
    {
        result = sumBlock * (Tacc)(((uint32_t)1 << (uint32_t)roundVal) / (uint32_t)poolSize);
    }

    accumulator[(b * outBatchPitch) + (c * outChPitch) + (oh * outPitch) + ow] = result;
}

// ============================================================================
// Kernel: Pooling Quantize
// Shared by spatial avg pool and global avg pool quantization pass.
// Reads from accumulator, applies round/saturate, writes to output.
// For float output: applies float saturation clamp.
// For integer output: applies fixed-point round-and-saturate with optional
//   8-bit right-shift for mixed-precision paths.
// ============================================================================
template <class Tacc, class Tout>
__global__ void PoolingQuantizeKernel(
    const Tacc* __restrict__ accumulator,
    Tout*       __restrict__ output,
    int32_t numBatches, int32_t numChannels,
    int32_t outHeight,  int32_t outWidth,
    int32_t outBatchPitch, int32_t outChPitch, int32_t outPitch,
    int32_t roundBits,
    int32_t satLow,  int32_t satHigh,
    int32_t mixedPrecision,
    float   floatSatLow, float floatSatHigh)
{
    int32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t total_elements = numBatches * numChannels * outHeight * outWidth;

    if (idx >= total_elements) return;

    int32_t b  = idx / (numChannels * outHeight * outWidth);
    int32_t c  = (idx / (outHeight * outWidth)) % numChannels;
    int32_t oh = (idx / outWidth) % outHeight;
    int32_t ow = idx % outWidth;

    uint32_t offset = (b * outBatchPitch) + (c * outChPitch) + (oh * outPitch) + ow;
    Tacc result = accumulator[offset];

    if (std::is_same<Tout, bfloat16_tidl>::value)
    {
        // BFloat16 Saturation - similar to TIDL_BF16Sat.
        // Branch on Tout only: (float)result converts correctly whether the
        // accumulator (Tacc) is float or bfloat16_tidl, and std::is_floating_point
        // is false for the bfloat16_tidl class type, so it must not gate this path.
        result = (Tacc)cuda_bf16Sat((float)result, floatSatLow, floatSatHigh);
    }
    else if (std::is_floating_point<Tout>::value)
    {
        result = (Tacc)cuda_floatSat((float)result, floatSatHigh, floatSatLow);
    }
    else
    {
        result = (Tacc)cuda_roundSat((int64_t)result, roundBits, (float)satLow, (float)satHigh);
        if (mixedPrecision == 1)
        {
            result = (int64_t)result >> 8;
        }
    }

    output[offset] = (Tout)result;
}

template <class Tin>
__global__ void GlobalMaxPoolKernel(
    const Tin* __restrict__ input,
    Tin*       __restrict__ output,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,   int32_t inWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch)
{
    int32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t tx = threadIdx.x, ty = threadIdx.y;
    int32_t total_elements = numBatches * numChannels;

    if (idx >= total_elements) return;

    int32_t b = (idx / numChannels);
    int32_t c = (idx % numChannels);

    __shared__ Tin reduction_max[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    Tin maxVal = cuda::std::numeric_limits<Tin>::lowest();

    for (int32_t i = ty; i < inHeight * inWidth; i += THREADS_PER_BLOCK_Y)
    {
        int32_t h = i / inWidth;
        int32_t w = i % inWidth;
        Tin inVal = input[(b * inBatchPitch) + (c * inChPitch) + (h * inPitch) + w];
        maxVal = max(maxVal, inVal);
    }
    reduction_max[tx][ty] = maxVal;

    for (int32_t stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if (ty < stride)
            reduction_max[tx][ty] = max(reduction_max[tx][ty], reduction_max[tx][ty + stride]);
    }
    __syncthreads();

    if (ty == 0)
        output[(b * outBatchPitch) + (c * outChPitch)] = reduction_max[tx][0];
}

template <class Tin, class Tacc, class Tscale>
__global__ void GlobalAvgPoolComputeKernel(
    const Tin*  __restrict__ input,
    Tacc*       __restrict__ accumulator,
    Tscale scaleValue,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,   int32_t inWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch)
{
    int32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t tx = threadIdx.x, ty = threadIdx.y;
    int32_t total_elements = numBatches * numChannels;

    if (idx >= total_elements) return;

    int32_t b = (idx / numChannels);
    int32_t c = (idx % numChannels);

    __shared__ Tacc reduction_sum[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    Tacc sumBlock = 0;
    for (int32_t i = ty; i < inHeight * inWidth; i += THREADS_PER_BLOCK_Y)
    {
        int32_t h = i / inWidth;
        int32_t w = i % inWidth;
        sumBlock += (Tacc)input[(b * inBatchPitch) + (c * inChPitch) + (h * inPitch) + w];
    }
    reduction_sum[tx][ty] = sumBlock;

    for (int32_t stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if (ty < stride)
            reduction_sum[tx][ty] += reduction_sum[tx][ty + stride];
    }
    __syncthreads();

    if (ty == 0)
        accumulator[(b * outBatchPitch) + (c * outChPitch)] = (Tacc)(reduction_sum[tx][0] * (Tacc)scaleValue);
}

template <class Tin, class Tacc>
__global__ void GlobalAvgPoolComputeKernelv2(
    const Tin*  __restrict__ input,
    Tacc*       __restrict__ accumulator,
    uint8_t  mmaScale,
    int32_t  biasTerm,
    int32_t  numBatches, int32_t numChannels,
    int32_t  inHeight,   int32_t inWidth,
    int32_t  inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t  outBatchPitch, int32_t outChPitch)
{
    int32_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    int32_t tx = threadIdx.x, ty = threadIdx.y;
    int32_t total_elements = numBatches * numChannels;

    if (idx >= total_elements) return;

    int32_t b = (idx / numChannels);
    int32_t c = (idx % numChannels);

    __shared__ Tacc reduction_sum[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    Tacc sumBlock = 0;
    for (int32_t i = ty; i < inHeight * inWidth; i += THREADS_PER_BLOCK_Y)
    {
        int32_t h = i / inWidth;
        int32_t w = i % inWidth;
        sumBlock += (Tacc)input[(b * inBatchPitch) + (c * inChPitch) + (h * inPitch) + w];
    }
    reduction_sum[tx][ty] = sumBlock;

    for (int32_t stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if (ty < stride)
            reduction_sum[tx][ty] += reduction_sum[tx][ty + stride];
    }
    __syncthreads();

    if (ty == 0)
    {
        Tacc result = reduction_sum[tx][0] + (Tacc)biasTerm;
        accumulator[(b * outBatchPitch) + (c * outChPitch)] = result * (Tacc)mmaScale;
    }
}

template <class Tin>
int TIDL_cudaSpatialMaxPooling(
    const Tin* input,
    Tin*       output,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,   int32_t inWidth,
    int32_t outHeight,  int32_t outWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch, int32_t outPitch,
    int32_t kernelH, int32_t kernelW,
    int32_t strideH,  int32_t strideW,
    int32_t startRowNumberInTensor,
    int32_t validPosXMin, int32_t validPosXMax,
    int32_t validPosYMin, int32_t validPosYMax, int32_t padT, int32_t padL)
{
    size_t input_size  = sizeof(Tin) * (numBatches * inBatchPitch);
    size_t output_size = sizeof(Tin) * (numBatches * outBatchPitch);

    Tin* d_input  = NULL;
    Tin* d_output = NULL;

    // pad_offset is the backward shift applied to the input pointer before this function
    // was called (OTF padding: inData -= padT * inPitch + padL).  We must not read from
    // addresses before the real CPU allocation, so allocate a padded GPU buffer and copy
    // only the real data to the correct offset inside it.
    size_t pad_offset_elems = (size_t)padT * inPitch + padL;
    size_t padded_input_size = input_size + pad_offset_elems * sizeof(Tin);

    const Tin* input_base = input + pad_offset_elems;
    bool isInputAllocated = 0;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, padded_input_size) != IALG_EOK)
    {
        // Use the original (non-adjusted) pointer as the allocation key so the memory
        // manager does not attempt to dereference the backward-shifted address.
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input_base, (void**)&d_input, padded_input_size) != IALG_EOK)
        {
            printf("SpatialMaxPool: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
        isInputAllocated = 1;
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("SpatialMaxPool: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());
    // d_input points to the virtual-pad start in GPU memory.
    // Copy only the real data to d_input + pad_offset (safe: reads from valid CPU address).
    if (isInputAllocated)
    {
        checkCudaErr(cudaMemcpyAsync(d_input + pad_offset_elems, input_base, input_size, cudaMemcpyHostToDevice, stream));
    }
    else
    {
       checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream)); 
    }
    
    checkCudaErr(cudaMemcpyAsync(d_output, output, output_size, cudaMemcpyHostToDevice, stream));
    
    int32_t total_elements = numBatches * numChannels * outHeight * outWidth;
    int32_t grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    SpatialMaxPoolKernel<Tin><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_input, d_output,
        numBatches, numChannels, outHeight, outWidth,
        inBatchPitch, inChPitch, inPitch,
        outBatchPitch, outChPitch, outPitch,
        kernelH, kernelW, strideH, strideW,
        startRowNumberInTensor,
        validPosXMin, validPosXMax, validPosYMin, validPosYMax);

    checkCudaErr(cudaGetLastError());

    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template <class Tin, class Tacc>
int TIDL_cudaSpatialAvgPooling(
    const Tin*  input,
    Tacc*       accumulator,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,   int32_t inWidth,
    int32_t outHeight,  int32_t outWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch, int32_t outPitch,
    int32_t kernelH, int32_t kernelW,
    int32_t strideH,  int32_t strideW,
    int32_t padH,     int32_t padW,
    int32_t roundVal, int32_t isOTFpad,
    int32_t countIncludePad,
    int32_t startRowNumberInTensor,
    int32_t validPosXMin, int32_t validPosXMax,
    int32_t validPosYMin, int32_t validPosYMax, int32_t padT, int32_t padL)
{
    size_t input_size  = sizeof(Tin) * (numBatches * inBatchPitch);
    size_t accum_size = sizeof(Tacc) * (numBatches * outBatchPitch);

    Tin* d_input  = NULL;
    Tacc* d_accumulator = NULL;

    input_size += numBatches * numChannels * (padT * (inWidth + padL)) * sizeof(Tin);

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("SpatialAvgPool: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
        {
            printf("SpatialAvgPool: Unable to find accumulator pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemsetAsync(d_accumulator, 0, accum_size, stream));

    int32_t total_elements = numBatches * numChannels * outHeight * outWidth;
    int32_t grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    SpatialAvgPoolComputeKernel<Tin, Tacc><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_input, d_accumulator,
        numBatches, numChannels,
        inHeight, inWidth, outHeight, outWidth,
        inBatchPitch, inChPitch, inPitch,
        outBatchPitch, outChPitch, outPitch,
        kernelH, kernelW, strideH, strideW,
        padH, padW, roundVal, isOTFpad,
        countIncludePad,
        startRowNumberInTensor,
        validPosXMin, validPosXMax, validPosYMin, validPosYMax);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template <class Tacc, class Tout>
int TIDL_cudaPoolingQuantize(
    const Tacc* accumulator,
    Tout*       output,
    int32_t numBatches, int32_t numChannels,
    int32_t outHeight,  int32_t outWidth,
    int32_t outBatchPitch, int32_t outChPitch, int32_t outPitch,
    int32_t roundBits,
    int32_t satLow,   int32_t satHigh,
    int32_t mixedPrecision,
    float   floatSatLow, float floatSatHigh)
{
    size_t accum_size  = sizeof(Tacc) * (numBatches * outBatchPitch);
    size_t output_size = sizeof(Tout) * (numBatches  * outBatchPitch);

    Tacc* d_accumulator = NULL;
    Tout* d_output      = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
    {
        printf("PoolingQuantize: Unable to find accumulator pointer\n");
        return IALG_EFAIL;
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("PoolingQuantize: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_output, output, output_size, cudaMemcpyHostToDevice, stream));

    int32_t total_elements = numBatches * numChannels * outHeight * outWidth;
    int32_t grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    PoolingQuantizeKernel<Tacc, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_accumulator, d_output,
        numBatches, numChannels, outHeight, outWidth,
        outBatchPitch, outChPitch, outPitch,
        roundBits, satLow, satHigh, mixedPrecision,
        floatSatLow, floatSatHigh);

    checkCudaErr(cudaGetLastError());

    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template <class Tin>
int TIDL_cudaGlobalMaxPooling(
    const Tin* input,
    Tin*       output,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,   int32_t inWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch)
{
    size_t input_size  = sizeof(Tin) * (numBatches * inBatchPitch);
    size_t output_size = sizeof(Tin) * (numBatches * outBatchPitch);
    
    Tin* d_input  = NULL;
    Tin* d_output = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("GlobalMaxPool: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("GlobalMaxPool: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    int32_t total_elements = numBatches * numChannels;
    dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    GlobalMaxPoolKernel<Tin><<<gridDim, blockDim, 0, stream>>>(
        d_input, d_output,
        numBatches, numChannels, inHeight, inWidth,
        inBatchPitch, inChPitch, inPitch,
        outBatchPitch, outChPitch);

    checkCudaErr(cudaGetLastError());

    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template <class Tin, class Tacc, class Tscale>
int TIDL_cudaGlobalAvgPooling(
    const Tin* input,
    Tacc*      accumulator,
    Tscale     scaleValue,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,   int32_t inWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch)
{
    size_t input_size  = sizeof(Tin) * (numBatches * inBatchPitch);
    size_t accum_size  = sizeof(Tacc) * (numBatches * outBatchPitch);

    Tin*  d_input       = NULL;
    Tacc* d_accumulator = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("GlobalAvgPool: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
        {
            printf("GlobalAvgPool: Unable to find accumulator pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemsetAsync(d_accumulator, 0, accum_size, stream));

    int32_t total_elements = numBatches * numChannels;
    dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    GlobalAvgPoolComputeKernel<Tin, Tacc, Tscale><<<gridDim, blockDim, 0, stream>>>(
        d_input, d_accumulator, scaleValue,
        numBatches, numChannels, inHeight, inWidth,
        inBatchPitch, inChPitch, inPitch,
        outBatchPitch, outChPitch);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template <class Tin, class Tacc>
int TIDL_cudaGlobalAvgPoolingv2(
    const Tin* input,
    Tacc*      accumulator,
    uint8_t    mmaScale,
    int32_t    biasTerm,
    int32_t numBatches, int32_t numChannels,
    int32_t inHeight,   int32_t inWidth,
    int32_t inBatchPitch, int32_t inChPitch, int32_t inPitch,
    int32_t outBatchPitch, int32_t outChPitch)
{
    size_t input_size  = sizeof(Tin) * (numBatches * inBatchPitch);
    size_t accum_size  = sizeof(Tacc) * (numBatches * outBatchPitch);

    Tin*  d_input       = NULL;
    Tacc* d_accumulator = NULL;

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("GlobalAvgPoolv2: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), accumulator, (void**)&d_accumulator, accum_size) != IALG_EOK)
        {
            printf("GlobalAvgPoolv2: Unable to find accumulator pointer\n");
            return IALG_EFAIL;
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(
        TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));
    checkCudaErr(cudaMemsetAsync(d_accumulator, 0, accum_size, stream));

    int32_t total_elements = numBatches * numChannels;
    dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    GlobalAvgPoolComputeKernelv2<Tin, Tacc><<<gridDim, blockDim, 0, stream>>>(
        d_input, d_accumulator, mmaScale, biasTerm,
        numBatches, numChannels, inHeight, inWidth,
        inBatchPitch, inChPitch, inPitch,
        outBatchPitch, outChPitch);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}


// ============================================================================
// Explicit template instantiations
// ============================================================================

// --- Spatial Max Pool ---
template int TIDL_cudaSpatialMaxPooling<uint8_t>(uint8_t const*, uint8_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialMaxPooling<int8_t>(int8_t const*, int8_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialMaxPooling<uint16_t>(uint16_t const*, uint16_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialMaxPooling<int16_t>(int16_t const*, int16_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialMaxPooling<float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialMaxPooling<floating_point::bfloat16_t>(floating_point::bfloat16_t const*, floating_point::bfloat16_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

// --- Spatial Avg Pool Compute ---
template int TIDL_cudaSpatialAvgPooling<uint8_t,  int32_t>(uint8_t const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialAvgPooling<uint8_t,  int64_t>(uint8_t const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialAvgPooling<int8_t,   int32_t>(int8_t const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialAvgPooling<uint16_t, int32_t>(uint16_t const*, int32_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialAvgPooling<uint16_t, int64_t>(uint16_t const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialAvgPooling<int16_t,  int64_t>(int16_t const*, int64_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialAvgPooling<float,    float>  (float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaSpatialAvgPooling<floating_point::bfloat16_t, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, floating_point::bfloat16_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);

// --- Pooling Quantize (shared for spatial and global avg pool) ---
template int TIDL_cudaPoolingQuantize<int32_t,  uint8_t> (int32_t const*, uint8_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<uint32_t, uint8_t> (uint32_t const*, uint8_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<int32_t,  int8_t>  (int32_t const*, int8_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<int64_t,  uint16_t>(int64_t const*, uint16_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<uint64_t, uint16_t>(uint64_t const*, uint16_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<int64_t,  int16_t> (int64_t const*, int16_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<int64_t,  uint8_t> (int64_t const*, uint8_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<float,   float>   (float const*, float*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<float, floating_point::bfloat16_t>(float const*, floating_point::bfloat16_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);
template int TIDL_cudaPoolingQuantize<floating_point::bfloat16_t, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, floating_point::bfloat16_t*, int, int, int, int, int, int, int, int, int, int, int, float, float);

// --- Global Max Pool ---
template int TIDL_cudaGlobalMaxPooling<uint8_t> (uint8_t const*, uint8_t*, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalMaxPooling<int8_t>  (int8_t const*, int8_t*, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalMaxPooling<uint16_t>(uint16_t const*, uint16_t*, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalMaxPooling<int16_t> (int16_t const*, int16_t*, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalMaxPooling<float>   (float const*, float*, int, int, int, int, int, int, int, int, int);

// --- Global Avg Pool Compute v1 ---
template int TIDL_cudaGlobalAvgPooling<uint8_t,  int32_t,  uint32_t>(uint8_t const*, int32_t*, uint32_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<uint8_t,  uint32_t, uint32_t>(uint8_t const*, uint32_t*, uint32_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<uint8_t,  int64_t,  uint64_t>(uint8_t const*, int64_t*, uint64_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<uint8_t,  uint64_t, uint64_t>(uint8_t const*, uint64_t*, uint64_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<int8_t,   int32_t, int32_t> (int8_t const*, int32_t*, int32_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<int8_t,   int64_t, int64_t> (int8_t const*, int64_t*, int64_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<uint16_t, int64_t,  uint64_t>(uint16_t const*, int64_t*, uint64_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<uint16_t, uint64_t, uint64_t>(uint16_t const*, uint64_t*, uint64_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<int16_t,  int64_t, int64_t> (int16_t const*, int64_t*, int64_t, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<float,    float,   float>   (float const*, float*, float, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGlobalAvgPooling<floating_point::bfloat16_t, float, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, float*, floating_point::bfloat16_t, int, int, int, int, int, int, int, int, int);

// --- Global Avg Pool Compute v2 (TFL asymmetric) ---
template int TIDL_cudaGlobalAvgPoolingv2<int8_t, int32_t>(int8_t const*, int32_t*, uint8_t, int, int, int, int, int, int, int, int, int, int);
