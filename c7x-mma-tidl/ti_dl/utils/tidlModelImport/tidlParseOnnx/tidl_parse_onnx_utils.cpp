/*
*
* Copyright (c) {2015 - 2024} Texas Instruments Incorporated
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

#include "tidl_parse_onnx.h"


  /* map onnx elem_type to TIDL element types:
      Onnx element types:
      float -> 1
      uint8 -> 2
      int8 -> 3
      uint16 -> 4
      int16 -> 5
      uint32 -> 12
      int32 -> 6
      uint64 -> 13
      int64 -> 7
  */
  map<int,int>typesTable = {
    {1, TIDL_SinglePrecFloat},
    {2, TIDL_UnsignedChar},
    {3, TIDL_SignedChar},
    {4, TIDL_UnsignedShort},
    {5, TIDL_SignedShort},
    {6, TIDL_SignedWord},
    {12, TIDL_UnsignedWord},
    {7, TIDL_SignedDoubleWord},
    {13, TIDL_UnsignedDoubleWord},
    {9, TIDL_Bool},
  };

/** Structure to define output type inference rules */
  struct OutputTypeInferenceRule
  {
    std::string opType;
    int32_t inputIdx;
    int32_t outputIdx;
  };

/** Vector to store output type inference rules
 * For example, for TopK operator, output (values - idx 0) type is same as input (idx - 0) type. 
 * This is used during parsing when output data type needs to be inferred for operators which don't have explicit output data type set. 
 * In such cases, this vector is looked up to find the input index from which output data type can be inferred.
 */
  std::vector<OutputTypeInferenceRule> outputTypeInferenceRules = {
    {"TopK", 0, 0},
  };


/** Function to compute number of variable inputs of layer
 * Also populates corresponding indices of variable inputs
 */
int32_t TidlParseOnnx::numVariableLayerInputs(GraphProto& onnGraph, int32_t i, vector<int32_t>& varIdx)
{
  int32_t numVarInputs = 0;
  for(int j = 0; j < onnGraph.node(i).input_size(); j++)
  {
    if((strcmp(onnGraph.node(i).input(j).c_str(),"") != 0) && !(TIDL_isInputConstInGraph(onnGraph, onnGraph.node(i).input(j)) || doesQuantOrDequantHaveConstTensor(onnGraph, onnGraph.node(i).input(j))))
    {
      numVarInputs++;
      varIdx.push_back(j);
    }
  }
  return numVarInputs;
}

int32_t TidlParseOnnx::getAttrIdx(const NodeProto& node, char * name)
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


TensorProto TidlParseOnnx::getInitializerTensor(GraphProto& onnGraph, const string name, int32_t nIdx, int32_t& status)
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
      int32_t DQNodeIdx = getNodeIdx(onnGraph, name.c_str());
      /*Return DQ's input (inIdx)*/
      if(DQNodeIdx != -1)
      {
        return (getInitializerTensor(onnGraph, onnGraph.node(DQNodeIdx).input(0) , DQNodeIdx, status));
      }
      else
      {
        continue;
      }
    }
    else if((strcmp(onnGraph.node(i).output(0).c_str(), name.c_str()) == 0) && (strcmp(onnGraph.node(i).op_type().c_str(), "QuantizeLinear") == 0))
    {
      int32_t QNodeIdx = getNodeIdx(onnGraph, name.c_str());
      /*Return Q's input (inIdx)*/
      if(QNodeIdx != -1)
      {
        return (getInitializerTensor(onnGraph, onnGraph.node(QNodeIdx).input(0) , QNodeIdx, status));
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


bool TidlParseOnnx::doesQuantOrDequantHaveConstTensor(GraphProto& onnGraph, const string name)
{
  /*Check if buffer is output of dequant, and if part of dequant check if it is a constant buffer */
  bool isConstBuf = false;
  /*Find the corresponding DQ layer via it's output name*/
  int32_t DQNodeIdx = getNodeIdx(onnGraph, name.c_str());
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
      int32_t QNodeIdx = getNodeIdx(onnGraph, onnGraph.node(DQNodeIdx).input(0).c_str());

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

int32_t TidlParseOnnx::getNodeIdx(GraphProto& onnxGraph, const char *bufName)
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

int32_t TidlParseOnnx::getTIDLDataTypeFromOnnxDataType (int32_t onnxDataType)
{
  int32_t tidlDataType = -1;

  if (typesTable.find(onnxDataType) != typesTable.end())
  {
    tidlDataType = typesTable[onnxDataType];
  }

  return tidlDataType;
}

std::string TidlParseOnnx::getOriginalNameFromDuplicateName(std::string duplicateName)
{
  std::string originalName = duplicateName;
  if(duplicateName.find("/duplicated") != std::string::npos)
  {
    originalName = duplicateName.substr(0, duplicateName.find("/duplicated"));
  }
  return originalName;
}

int32_t TidlParseOnnx::getInputDataType(GraphProto& onnxGraph, int32_t nodeIdx, int32_t inputIdx)
{
  if(onnxGraph.node(nodeIdx).input_size() <= inputIdx)
    return -1;

  std::string inputName = onnxGraph.node(nodeIdx).input(inputIdx);
  inputName = getOriginalNameFromDuplicateName(inputName);

  for(int j = 0; j < onnxGraph.initializer_size(); j++)
  {
    if(onnxGraph.initializer(j).name().compare(inputName) == 0)
    {
      return onnxGraph.initializer(j).data_type();
    }
  }

  for(int j = 0; j < onnxGraph.input_size(); j++)
  {
    if(onnxGraph.input(j).name().compare(inputName) == 0)
    {
      return onnxGraph.input(j).type().tensor_type().elem_type();
    }
  }

  for(int j = 0; j < onnxGraph.value_info_size(); j++)
  {
    if(onnxGraph.value_info(j).name().compare(inputName) == 0)
    {
      return onnxGraph.value_info(j).type().tensor_type().elem_type();
    }
  }

  return -1;
}

vector<vector<int32_t>> TidlParseOnnx::getNodeInputOutputTypes(GraphProto& onnxGraph, int32_t idx)
{
  vector<vector<int32_t>> dataTypes;
  vector<int32_t> types;

  /* element type of inputs */
  for(int i = 0; i < onnxGraph.node(idx).input_size(); i++)
  {
    std::string name = onnxGraph.node(idx).input(i);
    name = getOriginalNameFromDuplicateName(name);
    /*  If the input node is not a dequantize node then set the name value to the original name */
    int32_t inputIdx = getNodeIdx(onnxGraph, name.c_str());
    if (inputIdx != -1 && strcmp(onnxGraph.node(inputIdx).op_type().c_str(), "DequantizeLinear") != 0)
    {
      name = onnxGraph.node(idx).input(i);
    }

    int32_t found = 0;
    for (int j = 0; j < onnxGraph.initializer_size(); j++)
    {
      if (onnxGraph.initializer(j).name().compare(name) == 0)
      {
        int32_t elemType = onnxGraph.initializer(j).data_type();
        types.push_back(elemType);
        found = 1;
        break;
      }
    }

    if(found == 0)
    {
      for (int j = 0; j < onnxGraph.input_size(); j++)
      {
        if(onnxGraph.input(j).name().compare(name) == 0)
        {
          int32_t elemType = onnxGraph.input(j).type().tensor_type().elem_type();
          types.push_back(elemType);
          found = 1;
          break;
        }
      }
    }

    if(found == 0)
    {
      for (int j = 0; j < onnxGraph.value_info_size(); j++)
      {
        if(onnxGraph.value_info(j).name().compare(name) == 0)
        {
          int32_t elemType = onnxGraph.value_info(j).type().tensor_type().elem_type();
          types.push_back(elemType);
          found = 1;
          break;
        }
      }
    }

    if(found == 0)
    {
      for (int j = 0; j < onnxGraph.output_size(); j++)
      {
        if(onnxGraph.output(j).name().compare(name) == 0)
        {
          int32_t elemType = onnxGraph.output(j).type().tensor_type().elem_type();
          types.push_back(elemType);
          found = 1;
          break;
        }
      }
    }
    if((found == 0) && (checkShapeInferenceforOnnx(layer.allowlistingMetaData) == TIDL_IMPORT_DIAGNOSIS_RETURN_OK))
    {
      if (name != "")
      {
        TIDL_GLOBAL_REPORT_WARNING("Data type not found for input '%s' of node %s. Setting default data type as float", name.c_str(), onnxGraph.node(idx).name().c_str());
      }
      for(auto i2 : typesTable)
      {
        if(i2.second == TIDL_SinglePrecFloat)
        {
          types.push_back(i2.first); // setting this datatype to default value which is float
          break;
        }
      }
      found = 1;
    }
  }

  dataTypes.push_back(types);
  types.clear();

  /* element type of outputs */
  for(int i = 0; i < onnxGraph.node(idx).output_size(); i++)
  {
    std::string outputName = onnxGraph.node(idx).output(i);
    int32_t found = 0;
    for (int j = 0; j < onnxGraph.output_size(); j++)
    {
      if(onnxGraph.output(j).name().compare(outputName) == 0)
      {
        int32_t elemType = onnxGraph.output(j).type().tensor_type().elem_type();
        types.push_back(elemType);
        found = 1;
        break;
      }
    }

    if(found == 0)
    {
      for (int j = 0; j < onnxGraph.value_info_size(); j++)
      {
        if(onnxGraph.value_info(j).name().compare(outputName) == 0)
        {
          int32_t elemType = onnxGraph.value_info(j).type().tensor_type().elem_type();
          types.push_back(elemType);
          found = 1;
          break;
        }
      }
    }

    if(found == 0)
    {
      /** If data type still not found, try to see if we can infer this from input data type */
      for(const auto& rule : outputTypeInferenceRules)
      {
        if(rule.opType == onnxGraph.node(idx).op_type() && rule.outputIdx == i)
        {
          int32_t inputDataType = getInputDataType(onnxGraph, idx, rule.inputIdx);
          if(inputDataType > 0)
          {
            types.push_back(inputDataType);
            found = 1;
            break;
          }
        }
      }
    }

    if((found == 0) && (checkShapeInferenceforOnnx(layer.allowlistingMetaData) == TIDL_IMPORT_DIAGNOSIS_RETURN_OK))
    {
      if (onnxGraph.node(idx).output(i) != "")
      {
        TIDL_GLOBAL_REPORT_WARNING("Data type not found for output '%s' of node %s. Setting default data type as float", onnxGraph.node(idx).output(i), onnxGraph.node(idx).name().c_str());
      }
      for(auto i2 : typesTable)
      {
        if(i2.second == TIDL_SinglePrecFloat)
        {
          types.push_back(i2.first); // setting this datatype to default value which is float
          break;
        }
      }
      found = 1;
    }
  }

  dataTypes.push_back(types);

  for(auto &types: dataTypes)
  {
    for(auto &type: types)
    {
      type = getTIDLDataTypeFromOnnxDataType (type);
    }
  }

  return dataTypes;
}

vector<int32_t> TidlParseOnnx::getNodeInputShape(GraphProto& onnxGraph, const string name, int32_t debugLevel)
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

vector<int32_t> TidlParseOnnx::getNodeOutputShape(GraphProto& onnxGraph, const string name, int32_t debugLevel)
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
          }
        }
      }
    }
  }

  return dims;
}

int32_t TidlParseOnnx::getIntAttr(const NodeProto& node, char * name, int32_t * valuePtr, int32_t idx)
{
  int32_t i = getAttrIdx(node, name);
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

int32_t TidlParseOnnx::getStringAttr(const NodeProto& node, char * name, char * valuePtr, int32_t idx)
{
  strcpy(valuePtr, "");
  int32_t i = getAttrIdx(node, name);
  if (i != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
      strcpy(valuePtr, node.attribute(i).s().c_str());
      return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
}

int32_t TidlParseOnnx::getTensorDim(TensorProto& tensor, int32_t idx)
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

int32_t TidlParseOnnx::copyTensor(GraphProto& onnGraph, TensorProto& tensor, void ** ptr, int32_t * size, const string name, int32_t inputRequired)
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
  else if (tensor.data_type() == TensorProto_DataType_INT64) //DT_INT64
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
  else if(isTensorDQ(onnGraph, tensor))
  {
      /*Get DQ Node:*/
      bool tensorScaleFromRaw = false;
      bool tensorZPFromRaw = false;
      int32_t DQNodeIdx = getNodeIdxFromInput(onnGraph, tensor.name().c_str());
      if(DQNodeIdx == -1)
      {
        TIDL_GLOBAL_REPORT_UNSUPPORTED("Unsupported data type for onnxImport");
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      int32_t status = 0;
      TensorProto tensorScales  = getInitializerTensor(onnGraph, onnGraph.node(DQNodeIdx).input(1), DQNodeIdx, status);
      if(status != 0)
      {
        if(inputRequired == INPUT_REQUIRED)
        {
          TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Unable to find initializer at index %d for node %s", 1, onnGraph.node(DQNodeIdx).name().c_str());
        }
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
      TensorProto tensorZeroPoints;
      int32_t zeroPointStatus = -1;
      if(onnGraph.node(DQNodeIdx).input_size() > 2)
      {
        tensorZeroPoints = getInitializerTensor(onnGraph, onnGraph.node(DQNodeIdx).input(2), DQNodeIdx, zeroPointStatus);
      }
      tensorScaleFromRaw = tensorScales.float_data_size() == 0;
      if(zeroPointStatus == 0)
      {
        tensorZPFromRaw = tensorZeroPoints.int32_data_size() == 0;
      }
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
        if(zeroPointStatus != 0 || TensorProto_DataType_INT32 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            int32_t tensorZeroPoint;
            float  tensorScale;

            if(zeroPointStatus != 0)
            {
              tensorZeroPoint = 0;
            }
            else if(tensorZPFromRaw)
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
        if(zeroPointStatus != 0 || TensorProto_DataType_INT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            int8_t tensorZeroPoint;
            float  tensorScale;

            if(zeroPointStatus != 0)
            {
              tensorZeroPoint = 0;
            }
            else if(tensorZPFromRaw)
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
        else if(zeroPointStatus != 0 || TensorProto_DataType_UINT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            uint8_t tensorZeroPoint;
            float  tensorScale;

            if(zeroPointStatus != 0)
            {
              tensorZeroPoint = 0;
            }
            else if(tensorZPFromRaw)
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
        if(zeroPointStatus != 0 || TensorProto_DataType_UINT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            uint8_t tensorZeroPoint;
            float  tensorScale;

            if(zeroPointStatus != 0)
            {
              tensorZeroPoint = 0;
            }
            else if(tensorZPFromRaw)
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
        else if(zeroPointStatus != 0 || TensorProto_DataType_INT8 == tensorZeroPoints.data_type())
        {
          for(i = 0; i < numOutChannels; i++)
          {
            int8_t tensorZeroPoint;
            float  tensorScale;

            if(zeroPointStatus != 0)
            {
              tensorZeroPoint = 0;
            }
            else if(tensorZPFromRaw)
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
      }
  }
  else if (tensor.data_type() == TensorProto_DataType_INT32) //DT_INT32
  {
    int32_t *dst;
    for (i = 0; i < tensor.dims_size(); i++)
    {
      tensorSize *= tensor.dims(i);
    }
    *size = tensorSize;
    dst = (int32_t*)my_malloc((*size) * sizeof(int32_t));

    if (tensor.int32_data_size() > 0)
    {
      if (tensor.int32_data_size() != tensorSize)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Tensor size and Dims size is not matching");
      }

      for (i = 0; i < tensorSize; i++)
      {
        dst[i] = tensor.int32_data(i);
      }
    }
    else if (tensor.raw_data().size() > 0)
    {
      if ((tensor.raw_data().size() / sizeof(int32_t)) != tensorSize)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Tensor size and Dims size is not matching");
      }
      memcpy(dst, (int32_t *)tensor.raw_data().c_str(), tensor.raw_data().size());
    }
    *ptr = dst;
    return 0;
  }
  else
  {
    TIDL_GLOBAL_REPORT_UNSUPPORTED("Unsupported Onnx import data type : %d ", tensor.data_type());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}


bool TidlParseOnnx::isTensorDQ(GraphProto& onnGraph, TensorProto& tensor)
{
  bool isDQ = false;
  int32_t dType = tensor.data_type();
  if(dType == TensorProto_DataType_INT32  ||
     dType == TensorProto_DataType_INT8   ||
     dType == TensorProto_DataType_UINT8)
  {
    isDQ = true;
  }

  int32_t DQNodeIdx = getNodeIdxFromInput(onnGraph, tensor.name().c_str());
  if(DQNodeIdx == -1 || strcmp(onnGraph.node(DQNodeIdx).op_type().c_str(), "DequantizeLinear") != 0)
  {
    isDQ = false;
  }

  return isDQ;
}

int32_t TidlParseOnnx::getNodeIdxFromInput(GraphProto& onnxGraph, const char *bufName)
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

/* inputRequired - if given 0 then don't print message, will use default values */
int32_t TidlParseOnnx::copyFloatConst(GraphProto& onnGraph, int32_t nIdx, int32_t inIdx, sBuffer_t &buf, int32_t inputRequired)
{
  if (onnGraph.node(nIdx).input_size() > inIdx)
  {
    int32_t status = 0;
    TensorProto tensor = getInitializerTensor(onnGraph, onnGraph.node(nIdx).input(inIdx), nIdx, status);
    if(status != 0)
    {
      if(inputRequired == INPUT_REQUIRED)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Unable to find initializer at index - %d for node %s", inIdx, onnGraph.node(nIdx).name().c_str());
      }
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    return copyTensor(onnGraph, tensor, &buf.ptr, &buf.bufSize, onnGraph.node(nIdx).input(inIdx), inputRequired);
  }
  else
  {
    if(inputRequired == INPUT_REQUIRED)
    {
      TIDL_GLOBAL_REPORT_ERROR("Requested constant tensor %d in %s is not found", inIdx, onnGraph.node(nIdx).name().c_str());
    }
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  return 0;
}

int32_t TidlParseOnnx::getFloatAttr(const NodeProto& node, char * name, float * valuePtr, int32_t idx)
{
  int32_t i = getAttrIdx(node, name);
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

/** Broadcast floating point vector buffer to have a modified dimension */
void TidlParseOnnx::broadcastFloatTensor(sBuffer_t &buf, int32_t bufSize)
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

int32_t TidlParseOnnx::getConstSize (TensorProto& tensor)
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

int32_t TidlParseOnnx::containsSuffix(std::string str, std::string suffix)
{
  if (str.size() >= suffix.size() &&
      str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0)
  {
    return 1;
  }
  return 0;
}

 int32_t TidlParseOnnx::convertIndicesToInt32(sBuffer_t& weights)
{
    
        int32_t *index_dst_ptr = (int32_t*)my_malloc(sizeof(int32_t) * weights.bufSize);
        if (index_dst_ptr == NULL)
        {
            TIDL_LOG_ERROR(gDiags.gDiagList, "ScatterElementsLayer  : Could not allocate memory for ScatterElements constant indices");
            return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        
        int64_t *index_src_ptr = (int64_t*)weights.ptr;
        int64_t val = 0;
        for (int32_t j = 0; j < weights.bufSize; j++)
        {
            val = index_src_ptr[j];
            index_dst_ptr[j] = (int32_t)val;
        }
        my_free(index_src_ptr);
        weights.ptr = (void*)index_dst_ptr;
    
    return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}

bool TidlParseOnnx::isLayerIONumSupported(sTIDL_LayerPC_t &layer)
{
  bool isLayerSupported = true;
  sTIDL_allowlistingMetaData& md = layer.allowlistingMetaData;

  if (md.numInputs > TIDL_NUM_IN_BUFS)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported number of input tensors (> %d)", (char *)layer.name, TIDL_NUM_IN_BUFS);
    isLayerSupported = false;
  }
  else if (md.numOutputs > TIDL_NUM_OUT_BUFS)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported number of output tensors (> %d)", (char *)layer.name, TIDL_NUM_OUT_BUFS);
    isLayerSupported = false;
  }

  return isLayerSupported;
}

bool TidlParseOnnx::isLayerIODataTypesSupported(sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  bool isLayerSupported = true;
  sTIDL_allowlistingMetaData& md = layer.allowlistingMetaData;
  std::vector<int32_t> varTensorIndices = md.varTensorIndices;
  bool isDataTypeSupported = true;

  for(int32_t i = 0; i < md.numVarInputs; i++)
  {
    if((md.inputDataTypes.size() > i) && (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.inputDataTypes[varTensorIndices[i]]) != unsupportedDataTypes.end()))
    {
      isDataTypeSupported = false;
    }
  }
  for(int32_t i = 0; i < md.numOutputs; i++)
  {
    if((md.outputDataTypes.size() > i) && (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.outputDataTypes[i]) != unsupportedDataTypes.end()))
    {
      isDataTypeSupported = false;
    }
  }

  if(!isDataTypeSupported)
  {
    //check for layer type
    if(find(supportedLayerTypes.begin(), supportedLayerTypes.end(), layer.layerType) == supportedLayerTypes.end())
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported data type for one of the input/output tensors", (char *)layer.name);
      status = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    else
    {
      // have the layer level checks
      if(layer.layerType == TIDL_GatherLayer || layer.layerType == TIDL_GatherNDLayer || layer.layerType == TIDL_GatherElementsLayer)
      {
        // If Data is a variable input, it should not be int64/uint64 dataType
        if((varTensorIndices[0] == 0) &&  // This incdicateds data is a variable tensors
           (md.inputDataTypes.size() > varTensorIndices[0]) &&
           (md.inputDataTypes[varTensorIndices[0]] == TIDL_SignedDoubleWord || md.inputDataTypes[varTensorIndices[0]] == TIDL_UnsignedDoubleWord))
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported data type for input tensor %d", (char *)layer.name, varTensorIndices[0]);
          status = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else if(layer.layerType == TIDL_ArgOpLayer)
      {
        // variable input should not be in unsupported input list
        if((md.inputDataTypes.size() > varTensorIndices[0]) && (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.inputDataTypes[varTensorIndices[0]]) != unsupportedDataTypes.end()))
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported data type for input tensor %d", (char *)layer.name, varTensorIndices[0]);
          status = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else if(layer.layerType == TIDL_TopKLayer)
      {
        //Indices output for the case of TopK Layer can be in INT64 but the values and input cannot be 
        if(((md.inputDataTypes.size() > varTensorIndices[0]) && (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.inputDataTypes[varTensorIndices[0]]) != unsupportedDataTypes.end())) || 
            ((md.outputDataTypes.size() > 0) && (find(unsupportedDataTypes.begin(), unsupportedDataTypes.end(), md.outputDataTypes[0]) != unsupportedDataTypes.end())))
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported data type for input tensor %d or output tensor 0", (char *)layer.name, varTensorIndices[0]);
          status = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      else if(layer.layerType == TIDL_EltWiseLayer)
      {
        // int32 and int64 dataType are supported for Mod operator
        if(layer.layerParams.eltWiseParams.eltWiseType == TIDL_EltWiseMod)
        {
          for(int32_t i = 0; i < md.inputDataTypes.size(); i++)
          {
            if(md.inputDataTypes[i] != TIDL_SignedDoubleWord && md.inputDataTypes[i] != TIDL_SignedWord)
            {
              TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported data type for input tensor %d", (char *)layer.name, varTensorIndices[i]);
              status = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
              break;
            }
          }
        }
        else
        {
          // Not supported for other eltwise operations
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Layer %s : Unsupported data type for one of the input/output tensors", (char *)layer.name);
          status = TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
      }
      // else layer is supported
    }
  }

  if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    isLayerSupported = false;
  }

  return isLayerSupported;
}