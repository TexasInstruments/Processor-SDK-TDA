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
/* External declaration of the global memory manager pointer */
extern TIDL_CudaMemManager* g_cudaMemManager;

// Persistent memory
typedef struct {
  int isInit;
  void *dpInput;
  void *dpInputB;
  void *dpWeights;
  void *dpBias;
  void *dpAcc;
  void *dpOutput;
  void *dpScales;
  void *dpShifts;
} TIDL_cudaIP;

static TIDL_cudaIP CUDAIP[MEM_BUFF_ARRAY_LEN] = {0};


// Frees device pointers as well as resets the initialization flag for Inner Product CUDA operations
void TIDL_cudaFreeInnerProductCudaPtrs()
{
  for(int i = 0; i < MEM_BUFF_ARRAY_LEN; i++)
  {
     if(CUDAIP[i].dpInput) cudaFree(CUDAIP[i].dpInput);
     if(CUDAIP[i].dpInputB) cudaFree(CUDAIP[i].dpInputB);
     if(CUDAIP[i].dpWeights) cudaFree(CUDAIP[i].dpWeights);
     if(CUDAIP[i].dpBias) cudaFree(CUDAIP[i].dpBias);
     if(CUDAIP[i].dpAcc) cudaFree(CUDAIP[i].dpAcc);
     if(CUDAIP[i].dpOutput) cudaFree(CUDAIP[i].dpOutput);
     if(CUDAIP[i].dpScales) cudaFree(CUDAIP[i].dpScales);
     if(CUDAIP[i].dpShifts) cudaFree(CUDAIP[i].dpShifts);
     
     CUDAIP[i].dpInput = 0;
     CUDAIP[i].dpInputB = 0;
     CUDAIP[i].dpWeights = 0;
     CUDAIP[i].dpBias  = 0;
     CUDAIP[i].dpAcc = 0;
     CUDAIP[i].dpOutput = 0;
     CUDAIP[i].dpScales = 0;
     CUDAIP[i].dpShifts = 0;
     CUDAIP[i].isInit = 0;
  }
  cudaDeviceSynchronize();
}

void TIDL_cudaSetInnerProductInitFlag(int32_t layerIdx)
{
    if(layerIdx >= 0 && layerIdx < MEM_BUFF_ARRAY_LEN) {
        CUDAIP[layerIdx].isInit = 1;
    }
}

// for elementType == float
template <class Tout, class Tacc>
__global__ void InnerProductSaturationFloatKernel(
    const Tacc* __restrict__ accPtr, 
    Tout* __restrict__ outPtr,   
    
    // Validated tensor dimensions
    int32_t batches,
    int32_t dim1,
    int32_t dim2,
    int32_t channels,
    uint32_t numOutRows,
    uint32_t numOutCols,
    
    // Output pitch parameters
    uint32_t outChPitch,
    uint32_t outLinePitch,
    uint32_t outDIM1Pitch,
    uint32_t outDIM2Pitch,
    uint32_t outBatchPitch,
    
    // Float saturation parameters
    int32_t satLow,
    int32_t satHigh)
{
    // Calculate global thread index
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = batches * dim1 * dim2 * channels * numOutRows * numOutCols;
    if (idx >= total_elements) return;
    
    // Convert linear index to 6D coordinates
    int batchIdx = idx / (dim1 * dim2 * channels * numOutRows * numOutCols);
    int dim1Idx = (idx / (dim2 * channels * numOutRows * numOutCols)) % dim1;
    int dim2Idx = (idx / (channels * numOutRows * numOutCols)) % dim2;
    int chIdx = (idx / (numOutRows * numOutCols)) % channels;
    int outRwIdx = (idx / numOutCols) % numOutRows;
    int outColIdx = idx % numOutCols;
    
    // Calculate offset
    uint32_t offset = (batchIdx * outBatchPitch) + (dim1Idx * outDIM1Pitch) + 
                     (dim2Idx * outDIM2Pitch) + (chIdx * outChPitch) + 
                     (outRwIdx * outLinePitch) + outColIdx;
    
    // Phase 2 Float: Simple float saturation
    Tacc sum = accPtr[offset];
    Tout result = cuda_floatSat(sum, satLow, satHigh);
    outPtr[offset] = result;
}

// for isHighPrecision == 1
template <class Tout, class Tacc>
__global__ void InnerProductSaturationHighPrecisionKernel(
    const Tacc* __restrict__ accPtr,
    Tout* __restrict__ outPtr,
    const uint8_t* __restrict__ mmav2_Scales,  
    const uint8_t* __restrict__ mmav2_Shifts, 
    
    // Validated tensor dimensions
    int32_t batches,
    int32_t dim1,
    int32_t dim2,
    int32_t channels,
    uint32_t numOutRows,
    uint32_t numOutCols,
    
    // Output pitch parameters
    uint32_t outChPitch,
    uint32_t outLinePitch,
    uint32_t outDIM1Pitch,
    uint32_t outDIM2Pitch,
    uint32_t outBatchPitch,
    
    // Fixed-point saturation parameters
    int32_t satLow,
    int32_t satHigh)
{
    // Calculate global thread index
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = batches * dim1 * dim2 * channels * numOutRows * numOutCols;
    if (idx >= total_elements) return;
    
    // Convert linear index to 6D coordinates
    int batchIdx = idx / (dim1 * dim2 * channels * numOutRows * numOutCols);
    int dim1Idx = (idx / (dim2 * channels * numOutRows * numOutCols)) % dim1;
    int dim2Idx = (idx / (channels * numOutRows * numOutCols)) % dim2;
    int chIdx = (idx / (numOutRows * numOutCols)) % channels;
    int outRwIdx = (idx / numOutCols) % numOutRows;
    int outColIdx = idx % numOutCols;
    
    // Calculate offset
    uint32_t offset = (batchIdx * outBatchPitch) + (dim1Idx * outDIM1Pitch) + 
                     (dim2Idx * outDIM2Pitch) + (chIdx * outChPitch) + 
                     (outRwIdx * outLinePitch) + outColIdx;
    
    // Phase 2 High Precision: Per-column scaling + TIDL_roundSatMMA
    Tacc sum = accPtr[offset];
    
    // Get per-column scale and shift values
    uint8_t scale = mmav2_Scales[outColIdx];
    uint8_t shift = mmav2_Shifts[outColIdx];
    
    // Apply per-column scaling and MMA rounding/saturation
    int64_t scaled = (int64_t)sum * (int64_t)scale;
    Tout result = (Tout)cuda_roundSat(scaled, (int32_t)shift, satLow, satHigh);
    outPtr[offset] = result;
}

template <class Tout, class Tacc>
__global__ void InnerProductSaturationKernel(
    const Tacc* __restrict__ accPtr, 
    Tout* __restrict__ outPtr,  
    
    // Validated tensor dimensions
    int32_t batches,
    int32_t dim1,
    int32_t dim2,
    int32_t channels,
    uint32_t numOutRows,
    uint32_t numOutCols,
    
    // Output pitch parameters
    uint32_t outChPitch,
    uint32_t outLinePitch,
    uint32_t outDIM1Pitch,
    uint32_t outDIM2Pitch,
    uint32_t outBatchPitch,
    
    // Standard quantization parameters
    int32_t roundBits,
    int32_t mmaScale,
    int32_t satLow,
    int32_t satHigh)
{
    // Calculate global thread index
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = batches * dim1 * dim2 * channels * numOutRows * numOutCols;
    if (idx >= total_elements) return;
    
    // Convert linear index to 6D coordinates
    int batchIdx = idx / (dim1 * dim2 * channels * numOutRows * numOutCols);
    int dim1Idx = (idx / (dim2 * channels * numOutRows * numOutCols)) % dim1;
    int dim2Idx = (idx / (channels * numOutRows * numOutCols)) % dim2;
    int chIdx = (idx / (numOutRows * numOutCols)) % channels;
    int outRwIdx = (idx / numOutCols) % numOutRows;
    int outColIdx = idx % numOutCols;
    
    // Calculate offset
    uint32_t offset = (batchIdx * outBatchPitch) + (dim1Idx * outDIM1Pitch) + 
                     (dim2Idx * outDIM2Pitch) + (chIdx * outChPitch) + 
                     (outRwIdx * outLinePitch) + outColIdx;
    
    // Phase 2 Standard: Standard quantization + TIDL_roundSat (matching reference standard path)
    Tacc sum = accPtr[offset];
    
    // Apply standard scaling and rounding/saturation
    int64_t scaled = (int64_t)sum * (int64_t)mmaScale;
    Tout result = (Tout)cuda_roundSat(scaled, roundBits, satLow, satHigh);
    outPtr[offset] = result;
}

// Compute dot products and store in accumulator
template <class Tin, class TBin, class Tw, class Tb, class Tout, class Tacc>
__global__ void InnerProductKernel(
    // Validated pointers
    const Tin* __restrict__ inPtr,
    const TBin* __restrict__ inBPtr,
    Tacc* __restrict__ accPtr,        // Write to accumulator buffer
    const Tb* __restrict__ biasPtr,
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
    int32_t constIdx)
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
    
    // Initialize accumulator with bias if present
    Tacc sum = 0;
    if (biasPtr != NULL && isBias != 0) {
        if (constIdx == -1) {
            sum = biasPtr[(outRwIdx * numOutCols) + outColIdx];
        } else if (constIdx == 0) {
            sum = biasPtr[outRwIdx];
        } else if (constIdx == 1) {
            sum = biasPtr[outColIdx];
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
    // Store raw accumulator result
    accPtr[(batchIdx * outBatchPitch) + (dim1Idx * outDIM1Pitch) + (dim2Idx * outDIM2Pitch) + (chIdx * outChPitch) + (outRwIdx * outLinePitch) + outColIdx] = sum;
}


// TIDL CUDA wrapper for Inner Product main computation (Phase 1)
template <class Tin, class TBin, class Tw, class Tb, class Tout, class Tacc>
int TIDL_cudaInnerProduct(
    const Tin* inPtr,
    const TBin* inBPtr,
    Tacc* accPtr,
    const Tb* biasPtr,
    const Tw* weightsPtr,
    
    // Tensor dimensions
    int32_t batches,
    int32_t dim1,
    int32_t dim2,
    int32_t channels,
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
    int32_t constIdx
)
{
    // Calculate memory sizes
    size_t input_size = batches * inABatchPitch * sizeof(Tin);
    size_t inputB_size = (inBPtr != NULL) ? batches * inBBatchPitch * sizeof(TBin) : 0;
    size_t weights_size = (weightsPtr != NULL) ? numInCols * numOutCols * sizeof(Tw) : 0;
    size_t bias_size = (biasPtr != NULL) ? numOutRows * numOutCols * sizeof(Tb) : 0;
    size_t acc_size = batches * outBatchPitch * sizeof(Tacc);

    // Launch kernel parameters
    int total_elements = batches * dim1 * dim2 * channels * numOutRows * numOutCols;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    // Persistent memory allocation
    if(!CUDAIP[CUDNNLC].isInit) {
        checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpInput, input_size));
        if(inBPtr != NULL) {
            checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpInputB, inputB_size));
        }
        if(weightsPtr != NULL) {
            checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpWeights, weights_size));
        }
        if(biasPtr != NULL) {
            checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpBias, bias_size));
        }
        checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpAcc, acc_size));
    }

    // Get persistent pointers
    Tin* d_input = (Tin*)CUDAIP[CUDNNLC].dpInput;
    TBin* d_inputB = (inBPtr != NULL) ? (TBin*)CUDAIP[CUDNNLC].dpInputB : NULL;
    Tw* d_weights = (weightsPtr != NULL) ? (Tw*)CUDAIP[CUDNNLC].dpWeights : NULL;
    Tb* d_bias = (biasPtr != NULL) ? (Tb*)CUDAIP[CUDNNLC].dpBias : NULL;
    Tacc* d_acc = (Tacc*)CUDAIP[CUDNNLC].dpAcc;

    // Copy data to GPU
    checkCudaErr(cudaMemcpy(d_input, inPtr, input_size, cudaMemcpyHostToDevice));
    if(inBPtr != NULL) {
        checkCudaErr(cudaMemcpy(d_inputB, inBPtr, inputB_size, cudaMemcpyHostToDevice));
    }
    if(weightsPtr != NULL) {
        checkCudaErr(cudaMemcpy(d_weights, weightsPtr, weights_size, cudaMemcpyHostToDevice));
    }
    if(biasPtr != NULL) {
        checkCudaErr(cudaMemcpy(d_bias, biasPtr, bias_size, cudaMemcpyHostToDevice));
    }

    // Launch kernel
    InnerProductKernel<Tin, TBin, Tw, Tb, Tout, Tacc><<<grid_size, THREADS_PER_BLOCK>>>(
        d_input, d_inputB, d_acc, d_bias, d_weights,
        numInCols, numOutCols, numInRows, numOutRows,
        batches, dim1, dim2, channels,
        inAChPitch, inALinePitch, inADIM1Pitch, inADIM2Pitch, inABatchPitch,
        inBChPitch, inBLinePitch, inBDIM1Pitch, inBDIM2Pitch, inBBatchPitch,
        outChPitch, outLinePitch, outDIM1Pitch, outDIM2Pitch, outBatchPitch,
        inputBTranspose, isBias, constIdx);

    checkCudaErr(cudaGetLastError());
    
    // Copy accumulator result back to CPU
    checkCudaErr(cudaMemcpy(accPtr, d_acc, acc_size, cudaMemcpyDeviceToHost));

    return IALG_EOK;
}

// TIDL CUDA wrapper for Inner Product saturation (Phase 2)
template <class Tout, class Tacc>
int TIDL_cudaInnerProductSaturation(
    const Tacc* accPtr,
    Tout* outPtr,
    
    int32_t batches,
    int32_t dim1,
    int32_t dim2,
    int32_t channels,
    uint32_t numOutRows,
    uint32_t numOutCols,
    
    uint32_t outChPitch,
    uint32_t outLinePitch,
    uint32_t outDIM1Pitch,
    uint32_t outDIM2Pitch,
    uint32_t outBatchPitch,
    
    // Saturation parameters
    int32_t isFloat,
    int32_t isHighPrecision,
    int32_t roundBits,
    int32_t mmaScale,
    int32_t satLow,
    int32_t satHigh,
    const uint8_t* mmav2_Scales,
    const uint8_t* mmav2_Shifts
)
{
    // Calculate memory sizes
    size_t output_size = batches * outBatchPitch * sizeof(Tout);
    size_t acc_size = batches * outBatchPitch * sizeof(Tacc);
    size_t scales_size = numOutCols * sizeof(uint8_t);
    size_t shifts_size = numOutCols * sizeof(uint8_t);

    // Launch kernel parameters
    int total_elements = batches * dim1 * dim2 * channels * numOutRows * numOutCols;
    int grid_size = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;

    // Persistent memory allocation
    if(!CUDAIP[CUDNNLC].isInit) {
        checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpAcc, acc_size));
        checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpOutput, output_size));
        if(!isFloat && isHighPrecision){
            checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpScales, scales_size));
            checkCudaErr(cudaMalloc((void**)&CUDAIP[CUDNNLC].dpShifts, shifts_size));
        }
        CUDAIP[CUDNNLC].isInit = 1;
    }
    // Get persistent pointers
    Tacc* d_acc = (Tacc*)CUDAIP[CUDNNLC].dpAcc;
    Tout* d_output = (Tout*)CUDAIP[CUDNNLC].dpOutput;
    // Copy accumulator data to GPU
    checkCudaErr(cudaMemcpy(d_acc, accPtr, acc_size, cudaMemcpyHostToDevice));

    // Apply saturation and quantization
    if(isFloat) 
    {
        InnerProductSaturationFloatKernel<Tout, Tacc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_acc, d_output,
            batches, dim1, dim2, channels, numOutRows, numOutCols,
            outChPitch, outLinePitch, outDIM1Pitch, outDIM2Pitch, outBatchPitch,
            satLow, satHigh);

    }else if (isHighPrecision) 
    {
        uint8_t* d_scales = (uint8_t*)CUDAIP[CUDNNLC].dpScales;
        uint8_t* d_shifts = (uint8_t*)CUDAIP[CUDNNLC].dpShifts;
        checkCudaErr(cudaMemcpy(d_scales, mmav2_Scales, scales_size, cudaMemcpyHostToDevice));
        checkCudaErr(cudaMemcpy(d_shifts, mmav2_Shifts, shifts_size, cudaMemcpyHostToDevice));

        InnerProductSaturationHighPrecisionKernel<Tout, Tacc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_acc, d_output, d_scales, d_shifts,
            batches, dim1, dim2, channels, numOutRows, numOutCols,
            outChPitch, outLinePitch, outDIM1Pitch, outDIM2Pitch, outBatchPitch,
            satLow, satHigh);

    }else 
    {
        InnerProductSaturationKernel<Tout, Tacc><<<grid_size, THREADS_PER_BLOCK>>>(
            d_acc, d_output,
            batches, dim1, dim2, channels, numOutRows, numOutCols,
            outChPitch, outLinePitch, outDIM1Pitch, outDIM2Pitch, outBatchPitch,
            roundBits, mmaScale, satLow, satHigh);
    }

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpy(outPtr, d_output, output_size, cudaMemcpyDeviceToHost));

    return IALG_EOK;
}


template int TIDL_cudaInnerProduct<unsigned char, unsigned char, signed char, int, unsigned char, int>(unsigned char const*, unsigned char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, short, int, unsigned char, long>(unsigned char const*, unsigned char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, short, long, short, long>(unsigned short const*, short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<unsigned short, float>(float const*, unsigned short*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductSaturation<signed char, int>(int const*, signed char*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<short, unsigned short, signed char, short, unsigned char, int>(short const*, unsigned short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, signed char, signed char, int, signed char, int>(unsigned char const*, signed char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<float, float, short, long, float, long>(float const*, float const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<float, float, signed char, int, float, int>(float const*, float const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, signed char, int, short, int>(short const*, unsigned short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, short, long, unsigned short, long>(unsigned short const*, short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, short, long, unsigned short, long>(short const*, short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<float, float, short, int, float, long>(float const*, float const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, signed char, int, unsigned char, int>(short const*, short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, short, int, short, long>(short const*, short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, signed char, signed char, short, unsigned char, int>(unsigned char const*, signed char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, signed char, short, unsigned char, int>(short const*, short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, signed char, int, unsigned char, int>(signed char const*, unsigned char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, signed char, int, short, int>(unsigned short const*, unsigned short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, float, float, unsigned char, float>(unsigned short const*, short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, signed char, short, int, unsigned char, long>(unsigned char const*, signed char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, float, float, short, float>(short const*, unsigned short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, short, int, unsigned char, long>(signed char const*, signed char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, float, float, signed char, float>(signed char const*, signed char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, float, float, signed char, float>(unsigned char const*, unsigned char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, signed char, int, unsigned char, int>(unsigned short const*, short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, signed char, int, signed char, int>(signed char const*, signed char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<float, float, signed char, short, float, int>(float const*, float const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, signed char, short, short, int>(unsigned short const*, unsigned short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, signed char, int, short, int>(unsigned short const*, short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, short, long, unsigned short, long>(unsigned short const*, unsigned short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<signed char, long>(long const*, signed char*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProductSaturation<unsigned short, int>(int const*, unsigned short*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<short, short, signed char, short, unsigned short, int>(short const*, short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, signed char, int, unsigned short, int>(short const*, short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<short, float>(float const*, short*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<unsigned char, signed char, float, float, signed char, float>(unsigned char const*, signed char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, signed char, short, unsigned char, int>(signed char const*, unsigned char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<float, int>(int const*, float*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<short, short, float, float, short, float>(short const*, short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, short, int, short, long>(unsigned short const*, short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<short, int>(int const*, short*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<unsigned short, short, short, long, unsigned char, long>(unsigned short const*, short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, signed char, short, signed char, int>(signed char const*, unsigned char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, signed char, short, unsigned char, int>(signed char const*, signed char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, signed char, int, unsigned char, int>(short const*, unsigned short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, signed char, short, unsigned char, int>(unsigned short const*, short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, short, int, unsigned char, long>(short const*, short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, short, long, unsigned char, long>(unsigned char const*, unsigned char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, short, int, unsigned char, long>(short const*, unsigned short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, short, long, short, long>(short const*, short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, short, int, unsigned short, long>(unsigned short const*, unsigned short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, short, int, signed char, long>(signed char const*, signed char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, short, int, signed char, long>(signed char const*, unsigned char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, signed char, short, short, int>(unsigned short const*, short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, short, int, short, long>(unsigned short const*, unsigned short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, float, float, unsigned short, float>(unsigned short const*, short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, signed char, int, signed char, int>(signed char const*, unsigned char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, short, long, unsigned char, long>(short const*, short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, signed char, short, signed char, int>(signed char const*, signed char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<unsigned char, int>(int const*, unsigned char*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<unsigned short, short, signed char, short, unsigned short, int>(unsigned short const*, short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, signed char, int, unsigned short, int>(unsigned short const*, unsigned short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, short, int, signed char, long>(unsigned char const*, unsigned char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, float, float, short, float>(unsigned short const*, short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, short, int, unsigned char, long>(unsigned short const*, short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, short, long, short, long>(unsigned short const*, unsigned short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, signed char, int, unsigned short, int>(unsigned short const*, short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, float, float, signed char, float>(signed char const*, unsigned char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, float, float, unsigned char, float>(unsigned short const*, unsigned short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<signed char, float>(float const*, signed char*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<short, short, signed char, short, short, int>(short const*, short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, short, short, int, unsigned short, long>(unsigned short const*, short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, float, float, unsigned short, float>(unsigned short const*, unsigned short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<float, float, float, float, float, float>(float const*, float const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, signed char, int, short, int>(short const*, short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, signed char, short, int, signed char, long>(unsigned char const*, signed char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<unsigned short, long>(long const*, unsigned short*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<short, short, short, int, unsigned short, long>(short const*, short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, short, long, unsigned char, long>(short const*, unsigned short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, signed char, short, unsigned short, int>(unsigned short const*, unsigned short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, float, float, short, float>(unsigned short const*, unsigned short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, signed char, short, short, int>(short const*, unsigned short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, signed char, short, long, unsigned char, long>(unsigned char const*, signed char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, float, float, unsigned char, float>(short const*, unsigned short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, float, float, unsigned char, float>(unsigned char const*, unsigned char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<short, long>(long const*, short*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, short, long, signed char, long>(unsigned char const*, unsigned char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, signed char, short, signed char, int>(unsigned char const*, unsigned char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, signed char, short, unsigned char, int>(unsigned short const*, unsigned short const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, short, long, signed char, long>(signed char const*, unsigned char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, short, long, signed char, long>(signed char const*, signed char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, float, float, unsigned char, float>(short const*, short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<float, long>(long const*, float*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, signed char, int, unsigned char, int>(unsigned short const*, unsigned short const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, short, long, short, long>(short const*, unsigned short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<unsigned char, long>(long const*, unsigned char*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<unsigned char, signed char, signed char, int, unsigned char, int>(unsigned char const*, signed char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, signed char, signed char, short, signed char, int>(unsigned char const*, signed char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, short, long, unsigned char, long>(unsigned short const*, unsigned short const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, float, float, unsigned char, float>(signed char const*, signed char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<float, float>(float const*, float*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<unsigned char, signed char, short, long, signed char, long>(unsigned char const*, signed char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, signed char, int, signed char, int>(unsigned char const*, unsigned char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProductSaturation<unsigned char, float>(float const*, unsigned char*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int, int, int, int, unsigned char const*, unsigned char const*);
template int TIDL_cudaInnerProduct<signed char, signed char, short, long, unsigned char, long>(signed char const*, signed char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, unsigned short, short, int, short, long>(short const*, unsigned short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, signed char, signed char, int, unsigned char, int>(signed char const*, signed char const*, int*, int const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, signed char, float, float, unsigned char, float>(unsigned char const*, signed char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, short, long, unsigned char, long>(signed char const*, unsigned char const*, long*, long const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, float, float, unsigned char, float>(signed char const*, unsigned char const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned short, unsigned short, short, int, unsigned char, long>(unsigned short const*, unsigned short const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<short, short, float, float, unsigned short, float>(short const*, short const*, float*, float const*, float const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<unsigned char, unsigned char, signed char, short, unsigned char, int>(unsigned char const*, unsigned char const*, int*, short const*, signed char const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
template int TIDL_cudaInnerProduct<signed char, unsigned char, short, int, unsigned char, long>(signed char const*, unsigned char const*, long*, int const*, short const*, int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int, int, int);
