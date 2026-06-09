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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <asprintf.h>
#else
#include<dirent.h>
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
using namespace std;
using ::google::protobuf::Message;

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tidl_import_config.h"
#include "tidl_import_common.h"
#include "tidl_custom_import.h"
#include "tidl_import_common_model_check.h"
#include "perfsim.h"
#include "tidl_deviceInfo.h"
#include "tidl_import_quantize.h"
#include "tidl_import_common_tidlrt_osrt.h"
#include "gc.h"
#include "gc_helper.h"

#define __MODULE__ ""

sTIDL_OrgNetwork_t  orgTIDLNetStructure;
sTIDL_OrgNetwork_t tempTIDLNetStructure;
sTIDL_Network_t tIDLNetStructure;
unordered_set<int32_t> unsupportedLayersForOTF = { TIDL_Deconv2DLayer };
unordered_set<int32_t> padUnsupportedLayers = { TIDL_ReshapeLayer, TIDL_TransposeLayer };
char inDataNames[TIDL_MAX_ALG_IN_BUFS][TIDL_MAX_DATA_NAME];
char outDataNames[TIDL_MAX_ALG_OUT_BUFS][TIDL_MAX_DATA_NAME];
char outMetaLayerNames[TIDL_MAX_ALG_OUT_BUFS][TIDL_MAX_DATA_NAME];
int32_t numNetInData = 0;
int32_t numNetOutData = 0;
char inConfigFilename[2*FILE_NAME_SIZE];
tidl_import_config gParams;
sTIDL_multiCoreBatchProcessing multiCoreBatchProcInfo;
extern int32_t numTFMetaLayers;
uint32_t tidl_reorderedParamsOffset = 0;

char* getFileNameFromPath(char* path)
{
  for (int32_t i = strlen(path) - 1; i; i--)
  {
    if ((path[i] == '/') || (path[i] == '\\'))
    {
      return &path[i + 1];
    }
  }
  return path;
}

void getDirFromPath(char* path)
{
  for (int32_t i = strlen(path) - 1; i; i--)
  {
    if ((path[i] == '/') || (path[i] == '\\'))
    {
      path[i] = '\0';
      return;
    }
  }
  path[0] = '\0';
  return;
}
void getDosPath(char* orgpath, char * dosPath)
{
  strcpy(dosPath, orgpath);
  for (int32_t i = 0; i < strlen(dosPath); i++)
  {
    if (dosPath[i] == '/')
    {
      dosPath[i] = '\\';
    }
  }
  return;
}

void getAbsPath(char* path, char * absPath)
{
  char syscmd[500];
  char dosPath[500];
#ifdef _WIN32
  getDosPath(path, dosPath);
  sprintf(syscmd, "dir /b /s %s", dosPath);
#else
  sprintf(syscmd, "readlink -f %s", path);
#endif

#ifdef _WIN32
  FILE * fp = _popen(syscmd,  "r");
#else
  FILE * fp = popen(syscmd,  "r");
#endif

  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Error while runing command: %s", syscmd);
  }
  fscanf(fp, "%s", absPath);
  fclose(fp);
  return;
}

void setDefaultParams(tidl_import_config * params)
{
  int32_t i;
  params->randParams          = 0;
  params->modelType           = 0; // 0 - caffe, 1- tensorFlow
  params->quantizationStyle   = TIDL_QuantStyleNP2Fixed;
  params->calibrationOption = 0;
  params->activationRangeMethod = TIDL_ActivationRangeMethodHistogram;
  params->weightRangeMethod = TIDL_WeightRangeMethodMedian;
  params->percentileActRangeShrink = 0.01;
  params->percentileWtRangeShrink = 0.01;
  params->biasCalibrationFactor = 0.05;
  params->biasCalibrationIterations = -1;
  params->quantRoundAdd       = 50; // 0 - caffe, 1- tensorFlow
  params->numParamBits        = 8;
  params->inFileFormat        = 2; // 0 - Encoded, 1- RAW
  params->numFrames           = -1;
  params->numFramesBiasCalibration = -1;  // number of frames to be used for bias calibration
  params->foldBnInConv2D      = 1;
  params->foldPreBnConv2D      = 1;
  params->foldEltWiseInConv2D = 0;
  params->foldMaxPoolInConv2D = 0;
  params->foldDepthToSpaceInConv2D = 0;
  params->postProcType        = 0;
  params->postProcDataId      = 0;
  params->numFeatureBits      = 8;
  params->metaArchType        =-1;
  params->debugTraceLevel     = 0;
  params->writeTraceLevel     = 0;
  params->quantRangeUpdateFactor       = -1.0;
  params->compileConstraintsFlag = DEFAULT_COMPILE_CONSTRAINT_NC_FLAGS;
  params->executeNetworkCompiler = 1;
  params->executeQuantsTool   = 1;
  params->enableHighResOptimization = 0;
  params->enableCustomLayers = 0;
  params->packetizeMode = -1;
  params->msmcSizeKB = -1;
  params->deviceName = -1;
  params->numCores = 1;
  params->inferenceMode = TIDL_inferenceModeDefault;
  params->quantRangeExpansionFactor = 1.0;
  params->addDataConvertToNet = 0;
  params->mixedPrecisionFactor = -1;
  params->modelGroupId = 0 ;
  params->enableOtfPad = 0;
  params->isQuantParamsToBeExported = false;
  params->preQuantizedModel = 0;
  params->enableBiasFusion = 0;
  params->batchMode = 0;
  params->partialInitDuringCompile = 0;
  params->softmax16BitScaleUpdate = 0;
  params->use16BitForTopK = 0;
  params->enableTFROptimization = -1;
  params->enableShapeFolding = 0;
  params->optimizeBatchNormHigherDims = 0;
  params->forceBatchSplitInLowLatencyMode = 0;
  params->forceSpatialSplitInLowLatencyMode = 0;
  params->optimizeTopKAxis = 1;

#ifdef _WIN32
  strcpy((char*)params->tidlStatsTool,"..\\..\\test\\PC_dsp_test_dl_algo.out.exe");
  strcpy((char*)params->perfSimTool,  "..\\..\\utils\\perfsim\\ti_cnnperfsim.out.exe");
  strcpy((char*)params->graphVizTool, "..\\..\\utils\\tidlModelGraphviz\\out\\tidl_graphVisualiser.out.exe");
  strcpy((char*)params->modelDumpTool, "..\\..\\utils\\tidlModelDump\\out\\tidl_dump.out.exe");
  strcpy((char*)params->perfSimConfig, "..\\..\\test\\testvecs\\config\\import\\device_config.cfg");
#else
  strcpy((char*)params->tidlStatsTool, "../../test/PC_dsp_test_dl_algo.out");
  strcpy((char*)params->perfSimTool, "../../utils/perfsim/ti_cnnperfsim.out");
  strcpy((char*)params->graphVizTool, "../../utils/tidlModelGraphviz/out/tidl_graphVisualiser.out");
  strcpy((char*)params->modelDumpTool, "../../utils/tidlModelDump/out/tidl_dump.out");
  strcpy((char*)params->perfSimConfig, "../../test/testvecs/config/import/device_config.cfg");
#endif

  strcpy((char*)params->inDataNamesList, "");
  strcpy((char*)params->outDataNamesList, "");
  strcpy((char*)params->singleCoreLayersNames, "");
  strcpy((char*)params->spatialSplitLayersNames, "");
  strcpy((char*)params->channelSplitLayersNames, "");
  strcpy((char*)params->outputFeature16bitNamesList, "");
  strcpy((char*)params->params16bitNamesList, "");
  strcpy((char*)params->fileNameGrpInfo, "");
  strcpy((char*)params->c7xFirmwareVersion, C7X_FIRMWARE_VERSION);
  strcpy((char*)params->networkName, "");
  strcpy((char*)params->logFileName, "");
  strcpy((char*)params->negativeTestUpdate, "");
  strcpy((char*)params->ncTempInfoDir, "/tmp");
  strcpy((char*)params->actType, "");

  for (i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++)
  {
    params->inElementType[i] = TIDL_UnsignedChar;
    params->NetInElementType[i] = -1;
    params->rawDataInElementType[i] = -1;
    params->inZeroPoint[i] = 0;
    params->inLayout[i] = TIDL_LT_NCHW;
    params->inQuantFactor[i] =  1.0;
    params->inWidth[i]       = -1;
    params->inHeight[i]      = -1;
    params->inNumChannels[i] = -1;
    params->inDIM2[i]      = 1;
    params->inDIM1[i]      = 1;
    params->numBatches[i] = 1;
    params->resizeWidth[i]  = -1;
    params->resizeHeight[i] = -1;
    params->inResizeType[i] = TIDL_inResizeTypeDefault;
    params->inDataFormat[i] = TIDL_inDataFormatRGBPlanar;
    params->reserved[i]     = 0;
    params->inDataNorm[i]   = 0;
    params->inYuvFormat[i]  = NOT_VALID;
    params->inDataPadInTIDL[i]  = 0;
    params->actMethod[i] = -1;
  }
  for (i = 0; i < TIDL_MAX_ALG_OUT_BUFS; i++)
  {
    params->outElementSize[i] = -1;
    params->outElementType[i] = -1;
   /* set outTensorScale 0, this will be used to distinguish
     whether user expects the outTensorScale be computed by TIDL or want TIDL
     to honor user given value */
    params->outTensorScale[i] = 0.0;
    params->outZeroPoint[i]   = 0;
    params->outLayout[i]      = TIDL_LT_NCHW;

  }
  params->ddrLayers[0] = -1;

  multiCoreBatchProcInfo.numBatches = 1;
  multiCoreBatchProcInfo.numCores = 1;

}

int32_t isSupertilingEnabled(tidl_import_config * params)
{
  int32_t supertilingEnabled = 0;
  if ((params->enableHighResOptimization > 0U           ) ||
       ((params->compileConstraintsFlag & 0x80)== 0x80    ) ||
       (strlen((const char*)params->fileNameGrpInfo) != 0 ))
  {
    supertilingEnabled = 1;
  }
  return supertilingEnabled;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_Import
@fn         tidlValidateImportParams
@brief      Function validates input parameters related to tidl import
            sets appropriate error in response to violation from
            expected values.

@param      params : TIDL Create time parameters
@remarks    None
@return     Error related to parameter.
----------------------------------------------------------------------------
*/
int32_t tidlValidateImportParams(tidl_import_config * params)
{
  /* randParams can be either 0 or 1*/
  if(params->foldMaxPoolInConv2D == 1)
  {
     params->foldMaxPoolInConv2D = 0;
     TIDL_GLOBAL_REPORT_UNSUPPORTED("foldMaxPoolInConv2D is not suported in the current release. Disabling now");
  }
  if(params->foldEltWiseInConv2D == 1)
  {
     params->foldEltWiseInConv2D = 0;
     TIDL_GLOBAL_REPORT_UNSUPPORTED("foldEltWiseInConv2D is not suported in the current release. Disabling now");
  }
  if(params->numParamBits == 32)
  {
    params->numFeatureBits = 32;
  }

  if((params->numParamBits > 8) && (params->numFeatureBits <= 8))
  {
    params->numFeatureBits = 16;
  }
  if((params->numParamBits <= 8) && (params->numFeatureBits > 8))
  {
    params->numParamBits = 12;
  }
  /* Set inElementType based on numFeatureBits if not set in config file */
  if ((params->numFeatureBits > 8) && ((gParams.addDataConvertToNet & ADD_DC_LAYER_AT_INPUT) == 0))
  {
    for (int32_t i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++)
    {
      if((params->inElementType[i] == TIDL_SinglePrecFloat) ||
         (params->inElementType[i] == TIDL_SignedWord) ||
         (params->inElementType[i] == TIDL_UnsignedWord)
        )
      {
        continue;
      }
      if(params->numFeatureBits == 32)
      {
        params->inElementType[i] = TIDL_SinglePrecFloat;
      }
      else if(params->inElementType[i] == TIDL_UnsignedChar)
      {
        params->inElementType[i] = TIDL_UnsignedShort;
      }
      else if(params->inElementType[i] == TIDL_SignedChar)
      {
        params->inElementType[i] = TIDL_SignedShort;
      }
    }
  }

  if((params->randParams != 0) && (params->randParams != 1))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid randParams(%d) setting, it should be either 0 or 1", params->randParams);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* modelType must be one of the supported types */
  else if((params->modelType != 0) && (params->modelType != 1) && (params->modelType != 2)
          && (params->modelType != 3) && (params->modelType != 4) && (params->modelType != 5)&& (params->modelType != 6))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid modelType parameter(%d) setting, it should be in between 0 and 6", params->modelType);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* Currently quantizationStyle = 1 is supported */
  /*else if(params->quantizationStyle != 1)
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid quantizationStyle parameter setting : set it to 1");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }*/
  /* quantRoundAdd can be 0 to 100 */
  else if((params->quantRoundAdd < 0) || (params->quantRoundAdd > 100))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid quantRoundAdd parameter(%d) setting, it should be in between 0 and 100", params->quantRoundAdd);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* numParamBits can be 4 to 12 */
  else if(((params->numParamBits < 4) || (params->numParamBits > 16)) && (params->numParamBits != 32))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid numParamBits parameter(%d) setting, it should be either 4 to 16 or 32 for Float mode", params->numParamBits);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* inFileFormat can be either 0 or 1*/
  else if((params->inFileFormat < 0) && (params->inFileFormat > 2))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inFileFormat parameter(%d) setting, it can be either 0 or 1", params->inFileFormat);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* numFrames can be >0  */
  else if(params->numFrames < -1)
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid numFrames parameter(%d) setting, it should be >0", params->numFrames);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else if(params->numFramesBiasCalibration < -1)
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid numFramesBiasCalibration parameter(%d) setting, it should be >0", params->numFramesBiasCalibration);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* foldBnInConv2D can be either 0 or 1*/
  else if((params->foldBnInConv2D != 0) && (params->foldBnInConv2D != 1))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid foldBnInConv2D parameter(%d) setting, it should be either 0 or 1", params->foldBnInConv2D);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else if((params->foldPreBnConv2D != 0) && (params->foldPreBnConv2D != 1) && (params->foldPreBnConv2D != 2))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid foldPreBnConv2D parameter(%d) setting, it should be either 0, 1 or 2", params->foldPreBnConv2D);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* inElementType can be either 0 or 1*/
  else if((params->inElementType[0] < 0) && (params->inElementType[0] > 3 ))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inElementType parameter(%d) setting, it should be in between 0 and 3", params->inElementType[0]);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* inQuantFactor can be >0  */
  else if(params->inQuantFactor[0] <= 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inQuantFactor parameter(%d) setting, it should be >0", params->inQuantFactor[0]);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* inWidth can be >0  */
  else if((params->inWidth[0] < -1) || (params->inWidth[0] == 0))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inWidth parameter(%d) setting, it should be >0", params->inWidth[0]);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* inHeight can be >0  */
  else if((params->inHeight[0] < -1) || (params->inHeight[0] == 0))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inHeight parameter(%d) setting, it should be >0", params->inHeight[0]);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  /* inNumChannels must be >= 1 */
  else if((params->inNumChannels[0] < -1) || (params->inNumChannels[0] == 0))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inNumChannels parameter(%d) setting, inNumChannels must be non-zero and positive", params->inNumChannels[0]);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else if((params->numBatches[0] < -1) || (params->numBatches[0] == 0) || (params->numBatches[0] > 1024))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid numBatches parameter(%d) setting, it should be in between 1 and 1024 ", params->numBatches[0]);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
#if defined (SOC_J784S4) || defined (SOC_J722S) || defined(SOC_J742S2) || defined(SOC_TDA54)
  else if((params->numCores != 1) && (params->numCores != 2) && (params->numCores != 3) && (params->numCores != 4))
  {
    std::string cores="1";
    for(int i = 2;i <= TIDL_MAX_NUM_CORES;i++)
    {
      cores+=", ";
      cores+=std::to_string(i);
    }
    /*prints upto the number of cores for the device */
    TIDL_GLOBAL_REPORT_ERROR("Invalid numCores parameter(%d) setting, number of cores must be either %s", params->numCores, cores.c_str());

    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else if((params->inferenceMode != 0) && (params->inferenceMode != 1) && (params->inferenceMode != 2))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inferenceMode parameter(%d) setting, it should be either 0, 1 or 2", params->inferenceMode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else if((params->numCores) > TIDL_MAX_NUM_CORES) /*the macro value TIDL_MAX_NUM_CORES is decided based on device*/
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid numCores(%d) : Device has total %d cores", params->numCores, TIDL_MAX_NUM_CORES);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
#if defined (SOC_J742S2)
  else if((params->numCores) > 2) /*the macro value TIDL_MAX_NUM_CORES set to 4 for SOC_J742S2 for compatibility*/
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid numCores(%d) : Device has total 2 cores", params->numCores);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
#endif
  else if(params->inferenceMode == 2 && params->numCores == 1) /*when runing on one core inference mode must be 0*/
  {
    TIDL_GLOBAL_REPORT_ERROR("inferenceMode = 2 is supported only for numCores > 1, use inferenceMode = 0 for numCores = 1");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
#else
  else if((params->numCores != 1))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid numCores parameter(%d) setting, number of cores must be 1", params->numCores);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else if(params->inferenceMode != 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid inferenceMode parameter(%d) setting, must be 0", params->inferenceMode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
#endif
  else if(params->inferenceMode == TIDL_inferenceModeHighThroughput) /* Multi core batch processing checks */
  {
    // if(params->deviceName != TIDL_TDA4VH) /* TODO_NG : Enable this check after adding separate device name for TDA4VH */
    // {
    //   TIDL_GLOBAL_REPORT_ERROR("Inference mode 'TIDL_inferenceModeHighThroughput' is not supported for this device");
    //   return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    // }

    if((params->modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE) || (params->modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW) ||
    (params->modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE) || (params->modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX))
    {
      /* Batch processing only supported for inFileFormat = 2 - check required only for standalone mode. OSRT does not use inFileFormat option */
      if((params->inFileFormat != 1) && (params->inFileFormat != 2))
      {
        TIDL_GLOBAL_REPORT_ERROR("Parameter inFileFormat must be either 1 or 2 for multi core batch processing");
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
    }
  }
  else if((params->inferenceMode == 0) && (params->numCores > 1))
  {
    TIDL_GLOBAL_REPORT_ERROR("Only numCores = 1 supported for inferenceMode = 0");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  else if(params->inferenceMode == TIDL_inferenceModeLowLatency)
  {
    if (isSupertilingEnabled(&gParams) == 1)
    {
      TIDL_GLOBAL_REPORT_ERROR("High resolution optimization is currently not enabled for inference mode = 2, please use inference mode = 0");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    if(gParams.batchMode == 1)
    {
      TIDL_GLOBAL_REPORT_ERROR("Batch Stitch Mode is not supported for inference mode = 2, please use inference mode = 0");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
  }
  else
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

#define TIDL_CFG_MAX_LINE_SIZE (3000)
int32_t getNumberOfLinesIntheFile(char * fileName)
{
  FILE * fp1;
  int32_t i, lineCnt = 0;
  char line[TIDL_CFG_MAX_LINE_SIZE];

  fp1 = fopen((const char *)fileName, "r");
  if (fp1 == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open %s file for reading, check if the file exists and is readable", fileName);
    return 0;
  }
  while (!feof(fp1))
  {
    fgets(line, TIDL_CFG_MAX_LINE_SIZE, fp1);
    lineCnt++;
  }
  return(lineCnt);
}

int32_t getAVXKernelEnv()
{
int32_t flowCtrl = 3;
#if (HOST_EMULATION)
    char *str;
    str = getenv("TIDL_RT_AVX_REF");
    if(!str)
    {
        flowCtrl |= 0x00000020;
    }
    else
    {
        int32_t
        env = atoi(str);
        if((env != 0) && (env != 1) && (env != 3))
        {
          TIDL_GLOBAL_REPORT_UNSUPPORTED("TIDL_RT_AVX_REF is expected to be either 0, 1 or 3. -  %d is not supported. Setting it to zero", env);
          env = 0;
        }
        if(env)
        {
          flowCtrl |= env << 5;
        }
   }
#endif
    return flowCtrl;
}

void tidlWriteInferenceConfig(FILE * fp, tidl_import_config * params)
{
  char absPath[500];
  char fileName[500];
  char outDirName[500];

  getAbsPath((char *)params->outputNetFile, outDirName);
  getDirFromPath(outDirName);

  fprintf(fp, "inFileFormat    = %d\n",params->inFileFormat);
  fprintf(fp, "numFrames   = %d\n",params->numFrames);
  fprintf(fp, "postProcType   = %d\n",params->postProcType);
  fprintf(fp, "postProcDataId   = %d\n", params->postProcDataId);
  fprintf(fp, "quantRangeUpdateFactor   = %f\n", params->quantRangeUpdateFactor);
  getAbsPath((char *)params->inData, absPath);
  fprintf(fp, "inData   = %s\n",absPath);
  fprintf(fp, "outData   = \"%s/%s_stats_tool_out.bin\"\n", outDirName,getFileNameFromPath(inConfigFilename));

  getAbsPath((char *)params->outputNetFile, absPath);
  fprintf(fp, "netBinFile     = %s\n", absPath);

  sprintf(fileName, "%s%d.bin", (char *)params->outputParamsFile, 1);
  getAbsPath(fileName, absPath);
  fprintf(fp, "ioConfigFile        = %s\n", absPath);
  fprintf(fp, "writeTraceLevel        = %d\n", params->writeTraceLevel);
  fprintf(fp, "debugTraceLevel        = %d\n", params->debugTraceLevel);
  fprintf(fp, "traceDumpBaseName        = \"%s/%s\n", outDirName,getFileNameFromPath(inConfigFilename));
}

int32_t tidlQuantStatsTool(tidl_import_config * params)
{
  FILE * fp;
  char sysCommand[500];
  char orgPath[500];
  char absPath[500];
  char fileName[500];
  char qsFileName[500];
  char cfgFileName[500];
  char dirName[500];
  char outDirName[500];
  int systemReturnCode;

  getAbsPath((char *)params->outputNetFile, outDirName);
  getDirFromPath(outDirName);

  sprintf(qsFileName, "%s/%s.qunat_stats_config.txt", outDirName,getFileNameFromPath(inConfigFilename));
  fp = fopen(qsFileName, "w+");
  if(fp== NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open config file %s, check if file exists and is writable", qsFileName);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  tidlWriteInferenceConfig(fp, params);
  fprintf(fp, "flowCtrl               = %d\n", getAVXKernelEnv());

  fclose(fp);

  strcpy(orgPath, qsFileName);
  getAbsPath(orgPath, absPath);

  strcpy(fileName, getFileNameFromPath((char *)params->tidlStatsTool));

  getAbsPath((char *)params->tidlStatsTool, dirName);
  getDirFromPath(dirName);

#ifdef _WIN32
  sprintf(sysCommand, "cd %s && %s s:%s", dirName, fileName, absPath);
#else
  sprintf(sysCommand, "cd %s && ./%s s:%s", dirName, fileName, absPath);
#endif
  if(params->debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s", sysCommand);
  }

  systemReturnCode = system(sysCommand);
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("Failed to run calibration pass, system command returned error: %d", systemReturnCode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int64_t tidl_getFileSize(uint8_t * fileString)
{
  FILE * fptr;
  int64_t netSize;
  fptr = fopen((const char *)fileString, "rb");
  if (fptr)
  {
    fseek(fptr, 0L, SEEK_END);
    netSize = ftell(fptr);
    fclose(fptr);
    return netSize;
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open file %s", fileString);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

}

int tidlRunGraphVizTool(tidl_import_config * params)
{
  FILE * fp;
  char sysCommand[500];
  char absPath[500];
  int systemReturnCode;

  fp = fopen((const char *)params->graphVizTool, "r");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Couldn't open graphVizTool file: %s . Skipping Graph Visualization.", params->graphVizTool);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  fclose(fp);

  getAbsPath((char *)params->graphVizTool, absPath);
  sprintf(sysCommand, "%s %s", absPath, params->outputNetFile);
  if(gParams.debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s", sysCommand);
  }
  systemReturnCode = system(sysCommand);
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_WARNING("System command failed with return code : %d. Skipping Graph Visualization.", systemReturnCode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int tidlRunModelDumpTool(tidl_import_config * params)
{
  FILE * fp;
  char sysCommand[2048];
  char absPath[500];
  char dumpFileName[500];
  int systemReturnCode;

  fp = fopen((char *)params->modelDumpTool, "r");
  if (fp == NULL)
  {
    //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Couldn't open modelDumpTool file: %s , Skipping\n", params->modelDumpTool);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  fclose(fp);
  getAbsPath((char *)params->modelDumpTool, absPath);

  strcpy(dumpFileName, (char*)params->outputNetFile);
  strcat(dumpFileName, ".txt");

  sprintf(sysCommand, "%s -model %s -io %s1.bin -perfsim -o %s",
          absPath, params->outputNetFile, params->outputParamsFile, dumpFileName);
  if(gParams.debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s", sysCommand);
  }

  systemReturnCode = system(sysCommand);
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_WARNING("System command failed with return code : %d. Skipping Model Dump.", systemReturnCode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int tidlWriteTensorNamesToFile(tidl_import_config * params, const char* suffix)
{
  FILE  * fp;
  char *fname;
  asprintf(&fname, "%s%s.layer_info.txt", params->outputNetFile, suffix);
  FILE *layerInfoFile = fopen(fname, "w");
  free(fname);
  if(layerInfoFile == NULL)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not open %s", fname);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  int32_t i;
  int64_t modelSize = tidl_getFileSize(params->outputNetFile);
  if(modelSize == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  sTIDL_Network_t * tidlNet = (sTIDL_Network_t *)malloc(modelSize);
  if (tidlNet == NULL)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not allocate memory for model read");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  fp = fopen((const char *)params->outputNetFile, "rb");
  if (fp)
  {
    fread(tidlNet, 1, modelSize, fp);
    fclose(fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not open %s", params->outputNetFile);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  for (i = 0; i < tidlNet->numLayers; i++)
  {
    if(tidlNet->inferenceMode != TIDL_inferenceModeLowLatency)/*single core inference*/
    {
      fprintf(layerInfoFile, "%d %d %s \n", i, tidlNet->TIDLLayers[i].outData.dataId,
        TIDL_getOutDataName(&orgTIDLNetStructure, tidlNet->TIDLLayers[i].outData.dataId));
    }
    else
    {
      fprintf(layerInfoFile, "%d %d %d %s \n", i, tidlNet->TIDLLayers[i].outData.dataId, tidlNet->TIDLLayers[i].multiCoreMode,
        TIDL_getOutDataName(&orgTIDLNetStructure, tidlNet->TIDLLayers[i].outData.dataId));
    }
  }
  free(tidlNet);
  fclose(layerInfoFile);
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** This function removes the string "tidl_net_" and ".bin" from the net bin file names to reduce the folder name size for perfsim
 * NOTE : THIS FUNCTION MUST BE IN SYNC WITH THE FUNCTION WITH SAME NAME IN PERFSIM
 */
std::vector<string> TIDL_shortenFolderNameForPerfSim(char * dirName)
{
  string netFilePath = dirName;
#ifdef _WIN32
    string netFileName = netFilePath.substr(netFilePath.find_last_of("\\/") + 1);
    string netDirName = netFilePath.substr(0, netFilePath.find_last_of("\\/"));
#else
    string netFileName = netFilePath.substr(netFilePath.find_last_of("/") + 1);
    string netDirName = netFilePath.substr(0, netFilePath.find_last_of("/"));
#endif

  if(netFileName.find("tidl_net_") != std::string::npos) netFileName.replace(netFileName.find("tidl_net_"), 9, "");
  if(netFileName.find(".bin") != std::string::npos) netFileName.replace(netFileName.find(".bin"), 4, "");
  std::vector<string> netInfo;
  netInfo.push_back(netDirName);
  netInfo.push_back(netFileName);
  return netInfo;
}

/** This function calculates network level performance
 *  by adding level performance
 * */
float TIDL_getPerfsimPerformance(sPerfSim_t * perfSimInfo)
{
  double perfTotal = 0.0;
  for(int i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    perfTotal += perfSimInfo->procTime[i];
  }
  return (float)(perfTotal);
}

/* Function to copy a file without its properties */
int32_t TIDL_copyFileWithoutPermissions(const char* sourceFile, const char* destinationFile)
{
  std::ifstream src(sourceFile, std::ios::binary); // Open source file in binary mode
  if(!src.is_open())
  {
    TIDL_GLOBAL_REPORT_ERROR("Failed to open file %s", sourceFile);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  std::ofstream dest(destinationFile, std::ios::binary); // Open destination file in binary mode
  if(!dest.is_open())
  {
    src.close();
    TIDL_GLOBAL_REPORT_ERROR("Failed to create file %s", destinationFile);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  /* Copy contents from source to destination */
  dest << src.rdbuf(); 

  src.close();
  dest.close();

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

float tidlRunPerfSimTool(tidl_import_config * params, int32_t NCExecMode)
{
  FILE * fp;
  char sysCommand[500];
  char orgPath[500];
  char absPath[500];
  char fileName[500];
  char psFileName[500];
  char dirName[500];
  int systemReturnCode;

  sPerfSim_t * perSimInfo = (sPerfSim_t *)malloc(sizeof(sPerfSim_t));
  sPerfSim_t * orderedPerSimInfo = (sPerfSim_t *)malloc(sizeof(sPerfSim_t));

  int i, j, k, l, foundInData;
  int32_t currLayersGroupId = 1;

  fp = fopen((const char *)params->perfSimConfig, "r");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Couldn't open perfSimConfig file: %s. Skipping Performance Simulation.", params->perfSimConfig);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  fclose(fp);

  fp = fopen((const char *)params->perfSimTool, "r");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Couldn't open perfSimTool file: %s. Skipping Performance Simulation.", params->perfSimTool);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  fclose(fp);


  getAbsPath((char *)params->perfSimConfig, absPath);

  getAbsPath((char *)params->outputNetFile, dirName);
  getDirFromPath(dirName);

#ifdef _WIN32
  sprintf(psFileName, "%s\\%s.perf_sim_config.txt", dirName, getFileNameFromPath(inConfigFilename));
#else
  sprintf(psFileName, "%s/%s.perf_sim_config.txt", dirName, getFileNameFromPath(inConfigFilename));
#endif
  systemReturnCode = TIDL_copyFileWithoutPermissions(absPath, psFileName);

  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_WARNING("System command failed with return code : %d. Skipping Performance Simulation.", systemReturnCode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }

  fp = fopen(psFileName, "a+");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not open config file %s. Skipping Performance Simulation.", psFileName);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  getAbsPath((char *)params->outputNetFile, absPath);
  fprintf(fp, "FILENAME_NET     = %s\n", absPath);
  fprintf(fp, "FILEFORMAT_NET     = -1\n");
  if(strcmp((char *)params->fileNameGrpInfo,"") != 0)
  {
    getAbsPath((char *)params->fileNameGrpInfo, absPath);
    fprintf(fp, "FILENAME_GRPINFO     = %s\n", absPath);
  }

  if (params->msmcSizeKB > 0 )
  {
    fprintf(fp, "MSMCSIZE_KB     = %d\n", params->msmcSizeKB);
  }
  if (params->deviceName != -1 )
  {
    fprintf(fp, "DEVICE_NAME     = %d\n", params->deviceName);
  }
  if (params->numCores != 1 )
  {
    fprintf(fp, "NUMCORES     = %d\n", params->numCores);
  }
  if (params->packetizeMode != -1 )
  {
    fprintf(fp, "PACKETIZE_MODE     = %d\n", params->packetizeMode);
  }

#ifdef _WIN32
  fprintf(fp, "OUTPUT_DIR     = %s\\%s\n", dirName,getFileNameFromPath((char *)params->outputNetFile));
#else
  fprintf(fp, "OUTPUT_DIR     = %s/%s\n", dirName,getFileNameFromPath((char *)params->outputNetFile));
#endif

  if ((params->numParamBits > 8) || (params->numFeatureBits > 8))
  {
	  fprintf(fp, "DATATYPE     =   1\n");
  }
  if(params->ddrLayers[0] != -1) {
      int32_t *ddrLayers = &params->ddrLayers[0];

	  fprintf(fp, "DDRLAYERS    = ");
      while(*ddrLayers >= 0) {
          fprintf(fp, "%d ", *ddrLayers);
          ddrLayers++;
      }
	  fprintf(fp, "-1\n");
  }
  fclose(fp);

  strcpy(orgPath, psFileName);
  getAbsPath(orgPath, absPath);

  strcpy(fileName, getFileNameFromPath((char *)params->perfSimTool));

  getAbsPath((char *)params->perfSimTool, dirName);
  getDirFromPath(dirName);

  /* Add default compile  constraint incase user gives something else from the config file */
  params->compileConstraintsFlag |= DEFAULT_COMPILE_CONSTRAINT_NC_FLAGS;
  if(NCExecMode == GET_PAD_INFO)
  {
    /*
    * (1 << 12) corresponds to macro defined in perfsim/src/layerinfo.h
    *  TIDL_CONSTRAINT_EXECMODE_GETPAD           (0x00001000)
    */
    params->compileConstraintsFlag |= (1 << 12) ;
  }
  else if(NCExecMode == NORMAL_EXEC)
  {
    params->compileConstraintsFlag &= ~(1 << 12) ;
  }
  //:TODO: Handle this with int instead of string
  params->compileConstraintsFlag |= 0x800; /* This is equivalent of TIDL_CONSTRAINT_FORCE_GROUP_SPLIT_WL*/
  /* Available in Firmware from 09_02_08_00 (>09_02_07_00) */
  if(TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"09_02_07_00"))
  {
    params->compileConstraintsFlag &= (~0x800); /* This is equivalent of TIDL_CONSTRAINT_FORCE_GROUP_SPLIT_WL*/
  }

#ifdef _WIN32
  sprintf(sysCommand, "cd %s && %s %s %d %d 2", dirName, fileName, absPath, params->compileConstraintsFlag, params->reserved[0]);
#else
  sprintf(sysCommand, "cd %s && ./%s %s %s %d", dirName, fileName, absPath, (char*)params->ncTempInfoDir, params->compileConstraintsFlag);
#endif
  if(gParams.debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s", sysCommand);
  }
  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Memory Planning Started]")
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
  systemReturnCode = system(sysCommand);
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Network Compiler failed to execute - Memory planning failed with return code - %d", systemReturnCode);
  }
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Memory Planning Completed]")
  getAbsPath((char *)params->outputNetFile, dirName);

  vector<string> netInfo = TIDL_shortenFolderNameForPerfSim(dirName);
  string netDirName = netInfo[0];
  string netFileName = netInfo[1];
#ifdef _WIN32
  string perfSimInfoFile = netDirName + "\\" + netFileName + "\\perfSimInfo.bin";
#else
  string perfSimInfoFile = netDirName + "/" + netFileName + "/perfSimInfo.bin";
#endif
  fp = fopen(perfSimInfoFile.c_str(), "rb");
  if (fp)
  {
    fread(perSimInfo, 1, sizeof(sPerfSim_t), fp);
    fclose(fp);

#if 0 //Why to remove the bin file generated, its useful for dump comparison
#ifdef _WIN32
  sprintf(sysCommand, "del %s", netDirName.c_str());
#else
  sprintf(sysCommand, "rm %s", netDirName.c_str());
#endif
  system(sysCommand);
#endif

  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open %s", perfSimInfoFile.c_str());
    return 0;
  }
  memcpy(orderedPerSimInfo, perSimInfo, sizeof(sPerfSim_t));

  float networkPerf = TIDL_getPerfsimPerformance(perSimInfo);

  int64_t modelSize = orgTIDLNetStructure.modelSizeWithMaxNC; /* Allocate max possible size of network - later we write part of this to net.bin */
  sTIDL_Network_t * tidlNet = (sTIDL_Network_t *)malloc(modelSize);
  if(tidlNet == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not allocate memory for model read");
    return 0;
  }
  fp = fopen((const char *)params->outputNetFile, "rb");
  if (fp)
  {
    fread(tidlNet, 1, modelSize, fp);
    fclose(fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not open %s", params->outputNetFile);
    return 0;
  }

  tidlNet->deviceName = perSimInfo->simConfig.deviceName; //Copying over the deviceName into the n/w structure
  tidlNet->preEmptWorstCaseDelay = (float32_tidl)perSimInfo->preEmptWorstCaseDelay; //Copying preEmptWorstCaseDelay into the n/w structure
  tidlNet->modelGroupId = params->modelGroupId ;
  tidlNet->inferenceMode = params->inferenceMode;
  tidlNet->numCores = params->numCores;

  /* Reduction of dbPayLoad[MAX_DB_COMMON_SIZE] which located at the end of the sGCCommonDataBase_t which is located at the end of sGraphCompilerOutArgs_t*/
  /* Decrease the size of TIDL_NET_GC_BUF in ALG_PERSIST_DDR_NET_MEMREC */
  sGCCommonDataBase_t *commonDataBase = (sGCCommonDataBase_t *)getGCDataBase(&orderedPerSimInfo->graphCompOut,DATABASE_COMMON);
  int32_t size = sizeof(orderedPerSimInfo->graphCompOut) - MAX_GC_DB_SIZE + orderedPerSimInfo->graphCompOut.totalDataBaseSize;
  tidlNet->netBufInfo[TIDL_NET_GC_BUF].size = size ;
  /* Cannot use modelSize = tidl_getFileSize(params->outputNetFile) as is for below calculation of updated optimized model size,
      since it would get updated/optimized with each NC run (creating issues when NC is run multiple times)
      Need to use modelSizeWithMaxNC instead */
  modelSize = orgTIDLNetStructure.modelSizeWithMaxNC - sizeof(orderedPerSimInfo->graphCompOut) + tidlNet->netBufInfo[TIDL_NET_GC_BUF].size;

  /* Perfsim has run in this function above, so GC info is available. GC info is not available in stats collection flow
  or cases where import is run with executeNetworkCompiler = 0 option. In these cases, isGCInfoAvailable = 0 */
  tidlNet->isGCInfoAvailable = 1 ;

  memcpy((uint8_t*)tidlNet + tidlNet->netBufInfo[TIDL_NET_GC_BUF].offset , &orderedPerSimInfo->graphCompOut,
    tidlNet->netBufInfo[TIDL_NET_GC_BUF].size);

  int32_t currAlgLayer = 0;
  int32_t wlRepeatIter;
  sWorkloadUnit_t  *workloadUnit = NULL;
  sGetLayerIdContext_t getLayerIdContext;
  int32_t layerIdxCount = 1;
  sGraphCompilerOutArgs_t *gcOutArgs = NULL;
  sGraphCompilerOutArgsCore_t *gcOut = NULL;
  sGCCommonDataBase_t *commonDatabase = NULL;
  sGCDataBase_t           *gcDataBase = NULL;
  sWorkloadSuperGroup_t *wlSuperGrp = NULL;
  int32_t visited[TIDL_NUM_MAX_LAYERS] = {0};
  sGCHelperHandle           *gcHelperHandle = NULL;
  sGCHelperHandle            gcHelperHandleObj;

  if ( tidlNet->netBufInfo[TIDL_NET_GC_BUF].offset != 0 )
  {
    gcOutArgs  = (sGraphCompilerOutArgs_t *)get_int8_t_pointer((int8_t *)(tidlNet), tidlNet->netBufInfo[TIDL_NET_GC_BUF].offset);
    commonDatabase = (sGCCommonDataBase_t *)getGCDataBase(gcOutArgs,DATABASE_COMMON);
    gcOut = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs , DATABASE_GCOUT_1);
    gcDataBase = &gcOut->dataBase;

    sBufParams_t* bufNodesDB = (sBufParams_t*) &commonDatabase->dbPayLoad[commonDatabase->dbParams[DB_COMMON_BUF].startOffset];
    sWorkloadUnit_t* workLoadUnitDB = (sWorkloadUnit_t*) &gcDataBase->dbPayLoad[gcDataBase->dbParams[DB_WL].startOffset];
    // sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) gcDataBase->dbPayLoad[gcDataBase->dbParams[DB_BUF].size +
    //                                                                       gcDataBase->dbParams[DB_WL].size];
    sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) &gcDataBase->dbPayLoad[gcDataBase->dbParams[DB_AUXWL].startOffset];

    gcHelperHandle = &gcHelperHandleObj;
    gcOutArgs->commonDatabaseInfo.bufDB.pBufList = bufNodesDB;
    gcOut->dataBaseInfo.wlDB.pWLList = workLoadUnitDB;
    gcOut->dataBaseInfo.wlAuxDB.pWLAuxList = auxilaryWorkloadDB;
    (void)initGCHelperHandle(gcHelperHandle, tidlNet, &gcOut->dataBaseInfo, &gcOutArgs->commonDatabaseInfo);

    wlSuperGrp = &gcOut->superWorkload;
  }

  for (i = 0; i < tidlNet->numLayers; i++)
  {
    if(tidlNet->TIDLLayers[i].layerType == TIDL_DataLayer && tidlNet->TIDLLayers[i].numOutBufs > 0)
    {
      int32_t nextLyrIdx = tidl_getOutLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[i].outData.dataId);
      if(nextLyrIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
      }
      sBufParams_t *bufParams;
      /*we get the outbufParams of the Data-Layer*/
      bufParams = TIDL_getOutBufParams(tidlNet, i, tidlNet->TIDLLayers[i].layersGroupId);
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_LINE_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH] + bufParams->padParams.cols;
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] = bufParams->bufWidth;
      /* Batches are expected to be arranged along the channel axis. So the next batch is expected to start one channel pitch after the last channel of the previous batch */
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH];
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
      tidlNet->TIDLLayers[i].outData.padW = bufParams->padParams.cols;
      tidlNet->TIDLLayers[i].outData.padH = bufParams->padParams.rows;

      if((tidlNet->TIDLLayers[i].layerType == TIDL_DataLayer) && (tidlNet->TIDLLayers[nextLyrIdx].layerType == TIDL_DataConvertLayer) &&
        (tidlNet->TIDLLayers[nextLyrIdx].layerParams.dataConvertParams.layout == TIDL_LT_NHWC))
      {
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH] = tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] * tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
      }
    }
  }

  getLayerIdInit(&getLayerIdContext,
                    0,
                    -1,
                    tidlNet,
                    gcHelperHandle,
                    wlSuperGrp);

  while(1)
  {

    i = getLayerIdToExecute(&getLayerIdContext,
                                     0,
                                     &wlRepeatIter,
                                     &currAlgLayer,
                                     &workloadUnit);

    if ( i == NOT_VALID)
    {
      break;
    }

    int32_t isnextLyrReshape = 0;
    int32_t nextLyrIdx = tidl_getOutLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[i].outData.dataId);
    if(nextLyrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        isnextLyrReshape = (tidlNet->TIDLLayers[nextLyrIdx].layerType == TIDL_ReshapeLayer) && (tidlNet->TIDLLayers[i].layerType != TIDL_DataConvertLayer);

    if( visited[tidlNet->TIDLLayers[i].outData.dataId] != 1 && tidlNet->TIDLLayers[i].layerType != TIDL_DataLayer)
    {
      /* Batch processing exit conditions for Large feature map and ST cases */
      if (tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_BATCH] > 1 && params->batchMode == 1 && TIDL_doesLayerSupportBatchStitching(&orgTIDLNetStructure.TIDLPCLayers[i]) )
      {
        int32_t numSplits = getNumSplits((const void *)workloadUnit);
        if (numSplits > 1)
        {
          TIDL_GLOBAL_REPORT_ERROR("Batch size of %d is not supported for your network configuration, please reduce the batch size. Aborting", params->numBatches[0]);
          return NAN;
        }
      }

      //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%2d : | %d %d | %d %d |", i, tidlNet->TIDLLayers[i].outData.padW, perSimInfo->sdataFlowInfo[i].bufInfo[OUT_FEAT_MAP][WRITE].padC,
      if(tidlNet->TIDLLayers[i].numOutBufs > 0)
      {
        sLink_t *linkPtrList[MAX_LINKS_PER_WL];
        sLink_t *link, *procLink=NULL, *outputDmaLink = NULL, *inputDmaLink=NULL;
        sBufParams_t *bufParams;
        for (int32_t linkIdx = 0; linkIdx < workloadUnit->numLinks; linkIdx++)
        {
          getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
          link = linkPtrList[0];
          if(link->type == LINK_PROC)
          {
            getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
            procLink = linkPtrList[0];
          }
          if(link->subType == LINK_X_FM_OUT_PART_TO_FULL)
          {
            getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
            outputDmaLink = linkPtrList[0];
          }
          if(link->subType == LINK_X_FM_IN_FULL_TO_PART)
          {
            getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
            inputDmaLink = linkPtrList[0];
          }
        }

        if(outputDmaLink == NULL && procLink == NULL)
        {
          bufParams = getBufParamsFromBufIndex(gcHelperHandle, inputDmaLink->sink.bufDBindex);
        }
        else if(outputDmaLink == NULL)
        {
          bufParams = getBufParamsFromBufIndex(gcHelperHandle, procLink->sink.bufDBindex);
        }
        else
        {
          bufParams = getBufParamsFromBufIndex(gcHelperHandle, outputDmaLink->sink.bufDBindex);
        }

        tidlNet->TIDLLayers[i].outData.pitch[TIDL_LINE_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH] + bufParams->padParams.cols;
        tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] = bufParams->bufWidth;
        /* Batches are expected to be arranged along the channel axis. So the next batch is expected to start one channel pitch after the last channel of the previous batch */
        tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH];
        tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
        tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
        tidlNet->TIDLLayers[i].outData.padW = bufParams->padParams.cols;
        tidlNet->TIDLLayers[i].outData.padH = bufParams->padParams.rows;

        if(tidlNet->TIDLLayers[i].layerType == TIDL_GridSampleLayer){
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH] * tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
        }

        if(tidlNet->TIDLLayers[i].layerType == TIDL_ReshapeLayer)
        {
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH] * tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
        }
        if(tidlNet->TIDLLayers[i].layerType == TIDL_DataConvertLayer)
        {
          if (tidlNet->TIDLLayers[i].layerParams.dataConvertParams.layout == TIDL_LT_NCHW && tidlNet->TIDLLayers[i].layerParams.dataConvertParams.outLayout == TIDL_LT_NHWC)
          {
            tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH] = tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] * tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT];
            tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
            tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
          }
        }

        if((tidlNet->TIDLLayers[i].layerType == TIDL_PoolingLayer) && (tidlNet->TIDLLayers[i].layerParams.poolParams.kernelW == 0))
        {
          /* The TIDL_LINE_PITCH updated above should be restored back incase of golbalAvg pooling layer is the last layer in the network with pad > 0 */
          if (tidltb_isOutDataBuff(tidlNet, tidlNet->TIDLLayers[i].outData.dataId,currLayersGroupId))
          {
            tidlNet->TIDLLayers[i].outData.pitch[TIDL_LINE_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH];
          }
          /* If pad=0, we treat the output as flattened, i.e, as 1x1x1xC instead of 1xCx1x1 */
          if ((tidlNet->TIDLLayers[i].outData.padW == 0) && (tidlNet->TIDLLayers[i].outData.padH == 0))
          {
            tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH];
          }
          tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]  = tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH]*tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH];
        }
        else if(tidlNet->TIDLLayers[i].layerType == TIDL_InnerProductLayer)
        {
          // tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] =  tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_LINE_PITCH];

          // tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH] =  tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH];
        }

        // if(isnextLyrReshape)
        // {
        //   tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] = tidlNet->TIDLLayers[i].outData.pitch[TIDL_LINE_PITCH] * tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT];
        //   tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH]  = tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH]*tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH];
        //   tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
        //   tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
        // }

      }
      visited[tidlNet->TIDLLayers[i].outData.dataId] = 1;
    }
  }

  uint32_t alignedModelSize = modelSize;


#if 0
  /* We don't want to copy dataFlowInfo for MAX_LAYERS but we still want to
  copy other output in sPerfSim_t hence find the size of only relevant information
  needed during TIDL inference. The assumption is nothing beyond sdataFlowInfo will
  be used by TIDL */
  sBuffer_t buf;
  sBuffer_t bufGc;
  buf.ptr = (void*)orderedPerSimInfo;
  buf.bufSize = sizeof(sPerfSim_t ) -
                    sizeof(orderedPerSimInfo->layerExecutionOrder) -
                    sizeof(orderedPerSimInfo->sdataFlowInfo) -
                    sizeof(orderedPerSimInfo->graphCompOut) +
                    (sizeof(sDataFlowInfo_t) * tidlNet->numLayers);

  tidlNet->dataFlowInfo = TIDL_alignParamsWrite(NULL, &buf, &alignedModelSize, 1);
  tidlNet->netBufInfo[TIDL_NET_COMPILER_BUF].offset  = tidlNet->dataFlowInfo;
  tidlNet->netBufInfo[TIDL_NET_COMPILER_BUF].size = buf.bufSize ;

  //:TODO:Currently write the complete graphCompOut but eventually should limit to current number of cores and number of actual layers
  bufGc.bufSize = sizeof(orderedPerSimInfo->graphCompOut);
  bufGc.ptr = (void *)&orderedPerSimInfo->graphCompOut;
  tidlNet->isGCInfoAvailable = TIDL_alignParamsWrite(NULL, &bufGc, &alignedModelSize, 1);
  tidlNet->netBufInfo[TIDL_NET_GC_BUF].offset  = tidlNet->graphCompOut;
  tidlNet->netBufInfo[TIDL_NET_GC_BUF].size = bufGc.bufSize ;

  fp = fopen((const char *)params->outputNetFile, "wb+");
  if (fp)
  {
    fwrite(tidlNet, 1, modelSize, fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open %s", params->outputNetFile);
    return 0;
  }
  alignedModelSize = modelSize;
  TIDL_alignParamsWrite(fp, &buf, &alignedModelSize, 1);
  TIDL_alignParamsWrite(fp, &bufGc, &alignedModelSize, 1);
#else

  tidlNet->dataFlowInfo = tidlNet->netBufInfo[TIDL_NET_COMPILER_BUF].offset ;
  memcpy((uint8_t*)tidlNet + tidlNet->dataFlowInfo , orderedPerSimInfo,
    tidlNet->netBufInfo[TIDL_NET_COMPILER_BUF].size);

  fp = fopen((const char *)params->outputNetFile, "wb+");
  if (fp)
  {
    fwrite(tidlNet, 1, modelSize, fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not open %s", params->outputNetFile);
    return 0;
  }

#endif

  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_writeInfo(tidlNet,
                        &orgTIDLNetStructure,
                        (const char *)params->outputParamsFile,
                        tidlNet->numLayers,
                        1,
                        orderedPerSimInfo,
                        &gParams), "");

  if(gParams.debugTraceLevel > 0){
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Network buffer segments:");
    for(int32_t idx = 0 ; idx < TIDL_NET_TOTAL_BUF ; idx++)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "buf(%d), offset(%8d), size(%8d)", idx,
        tidlNet->netBufInfo[idx].offset, tidlNet->netBufInfo[idx].size);
    }
  }

  for (i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    if( (perSimInfo->checkErrorFlag[i] == ERR_FLAG_OTF_NOT_SUPPORTED) && ((gParams.deviceName & 0xFF) == TIDL_AM62A))
    {
      TIDL_GLOBAL_REPORT_ERROR("Layer - %s - is not supported on accelerator due to internal memory allocation constraints in this particular scenario. Please re-try compilation using open source runtimes offering and specify deny_list:layer_name:%s as part of compilation options",
                  (char *)orgTIDLNetStructure.TIDLPCLayers[i].outDataNames[0], (char *)orgTIDLNetStructure.TIDLPCLayers[i].outDataNames[0]);
    }
    else if( (perSimInfo->checkErrorFlag[i]  == ERR_FLAG_OTF_NOT_SUPPORTED) && ((gParams.deviceName & 0xFF) == TIDL_TDA4AEP || (gParams.deviceName & 0xFF) == TIDL_TDA4AMPlus) )
    {
      gParams.enableOtfPad = 0;
      break;
    }
    else if((perSimInfo->checkErrorFlag[i] == ERR_FLAG_SPATIAL_JOIN_NOT_SUPPORTED))
    {
      orgTIDLNetStructure.TIDLPCLayers[i].multiCoreMode = TIDL_MULTI_CORE_SPATIAL_JOIN_WITH_CROP;
    }
  }

  if ( orderedPerSimInfo != NULL )
  {
    free(orderedPerSimInfo);
  }
  free(perSimInfo);
  free(tidlNet);
  fclose(fp);

  if(gParams.enableOtfPad == 0 && gParams.inferenceMode == TIDL_inferenceModeLowLatency && gParams.numCores > 1)
  {
    TIDL_GLOBAL_REPORT_ERROR("%s", "This network does not support OTF Padding, hence cannot be run with Inference Mode 2. Run with Inference Mode: 0") ;
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  return networkPerf;

}

void tidlAddPersistentData(tidl_import_config * params)
{
  FILE * fp;
  char sysCommand[500];
  char orgPath[500];
  char absPath[500];
  char fileName[500];
  char qsFileName[500];
  char dirName[500];
  char outDirName[500];
  int systemReturnCode;

  fp = fopen((const char *)params->tidlStatsTool, "r");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Couldn't open tidlStatsTool file: %s. Skipping persistant data addition.", params->tidlStatsTool);
    return;
  }
  fclose(fp);

  printf("\n~~~~~Running TIDL in Target Flow mode to dump persistent data~~~~~\n");

  getAbsPath((char *)params->outputNetFile, outDirName);
  getDirFromPath(outDirName);

  sprintf(qsFileName, "%s/%s.target_config.txt", outDirName,getFileNameFromPath(inConfigFilename));
  fp = fopen(qsFileName, "w+");
  if(fp== NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not open config file: %s. Skipping persistant data addition.", qsFileName);
    return;
  }
  tidlWriteInferenceConfig(fp, params);
  fprintf(fp, "flowCtrl               = %d\n", 0);
  fprintf(fp, "initOnly           = %d\n", 1);
  fprintf(fp, "dumpNetInitBackupData = %d\n", 1);

  fclose(fp);

  strcpy(orgPath, qsFileName);
  getAbsPath(orgPath, absPath);

  strcpy(fileName, getFileNameFromPath((char *)params->tidlStatsTool));

  getAbsPath((char *)params->tidlStatsTool, dirName);
  getDirFromPath(dirName);

#ifdef _WIN32
  sprintf(sysCommand, "cd %s && %s s:%s", dirName, fileName, absPath);
#else
  sprintf(sysCommand, "cd %s && ./%s s:%s", dirName, fileName, absPath);
#endif
  if(params->debugTraceLevel > 0)
  {
    printf("%s\n", sysCommand);
  }

  systemReturnCode = system(sysCommand);
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_WARNING("System command failed with return code : %d. Skipping persistant data addition.", systemReturnCode);
    return;
  }

  int64_t modelSize = tidl_getFileSize(params->outputNetFile);
  int64_t persistantDataSize[TIDL_MAX_NUM_CORES] = {0};
  modelSize = ALIGN(modelSize, ALIGN_NET_DATA);

  sTIDL_Network_t * tidlNet = (sTIDL_Network_t *)malloc(modelSize);
  if(tidlNet == NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not Allocate memory for model read. Skipping persistant data addition.");
    return;
  }
  fp = fopen((const char *)params->outputNetFile, "rb");
  if (fp)
  {
    fread(tidlNet, 1, modelSize, fp);
    fclose(fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not open: %s. Skipping persistant data addition.", params->outputNetFile);
    return;
  }

  int64_t modelSizeOut = modelSize - (tidlNet->netBufInfo[TIDL_NET_GC_BUF].offset - tidl_reorderedParamsOffset);

  for (int i = 0; i < params->numCores; i++)
  {
    sprintf(fileName, "%s/%s_persistent_data_%d", outDirName, getFileNameFromPath(inConfigFilename), i);
    persistantDataSize[i] = tidl_getFileSize((uint8_t *)fileName);
    if (persistantDataSize[i] == -1)
    {
      TIDL_GLOBAL_REPORT_WARNING("Persistent data dumps not found for core %d. Skipping persistant data addition.", i);
      return;
    }
    persistantDataSize[i] = ALIGN(persistantDataSize[i], ALIGN_NET_DATA);
    modelSizeOut += persistantDataSize[i];
  }

  sTIDL_Network_t * tidlNetOut = (sTIDL_Network_t *)malloc(modelSizeOut);

  memcpy(tidlNetOut, tidlNet, tidl_reorderedParamsOffset);
  memcpy((void *)((uint8_t *)tidlNetOut + tidl_reorderedParamsOffset),
         (void *)((uint8_t *)tidlNet + tidlNet->netBufInfo[TIDL_NET_GC_BUF].offset),
         tidlNet->netBufInfo[TIDL_NET_GC_BUF].size);

  tidlNetOut->netBufInfo[TIDL_NET_GC_BUF].offset = tidl_reorderedParamsOffset;
  tidlNetOut->originalGcBufInfoOffset = tidl_reorderedParamsOffset;
  tidlNetOut->netBufInfo[TIDL_NET_PARAMS_BUF].size = tidl_reorderedParamsOffset - tidlNetOut->netBufInfo[TIDL_NET_PARAMS_BUF].offset;
  int64_t offset = tidl_reorderedParamsOffset + tidlNet->netBufInfo[TIDL_NET_GC_BUF].size;

  for (int i = 0; i < params->numCores; i++)
  {
    sprintf(fileName, "%s/%s_persistent_data_%d", outDirName, getFileNameFromPath(inConfigFilename), i);
    fp = fopen(fileName, "rb");
    if (fp)
    {
      fread((uint8_t *)tidlNetOut + offset, 1, persistantDataSize[i], fp);
      fclose(fp);
    }
    tidlNetOut->netInitBackupDataOffset[i] = offset;
    offset += persistantDataSize[i];
  }

  fp = fopen((const char *)params->outputNetFile, "wb+");
  if (fp)
  {
    fwrite(tidlNetOut, 1, modelSizeOut, fp);
    fclose(fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not open: %s", params->outputNetFile);
    return;
  }

  free(tidlNet);
  free(tidlNetOut);

  return;
}

/* Copy tensor stats from float model after STATS_COLLECTION_FLOAT calibration
 *                   to   quantized model
 * skip_input_datalayers: we don't want to update the input DataLayers
 */
void TIDL_copyTensorStats(sTIDL_OrgNetwork_t * pOrgTIDLNetStructureDst,
                          sTIDL_OrgNetwork_t * pOrgTIDLNetStructureSrc,
                          int32_t skip_input_datalayers)
{
  int32_t i, layerIdx;
  pOrgTIDLNetStructureDst->quantStats = pOrgTIDLNetStructureSrc->quantStats;
  for ( layerIdx = 0; layerIdx <pOrgTIDLNetStructureSrc->numLayers; layerIdx++)
  {
    sTIDL_LayerPC_t& srcLayer = pOrgTIDLNetStructureSrc->TIDLPCLayers[layerIdx];
    sTIDL_LayerPC_t& dstLayer = pOrgTIDLNetStructureDst->TIDLPCLayers[layerIdx];
    int32_t dataConvertInput, dataConvertOutput;
    if (skip_input_datalayers && srcLayer.layerType == TIDL_DataLayer &&
                                 srcLayer.numInBufs == -1)
      continue;

    dataConvertInput = 0;
    dataConvertOutput = 0;

    if ( srcLayer.layerType == TIDL_DataConvertLayer )
    {
      const sTIDL_dataConvertParams_t *params = &dstLayer.layerParams.dataConvertParams;
      if ( params->type == TIDL_DC_TYPE_INPUT )
      {
        dataConvertInput = 1;
      }
      else
      {
        dataConvertOutput = 1;
      }

    }

    for ( i = 0; i < srcLayer.numInBufs; i++)
    {
      dstLayer.inData[i].maxTensorValue = srcLayer.inData[i].maxTensorValue;
      dstLayer.inData[i].minTensorValue = srcLayer.inData[i].minTensorValue;
      if ( dataConvertInput != 1 )
      {
        /* Do not overwrite the tensorScale if its user given i.e. when its input data convert layer */
        dstLayer.inData[i].tensorScale    = srcLayer.inData[i].tensorScale;
        dstLayer.inData[i].tensorZeroPoint = srcLayer.inData[i].tensorZeroPoint;
      }
      dstLayer.inData[i].roundBits      = srcLayer.inData[i].roundBits;

    }

    for ( i = 0; i < srcLayer.numOutBufs; i++)
    {
      dstLayer.outData[i].maxTensorValue = srcLayer.outData[i].maxTensorValue;
      dstLayer.outData[i].minTensorValue = srcLayer.outData[i].minTensorValue;
      dstLayer.outData[i].roundBits      = srcLayer.outData[i].roundBits;
      if ( dataConvertOutput != 1 && srcLayer.quantizeConstraint != TIDL_QuantizeConstrained)
      {
        /* Do not overwrite the tensorScale if its user given i.e. when its output data convert layer */
        dstLayer.outData[i].tensorScale    = srcLayer.outData[i].tensorScale;
        dstLayer.outData[i].tensorZeroPoint = srcLayer.outData[i].tensorZeroPoint;
      }
    }
  }
}
int32_t TIDL_readQuantStats(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure, tidl_import_config * params, uint32_t numLayers)
{
  int32_t layerIdx;
  int32_t idx;

  FILE  * fp;
  int32_t i;
  int32_t isQuantStatsAvail = 0;
  int64_t modelSize = tidl_getFileSize(params->outputNetFile);
  if(modelSize == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  sTIDL_Network_t * tidlNet = (sTIDL_Network_t *)malloc(modelSize);
  if (tidlNet == NULL)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not allocate memory for model read");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  fp = fopen((const char *)params->outputNetFile, "rb");
  if (fp)
  {
    fread(tidlNet, 1, modelSize, fp);
    fclose(fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Could not open %s", params->outputNetFile);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  pOrgTIDLNetStructure->quantStats = TIDL_QUANT_STATS_NONE;

  if ( tidlNet->isQuantStatsAvailable == 1 )
  {
    pOrgTIDLNetStructure->quantStats = TIDL_QUANT_STATS_FIXED;
    if (tidlNet->weightsElementSize == 4 )
    {
      pOrgTIDLNetStructure->quantStats = TIDL_QUANT_STATS_FLOAT;
    }

    for ( layerIdx = 0; layerIdx < numLayers; layerIdx++)
    {
      for ( idx = 0; idx < tidlNet->TIDLLayers[layerIdx].numInBufs; idx++)
      {
        const sTIDL_DataParams_t * inDataParms =  TIDL_getOutDataInNet(tidlNet, tidlNet->TIDLLayers[layerIdx].inData[idx]);
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[idx].maxTensorValue =
            inDataParms->maxTensorValue;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[idx].minTensorValue=
            inDataParms->minTensorValue;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[idx].tensorScale =
            inDataParms->tensorScale;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[idx].tensorZeroPoint =
            inDataParms->tensorZeroPoint;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].inData[idx].roundBits =
            inDataParms->roundBits;
        if (isinf(inDataParms->minTensorValue) || isinf(inDataParms->maxTensorValue) || abs(inDataParms->tensorScale) <= 0.00001)
        {
          TIDL_GLOBAL_REPORT_WARNING("Layer %d: Invalid tensor stats: inData[%d].min/maxTensorValue is inf or inData[%d].tensorScale is close to zero. Quantization may produce unstable results.", layerIdx, idx);
        }
      }

      for ( idx = 0; idx < tidlNet->TIDLLayers[layerIdx].numOutBufs; idx++)
      {
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[idx].maxTensorValue =
            tidlNet->TIDLLayers[layerIdx].outData.maxTensorValue;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[idx].minTensorValue=
            tidlNet->TIDLLayers[layerIdx].outData.minTensorValue;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[idx].tensorScale =
            tidlNet->TIDLLayers[layerIdx].outData.tensorScale;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[idx].tensorZeroPoint =
            tidlNet->TIDLLayers[layerIdx].outData.tensorZeroPoint;
        pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outData[idx].roundBits =
            tidlNet->TIDLLayers[layerIdx].outData.roundBits;
        if (isinf(tidlNet->TIDLLayers[layerIdx].outData.minTensorValue) || isinf(tidlNet->TIDLLayers[layerIdx].outData.maxTensorValue) || abs(tidlNet->TIDLLayers[layerIdx].outData.tensorScale) <= 0.00001)
        {
          TIDL_GLOBAL_REPORT_WARNING("Layer %d: Invalid tensor stats: outData.min/maxTensorValue is inf or outData.tensorScale is close to zero. Quantization may produce unstable results.", layerIdx, idx);
        }

      }
    }
  }

  isQuantStatsAvail = tidlNet->isQuantStatsAvailable;

  if ( tidlNet != NULL )
  {
    free(tidlNet);
  }
  return (isQuantStatsAvail);
}

int tidlRunQuantStatsTool(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                                sTIDL_Network_t      *  pTIDLNetStructure,
                                                tidl_import_config * params,
                                                uint32_t numLayers)
{
  FILE * fp;
  int32_t quantStatus;

  if(!params->executeQuantsTool)
  {
    return 0;
  }
  fp = fopen((const char *)params->inData, "r");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Couldn't open inData file: %s  , Skipping Range Collection for Quantization", params->inData);
    return(0);
  }
  fclose(fp);

  fp = fopen((const char *)params->tidlStatsTool, "r");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Couldn't open tidlStatsTool file: %s", params->tidlStatsTool);
    return(0);
  }
  fclose(fp);

  if (params->numFrames == -1)
  {
    if ((params->inFileFormat == 2) || (params->inFileFormat == 5))
    {
     params->numFrames = getNumberOfLinesIntheFile((char *)params->inData);
    }
    else if ((params->inFileFormat == 0) || (params->inFileFormat == 1) || (params->inFileFormat == 3))
    {
      params->numFrames = 1;
    }
  }
  if (params->numFrames > 0)
  {
    //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n~~~~~Running TIDL in PC emulation mode to collect Activations range for each layer~~~~~\n");
    if(params->numParamBits == 32U || params->biasCalibrationIterations == 0)
    {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"");
    TIDL_GLOBAL_REPORT_SUBHEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_NoColor, "Running Calibration in Float Mode to Collect Tensor Statistics");
    }
    TIDL_IMPORT_CHECK_AND_RETURN(tidlQuantStatsTool(params), "Failed to run Calibration");
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"\n");
  }

  /* Read back the stats from the model written after quants stats tool */
 quantStatus = TIDL_readQuantStats(pOrgTIDLNetStructure, params, pOrgTIDLNetStructure->numLayers);
  if (quantStatus != -1 )
  {
    pTIDLNetStructure->isQuantStatsAvailable = quantStatus;
  }
  return 0;
}

void tidl_updateNetPitch(sTIDL_Network_t * tidlNet)
{
  int32_t i, j;
  for (i = 0; i < tidlNet->numLayers; i++)
  {
    for (j = 0; j < tidlNet->TIDLLayers[i].numOutBufs; j++)
    {
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_LINE_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_WIDTH] +
        tidlNet->TIDLLayers[i].outData.padW;
      int32_t isPadW = tidlNet->TIDLLayers[i].outData.padW ? 1 : 0;
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH] = tidlNet->TIDLLayers[i].outData.pitch[TIDL_LINE_PITCH] * (tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_HEIGHT] + tidlNet->TIDLLayers[i].outData.padH * 2 + isPadW);
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH] = tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_NUMCH] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_CHANNEL_PITCH];
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM2] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM2_PITCH];
      tidlNet->TIDLLayers[i].outData.pitch[TIDL_ROI_PITCH]= tidlNet->TIDLLayers[i].outData.dimValues[TIDL_DIM_DIM1] * tidlNet->TIDLLayers[i].outData.pitch[TIDL_DIM1_PITCH];
    }
  }
}

sTIDL_tfOutReshapeMap_t sTIDL_OutReshapeTable[] =
{
  { TIDL_DataLayer                     ,  TIDL_tfOutReshapeDataLayer },
  { TIDL_ConvolutionLayer              ,  TIDL_tfOutReshapeConvLayer },
  { TIDL_PoolingLayer                  ,  TIDL_tfOutReshapePoolingLayer },
  { TIDL_ReLULayer                     ,  TIDL_tfOutReshapeRelu },
  { TIDL_PReLULayer                    ,  TIDL_tfOutReshapePRelu },
  { TIDL_EltWiseLayer                  ,  TIDL_tfOutReshapeEltwise },
  { TIDL_InnerProductLayer             ,  TIDL_tfOutReshapeIPLayer },
  { TIDL_SoftMaxLayer                  ,  TIDL_tfOutReshapeSoftmax },
  { TIDL_BatchNormLayer                ,  TIDL_tfOutReshapeBN },
  { TIDL_BiasLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ScaleLayer                    ,  TIDL_tfOutReshapeIdentity },
  { TIDL_Deconv2DLayer                 ,  TIDL_tfOutReshapeDeConvLayer },
  { TIDL_ConcatLayer                   ,  TIDL_tfOutReshapeConcatLayer },
  { TIDL_SplitLayer                    ,  TIDL_tfOutReshapeSliceLayer },
  { TIDL_SliceLayer                    ,  TIDL_tfOutReshapeSliceLayer },
  { TIDL_CropLayer                     ,  TIDL_tfOutReshapeCropLayer },
  { TIDL_FlattenLayer                  ,  TIDL_tfOutReshapeFlattenLayer },
  { TIDL_DropOutLayer                  ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ArgOpLayer                    ,  TIDL_tfOutReshapeArgOpLayer },
  { TIDL_DetectionOutputLayer          ,  TIDL_tfOutReshapeDetOutLayer },
  { TIDL_ShuffleChannelLayer           ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ResizeLayer                   ,  TIDL_tfOutReshapeResize },
  { TIDL_RoiPoolingLayer               ,  TIDL_tfOutReshapeRoiPoolingLayer },
  { TIDL_OdPostProcessingLayer         ,  TIDL_tfOutReshapeOdPostProcessingLayer },
  { TIDL_DepthToSpaceLayer             ,  TIDL_tfOutReshapeDepthtoSpace },
  { TIDL_SigmoidLayer                  ,  TIDL_tfOutReshapeSigmoid },
  { TIDL_PadLayer                      ,  TIDL_tfOutReshapePadLayer },
  { TIDL_ColorConversionLayer          ,  TIDL_tfOutReshapeColorConversionLayer },
  { TIDL_OdOutputReformatLayer         ,  TIDL_tfOutReshapeOdOutputReformatLayer },
  { TIDL_DataConvertLayer              ,  TIDL_tfOutReshapeIdentity },
  { TIDL_CustomLayer                   ,  TIDL_tfOutReshapeCustomLayer },
  { TIDL_BatchReshapeLayer             ,  TIDL_tfOutReshapeBatchReshape },
  { TIDL_ReduceLayer                   ,  TIDL_tfOutReshapeReduceLayer },
  { TIDL_ScatterElementsLayer          ,  TIDL_tfOutReshapeScatterElementsLayer},
  { TIDL_SqueezeLayer                  ,  TIDL_tfOutReshapeSqueeze },
  { TIDL_TanhLayer                     ,  TIDL_tfOutReshapeTanh },
  { TIDL_HardSigmoidLayer              ,  TIDL_tfOutReshapeHardSigmoid },
  { TIDL_ELULayer                      ,  TIDL_tfOutReshapeELU },
  { TIDL_ReshapeLayer                  ,  TIDL_tfOutReshapeReshapeLayer },
  { TIDL_ConstDataLayer                ,  TIDL_tfOutReshapeConstDataLayer },
  { TIDL_GatherLayer                   ,  TIDL_tfOutReshapeGatherLayer},
  { TIDL_TransposeLayer                ,  TIDL_tfOutReshapeTransposeLayer },
  { TIDL_LayerNormLayer                ,  TIDL_tfOutReshapeIdentity },
  { TIDL_GridSampleLayer               ,  TIDL_tfOutReshapeGridSampleLayer },
  { TIDL_TopKLayer                     ,  TIDL_tfOutReshapeTopKLayer},
  { TIDL_DeformableConvLayer           ,  TIDL_tfOutReshapeDeformConvLayer },
  { TIDL_TileLayer                     ,  TIDL_tfOutReshapeTileLayer},
  { TIDL_LogicalOpLayer                ,  TIDL_tfOutReshapeLogicalOpLayer},
  { TIDL_RMSNormalizationLayer         ,  TIDL_tfOutReshapeIdentity },
  { TIDL_LSTMLayer                     ,  TIDL_tfOutReshapeLSTMLayer },
  { TIDL_GRULayer                      ,  TIDL_tfOutReshapeGRULayer },
  { TIDL_RNNLayer                      ,  TIDL_tfOutReshapeRNNLayer },
  { TIDL_UnsupportedLayer              ,  TIDL_tfOutReshapeIdentity },
  { TIDL_PriorBoxLayer                 ,  TIDL_tfOutReshapeIdentity },
  { TIDL_PermuteLayer                  ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ShapeLayer                    ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ClipLayer                     ,  TIDL_tfOutReshapeClip },
  { TIDL_MinimumLayer                  ,  TIDL_tfOutReshapeIdentity },
  { TIDL_LeakyReluLayer                ,  TIDL_tfOutReshapeIdentity },
  { TIDL_IdentityLayer                 ,  TIDL_tfOutReshapeIdentity },
  { TIDL_BatchToSpaceLayer             ,  TIDL_tfOutReshapeBatchtoSpace },
  { TIDL_SpaceToBatchLayer             ,  TIDL_tfOutReshapeSpacetoBatch },
  { TIDL_PackLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_DequantizeLayer               ,  TIDL_tfOutReshapeIdentity },
  { TIDL_QuantizeLayer                 ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SqrtLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ReduceMeanLayer               ,  TIDL_tfOutReshapeReduceLayer },
  { TIDL_ReduceSumLayer                ,  TIDL_tfOutReshapeReduceLayer },
  { TIDL_PowLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_DivLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SubLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_PatchMergeLayer               ,  TIDL_tfOutReshapeIdentity },
  { TIDL_CastLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_AsinLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_AsinhLayer                    ,  TIDL_tfOutReshapeIdentity },
  { TIDL_HardSwishLayer                ,  TIDL_tfOutReshapeIdentity },
  { TIDL_MishLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_LogLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_UnsqueezeLayer                ,  TIDL_tfOutReshapeUnsqueezeLayer},
  { TIDL_AbsLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_FloorLayer                    ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ExpLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SinLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ErfLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_InstanceNormLayer             ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SpaceToDepthLayer             ,  TIDL_tfOutReshapeSpacetoDepth},
  { TIDL_AcosLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_AtanLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SinhLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_CosLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_CoshLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_NegLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_TanLayer                      ,  TIDL_tfOutReshapeIdentity },
  { TIDL_ExpandLayer                   ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SwishLayer                    ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SoftPlusLayer                 ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SoftSignLayer                 ,  TIDL_tfOutReshapeIdentity },
  { TIDL_CeilLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_CeluLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SeluLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_RoundLayer                    ,  TIDL_tfOutReshapeIdentity },
  { TIDL_SignLayer                     ,  TIDL_tfOutReshapeIdentity },
  { TIDL_GroupNormLayer                ,  TIDL_tfOutReshapeIdentity },
};

int32_t tidl_updateHighResOptimization(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  if(gParams.numFeatureBits > 8)
  {
    gParams.enableHighResOptimization = 0;
  }

  return 0;
}

bool tidl_isYuvLayerPresent(void)
{
  bool isYuvLayerPresent = false;
  int32_t i;
  for(i=0; i<TIDL_MAX_ALG_IN_BUFS; i++)
  {
    if(TIDL_inYuvFormatYuv420_NV12 == gParams.inYuvFormat[i])
    {
      isYuvLayerPresent = true;
      break;
    }
  }
  return isYuvLayerPresent;
}

void tidl_printOpsList(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure)
{
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s,%s,%s,%s,%s,%s,","S.No","Layer Type","Output Data","Out Id","Input Data","In IDs");
  for (int i = 0; i < pOrgTIDLNetStructure.numLayers; i++)
  {
    // TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%5d|%-30s|%-50s|%-50s|%5d|%5d|", i, TIDL_LayerString[pOrgTIDLNetStructure.TIDLPCLayers[i].layerType], pOrgTIDLNetStructure.TIDLPCLayers[i].inDataNames[0], pOrgTIDLNetStructure.TIDLPCLayers[i].outDataNames[0], pOrgTIDLNetStructure.TIDLPCLayers[i].inData[0].dataId, pOrgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId);
    printf( "%d,%s,%s,%d,", i, TIDL_LayerString[pOrgTIDLNetStructure.TIDLPCLayers[i].layerType], pOrgTIDLNetStructure.TIDLPCLayers[i].outDataNames[0], pOrgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId);
    //pOrgTIDLNetStructure.TIDLPCLayers[i].inDataNames[0]pOrgTIDLNetStructure.TIDLPCLayers[i].inData[0].dataId
    for(int j = 0 ; j < pOrgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
    {
      printf("%s |",pOrgTIDLNetStructure.TIDLPCLayers[i].inDataNames[j]);
    }
    printf(",");
    for(int j = 0 ; j < pOrgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
    {
      printf("%d |",pOrgTIDLNetStructure.TIDLPCLayers[i].inData[j].dataId);
    }
    printf("\n");
  }
}


void tidl_printOptimizeTable(std::unordered_map<std::string, std::pair<int32_t,int32_t>> frequency_map)
{
  std::stringstream tableStream;
  std::string tableString;
  std::vector<std::string> header = {"Layer",
                                     "Nodes before optimization",
                                     "Nodes after optimization"};
  std::vector<std::vector<std::string>> data = {};
  std::vector<TIDL_table_align_t> columnAlignment = {ALIGN_LEFT,ALIGN_RIGHT,ALIGN_RIGHT};

  for (auto &i : frequency_map)
  {
    if(i.first == "TIDL_DataLayer" || i.first == "TIDL_DataConvertLayer" || i.first == "TIDL_ReshapeLayer")
    {
      continue;
    }
    std::string layerName = i.first;
    std::string beforeOptimize = std::to_string(i.second.first);
    std::string afterOptimize = std::to_string(i.second.second);

    // Add color
    if(i.second.first > 0)
    {
        if(i.second.second < i.second.first)
        {
          afterOptimize  = "\033[32;1m" + afterOptimize + "\033[0m"; // Green
        }
        else if(i.second.second > i.second.first)
        {
          afterOptimize  = "\033[31;1m" + afterOptimize + "\033[0m"; // Red
        }
    }
    data.push_back({layerName,beforeOptimize,afterOptimize});
  }

  if(!data.empty())
  {
    TIDL_createTable(tableStream, header, data, 1, columnAlignment);
    tableString = tableStream.str();
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"%s\n",tableString.c_str());
  }
}

void wrapString (std::string &str, int32_t wrapLength)
{
  int32_t charCount = 0;
  for(int32_t i = 0; i < str.size(); i++)
  {
    charCount++;
    if ((charCount > wrapLength) && (str[i] == ' '))
    {
      charCount = 0;
      str[i] = '\n';
    }
  }
}

int32_t countLines (std::string str)
{
  int32_t cnt = 0;
  for (int32_t i = 0; i < str.size(); i++)
  {
    if (str[i] == '\n') cnt++;
  }
  return cnt;
}

std::vector<std::string> splitStringByNewline(const std::string& str)
{
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{str};

    for (std::string line; std::getline(ss, line, '\n');)
        result.push_back(line);

    return result;
}

void tidl_printOptimizeLocationTable(std::vector<std::pair<std::string, std::string>> optimizationLocations)
{
  std::stringstream tableStream;
  std::string tableString;
  std::vector<std::string> header = {"Layer Outputs Before Optimization", "Layer Outputs After Optimization"};
  std::vector<std::vector<std::string>> data = {};
  std::vector<TIDL_table_align_t> columnAlignment = {ALIGN_LEFT, ALIGN_LEFT};
  int32_t beforeMaxLen = 0, afterMaxLen = 0, sep = 0;
  std::vector<int32_t> separatorPositions;

  for (auto &i : optimizationLocations)
  {
    std::string beforeOptimize = i.first;
    std::string afterOptimize = i.second;
    sep += 1;
    /* wrap strings */
    if (beforeOptimize.size() > 40) wrapString(beforeOptimize, 40);
    if (afterOptimize.size() > 40)  wrapString(afterOptimize, 40);
    int32_t beforeLines = countLines(beforeOptimize), afterLines = countLines(afterOptimize);
    int32_t maxLines = std::max(beforeLines, afterLines);
    if (beforeLines < maxLines)
    {
      beforeOptimize.insert(beforeOptimize.size(), (maxLines - beforeLines), '\n');
    }
    if (afterLines < maxLines)
    {
      afterOptimize.insert(afterOptimize.size(), (maxLines - afterLines), '\n');
    }
    beforeOptimize.insert(beforeOptimize.size(), 1, '\n');
    afterOptimize.insert(afterOptimize.size(), 1, '\n');
    /* Add newline at the end */
    auto beforeOptimizeWrapped = splitStringByNewline(beforeOptimize);
    auto afterOptimizeWrapped = splitStringByNewline(afterOptimize);
    for (int32_t l = 0; l <= maxLines; l++)
    {
      beforeMaxLen = std::max(beforeMaxLen, (int32_t)beforeOptimizeWrapped[l].size());
      afterMaxLen = std::max(afterMaxLen, (int32_t)afterOptimizeWrapped[l].size());
      data.push_back({beforeOptimizeWrapped[l], afterOptimizeWrapped[l]});
    }

    sep += maxLines;
    separatorPositions.push_back(sep);
  }

  std::string beforeSep, afterSep;
  beforeSep.insert(beforeSep.size(), beforeMaxLen, '-');
  afterSep.insert(afterSep.size(), afterMaxLen, '-');
  int32_t cnt = 0;
  for (auto &j: separatorPositions)
  {
    if (j != separatorPositions.back())
    {
      data.insert(data.begin() + j + cnt, {beforeSep, afterSep});
      cnt += 1;
    }
  }

  if(!data.empty())
  {
    TIDL_createTable(tableStream, header, data, 1, columnAlignment);
    tableString = tableStream.str();
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"%s\n",tableString.c_str());
  }
}

enum otfDisableCodes{
  NOT_DISABLE,
  DISABLE_DUE_TO_DECONV_2D,
  DISABLE_DUE_TO_DEPTH_TO_SPACE,
  DISABLE_DUE_TO_CUSTOM_LAYER,
  DISABLE_DUE_TO_DECONV_16_BIT,
  DISABLE_DUE_TO_CONV_PAD,
  DISABLE_DUE_TO_CONV_STRIDE,
  DISABLE_DUE_TO_CONV_DEPTH_TO_SPACE,
};

int32_t doesNetworkContainLayer(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t layerType, int32_t *layerId)
{
  int32_t numLayers = orgTIDLNetStructure.numLayers;
  int32_t doesNwContainLayer = 0;
  int32_t i;
  for(i = 0; i < numLayers; i++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == layerType)
    {
      doesNwContainLayer = 1;
      if(layerId != NULL) /*this function can be used if we do not want to get the layer_id*/
      {
        *layerId = i;
      }
      break;
    }
  }
  return doesNwContainLayer;
}

int32_t doesNetworkContainDeconv16Bit(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t *layerId)
{
  int32_t numLayers = orgTIDLNetStructure.numLayers;
  int32_t doesNwContainDeconv16bit = 0;
  int32_t weightElemBytes = 1U;
  int32_t i;
  for(i = 0; i < numLayers; i++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_Deconv2DLayer)
    {
      weightElemBytes = ((orgTIDLNetStructure.TIDLPCLayers[i].weightsElementSizeInBits + 7) / 8);
      if(weightElemBytes > 1U)
      {
        doesNwContainDeconv16bit = 1U;
        if(layerId != NULL)
        {
          *layerId = i;
        }
        break;
      }
    }
  }
  return doesNwContainDeconv16bit;
}

int32_t doesNetworkContainUnsupportedConvolutionConfig(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t *layerId, int32_t *otfUnsupportedCode)
{
  int32_t numLayers = orgTIDLNetStructure.numLayers;
  int32_t status = 0;
  int32_t i;
  for(i = 0; i < numLayers; i++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer)
    {
      if((orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.padW > orgTIDLNetStructure.TIDLPCLayers[i].inData->dimValues[TIDL_DIM_WIDTH]) && (orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.numGroups != orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.numInChannels))
      {
        status = 1;
        *layerId = i;
        *otfUnsupportedCode = DISABLE_DUE_TO_CONV_PAD;
        break;
      }
      else if( (orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.strideW > 2 || orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.strideH > 2) && (orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.padH > 0 || orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.padW > 0))
      {
        status = 1;
        *layerId = i;
        *otfUnsupportedCode = DISABLE_DUE_TO_CONV_STRIDE;
        break;
      }
    }
  }
  return status;
}

int32_t doesNetworkContainD2S(sTIDL_OrgNetwork_t &orgTIDLNetStructure, int32_t *layerId)
{
  int32_t numLayers = orgTIDLNetStructure.numLayers;
  int32_t doesNetworkContainD2SConv = 0;
  int32_t i;
  for(i = 0; i < numLayers; i++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_ConvolutionLayer)
    {
      if(orgTIDLNetStructure.TIDLPCLayers[i].layerParams.convParams.enableDepthToSpace == 1U)
      {
        doesNetworkContainD2SConv = 1;
        if(layerId != NULL)
        {
          *layerId = i;
        }
        break;
      }
    }
  }
  return doesNetworkContainD2SConv;
}

/* state = 0 -> Disables ST and backs up the original parameters enabling ST -- Arguments NULL indicates back up not required
   state = 1 -> Repopulates original parameters to enable ST
*/
int32_t enableOrDisableST(int32_t * enableHighResOptimization, int32_t * compileConstraintsFlag, uint8_t fileNameGrpInfo[], tidl_import_config * params, int32_t state)
{
  int32_t supertilingEnabled = 0;
  if(state == 0) /* Disable ST */
  {
    /*Backup original params*/
    if((enableHighResOptimization != NULL) && (compileConstraintsFlag != NULL) && (fileNameGrpInfo != NULL))
    {
      *enableHighResOptimization = params->enableHighResOptimization;
      *compileConstraintsFlag = params->compileConstraintsFlag;
      strcpy((char *)fileNameGrpInfo, (char*)params->fileNameGrpInfo);
    }
    if(isSupertilingEnabled(params) == 1)
    {
      /* Update gParams to disable supertiling */
      params->enableHighResOptimization = 0U;
      params->compileConstraintsFlag &= ~(0x80);
      strcpy((char*)params->fileNameGrpInfo, "\0");

      supertilingEnabled = 1;
    }
  }
  else if(state == 1) /* Enable ST */
  {
    if((enableHighResOptimization != NULL) && (compileConstraintsFlag != NULL) && (fileNameGrpInfo != NULL))
    {
      /* Restore the saved parameters to enable ST */
      params->enableHighResOptimization = *enableHighResOptimization;
      params->compileConstraintsFlag = *compileConstraintsFlag;
      strcpy((char*)params->fileNameGrpInfo, (char *)fileNameGrpInfo);
      supertilingEnabled = 1;
    }
  }
  return supertilingEnabled;
}


std::string generateLowLatencyNotSupportedMessage(pair<std::int32_t, std::string> disableOtfState)
{
  std::unordered_map<std::int32_t , std::string> otfDisableCodeMessageMap;

  otfDisableCodeMessageMap[DISABLE_DUE_TO_DECONV_2D] = "is a Deconv2DLayer";
  otfDisableCodeMessageMap[DISABLE_DUE_TO_DEPTH_TO_SPACE] = "is a Depth to Space Layer";
  otfDisableCodeMessageMap[DISABLE_DUE_TO_CUSTOM_LAYER] = "is a Custom Layer";
  otfDisableCodeMessageMap[DISABLE_DUE_TO_DECONV_16_BIT] = "is a 16-bit Deconv2DLayer";
  otfDisableCodeMessageMap[DISABLE_DUE_TO_CONV_PAD] = "is a Convolution Layer with pad more then the width/height of feature map";
  otfDisableCodeMessageMap[DISABLE_DUE_TO_CONV_STRIDE] = "is a Convolution Layer with stride more then 2";
  otfDisableCodeMessageMap[DISABLE_DUE_TO_CONV_DEPTH_TO_SPACE] = "is a Convolution Layer with Depth to Space";

  std::string message = "";

  if(disableOtfState.first != NOT_DISABLE)
  {
    message = "Network is not supported for low latency mode as layer " + disableOtfState.second + " " + otfDisableCodeMessageMap[disableOtfState.first] + ", Running inference with default mode";
  }

  return message;
}

/*Toggle OTF Padding "ON" for Supported Cases:*/
void TIDL_correctDeviceConfig(sTIDL_OrgNetwork_t &orgTIDLNetStructure,pair<std:: int32_t, std:: string> &disableOtfState)
{
  bool disableOtf = false;
  int32_t layerId = NOT_VALID;
  int32_t otfUnsupportedCode = NOT_DISABLE;

  if((gParams.deviceName == TIDL_TDA4AEP) || (gParams.deviceName == TIDL_TDA4AMPlus))
  {
    /*Evaluate whether OTF should be enabled*/
    if(gParams.inferenceMode == TIDL_inferenceModeLowLatency)
    {
      /*High Resolution optimizations*/
      if(doesNetworkContainLayer(orgTIDLNetStructure, TIDL_Deconv2DLayer, &layerId))
      {
        disableOtf = true;
        otfUnsupportedCode = DISABLE_DUE_TO_DECONV_2D;
      }
      else
      {
        disableOtf = false;
      }
    }
    
    if (isSupertilingEnabled(&gParams) == 1)
    {
      /*High Resolution optimizations*/
      if(doesNetworkContainLayer(orgTIDLNetStructure, TIDL_Deconv2DLayer, &layerId))
      {
        disableOtf = true;
        otfUnsupportedCode = DISABLE_DUE_TO_DECONV_2D;
      }
      else
      {
        disableOtf = false;
      }
    }
    else if(doesNetworkContainLayer(orgTIDLNetStructure, TIDL_DepthToSpaceLayer, &layerId))
    {
      /*Pixel Shuffle*/
      disableOtf = true;
      otfUnsupportedCode = DISABLE_DUE_TO_DEPTH_TO_SPACE;
    }
    else if(doesNetworkContainLayer(orgTIDLNetStructure, TIDL_CustomLayer, &layerId))
    {
      /*Custom Layer*/
      disableOtf = true;
      otfUnsupportedCode = DISABLE_DUE_TO_CUSTOM_LAYER;
    }
    else if(doesNetworkContainDeconv16Bit(orgTIDLNetStructure, &layerId))
    {
      /*16-bit deconvolution Layer*/
      disableOtf = true;
      otfUnsupportedCode = DISABLE_DUE_TO_DECONV_16_BIT;
    }
    else if(doesNetworkContainUnsupportedConvolutionConfig(orgTIDLNetStructure, &layerId, &otfUnsupportedCode))
    {
      /*Unsupported padding configuration*/
      disableOtf = true;
    }
    else if(doesNetworkContainD2S(orgTIDLNetStructure, &layerId))
    {
      otfUnsupportedCode = DISABLE_DUE_TO_CONV_DEPTH_TO_SPACE;
      /*D2S*/
      disableOtf = true;
    }
    if(!disableOtf)
    {
      /*Enable OTF:*/
      gParams.deviceName = gParams.deviceName | TIDL_OTF_FLAG_BIT ;
      gParams.enableOtfPad = 1U;
    }
  }
  else if(gParams.deviceName == TIDL_AM62A)
  {
    /*Enable OTF:*/
    gParams.deviceName = gParams.deviceName | TIDL_OTF_FLAG_BIT;
    gParams.enableOtfPad = 1U;
    /*Disable supertiling for AM62A*/
    if(isSupertilingEnabled(&gParams) == 1)
    {
      //gParams.enableHighResOptimization = 0U;
      //gParams.compileConstraintsFlag &= ~(0x80);
    }
  }
  else if((gParams.deviceName & TIDL_OTF_FLAG_BIT) == TIDL_OTF_FLAG_BIT)
  {
    gParams.enableOtfPad = 1U;
  }
  string str((char* )orgTIDLNetStructure.TIDLPCLayers[layerId].outDataNames[0]);
  disableOtfState = make_pair(otfUnsupportedCode, str);
}

void topoSort (int32_t layerIdx, int32_t numLayers, std::vector<bool>& visited, std::vector<int32_t>& stack)
{
  visited[layerIdx] = true;
  int32_t layerNumOutBufs = orgTIDLNetStructure.TIDLPCLayers[layerIdx].numOutBufs;
  std::vector<int> outLayerIndices;
  for (int32_t i = 0; i < layerNumOutBufs; i++)
  {
    auto outLayers = tidl_getOutLayers(orgTIDLNetStructure, numLayers,
          orgTIDLNetStructure.TIDLPCLayers[layerIdx].outData[i].dataId);
    outLayerIndices.insert(outLayerIndices.end(), outLayers.begin(), outLayers.end());
  }

  for (auto outLayerIdx: outLayerIndices)
  {
    if(!visited[outLayerIdx])
    {
      topoSort(outLayerIdx, numLayers, visited, stack);
    }
  }

  stack.push_back(layerIdx);
}

/**
 * Populates a vector of strings of the layer names
 * of the TIDL net structure. The order of layers
 * is a topological sorted order
*/
void getNetworkString (int32_t numLayers, std::vector<std::string> &networkString)
{
  std::vector<int32_t> stack;
  std::vector<bool> visited(numLayers, false);
  for (int32_t i = 0; i < numLayers; i++)
  {
    if (!visited[i])
    {
      topoSort(i, numLayers, visited, stack);
    }
  }
  while (!stack.empty())
  {
    auto layer = orgTIDLNetStructure.TIDLPCLayers[stack.back()];
    stack.pop_back();
    networkString.push_back(std::string((char*)layer.outDataNames[0]));
  }
}

/**
 * Given the initial and final network strings, localizes
 * the position of optimizations and populates an vector
 * organized like <before,after>
*/
void locateOptimizations (std::vector<std::string> initialNetworkString, std::vector<std::string> finalNetworkString,
                          std::vector<std::pair<std::string, std::string>> &optimizationLocations)
{
  int32_t initIdx, finalIdx;
  std::vector<std::pair<int32_t, int32_t>> matchIdx;
  std::unordered_set<std::string> uniqueNameSet;  // set to handle repeated names
  for (initIdx = 0; initIdx < initialNetworkString.size(); initIdx++)
  {
    if (uniqueNameSet.find(initialNetworkString[initIdx]) == uniqueNameSet.end())
    {
      for (finalIdx = 0; finalIdx < finalNetworkString.size(); finalIdx++)
      {
        if (initialNetworkString[initIdx].compare(finalNetworkString[finalIdx]) == 0)
        {
          uniqueNameSet.insert(initialNetworkString[initIdx]);  // add to set when a match is found as should be used again
          matchIdx.push_back(std::make_pair(initIdx, finalIdx));
          break;
        }
      }
    }
  }

  for (int32_t match = 0; match < matchIdx.size() - 1; match++)
  {
    int32_t next_match = match + 1;
    int32_t f1 = matchIdx[match].first, f2 = matchIdx[next_match].first;
    int32_t s1 = matchIdx[match].second, s2 = matchIdx[next_match].second;
    /* get before string */
    std::string before;
    for(int32_t i = f1+1; i <= f2; i++)
    {
      if (i == f2)
      {
        before.append(initialNetworkString[i]);
      }
      else
      {
        before.append(initialNetworkString[i] + std::string(", "));
      }
    }
    /* get after string */
    std::string after;
    for(int32_t i = s1+1; i <= s2; i++)
    {
      if (i == s2)
      {
        after.append(finalNetworkString[i]);
      }
      else
      {
        after.append(finalNetworkString[i] + std::string(", "));
      }
    }
    if (before.compare(after) != 0)
    {
      optimizationLocations.push_back(std::make_pair(before, after));
    }
  }
}

int32_t TIDL_updateMixedPrecisionFlagsProducers(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                                         int32_t consumerLayerIdx)
{
  /*Update the inputs of a given layer to indicate mixed precision if they do not support transition*/
  int32_t layerIdx;
  int32_t inIdx = 0;
  sTIDL_LayerPC_t * currLayer;
  sTIDL_LayerPC_t * consumerLayer;
  int32_t updated = 0;
  consumerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[consumerLayerIdx];
  if (consumerLayer->numInBufs > 0 && consumerLayer->mixedPrecisionState == PRECISION_LAYER_FEATURE_IN_16BIT && (TIDL_doesLayerSupportMixedPrecision(consumerLayer) == 0))
  {
    /*Identify the inputs to this layer and then check whether or not they need to update the state of inputs*/
    /*For all inputs of the layer:*/
    for(inIdx = 0; inIdx < consumerLayer->numInBufs; inIdx++)
    {
      /*Find the layer that produces the input data*/
      /* we should search for producer in whole network instead of using dataId */
      int32_t producerLayerIdx = tidl_getInLayer(*pOrgTIDLNetStructure, pOrgTIDLNetStructure->numLayers,
            consumerLayer->inData[inIdx].dataId);
      sTIDL_LayerPC_t * producerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx];
      if(TIDL_doesLayerSupportMixedPrecision(producerLayer) == 0)
      {
        /*If the producer layer does not support mixed precision then update the
        consumer layer to match the producer layer*/
        if(producerLayer->mixedPrecisionState == PRECISION_LAYER_DEFAULT)
        {
          producerLayer->mixedPrecisionState = PRECISION_LAYER_FEATURE_IN_16BIT;
          updated = 1;
        }
      }
    }
  }
  return updated;
}
                                                      

int32_t TIDL_updateMixedPrecisionFlagsConsumers(sTIDL_OrgNetwork_t * pOrgTIDLNetStructure,
                                                         int32_t producerLayerIdx)
{
  int32_t layerIdx;
  sTIDL_LayerPC_t * currLayer;
  sTIDL_LayerPC_t * producerLayer;
  int32_t producerElemType;
  int32_t producerDataId;
  int32_t inIdx;
  int32_t updated = 0;
  int32_t outDataId;
  producerLayer = &pOrgTIDLNetStructure->TIDLPCLayers[producerLayerIdx];
  producerDataId   = producerLayer->outData[0].dataId;

  if ( producerLayer->numOutBufs > 0 && producerLayer->mixedPrecisionState == PRECISION_LAYER_FEATURE_IN_16BIT)
  {
    /* Go through all the layers */
    for ( layerIdx = 0; layerIdx < pOrgTIDLNetStructure->numLayers; layerIdx++)
    {
      currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
      /* Find layers whos input contains the data Id. This will indicate the producer
      layer's output goes to this layer */
      for (inIdx = 0; inIdx < currLayer->numInBufs; inIdx++)
      {
        /* Indicates one of the consumer layer is found */
        if ( producerDataId == currLayer->inData[inIdx].dataId)
        {
          if(TIDL_doesLayerSupportMixedPrecision(currLayer) == 0)
          {
            /* If the producer layer does not support mixed precision then update the
            consumer layer to match the producer layer */
            if(currLayer->mixedPrecisionState == PRECISION_LAYER_DEFAULT)
            {
              currLayer->mixedPrecisionState = PRECISION_LAYER_FEATURE_IN_16BIT;
              updated = 1;
            }
          }
        }
      }

      if ( updated == 1 )
      {
        break;
      }
    }
  }
  return updated;
}

#define TIDL_MAX_MIXED_PREC_TRAVERSAL_ITER (32U)

int32_t tidl_updateMixedPrecisionState(sTIDL_OrgNetwork_t &orgTIDLNetStructure)
{
  int32_t layerIdx;
  int32_t outBufIdx;
  sTIDL_LayerPC_t* currLayer;
  int32_t outDataId;
  int32_t precision;
  sTIDL_OrgNetwork_t *pOrgTIDLNetStructure = &orgTIDLNetStructure;
  for ( layerIdx = 0; layerIdx < orgTIDLNetStructure.numLayers; layerIdx++)
  {
    currLayer = &pOrgTIDLNetStructure->TIDLPCLayers[layerIdx];
    outDataId = currLayer->outData[0].dataId;
    precision = getUserPrecisionFromDataId(pOrgTIDLNetStructure, &gParams, outDataId);
    /*If precision is PRECISION_LAYER_PARAMS_IN_16BIT update the mixedPrecisionInfo in the struct:*/
    if(precision == PRECISION_LAYER_FEATURE_IN_16BIT)
    {
      currLayer->mixedPrecisionState = PRECISION_LAYER_FEATURE_IN_16BIT;
    }
  }
  /* Second pass: Run through the network and update the consumer/producer mixed precision state if
  there is a mismtach between the two. Repeat this process till we go to a stage
  that no update is done in the network */
  int32_t updated = 0;
  do
  {
    updated = 0;
    for ( layerIdx = 0; layerIdx < orgTIDLNetStructure.numLayers; layerIdx++)
    {
      updated |= TIDL_updateMixedPrecisionFlagsProducers(&orgTIDLNetStructure, layerIdx);
      updated |= TIDL_updateMixedPrecisionFlagsConsumers(&orgTIDLNetStructure, layerIdx);
    }
  } while (updated != 0);

  return 0;
}

int32_t tidl_initializeMixedPrecision(sTIDL_OrgNetwork_t &orgTIDLNetStructure)
{
  /*Initialize the shadow copy of mixed precision*/
  int32_t i;
  for(i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
      orgTIDLNetStructure.TIDLPCLayers[i].mixedPrecisionState = PRECISION_LAYER_DEFAULT;
  }
  return 0;
}

int32_t tidl_optimizeNet(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t &layerIndex, int32_t &dataIndex)
{
  if(gParams.debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Running tidl_optimizeNet ");
  }
  pOrgTIDLNetStructure.numLayers = layerIndex;
  std::vector<std::string> initialNetworkString, finalNetworkString;
  std::unordered_map<std::string, std::pair<int32_t, int32_t>> frequency_map;

  for (int i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    std::string key = TIDL_LayerString[orgTIDLNetStructure.TIDLPCLayers[i].layerType];
    if (frequency_map.find(key) == frequency_map.end())
    {
      frequency_map[key] = {0,0};
    }
    frequency_map[key].first += 1;
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertEltwiseToBNLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addConstDataLayers (orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addInDataLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  // store network string for optimization logging
  getNetworkString(layerIndex, initialNetworkString);
  // Update input data shape with import config file
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_fillInDataLayerShape(orgTIDLNetStructure, &gParams, orgTIDLNetStructure.numLayers), "");

  //Merge Q/DQ Layers (ONNX)
  if(gParams.preQuantizedModel)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_handleDuplicateDQLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeInitializerQDQLayers(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeReluLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeInitializerDQLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeQDQLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, false), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeQDQLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, true), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_propagateCalibrationParams(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  }

  // Split single outDataLayer with multi-data to multiple outDataLayers
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_splitOutDataLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure,
                             orgTIDLNetStructure.numLayers), "");

  //logit
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeLogitLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  //layerNorm:
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_splitNormLayer(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeLayerNormLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  //gelu
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeGeluLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  //silu
  if(TIDL_isSupportedInFirmwareVersion((char*)gParams.c7xFirmwareVersion, "11_00_07_00"))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeSiluLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  if(true == tidl_isYuvLayerPresent())
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addLayersForUVConversion(orgTIDLNetStructure, &dataIndex, &gParams), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  /* Add Normalization layer if user requested the same */
  if ((gParams.inDataNorm[0]) && (false == tidl_isYuvLayerPresent()))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addNormLayerToInData(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  // Parse Shuffle Layer (ONNX ONLY)
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_FindHardSigmoidLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  // Remove Split Layers
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeSplitLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeBiasLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  /*Merge Clip layers*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeClipLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  //Replace Pack-Pack-Reshape with NN resize
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergePackToNNResizeLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  /*DCN block to optimize:*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeDCNv2Block(orgTIDLNetStructure, layerIndex, dataIndex), "");

  // Remove Padding Layer, not appliable in Caffe (ONNX ONLY)
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergePadLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  /*Update input tensor of DCN to be in transposed form (CHW->HWC) for optimal inference*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addTransposeForDCN(orgTIDLNetStructure, layerIndex, dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  if(!tidlInputTensorDimCheck(orgTIDLNetStructure))
  {
    TIDL_GLOBAL_REPORT_ERROR("Invalid input tensor dimension, aborting");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  if(gParams.debugTraceLevel > 0)
  {
    tidl_printOpsList(orgTIDLNetStructure);
  }

  /*Update input tensor of GridSample to be in transposed form (CHW->HWC) for optimal inference - This must be called before updating outputDataShapes*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addTransposeForGridSample(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addPadLayerBeforeConvForAsymmetricPadding(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  // Call Reshape functions to determine the input/output shape of each layer.
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  /* Should be called after the updateOutDataShape function call when the dimensions are induced*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addReshapeAcrossRMSNormLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  // Covert Concat with axis > channel to concat on channel axis
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertConcatAxisToChannel(orgTIDLNetStructure, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  //swish
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeSwishLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  TIDL_IMPORT_CHECK_AND_RETURN(addReshapeAcrossBatchNormLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  /*Conversion of reduceMean Layer to matmul*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertReduceMeanReduceSumToMatMulLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
  
  // Remove no-op tile layer where repeats are all 1
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeNoOpTileLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  /*Conversion of expand operator to eltwise sum layer*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertExpandToEltwiseSumLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  /* DepthToSpace/SpaceToDepth optimization */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeDepthToSpaceLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");

  /* Converting no-op gather to reshape */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_replaceGatherWithReshape(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  /* Converting gather with singular/contiguous indices to slice */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertGatherToSlice(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  /* Only line gather is supported (axis = TIDL_DIM_HEIGHT), converting rest to line gather if possible */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertGatherToLineGather(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  if(gParams.preQuantizedModel){
    // Update: Added rule to remove the transpose between dequant and Matmul
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeTranposeBetweenDequantizeAndMatMul(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  // converting instance normalization layer into layer norm
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertInstanceNormalizationToLayerNorm(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  //layerNorm:
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_splitNormLayer(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers, true), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  //converting groupNormalization layer into instance normalization
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertGroupNormToInstanceNorm(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  /** Fuse Patch Merging Block */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_fusePatchMergingBlock (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  /* recheck through slices if something is still there with stride > 1*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_recheckSliceStrideSupport (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");


  // Change the slice axis to channel dim if it is on higher dimensions
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_reduceSliceDims (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  // Duplicate Slice Layer. 3 layers for 3 outputs.
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_duplicateSliceLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  /**
   * Find WrapAroundPadLayer structure in ONNX -> (Pad + 2 Slice's + Concat) 
   * Replace the structure with a single Pad layer with PadWrapAround flag set
   */ 
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_FindWrapAroundPadLayer (orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeConsecutiveSliceLayers (orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  // Split Resize layer into multiple layers when resize ratio is > 4x.
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_splitResizeLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertPoolToIdentityLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeIdentitytLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeReluLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertBiasToBNLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeBNLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  //optimizer deconv layers
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeDeConvLayers(orgTIDLNetStructure, layerIndex, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_handleNegativePadValuesForPadLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
  
  if (gParams.optimizeBatchNormHigherDims == 1)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeBatchNormLayers (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeReluLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeGlobalAveragePoolingLayers (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  /*Below functions are now deprecated*/
  // TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeGridSampleLayers (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  // TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  // tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizePatchEmbedding(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  if (gParams.deviceName != TIDL_TDA4VM)
  {
    /* Self-Attention Optimization: MatMul B transpose */
    /** Absorb B-side transpose in Innerproduct if present
     * Provided the transpose is swapping height and width dimension
    */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_InnerProductBTranspose (orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }


  if(gParams.foldMaxPoolInConv2D == 1)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergePoolingLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_fuseMSDAMulMatMul(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertVariableDivToBN(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeDivideByOneLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeBatchToSpaceInDWConvLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_merge1x1MaxPoolingLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_merge1x1ResizeLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeMinimumLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convert1x1s1DWSConvToBatchNorm(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_initializeMixedPrecision(orgTIDLNetStructure),"");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_updateMixedPrecisionState(orgTIDLNetStructure),"");

  /* Neg layer is only supported above 10_01_06_00 firmware */
  if(TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_06_00"))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertVariableSubToAddWithNeg(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertRelUToBNLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_FindFlattenLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertConv2DToIpLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT || gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addReshapeBeforeIP(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeFlattenLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");

  /*
   * Parsing SSD Structure
   * Steps:
   * 1. Merge Softmax Layer into DetectionOut Layer
   * 2. Merge Reshape Layer into DetectionOut Layer
   * 3. Handle width-wise Concat shape
   * 4. Remove Permute Layer
   * 5. Remove all Priorbox Layer, and send params to DetectionOut Layer.
   *    Search all concat layer with priorbox input and detection out output.
   *    Parse each priorbox in order.
   */
   if  (gParams.metaArchType == -1)
   {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeDetectionoutLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
   }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addPadLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  if(gParams.foldPreBnConv2D && gParams.quantizationStyle != TIDL_QuantStyleAsymNP2)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergePreBNLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  }

  if(true == tidl_isYuvLayerPresent())
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeYUVtoRGBConversion(orgTIDLNetStructure, &dataIndex, &gParams), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  if ( (gParams.metaArchType == TIDL_metaArchTIDLSSD) ||
       (gParams.metaArchType == TIDL_metaArchTFSSD)   ||
       (gParams.metaArchType == TIDL_metaArchTFFasterRcnn) ||
       (gParams.metaArchType == TIDL_metaArchTIDLYolo) ||
       (gParams.metaArchType == TIDL_metaArchTIDLRetinaNet) ||
       (gParams.metaArchType == TIDL_metaArchTIDLYoloV5)    ||
       (gParams.metaArchType == TIDL_metaArchTIDLYoloV8)    ||
       (gParams.metaArchType == TIDL_metaArchTIDLCenterPoint)    ||
       (gParams.metaArchType == TIDL_metaArchTIDL3DOD)  ||
       (gParams.metaArchType == TIDL_metaArchTIDLBEVFormer)  ||
       (gParams.metaArchType == TIDL_metaArchTfliteRtSsdDetectionLayer)
       ) //TODO : can we just write (gParams.metaArchType != -1) instead of all above conditions
  {
    if(gParams.metaArchType == TIDL_metaArchTfliteRtSsdDetectionLayer)
    {
      numTFMetaLayers = 1;
    }

    int32_t numDetlayer = numTFMetaLayers;

    for(int32_t i = 0; i < numTFMetaLayers; i++)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_addMetaArchLayersToNet(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams, i), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
  }

  if (gParams.metaArchType != TIDL_metaArchTIDLYoloV5)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeConcatFlattenLayerInDetNet(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT && gParams.metaArchType != -1)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_AddTfODOutputLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, gParams.metaArchType), "");
  }
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT ||
     gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_AddOnnxODOutputLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  }
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertFlattenToReshape (orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  if (gParams.addDataConvertToNet)
  {
    if(false == tidl_isYuvLayerPresent())
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_addDataConvertLayer(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
      /*updating dataType after inserting dataConvertLayer to ensure the dataType changes are propagated to furteher layer*/
      tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
    }
    else
    {
      TIDL_GLOBAL_REPORT_WARNING("addDataConvertToNet is ignored as this feature is not supported for YUV inputs");
    }
  }

  layerIndex = orgTIDLNetStructure.numLayers;

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addDataConvertForScatterLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  if (gParams.optimizeTopKAxis == 1)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_handleTopKAxis(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_handleTopKLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_handleRecurrentLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_fuseTransposeMatMul(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_supportMultiChannelMatMulBias(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_supportMultiChannelMatMul(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  if (gParams.quantizationStyle == TIDL_QuantStyleAsymNP2)
  {
    /*Fuse bias to Matmul for asymmetric quantization (& devices which allow it):*/
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_FindMatMulBias (orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

    if (gParams.numParamBits <= 16U)
    {
      /*Layernorm Beta & Gamma optimization:*/
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_fuseLayerNormBetaGamma(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
      /**************************************/
      /* Fusing channel into matmul after LN beta gamma fusion if input contains channel */
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_supportMultiChannelMatMul(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeNoOpReshapeLayers (orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeSqueezeLayers (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  /* Convert unsqueeze to reshape */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertUnsqueezeToReshape(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  tidl_addPadLayerBeforeConv(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers);
  tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers);
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  /*
   * Parsing Softmax/InnerProduct Structure
   * Steps:
   * 1. Change the shape of pooling output & IP input.
   * 2. Change the shape of pooling output & Softmax input.
   *
   * Support 2 formats:
   * 1. global_pooling: true
   * 2. 7x7 avg pooling with channelx7x7 blob
   */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertSoftMaxLayerInputShape(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  TIDL_convertDeconv2DtoConv(orgTIDLNetStructure, layerIndex);
  pair<std::int32_t, std::string> disableOtfState; /*first=errorcode, second=layerIndex*/
  

  TIDL_correctDeviceConfig(orgTIDLNetStructure, disableOtfState);

  /*If OTF is disabled, change quantization style to TIDL_QuantStyleNP2Fixed:*/
  if(disableOtfState.first != NOT_DISABLE && gParams.quantizationStyle == TIDL_QuantStyleAsymNP2)
  {
    gParams.quantizationStyle = TIDL_QuantStyleNP2Fixed;
  }
  
  if(TIDL_isPadOTF(gParams.deviceName))
  {
    int32_t padLayerRemoved = tidl_mergeColumnPadIntoConv (orgTIDLNetStructure, orgTIDLNetStructure.numLayers);
    if (padLayerRemoved == 1)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addDataConvertLayerForOTFsupport(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams, unsupportedLayersForOTF), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addCropLayerForValidPadSupport(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  // Support for unit conv with padding
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_supportUnitConvWithPad(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  if(!TIDL_isPadOTF(gParams.deviceName) && gParams.inferenceMode == TIDL_inferenceModeLowLatency && gParams.numCores > 1)
  {
    TIDL_GLOBAL_REPORT_WARNING("%s", generateLowLatencyNotSupportedMessage(disableOtfState).c_str()) ;
    gParams.inferenceMode = TIDL_inferenceModeDefault;
    gParams.numCores = 1;
  }

  /*
  ** Disabling matmul layer specifically from ST, instead of preventing whole network from going into ST**
  if(tidl_doesNetworkHasMatmulLayer(orgTIDLNetStructure))
  {
    if(gParams.enableHighResOptimization == 1)
    {
      TIDL_GLOBAL_REPORT_WARNING("High Resolution optimization is not available for matmul layer, current execution is running without high resolution optimization");
      gParams.enableHighResOptimization = 0;
    }
  }
  */


  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeSoftmaxLayer(orgTIDLNetStructure, &dataIndex, &gParams, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_fuseSoftMaxDataConvert(orgTIDLNetStructure, &dataIndex, &gParams, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  /*VTFR Transpose Handling:*/
  /*A: Support for optimized softmax:
      -> Softmax (axis=6) -> == -> Transpose(0,1->1,0) -> Softmax (axis=5) -> Transpose(1,0->0,1) */

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeSoftmaxAxis(orgTIDLNetStructure, &dataIndex, &gParams, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
  /*End of transpose handling*/

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeReshapeLayers (orgTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");


  TIDL_IMPORT_CHECK_AND_RETURN(tidl_fuseTransposeLayers (orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  if(TIDL_isSupportedInFirmwareVersion((const char*)gParams.c7xFirmwareVersion,"10_01_04_00"))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_removeTransposeFromNet(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }
  else
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeTransposeLayers (orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  }

  /* Add dataconvert between slice and reshape in the indices input of gather */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_addDataConvertForGather(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");  

  /*
   * Shape Folding logic:
   * 1. Remove stray singletons if possible
   * 2. Merge dimensions with continuous broadcast
   * 3. Merge higher dims to channel except batch
   * 
   * Shape folding is only applied if inference mode is not high throughput
   * and batchmode is not defined explicitly
   * 
   * TODO Future: Fusion rules i.e 2,3 can still be applied in above modes if
   *              we skip manipulation at dimension where batch is detected.
   */

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  if(gParams.inferenceMode == TIDL_inferenceModeHighThroughput)
  {
    gParams.enableShapeFolding = 0;
  }

  if(gParams.enableShapeFolding == 1)
  {
    /* Apply push to batch logic based on numCores */
    if(!(gParams.batchMode == 1 || gParams.inferenceMode == TIDL_inferenceModeHighThroughput))
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_pushToBatch(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
      if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
      {
        TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
        TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
      }
      tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
    }

    /*
     * Detecting batch groups only in case of batchMode or TIDL_inferenceModeHighThroughput mode,
     * since we are now doing tidl_batchProcessing only in these cases
     */
    if(gParams.batchMode == 1 || gParams.inferenceMode == TIDL_inferenceModeHighThroughput)
    {
      /* Detect batch processing groups to avoid shape folding at those layers */
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_satisfyHighThroughputBatchRequirement(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "Model could not satisfy High Throughput requirement for batch dimension");
      std::vector<std::pair<std::pair<int32_t, int32_t>, std::vector<int32_t>>> batchProcessingGroups;
      std::vector<std::vector<std::vector<int32_t>>> batchGroupBoundaries;
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_detectBatchSupportedGroups(orgTIDLNetStructure, batchProcessingGroups, batchGroupBoundaries, 0), "");
    }

    /* Remove out unecessary singleton dimensions */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeSingletonDimensions(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

    /* Merge dimensions with continuous broadcast */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeContiguousBroadcast(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

    /* Merge higher dimensions except batch to lower */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeHigherDimsToChannel(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeHigherDimsInSpecialCase(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

    /* Remove out unecessary singleton dimensions */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeSingletonDimensions(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

    /* Merge dimensions with continuous broadcast */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeContiguousBroadcast(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

    /* Explode width in case it > 64KB (LFMP constraint) */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_explodeWidthForLFMP(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
    if(tidl_mergeReshapeLayers(orgTIDLNetStructure) == 0)
    {
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
    }
    tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
  }

  /* Remove extra reshape layers */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mergeNoOpReshapeLayers (orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  if(tidl_mergeReshapeLayers(orgTIDLNetStructure))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);

  /* Sorting the network and updating shapes before detecting batchSupported groups and shifting batch dimension */
  if(gParams.batchMode == 1 || gParams.inferenceMode == TIDL_inferenceModeHighThroughput || gParams.enableShapeFolding != 1)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_satisfyHighThroughputBatchRequirement(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "Model could not satisfy High Throughput requirement for batch dimension");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_batchProcessing(orgTIDLNetStructure, dataIndex), "Error while detecting batch processing groups");
  }

  /* Mapping gemm/matmul layer to equivalent IP layer. It has to be done after batchProcessing only - to identify the gemm categories */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_mapGemmMatMulToIP(orgTIDLNetStructure, &dataIndex, orgTIDLNetStructure.numLayers), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  tidl_updateOutDataShape(orgTIDLNetStructure, 0, orgTIDLNetStructure.numLayers, (sTIDL_tfOutReshapeMap_t *)&sTIDL_OutReshapeTable);
  
  /*Enabling ST for >8bits as well*
  tidl_updateHighResOptimization(orgTIDLNetStructure, orgTIDLNetStructure.numLayers);
  */
  tidl_updateScratchMemoryRequirement(orgTIDLNetStructure);
  

  /*Operations after which shape determination (tidl_updateOutDataShape) cannot be called again:*/
  /*1. Flatten softmax<->GAP*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_convertSoftMaxLayerInputShape(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  /*2. Update pooling params:*/
  tidl_poolPadParams(orgTIDLNetStructure, layerIndex);


  /* Merge left over reshape layers */
  if(tidl_mergeReshapeLayers(orgTIDLNetStructure))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  // Merge duplicate outputs
  if(tidl_mergeDuplicateOutputs(orgTIDLNetStructure))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  bool isReshape = tidl_findReshapeinNetwork(orgTIDLNetStructure, orgTIDLNetStructure.numLayers);
  if(isReshape)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addDataConvertLayerForOTFsupport(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams, padUnsupportedLayers), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_updateConstDataLayerRange(&orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  
  if(gParams.numParamBits <= 8) {
    /* Adding DC layers to prevent the propagation of 16bit support in producer layers */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addDataConvertLayerForMPConstraining(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, &gParams), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }
  
  if(gParams.inferenceMode == TIDL_inferenceModeLowLatency && gParams.numCores > 1) /*if NC adds or removes layers we update multi-core state and crop layers*/
  {
    tidl_updateMultiCoreState(orgTIDLNetStructure, &gParams);
    /* IMP :: NOTE :: This function should be called after all layers addition/merge is complete, layer deletion post this state is till ok */
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_addCropLayerForMultiCore(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, &dataIndex, gParams.quantizationStyle, gParams.numCores), "");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers), "");
  }

  /*4. Making dataId & layerId same should be called at the end after all the optimizations, no structure change after this*/
  tidl_makeDataIdLayerIdSame(&orgTIDLNetStructure, orgTIDLNetStructure.numLayers);

  /* Changing the outElementType if given in import config - needs to called at end of all optimizations */
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_changeOutElementType(orgTIDLNetStructure, orgTIDLNetStructure.numLayers), "");

  for (int i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    std::string key = TIDL_LayerString[orgTIDLNetStructure.TIDLPCLayers[i].layerType];
    if (frequency_map.find(key) == frequency_map.end())
    {
      frequency_map[key] = {0,0};
    }
    frequency_map[key].second += 1;
  }

  // Generate table
  TIDL_GLOBAL_REPORT_SUBHEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_NoColor, "Optimization Summary")
  tidl_printOptimizeTable(frequency_map);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"Total nodes in subgraph: %d\n", orgTIDLNetStructure.numLayers);


  getNetworkString(orgTIDLNetStructure.numLayers, finalNetworkString);
  std::vector<std::pair<std::string, std::string>> optimLocations;
  locateOptimizations(initialNetworkString, finalNetworkString, optimLocations);
  if(gParams.debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_SUBHEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_NoColor, "Layer mapping to TIDL Network Layers")
    tidl_printOptimizeLocationTable(optimLocations);
  }
  
  if(gParams.debugTraceLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Completed tidl_optimizeNet ");
  }
  layerIndex = orgTIDLNetStructure.numLayers;

  // Set optimize flag for each layer to 1
  for (int i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    orgTIDLNetStructure.TIDLPCLayers[i].optimized = 1;
  }
  /*Final update of mixed precision state:*/
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_initializeMixedPrecision(orgTIDLNetStructure),"");
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_updateMixedPrecisionState(orgTIDLNetStructure),"");
  /* NC expects layerId and dataId to be in sync*/
  dataIndex = tidl_sortDataIDsinPCNet(&orgTIDLNetStructure, MAKE_LAYERID_DATAID_SAME);

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t updatePadAndWriteModel(sTIDL_OrgNetwork_t * pTidlOrigNet,
                                                             sTIDL_Network_t  * pTidlNet,
                                                             tidl_import_config * configParams)
{
  int32_t numLayers = pTidlOrigNet->numLayers;
  if(numLayers > TIDL_NUM_MAX_LAYERS)
  {
    TIDL_GLOBAL_REPORT_ERROR("TIDL does not support more than %d layers", TIDL_NUM_MAX_LAYERS);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  int32_t tiLayerIndex = tidl_copyPCNetToDeviceNet(pTidlOrigNet,
                                                                              pTidlNet,
                                                                              configParams,
                                                                              numLayers);

  tidl_addOutDataLayer(pTidlNet , tiLayerIndex);

  TIDL_fillDataBufPadRequirements(pTidlNet);
  tidl_updateNetPitch(pTidlNet);
  /* Offsets in the Net needs to be updated before writing the net to file */
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_writeModel(pTidlNet, pTidlOrigNet, (const char *)NULL, numLayers, configParams), "");
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_writeModel(pTidlNet, pTidlOrigNet, (const char *)(configParams)->outputNetFile, numLayers, configParams), "");
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_writeInfo(pTidlNet,  pTidlOrigNet,  (const char *)(configParams)->outputParamsFile, numLayers, 1, NULL, configParams), "");

  int64_t modelSize = tidl_getFileSize((configParams)->outputNetFile);
  if(modelSize == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  pTidlOrigNet->modelSizeWithMaxNC = modelSize;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_doesLayerSupportBatchStitching(sTIDL_LayerPC_t * layer)
{

  int32_t doesLayerSupportBatchProcessing = 0;

  if ( layer->layerType == TIDL_ConvolutionLayer )
  {
    doesLayerSupportBatchProcessing = 1;
  }
  else if ( layer->layerType == TIDL_PoolingLayer)
  {
    doesLayerSupportBatchProcessing = 1;
    if (layer->layerParams.poolParams.kernelW == 0)
      doesLayerSupportBatchProcessing = 0;
  }
  else if ( layer->layerType == TIDL_BatchNormLayer)
  {
    doesLayerSupportBatchProcessing = 1;
  }
  else if ( layer->layerType == TIDL_EltWiseLayer)
  {
    doesLayerSupportBatchProcessing = 1;
    for(int32_t i = 0; i < TIDL_DIM_MAX; i++)
    {
      if(layer->inData[0].dimValues[i] != layer->inData[1].dimValues[i])
      {
        doesLayerSupportBatchProcessing = 0;
        break;
      }
    }
  }
  else if ( layer->layerType == TIDL_ConcatLayer)
  {
    doesLayerSupportBatchProcessing = 1;
  }
  else if ( layer->layerType == TIDL_InnerProductLayer)
  {
    doesLayerSupportBatchProcessing = 0;
  }
  else if ( layer->layerType == TIDL_SoftMaxLayer)
  {
    doesLayerSupportBatchProcessing = 0;
  }
  else if ( layer->layerType == TIDL_DataLayer)
  {
    doesLayerSupportBatchProcessing = 0;
  }
  else if ( layer->layerType == TIDL_ConstDataLayer)
  {
    doesLayerSupportBatchProcessing = 0;
  }
  else if ( layer->layerType == TIDL_BatchReshapeLayer)
  {
    int32_t batchReshapeType = layer->layerPCParams.batchReshapeParams.batchReshapeType;
    if(batchReshapeType == TIDL_BATCH_RESHAPE_STITCH || batchReshapeType == TIDL_BATCH_RESHAPE_INTERMEDIATE)
    {
      doesLayerSupportBatchProcessing = 1;
    }
    else
    {
      doesLayerSupportBatchProcessing = 0;
    }
  }
  else if ( layer->layerType == TIDL_ReshapeLayer)
  {
    doesLayerSupportBatchProcessing = 0;
  }
  else if ( layer->layerType == TIDL_TransposeLayer)
  {
    doesLayerSupportBatchProcessing = 0;
  }
  else if (layer->layerType == TIDL_SliceLayer)
  {
    /*
    check if slice axis is on batch
    */
    if (layer->layerParams.sliceParams.axis == TIDL_DIM_BATCH)
    {
      doesLayerSupportBatchProcessing = 0;
    }
    else
    {
      doesLayerSupportBatchProcessing = 1;
    }
  }

  return doesLayerSupportBatchProcessing;
}

std::string TIDL_augmentCharArrayWithSuffix(char * oldArr, std::string augStr)
{
  string oldStr = oldArr;
  return (oldStr + augStr);
}

/** Used as part of algorithm for automating mixed precision
 * The algorithm requires the following to be done multiple times:
 * Quantize network (calibration)
 * Run inference to collect layer level trace data and perfsim performance
 * This function accepts "dumpTrace" as a parameter to indicate whether layer level traces are required or not
 * */
float TIDL_quantizeInferPerfsim(tidl_import_config * configParams, int32_t layerIndex, bool dumpTrace = false)
{
  int32_t status = 0;
  status = TIDL_import_quantize(layerIndex);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return NAN;
  }
  //dumpTrace is a boolean which which will signify whether we want to dump the trace of all layer
  // corresponding to the current mode(float,8bit , MP) for use in calculation of impact factors
  if(dumpTrace){
    configParams->writeTraceLevel = 3;
  }
  status = tidlRunQuantStatsTool(&orgTIDLNetStructure,
                                      &tIDLNetStructure,
                                      configParams,
                                      orgTIDLNetStructure.numLayers);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return NAN;
  }
  float networkTime = tidlRunPerfSimTool(configParams, orgTIDLNetStructure.NCExecMode);
  return networkTime;
}

void TIDL_restoreOrgNetAndConfigParams(sTIDL_OrgNetwork_t * orgTIDLNetStructureOrig, tidl_import_config * configParamsOrig)
{
  orgTIDLNetStructure = *orgTIDLNetStructureOrig;
  TIDL_allocAndCopyModelParams(&orgTIDLNetStructure, orgTIDLNetStructureOrig, orgTIDLNetStructure.numLayers);
  gParams = *configParamsOrig;
}


/*
This function returns the file name in the folder referenced by path
and contains word1 and word2 in the file name
*/
std::string TIDL_getFileNameContainingWords(std::string path,string word1,string word2){
#ifdef _WIN32
  return 0;
#else

  struct dirent *entry;
  DIR *dir = opendir((const char *)path.c_str());
  // directory doesn't exists hence returing empty string
  if(dir == NULL){
    return "";
  }
  std::string reqFileName = "";
  while((entry = readdir(dir)) != NULL){
    std::string fileName = entry->d_name;
    if(fileName.find(word1) != string::npos and fileName.find(word2) != string::npos){
      reqFileName = fileName;
      break;
    }
  }
  closedir(dir);
  return reqFileName;
#endif
}

/*
This function updates the gParams parameter as per the passed value
*/
void TIDL_updateConfigParameters(
  tidl_import_config * configParams,
  int32_t biasCalibrationIterations,
  int32_t numFramesBiasCalibration,
  int32_t calibrationOption,
  int32_t numFeatureBits,
  int32_t numParamBits,
  int32_t numFrames
) {
  /* change the configParams as specified as arguments to function */
  if(biasCalibrationIterations != -1)
  {
    if(biasCalibrationIterations < 1)
    {
      configParams->biasCalibrationIterations = 1;
      TIDL_GLOBAL_REPORT_WARNING("Using only 1 iteration for automated mixed precision");
    }
    else
    {
      configParams->biasCalibrationIterations = biasCalibrationIterations;
    }
  }

  if(numFramesBiasCalibration != -1)
  {
    if(numFramesBiasCalibration < 1)
    {
      configParams->numFramesBiasCalibration = 1;
      TIDL_GLOBAL_REPORT_WARNING("Using only 1 frame for automated mixed precision");
    }
    else
    {
      configParams->numFramesBiasCalibration = numFramesBiasCalibration;
    }
  }

  if(calibrationOption != -1){
    configParams->calibrationOption = calibrationOption;
  }

  if(numFeatureBits != -1){
    configParams->numFeatureBits = numFeatureBits;
  }

  if(numParamBits != -1){
    configParams->numParamBits = numParamBits;
  }

  if(numFrames != -1)
  {
    if(numFrames < 1)
    {
      configParams->numFrames = 1;
      TIDL_GLOBAL_REPORT_WARNING("Using only 1 frame for automated mixed precision");
    }
    else
    {
      configParams->numFrames = numFrames;
    }
  }
}


/*
This function calculates the MSE Error from the data present in the float vectors
data1 and data2
*/
float TIDL_calculateMeanSquaredErrorByVectors(vector<vector<float>>& data1, vector<vector<float>>& data2){
  float mse = 0.0;
  vector<float> totalError(data1.size(), 0.0);
  for(int i = 0; i < data1.size(); i++)
  {
    for(int j = 0; j < data1[i].size(); j++)
    {
      totalError[i] += (data1[i][j] - data2[i][j]) * (data1[i][j] - data2[i][j]);
    }
    totalError[i] /= (float)(data1[i].size());
  }

  for(int i = 0; i < totalError.size(); i++)
  {
    mse += totalError[i];
  }
  mse = mse / (float)(totalError.size());

  return mse;
}

/*
This function calculates the standard deviation of an array of number
*/
float TIDL_calculateStandardDeviation(vector<float>& nums){
  float sum = 0.0;
  int n = nums.size();
  for(int i = 0; i < n; i++){
    sum += nums[i];
  }
  float mean = sum / n;
  float standardDeviation = 0.0;
  for(int i = 0; i < n; i++){
    standardDeviation += pow(nums[i] - mean,2);
  }
  standardDeviation = standardDeviation/n;
  standardDeviation = sqrt(standardDeviation);
  return standardDeviation;
}

float TIDL_calculateMeanAbsoluteErrorByVectorsHelper(vector<float>& x,vector<float>& y){
  float eps = 1e-5;
  float standardDeviation = TIDL_calculateStandardDeviation(x);
  vector<float> diff;
  for(int i = 0; i < x.size(); i++){
    float val = (abs(x[i] - y[i]) + eps) / (standardDeviation + eps);
    diff.push_back(val);
  }
  int cnt = 0;
  float sum = 0;
  for(int i = 0; i < diff.size(); i++){
    if(abs(diff[i]) < 6.0){
      sum += diff[i];
      cnt++;
    }
  }
  return sum / (float)cnt;
}

float TIDL_calculateMeanAbsoluteErrorByVectors(vector<vector<float>>& data1,vector<vector<float>>& data2){
  float error = 0.0;
  vector<float> totalError(data1.size(), 0.0);
  for(int i = 0; i < data1.size(); i++){
    totalError[i] = TIDL_calculateMeanAbsoluteErrorByVectorsHelper(data1[i],data2[i]);
  }
  for(int i = 0; i < totalError.size(); i++){
    error += totalError[i];
  }
  error = error / (float)(totalError.size());
  return error;
}

/** Calculate distance metric given input data in the form of vectors for specified metric argument
 * Currently supported metrics : (1) MSE = (sum ((x - y)^2)/n and (2) |x - y|/std(x)
*/
float TIDL_calculateErrorByVectors(vector<vector<float>> data1, vector<vector<float>> data2, string errorType){
  if(errorType == "MSE"){
    return TIDL_calculateMeanSquaredErrorByVectors(data1, data2);
  }
  else if(errorType == "MAE"){
    return TIDL_calculateMeanAbsoluteErrorByVectors(data1, data2);
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Specified distance metric is not valid, it should be either MSE or MAE\nProceeding with MSE metric");
    return TIDL_calculateMeanSquaredErrorByVectors(data1, data2);
  }
}

/** This function gets data from traces dumped for layers with dataId specified in outputLayerIndices
 * And normalizes the traces for further distance metric computation
 * */
vector<vector<float>> TIDL_readAndNormalizeTraces(string modePrefix,char outDirName[],vector<int>& outputLayerIndices)
{
  vector<vector<float>> values;
  for(int i = 0; i < (int)outputLayerIndices.size(); i++){
    int dataId = outputLayerIndices[i];
    string dataIdString = to_string(dataId);
    while(dataIdString.size() < 4){
      dataIdString = "0" + dataIdString;
    }
    dataIdString = modePrefix + dataIdString;
    string fname = TIDL_getFileNameContainingWords(outDirName,dataIdString,"_float.bin");
    string absFilePath = outDirName;
    absFilePath = absFilePath + "/" + fname;
    cout << absFilePath << endl;
    vector<float> value;
    float val;
    std::ifstream fin(absFilePath, std::ios::binary);
    while(fin.read(reinterpret_cast<char*>(&val),sizeof(float))){
      value.push_back(val);
    }
    values.push_back(value);
  }
  vector<float> errors;
  int minSize = INT_MAX;
  for(int i = 0; i < (int)values.size(); i++)
  {
    float maxValue = -(FLT_MAX);
    float minValue = FLT_MAX;
    for(int j = 0; j < (int)values[i].size(); j++)
    {
      maxValue = max(maxValue,values[i][j]);
      minValue = min(minValue,values[i][j]);
    }
    for(int j = 0; j < (int)values[i].size(); j++)
    {
      values[i][j] = (values[i][j] - minValue) / (maxValue - minValue);
    }
  }
  return values;
}

/** Main algorithm for automating mixed precision
 * Sort the impact factors in descending order
 * Keep adding the corresponding layers to 16 bit list sequentially till performance stays below limit specified by mixed precision factor
 * Return list of layers to be put in 16 bit
 * */
string TIDL_getLayersToKeepIn16BitByImpactFactors(sTIDL_OrgNetwork_t * orgTIDLNetStructureOrig,tidl_import_config * configParamsOrig,
                                                  vector<pair<float,int>>& impactFactors, uint32_t layerIndex,
                                                  float networkTimeFor8Bit, string outDirNameStr, string netFileName, string ioFileName)
{
  ofstream loggerFile;
  loggerFile.open(outDirNameStr + "/layers_selection.txt");
  loggerFile << "Information Regarding which layers are getting added" << endl;
  TIDL_restoreOrgNetAndConfigParams(orgTIDLNetStructureOrig, configParamsOrig);
  strcpy((char *)gParams.outputNetFile, netFileName.c_str());
  strcpy((char *)gParams.outputParamsFile, ioFileName.c_str());
  sort(impactFactors.begin(),impactFactors.end(),greater<pair<float,int>>());
  float mixedPrecisionFactor = gParams.mixedPrecisionFactor;
  loggerFile << "Mixed precision factor = " << mixedPrecisionFactor << endl;
  float cur_mixed_precision_factor = 0;
  int i = 0;
  std::string layers = (char *)gParams.outputFeature16bitNamesList;
  std::string layersOld = (char *)gParams.outputFeature16bitNamesList;
  std::string layerIdsIncluded = "";
  std::string layerIdsIncludedOld = "";
  int iteration = 0;
  while (i < impactFactors.size() && cur_mixed_precision_factor < mixedPrecisionFactor)
  {
    if(layers.find((char *)orgTIDLNetStructureOrig->TIDLPCLayers[impactFactors[i].second].outDataNames[0]) != string::npos){
      i++;
      continue;
    }
    if(layers.size() == 0){
      layers = (char *)orgTIDLNetStructureOrig->TIDLPCLayers[impactFactors[i].second].outDataNames[0];
      layerIdsIncluded = to_string(impactFactors[i].second);
    } else {
      layers += ",";
      layers += (char *)orgTIDLNetStructureOrig->TIDLPCLayers[impactFactors[i].second].outDataNames[0];
      layerIdsIncluded += ",";
      layerIdsIncluded += to_string(impactFactors[i].second);
    }
    strcpy((char *)gParams.outputFeature16bitNamesList,(char *)layers.c_str());
    TIDL_updateConfigParameters(&gParams,1,1,0,-1,-1,1); //Calibration option set to 0 since only perf data required, not accuracy
    float networkTime = TIDL_quantizeInferPerfsim(&gParams, layerIndex);
    /* Aborting here as we can't propagate the error up*/
    if(isnan(networkTime))
    {
      TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("");
    }
    cur_mixed_precision_factor = (networkTime / networkTimeFor8Bit);
    if(cur_mixed_precision_factor > mixedPrecisionFactor) /* Do not include current layer for mixed precision */
    {
      layers = layersOld;
      layerIdsIncluded = layerIdsIncludedOld;
    }
    else
    {
      layersOld = layers;
      layerIdsIncludedOld = layerIdsIncluded;
    }
    loggerFile << "data id of layer added in " << iteration << " = " << orgTIDLNetStructure.TIDLPCLayers[impactFactors[i].second].outData[0].dataId << endl;
    loggerFile << "data id of layer added in " << iteration << " = " << (char *)orgTIDLNetStructure.TIDLPCLayers[impactFactors[i].second].outDataNames[0] << endl;
    loggerFile << "overall added layers dataIds " << layerIdsIncluded << endl;
    loggerFile << "overall added layers names " << layers << endl;
    loggerFile << "current mixed precision factor " << cur_mixed_precision_factor << endl;
    i++;
    iteration++;
    /* Restore original model/parameters/variables for subsequent import */
    TIDL_restoreOrgNetAndConfigParams(orgTIDLNetStructureOrig, configParamsOrig);
  }
  return layers;
}


/** Specify layer at which distance metric to be calculated
 * Classification : Layer before softmax (If softmax last layer of network)
 * Object detection : Convolution heads - inputs of Detection output layer
 * Segmentation : Layer before argmax layer (if argmax last layer of network)
 * Else, use last layer of network
 * */
int32_t TIDL_getLayerIndicesToBeUsedForCalculatingDistanceMetric(int32_t layerIndex, std::vector<int32_t>&outputLayerIndices)
{
  for(int i = orgTIDLNetStructure.numLayers - 1; i >= 0; i--)
  {
    int32_t  idx_next;

    /* Check for OD networks */
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_DetectionOutputLayer)
    {
      // [Doubt] if it is TIDL_DetectionOutputLayer then also we are checking its consumer? : Return type as int32_t and change to vector by reference (parameter)
      int32_t idx_next = tidl_getOutLayer(orgTIDLNetStructure, layerIndex, orgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId);
      if(idx_next == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_LayerPC_t &nextLayer = orgTIDLNetStructure.TIDLPCLayers[idx_next];
      if(((nextLayer.layerType == TIDL_DataLayer) && (nextLayer.numOutBufs == -1)) || (nextLayer.layerType == TIDL_OdOutputReformatLayer))
      {
        for(int j = 0; j < orgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
        {
          outputLayerIndices.push_back(orgTIDLNetStructure.TIDLPCLayers[i].inData[j].dataId);
        }
      }
    }
    /* Check for classification networks */
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_SoftMaxLayer)
    {
      idx_next = tidl_getOutLayer(orgTIDLNetStructure, layerIndex, orgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId);
      if(idx_next == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_LayerPC_t &nextLayer = orgTIDLNetStructure.TIDLPCLayers[idx_next];
      if((nextLayer.layerType == TIDL_DataLayer) && (nextLayer.numOutBufs == -1))
      {
        for(int j = 0; j < orgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
        {
          outputLayerIndices.push_back(orgTIDLNetStructure.TIDLPCLayers[i].inData[j].dataId);
        }
      }
    }
    /* Check for segmentation networks */
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_ArgOpLayer)
    {
      idx_next = tidl_getOutLayer(orgTIDLNetStructure, layerIndex, orgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId);
      if(idx_next == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_LayerPC_t &nextLayer = orgTIDLNetStructure.TIDLPCLayers[idx_next];
      if((nextLayer.layerType == TIDL_DataLayer) && (nextLayer.numOutBufs == -1))
      {
        for(int j = 0; j < orgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
        {
          outputLayerIndices.push_back(orgTIDLNetStructure.TIDLPCLayers[i].inData[j].dataId);
        }
      }
    }
  }
  if(outputLayerIndices.size() == 0) //None of above present, then use all the output layers of network for distance metric computation
  {
    for(int i = 0; i < orgTIDLNetStructure.numLayers; i++)
    {
      int32_t  idx_next = tidl_getOutLayer(orgTIDLNetStructure, layerIndex, orgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId);
      if(idx_next == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_LayerPC_t &nextLayer = orgTIDLNetStructure.TIDLPCLayers[idx_next];
      if((nextLayer.layerType == TIDL_DataLayer) && (nextLayer.numOutBufs == -1))
      {
        outputLayerIndices.push_back(i);
      }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** This function computes the distance metric and perfsim performance time needed for computing impact factors
 * Returns the distance metric and networkTime
*/
float TIDL_getDistanceAndPerfData(sTIDL_OrgNetwork_t * orgTIDLNetStructureOrig, tidl_import_config * configParamsOrig, std::string modeSuffix,
                               char inConfigFileNameOrig[], int32_t layerIndex, char outDirName[], std::string netFileName, std::string ioFileName, std::vector<int32_t> outputLayerIndices,
                               vector<vector<float>> error32BitVector, float * networkTime,int i)
{
  TIDL_restoreOrgNetAndConfigParams(orgTIDLNetStructureOrig, configParamsOrig);
  /* Restore net and io names set in main function for auto mixed precision */
  strcpy((char *)gParams.outputNetFile, netFileName.c_str());
  strcpy((char *)gParams.outputParamsFile, ioFileName.c_str());

  strcpy(inConfigFilename, TIDL_augmentCharArrayWithSuffix(inConfigFileNameOrig, modeSuffix).c_str());
  TIDL_updateConfigParameters(&gParams, gParams.biasCalibrationIterations/4, gParams.numFramesBiasCalibration/4, 7, 8, 8, gParams.numFramesBiasCalibration/4);
  if(i != -1){
    strcpy((char *)gParams.outputFeature16bitNamesList, (char *)orgTIDLNetStructure.TIDLPCLayers[i].outDataNames[0]);
  }

  float netTime = TIDL_quantizeInferPerfsim(&gParams, layerIndex, true);
  if(isnan(netTime))
  {
    return NAN;
  }
  *networkTime = netTime;

  // get distance metric
  vector<vector<float>> errorVector = TIDL_readAndNormalizeTraces(modeSuffix,outDirName,outputLayerIndices);
  float distance = TIDL_calculateErrorByVectors(error32BitVector,errorVector,"MSE");

  return distance;
}

/** Main function for performing automated mixed precision */
int32_t TIDL_executeAutomatedMixedPrecision(uint32_t layerIndex, sTIDL_OrgNetwork_t * orgTIDLNetStructureOrig, tidl_import_config * configParamsOrig)
{
  int systemReturnCode;
  /* Check if value of mixed precision factor is valid */
  if(gParams.mixedPrecisionFactor <= 1)
  {
    TIDL_GLOBAL_REPORT_WARNING("Mixed precision factor <= 1 is not a valid argument, proceeding without automated mixed precision");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }

  std::vector<int32_t> outputLayerIndices;
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_getLayerIndicesToBeUsedForCalculatingDistanceMetric(layerIndex, outputLayerIndices), "");

  /* Take backup of inConfigFilename */
  char inConfigFileNameOrig[2*FILE_NAME_SIZE];
  strcpy(inConfigFileNameOrig, inConfigFilename);

  char outDirName[500];

  /** Create a separate folder for saving auto mixed precision related artifacts */
  strcpy(outDirName, (char *)gParams.outputNetFile);
  strcat(outDirName, "_autoMixedPrecArtifacts");

  std::string outDirNameStr(outDirName);
  /* Delete auto mixed precision artifacts folder if present */
  system(("rm -rf " + outDirNameStr).c_str());
  systemReturnCode = system(("mkdir " + outDirNameStr).c_str());
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("System command failed with return code : %d", systemReturnCode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  /* Update net and params file names to be inside the above create auto mixed precision folder
     Since the above folder is specific to the model, it is ok not to have model name as part of net and params file names */
  systemReturnCode = system(("touch " + outDirNameStr + "/tidl_net_model.bin").c_str()); /* getAbsPath requires actual file to be present in folder */
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("System command failed with return code : %d", systemReturnCode);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  strcpy((char *)gParams.outputNetFile, (outDirNameStr + "/tidl_net_model.bin").c_str());
  strcpy((char *)gParams.outputParamsFile, (outDirNameStr + "/tidl_io_model_").c_str());
  std::string netFileName = (char *)gParams.outputNetFile;
  std::string ioFileName = (char *)gParams.outputParamsFile;


  ofstream loggerFile;
  loggerFile.open(outDirNameStr + "/impact_factor_calculation.txt");
  /***** Run entire model in FLOATING POINT mode *****/

  strcpy(inConfigFilename, TIDL_augmentCharArrayWithSuffix(inConfigFileNameOrig, "_float").c_str());
  sTIDL_Network_t * tidlNetStructureFloat = new sTIDL_Network_t;
  TIDL_updateConfigParameters(&gParams,-1,-1,-1,-1,-1,gParams.numFramesBiasCalibration/4);
  gParams.writeTraceLevel = 3;
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_quantStatsFixedOrFloat(&orgTIDLNetStructure,
                                  tidlNetStructureFloat,
                                  &gParams,
                                  STATS_COLLECTION_FLOAT), "");
  vector<vector<float>> error32BitVector = TIDL_readAndNormalizeTraces("_float",outDirName,outputLayerIndices);
  delete tidlNetStructureFloat;

  /****** Run model with ALL LAYERS IN 8 BITS *****/

  std::string modeSuffix = "_8bit";
  float networkTimeFor8Bit = 0.0;
  float errorIn8Bit = TIDL_getDistanceAndPerfData(orgTIDLNetStructureOrig, configParamsOrig, modeSuffix, inConfigFileNameOrig, layerIndex,
                            outDirName, netFileName, ioFileName, outputLayerIndices, error32BitVector, &networkTimeFor8Bit,-1);
  if(isnan(errorIn8Bit))
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  loggerFile << "MODE: 8 bit" << endl;
  loggerFile << "Error = " << errorIn8Bit << endl;
  loggerFile << "-----------------------------------------------" << endl;

  /***** Loop over all layers to run mixed precision with individual layers in 16 bit precision ****/

  vector<pair<float,int>> impactFactors;
  loggerFile << "Mixed Precision Mode " << endl;
  for(int i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    if((orgTIDLNetStructure.TIDLPCLayers[i].layerType != TIDL_DataLayer) && (orgTIDLNetStructure.TIDLPCLayers[i].layerType != TIDL_DataConvertLayer)) // don't set data layer to 16 bit precision
    {
      std::string modeSuffix = "_MP_layer_" + to_string(i);
      float networkTimeForMPLayer = 0.0;
      float errorInMPLayer = TIDL_getDistanceAndPerfData(orgTIDLNetStructureOrig, configParamsOrig, modeSuffix, inConfigFileNameOrig, layerIndex,
                                outDirName, netFileName, ioFileName, outputLayerIndices, error32BitVector, &networkTimeForMPLayer,i);
      if(isnan(errorInMPLayer))
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      //Calculate impact factor for layer i
      float eps = 1e-5;
      float impactFactor = (errorIn8Bit - errorInMPLayer + eps)/ ((networkTimeForMPLayer - networkTimeFor8Bit + eps) * 1e-6); // network time converted to seconds
      loggerFile << "Data Id = " << orgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId << endl;
      loggerFile << "Layer Name = " << (char *)orgTIDLNetStructure.TIDLPCLayers[i].outDataNames[0] << endl;
      loggerFile << "Error = " << errorInMPLayer << endl;
      loggerFile << "Impact factor = " << impactFactor << endl;
      loggerFile << "--------------------------------------------------" << endl;
      impactFactors.push_back(make_pair(impactFactor,i));
    }
  }

  //Algorithm to find layers to be put in 16 bit
  string layersToKeepIn16Bit = TIDL_getLayersToKeepIn16BitByImpactFactors(orgTIDLNetStructureOrig, configParamsOrig, impactFactors,
                                                                            layerIndex, networkTimeFor8Bit, outDirNameStr, netFileName, ioFileName);
  /* Restore original model/parameters/variables for subsequent import */
  TIDL_restoreOrgNetAndConfigParams(orgTIDLNetStructureOrig, configParamsOrig);

  // Restore the inConfigFilename
  strcpy(inConfigFilename, inConfigFileNameOrig);

  //Populate layers to be put in 16 bit for subsequent final import
  strcpy((char *)gParams.outputFeature16bitNamesList,(char *)layersToKeepIn16Bit.c_str());

  // Deletes all the artifacts of automated mixed precision algorithm
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Removing mixed precision artifacts");
  std:string configFileName = getFileNameFromPath(inConfigFileNameOrig);
  /* Copy log files to another location before deleting artifacts */
  system(("mv " + outDirNameStr + "/layers_selection.txt " + outDirNameStr + "/../" + configFileName + "_layers_selection.txt").c_str());
  system(("mv " + outDirNameStr + "/impact_factor_calculation.txt " + outDirNameStr + "/../" + configFileName + "_impact_factor_calculation.txt").c_str());
  /* Delete the auto mixed prec artifacts folder */
  system(("rm -rf " + outDirNameStr).c_str());

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_mergeReshapeLayers(sTIDL_OrgNetwork_t& orgTIDLNetStructure)
{
  /*
   * This function merges consecutive reshape layers, only the last reshape has effect, so it removes any
   * prior reshape layers if present.
  */
  int32_t numLayers = orgTIDLNetStructure.numLayers, i1 = 0;
  int32_t inIdx = 0, innerIdx = 0, merged = 0;
  sTIDL_LayerPC_t *TIDLPCLayersOut;
  for(int32_t i=0;i<numLayers;i++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_ReshapeLayer)
    {
      inIdx = tidl_getInLayer(orgTIDLNetStructure, numLayers, orgTIDLNetStructure.TIDLPCLayers[i].inData[0].dataId);
      if(orgTIDLNetStructure.TIDLPCLayers[inIdx].layerType == TIDL_ReshapeLayer)
      {
        /* Consecutive reshape layers can be merged */
        innerIdx = tidl_getInLayer(orgTIDLNetStructure, numLayers, orgTIDLNetStructure.TIDLPCLayers[inIdx].inData[0].dataId);
        std::vector<int32_t> out_layers =  tidl_getOutLayers(orgTIDLNetStructure, orgTIDLNetStructure.numLayers, orgTIDLNetStructure.TIDLPCLayers[inIdx].outData[0].dataId);

        int32_t merge = 0;
        if (out_layers.size() == 1)
        {
          merge = 1;
        }
        else if (out_layers.size() > 1)
        {
          /*
           * If reshape has multiple consumers, it can still be merged if all
           * consumer are also reshape layer
           */
          merge = 1;
          for (int32_t outLayerIdx : out_layers)
          {
            TIDLPCLayersOut = &orgTIDLNetStructure.TIDLPCLayers[outLayerIdx];
            if(TIDLPCLayersOut->layerType != TIDL_ReshapeLayer)
            {
              merge = 0;
              break;
            }
          }
        }
        if (merge == 1 && innerIdx != -1)
        {
          for (int32_t outLayerIdx : out_layers)
          {
            TIDLPCLayersOut = &orgTIDLNetStructure.TIDLPCLayers[outLayerIdx];

            for (i1 = 0; i1 < TIDLPCLayersOut->numInBufs; i1++)
            {
              if (TIDLPCLayersOut->inData[i1].dataId == orgTIDLNetStructure.TIDLPCLayers[inIdx].outData[0].dataId)
              {
                TIDLPCLayersOut->inData[i1] = orgTIDLNetStructure.TIDLPCLayers[innerIdx].outData[0];
                strcpy((char*)TIDLPCLayersOut->inDataNames[i1], (char*)orgTIDLNetStructure.TIDLPCLayers[innerIdx].outDataNames[0]);
                if(orgTIDLNetStructure.TIDLPCLayers[inIdx].layerPCParams.reshapeParams.isReInterpret == 1)
                {
                  TIDLPCLayersOut->layerPCParams.reshapeParams.isReInterpret = 1;
                }
              }
            }
          }
          orgTIDLNetStructure.TIDLPCLayers[inIdx].numInBufs = -1;
          orgTIDLNetStructure.TIDLPCLayers[inIdx].numOutBufs = -1;
          merged = 1;
        }
      }
    }
  }

  if(merged)
  {
    tidl_removeMergedLayersFromNet(&orgTIDLNetStructure, &tempTIDLNetStructure);
    tidl_sortLayersInProcOrder(&orgTIDLNetStructure, &tempTIDLNetStructure, orgTIDLNetStructure.numLayers);
  }

  return merged;
}

int32_t tidl_checkParamsForDCLayers (sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t dataConvertIdx)
{
  /* Do not remove the data convert layer if it does a data type conversion*/
  int32_t paramCheck = 1;
  int32_t inLyrIdx  = tidl_getInLayer (pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].inData[0].dataId);

  if (inLyrIdx != -1)
  {
    if (pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].outData[0].elementType != pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].outData[0].elementType ||
        pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].outData[0].tensorScale != pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].outData[0].tensorScale ||
        pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].outData[0].tensorZeroPoint != pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].outData[0].tensorZeroPoint)
    {
      /* Data convert changing the data, do not remove it*/
      paramCheck = 0;
    }
    
    if (pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].layerParams.dataConvertParams.type != TIDL_DC_TYPE_INTERMEDIATE)
    {
      if (pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].outData[0].roundBits != pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].outData[0].roundBits)
      {
        /* Data convert changing the data, do not remove it*/
        paramCheck = 0;        
      }

    }
  }

  return paramCheck;
}

int32_t tidl_checkIODCLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t dataConvertIdx)
{
  /** Do not remove the Input and Output DataConvert when associated with reshape ayers
   *  Reshape layers cannot be the first/last layers as they have forceInPlace constraint and the last layer output 
   *  go to DDR
  */
  int32_t IOCheck   = 1;
  int32_t outLyrIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].outData[0].dataId);
  int32_t inLyrIdx  = tidl_getInLayer (pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].inData[0].dataId);
  if(outLyrIdx == -1 || inLyrIdx == -1)
  {
    return 0;
  }

  if ((pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].layerType  == TIDL_DataLayer && 
       pOrgTIDLNetStructure.TIDLPCLayers[outLyrIdx].layerType == TIDL_ReshapeLayer) || 
      (pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].layerType  == TIDL_ReshapeLayer &&
       pOrgTIDLNetStructure.TIDLPCLayers[outLyrIdx].layerType == TIDL_DataLayer))
  {
    IOCheck = 0;
  }
  return IOCheck;
}
int32_t tidl_checkLayoutForDCLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t dataConvertIdx)
{
  /** Remove the DC Layers if there is no data movement because of the layout change*/
  int32_t layoutCheck = 1, notOne = 0;
  sTIDL_LayerPC_t& layer = pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx];

  if (layer.layerParams.dataConvertParams.layout != layer.layerParams.dataConvertParams.outLayout)
  {
    /* remove dataconvert if input is flattened */
    for (int32_t i=1;i<TIDL_DIM_MAX;i++)
    {
      if (layer.inData[0].dimValues[i] != 1 )
        notOne++;
    }
    if (notOne > 1)
    {
      layoutCheck = 0;
    }
  }
  return layoutCheck;
}
int32_t tidl_checkPadForDCLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t dataConvertIdx, sTIDL_Network_t *tidlNet)
{
  /** Remove the DataConvert Layers if there is no Pad Addition/Removal*/
  int32_t padCheck = 1;
  int32_t outDataId = pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].outData[0].dataId;
  int32_t tidlLayerIdx = -1;
  for(int32_t i=0;i<tidlNet->numLayers;i++)
  {
    if(tidlNet->TIDLLayers[i].outData.dataId == outDataId)
    {
      tidlLayerIdx = i;
      break;
    }
  }
  if (tidlLayerIdx == -1)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Cannot find equivalent data convert in tidlNet");
    return 0;
  }
  int32_t inLayerIdx  = tidl_getInLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[tidlLayerIdx].inData[0]);
  int32_t outLayerIdx = tidl_getOutLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[tidlLayerIdx].outData.dataId);
  if(inLayerIdx == -1 || outLayerIdx == -1)
  {
    return 0;
  }
  //int32_t inPad  = tidlNet->TIDLLayers[inLayerIdx].outData.padH != 0 || tidlNet->TIDLLayers[inLayerIdx].outData.padW != 0;
  //int32_t outPad = tidlNet->TIDLLayers[outLayerIdx].outData.padH != 0 || tidlNet->TIDLLayers[outLayerIdx].outData.padW != 0;

  /*Check for the previous Layer if its a Reshape or transpose layer then in this case we will check for the current DC layer to have a padding or not in its outData*/
  if(tidlNet->TIDLLayers[inLayerIdx].layerType == TIDL_ReshapeLayer || tidlNet->TIDLLayers[inLayerIdx].layerType == TIDL_TransposeLayer){
    bool HPad =  tidlNet->TIDLLayers[tidlLayerIdx].outData.padH;
    bool WPad =  tidlNet->TIDLLayers[tidlLayerIdx].outData.padW;
    if (HPad || WPad)
    {
      padCheck = 0;
    }
  }
  /*Check for the Next Layer if its a Reshape or transpose layer then we should check for the inLayer whther it contains any padding or not in its outdata*/
  else if(tidlNet->TIDLLayers[outLayerIdx].layerType == TIDL_ReshapeLayer || tidlNet->TIDLLayers[outLayerIdx].layerType == TIDL_TransposeLayer){
    bool HPad =  tidlNet->TIDLLayers[inLayerIdx].outData.padH;
    bool WPad =  tidlNet->TIDLLayers[inLayerIdx].outData.padW;
    if (HPad || WPad)
    {
      padCheck = 0;
    }
  }
  else{
    padCheck = 0;
  }
  
  return padCheck;
}

/*this function removes a layer from a network (this is only used for dataconvert currently not tested for multi-input layers)*/
void tidl_removeLayerFromNet(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIdx)
{
  sTIDL_LayerPC_t *TIDLPCLayersOut;
  sTIDL_LayerPC_t& layer = pOrgTIDLNetStructure.TIDLPCLayers[layerIdx];

  std::vector<int32_t> out_layers =  tidl_getOutLayers(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, layer.outData[0].dataId);

  for ( int32_t outLayerIdx: out_layers)
  {
    TIDLPCLayersOut = &pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx];

    for (int32_t i = 0; i < TIDLPCLayersOut->numInBufs; i++)
    {
      if (TIDLPCLayersOut->inData[i].dataId == layer.outData[0].dataId)
      {
        TIDLPCLayersOut->inData[i].dataId = layer.inData[0].dataId;
        strcpy((char *)TIDLPCLayersOut->inDataNames[i], (char *)layer.inDataNames[0]);
      }
    }
  }

  layer.numInBufs = -1;
  layer.numOutBufs = -1;

  return;
}

int32_t tidl_checkSpecialCasesForDCLayers(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t dataConvertIdx)
{
  /** Don't remove data convert layers if removal creates a scenario where a convolution of strideH/W > 1 is followed by a reshape layer
   *  Network compiler has some limitation on supporting the above scenario, hence we need to keep the data convert layer in such cases
  */
  int32_t specialCheck = 1;
  sTIDL_LayerPC_t& layer = pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx];
  if (layer.layerParams.dataConvertParams.type == TIDL_DC_TYPE_INTERMEDIATE)
  {
    int32_t outLyrIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].outData[0].dataId);
    int32_t inLyrIdx  = tidl_getInLayer (pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[dataConvertIdx].inData[0].dataId);
    if ((pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].layerType  == TIDL_ConvolutionLayer &&
         pOrgTIDLNetStructure.TIDLPCLayers[outLyrIdx].layerType == TIDL_ReshapeLayer) &&
        (pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].layerParams.convParams.strideH > 1 ||
         pOrgTIDLNetStructure.TIDLPCLayers[inLyrIdx].layerParams.convParams.strideW > 1))
    {
      specialCheck = 0;
    }
  }

  return specialCheck;
}

int32_t tidl_inputDataConvertLayerChecks (sTIDL_Network_t * tidlNet, int32_t layerIdx)
{
  int32_t addDataConvert = false;
  /* Check if the input layer is a data layer in the network */
  int32_t inLayerIdx = tidl_getInLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[layerIdx].inData[0]);
  if (tidlNet->TIDLLayers[inLayerIdx].layerType == TIDL_DataLayer &&
      tidlNet->TIDLLayers[layerIdx].layerType == TIDL_ReshapeLayer)
  {
    addDataConvert = true;
  }
  
  int32_t inLyrChannelPitchWithoutPad = tidlNet->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_HEIGHT] *
                                    tidlNet->TIDLLayers[inLayerIdx].outData.dimValues[TIDL_DIM_WIDTH];

  if (tidlNet->TIDLLayers[inLayerIdx].outData.padW != 0 || tidlNet->TIDLLayers[inLayerIdx].outData.padH != 0 ||
      (inLyrChannelPitchWithoutPad != tidlNet->TIDLLayers[inLayerIdx].outData.pitch[TIDL_CHANNEL_PITCH]))
  {
    addDataConvert = true;
  }

  /** 
   *  Network compiler has some limitation on supporting the below scenario (convolution followed by reshape)
   *  Hence we need to add the data convert layer in such cases
  */

  if (tidlNet->TIDLLayers[layerIdx].layerType == TIDL_ConvolutionLayer && 
      (tidlNet->TIDLLayers[layerIdx].layerParams.convParams.strideH > 1 ||
       tidlNet->TIDLLayers[layerIdx].layerParams.convParams.strideW > 1))
  {
    addDataConvert = true;
  }

  return addDataConvert;
}

int32_t tidl_outputDataConvertLayerChecks (sTIDL_Network_t * tidlNet, int32_t layerIdx)
{
  int32_t addDataConvert = false;
  /* Check if the output layer is a data layer in the network */
  int32_t outLayerIdx = tidl_getOutLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[layerIdx].outData.dataId);
  if (tidlNet->TIDLLayers[outLayerIdx].layerType == TIDL_DataLayer &&
      tidlNet->TIDLLayers[layerIdx].layerType == TIDL_ReshapeLayer)
  {
    addDataConvert = true;
  }
  
  if (tidlNet->TIDLLayers[layerIdx].outData.padW != 0 || tidlNet->TIDLLayers[layerIdx].outData.padH != 0)
  {
    addDataConvert = true;
  }

  return addDataConvert;
}

int32_t TIDL_addDCLayersForPadUnsupportedLayers (sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, tidl_import_config *params, int32_t& dataIndex, sTIDL_Network_t * tidlNet)
{
  int32_t numLayers = pOrgTIDLNetStructure.numLayers;
  int32_t i1 = 0, i2 = 0, layerAdded = false;

  for (i1 = 0; i1 < numLayers; i1++)
  {
    if (padUnsupportedLayers.find(pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType) != padUnsupportedLayers.end())
    {
      sTIDL_LayerPC_t& layer = pOrgTIDLNetStructure.TIDLPCLayers[i1];
      int32_t outDataId = layer.outData[0].dataId;
      int32_t tidlLayerIdx = -1;
      for(int32_t i=0;i<tidlNet->numLayers;i++)
      {
        if(tidlNet->TIDLLayers[i].outData.dataId == outDataId)
        {
          tidlLayerIdx = i;
          break;
        }
      }
      if (tidlLayerIdx == -1)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Cannot find equivalent layer in tidlNet");
        return 0;
      }

      int32_t inLayerIdx  = tidl_getInLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[tidlLayerIdx].inData[0]);
      int32_t outLayerIdx = tidl_getOutLayer(tidlNet, tidlNet->numLayers, tidlNet->TIDLLayers[tidlLayerIdx].outData.dataId);

      if (inLayerIdx != -1)
      {
        if (tidl_inputDataConvertLayerChecks (tidlNet, tidlLayerIdx) == true)
        {
          /* Add a data convert layer to remove the padding as the consumer layer cannot handle pad */
          TIDL_addLayer (pOrgTIDLNetStructure, i1, TIDL_DataConvertLayer, &dataIndex, ADD_LAYER_BEFORE);
          sTIDL_LayerPC_t& dataConvertLayer = pOrgTIDLNetStructure.TIDLPCLayers[pOrgTIDLNetStructure.numLayers - 1];

          /* Setup the data convert layer params*/
          dataConvertLayer.layerParams.dataConvertParams.type = TIDL_DC_TYPE_INTERMEDIATE;
          dataConvertLayer.layerParams.dataConvertParams.layout = TIDL_LT_NCHW;
          dataConvertLayer.layerParams.dataConvertParams.outLayout = TIDL_LT_NCHW;

          /* Copy the quantization params from inLayer*/
          dataConvertLayer.outData[0].tensorScale = tidlNet->TIDLLayers[inLayerIdx].outData.tensorScale;
          dataConvertLayer.outData[0].tensorZeroPoint = tidlNet->TIDLLayers[inLayerIdx].outData.tensorZeroPoint;
          dataConvertLayer.outData[0].roundBits = 0;
          layerAdded = true;
        }
      }

      if (tidl_outputDataConvertLayerChecks (tidlNet, tidlLayerIdx) == true)     
      {
        /* Add a data convert layer to add the padding as the producer layer cannot handle pad */
        TIDL_addLayer (pOrgTIDLNetStructure, i1, TIDL_DataConvertLayer, &dataIndex, ADD_LAYER_AFTER);
        sTIDL_LayerPC_t& dataConvertLayer = pOrgTIDLNetStructure.TIDLPCLayers[pOrgTIDLNetStructure.numLayers - 1];

        /* Setup the data convert layer params*/
        dataConvertLayer.layerParams.dataConvertParams.type = TIDL_DC_TYPE_INTERMEDIATE;
        dataConvertLayer.layerParams.dataConvertParams.layout = TIDL_LT_NCHW;
        dataConvertLayer.layerParams.dataConvertParams.outLayout = TIDL_LT_NCHW;

        /* Copy the quantization params from outLayer*/
        dataConvertLayer.outData[0].tensorScale = tidlNet->TIDLLayers[tidlLayerIdx].outData.tensorScale;
        dataConvertLayer.outData[0].tensorZeroPoint = tidlNet->TIDLLayers[tidlLayerIdx].outData.tensorZeroPoint;
        dataConvertLayer.outData[0].roundBits = 0;
        layerAdded = true;
      }
    }
  }
  return layerAdded;
}
bool TIDL_removeDCLayersfromPCNet(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, tidl_import_config *params, sTIDL_Network_t * tidlNet)
{
  bool removed = false;
  int32_t numLayers = pOrgTIDLNetStructure.numLayers, outLayerIdx, i3 = 0;

  for (int32_t i=0;i<numLayers;i++)
  {
    if (pOrgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_DataConvertLayer)
    {
      sTIDL_LayerPC_t& layer = pOrgTIDLNetStructure.TIDLPCLayers[i];
      {
        int32_t padCheck=0, layoutCheck=0, IOCheck = 0, paramCheck = 0, specialCheck = 0;

        padCheck      = tidl_checkPadForDCLayers(pOrgTIDLNetStructure, i, tidlNet);
        layoutCheck   = tidl_checkLayoutForDCLayers(pOrgTIDLNetStructure, i);
        IOCheck       = tidl_checkIODCLayers(pOrgTIDLNetStructure, i);
        paramCheck    = tidl_checkParamsForDCLayers(pOrgTIDLNetStructure, i);
        specialCheck  = tidl_checkSpecialCasesForDCLayers(pOrgTIDLNetStructure, i);

        if(padCheck && layoutCheck && IOCheck && paramCheck && specialCheck)
        {
          std::vector<int32_t> out_layers =  tidl_getOutLayers(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, layer.outData[0].dataId);
          std::vector<int32_t> in_layers =  tidl_getInLayers_v2(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, i);
          
          /*before removing dataconvert layers we remove any multi-core crop layers added due to dataconvert for both input and output of Data Convert*/

          for(int32_t outLayerIdx: out_layers)
          {
            if (pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].layerType == TIDL_CropLayer && 
                pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].layerParams.cropParams.multiCoreMode != TIDL_NOT_MULTI_CORE)
            {
              tidl_removeLayerFromNet(pOrgTIDLNetStructure, outLayerIdx); /*removing Crop Layer in output of Data Convert*/
            }
          }

          for(int32_t inLayerIdx: in_layers)
          {
            if (pOrgTIDLNetStructure.TIDLPCLayers[inLayerIdx].layerType == TIDL_CropLayer && 
                pOrgTIDLNetStructure.TIDLPCLayers[inLayerIdx].layerParams.cropParams.multiCoreMode != TIDL_NOT_MULTI_CORE)
            {
              tidl_removeLayerFromNet(pOrgTIDLNetStructure, inLayerIdx); /*removing Crop Layer in input of Data Convert*/
            }
          }
          
          tidl_removeLayerFromNet(pOrgTIDLNetStructure, i); /* Remove the DataConvert layer from the network */
          removed = true;

        }
      }
    }
  }

  if (removed == true)
  {
    int32_t dataIndex = orgTIDLNetStructure.numLayers;
    tidl_removeMergedLayersFromNet(&pOrgTIDLNetStructure, &tempTIDLNetStructure);
    tidl_addCropLayerForMultiCore(pOrgTIDLNetStructure, orgTIDLNetStructure.numLayers, &(dataIndex), gParams.quantizationStyle, gParams.numCores);
    tidl_sortLayersInProcOrder(&pOrgTIDLNetStructure, &tempTIDLNetStructure, pOrgTIDLNetStructure.numLayers);
  }
  
  return removed;
}

bool TIDL_removeUnusedMPDCLayersfromPCNet(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, tidl_import_config *params, sTIDL_Network_t * tidlNet)
{
  bool removed = false;
  int32_t numLayers = pOrgTIDLNetStructure.numLayers, outLayerIdx, i3 = 0;

  for (int32_t i=0;i<numLayers;i++)
  {
    if (pOrgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_DataConvertLayer && pOrgTIDLNetStructure.TIDLPCLayers[i].layerParams.dataConvertParams.type == TIDL_DC_TYPE_MIXED_PRECISION)
    {
      int32_t specialMPCheck = 0;
      sTIDL_LayerPC_t& layer = pOrgTIDLNetStructure.TIDLPCLayers[i];

      const sTIDL_DataParams_t * inData = TIDL_getOutData(&pOrgTIDLNetStructure, layer.inData[0].dataId);
      const sTIDL_DataParams_t * outData = TIDL_getOutData(&pOrgTIDLNetStructure, layer.outData[0].dataId);

      if(inData->elementType == outData->elementType)
      {
        specialMPCheck = 1;
      }

      if(specialMPCheck)
      {
        std::vector<int32_t> out_layers =  tidl_getOutLayers(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, layer.outData[0].dataId);
        std::vector<int32_t> in_layers =  tidl_getInLayers_v2(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, i);
        
        /*before removing dataconvert layers we remove any multi-core crop layers added due to dataconvert for both input and output of Data Convert*/

        for(int32_t outLayerIdx: out_layers)
        {
          if (pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].layerType == TIDL_CropLayer && 
              pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].layerParams.cropParams.multiCoreMode != TIDL_NOT_MULTI_CORE)
          {
            tidl_removeLayerFromNet(pOrgTIDLNetStructure, outLayerIdx); /*removing Crop Layer in output of Data Convert*/
          }
        }

        for(int32_t inLayerIdx: in_layers)
        {
          if (pOrgTIDLNetStructure.TIDLPCLayers[inLayerIdx].layerType == TIDL_CropLayer && 
              pOrgTIDLNetStructure.TIDLPCLayers[inLayerIdx].layerParams.cropParams.multiCoreMode != TIDL_NOT_MULTI_CORE)
          {
            tidl_removeLayerFromNet(pOrgTIDLNetStructure, inLayerIdx); /*removing Crop Layer in input of Data Convert*/
          }
        }
        
        tidl_removeLayerFromNet(pOrgTIDLNetStructure, i); /* Remove the DataConvert layer from the network */
        removed = true;
      }
    }
  }

  if (removed == true)
  {
    int32_t dataIndex = orgTIDLNetStructure.numLayers;
    tidl_removeMergedLayersFromNet(&pOrgTIDLNetStructure, &tempTIDLNetStructure);
    tidl_addCropLayerForMultiCore(pOrgTIDLNetStructure, orgTIDLNetStructure.numLayers, &(dataIndex), gParams.quantizationStyle, gParams.numCores);
    tidl_sortLayersInProcOrder(&pOrgTIDLNetStructure, &tempTIDLNetStructure, pOrgTIDLNetStructure.numLayers);
  }
  
  return removed;
}

int32_t isTransposeSupportedSoftmax(int32_t inAxis,
                                      int32_t outTensorScale,
                                      int32_t targetDeviceName)
{
  int32_t isTransposeSupported = 0;

  if(inAxis == TIDL_DIM_HEIGHT)
  {
    if( ((outTensorScale/inAxis) > SAT_HI_UINT8) || (TIDL_TDA4VM == gParams.deviceName & ~TIDL_OTF_FLAG_BIT))
    {
      isTransposeSupported = 0;
    }
    else
    {
      isTransposeSupported = 1;
    }
  }
  else if(inAxis == TIDL_DIM_WIDTH)
  {
    isTransposeSupported = 0;
  }
  
  return isTransposeSupported;
}

int32_t TIDL_isSoftmaxOutputTransposeSupported (sTIDL_LayerPC_t& softMaxLayer, tidl_import_config *params)
{
  int32_t isTransposeSupported = 1;

  if (softMaxLayer.layerParams.softMaxParams.axis == TIDL_DIM_HEIGHT)
  {
    int32_t inputHeight = softMaxLayer.inData[0].dimValues[TIDL_DIM_HEIGHT];
    float32_tidl outTensorScale = softMaxLayer.outData[0].tensorScale;

    if( ((outTensorScale/inputHeight) > SAT_HI_UINT8) || (TIDL_TDA4VM == params->deviceName & ~TIDL_OTF_FLAG_BIT))
    {
      isTransposeSupported = 0;
    }
    else
    {
      isTransposeSupported = 1;
    }      
  }
  else 
  {
    isTransposeSupported = 0;
  }

  return isTransposeSupported;
}

int32_t TIDL_handleSoftmaxTranspose (sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, tidl_import_config *params, int32_t& dataIndex)
{
  int32_t status = 0, i1 = 0;
  int32_t numLayers = pOrgTIDLNetStructure.numLayers;

  for (i1 = 0; i1 < numLayers; i1++)
  {
    if (pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType == TIDL_SoftMaxLayer)
    {
      if (pOrgTIDLNetStructure.TIDLPCLayers[i1].layerParams.softMaxParams.outTranspose == 1 &&
          (TIDL_isSoftmaxOutputTransposeSupported (pOrgTIDLNetStructure.TIDLPCLayers[i1], params) == 0))
      { 
        /**
         * Softmax output side transpose cannot be supported
         * Add an explicit transpose & disable the flag
        */
        status = 1;
        pOrgTIDLNetStructure.TIDLPCLayers[i1].layerParams.softMaxParams.outTranspose = 0;
        TIDL_addLayer (pOrgTIDLNetStructure, i1, TIDL_TransposeLayer, &dataIndex, ADD_LAYER_AFTER);
        int32_t perm1[] = {0,1,2,3,5,4};
        sTIDL_LayerPC_t& transposeLayer = pOrgTIDLNetStructure.TIDLPCLayers[pOrgTIDLNetStructure.numLayers - 1];
        memcpy (transposeLayer.inData[0].dimValues, pOrgTIDLNetStructure.TIDLPCLayers[i1].inData[0].dimValues, sizeof(int32_t) * TIDL_DIM_MAX);
        memcpy (transposeLayer.outData[0].dimValues, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dimValues, sizeof(int32_t) * TIDL_DIM_MAX);
        memcpy (pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dimValues, pOrgTIDLNetStructure.TIDLPCLayers[i1].inData[0].dimValues, sizeof(int32_t) * TIDL_DIM_MAX);
        memcpy (transposeLayer.layerParams.transposeParams.perm, perm1, sizeof(int32_t) * TIDL_DIM_MAX);
      }
    }
  }

  if (status == 1)
  {
    tidl_sortLayersInProcOrder(&pOrgTIDLNetStructure, &tempTIDLNetStructure, pOrgTIDLNetStructure.numLayers);
  }

  return status;
}

/**
 * This function runs few optimizations on the network after the quantization and memory planner has run
 * Mainly because these optimizations depend on some information from quantizer & memory planner modules
*/

int32_t TIDL_runOptimizerPostMemoryPlanner(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, tidl_import_config *params, int32_t& dataIndex)
{
  int32_t status = 0;

  FILE * fp;
  int64_t modelSize = tidl_getFileSize(params->outputNetFile);
  if(modelSize == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return 0;
  }

  sTIDL_Network_t * tidlNet = (sTIDL_Network_t *)malloc(modelSize);
  if(tidlNet == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not allocate memory for model read");
    return 0;
  }
  fp = fopen((const char *)params->outputNetFile, "rb");
  if (fp)
  {
    fread(tidlNet, 1, modelSize, fp);
    fclose(fp);
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open %s", params->outputNetFile);
    return 0;
  }

  if ((gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW || gParams.modelType ==  TIDL_IMPORT_MODEL_FORMAT_TFLITE ||
            gParams.modelType ==  TIDL_IMPORT_MODEL_FORMAT_TFLITE_RT))
  {
    /* Remove data converts added for tflite networks if not necessary*/
    status |= TIDL_removeDCLayersfromPCNet(orgTIDLNetStructure, params, tidlNet);
  }
  else
  {
    /* Add data converts if needed for other frame works*/
    status |= TIDL_addDCLayersForPadUnsupportedLayers (orgTIDLNetStructure, params, dataIndex, tidlNet);
  }
  status |= TIDL_removeUnusedMPDCLayersfromPCNet(orgTIDLNetStructure, params, tidlNet);
  status |= tidl_mergeReshapeLayers(pOrgTIDLNetStructure);
  status |= TIDL_handleSoftmaxTranspose (orgTIDLNetStructure, params, dataIndex);

  if(status == 1 || (gParams.inferenceMode == TIDL_inferenceModeLowLatency && gParams.numCores > 1))
  {
    tidl_removeMergedLayersFromNet(&pOrgTIDLNetStructure, &tempTIDLNetStructure);
    tidl_sortLayersInProcOrder(&pOrgTIDLNetStructure, &tempTIDLNetStructure, pOrgTIDLNetStructure.numLayers);
 
    if(gParams.inferenceMode == TIDL_inferenceModeLowLatency && gParams.numCores > 1)
    {
      tidl_updateMultiCoreState(pOrgTIDLNetStructure, &gParams);
      /* IMP :: NOTE :: This function should be called after all layers addition/merge is complete, layer deletion post this state is till ok */
      TIDL_IMPORT_CHECK_AND_RETURN(tidl_addCropLayerForMultiCore(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, &dataIndex, gParams.quantizationStyle, gParams.numCores), "");
      tidl_sortLayersInProcOrder(&pOrgTIDLNetStructure, &tempTIDLNetStructure, pOrgTIDLNetStructure.numLayers);
    }
    dataIndex = tidl_sortDataIDsinPCNet(&pOrgTIDLNetStructure, MAKE_LAYERID_DATAID_SAME);
    TIDL_IMPORT_CHECK_AND_ABORT(updatePadAndWriteModel(&pOrgTIDLNetStructure, tidlNet, params), "");
  }

  if (tidlNet != NULL)
  {
    free(tidlNet);
  }

  return status;
}


// This function post-processes the network after it has been imported and optimized.
int32_t TIDL_import_backend(uint32_t layerIndex, int32_t dataIndex)
{
  /* At this point model is frozen */
  int32_t i;
  /* If user has not set any value for raw element type then use the default
  which is same as input element type */
  int32_t num_inps = 0;

  /*Below code find the total number of inputs to the network*/
  for (i = 0; i < layerIndex; i++)
  {
    if((orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_DataLayer)&&(orgTIDLNetStructure.TIDLPCLayers[i].numInBufs == -1))
    {
      num_inps++;
    }
  }

  /*If user has forgotten to provide raw element type then below code assumes it be same as element type.
    Doing this just for number of inputs, not for max number of inputs
  */
  for ( i = 0; i < num_inps; i++)
  {
    if ( ( gParams.inFileFormat == 1 ) || ( gParams.inFileFormat == 3 ) )
    {
      if ( gParams.rawDataInElementType[i] == -1 )
      {
        gParams.rawDataInElementType[i]   = gParams.inElementType[i];
        TIDL_GLOBAL_REPORT_WARNING("Raw data format used is same as inElementType %d, as it was not provided by user ", gParams.inElementType[i]);
      }
    }
  }

  /* Below automated mixed precision algorithm finds the layers to be computed in 16 bits and sets as part of gParams.outputFeature16bitNamesList
   so final import is done with this setting */

  if(gParams.numParamBits < 16)
  /** Automated mixed precision not needed if running floating point mode -
   * this is protected to prevent running MP when doing subgraph input/output layers data for OSRT
   * */
  {
    if(gParams.mixedPrecisionFactor != -1) /* automated mixed precision not executed if mixedPrecisionFactor is not defined i.e. -1 */
    {
      /* Take backup of original gParams and orgTIDLNetStructure before running automated mixed precision pass */
      tidl_import_config configParamsOrig = gParams;
      sTIDL_OrgNetwork_t * orgTIDLNetStructureOrig = new sTIDL_OrgNetwork_t;
      *orgTIDLNetStructureOrig = orgTIDLNetStructure;
      TIDL_allocAndCopyModelParams(orgTIDLNetStructureOrig, &orgTIDLNetStructure, orgTIDLNetStructure.numLayers);

      /* Execute the algorithm */
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_executeAutomatedMixedPrecision(layerIndex, orgTIDLNetStructureOrig, &configParamsOrig), "");

      delete orgTIDLNetStructureOrig;
    }
    /* Needs review on when exactly we want to abort if this function fails */
    TIDL_importBitDepthProtoTxt(&orgTIDLNetStructure, &gParams);
  }

  /* Run quantization and perfsim based on modified gParams */
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_import_quantize(layerIndex), "");
  
  if(gParams.numParamBits != 32U)
  {
    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Quantization & Calibration Completed]");
  }
  int32_t dimcheck = tidlModelTensorDimCheck(orgTIDLNetStructure);

  /* Save calib run's copy.  perfsim run could update network layer ordering */
  if (gParams.modelType == 4 /* TVM Relay */)
    TIDL_IMPORT_CHECK_AND_RETURN(tidlWriteTensorNamesToFile(&gParams, "_calib"), "");
  if(gParams.executeNetworkCompiler && dimcheck)
  {
    /** We run NC multiple times under certain conditions. Supertiling pass takes time and is required only for the final NC run
     * Below code disables ST first, runs NC, re-runs based on conditions, and the end, if supertiling is indeed enabled, runs
     * a final NC pass with ST
     * This is more optimal -- Without this change if all re-runs below happen, time taken = (N + ST) + (N + ST) + (N + ST) = 3N + 3ST
     * is now replaced by (N) + (N) + (N) + (N + ST) = 4N + ST ...
     * effectively 1 additional core NC pass saving 2 ST passes
    */

    /* Disable ST and back up original params if ST is enabled */
    int32_t enableHighResOptimization, compileConstraintsFlag;
    uint8_t fileGrpInfo[TIDL_STRING_SIZE];
    int32_t isSupertilingEnabled = enableOrDisableST(&enableHighResOptimization, &compileConstraintsFlag, fileGrpInfo, &gParams, 0);

    /* First/base NC call */
    orgTIDLNetStructure.NCExecMode = GET_PAD_INFO;
    TIDL_IMPORT_CHECK_AND_RETURN(tidlRunPerfSimTool(&gParams, orgTIDLNetStructure.NCExecMode), "");
    orgTIDLNetStructure.NCExecMode = NORMAL_EXEC;
    int32_t isNcExecuted = 0;
    if(((gParams.deviceName & TIDL_OTF_FLAG_BIT) == TIDL_OTF_FLAG_BIT) && gParams.enableOtfPad == 0)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Rerunning network compiler...");
      gParams.deviceName &= 0xFF;
      TIDL_IMPORT_CHECK_AND_RETURN(tidlRunPerfSimTool(&gParams, orgTIDLNetStructure.NCExecMode), "");
      tidlRunGraphVizTool(&gParams);
      isNcExecuted = 1;
    }
    
    if(TIDL_runOptimizerPostMemoryPlanner(orgTIDLNetStructure, &gParams, dataIndex) == 1)
    {
      //Removed Data Convert Layer, rerunning NC
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Rerunning network compiler...");
      TIDL_IMPORT_CHECK_AND_RETURN(tidlRunPerfSimTool(&gParams, orgTIDLNetStructure.NCExecMode), "");
      tidlRunGraphVizTool(&gParams);
      isNcExecuted = 1;
    }

    /* Run one final NC pass with ST if ST is enabled */
    if(isSupertilingEnabled == 1)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nRerunning network compiler with high resolution optimization...");
      /* Re-enable ST */
      enableOrDisableST(&enableHighResOptimization, &compileConstraintsFlag, fileGrpInfo, &gParams, 1);
      TIDL_IMPORT_CHECK_AND_RETURN(tidlRunPerfSimTool(&gParams, orgTIDLNetStructure.NCExecMode), "");
      isNcExecuted = 1;
    }
    
    /*Executing NC, if it is not executed yet.*/
    if(isNcExecuted == 0){
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Rerunning network compiler...");
      TIDL_IMPORT_CHECK_AND_RETURN(tidlRunPerfSimTool(&gParams, orgTIDLNetStructure.NCExecMode), "");
      tidlRunGraphVizTool(&gParams);
      isNcExecuted = 1;
    }
  }
  // What about else part?? when dimcheck is false what happens?
  TIDL_IMPORT_CHECK_AND_RETURN(tidlWriteTensorNamesToFile(&gParams, ""), "");
  tidlRunGraphVizTool(&gParams);
  if (gParams.partialInitDuringCompile != 0)
     tidlAddPersistentData(&gParams);
  if (gParams.debugTraceLevel > 0)
     tidlRunModelDumpTool(&gParams);

  int errorCount = tidlModelCheck(&gParams, &orgTIDLNetStructure);

  if(strcmp((char*)gParams.negativeTestUpdate, "") != 0)
  {
    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Cyan, "Overwriting Compiled Model");
    char command[500] = "../testAutomation/tidl_safety/out/tidl_model_corrupt.out ";
    strcat(command, (char*)gParams.negativeTestUpdate);
    strcat(command, " ");
    strcat(command, (char*)gParams.outputNetFile);
    system(command);
  }

  TIDL_freeModelParams(&orgTIDLNetStructure, layerIndex);

  return (errorCount > 0)? TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL : TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
