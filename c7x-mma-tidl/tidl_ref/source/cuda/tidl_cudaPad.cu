/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaPad.cu
 @brief   This file contains CUDA implementation of Pad layer.
 @version 0.1 (Nov 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/

#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

template <class Tin, class Tout>
__global__ void PadKernel(
  const Tin*    __restrict__ input,
  Tout*   __restrict__ output,
  const float*  __restrict__ padPerChannelPtr,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t inHeight, int32_t outWidth, int32_t outHeight, int32_t numChs, 
  int32_t outDataDim1, int32_t outDataDim2, int32_t numROIs,
  int32_t inLinePitch, int32_t outLinePitch, int32_t inChPitch, int32_t outChPitch,
  int32_t inDim1Pitch, int32_t outDim1Pitch, int32_t inDim2Pitch, int32_t outDim2Pitch,
  int32_t inROIPitch, int32_t outROIPitch,
  int32_t padT, int32_t padB, int32_t padL, int32_t padR,
  int32_t padValue, int32_t padType, float tensorScaleInput)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numROIs * outDataDim1 * outDataDim2 * numChs * outHeight * outWidth;
    
    if (idx >= total_elements) return;
    
    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, height, width)
    int b = idx / (outDataDim1 * outDataDim2 * numChs * outHeight * outWidth);
    int d1 = (idx / (outDataDim2 * numChs * outHeight * outWidth)) % outDataDim1;
    int d2 = (idx / (numChs * outHeight * outWidth)) % outDataDim2;
    int c = (idx / (outHeight * outWidth)) % numChs;
    int h = (idx / outWidth) % outHeight;
    int w = idx % outWidth;

    int32_t outOffset = outPtrOffset + (b*outROIPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + (c*outChPitch) + (h*outLinePitch) + w;
    
    Tout padValueFinal = (Tout)padValue;
    if(padType == (int32_t)TIDL_PadPerChannel)
    {
        padValueFinal = (Tout)(padPerChannelPtr[c] * tensorScaleInput);
    }

    if (padType == TIDL_PadWrapAround)
    {
        int32_t inOffset;
        if ((w < padL) || (w >= (outWidth - padR)))
        {
            output[outOffset] = padValueFinal;
        }
        else if (h < padT)
        {
            inOffset = inPtrOffset + (b*inROIPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + ((inHeight - padT + h) * inLinePitch) + (w - padL);
            output[outOffset] = input[inOffset];
        }
        else if (h < (outHeight - padB))
        {
            inOffset = inPtrOffset + (b*inROIPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + ((h - padT) * inLinePitch) + (w - padL);
            output[outOffset] = input[inOffset];
        }
        else 
        {
            inOffset = inPtrOffset + (b*inROIPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + ((h - (outHeight - padB)) * inLinePitch) + (w - padL);
            output[outOffset] = input[inOffset];
        }
        
    }
    else
    {
        if((w < padL) || (w >= (outWidth - padR)) || (h < padT) || (h >= (outHeight - padB)))
        {
            output[outOffset] = padValueFinal;
        }
        else
        {
            int32_t inOffset = inPtrOffset + (b*inROIPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*inChPitch) + ((h - padT) * inLinePitch) + (w - padL);
            output[outOffset] = input[inOffset];
        }
    }
}

template <class Tin, class Tout>
int TIDL_refCudaPad(
  const Tin* pIn,
  Tout*  pOut,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t inHeight, int32_t outWidth, int32_t outHeight, int32_t numChs,
  int32_t outDataDim1, int32_t outDataDim2, int32_t numROIs,
  int32_t inLinePitch, int32_t outLinePitch, int32_t inChPitch, int32_t outChPitch,
  int32_t inDim1Pitch, int32_t outDim1Pitch, int32_t inDim2Pitch, int32_t outDim2Pitch,
  int32_t inROIPitch, int32_t outROIPitch,
  int32_t padT, int32_t padB, int32_t padL, int32_t padR,
  int32_t padValue, int32_t padType, float* padPerChannelPtr, float tensorScaleInput)
{
    size_t input_size = numROIs * inROIPitch * sizeof(Tin);
    size_t output_size = numROIs * outROIPitch * sizeof(Tout);
    size_t channel_pad_size = numChs * sizeof(float);

    Tin* d_input = NULL;
    Tout* d_output = NULL;
    float* d_channel_pad = NULL;

    // Get GPU pointers (allocate if needed)
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pIn, (void**)&d_input, input_size) != IALG_EOK)
        {
            printf("Pad: Unable to find input pointer\n");
            return IALG_EFAIL;
        }
    }

    if(padType == TIDL_PadPerChannel)
    {
        if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), padPerChannelPtr, (void**)&d_channel_pad, channel_pad_size) != IALG_EOK)
        {
            if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), padPerChannelPtr, (void**)&d_channel_pad, channel_pad_size) != IALG_EOK)
            {
                printf("Pad: Unable to find Pad Per Channel pointer\n");
                return IALG_EFAIL;
            }
        }
    }

    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
    {
        if (TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), pOut, (void**)&d_output, output_size) != IALG_EOK)
        {
            printf("Pad: Unable to find output pointer\n");
            return IALG_EFAIL;
        }
    }

    // ---- CUDA Streams Optimization ----------------------------------------
    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    // Async H2D
    checkCudaErr(cudaMemcpyAsync(d_input, pIn, input_size, cudaMemcpyHostToDevice, stream));
    
    if(padType == TIDL_PadPerChannel)
        checkCudaErr(cudaMemcpyAsync(d_channel_pad, padPerChannelPtr, channel_pad_size, cudaMemcpyHostToDevice, stream));
        
    int total_elements = numROIs * outDataDim1 * outDataDim2 * numChs * outHeight * outWidth;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    PadKernel<Tin, Tout><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        d_input, d_output, d_channel_pad,
        inPtrOffset, outPtrOffset,
        inHeight, outWidth, outHeight, numChs,
        outDataDim1, outDataDim2, numROIs,
        inLinePitch, outLinePitch, inChPitch, outChPitch,
        inDim1Pitch, outDim1Pitch, inDim2Pitch, outDim2Pitch,
        inROIPitch, outROIPitch,
        padT, padB, padL, padR, padValue, padType, tensorScaleInput);

    checkCudaErr(cudaGetLastError());
    
    // Copy results back to CPU
    checkCudaErr(cudaMemcpyAsync(pOut, d_output, output_size, cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_refCudaPad<float, float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float*, float);
template int TIDL_refCudaPad<short, short>(short const*, short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float*, float);
template int TIDL_refCudaPad<unsigned char, unsigned char>(unsigned char const*, unsigned char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float*, float);
template int TIDL_refCudaPad<unsigned short, unsigned short>(unsigned short const*, unsigned short*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float*, float);
template int TIDL_refCudaPad<signed char, signed char>(signed char const*, signed char*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float*, float);
template int TIDL_refCudaPad<floating_point::bfloat16_t, floating_point::bfloat16_t>(floating_point::bfloat16_t const*, floating_point::bfloat16_t*, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, float*, float);
