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

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
//#include <io.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <cmath>

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tidl_import_config.h"
#include "caffe.pb.h"

using namespace std;
using namespace caffe;
using ::google::protobuf::Message;
using ::google::protobuf::io::FileInputStream;
using ::google::protobuf::io::FileOutputStream;
using ::google::protobuf::io::ZeroCopyInputStream;
using ::google::protobuf::io::CodedInputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;
using ::google::protobuf::io::CodedOutputStream;

#include "tidl_import_common.h"
#include "tidl_caffeImport_param.h"
#include "tidl_custom_import.h"
#include "tidl_import_diag.h"

#undef __MODULE__
#define __MODULE__ "[PARSER]"

extern sTIDL_OrgNetwork_t      orgTIDLNetStructure;
extern sTIDL_OrgNetwork_t      tempTIDLNetStructure;
extern sTIDL_Network_t         tIDLNetStructure;

typedef struct sTIDL_caffeLayerParamMap {
  int8_t name[TIDL_STRING_SIZE];
  int32_t(*tidl_caffeMapFunc)(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
    int32_t              i,
    int32_t&             layerIndex,
    int32_t&             dataIndex,
    NetParameter&        netStructure,
    NetParameter&        netParams);
}sTIDL_caffeLayerParamMap_t;

sTIDL_caffeLayerParamMap_t tidl_caffeLayerParamMapTable[] =
{
  { "Concat",                          TIDL_caffeMapConcatParams },
  { "Convolution",                     TIDL_caffeMapConvParams },
  { "ConvolutionDepthwise",            TIDL_caffeMapConvParams },
  { "Pooling",                         TIDL_caffeMapPoolingParams },
  { "ReLU",                            TIDL_caffeMapReluParams },
  { "PReLU",                           TIDL_caffeMapPReLUParams },
  { "Sigmoid",                         TIDL_caffeMapSigmoidParams },
  { "Logistic",                        TIDL_caffeMapSigmoidParams },
  { "Tanh",                            TIDL_caffeMapTanhParams },
  { "HardSigmoid",                     TIDL_caffeMapHardSigmoidParams },
  { "ELU",                             TIDL_caffeMapELUParams },
  { "Dropout",                         TIDL_caffeMapDropoutParams },
  { "Softmax",                         TIDL_caffeMapSoftmaxParams },
  { "softmax",                         TIDL_caffeMapSoftmaxParams },
  { "Deconvolution",                   TIDL_caffeMapDeconvParams },
  { "Argmax",                          TIDL_caffeMapArgmaxParams },
  { "ArgMax",                          TIDL_caffeMapArgmaxParams },
  { "Bias",                            TIDL_caffeMapBiasParams },
  { "ShuffleChannel",                  TIDL_caffeMapShuffleParams },
  { "Eltwise",                         TIDL_caffeMapEltwiseParams },
  { "BatchNorm",                       TIDL_caffeMapBatchNormParams },
  { "Scale",                           TIDL_caffeMapScaleParams },
  { "InnerProduct",                    TIDL_caffeMapInnerProductParams },
  { "Split",                           TIDL_caffeMapSplitParams },
  { "Slice",                           TIDL_caffeMapSliceParams },
  { "Crop",                            TIDL_caffeMapCropParams },
  { "Flatten",                         TIDL_caffeMapFlattenParams },
  { "Permute",                         TIDL_caffeMapPermuteParams },
  { "PriorBox",                        TIDL_caffeMapPriorBoxParams },
  { "Reshape",                         TIDL_caffeMapReshapeParams },
  { "DetectionOutput",                 TIDL_caffeMapDetectionOutputParams }
};

static bool isInplaceLayer(NetParameter& netStructure, int32_t idxProto)
{
  bool result = false;
  int32_t i1, i2;

  for (i1 = 0; i1 < netStructure.layer(idxProto).top_size(); i1++)
  {
    for(i2 = 0;i2 < netStructure.layer(idxProto).bottom_size(); i2++)
    {
      if(netStructure.layer(idxProto).top(i1) == netStructure.layer(idxProto).bottom(i2))
        result = true;
    }
  }
  return result;
}

int32_t TIDL_caffeGetLayerIdx(NetParameter& netStructure, const char *bufName)
{
  int32_t i, j, nameLen, layerIdx = -1;
  char layerName[TIDL_MAX_DATA_NAME];
  for (i = 0; i < netStructure.layer_size(); i++)
  {
    if (strcmp(netStructure.layer(i).name().c_str(), bufName) == 0)
    {
      layerIdx = i;
      break;
    }
  }
  return layerIdx;
}

int32_t TIDL_getCaffeLayerParamMapId(const char  * name)
{
  int32_t i = -1;
  for (i = 0; i < sizeof(tidl_caffeLayerParamMapTable) / sizeof(sTIDL_caffeLayerParamMap_t); i++)
  {
    if ((strcmp(name, (const char *)tidl_caffeLayerParamMapTable[i].name) == 0))
    {
      return (i);
    }
  }
  return (-1);
}

int32_t tidl_caffeLayerUpdateConsumerCount(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure,
                                           int32_t             i,
                                           int32_t             layerIndex,
                                           NetParameter&       netStructure)
{
  int32_t i0, i1, i2, i3;

  for (i0 = 0; i0 < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; i0++)
  {
    int32_t numCons = 0;
    const char* pCurrentLayerOutputName = (const char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[i0];
    for (i1 = 0; i1 < netStructure.layer_size(); i1++)
    {
      for(i2 = 0;i2 < netStructure.layer(i1).bottom_size(); i2++)
      {
        if (strcmp(pCurrentLayerOutputName, netStructure.layer(i1).bottom(i2).c_str()) == 0)
        {
          numCons++;
        }
      }
    }

    // In case input equals output
    for (i1 = 0; i1 < numNetOutData; i1++)
    {
      if (strcmp(pCurrentLayerOutputName, outDataNames[i1]) == 0)
      {
        numCons++;
      }
    }

    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerCnt[i0] = numCons;
  }
  return 0;
}

int32_t tidl_caffeLayerUpdateInPlaceConsumerCount(sTIDL_OrgNetwork_t *pOrgTIDLNetStructure,
                                           int32_t             i,
                                           int32_t             layerIndex,
                                           NetParameter&       netStructure,
                                           int32_t             inPlaceIdx,
                                           int32_t             inPlaceTotal)
{
  int32_t i0, i1, i2;

  for (i0 = 0; i0 < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; i0++)
  {

    int32_t numCons = 0;
    const char* pCurrentLayerOutputName = (const char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[i0];

    if(inPlaceIdx != 0)
    {
      // intermediate layer
      numCons = 1;
    }
    else
    {
      // Last layer only one output

      for (i1 = 0; i1 < netStructure.layer_size(); i1++)
      {
        for(i2 = 0;i2 < netStructure.layer(i1).bottom_size(); i2++)
        {
          if (strcmp(pCurrentLayerOutputName, netStructure.layer(i1).bottom(i2).c_str()) == 0 &&
              strcmp(netStructure.layer(i1).top(i0).c_str(), netStructure.layer(i1).bottom(i2).c_str()) != 0)
          {
            numCons++;
          }
        }
      }

      // In case input equals output
      for (i1 = 0; i1 < numNetOutData; i1++)
      {
        if (strcmp(pCurrentLayerOutputName, outDataNames[i1]) == 0)
        {
          numCons++;
        }
      }
    }

    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerCnt[i0] = numCons;
  }

  return 0;
}

int32_t tidl_caffeLayerFillTensorNames(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  NetParameter&        netStructure)
{
  int32_t j, k;
  strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].name, netStructure.layer(i).name().c_str());

  for (j = 0; j < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs; j++)
  {
    strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inDataNames[j], netStructure.layer(i).bottom(j).c_str());
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inData[j].dataId = -1;
  }

  for (j = 0; j < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; j++)
  {
    strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[j], netStructure.layer(i).top(j).c_str());
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerLinked[j] = 0;
  }

  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].weightsElementSizeInBits = NUM_WHGT_BITS;

  return 0;
}

int32_t tidl_caffeAddSingleNewLayer(sTIDL_OrgNetwork_t &orgTIDLNetStructure,
                                  int32_t            layerIndex,
                                  int32_t&           dataIndex,
                                  NetParameter&      netStructure,
                                  NetParameter&      netParams,
                                  int                caffeLayerIdx)
{
  int32_t         mapTblIdx  = -1;
  int32_t status = TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].layerType = TIDL_UnsupportedLayer;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = 1;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = 1;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].inData[0].dataId = -1;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].actParams.actType = TIDL_NoAct;

  mapTblIdx = TIDL_getCaffeLayerParamMapId(netStructure.layer(caffeLayerIdx).type().c_str());
  
  if (gParams.enableCustomLayers)
  {
#ifdef TIDL_CUSTOM_LAYER_API_v1
    status = TIDL_MapCustomParams(&orgTIDLNetStructure,
                                  caffeLayerIdx,
                                  layerIndex,
                                  &dataIndex,
                                  &netStructure,
                                  &netParams,
                                  TIDL_IMPORT_MODEL_FORMAT_CAFFE);
#elif defined(TIDL_CUSTOM_LAYER_API_v2)
    sTIDL_LayerPC_t* layer = &orgTIDLNetStructure.TIDLPCLayers[layerIndex];
    if(layer->weights.ptr == NULL)
    {
      status = TIDL_mapCustomParams_v2(layer,
                                    caffeLayerIdx,
                                    &netStructure,
                                    &netParams,
                                    TIDL_IMPORT_MODEL_FORMAT_CAFFE);
      
      if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        layer->outData[0].dataId = dataIndex++;
      }
    }
#endif
  }

  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    if (mapTblIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      TIDL_GLOBAL_REPORT_ERROR("%s layer %s is not supported now, By passing", netStructure.layer(caffeLayerIdx).type().c_str(), netStructure.layer(caffeLayerIdx).name().c_str());
      TIDL_caffeMapUnSuportedlayerParams(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, dataIndex, netStructure, netParams);
    }
    else
    {
      int32_t status = tidl_caffeLayerParamMapTable[mapTblIdx].tidl_caffeMapFunc(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, dataIndex, netStructure, netParams);
      if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        for(auto &diag: gDiags.gDiagList)
        {
          if(! diag.getString().empty())
          {
            gDiags.modelDiag(diag);
          }
        }
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
    }
  }
  tidl_caffeLayerFillTensorNames(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, netStructure);
  tidl_caffeLayerUpdateConsumerCount(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, netStructure);

  tidl_linkInputTensors(&orgTIDLNetStructure, layerIndex);
  tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_caffeLayerFillRenameTensorNames(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  NetParameter&        netStructure,
  int32_t              caffeLayerIdx,
  int32_t              inPlaceIdx,
  int32_t              inPlaceTotal)
{
  int32_t m, n;
  strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].name, netStructure.layer(i).name().c_str());

  // Fill in input data names
  for(n=0;n<netStructure.layer(caffeLayerIdx).bottom_size();n++)
  {
    // Multi-input & Multi-output with in-place
    if(inPlaceIdx != inPlaceTotal-1)
    {
      // not the last layer, bottom is changed
      char appendix[100];
      strcpy(appendix, netStructure.layer(caffeLayerIdx).bottom(n).c_str());
      sprintf((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inDataNames[n], "%s_TIDL_%d", appendix, inPlaceIdx+1);
    }
    else
    {
      // this is not a in-place layer, bottom not change
      strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inDataNames[n], netStructure.layer(caffeLayerIdx).bottom(n).c_str());
    }
  }

  // Fill in output data names
  for(m=0;m<netStructure.layer(caffeLayerIdx).top_size();m++)
  {
    if(inPlaceIdx == 0)
    {
      // last layer, top/output not change
      strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[m], netStructure.layer(caffeLayerIdx).top(m).c_str());
    }
    else
    {
      // not the last layer, top/output is changed
      char appendix[100];
      strcpy(appendix, netStructure.layer(caffeLayerIdx).top(m).c_str());
      sprintf((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[m], "%s_TIDL_%d", appendix, inPlaceIdx);
    }
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerLinked[m] = 0;
  }

  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].weightsElementSizeInBits = NUM_WHGT_BITS;

  return 0;
}

int32_t tidl_caffeAddRenameNewLayers(sTIDL_OrgNetwork_t &orgTIDLNetStructure,
                                  int32_t            layerIndex,
                                  int32_t&           dataIndex,
                                  NetParameter&      netStructure,
                                  NetParameter&      netParams,
                                  int                caffeLayerIdx,
                                  int                inPlaceIdx,
                                  int                inPlaceTotal)
{
  int k, m, n;
  int32_t         mapTblIdx  = -1;
  int32_t status= TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].layerType = TIDL_UnsupportedLayer;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = 1;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = 1;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].inData[0].dataId = -1;
  orgTIDLNetStructure.TIDLPCLayers[layerIndex].actParams.actType = TIDL_NoAct;

  mapTblIdx = TIDL_getCaffeLayerParamMapId(netStructure.layer(caffeLayerIdx).type().c_str());
  
  if (gParams.enableCustomLayers)
  {
#ifdef TIDL_CUSTOM_LAYER_API_v1
    status = TIDL_MapCustomParams(&orgTIDLNetStructure,
                                  caffeLayerIdx,
                                  layerIndex,
                                  &dataIndex,
                                  &netStructure,
                                  &netParams,
                                  TIDL_IMPORT_MODEL_FORMAT_CAFFE);
#elif defined(TIDL_CUSTOM_LAYER_API_v2)
    sTIDL_LayerPC_t* layer = &orgTIDLNetStructure.TIDLPCLayers[layerIndex];
    if(layer->weights.ptr == NULL)
    {
      status = TIDL_mapCustomParams_v2(layer,
                                    caffeLayerIdx,
                                    &netStructure,
                                    &netParams,
                                    TIDL_IMPORT_MODEL_FORMAT_CAFFE);
      
      if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        layer->outData[0].dataId = dataIndex++;
      }
    }
#endif
  }

  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    if (mapTblIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      TIDL_GLOBAL_REPORT_ERROR("%s layer %s is not supported now, By passing", netStructure.layer(caffeLayerIdx).type().c_str(), netStructure.layer(caffeLayerIdx).name().c_str());
      TIDL_caffeMapUnSuportedlayerParams(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, dataIndex, netStructure, netParams);
    }
    else
    {
      int32_t status = tidl_caffeLayerParamMapTable[mapTblIdx].tidl_caffeMapFunc(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, dataIndex, netStructure, netParams);
      if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        for(auto &diag: gDiags.gDiagList)
        {
          if(! diag.getString().empty())
          {
            gDiags.modelDiag(diag);
          }
        }
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
    } 
  }
  tidl_caffeLayerFillRenameTensorNames(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, netStructure, caffeLayerIdx, inPlaceIdx, inPlaceTotal);
  tidl_caffeLayerUpdateInPlaceConsumerCount(&orgTIDLNetStructure, caffeLayerIdx, layerIndex, netStructure, inPlaceIdx, inPlaceTotal);

  tidl_linkInputTensors(&orgTIDLNetStructure, layerIndex);
  tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_caffeProducerLayersCount(NetParameter& netStructure, const char *bufName, int producerList[])
{
  int32_t i, j, layerCount = 0;
  for (i = 0; i < netStructure.layer_size(); i++)
  {
    for(j=0;j<netStructure.layer(i).top_size();j++)
    {
      if ((strcmp(netStructure.layer(i).top(j).c_str(), bufName) == 0) &&
      (((netStructure.layer(i).include_size() == 1) &&
      (netStructure.layer(i).include(0).has_phase()) &&
      (netStructure.layer(i).include(0).phase() == TEST)) || (netStructure.layer(i).include_size() == 0)))
      {
        producerList[layerCount++] = i;
      }
    }
  }
  return layerCount;
}

int32_t tidl_caffeAddNewLayers(sTIDL_OrgNetwork_t &orgTIDLNetStructure,
                                  int32_t            layerIndex,
                                  int32_t&           dataIndex,
                                  NetParameter&      netStructure,
                                  NetParameter&      netParams)
{

  int32_t i, j, k, newLayerCount = 0;

  for (i = 0; i < layerIndex; i++)
  {
    for (j = 0; j < orgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
    {
      const char* inDataName = (const char *)orgTIDLNetStructure.TIDLPCLayers[i].inDataNames[j];

      if (TIDL_getLayerIdx(&orgTIDLNetStructure, layerIndex, inDataName) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        // if the input data is not in orgTIDLNetStructure

        // check if there is in place layer is in Caffe Net
        #define PRODUCER_IDX_MAX (64)
        int producerList[PRODUCER_IDX_MAX];
        int producerCount = TIDL_caffeProducerLayersCount(netStructure, inDataName, producerList);

        if(producerCount > 1)
        {
          // in place layer detected. Backwards
          for(k=producerCount;k>0;k--)
          {
            TIDL_IMPORT_CHECK_AND_RETURN(tidl_caffeAddRenameNewLayers(orgTIDLNetStructure, layerIndex, dataIndex, netStructure, netParams, producerList[k-1], producerCount-k, producerCount), "");
            layerIndex++;
          }

        }
        else if(producerCount == 1)
        {
          // not in place layer.
          TIDL_IMPORT_CHECK_AND_RETURN(tidl_caffeAddSingleNewLayer(orgTIDLNetStructure, layerIndex, dataIndex, netStructure, netParams, producerList[0]), "");
          layerIndex++;
        }
        else
        {
          // Handle corner error case: if this data is not provided as a Input layer.
          TIDL_GLOBAL_REPORT_ERROR("Cannot import this model. Some data is required but not provided as Input data layer. Please double check your model's input data top names");
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }

        newLayerCount += producerCount;
      }
    }
  }

  return newLayerCount;
}


int TIDL_caffeConvertCaffemodelToFloat(NetParameter &netParams)
{
  int i, j, k;
  int layerNum = netParams.layer_size();
  for (i = 0; i < layerNum; i++)
  {
    for (j = 0; j < netParams.layer(i).blobs_size(); j++)
    {
      if (netParams.layer(i).blobs(j).has_raw_data())
      {
        Type raw_type = netParams.layer(i).blobs(j).raw_data_type();
        const ::std::string& hd = netParams.layer(i).blobs(j).raw_data();
        if (raw_type == caffe::FLOAT)
        {
          int data_size = hd.size() / 4;
          float *Y = (float*)(&hd.front());
          BlobProto & blob = (BlobProto&)netParams.layer(i).blobs(j);
          for (k = 0; k < data_size; k++)
          {
            blob.add_data(Y[k]);
          }
        }
        else
        {
          TIDL_GLOBAL_REPORT_UNSUPPORTED("Unsupported raw_data_type, only caffe::FLOAT is supported");
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
      }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

TIDL_TFLayerMapping_t TIDL_CaffeLayerMap[] =
{
  { (char*)"TIDL_OnnxShuffle",        (char*)"TIDL_ReshapeLayerTIDL_TransposeLayerTIDL_ReshapeLayer"   , 3 },
  { (char*)"TIDL_TFSlimShuffle",        (char*)"NANA"              , 3 }
};

int32_t tidl_FindCaffeShuffleLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t i1, i2, i3, i4;
  int32_t status = 0;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
    if (tf_isLayerType("TIDL_OnnxShuffle", i1, pOrgTIDLNetStructure, TIDL_CaffeLayerMap, (sizeof(TIDL_CaffeLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
    {
      int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_OnnxShuffle", TIDL_CaffeLayerMap, (sizeof(TIDL_CaffeLayerMap) / sizeof(TIDL_TFLayerMapping_t)));
      pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType = TIDL_FlattenLayer;

      sTIDL_LayerPC_t &TIDLPCLayers1 = pOrgTIDLNetStructure.TIDLPCLayers[i1];
      int32_t nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
      sTIDL_LayerPC_t &TIDLPCLayers2 = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];
      nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx].outData[0].dataId);
      sTIDL_LayerPC_t &TIDLPCLayers3 = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];

      if ((TIDLPCLayers1.weights.bufSize ==  5) && (TIDLPCLayers2.weights.bufSize ==  5) && (TIDLPCLayers3.weights.bufSize == 4))
      {
        int64_t * reshape1  = (int64_t *)TIDLPCLayers1.weights.ptr;
        int64_t * reshape2  = (int64_t *)TIDLPCLayers3.weights.ptr;
        int32_t * transpose = (int32_t *)TIDLPCLayers2.weights.ptr;
        if ((reshape1[4] == reshape2[3]) && (reshape1[3] == reshape2[2]) && (4 == transpose[4]) && (3 == transpose[3]) && (1 == transpose[2]) && (2 == transpose[1]))
        {
          pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0] = TIDLPCLayers3.outData[0];
          strcpy((char *)pOrgTIDLNetStructure.TIDLPCLayers[i1].outDataNames[0], (char *)TIDLPCLayers3.outDataNames[0]);
          pOrgTIDLNetStructure.TIDLPCLayers[i1].outConsumerCnt[0] = TIDLPCLayers3.outConsumerCnt[0];

          TIDLPCLayers2.numInBufs = -1;
          TIDLPCLayers2.numOutBufs = -1;
          TIDLPCLayers3.numInBufs = -1;
          TIDLPCLayers3.numOutBufs = -1;

          TIDLPCLayers1.layerType = TIDL_ShuffleChannelLayer;
          TIDLPCLayers1.layerParams.shuffleLayerParams.numGroups = reshape1[1];
        }
      }
    }
  }
  return 0;
}

int32_t tidl_findCaffeInputNames(NetParameter& netStructure, char * inList)
{
  int i;
  char tensorName[FILE_NAME_SIZE];
  char inTensorName[FILE_NAME_SIZE];
  int outPutSize = 0;

  // detect Input Layer
  for (i = 0; i < netStructure.layer_size(); i++)
  {
    if (netStructure.layer(i).type() == "Input")
    {
        strcat(inList, netStructure.layer(i).top(0).c_str());
        strcat(inList, ",");
    }
    else
    {
      /* This layer is not input layer, do nothing here */
    }
  }

  // To comptatable to another expression format
  if(strlen(inList) == 0)
  {
    for (i = 0; i < netStructure.input_size(); i++)
    {
        strcat(inList, netStructure.input(i).c_str());
        strcat(inList, ",");
    }
    return (0);
  }
  else
  {
    return 1;
  }
}

int32_t tidl_findCaffeOutputNames(NetParameter& netStructure, char * outList)
{
  int i0, i1, i2, i3;
  char tensorName[FILE_NAME_SIZE];
  char inTensorName[FILE_NAME_SIZE];
  int outPutSize = 0;

  for (i0 = 0; i0 < netStructure.layer_size(); i0++)
  {
    for(i3 = 0; i3 < netStructure.layer(i0).top_size(); i3++)
    {
      // non-zero means terminal layer.
      int compare_result = 1;
      for (i1 = 0; i1 < netStructure.layer_size(); i1++)
      {
        for (i2 = 0; i2 < netStructure.layer(i1).bottom_size(); i2++)
        {
          // If I am not the bottom of another layer
          // && I will not compare to myself
          // && do not compare with in-place layers
          if(netStructure.layer(i1).bottom(i2) ==  netStructure.layer(i0).top(i3) &&
            i0 != i1 &&
            !isInplaceLayer(netStructure, i1))
          {
            compare_result = 0;
          }
          else
          {
            /* Do nothing here */
          }
        }
      }

      // Handle corner error case: Input layer is not used by anyone.
      if(compare_result == 1 && netStructure.layer(i0).bottom_size() == 0)
      {
        compare_result = 0;
        TIDL_GLOBAL_REPORT_ERROR("Input layer is not used by any other layer. The prototxt must be wrong");
      }

      // check flag && in-place layer should not be output.
      if(compare_result == 1 && !isInplaceLayer(netStructure, i0))
      {
        strcat(outList, netStructure.layer(i0).top(i3).c_str());
        strcat(outList, ",");
      }
    }
  }
  return (0);
}

int32_t caffe_import_new(tidl_import_config * params, int32_t *totalData, int32_t* totalLayers)
{
  int32_t         i,j;

  /* Prototxt */
  NetParameter    netStructure;
  /* Caffemodel */
  NetParameter    netParams;

  /* total layer number in orgTIDLNetStructure */
  int32_t         layerIndex = 0;

  /* total dataId number */
  int32_t         dataIndex  = 0;


  /*
   * Convert RawData in caffemodel to Float Array
   */

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Caffe network file : %s", (const char *)params->inputNetFile);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Caffe model file   : %s", (const char *)params->inputParamsFile);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL network file  : %s", (const char *)params->outputNetFile);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL IO info file  : %s", (const char *)params->outputParamsFile);

  bool success = TIDL_readProtoFromTextFile((const char *)params->inputNetFile, &netStructure);
  if(!success){
    TIDL_GLOBAL_REPORT_ERROR("Reading prototext file failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  success = TIDL_readProtoFromBinaryFile((const char *)params->inputParamsFile, &netParams);
  if(!success){
    TIDL_GLOBAL_REPORT_ERROR("Reading models params file failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_caffeConvertCaffemodelToFloat(netParams), "");

  if (netStructure.has_name())
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Name of the network : %15s", netStructure.name().c_str());
  }



  /*
   * Handle Output Data Names
   * Steps:
   * 1. Search output data names from config file
   * 2. Search all data names without a consumer
   * 3. Add data layers with different shape descriptions
   */
  if (strcmp((char*)params->outDataNamesList, "") == 0)
  {
    char outDataNamesList[TIDL_MAX_ALG_OUT_BUFS * TIDL_STRING_SIZE] = "";
    tidl_findCaffeOutputNames(netStructure, (char*)outDataNamesList);
    /* Break Single line names into pieces */
    numNetOutData = tidl_getStringsFromList((char *)outDataNamesList, (char*)outDataNames, TIDL_MAX_DATA_NAME);
  }
  else
  {
    /* Break Single line names into pieces */
    numNetOutData = tidl_getStringsFromList((char *)params->outDataNamesList, (char*)outDataNames, TIDL_MAX_DATA_NAME);
  }
  /* Add data layers to local structure */
  for (i = 0; i < numNetOutData; i++)
  {
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].layerType = TIDL_DataLayer;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = -1;
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].name, outDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].inDataNames[0], outDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[0], outDataNames[i]);
    layerIndex++;
  }


  /*
   * Handle Input Data Names
   * Steps:
   * 1. Search all the input data names
   * 2. Add data layers with different shape descriptions
   *
   * Current support:
   * 1. input layer
   * 2. input_param.shape.dim
   * 3. input_shape.dim
   */
  int flag_layerStyleInput = 0;
  if (strlen((char *)params->inDataNamesList) == 0)
  {
    char inDataNamesList[500] = "";
    flag_layerStyleInput = tidl_findCaffeInputNames(netStructure, (char*)inDataNamesList);
    /* Break Single line names into pieces */
    numNetInData = tidl_getStringsFromList((char *)inDataNamesList, (char *)inDataNames, TIDL_MAX_DATA_NAME);
  }
  else
  {
    /* Break Single line names into pieces */
    numNetInData = tidl_getStringsFromList((char *)params->inDataNamesList, (char *)inDataNames, TIDL_MAX_DATA_NAME);
  }

  /* Add data layers to local structure */
  for (i = 0; i < numNetInData; i++)
  {
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].layerType = TIDL_DataLayer;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = -1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = 1;
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].name, inDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[0], inDataNames[i]);
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerCnt[0] = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerLinked[0] = 0;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dataId = dataIndex++;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].numDim = TIDL_DIM_MAX;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].minValue = 0x7FFFFFFFU;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].maxValue = 0x80000000U;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_DIM1] = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_DIM2] = 1;
    if(flag_layerStyleInput)
    {
      // input layer format
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_BATCH] = netStructure.layer(i).input_param().shape(0).dim(0);
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_NUMCH] = netStructure.layer(i).input_param().shape(0).dim(1);
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_HEIGHT] = netStructure.layer(i).input_param().shape(0).dim(2);
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_WIDTH ] = netStructure.layer(i).input_param().shape(0).dim(3);
    }
    else
    {
      if(netStructure.input_shape_size() > 0)
      {
        // input_shape format
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_BATCH] = netStructure.input_shape(i).dim(0);
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_NUMCH] = netStructure.input_shape(i).dim(1);
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_HEIGHT] = netStructure.input_shape(i).dim(2);
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_WIDTH ] = netStructure.input_shape(i).dim(3);
      }
      else if(netStructure.input_dim_size() > 0)
      {
        // input_dim format
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_BATCH] = netStructure.input_dim(0);
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_NUMCH] = netStructure.input_dim(1);
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_HEIGHT] = netStructure.input_dim(2);
        orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_WIDTH ] = netStructure.input_dim(3);
      }

    }
#if 0   // Now added batch processing support, so commented this code
    if(orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_BATCH] > 1)
    {
      TIDL_GLOBAL_REPORT_WARNING("Detected batch process from input size, overwritten to single frame inference");
      orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dimValues[TIDL_DIM_BATCH] = 1;
    }
#endif

    // Prepare the consumer count for input data
    tidl_caffeLayerUpdateConsumerCount(&orgTIDLNetStructure, layerIndex, layerIndex, netStructure);
    tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);
    layerIndex++;
  }

  /*
   * Parsing All model Structure backwards
   * Steps:
   * 1. Search for all output data names
   * 2. Check the provider of the output data names
   * 3. Recursively add the provider.
   */
  int newLayerCount;
  do
  {
    newLayerCount = tidl_caffeAddNewLayers(orgTIDLNetStructure, layerIndex, dataIndex, netStructure, netParams);
    if(newLayerCount==TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL){
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    layerIndex += newLayerCount;
  }
  while(newLayerCount > 0);
  *totalData = dataIndex;
  *totalLayers = layerIndex;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
