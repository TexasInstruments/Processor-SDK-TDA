/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaSoftmax.cu
 @brief   This file contains CUDA implementation of softmax activation.
 @version 0.1 (Dec 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

using namespace floating_point::bf16_c7x;

__global__ void SoftmaxFloatKernel(
    const float* __restrict__ input,
    float* __restrict__ output,
    int inDimVal5, int inDimVal4, int inDimVal3, int inDimVal2, int inDimVal1, int inDimVal0,
    int inPitch5, int inPitch4, int inPitch3, int inPitch2, int inPitch1, int inPitch0,
    int outPitch5, int outPitch4, int outPitch3, int outPitch2, int outPitch1, int outPitch0
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = inDimVal5 * inDimVal4 * inDimVal3 * inDimVal2 * inDimVal1;
    if(idx >= total_elements) return;
    __shared__ float reduction[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    int i5 = idx / (inDimVal4 * inDimVal3 * inDimVal2 * inDimVal1);
    int i4 = (idx / (inDimVal3 * inDimVal2 * inDimVal1)) % inDimVal4;
    int i3 = (idx / (inDimVal2 * inDimVal1)) % inDimVal3;
    int i2 = (idx / inDimVal1) % inDimVal2;
    int i1 = idx % inDimVal1;

    float inDataVal, maxVal, denom = 0;
    int i0, stride;
    maxVal = -FLT_MAX;

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        inDataVal = input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0];
        maxVal = inDataVal > maxVal ? inDataVal : maxVal;
    }
    reduction[tx][ty] = maxVal;
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if(ty < stride)
        {
            reduction[tx][ty] = max(reduction[tx][ty], reduction[tx][ty + stride]);
        }
    }
    __syncthreads();
    maxVal = reduction[tx][0];

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        inDataVal = input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0];
        denom += exp(inDataVal - maxVal);
    }
    reduction[tx][ty] = denom;
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if(ty < stride)
        {
            reduction[tx][ty] += reduction[tx][ty + stride];
        }
    }
    __syncthreads();
    denom = reduction[tx][0];

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        inDataVal = input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0];
        output[i5*outPitch5 + i4*outPitch4 + i3*outPitch3 + i2*outPitch2 + i1*outPitch1 + i0*outPitch0] = exp(inDataVal - maxVal) / denom; 
    }
}

template <class Tin, class Tout>
__global__ void SoftmaxBFloat16Kernel(
    const Tin* __restrict__ input,
    Tout* __restrict__ output,
    int inDimVal5, int inDimVal4, int inDimVal3, int inDimVal2, int inDimVal1, int inDimVal0,
    int inPitch5, int inPitch4, int inPitch3, int inPitch2, int inPitch1, int inPitch0,
    int outPitch5, int outPitch4, int outPitch3, int outPitch2, int outPitch1, int outPitch0
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = inDimVal5 * inDimVal4 * inDimVal3 * inDimVal2 * inDimVal1;
    if(idx >= total_elements) return;
    __shared__ Tout reduction[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];

    int i5 = idx / (inDimVal4 * inDimVal3 * inDimVal2 * inDimVal1);
    int i4 = (idx / (inDimVal3 * inDimVal2 * inDimVal1)) % inDimVal4;
    int i3 = (idx / (inDimVal2 * inDimVal1)) % inDimVal3;
    int i2 = (idx / inDimVal1) % inDimVal2;
    int i1 = idx % inDimVal1;

    Tin inDataVal, maxVal;
    Tout denom = 0, numer = 0;
    int i0, stride;
    maxVal = -cuda::std::numeric_limits<Tin>::max();

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        inDataVal = input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0];
        maxVal = inDataVal > maxVal ? inDataVal : maxVal;
    }
    reduction[tx][ty] = maxVal;
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if(ty < stride)
        {
            reduction[tx][ty] = max(reduction[tx][ty], reduction[tx][ty + stride]);
        }
    }
    __syncthreads();
    maxVal = reduction[tx][0];

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        inDataVal = input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0];
        if(std::is_same<Tout, bfloat16_tidl>::value)
        {
            denom += cuda_exp_taylor_bf16(inDataVal - maxVal);
        }
        else 
        {
            denom += exp(inDataVal - maxVal);
        }
    }
    reduction[tx][ty] = denom;
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if(ty < stride)
        {
            reduction[tx][ty] += reduction[tx][ty + stride];
        }
    }
    __syncthreads();
    denom = reduction[tx][0];

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        inDataVal = input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0];
        
        if(std::is_same<Tout, bfloat16_tidl>::value)
        {
            numer = cuda_exp_taylor_bf16(inDataVal - maxVal);
        }
        else 
        {
            numer = exp(inDataVal - maxVal);
        }
        output[i5*outPitch5 + i4*outPitch4 + i3*outPitch3 + i2*outPitch2 + i1*outPitch1 + i0*outPitch0] =  (Tout)numer * (Tout)bf16_recip_bitmatch(denom); 
    }
}


template <class Tin, class Tout, class Tacc>
__global__ void SoftmaxFixedKernel(
    const Tin* __restrict__ input,
    Tout* __restrict__ output,
    int inDimVal5, int inDimVal4, int inDimVal3, int inDimVal2, int inDimVal1, int inDimVal0,
    int inPitch5, int inPitch4, int inPitch3, int inPitch2, int inPitch1, int inPitch0,
    int outPitch5, int outPitch4, int outPitch3, int outPitch2, int outPitch1, int outPitch0,
    float inputTensorScale,
    float quantScale,
    float outputTensorScale,
    Tin outputTensorZP,
    float lutTensorScale
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tx = threadIdx.x, ty = threadIdx.y;
    int total_elements = inDimVal5 * inDimVal4 * inDimVal3 * inDimVal2 * inDimVal1;
    if(idx >= total_elements) return;
    __shared__ Tacc reduce_denom[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];
    __shared__ Tin reduce_max[THREADS_PER_BLOCK_X][THREADS_PER_BLOCK_Y];


    int i5 = idx / (inDimVal4 * inDimVal3 * inDimVal2 * inDimVal1);
    int i4 = (idx / (inDimVal3 * inDimVal2 * inDimVal1)) % inDimVal4;
    int i3 = (idx / (inDimVal2 * inDimVal1)) % inDimVal3;
    int i2 = (idx / inDimVal1) % inDimVal2;
    int i1 = idx % inDimVal1;
    int i0, stride;

    Tin inDataVal, maxVal = cuda::std::numeric_limits<Tin>::lowest();
    Tacc denomSum = 0;
    Tout expOut;
    Tacc prod, shiftAcc;
    Tout softmaxOut;
    float inputTensorScaleInv = 1 / inputTensorScale;

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        inDataVal = input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0];
        maxVal = inDataVal > maxVal ? inDataVal : maxVal;
    }
    reduce_max[tx][ty] = maxVal;
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if(ty < stride)
        {
            reduce_max[tx][ty] = reduce_max[tx][ty] > reduce_max[tx][ty + stride] ? reduce_max[tx][ty] : reduce_max[tx][ty + stride];
        }
    }
    __syncthreads();
    maxVal = reduce_max[tx][0];

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        if(sizeof(Tin) == sizeof(int16_t))
        {
            int32_t inputLUT = (int32_t)(input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0] - maxVal);
            expOut = cuda_float_to_int(cuda_exp_taylor(inputLUT * inputTensorScaleInv) * quantScale);
        }
        else
#if defined(__C7504__) || defined(__C7524__)
        {
            int32_t inputLUT = (int32_t)(input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0] - maxVal);
            expOut = round(cuda_exp_taylor(inputLUT * inputTensorScaleInv) * quantScale);
        }
#else
        {
            uint8_t inputLUT = (uint8_t)(maxVal - input[i5*inPitch5 + i4*inPitch4 + i3*inPitch3 + i2*inPitch2 + i1*inPitch1 + i0*inPitch0]);
            expOut = round((exp(-1.0 * (float)inputLUT / inputTensorScale) * (1.0 / lutTensorScale)) + lutTensorZP);
        }
#endif
        denomSum += expOut;
        output[i5*outPitch5 + i4*outPitch4 + i3*outPitch3 + i2*outPitch2 + i1*outPitch1 + i0*outPitch0] = (Tout)expOut;
    }
    reduce_denom[tx][ty] = denomSum;
    for(stride = THREADS_PER_BLOCK_Y / 2; stride >= 1; stride /= 2)
    {
        __syncthreads();
        if(ty < stride)
        {
            reduce_denom[tx][ty] += reduce_denom[tx][ty + stride];
        }
    }
    __syncthreads();
    denomSum = reduce_denom[tx][0];


    float mulFactor = __recip_bitmatch((float)denomSum) * outputTensorScale;
    int32_t OutScale, OutShift;
    convertFloatToScaleAndShift(mulFactor, &OutScale, &OutShift, SCALE_PRECISION_BITS);
    uint8_t scaleFactor = (uint8_t)OutScale;
    uint32_t absShift = abs(OutShift);

    for(i0 = ty; i0 < inDimVal0; i0+=THREADS_PER_BLOCK_Y)
    {
        expOut = output[i5*outPitch5 + i4*outPitch4 + i3*outPitch3 + i2*outPitch2 + i1*outPitch1 + i0*outPitch0];
        prod = expOut * scaleFactor;
        shiftAcc = OutShift >= 0 ? prod << (uint64_t)OutShift : (prod + (1U << (absShift - 1))) >> absShift;
        shiftAcc = shiftAcc + outputTensorZP;
        softmaxOut = (shiftAcc > cuda::std::numeric_limits<Tout>::max()) ? cuda::std::numeric_limits<Tout>::max() : shiftAcc;
        output[i5*outPitch5 + i4*outPitch4 + i3*outPitch3 + i2*outPitch2 + i1*outPitch1 + i0*outPitch0] = (Tout)softmaxOut;
    }
}


template <class Tin, class Tout, class Tacc>
int TIDL_cudaSoftmaxFixed(
    Tin* input,
    Tout* output,
    int32_t* inDims,
    int32_t* inPitches,
    int32_t* outPitches,
    float inputTensorScale,
    float quantScale,
    float outputTensorScale,
    Tin outputTensorZP
)
{
    size_t input_size = sizeof(Tin)*(1+ (inDims[5]-1) * inPitches[5] + (inDims[4]-1) * inPitches[4] + (inDims[3]-1) * inPitches[3] + (inDims[2]-1) * inPitches[2] + (inDims[1]-1) * inPitches[1] + (inDims[0]-1) * inPitches[0]);
    size_t output_size = sizeof(Tout)*(1+ (inDims[5]-1) * outPitches[5] + (inDims[4]-1) * outPitches[4] + (inDims[3]-1) * outPitches[3] + (inDims[2]-1) * outPitches[2] + (inDims[1]-1) * outPitches[1] + (inDims[0]-1) * outPitches[0]);

    float lutTensorScale = 1.0 / 255.0;
    if(sizeof(Tin) == sizeof(int16_t)) lutTensorScale = 1.0 / 65535.0;

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Softmax: Unable to find input pointer\n");
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Softmax: Unable to find output pointer\n");
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    // Launch kernel
    int total_elements = inDims[5] * inDims[4] * inDims[3] * inDims[2] * inDims[1];
    dim3 blockDim(THREADS_PER_BLOCK_X,  THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    SoftmaxFixedKernel<Tin, Tout, Tacc><<<gridDim, blockDim, 0, stream>>>(
        d_input, d_output,
        inDims[5], inDims[4], inDims[3], inDims[2], inDims[1], inDims[0],
        inPitches[5], inPitches[4], inPitches[3], inPitches[2], inPitches[1], inPitches[0],
        outPitches[5], outPitches[4], outPitches[3], outPitches[2], outPitches[1], outPitches[0],
        inputTensorScale, quantScale, outputTensorScale, outputTensorZP, lutTensorScale
    );

    // Async D2H
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template <class Tin, class Tout>
int TIDL_cudaSoftmaxFloat(
    Tin* input,
    Tout* output,
    int32_t* inDims,
    int32_t* inPitches,
    int32_t* outPitches
)
{
    size_t input_size = sizeof(Tin)*(1+ (inDims[5]-1) * inPitches[5] + (inDims[4]-1) * inPitches[4] + (inDims[3]-1) * inPitches[3] + (inDims[2]-1) * inPitches[2] + (inDims[1]-1) * inPitches[1] + (inDims[0]-1) * inPitches[0]);
    size_t output_size = sizeof(Tout)*(1+ (inDims[5]-1) * outPitches[5] + (inDims[4]-1) * outPitches[4] + (inDims[3]-1) * outPitches[3] + (inDims[2]-1) * outPitches[2] + (inDims[1]-1) * outPitches[1] + (inDims[0]-1) * outPitches[0]);

    Tin* d_input = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Softmax: Unable to find input pointer\n");
        }
    }
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Softmax: Unable to find output pointer\n");
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(d_input, input, input_size, cudaMemcpyHostToDevice, stream));

    // Launch kernel
    int total_elements = inDims[5] * inDims[4] * inDims[3] * inDims[2] * inDims[1];
    dim3 blockDim(THREADS_PER_BLOCK_X,  THREADS_PER_BLOCK_Y);
    dim3 gridDim(GRID_SIZE(total_elements, THREADS_PER_BLOCK_X));

    if(std::is_same<Tin, bfloat16_tidl>::value)
    {
        SoftmaxBFloat16Kernel<Tin, Tout><<<gridDim, blockDim, 0, stream>>>(
            d_input, d_output,
            inDims[5], inDims[4], inDims[3], inDims[2], inDims[1], inDims[0],
            inPitches[5], inPitches[4], inPitches[3], inPitches[2], inPitches[1], inPitches[0],
            outPitches[5], outPitches[4], outPitches[3], outPitches[2], outPitches[1], outPitches[0]
        );
    }
    else
    {
        SoftmaxFloatKernel<<<gridDim, blockDim, 0, stream>>>(
            reinterpret_cast<const float *>(d_input),
            reinterpret_cast<float *>(d_output),
            inDims[5], inDims[4], inDims[3], inDims[2], inDims[1], inDims[0],
            inPitches[5], inPitches[4], inPitches[3], inPitches[2], inPitches[1], inPitches[0],
            outPitches[5], outPitches[4], outPitches[3], outPitches[2], outPitches[1], outPitches[0]
        );
    }

    // Async D2H
    checkCudaErr(cudaMemcpyAsync(output, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_cudaSoftmaxFixed<unsigned char, float, unsigned int>(unsigned char*, float*, int*, int*, int*, float, float, float, unsigned char);
template int TIDL_cudaSoftmaxFixed<signed char, signed char, unsigned int>(signed char*, signed char*, int*, int*, int*, float, float, float, signed char);
template int TIDL_cudaSoftmaxFixed<unsigned char, signed char, unsigned int>(unsigned char*, signed char*, int*, int*, int*, float, float, float, unsigned char);
template int TIDL_cudaSoftmaxFixed<unsigned short, float, unsigned int>(unsigned short*, float*, int*, int*, int*, float, float, float, unsigned short);
template int TIDL_cudaSoftmaxFixed<unsigned char, unsigned char, unsigned int>(unsigned char*, unsigned char*, int*, int*, int*, float, float, float, unsigned char);
template int TIDL_cudaSoftmaxFixed<signed char, unsigned char, unsigned int>(signed char*, unsigned char*, int*, int*, int*, float, float, float, signed char);
template int TIDL_cudaSoftmaxFixed<signed char, float, unsigned int>(signed char*, float*, int*, int*, int*, float, float, float, signed char);
template int TIDL_cudaSoftmaxFixed<short, unsigned short, unsigned int>(short*, unsigned short*, int*, int*, int*, float, float, float, short);
template int TIDL_cudaSoftmaxFixed<short, float, unsigned int>(short*, float*, int*, int*, int*, float, float, float, short);
template int TIDL_cudaSoftmaxFixed<unsigned short, unsigned short, unsigned int>(unsigned short*, unsigned short*, int*, int*, int*, float, float, float, unsigned short);
template int TIDL_cudaSoftmaxFloat<float, float>(float*, float*, int*, int*, int*);
template int TIDL_cudaSoftmaxFloat<bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl*, bfloat16_tidl*, int*, int*, int*);
template int TIDL_cudaSoftmaxFloat<bfloat16_tidl, float>(bfloat16_tidl*, float*, int*, int*, int*);