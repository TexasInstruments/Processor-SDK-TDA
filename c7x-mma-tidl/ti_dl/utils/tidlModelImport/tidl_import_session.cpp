/*
*
* Copyright (c) {2025} Texas Instruments Incorporated
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

#include "ti_dl.h"
#include "itidl_ti.h"
#include "tidl_import_api.h"
#include "tidl_import_config.h"
#include "tidl_import_diag.h"
#include "tidl_import_common.h"
#include "tidl_import_session.h"

#include <fstream>
#include <cstring>
#include <cmath>

template <typename T>
void convertToFloat(float *dst, T *src, size_t size)
{
  for(int32_t i = 0; i < size; i++)
  {
    dst[i] = (float)src[i];
  }
}

int32_t handleInData(tidl_import_config *params, sTIDL_OrgNetwork_t  &orgTIDLNetStructure, int32_t currFrame, calibTensor *in[], size_t size)
{
	int32_t status = 0;
  FILE *fp = NULL;

  // If inData is set explicitly, do nothing
  if (strcmp((char *)params->inData, "") != 0)
  {
    // Validate if inData exists, if not we will use data from input tensors
    fp = fopen((const char*)params->inData, "rb");
    if (fp != NULL)
    {
      fclose(fp);
      return status;
    }
    TIDL_GLOBAL_REPORT_WARNING("inData (%s) does not exist. Using user provided inputs\n", (char *)params->inData);
  }

  // If inData is not provided or does not exist, check if user provided input
  if (in == nullptr || size == 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("No Input provided for calibration");
    status = -1;
    return status;	
  }

  // Check if enough inputs are provided
  int32_t numInputs = 0;
  for (int32_t i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_DataLayer &&
       orgTIDLNetStructure.TIDLPCLayers[i].numInBufs <= 0 &&
       orgTIDLNetStructure.TIDLPCLayers[i].numOutBufs > 0)
    {
      // Input layers
      numInputs++;
    }
  }

  if (numInputs == 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("No inputs found in the provided model");
    return -1;
  }

  if (size < numInputs)
  {
    TIDL_GLOBAL_REPORT_ERROR("Not enough input data is provided for calibration");
    return -1;
  }

  char tempInData[FILE_NAME_SIZE];
  char outDirName[FILE_NAME_SIZE];

  getAbsPath((char *)params->outputNetFile, outDirName);
  getDirFromPath(outDirName);
  sprintf((char *)tempInData, "%s/calib_raw_data.bin", (char *)outDirName);

  if (currFrame == 1)
  {
    remove(tempInData);
    fp = fopen(tempInData, "wb+");
  }
  else
  {
    fp = fopen(tempInData, "a");
  }

  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open file (%s) to save the input data", tempInData);
    return -1;
  }

  for (int32_t i = 0; i < size; i++)
  {
    calibTensor* tensor = in[i];
    size_t tensorSize = 1;
    for (int32_t j = 0; j < TIDL_DIM_MAX; j++)
    {
      tensorSize *= tensor->dimValues[j];
    }

    float* scratch_mem = (float*)malloc(sizeof(float) * tensorSize);
    if (!scratch_mem)
    {
      TIDL_GLOBAL_REPORT_ERROR("Could not allocate memory to dump the input data");
      fclose(fp);
      return -1;
    }

    // Convert input to float
    switch (tensor->elementType)
    {
      case TIDL_UnsignedChar:
        convertToFloat((float *)scratch_mem, (uint8_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_SignedChar:
        convertToFloat((float *)scratch_mem, (int8_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_UnsignedShort:
        convertToFloat((float *)scratch_mem, (uint16_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_SignedShort:
        convertToFloat((float *)scratch_mem, (int16_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_UnsignedWord:
        convertToFloat((float *)scratch_mem, (uint32_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_SignedWord:
        convertToFloat((float *)scratch_mem, (int32_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_UnsignedDoubleWord:
        convertToFloat((float *)scratch_mem, (uint64_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_SignedDoubleWord:
        convertToFloat((float *)scratch_mem, (int64_t *)tensor->ptr, tensorSize);
        break;
      case TIDL_SinglePrecFloat:
        convertToFloat((float *)scratch_mem, (float *)tensor->ptr, tensorSize);
        break;
      default:
        break;
    }
    fwrite((float *)scratch_mem, 1, tensorSize * sizeof(float), fp);
    free(scratch_mem);
    scratch_mem = NULL;
  }
  fclose(fp);

  if (currFrame == 1)
  {
    params->inFileFormat = 1; // Indicating binary file
    if (params->numFrames == -1)
    {
      params->numFrames = 1;
    }
  }

  if (currFrame == gParams.numFrames)
  {
    strcpy((char*)params->inData, tempInData);
    for(int i = 0; i < numInputs; i++)
    {
      params->rawDataInElementType[i] = TIDL_SinglePrecFloat; //indicating floating point raw data
    }
  }

	return status;
}

extern "C"
{
int32_t TIDL_createImportSession(importState *state, tidl_import_config *params)
{

  // Initialize data before importing
  memset(&orgTIDLNetStructure, 0, sizeof(sTIDL_OrgNetwork_t));
  state->dataIndex = 0;
  state->layerIndex = 0;
  state->currFrame = 0;

  // Parse and copy to gParams if tidl_import_config is provided
  if (params != nullptr)
  {

    // Set log file name if provided
    if (((const char*)params->logFileName)[0] != '\0')
    {
      gDiags.setLogFile((const char*)params->logFileName);
    }

    // Set isQuantParamsToBeExported flag is quantParamsPrototxtFile is given
    std::string quantParamsPrototxtFile((char*)params->quantParamsPrototxtFile);
    if(quantParamsPrototxtFile != "" )
    {
      FILE* fd = fopen((char*)params->quantParamsPrototxtFile, "r");
      if(fd == NULL)
      {
        params->isQuantParamsToBeExported = 1;
      }
      else
      {
        params->isQuantParamsToBeExported = 0;
        fclose(fd);
      }
    }
    else
    {
      params->isQuantParamsToBeExported = 0;
    }

    // Parse deviceName and/or numCores from config if not provided
    if (params->deviceName < 0 || params->numCores < 1)
    {
      std::ifstream perfSimConfig((char *)params->perfSimConfig);

      if (perfSimConfig.is_open())
      {
        std::string line;
        while (std::getline(perfSimConfig, line))
        {
          size_t pos = line.find('=');
          if (pos != std::string::npos)
          {
            std::string val1 = line.substr(0, pos);
            std::string val2 = line.substr(pos + 1);
            if (params->deviceName < 0 && val1.find("DEVICE_NAME") != std::string::npos)
            {
              try
              {
                params->deviceName = std::stoi(val2);
              }
              catch (...)
              {
                //pass
              }
            }
            if (params->numCores < 1 && val1.find("NUMCORES") != std::string::npos)
            {
              try
              {
                params->numCores = std::stoi(val2);
              }
              catch (...)
              {
                //pass
              }

            }
          }
        }
        perfSimConfig.close();
      }
    }

    // Set numCores to 1 by default if not provided and not available in  perfSimConfig file
    if(params->numCores < 1)
    {
      params->numCores = 1;
    }

    // Copy params to gParams
    gParams = *params;
    strcpy((char *)inConfigFilename, (char *)gParams.outputNetFile);

  }

  // Validate parameters
  TIDL_IMPORT_CHECK_AND_RETURN(tidlValidateImportParams(&gParams), "Validation of import parameters failed");
  
  // Extra validation for inputNetFile
  FILE *fp = fopen((const char *)gParams.inputNetFile, "r");
  if(fp== NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Couldn't open inputNetFile file: %s", gParams.inputNetFile);
    return -1;
  }
  fclose(fp);

  if(gParams.modelType == 0)
  {
    // Extra validation for inputParamsFile in case of caffee model
    fp = fopen((const char *)gParams.inputParamsFile, "r");
    if(fp== NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Couldn't open inputParamsFile file: %s", gParams.inputParamsFile);
      return -1;
    }
    fclose(fp);
  }

  // Start import based on model type
  TIDL_printCompilationStages();
  TIDL_printVersionTable(TIDL_TOOLS_VERSION, (char *)gParams.c7xFirmwareVersion);

  if((gParams.metaArchType == TIDL_metaArchTFSSD) ||
     (gParams.metaArchType == TIDL_metaArchTFFasterRcnn))
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tf_metaArch_import(&gParams), "TF Meta Arch import failed");
  }
  else if((gParams.metaArchType == TIDL_metaArchTIDLSSD) ||
          (gParams.metaArchType == TIDL_metaArchTIDLYolo) ||
          (gParams.metaArchType == TIDL_metaArchTIDLRetinaNet) ||
          (gParams.metaArchType == TIDL_metaArchTIDLYoloV5) ||
          (gParams.metaArchType == TIDL_metaArchTIDL3DOD) ||
          (gParams.metaArchType == TIDL_metaArchTIDLYoloV8) ||
          (gParams.metaArchType == TIDL_metaArchTIDLCenterPoint) ||
          (gParams.metaArchType == TIDL_metaArchTIDLBEVFormer))

  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_metaArch_import(&gParams), "TIDL Meta Arch import failed");
  }
  else if(gParams.metaArchType != -1)
  {
    TIDL_GLOBAL_REPORT_ERROR("Unsupported metaArchType: %d", gParams.metaArchType);
    return -1;
  }

  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_CAFFE)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(caffe_import_new(&gParams, &state->dataIndex, &state->layerIndex), "Caffe model import failed");
  }
  else if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TENSORFLOW)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tf_import(&gParams, &state->dataIndex, &state->layerIndex), "Tensorflow model import failed");
  }
  else if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(onnx_import(&gParams, &state->dataIndex, &state->layerIndex), "ONNX model import failed");
  }
  else if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TFLITE)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tfLite_import(&gParams, &state->dataIndex, &state->layerIndex), "TFLITE model import failed");
  }

  if (params != nullptr)
  {
    *params = gParams;
  }

  return 0;
}

int32_t TIDL_runImportSession(importState *state, calibTensor *in[], size_t size)
{
  int32_t status = 0;
  state->currFrame++;

  bool runOptimizeNet = false;
  bool runImportBackend = false;

  if (in == nullptr)
  {
    runOptimizeNet = true;
    runImportBackend = true;
  }
  else
  {
    if (state->currFrame == 1)
    {
      runOptimizeNet = true;
    }
    if (state->currFrame == gParams.numFrames)
    {
      runImportBackend = true;
    }
  }

  if (runOptimizeNet)
  {
    // Run the optimization pipeline and convert the graph to target format.
    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization started]");
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeNet(orgTIDLNetStructure, state->layerIndex, state->dataIndex), "Network Optimization failed");
    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization completed]");
  }

  if (in != nullptr)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(handleInData(&gParams, orgTIDLNetStructure, state->currFrame, in, size), "Input data handler failed");
  }

  if (runImportBackend)
  {
    // Run quantization and the network compiler passes.
    status = TIDL_import_backend(state->layerIndex, state->dataIndex);
    if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      string outputNetFileStr((const char*)gParams.outputNetFile);
      string outputParamsFileStr((const char*)gParams.outputParamsFile);
      outputParamsFileStr = outputParamsFileStr + "1.bin";
      system(("rm -f " + outputNetFileStr + " 2> /dev/null").c_str());
      system(("rm -f " + outputParamsFileStr + " 2> /dev/null").c_str());
      TIDL_GLOBAL_REPORT_WARNING("\n*****************************************************************************"
                                "\n*     TIDL Import returned with error, model artifacts are not reliable     *"
                                "\n*                   Please resolve the compilation issues                   *"
                                "\n*****************************************************************************\n");
      TIDL_IMPORT_CHECK_AND_ABORT(status, "Quantization/NC Pass/Model checker failed");
    }

    // Populate quantParamsPrototxt file
    if(gParams.isQuantParamsToBeExported)
    {
      TIDL_IMPORT_CHECK_AND_ABORT(TIDL_exportQuantParamsIntoUserPath(&gParams), "Exporting quant params failed");
    }
  }

  gDiags.freeFile();

  /* Hack for ANSI color termination */
  TIDL_GLOBAL_REPORT_INFO(1, "");

  return status;
}


} //extern "C"