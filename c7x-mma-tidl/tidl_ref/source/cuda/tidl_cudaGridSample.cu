/**
 ----------------------------------------------------------------------------
 @file    tidl_cudaGridSample.cu
 @brief   This file contains CUDA implementation of GridSample layer.
 @version 0.1 (Sep 2025) : Initial CUDA implementation
 ----------------------------------------------------------------------------
*/


#include <cuda_runtime.h>
#include <math.h>
#include <cuda.h>
#include "itidl_ti.h"
#include "tidl_cudaUtilities.cu"
#include "tidl_cuda_mem_manager.h"

using namespace floating_point::bf16_c7x;

#define TIDL_GRID_INTERNAL_SCALE_8BIT (16U)
#define TIDL_GRID_INTERNAL_SCALE_16BIT (256U)

// Note: CUDA numeric limits and utility functions are now defined in tidl_cudaUtilities.cu
// and declared in tidl_cuda.h for shared use across CUDA implementations

// Template-based GridSample kernel using conv2d style syntax
template <class Tin, class Tgrid, class Tout>
__global__ void GridSampleKernelFloat(
    const Tin* __restrict__ input,
    const Tgrid* __restrict__ grid,
    Tout* __restrict__ output,
    int align_mode,
    bool align_corners,
    int numBatches, int numDim1, int numDim2, int numChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inDim1Pitch, int inDim2Pitch, int inChannelPitch, int inLinePitch,
    int gridBatchPitch, int gridDim1Pitch, int gridDim2Pitch, int gridChannelPitch, int gridLinePitch,
    int outBatchPitch, int outDim1Pitch, int outDim2Pitch, int outChannelPitch, int outLinePitch,
    int inPadH, int inPadW, int gridPadH, int gridPadW, int outPadH, int outPadW, 
    int inTensorScale, int gridTensorScale, int outTensorScale, int isGridPrecomputed)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numDim1 * numDim2 * outHeight * outWidth;
    if(idx >= total_elements) return;

    // Apply padding offsets to pointers
    Tin* data = (Tin*)input + (inPadH * inLinePitch) + inPadW;
    Tgrid* grid_data = (Tgrid*)grid + (gridPadH * gridLinePitch) + gridPadW;
    Tout* out = (Tout*)output + (outPadH * outLinePitch) + outPadW;

    // Convert linear index to 5D coordinates (batch, dim1, dim2, height, width)
    int b = idx / (numDim1 * numDim2 * outHeight * outWidth);
    int d1 = (idx / (numDim2 * outHeight * outWidth)) % numDim1;
    int d2 = (idx / (outHeight * outWidth)) % numDim2;
    int h = (idx / outWidth) % outHeight;
    int w = idx % outWidth;

    int grid_offset = b * gridBatchPitch + d1 * gridDim1Pitch + d2 * gridDim2Pitch + h * gridChannelPitch + w * gridLinePitch;

    float x, y, out_val;

    x = (float)(grid_data[grid_offset]) / (float)gridTensorScale;
    y = (float)(grid_data[grid_offset + 1]) / (float)gridTensorScale;

    // Map coordinates from [-1,1] to pixel coordinates
    if (isGridPrecomputed == 0)
    {
        if (align_corners) {
            x = ((x + 1.0f) / 2.0f) * (inWidth - 1);
            y = ((y + 1.0f) / 2.0f) * (inHeight - 1);
        } else {
            x = (((x + 1.0f) * inWidth) - 1.0f) / 2.0f;
            y = (((y + 1.0f) * inHeight) - 1.0f) / 2.0f;
        }
    }

    if(align_mode == TIDL_ModeBilinear){
        float bL, bR, tL, tR;
        Tin bL_val, bR_val, tL_val, tR_val;
        int32_t x_bL, y_bL, x_bR, y_bR, x_tR, y_tR, x_tL, y_tL;

        x_bL = floorf(x);
        y_bL = floorf(y);
        x_bR = x_bL + 1;
        y_bR = y_bL;
        x_tL = x_bL;
        y_tL = y_bL + 1;
        x_tR = x_bL + 1;
        y_tR = y_bL + 1;
        
        // Weights for bilinear interpolation
        bL = (x_tR - x) * (y_tR - y);
        bR = (x - x_tL) * (y_tL - y);
        tL = (x_bR - x) * (y - y_bR);
        tR = (x - x_bL) * (y - y_bL);

        for (int c = 0; c < numChannels; c++) {
            // Sample four corner points with bounds checking
            bR_val = ((y_bR >=0) && (y_bR < inHeight) && (x_bR >=0) && (x_bR < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_bR*inChannelPitch) + (x_bR*inLinePitch)] : (Tin)0;
            bL_val = ((y_bL >=0) && (y_bL < inHeight) && (x_bL >=0) && (x_bL < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_bL*inChannelPitch) + (x_bL*inLinePitch)] : (Tin)0;
            tL_val = ((y_tL >=0) && (y_tL < inHeight) && (x_tL >=0) && (x_tL < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_tL*inChannelPitch) + (x_tL*inLinePitch)] : (Tin)0;
            tR_val = ((y_tR >=0) && (y_tR < inHeight) && (x_tR >=0) && (x_tR < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_tR*inChannelPitch) + (x_tR*inLinePitch)] : (Tin)0;
            
            // Compute bilinear interpolation
            out_val = ((float)bL_val * bL + (float)bR_val * bR + (float)tL_val * tL + (float)tR_val * tR) * (float)(outTensorScale) / (float)(inTensorScale);
            out[b * outBatchPitch + d1 * outDim1Pitch + d2 * outDim2Pitch + c + h * outChannelPitch + w * outLinePitch] = cuda_sat<Tout>(out_val);
        }
    }
    else if (align_mode == TIDL_ModeNearest){
        for (int c = 0; c < numChannels; c++){
            int x_nearest, y_nearest;

            if (isGridPrecomputed == 1)
            {
                x_nearest = (int)grid[grid_offset];
                y_nearest = (int)grid[grid_offset + 1];
            }
            else
            {
                x_nearest = nearbyintf(x);
                y_nearest = nearbyintf(y);
            }
            out_val = ((y_nearest >=0) && (y_nearest < inHeight) && (x_nearest >=0) && (x_nearest < inWidth)) ? (float)data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_nearest*inChannelPitch) + (x_nearest*inLinePitch)] : 0.f;
            out_val = out_val * (float)(outTensorScale) / (float)(inTensorScale);
            out[(b*outBatchPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + c + (h*outChannelPitch) + (w*outLinePitch)] = (Tout)out_val;
        }
    }
    else{
        printf("Mode not supported\n");
        return;
    }
}

// Template-based GridSample kernel using conv2d style syntax
template <class Tin, class Tgrid, class Tout>
__global__ void GridSampleKernelBFloat16(
    const Tin* __restrict__ input,
    const Tgrid* __restrict__ grid,
    Tout* __restrict__ output,
    int align_mode,
    bool align_corners,
    int numBatches, int numDim1, int numDim2, int numChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inDim1Pitch, int inDim2Pitch, int inChannelPitch, int inLinePitch,
    int gridBatchPitch, int gridDim1Pitch, int gridDim2Pitch, int gridChannelPitch, int gridLinePitch,
    int outBatchPitch, int outDim1Pitch, int outDim2Pitch, int outChannelPitch, int outLinePitch,
    int inPadH, int inPadW, int gridPadH, int gridPadW, int outPadH, int outPadW, 
    int inTensorScale, int gridTensorScale, int outTensorScale, int isGridPrecomputed)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numDim1 * numDim2 * outHeight * outWidth;
    if(idx >= total_elements) return;

    // Apply padding offsets to pointers
    Tin* data = (Tin*)input + (inPadH * inLinePitch) + inPadW;
    Tgrid* grid_data = (Tgrid*)grid + (gridPadH * gridLinePitch) + gridPadW;
    Tout* out = (Tout*)output + (outPadH * outLinePitch) + outPadW;

    // Convert linear index to 5D coordinates (batch, dim1, dim2, height, width)
    int b = idx / (numDim1 * numDim2 * outHeight * outWidth);
    int d1 = (idx / (numDim2 * outHeight * outWidth)) % numDim1;
    int d2 = (idx / (outHeight * outWidth)) % numDim2;
    int h = (idx / outWidth) % outHeight;
    int w = idx % outWidth;

    int grid_offset = b * gridBatchPitch + d1 * gridDim1Pitch + d2 * gridDim2Pitch + h * gridChannelPitch + w * gridLinePitch;

    bfloat16_tidl x, y, out_val;

    x = (grid_data[grid_offset]);
    y = (grid_data[grid_offset + 1]);

    // Map coordinates from [-1,1] to pixel coordinates
    if (isGridPrecomputed == 0)
    {
        if (align_corners == true)
        {
            x = ((x + (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5F) * ((bfloat16_tidl)inWidth - (bfloat16_tidl)1.0F);
            y = ((y + (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5F) * ((bfloat16_tidl)inHeight - (bfloat16_tidl)1.0F);
        }
        else
        {
            x = (((x + (bfloat16_tidl)1.0F) * ((bfloat16_tidl)inWidth)) - (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5F;
            y = (((y + (bfloat16_tidl)1.0F) * ((bfloat16_tidl)inHeight)) - (bfloat16_tidl)1.0F) * (bfloat16_tidl)0.5F;
        }
    }

    if(align_mode == TIDL_ModeBilinear){
        bfloat16_tidl bL, bR, tL, tR;
        Tin bL_val, bR_val, tL_val, tR_val;
        int32_t x_bL, y_bL, x_bR, y_bR, x_tR, y_tR, x_tL, y_tL;

        x_bL = floorf(x);
        y_bL = floorf(y);
        x_bR = x_bL + 1;
        y_bR = y_bL;
        x_tL = x_bL;
        y_tL = y_bL + 1;
        x_tR = x_bL + 1;
        y_tR = y_bL + 1;
        
        // Weights for bilinear interpolation
        bL = ((bfloat16_tidl)x_tR - x) * ((bfloat16_tidl)y_tR - y);
        bR = (x - (bfloat16_tidl)x_tL) * ((bfloat16_tidl)y_tL - y);
        tL = ((bfloat16_tidl)x_bR - x) * (y - (bfloat16_tidl)y_bR);
        tR = (x - (bfloat16_tidl)x_bL) * (y - (bfloat16_tidl)y_bL);

        for (int c = 0; c < numChannels; c++) {
            // Sample four corner points with bounds checking
            bR_val = ((y_bR >=0) && (y_bR < inHeight) && (x_bR >=0) && (x_bR < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_bR*inChannelPitch) + (x_bR*inLinePitch)] : (Tin)0.0f;
            bL_val = ((y_bL >=0) && (y_bL < inHeight) && (x_bL >=0) && (x_bL < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_bL*inChannelPitch) + (x_bL*inLinePitch)] : (Tin)0.0f;
            tL_val = ((y_tL >=0) && (y_tL < inHeight) && (x_tL >=0) && (x_tL < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_tL*inChannelPitch) + (x_tL*inLinePitch)] : (Tin)0.0f;
            tR_val = ((y_tR >=0) && (y_tR < inHeight) && (x_tR >=0) && (x_tR < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_tR*inChannelPitch) + (x_tR*inLinePitch)] : (Tin)0.0f;
            
            // Compute bilinear interpolation
            out_val = ((float)bL_val * (float)bL + 
                       (float)bR_val * (float)bR + 
                       (float)tL_val * (float)tL + 
                       (float)tR_val * (float)tR);

            out[b * outBatchPitch + d1 * outDim1Pitch + d2 * outDim2Pitch + c + h * outChannelPitch + w * outLinePitch] = cuda_sat_bf16(out_val);
        }
    }
    else if (align_mode == TIDL_ModeNearest){
        for (int c = 0; c < numChannels; c++){
            int x_nearest, y_nearest;

            if (isGridPrecomputed == 1)
            {
                x_nearest = (int32_t)grid[grid_offset];
                y_nearest = (int32_t)grid[grid_offset + 1];
            }
            else
            {
                x_nearest = nearbyintf(x);
                y_nearest = nearbyintf(y);
            }
            out_val = ((y_nearest >=0) && (y_nearest < inHeight) && (x_nearest >=0) && (x_nearest < inWidth)) ? (Tout)data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (y_nearest*inChannelPitch) + (x_nearest*inLinePitch)] : (Tout)0.0f;
            out[(b*outBatchPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + c + (h*outChannelPitch) + (w*outLinePitch)] = (Tout)out_val;
        }
    }
    else{
        printf("Mode not supported\n");
        return;
    }
}

// Fixed-point GridSample kernel template
template<class Tin, class Tgrid, class Tacc, class TgridDeNorm, class TgridWeight, class Tout>
__global__ void GridSampleKernelFixed(
    const Tin* __restrict__ input,
    const Tgrid* __restrict__ grid,
    Tout* __restrict__ output,
    int align_mode,
    bool align_corners,
    int numBatches, int numDim1, int numDim2, int numChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inDim1Pitch, int inDim2Pitch, int inChannelPitch, int inLinePitch,
    int gridBatchPitch, int gridDim1Pitch, int gridDim2Pitch, int gridChannelPitch, int gridLinePitch,
    int outBatchPitch, int outDim1Pitch, int outDim2Pitch, int outChannelPitch, int outLinePitch,
    int inPadH, int inPadW, int gridPadH, int gridPadW, int outPadH, int outPadW,
    int inTensorScale, int gridTensorScale, int outTensorScale)
{

    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int total_elements = numBatches * numDim1 * numDim2 * outHeight * outWidth;
    if(idx >= total_elements) return;

    Tgrid x, y;
    Tout out_val = 0;
    Tgrid internalGridScale;
    uint32_t internalPrecisionBits;
    int32_t rescaleFactor;
    TgridDeNorm x_1, y_1;
    Tgrid xint_floor, xint_ceil, yint_floor, yint_ceil, xint_round = 0, yint_round = 0;

    // Apply padding offsets to pointers
    Tin* data = (Tin*)input + (inPadH * inLinePitch) + inPadW;
    Tgrid* grid_data = (Tgrid*)grid + (gridPadH * gridLinePitch) + gridPadW;
    Tout* out = (Tout*)output + (outPadH * outLinePitch) + outPadW;

    // Convert linear index to 5D coordinates
    int b = idx / (numDim1 * numDim2 * outHeight * outWidth);
    int d1 = (idx / (numDim2 * outHeight * outWidth)) % numDim1;
    int d2 = (idx / (outHeight * outWidth)) % numDim2;
    int h = (idx / outWidth) % outHeight;
    int w = idx % outWidth;

    int grid_offset = b * gridBatchPitch + d1 * gridDim1Pitch + d2 * gridDim2Pitch + h * gridChannelPitch + w * gridLinePitch;

    // Get quantized grid coordinates
    x = (Tgrid)grid_data[grid_offset];
    y = (Tgrid)grid_data[grid_offset + 1];

    // Type-dependent parameters (following TIDL reference implementation)
    if (sizeof(Tin) == 1) 
    { // uint8_t or int8_t
        rescaleFactor = gridTensorScale / (float)TIDL_GRID_INTERNAL_SCALE_8BIT;
        internalPrecisionBits = (uint32_t)(__log2f(TIDL_GRID_INTERNAL_SCALE_8BIT));
        internalGridScale = TIDL_GRID_INTERNAL_SCALE_8BIT;
    } 
    else 
    { // uint16_t or int16_t
        rescaleFactor = gridTensorScale / (float)TIDL_GRID_INTERNAL_SCALE_16BIT;
        internalPrecisionBits = (uint32_t)(__log2f(TIDL_GRID_INTERNAL_SCALE_16BIT));
        internalGridScale = TIDL_GRID_INTERNAL_SCALE_16BIT;
    }
    
    uint32_t rescaleShift = (uint32_t)(__log2f(rescaleFactor));
    int32_t gridScaleFixed = (int32_t)gridTensorScale;

    // Map to actual position in internal scale
    if (align_corners) {
        x_1 = (((x + gridScaleFixed) >> 1) * (inWidth - 1)) >> rescaleShift;
        y_1 = (((y + gridScaleFixed) >> 1) * (inHeight - 1)) >> rescaleShift;
    } else {
        x_1 = (((x + gridScaleFixed) * inWidth) - gridScaleFixed) >> (1U + rescaleShift);
        y_1 = (((y + gridScaleFixed) * inHeight) - gridScaleFixed) >> (1U + rescaleShift);
    }

    if (align_mode == TIDL_ModeBilinear) {
        // Integer components for bilinear
        xint_floor = x_1 >> internalPrecisionBits;
        yint_floor = y_1 >> internalPrecisionBits;
        xint_ceil = xint_floor + 1;
        yint_ceil = yint_floor + 1;

        // Get fractional parts
        Tin fractionalMask = (1U << internalPrecisionBits) - 1U;
        Tin x_frac = x_1 & fractionalMask;
        Tin y_frac = y_1 & fractionalMask;

        TgridWeight w00, w01, w10, w11;

        // Calculate weights in internalGridScale^2 format
        w00 = (internalGridScale - x_frac) * (internalGridScale - y_frac);
        if (x_frac == 0 && y_frac == 0) {
            // Saturate w00 to max value for TgridWeight type (using clean numeric limits)
            w00 = cuda::std::numeric_limits<TgridWeight>::max();
        }
        w01 = x_frac * (internalGridScale - y_frac);
        w10 = (internalGridScale - x_frac) * y_frac;
        w11 = x_frac * y_frac;

        Tacc accumulatorValue;

        // Process each channel
        for (int c = 0; c < numChannels; c++) {
            // Sample four corner points with bounds checking
            Tin x00 = 0, x01 = 0, x10 = 0, x11 = 0;

            x00 = ((yint_floor >=0) && (yint_floor < inHeight) && (xint_floor >=0) && (xint_floor < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (yint_floor*inChannelPitch) + (xint_floor*inLinePitch)] : (Tin)0;
            x01 = ((yint_floor >=0) && (yint_floor < inHeight) && (xint_ceil >=0) && (xint_ceil < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (yint_floor*inChannelPitch) + (xint_ceil*inLinePitch)] : (Tin)0;
            x10 = ((yint_ceil >=0) && (yint_ceil < inHeight) && (xint_floor >=0) && (xint_floor < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (yint_ceil*inChannelPitch) + (xint_floor*inLinePitch)] : (Tin)0;
            x11 = ((yint_ceil >=0) && (yint_ceil < inHeight) && (xint_ceil >=0) && (xint_ceil < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (yint_ceil*inChannelPitch) + (xint_ceil*inLinePitch)] : (Tin)0;

            // Accumulate with higher precision
            accumulatorValue = ((x00 * w00) + (x01 * w01) + (x10 * w10) + (x11 * w11));
            
            // Round and saturate back to output scale (using clean numeric limits)
            out_val = cuda_roundSat(accumulatorValue, internalPrecisionBits * 2U, (int64_t)cuda::std::numeric_limits<Tout>::lowest(), (int64_t)cuda::std::numeric_limits<Tout>::max());
            
            // Write to output
            out[b * outBatchPitch + d1 * outDim1Pitch + d2 * outDim2Pitch + c + h * outChannelPitch + w * outLinePitch] = out_val;
        }
    }
    else if (align_mode == TIDL_ModeNearest) {
        // Round grid values and access
        for (int c = 0; c < numChannels; c++) {
            Tgrid xint_round = (Tgrid)cuda_roundSat(x_1, internalPrecisionBits, (int64_t)cuda::std::numeric_limits<TgridDeNorm>::lowest(), (int64_t)cuda::std::numeric_limits<TgridDeNorm>::max());
            Tgrid yint_round = (Tgrid)cuda_roundSat(y_1, internalPrecisionBits, (int64_t)cuda::std::numeric_limits<TgridDeNorm>::lowest(), (int64_t)cuda::std::numeric_limits<TgridDeNorm>::max());
            
            out_val = ((yint_round >=0) && (yint_round < inHeight) && (xint_round >=0) && (xint_round < inWidth)) ? data[(b*inBatchPitch) + (d1*inDim1Pitch) + (d2*inDim2Pitch) + (c*1) + (yint_round*inChannelPitch) + (xint_round*inLinePitch)] : (Tin)0;
            out[(b*outBatchPitch) + (d1*outDim1Pitch) + (d2*outDim2Pitch) + c + (h*outChannelPitch) + (w*outLinePitch)] = out_val;
        }
    }
    else{
        printf("Mode not supported\n");
        return;
    }
}

// TIDL CUDA wrapper for floating-point GridSample
template <class Tin, class Tgrid, class Tout>
int TIDL_cudaGridSampleFloat(
    const Tin* input,
    const Tgrid* grid, 
    Tout* output,
    int align_mode,
    bool align_corners,
    int numBatches, int numDim1, int numDim2, int numChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inDim1Pitch, int inDim2Pitch, int inChannelPitch, int inLinePitch,
    int gridBatchPitch, int gridDim1Pitch, int gridDim2Pitch, int gridChannelPitch, int gridLinePitch,
    int outBatchPitch, int outDim1Pitch, int outDim2Pitch, int outChannelPitch, int outLinePitch,
    int inPadH, int inPadW, int gridPadH, int gridPadW, int outPadH, int outPadW,
    int inTensorScale, int gridTensorScale, int outTensorScale, int isGridPrecomputed
)
{
    // Calculate memory sizes
    size_t input_size = numBatches * inBatchPitch * sizeof(Tin);
    size_t grid_size = numBatches * gridBatchPitch * sizeof(Tgrid);
    size_t output_size = numBatches * outBatchPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tgrid* d_grid = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers after synchronization
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), grid, (void**)&d_grid, grid_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size);

    checkCudaErr(cudaMemcpy(d_input, input, input_size, cudaMemcpyHostToDevice));
    checkCudaErr(cudaMemcpy(d_grid, grid, grid_size, cudaMemcpyHostToDevice));
    
    // Launch kernel
    int total_elements = numBatches * numDim1 * numDim2 * outHeight * outWidth;
    int grid_size_launch = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    if(std::is_same<Tin, bfloat16_tidl>::value)
    {
        GridSampleKernelBFloat16<Tin, Tgrid, Tout><<<grid_size_launch, THREADS_PER_BLOCK>>>(
        d_input, d_grid, d_output, align_mode, align_corners,
        numBatches, numDim1, numDim2, numChannels, inHeight, inWidth,
        outHeight, outWidth,
        inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
        gridBatchPitch, gridDim1Pitch, gridDim2Pitch, gridChannelPitch, gridLinePitch,
        outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
        inPadH, inPadW, gridPadH, gridPadW, outPadH, outPadW,
        inTensorScale, gridTensorScale, outTensorScale, isGridPrecomputed);

    }
    else 
    {

        GridSampleKernelFloat<Tin, Tgrid, Tout><<<grid_size_launch, THREADS_PER_BLOCK>>>(
            d_input, d_grid, d_output, align_mode, align_corners,
            numBatches, numDim1, numDim2, numChannels, inHeight, inWidth,
            outHeight, outWidth,
            inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
            gridBatchPitch, gridDim1Pitch, gridDim2Pitch, gridChannelPitch, gridLinePitch,
            outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
            inPadH, inPadW, gridPadH, gridPadW, outPadH, outPadW,
            inTensorScale, gridTensorScale, outTensorScale, isGridPrecomputed);
    }
            
    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpy(output, d_output, output_size, cudaMemcpyDeviceToHost));

    return IALG_EOK;
}

// TIDL CUDA wrapper for fixed-point GridSample
template <class Tin, class Tgrid, class Tacc, class TgridDeNorm, class TgridWeight, class Tout>
int TIDL_cudaGridSample(
    const Tin* input,
    const Tgrid* grid, 
    Tout* output,
    int align_mode,
    bool align_corners,
    int numBatches, int numDim1, int numDim2, int numChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inDim1Pitch, int inDim2Pitch, int inChannelPitch, int inLinePitch,
    int gridBatchPitch, int gridDim1Pitch, int gridDim2Pitch, int gridChannelPitch, int gridLinePitch,
    int outBatchPitch, int outDim1Pitch, int outDim2Pitch, int outChannelPitch, int outLinePitch,
    int inPadH, int inPadW, int gridPadH, int gridPadW, int outPadH, int outPadW,
    int inTensorScale, int gridTensorScale, int outTensorScale
)
{
    // Calculate memory sizes
    size_t input_size = numBatches * inBatchPitch * sizeof(Tin);
    size_t grid_size = numBatches * gridBatchPitch * sizeof(Tgrid);
    size_t output_size = numBatches * outBatchPitch * sizeof(Tout);

    Tin* d_input = NULL;
    Tgrid* d_grid = NULL;
    Tout* d_output = NULL;

    // Get GPU pointers after synchronization
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), input, (void**)&d_input, input_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), grid, (void**)&d_grid, grid_size);
    TIDL_cudaTranslatePtrCPUtoGPU(TIDL_cudaGetThreadManager(), output, (void**)&d_output, output_size);
    
    checkCudaErr(cudaMemcpy(d_input, input, input_size, cudaMemcpyHostToDevice));
    checkCudaErr(cudaMemcpy(d_grid, grid, grid_size, cudaMemcpyHostToDevice));

    // Launch kernel
    int total_elements = numBatches * numDim1 * numDim2 * outHeight * outWidth;
    int grid_size_launch = (total_elements + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
    
    GridSampleKernelFixed<Tin, Tgrid, Tacc, TgridDeNorm, TgridWeight, Tout><<<grid_size_launch, THREADS_PER_BLOCK>>>(
        d_input, d_grid, d_output, align_mode, align_corners,
        numBatches, numDim1, numDim2, numChannels, inHeight, inWidth,
        outHeight, outWidth,
        inBatchPitch, inDim1Pitch, inDim2Pitch, inChannelPitch, inLinePitch,
        gridBatchPitch, gridDim1Pitch, gridDim2Pitch, gridChannelPitch, gridLinePitch,
        outBatchPitch, outDim1Pitch, outDim2Pitch, outChannelPitch, outLinePitch,
        inPadH, inPadW, gridPadH, gridPadW, outPadH, outPadW,
        inTensorScale, gridTensorScale, outTensorScale);

    checkCudaErr(cudaGetLastError());
    checkCudaErr(cudaMemcpy(output, d_output, output_size, cudaMemcpyDeviceToHost));

    return IALG_EOK;
}



template int TIDL_cudaGridSample<unsigned char, short, unsigned int, short, unsigned char, unsigned char>(unsigned char const*, short const*, unsigned char*, int, bool, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGridSample<unsigned short, short, unsigned long, int, unsigned short, unsigned short>(unsigned short const*, short const*, unsigned short*, int, bool, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGridSample<short, short, long, int, unsigned short, short>(short const*, short const*, short*, int, bool, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGridSample<signed char, short, int, short, unsigned char, signed char>(signed char const*, short const*, signed char*, int, bool, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGridSampleFloat<float, float, float>(float const*, float const*, float*, int, bool, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);
template int TIDL_cudaGridSampleFloat<bfloat16_tidl, bfloat16_tidl, bfloat16_tidl>(bfloat16_tidl const*, bfloat16_tidl const*, bfloat16_tidl*, int, bool, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int);