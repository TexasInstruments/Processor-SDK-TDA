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
#include <float.h>
#include <cmath>
#include <limits>

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tidl_import_config.h"
//#include "onnx/onnx-ml.proto3.pb.h"
#include "onnx/onnx-ml.proto3.pb.h"
#include "tidl_custom_import.h"
#include "tidl_import_common.h"
#include "tidl_onnxImport.h"
#include "tidl_import_common_model_check.h"
#include "tidl_import_common_tidlrt_osrt.h"
#include "tidl_deviceInfo.h"

#include "tidlParseOnnx/tidl_parse_onnx.h"
#include "tidlAllowlistingConstraints/tidl_constraint.h"
#define __MODULE__ "[PARSER]"

using namespace std;
using namespace onnx;
using ::google::protobuf::Message;
using ::google::protobuf::io::FileInputStream;
using ::google::protobuf::io::FileOutputStream;
using ::google::protobuf::io::ZeroCopyInputStream;
using ::google::protobuf::io::CodedInputStream;
using ::google::protobuf::io::ZeroCopyOutputStream;
using ::google::protobuf::io::CodedOutputStream;


#include "tidl_import_common.h"
#define HIGHEST_OPSET_VERSION_SUPPORTED 18
int64_t onnxOpSetVersion;

extern sTIDL_OrgNetwork_t      orgTIDLNetStructure;
extern sTIDL_OrgNetwork_t      tempTIDLNetStructure;
extern sTIDL_Network_t         tIDLNetStructure;

static const unordered_set<std::string> layersWithMultipleOutputBuffers = { 
  "Split",
  "TopK",
  "LSTM",
  "GRU",
  "RNN"
};

map<int,int>onnxToTidlTypeMap  = {
    {1, TIDL_SinglePrecFloat}, //ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT
    {2, TIDL_UnsignedChar},  // ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8
    {3, TIDL_SignedChar}, // ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8
    {4, TIDL_UnsignedShort}, // ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16
    {5, TIDL_SignedShort}, // ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16
    {6, TIDL_SignedWord}, // ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32
    {12, TIDL_UnsignedWord}, // ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32
    {7, TIDL_SignedDoubleWord}, // ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64
    {13, TIDL_UnsignedDoubleWord}, // ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64
    {9, TIDL_Bool}, //ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL
  };

extern "C"
{
int32_t TIDL_isInputConstInGraph(GraphProto &onnGraph, const string name);
int32_t TIDL_isInputConstInGraph(GraphProto& onnGraph, const string name)
{
  int i;
  for (i = 0; i < numNetInData; i++)
  {
    if ((strcmp(inDataNames[i], name.c_str()) == 0))
    {
      return(0);
    }
  }
  for (i = 0; i < onnGraph.initializer_size(); i++)
  {
    if ((strcmp(onnGraph.initializer(i).name().c_str(), name.c_str()) == 0))
    {
      return(1);
    }
  }
  for (i = 0; i < onnGraph.node_size(); i++)
  {
    if ((strcmp(onnGraph.node(i).output(0).c_str(), name.c_str()) == 0) && (strcmp(onnGraph.node(i).op_type().c_str(), "Constant") == 0))
    {
      return(1);
    }
  }
  return (0);
}
}

int32_t TIDL_onnxGetNodeIdxFromInput(GraphProto& onnxGraph, const char *bufName)
{
  int32_t i, j, nameLen, nodeIdx = -1;
  char nodeName[TIDL_MAX_DATA_NAME];
  char inDataName[TIDL_MAX_DATA_NAME];
  for (i = 0; i < onnxGraph.node_size(); i++)
  {
    for (j = 0; j < onnxGraph.node(i).input_size(); j++)
    {
      if (strcmp(onnxGraph.node(i).input(j).c_str(), bufName) == 0)
      {
        nodeIdx = i;
        break;
      }
    }
  }
  return nodeIdx;
}

int32_t TIDL_onnxGetNodeIdx(GraphProto&             onnxGraph, const char *bufName)
{
  int32_t i, j, nameLen, nodeIdx = -1;
  char nodeName[TIDL_MAX_DATA_NAME];
  char inDataName[TIDL_MAX_DATA_NAME];
  for (i = 0; i < onnxGraph.node_size(); i++)
  {
    for (j = 0; j < onnxGraph.node(i).output_size(); j++)
    {
      if (strcmp(onnxGraph.node(i).output(j).c_str(), bufName) == 0)
      {
        nodeIdx = i;
        break;
      }
    }
  }
  return nodeIdx;
}

std::vector<int> TIDL_getInputNodes(int idx, GraphProto& onnxGraph)
{
  std::vector<int> inputIndices;
  for(int inIdx=0;inIdx<onnxGraph.node(idx).input_size();inIdx++)
  {
    for(int j=0;j<onnxGraph.node_size();j++)
    {
      for(int outIdx=0;outIdx<onnxGraph.node(j).output_size();outIdx++)
      {
        if(onnxGraph.node(j).output(outIdx).compare(onnxGraph.node(idx).input(inIdx)) == 0)
        {
          inputIndices.push_back(j);
          break;
        }
      }
    }

    //check if input exists in onnxGraph input list
    for (int k = 0; k < onnxGraph.input_size(); k++)
    {
      if((strcmp(onnxGraph.input(k).name().c_str(), onnxGraph.node(idx).input(inIdx).c_str()) == 0))
      {
        /* Using -1 as placeHolder to differentiate the main inputs and node inputs */
        inputIndices.push_back(-1);
        break;
      }
    }
  }
  return inputIndices;
}

std::vector<int> TIDL_getOutputNodes(int idx, GraphProto& onnxGraph)
{
  std::vector<int> outputIndices;
  for(int outIdx=0;outIdx<onnxGraph.node(idx).output_size();outIdx++)
  {
    for(int j=0;j<onnxGraph.node_size();j++)
    {
      for(int inIdx=0;inIdx<onnxGraph.node(j).input_size();inIdx++)
      {
        if(onnxGraph.node(j).input(inIdx).compare(onnxGraph.node(idx).output(outIdx)) == 0)
        {
          outputIndices.push_back(j);
          break;
        }
      }
    }

    //check if output exists in onnxGraph output list
    for (int k = 0; k < onnxGraph.output_size(); k++)
    {
      if((strcmp(onnxGraph.output(k).name().c_str(), onnxGraph.node(idx).output(outIdx).c_str()) == 0))
      {
        /* Using -1 as placeHolder to differentiate the main inputs and node inputs */
        outputIndices.push_back(-1);
        break;
      }
    }
  }

  return outputIndices;
}

bool TIDL_doesQuantOrDequantHaveConstTensor(GraphProto& onnGraph, const string name)
{
  /*Check if buffer is output of dequant, and if part of dequant check if it is a constant buffer */
  bool isConstBuf = false;
  /*Find the corresponding DQ layer via it's output name*/
  int32_t DQNodeIdx = TIDL_onnxGetNodeIdx(onnGraph, name.c_str());
  /*Check the buffer to be dequantized*/
  if(DQNodeIdx != -1)
  {
    /*
    Previous layer is Dequant AND
    Input to previous layer is const*/
    if(strcmp(onnGraph.node(DQNodeIdx).op_type().c_str(), "DequantizeLinear") == 0)
    {
      if(TIDL_isInputConstInGraph(onnGraph, onnGraph.node(DQNodeIdx).input(0)))
      {
        isConstBuf = true;
      }
    }

    /**
     * Check if dequant has quant as input whether quant has a const input
    */
    if (isConstBuf == false)
    {
      int32_t QNodeIdx = TIDL_onnxGetNodeIdx(onnGraph, onnGraph.node(DQNodeIdx).input(0).c_str());

      if (QNodeIdx != -1)
      {
        if(strcmp(onnGraph.node(QNodeIdx).op_type().c_str(), "QuantizeLinear") == 0)
        {
          if(TIDL_isInputConstInGraph(onnGraph, onnGraph.node(QNodeIdx).input(0)))
          {
            isConstBuf = true;
          }          
        }
      }
    }
  }
 return isConstBuf;
}


/** Function to compute number of variable inputs of layer
 * Also populates corresponding indices of variable inputs
 */
int32_t TIDL_onnxNumVariableLayerInputs(GraphProto& onnGraph, int32_t i, std::vector<int32_t>& varIdx)
{
  int32_t numVarInputs = 0;
  for(int j = 0; j < onnGraph.node(i).input_size(); j++)
  {
    if((strcmp(onnGraph.node(i).input(j).c_str(),"") != 0) && !(TIDL_isInputConstInGraph(onnGraph, onnGraph.node(i).input(j)) || TIDL_doesQuantOrDequantHaveConstTensor(onnGraph, onnGraph.node(i).input(j))))
    {
      numVarInputs++;
      varIdx.push_back(j);
    }
  }
  return numVarInputs;
}

int32_t TIDL_onnxGetAttrIdx(const NodeProto& node, char * name)
{
  int32_t i;
  for (i = 0; i < node.attribute_size(); i++)
  {
    if ((strcmp(node.attribute(i).name().c_str(), name) == 0))
    {
      return(i);
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
}

std::vector<int32_t> getNodeOutputShape(GraphProto& onnxGraph, const std::string name, int32_t debugLevel)
{
  std::vector<int32_t> dims;
  int32_t nodeFound = 0;

  for (int k = 0; k < onnxGraph.output_size(); k++)
  {
    if((strcmp(onnxGraph.output(k).name().c_str(), name.c_str()) == 0))
    {
      if( onnxGraph.output(k).type().tensor_type().has_shape() )
      {
        auto& tensorShape = onnxGraph.output(k).type().tensor_type().shape();
        for (int j = 0; j < tensorShape.dim_size(); j++)
        {
          nodeFound = 1;
          dims.push_back(tensorShape.dim(j).dim_value());
        }
      }
    }
  }

  if (nodeFound == 0)
  {
    for (int k = 0; k < onnxGraph.value_info_size(); k++)
    {
      if((strcmp(onnxGraph.value_info(k).name().c_str(), name.c_str()) == 0))
      {
        if( onnxGraph.value_info(k).type().tensor_type().has_shape() )
        {
          auto& tensorShape = onnxGraph.value_info(k).type().tensor_type().shape();
          for (int j = 0; j < tensorShape.dim_size(); j++)
          {
            dims.push_back(tensorShape.dim(j).dim_value());
          }
        }
      }
    }
  }

  return dims;
}

std::vector<int32_t> getNodeInputShape(GraphProto& onnxGraph, const std::string name, int32_t debugLevel)
{
  std::vector<int32_t> dims;
  int32_t nodeFound = 0;

  for (int k = 0; k < onnxGraph.input_size(); k++)
  {
    if((strcmp(onnxGraph.input(k).name().c_str(), name.c_str()) == 0))
    {
      if( onnxGraph.input(k).type().tensor_type().has_shape() )
      {
        auto& tensorShape = onnxGraph.input(k).type().tensor_type().shape();
        for (int j = 0; j < tensorShape.dim_size(); j++)
        {
          dims.push_back(tensorShape.dim(j).dim_value());
          nodeFound = 1;
        }
      }
    }
  }


  if (nodeFound == 0)
  {
    for (int k = 0; k < onnxGraph.value_info_size(); k++)
    {
      if((strcmp(onnxGraph.value_info(k).name().c_str(), name.c_str()) == 0))
      {
        if( onnxGraph.value_info(k).type().tensor_type().has_shape() )
        {
          auto& tensorShape = onnxGraph.value_info(k).type().tensor_type().shape();
          for (int j = 0; j < tensorShape.dim_size(); j++)
          {
            dims.push_back(tensorShape.dim(j).dim_value());
            nodeFound = 1;
          }
        }
      }
    }
  }

  /**
   * Protecting the search under the flag as the dims vector can be filled twice when there is a node present as
   * both input & output in the onnx graph.
  */
  if (nodeFound == 0)
  {
    for (int k = 0; k < onnxGraph.output_size(); k++)
    {
      if((strcmp(onnxGraph.output(k).name().c_str(), name.c_str()) == 0))
      {
        if( onnxGraph.output(k).type().tensor_type().has_shape() )
        {
          auto& tensorShape = onnxGraph.output(k).type().tensor_type().shape();
          for (int j = 0; j < tensorShape.dim_size(); j++)
          {
            dims.push_back(tensorShape.dim(j).dim_value());
          }
        }
      }
    }
  }
  return dims;
}

int32_t TIDL_onnxGetIntAttr(const NodeProto& node, char * name, int32_t * valuePtr, int32_t idx)
{
  int32_t i = TIDL_onnxGetAttrIdx(node, name);
  if(i != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    std::string nodeName;
    int64_t value;

    // Get attribute
    if (node.attribute(i).ints_size() > idx)
    {
      value = (int64_t)node.attribute(i).ints(idx);
    }
    else if (idx == 0)
    {
      value = (int64_t)node.attribute(i).i();
    }
    else
    {
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }

    // Get node name
    nodeName = (!node.name().empty()) ? node.name() : node.output(0);

    // Restrict value if beyond INT32 range to avoid overflow
    if (value > INT32_MAX)
    {
      value = (int32_t)INT32_MAX;
      TIDL_GLOBAL_REPORT_WARNING("Value of '%s' in %s is more than INT32_MAX. Restricting value to INT32_MAX", name, nodeName.c_str());
    }
    else if (value < INT32_MIN)
    {
      value = (int32_t)INT32_MIN;
      TIDL_GLOBAL_REPORT_WARNING("Value of '%s' in %s is less than INT32_MIN. Restricting value to INT32_MIN", name, nodeName.c_str());
    }

    *valuePtr = (int32_t)value;
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
}

int32_t TIDL_onnxGetFloatAttr(const NodeProto& node, char * name, float * valuePtr, int32_t idx)
{
  int32_t i = TIDL_onnxGetAttrIdx(node, name);
  if (i != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    if (node.attribute(i).floats_size() > idx)
    {
      *valuePtr = node.attribute(i).floats(idx);
    }
    else if (idx == 0)
    {
      *valuePtr = node.attribute(i).f();
    }
    else
    {
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
}

int32_t TIDL_geTensorDim(TensorProto& tensor, int32_t idx)
{
  if (tensor.dims_size() > idx)
  {
    return(tensor.dims(idx));
  }
  else
  {
    return 0;
  }
}


int32_t TIDL_onnxGetStringAttr(const NodeProto& node, char * name, char * valuePtr, int32_t idx)
{
  strcpy(valuePtr, "");
  int32_t i = TIDL_onnxGetAttrIdx(node, name);
  if (i != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
      strcpy(valuePtr, node.attribute(i).s().c_str());
      return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
}

TensorProto TIDL_getInitializerTensor(GraphProto& onnGraph, const string name, int32_t nIdx, int32_t& status)
{
  int i;
  status = 0;
  for (i = 0; i < onnGraph.initializer_size(); i++)
  {
    if ((strcmp(onnGraph.initializer(i).name().c_str(), name.c_str()) == 0))
    {
      auto & tensor = onnGraph.initializer(i);
      return(tensor);
    }
  }
  for (i = 0; i < onnGraph.node_size(); i++)
  {
    if ((strcmp(onnGraph.node(i).output(0).c_str(), name.c_str()) == 0) && (strcmp(onnGraph.node(i).op_type().c_str(), "Constant") == 0))
    {
      auto & tensor = onnGraph.node(i).attribute(0).t();
      return(tensor);
    }
    else if((strcmp(onnGraph.node(i).output(0).c_str(), name.c_str()) == 0) && (strcmp(onnGraph.node(i).op_type().c_str(), "DequantizeLinear") == 0))
    {
      int32_t DQNodeIdx = TIDL_onnxGetNodeIdx(onnGraph, name.c_str());
      /*Return DQ's input (inIdx)*/
      if(DQNodeIdx != -1)
      {
        return (TIDL_getInitializerTensor(onnGraph, onnGraph.node(DQNodeIdx).input(0) , DQNodeIdx, status));
      }
      else
      {
        continue;
      }
    }
    else if((strcmp(onnGraph.node(i).output(0).c_str(), name.c_str()) == 0) && (strcmp(onnGraph.node(i).op_type().c_str(), "QuantizeLinear") == 0))
    {
      int32_t QNodeIdx = TIDL_onnxGetNodeIdx(onnGraph, name.c_str());
      /*Return Q's input (inIdx)*/
      if(QNodeIdx != -1)
      {
        return (TIDL_getInitializerTensor(onnGraph, onnGraph.node(QNodeIdx).input(0) , QNodeIdx, status));
      }
      else
      {
        continue;
      }
    }
  }
  status = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  return (onnx::TensorProto::default_instance());
}

bool TIDL_tfIsTensorDQ(int32_t dType)
{
  bool isDQ = false;
  if(dType == TensorProto_DataType_INT32  ||
     dType == TensorProto_DataType_INT8   ||
     dType == TensorProto_DataType_UINT8)
     {
      isDQ = true;
     }
  return isDQ;
}

int32_t TIDL_tfCopyTensor(GraphProto& onnGraph, TensorProto& tensor, void ** ptr, int32_t * size, const string name)
{
  int32_t i, tensorSize = 1;

  * ptr = NULL;
  if (tensor.data_type() == TensorProto_DataType_FLOAT) //DT_FLOAT
  {
    float *dst;
    for (i = 0; i < tensor.dims_size(); i++)
    {
      tensorSize *= tensor.dims(i);
    }
    *size = tensorSize;
    dst = (float *)my_malloc(*size *sizeof(float));

    if (tensor.float_data_size() > 0)
    {
      if (tensor.float_data_size() != tensorSize)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Tensor size and Dims size is not matching");
      }

      for (i = 0; i < tensorSize; i++)
      {
        dst[i] = tensor.float_data(i);
      }
    }
    else if (tensor.raw_data().size() > 0)
    {
      if ((tensor.raw_data().size() / sizeof(float)) != tensorSize)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Tensor size and Dims size is not matching");
      }
      memcpy(dst, (float *)tensor.raw_data().c_str(), tensor.raw_data().size());
    }
    *ptr = dst;
    return 0;
  }
  else if (tensor.data_type() == TensorProto_DataType_INT64) //DT_FLOAT
  {
    long long int *dst;
    for (i = 0; i < tensor.dims_size(); i++)
    {
      tensorSize *= tensor.dims(i);
    }
    *size = tensorSize;
    dst = (long long int *)my_malloc(*size *sizeof(long long int));

    if (tensor.int64_data_size() > 0)
    {
      if (tensor.int64_data_size() != tensorSize)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Tensor size and Dims size is not matching");
      }

      for (i = 0; i < tensorSize; i++)
      {
        dst[i] = tensor.int64_data(i);
      }
    }
    else if (tensor.raw_data().size() > 0)
    {
      if ((tensor.raw_data().size() / sizeof(long long int)) != tensorSize)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Tensor size and Dims size is not matching");
      }
      memcpy(dst, (long long int *)tensor.raw_data().c_str(), tensor.raw_data().size());
    }
    *ptr = dst;
    return 0;
  }
  /*All DQ Combinations:*/
  else if(TIDL_tfIsTensorDQ(tensor.data_type()))
  {
      /*Get DQ Node:*/
      bool tensorScaleFromRaw = false;
      bool tensorZPFromRaw = false;
      int32_t DQNodeIdx = TIDL_onnxGetNodeIdxFromInput(onnGraph, tensor.name().c_str());
      if(DQNodeIdx == -1)
      {
        TIDL_GLOBAL_REPORT_UNSUPPORTED("Unsupported data type for onnxImport");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      int32_t status = 0;
      TensorProto tensorScales  = TIDL_getInitializerTensor(onnGraph, onnGraph.node(DQNodeIdx).input(1), DQNodeIdx, status);
      if(status != 0)
      {
        TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index %d for node %s", 1, onnGraph.node(DQNodeIdx).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      TensorProto tensorZeroPoints = TIDL_getInitializerTensor(onnGraph, onnGraph.node(DQNodeIdx).input(2), DQNodeIdx, status);
      if(status != 0)
      {
        TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index %d for node %s", 2, onnGraph.node(DQNodeIdx).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      tensorScaleFromRaw = tensorScales.float_data_size() == 0;
      tensorZPFromRaw = tensorZeroPoints.int32_data_size() == 0;
      /*Scales and ZeroPoints are 1D*/
      //assert(tensorScales.dims(0) == tensorZeroPoints.dims(0));
      int32_t numOutChannels = 1;
      if(tensorScales.dims_size() > 0)
      {
        numOutChannels = tensorScales.dims(0);
      }

      int32_t i,j;

      for(i = 0 ; i < tensor.dims_size(); i++)
      {
        tensorSize *= tensor.dims(i);
      }
      *size = tensorSize;
      int32_t perChannelSize = tensorSize / numOutChannels;
      /*Allocate float buffer:*/
      float *output = NULL;
      output = (float*) malloc(sizeof(float) * tensorSize);
      *ptr = output;
      if(output == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("Unable to allocate buffer for dequantized parameters");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      if(tensor.data_type() == TensorProto_DataType_INT32)
      {
        if(TensorProto_DataType_INT32 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            int32_t tensorZeroPoint;
            float  tensorScale;

            if(tensorZPFromRaw)
            {
              tensorZeroPoint = ((int32_t* )(tensorZeroPoints.raw_data().c_str()))[i];
            }
            else
            {
              tensorZeroPoint = (int32_t)(tensorZeroPoints.int32_data(i));
            }

            if(tensorScaleFromRaw)
            {
              tensorScale = ((float* )(tensorScales.raw_data().c_str()))[i];
            }
            else
            {
              tensorScale = (float )(tensorScales.float_data(i));
            }
            for(j = 0; j < perChannelSize; j++)
            {
              int32_t quantizedTensorVal = ((int32_t* )(tensor.raw_data().c_str()))[(i * perChannelSize) + j];
              output[(i * perChannelSize) + j] = ((((float)quantizedTensorVal) - tensorZeroPoint) * tensorScale);
            }
          }
        }
      }
      else if(tensor.data_type() == TensorProto_DataType_INT8)
      {
        if(TensorProto_DataType_INT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            int8_t tensorZeroPoint;
            float  tensorScale;

            if(tensorZPFromRaw)
            {
              tensorZeroPoint = ((int8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
            }
            else
            {
              tensorZeroPoint = (int8_t)(tensorZeroPoints.int32_data(i));
            }

            if(tensorScaleFromRaw)
            {
              tensorScale = ((float* )(tensorScales.raw_data().c_str()))[i];
            }
            else
            {
              tensorScale = (float )(tensorScales.float_data(i));
            }
            for(j = 0; j < perChannelSize; j++)
            {
              int8_t quantizedTensorVal = ((int8_t* )(tensor.raw_data().c_str()))[(i * perChannelSize) + j];
              output[(i * perChannelSize) + j] = ((((float)quantizedTensorVal) - tensorZeroPoint) * tensorScale);
            }
          }
        }
        else if(TensorProto_DataType_UINT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            uint8_t tensorZeroPoint;
            float  tensorScale;

            if(tensorZPFromRaw)
            {
              tensorZeroPoint = ((uint8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
            }
            else
            {
              tensorZeroPoint = (uint8_t)(tensorZeroPoints.int32_data(i));
            }

            if(tensorScaleFromRaw)
            {
              tensorScale = ((float* )(tensorScales.raw_data().c_str()))[i];
            }
            else
            {
              tensorScale = (float )(tensorScales.float_data(i));
            }
            for(j = 0; j < perChannelSize; j++)
            {
              int8_t quantizedTensorVal = ((int8_t* )(tensor.raw_data().c_str()))[(i * perChannelSize) + j];
              output[(i * perChannelSize) + j] = ((((float)quantizedTensorVal) - tensorZeroPoint) * tensorScale);
            }
          }
        }
      }
      else if(tensor.data_type() == TensorProto_DataType_UINT8)
      {
        if(TensorProto_DataType_INT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            int8_t tensorZeroPoint;
            float  tensorScale;

            if(tensorZPFromRaw)
            {
              tensorZeroPoint = ((int8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
            }
            else
            {
              tensorZeroPoint = (int8_t)(tensorZeroPoints.int32_data(i));
            }

            for(j = 0; j < perChannelSize; j++)
            {
              uint8_t quantizedTensorVal = ((uint8_t* )(tensor.raw_data().c_str()))[(i * perChannelSize) + j];
              output[(i * perChannelSize) + j] = ((((float)quantizedTensorVal) - tensorZeroPoint) * tensorScale);
            }
          }
        }
        else if(TensorProto_DataType_UINT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            uint8_t tensorZeroPoint;
            float  tensorScale;

            if(tensorZPFromRaw)
            {
              tensorZeroPoint = ((uint8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
            }
            else
            {
              tensorZeroPoint = (uint8_t)(tensorZeroPoints.int32_data(i));
            }

            if(tensorScaleFromRaw)
            {
              tensorScale = ((float* )(tensorScales.raw_data().c_str()))[i];
            }
            else
            {
              tensorScale = (float )(tensorScales.float_data(i));
            }
            for(j = 0; j < perChannelSize; j++)
            {
              uint8_t quantizedTensorVal = ((uint8_t* )(tensor.raw_data().c_str()))[(i * perChannelSize) + j];
              output[(i * perChannelSize) + j] = ((((float)quantizedTensorVal) - tensorZeroPoint) * tensorScale);
            }
          }
        }
      }
  }
  else
  {
    TIDL_GLOBAL_REPORT_UNSUPPORTED("Unsupported Onnx import data type : %d ", tensor.data_type());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}


TensorProto TIDL_getConstInTensorFromDQLinear(GraphProto& onnGraph, const string name, int32_t inputIdx, int32_t& status)
{
  /*Find the corresponding DQ layer via it's output name*/
  int32_t DQNodeIdx = TIDL_onnxGetNodeIdx(onnGraph, name.c_str());
  /*Return DQ's input (inIdx)*/
  return (TIDL_getInitializerTensor(onnGraph, onnGraph.node(DQNodeIdx).input(inputIdx) , DQNodeIdx, status));
}



/*
  nIdx = Node IDX (e.g. Conv)
  inIdx = Which input to the conv layer
*/
int32_t TIDL_tfDequantizeAndCopyFloatTensor(GraphProto& onnGraph, int32_t nIdx, int32_t inIdx, sBuffer_t &buf)
{
  /*Identify the corresponding DQ node and retrieve it's input tensors to perform dequantization*/
  int32_t status = 0;
  TensorProto quantizedTensor = TIDL_getConstInTensorFromDQLinear(onnGraph, onnGraph.node(nIdx).input(inIdx), 0, status);
  if(status != 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index %d for node %s", 0, onnGraph.node(nIdx).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  TensorProto tensorScales  = TIDL_getConstInTensorFromDQLinear(onnGraph, onnGraph.node(nIdx).input(inIdx), 1, status);
  if(status != 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index %d for node %s", 1, onnGraph.node(nIdx).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  TensorProto tensorZeroPoints = TIDL_getConstInTensorFromDQLinear(onnGraph, onnGraph.node(nIdx).input(inIdx), 2, status);
  if(status != 0)
  {
    TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index %d for node %s", 2, onnGraph.node(nIdx).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
   /*Determine dimensions:*/
  bool perAxis = false;
  /*Scales and ZeroPoints are 1D*/
  assert(tensorScales.dims(0) == tensorZeroPoints.dims(0));
  int32_t numOutChannels = tensorScales.dims(0);
  if(numOutChannels == 1)
  {
    perAxis = false;
  }
  else
  {
    perAxis = true;
  }

  int32_t tensorSize = 1;
  int32_t i,j;
  for(i = 0 ; i < quantizedTensor.dims_size(); i++)
  {
    tensorSize *= quantizedTensor.dims(i);
  }
  int32_t perChannelSize = tensorSize / numOutChannels;
  /*Allocate float buffer:*/
  buf.ptr = (float*) malloc(sizeof(float) * tensorSize);
  if(buf.ptr == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Unable to allocate buffer for convolution parameters");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  buf.bufSize = tensorSize;
  float *output = (float*)buf.ptr;
  /*Dequantize and copy over:*/
  if(TensorProto_DataType_INT8 == quantizedTensor.data_type())
  {
    if(TensorProto_DataType_INT8 == tensorZeroPoints.data_type())
    {
      for(i = 0; i < numOutChannels; i++)
      {
        int8_t tensorZeroPoint = ((int8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
        float  tensorScale     = ((float* )(tensorScales.raw_data().c_str()))[i];
        for(j = 0; j < perChannelSize; j++)
        {
          int8_t quantizedTensorVal = ((int8_t* )(quantizedTensor.raw_data().c_str()))[(i * perChannelSize) + j];
          output[(i * perChannelSize) + j] = (((float)quantizedTensorVal)/tensorScale) + tensorZeroPoint;
        }
      }
    }
    else if(TensorProto_DataType_UINT8 == tensorZeroPoints.data_type())
    {
      for(i = 0; i < numOutChannels; i++)
      {
        uint8_t tensorZeroPoint = ((int8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
        float  tensorScale     = ((float* )(tensorScales.raw_data().c_str()))[i];
        for(j = 0; j < perChannelSize; j++)
        {
          int8_t quantizedTensorVal = ((int8_t* )(quantizedTensor.raw_data().c_str()))[(i * perChannelSize) + j];
          output[(i * perChannelSize) + j] = (((float)quantizedTensorVal)/tensorScale) + tensorZeroPoint;
        }
      }
    }
  }
  else if(TensorProto_DataType_UINT8 == quantizedTensor.data_type())
  {
    if(TensorProto_DataType_INT8 == tensorZeroPoints.data_type())
    {
      for(i = 0; i < numOutChannels; i++)
      {
        int8_t tensorZeroPoint = ((int8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
        float  tensorScale     = ((float* )(tensorScales.raw_data().c_str()))[i];
        for(j = 0; j < perChannelSize; j++)
        {
          uint8_t quantizedTensorVal = ((uint8_t* )(quantizedTensor.raw_data().c_str()))[(i * perChannelSize) + j];
          output[(i * perChannelSize) + j] = (((float)quantizedTensorVal)/tensorScale) + tensorZeroPoint;
        }
      }
    }
    else if(TensorProto_DataType_UINT8 == tensorZeroPoints.data_type())
    {
      for(i = 0; i < numOutChannels; i++)
      {
        uint8_t tensorZeroPoint = ((int8_t* )(tensorZeroPoints.raw_data().c_str()))[i];
        float  tensorScale     = ((float* )(tensorScales.raw_data().c_str()))[i];
        for(j = 0; j < perChannelSize; j++)
        {
          uint8_t quantizedTensorVal = ((uint8_t* )(quantizedTensor.raw_data().c_str()))[(i * perChannelSize) + j];
          output[(i * perChannelSize) + j] = (((float)quantizedTensorVal)/tensorScale) + tensorZeroPoint;
        }
      }
    }
  }
  else if(TensorProto_DataType_INT32 == quantizedTensor.data_type())
  {
    if(TensorProto_DataType_INT32 == tensorZeroPoints.data_type())
    {
      for(i = 0; i < numOutChannels; i++)
      {
        int32_t tensorZeroPoint = ((int32_t* )(tensorZeroPoints.raw_data().c_str()))[i];
        float  tensorScale     = ((float* )(tensorScales.raw_data().c_str()))[i];
        for(j = 0; j < perChannelSize; j++)
        {
          int32_t quantizedTensorVal = ((int32_t* )(quantizedTensor.raw_data().c_str()))[(i * perChannelSize) + j];
          output[(i * perChannelSize) + j] = (((float)quantizedTensorVal)/tensorScale) + tensorZeroPoint;
        }
      }
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("Unable to dequantize bias values");
    }
  }
  else if(TensorProto_DataType_UINT32 == quantizedTensor.data_type())
  {
    if(TensorProto_DataType_UINT32 == tensorZeroPoints.data_type())
    {
      for(i = 0; i < numOutChannels; i++)
      {
        uint32_t tensorZeroPoint = ((uint32_t* )(tensorZeroPoints.raw_data().c_str()))[i];
        float  tensorScale     = ((float* )(tensorScales.raw_data().c_str()))[i];
        for(j = 0; j < perChannelSize; j++)
        {
          uint32_t quantizedTensorVal = ((uint32_t* )(quantizedTensor.raw_data().c_str()))[(i * perChannelSize) + j];
          output[(i * perChannelSize) + j] = (((float)quantizedTensorVal)/tensorScale) + tensorZeroPoint;
        }
      }
    }
    else
    {
      TIDL_GLOBAL_REPORT_ERROR("Unable to dequantize bias values");
    }
  }
  return 0;
}

int32_t TIDL_getConstSize (TensorProto& tensor)
{
  int32_t size = 0, i1;

  if (tensor.dims_size() == 0)
  {
    /* Constant*/
    size = 1;
  }
  else
  {
    size = 1;
    for (i1 = 0; i1 < tensor.dims_size(); i1++)
    {
      size *= tensor.dims(i1);
    }
  }

  return size;
}

int32_t TIDL_tfCopyFloatInitializerTensor(GraphProto& onnGraph, int32_t nIdx, int32_t inIdx, sBuffer_t &buf)
{
  if (onnGraph.node(nIdx).input_size() > inIdx)
  {
    int32_t status = 0;
    TensorProto tensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(nIdx).input(inIdx), nIdx, status);
    if(status != 0)
    {
      TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index %d for node %s", inIdx, onnGraph.node(nIdx).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    return TIDL_tfCopyTensor(onnGraph, tensor, &buf.ptr, &buf.bufSize, onnGraph.node(nIdx).input(inIdx));
  }
  else
  {
    /*Particular initializer/input does not exist (This is valid situation when the input is optional)*/
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
}

int32_t TIDL_tfCopyFloatConst(GraphProto& onnGraph, int32_t nIdx, int32_t inIdx, sBuffer_t &buf)
{
  if (onnGraph.node(nIdx).input_size() > inIdx)
  {
    int32_t status = 0;
    TensorProto tensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(nIdx).input(inIdx), nIdx, status);
    if(status != 0)
    {
      TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index - %d for node %s", inIdx, onnGraph.node(nIdx).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    return TIDL_tfCopyTensor(onnGraph, tensor, &buf.ptr, &buf.bufSize, onnGraph.node(nIdx).input(inIdx));
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Requested constant tensor %d in %s is not found", inIdx, onnGraph.node(nIdx).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return 0;
}

/** Broadcast floating point vector buffer to have a modified dimension */
void TIDL_onnxBroadcastFloatTensor(sBuffer_t &buf, int32_t bufSize)
{
  float * ptr = (float *)buf.ptr;
  float val = ptr[0]; //only one const val for computation
  void * newPtr = realloc(ptr, bufSize * sizeof(float)); //ptr is void* so it should be reallocated to a void * ptr
  float *newPtrCast = (float *)newPtr;
  for(int i = 0; i < bufSize; i++)
  {
    newPtrCast[i] = val;  //broadcast the constant along number of channels
  }
  buf.ptr = newPtr;
  buf.bufSize = bufSize;
}

/** Function to save layer level tensor properties metadata to be used for allowlisting
 * Saves data for sTIDL_allowlistingMetaData structure
 */
int32_t TIDL_onnxSaveAllowlistingMetaData(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  /*
   Form onnx 1.15, if DQ layer might be duplicated. The node name of duplicated
   node is appended with "/duplicate". Get metadata from original dq layer since
   shape inference wouldn't have run on the duplicated.
  */
  if (strcmp(onnGraph.node(i).op_type().c_str(), "DequantizeLinear") == 0)
  {
    std::string name = onnGraph.node(i).name();
    if(name.find("/duplicated") != std::string::npos)
    {
      name = name.substr(0, name.find("/duplicated"));
      for (int j = 0; j < onnGraph.node_size(); j++)
      {
        if(strcmp(onnGraph.node(j).name().c_str(), name.c_str()) == 0)
        {
          i = j;
          break;
        }
      }
    }
  }

  layer.allowlistingMetaData.numInputs = onnGraph.node(i).input_size();
  layer.allowlistingMetaData.numOutputs = onnGraph.node(i).output_size();
  layer.allowlistingMetaData.numValidInputs = 0;
  layer.allowlistingMetaData.validInputIndices = {};
  for (int j = 0; j < onnGraph.node(i).input_size(); j++)
  {
    if (strcmp(onnGraph.node(i).input(j).c_str(),"") != 0)
    {
      layer.allowlistingMetaData.numValidInputs++;
      layer.allowlistingMetaData.validInputIndices.push_back(j);
    }
  }
  layer.allowlistingMetaData.numVarInputs = TIDL_onnxNumVariableLayerInputs(onnGraph, i,  layer.allowlistingMetaData.varTensorIndices);
  layer.allowlistingMetaData.numConstInputs = layer.allowlistingMetaData.numValidInputs - layer.allowlistingMetaData.numVarInputs;

  //Find indices of constant tensors
  for(auto& idx : layer.allowlistingMetaData.validInputIndices)
  {
    if(std::find(layer.allowlistingMetaData.varTensorIndices.begin(), layer.allowlistingMetaData.varTensorIndices.end(), idx) == layer.allowlistingMetaData.varTensorIndices.end())
    //idx not found in variable tensors indices => constant tensor idx
    {
      layer.allowlistingMetaData.constTensorIndices.push_back(idx);
    }
  }
  //Populate dimensions of variable tensors
  for(auto& varIdx : layer.allowlistingMetaData.varTensorIndices)
  {
    std::string inputName = onnGraph.node(i).input(varIdx);

    // If input at varIdx is duplicated DQ then use the original DQ to get dims
    if(inputName.find("/duplicated") != std::string::npos)
    {
      int32_t found = 0;
      for (int j = 0; j < onnGraph.node_size(); j++)
      {
        if (strcmp(onnGraph.node(j).op_type().c_str(), "DequantizeLinear") == 0)
        {
          for (int k = 0; k < onnGraph.node(j).output_size(); k++)
          {
            if (strcmp(onnGraph.node(j).output(k).c_str(), inputName.c_str()) == 0)
            {
              inputName = inputName.substr(0, inputName.find("/duplicated"));
              found = 1;
              break;
            }
          }
        }
        if(found)
        {
          break;
        }
      }
    }

    layer.allowlistingMetaData.varTensorsDims.push_back(getNodeInputShape(onnGraph, inputName, 0));
  }
  //Populate dimensions of constant tensors
  for(auto& constIdx : layer.allowlistingMetaData.constTensorIndices)
  {
    int32_t status = 0;
    TensorProto tensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(constIdx), i, status);
    if(status != 0)
    {
      TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index - %d for node %s", constIdx, onnGraph.node(i).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    layer.allowlistingMetaData.constTensorNames.push_back(onnGraph.node(i).input(constIdx).c_str());
    std::vector<int32_t> dims;
    for(int j = 0; j < tensor.dims_size(); j++)
    {
      dims.push_back(tensor.dims(j));
    }
    layer.allowlistingMetaData.constTensorsDims.push_back(dims);
  }
  //Populate dimensions of output tensors
  for(int outputIdx=0; outputIdx<layer.allowlistingMetaData.numOutputs; outputIdx++)
  {
    layer.allowlistingMetaData.outputTensorDims.push_back(getNodeOutputShape(onnGraph,  onnGraph.node(i).output(outputIdx), 0));
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

/* Check if Reshape-Transpose-Reshape corresponds to channel-wise shuffle operation */
bool TIDL_checkShuffleOperation(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices)
{
  bool isSupported = false;
  sBuffer_t reshape1OutShape, reshape2OutShape, transposeOutShape;

  int32_t reshape1Idx = supportedNodesIndices[0];
  int32_t transposeIdx = supportedNodesIndices[1];
  int32_t reshape2Idx = supportedNodesIndices[2];

  int32_t status = TIDL_tfCopyFloatInitializerTensor(onnxGraph, reshape1Idx, 1, reshape1OutShape);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return false;
  }
  status = TIDL_tfCopyFloatInitializerTensor(onnxGraph, reshape2Idx, 1, reshape2OutShape);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    return false;
  }
  NodeProto node = onnxGraph.node(transposeIdx);
  int32_t attrIdx = TIDL_onnxGetAttrIdx(node, "perm");
  if (attrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    int32_t j;
    int32_t dims = node.attribute(attrIdx).ints_size();
    transposeOutShape.ptr = (void*)malloc(sizeof(int)*dims);
    transposeOutShape.bufSize = dims;
    int32_t *dimVlaue = (int*)transposeOutShape.ptr;
    for (j = 0; j < transposeOutShape.bufSize; j++)
    {
      dimVlaue[j] = node.attribute(attrIdx).ints(j);
    }
  }
  if ((reshape1OutShape.bufSize ==  5) && (transposeOutShape.bufSize ==  5) && (reshape2OutShape.bufSize == 4))
  {
    int64_t * reshape1  = (int64_t *)reshape1OutShape.ptr;
    int64_t * reshape2  = (int64_t *)reshape2OutShape.ptr;
    int32_t * transpose = (int32_t *)transposeOutShape.ptr;
    if ((reshape1[4] == reshape2[3]) && (reshape1[3] == reshape2[2]) && (4 == transpose[4]) && (3 == transpose[3]) && (1 == transpose[2]) && (2 == transpose[1]))
    {
      isSupported = true;
    }
  }
  /* Depth-to-Space layer after fusion */
  if ((reshape1OutShape.bufSize ==  6) && (transposeOutShape.bufSize ==  6) && (reshape2OutShape.bufSize == 4))
  {
    int64_t * reshape1  = (int64_t *)reshape1OutShape.ptr;
    int64_t * reshape2  = (int64_t *)reshape2OutShape.ptr;
    int32_t * transpose = (int32_t *)transposeOutShape.ptr;
    if ((reshape1[4]*reshape1[2] == reshape2[2]) && (reshape1[3]*reshape1[5] == reshape2[3]) && (reshape1[1] == reshape2[1]) && (reshape1[2] == reshape1[3]) &&
        (3 == transpose[5]) && (5 == transpose[4]) && (2 == transpose[3]) && (4 == transpose[2]) && (1 == transpose[1]))
    {
      isSupported = true;
    }
     if((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A) /* Pixel shuffle layer is not supported on AM62A and AM67A */
    {
      isSupported = false;
    }
    /* block size check */
    if(reshape1[2] != 8 && reshape1[2] != 4 && reshape1[2] != 2)
    {
      isSupported = false;
    }
  }

  return isSupported;
}



/* Check if Div-Erf-Add-Mul corresponds to GeLU operation

	checks:
  1. if div has exactly have one const and one var input const must be sqrt(2)
  2. if add has exactly have 1 const and 1 var input, also const input must be 1
  3. if one of the input to mul is same as var input to div

*/
bool TIDL_checkGELUOperation(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices)
{
	bool isSupported = true;
  int32_t status = 0;
  sBuffer_t divConst, addConst, mul2const;
  sTIDL_LayerPC_t divLayer, addLayer, mulLayer, mul2Layer;
  sTIDL_allowlistingMetaData md;
  int32_t divIdx = supportedNodesIndices[0];
  int32_t erfIdx = supportedNodesIndices[1];
  int32_t addIdx = supportedNodesIndices[2];
  int32_t mulIdx = supportedNodesIndices[3];
  int32_t mul2Idx = supportedNodesIndices[4];

  /*
  div must have one const and one var input
  const must be sqrt(2)
  */
  status = TIDL_onnxSaveAllowlistingMetaData(onnxGraph, divIdx, divLayer);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  md = divLayer.allowlistingMetaData;

  if((md.numConstInputs == 1) && (md.numVarInputs == 1))
  {
    /* check for value of const */
    status = TIDL_tfCopyFloatInitializerTensor(onnxGraph, divIdx, md.constTensorIndices[0], divConst);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      return false;
    }
    if (divConst.bufSize == 1)
    {
      float32_tidl divConstVal = *((float32_tidl*)divConst.ptr);
      if ((divConstVal == sqrtf(2.0)))
      {
        /* okay */
      }
      else
      {
        isSupported = false;
      }
    }
    /* not a single float pt buffer */
    else
    {
      isSupported = false;
    }
  }
  /* not matching 1 const and 1 var input */
  else
  {
    isSupported = false;
  }

  /*
  Add must have 1 const and 1 var input
  also const input must be 1
  */
  status = TIDL_onnxSaveAllowlistingMetaData(onnxGraph, addIdx, addLayer);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  md = addLayer.allowlistingMetaData;

  if((md.numConstInputs == 1) && (md.numVarInputs == 1))
  {
    /* check for value of const */
    status = TIDL_tfCopyFloatInitializerTensor(onnxGraph, addIdx, md.constTensorIndices[0], addConst);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      return false;
    }
    if (addConst.bufSize == 1)
    {
      float32_tidl addConstVal = *((float32_tidl*)addConst.ptr);
      if (addConstVal == 1.0)
      {
        /* okay */
      }
      else
      {
        isSupported = false;
      }
    }
    /* not a single float pt buffer */
    else
    {
      isSupported = false;
    }
  }
  /* not matching 1 const and 1 var input */
  else
  {
    isSupported = false;
  }


  /* One of the input to mul must be same as var input to Div */
  status = TIDL_onnxSaveAllowlistingMetaData(onnxGraph, mulIdx, mulLayer);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  md = mulLayer.allowlistingMetaData;
  if ((md.numInputs == 2) && (md.numVarInputs == 2))  /* both inputs should be var */
  {
    auto divVarInput = onnxGraph.node(divIdx).input(divLayer.allowlistingMetaData.varTensorIndices[0]);
    if (
          (strcmp(onnxGraph.node(mulIdx).input(0).c_str(), divVarInput.c_str()) == 0) ||
          (strcmp(onnxGraph.node(mulIdx).input(1).c_str(), divVarInput.c_str()) == 0)
      )
      {
        /* okay */
      }
      else
      {
        isSupported = false;
      }
  }
  /* both inputs are not var */
  else
  {
    isSupported = false;
  }


  /*
    Mul with 0.5
  */
  status = TIDL_onnxSaveAllowlistingMetaData(onnxGraph, mul2Idx, mul2Layer);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  md = mul2Layer.allowlistingMetaData;

  if((md.numConstInputs == 1) && (md.numVarInputs == 1))
  {
    /* check for value of const */
    status = TIDL_tfCopyFloatInitializerTensor(onnxGraph, mul2Idx, md.constTensorIndices[0], mul2const);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      return false;
    }
    if (mul2const.bufSize == 1)
    {
      float32_tidl mul2ConstVal = *((float32_tidl*)mul2const.ptr);
      if (mul2ConstVal == 0.5)
      {
        /* okay */
      }
      else
      {
        isSupported = false;
      }
    }
    /* not a single float pt buffer */
    else
    {
      isSupported = false;
    }
  }
  /* not matching 1 const and 1 var input */
  else
  {
    isSupported = false;
  }

	return isSupported;

}

/* Check if ReduceMean-Sub-Pow-ReduceMean-Add-Sqrt-Div corresponds to LN operation

	checks:
  1. ReduceMean axis are width for both
  2. Sub 1st input is same as 1st ReduceMean input
  3. Pow has power of 2
  4. Div first input is same as Pow input
*/
bool TIDL_checkLNOperation(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices)
{
  bool isSupported = true;
  int32_t status = 0;
  int32_t reduceMean1Idx = supportedNodesIndices[0];
  int32_t subIdx         = supportedNodesIndices[1];
  int32_t divIdx;

  /* check next node is div or pow in supportedNodesIndices */
  int32_t idx = supportedNodesIndices[2];
  int32_t offset = 0;
  if(onnxGraph.node(idx).op_type().compare("Div") == 0)
  {
    divIdx = idx;
    offset = 1;
  }
  else
  {
    divIdx = supportedNodesIndices[supportedNodesIndices.size()-1];
  }
  int32_t powIdx         = supportedNodesIndices[offset+2];
  int32_t reduceMean2Idx = supportedNodesIndices[offset+3];
  int32_t addIdx         = supportedNodesIndices[offset+4];
  int32_t sqrtIdx        = supportedNodesIndices[offset+5];

  /* ReduceMean axis = width */
  NodeProto node;
  sTIDL_allowlistingMetaData md;
  sBuffer_t powExponent;
  int32_t axis, axisIdx, inputDimSize;
  sTIDL_LayerPC_t reduceMeanLayer1, reduceMeanLayer2, powLayer;
  /* Check 1st ReduceMean */
  node = onnxGraph.node(reduceMean1Idx);
  status = TIDL_onnxSaveAllowlistingMetaData(onnxGraph, reduceMean1Idx, reduceMeanLayer1);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  md = reduceMeanLayer1.allowlistingMetaData;
  axisIdx = TIDL_onnxGetAttrIdx(node, "axes");
  TIDL_onnxGetIntAttr(node, "axes", &axis, 0);
  if (
    (node.attribute(axisIdx).ints_size() == 1) && // must be single axis only
    (
      (axis == -1) ||   // negative index
      (axis == (md.varTensorsDims[0].size() - 1)) // positive index
    )
  )
  {
    /* okay */
  }
  else
  {
    isSupported = false;
  }
  /* Check 2nd ReduceMean */
  node = onnxGraph.node(reduceMean2Idx);
  status = TIDL_onnxSaveAllowlistingMetaData(onnxGraph, reduceMean2Idx, reduceMeanLayer2);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  md = reduceMeanLayer2.allowlistingMetaData;
  axisIdx = TIDL_onnxGetAttrIdx(node, "axes");
  TIDL_onnxGetIntAttr(node, "axes", &axis, 0);
  if (
    (node.attribute(axisIdx).ints_size() == 1) && // must be single axis only
    (
      (axis == -1) ||   // negative index
      (axis == (md.varTensorsDims[0].size() - 1)) // positive index
    )
  )
  {
    /* okay */
  }
  else
  {
    isSupported = false;
  }

  /* Check if sub first input is same as 1st reducemean input */
  std::string subInp = onnxGraph.node(subIdx).input(0);
  std::string reduceMean1Inp = onnxGraph.node(reduceMean1Idx).input(0);

  // the gParams.preQuantized is not populated hence manual check is needed
  std::vector<int> subInpIdxs = TIDL_getInputNodes(subIdx, onnxGraph); 
  if((subInpIdxs.size() > 0 && subInpIdxs[0] != -1) &&
     (strcmp(onnxGraph.node(subInpIdxs[0]).op_type().c_str(), "DequantizeLinear") == 0 && subInp.find("/duplicated") != std::string::npos))
  {
    subInp = subInp.substr(0, subInp.find("/duplicated"));
  }
  std::vector<int> reduceMean1InpIdxs = TIDL_getInputNodes(reduceMean1Idx, onnxGraph); 
  if((reduceMean1InpIdxs.size() > 0 && reduceMean1InpIdxs[0] != -1) &&
     (strcmp(onnxGraph.node(reduceMean1InpIdxs[0]).op_type().c_str(), "DequantizeLinear") == 0 && reduceMean1Inp.find("/duplicated") != std::string::npos))
  {
    reduceMean1Inp = reduceMean1Inp.substr(0, reduceMean1Inp.find("/duplicated"));
  }

  if (strcmp(subInp.c_str(), reduceMean1Inp.c_str()) != 0)
  {
    isSupported = false;
  }

  /* Check power of Pow*/
  node = onnxGraph.node(powIdx);
  status = TIDL_onnxSaveAllowlistingMetaData(onnxGraph, powIdx, powLayer);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  md = powLayer.allowlistingMetaData;
  status = TIDL_tfCopyFloatInitializerTensor(onnxGraph, powIdx, md.constTensorIndices[0], powExponent);
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    return false;
  }
  float32_tidl exponentVal = *((float32_tidl*)powExponent.ptr);
  if (exponentVal != 2)
  {
    isSupported = false;
  }

  /* Check Div first input is same as Pow input */
  if (strcmp(onnxGraph.node(divIdx).input(0).c_str(),
             onnxGraph.node(powIdx).input(0).c_str()) != 0
  )
  {
    isSupported = false;
  }

  return isSupported;

}

/* Check if MUL->MATMUL (Reducesum in width axis) can be fused. This patter is
  usually found in deformable attention.
  Rule:
  1. The inputs to Mul should be channel broadcastable i.e N1HW <-> NCHW
  2. Matmul should strictly be a reducesum in width axis
*/
bool TIDL_checkMSDAMulMatmul(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices)
{
  sTIDL_allowlistingMetaData mulMd;
  int32_t mulIdx;
  int32_t matMulIdx;
  sTIDL_LayerPC_t mulLayer;
  sTIDL_LayerPC_t matMulLayer;

  if(onnxGraph.node(supportedNodesIndices[0]).op_type().compare("Mul") == 0)
  {
    mulIdx = supportedNodesIndices[0];
    matMulIdx = supportedNodesIndices[1];
  }
  else
  {
    mulIdx = supportedNodesIndices[1];
    matMulIdx = supportedNodesIndices[0];
  }

  TIDL_onnxSaveAllowlistingMetaData(onnxGraph, mulIdx, mulLayer);
  TIDL_onnxSaveAllowlistingMetaData(onnxGraph, matMulIdx, matMulLayer);
  bool isSupported = false;

  if (mulLayer.allowlistingMetaData.numInputs == 2)
  {

    std::vector<std::vector<int32_t>> inputDims;

    for (auto &dim : mulLayer.allowlistingMetaData.varTensorsDims)
    {
      inputDims.push_back(dim);
    }
    for (auto &dim : mulLayer.allowlistingMetaData.constTensorsDims)
    {
      inputDims.push_back(dim);
    }

    if (inputDims.size() == 2)
    {
      std::vector<int32_t> dim1 = inputDims[0];
      std::vector<int32_t> dim2 = inputDims[1];

      if(dim1.size() < 3 || dim2.size() < 3)
      {
        isSupported = true;
      }
      else
      {
        if((dim1[dim1.size() - 3] == 1 || dim2[dim2.size() - 3] == 1) && (dim1.size() == dim2.size()))
        {
          isSupported = true;
          for(int i = 0; i < dim1.size(); i++)
          {
            if(i == (dim1.size() - 3)) continue;
            if (dim1[i] != dim2[i])
            {
              isSupported = false;
              break;
            }
          }
        }
      }
    }
  }

  /* TODO: Add feature to check if reducesum in width axis i.e the const tensor
   * is all 1s
   */
  int32_t numOutTensorDims = matMulLayer.allowlistingMetaData.outputTensorDims[0].size();
  if(matMulLayer.allowlistingMetaData.numInputs != 2 ||
     matMulLayer.allowlistingMetaData.numConstInputs != 1 ||
     matMulLayer.allowlistingMetaData.outputTensorDims[0][numOutTensorDims-1] != 1)
  {
    isSupported = false;
  }

  return isSupported;
}

/* Check if Reshape - Slice x8 -Concat corresponds to Patch Merging operation
  1. The first 4 Slices connected to Reshape should be along channel axis and starts should be [0,1,0,1]
  2. The next 4 Slice connected to Concat should be along height axis and starts should be [0,0,1,1]
  2. Slice steps = 2
*/
bool TIDL_checkPatchMerging(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices)
{
  bool isSupported = true;
  sTIDL_allowlistingMetaData md;
  sTIDL_LayerPC_t sliceLayer;
  sBuffer_t starts, axes, steps;

  int32_t reshapeIdx = supportedNodesIndices[0];
  int32_t concatIdx;

  std::vector<int32_t> sliceIndices;
  std::vector<int32_t> firstLevelSliceIndices;
  std::vector<int32_t> secondLevelSliceIndices;
  for(int32_t i = 1;i < supportedNodesIndices.size(); i++)
  {
    int32_t nodeIdx = supportedNodesIndices[i];
    if(onnxGraph.node(nodeIdx).op_type().compare("Concat") == 0)
    {
      concatIdx = nodeIdx;
    }
    else
    {

      std::vector<int> output = TIDL_getOutputNodes(supportedNodesIndices[i], onnxGraph);
      if(onnxGraph.node(output[0]).op_type().compare("Concat") == 0)
      {
        secondLevelSliceIndices.push_back(supportedNodesIndices[i]);
      }
      else
      {
        firstLevelSliceIndices.push_back(supportedNodesIndices[i]);
      }
    }
  }

  sliceIndices = firstLevelSliceIndices;
  sliceIndices.insert(sliceIndices.end(), secondLevelSliceIndices.begin(), secondLevelSliceIndices.end());

  /* check for starts, axes and stride */
  uint8_t startAcc = 0;
  for (int32_t i = 0; i < sliceIndices.size(); i++)
  {
    int32_t sliceIdx = sliceIndices[i];
    auto node = onnxGraph.node(sliceIdx);
    TIDL_onnxSaveAllowlistingMetaData(onnxGraph, sliceIdx, sliceLayer);
    md = sliceLayer.allowlistingMetaData;
    int32_t numInDims = md.varTensorsDims[0].size();
    if (node.input_size() == 5)
    {
      TIDL_tfCopyFloatInitializerTensor(onnxGraph, sliceIdx, 1, starts);
      TIDL_tfCopyFloatInitializerTensor(onnxGraph, sliceIdx, 3, axes);
      TIDL_tfCopyFloatInitializerTensor(onnxGraph, sliceIdx, 4, steps);
      /* single starts, single axes, single step*/
      if ((starts.bufSize == 1) && (axes.bufSize == 1) && (steps.bufSize == 1))
      {
        int64_t startsVal = *((int64_t*)starts.ptr);
        int64_t axesVal = *((int64_t*)axes.ptr);
        int64_t stepVal = *((int64_t*)steps.ptr);

        /* accumulate starts value */
        if (startsVal == 0 || startsVal == 1)
        {
          /* accumulate to check later */
          startAcc <<= 1;
          startAcc |= startsVal;
        }
        else
        {
          isSupported = false;
          break;
        }

        /* if slice 0, 1, 2, 3 (firstLevelSlice) - axes = channel*/
        if (i < 4)
        {
          bool ischannelAxis = ((numInDims - axesVal) == 3);
          if(!ischannelAxis)
          {
            isSupported = false;
            break;
          }
        }
        /* if slice 4, 5, 6, 7 (secondLevelSlice) - axes = height */
        else
        {
          bool isHeightAxis = ((numInDims - axesVal) == 2);
          if (!isHeightAxis)
          {
            isSupported = false;
            break;
          }
        }

        /* step = 2 */
        if (stepVal == 2)
        {
          /* okay */
        }
        else
        {
          isSupported = false;
          break;
        }

      }
      else
      {
        isSupported = false;
        break;
      }
    }
    else
    {
      isSupported = false;
      break;
    }
  }

  /* check if starts is b01010011 = 83 */
  if (startAcc == 83)
  {
    /* okay */
  }
  else
  {
    isSupported = false;
  }

  return isSupported;
}


int32_t TIDL_onnxMapConvBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status;
  int32_t padType;
  int32_t idx1, idx2;
  int32_t pads[4] = {0};
  int32_t kernel_shape[2];
  int32_t padL=-1, padR=-1, padT=-1, padB=-1;

  sTIDL_ConvParams_t &convParams      = layer.layerParams.convParams;
  layer.layerType = TIDL_ConvolutionLayer;

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /*Check that the dimensionality of inputs is less than 4 (Supported configuration)*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    /*Shape inference information is present:*/
    for(int32_t j = 0; j < layer.allowlistingMetaData.varTensorsDims.size(); j++)
    {
      int32_t numDim = layer.allowlistingMetaData.varTensorsDims[j].size();
      if(numDim > 4)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Unsupported number of dimensions for convolution (>4)");
        return -1;
      }
    }
  }


  layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  layer.outData[0].elementType = tidl_getElementType(1);
  TensorProto tensor;

  tensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(1), i, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Conv layer : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  convParams.numOutChannels = TIDL_geTensorDim(tensor, 0);
  convParams.numInChannels  = TIDL_geTensorDim(tensor, 1);
  convParams.kernelH        = TIDL_geTensorDim(tensor, 2);
  convParams.kernelW        = TIDL_geTensorDim(tensor, 3);

  convParams.numGroups = 1;
  convParams.dilationW = 1;
  convParams.dilationH = 1;
  convParams.strideW = 1;
  convParams.strideH = 1;
  convParams.padW = 0;
  convParams.padH = 0;
  convParams.enableBias = 0;
  convParams.enablePooling = 0;

  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "strides",   &convParams.strideH, 0);
  TIDL_onnxGetIntAttr(node, "strides",   &convParams.strideW, 1);
  TIDL_onnxGetIntAttr(node, "dilations", &convParams.dilationH, 0);
  TIDL_onnxGetIntAttr(node, "dilations", &convParams.dilationW, 1);
  TIDL_onnxGetIntAttr(node, "group",     &convParams.numGroups, 0);
  TIDL_onnxGetIntAttr(node, "kernel_shape", &kernel_shape[0], 0);
  TIDL_onnxGetIntAttr(node, "kernel_shape", &kernel_shape[1], 1);

  char autoPad[50];
  status = TIDL_onnxGetStringAttr(node, "auto_pad", autoPad, 0);
  if((status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) || (strcmp(autoPad, "NOTSET") == 0))
  {
    TIDL_onnxGetIntAttr(node, "pads",       &pads[0], 0);
    TIDL_onnxGetIntAttr(node, "pads",       &pads[1], 1);
    TIDL_onnxGetIntAttr(node, "pads",       &pads[2], 2);
    TIDL_onnxGetIntAttr(node, "pads",       &pads[3], 3);
    if((pads[0] < pads[2]) && (pads[1] < pads[3])) //padL < padR && padT < padB
    {
      layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
    }
    pads[2] = max(pads[0], pads[2]);
    pads[3] = max(pads[1], pads[3]);
    padL = pads[1];
    padR = pads[3];
    padT = pads[0];
    padB = pads[2];
  }
  else if ((strcmp(autoPad, "SAME_UPPER") == 0) || (strcmp(autoPad, "SAME_LOWER") == 0))
  {
    /* TCs pertaining to this condition are not existent as of now, hence this cannot be validated */
    pads[3] = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    pads[2] = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
    if(strcmp(autoPad, "SAME_UPPER") == 0)
    {
      layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
    }
    else  //SAME_LOWER
    {
      layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
    }
  }
  else if(strcmp(autoPad, "VALID") == 0)
  {
    pads[3] = 0;
    pads[2] = 0;
    padL = 0;
    padR = 0;
    padT = 0;
    padB = 0;
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Conv layer : Unsupported auto_pad attribute -- Node name - %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }



  convParams.padW = pads[3];
  convParams.padH = pads[2];

  convParams.padT = padT;
  convParams.padL = padL;
  convParams.padB = padB;
  convParams.padR = padR;

  if ((kernel_shape[0] != convParams.kernelH) || (kernel_shape[1] != convParams.kernelW))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Weight tensor size is not matching with proto kernel_shape -- Node name - %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, layer.weights), "");
  if (onnGraph.node(i).input_size() > 2)
  {
    convParams.enableBias = 1;
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 2, layer.bias), "");
  }


  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}


int32_t TIDL_onnxMapConvParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapConvBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapConvTransposeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status;
  int32_t padType;
  int32_t idx1, idx2;
  sTIDL_ConvParams_t &convParams = layer.layerParams.convParams;
  int32_t pads[4];
  int32_t kernel_shape[2];
  int32_t output_shape[2];

  layer.layerType = TIDL_Deconv2DLayer;

  layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  layer.outData[0].elementType = tidl_getElementType(1);

  TensorProto tensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(1), i, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Transpose Conv : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  convParams.numInChannels =  TIDL_geTensorDim(tensor, 0);
  convParams.kernelH =        TIDL_geTensorDim(tensor, 2);
  convParams.kernelW =        TIDL_geTensorDim(tensor, 3);

  convParams.numGroups = 1;
  convParams.dilationW = 1;
  convParams.dilationH = 1;
  convParams.strideW = 1;
  convParams.strideH = 1;
  convParams.padW = 0;
  convParams.padH = 0;
  convParams.enableBias = 0;
  convParams.enablePooling = 0;

  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "strides", &convParams.strideH, 0);
  TIDL_onnxGetIntAttr(node, "strides", &convParams.strideW, 1);
  TIDL_onnxGetIntAttr(node, "dilations", &convParams.dilationH, 0);
  TIDL_onnxGetIntAttr(node, "dilations", &convParams.dilationW, 1);
  TIDL_onnxGetIntAttr(node, "group", &convParams.numGroups, 0);
  TIDL_onnxGetIntAttr(node, "kernel_shape", &kernel_shape[0], 0);
  TIDL_onnxGetIntAttr(node, "kernel_shape", &kernel_shape[1], 1);

  convParams.numOutChannels = TIDL_geTensorDim(tensor, 1) * convParams.numGroups;

  status = TIDL_onnxGetIntAttr(node, "output_shape", &output_shape[0], 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_onnxGetIntAttr(node, "output_shape", &output_shape[0], 0);
    TIDL_onnxGetIntAttr(node, "output_shape", &output_shape[1], 1);

    layer.outData[0].dimValues[TIDL_DIM_HEIGHT] = -output_shape[0];
    layer.outData[0].dimValues[TIDL_DIM_WIDTH]  = -output_shape[1];

    pads[3] = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
    pads[2] = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
  }
  else
  {
    char autoPad[50];
    status = TIDL_onnxGetStringAttr(node, "auto_pad", autoPad, 0);
    if((status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) || (strcmp(autoPad, "NOTSET") == 0))
    {
      TIDL_onnxGetIntAttr(node, "pads",       &pads[0], 0);
      TIDL_onnxGetIntAttr(node, "pads",       &pads[1], 1);
      TIDL_onnxGetIntAttr(node, "pads",       &pads[2], 2);
      TIDL_onnxGetIntAttr(node, "pads",       &pads[3], 3);

      if((pads[0] < pads[2]) && (pads[1] < pads[3])) //padL < padR && padT < padB
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      pads[2] = max(pads[0], pads[2]);
      pads[3] = max(pads[1], pads[3]);
    }
    else if ((strcmp(autoPad, "SAME_UPPER") == 0) || (strcmp(autoPad, "SAME_LOWER") == 0))
    {
      pads[3] = ((convParams.kernelW - 1)*convParams.dilationW) / 2;
      pads[2] = ((convParams.kernelH - 1)*convParams.dilationH) / 2;
      if(strcmp(autoPad, "SAME_UPPER") == 0)
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      else  //SAME_LOWER
      {
        layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
      }
    }
    else if(strcmp(autoPad, "VALID") == 0)
    {
      pads[3] = 0;
      pads[2] = 0;
    }
    else
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Convolution layer : Unsupported auto_pad attribute %s", autoPad);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }


/*
if ((pads[0] != 0) || (pads[1] != 0))
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Pading is only supported in H and W axis ");
  }
*/
  convParams.padW = pads[3];
  convParams.padH = pads[2];
  if ((kernel_shape[0] != convParams.kernelH) || (kernel_shape[1] != convParams.kernelW))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Weight tensor size is not matching with proto kernel_shape, %dx%d vs %dx%d",
    convParams.kernelH, convParams.kernelW, kernel_shape[0], kernel_shape[1]);
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, layer.weights), "");
  /*Permute 0123 -> 1023 for onnx deconv weights based on https://github.com/onnx/onnx/blob/main/docs/Operators.md#ConvTranspose*/
  /*W Shape: No X Ni/G X Fr X FC */
  int32_t i1, i2, i3;
  int32_t tensorSize = convParams.numOutChannels * convParams.numInChannels * convParams.kernelH * convParams.kernelW;
  void ** ptr;
  ptr = &layer.weights.ptr;
  float* srcWtPtr = (float*) *ptr;
  float* weightPtr = (float*)malloc(sizeof(float) * tensorSize);
  if(weightPtr == NULL)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Could not allocate transpose buffer for deconvolution");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  int32_t linePitch = convParams.kernelH * convParams.kernelW;

  for(i1 = 0; i1 < convParams.numOutChannels; i1++)
  {
    for(i2 = 0; i2 < convParams.numInChannels; i2++)
    {
      for(i3 = 0; i3 < linePitch; i3++)
      {
        weightPtr[ i1 * (linePitch * convParams.numInChannels) + i2 * linePitch + i3] = srcWtPtr[i2 * (linePitch * convParams.numOutChannels) + i1 * linePitch  + i3 ];
      }
    }
  }
  free(*ptr);
  *ptr = (void*)weightPtr;
  /******************************************************************************************************************************/
  if (onnGraph.node(i).input_size() > 2)
  {
    convParams.enableBias = 1;
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 2, layer.bias), "");
  }
   return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_onnxMapConvTransposeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapConvTransposeBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSplitBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status, j;
  int32_t axis = 0;
  int32_t splitSize, splitIdx = -1, numDim, attrIdx = -1, split_status = -1, num_out_status = -1, num_outputs = 0;
  sTIDL_SliceLayerParams_t &sliceParams = layer.layerParams.sliceParams;

  layer.layerType = TIDL_SliceLayer;
  layer.numOutBufs = onnGraph.node(i).output_size();
  sliceParams.stride = 1;

  for (j = 0; j < layer.numOutBufs; j++)
  {
    layer.outData[j].elementType = tidl_getElementType(1);
  }

  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "axis", &axis, 0);

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }
  layer.inData[0].numDim = numDim;

  if (axis < 0)
  {
    axis += numDim;
  }

  layer.layerParams.sliceParams.axis = axis + (TIDL_DIM_MAX - numDim);

  sliceParams.slicePoints[0] = 0;
  splitIdx = TIDL_onnxGetAttrIdx(node, "split");

  if (splitIdx != -1)
  {
    split_status = splitIdx;
    splitSize = node.attribute(splitIdx).ints_size();
    if(splitSize != layer.numOutBufs)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Unsupported slice - numSplits %d doesn't match with number of output bufs %d",
      splitSize, layer.numOutBufs);
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    for (j = 0; j < layer.numOutBufs; j++)
    {
      /* split attribute of split layer indicates the length of each slice */
      sliceParams.slicePoints[j+1] = sliceParams.slicePoints[j] + node.attribute(splitIdx).ints(j);
    }
  }
  else
  {
    /**Split can be a input in opset 18*/
    sBuffer_t buf;
    split_status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, buf);
    if (split_status != -1)
    {
      if (buf.bufSize != layer.numOutBufs)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Unsupported slice - numSplits %d doesn't match with number of output bufs %d",
        splitSize, layer.numOutBufs);
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      int64_t *split_ptr = (int64_t*)buf.ptr;
      for (j = 0; j < layer.numOutBufs; j++)
      {
        /* split attribute of split layer indicates the length of each slice */
        sliceParams.slicePoints[j+1] = sliceParams.slicePoints[j] + split_ptr[j];
      }
      free (buf.ptr);
      buf.ptr = NULL;
      buf.bufSize = 0;
    }
    else if(md.numVarInputs <= 1)
    {
      /*Split input does not exist*/
      for (j = 0; j < layer.numOutBufs; j++)
      {
        sliceParams.slicePoints[j] = -1;
      }
    }
    else
    {
      /*Split as variable information is not supported*/
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Variable input for axes is not supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  num_out_status = TIDL_onnxGetIntAttr(node, "num_outputs", &num_outputs, 0);

  /* Either num_outputs or split should be defined, but not both*/
  if (num_out_status != -1 && split_status == -1)
  {
    if (layer.numOutBufs != num_outputs)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Split layer : Either num_outputs or split should be defined, but not both");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }

  return 0;
}
int32_t TIDL_onnxSplitParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status, j;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.numOutBufs = onnGraph.node(i).output_size();
  for (j = 0; j < TIDLPCLayers.numOutBufs; j++)
  {
    TIDLPCLayers.outData[j].dataId = (*dataIndex)++;
  }
  status = TIDL_onnxMapSplitBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSliceBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status, j, isMultipleAxis;
  int32_t axis = 0, numDim;
  sTIDL_SliceLayerParams_t &sliceParams = layer.layerParams.sliceParams;
  sBuffer_t buf, starts, ends, axes;

  layer.layerType  = TIDL_SliceLayer;

  NodeProto node = onnGraph.node(i);
  sliceParams.stride = 1;

  /* In Slice-1, axis can be attribute */
  int32_t axis_status = TIDL_onnxGetIntAttr(node, "axes", &axis, 0);
  /* In Slice-1, starts and ends can be attribute */
  int32_t startsAttrStatus =  TIDL_onnxGetIntAttr(node, "starts", &sliceParams.slicePoints[0], 0);
  int32_t endsAttrStatus   =  TIDL_onnxGetIntAttr(node, "ends",   &sliceParams.slicePoints[1], 0);

  if (TIDL_onnxGetIntAttr(node, "starts", &isMultipleAxis, 1) != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : TIDL_RT only supports single slice axis");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }



  if(node.input_size() < 4)
  {
    if (axis_status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      axis = 0;
    }
  }
  else
  {
    /* if Steps input is present */
    if (node.input_size() == 5)
    {
      status = TIDL_tfCopyFloatConst(onnGraph, i, 4, buf);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *ptr = (long long int *)buf.ptr;
      if (ptr[0] != 1)
      {
        sliceParams.stride = ptr[0];
      }
      free(ptr);
      buf.ptr = NULL;
      buf.bufSize = 0;
    }
  }

  /* Read starts, ends and axes input Slice-10 and above*/
  if (
    (startsAttrStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) &&
    (endsAttrStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)   &&
    (axis_status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  )
  {
      sBuffer_t starts;
      status = TIDL_tfCopyFloatConst(onnGraph, i, 1, starts);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *startsPtr = (long long int *)starts.ptr;

      sBuffer_t ends;
      status = TIDL_tfCopyFloatConst(onnGraph, i, 2, ends);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *endsPtr = (long long int *)ends.ptr;

      sBuffer_t axes;
      status = TIDL_tfCopyFloatConst(onnGraph, i, 3, axes);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      long long int *axesPtr = (long long int *)axes.ptr;

      if(starts.bufSize > 1)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : TIDL_RT only supports single slice axis");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      else
      {
        sliceParams.slicePoints[0] = startsPtr[0];
        if(endsPtr[0] == 0x7fffffffffffffff)
        {
          sliceParams.slicePoints[1] = 0x7fffffff;
        }
        else if(endsPtr[0] == -1)
        {
          sliceParams.slicePoints[1] = -1;
        }
        else
        {
          sliceParams.slicePoints[1] = endsPtr[0];
        }
        axis = axesPtr[0];
      }
      free(startsPtr);
      starts.ptr = NULL;
      starts.bufSize = 0;
      free(endsPtr);
      ends.ptr = NULL;
      ends.bufSize = 0;
      free(axesPtr);
      axes.ptr = NULL;
      axes.bufSize = 0;
  }

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }

  /** Adjust axis to TIDL dimensions*/
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDim);
  }
  else
  {
    /*-ve Axis*/
    axis += TIDL_DIM_MAX;
  }
  layer.layerParams.sliceParams.axis = axis;

  if ((axis < TIDL_DIM_NUMCH) || (axis > TIDL_DIM_WIDTH))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Slice layer : Unsupported axis parameters %d, supported range is [%d,%d]", axis, TIDL_DIM_NUMCH, TIDL_DIM_WIDTH);
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
int32_t TIDL_onnxSliceParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSliceBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxGemmBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  int32_t transA = 0;
  int32_t transB = 1;
  float alpha = 1.0;
  float beta  = 1.0;
  sTIDL_InnerProductParams_t &innerProductParams = layer.layerParams.innerProductParams;
  
  layer.layerType = TIDL_InnerProductLayer;
  layer.outData[0].elementType = tidl_getElementType(1);

  /*Default value for numBchannels:*/
  innerProductParams.numBChannels = 1;
  //Do ONNX attribute checks
  NodeProto node = onnGraph.node(i);

  if(TIDL_onnxGetIntAttr(node, "transA", &transA, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    transA = 0;
  }

  if(TIDL_onnxGetIntAttr(node, "transB", &transB, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    transB = 0;
  }

  if(TIDL_onnxGetFloatAttr(node, "alpha", &alpha, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    alpha = 1.0;
  }

  if(TIDL_onnxGetFloatAttr(node, "beta", &beta, 0) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    beta = 1.0;
  }

  innerProductParams.inputATranspose = transA;
  innerProductParams.inputBTranspose = transB;

  if ((transA != 0) || (alpha != 1.0) || (beta != 1.0))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gemm : Only supported Gemm params are transA = 0, alpha = 1.0 and beta = 1.0. The same will processed as inner product or fully connected layer in TIDL");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  status = TIDL_checkInnerProductTensorProperties(layer);
  if(status == -1) return -1;

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  /**
   * Set the numInBufs to 2 in case of MatMul operator
   * In case of Gemm operator keep the numInBufs to 1 to avoid the addition of const data layer (which results in layer going through MatMul flow)
  */
  if (strcmp ("Gemm", (char*)node.op_type().c_str()) == 0)
  {
    layer.numInBufs = 1;
  }
  else
  {
    layer.numInBufs = 2;
  }

  if((md.constTensorIndices.size() == 1) || (md.constTensorIndices.size() == 2)) //filter, bias. If there is no const means it pure matrix multiplication with both inputs as variable
  {
    int32_t filterIdx = md.constTensorIndices[0]; //first constant tensor should be filter
    TensorProto filterTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(filterIdx), i, status);
    if(status != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gemm : Unable to find initializer at index %d for node %s", filterIdx, onnGraph.node(i).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    int32_t filter_rows, filter_cols;

    {
      // 1xK(inp) X KxN(wt)   = 1xN --> M = 1
      // filterTensor.dims(0) = K
      // filterTensor.dims(1) = N

      int dimDiff = TIDL_DIM_MAX -  md.constTensorsDims[0].size();

      filter_rows = md.constTensorsDims[0][TIDL_DIM_HEIGHT - dimDiff];
      filter_cols = md.constTensorsDims[0][TIDL_DIM_WIDTH - dimDiff];

      if (md.varTensorIndices[0] < md.constTensorIndices[0])
      {
        /* If const is the second input*/
        if(transB)
        {
          innerProductParams.numOutCols = filter_rows;
          innerProductParams.numInCols  = filter_cols;
        }
        else
        {
          innerProductParams.numOutCols = filter_cols;
          innerProductParams.numInCols  = filter_rows;
        }
      }
      else
      {
        /* If const is the second input*/
        if(transA)
        {
          innerProductParams.numInCols = filter_rows;
          innerProductParams.numInRows  = filter_cols;
        }
        else
        {
          innerProductParams.numInCols = filter_cols;
          innerProductParams.numInRows  = filter_rows;
        }
      }

      layer.inData[1].dimValues[TIDL_DIM_BATCH]  = (filterTensor.dims_size() > 3) ? filterTensor.dims(filterTensor.dims_size() - 4) : 1;
      layer.inData[1].dimValues[TIDL_DIM_NUMCH]  = (filterTensor.dims_size() > 2) ? filterTensor.dims(filterTensor.dims_size() - 3) : 1;
      layer.inData[1].dimValues[TIDL_DIM_HEIGHT] = filterTensor.dims(filterTensor.dims_size() - 2);
      layer.inData[1].dimValues[TIDL_DIM_WIDTH]  = filterTensor.dims(filterTensor.dims_size() - 1);

      /* Internally channels*batches is treated as num of batches for matrix multiplication*/
      innerProductParams.numBatches = layer.inData[1].dimValues[TIDL_DIM_BATCH]*layer.inData[1].dimValues[TIDL_DIM_NUMCH];

      assert(filter_rows == layer.inData[1].dimValues[TIDL_DIM_HEIGHT]);
      assert(filter_cols == layer.inData[1].dimValues[TIDL_DIM_WIDTH]);

    }

    status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, filterIdx, layer.weights);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if(md.constTensorIndices.size() == 2) //both weight and bias present
    {
      int32_t biasIdx = md.constTensorIndices[1]; //2nd constant tensor should be bias
      innerProductParams.isBias = 1;
      if(layer.numInBufs == 2)
      {
        innerProductParams.constIdx = 1;
        /*Number of B channels*/
        innerProductParams.numBChannels = layer.inData[1].dimValues[TIDL_DIM_NUMCH];
      }
      else
      {
        innerProductParams.constIdx = -1;
      }
      TensorProto biasTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(biasIdx), i, status);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gemm : Unable to find initializer at index %d for node %s", biasIdx, onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }

      status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, biasIdx, layer.bias);
      if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    else  //bias tensor not specified, values should default to 0
    {
      innerProductParams.isBias = 0;
      innerProductParams.constIdx = 1;
      layer.bias.ptr = calloc(innerProductParams.numOutCols * innerProductParams.numBChannels, sizeof(float));
      layer.bias.bufSize = innerProductParams.numOutCols * innerProductParams.numBChannels;
    }
  }
  else
  {
    if (strcmp ("Gemm", (char*)node.op_type().c_str()) == 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gemm layer : Both variable input is not supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    else
    {
      /*Number of constant tensro is zero*/
      innerProductParams.numInRows  = -1;
      innerProductParams.numInCols  = -1;
      innerProductParams.numOutCols = -1;
      innerProductParams.numBatches = -1;
      innerProductParams.isBias     = 0;
      innerProductParams.constIdx = -1;
      layer.weights.ptr = NULL;
      layer.weights.bufSize = 0;
      layer.bias.ptr= NULL;
      layer.bias.bufSize = NULL;
    }
  }

  return 0;
}

int32_t TIDL_onnxGemmParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxGemmBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSoftmaxBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SoftMaxLayer;
  int32_t axis = -1;
  int32_t status;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  int32_t numDim = md.varTensorsDims[0].size();
  if(numDim > TIDL_DIM_MAX)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Unsupported number of dims");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "axis", &axis, 0);
  /*Translate to TIDL Axis:*/
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDim);
  }
  else
  {
    /*-ve Axis*/
    axis = (TIDL_DIM_MAX + axis);
  }
  layer.layerParams.softMaxParams.axis = axis;
  /* Only support axis are last 2 dimension */
  if ((axis != TIDL_DIM_HEIGHT) && (axis != TIDL_DIM_WIDTH))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : SoftMax layer : TIDL_RT only supports axis width or height");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /*Prevent transpose of output*/
  layer.layerParams.softMaxParams.outTranspose = 0;
  return 0;
}

int32_t TIDL_onnxSoftmaxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSoftmaxBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSoftplusBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SoftPlusLayer;
  layer.actParams.actType = TIDL_SoftPlus;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapSoftplusParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSoftplusBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSoftsignBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SoftSignLayer;
  layer.actParams.actType = TIDL_SoftSign;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapSoftsignParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSoftsignBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapCeilBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_CeilLayer;
  layer.actParams.actType = TIDL_Ceil;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapCeilParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapCeilBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapCeluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_CeluLayer;
  layer.actParams.actType = TIDL_Celu;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapCeluParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapCeluBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSeluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SeluLayer;
  layer.actParams.actType = TIDL_Selu;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapSeluParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSeluBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapRoundBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_RoundLayer;
  layer.actParams.actType = TIDL_Round;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapRoundParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapRoundBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSignBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SignLayer;
  layer.actParams.actType = TIDL_Sign;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapSignParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSignBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

void TIDL_tfBNToScaleBias(
  float    * scale,
  float    * bias,
  uint32_t  numCh,
  float * mean,
  float * var,
  float * gamma,
  float * beta,
  float eps
  );

int32_t TIDL_onnxMapBNBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  sBuffer_t gamma;
  sBuffer_t beta;
  sBuffer_t mean;
  sBuffer_t variance;
  float epsilon = 1e-05;
  int32_t dataSize;

  layer.layerType = TIDL_BatchNormLayer;
  NodeProto node = onnGraph.node(i);
  int32_t status;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, gamma);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Batchnorm : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 2, beta);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Batchnorm : Unable to find initializer at index %d for node %s", 2, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 3, mean);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Batchnorm : Unable to find initializer at index %d for node %s", 3, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 4, variance);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Batchnorm : Unable to find initializer at index %d for node %s", 4, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  TIDL_onnxGetFloatAttr(node, "epsilon", &epsilon, 0);

  /* ONNX Opset Version 18 introduces a new attribute called training mode in the Batch Normalization Operator.*/
  int32_t training_mode = 0;
  TIDL_onnxGetIntAttr(node, "training_mode", &training_mode, 0);
  /* TIDL_RT doesn't support training mode in Batch Normalization. */
  if(training_mode)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Batchnorm : Training mode not supported by TIDL_RT");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  dataSize = gamma.bufSize;
  layer.weights.ptr = my_malloc(dataSize*sizeof(float));
  layer.weights.bufSize = dataSize;
  layer.bias.ptr = my_malloc(dataSize*sizeof(float));
  layer.bias.bufSize = dataSize;


  TIDL_tfBNToScaleBias((float *)layer.weights.ptr,
    (float *)layer.bias.ptr, dataSize,
    (float *)mean.ptr, (float *)variance.ptr, (float *)gamma.ptr, (float *)beta.ptr, epsilon
    );
  my_free(mean.ptr);
  my_free(variance.ptr);
  my_free(gamma.ptr);
  my_free(beta.ptr);

  return 0;
}

int32_t TIDL_onnxMapBNParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapBNBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapMaxPoolBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_checkPoolingTensorProperties(layer);
  if(status == -1) return -1;

  /*Check that the dimensionality of inputs is less than 4 (Supported configuration)*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    /*Shape inference information is present:*/
    for(int32_t j = 0; j < layer.allowlistingMetaData.varTensorsDims.size(); j++)
    {
      int32_t numDim = layer.allowlistingMetaData.varTensorsDims[j].size();
      if(numDim > 4)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Unsupported number of dimensions for Pooling (>4)");
        return -1;
      }
    }
  }

  int32_t padType;
  int32_t idx1, idx2;
  int32_t pads[4] = {0};
  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;
  layer.layerType = TIDL_PoolingLayer;
  poolParams.poolingType = TIDL_MaxPooling;
  poolParams.strideW = 1;
  poolParams.strideH = 1;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padT = 0;
  poolParams.padB = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;

  poolParams.useCeil = 0;
  NodeProto node = onnGraph.node(i);

  std::vector<int32_t> attr;
  attr.push_back(TIDL_onnxGetIntAttr(node, "kernel_shape", &poolParams.kernelW, 1));
  attr.push_back(TIDL_onnxGetIntAttr(node, "kernel_shape", &poolParams.kernelH, 0));

  if(TIDL_onnxGetAttrIdx(node, "strides") != -1)
  {
    attr.push_back(TIDL_onnxGetIntAttr(node, "strides", &poolParams.strideW, 1));
    attr.push_back(TIDL_onnxGetIntAttr(node, "strides", &poolParams.strideH, 0));
  }

  char autoPad[50];
  status = TIDL_onnxGetStringAttr(node, "auto_pad", autoPad, 0);
  if((status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) || (strcmp(autoPad, "NOTSET") == 0))
  {
    if(TIDL_onnxGetAttrIdx(node, "pads") != -1)
    {
      attr.push_back(TIDL_onnxGetIntAttr(node, "pads",       &pads[0], 0));
      attr.push_back(TIDL_onnxGetIntAttr(node, "pads",       &pads[1], 1));
      attr.push_back(TIDL_onnxGetIntAttr(node, "pads",       &pads[2], 2));
      attr.push_back(TIDL_onnxGetIntAttr(node, "pads",       &pads[3], 3));
      if((pads[0] < pads[2]) && (pads[1] < pads[3]) && (poolParams.strideW != 1) && (poolParams.strideH != 1)) //padL < padR && padT < padB
      {
        /* This condition will occur when stride = 2 and size of input tensor is even
          strideOffsetMethod is not handled in pooling layer, so take care of this in TIDL_tfOutReshapePoolingLayer */
        layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
      }
      poolParams.padT = pads[0];
      poolParams.padB = pads[1];
      poolParams.padL = pads[2];
      poolParams.padR = pads[3];
      pads[2] = max(pads[0], pads[2]);
      pads[3] = max(pads[1], pads[3]);

    }
  }
  else if ((strcmp(autoPad, "SAME_UPPER") == 0) || (strcmp(autoPad, "SAME_LOWER") == 0))
  {
    if(poolParams.strideW > 1)
    {
      pads[3] = (poolParams.kernelW - 1) / 2;
    }
    else
    {
      pads[3] = poolParams.kernelW / 2;
    }
    if(poolParams.strideH > 1)
    {
      pads[2] = (poolParams.kernelH - 1) / 2;
    }
    else
    {
      pads[2] = poolParams.kernelH / 2;
    }

    if(strcmp(autoPad, "SAME_UPPER") == 0)
    {
      layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
    }
    else  //SAME_LOWER
    {
      layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
    }
  }
  else if(strcmp(autoPad, "VALID") == 0)
  {
    pads[3] = 0;
    pads[2] = 0;
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MaxPool layer : Unsupported auto_pad attribute %d", autoPad);
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if(std::find(attr.begin(), attr.end(), -1) != attr.end())
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : MaxPool layer : Missing attribute");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  poolParams.padW = pads[3];
  poolParams.padH = pads[2];

  return 0;
}

int32_t TIDL_onnxMapMaxPoolParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapMaxPoolBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapArgOpBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status, numDims;
  int32_t axis = 0, keepdim = 1;
  sTIDL_ArgOpParams_t &argOpParams = layer.layerParams.argOpParams;
  layer.layerType = TIDL_ArgOpLayer;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
   if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDims = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDims = (numDims == 0)? 4:numDims;
  }
  else
  {
    numDims = 4;
  }
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "axis", &axis, 0);
  TIDL_onnxGetIntAttr(node, "keepdims", &keepdim, 0);
  if (keepdim != 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ArgOp layer : only keepdims = 1 is supported for ArgOp layer");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /* Adjust axis to TIDL dimensions */
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDims);
  }
  else
  {
    /*-ve Axis*/
    axis += TIDL_DIM_MAX;
  }
  if (axis != TIDL_DIM_NUMCH)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ArgOp layer: Only axis = 1 is supported for ArgOp layer");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /*New attribute called select last index introduced. */
  int32_t select_last_index = 0;
  TIDL_onnxGetIntAttr(node, "select_last_index", &select_last_index, 0);
  /* If select_last_index is set true then exit. */
  if(select_last_index)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ArgOp layer: Selecting last index is not supported by TIDL_RT");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return 0;
}
int32_t TIDL_onnxMapArgOpParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapArgOpBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapReluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_ReLULayer;
  layer.actParams.actType = TIDL_RelU;
  return 0;
}

int32_t TIDL_onnxMapReluParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapReluBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapClipBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  layer.layerType = TIDL_ClipLayer;
  layer.actParams.actType = TIDL_Clip;
  NodeProto node = onnGraph.node(i);
  int32_t minStatus = -1, maxStatus = -1;
  int32_t numInputs = node.input_size();
  sBuffer_t buf;

  /* Try to fetch min and max from attributes */
  minStatus = TIDL_onnxGetFloatAttr(node, "min", &layer.actParams.clipMin, 0);
  maxStatus = TIDL_onnxGetFloatAttr(node, "max", &layer.actParams.clipMax, 0);

  /* If min not found */
  if (minStatus == -1)
  {
    /* Try to fetch min from 2nd input*/
    if (numInputs > 1)
    {
      status = TIDL_tfCopyFloatConst(onnGraph, i, 1, buf);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Clip layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      float *ptr = (float *)buf.ptr;
      layer.actParams.clipMin = *ptr;
      my_free(ptr);
    }
    /* Set default value of min */
    else
    {
      layer.actParams.clipMin = (float32_tidl)std::numeric_limits<float>::lowest();
    }
  }
  /* If max not found */
  if (maxStatus == -1)
  {
    /* Try to fetch max from 3rd input*/
    if ( numInputs > 2 )
    {
      status = TIDL_tfCopyFloatConst(onnGraph, i, 2, buf);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Clip layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      float *ptr = (float *)buf.ptr;
      layer.actParams.clipMax = *ptr;
      my_free(ptr);
    }
    /* Set default value of max */
    else
    {
      layer.actParams.clipMax = (float32_tidl)std::numeric_limits<float>::max();
    }
  }

  return 0;
}

int32_t TIDL_onnxMapClipParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapClipBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapPReluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  if (md.numVarInputs > 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : PReLU does not support variable slope");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_onnxMapReluBaseParams(onnGraph, i, layer);
  layer.actParams.actType = TIDL_PRelU;
  return 0;
}

int32_t TIDL_onnxMapPReluParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapPReluBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSigmoidBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SigmoidLayer;
  layer.actParams.actType = TIDL_Sigmoid;
  layer.outData[0].elementType = tidl_getElementType(0);
  return 0;
}

int32_t TIDL_onnxMapSigmoidParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSigmoidBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapTanhBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_TanhLayer;
  layer.actParams.actType = TIDL_Tanh;
  layer.outData[0].elementType = tidl_getElementType(1);
  return 0;
}

int32_t TIDL_onnxMapTanhParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status = 0;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapTanhBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapHardSigmoidBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_HardSigmoidLayer;
  layer.actParams.actType = TIDL_HardSigmoid;
  layer.outData[0].elementType = tidl_getElementType(0);
  sTIDL_HardSigmoidParams_t &hardSigmoidParams      = layer.layerPCParams.hardSigmoidParams;
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetFloatAttr(node, "alpha",   &hardSigmoidParams.alpha, 0);
  hardSigmoidParams.beta = 0.5;
  return 0;
}

int32_t TIDL_onnxMapHardSigmoidParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status = 0;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapHardSigmoidBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapELUBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_ELULayer;
  layer.actParams.actType = TIDL_ELU;
  layer.outData[0].elementType = tidl_getElementType(1);
  sTIDL_EluParams_t &eluParams      = layer.layerPCParams.eluParams;
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetFloatAttr(node, "alpha",   &eluParams.alpha, 0);
  return 0;
}

int32_t TIDL_onnxMapELUParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status = 0;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapELUBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapLeakyReluBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_LeakyReluLayer;
  layer.outData[0].elementType = tidl_getElementType(1);
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetFloatAttr(node, "alpha", &layer.layerPCParams.leakyReluParams.alpha, 0);
  return 0;
}
int32_t TIDL_onnxMapLeakyReluParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapLeakyReluBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapFlattenBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t axis, numDims;
  int32_t status=0;
  layer.layerType = TIDL_FlattenLayer;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDims = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDims = (numDims == 0)? 4:numDims;
  }
  else
  {
    numDims = 4;
  }

  if (TIDL_onnxGetIntAttr(onnGraph.node(i), "axis", &axis, 0) == -1)
  {
    axis = 0;
  }

  /* Adjust axis to TIDL dimensions */
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDims);
  }
  else
  {
    /*-ve Axis*/
    axis += TIDL_DIM_MAX;
  }
  layer.layerPCParams.flattenParams.axis = axis;

  return 0;
}
int32_t TIDL_onnxMapFlattenParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapFlattenBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapPadBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t idx, j;
  layer.layerType = TIDL_PadLayer;

  NodeProto node = onnGraph.node(i);
  int32_t padT, padB, padL, padR;
  int32_t tot_axis, atrIdx, axes_status, numDims;
  char padMode[50];
  float32_tidl constant_value;
  sBuffer_t buf, axes;
  int32_t* padValStore;

  int32_t status;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDims = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDims = (numDims == 0)? 4:numDims;
  }
  else
  {
    numDims = 4;
  }

  TIDL_onnxGetStringAttr(node, "mode", padMode, 0);

  if ((strcmp(padMode, "constant") != 0))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pad layer : only constant pad mode is supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /* Pad v1 & v2 can have value in Attributes */
  atrIdx = TIDL_onnxGetFloatAttr(node, "value", &constant_value, 0);
  if(atrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    layer.layerParams.padLayerParams.padConstValue = (int32_t)constant_value;
  }
  else
  {
    /* In other cases pad value can be in 3rd input */
    atrIdx = TIDL_tfCopyFloatConst(onnGraph, i, 2, buf);
    if (atrIdx != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      float32_tidl *padPtr = (float32_tidl *)buf.ptr;
      layer.layerParams.padLayerParams.padConstValue = (int32_t)padPtr[0];
      free(buf.ptr);
      buf.bufSize = 0;
      buf.ptr = NULL;
    }
    /* If still not found, set default value as 0 */
    else
    {
      layer.layerParams.padLayerParams.padConstValue = 0;
    }
  }


  /* Axes can be input in v18 */
  axes_status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 3, axes);
  int64_t *axesPtr = (int64_t *)axes.ptr; /* The axesPtr points to the pointer in the axes buffer.*/

  /* If the optional input axes is populated.*/
  if(axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    /** Iterating through the axes buffer.
     * Ignore repetition as described as undefined behavior in onnx
    */
    for(int j = 0; j < axes.bufSize; j++)
    {
      /* Adjust to TIDL dimensions */
      if(axesPtr[j] >= 0)
      {
        /*+ve Axis*/
        axesPtr[j] += (TIDL_DIM_MAX - numDims);
      }
      else
      {
        /*-ve Axis*/
        axesPtr[j] += TIDL_DIM_MAX;
      }

      /* If axes are not height or width, do not support */
      if ((axesPtr[j] != TIDL_DIM_HEIGHT) && (axesPtr[j] != TIDL_DIM_WIDTH))
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pad layer : Padding is only supported for width/height axes");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  /**
   * Pad can be in attribute
   * in v1 named as padding
   * in v2 named as pads
  */
  atrIdx = TIDL_onnxGetAttrIdx(node, "pads");
  if (atrIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    atrIdx = TIDL_onnxGetAttrIdx(node, "paddings");
  }
  /* If found in attribute */
  if (atrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    tot_axis = node.attribute(atrIdx).ints_size() >> 1;
    padValStore = (int32_t *)my_malloc(tot_axis * 2 * sizeof(int32_t));

    for (j = 0; j < tot_axis*2; j++)
    {
      TIDL_onnxGetIntAttr(node, "pads", &padValStore[j], j);
    }

    if(tot_axis > 2) /* more than width and height */
    {
      for (j = 0; j < (tot_axis-2); j++)
      {
        /* Cannot be non-zero in these dimensions */
        if(( padValStore[j] != 0 ) || ( padValStore[tot_axis + j] != 0 ))
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pad layer : Padding is only supported in width/height axis");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
    }

    padT = padValStore[tot_axis   - 2];
    padB = padValStore[tot_axis*2 - 2];
    padL = padValStore[tot_axis   - 1];
    padR = padValStore[tot_axis*2 - 1];
    my_free(padValStore);
  }
  /* Pad value can be in 2nd input */
  else
  {
    status = TIDL_tfCopyFloatConst(onnGraph, i, 1, buf);
    if(status != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pad layer : Unable to find float intializer");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    int64_t *padPtr = (int64_t *)buf.ptr;
    tot_axis = buf.bufSize >> 1;

    /* Axes is present */
    if(axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      if (tot_axis != axes.bufSize) /*The size of the axes tensor must be half the pad input size if axes exists*/
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pad layer : Invalid input for axes or pads - dimensions are not matching");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      else
      {
        for (int j = 0; j < tot_axis; j++)
        {
          if (axesPtr[j] == TIDL_DIM_WIDTH)
          {
            padL = padPtr[j];
            padR = padPtr[tot_axis+j];
          }
          else if (axesPtr[j] == TIDL_DIM_HEIGHT)
          {
            padT = padPtr[j];
            padB = padPtr[tot_axis+j];
          }
          else
          {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pad layer : Padding is only supported for width/height axes");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
          }
        }
      }
    }
    /* Axes is not given */
    else
    {
      if(tot_axis > 2) /*more than width and height*/
      {
        for (j = 0; j < (tot_axis-2); j++)
        {
          /* Cannot be non-zero in these dimensions */
          if(( padPtr[j] != 0 ) || ( padPtr[tot_axis + j] != 0 ))
          {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pad layer : Padding is only supported in width/height axis");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
          }
        }
      }

      padT = padPtr[tot_axis   - 2];
      padB = padPtr[tot_axis*2 - 2];
      padL = padPtr[tot_axis   - 1];
      padR = padPtr[tot_axis*2 - 1];
    }
    my_free(buf.ptr);
  }


  /* if axis was being used, free axis ptr */
  if (axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    my_free(axes.ptr);
    axes.bufSize = 0;
    axes.ptr = NULL;
  }

  layer.layerParams.padLayerParams.padT = padT;
  layer.layerParams.padLayerParams.padB = padB;
  layer.layerParams.padLayerParams.padL = padL;
  layer.layerParams.padLayerParams.padR = padR;
  return 0;
}
int32_t TIDL_onnxMapPadParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapPadBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapAddBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0, constSize = 0;

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_checkAddMulSubDivTensorProperties(layer,BASE_OP_ADD);
  if(status == -1) return -1;

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  int32_t is1DConstTensor = TIDL_canConvertToBatchNorm (layer);
  if(md.numConstInputs > 0) //addition with constant tensor
  {
    int constTensorIdx = md.constTensorIndices[0];
    if (is1DConstTensor == 1)
    {
      TensorProto constTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(constTensorIdx), i, status);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Add layer : Unable to find initializer at index %d for node %s", constTensorIdx, onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }

      int32_t varTensorIdx = md.varTensorIndices[0];

      constSize = TIDL_getConstSize (constTensor);
      if(constSize == 1) /*  Add constant to layer */
      {
          layer.layerType = TIDL_BatchNormLayer;
          status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.bias);
          if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
          {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
          }
          int32_t biasSize = 1;
          if (md.varTensorsDims[0].size() >= 3)
          {
            biasSize = md.varTensorsDims[0][md.varTensorsDims[0].size()-3]; //broadcast the constant along number of channels
          }
          else
          {
            /* We have only H and W dims, so constant addition to the available single channel, no need to broadcast
            We preserve default biasSize = 1 set above */
          }

          int32_t nodeIdx = TIDL_onnxGetNodeIdx(onnGraph, onnGraph.node(i).input(constTensorIdx).c_str());
          int32_t isInputDQ = 0;
          if (nodeIdx != -1)
          {
            isInputDQ = strcmp(onnGraph.node(nodeIdx).op_type().c_str(), "DequantizeLinear") == 0;
          }
          if(constTensor.data_type() == TensorProto_DataType_FLOAT ||
              isInputDQ)
          {
            TIDL_onnxBroadcastFloatTensor(layer.bias, biasSize);
          }
          else
          {
            TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Add layer : Supported only if data type of constant is float");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
          }
      }
      else //addition with constant vector
      {
        layer.layerType = TIDL_BatchNormLayer;
        status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.bias);
        if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }

      layer.weights.ptr             = my_malloc(layer.bias.bufSize*sizeof(float));
      layer.weights.bufSize         = layer.bias.bufSize;
      float *ptr = (float *)layer.weights.ptr;
      for (int lc = 0; lc < layer.weights.bufSize; lc++)
        ptr[lc] = 1.0;
      return 0;
    }
    else
    {
      layer.layerType = TIDL_EltWiseLayer;
      layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseSum;
      layer.numInBufs = md.numInputs;
      status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.weights);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }
  else //both inputs variable
  {
    layer.layerType = TIDL_EltWiseLayer;
    layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseSum;
    layer.numInBufs = md.numInputs;
  }
  return 0;
}

int32_t TIDL_onnxMapAddParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapAddBaseParams(onnGraph, i, TIDLPCLayers);

  return status;
}

int32_t TIDL_onnxMapMaxBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_EltWiseLayer;
  TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseMax;
  layer.numInBufs = md.numInputs;
  layer.layerParams.eltWiseParams.numInData = layer.numInBufs ;
  return 0;
}

int32_t TIDL_onnxMapMaxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapMaxBaseParams(onnGraph, i, TIDLPCLayers);

  return status;
}

int32_t TIDL_onnxMapSubBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0, constSize = 0;
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX && gParams.debugTraceLevel > 2)
  {
    TIDL_GLOBAL_REPORT_WARNING("Subtract with constant tensor requires input dimensions of sub layer to be present as part of the network.\
      If present, this warning can be ignored. If not, please use open source runtimes offering to run this model or run shape inference on this model before executing import");
  }

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_checkAddMulSubDivTensorProperties(layer, BASE_OP_SUB);
  if(status == -1) return -1;
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  if(md.numConstInputs > 0) //subtraction with constant tensor
  {
    int constTensorIdx = md.constTensorIndices[0];
    TensorProto constTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(constTensorIdx), i, status);
    if(status != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Sub layer : Unable to find initializer at index %d for node %s", constTensorIdx, onnGraph.node(i).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    int32_t varTensorIdx = md.varTensorIndices[0];
    constSize = TIDL_getConstSize (constTensor);

    if(constSize == 1) /*  Sub constant to layer */
    {
        layer.layerType = TIDL_BatchNormLayer;
        status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.bias);
        if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }

        int32_t biasSize = 1;
        if (md.varTensorsDims[0].size() >= 3)
        {
          biasSize = md.varTensorsDims[0][md.varTensorsDims[0].size()-3]; //broadcast the constant along number of channels
        }
        else
        {
          /* We have only H and W dims, so constant addition to the available single channel, no need to broadcast
          We preserve default biasSize = 1 set above */
        }

        int32_t nodeIdx = TIDL_onnxGetNodeIdx(onnGraph, onnGraph.node(i).input(constTensorIdx).c_str());
        int32_t isInputDQ = 0;
        if (nodeIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          isInputDQ = strcmp(onnGraph.node(nodeIdx).op_type().c_str(), "DequantizeLinear") == 0;
        }

        if(constTensor.data_type() == TensorProto_DataType_FLOAT ||
            isInputDQ)
        {
          TIDL_onnxBroadcastFloatTensor(layer.bias, biasSize);
        }
        else
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Sub layer : Supported only if data type of constant is float");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
    }
    else //Subtraction with constant vector
    {
      layer.layerType = TIDL_BatchNormLayer;
      status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.bias);
      if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }

    layer.weights.ptr             = my_malloc(layer.bias.bufSize*sizeof(float));
    layer.weights.bufSize         = layer.bias.bufSize;
    float *ptr = (float *)layer.weights.ptr;
    /* case: const - X */
    if(md.constTensorIndices[0]  < md.varTensorIndices[0] )
    {
      for (int lc = 0; lc < layer.weights.bufSize; lc++)
        ptr[lc] = -1.0;
    }
    /* case: X - const */
    else
    {
      /* scaling bias with: -1 */
      float *biasPtr = (float *)layer.bias.ptr;
      for (int lc = 0; lc < layer.bias.bufSize; lc++)
        biasPtr[lc] = -1.0*biasPtr[lc];

      for (int lc = 0; lc < layer.weights.bufSize; lc++)
        ptr[lc] = 1.0;
    }
    return 0;
  }
  else
  {
    layer.layerType = TIDL_SubLayer;
    layer.numInBufs = md.numInputs;
  }
  return 0;
}

int32_t TIDL_onnxMapSubParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSubBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapLayerNormBaseParams (GraphProto&  onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t axis, status=0;
  float32_tidl epsilon;
  
  layer.layerType = TIDL_LayerNormLayer;
  layer.numInBufs = 1;
  NodeProto node = onnGraph.node(i);

  TIDL_onnxGetIntAttr(node, "axis", &axis, 0);
  layer.layerParams.layerNormParams.axis = axis;

  if (axis < 0)
  {
    axis += TIDL_DIM_MAX;
    layer.layerParams.layerNormParams.axis = axis;
  }

  TIDL_onnxGetFloatAttr(node, "epsilon", &epsilon, 0);
  layer.layerParams.layerNormParams.epsilon = epsilon;

  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, layer.weights);
  if(status == -1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : TIDL_LayerNormLayer  -- file info - ");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 2, layer.bias);
  if(status == -1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : TIDL_LayerNormLayer  -- file info - ");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  return status;
}

int32_t TIDL_onnxMapLayerNormParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapLayerNormBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapRMSNormBaseParams (GraphProto&  onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t axis=-1, stash_type=1, status=0;
  float32_tidl epsilon=1e-5;
  
  layer.layerType = TIDL_RMSNormalizationLayer;
  layer.numInBufs = 1;
  NodeProto node = onnGraph.node(i);

  int32_t numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
  status = TIDL_onnxGetIntAttr(node, "axis", &axis, 0);
  if (status != TIDL_IMPORT_DIAGNOSIS_RETURN_OK) 
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read axis attribute : TIDL_RMSNormalizationLayer");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  if(numDim == 0)
  {
    /*Shape inference is not available - assuming 4 dimensions:*/
    numDim = 4;
  }
  /*Translate to TIDL Axis:*/
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDim);
  }
  else
  {
    /*-ve Axis*/
    axis = (TIDL_DIM_MAX + axis);
  }
  layer.layerParams.rmsNormParams.axis = axis;

  TIDL_onnxGetFloatAttr(node, "epsilon", &epsilon, 0);
  /* Handle the case of divide by 0 */
  if(epsilon == (float32_tidl)0)
  {
    epsilon = epsilon + 10e-5;
  }
  layer.layerParams.rmsNormParams.epsilon = epsilon;

  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, layer.weights);
  if(status == -1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : TIDL_RMSNormalizationLayer  -- file info - ");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  return status;
}

int32_t TIDL_onnxMapRMSNormParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapRMSNormBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapDivParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapDivBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapMulBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0, constSize = 0;
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX  && gParams.debugTraceLevel > 2)
  {
    TIDL_GLOBAL_REPORT_WARNING("Mul with constant tensor requires input dimensions of mul layer to be present as part of the network.\
      If present, this warning can be ignored. If not, please use open source runtimes offering to run this model or run shape inference on this model before executing import");
  }

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  status = TIDL_checkAddMulSubDivTensorProperties(layer, BASE_OP_MUL);
  if(status == -1) return -1;

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  int32_t is1DConstTensor = TIDL_canConvertToBatchNorm (layer);
  if(md.numConstInputs > 0) //multiply with constant tensor
  {
    int constTensorIdx = md.constTensorIndices[0];
    status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.weights);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if (is1DConstTensor == 1)
    {
      layer.layerType = TIDL_BatchNormLayer;

      TensorProto constTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(constTensorIdx), i, status);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Mul layer : Unable to find initializer at index %d for node %s", constTensorIdx, onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      if(constTensor.dims_size() == 0) //tensor dim size = 0 --- Mul constant to layer
      {
        int32_t nodeIdx = TIDL_onnxGetNodeIdx(onnGraph, onnGraph.node(i).input(constTensorIdx).c_str());
        int32_t isInputDQ = 0;
        if (nodeIdx != -1)
        {
          isInputDQ = strcmp(onnGraph.node(nodeIdx).op_type().c_str(), "DequantizeLinear") == 0;
        }

        int32_t wtSize = 1;
        if (md.varTensorsDims[0].size() >= 3)
        {
          wtSize = md.varTensorsDims[0][md.varTensorsDims[0].size()-3]; //broadcast the constant along number of channels
        }
        else
        {
          /* We have only H and W dims, so constant addition to the available single channel, no need to broadcast
          We preserve default wtSize = 1 set above */
        }

        if(constTensor.data_type() == TensorProto_DataType_FLOAT ||
            isInputDQ)
        {
          TIDL_onnxBroadcastFloatTensor(layer.weights, wtSize);
        }
        else
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Mul layer : Supported only if data type of constant is float");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else //Multiply with constant vector
      {
        /* no broadcast required for vector */
      }

      /* Update bias for batchnorm */
      layer.bias.ptr = calloc(layer.weights.bufSize, sizeof(float));
      layer.bias.bufSize = layer.weights.bufSize;
    }
    else /* > 1D const tensor */
    {
      layer.layerType = TIDL_EltWiseLayer;
      layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseProduct;
      layer.numInBufs = md.numInputs;
    }
  }
  else /* both inputs variable */
  {
    layer.layerType = TIDL_EltWiseLayer;
    layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseProduct;
    layer.numInBufs = md.numInputs;
  }
  return 0;
}

int32_t TIDL_onnxMapMulParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapMulBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}



int32_t TIDL_onnxMapDivBaseParams (GraphProto&  onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0, constSize = 0;
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX && gParams.debugTraceLevel > 2)
  {
    TIDL_GLOBAL_REPORT_WARNING("Div by constant tensor requires input dimensions of div layer to be present as part of the network.\
      If present, this warning can be ignored. If not, please use open source runtimes offering to run this model or run shape inference on this model before executing import -- Node name - %s", onnGraph.node(i).name());
  }

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  int32_t is1DConstTensor = TIDL_canConvertToBatchNorm (layer);
	if (md.numConstInputs > 0)	// div by constant tensor
	{
    int constTensorIdx = md.constTensorIndices[0];

    status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.weights);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if (is1DConstTensor == 1) /* 1D constant - either scalar value or 1D vector */
    {
      layer.layerType = TIDL_BatchNormLayer;

      /**
       * TIDL doesnot support div support as is,
       * Change the div to mul by adjusting the constant
       * VarTensor/Constant -> VarTensor * (1/Constant)
      */
      float* weights_ptr = (float*)layer.weights.ptr;
      for(int i = 0; i < layer.weights.bufSize; i++)
      {
        float val = weights_ptr[i];
        val = 1/val;  /* ONNX checks for division by 0 */
        weights_ptr[i] = val;
      }

      TensorProto constTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(constTensorIdx), i, status);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Div layer : Unable to find initializer at index %d for node %s", constTensorIdx, onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      if (constTensor.dims_size() == 0) /* scalar constant */
      {
        int32_t nodeIdx = TIDL_onnxGetNodeIdx(onnGraph, onnGraph.node(i).input(constTensorIdx).c_str());
        int32_t isInputDQ = 0;
        if (nodeIdx != -1)
        {
          isInputDQ = strcmp(onnGraph.node(nodeIdx).op_type().c_str(), "DequantizeLinear") == 0;
        }

        int32_t wtSize = 1;
        if (md.varTensorsDims[0].size() >= 3)
        {
          wtSize = md.varTensorsDims[0][md.varTensorsDims[0].size()-3]; //broadcast the constant along number of channels
        }
        else
        {
          /* We have only H and W dims, so constant addition to the available single channel, no need to broadcast
          We preserve default wtSize = 1 set above*/
        }

        if(constTensor.data_type() == TensorProto_DataType_FLOAT ||
            isInputDQ)
        {
          TIDL_onnxBroadcastFloatTensor(layer.weights, wtSize);
        }
        else
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Div layer : Supported only if data type of constant is float");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else //Divide with constant vector
      {
        /* no broadcast required for vector */
      }

      /* Update bias for batchnorm */
      layer.bias.ptr = calloc(layer.weights.bufSize, sizeof(float));
      layer.bias.bufSize = layer.weights.bufSize;
    }
    else /* > 1D const tensor */
    {
      layer.layerType = TIDL_EltWiseLayer;
      layer.layerParams.eltWiseParams.eltWiseType = TIDL_EltWiseProduct;
      layer.numInBufs = md.numInputs;
    }
	}
  else	/* both input variable - not supported */
	{
    /* TODO : This is done specifically for layernorm. Ideally should exit as not supported
    if div is not in layernorm fusable combination */
    layer.layerType = TIDL_DivLayer;
    layer.numInBufs = md.numInputs;
  }

  status = TIDL_checkAddMulSubDivTensorProperties(layer, BASE_OP_DIV);
  if (status == -1)
  {
    return -1;
  }

	return 0;
}


int32_t TIDL_onnxMapConcatBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t axis, status=0, numDim;
  layer.layerType = TIDL_ConcatLayer;
  layer.numInBufs = onnGraph.node(i).input_size();
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "axis", &axis, 0);
  layer.layerParams.concatParams.axis = axis;

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  if (md.numConstInputs == 1)
  {
    status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, md.constTensorIndices[0], layer.weights);
  }
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return 0;
}

int32_t TIDL_onnxMapConcatParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapConcatBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapUpsampleBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t idx, j;
  int32_t status = 0;
  char upsampleMode[50];
  layer.layerType = TIDL_ResizeLayer;
  int32_t axis;
  layer.numInBufs = 1;
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetStringAttr(node, "mode", upsampleMode, 0);
  if ((strcmp(upsampleMode, "nearest") == 0))
  {
    layer.layerParams.resizeParams.mode = TIDL_ResizeNearest;
  }
  else if((strcmp(upsampleMode, "linear") == 0))
  {
    layer.layerParams.resizeParams.mode = TIDL_ResizeBilinear;
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Unsupported upsample mode ");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  if ((layer.layerParams.resizeParams.mode == TIDL_ResizeBilinear) ||
      (layer.layerParams.resizeParams.mode == TIDL_ResizeNearest))
  {
    sBuffer_t buf;
    status = TIDL_tfCopyFloatConst(onnGraph, i, 1, buf);
    if(status != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Upsample layer : Unable to find float intializer");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    float *ptr = (float *)buf.ptr;

    int indices[6]={TIDL_DIM_WIDTH,TIDL_DIM_HEIGHT,TIDL_DIM_NUMCH,TIDL_DIM_BATCH,TIDL_DIM_DIM2,TIDL_DIM_DIM1};
    for(int i=buf.bufSize-1,j=0;i>=0;i--,j++)
    {
      layer.layerParams.resizeParams.resizeRatio[indices[j]]=ptr[i];
    }
    my_free(ptr);
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_onnxMapUpsampleParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapUpsampleBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapResizeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t idx, j, axes_idx = -1;
  int32_t status = 0;
  char upsampleMode[50];
  layer.layerType = TIDL_ResizeLayer;
  int32_t axis;
  layer.numInBufs = 1;
  sBuffer_t buf;
  NodeProto node = onnGraph.node(i);
  int32_t attrRetVal = TIDL_onnxGetStringAttr(node, "mode", upsampleMode, 0);
  layer.layerParams.resizeParams.mode = TIDL_ResizeNearest; /* default value of attribute */
  if ((strcmp(upsampleMode, "nearest") == 0))
  {
    layer.layerParams.resizeParams.mode = TIDL_ResizeNearest;
  }
  else if((strcmp(upsampleMode, "linear") == 0))
  {
    layer.layerParams.resizeParams.mode = TIDL_ResizeBilinear;
  }
  else if(attrRetVal != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) /* Deny only if attribute(mode) actually present in the model but none of above */
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize layer : Unsupported resize mode %s", upsampleMode);
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if ((layer.layerParams.resizeParams.mode == TIDL_ResizeBilinear) ||
      (layer.layerParams.resizeParams.mode == TIDL_ResizeNearest))
  {
    /* Copy scales from input */
    /* For versions lower than 10, it is 2nd input */
    if (onnxOpSetVersion <= 10)
    {
      status = TIDL_tfCopyFloatConst(onnGraph, i, 1, buf);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize layer : Unable to find float intializer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    /* For any version upper, it is 3rd input */
    else
    {
      if ( node.input_size() <= 4 )
      {
        status = TIDL_tfCopyFloatConst(onnGraph, i, 2, buf);
        if(status != 0)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize layer : Unable to find float intializer");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize layer : More than 4 inputs are not supported for Resize layer");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }

    /* New Operator called antialias introduced. */
    int32_t antialias = 0;
    TIDL_onnxGetIntAttr(node, "antialias",   &antialias, 0);

    /* Antialiasing operation is not supported by TIDL_RT hence exit condition initiated. */
    if(antialias != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize layer : Antialiasing feature is not supported by TIDL_RT");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    /* New attribute keep_aspect_ratio_policy introduced. */
    char keep_aspect_ratio[50];
    int32_t status = TIDL_onnxGetStringAttr(node, "keep_aspect_ratio_policy", keep_aspect_ratio, 0);

    /* Keep_Aspect_Ratio only is required when interpreting the sizes input. So either reject the attribute if not provided or compare with the default value and proceed */
    if((status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) && (strcmp(keep_aspect_ratio,"stretch") != 0))
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize layer : The attribute keep_aspect_ratio_policy only required when sizes given as input. Size input is not supported by TIDL_RT");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    /* Interpret axes attibute */
    int32_t numDims;
    status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
    if(status != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
    if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
    {
      numDims = layer.allowlistingMetaData.varTensorsDims[0].size();
      numDims = (numDims == 0)? 4:numDims;
    }
    else
    {
      numDims = 4;
    }
    int32_t *axes = NULL;
    axes_idx = TIDL_onnxGetAttrIdx(node, "axes");
    if (axes_idx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      /* Read the axes values only if length of scales and axes are same*/
      if (node.attribute(axes_idx).ints_size() == buf.bufSize)
      {
        /* Read axes values */
        axes = (int32_t*)malloc (node.attribute(axes_idx).ints_size() * sizeof (int32_t));
        for (int32_t i = 0; i < node.attribute(axes_idx).ints_size(); i++)
        {
          TIDL_onnxGetIntAttr(node, "axes", &axes[i], i);
          /* Adjust axes[i] to TIDL dimension */
          if(axes[i] >= 0)
          {
            /*+ve Axis*/
            axes[i] += (TIDL_DIM_MAX - numDims);
          }
          else
          {
            /*-ve Axis*/
            axes[i] += TIDL_DIM_MAX;
          }
        }

        /* Read resize Ratios from scales according to axes provided */
        float32_tidl* resizeRatios = (float32_tidl*)buf.ptr;
        for (int32_t i = 0; i < node.attribute(axes_idx).ints_size(); i++)
        {
          layer.layerParams.resizeParams.resizeRatio[axes[i]] = resizeRatios[i];
        }
      }
      else
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize layer : Axes size & scales size mismatch");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    /* No axes, copy all scale params to layer params */
    else
    {
      float *ptr = (float *)buf.ptr;
      for (int i = buf.bufSize-1, j = 1; i >= 0, j <= TIDL_DIM_MAX; i--, j++)
      {
        layer.layerParams.resizeParams.resizeRatio[TIDL_DIM_MAX - j] = ptr[i];
      }
      my_free(ptr);
    }
  }
  return 0;
}


int32_t TIDL_onnxMapResizeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapResizeBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapAvgPoolBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  status = TIDL_checkPoolingTensorProperties(layer);
  if(status == -1) return -1;

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;

  std::vector<int32_t> attr;
  attr.push_back(TIDL_onnxGetIntAttr(onnGraph.node(i), "kernel_shape", &poolParams.kernelW, 0));
  attr.push_back(TIDL_onnxGetIntAttr(onnGraph.node(i), "kernel_shape", &poolParams.kernelH, 1));

  if(std::find(attr.begin(), attr.end(), -1) != attr.end())
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : AvgPool layer : Missing attribute");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  int32_t isGlobalAvgPool = 0;
  if(gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_ONNX_RT)
  {
    if((md.varTensorsDims[0][md.varTensorsDims[0].size()-2] == poolParams.kernelH) && (md.varTensorsDims[0][md.varTensorsDims[0].size()-1] == poolParams.kernelW)) //global avg pool - e.g. squeezenet1.1
    {
      isGlobalAvgPool = 1;
    }
  }
  if(isGlobalAvgPool)
  {
    status = TIDL_onnxMapGlobalAvgPoolBaseParams(onnGraph, i, layer);
  }
  else
  {
    status = TIDL_onnxMapMaxPoolBaseParams(onnGraph, i, layer);
    poolParams.poolingType = TIDL_AveragePooling;

    // Convert 1x1 average pooling into max pooling (operations are identical)
    // mxnet_resnet50_v1d
    if (poolParams.kernelH == 1 && poolParams.kernelW == 1 &&
        ((poolParams.strideH == 2 && poolParams.strideW == 2) ||
        (poolParams.strideH == 1 && poolParams.strideW == 1)))
    {
      poolParams.poolingType = TIDL_MaxPooling;
    }
  }

  return 0;
}

int32_t TIDL_onnxMapAvgPoolParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  TIDL_onnxMapMaxPoolParams(pOrgTIDLNetStructure, i, layerIndex, dataIndex, onnGraph);
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerParams.poolParams.poolingType = TIDL_AveragePooling;
  return 0;
}

int32_t TIDL_onnxMapGlobalAvgPoolBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  status = TIDL_checkPoolingTensorProperties(layer);
  if(status == -1) return -1;

  int32_t padType;
  int32_t idx1, idx2;
  int32_t pads[4];
  sTIDL_PoolingParams_t &poolParams = layer.layerParams.poolParams;


  if (layer.allowlistingMetaData.varTensorsDims[0].size() != 0 && layer.allowlistingMetaData.varTensorsDims[0].size() != 4 &&
      layer.allowlistingMetaData.varTensorsDims[0].size() > 2)
  {
    /** Global average pool is different for non-image done
     * Ex: NxCxHxW ( image ) -> Global average pool is done over dims H, W
     * Ex: NxCxD1 (non image) -> Global average pool is done over D1
     * Ex: NxCxD1XD2xD3 (non image) -> Global average pool is done over D1, D2, D3
    */

    poolParams.avgDims = layer.allowlistingMetaData.varTensorsDims[0].size() - 2;
  }
  else
  {
    poolParams.avgDims = 2;
  }

  layer.layerType = TIDL_PoolingLayer;
  poolParams.poolingType = TIDL_AveragePooling;
  poolParams.strideW = 1;
  poolParams.strideH = 1;
  poolParams.padW = 0;
  poolParams.padH = 0;
  poolParams.padT = 0;
  poolParams.padB = 0;
  poolParams.padL = 0;
  poolParams.padR = 0;

  poolParams.kernelW = 0;
  poolParams.kernelH = 0;
  return 0;
}

int32_t TIDL_onnxMapGlobalAvgPoolParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapGlobalAvgPoolBaseParams(onnGraph, i, TIDLPCLayers);

  return status;

}

int32_t TIDL_onnxMapReshapeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0, attrIdx = -1, allowzero = 0;
  layer.layerType = TIDL_ReshapeLayer;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /* check for variable shape input and return */
  if (md.numVarInputs > 1)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Reshape layer : More than 1 variable input, variable shape is not supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  status = TIDL_checkReshapeTensorProperties(layer);
  if(status == -1) return -1;
  status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, layer.weights);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  NodeProto node = onnGraph.node(i);

  attrIdx = TIDL_onnxGetIntAttr(node, "allowzero", &allowzero, 0);

  if (attrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL && allowzero == 1)
  {
    /** TIDL doesn't support allowzero attribute*/
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Reshape layer : Unsupported allow_zero attribute");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  return 0;
}

int32_t TIDL_onnxMapReshapeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapReshapeBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapTransposeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t attrIdx, numDims, status = 0;
  layer.layerType = TIDL_TransposeLayer;
  NodeProto node = onnGraph.node(i);
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /* calculate number of input dimesions */
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDims = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDims = (numDims == 0)? 4:numDims;
  }
  else
  {
    numDims = 4;
  }
  attrIdx = TIDL_onnxGetAttrIdx(node, "perm");
  if (attrIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    int32_t j;
    int32_t dims = node.attribute(attrIdx).ints_size();
    layer.weights.ptr = (void*)malloc(sizeof(int)*dims);
    layer.weights.bufSize = dims;
    int32_t *dimVlaue = (int*)layer.weights.ptr;
    for (j = 0; j < layer.weights.bufSize; j++)
    {
      dimVlaue[j] = node.attribute(attrIdx).ints(j);
    }
  }
  /* by default the perm is reverse of input */
  else
  {
    layer.weights.ptr = (void*)malloc(sizeof(int)*numDims);
    layer.weights.bufSize = numDims;
    int32_t *dimVlaue = (int*)layer.weights.ptr;
    for (int32_t j = 0; j < layer.weights.bufSize; j++)
    {
      dimVlaue[j] = (numDims - 1 - j);
    }
  }
  status = TIDL_checkTransposeTensorProperties(layer);
  return status;
}

int32_t TIDL_onnxMapTransposeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapTransposeBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapDepthToSpaceBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_DepthToSpaceLayer;
  NodeProto node = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "blocksize",   &layer.layerParams.depthToSpaceParams.blockSize, 0);
  return 0;
}

int32_t TIDL_onnxMapDepthToSpaceParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapDepthToSpaceBaseParams(onnGraph, i, TIDLPCLayers);
  return 0;
}

int32_t TIDL_onnxMapDropOutBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_DropOutLayer;
  return 0;
}

int32_t TIDL_onnxMapDropoutParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapDropOutBaseParams(onnGraph, i, TIDLPCLayers);
  return 0;
}

int32_t TIDL_onnxMapReduceMaxBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t numDim;
  int32_t axis, axisStatus = 0;
  int32_t status = 0;
  layer.layerType = TIDL_ReduceLayer;
  layer.numInBufs = onnGraph.node(i).input_size();
  NodeProto node  = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "keepdims",   &layer.layerParams.reduceParams.keepDims, 0);
  layer.layerParams.reduceParams.ops = 0;

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }

  layer.layerParams.reduceParams.ops = 0;  /* This value is reset if ReduceMax and set if ReduceMin. */

  /* axes attribute is there in lower than 18 opset */
  axisStatus = TIDL_onnxGetIntAttr(node, "axes",   &axis, 0);

  /* axis not found in attibute */
  if (axisStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    /* Attribute added in opset 18 */
    int32_t noop_with_empty_axes = 0;
    TIDL_onnxGetIntAttr(node, "noop_with_empty_axes", &noop_with_empty_axes, 0);

    TensorProto axes, inputTensor;
    /* Obtaining the axes input from the node. */
    if(md.numInputs > 1)
    {
      axes = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(1), i, status);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      /* Obtaining the input tensor from the node. */
      inputTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(0), i, status);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Resize : Unable to find initializer at index %d for node %s", 0, onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    else
    {
      /*Optional input not provided*/
      axes = onnx::TensorProto::default_instance();
    }

    if(axes.dims_size() > 0) /* Check whether axes input is given. */
    {
      /* Only 1 element in axes we take*/
      if (axes.dims(0) >= 1)
      {
        axis = axes.int32_data(0);
        if (axes.dims(0) > 1)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMax layer : TIDL_RT only supports 1 axis");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
    }
    else
    {
      /* If axes value is not given, and noop_with_empty_axes is true then perform no operation on the input tensor. Return it as such.*/
      if(noop_with_empty_axes)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMax layer : No operation to be performed on the input tensor");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      /* If noop_with_empty_axes is not true(not set) then reduce all the axes in the input tensor. */
      else
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMax layer : Reducing in all dimensions is not supported in TIDL-RT");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  layer.layerParams.reduceParams.axis = axis;
  /** Adjust axis to TIDL dimensions*/
  layer.layerParams.reduceParams.axis += (TIDL_DIM_MAX - numDim);

  return 0;
}

int32_t TIDL_onnxMapReduceMinBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t numDim;
  int32_t axis, axisStatus = 0;
  int32_t status;
  layer.layerType = TIDL_ReduceLayer;
  layer.numInBufs = onnGraph.node(i).input_size();
  NodeProto node  = onnGraph.node(i);
  TIDL_onnxGetIntAttr(node, "keepdims",   &layer.layerParams.reduceParams.keepDims, 0);
  layer.layerParams.reduceParams.ops = 1;

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim= 4;
  }

  layer.layerParams.reduceParams.ops = 1; /* This value is reset(=0) if ReduceMax and set(=1) if ReduceMin. */

  /* axes attribute is there in lower than 18 opset */
  axisStatus = TIDL_onnxGetIntAttr(node, "axes",   &axis, 0);

  /* axis not found in attibute */
  if(axisStatus == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {

    /* Attribute added in opset 18 */
    int32_t noop_with_empty_axes = 0;
    TIDL_onnxGetIntAttr(node, "noop_with_empty_axes", &noop_with_empty_axes, 0);

    TensorProto axes, inputTensor;
    /* Obtaining the axes input from the node. */\
    if(md.numInputs > 1)
    {
      axes = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(1), i, status);
      if(status != 0)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMin layer : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
    else
    {
      axes = onnx::TensorProto::default_instance();
    }
    /* Obtaining the input tensor from the node. */
    inputTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(0), i, status);
    if(status != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMin layer : Unable to find initializer at index %d for node %s", 0, onnGraph.node(i).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }

    if(axes.dims_size() > 0) /* Check whether axes input is given. */
    {
      for(int j = 0; j < axes.dims(0); j++) /* Axes is an optional 1D Tensor Input. */
      /* Only 1 element in axes we take*/
      if (axes.dims(0) >= 1)
      {
        axis = axes.int32_data(0);
        if (axes.dims(0) > 1)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMin layer : TIDL_RT only supports 1 axis");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
    }
    else
    {
      /* If axes value is not given, and noop_with_empty_axes is true then perform no operation on the input tensor. Return it as such.*/
      if(noop_with_empty_axes)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMin layer : No operation to be performed on the input tensor");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      /* If noop_with_empty_axes is not true(not set) then reduce all the axes in the input tensor. */
      else
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ReduceMin layer : Reducing in all dimensions is not supported in TIDL-RT");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  layer.layerParams.reduceParams.axis = axis;
  /** Adjust axis to TIDL dimensions*/
  layer.layerParams.reduceParams.axis += (TIDL_DIM_MAX - numDim);


  return 0;
}

int32_t TIDL_onnxMapReduceMaxParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapReduceMaxBaseParams(onnGraph, i, TIDLPCLayers);

  return 0;
}

int32_t TIDL_onnxMapReduceMinParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapReduceMinBaseParams(onnGraph, i, TIDLPCLayers);

  return 0;
}

int32_t TIDL_onnxMapScatterElementsBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t numDim;
  layer.layerType = TIDL_ScatterElementsLayer;
  layer.numInBufs = onnGraph.node(i).input_size();
  TIDL_onnxGetIntAttr(onnGraph.node(i), "axis",   &layer.layerParams.scatterElementsParams.axis, 0);

  int32_t status;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim= 4;
  }

  /** Adjust axis to TIDL dimensions*/
  layer.layerParams.scatterElementsParams.axis += (TIDL_DIM_MAX - numDim);

  strcpy((char*)layer.layerParams.scatterElementsParams.dataTensorName, (char*)onnGraph.node(i).input(0).c_str());
  strcpy((char*)layer.layerParams.scatterElementsParams.indicesTensorName, (char*)onnGraph.node(i).input(1).c_str());
  strcpy((char*)layer.layerParams.scatterElementsParams.updateTensorName, (char*)onnGraph.node(i).input(2).c_str());

  /* New attribute called reduction introduced. */
  char reduction[50];
  int32_t reduction_status = -1;

  /** The reduction attribute can have the following values:
   * "none". "add", "mul", "max", "min".
   * Only none value(also the default value) supported by TIDL_RT.
  */
  reduction_status = TIDL_onnxGetStringAttr(onnGraph.node(i), "reduction", reduction, 0);
  if( (reduction_status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) &&
      (strcmp(reduction, "none") != 0)
    )
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : ScatterElements layer : Reduction attribute value(%s) is not supported, only 'none' is supported by TIDL_RT", reduction);
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsNone;
  return 0;
}

int32_t TIDL_onnxMapScatterElementsParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapScatterElementsBaseParams(onnxGraph, i, TIDLPCLayers);
  return 0;
}


int32_t TIDL_onnxMapScatterNDBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0, status2 = 0;
  layer.layerType = TIDL_ScatterElementsLayer;
  layer.numInBufs = onnGraph.node(i).input_size();
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  if (md.numConstInputs == 1)
  {
    status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, md.constTensorIndices[0], layer.weights);
  }
  else if (md.numConstInputs == 2)
  {
    status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, md.constTensorIndices[0], layer.weights);
    status2 = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, md.constTensorIndices[1], layer.bias);
  }
  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED || status2 == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  layer.layerParams.scatterElementsParams.axis = -1;

  NodeProto node  = onnGraph.node(i);
  char reduction[32];
  int32_t reduction_status = TIDL_onnxGetStringAttr(node, "reduction", reduction, 0);
  /* No reduction attribute */
  if(reduction_status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsNone;
  }
  /* reduction must be one of none, add, min, max */
  else
  {
    if (strcmp("add", reduction) == 0)
    {
      layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsAdd;
    }
    else if(strcmp("max", reduction) == 0)
    {
      layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsMax;
    }
    else if(strcmp("min", reduction) == 0)
    {
      layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsMin;
    }
    else if(strcmp("none", reduction) == 0)
    {
      layer.layerParams.scatterElementsParams.reduction = TIDL_ScatterElementsNone;
    }
  }

  return status;
}

int32_t TIDL_onnxMapGatherBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t status =0, axis, numDim;
  layer.layerType = TIDL_GatherLayer;
  layer.numInBufs = onnGraph.node(i).input_size();

  TIDL_onnxGetIntAttr(onnGraph.node(i), "axis", &axis, 0);
  layer.layerParams.gatherParams.axis = axis;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  if(md.numConstInputs > 0)
  {
    int constTensorIdx = md.constTensorIndices[0];
    status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.weights);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    TensorProto indexTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(1), i, status);
    if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Gather layer : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    if (indexTensor.data_type() == TensorProto_DataType_INT64)
    {
      /**
       * TIDL_GatherLayer doesn't support int64 indices, hence convert them to int32
      */
      int32_t *index_dst_ptr = (int32_t*)malloc (sizeof(int32_t) * layer.weights.bufSize);

      if (index_dst_ptr == NULL)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "GatherLayer -- Node name %s : Could not allocate memory for gather constant indices",
        onnGraph.node(i).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }

      int64_t *index_src_ptr = (int64_t*) layer.weights.ptr;
      int64_t val = 0;

      for (int32_t j = 0; j < layer.weights.bufSize; j++)
      {
        val = index_src_ptr[j];
        index_dst_ptr[j] = (int32_t)val;
      }

      /* Update the buffer pointer*/
      free(index_src_ptr);
      layer.weights.ptr = index_dst_ptr;
    }
  }
  layer.inData[0].numDim = md.varTensorsDims[0].size();
  layer.inData[1].numDim = md.varTensorsDims[1].size();

  status = TIDL_checkGatherTensorProperties(layer);
  if (status == -1) return -1;
  return 0;
}

int32_t TIDL_onnxMapReduceMeanParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  sTIDL_LayerPC_t &TIDLPCLayers       = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  int32_t status = TIDL_onnxMapReduceMeanBaseParams(onnxGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapReduceMeanBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t numDim;
  layer.layerType = TIDL_ReduceMeanLayer;
  layer.numInBufs = onnGraph.node(i).input_size();
  NodeProto node  = onnGraph.node(i);
  int32_t axis = -1;
  int32_t status = TIDL_onnxGetIntAttr(node, "axes",   &layer.layerPCParams.reduceMeanParams.axis, 0);
  TIDL_onnxGetIntAttr(node, "keepdims", &layer.layerPCParams.reduceMeanParams.reduceDims, 0);

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }

  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    axis = -1;
  }
  /*Translate to TIDL Axis:*/
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDim);
  }
  else
  {
    /*-ve Axis*/
    axis = (TIDL_DIM_MAX + axis);
  }
  /** Adjust axis to TIDL dimensions*/
  layer.layerParams.reduceParams.axis = axis;
  return 0;
}

int32_t TIDL_onnxMapReduceSumParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  sTIDL_LayerPC_t &TIDLPCLayers       = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  int32_t status = TIDL_onnxMapReduceSumBaseParams(onnxGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapReduceSumBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  int32_t numDim;
  layer.layerType = TIDL_ReduceSumLayer;
  layer.numInBufs = onnGraph.node(i).input_size();
  NodeProto node  = onnGraph.node(i);
  int32_t axis = -1;
  int32_t status = TIDL_onnxGetIntAttr(node, "axes",   &layer.layerPCParams.reduceSumParams.axis, 0);
  TIDL_onnxGetIntAttr(node, "keepdims", &layer.layerPCParams.reduceSumParams.reduceDims, 0);

  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }

  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    axis = -1;
  }
  /*Translate to TIDL Axis:*/
  if(axis >= 0)
  {
    /*+ve Axis*/
    axis += (TIDL_DIM_MAX - numDim);
  }
  else
  {
    /*-ve Axis*/
    axis = (TIDL_DIM_MAX + axis);
  }
  /** Adjust axis to TIDL dimensions*/
  layer.layerParams.reduceParams.axis = axis;
  return 0;
}

int32_t TIDL_onnxMapPowParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  sTIDL_LayerPC_t &TIDLPCLayers       = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  int32_t status = TIDL_onnxMapPowBaseParams(onnxGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapPowBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{

  int32_t status =0, constSize = 0;
  layer.layerType = TIDL_PowLayer;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  /*Currently only supporting a single global pow() fxn*/
  if(md.numConstInputs > 0)
  {
    int constTensorIdx = md.constTensorIndices[0];
    TensorProto constTensor = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(constTensorIdx), i, status);
    if(status != 0)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Pow : Unable to find initializer at index %d for node %s", constTensorIdx, onnGraph.node(i).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    int32_t varTensorIdx = md.varTensorIndices[0];
    constSize = TIDL_getConstSize (constTensor);

    if(constSize == 1)
    {
        status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, constTensorIdx, layer.bias);
        if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Cannot read initializer tensor : Only float and int64 tensor is supported");
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        layer.layerPCParams.powParams.pow = *((float*)layer.bias.ptr);
    }
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Only global Pow() is supported currently -- Node: %s", onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

int32_t TIDL_onnxMapSqrtParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  sTIDL_LayerPC_t &TIDLPCLayers       = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  int32_t status = TIDL_onnxMapSqrtBaseParams(onnxGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapSqrtBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_SqrtLayer;
  return 0;
}

int32_t TIDL_onnxMapDequantizeLinearParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  sTIDL_LayerPC_t &TIDLPCLayers       = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  int32_t status = TIDL_onnxMapDequantizeLinearBaseParams(onnxGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapQuantizeLinearParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  sTIDL_LayerPC_t &TIDLPCLayers       = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  int32_t status = TIDL_onnxMapQuantizeLinearBaseParams(onnxGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_onnxMapDequantizeLinearBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{

  /*Dequantize nodes are not directly supported - they need to either be optimized out (with stats collected)
    or routed to DataConvert layers which implement it's functionality*/
  int32_t status = 0;
  layer.layerType = TIDL_DequantizeLayer;
  /*Map Dequantize nodes corresponding to activations: */
  TensorProto tensorScale       = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(1) , i, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Dequantize : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  TensorProto tensorZeroPoint   = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(2) , i, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Dequantize : Unable to find initializer at index %d for node %s", 2, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /*Update scale and zero point:*/
  float tZeroPoint = 0.0;
  if(TensorProto_DataType_INT8 == tensorZeroPoint.data_type())
  {
    bool tensorZPFromRaw = tensorZeroPoint.int32_data_size() == 0;
    int8_t tZP;
    if(tensorZPFromRaw)
    {
      tZP = ((int8_t* )(tensorZeroPoint.raw_data().c_str()))[0];
    }
    else
    {
      tZP = (int8_t )(tensorZeroPoint.int32_data(0));
    }
    tZeroPoint = (float)tZP;
    layer.layerPCParams.quantizeParams.elementType = TIDL_SignedChar;
  }
  else
  {
    bool tensorZPFromRaw = tensorZeroPoint.int32_data_size() == 0;
    uint8_t tZP;
    if(tensorZPFromRaw)
    {
      tZP = ((uint8_t* )(tensorZeroPoint.raw_data().c_str()))[0];
    }
    else
    {
      tZP = (uint8_t )(tensorZeroPoint.int32_data(0));
    }
    tZeroPoint = (float)tZP;
    layer.layerPCParams.quantizeParams.elementType = TIDL_UnsignedChar;
  }
  float  tScale;
  bool tensorScaleFromRaw = tensorScale.float_data_size() == 0;
  if(tensorScaleFromRaw)
  {
    tScale = ((float* )(tensorScale.raw_data().c_str()))[0];
  }
  else
  {
    tScale = (float )(tensorScale.float_data(0));
  }
  layer.layerPCParams.dequantParams.scale = tScale;
  layer.layerPCParams.dequantParams.zeroPoint = tZeroPoint;
  layer.outData[0].tensorScale = 1/tScale;
  layer.outData[0].tensorZeroPoint = tZeroPoint;
  /*OSRT Metadata:*/
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  layer.numInBufs = md.numVarInputs;

  return 0;
}


int32_t TIDL_onnxMapQuantizeLinearBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  /*Dequantize nodes are not directly supported - they need to either be optimized out (with stats collected)
    or routed to DataConvert layers which implement it's functionality*/
  int32_t status = 0;
  layer.layerType = TIDL_QuantizeLayer;
  /*Map Dequantize nodes corresponding to activations: */
  TensorProto tensorScale       = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(1) , i, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Quantize : Unable to find initializer at index %d for node %s", 1, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  TensorProto tensorZeroPoint   = TIDL_getInitializerTensor(onnGraph, onnGraph.node(i).input(2) , i, status);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Quantize : Unable to find initializer at index %d for node %s", 2, onnGraph.node(i).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  /*Update scale and zero point:*/
  float tZeroPoint = 0.0;
  if(TensorProto_DataType_INT8 == tensorZeroPoint.data_type())
  {
    bool tensorZPFromRaw = tensorZeroPoint.int32_data_size() == 0;
    int8_t tZP;
    if(tensorZPFromRaw)
    {
      tZP = ((int8_t* )(tensorZeroPoint.raw_data().c_str()))[0];
    }
    else
    {
      tZP = (int8_t )(tensorZeroPoint.int32_data(0));
    }
    tZeroPoint = (float)tZP;
    layer.layerPCParams.quantizeParams.elementType = TIDL_SignedChar;
  }
  else
  {
    bool tensorZPFromRaw = tensorZeroPoint.int32_data_size() == 0;
    uint8_t tZP;
    if(tensorZPFromRaw)
    {
      tZP = ((uint8_t* )(tensorZeroPoint.raw_data().c_str()))[0];
    }
    else
    {
      tZP = (uint8_t )(tensorZeroPoint.int32_data(0));
    }
    tZeroPoint = (float)tZP;
    layer.layerPCParams.quantizeParams.elementType = TIDL_UnsignedChar;
  }

  float  tScale;
  bool tensorScaleFromRaw = tensorScale.float_data_size() == 0;
  if(tensorScaleFromRaw)
  {
    tScale = ((float* )(tensorScale.raw_data().c_str()))[0];
  }
  else
  {
    tScale = (float )(tensorScale.float_data(0));
  }


  layer.layerPCParams.quantizeParams.scale = tScale;
  layer.layerPCParams.quantizeParams.zeroPoint = tZeroPoint;

  /*OSRT Metadata:*/
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;
  layer.numInBufs = md.numVarInputs;

  return 0;
}

int32_t TIDL_onnxMapGatherParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapGatherBaseParams(onnxGraph, i, TIDLPCLayers);
  /*Need to put some error checks on the input and weight dimension*/
  //@TODO
  return 0;
}

int32_t TIDL_onnxMapScatterNDParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnxGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  TIDLPCLayers.outData[0].numDim = 4;
  status = TIDL_onnxMapScatterNDBaseParams(onnxGraph, i, TIDLPCLayers);
  /*Need to put some error checks on the input and weight dimension*/
  //@TODO
  return 0;
}


int32_t TIDL_onnxMapSqueezeBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  /** Num inputs for squeeze is 2 from onnx opset 13 (includes axes as an input), but it's an initializer actually, hence hard coding numInputs to 1*/
  layer.layerType = TIDL_SqueezeLayer;
  layer.numInBufs = 1;
  NodeProto node  = onnGraph.node(i);

  int32_t axes_status = -1, axesIdx = -1;
  int32_t j, ii, numDim = 0;
  int32_t num_axes_4_squeeze = -1;
  int32_t axes[TIDL_DIM_MAX];

  int32_t status;
  status = TIDL_onnxSaveAllowlistingMetaData(onnGraph, i, layer);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Issue in generating allowlisting meta data");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  sTIDL_allowlistingMetaData md = layer.allowlistingMetaData;

  axesIdx = TIDL_onnxGetAttrIdx(node, "axes");
  if (axesIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    axes_status = axesIdx;
    num_axes_4_squeeze = node.attribute(axesIdx).ints_size();
    for(ii = 0; ii< num_axes_4_squeeze; ii++)
    {
      TIDL_onnxGetIntAttr(node, "axes",   &axes[ii], ii);
    }
  }
  else if(md.numVarInputs <= 1)
  {
    /* axes can be an input in opset 18*/
    sBuffer_t buf;
    axes_status = TIDL_tfCopyFloatInitializerTensor(onnGraph, i, 1, buf);
    if (axes_status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      num_axes_4_squeeze = buf.bufSize;
      memcpy (axes, buf.ptr, sizeof(int32_t) * num_axes_4_squeeze);
    }
    free (buf.ptr);
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : Variable input for axes is not supported");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  if (axes_status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Squeeze layer : No axis given for Squeeze");
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /** If shape inference is not done on model, we assume the tensor is of 4 dimensions by default*/
  if (layer.allowlistingMetaData.varTensorsDims.size() != 0)
  {
    numDim = layer.allowlistingMetaData.varTensorsDims[0].size();
    numDim = (numDim == 0)? 4:numDim;
  }
  else
  {
    numDim = 4;
  }

  /* Updating the axes to TIDL max dimensions*/
  for (ii = 0; ii < num_axes_4_squeeze; ii++)
  {
    axes[ii] = axes[ii] + (TIDL_DIM_MAX - numDim);
  }

  for(ii = 0; ii< TIDL_DIM_MAX; ii++)
  {
    layer.layerPCParams.squeezeParams.axis[ii] = 0;
  }

  for(ii = 0; ii< TIDL_DIM_MAX; ii++)
  {
    for(j=0;j<num_axes_4_squeeze;j++)
    {
      if(ii == axes[j])
      {
        layer.layerPCParams.squeezeParams.axis[ii] = 1; // squeeze this particular axis
      }
    }
  }

  return 0;
}
int32_t TIDL_onnxMapSqueezeParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph )
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapSqueezeBaseParams(onnGraph, i, TIDLPCLayers);

  return 0;
}
int32_t TIDL_onnxMapConstParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_ConstDataLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs = 0;
  return 0;
}


int32_t TIDL_onnxMapIdentityBaseParams(GraphProto&   onnGraph, int32_t i, sTIDL_LayerPC_t &layer)
{
  layer.layerType = TIDL_IdentityLayer;
  /* if layer type identity, deny if there is a input node */
  if (strcmp(onnGraph.node(i).op_type().c_str(), "Identity") == 0)
  {
    bool hasInputNode = false;
    for (int32_t idx = 0; idx < onnGraph.node_size(); idx++)
    {
      for (int32_t outIdx = 0; outIdx < onnGraph.node(idx).output_size(); outIdx++)
      {
        if (strcmp(onnGraph.node(i).input(0).c_str(), onnGraph.node(idx).output(outIdx).c_str()) == 0)
        {
          hasInputNode = true;
        }
      }
    }
    if (hasInputNode)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Identity layer with a input node is not supported, only Identity layer at graph input is supported");
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
  return 0;
}

int32_t TIDL_onnxMapIdentityParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  int32_t status;
  sTIDL_LayerPC_t &TIDLPCLayers = pOrgTIDLNetStructure->TIDLPCLayers[layerIndex];
  TIDLPCLayers.outData[0].dataId = (*dataIndex)++;
  status = TIDL_onnxMapIdentityBaseParams(onnGraph, i, TIDLPCLayers);
  return status;
}

int32_t TIDL_tfMapUnSuportedlayerParams(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  int32_t              *dataIndex,
  GraphProto&            onnGraph)
{
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].layerType = TIDL_UnsupportedLayer;
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outData[0].dataId = (*dataIndex)++;
  return 0;
}

typedef struct {
  int8_t name[TIDL_STRING_SIZE];
  int32_t(*tidl_onnxMapFunc)(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
    int32_t              i,
    int32_t              layerIndex,
    int32_t              *dataIndex,
    GraphProto&             onnxGraph);
}sTIDL_onnxOpParamMap_t;

sTIDL_onnxOpParamMap_t tidl_onnxOpParamMapTable[] =
{
  { "Conv",                          TIDL_onnxMapConvParams },
  { "MaxPool",                       TIDL_onnxMapMaxPoolParams },
  { "Relu",                          TIDL_onnxMapReluParams },
  { "Clip",                          TIDL_onnxMapClipParams },
  { "PRelu",                         TIDL_onnxMapPReluParams },
  { "Sigmoid",                       TIDL_onnxMapSigmoidParams },
  { "Tanh",                          TIDL_onnxMapTanhParams },
  { "HardSigmoid",                   TIDL_onnxMapHardSigmoidParams },
  { "Elu",                           TIDL_onnxMapELUParams },
  { "Logistic",                      TIDL_onnxMapSigmoidParams },
  { "LeakyRelu",                     TIDL_onnxMapLeakyReluParams },
  { "Concat",                        TIDL_onnxMapConcatParams },
  { "AveragePool",                   TIDL_onnxMapAvgPoolParams },
  { "GlobalAveragePool",             TIDL_onnxMapGlobalAvgPoolParams },
  { "Upsample",                      TIDL_onnxMapUpsampleParams },
  { "Resize",                        TIDL_onnxMapResizeParams },
  { "ConvTranspose",                 TIDL_onnxMapConvTransposeParams },
  { "Reshape",                       TIDL_onnxMapReshapeParams },
  { "Transpose",                     TIDL_onnxMapTransposeParams },
  { "Add",                           TIDL_onnxMapAddParams },
  { "Sum",                           TIDL_onnxMapAddParams },
  { "Sub",                           TIDL_onnxMapSubParams },
  { "Max",                           TIDL_onnxMapMaxParams }, 
  { "ArgOp",                         TIDL_onnxMapArgOpParams },
  { "BatchNormalization",            TIDL_onnxMapBNParams },
  { "Gemm",                          TIDL_onnxGemmParams },
  { "MatMul",                        TIDL_onnxGemmParams },
  { "Softmax",                       TIDL_onnxSoftmaxParams },
  { "Split",                         TIDL_onnxSplitParams },
  { "Slice",                         TIDL_onnxSliceParams },
  { "Flatten",                       TIDL_onnxMapFlattenParams },
  { "Pad",                           TIDL_onnxMapPadParams },
  { "Mul",                           TIDL_onnxMapMulParams },
  { "DepthToSpace",                  TIDL_onnxMapDepthToSpaceParams },
  { "ReduceMax",                     TIDL_onnxMapReduceMaxParams },
  { "ReduceMin",                     TIDL_onnxMapReduceMinParams },
  { "ScatterElements",               TIDL_onnxMapScatterElementsParams },
  { "Squeeze",                       TIDL_onnxMapSqueezeParams },
  { "Dropout",                       TIDL_onnxMapDropoutParams },
  { "ScatterND",                     TIDL_onnxMapScatterNDParams},
  { "Cast",                          TIDL_onnxMapIdentityParams },
  { "Gather",                        TIDL_onnxMapGatherParams },
  { "DequantizeLinear",              TIDL_onnxMapDequantizeLinearParams },
  { "QuantizeLinear",                TIDL_onnxMapQuantizeLinearParams },
  { "Erf",                           TIDL_onnxMapIdentityParams},
  { "ReduceMean",                    TIDL_onnxMapReduceMeanParams},
  { "ReduceSum",                     TIDL_onnxMapReduceSumParams},
  { "Pow",                           TIDL_onnxMapPowParams},
  { "Sqrt",                          TIDL_onnxMapSqrtParams},
  { "Div",                           TIDL_onnxMapDivParams},
  { "LayerNormalization",            TIDL_onnxMapLayerNormParams},
  { "RMSNormalization",            TIDL_onnxMapRMSNormParams},
  { "Softplus",                      TIDL_onnxMapSoftplusParams },
  { "Softsign",                      TIDL_onnxMapSoftsignParams }
};
int32_t TIDL_getOnnxOpParamMapId(const char  * name)
{
  int32_t i = -1;
  for (i = 0; i < sizeof(tidl_onnxOpParamMapTable) / sizeof(sTIDL_onnxOpParamMap_t); i++)
  {
    if ((strcmp(name, (const char *)tidl_onnxOpParamMapTable[i].name) == 0))
    {
      return (i);
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
}

TIDL_TFLayerMapping_t TIDL_OnnxLayerMap[] =
{
  { (char*)"TIDL_MatMulBias",         (char*)"TIDL_InnerProductLayerTIDL_EltWiseLayer", 2},
  { (char*)"TIDL_OnnxShuffle",        (char*)"TIDL_ReshapeLayerTIDL_TransposeLayerTIDL_ReshapeLayer", 3},
  { (char*)"TIDL_TFSlimShuffle",        (char*)"NANA"              , 3 },
  { (char*)"TIDL_LayernormGamma",     (char*)"TIDL_LayerNormLayerTIDL_EltWiseLayerTIDL_EltWiseLayer", 3},
  { (char*)"TIDL_PatchEmbedding",      (char*)"TIDL_ConvolutionLayer",  1}
};

bool tidl_is1DBroadCastTensor(sTIDL_LayerPC_t &layer)
{
  bool status = true;
  int32_t i;
  /*Const idx:*/
  if (layer.allowlistingMetaData.numConstInputs == 1)
  {
    int32_t bufIdx = layer.allowlistingMetaData.constTensorIndices[0];
    for(i = TIDL_DIM_HEIGHT; i >= 0; i--)
    {
      if(i != TIDL_DIM_NUMCH)
      {
        if(layer.inData[bufIdx].dimValues[i] != 1)
        {
          status = false;
          break;
        }
      }
    }
  }
  else
  {
    status = false;
  }

  return status;
}

void tidl_updateOutConsumerCount(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, sTIDL_LayerPC_t &TIDLLayer, int32_t layerIndex)
{
  int32_t numConsumer = 0;
  int32_t i0, i1;
  //TIDLLayer.outDataNames[0]
  for(i0 = 0; i0 < layerIndex; i0++)
  {
    sTIDL_LayerPC_t &curLayer = pOrgTIDLNetStructure.TIDLPCLayers[i0];
    for(i1 = 0; i1 < curLayer.numInBufs; i1++)
    {
       if (strcmp((const char *)TIDLLayer.outDataNames[0], (const char *)curLayer.inDataNames[i1]) == 0)
       {
         numConsumer++;
       }
    }
  }
  TIDLLayer.outConsumerCnt[0] = numConsumer;
}

int32_t tidl_fuseLayerNormBetaGamma(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex)
{
  /*This function attempts to fuse the beta & gamma terms into a projection in front of the layernorm,
    if a projection is not found, it will introduce an innerproduct layer instead (Identity matrix mutated)*/
  int32_t i1;
  int32_t status = 0;
  int32_t outLayerIdx = -1;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
     if (tf_isLayerType("TIDL_LayernormGamma", i1, pOrgTIDLNetStructure, TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
     {
        int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_LayernormGamma", TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t)));
        sTIDL_LayerPC_t &TIDLLayerNormLayer = pOrgTIDLNetStructure.TIDLPCLayers[i1];
        if(TIDLLayerNormLayer.layerParams.layerNormParams.isInstanceNorm == 1) continue;
        /*Update layernorm output consumer count:*/
        tidl_updateOutConsumerCount(pOrgTIDLNetStructure, TIDLLayerNormLayer, layerIndex);
        int32_t eltwiseMulLayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
        if(eltwiseMulLayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        sTIDL_LayerPC_t &eltwiseMulLayer = pOrgTIDLNetStructure.TIDLPCLayers[eltwiseMulLayerIdx];
        int32_t eltwiseAddLayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[eltwiseMulLayerIdx].outData[0].dataId);
        if(eltwiseAddLayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        sTIDL_LayerPC_t &eltwiseAddLayer = pOrgTIDLNetStructure.TIDLPCLayers[eltwiseAddLayerIdx];
        if(!(eltwiseMulLayer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseProduct && eltwiseAddLayer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum && tidl_is1DBroadCastTensor(eltwiseMulLayer) && tidl_is1DBroadCastTensor(eltwiseAddLayer)))
        {
          /*Non-Fusable pattern*/
          continue;
        }
        else if((TIDLLayerNormLayer.layerParams.layerNormParams.axis != TIDL_DIM_WIDTH) || (TIDLLayerNormLayer.outConsumerCnt[0] > 1))
        {
          /*Only widthwise layernorm with a single output branch is currently fused/optimized*/
          continue;
        }
        else
        {
          /*Layernorm Beta and Gamma Fusion*/
          std::vector<int32_t> layerNormConsumerIdxs = tidl_getOutLayers(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[eltwiseAddLayerIdx].outData[0].dataId);
          if(layerNormConsumerIdxs.size() == 0) 
          {
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
          /*Case one: MatMul exists ahead (Projection)*/
          /*Weights (Part of const layer) are first scaled (Per column)*/
          /*Get the const weight buffer (Gamma) from EltwiseMul [It has been already checked that this exists]*/
          int32_t gammaIdx = tidl_getInLayer (pOrgTIDLNetStructure, eltwiseMulLayerIdx, eltwiseMulLayer.inData[eltwiseMulLayer.allowlistingMetaData.constTensorIndices[0]].dataId);
          if(gammaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
          {
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
          float* pGamma = (float*) pOrgTIDLNetStructure.TIDLPCLayers[gammaIdx].weights.ptr;
          /*Get Beta*/
          int32_t betaIdx = tidl_getInLayer (pOrgTIDLNetStructure, eltwiseAddLayerIdx, eltwiseAddLayer.inData[eltwiseAddLayer.allowlistingMetaData.constTensorIndices[0]].dataId);
          if(betaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
          {
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
          float* pBeta = (float*) pOrgTIDLNetStructure.TIDLPCLayers[betaIdx].weights.ptr;
          if(layerNormConsumerIdxs.size() == 1 && (eltwiseAddLayer.outConsumerCnt[0] == 1U) && pOrgTIDLNetStructure.TIDLPCLayers[layerNormConsumerIdxs[0]].layerParams.innerProductParams.constIdx == 1U && pOrgTIDLNetStructure.TIDLPCLayers[layerNormConsumerIdxs[0]].layerParams.innerProductParams.numBChannels == 1 && pOrgTIDLNetStructure.TIDLPCLayers[layerNormConsumerIdxs[0]].layerType == TIDL_InnerProductLayer)
          {
            int32_t layerNormConsumerIdx = layerNormConsumerIdxs[0];
            sTIDL_LayerPC_t &layerNormConsumer = pOrgTIDLNetStructure.TIDLPCLayers[layerNormConsumerIdx];

            int32_t matmulWeightsIdx = tidl_getInLayer (pOrgTIDLNetStructure, layerNormConsumerIdx, layerNormConsumer.inData[1].dataId);
            if(matmulWeightsIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            assert(pOrgTIDLNetStructure.TIDLPCLayers[matmulWeightsIdx].layerType == TIDL_ConstDataLayer);
            /*FC Weights & biases*/
            float* pWeights = (float*) pOrgTIDLNetStructure.TIDLPCLayers[matmulWeightsIdx].weights.ptr;
            float* pBias = (float*) layerNormConsumer.bias.ptr;
            //assert(layerNormConsumer.bias.bufSize == layerNormConsumer.layerParams.innerProductParams.numOutCols);
            int32_t i2, i3;
            int32_t numInCols = layerNormConsumer.layerParams.innerProductParams.numInCols;
            int32_t numOutCols = layerNormConsumer.layerParams.innerProductParams.numOutCols;
            if(layerNormConsumer.layerParams.innerProductParams.isBias == 0)
            {
              /*Enable Bias for Innerproduct layer ahead:*/
              layerNormConsumer.layerParams.innerProductParams.isBias = 1;
              layerNormConsumer.bias.bufSize = numOutCols;
              layerNormConsumer.bias.ptr = (float*) malloc(layerNormConsumer.bias.bufSize * sizeof(float));
              pBias = (float*) layerNormConsumer.bias.ptr;
              for(i2 = 0; i2 < layerNormConsumer.bias.bufSize; i2++)
              {
                pBias[i2] = 0;
              }
            }
            /*Update bias by adding broadCastAdd x W*/
            for(i3 = 0; i3 < numOutCols; i3++)
            {
              for(i2 = 0; i2 < numInCols; i2++)
              {
                pBias[i3] += pWeights[i2*numOutCols + i3] * pBeta[i2];
              }
            }
            free(pBeta);
            /*Weights are numInCols X numOutCols & Gamme is numOutCols wide*/
            for(i2 = 0; i2 < numInCols; i2++)
            {
              for(i3 = 0; i3 < numOutCols; i3++)
              {
                pWeights[i2*numOutCols + i3] *= pGamma[i2];
              }
            }
            free(pGamma);
            /*Update link from Layernorm -> Layernormconsumer*/
            layerNormConsumer.inData[0] = TIDLLayerNormLayer.outData[0];
            strcpy((char*)layerNormConsumer.inDataNames[0],(const char*)TIDLLayerNormLayer.outDataNames[0]);
          }
          else
          {
            /*Replace Gamma (eltwise mul) & Beta (eltwise add) with an innerproduct layer:*/
            /*Add an innerproduct layer:*/
            char tempName[512];
            strcpy(tempName, (const char*)TIDLLayerNormLayer.outDataNames[0]);

            TIDL_IMPORT_CHECK_AND_RETURN(TIDL_addLayer(pOrgTIDLNetStructure, i1, TIDL_InnerProductLayer, dataIndex, ADD_LAYER_AFTER), "");
            sTIDL_LayerPC_t& ip       = pOrgTIDLNetStructure.TIDLPCLayers[pOrgTIDLNetStructure.numLayers-1];

            /**
             * Setup Inner Product Layer
            */
            int32_t numInCols = TIDLLayerNormLayer.outData[0].dimValues[TIDL_DIM_WIDTH];
            int32_t numBChannels = pOrgTIDLNetStructure.TIDLPCLayers[gammaIdx].outData[0].dimValues[TIDL_DIM_NUMCH];
            ip.numInBufs = 2;
            ip.layerParams.innerProductParams.numInRows  = TIDLLayerNormLayer.outData[0].dimValues[TIDL_DIM_HEIGHT];
            ip.layerParams.innerProductParams.numOutCols = numInCols;
            ip.layerParams.innerProductParams.numInCols  = numInCols;
            ip.layerParams.innerProductParams.numBChannels = 1;
            ip.layerParams.innerProductParams.isBias     = 1;
            ip.layerParams.innerProductParams.numBChannels = numBChannels;
            ip.layerParams.innerProductParams.inputBTranspose = 0;
            ip.layerParams.innerProductParams.constIdx = 1; /*"B" is the constant buffer*/
            ip.allowlistingMetaData.varTensorIndices = {0};
            ip.allowlistingMetaData.numConstInputs = 1;
            ip.allowlistingMetaData.constTensorIndices = {1};
            int32_t bufSize = numInCols * numInCols * numBChannels;
            int32_t biasSize = numInCols * numBChannels; /*numOutCols == numInCols*/
            float *weightsPtr = (float*)malloc(sizeof(float)*bufSize);

            int32_t i1, i2, i3;
            for(i1 = 0; i1 < numBChannels; i1++)
            {
              for(i2 = 0; i2 < numInCols; i2++)
              {
                for(i3 = 0; i3 < numInCols; i3++)
                {
                  if(i2 == i3)
                  {
                    /*Diagonal elements are populated:*/
                    weightsPtr[i1 * (numInCols * numInCols) + i2*numInCols + i3] = pGamma[i2 + i1 * numInCols];
                  }
                  else
                  {
                    weightsPtr[i1 * (numInCols * numInCols) + i2*numInCols + i3] = 0.0;
                  }
                }
              }
            }

            free(pGamma);
            /*Create a const buffer for weights:*/
            int32_t constIdx = tidl_createConstDataLayer (pOrgTIDLNetStructure, dataIndex, weightsPtr, bufSize, pOrgTIDLNetStructure.numLayers);
            if(constIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            sTIDL_LayerPC_t& constLayer = pOrgTIDLNetStructure.TIDLPCLayers[constIdx];
            constLayer.outData[0].dimValues[TIDL_DIM_BATCH]  = 1;
            constLayer.outData[0].dimValues[TIDL_DIM_DIM1]   = 1;
            constLayer.outData[0].dimValues[TIDL_DIM_DIM2]   = 1;
            constLayer.outData[0].dimValues[TIDL_DIM_NUMCH]  = numBChannels;
            constLayer.outData[0].dimValues[TIDL_DIM_HEIGHT] = numInCols;
            constLayer.outData[0].dimValues[TIDL_DIM_WIDTH]  = numInCols;
            /*Populate the bias buffer:*/
            ip.bias.bufSize = biasSize;
            float * pBias = (float*) malloc(sizeof(float)*biasSize);
            ip.bias.ptr = (void*) pBias;
            /*Populate bias:*/
            if(ip.bias.ptr != NULL)
            {
              for(i1 = 0; i1 < numBChannels; i1++)
              {
                for(i2 = 0; i2 < numInCols; i2++)
                {
                  pBias[i2 + i1*numInCols] = pBeta[i2 + i1*numInCols];
                }
              }
              free(pBeta);
            }
            else
            {
              TIDL_GLOBAL_REPORT_ERROR("Error populating layernorm innerproduct bias. Aborting");
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            /*Create network links*/
            /*Populate const input for innerproduct:*/
            ip.inData[1] = constLayer.outData[0];
            strcpy((char*)ip.inDataNames[1], (char*)constLayer.outDataNames[0]);
            /*Populate the regular input to innerproduct:*/
            ip.inData[0] = TIDLLayerNormLayer.outData[0];
            strcpy((char*)TIDLLayerNormLayer.outDataNames[0], tempName);
            strcpy((char*)ip.inDataNames[0], (char*)TIDLLayerNormLayer.outDataNames[0]);
            /*Link output*/
            ip.outData[0] = eltwiseAddLayer.outData[0];
            strcpy((char*)ip.outDataNames[0], (char*)eltwiseAddLayer.outDataNames[0]);
            ip.outConsumerCnt[0] = eltwiseAddLayer.outConsumerCnt[0];

            /*Inherit activation properties from the final add layer:*/
            ip.actParams = eltwiseAddLayer.actParams;

            if(gParams.preQuantizedModel && eltwiseAddLayer.actParams.actType == TIDL_Clip)
            {
              ip.actParams = eltwiseAddLayer.actParams;
            }
          }
          /*Remove eltwise add and eltwise mul layers (& corresponding const buffers)*/
          eltwiseAddLayer.numInBufs  = -1;
          eltwiseAddLayer.numOutBufs = -1;
          eltwiseMulLayer.numInBufs  = -1;
          eltwiseMulLayer.numOutBufs = -1;
          /*Eltwise Add const:*/
          pOrgTIDLNetStructure.TIDLPCLayers[betaIdx].numInBufs  = -1;
          pOrgTIDLNetStructure.TIDLPCLayers[betaIdx].numOutBufs = -1;
          /*Eltwise Mul Const:*/
          pOrgTIDLNetStructure.TIDLPCLayers[gammaIdx].numInBufs  = -1;
          pOrgTIDLNetStructure.TIDLPCLayers[gammaIdx].numOutBufs = -1;
        }

     }

  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_FindMatMulBias (sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t i1, i2, i3, i4;
  int32_t status = 0;
  int32_t outLayerIdx = -1;

  /**
   * Do not fuse bias for TDA4VM
  */
  if (gParams.deviceName != TIDL_TDA4VM)
  {
    for (i1 = 0; i1 < layerIndex; i1++)
    {
      if (tf_isLayerType("TIDL_MatMulBias", i1, pOrgTIDLNetStructure, TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
      {
        int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_MatMulBias", TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t)));

        sTIDL_LayerPC_t &TIDLPCLayers1 = pOrgTIDLNetStructure.TIDLPCLayers[i1];
        int32_t nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
        if(nextlayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
        sTIDL_LayerPC_t &TIDLPCLayers2 = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];

        /** Check if the eltwise is an add layer and identify if it is a bias to the innerproduct layer*/
        /** Do not merge if  activation type is Relu due to kernel bug */
        if (TIDLPCLayers2.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum &&
            TIDLPCLayers2.allowlistingMetaData.numConstInputs == 1 &&
            (TIDLPCLayers2.allowlistingMetaData.constTensorsDims[0].size() == 1 || TIDLPCLayers2.allowlistingMetaData.constTensorsDims[0].size() == 3)&&
            TIDLPCLayers1.outData[0].dimValues[TIDL_DIM_DIM2] == 1  &&
            TIDLPCLayers1.outData[0].dimValues[TIDL_DIM_DIM1] == 1 )
        {
          int32_t numCh = 1;
          int32_t biasPerCh;
          if (TIDLPCLayers2.allowlistingMetaData.constTensorsDims[0].size() == 1)
          {
            biasPerCh = TIDLPCLayers2.allowlistingMetaData.constTensorsDims[0][0];
          }
          else
          {
            numCh = TIDLPCLayers2.allowlistingMetaData.constTensorsDims[0][0];
            biasPerCh = TIDLPCLayers2.allowlistingMetaData.constTensorsDims[0][2];
          }
          if (biasPerCh == TIDLPCLayers1.layerParams.innerProductParams.numInRows ||
              biasPerCh == TIDLPCLayers1.layerParams.innerProductParams.numOutCols &&
              (TIDL_doesLayerSupportAsymTensors(&TIDLPCLayers1)))
          {
            int32_t constIdx = tidl_getInLayer (pOrgTIDLNetStructure, layerIndex, TIDLPCLayers2.inData[0].dataId);
            if(constIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
            {
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
            if (pOrgTIDLNetStructure.TIDLPCLayers[constIdx].layerType != TIDL_ConstDataLayer)
            {
              constIdx = tidl_getInLayer (pOrgTIDLNetStructure, layerIndex, TIDLPCLayers2.inData[1].dataId);
              if(constIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
              {
                return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
              }
            }
            float* bias = (float*)malloc (sizeof (float) * TIDLPCLayers1.layerParams.innerProductParams.numBChannels * TIDLPCLayers1.layerParams.innerProductParams.numOutCols);
            float* src_bias = (float*) pOrgTIDLNetStructure.TIDLPCLayers[constIdx].weights.ptr;
            if (((TIDLPCLayers1.layerParams.innerProductParams.numOutCols * TIDLPCLayers1.layerParams.innerProductParams.numBChannels)  == pOrgTIDLNetStructure.TIDLPCLayers[constIdx].weights.bufSize) && (pOrgTIDLNetStructure.TIDLPCLayers[constIdx].weights.bufSize > 0U))
            {
              int32_t offset = 0;
              if (biasPerCh == TIDLPCLayers1.layerParams.innerProductParams.numOutCols)
              {
                /** Only update bias for the exact size */
                for (int32_t i = 0; i < TIDLPCLayers1.layerParams.innerProductParams.numBChannels * TIDLPCLayers1.layerParams.innerProductParams.numOutCols; i++)
                {
                  bias[i] = src_bias[i];
                }
                free (src_bias);
              }
              else if (biasPerCh == TIDLPCLayers1.layerParams.innerProductParams.numInRows)
              {
                /** Broadcast the bias row wise*/
                for (int32_t i = 0; i < TIDLPCLayers1.layerParams.innerProductParams.numInRows; i++)
                {
                  for (int32_t j = 0; j < TIDLPCLayers1.layerParams.innerProductParams.numOutCols; j++)
                  {
                    bias[offset++] = src_bias[i];
                  }
                }
                free (src_bias);
              }
              TIDLPCLayers1.layerParams.innerProductParams.isBias = 1;
              TIDLPCLayers1.bias.ptr = bias;
              TIDLPCLayers1.bias.bufSize = TIDLPCLayers1.layerParams.innerProductParams.numInRows * TIDLPCLayers1.layerParams.innerProductParams.numOutCols;
              TIDLPCLayers1.actParams = TIDLPCLayers2.actParams;
              if((TIDLPCLayers1.actParams.actType == TIDL_RelU6) || (TIDLPCLayers1.actParams.actType == TIDL_RelU)||
                ((TIDLPCLayers1.actParams.actType == TIDL_Clip) && (TIDLPCLayers1.actParams.clipMin >= 0)))
              {
                TIDLPCLayers1.outData[0].elementType = tidl_getElementType(0);
              }
              outLayerIdx = tidl_getOutLayer (pOrgTIDLNetStructure, layerIndex, TIDLPCLayers2.outData[0].dataId);
              if(outLayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
              {
                return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
              }
              /** Remove the Eltwise & ConstData layers*/
              do
              {
                for (int32_t i = 0; i < pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].numInBufs; i++)
                {
                  if (pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].inData[i].dataId == TIDLPCLayers2.outData[0].dataId)
                  {
                    pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].inData[i].dataId = TIDLPCLayers1.outData[0].dataId;
                    strcpy ((char*)TIDLPCLayers1.outDataNames[0], (char*)TIDLPCLayers2.outDataNames[0]);
                    strcpy ((char*)pOrgTIDLNetStructure.TIDLPCLayers[outLayerIdx].inDataNames[i], (char*)TIDLPCLayers1.outDataNames[0]);
                  }
                }
                outLayerIdx = tidl_getOutLayer (pOrgTIDLNetStructure, layerIndex, TIDLPCLayers2.outData[0].dataId);
              } while (outLayerIdx != -1);

              TIDLPCLayers2.numInBufs  = -1;
              TIDLPCLayers2.numOutBufs = -1;
              pOrgTIDLNetStructure.TIDLPCLayers[constIdx].numInBufs  = -1;
              pOrgTIDLNetStructure.TIDLPCLayers[constIdx].numOutBufs = -1;
            }
          }
        }
      }
    }
  }

  return status;
}


int32_t tidl_FindOnnxShuffleLayer(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t i1, i2, i3, i4;
  int32_t status = 0;
  for (i1 = 0; i1 < layerIndex; i1++)
  {
    if (tf_isLayerType("TIDL_OnnxShuffle", i1, pOrgTIDLNetStructure, TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
    {
      int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_OnnxShuffle", TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t)));

      sTIDL_LayerPC_t &TIDLPCLayers1 = pOrgTIDLNetStructure.TIDLPCLayers[i1];
      int32_t nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
      if(nextlayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        continue;
      sTIDL_LayerPC_t &TIDLPCLayers2 = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];
      nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx].outData[0].dataId);
      if(nextlayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        continue;
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

      if ((TIDLPCLayers1.weights.bufSize ==  6) && (TIDLPCLayers2.weights.bufSize ==  6) && (TIDLPCLayers3.weights.bufSize == 4))
      {
        int64_t * reshape1  = (int64_t *)TIDLPCLayers1.weights.ptr;
        int64_t * reshape2  = (int64_t *)TIDLPCLayers3.weights.ptr;
        int32_t * transpose = (int32_t *)TIDLPCLayers2.weights.ptr;
        if ((reshape1[4]*reshape1[2] == reshape2[2]) && (reshape1[3]*reshape1[5] == reshape2[3]) && (reshape1[1] == reshape2[1]) && (reshape1[2] == reshape1[3]) &&
            (3 == transpose[5]) && (5 == transpose[4]) && (2 == transpose[3]) && (4 == transpose[2]) && (1 == transpose[1]) &&
            ((reshape1[2] == 8) || (reshape1[2] == 4) || (reshape1[2] == 2)))
        {
          pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0] = TIDLPCLayers3.outData[0];
          strcpy((char *)pOrgTIDLNetStructure.TIDLPCLayers[i1].outDataNames[0], (char *)TIDLPCLayers3.outDataNames[0]);
          pOrgTIDLNetStructure.TIDLPCLayers[i1].outConsumerCnt[0] = TIDLPCLayers3.outConsumerCnt[0];

          TIDLPCLayers2.numInBufs = -1;
          TIDLPCLayers2.numOutBufs = -1;
          TIDLPCLayers3.numInBufs = -1;
          TIDLPCLayers3.numOutBufs = -1;

          TIDLPCLayers1.layerType = TIDL_DepthToSpaceLayer;

          if((gParams.deviceName & ~TIDL_OTF_FLAG_BIT) == TIDL_AM62A)
          {
            TIDL_GLOBAL_REPORT_ERROR("Device AM62A and AM67A currently does not support Pixel Shuffle layer");
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
          TIDLPCLayers1.layerParams.depthToSpaceParams.blockSize = reshape1[2];
        }
      }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_identifyPatchEmbeddingBlock(sTIDL_OrgNetwork_t  &pOrgTIDLNetStructure, int32_t layerIndex)
{
  int32_t i1, i2, i3, i4;
  int32_t status = 0;

  /**
   * Identify patch embedding block consisting of convolution layer
   * Convolution parameters - kernel size (x,y) and stride size (a,b) should match
  */
  if (tf_isLayerType("TIDL_PatchEmbedding", layerIndex, pOrgTIDLNetStructure, TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t))) &&
      (!gParams.preQuantizedModel))
  {
    sTIDL_LayerPC_t &TIDLPCLayers1 = pOrgTIDLNetStructure.TIDLPCLayers[layerIndex];
    sTIDL_ConvParams_t* params = &TIDLPCLayers1.layerParams.convParams;
    if (params->kernelH == params->strideH &&
        params->kernelW == params->strideW &&
        params->kernelH == params->kernelW &&
        params->kernelH >= 3 &&
        params->dilationH == 1 && params->dilationW == 1 &&
        params->numGroups == 1 &&
        (params->padT == 0 && params->padL == 0 && params->padB == 0 && params->padR == 0 && params->padH == 0 && params->padW == 0))
        {
          /**
           * Identified a patch embedding block
          */
          status = 1;
        }
  }

  return status;
}

int32_t tidl_isConstantUsedAsInData(sTIDL_LayerPC_t& layer, int32_t onnxInputIdx)
{
  int32_t isConstantUsedAsInData = 0;
  /*
  Below are the layer for which constant/initializer can be input data to TIDL node
  For these layers constant/initializer names also can be  used for inData names
  */
  if((layer.layerType == TIDL_InnerProductLayer) ||
     (layer.layerType == TIDL_EltWiseLayer) ||
     (layer.layerType == TIDL_ScatterElementsLayer) ||
     (layer.layerType == TIDL_ConcatLayer) ||
     (layer.layerType == TIDL_GatherLayer) ||
     (layer.layerType == TIDL_TransposeLayer)
  )
  {
    isConstantUsedAsInData = 1;
  }
  else if(layer.layerType == TIDL_LSTMLayer)
  {
    /* W, R, initial_h, initial_C or peepholes */
    if(onnxInputIdx == 1 || onnxInputIdx == 2 || onnxInputIdx == 5 ||
       onnxInputIdx == 6 || onnxInputIdx == 7)
    {
      isConstantUsedAsInData = 1;
    }
    else
    {
      isConstantUsedAsInData = 0;
    }
  }
  else if(layer.layerType == TIDL_GRULayer || layer.layerType == TIDL_RNNLayer)
  {
    /* W, R or initial_h input */
    if(onnxInputIdx == 1 || onnxInputIdx == 2 || onnxInputIdx == 5)
    {
      isConstantUsedAsInData = 1;
    }
    else
    {
      isConstantUsedAsInData = 0;
    }
  }

  return isConstantUsedAsInData;
}

int32_t tidl_doesTIDLchangenumInputs (sTIDL_LayerPC_t &layer)
{
  /** This function identifies layers where TIDL could potentially change numInputs from what is
   * expected in the model
  */
  int32_t status = 0;

  if (layer.layerType == TIDL_BatchNormLayer)
  {
    /** Eltwise Add/Mul with const Input (1D) gets converted to BatchNorm Layer
     * Model has numInputs 2 for this layer, but batchnorm has only one input in TIDL
    */
    status = 1;
  }

  return status;
}

int32_t tidl_updateInDataNameForInputChangingLayers (sTIDL_LayerPC_t &layer,
  int32_t              i,
  GraphProto&             onnxGraph)
{
  /** Eltwise Add/Mul can be converted to BatchNorm Layer that has only one input
   * If the Add/Mul first input is const input, inDataName for BatchNorm Layer is not filled
   * Check if the first input is const, and fill the second input name in BatchNorm inDataName
  */

  if (tidl_doesTIDLchangenumInputs (layer) == 1)
  {
    if (layer.allowlistingMetaData.numInputs == 2 &&
        layer.allowlistingMetaData.numVarInputs == 1 &&
        layer.allowlistingMetaData.numConstInputs == 1 &&
        layer.allowlistingMetaData.constTensorIndices[0] == 0 &&
        layer.allowlistingMetaData.varTensorIndices[0] == 1)
    {
      strcpy((char*)layer.inDataNames[0], onnxGraph.node(i).input(1).c_str());
    }
  }

  return 0;
}

int32_t tidl_onnxLayerFillTensorNames(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  GraphProto&             onnxGraph)
{
  int32_t j;
  if(strlen(onnxGraph.node(i).name().c_str()) < TIDL_STRING_SIZE)
  {
    strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].name, onnxGraph.node(i).name().c_str());
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", onnxGraph.node(i).name().c_str(), TIDL_STRING_SIZE);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  int32_t numInBufsOnnx = onnxGraph.node(i).input_size();
  if (numInBufsOnnx > 0)
  {
    int k=0;
    for (j = 0; j < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numInBufs; j++)
    {
      if(strlen(onnxGraph.node(i).input(j + k).c_str()) < TIDL_STRING_SIZE)
      {
          /* If constant/initializer is used as input to TIDL node/layer then use constant name itself for inData name
             If constant/initializer is not used as input to TIDL node/layer then dont use constant name for inData name.
          */
        while((j + k) < numInBufsOnnx)
        {
          if((strcmp(onnxGraph.node(i).input(j + k).c_str(), "") == 0) ||
            (((1 == TIDL_isInputConstInGraph(onnxGraph, onnxGraph.node(i).input(j + k).c_str())) || (TIDL_doesQuantOrDequantHaveConstTensor(onnxGraph, onnxGraph.node(i).input(j + k)))) &&
             (tidl_isConstantUsedAsInData(pOrgTIDLNetStructure->TIDLPCLayers[layerIndex], j + k) == 0)))
          {
            k++;
          }
          else
          {
            break;
          }
        }
        if((j + k) <numInBufsOnnx)
        {
          strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inDataNames[j], onnxGraph.node(i).input(j + k).c_str());
        }

        tidl_updateInDataNameForInputChangingLayers (pOrgTIDLNetStructure->TIDLPCLayers[layerIndex], i, onnxGraph);
      }
      else
      {
        TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", onnxGraph.node(i).input(j + k).c_str(), TIDL_STRING_SIZE);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].inData[j].dataId = -1;
    }
  }
  if (pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs > 0)
  {
    for (j = 0; j < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; j++)
    {
      if(strlen(onnxGraph.node(i).output(j).c_str()) < TIDL_STRING_SIZE)
      {
        strcpy((char*)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[j], onnxGraph.node(i).output(j).c_str());
      }
      else
      {
        TIDL_GLOBAL_REPORT_ERROR("Tensor name(%s) exceeds %d characters, exiting", onnxGraph.node(i).output(j).c_str(), TIDL_STRING_SIZE);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerLinked[j] = 0;
    }
  }
  pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].weightsElementSizeInBits = NUM_WHGT_BITS;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_onnxLayerUpdateConsumerCount(sTIDL_OrgNetwork_t   *pOrgTIDLNetStructure,
  int32_t              i,
  int32_t              layerIndex,
  GraphProto&             onnxGraph)
{
  int32_t i0, i1, i2;
  int32_t numCons = 0;
  for (i0 = 0; i0 < pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].numOutBufs; i0++)
  {

    for (i1 = 0; i1 < onnxGraph.node_size(); i1++)
    {
      for (i2 = 0; i2 < onnxGraph.node(i1).input_size(); i2++)
      {
        if (strcmp((const char *)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[i0], onnxGraph.node(i1).input(i2).c_str()) == 0)
        {
          numCons++;
        }
      }
    }
    for (i2 = 0; i2 < numNetOutData; i2++)
    {
      if (strcmp((const char *)pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outDataNames[i0], outDataNames[i2]) == 0)
      {
        numCons++;
      }
    }
    pOrgTIDLNetStructure->TIDLPCLayers[layerIndex].outConsumerCnt[i0] = numCons;
  }
  return 0;
}

int32_t tidl_onnxGetNewNodeToAdd(sTIDL_OrgNetwork_t   &orgTIDLNetStructure,
  int32_t              layerIndex,
  GraphProto&             onnxGraph)
{
  int32_t i, j, nodeIdx = -1;

  for (i = 0; i < layerIndex; i++)
  {
    for (j = 0; j < orgTIDLNetStructure.TIDLPCLayers[i].numInBufs; j++)
    {
      if (TIDL_getLayerIdx(&orgTIDLNetStructure, layerIndex, (const char *)orgTIDLNetStructure.TIDLPCLayers[i].inDataNames[j]) == -1)
      {
        nodeIdx = TIDL_onnxGetNodeIdx(onnxGraph, (const char *)orgTIDLNetStructure.TIDLPCLayers[i].inDataNames[j]);
        if (nodeIdx != -1)
        {
          if (strcmp(onnxGraph.node(nodeIdx).op_type().c_str(), "Constant") == 0)
          {
            nodeIdx = -1;
          }
        }
        if (nodeIdx != -1)
        {
          break;
        }
      }
    }
    if (nodeIdx != -1)
    {
      break;
    }
  }
  return nodeIdx;
}

int32_t tidl_findOnnxInputNames(GraphProto &onnxGraph, char *inList)
{
  int i, j, k, l;
  char tensorName[TIDL_STRING_SIZE];
  char outTensorName[TIDL_STRING_SIZE];
  int inputSize = 0;

  std::unordered_set<std::string> initializerNames;
  for(int i = 0; i < onnxGraph.initializer_size(); i++)
  {
    initializerNames.insert(onnxGraph.initializer(i).name());
  }

  if(onnxGraph.input_size() > 0)
  {
    for (i = 0; i < onnxGraph.input_size(); i++)
    {
      const std::string &name = onnxGraph.input(i).name(); 
      if(initializerNames.count(name))
      {
        continue;
      }
      strcat(inList, name.c_str());
      if (i != onnxGraph.input_size() - 1)
      {
        strcat(inList, ",");
      }
    }
  }
  else
  {
    for (i = 0; i < onnxGraph.node_size(); i++)
    {
      const auto& node = onnxGraph.node(i);
      for (j = 0; j < node.input_size(); j++)
      {
        int inDataUsed = 0;
        const std::string &name = node.input(j);

        if(name.empty() ||
           initializerNames.count(name) ||
           TIDL_isInputConstInGraph(onnxGraph, name) ||
           TIDL_doesQuantOrDequantHaveConstTensor(onnxGraph, name)
          )
        {
          continue;
        }

        strncpy((char *)tensorName, name.c_str(), TIDL_STRING_SIZE);
        for (k = 0; k < onnxGraph.node_size(); k++)
        {
          for (l = 0; l < onnxGraph.node(k).output_size(); l++)
          {
            strncpy((char *)outTensorName, onnxGraph.node(k).output(l).c_str(), TIDL_STRING_SIZE);
            if (strcmp(tensorName, outTensorName) == 0)
            {
              inDataUsed = 1;
              break;
            }
          }
          if (inDataUsed)
          {
            break;
          }
        }
        if (inDataUsed == 0)
        {
          strcat(inList, tensorName);
          strcat(inList, ",");
        }
      }
    }

    TIDL_GLOBAL_REPORT_WARNING("Import configuration does not have inDataNamesList specified, assuming input order - %s", inList);
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

// Function to convert ONNX element type to TIDL data type
int32_t mapOnnxTypeToTidlOutputType(int onnx_elem_type,tidl_import_config* params,int32_t outputIdx) 
{
  if (onnxToTidlTypeMap.find(onnx_elem_type) != onnxToTidlTypeMap.end())
  {
    params->outElementType[outputIdx] = onnxToTidlTypeMap[onnx_elem_type];
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("ERROR : ONNX RT data type : %d not supported by TIDL\n", (int32_t)onnx_elem_type);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  if(params->metaArchType == -1)
  {
    if(params->addDataConvertToNet & ADD_DC_LAYER_AT_OUTPUT)
    {
      params->outTensorScale[outputIdx] = 1.0;
      params->outZeroPoint[outputIdx] = 0.0;
    }
    else
    {
      if(params->numFeatureBits == 32)
      {
        params->outElementType[outputIdx] = TIDL_SinglePrecFloat;
      }
      else if (params->numParamBits > 8)
      {
        if(params->outElementType[outputIdx] == TIDL_UnsignedChar || params->outElementType[outputIdx] == TIDL_UnsignedShort)
        {
          params->outElementType[outputIdx] = TIDL_UnsignedShort;
        }
        else
        {
          params->outElementType[outputIdx] = TIDL_SignedShort;
        }
      }
      else
      {
        if(params->outElementType[outputIdx] == TIDL_UnsignedChar || params->outElementType[outputIdx] == TIDL_UnsignedShort)
        {
          params->outElementType[outputIdx] = TIDL_UnsignedChar;
        }
        else
        {
          params->outElementType[outputIdx] = TIDL_SignedChar;
        }
      }
    }
  }
  else
  {
    if(params->addDataConvertToNet & ADD_DC_LAYER_AT_OUTPUT)
    {
      params->outTensorScale[outputIdx] = 1.0;
      params->outZeroPoint[outputIdx] = 0.0;
      params->outElementType[outputIdx] = TIDL_SinglePrecFloat; /* conv heads will have float output */
    }
    else
    {
      if(params->numFeatureBits == 32)
      {
        params->outElementType[outputIdx] = TIDL_SinglePrecFloat;
      }
      else if (params->numParamBits > 8)
      {
        params->outElementType[outputIdx] = TIDL_SignedShort;
      }
      else
      {
        params->outElementType[outputIdx] = TIDL_SignedChar;
      }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_findOnnxOutElementTypes(GraphProto &onnxGraph,char *outputName,int32_t outputIdx,tidl_import_config* params)
{
  int i;
  bool found = false;
  int status;
  
  for(i = 0; i< onnxGraph.output_size(); i++)
  {
    const auto& output = onnxGraph.output(i);
    if(strcmp(outputName, output.name().c_str()) == 0)
    {
      if(output.has_type() && output.type().has_tensor_type())
      {
        int elemType = output.type().tensor_type().elem_type();
        status = mapOnnxTypeToTidlOutputType(elemType,params,outputIdx);
        found = true;
      }
      else
      {
        TIDL_GLOBAL_REPORT_ERROR("Output %s - Type information is not available",outputName);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
    }
  }

  if(!found)
  {
    for (int i = 0; i < onnxGraph.value_info_size(); i++)
    {
      const auto& value_info = onnxGraph.value_info(i);
      if(strcmp( value_info.name().c_str(), outputName) == 0)
      {
        if ( value_info.has_type() && value_info.type().has_tensor_type())
        {
          int elemType = value_info.type().tensor_type().elem_type();
          status = mapOnnxTypeToTidlOutputType(elemType,params,outputIdx);
        }
        else
        {
          TIDL_GLOBAL_REPORT_ERROR("Output %s - Type information is not available",outputName);
          return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
        }
      }
    }
  }
  return status;
}

int32_t mapOnnxTypeToTidlInputType(int onnx_elem_type,tidl_import_config* params,int32_t inputIdx)
{
  if ( onnxToTidlTypeMap.find(onnx_elem_type) != onnxToTidlTypeMap.end())
  {
    params->inElementType[inputIdx] = onnxToTidlTypeMap[onnx_elem_type];
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("ERROR : ONNX RT data type : %d not supported by TIDL\n", (int32_t)onnx_elem_type);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  if (params->inElementType[inputIdx] == TIDL_SignedDoubleWord)
  {
    params->inElementType[inputIdx] = TIDL_SignedWord;
  }

  int8_t inputDCAdded = 0;

  if(params->addDataConvertToNet & ADD_DC_LAYER_AT_INPUT)
  {
    /*
      * Dataconvert layer does not supoort these input element type.
      * In this case, use arm data conversion
      */
    if (params->inElementType[inputIdx] == TIDL_UnsignedWord ||
        params->inElementType[inputIdx] == TIDL_SignedWord ||
        params->inElementType[inputIdx] == TIDL_UnsignedDoubleWord ||
        params->inElementType[inputIdx] == TIDL_SignedDoubleWord)
    {
      inputDCAdded = 0;
    }
    else
    {
      params->inQuantFactor[inputIdx] = 1.0;
      params->inZeroPoint[inputIdx] = 0.0;
      inputDCAdded = 1;
    }
  }

  if (inputDCAdded == 0)
  {
    if(params->inElementType[inputIdx] != TIDL_SignedWord && params->inElementType[inputIdx] != TIDL_SignedDoubleWord) //For pointpillars, keep the inElementType same (int32) for indices
    {
      if(params->numFeatureBits == 32)
      {
        params->inElementType[inputIdx] = TIDL_SinglePrecFloat;
      }
      else if (params->numParamBits > 8)
      {
        if(params->inElementType[inputIdx] == TIDL_UnsignedChar || params->inElementType[inputIdx] == TIDL_UnsignedShort)
        {
          params->inElementType[inputIdx] = TIDL_UnsignedShort;
        }
        else
        {
          params->inElementType[inputIdx] = TIDL_SignedShort;
        }
      }
      else
      {
        if(params->inElementType[inputIdx] == TIDL_UnsignedChar || params->inElementType[inputIdx] == TIDL_UnsignedShort)
        {
          params->inElementType[inputIdx] = TIDL_UnsignedChar;
        }
        else
        {
          params->inElementType[inputIdx] = TIDL_SignedChar;
        }
      }
    }
    if(params->inElementType[inputIdx] == TIDL_SignedDoubleWord)
    {
      // indices can be 64-bit for scatter/gather, tidl doesn't support 64-bit indices, hence convert them to 32-bit
      params->inElementType[inputIdx] = TIDL_SignedWord;
    }
  }
  return (0);
}

int32_t tidl_findOnnxInputElementTypes(GraphProto& onnxGraph,char *inputName,int32_t inputIdx,tidl_import_config *params)
{
  int i,j,found;
  found = 0;
  int status;

  auto findValueInfo =[&](const std::string &name)->const onnx::ValueInfoProto* {
    for(int i = 0; i < onnxGraph.value_info_size(); i++){
      if(onnxGraph.value_info(i).name() == name)
      {
        return &onnxGraph.value_info(i);
      }
    }
    return nullptr;
  };

  for(i = 0; i < onnxGraph.input_size(); i++)
  {
    const auto &input = onnxGraph.input(i);
    const std::string &name = input.name();
    if(strcmp(inputName, name.c_str()) == 0)
    {
      if(input.has_type() && input.type().has_tensor_type())
      {
        int elemType = input.type().tensor_type().elem_type();
        status = mapOnnxTypeToTidlInputType(elemType,params,inputIdx);
        found = 1;
      }
      else
      {
        TIDL_GLOBAL_REPORT_ERROR("Input %s - Type information is not available",name.c_str());
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
    }
  }
  if(found == 0)
  {
    for(i = 0; i < onnxGraph.node_size(); i++)
    {
      const auto &node = onnxGraph.node(i);
      for(j = 0; j < node.input_size(); j++)
      {
        const std::string &name = node.input(j);
        if(strcmp(inputName, name.c_str()) == 0)
        {
          const auto *info = findValueInfo(name);
          if(info)
          {
            if(info->has_type() && info->type().has_tensor_type())
            {
              int elemType = info->type().tensor_type().elem_type();
              status = mapOnnxTypeToTidlInputType(elemType,params,inputIdx);
              found = 1;
            }
          }
        }
      }
    }
  }
  if(found == 0)
  {
    for(i = 0; i < onnxGraph.initializer_size(); i++)
    {
      const auto& initializer = onnxGraph.initializer(i);
      if(strcmp(inputName , initializer.name().c_str()) == 0)
      {
        int elemType = initializer.data_type();
        status = mapOnnxTypeToTidlInputType(elemType,params,inputIdx);
      }
    }
  }
  return status;
}


int32_t tidl_findOnnxInputShapes(GraphProto& onnxGraph, char *inputName, int32_t inputIdx, tidl_import_config *params)
{
  int i, j, k;

  auto findValueInfo =[&](const std::string &name)->const onnx::ValueInfoProto* {
    for(int i = 0; i < onnxGraph.value_info_size(); i++){
      if(onnxGraph.value_info(i).name() == name)
      {
        return &onnxGraph.value_info(i);
      }
    }
    return nullptr;
  };

  int8_t found = 0;
  for (i = 0; i < onnxGraph.input_size(); i++)
  {
    const auto &input = onnxGraph.input(i);
    const std::string &name = input.name();
    if (strcmp(inputName, name.c_str()) == 0)
    {
      found = 1;
      if(input.has_type() && input.type().has_tensor_type())
      {
        const auto &shape = input.type().tensor_type().shape();
        int dimSize = shape.dim_size();

        params->inWidth[inputIdx] = 1;
        params->inHeight[inputIdx] = 1;
        params->inNumChannels[inputIdx] = 1;
        params->inDIM2[inputIdx] = 1;
        params->inDIM1[inputIdx] = 1;
        params->numBatches[inputIdx] = 1;

        for(k = 0; k < dimSize; k++)
        {
          const auto &dim = shape.dim(k);
          if(dim.has_dim_value()  && dim.dim_value() !=0)
          {
            if((dimSize-1) == k) params->inWidth[inputIdx] = shape.dim(dimSize-1).dim_value();
            if((dimSize-2) == k) params->inHeight[inputIdx] = shape.dim(dimSize-2).dim_value();
            if((dimSize-3) == k) params->inNumChannels[inputIdx] = shape.dim(dimSize-3).dim_value();
            if((dimSize-4) == k) params->inDIM2[inputIdx] = shape.dim(dimSize-4).dim_value();
            if((dimSize-5) == k) params->inDIM1[inputIdx] = shape.dim(dimSize-5).dim_value();
            if((dimSize-6) == k) params->numBatches[inputIdx] = shape.dim(dimSize-6).dim_value();
          }
          else
          {
            TIDL_GLOBAL_REPORT_ERROR("Onnx graph have dynamic/unknown input shape for input %s",name.c_str());
            return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
          }
        }
      }
      else
      {
        TIDL_GLOBAL_REPORT_ERROR("Onnx graph doesnot have input shape for input %s",name.c_str());
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
    }
  }

  if(found == 0)
  {
    for (i = 0; i < onnxGraph.node_size(); i++)
    {
      const auto &node = onnxGraph.node(i);
      for (j = 0; j < node.input_size(); j++)
      {
        const std::string &name = node.input(j);
        if (strcmp(inputName, name.c_str()) == 0)
        {
          const auto *info = findValueInfo(name);
          if(info)
          {
            if(info->has_type() && info->type().has_tensor_type())
            {
              const auto &shape = info->type().tensor_type().shape();
              int dimSize = shape.dim_size();

              params->inWidth[inputIdx] = 1;
              params->inHeight[inputIdx] = 1;
              params->inNumChannels[inputIdx] = 1;
              params->inDIM2[inputIdx] = 1;
              params->inDIM1[inputIdx] = 1;
              params->numBatches[inputIdx] = 1;
            
              for(k = 0; k < dimSize; k++)
              {
                const auto &dim = shape.dim(k);
                if(dim.has_dim_value()  && dim.dim_value() !=0)
                {
                  if((dimSize-1) == k) params->inWidth[inputIdx] = shape.dim(dimSize-1).dim_value();
                  if((dimSize-2) == k) params->inHeight[inputIdx] = shape.dim(dimSize-2).dim_value();
                  if((dimSize-3) == k) params->inNumChannels[inputIdx] = shape.dim(dimSize-3).dim_value();
                  if((dimSize-4) == k) params->inDIM2[inputIdx] = shape.dim(dimSize-4).dim_value();
                  if((dimSize-5) == k) params->inDIM1[inputIdx] = shape.dim(dimSize-5).dim_value();
                  if((dimSize-6) == k) params->numBatches[inputIdx] = shape.dim(dimSize-6).dim_value();
                }
                else
                {
                  TIDL_GLOBAL_REPORT_ERROR("Onnx graph have dynamic/unknown input shape for input %s",name.c_str());
                  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
                }
              }
            }
            else
            {
              TIDL_GLOBAL_REPORT_ERROR("Onnx graph doesnot have input shape for input %s",name.c_str());
              return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
            }
          }
        }
      }
    }
  }
  if(found == 0)
  {
    for(int k=0; k< onnxGraph.initializer_size();k++)
    {
      if((strcmp(onnxGraph.initializer(k).name().c_str(), inputName) == 0))
      {        
        int dimSize = onnxGraph.initializer(k).dims_size();
        params->inWidth[inputIdx] = 1;
        params->inHeight[inputIdx] = 1;
        params->inNumChannels[inputIdx] = 1;
        params->inDIM2[inputIdx] = 1;
        params->inDIM1[inputIdx] = 1;
        params->numBatches[inputIdx] = 1;

        for(i = 0; i < dimSize; i++)
        { 
          if((dimSize-1) == i) params->inWidth[inputIdx] = onnxGraph.initializer(k).dims(dimSize-1);
          if((dimSize-2) == i) params->inHeight[inputIdx] = onnxGraph.initializer(k).dims(dimSize-2);
          if((dimSize-3) == i) params->inNumChannels[inputIdx] = onnxGraph.initializer(k).dims(dimSize-3);
          if((dimSize-4) == i) params->inDIM2[inputIdx] = onnxGraph.initializer(k).dims(dimSize-4);
          if((dimSize-5) == i) params->inDIM1[inputIdx] = onnxGraph.initializer(k).dims(dimSize-5);
          if((dimSize-6) == i) params->numBatches[inputIdx] = onnxGraph.initializer(k).dims(dimSize-6); 
        }
      }
    }
  }

  return (0);
}


int32_t tidl_findOnnxOutputNames(GraphProto &onnxGraph, char *outList)
{
  int i, j, k, l;
  char tensorName[TIDL_STRING_SIZE];
  char inTensorName[TIDL_STRING_SIZE];
  int outPutSize = 0;

  if(onnxGraph.output_size() > 0)
  {
    for (i = 0; i < onnxGraph.output_size(); i++)
    {
      strcat(outList, onnxGraph.output(i).name().c_str());
      strcat(outList, ",");
    }
  }
  else
  {
    for (i = 0; i < onnxGraph.node_size(); i++)
    {
      outPutSize = onnxGraph.node(i).output_size();
      if (strcmp(onnxGraph.node(i).op_type().c_str(), "Dropout") == 0)
      {
        outPutSize = 1;
      }
      for (j = 0; j < outPutSize; j++)
      {
        int outDataUsed = 0;
        strncpy((char *)tensorName, onnxGraph.node(i).output(j).c_str(), TIDL_STRING_SIZE);
        for (k = 0; k < onnxGraph.node_size(); k++)
        {
          for (l = 0; l < onnxGraph.node(k).input_size(); l++)
          {
            strncpy((char *)inTensorName, onnxGraph.node(k).input(l).c_str(), TIDL_STRING_SIZE);
            if (strcmp(tensorName, inTensorName) == 0)
            {
              outDataUsed = 1;
              break;
            }
          }
          if (outDataUsed)
            break;
        }
        if (outDataUsed == 0)
        {
          strcat(outList, tensorName);
          strcat(outList, ",");
        }
      }
    }
  }
  return (0);
}

int32_t tidl_handleLayerswithMultipleOutputs (sTIDL_LayerPC_t &TIDLPCLayer, GraphProto& onnxGraph, int32_t newNode, int32_t& dataIndex)
{
  int32_t j = 0;
  if (layersWithMultipleOutputBuffers.find(onnxGraph.node(newNode).op_type()) != layersWithMultipleOutputBuffers.end())
  {
      TIDLPCLayer.numOutBufs = onnxGraph.node(newNode).output_size();
      for (j = 0; j < TIDLPCLayer.numOutBufs; j++)
      {
        TIDLPCLayer.outData[j].dataId = dataIndex++;
      }
  }
  else
  {
    TIDLPCLayer.outData[0].dataId = dataIndex++;
  }
  return 0;
}

int32_t onnx_import(tidl_import_config * params, int32_t *totalData, int32_t* totalLayers)
{
  int32_t                    i,j;
  int32_t                    layerNum;
  int32_t                    inputSize;
  int32_t                    pad,stride;
  int32_t                    layerIndex;
  int32_t                    tiLayerIndex;
  int32_t                    dataIndex;
  const uint8_t             *name;
  const uint8_t             *inputName[10];
  const uint8_t             *outputName;
  ModelProto         onnxModel;
  int32_t status = 0;
  int32_t                    dataSize;
  int32_t                    id;
  int paramSet  = 0;
  int conv2DRandParams = 0;
  string attrKey;
  int32_t inLayerId = 0;
  int32_t weightsElementSizeInBits;
  int32_t mapTblIdx = -1;

  string key = "value";

  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "ONNX model (Proto) file      : %s  ",(const char *)params->inputNetFile);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL network file            : %s  ", (const char *)params->outputNetFile);
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL IO info file            : %s  ", (const char *)params->outputParamsFile);

  bool success = TIDL_readProtoFromBinaryFile((const char *)params->inputNetFile, (google::protobuf::Message*)&onnxModel);
  if(!success){
    TIDL_GLOBAL_REPORT_ERROR("Reading models params file failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  GraphProto onnxGraph = onnxModel.graph();
  auto & opset_import = onnxModel.opset_import(0);
  onnxOpSetVersion =  opset_import.version();
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Current ONNX OpSet version   : %d  ", onnxOpSetVersion);
  if (onnxOpSetVersion > HIGHEST_OPSET_VERSION_SUPPORTED)
  {
    TIDL_GLOBAL_REPORT_WARNING("TIDL-RT only supports operators upto OPSET %d. Using operators from a higher OPSET may result in undefined behavior and/or errors", HIGHEST_OPSET_VERSION_SUPPORTED);
  }

  layerIndex = 0;
  dataIndex  = 0;
  
  if (strcmp((char*)params->outDataNamesList, "") == 0)
  {
    char outDataNamesList[TIDL_MAX_ALG_OUT_BUFS * TIDL_STRING_SIZE] = "";
    tidl_findOnnxOutputNames(onnxGraph, (char*)outDataNamesList);
    numNetOutData = tidl_getStringsFromList((char *)outDataNamesList, (char*)outDataNames, TIDL_MAX_DATA_NAME);
  }
  else
  {
    numNetOutData = tidl_getStringsFromList((char *)params->outDataNamesList, (char*)outDataNames, TIDL_MAX_DATA_NAME);
  }
  
  for (i = 0; i < numNetOutData; i++)
  {
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = -1;
    strcpy((char*)orgTIDLNetStructure.TIDLPCLayers[layerIndex].name, outDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].inDataNames[0], outDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[0], outDataNames[i]);
    layerIndex++;
  }

  if(params->inElementType[0] == -1)
  {
    for(i = 0; i < numNetOutData ; i++)
    {
      if(params->outElementType[i] == -1)
      {
        tidl_findOnnxOutElementTypes(onnxGraph, outDataNames[i], i, params);
      }
    }
  }

  /* This fixes TIDL-4015 */
  if (strcmp((char *)params->inDataNamesList, "") == 0)
  {
    char inDataNamesList[TIDL_MAX_ALG_OUT_BUFS * TIDL_STRING_SIZE] = "";
    tidl_findOnnxInputNames(onnxGraph, (char*)inDataNamesList);
    numNetInData = tidl_getStringsFromList((char *)inDataNamesList, (char*)inDataNames, TIDL_MAX_DATA_NAME);
    snprintf((char*)params->inDataNamesList,sizeof(params->inDataNamesList),"%s",inDataNamesList);
  }
  else
  {
    numNetInData = tidl_getStringsFromList((char *)params->inDataNamesList, (char*)inDataNames, TIDL_MAX_DATA_NAME);
  }

  for (i = 0; i < numNetInData; i++)
  {
    //Automatically parse shape from inputs if not set explicitly
    if(params->inWidth[i] == -1 || params->inHeight[i] == -1 || params->inNumChannels[i] == -1)
    {
      tidl_findOnnxInputShapes(onnxGraph, inDataNames[i], i, params);
    }
  }

  for (i = 0; i < numNetInData; i++)
  {
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = -1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = 1;
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].name, inDataNames[i]);
    strcpy((char *)orgTIDLNetStructure.TIDLPCLayers[layerIndex].outDataNames[0], inDataNames[i]);
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerCnt[0] = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outConsumerLinked[0] = 0;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dataId = dataIndex++;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].numDim = TIDL_DIM_MAX;
    tidl_onnxLayerUpdateConsumerCount(&orgTIDLNetStructure, layerIndex, layerIndex, onnxGraph);
    tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);
    layerIndex++;
  }

  for(i = 0; i < numNetInData ; i++)
  {
    if(params->inElementType[i] == -1)
    {
      tidl_findOnnxInputElementTypes(onnxGraph, inDataNames[i], i, params);
    }
  }

  bool isModelShapeInfered = true;
       
  int newNode = tidl_onnxGetNewNodeToAdd(orgTIDLNetStructure, layerIndex, onnxGraph);
  while (newNode != -1)
  {
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numInBufs = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].numOutBufs = 1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].inData[0].dataId = -1;
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].actParams.actType = TIDL_NoAct;
    sTIDL_LayerPC_t &TIDLPCLayer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
    /**
     * exceptions for onnxMapParams functions -
     * TODO: handle elegantly
     *
     * */
    tidl_handleLayerswithMultipleOutputs(TIDLPCLayer, onnxGraph, newNode, dataIndex);

    if (strcmp(onnxGraph.node(newNode).op_type().c_str(), "ScatterND") == 0)
    {
      TIDLPCLayer.outData[0].numDim = 4;
    }
    /* Parse layer */
    TidlParseOnnx onnxParsed = TidlParseOnnx(onnxGraph, newNode, TIDLPCLayer, isModelShapeInfered);
    TIDLPCLayer = onnxParsed.layer;
    status = onnxParsed.status;
    /* If Unsupported layer, i.e., layer will not be parsed */
    if (TIDLPCLayer.layerType == TIDL_UnsupportedLayer)
    {
      TIDL_GLOBAL_REPORT_ERROR("Layer type not supported by TIDL --- layer type - %s,  Node name - %s", onnxGraph.node(newNode).op_type().c_str(), onnxGraph.node(newNode).name().c_str());
    }
    /* Or If parse fails */
    if(TIDLPCLayer.layerType == TIDL_UnsupportedLayer ||
      TIDLPCLayer.parseStatus == TIDL_ParseFailed)
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

    TIDL_IMPORT_CHECK_AND_RETURN(tidl_onnxLayerFillTensorNames(&orgTIDLNetStructure, newNode, layerIndex, onnxGraph),"");
    tidl_onnxLayerUpdateConsumerCount(&orgTIDLNetStructure, newNode, layerIndex, onnxGraph);
    tidl_linkInputTensors(&orgTIDLNetStructure, layerIndex);
    tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);
    layerIndex++;

    newNode = tidl_onnxGetNewNodeToAdd(orgTIDLNetStructure, layerIndex, onnxGraph);
  }
  if(!isModelShapeInfered)
  {
    TIDL_GLOBAL_REPORT_WARNING("Shape inference has not been performed on the model. Skipping data type validation checks.");
  }
  *totalData = dataIndex;
  *totalLayers = layerIndex;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_supportMultiChannelMatMulBias(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex){
  for (int32_t i1 = 0; i1 < layerIndex; i1++)
  {
    if (tf_isLayerType("TIDL_MatMulBias", i1, pOrgTIDLNetStructure, TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t))))
    {
      int32_t mapIdx = tf_getLayreTypeMapIdx("TIDL_MatMulBias", TIDL_OnnxLayerMap, (sizeof(TIDL_OnnxLayerMap) / sizeof(TIDL_TFLayerMapping_t)));

      sTIDL_LayerPC_t &matmulLayer = pOrgTIDLNetStructure.TIDLPCLayers[i1];
      if(matmulLayer.allowlistingMetaData.numConstInputs == 1){
        int32_t numSingletonDims = 0;
        std::vector<int> dims = matmulLayer.allowlistingMetaData.constTensorsDims[0];
        for(int32_t j = 0; j < dims.size(); j++){
          if(dims[j] != 1){
            numSingletonDims += 1;
          }
        }
        if(numSingletonDims > 2) continue;
      }
      int32_t nextlayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[i1].outData[0].dataId);
      if(nextlayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      sTIDL_LayerPC_t &addLayer = pOrgTIDLNetStructure.TIDLPCLayers[nextlayerIdx];
        if (addLayer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum &&
            addLayer.allowlistingMetaData.numConstInputs == 1 &&
            addLayer.allowlistingMetaData.constTensorsDims[0].size() == 1 &&
            matmulLayer.outData[0].dimValues[TIDL_DIM_DIM2] == 1  &&
            matmulLayer.outData[0].dimValues[TIDL_DIM_DIM1] == 1 &&
            matmulLayer.layerParams.innerProductParams.constIdx == 1 &&
            (addLayer.allowlistingMetaData.constTensorsDims[0][0] == matmulLayer.layerParams.innerProductParams.numInRows ||
              addLayer.allowlistingMetaData.constTensorsDims[0][0] == matmulLayer.layerParams.innerProductParams.numOutCols)
        ){
          if(matmulLayer.outData[0].dimValues[TIDL_DIM_NUMCH] == addLayer.outData[0].dimValues[TIDL_DIM_NUMCH] && matmulLayer.outData[0].dimValues[TIDL_DIM_NUMCH] > 1){
            std::vector<int32_t> inputIndices = {0};
            TIDL_addLayer(pOrgTIDLNetStructure, i1, TIDL_ReshapeLayer, dataIndex, ADD_LAYER_BEFORE, inputIndices);
            sTIDL_LayerPC_t& preReshape = pOrgTIDLNetStructure.TIDLPCLayers[pOrgTIDLNetStructure.numLayers - 1];
            TIDL_addLayer(pOrgTIDLNetStructure, nextlayerIdx, TIDL_ReshapeLayer, dataIndex, ADD_LAYER_AFTER);
            sTIDL_LayerPC_t& postReshape = pOrgTIDLNetStructure.TIDLPCLayers[pOrgTIDLNetStructure.numLayers - 1];

            preReshape.layerPCParams.reshapeParams.isInduced = 1;
            postReshape.layerPCParams.reshapeParams.isInduced = 1;

            memcpy(preReshape.inData[0].dimValues, matmulLayer.inData[0].dimValues, sizeof(int32_t) * TIDL_DIM_MAX);
            memcpy(postReshape.outData[0].dimValues, addLayer.outData[0].dimValues, sizeof(int32_t) * TIDL_DIM_MAX);

            int32_t newDimVals[TIDL_DIM_MAX];
            
            int32_t b, dim1, dim2, numch, height, width;
            b = matmulLayer.inData[0].dimValues[TIDL_DIM_BATCH]; 
            dim1 = matmulLayer.inData[0].dimValues[TIDL_DIM_DIM1]; 
            dim2 = matmulLayer.inData[0].dimValues[TIDL_DIM_DIM2]; 
            numch = matmulLayer.inData[0].dimValues[TIDL_DIM_NUMCH]; 
            height = matmulLayer.inData[0].dimValues[TIDL_DIM_HEIGHT]; 
            width = matmulLayer.inData[0].dimValues[TIDL_DIM_WIDTH];

            newDimVals[TIDL_DIM_BATCH] = b;
            newDimVals[TIDL_DIM_DIM1] = 1;
            newDimVals[TIDL_DIM_DIM2] = 1;
            newDimVals[TIDL_DIM_NUMCH] = 1;
            newDimVals[TIDL_DIM_HEIGHT] = numch * height * dim2 * dim1;
            newDimVals[TIDL_DIM_WIDTH] = width;

            memcpy(preReshape.outData[0].dimValues, newDimVals, sizeof(int32_t) * TIDL_DIM_MAX);
            memcpy(matmulLayer.inData[0].dimValues, newDimVals, sizeof(int32_t) * TIDL_DIM_MAX);
            memcpy(matmulLayer.outData[0].dimValues, newDimVals, sizeof(int32_t) * TIDL_DIM_MAX);
            memcpy(addLayer.inData[0].dimValues, newDimVals, sizeof(int32_t) * TIDL_DIM_MAX);
            memcpy(addLayer.outData[0].dimValues, newDimVals, sizeof(int32_t) * TIDL_DIM_MAX);
            memcpy(postReshape.inData[0].dimValues, newDimVals, sizeof(int32_t) * TIDL_DIM_MAX);

            matmulLayer.layerParams.innerProductParams.numInRows = newDimVals[TIDL_DIM_HEIGHT];
          }
        }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_supportMultiChannelMatMul(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex){
  for (int32_t i1 = 0; i1 < layerIndex; i1++)
  {
    if(pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType == TIDL_InnerProductLayer && pOrgTIDLNetStructure.TIDLPCLayers[i1].allowlistingMetaData.numConstInputs == 1){
      sTIDL_LayerPC_t &matmulLayer = pOrgTIDLNetStructure.TIDLPCLayers[i1];
      if(matmulLayer.allowlistingMetaData.numConstInputs == 1){
        int32_t numSingletonDims = 0;
        int32_t constIdx = matmulLayer.allowlistingMetaData.constTensorIndices[0];
        int32_t constLayerIdx = tidl_getInLayer(pOrgTIDLNetStructure, layerIndex, matmulLayer.inData[constIdx].dataId);
        if(constLayerIdx == -1 || pOrgTIDLNetStructure.TIDLPCLayers[constLayerIdx].layerType != TIDL_ConstDataLayer)
        {
          continue;
        }
        int32_t* dims = pOrgTIDLNetStructure.TIDLPCLayers[constLayerIdx].outData[0].dimValues;
        for(int32_t j = 0; j < TIDL_DIM_MAX; j++){
          if(dims[j] != 1){
            numSingletonDims += 1;
          }
        }
        if(numSingletonDims > 2) continue;
      }
      int32_t varTensorIdx = matmulLayer.allowlistingMetaData.varTensorIndices[0];
      if(matmulLayer.inData[varTensorIdx].dimValues[TIDL_DIM_NUMCH] == 1)
      {
        continue;
      }
      std::vector<int32_t> inputIndices = {varTensorIdx};

      int32_t newDimVals[TIDL_DIM_MAX];

      newDimVals[TIDL_DIM_BATCH] = matmulLayer.inData[0].dimValues[TIDL_DIM_BATCH];
      newDimVals[TIDL_DIM_DIM1] = 1;
      newDimVals[TIDL_DIM_DIM2] = 1;
      newDimVals[TIDL_DIM_NUMCH] = 1;
      newDimVals[TIDL_DIM_HEIGHT] = matmulLayer.inData[0].dimValues[TIDL_DIM_DIM1] * matmulLayer.inData[0].dimValues[TIDL_DIM_DIM2] * matmulLayer.inData[0].dimValues[TIDL_DIM_NUMCH] * matmulLayer.inData[0].dimValues[TIDL_DIM_HEIGHT];
      newDimVals[TIDL_DIM_WIDTH] = matmulLayer.inData[0].dimValues[TIDL_DIM_WIDTH];

      TIDL_wrapWithReshape(pOrgTIDLNetStructure, i1, dataIndex, newDimVals, inputIndices);

      matmulLayer.layerParams.innerProductParams.numInRows = newDimVals[TIDL_DIM_HEIGHT];
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t tidl_supportMultiChannelEltWise(sTIDL_OrgNetwork_t &pOrgTIDLNetStructure, int32_t layerIndex, int32_t* dataIndex){
  for (int32_t i1 = 0; i1 < layerIndex; i1++)
  {
    if(pOrgTIDLNetStructure.TIDLPCLayers[i1].layerType == TIDL_EltWiseLayer && pOrgTIDLNetStructure.TIDLPCLayers[i1].allowlistingMetaData.numConstInputs == 1){
      sTIDL_LayerPC_t &eltwiseLayer = pOrgTIDLNetStructure.TIDLPCLayers[i1];
      if(eltwiseLayer.allowlistingMetaData.numConstInputs == 1){
        int32_t numSingletonDims = 0;
        int32_t constIdx = eltwiseLayer.allowlistingMetaData.constTensorIndices[0];
        std::vector<int> dims = eltwiseLayer.allowlistingMetaData.constTensorsDims[0];
        for(int32_t j = 0; j < dims.size(); j++){
          if(dims[j] != 1){
            numSingletonDims += 1;
          }
        }
        if(numSingletonDims > 2) continue;
      }
      int32_t parentIdx1 = tidl_getInLayer(pOrgTIDLNetStructure, layerIndex, eltwiseLayer.inData[0].dataId);
      if(parentIdx1 != -1 && eltwiseLayer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseProduct && pOrgTIDLNetStructure.TIDLPCLayers[parentIdx1].layerType == TIDL_LayerNormLayer)
      {
        continue;
      }
      if(
        parentIdx1 != -1 &&
        eltwiseLayer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum && 
        pOrgTIDLNetStructure.TIDLPCLayers[parentIdx1].layerType == TIDL_EltWiseLayer && 
        pOrgTIDLNetStructure.TIDLPCLayers[parentIdx1].layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseProduct)
      {
        int32_t parentIdx2 = tidl_getInLayer(pOrgTIDLNetStructure, layerIndex, pOrgTIDLNetStructure.TIDLPCLayers[parentIdx1].inData[0].dataId);
        if(parentIdx2 != -1 && pOrgTIDLNetStructure.TIDLPCLayers[parentIdx2].layerType == TIDL_LayerNormLayer){
          continue;
        }
      }
      int32_t constTensorIdx = eltwiseLayer.allowlistingMetaData.constTensorIndices[0];
      int32_t numNonSingeletonIdxs = 0;
      for(int32_t i = 0; i < TIDL_DIM_MAX; i++){
        if(eltwiseLayer.inData[constTensorIdx].dimValues[i] != 1){
          numNonSingeletonIdxs++;
        }
      }

      // checking if the const data in eltwise is broadcastable or not
      if(!(numNonSingeletonIdxs == 1 && eltwiseLayer.inData[constTensorIdx].dimValues[TIDL_DIM_WIDTH] != 1))
      {
        continue;
      }

      // checking if the previous layer is not innerProduct
      int32_t prevLayerIdx = tidl_getOutLayer(pOrgTIDLNetStructure, pOrgTIDLNetStructure.numLayers, pOrgTIDLNetStructure.TIDLPCLayers[i1].inData[0].dataId);
      if(prevLayerIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }
      if(pOrgTIDLNetStructure.TIDLPCLayers[prevLayerIdx].layerType == TIDL_InnerProductLayer && eltwiseLayer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseSum){
        continue;
      }

      std::vector<int32_t> inputIndices = {eltwiseLayer.allowlistingMetaData.varTensorIndices[0]};
      int32_t newDimVals[TIDL_DIM_MAX];

      newDimVals[TIDL_DIM_BATCH] = eltwiseLayer.inData[0].dimValues[TIDL_DIM_BATCH];
      newDimVals[TIDL_DIM_DIM1] = eltwiseLayer.inData[0].dimValues[TIDL_DIM_DIM1];
      newDimVals[TIDL_DIM_DIM2] = eltwiseLayer.inData[0].dimValues[TIDL_DIM_DIM2];
      newDimVals[TIDL_DIM_NUMCH] = 1;
      newDimVals[TIDL_DIM_HEIGHT] = eltwiseLayer.inData[0].dimValues[TIDL_DIM_NUMCH] * eltwiseLayer.inData[0].dimValues[TIDL_DIM_HEIGHT];
      newDimVals[TIDL_DIM_WIDTH] = eltwiseLayer.inData[0].dimValues[TIDL_DIM_WIDTH];

      TIDL_wrapWithReshape(pOrgTIDLNetStructure, i1, dataIndex, newDimVals, inputIndices);

      eltwiseLayer.layerParams.eltWiseParams.numChannels = 1;
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}
