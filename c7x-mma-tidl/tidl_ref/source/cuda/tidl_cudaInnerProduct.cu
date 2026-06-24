/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaInnerProduct.cu
 @brief   This file contains CUDA implementation of Inner Product layer.
 @version 0.1 (Sep 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

template <class Tin, class TBin, class Tw, class Tb, class Tout, class Tacc>
__global__ void TIDL_cudaInnerProductKernel(
    // Validated pointers
    const Tin* __restrict__ inPtr,
    const TBin* __restrict__ inBPtr,
    Tout* __restrict__ outPtr,
    const Tb* biasPtr,
    const Tw* __restrict__ weightsPtr,
    
    // Validated dimensions
    uint32_t numInCols,
    uint32_t numOutCols,
    uint32_t numInRows,
    uint32_t numOutRows,
    
    // Validated tensor dimensions
    int32_t batches,
    int32_t dim1,
    int32_t dim2,
    int32_t channels,

    int32_t numBChannels,
    
    // Validated pitch parameters
    uint32_t inAChPitch,
    uint32_t inALinePitch,
    uint32_t inADIM1Pitch,
    uint32_t inADIM2Pitch,
    uint32_t inABatchPitch,
    
    uint32_t inBChPitch,
    uint32_t inBLinePitch,
    uint32_t inBDIM1Pitch,
    uint32_t inBDIM2Pitch,
    uint32_t inBBatchPitch,
    
    uint32_t outChPitch,
    uint32_t outLinePitch,
    uint32_t outDIM1Pitch,
    uint32_t outDIM2Pitch,
    uint32_t outBatchPitch,
    
    // Validated flags
    int32_t inputBTranspose,
    int32_t isBias,
    int32_t constIdx,
    int32_t isFloat,
    int32_t isHighPrecision,
    
    //float saturation limits
    float floatSatLow,
    float floatSatHigh,

    // fixed saturation limits
    int32_t satLow,
    int32_t satHigh,

    // fixed saturation params
    int32_t roundBits,
    int32_t mmaScale,
    const uint8_t* __restrict__ mmav2_Scales,  
    const uint8_t* __restrict__ mmav2_Shifts
)
{
    // Calculate global thread index
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Total number of output elements to compute
    int total_elements = batches * dim1 * dim2 * channels * numOutRows * numOutCols;
    
    if (idx >= total_elements) return;
    
    // Convert linear index to 6D coordinates (batch, dim1, dim2, channel, outRow, outCol)
    int batchIdx = idx / (dim1 * dim2 * channels * numOutRows * numOutCols);
    int dim1Idx = (idx / (dim2 * channels * numOutRows * numOutCols)) % dim1;
    int dim2Idx = (idx / (channels * numOutRows * numOutCols)) % dim2;
    int chIdx = (idx / (numOutRows * numOutCols)) % channels;
    int outRwIdx = (idx / numOutCols) % numOutRows;
    int outColIdx = idx % numOutCols;
    
    // Calculate input and bias offsets
    uint32_t inOffset = (inABatchPitch * batchIdx) + (inADIM1Pitch * dim1Idx) + (inADIM2Pitch * dim2Idx) + (chIdx * inAChPitch);
    uint32_t biasOffset = (batchIdx * inBBatchPitch) + (dim1Idx * inBDIM1Pitch) + (dim2Idx * inBDIM2Pitch) + (chIdx * inBChPitch);
    uint32_t outOffset = (batchIdx * outBatchPitch) + (dim1Idx * outDIM1Pitch) + (dim2Idx * outDIM2Pitch) + (chIdx * outChPitch) + (outRwIdx * outLinePitch) + outColIdx;
    
    // Initialize accumulator with bias if present
    Tacc sum = 0;
    if (biasPtr != NULL && isBias != 0) {
        if (constIdx == -1) {
            sum = biasPtr[(outRwIdx * numOutCols) + outColIdx];
        } else if (constIdx == 0) {
            sum = biasPtr[outRwIdx];
        } else if (constIdx == 1) {
            sum = (numBChannels > 1) ? biasPtr[(chIdx * numOutCols) + outColIdx] : biasPtr[outColIdx];
        }
    }
    
    // Perform dot product computation
    if (inBPtr != NULL) {
        // Use dynamic input B
        if (inputBTranspose != 0) {
            // Transpose needed for inputB
            inOffset = inOffset + (inALinePitch * outRwIdx);
            biasOffset = biasOffset + (outColIdx * inBLinePitch);
            for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++) {
                sum += inPtr[inOffset + inColIdx] * inBPtr[biasOffset + inColIdx];
            }
        } else {
            // No transpose needed
            inOffset = inOffset + (inALinePitch * outRwIdx);
            biasOffset = biasOffset + outColIdx;
            for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++) {
                sum += inPtr[inOffset + inColIdx] * inBPtr[biasOffset + (inColIdx * inBLinePitch)];
            }
        }
    } else {
        // Use static weights
        if (inputBTranspose != 0) {
            // Transpose needed for inputB
            inOffset = inOffset + (inALinePitch * outRwIdx);
            biasOffset = biasOffset + (outColIdx * inBLinePitch);
            for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++) {
                sum += inPtr[inOffset + inColIdx] * weightsPtr[biasOffset + inColIdx];
            }
        } else {
            // No transpose needed
            inOffset = inOffset + (inALinePitch * outRwIdx);
            biasOffset = biasOffset + outColIdx;
            for (uint32_t inColIdx = 0; inColIdx < numInCols; inColIdx++) {
                sum += inPtr[inOffset + inColIdx] * weightsPtr[biasOffset + (inColIdx * inBLinePitch)];
            }
        }
    }

    if(isFloat) 
    {
        Tout result = cuda_floatSat(sum, floatSatHigh, floatSatLow);
        outPtr[outOffset] = result;
    }
    else if(isHighPrecision) 
    {
        int32_t outIdx = (outRwIdx * outLinePitch) + outColIdx;
        int32_t satIdx = (numBChannels == 1) ? (outIdx % numOutCols) : (outIdx % numOutCols + chIdx * numOutCols); 
        
        // Apply per-column scaling and MMA rounding/saturation
        int64_t scaled = (int64_t)sum * (int64_t)mmav2_Scales[satIdx];
        Tout result = (Tout)cuda_roundSatMMA(scaled, (int32_t)mmav2_Shifts[satIdx], satLow, satHigh);
        outPtr[outOffset] = result;
    }
    else 
    {
        int64_t scaled = (int64_t)sum * (int64_t)mmaScale;
        Tout result = (Tout)cuda_roundSat(scaled, roundBits, satLow, satHigh);
        outPtr[outOffset] = result;
    }
}

// TIDL CUDA wrapper for Inner Product
template <class Tin, class TBin, class Tw, class Tb, class Tout, class Tacc>
int TIDL_cudaInnerProductFused(
    const Tin* inPtr,
    const TBin* inBPtr,
    Tout* outPtr,
    const Tb* biasPtr,
    const Tw* weightsPtr,
    
    // Tensor dimensions
    int32_t batches,
    int32_t dim1,
    int32_t dim2,
    int32_t channels,
    int32_t numBChannels,
    uint32_t numInCols,
    uint32_t numOutCols,
    uint32_t numInRows,
    uint32_t numOutRows,
    
    // Pitch parameters
    uint32_t inAChPitch,
    uint32_t inALinePitch,
    uint32_t inADIM1Pitch,
    uint32_t inADIM2Pitch,
    uint32_t inABatchPitch,
    
    uint32_t inBChPitch,
    uint32_t inBLinePitch,
    uint32_t inBDIM1Pitch,
    uint32_t inBDIM2Pitch,
    uint32_t inBBatchPitch,
    
    uint32_t outChPitch,
    uint32_t outLinePitch,
    uint32_t outDIM1Pitch,
    uint32_t outDIM2Pitch,
    uint32_t outBatchPitch,

    // Flags
    int32_t inputBTranspose,
    int32_t isBias,
    int32_t constIdx,
    int32_t numInBufs,
    int32_t isFloat,
    int32_t isHighPrecision,

    //float saturation limits
    float floatSatLow,
    float floatSatHigh,

    // fixed saturation limits
    int32_t satLow,
    int32_t satHigh,

    // fixed saturation params
    int32_t roundBits,
    int32_t mmaScale,
    const uint8_t* __restrict__ mmav2_Scales,  
    const uint8_t* __restrict__ mmav2_Shifts
)
{
    // Calculate memory sizes in elements:
    size_t inputSize = 1 + (batches-1)*inABatchPitch + (dim1-1)*inADIM1Pitch + (dim2-1)*inADIM2Pitch + (channels-1)*inAChPitch + (numOutRows-1)*inALinePitch + (numInCols-1);
    size_t inputBSize = 1 + (batches-1)*inBBatchPitch + (dim1-1)*inBDIM1Pitch + (dim2-1)*inBDIM2Pitch + (channels-1)*inBChPitch + (numInCols-1)*inBLinePitch + (numOutCols-1);
    if(inputBTranspose)
    {
        inputBSize = 1 + (batches-1)*inBBatchPitch + (dim1-1)*inBDIM1Pitch + (dim2-1)*inBDIM2Pitch + (channels-1)*inBChPitch + (numOutCols-1)*inBLinePitch + (numInCols-1);
    }
    size_t outputSize = batches * outBatchPitch;

    size_t mmav2_scales_size = sizeof(uint8_t)* numBChannels * numOutCols;
    size_t mmav2_shifts_size = sizeof(uint8_t)* numBChannels * numOutCols;

    Tin* dpIn = NULL;
    TBin* dpInB = NULL;
    Tw* dpWeights = NULL;
    Tb* dpBias = NULL;
    Tout* dpOut = NULL;
    uint8_t* dpScales = NULL;
    uint8_t* dpShifts = NULL;

    #if 0
    void* inPtrs[numInBufs] = {NULL, NULL};
    uint32_t inDataSizes[numInBufs] = {0, 0};

    if(numInBufs == 2)
    {
        // sync A and B
        inPtrs[0] = (void*)inPtr;
        inDataSizes[0] = (uint32_t)input_size;
        inPtrs[1] = (void*)inBPtr;
        inDataSizes[1] = (uint32_t)inputB_size;
    }
    else
    {
        if(constIdx == 0)
        {
            // sync B
            inPtrs[0] = (void*)inBPtr;
            inDataSizes[0] = (uint32_t)inputB_size;
        }
        else
        {
            // sync A
            inPtrs[0] = (void*)inPtr;
            inDataSizes[0] = (uint32_t)input_size;
        }
    }

    TIDL_cudaMemManagerPreLayerSync(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx(), inPtrs, NULL, numInBufs, inDataSizes);
    #endif

    /*Setup device pointers:*/
    /*inPtr : Always available*/
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inPtr, (void**)&dpIn, sizeof(Tin) * inputSize) != IALG_EOK)
    {
        /*If it's the first layer - it is possible that the input doesn't have a GPU mirror pointer, allocate a new buffer*/
        if( TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inPtr, (void**)&dpIn, sizeof(Tin) * inputSize) != IALG_EOK)
        {
            /*Log Error*/
            printf("InnerProduct: Unable to find input pointer\n");
        }
    }

    cudaStream_t stream = (cudaStream_t)TIDL_cudaGetLayerStream(TIDL_cudaGetThreadManager(), TIDL_cudaGetThreadLayerIdx());

    checkCudaErr(cudaMemcpyAsync(dpIn, inPtr, sizeof(Tin) * inputSize,
                                  cudaMemcpyHostToDevice, stream));

    if(inBPtr != NULL)
    {
        if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), inBPtr, (void**)&dpInB, sizeof(TBin) *inputBSize) != IALG_EOK)
        {
            if(TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), inBPtr, (void**)&dpInB, sizeof(TBin) * inputBSize) != IALG_EOK)
            {
                /*Log Error*/
                printf("InnerProduct: Unable to find inputB pointer\n");
            }
        }
        checkCudaErr(cudaMemcpyAsync(dpInB, inBPtr, sizeof(TBin) * inputBSize,
                                      cudaMemcpyHostToDevice, stream));
    }

    if(weightsPtr != NULL && inBPtr == NULL)
    {
      /*Translate & weights should be present (already resident – no copy needed):*/
      TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), weightsPtr, (void**)&dpWeights, sizeof(TBin) * inputBSize);
    }

    if (isBias != 0) 
    {
        int32_t bias_size;
        if (constIdx == -1) {
            bias_size = numOutCols * numOutRows;
        } else if (constIdx == 0) {
            bias_size = numOutRows;
        } else if (constIdx == 1) {
            bias_size = (numBChannels > 1) ? (channels * numOutCols) : (numOutCols);
        }
        /*Transfer Bias explicitly:*/
        TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), biasPtr, (void**)&dpBias, bias_size*sizeof(Tb));
        if(biasPtr != NULL)
        {
            checkCudaErr(cudaMemcpyAsync(dpBias, biasPtr, bias_size*sizeof(Tb),
                                          cudaMemcpyHostToDevice, stream));
        }
    }

    /*outPtr : Always available*/
    if (TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * outputSize) != IALG_EOK)
    {
        /*If it's the last layer - it is possible that the output doesn't have a GPU mirror pointer, allocate a new buffer*/
        if( TIDL_cudaAllocBuffer(TIDL_cudaGetThreadManager(), outPtr, (void**)&dpOut, sizeof(Tout) * outputSize) != IALG_EOK)
        {
            /*Log Error*/
            printf("InnerProduct: Unable to find output pointer\n");
        }
    }

    if(mmav2_Scales != NULL && mmav2_Shifts != NULL)
    {
        if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), mmav2_Scales, (void**)&dpScales, mmav2_scales_size) != IALG_EOK)
        {
            printf("InnerProduct: Unable to find scales pointer\n");
        }

        if(TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), mmav2_Shifts, (void**)&dpShifts, mmav2_shifts_size) != IALG_EOK)
        {
            printf("InnerProduct: Unable to find shifts pointer\n");
        }
    }

    // Launch kernel parameters
    int total_elements = batches * dim1 * dim2 * channels * numOutRows * numOutCols;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;


    // Launch kernel on stream (all inputs guaranteed resident)
    TIDL_cudaInnerProductKernel<Tin, TBin, Tw, Tb, Tout, Tacc><<<grid_size, THREADS_PER_BLOCK, 0, stream>>>(
        dpIn, dpInB, dpOut, dpBias, dpWeights,
        numInCols, numOutCols, numInRows, numOutRows,
        batches, dim1, dim2, channels, numBChannels,
        inAChPitch, inALinePitch, inADIM1Pitch, inADIM2Pitch, inABatchPitch,
        inBChPitch, inBLinePitch, inBDIM1Pitch, inBDIM2Pitch, inBBatchPitch,
        outChPitch, outLinePitch, outDIM1Pitch, outDIM2Pitch, outBatchPitch,
        inputBTranspose, isBias, constIdx, isFloat, isHighPrecision,
        floatSatLow, floatSatHigh, satLow, satHigh, roundBits, mmaScale, dpScales, dpShifts);

    checkCudaErr(cudaMemcpyAsync(outPtr, dpOut, outputSize * sizeof(Tout),
                                  cudaMemcpyDeviceToHost, stream));
    checkCudaErr(cudaStreamSynchronize(stream));

    return IALG_EOK;
}

template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, short, int, unsigned char, long>(unsigned char const*, unsigned char const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, short, long, unsigned short, long>(unsigned short const*, short const*, unsigned short*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, short, long, unsigned char, long>(unsigned char const*, unsigned char const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, float, float, short, float>(unsigned short const*, unsigned short const*, short*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, float, float, signed char, float>(signed char const*, unsigned char const*, signed char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, short, int, unsigned char, long>(signed char const*, signed char const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, float, float, signed char, float>(signed char const*, signed char const*, signed char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, short, int, unsigned short, long>(short const*, short const*, unsigned short*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, short, long, short, long>(unsigned short const*, short const*, short*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, short, int, signed char, long>(unsigned char const*, unsigned char const*, signed char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, short, int, short, long>(short const*, short const*, short*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<float, float, float, float, float, float>(float const*, float const*, float*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, short, long, signed char, long>(signed char const*, unsigned char const*, signed char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<float, float, short, long, float, long>(float const*, float const*, float*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, short, long, unsigned short, long>(unsigned short const*, unsigned short const*, unsigned short*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, short, long, short, long>(unsigned short const*, unsigned short const*, short*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, short, int, unsigned char, long>(short const*, unsigned short const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, short, int, unsigned char, long>(short const*, short const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, float, float, unsigned char, float>(unsigned short const*, short const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, float, float, unsigned short, float>(short const*, short const*, unsigned short*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, short, long, signed char, long>(unsigned char const*, unsigned char const*, signed char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, float, float, short, float>(short const*, short const*, short*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, short, long, unsigned char, long>(unsigned short const*, unsigned short const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, short, int, unsigned char, long>(signed char const*, unsigned char const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, short, long, short, long>(short const*, short const*, short*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, short, long, unsigned char, long>(unsigned char const*, signed char const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, short, long, signed char, long>(signed char const*, signed char const*, signed char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, float, float, signed char, float>(unsigned char const*, unsigned char const*, signed char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, float, float, unsigned char, float>(short const*, short const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, short, int, signed char, long>(unsigned char const*, signed char const*, signed char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, short, long, unsigned char, long>(short const*, unsigned short const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, float, float, unsigned char, float>(short const*, unsigned short const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, short, int, short, long>(unsigned short const*, short const*, short*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, float, float, signed char, float>(unsigned char const*, signed char const*, signed char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, short, long, unsigned char, long>(short const*, short const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, short, int, unsigned short, long>(unsigned short const*, unsigned short const*, unsigned short*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, short, long, short, long>(short const*, unsigned short const*, short*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, float, float, unsigned char, float>(signed char const*, unsigned char const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, short, int, short, long>(unsigned short const*, unsigned short const*, short*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, float, float, unsigned char, float>(unsigned short const*, unsigned short const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, short, long, unsigned short, long>(short const*, short const*, unsigned short*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, short, int, short, long>(short const*, unsigned short const*, short*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, float, float, unsigned char, float>(signed char const*, signed char const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, short, int, signed char, long>(signed char const*, unsigned char const*, signed char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, short, int, unsigned char, long>(unsigned short const*, unsigned short const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, short, int, signed char, long>(signed char const*, signed char const*, signed char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, short, int, unsigned short, long>(unsigned short const*, short const*, unsigned short*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, short, long, unsigned char, long>(signed char const*, signed char const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<float, float, short, int, float, long>(float const*, float const*, float*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, short, long, unsigned char, long>(unsigned short const*, short const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, short, long, unsigned char, long>(signed char const*, unsigned char const*, unsigned char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, float, float, short, float>(short const*, unsigned short const*, short*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, short, long, signed char, long>(unsigned char const*, signed char const*, signed char*, long const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, float, float, unsigned short, float>(unsigned short const*, unsigned short const*, unsigned short*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, float, float, unsigned char, float>(unsigned char const*, signed char const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, float, float, short, float>(unsigned short const*, short const*, short*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, short, int, unsigned char, long>(unsigned short const*, short const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, short, int, unsigned char, long>(unsigned char const*, signed char const*, unsigned char*, int const*, short const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, float, float, unsigned char, float>(unsigned char const*, unsigned char const*, unsigned char*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, float, float, unsigned short, float>(unsigned short const*, short const*, unsigned short*, float const*, float const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, signed char, short, unsigned char, long>(short const*, short const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, signed char, int, unsigned char, long>(unsigned short const*, short const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, signed char, short, unsigned char, long>(signed char const*, signed char const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, signed char, short, short, long>(short const*, short const*, short*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, signed char, short, signed char, long>(unsigned char const*, signed char const*, signed char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, signed char, int, unsigned char, long>(unsigned char const*, signed char const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<float, float, signed char, int, float, long>(float const*, float const*, float*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, signed char, short, unsigned short, long>(unsigned short const*, short const*, unsigned short*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, signed char, int, unsigned short, long>(short const*, short const*, unsigned short*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, signed char, int, signed char, long>(unsigned char const*, signed char const*, signed char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, signed char, short, short, long>(unsigned short const*, short const*, short*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, signed char, short, unsigned short, long>(unsigned short const*, unsigned short const*, unsigned short*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, signed char, short, unsigned char, long>(signed char const*, unsigned char const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, signed char, int, signed char, long>(signed char const*, signed char const*, signed char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, signed char, int, unsigned char, long>(short const*, short const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, signed char, int, unsigned char, long>(signed char const*, unsigned char const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, signed char, short, signed char, long>(signed char const*, unsigned char const*, signed char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, signed char, int, short, long>(unsigned short const*, short const*, short*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, signed char, short, unsigned char, long>(unsigned short const*, short const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, signed char, int, signed char, long>(unsigned char const*, unsigned char const*, signed char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, signed char, short, short, long>(unsigned short const*, unsigned short const*, short*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, signed char, short, unsigned char, long>(short const*, unsigned short const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, unsigned char, signed char, int, signed char, long>(signed char const*, unsigned char const*, signed char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, signed char, signed char, short, unsigned char, long>(unsigned char const*, signed char const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, signed char, short, unsigned char, long>(unsigned short const*, unsigned short const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, signed char, int, unsigned char, long>(short const*, unsigned short const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, signed char, int, unsigned short, long>(unsigned short const*, unsigned short const*, unsigned short*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, signed char, short, unsigned char, long>(unsigned char const*, unsigned char const*, unsigned char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, signed char, int, unsigned char, long>(signed char const*, signed char const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, signed char, int, unsigned char, long>(unsigned short const*, unsigned short const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, signed char, short, signed char, long>(unsigned char const*, unsigned char const*, signed char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<float, float, signed char, short, float, long>(float const*, float const*, float*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, signed char, int, short, long>(short const*, unsigned short const*, short*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, unsigned short, signed char, short, short, long>(short const*, unsigned short const*, short*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned char, unsigned char, signed char, int, unsigned char, long>(unsigned char const*, unsigned char const*, unsigned char*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, short, signed char, int, unsigned short, long>(unsigned short const*, short const*, unsigned short*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, signed char, short, unsigned short, long>(short const*, short const*, unsigned short*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<short, short, signed char, int, short, long>(short const*, short const*, short*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<unsigned short, unsigned short, signed char, int, short, long>(unsigned short const*, unsigned short const*, short*, int const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductFused<signed char, signed char, signed char, short, signed char, long>(signed char const*, signed char const*, signed char*, short const*, signed char const*, int, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, float, float, int, int, int, int, unsigned char const*, unsigned char const*);
