/*
*
* Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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


#include "tidl_parse_tvm.h"
#include "tidl_custom_import.h"
#include "tidl_parse_tvm_jumptable.h"
#include "tidl_runtimes_infer_common.h"
#include <tvm/relay/function.h>

// // #include "tidlParseTVM/tidl_parse_tvm.h"

// #define __MODULE__ "[PARSER]"

extern int32_t gloab_data_format;

// TVM converts some operators to different names, so we need to map them to TIDL supported ops.
std::unordered_map<string, string> tvm_tidl_op_map =
{
  {"adaptive_avg_pool1d", "global_avg_pool2d"},
  {"adaptive_avg_pool3d", "global_avg_pool2d"}
};

TidlParseTVM::TidlParseTVM(relay::Call call, int32_t index, sTIDL_LayerPC_t layerPC) :
     call(call), index(index), layer(layerPC)
{
    std::string op_name = TIDL_relayGetOpName(call);
    int32_t num_inputs = call->args.size();

  status = saveAllowlistingMetaData(call,num_inputs);
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Error while saving allowlisting meta data");
    layer.layerType = TIDL_UnsupportedLayer;
    layer.parseStatus = TIDL_ParseFailed;
    return;
  }

  // #######################
  // Adding things from relayMapNode
  string op_name_str = op_name;
  op_name_str = (op_name_str.substr(0,2)=="nn")?op_name_str.substr(3):op_name_str;
  op_name_str = (op_name_str.substr(0,3)=="qnn")?op_name_str.substr(4):op_name_str;
  op_name_str = (op_name_str.substr(0,5)=="image")?op_name_str.substr(6):op_name_str;
  // #######################

  // Map operator names set by TVM to TIDL supported ops
  auto op_name_tidl = tvm_tidl_op_map.find(op_name_str);

  if (op_name_tidl != tvm_tidl_op_map.end())
  {
    op_name_str = op_name_tidl->second;
  }

  auto jumpTableEntry = tidlParseTVMJumpTable.find(op_name_str);

  if (jumpTableEntry != tidlParseTVMJumpTable.end())
  {
    status = (this->*(jumpTableEntry->second))(); // Call the parse function

    if (status == -1)
    {
      layer.parseStatus = TIDL_ParseFailed;
    }
    else
    {
      layer.parseStatus = TIDL_ParsePassed;
    }
  }
  // else if (gParams.enableCustomLayers)
  // {
  //   TIDL_MapCustomParamsOnnx( layer,
  //                             index,
  //                             &onnxGraph);
  // }
  else
  {
    layer.layerType = TIDL_UnsupportedLayer;
    status = TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED;
  }

}


/** Function to save layer level tensor properties metadata to be used for allowlisting
 * Saves data for sTIDL_allowlistingMetaData structure
 */
int32_t TidlParseTVM::saveAllowlistingMetaData(const relay::Call call, int32_t numInputs)
{
  /*
  Form onnx 1.15, if DQ layer might be duplicated. The node name of duplicated
  node is appended with "/duplicate". Get metadata from original dq layer since
  shape inference wouldn't have run on the duplicated.
  */
  auto & almd = layer.allowlistingMetaData;
  Array<Expr> inputs = call->args;
  if (call->args[0].as<TupleNode>())
  {
   inputs = call->args[0].as<TupleNode>()->fields;
   numInputs = inputs.size();
  }
  if (numInputs == 0)
  {
    numInputs = inputs.size();
  }
  int32_t numOutputs = 1;
  if (call->checked_type().as<TupleTypeNode>())
  {
    numOutputs = call->checked_type().as<TupleTypeNode>()->fields.size();
  }
  else if (call->checked_type().as<TensorTypeNode>())
  {
    numOutputs = 1;
  }
  else
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Unsupported output type");
    return TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED;
  }
  almd.numInputs = almd.numValidInputs = numInputs;
  almd.numOutputs = numOutputs;
  almd.validInputIndices = {};
  almd.varTensorIndices.clear();
  almd.constTensorIndices.clear();
  almd.varTensorsDims.clear();
  almd.constTensorsDims.clear();
  almd.outputTensorDims.clear();
  almd.inputDataTypes.clear();
  almd.outputDataTypes.clear();
  for (int i = 0; i < numInputs; i++)
  {
    std::vector<int32_t> dims = TIDL_relayNormalizeShape(inputs[i]);
    auto tensorType = inputs[i]->checked_type().as<TensorTypeNode>();
    int32_t tidlType = tensorType ? getTIDLDataTypeFromTVMDataType((int32_t)tensorType->dtype.code()) : -1;
    almd.inputDataTypes.push_back(tidlType);
    if(inputs[i].as<ConstantNode>())
    {
      almd.constTensorIndices.push_back(i);
      almd.constTensorsDims.emplace_back(dims);
    }
    else
    {
      almd.varTensorIndices.push_back(i);
      almd.varTensorsDims.emplace_back(dims);
    }
  }
  almd.numVarInputs = almd.varTensorIndices.size();
  almd.numConstInputs = almd.constTensorIndices.size();

  if (numOutputs > 1 && call->checked_type().as<TupleTypeNode>())
  {
    auto tuple_type = call->checked_type().as<TupleTypeNode>();
    for (int i = 0; i < tuple_type->fields.size(); i++)
    {
      if (tuple_type->fields[i].as<TensorTypeNode>())
      {
        Array<PrimExpr> shape = tuple_type->fields[i].as<TensorTypeNode>()->shape;
        std::vector<int32_t> tidl_shape;
        for (int j = 0; j < shape.size(); j++)
        {
          tidl_shape.push_back(shape[j].as<IntImmNode>()->value);
        }
        almd.outputTensorDims.emplace_back(tidl_shape);
        almd.outputDataTypes.push_back(getTIDLDataTypeFromTVMDataType(
          (int32_t)tuple_type->fields[i].as<TensorTypeNode>()->dtype.code()));
      }
      else
      {
        almd.outputTensorDims.emplace_back(std::vector<int32_t>());
        almd.outputDataTypes.push_back(-1);
      }
    }
  }
  else
  {
    almd.outputTensorDims.emplace_back(TIDL_relayNormalizeShape(call));
    auto outTensorType = call->checked_type().as<TensorTypeNode>();
    almd.outputDataTypes.push_back(outTensorType ?
      getTIDLDataTypeFromTVMDataType((int32_t)outTensorType->dtype.code()) : -1);
  }
  // if the output dims is 0 then donot offload to TIDL
  if(numOutputs==1 &&almd.outputTensorDims[0].size()==0 ) 
  {
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
  
}