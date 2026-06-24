/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaComputeStats.cu
 @brief   This file contains CUDA implementation of min/max, histograms, channel means computation.
 @version 0.1 (Dec 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

int TIDL_isLayerSupportedOnGPU()
{
    return (TIDL_cudaGetThreadManager() != NULL) && (TIDL_cudaGetThreadManager()->layerGpuSupport != NULL) && (TIDL_cudaGetThreadManager()->layerGpuSupport[TIDL_cudaGetThreadLayerIdx()]);
}

template<class Tsrc, class TminMax>
void TIDL_cudaMinMax(
    const Tsrc* ptr,
    TminMax* min,
    TminMax* max,
    int32_t dataSize
)
{
    Tsrc* d_ptr = NULL;
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), ptr, (void**)&d_ptr, NULL);
    thrust::device_ptr<const Tsrc>pWrapper(d_ptr);
    thrust::pair<const thrust::device_ptr<const Tsrc>, const thrust::device_ptr<const Tsrc> > result = thrust::minmax_element(pWrapper, pWrapper + dataSize);
    *min=(TminMax)(*result.first);
    *max=(TminMax)(*result.second);
}

template<class Tsrc, class TminMax>
__global__ void HistogramKernel(
    const Tsrc* __restrict__ ptr,
    int32_t* __restrict__ histogramPtr,
    int32_t padOffset,
    float minValue,
    float maxValue,
    int32_t numBins,
    int32_t tensorZeroPoint,
    float tensorScale,
    int32_t numBatches,
    int32_t numChannels,
    int32_t height,
    int32_t width,
    int32_t batchPitch,
    int32_t channelPitch,
    int32_t linePitch
)
{
    // Calculate global thread index
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    // Total number of output elements to compute
    int total_elements = numBatches * numChannels * height * width;
    
    if (idx >= total_elements) return;

    extern __shared__ int32_t shared_histogram[];

    for(int i = threadIdx.x; i < numBins; i+=blockDim.x)
    {
        shared_histogram[i] = 0;
    }
    __syncthreads();

    // Convert linear index to 4D coordinates (batch, channel, height, width)
    int b = idx / (numChannels * height * width);
    int c = (idx / (height * width)) % numChannels;
    int h = (idx / width) % height;
    int w = idx % width;

    TminMax val;
    float valFloat, valNorm;
    int32_t binIdx;

    val = (TminMax)ptr[padOffset + b*batchPitch + c*channelPitch + h*linePitch + w];
    valFloat = (float)(val - tensorZeroPoint) / tensorScale;
    valNorm = (numBins - 1) * (valFloat - minValue)/(maxValue - minValue);
    binIdx = valNorm + 0.5;
    binIdx = binIdx > (numBins-1) ? numBins - 1 : binIdx;

    atomicAdd(&shared_histogram[binIdx], 1);
    __syncthreads();

    for(int i = threadIdx.x; i < numBins; i += blockDim.x)
    {
        atomicAdd(&histogramPtr[i], shared_histogram[i]);
    }
}

template<class Tsrc, class TminMax>
int TIDL_cudaGetHistogram(
    const Tsrc* ptr,
    int32_t* histogramPtr,
    int32_t padOffset,
    float minValue,
    float maxValue,
    int32_t numBins,
    int32_t tensorZeroPoint,
    float tensorScale,
    int32_t numBatches,
    int32_t numChannels,
    int32_t height,
    int32_t width,
    int32_t batchPitch,
    int32_t channelPitch,
    int32_t linePitch
)
{
    Tsrc* d_ptr = NULL;
    int32_t* d_histPtr = NULL;
    size_t histogram_size = numBins * sizeof(int32_t);
    checkCudaErr(cudaMalloc((void**)&d_histPtr, histogram_size));
    checkCudaErr(cudaMemset(d_histPtr, 0, histogram_size));
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), ptr, (void**)&d_ptr, NULL);

    int total_elements = numBatches * numChannels * height * width;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    //launch histogram kernel
    HistogramKernel<Tsrc, TminMax><<<grid_size, THREADS_PER_BLOCK, histogram_size>>>(
        d_ptr, d_histPtr, padOffset, minValue, maxValue, numBins, tensorZeroPoint, tensorScale, 
        numBatches, numChannels, height, width,
        batchPitch, channelPitch, linePitch
    );

    checkCudaErr(cudaMemcpy(histogramPtr, d_histPtr, histogram_size, cudaMemcpyDeviceToHost));
    cudaFree(d_histPtr);

    return IALG_EOK;
}

// Stage 1: Parallel reduction kernels
template<class Tsrc>
__global__ void PerChannelReductionKernel(
    const Tsrc* __restrict__ ptr,
    float* __restrict__ reducedPtr,
    int32_t numBatches,
    int32_t numChannels,
    int32_t height,
    int32_t width,
    int32_t batchPitch,
    int32_t channelPitch,
    int32_t linePitch,
    int32_t tensorZeroPoint,
    float tensorScale
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numChannels;
    
    if (idx >= total_elements) return;

    // Convert linear index to (batch, channel) coordinates
    int b = idx / numChannels;
    int c = idx % numChannels;

    float sum = 0.0f;
    // reduction over height and width
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            Tsrc val = ptr[b*batchPitch + c*channelPitch + h*linePitch + w];
            sum += (float)val;
        }
    }
    
    float mean = sum / (float)(height * width);
    mean = (mean - (float)tensorZeroPoint) / tensorScale;
    reducedPtr[b * numChannels + c] = mean;
}

template<class Tsrc>
__global__ void InnerProductReductionKernel(
    const Tsrc* __restrict__ ptr,
    float* __restrict__ reducedPtr,
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
    int32_t tensorZeroPoint,
    float tensorScale
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numDim1 * numDim2 * numChannels * width;
    
    if (idx >= total_elements) return;

    // Convert linear index to (batch, dim1, dim2, channel, width) coordinates
    int b = idx / (numDim1 * numDim2 * numChannels * width);
    int d1 = (idx / (numDim2 * numChannels * width)) % numDim1;
    int d2 = (idx / (numChannels * width)) % numDim2;
    int c = (idx / width) % numChannels;
    int w = idx % width;

    float sum = 0.0f;
    // reduction over height dimension
    for (int h = 0; h < height; h++) {
        Tsrc val = ptr[b*batchPitch + d1*dim1Pitch + d2*dim2Pitch + c*channelPitch + h*linePitch + w];
        sum += (float)val;
    }
    
    float mean = sum / (float)height;
    mean = (mean - (float)tensorZeroPoint) / tensorScale;
    reducedPtr[b * (numDim1 * numDim2 * numChannels * width) + d1 * (numDim2 * numChannels * width) + d2 * (numChannels * width) + c * width + w] = mean;
}

// Stage 2: Running mean update kernels
__global__ void PerChannelMeansUpdateKernel(
    const float* __restrict__ reducedPtr,
    float* __restrict__ meanPtr,
    int32_t numBatches,
    int32_t numChannels,
    float updateFactor
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= numChannels) return;
    int c = idx;
    float currChannelMean = 0, runningChannelMean = 0;
    
    // Sequential update of running mean across all batches for this channel
    for (int b = 0; b < numBatches; b++) {
        currChannelMean = reducedPtr[b * numChannels + c];
        runningChannelMean = meanPtr[c];
        meanPtr[c] = (runningChannelMean * (1.0f - updateFactor)) + (currChannelMean * updateFactor);
    }
}

__global__ void InnerProductMeansUpdateKernel(
    const float* __restrict__ reducedPtr,
    float* __restrict__ meanPtr,
    int32_t numBatches,
    int32_t numDim1,
    int32_t numDim2,
    int32_t numChannels,
    int32_t width,
    float updateFactor
)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numChannels * width;
    
    if (idx >= total_elements) return;
    
    // Convert linear index to (channel, width) coordinates
    int c = idx / width;
    int w = idx % width;

    float currChannelMean = 0, runningChannelMean = 0;
    
    // Sequential update of running mean across all batches, dim1, dim2 for this (channel, width)
    for (int b = 0; b < numBatches; b++) {
        for (int d1 = 0; d1 < numDim1; d1++) {
            for (int d2 = 0; d2 < numDim2; d2++) {
                currChannelMean = reducedPtr[b * (numDim1 * numDim2 * numChannels * width) + d1 * (numDim2 * numChannels * width) + d2 * (numChannels * width) + c * width + w];
                runningChannelMean = meanPtr[c * width + w];
                meanPtr[c * width + w] = (runningChannelMean * (1.0f - updateFactor)) + (currChannelMean * updateFactor);
            }
        }
    }
}

template<class Tsrc>
int TIDL_cudaPerChannelMeans(
    const Tsrc* ptr,
    float* meanPtr,
    int32_t layerType,
    int32_t constIdx,
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
    float updateFactor,
    int32_t tensorZeroPoint,
    float tensorScale
)
{
    Tsrc* d_ptr = NULL;
    float* d_meanPtr = NULL;
    float* d_reducedPtr = NULL;
    int meanSizeInBytes = 0;
    int reducedSizeInBytes = 0;
    
    if(layerType == TIDL_InnerProductLayer && constIdx == 1)
    {
        meanSizeInBytes = numChannels * width * sizeof(float);
        reducedSizeInBytes = numBatches * numDim1 * numDim2 * numChannels * width * sizeof(float);
    }
    else
    {
        meanSizeInBytes = numChannels * sizeof(float);
        reducedSizeInBytes = numBatches * numChannels * sizeof(float);
    }

    checkCudaErr(cudaMalloc((void**)&d_meanPtr, meanSizeInBytes));
    checkCudaErr(cudaMalloc((void**)&d_reducedPtr, reducedSizeInBytes));
    checkCudaErr(cudaMemcpy(d_meanPtr, meanPtr, meanSizeInBytes, cudaMemcpyHostToDevice));
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), ptr, (void**)&d_ptr, NULL);

    if(layerType == TIDL_InnerProductLayer && constIdx == 1)
    {
        // Stage 1: reduction over height dimension
        int total_elements = numBatches * numDim1 * numDim2 * numChannels * width;
        int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        
        InnerProductReductionKernel<Tsrc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_ptr, d_reducedPtr, numBatches, numDim1, numDim2, numChannels, height, width,
            batchPitch, dim1Pitch, dim2Pitch, channelPitch, linePitch,
            tensorZeroPoint, tensorScale
        );
        
        // Stage 2: Sequential running mean update across c and w
        int update_elements = numChannels * width;
        int update_grid_size = (update_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        
        InnerProductMeansUpdateKernel<<<update_grid_size, THREADS_PER_BLOCK>>>(
            d_reducedPtr, d_meanPtr, numBatches, numDim1, numDim2, numChannels, width,
            updateFactor
        );
    }
    else
    {
        // Stage 1: reduction over height and width dimensions
        int total_elements = numBatches * numChannels;
        int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        
        PerChannelReductionKernel<Tsrc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_ptr, d_reducedPtr, numBatches, numChannels, height, width,
            batchPitch, channelPitch, linePitch,
            tensorZeroPoint, tensorScale
        );
        
        // Stage 2: Sequential running mean update across channels
        int update_grid_size = (numChannels + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
        
        PerChannelMeansUpdateKernel<<<update_grid_size, THREADS_PER_BLOCK>>>(
            d_reducedPtr, d_meanPtr, numBatches, numChannels,
            updateFactor
        );
    }

    checkCudaErr(cudaMemcpy(meanPtr, d_meanPtr, meanSizeInBytes, cudaMemcpyDeviceToHost));
    cudaFree(d_meanPtr);
    cudaFree(d_reducedPtr);

    return IALG_EOK;
}

template void TIDL_cudaMinMax<unsigned short, int>(unsigned short const*, int*, int*, int);
template void TIDL_cudaMinMax<float, float>(float const*, float*, float*, int);
template void TIDL_cudaMinMax<short, int>(short const*, int*, int*, int);
template void TIDL_cudaMinMax<unsigned char, int>(unsigned char const*, int*, int*, int);
template void TIDL_cudaMinMax<signed char, int>(signed char const*, int*, int*, int);

template int TIDL_cudaGetHistogram<signed char, int>(signed char const*, int*, int, float, float, int, int, float, int, int, int, int, int, int, int);
template int TIDL_cudaGetHistogram<unsigned char, int>(unsigned char const*, int*, int, float, float, int, int, float, int, int, int, int, int, int, int);
template int TIDL_cudaGetHistogram<unsigned short, int>(unsigned short const*, int*, int, float, float, int, int, float, int, int, int, int, int, int, int);
template int TIDL_cudaGetHistogram<short, int>(short const*, int*, int, float, float, int, int, float, int, int, int, int, int, int, int);

template int TIDL_cudaPerChannelMeans<short>(short const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, float, int, float);
template int TIDL_cudaPerChannelMeans<signed char>(signed char const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, float, int, float);
template int TIDL_cudaPerChannelMeans<float>(float const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, float, int, float);
template int TIDL_cudaPerChannelMeans<unsigned char>(unsigned char const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, float, int, float);
template int TIDL_cudaPerChannelMeans<unsigned short>(unsigned short const*, float*, int, int, int, int, int, int, int, int, int, int, int, int, int, float, int, float);