/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaLayerNorm.cu
 @brief   This file contains CUDA implementation of LayerNorm layer.
 @version 0.1 (Jan 2026) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include <type_traits>
#include "tidl_cuda_mem_manager.h"
#include "tidl_errors.h"

using namespace floating_point::bf16_c7x;

#define RSQRT_MASK (0xFFFFFFFFFFFF8000)
/* External declaration of the global memory manager pointer */
extern TIDL_CudaMemManager* TIDL_cudaGetThreadManager();

/**
 * CUDA kernel for LayerNorm floating point implementation
 * This kernel normalizes along the width dimension (axis = TIDL_DIM_WIDTH)
 */
__global__ void LayerNormFloatKernel(
    const float* __restrict__ input,
    float* __restrict__ output,
    int numBatches, int numDim1, int numDim2, int numChannels, int height, int width,
    int batchPitch, int dim1Pitch, int dim2Pitch, int channelPitch, int linePitch,
    float epsilon
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numBatches * numDim1 * numDim2 * numChannels * height;
    if(idx >= total_elements) return;
    
    __shared__ float reduction_mean[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    __shared__ float reduction_variance[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    
    // Calculate indices for the current element
    int b = idx / (numDim1 * numDim2 * numChannels * height); // batch
    int d1 = (idx / (numDim2 * numChannels * height)) % numDim1; // dim1
    int d2 = (idx / (numChannels * height)) % numDim2; // dim2
    int c = (idx / height) % numChannels; // channel
    int h = idx % height; // height
    int w, stride;
    
    // Calculate mean
    float mean = 0.0f;
    for(w = ty; w < width; w += THREADS_PER_BLOCK_Y) {
        mean += input[b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w];
    }
    reduction_mean[tx][ty] = mean;
    
    // Reduce to compute sum for mean
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
        __syncthreads();
        if(ty < stride) {
            reduction_mean[tx][ty] += reduction_mean[tx][ty + stride];
        }
    }
    __syncthreads();
    
    // Compute final mean
    mean = reduction_mean[tx][0] / width;
    
    // Calculate variance
    float variance = 0.0f;
    for(w = ty; w < width; w += THREADS_PER_BLOCK_Y) {
        float diff = input[b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w] - mean;
        variance += diff * diff;
    }
    reduction_variance[tx][ty] = variance;
    
    // Reduce to compute sum for variance
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
        __syncthreads();
        if(ty < stride) {
            reduction_variance[tx][ty] += reduction_variance[tx][ty + stride];
        }
    }
    __syncthreads();
    
    // Compute final variance and normalization factor
    variance = reduction_variance[tx][0] / width;
    float denom = rsqrtf(variance + epsilon);
    
    // Apply normalization
    for(w = ty; w < width; w += THREADS_PER_BLOCK_Y) {
        int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
        output[offset] = (input[offset] - mean) * denom;
    }
}

__global__ void LayerNormBFloat16Kernel(
    const bfloat16_tidl* __restrict__ input,
    bfloat16_tidl* __restrict__ output,
    int numBatches, int numDim1, int numDim2, int numChannels, int height, int width,
    int batchPitch, int dim1Pitch, int dim2Pitch, int channelPitch, int linePitch,
    float epsilon
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numBatches * numDim1 * numDim2 * numChannels * height;
    if(idx >= total_elements) return;
    
    __shared__ float reduction_mean[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    __shared__ float reduction_variance[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    
    // Calculate indices for the current element
    int b = idx / (numDim1 * numDim2 * numChannels * height); // batch
    int d1 = (idx / (numDim2 * numChannels * height)) % numDim1; // dim1
    int d2 = (idx / (numChannels * height)) % numDim2; // dim2
    int c = (idx / height) % numChannels; // channel
    int h = idx % height; // height
    int w, stride;
    
    float acceX = 0, acceX2 = 0, xf = 0;
    bfloat16_tidl mean, variance;
    bfloat16_tidl denom, inp_sqrt, inp, ep;

    for(w = ty; w < width; w += THREADS_PER_BLOCK_Y)
    {
        int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
        xf = (float)input[offset];
        acceX  += (xf);
        acceX2 += (xf * xf);
    }
    reduction_mean[tx][ty] = acceX;
    reduction_variance[tx][ty] = acceX2;

    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
        __syncthreads();
        if(ty < stride) {
            reduction_mean[tx][ty] += reduction_mean[tx][ty + stride];
            reduction_variance[tx][ty] += reduction_variance[tx][ty + stride];
        }
    }
    __syncthreads();
    
    mean = (bfloat16_tidl)reduction_mean[tx][0];
    variance = (bfloat16_tidl)reduction_variance[tx][0];
    inp = ((bfloat16_tidl)width * variance) - (mean * mean);
    ep = (bfloat16_tidl)epsilon * (bfloat16_tidl)(width * width);
    /* Clamp inp to 0: residual bf16 cancellation error can make it slightly negative */
    if ((float)inp < 0.0f) inp = (bfloat16_tidl)0.0f;
    inp_sqrt = inp + ep;
    denom = (bfloat16_tidl)__recip_sqrt_cuda((float)inp_sqrt);

    for(w = ty; w < width; w += THREADS_PER_BLOCK_Y)
    {
        int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
        bfloat16_tidl in = (bfloat16_tidl)width * input[offset] - mean;
        in = in * denom;
        output[offset] = in;
    }
}

/**
 * CUDA kernel for InstanceNorm floating point implementation
 * This kernel normalizes along the channel dimension (axis = TIDL_DIM_NUMCH)
 */
__global__ void InstanceNormFloatKernel(
    const float* __restrict__ input,
    float* __restrict__ output,
    int numBatches, int numDim1, int numDim2, int numChannels, int height, int width,
    int batchPitch, int dim1Pitch, int dim2Pitch, int channelPitch, int linePitch,
    float epsilon
) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numBatches * numDim1 * numDim2 * numChannels;
    if(idx >= total_elements) return;
    
    __shared__ float reduction_mean[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    __shared__ float reduction_variance[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    
    // Calculate indices for the current element
    int b = idx / (numDim1 * numDim2 * numChannels); // batch
    int d1 = (idx / (numDim2 * numChannels)) % numDim1; // dim1
    int d2 = (idx / numChannels) % numDim2; // dim2
    int c = idx % numChannels; // channel
    int h, w, stride;
    
    // Calculate mean across height and width
    float mean = 0.0f;
    
    for(int i = ty; i < height * width; i += THREADS_PER_BLOCK_Y) {
        h = i / width;
        w = i % width;
        int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
        mean += input[offset];
    }
        
    reduction_mean[tx][ty] = mean;
    
    // Reduce to compute sum for mean
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
        __syncthreads();
        if(ty < stride) {
            reduction_mean[tx][ty] += reduction_mean[tx][ty + stride];
        }
    }
    __syncthreads();
    
    // Compute final mean
    mean = reduction_mean[tx][0] / (height * width);
    
    // Calculate variance
    float variance = 0.0f;
    for(int i = ty; i < height * width; i += THREADS_PER_BLOCK_Y) {
        h = i / width;
        w = i % width;
        int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
        float diff = input[offset] - mean;
        variance += diff * diff;
    }
    
    reduction_variance[tx][ty] = variance;
    
    // Reduce to compute sum for variance
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
        __syncthreads();
        if(ty < stride) {
            reduction_variance[tx][ty] += reduction_variance[tx][ty + stride];
        }
    }
    __syncthreads();
    
    // Compute final variance and normalization factor
    variance = reduction_variance[tx][0] / (height * width);
    float denom = __recip_sqrt_cuda(variance + epsilon);
    
    // Apply normalization
    for(int i = ty; i < height * width; i += THREADS_PER_BLOCK_Y) {
        h = i / width;
        w = i % width;
        int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
        output[offset] = (input[offset] - mean) * denom;
    }
}

__global__ void InstanceNormBFloat16Kernel(
    const bfloat16_tidl* __restrict__ input,
    bfloat16_tidl* __restrict__ output,
    int numBatches, int numDim1, int numDim2, int numChannels, int height, int width,
    int batchPitch, int dim1Pitch, int dim2Pitch, int channelPitch, int linePitch,
    float epsilon
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numBatches * numDim1 * numDim2 * numChannels;
    if(idx >= total_elements) return;
    
    __shared__ float reduction_mean[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    __shared__ float reduction_variance[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    
    // Calculate indices for the current element
    int b = idx / (numDim1 * numDim2 * numChannels); // batch
    int d1 = (idx / (numDim2 * numChannels)) % numDim1; // dim1
    int d2 = (idx / numChannels) % numDim2; // dim2
    int c = idx % numChannels; // channel
    int h, w, stride;

    /* Per-row accumulation matching the ref:
     * 1. Each row: parallel-reduce partial sums across ty threads → full fp32 row sum
     * 2. ty=0 bf16-casts the full row sum and accumulates into fp32 channel accum
     * 3. After all rows, broadcast channel sum to all threads via shared memory
     * Using the existing reduction arrays for the per-row reduction. */
    float acceXChannel = 0.0f, acceX2Channel = 0.0f;
    bfloat16_tidl mean, variance;
    bfloat16_tidl denom, inp_sqrt, inp, ep;

    for(h = 0; h < height; h++) {
        float row_acceX = 0.0f, row_acceX2 = 0.0f;
        for(w = ty; w < width; w += THREADS_PER_BLOCK_Y) {
            int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
            float xf = (float)input[offset];
            row_acceX  += xf;
            row_acceX2 += (xf * xf);
        }
        /* Row-level reduction: sum all ty threads' partial row sums → full fp32 row sum */
        reduction_mean[tx][ty]     = row_acceX;
        reduction_variance[tx][ty] = row_acceX2;
        for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
            __syncthreads();
            if(ty < stride) {
                reduction_mean[tx][ty]     += reduction_mean[tx][ty + stride];
                reduction_variance[tx][ty] += reduction_variance[tx][ty + stride];
            }
        }
        __syncthreads();
        /* bf16-cast the full row sum and accumulate into channel fp32 (matches ref) */
        if(ty == 0) {
            acceXChannel  += (float)(bfloat16_tidl)reduction_mean[tx][0];
            acceX2Channel += (float)(bfloat16_tidl)reduction_variance[tx][0];
        }
    }

    /* Broadcast channel sums from ty=0 to all threads */
    if(ty == 0) {
        reduction_mean[tx][0]     = acceXChannel;
        reduction_variance[tx][0] = acceX2Channel;
    }
    __syncthreads();

    /* bf16-cast final channel sums — matches ref's eX / eX2 */
    mean     = (bfloat16_tidl)reduction_mean[tx][0];
    variance = (bfloat16_tidl)reduction_variance[tx][0];
    bfloat16_tidl N = (bfloat16_tidl)(width * height);

    /* Denom calculation in bf16, no Newton-Raphson refinement (matches ref) */
    inp      = (N * variance) - (mean * mean);
    ep       = (bfloat16_tidl)epsilon * N * N;
    /* Clamp inp to 0: residual bf16 cancellation error can make it slightly negative */
    if ((float)inp < 0.0f) inp = (bfloat16_tidl)0.0f;
    inp_sqrt = inp + ep;
    denom    = (bfloat16_tidl)__recip_sqrt_cuda((float)inp_sqrt);

    for(int i = ty; i < height * width; i += THREADS_PER_BLOCK_Y)
    {
        h = i / width;
        w = i % width;
        int offset = b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w;
        bfloat16_tidl in = N * input[offset] - mean;
        in = in * denom;
        output[offset] = in;
    }

}


template<class Tin, class Tout>
int TIDL_cudaLayerNormFloat(
    Tin* input,
    Tout* output,
    int32_t isInstanceNorm,
    int32_t axis,
    int32_t numBatches,
    int32_t numDim1,
    int32_t numDim2,
    int32_t numChannels,
    int32_t height,
    int32_t width,
    int32_t batchPitch,
    int32_t dim1Pitch,
    int32_t dim2Pitch,
    int32_t channelPitch,
    int32_t linePitch,
    float epsilon
)
{
    int32_t status = IALG_EOK;
    size_t inputSize = numBatches * batchPitch;
    size_t outputSize = numBatches * batchPitch;

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    /* Setup device pointers with explicit allocation */
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, sizeof(Tin) * inputSize) != IALG_EOK)
    {
        /* If it's the first layer - it is possible that the input doesn't have a GPU mirror pointer, allocate a new buffer */
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, sizeof(Tin) * inputSize) != IALG_EOK)
        {
            /* Log Error */
            printf("LayerNormFloat: Unable to find input pointer\n");
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, sizeof(Tout) * outputSize) != IALG_EOK)
    {
        /* If it's the last layer - it is possible that the output doesn't have a GPU mirror pointer, allocate a new buffer */
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, sizeof(Tout) * outputSize) != IALG_EOK)
        {
            /* Log Error */
            printf("LayerNormFloat: Unable to find output pointer\n");
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Single H2D copy of entire input buffer
    checkCudaErr(cudaMemcpyAsync(d_input, input, sizeof(Tin) * inputSize,
                                  cudaMemcpyHostToDevice, stream));

    // Launch kernel with full tensor dimensions (ordered after H2D by stream)
    if(isInstanceNorm)
    {
        if(axis == TIDL_DIM_NUMCH)
        {
            // Launch instancenorm float kernel
            int total_elements = numBatches * numDim1 * numDim2 * numChannels;
            dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
            dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

            if(std::is_same<Tin, bfloat16_tidl>::value && std::is_same<Tout, bfloat16_tidl>::value)
            {
                InstanceNormBFloat16Kernel<<<gridDim, blockDim, 0, stream>>>(
                    reinterpret_cast<const bfloat16_tidl *>(d_input),
                    reinterpret_cast<bfloat16_tidl *>(d_output),
                    numBatches, numDim1, numDim2, numChannels, height, width,
                    batchPitch, dim1Pitch, dim2Pitch, channelPitch, linePitch,
                    epsilon
                );
            }
            else
            {
                InstanceNormFloatKernel<<<gridDim, blockDim, 0, stream>>>(
                    reinterpret_cast<const float *>(d_input),
                    reinterpret_cast<float *>(d_output),
                    numBatches, numDim1, numDim2, numChannels, height, width,
                    batchPitch, dim1Pitch, dim2Pitch, channelPitch, linePitch,
                    epsilon
                );
            }
        }
        else
        {
            status = TIDL_ERROR_LAYERNORM_UNSUPPORTED_AXIS;
        }
    }
    else
    {
        if(axis == TIDL_DIM_WIDTH)
        {
            // Launch layernorm float kernel
            int total_elements = numBatches * numDim1 * numDim2 * numChannels * height;
            dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
            dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

            if(std::is_same<Tin, bfloat16_tidl>::value)
            {
                LayerNormBFloat16Kernel<<<gridDim, blockDim, 0, stream>>>(
                    reinterpret_cast<const bfloat16_tidl *>(d_input),
                    reinterpret_cast<bfloat16_tidl *>(d_output),
                    numBatches, numDim1, numDim2, numChannels, height, width,
                    batchPitch, dim1Pitch, dim2Pitch, channelPitch, linePitch,
                    epsilon
                );
            }
            else
            {
                LayerNormFloatKernel<<<gridDim, blockDim, 0, stream>>>(
                    reinterpret_cast<const float *>(d_input),
                    reinterpret_cast<float *>(d_output),
                    numBatches, numDim1, numDim2, numChannels, height, width,
                    batchPitch, dim1Pitch, dim2Pitch, channelPitch, linePitch,
                    epsilon
                );
            }
        }
        else
        {
            status = TIDL_ERROR_LAYERNORM_UNSUPPORTED_AXIS;
        }
    }

    // Single D2H copy of entire output buffer (ordered after kernel by stream)
    checkCudaErr(cudaMemcpyAsync(output, d_output, sizeof(Tout) * outputSize,
                                  cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));


    return status;
}

template<class Tin, class Tout, class typeExEx2>
__global__ void InstanceNormKernel(
    const Tin* __restrict__ input,
    Tout* __restrict__ output,
    int numBatches, int numDim1, int numDim2, int numChannels, int height, int width,
    int inBatchPitch, int inDim1Pitch, int inDim2Pitch, int inChannelPitch, int inLinePitch,
    int outBatchPitch, int outDim1Pitch, int outDim2Pitch, int outChannelPitch, int outLinePitch,
    float epsilon, float inScale, float outScale, uint8_t scaleAvg, uint8_t shiftAvg, int minValueAcc, int maxValueAcc, int netVersionCheck, int deviceCheck
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numBatches * numDim1 * numDim2 * numChannels;
    if(idx >= total_elements) return;
    
    __shared__ int64_t reduction_mean[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    __shared__ int64_t reduction_variance[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    
    // Calculate indices for the current element
    int b = idx / (numDim1 * numDim2 * numChannels); // batch
    int d1 = (idx / (numDim2 * numChannels)) % numDim1; // dim1
    int d2 = (idx / numChannels) % numDim2; // dim2
    int c = idx % numChannels; // channel
    int h, w, stride;

    int64_t acceX = 0, acceX2 = 0, inp;
    typeExEx2 mean, variance;
    float denom, inp_sqrt, inp_float, ep;
    int32_t shiftDenom, scaleDenom, absShift, tempAccOut;

    for(int i = ty; i < height * width; i += THREADS_PER_BLOCK_Y) {
        h = i / width;
        w = i % width;
        int offset = b*inBatchPitch + d1*inDim1Pitch + d2*inDim2Pitch + c*inChannelPitch + h*inLinePitch + w;
        inp = input[offset];
        acceX += inp;
        acceX2 += (inp * inp);
    }
    
    reduction_mean[tx][ty] = acceX;
    reduction_variance[tx][ty] = acceX2;

    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
        __syncthreads();
        if(ty < stride) {
            reduction_mean[tx][ty] += reduction_mean[tx][ty + stride];
            reduction_variance[tx][ty] += reduction_variance[tx][ty + stride];
        }
    }
    __syncthreads();

    if(netVersionCheck)
    {
        mean = reduction_mean[tx][0];
        variance = reduction_variance[tx][0];
        inp = (width * height * variance) - (mean * mean);
        inp_float = (int64_t)inp;
        ep = epsilon * (inScale * inScale * (width * height) * (width * height));
        inp_sqrt = inp_float + ep;
        denom = __recip_sqrt_cuda(inp_sqrt);
        /*Only for 16-bit:*/
        if(sizeof(Tin) == sizeof(short))
        {
            denom = denom * (1.5f - 0.5f * inp_sqrt * denom * denom);
        }
        denom = denom * outScale;

        for(int i = ty; i < height * width; i += THREADS_PER_BLOCK_Y)
        {
            h = i / width;
            w = i % width;
            int inOffset = b*inBatchPitch + d1*inDim1Pitch + d2*inDim2Pitch + c*inChannelPitch + h*inLinePitch + w;
            int outOffset = b*outBatchPitch + d1*outDim1Pitch + d2*outDim2Pitch + c*outChannelPitch + h*outLinePitch + w;
            float in = height * width * (float)input[inOffset] - (float)mean;
            in = in * denom;
            output[outOffset] = cuda_sat<Tout>(in);
        }
    }
    else
    {
        if(deviceCheck)
        {
            mean = reduction_mean[tx][0] / width;
            variance = reduction_variance[tx][0] / width;
        }
        else
        {
            mean = cuda_roundSat(reduction_mean[tx][0] * (int64_t)scaleAvg, shiftAvg, minValueAcc, maxValueAcc);
            variance = cuda_roundSat(reduction_variance[tx][0] * (int64_t)scaleAvg, shiftAvg, minValueAcc, maxValueAcc);
        }
        inp = variance - (mean * mean);
        inp = inp <= 0 ? 0 : inp;
        denom = __recip_sqrt_cuda(inp + (epsilon * (inScale * inScale)));
        denom = denom * outScale;
        convertFloatToScaleAndShift(denom, &scaleDenom, &shiftDenom, SCALE_PRECISION_BITS);
        absShift = abs(shiftDenom);

        for(int i = ty; i < height * width; i += THREADS_PER_BLOCK_Y)
        {
            h = i / width;
            w = i % width;
            int inOffset = b*inBatchPitch + d1*inDim1Pitch + d2*inDim2Pitch + c*inChannelPitch + h*inLinePitch + w;
            int outOffset = b*outBatchPitch + d1*outDim1Pitch + d2*outDim2Pitch + c*outChannelPitch + h*outLinePitch + w;
            tempAccOut = (input[inOffset] - mean) * scaleDenom;
            tempAccOut = (shiftDenom >= 0) ? (tempAccOut << shiftDenom) : ((tempAccOut + (1 << (absShift - 1))) >> absShift);
            output[outOffset] = cuda_sat<Tout>(tempAccOut);
        }
    }
}

template<class Tin, class Tout, class typeExEx2>
__global__ void TIDL_cudaLayerNormKernel(
    const Tin* __restrict__ input,
    Tout* __restrict__ output,
    int numBatches, int numDim1, int numDim2, int numChannels, int height, int width,
    int inBatchPitch, int inDim1Pitch, int inDim2Pitch, int inChannelPitch, int inLinePitch,
    int outBatchPitch, int outDim1Pitch, int outDim2Pitch, int outChannelPitch, int outLinePitch,
    float epsilon, float inScale, float outScale, uint8_t scaleAvg, uint8_t shiftAvg, int minValueAcc, int maxValueAcc, int netVersionCheck, int deviceCheck
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = numBatches * numDim1 * numDim2 * numChannels * height;
    if(idx >= total_elements) return;
    
    __shared__ int64_t reduction_mean[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    __shared__ int64_t reduction_variance[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    
    // Calculate indices for the current element
    int b = idx / (numDim1 * numDim2 * numChannels * height); // batch
    int d1 = (idx / (numDim2 * numChannels * height)) % numDim1; // dim1
    int d2 = (idx / (numChannels * height)) % numDim2; // dim2
    int c = (idx / height) % numChannels; // channel
    int h = idx % height; // height
    int w, stride;
    
    int64_t acceX = 0, acceX2 = 0, inp;
    typeExEx2 mean, variance;
    float denom, inp_sqrt, inp_float, ep;
    int32_t shiftDenom, scaleDenom, absShift, tempAccOut;

    for(w = ty; w < width; w += THREADS_PER_BLOCK_Y)
    {
        int offset = b*inBatchPitch + d1*inDim1Pitch + d2*inDim2Pitch + c*inChannelPitch + h*inLinePitch + w;
        inp = input[offset];
        acceX  += (inp);
        acceX2 += (inp * inp);
    }
    reduction_mean[tx][ty] = acceX;
    reduction_variance[tx][ty] = acceX2;

    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2) {
        __syncthreads();
        if(ty < stride) {
            reduction_mean[tx][ty] += reduction_mean[tx][ty + stride];
            reduction_variance[tx][ty] += reduction_variance[tx][ty + stride];
        }
    }
    __syncthreads();
    
    if(netVersionCheck)
    {
        mean = reduction_mean[tx][0];
        variance = reduction_variance[tx][0];
        inp = (width * variance) - (mean * mean);
        inp_float = (int64_t)inp;
        ep = epsilon * (inScale * inScale * width * width);
        inp_sqrt = inp_float + ep;
        denom = __recip_sqrt_cuda(inp_sqrt);
        if(sizeof(Tin) == sizeof(short))
        {
            denom = denom * (1.5f - 0.5f * inp_sqrt * denom * denom);
        }
        denom = denom * outScale;

        for(w = ty; w < width; w += THREADS_PER_BLOCK_Y)
        {
            int inOffset = b*inBatchPitch + d1*inDim1Pitch + d2*inDim2Pitch + c*inChannelPitch + h*inLinePitch + w;
            int outOffset = b*outBatchPitch + d1*outDim1Pitch + d2*outDim2Pitch + c*outChannelPitch + h*outLinePitch + w;
            float in = width * (float)input[inOffset] - (float)mean;
            in = in * denom;
            in = (float) cuda_float_to_int (in);
            output[outOffset] = cuda_sat<Tout>(in);
        }
    }
    else
    {
        if(deviceCheck)
        {
            mean = reduction_mean[tx][0] / width;
            variance = reduction_variance[tx][0] / width;
        }
        else
        {
            mean = cuda_roundSat(reduction_mean[tx][0] * (int64_t)scaleAvg, shiftAvg, minValueAcc, maxValueAcc);
            variance = cuda_roundSat(reduction_variance[tx][0] * (int64_t)scaleAvg, shiftAvg, minValueAcc, maxValueAcc);
        }
        inp = variance - (mean * mean);
        inp = inp <= 0 ? 0 : inp;
        denom = __recip_sqrt_cuda(inp + (epsilon * (inScale * inScale)));
        denom = denom * outScale;
        convertFloatToScaleAndShift(denom, &scaleDenom, &shiftDenom, SCALE_PRECISION_BITS);
        absShift = abs(shiftDenom);

        for(w = ty; w < width; w += THREADS_PER_BLOCK_Y)
        {
            int inOffset = b*inBatchPitch + d1*inDim1Pitch + d2*inDim2Pitch + c*inChannelPitch + h*inLinePitch + w;
            int outOffset = b*outBatchPitch + d1*outDim1Pitch + d2*outDim2Pitch + c*outChannelPitch + h*outLinePitch + w;
            tempAccOut = (input[inOffset] - mean) * scaleDenom;
            tempAccOut = (shiftDenom >= 0) ? (tempAccOut << shiftDenom) : ((tempAccOut + (1 << (absShift - 1))) >> absShift);
            output[outOffset] = cuda_sat<Tout>(tempAccOut);
        }
    }
}

template<class Tin, class Tout, class typeExEx2>
int TIDL_cudaLayerNorm(
    Tin* input,
    Tout* output,
    int32_t isInstanceNorm,
    int32_t axis,
    int32_t numBatches,
    int32_t numDim1,
    int32_t numDim2,
    int32_t numChannels,
    int32_t height,
    int32_t width,
    int32_t inBatchPitch,
    int32_t inDim1Pitch,
    int32_t inDim2Pitch,
    int32_t inChannelPitch,
    int32_t inLinePitch,
    int32_t outBatchPitch,
    int32_t outDim1Pitch,
    int32_t outDim2Pitch,
    int32_t outChannelPitch,
    int32_t outLinePitch,
    float epsilon,
    float inScale,
    float outScale,
    uint8_t scaleAvg,
    uint8_t shiftAvg,
    int32_t minValueAcc,
    int32_t maxValueAcc,
    int32_t netVersionCheck,
    int32_t deviceCheck
)
{
    int32_t status = IALG_EOK;
    size_t inputSize = numBatches * inBatchPitch;
    size_t outputSize = numBatches * outBatchPitch;

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    /* Setup device pointers with explicit allocation */
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, sizeof(Tin) * inputSize) != IALG_EOK)
    {
        /* If it's the first layer - it is possible that the input doesn't have a GPU mirror pointer, allocate a new buffer */
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, sizeof(Tin) * inputSize) != IALG_EOK)
        {
            /* Log Error */
            printf("LayerNorm: Unable to find input pointer\n");
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, sizeof(Tout) * outputSize) != IALG_EOK)
    {
        /* If it's the last layer - it is possible that the output doesn't have a GPU mirror pointer, allocate a new buffer */
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, sizeof(Tout) * outputSize) != IALG_EOK)
        {
            /* Log Error */
            printf("LayerNorm: Unable to find output pointer\n");
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Single H2D copy of entire input buffer
    checkCudaErr(cudaMemcpyAsync(d_input, input, sizeof(Tin) * inputSize,
                                  cudaMemcpyHostToDevice, stream));

    // Launch kernel with full tensor dimensions (ordered after H2D by stream)
    if(isInstanceNorm)
    {
        if(axis == TIDL_DIM_NUMCH)
        {
            // Launch instancenorm kernel
            int total_elements = numBatches * numDim1 * numDim2 * numChannels;
            dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
            dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

            InstanceNormKernel<Tin, Tout, typeExEx2><<<gridDim, blockDim, 0, stream>>>(
                d_input, d_output,
                numBatches, numDim1, numDim2, numChannels, height, width,
                inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
                outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
                epsilon, inScale, outScale, scaleAvg, shiftAvg,
                minValueAcc, maxValueAcc, netVersionCheck, deviceCheck);
        }
        else
            status = TIDL_ERROR_LAYERNORM_UNSUPPORTED_AXIS;
    }
    else
    {
        if(axis == TIDL_DIM_WIDTH)
        {
            int total_elements = numBatches * numDim1 * numDim2 * numChannels * height;
            dim3 blockDim(THREADS_PER_BLOCK_X, THREADS_PER_BLOCK_Y);
            dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

            TIDL_cudaLayerNormKernel<Tin, Tout, typeExEx2><<<gridDim, blockDim, 0, stream>>>(
                d_input, d_output,
                numBatches, numDim1, numDim2, numChannels, height, width,
                inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
                outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
                epsilon, inScale, outScale, scaleAvg, shiftAvg,
                minValueAcc, maxValueAcc, netVersionCheck, deviceCheck);
        }
        else
            status = TIDL_ERROR_LAYERNORM_UNSUPPORTED_AXIS;
    }

    // Single D2H copy of entire output buffer (ordered after kernel by stream)
    checkCudaErr(cudaMemcpyAsync(output, d_output, sizeof(Tout) * outputSize,
                                  cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));


    return status;
}

template int TIDL_cudaLayerNorm<signed char, signed char, long>(signed char*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float, float, unsigned char, unsigned char, int, int, int, int);
template int TIDL_cudaLayerNorm<unsigned char, signed char, unsigned long>(unsigned char*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float, float, unsigned char, unsigned char, int, int, int, int);
template int TIDL_cudaLayerNorm<unsigned short, short, unsigned long>(unsigned short*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float, float, unsigned char, unsigned char, int, int, int, int);
template int TIDL_cudaLayerNorm<short, short, long>(short*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float, float, float, unsigned char, unsigned char, int, int, int, int);
template int TIDL_cudaLayerNormFloat<float, float>(float*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
template int TIDL_cudaLayerNormFloat<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl*, bfloat16_tidl*, int, int, int, int, int, int, int, int, int, int, int, int, int, float);
