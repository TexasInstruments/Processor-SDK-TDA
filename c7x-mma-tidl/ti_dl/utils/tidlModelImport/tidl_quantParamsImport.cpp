/*
*
* Copyright (c) {2015 - 2023} Texas Instruments Incorporated
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

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
//#include <io.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <float.h>
#include <cmath>
#include <fstream>
#include <dirent.h>

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tidl_quant_params.pb.h"
#include "tidl_import_config.h"
#include "tidl_import_diag.h"

#define __MODULE__ "[QUANTIZATION]"

using namespace std;
using namespace tidl_quant_params;
using ::google::protobuf::Message;
using ::google::protobuf::io::FileInputStream;
using ::google::protobuf::io::FileOutputStream;
using ::google::protobuf::io::ZeroCopyInputStream;
using ::google::protobuf::io::CodedInputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;
using ::google::protobuf::io::CodedOutputStream;

#include "tidl_import_common.h"
#include "tidl_import_quantize.h"

#include <unordered_map>

/**
 * ---------------------------------------------------------------------------
 * @brief      Helper function to map TIDL element type integer to string
 * @param      elementType: integer representing TIDL element type
 * @remarks    None
 * @return     string representing TIDL element type
 * ---------------------------------------------------------------------------
 */

std::string TIDL_elementTypeToString(int32_t elementType) {
  static const std::unordered_map<int32_t, std::string> typeMap = {
    {0, "uint8"},
    {1, "int8"},
    {2, "uint16"},
    {3, "int16"},
    {4, "uint32"},
    {5, "int32"},
    {6, "float32"},
    {7, "uint64"},
    {8, "int64"}
  };
  auto it = typeMap.find(elementType);
  if (it != typeMap.end()) return it->second;
  return "unknown";
}

/**
 * ---------------------------------------------------------------------------
 * @brief      This function checks if per-channel quantization is enabled for weights of a layer
 * @param      currPCLayer: pointer to the current PC layer
 * @remarks    None
 * @return     1 if per-channel quantization is enabled, 0 otherwise
 * ---------------------------------------------------------------------------
 */

int32_t TIDL_isPerChannelEnabled (sTIDL_LayerPC_t *currPCLayer) 
{
  if (TIDL_doesLayerSupportAsymTensors (currPCLayer) &&
      currPCLayer->layerKernelType == TIDL_HighPrecisionKernel)
  {
    if (currPCLayer->layerType == TIDL_ConvolutionLayer)
    {
      return 1;
    }
  }
  return 0;
}


/**
----------------------------------------------------------------------------
@brief      This function checks if the file storing quantParams is empty. Returns true if so.

@param      filepath: path to the quantParams prototxt file
@remarks    None
@return     boolean value represented by uint8_t
----------------------------------------------------------------------------
*/
uint8_t TIDL_isQuantParamsProtoTxtFileEmpty(char* filePath)
{
  // peeks into the file and checks if the file is empty
  std::ifstream file(filePath);
  return file.peek() == std::ifstream::traits_type::eof();
}

/*
----------------------------------------------------------------------------
@brief      This function returns an array whose idx i stores "layerName_layerType" of the ith layer in the TIDLNetQuantParams message instance. This method is used to extract
            layer data from the protomsg on the basis of layer name and layer type.

@param      QuantParamsMsg: protomsg storing the quant params
@param      protoMsgLayerIdxArr: array that is supposed to store the layerName_layerType sequence
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_getProtoMsgLayerIndexingArr(TIDLNetQuantParams *QuantParamsMsg, 
                                      string *protoMsgLayerIdxArr)
{
  // fill up the map if the protomsg isnt empty
  if(QuantParamsMsg->ByteSize() > 0){

    // fill up the map with updated indexes
    for(int i = 0; i<QuantParamsMsg->layers_size(); i++)
    {
      string layerName = QuantParamsMsg->layers(i).layer_name();
      string layerType = QuantParamsMsg->layers(i).layer_type();
      protoMsgLayerIdxArr[i] = layerName+'_'+layerType;
    }
  }
}

/**
----------------------------------------------------------------------------
@brief      This function returns the index at which the quant params are stored for a certain layer in the protomsg instance. Returns -1 if quant params of a certain layer isnt 
            available

@param      pOrgTIDLNetStructure: pointer to PC network
@param      protoMsgLayerIdxArr: array that is supposed to store the layerName_layerType sequence
@param      protoMsgSize: number of layers in the protomsg
@param      layerIdx: index of the layer whose quant params are being searched for 
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
int32_t TIDL_getProtoMsgLayerIdx(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, 
                                 string *protoMsgLayerIdxArr,
                                 int32_t protoMsgSize,
                                 int32_t layerIdx)
{
  // get layer name
  char* layerNameCharArr = (char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].outDataNames[0];
  string layerName(layerNameCharArr);

  // get layer type
  int32_t layerTypeIdx = pOrgTIDLNetStructure->TIDLPCLayers[layerIdx].layerType;
  string layerType = TIDL_LayerString[layerTypeIdx];

  // generate key
  string key = layerName+'_'+layerType;

  //get the idx where the quant params for this certain layerName and layerType is available, if its available
  for(int i = 0; i < protoMsgSize; i++){
    if(protoMsgLayerIdxArr[i] == key) return i;
  }

  // layer unavailable in proto msg
  return -1;
}

/**
 * ---------------------------------------------------------------------------
 * @brief      This function finds min and max values in a float array
 * @param      data: pointer to the input float array
 * @param      size: size of the input array
 * @param      min: pointer to store the minimum value
 * @param      max: pointer to store the maximum value
 * @remarks    None
 * @return     None
 * ---------------------------------------------------------------------------
 */

void TIDL_findMinMax (float* data, int32_t size, float* min, float* max)
{
    *min = FLT_MAX;
    *max = -FLT_MAX;
    for (int32_t i = 0; i < size; i++)
    {
        if (data[i] < *min)
        {
            *min = data[i];
        }
        if (data[i] > *max)
        {
            *max = data[i];
        }
    }
}

/**
 * ---------------------------------------------------------------------------
 * @brief      This function retrieves the quantization scale for weights or bias of a layer
 * @param      currPCLayer: pointer to the current PC layer
 * @param      paramType: integer indicating whether weights or bias scale is requested
 * @remarks    None
 * @return     quantization scale as float32_tidl
 * ---------------------------------------------------------------------------
 */

float32_tidl TIDL_getLayerParamScale (sTIDL_LayerPC_t *currPCLayer, int32_t paramType)
{
    float32_tidl scale = 1.0f;
    if (currPCLayer->layerType == TIDL_ConvolutionLayer)
    {
        if (paramType == TIDL_WEIGHT_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.convParams.weightScale;
        }
        else if (paramType == TIDL_BIAS_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.convParams.biasScale;
        }
    }
    else if (currPCLayer->layerType == TIDL_InnerProductLayer)
    {
        if (paramType == TIDL_WEIGHT_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.innerProductParams.weightScale;
        }
        else if (paramType == TIDL_BIAS_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.innerProductParams.biasScale;
        }
    }
    else if (currPCLayer->layerType == TIDL_Deconv2DLayer)
    {
        if (paramType == TIDL_WEIGHT_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.convParams.weightScale;
        }
        else if (paramType == TIDL_BIAS_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.convParams.biasScale;
        }
    }
    else if (currPCLayer->layerType == TIDL_DeformableConvLayer)
    {
        if (paramType == TIDL_WEIGHT_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.deformConvParams.weightScale;
        }
        else if (paramType == TIDL_BIAS_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.deformConvParams.biasScale;
        }
    }
    else if (currPCLayer->layerType == TIDL_BatchNormLayer)
    {
        if (paramType == TIDL_WEIGHT_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.batchNormParams.weightScale;
        }
        else if (paramType == TIDL_BIAS_QUANT_PARAMS)
        {
            scale = currPCLayer->layerParams.batchNormParams.biasScale;
        }
    }
    return scale;
}


/**
----------------------------------------------------------------------------
@brief      This function exports the Quant params from given tensor into the proto message.

@param      currPCLayer : pointer to the current PC layer
@param      pTIDLNetStructure : pointer to the device network
@param      currLayer: pointer to the current layer in the proto message
@param      tensorQuantParams : integer pointing to the tensor for which values are getting stored
@param      configParams: pointer to import config parameters
@param      currLayerPreQuantizedBiasValues: pointer to the pre-quantized bias values of the PC network 
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_populateTensorQuantParms(sTIDL_LayerPC_t *currPCLayer, 
                                   sTIDL_Network_t *pTIDLNetStructure, 
                                   tidl_quant_params::TidlLayerQuantParams *currLayer, 
                                   int32_t tensorQuantParams,
                                   tidl_import_config *configParams,
                                   float* currLayerWeightValues,
                                   float* currLayerPreQuantizedBiasValues)
{
  
  // consider storing quant params iff its available
  if(currPCLayer->quantParams[tensorQuantParams].size > 0)
  {

    if (TIDL_isPerChannelEnabled(currPCLayer))
    {
      if (currPCLayer->layerType == TIDL_ConvolutionLayer)
      {
        int32_t numOutChannels = currPCLayer->layerParams.convParams.numOutChannels;
        if (tensorQuantParams == TIDL_WEIGHT_QUANT_PARAMS)
        {
          float32_tidl* weightScales = (float32_tidl*)currPCLayer->weightScales.ptr;
          int32_t weightPerChannelSize = (currPCLayer->layerParams.convParams.numInChannels *
                                        currPCLayer->layerParams.convParams.kernelH *
                                        currPCLayer->layerParams.convParams.kernelW) / currPCLayer->layerParams.convParams.numGroups;
          for (int32_t ch = 0; ch < numOutChannels; ch++)
          {
            tidl_quant_params::TidlTensorQuantParams *tensor = currLayer->add_weights();

            tensor->set_is_symmetric(true); // weights are always quantized symmetrically
            if((currPCLayer->weightsElementSizeInBits > 8) && (currPCLayer->weightsElementSizeInBits <= 16))
            {
              tensor->set_element_type("int16");
            }
            else
            {
              tensor->set_element_type("int8");
            }
            float min, max;
            TIDL_findMinMax (&currLayerWeightValues[ch*weightPerChannelSize], weightPerChannelSize, &min, &max);
            tensor->set_min(min);
            tensor->set_max(max);
            tensor->add_scale(weightScales[ch]);
            tensor->add_zero_point(0);
            tensor->set_size(1);
          }
        }

        if (tensorQuantParams == TIDL_BIAS_QUANT_PARAMS)
        {
          for (int32_t ch = 0; ch < numOutChannels; ch++)
          {
            float min, max;
            tidl_quant_params::TidlTensorQuantParams *tensor = currLayer->add_bias();

            tensor->set_min(currLayerPreQuantizedBiasValues[ch]);
            tensor->set_max(currLayerPreQuantizedBiasValues[ch]);
            tensor->set_size(1);
            tensor->set_is_symmetric(true); // biases are always quantized symmetrically
            if ((currPCLayer->weightsElementSizeInBits > 8) && (currPCLayer->weightsElementSizeInBits <= 16))
            {
              tensor->set_element_type("int64");
            }
            else
            {
              tensor->set_element_type("int32");
            }
            tensor->add_scale(1);
            tensor->add_zero_point(0);
            tensor->add_value(currLayerPreQuantizedBiasValues[ch]);
          }
        }
      }
    }
    else 
    {
      //add tensor
      tidl_quant_params::TidlTensorQuantParams *tensor = NULL;

      if(tensorQuantParams == TIDL_WEIGHT_QUANT_PARAMS) tensor = currLayer->add_weights();

      if(tensorQuantParams == TIDL_BIAS_QUANT_PARAMS) tensor = currLayer->add_bias();
      
      if(tensorQuantParams == TIDL_SLOPE_QUANT_PARAMS) tensor = currLayer->add_slope();
      
      // set min, max, elementType
      tensor->set_min(currPCLayer->quantParams[tensorQuantParams].min);
      tensor->set_max(currPCLayer->quantParams[tensorQuantParams].max);
      tensor->set_is_symmetric(true); // weights & biases are always quantized symmetrically

      if((tensorQuantParams == TIDL_BIAS_QUANT_PARAMS) || ((tensorQuantParams == TIDL_WEIGHT_QUANT_PARAMS) && (currPCLayer->weightsElementSizeInBits > 8) && (currPCLayer->weightsElementSizeInBits <= 16)))
      {
        tensor->set_element_type("int16");
      }
      else
      {
        tensor->set_element_type("int8");
      }

      // store scales and zero-points for only those layers whose parameters are getting updated during calibration
      if  (TIDL_isLayerParamsToBeUpdated(currPCLayer->layerType))
      {
        // for weight tensor
        if(tensorQuantParams == TIDL_WEIGHT_QUANT_PARAMS)
        {
          // if per channel calibration is enabled
          if((configParams->calibrationOption & TIDL_CalibOptionPerChannelWeightQuantization) == TIDL_CalibOptionPerChannelWeightQuantization)
          {
            // set size to no.of channels(as no.of scales = no.of channels)
            int32_t size = currPCLayer->perChannelWeightScale.bufSize;
            
            // add scales if present
            if(size > 0)
            {
              tensor->set_size(size);
              
              void* ptr = currPCLayer->perChannelWeightScale.ptr;
              float* tensor_scales = (float*)ptr;
              
              for(int idx = 0; idx < size; idx++){
                tensor->add_scale(tensor_scales[idx]);
                tensor->add_zero_point(0);
              }
            }
          }
          // else per tensor calibration is enabled
          else
          {
            tensor->set_size(1);
            float32_tidl weightScale = TIDL_getLayerParamScale (currPCLayer, TIDL_WEIGHT_QUANT_PARAMS);
            tensor->add_scale(weightScale);
            tensor->add_zero_point(0);
          }
        }
        
        // for bias tensor
        if(tensorQuantParams == TIDL_BIAS_QUANT_PARAMS)
        {
          float32_tidl biasScale = TIDL_getLayerParamScale (currPCLayer, TIDL_BIAS_QUANT_PARAMS);
          tensor->add_scale(biasScale);
          tensor->add_zero_point(0);
        }
          
      }

      // store tensor values only incase of bias tensor
      if(tensorQuantParams == TIDL_BIAS_QUANT_PARAMS)
      {
        // set the tensor size
        int32_t size = currPCLayer->bias.bufSize;
        tensor->set_size(size);

        for(int i=0; i<size; i++)
        {
          tensor->add_value(currLayerPreQuantizedBiasValues[i]);
        }
      }
    }
  }
}

/**
----------------------------------------------------------------------------
@brief      This function imports Quant params of given tensor type into the PC network.

@param      currPCLayer : pointer to the current PC layer
@param      tensorQuantParams : integer pointing to the tensor for which values are getting stored
@param      layerIndex: index of the current layer
@param      QuantParamsMsg: pointer to the quant params proto message
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_populateTIDLPCNet(sTIDL_LayerPC_t *currPCLayer, 
                            int32_t tensorQuantParams, 
                            int32_t layerIndex, 
                            TIDLNetQuantParams *QuantParamsMsg)
{

  // populate min/max for weight tensor
  if(tensorQuantParams == TIDL_WEIGHT_QUANT_PARAMS)
  {
    if (QuantParamsMsg->layers(layerIndex).weights_size() > 0)
    {
      float min = QuantParamsMsg->layers(layerIndex).weights(0).min();
      float max = QuantParamsMsg->layers(layerIndex).weights(0).max();
      for(int i= 0; i<QuantParamsMsg->layers(layerIndex).weights_size(); i++)
      {
        min = std::min (min, QuantParamsMsg->layers(layerIndex).weights(i).min());
        max = std::max (max, QuantParamsMsg->layers(layerIndex).weights(i).max());
      }
      currPCLayer->quantParams[tensorQuantParams].min = min;
      currPCLayer->quantParams[tensorQuantParams].max = max;
    }
  }
  // populate min/max for bias tensor
  else if(tensorQuantParams == TIDL_BIAS_QUANT_PARAMS){
    if(QuantParamsMsg->layers(layerIndex).bias_size() > 0)
    {
      int32_t bias_size = QuantParamsMsg->layers(layerIndex).bias_size();
      float min = QuantParamsMsg->layers(layerIndex).bias(0).min();
      float max = QuantParamsMsg->layers(layerIndex).bias(0).max();
      int32_t size = 0;
      for (int i = 0; i < bias_size; i++)
      {
        size += QuantParamsMsg->layers(layerIndex).bias(i).size();
        min = std::min (min, QuantParamsMsg->layers(layerIndex).bias(i).min());
        max = std::max (max, QuantParamsMsg->layers(layerIndex).bias(i).max());
      }
      currPCLayer->quantParams[tensorQuantParams].size = size;
      currPCLayer->quantParams[tensorQuantParams].min = min;
      currPCLayer->quantParams[tensorQuantParams].max = max;

      if(size>0)
      {
        // store bias values
        int32_t offset = 0;
        float ptr[size];
        for(int i=0; i<bias_size; i++)
        {
          for (int j=0; j<QuantParamsMsg->layers(layerIndex).bias(i).size(); j++)
          {
            ptr[offset++] = QuantParamsMsg->layers(layerIndex).bias(i).value(j);
          }
        }

        void* data = new float[size];
        memcpy(data, &ptr, size*sizeof(float));
        currPCLayer->bias.ptr = data;
      }
    }
  }
  // populate min/max for slope tensor
  else if(tensorQuantParams == TIDL_SLOPE_QUANT_PARAMS)
  {
    if(QuantParamsMsg->layers(layerIndex).slope_size() > 0)
    {
      currPCLayer->quantParams[tensorQuantParams].min = QuantParamsMsg->layers(layerIndex).slope(0).min();
      currPCLayer->quantParams[tensorQuantParams].max = QuantParamsMsg->layers(layerIndex).slope(0).max();
    }
  }

}

/**
----------------------------------------------------------------------------
@brief      This function generates the path to the prototxt file storing the quant params for the current subgraph and stores it in the import config

@param      gParams: pointer to import config parameters
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_getQuantParamsPrototxtPath(tidl_import_config *configParams){

  char* outNetFilePath = (char*)configParams->outputNetFile;
  //convert to string inorder to perform concatenation
  string outNetFilePathStr(outNetFilePath);

  //extract and set network name
  size_t lastSlashPos = outNetFilePathStr.find_last_of('/');
  size_t lastDotPos = outNetFilePathStr.find_last_of('.');
  string outputNetName = outNetFilePathStr.substr(lastSlashPos+1, lastDotPos-lastSlashPos-1);

  //extract the directory where the prototxt file is to be stored
  string protoTxtDir = outNetFilePathStr.substr(0, lastSlashPos+1);

  // generate the prototxt file path and store it into gParams
  string outputNetQuantParamsPrototxtFilePathStr = protoTxtDir+outputNetName+"_quant_params.prototxt";
  strcpy((char*)configParams->outputNetQuantParamsPrototxtFile, outputNetQuantParamsPrototxtFilePathStr.c_str());

  //generate the prototxt file
  FILE* quantParamsPrototxtFile = fopen((char*)configParams->outputNetQuantParamsPrototxtFile, "w");
  if(quantParamsPrototxtFile != NULL) fclose(quantParamsPrototxtFile);
}

/**
----------------------------------------------------------------------------
@brief      This function imports Mixed precision info stored in the protoTxt file into the PC network.

@param      pOrgTIDLNetStructure : pointer to the PC network
@param      pTIDLNetStructure : pointer to the device network
@param      configParams: pointer to import config parameters
@remarks    None
@return     0 on sucessfull import read and import of bit width information
            -1 on failure to read the file or information in the file is wrong
----------------------------------------------------------------------------
*/
int TIDL_importBitDepthProtoTxt(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, 
                                        tidl_import_config *configParams)
{
  
  // initialise proto message
  tidl_quant_params::TIDLNetQuantParams QuantParamsMsg;
  const char* filename = (char*)configParams->quantParamsPrototxtFile;
  if (filename != NULL && filename[0] == '\0')
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  FILE* fp = fopen(filename, "r");
  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not open %s for importing mixed precision info.\nThis will be generated after model compilation.", filename);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  fclose(fp);

  string outputFeature16bitNamesList("");
  string params16bitNamesList("");

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Importing mixed precision info from quant params : %s", filename);

  // pull the prototxt file data into a protomsg instance
  bool success = TIDL_readProtoFromTextFile(filename, &QuantParamsMsg);
  if(!success)
  {
    TIDL_GLOBAL_REPORT_ERROR("Reading prototext file failed. Aborting");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  // get the layer idx array
  string* protoMsgLayerIdxArr = new string[QuantParamsMsg.layers_size()];
  TIDL_getProtoMsgLayerIndexingArr(&QuantParamsMsg, protoMsgLayerIdxArr);

  // populate the PC network using the proto msg
  for(int i = 0; i < pOrgTIDLNetStructure->numLayers; i++)
  {
    // pointer to current layer
    sTIDL_LayerPC_t *currPCLayer = &pOrgTIDLNetStructure->TIDLPCLayers[i];

    // get the index of the layer in the protomsg at which the quant params for currPCLayer are available
    int32_t idx = TIDL_getProtoMsgLayerIdx(pOrgTIDLNetStructure, protoMsgLayerIdxArr, QuantParamsMsg.num_layers(), i);    

    if(QuantParamsMsg.layers(idx).outputs_size() > 0)
    {
      std::string elementType = QuantParamsMsg.layers(idx).outputs(0).element_type();
      if((elementType == "int16") || (elementType == "uint16"))
      {
        outputFeature16bitNamesList += (char *)currPCLayer->outDataNames[0];
        outputFeature16bitNamesList += ",";
      }
    }

    if(QuantParamsMsg.layers(idx).weights_size() > 0)
    {
      std::string elementType = QuantParamsMsg.layers(idx).weights(0).element_type();
      if((elementType == "int16") || (elementType == "uint16"))
      {
        params16bitNamesList += (char *)currPCLayer->outDataNames[0];
        params16bitNamesList += ",";
      }
    }
  }
  strcpy((char *)configParams->outputFeature16bitNamesList,(char *)outputFeature16bitNamesList.c_str());
  strcpy((char *)configParams->params16bitNamesList,(char *)params16bitNamesList.c_str());

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "16 Bit tensors : %s | %s", configParams->outputFeature16bitNamesList, configParams->params16bitNamesList);

  delete[] protoMsgLayerIdxArr;
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/**
----------------------------------------------------------------------------
@brief      This function imports quant params stored in the protoTxt file into the PC network.

@param      pOrgTIDLNetStructure : pointer to the PC network
@param      pTIDLNetStructure : pointer to the device network
@param      configParams: pointer to import config parameters
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
int32_t TIDL_importQuantParamsFromProtoTxt(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, 
                                        sTIDL_Network_t *pTIDLNetStructure, 
                                        tidl_import_config *configParams)
{
  pOrgTIDLNetStructure->quantStats = TIDL_QUANT_STATS_FIXED;
  
  // initialise proto message
  tidl_quant_params::TIDLNetQuantParams QuantParamsMsg;
  const char* filename = (char*)configParams->quantParamsPrototxtFile;
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Importing quant params from: %s", configParams->quantParamsPrototxtFile);

  // pull the prototxt file data into a protomsg instance
  bool success = TIDL_readProtoFromTextFile(filename, &QuantParamsMsg);
  if(!success)
  {
    TIDL_GLOBAL_REPORT_ERROR("Reading prototext file failed. Aborting");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
   

  // get the layer idx array
  string* protoMsgLayerIdxArr = new string[QuantParamsMsg.layers_size()];
  TIDL_getProtoMsgLayerIndexingArr(&QuantParamsMsg, protoMsgLayerIdxArr);

  // populate the PC network using the proto msg
  for(int i = 0; i < pOrgTIDLNetStructure->numLayers; i++)
  {
    // pointer to current layer
    sTIDL_LayerPC_t *currPCLayer = &pOrgTIDLNetStructure->TIDLPCLayers[i];

    // get the index of the layer in the protomsg at which the quant params for currPCLayer are available
    int32_t idx = TIDL_getProtoMsgLayerIdx(pOrgTIDLNetStructure, protoMsgLayerIdxArr, QuantParamsMsg.num_layers(), i);    

    //populate weights, bias, slope tensors
    TIDL_populateTIDLPCNet(currPCLayer, TIDL_WEIGHT_QUANT_PARAMS, idx, &QuantParamsMsg);
    TIDL_populateTIDLPCNet(currPCLayer, TIDL_BIAS_QUANT_PARAMS, idx, &QuantParamsMsg);
    TIDL_populateTIDLPCNet(currPCLayer, TIDL_SLOPE_QUANT_PARAMS, idx, &QuantParamsMsg);

    // populate output tensor if output tensor present in the proto msg
    if(QuantParamsMsg.layers(idx).outputs_size() > 0)
    {
      pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].minTensorValue = QuantParamsMsg.layers(idx).outputs(0).min();
      pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].maxTensorValue = QuantParamsMsg.layers(idx).outputs(0).max();

      if (gParams.preQuantizedModel == 1 && pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.actType == TIDL_Clip)
      {
        // For pre-quantized models with clip activation, adjust the clip values to match the imported quantization parameters
        pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.clipMin = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].minTensorValue;
        pOrgTIDLNetStructure->TIDLPCLayers[i].actParams.clipMax = pOrgTIDLNetStructure->TIDLPCLayers[i].outData[0].maxTensorValue;
      }
    }
  }
  
  // copy the imported data into the device network
  tidl_copyPCNetToDeviceNet(pOrgTIDLNetStructure, pTIDLNetStructure, &gParams, pOrgTIDLNetStructure->numLayers);
  pTIDLNetStructure->isQuantStatsAvailable = 1;

  // free indexing array
  delete[] protoMsgLayerIdxArr;
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Quantization parameters imported successfully");
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/**
----------------------------------------------------------------------------
@brief      This function exports the quantization parameters of the calibrated model into a prototxt file.

@param      pOrgTIDLNetStructure : pointer to the PC network
@param      pTIDLNetStructure : pointer to the device network
@param      configParams: pointer to import config parameters
@param      layerIndex: pointer to import config parameters
@param      preQuantizedBiasValueArr: pointer to the prequantized bias values of the PC Network
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_exportQuantParamsToProtoTxt(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure, 
                                      sTIDL_Network_t *pTIDLNetStructure, 
                                      tidl_import_config *configParams, 
                                      int32_t layerIndex, 
                                      float** weightValueArray,
                                      float** preQuantizedBiasValueArr)
{

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Output network quant params prototxt file path: %s", (char*)configParams->outputNetQuantParamsPrototxtFile);

  // initialise the proto msg
  tidl_quant_params::TIDLNetQuantParams QuantParamsMsg;

  // set number of layers in the network
  QuantParamsMsg.set_num_layers(layerIndex);

  // set the quantization style
  if(configParams->quantizationStyle == TIDL_QuantStyleAsymNP2_TFL ||
    configParams->quantizationStyle == TIDL_QuantStyleAsymNP2)  
  {
    QuantParamsMsg.set_quant_type(ASYMMETRIC);
  }
  else 
  {
    QuantParamsMsg.set_quant_type(SYMMETRIC);
  }

  // store quant params of each layer from current subgraph of the PC network into the proto msg
  for(int i=0; i<layerIndex; i++)
  {
    // pointer to current PC layer
    sTIDL_LayerPC_t *currPCLayer = &pOrgTIDLNetStructure->TIDLPCLayers[i];
    
    // pointer to struct storing current layer's params
    tidl_quant_params::TidlLayerQuantParams *currLayer = QuantParamsMsg.add_layers();

    // set layer name
    char* layerNameCharArr = (char*)currPCLayer->outDataNames[0];
    currLayer->set_layer_index(i);
    string layerName(layerNameCharArr);
    currLayer->set_layer_name(layerName);

    // set layer type
    int32_t layerTypeIdx = currPCLayer->layerType;
    string layerType = TIDL_LayerString[layerTypeIdx];
    currLayer->set_layer_type(layerType);

    // set bit depth
    int32_t bitDepth = (8*TIDLIT_getProcessingElementSizeInBytes((const sTIDL_LayerPC_t*)currPCLayer));
    currLayer->set_bit_depth(bitDepth);

    //populate weights, slope and bias tensors
    TIDL_populateTensorQuantParms(currPCLayer, pTIDLNetStructure, currLayer, TIDL_WEIGHT_QUANT_PARAMS, configParams, weightValueArray[i], preQuantizedBiasValueArr[i]);
    TIDL_populateTensorQuantParms(currPCLayer, pTIDLNetStructure, currLayer, TIDL_BIAS_QUANT_PARAMS, configParams, weightValueArray[i], preQuantizedBiasValueArr[i]);
    TIDL_populateTensorQuantParms(currPCLayer, pTIDLNetStructure, currLayer, TIDL_SLOPE_QUANT_PARAMS, configParams, weightValueArray[i], preQuantizedBiasValueArr[i]);

    //add and populate output tensor
    tidl_quant_params::TidlTensorQuantParams *curr_layer_outputs = currLayer->add_outputs();
    
    // set all the parameters
    curr_layer_outputs->set_size(1);
    curr_layer_outputs->set_element_type(TIDL_elementTypeToString(currPCLayer->outData[0].elementType));
    curr_layer_outputs->set_min(currPCLayer->outData[0].minTensorValue);
    curr_layer_outputs->set_max(currPCLayer->outData[0].maxTensorValue);
    curr_layer_outputs->set_is_symmetric(currPCLayer->layerKernelType); 
    curr_layer_outputs->add_scale(currPCLayer->outData[0].tensorScale);
    curr_layer_outputs->add_zero_point(currPCLayer->outData[0].tensorZeroPoint);
  }

  // store proto msg as string
  std::string *protoTxt = new string();
  google::protobuf::TextFormat::PrintToString(QuantParamsMsg, protoTxt);

  // access the prototxt file and wipe it off (clean up any sort of garbage value) inorder to write the instance into it
  ofstream protoTxtFile;
  protoTxtFile.open((char*)configParams->outputNetQuantParamsPrototxtFile, ofstream::out | ofstream::trunc);

  // write string to proto file
  protoTxtFile << *protoTxt;

  // delete the proto string instance
  delete protoTxt;
  protoTxtFile.close();

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Calibrated quant parameters stored in protoTxt format");
}

/**
----------------------------------------------------------------------------
@brief      This function checks if all the bias values are available for a given layer, if it has bias

@param      QuantParamsMsg : pointer to protomsg instance storing the quant params
@param      idx : index of the layer for which we are checking
@remarks    None
@return     a boolean represented by int8_t
----------------------------------------------------------------------------
*/
int8_t TIDL_areBiasValuesAvailable(TIDLNetQuantParams *QuantParamsMsg, 
                                   int32_t idx)
{
  //This method is called iff bias tensor is available so, we can skip the availability check
  //if bias has values, check if the no.of values and size is consistent, id not consistent, return false
  if(QuantParamsMsg->layers(idx).bias(0).size() == 0) return 0;
  else{
    return (QuantParamsMsg->layers(idx).bias(0).size() == QuantParamsMsg->layers(idx).bias(0).value_size());
  }
}

/**
----------------------------------------------------------------------------
@brief      This function checks if quantization parameters required for bypassing calibration for a certain layer is available

@param      QuantParamsMsg : pointer to protomsg instance storing the quant params
@param      idx : index of the layer for which we are checking
@remarks    None
@return     a boolean represented by int8_t
----------------------------------------------------------------------------
*/
int8_t TIDL_isAllQuantParamsToBeImportedAvailable(TIDLNetQuantParams *QuantParamsMsg, 
                                                  int32_t idx)
{
  //if layer is unavailable in the proto msg
  if(idx == -1) return 0;

  //if weights are available but min/max isnt calibrated, return false
  if(QuantParamsMsg->layers(idx).weights_size() > 0)
  {
    if(QuantParamsMsg->layers(idx).weights(0).max() == FLT_MAX || QuantParamsMsg->layers(idx).weights(0).min() == FLT_MIN ) return 0;
  }

  //if outputs are available but min/max isnt calibrated, return false
  if(QuantParamsMsg->layers(idx).outputs_size() > 0)
  {
    if(QuantParamsMsg->layers(idx).outputs(0).max() == FLT_MAX || QuantParamsMsg->layers(idx).outputs(0).min() == FLT_MIN ) return 0;
  }

  //if bias are available but min/max and values arent calibrated, return false
  if(QuantParamsMsg->layers(idx).bias_size() > 0)
  {
    if(QuantParamsMsg->layers(idx).bias(0).max() == FLT_MAX || QuantParamsMsg->layers(idx).bias(0).min() == FLT_MIN || !TIDL_areBiasValuesAvailable(QuantParamsMsg, idx) ) return 0;
  }

  return 1;
}

/**
----------------------------------------------------------------------------
@brief      This function checks if calibration step can be bypassed incase of the current subgraph

@param      pOrgTIDLNetStructure : pointer to PC network
@param      configParams : pointer to config params
@remarks    None
@return     a boolean represented by int8_t
----------------------------------------------------------------------------
*/
int8_t TIDL_canBypassCalibration(sTIDL_OrgNetwork_t* pOrgTIDLNetStructure, 
                                 tidl_import_config* configParams)
{
  // convert user given path to string for check
  string quantParamsPrototxtFile((char*)configParams->quantParamsPrototxtFile);
  // if a path is given by the user
  if(quantParamsPrototxtFile != ""){

    // if path is empty or doesnt exist
    if(TIDL_isQuantParamsProtoTxtFileEmpty((char*)configParams->quantParamsPrototxtFile))
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Parameters unavailable, running calibration");
      return 0;
    }
    // if path actually has a prototxt file
    else
    {
      int32_t layerIndex = pOrgTIDLNetStructure->numLayers;

      // pull prototxt instance into a protomsg
      tidl_quant_params::TIDLNetQuantParams QuantParamsMsg;
      const char* protoTxtFileName = (char*)configParams->quantParamsPrototxtFile;

      // if pulling isnt successful, throw error and do calibration
      if(!TIDL_readProtoFromTextFile(protoTxtFileName, &QuantParamsMsg))
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Inconsistent/Incorrect contents in given prototxt file, running calibration");
        return 0;
      }

      // get the indexing array inorder to check for quant params availability
      string* protoMsgLayerIdxArr = new string[QuantParamsMsg.layers_size()];
      TIDL_getProtoMsgLayerIndexingArr(&QuantParamsMsg, protoMsgLayerIdxArr);

      for(int i = 0; i < layerIndex; i++)
      {
        int32_t idx = TIDL_getProtoMsgLayerIdx(pOrgTIDLNetStructure, protoMsgLayerIdxArr, QuantParamsMsg.layers_size(), i);

        // if all the quant params required for calibration bypass for current layer are not present, return false
        if(!TIDL_isAllQuantParamsToBeImportedAvailable(&QuantParamsMsg, idx))
        {
          TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Inadequate parameters provided, running calibration");
          delete[] protoMsgLayerIdxArr;
          protoMsgLayerIdxArr = NULL;
          return 0;
        }
      }

      delete[] protoMsgLayerIdxArr;
      protoMsgLayerIdxArr = NULL;
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Parameter requirements satisfied, bypassing calibration");
      return 1;
    }
  }  
  else
  {
    TIDL_GLOBAL_REPORT_INFO(gParams.debugTraceLevel, "Empty prototxt path, running calibration");
    return 0;
  }
}

/**
----------------------------------------------------------------------------
@brief      This function copies layer data from source TIDLNetQuantParams protomsg to destination TIDLNetQuantParams protomsg

@param      dstProtoMsg : pointer to destination TIDLNetQuantParams protomsg
@param      srcProtoMsg : pointer to source TIDLNetQuantParams protomsg
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_copyLayerData(tidl_quant_params::TIDLNetQuantParams *dstProtoMsg, 
                        tidl_quant_params::TIDLNetQuantParams *srcProtoMsg)
{
  if(srcProtoMsg->layers_size() > 0)
  {
    for(int i=0; i < srcProtoMsg->num_layers(); i++)
    {
      // get pointer to current layer
      tidl_quant_params::TidlLayerQuantParams* currLayer = dstProtoMsg->add_layers();
      
      // copy layer name and type
      currLayer->set_layer_index(srcProtoMsg->layers(i).layer_index());
      currLayer->set_layer_name(srcProtoMsg->layers(i).layer_name());
      currLayer->set_layer_type(srcProtoMsg->layers(i).layer_type());
      currLayer->set_bit_depth(srcProtoMsg->layers(i).bit_depth());

      // copy output tensor
      if(srcProtoMsg->layers(i).outputs_size() > 0){
        tidl_quant_params::TidlTensorQuantParams *currTensor = currLayer->add_outputs();
        currTensor->set_min(srcProtoMsg->layers(i).outputs(0).min());
        currTensor->set_max(srcProtoMsg->layers(i).outputs(0).max());
        currTensor->set_is_symmetric(srcProtoMsg->layers(i).outputs(0).is_symmetric());
        currTensor->set_size(srcProtoMsg->layers(i).outputs(0).size());
        currTensor->set_element_type(srcProtoMsg->layers(i).outputs(0).element_type());
        currTensor->add_scale(srcProtoMsg->layers(i).outputs(0).scale(0));
        currTensor->add_zero_point(srcProtoMsg->layers(i).outputs(0).zero_point(0));
      }

      // copy weights tensor
      if(srcProtoMsg->layers(i).weights_size() > 0)
      {
        for(int j = 0; j < srcProtoMsg->layers(i).weights_size(); j++)
        {
          tidl_quant_params::TidlTensorQuantParams *currTensor = currLayer->add_weights();
          currTensor->set_min(srcProtoMsg->layers(i).weights(j).min());
          currTensor->set_max(srcProtoMsg->layers(i).weights(j).max());
          currTensor->set_is_symmetric(srcProtoMsg->layers(i).weights(j).is_symmetric());
          currTensor->set_size(srcProtoMsg->layers(i).weights(j).size());
          currTensor->set_element_type(srcProtoMsg->layers(i).weights(j).element_type());

          for(int k = 0; k < currTensor->size(); k++)
          {
            currTensor->add_scale(srcProtoMsg->layers(i).weights(j).scale(k));
            currTensor->add_zero_point(srcProtoMsg->layers(i).weights(j).zero_point(k));
          }
        }
      }

      // copy bias tensor
      if(srcProtoMsg->layers(i).bias_size() > 0)
      {
        // loop over bias tensors if multiple present
        for (int j = 0; j < srcProtoMsg->layers(i).bias_size(); j++)
        {
          tidl_quant_params::TidlTensorQuantParams *currTensor = currLayer->add_bias();
          currTensor->set_min(srcProtoMsg->layers(i).bias(j).min());
          currTensor->set_max(srcProtoMsg->layers(i).bias(j).max());
          currTensor->set_is_symmetric(srcProtoMsg->layers(i).bias(j).is_symmetric());
          currTensor->set_size(srcProtoMsg->layers(i).bias(j).size());
          currTensor->set_element_type(srcProtoMsg->layers(i).bias(j).element_type());
          if(srcProtoMsg->layers(i).bias(j).scale_size() > 0) currTensor->add_scale(srcProtoMsg->layers(i).bias(j).scale(0));
          if(srcProtoMsg->layers(i).bias(j).zero_point_size() > 0) currTensor->add_zero_point(srcProtoMsg->layers(i).bias(j).zero_point(0));
          // use a different loop counter here to avoid confusion
          for(int k = 0; k < currTensor->size(); k++)
          {
            currTensor->add_value(srcProtoMsg->layers(i).bias(j).value(k));
          }
        }
      }

      // copy slope tensor
      if(srcProtoMsg->layers(i).slope_size() > 0)
      {
        // slopes arent getting exported so didnt add this 
        // TODO: populate slope tensor
      }
    }
  }
}


/**
----------------------------------------------------------------------------
@brief      This function copies quantParams data stored in the prototx file of a subgraph into the user given prototxt file path

@param      userPrototxtFilePath : user given prototxt filepath
@param      outNetPrototxtFile : path to outputNet's protoxt file
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
int32_t TIDL_generateUserProtoFile(char* userPrototxtFilePath, 
                                char* outNetPrototxtFile)
{
  // initialise the protomsg toring the contents of the uder given prototxt and current output network's prototxt
  tidl_quant_params::TIDLNetQuantParams outputNetQuantParamsMsg, userQuantParamsMsg;
  bool success = TIDL_readProtoFromTextFile((const char*)outNetPrototxtFile, &outputNetQuantParamsMsg);
  if(!success)
  {
    TIDL_GLOBAL_REPORT_ERROR("Reading prototext file failed. Aborting");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  } 

  // if the first dubgraph is getting added into the user given proto path
  if(TIDL_isQuantParamsProtoTxtFileEmpty(userPrototxtFilePath))
  {
    // fill name into the user protomsg instance and copy rest all details from the outNet protomsg
    userQuantParamsMsg.set_quant_type(outputNetQuantParamsMsg.quant_type());
    userQuantParamsMsg.set_num_layers(outputNetQuantParamsMsg.num_layers());

    // copy layer data
    TIDL_copyLayerData(&userQuantParamsMsg, &outputNetQuantParamsMsg);
  }
  else
  {
    // pull contents of user given protopath and then add to that
    bool success = TIDL_readProtoFromTextFile((const char*)userPrototxtFilePath, &userQuantParamsMsg);
    if(!success)
    {
      TIDL_GLOBAL_REPORT_ERROR("Reading prototext file failed. Aborting");
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    
    // add the no.of layes of the current output network
    int32_t numLayersInCurrentOutNet = outputNetQuantParamsMsg.num_layers();
    int32_t totalNumLayersTillPrevOutNet = userQuantParamsMsg.num_layers();
    userQuantParamsMsg.set_num_layers(numLayersInCurrentOutNet+totalNumLayersTillPrevOutNet);

    // add layers from the outnet protomsg into the user proto msg
    TIDL_copyLayerData(&userQuantParamsMsg, &outputNetQuantParamsMsg);
  }
  
  // store proto msg as string
  std::string *protoTxt = new string();
  google::protobuf::TextFormat::PrintToString(userQuantParamsMsg, protoTxt);

  // access the prototxt file and wipe it off inorder to write the updated instance into it
  ofstream protoTxtFile;
  protoTxtFile.open(userPrototxtFilePath, ofstream::out | ofstream::trunc);

  // write string to proto file
  protoTxtFile << *protoTxt;

  // delete the proto string instance
  delete protoTxt;
  protoTxtFile.close();

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/**
----------------------------------------------------------------------------
@brief      This function searches through a directory and returns an array that contains paths to those files that have a certain suffix

@param      directoryPath : directory that is to be searched
@param      suffix : string that the desired file's path should be ending with
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
std::vector<std::string> TIDL_getprototxtFilePaths(string& directoryPath, 
                                                   string& suffix) {
    // initialise the vector that will be storing the paths
    std::vector<std::string> files;
    DIR* dir = opendir(directoryPath.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string filename = entry->d_name;
            //extract the suitable files
            if (filename.size() >= suffix.size() && filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0) {
                files.push_back(directoryPath + "/" + filename);
            }
        }
        closedir(dir);
    }
    return files;
}

/**
----------------------------------------------------------------------------
@brief      This function exports the quantization parameters of the model into the user specified prototxt path

@param      gParams : pointer to import config parameters
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
int32_t TIDL_exportQuantParamsIntoUserPath(tidl_import_config *configParams)
{
  // extract the directory having the prototxt files
  char* outNetFilePath = (char*)configParams->outputNetFile;
  string outNetFilePathStr(outNetFilePath);

  //extract the idx of the last slash
  size_t lastSlashPos = outNetFilePathStr.find_last_of('/');

  //extract the directory where the prototxt files are present
  string outNetQuantParamsPrototxtFileDir = outNetFilePathStr.substr(0, lastSlashPos+1);
  string suffix = "_quant_params.prototxt";

  // store the paths to all the prototxt file into an array
  std::vector<std::string> outNetPrototxtFiles = TIDL_getprototxtFilePaths(outNetQuantParamsPrototxtFileDir, suffix);
  
  // generate file at the user given path
  FILE* userGivenProtoPath = fopen((char*)configParams->quantParamsPrototxtFile, "w");
  if(userGivenProtoPath != NULL) fclose(userGivenProtoPath);

  for(auto& outNetPrototxtFile : outNetPrototxtFiles)
  {
      // copy quant params data from each outputNet prototxt into the user given path
      TIDL_IMPORT_CHECK_AND_RETURN(TIDL_generateUserProtoFile((char*)configParams->quantParamsPrototxtFile, (char*)outNetPrototxtFile.c_str()), "");
      
      // delete the outNet file
      int result = remove(outNetPrototxtFile.c_str());
      if(result != 0)
      {
        TIDL_GLOBAL_REPORT_ERROR("Error deleting outputNet proto file");
      }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
