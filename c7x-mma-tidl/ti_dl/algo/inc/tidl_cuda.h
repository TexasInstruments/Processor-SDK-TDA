/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
*
* All rights reserved not granted herein.
*
* Limited License.
*
* Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
* license under copyrights and patents it now or hereafter owns or controls to make,
* have made, use, import, offer to sell and sell ("Utilize") this software subject to the
* terms herein.  With respect to the foregoing patent license, such license is granted
* solely to the extent that any such patent is necessary to Utilize the software alone.
* The patent license shall not apply to any combinations which include this software,
* other than combinations with devices manufactured by or for TI ("TI Devices").
* No hardware patent is licensed hereunder.
*
* Redistributions must preserve existing copyright notices and reproduce this license
* (including the above copyright notice and the disclaimer and (if applicable) source
* code license limitations below) in the documentation and/or other materials provided
* with the distribution
*
* Redistribution and use in binary form, without modification, are permitted provided
* that the following conditions are met:
*
* *       No reverse engineering, decompilation, or disassembly of this software is
* permitted with respect to any software provided in binary form.
*
* *       any redistribution and use are licensed by TI for use only with TI Devices.
*
* *       Nothing shall obligate TI to provide you with source code for the software
* licensed and provided to you in object code.
*
* If software source code is provided to you, modification and redistribution of the
* source code are permitted provided that the following conditions are met:
*
* *       any redistribution and use of the source code, including any resulting derivative
* works, are licensed by TI for use only with TI Devices.
*
* *       any redistribution and use of any object code compiled from the source code
* and any resulting derivative works, are licensed by TI for use only with TI Devices.
*
* Neither the name of Texas Instruments Incorporated nor the names of its suppliers
*
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* DISCLAIMER.
*
* THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#ifndef TIDL_CUDA_
#define TIDL_CUDA_

#ifdef BUILD_WITH_CUDA
#include "itidl_ti.h"


template <class Tin, class Tw, class Tb, class Tacc>
int TIDL_cudaConvolve2d(
  Tin*     pInChannel,
  Tw*      pCoeffs,
  Tb*      pBias,
  Tacc *   accPtr,
  Tacc *   min,
  Tacc *   max,
  int32_t  numTotRoi,
  int32_t  numGroups,
  int32_t  numInChannels,
  int32_t  numOutChannels,
  int32_t  inChPitch,
  int32_t  outChPitch,
  int32_t  width,
  int32_t  height,
  int32_t  inImPitch,
  int32_t  outImPitch,
  int32_t  coeffsWidth,
  int32_t  coeffsHeight,
  int32_t  dilationWidth,
  int32_t  dilationHeight,
  int32_t  strideWidth,
  int32_t  strideHeight,
  int32_t enableBias,
  int32_t isOTFpad,
  int32_t leftPad,
  int32_t topPad,
  int32_t padVal,
  int32_t startRowNumberInTensor,
  int32_t inHeight,
  int32_t inWidth,
  int32_t outHeight,
  int32_t outWidth
  );
template <class Tx>
void minmaxGPU(const int numTotRoi, const Tx* x, int *min, int *max);
template <class Tx>
void minmax_Thrust(const int N,const Tx* x,Tx *min,Tx *max);

int TIDL_cudaConv2DSaturateFloat(float *accPtr,
                                 float *outPtr,
                                 int32_t numBatches,
                                 int32_t numOutChannels,
                                 int32_t height,
                                 int32_t width,
                                 int32_t strideHeight,
                                 int32_t strideWidth,
                                 int32_t outBatchPitch,
                                 int32_t outChPitch,
                                 int32_t outImPitch,
                                 float padVal,
                                 int32_t outHeight,
                                 int32_t outWidth,
                                 int32_t outPadOffset,
                                 sTIDL_Layer_t *pTIDLNet
                                );

template <class Tacc, class Tout>
int TIDL_cudaSaturateFixedPoint(
                                 Tacc *accPtr,
                                 Tout *outPtr,
                                 int32_t numBatches,
                                 int32_t numOutChannels,
                                 int32_t height,
                                 int32_t width,
                                 int32_t strideHeight,
                                 int32_t strideWidth,
                                 int32_t outBatchPitch,
                                 int32_t outChPitch,
                                 int32_t outImPitch,
                                 uint8_t *mmav2_Scales,
                                 uint8_t *mmav2_Shifts,
                                 int32_t outRoundBits, /*Required by V1 flow*/
                                 int32_t minSat,
                                 int32_t maxSat,
                                 int32_t padVal,
                                 int32_t outHeight,
                                 int32_t outWidth,
                                 int32_t outPadOffset,
                                 int32_t mixedPrecision,
                                 sTIDL_Layer_t *pTIDLNet
                                );


#include "tidl_cuda_mem_manager.h"

int TIDL_isLayerSupportedOnGPU();

template<class Tsrc, class TminMax>
void TIDL_cudaMinMax(
    const Tsrc* ptr,
    TminMax* min,
    TminMax* max,
    int32_t dataSize
);

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
);

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
);

// GridSample CUDA function declarations
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
    int inTensorScale, int gridTensorScale, int outTensorScale
);

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
);

template <class Tin, class TBin, class Tw, class Tb, class Tout, class Tacc>
int TIDL_cudaInnerProductFused(
    const Tin* inPtr, const TBin* inBPtr, Tout* outPtr, const Tb* biasPtr, const Tw* weightsPtr,
    int32_t batches, int32_t dim1, int32_t dim2, int32_t channels, int32_t numBChannels, uint32_t numInCols, uint32_t numOutCols, uint32_t numInRows, uint32_t numOutRows,
    uint32_t inAChPitch, uint32_t inALinePitch, uint32_t inADIM1Pitch, uint32_t inADIM2Pitch, uint32_t inABatchPitch,
    uint32_t inBChPitch, uint32_t inBLinePitch, uint32_t inBDIM1Pitch, uint32_t inBDIM2Pitch, uint32_t inBBatchPitch,
    uint32_t outChPitch, uint32_t outLinePitch, uint32_t outDIM1Pitch, uint32_t outDIM2Pitch, uint32_t outBatchPitch,
    int32_t inputBTranspose, int32_t isBias, int32_t constIdx, int32_t numInBufs, int32_t isFloat, int32_t isHighPrecision,
    float floatSatLow, float floatSatHigh,
    int32_t satLow, int32_t satHigh,
    int32_t roundBits, int32_t mmaScale,
    const uint8_t* mmav2_Scales,  
    const uint8_t* mmav2_Shifts
);

// BatchNorm CUDA function declarations
template <class Tin, class Tw, class Tb, class Tacc>
int TIDL_cudaBatchNorm(
    const Tin* input,
    const Tw* weights,
    const Tw* slopes,
    const Tb* bias,
    Tacc* accumulator,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int numDIM1, int numDIM2,
    int inBatchPitch, int inChPitch, int inDIM1Pitch, int inDIM2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch,
    int actType, float slopeScale
);

template <class Tacc, class Tout>
int TIDL_cudaBatchNormSaturation(
    const Tacc* accumulator,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int numDIM1, int numDIM2,
    int outBatchPitch, int outChPitch, int outDIM1Pitch, int outDIM2Pitch, int outPitch,
    int outRoundBits, int satLow, int satHigh, int mixedPrecision, float floatSatLow, float floatSatHigh
);

// High Accuracy Sigmoid CUDA function declaration
template <class Tin, class Tout>
int TIDL_cudaHighAccuracySigmoid(
    const Tin* input,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    float inputScale, float outputScale
);

// 4-Point Approximation Sigmoid CUDA function declarations
template <class Tin, class Tout, class Tacc>
int TIDL_cudaSigmoid(
    const Tin* input,
    Tacc* accumulator,
    int numTotBatches, int numChannels, int imWidth, int imHeight,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    uint32_t threshold0, uint32_t threshold1, uint32_t threshold2, uint16_t inDataScale,
    Tout* slope, Tout* offset, Tout offsetScale
);

template <class Tacc, class Tout>
int TIDL_cudaSigmoidSaturation(
    const Tacc* accumulator,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight,
    int outBatchPitch, int outChPitch, int outPitch,
    int outRoundBits, Tout satLow, Tout satHigh
);

template <class Tin, class Tout>
int TIDL_cudaFloatSigmoid(
    const Tin* input,
    Tout* output,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch
);

// Non-Linear LUT CUDA function declaration
template <class Tin, class Tout>
int TIDL_cudaNonLinearLUT(
    const Tin* input,
    Tout* output,
    const Tout* lutTable,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    int readOffsetLUT
);

// Non-Linear Interpolation LUT CUDA function declaration
template <class Tin, class Tout>
int TIDL_cudaNonLinearInterpolLUT(
    const Tin* input,
    Tout* output,
    const Tout* lutTable,
    int numTotBatches, int numChannels, int imWidth, int imHeight, int inDim1, int inDim2,
    int inBatchPitch, int inChPitch, int inDim1Pitch, int inDim2Pitch, int inPitch,
    int outBatchPitch, int outChPitch, int outDim1Pitch, int outDim2Pitch, int outPitch,
    int32_t f1, int32_t f2, int32_t inoffset
);

template <class Tin, class Tacc>
int TIDL_cudaEltWiseOp(
    const Tin* input,
    Tacc* accumulator,
    int32_t scale,
    int32_t zeropoint,
    int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch, int pixelPitch,
    int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int callno, int batchno, int eltWiseType, int fModValue, int outBatchPitch
);

template <class Tacc, class Tout>
int TIDL_cudaEltWiseMMAv2Quantize(
    const Tacc* accumulator,
    Tout* output,
    uint8_t mmaScale,
    uint8_t mmaShift,
    int32_t biasTerm,
    int numBatches, int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch, int tensorZeroPoint, int outPadOffset
);

template <class Tacc, class Tout>
int TIDL_cudaEltWiseQuantize(
    const Tacc* accumulator,
    Tout* output,
    int numBatches, int numDIM1, int numDIM2, int numChannels, int inHeight, int inWidth,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int roundBits, int32_t satLow, int32_t satHigh, int mixedPrecision,
    float floatSatLow, float floatSatHigh
);

template <class Tin, class Tout>
int TIDL_refCudaSlice(
  const Tin* pIn,
  Tout*  pOut,
  int32_t inPtrOffset, int32_t outPtrOffset,
  int32_t outWidth, int32_t outHeight,int32_t numChs,
  int32_t numDim1,int32_t numDim2,int32_t numROIs,
  int32_t inLinePitch,int32_t outLinePitch, int32_t inChPitch,
  int32_t outChPitch, int32_t inDim1Pitch, int32_t outDim1Pitch, int32_t inDim2Pitch,
  int32_t outDim2Pitch, int32_t inROIPitch, int32_t outROIPitch
);

template <class Tin, class Tout>
int TIDL_cudaTranspose(
    const Tin* input,
    Tout* output,
    int inBatches, int outBatches, int inDIM1, int inDIM2, int inChannels, int inHeight, int inWidth,
    int inBatchPitch, int inDIM1Pitch, int inDIM2Pitch, int inChPitch, int inPitch,
    int outBatchPitch, int outDIM1Pitch, int outDIM2Pitch, int outChPitch, int outPitch,
    int outDIM1, int outDIM2, int outChannels, int outHeight, int outWidth,
    int pp, int lp, int cp, int d1p, int d2p, int bp
);

// Concat CUDA function declarations
template <class Tin, class Tacc>
int TIDL_cudaConcatOp(
    const Tin* input,
    Tacc* accumulator,
    int32_t scale,
    int numChannels, int inHeight, int inWidth,
    int inChPitch, int outChPitch, int inPitch, int outPitch,
    int tensorIdx, int isKernelHighPrecision,
    uint8_t mmaScale, uint8_t mmaShift, int32_t biasTerm,
    int outElemType, int32_t satLow, int32_t satHigh
);

template <class Tacc, class Tout>
int TIDL_cudaConcatQuantize(
    const Tacc* accumulator,
    Tout* output,
    int numChannels, int inHeight, int inWidth,
    int outChPitch, int outPitch,
    int roundBits, int32_t satLow, int32_t satHigh,
    int isKernelHighPrecision, int outElemType, int32_t outPadOffset,
    float floatSatLow, float floatSatHigh, int32_t tensorZeroPoint
);

// Resize CUDA function declarations
template<class T>
int TIDL_cudaResize(
    const T* input,
    T* output,
    int numBatches, int numInChannels, int inHeight, int inWidth,
    int outHeight, int outWidth,
    int inBatchPitch, int inChPitch, int inPitch,
    int outBatchPitch, int outChPitch, int outPitch,
    float hRatio, float wRatio, int resizePadZeroOffset, int mode, int leftPadResize, int padHeight, int padWidth, int inOffset, int outOffset, int tensorZeroPoint
);

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
);

int TIDL_cudaSoftmaxFloat(
    float* input,
    float* output,
    int32_t* inDims,
    int32_t* inPitches,
    int32_t* outPitches
);

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
);

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
);

/* NOTE: CUDNNLC global has been removed for thread safety.
 * Pass layerIdx through function parameters instead.
 */


/*Enable layer level:*/
#define BUILD_WITH_CUDA_MEM_MANAGER
#define BUILD_WITH_CUDA_INNERPRODUCT
#define BUILD_WITH_CUDA_BATCHNORM
#define BUILD_WITH_CUDA_ELTWISE
#define BUILD_WITH_CUDA_LAYERNORM
// #define BUILD_WITH_CUDA_SLICE
#define BUILD_WITH_CUDA_SOFTMAX
#define BUILD_WITH_CUDA_TRANSPOSE
#define BUILD_WITH_CUDA_CONCAT
#define BUILD_WITH_CUDA_RESIZE
// #define BUILD_WITH_CUDA_GRIDSAMPLE
//#define BUILD_WITH_CUDA_STATS

#endif
#endif
