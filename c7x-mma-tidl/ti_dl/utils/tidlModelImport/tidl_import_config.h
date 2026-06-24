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


#ifndef _EVELIB_YUV_SCALAR_CONFIG_H_
#define _EVELIB_YUV_SCALAR_CONFIG_H_

#include "configparser.h"
#include "ti_dl.h"
#include <string>
#include <vector>
#include <utility>
using namespace std;
#define FILE_NAME_SIZE  (512)
#define TIDL_IN_NUF_MAX_CH (256)
//NCI, NCO, PRED_CIRC, DWS_CONV_STRIDE
#define DEFAULT_COMPILE_CONSTRAINT_NC_FLAGS (0x1 | 0x40 | 0x200 | 0x400)
/* Copy of NC TIDL_ENABLE_PERFLOG_TRACE flag */
#define TIDL_ENABLE_PERFLOG_TRACE (0x5000000)
#define ADD_DC_LAYER_AT_INPUT  (1)
#define ADD_DC_LAYER_AT_OUTPUT (2)

typedef enum{
TIDL_OPTIMIZE_LEVEL_BASIC = 0, //This enables the regular optimization
TIDL_OPTIMIZE_LEVEL_EXTENDED = 1  // This enables regular optimizations + shape folding.
} TIDL_OPTIMIZATION_LEVEL;

typedef struct
{
    int32_t  randParams;
    string   inStrVal;
    int32_t  modelType; // 0 - Caffe, 1 - TF, 2 - ONNX, 3 - TFLite, 4 - Relay
    int32_t  quantizationStyle;
    int32_t  calibrationOption;
    int32_t  activationRangeMethod;
    int32_t  weightRangeMethod;
    float    percentileActRangeShrink;
    float    percentileWtRangeShrink;
    float    biasCalibrationFactor;
    int32_t  biasCalibrationIterations;
    int32_t  quantRoundAdd;
    int32_t  numParamBits;
    int32_t  numFeatureBits;
    int32_t  inFileFormat;
    int32_t  numFrames;
    int32_t  numFramesBiasCalibration;
    int32_t  postProcType;
    int32_t  postProcDataId;
    int32_t  foldMaxPoolInConv2D;
    int32_t  foldDepthToSpaceInConv2D;
    int32_t  foldPreBnConv2D;
    int32_t  foldBnInConv2D;
    int32_t  foldEltWiseInConv2D;
    int32_t  metaArchType;
    int32_t  debugTraceLevel;
    int32_t  writeTraceLevel;
    int32_t  compileConstraintsFlag;
    int32_t  executeNetworkCompiler;
    int32_t  msmcSizeKB;
    int32_t  deviceName;
    int32_t  numCores;
    float    quantRangeUpdateFactor;
    float    quantRangeExpansionFactor;
    int32_t  addDataConvertToNet;
    bool     isQuantParamsToBeExported;
    int32_t  inDataNorm[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inDataPadInTIDL[TIDL_MAX_ALG_IN_BUFS];
    uint8_t  inData[FILE_NAME_SIZE];
    uint8_t  inDataNamesList[TIDL_MAX_ALG_IN_BUFS*FILE_NAME_SIZE];
    uint8_t  outDataNamesList[TIDL_MAX_ALG_OUT_BUFS*FILE_NAME_SIZE];
    uint8_t  metaLayersNamesList[TIDL_MAX_ALG_IN_BUFS*FILE_NAME_SIZE];
    uint8_t  perfSimTool[FILE_NAME_SIZE];
    uint8_t  graphVizTool[FILE_NAME_SIZE];
    uint8_t  modelDumpTool[FILE_NAME_SIZE];
    uint8_t  perfSimConfig[FILE_NAME_SIZE];
    uint8_t  tidlStatsTool[FILE_NAME_SIZE];
    uint8_t  inputNetFile[FILE_NAME_SIZE];
    uint8_t  inputParamsFile[FILE_NAME_SIZE];
    uint8_t  outputNetFile[FILE_NAME_SIZE];
    uint8_t  quantParamsPrototxtFile[FILE_NAME_SIZE];
    uint8_t  outputNetQuantParamsPrototxtFile[FILE_NAME_SIZE];
    uint8_t  outputParamsFile[FILE_NAME_SIZE];
    uint8_t  configLine[FILE_NAME_SIZE];
    uint8_t  negativeTestUpdate[FILE_NAME_SIZE];
    uint8_t  configParam[FILE_NAME_SIZE];
    uint8_t  fileNameGrpInfo[FILE_NAME_SIZE];
    float    inQuantFactor[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inElementType[TIDL_MAX_ALG_IN_BUFS];
    /*modelInElementType stores native element types from original model for each input buffer*/
    int32_t  modelInElementType[TIDL_MAX_ALG_IN_BUFS];
    int32_t  actMethod[TIDL_MAX_ALG_IN_BUFS];
    uint8_t  actType[TIDL_MAX_ALG_IN_BUFS*FILE_NAME_SIZE];
    int32_t  NetInElementType[TIDL_MAX_ALG_IN_BUFS];
    int32_t  rawDataInElementType[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inZeroPoint[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inLayout[TIDL_MAX_ALG_IN_BUFS];
    /*modelOutElementType stores native element types from original model for each output buffer*/
    int32_t  modelOutElementType[TIDL_MAX_ALG_OUT_BUFS];
    int32_t  outElementType[TIDL_MAX_ALG_OUT_BUFS];
    float    outTensorScale[TIDL_MAX_ALG_OUT_BUFS];
    int32_t  outZeroPoint[TIDL_MAX_ALG_OUT_BUFS];
    int32_t  outLayout[TIDL_MAX_ALG_OUT_BUFS];
    int32_t  outElementSize[TIDL_MAX_ALG_OUT_BUFS];
    /* Parameters to overwrite the proto txt first feature dimension */
    int32_t  inWidth[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inHeight[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inNumChannels[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inDIM2[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inDIM1[TIDL_MAX_ALG_IN_BUFS];
    int32_t  numBatches[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inferenceMode;
    float    inMean[TIDL_MAX_ALG_IN_BUFS*TIDL_IN_NUF_MAX_CH];
    float    inScale[TIDL_MAX_ALG_IN_BUFS*TIDL_IN_NUF_MAX_CH];
    int32_t  inDataFormat[TIDL_MAX_ALG_IN_BUFS];
    int32_t  inResizeType[TIDL_MAX_ALG_IN_BUFS];
    int32_t  resizeWidth[TIDL_MAX_ALG_IN_BUFS];
    int32_t  resizeHeight[TIDL_MAX_ALG_IN_BUFS];
    int32_t  reserved[TIDL_MAX_ALG_IN_BUFS];
    int32_t  executeQuantsTool;
    int32_t  enableHighResOptimization;
    int32_t  enableOtfPad;
    int32_t  enableCustomLayers;
    int32_t  packetizeMode;
    int32_t  inYuvFormat[TIDL_MAX_ALG_IN_BUFS];
    int32_t  ddrLayers[TIDL_NUM_MAX_LAYERS];
    uint8_t  singleCoreLayersNames[TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE];
    uint8_t  spatialSplitLayersNames[TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE];
    uint8_t  channelSplitLayersNames[TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE];
    uint8_t  outputFeature16bitNamesList[TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE];
    uint8_t  mp16bitResolvedFeatureNamesList[TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE];
    uint8_t  params16bitNamesList[TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE];
    uint8_t  nativeLayerNamesList[TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE];
    float    mixedPrecisionFactor;
    /**< Group for a given model, same network with different resolution
     * can share same mdoelGroupId to have optimization for DDR space */
    uint8_t  modelGroupId ;
    /*Does the network support preQuantizedModel*/
    int32_t  preQuantizedModel;
    /*Serialized protobuf for onnx:*/

    /* Enable biasFuse into Innerproduct for Matmul*/
    int32_t enableBiasFusion;
    string * onnxProto;
    /* Option to give custom network name, used when naming subgraphs*/
    uint8_t  networkName[FILE_NAME_SIZE];
    /* Option to give C7x firmware version via OSRT*/
    uint8_t  c7xFirmwareVersion[FILE_NAME_SIZE];
    /* Log file name */
    uint8_t logFileName[FILE_NAME_SIZE];
    /* Batch Mode */
    int32_t batchMode;
    /* Enable partial init during model compilation*/
    int8_t  partialInitDuringCompile;
    /* Flag indicating a scale update for softmax*/
    int32_t softmax16BitScaleUpdate;
    /* Flag indicating use of higher precision for TopK*/
    int32_t use16BitForTopK;
    /* Path to dump NC info files */
    uint8_t ncTempInfoDir[FILE_NAME_SIZE];
    /* Enable TFR optimization*/
    int32_t  enableTFROptimization;
    /* Flag to enable/disable optimization level*/
    int32_t graphOptimizationLevel;
    /* Flag to enable/disable shape folding*/
    int32_t enableShapeFolding;
    /* Flag to control batchnorm higher dims fuse*/
    int32_t optimizeBatchNormHigherDims;
    /* Flag to force spatial split in low latency mode*/
    int32_t forceSpatialSplitInLowLatencyMode;
    /* Flag to force batch split in low latency model*/
    int32_t forceBatchSplitInLowLatencyMode;
    /* Flag to optimize TopK axis*/
    int32_t optimizeTopKAxis;
    /* Copy tesnor quant spec */
    std::vector<std::pair<std::string,std::string>> copyTensorQuantSpec;
    /* Enable Concat No Op optimization*/
    int32_t  enableConcatNoOp;
    /* Enable zero-point for Output DataConvert*/
    int32_t enableZeroPointForOutputDataConvert;
} tidl_import_config;

extern sTokenMapping gsTokenMap_tidl_import_config[MAX_ITEMS_TO_PARSE];

#endif /* _EVELIB_YUV_SCALAR_CONFIG_H_*/
