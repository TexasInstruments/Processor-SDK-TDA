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

#include <typeinfo>

//! @file tidl_relayImport.cpp

// Standard header files
#include <stdint.h>
#include <stdio.h>
#include <float.h>

// TVM header files
#include <tvm/relay/expr.h>
#include <tvm/relay/function.h>
#include <tvm/relay/attrs/nn.h>
#include <tvm/relay/attrs/image.h>
#include <tvm/relay/attrs/transform.h>
#include <tvm/relay/attrs/reduce.h>
#include <tvm/runtime/ndarray.h>
#include <tvm/runtime/registry.h>
#include <tvm/runtime/ndarray.h>
#include <tvm/tir/op.h>

// TIDL header files
#include <tidl_import_config.h>
#include <tidl_import_common.h>
#include <tidl_import_common_model_check.h>
#include <tidl_import_api.h>
#include <tidl_runtimes_import_common.h>
#include "tidl_meta_arch.pb.h"

#include "tidlParseTVM/tidl_parse_tvm.h"
#include "tidlAllowlistingConstraints/tidl_constraint.h"

#define __MODULE__ "[PARSER]"

using namespace tvm;
using namespace tvm::relay;
extern int32_t gloab_data_format;
extern uint8_t tidl_OdOutputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_STRING_SIZE];
TIDL_osrtOptions * relayOptions = new TIDL_osrtOptions;

#define MAX_NAME_LEN 512
extern sTIDL_runtimesImportState_t runtimes_import_state;
int32_t gNumInputDataLayers;
/* Creating this vector to deny offload of TVM decomposed operators */
std::vector<std::string> deny_list_layer_name_composite;
/* Same data structure defined in python/tvm/relay/backend/contrib/tidl.py
*  so that it can be passed from python to C/C++
*/
typedef struct InOutNodes
{
  char *this_node;
  int   num_in_nodes;
  int   num_out_nodes;
  void *in_nodes;
  void *out_nodes;
} InOutNodes_t;

typedef struct ODPostProcInfo
{
  char  in_node_names[64][512];
  int   num_in_nodes;
  int   num_out_nodes;
  TensorDescriptor_t out_nodes[64];
} ODPostProcInfo_t;

static tidl_import_config relay_import_init_params;

// Functions
static int32_t TIDL_relayImportGetNewLayerIndex()
{
  int32_t index = runtimes_import_state.layerIndex++;
  if (index >= TIDL_NUM_MAX_PC_LAYERS)
  {
    TIDL_GLOBAL_REPORT_ERROR("TIDL cannot import more than %d layers", TIDL_NUM_MAX_PC_LAYERS);
    index = -1;
  }
  return index;
}

static int32_t TIDL_relayImportGetNewDataIndex()
{
  return runtimes_import_state.dataIndex++;
}

// Determine the dimensionality of a Relay Tensor expression. If the expression
// is a Tuple, use the type of the first subexpression
int32_t TIDL_relayExprDims(Expr expr)
{
  const Type& inDataType = expr->checked_type();
  const TensorTypeNode* inTensorType = inDataType.as<TensorTypeNode>();
  if (!inTensorType && inDataType.as<TupleTypeNode>())
    inTensorType = inDataType.as<TupleTypeNode>()->fields[0].as<TensorTypeNode>();

  if (!inTensorType)
  {
    TIDL_GLOBAL_REPORT_ERROR("Unknown type in ExprDims");
    return 0;
  }

  Array<PrimExpr> inShape = inTensorType->shape;
  int dims = inShape.size();
  return dims;
}

// Normalize an axis parameter to a positive value (negative values count
// from the rightmost dimension), and convert to N12CHW layout.
// Note that TIDL tensor is 6 dims now, N12CHW
//Note if mode is set to 0(default) conversion from NHWC to N12CHW if 1 NHWC to NCHW
int32_t TIDL_relayNormalizeAxis(int32_t axis, int32_t dims,
                                       int32_t *relay_axis, int32_t mode)
{
  if (axis < 0)
    axis += dims;

  if (relay_axis != nullptr)  *relay_axis = axis;

  // Don't normalize axis unless tensor is 4-D
  // if (dims < 4)
  //   return axis;
  if(mode==1)
  {
    if(dims==4)
    {
      //NHWC -->NCHW
      if (gloab_data_format == 0)
      {
        axis = axis == 1 ? 2 :  // H
                axis == 2 ? 3 :   // W
                axis == 3 ? 1 :   // C
                axis;
      }
        // NCHW --> NCHW  Do nothing

    }
  }
  else
  {
    // NHWC --> N12CHW
    if (gloab_data_format == 0)
    {
      axis = axis == 1 ? TIDL_DIM_HEIGHT :  // H
              axis == 2 ? TIDL_DIM_WIDTH :   // W
              axis == 3 ? TIDL_DIM_NUMCH :   // C
              axis;
    }
    else  // NCHW --> N12CHW
    {
      axis += (TIDL_DIM_MAX - dims);
    }
  }
  // TODO: error checking
  return axis;
}


// Normalize a tensor's shape to NCHW layout (if 4 dim) for allowlistingMetaData
std::vector<int32_t> TIDL_relayNormalizeShape(Expr e)
{  std::vector<int32_t> tidl_shape;
  if(e->checked_type().as<TensorTypeNode>())
  {
    Array<PrimExpr> shape = e->checked_type().as<TensorTypeNode>()->shape;

    for (int i = 0; i < shape.size(); i++)
    {
      tidl_shape.push_back(shape[i].as<IntImmNode>()->value);
    }
    if (shape.size() == 4 && gloab_data_format == 0)  // NHWC -> NCHW
    {
      int32_t ch = tidl_shape[3];
      tidl_shape[3] = tidl_shape[2];
      tidl_shape[2] = tidl_shape[1];
      tidl_shape[1] = ch;
    }
  }
  return tidl_shape;
}


// Ensure tensors have equal shapes, for elementwise operations
static bool TIDL_relayCheckShapesEqual(Expr e1, Expr e2)
{
  Array<PrimExpr> shape1 = e1->checked_type().as<TensorTypeNode>()->shape;
  Array<PrimExpr> shape2 = e2->checked_type().as<TensorTypeNode>()->shape;

  bool ok = shape1.size() == shape2.size();
  if (ok)
    for (int i = 0; i < shape1.size(); ++i)
      if (shape1[i].as<IntImmNode>()->value != shape2[i].as<IntImmNode>()->value)
	ok = false;
  if (!ok)
    RelayDebugPrint("elementwise operator: shapes must agree\n");
  return ok;
}

// Helper to convert a float constant to a constant data pointer, by
// replicating it 'nelem' times
static float* floatConstant(float value, int nelem)
{
  int32_t size = nelem * sizeof(float);
  float* data = (float *)my_malloc(size);
  for (int i = 0; i < nelem; ++i)
    data[i] = value;
  return data;
}
sBuffer_t TIDL_extractConstantTensorData(const relay::Call& call, int arg_index,
                                        const sTIDL_allowlistingMetaData& md, int const_index,int ndarr_zp, float ndarr_scale)
{
    sBuffer_t buffer;
    buffer.ptr = NULL;
    buffer.bufSize = 0;
    if (call->args[arg_index].as<tvm::relay::ConstantNode>())
    {
        string data_type = DLDataType2String(call->args[arg_index]->checked_type().as<TensorTypeNode>()->dtype);
        int32_t numElements = 1;

        for (int i = 0; i < md.constTensorsDims[const_index].size(); i++)
        {
            numElements *= md.constTensorsDims[const_index][i];
        }
        auto tensorExpr = call->args[arg_index].as<tvm::relay::ConstantNode>()->data;

        if (data_type == "int64")
        {
            buffer.ptr = NDArrtoInt64(tensorExpr, numElements);
        }
        else
        {
            buffer.ptr = NDArrtoFloat(tensorExpr, numElements, ndarr_zp, ndarr_scale);
        }

        buffer.bufSize = numElements;
    }

    return buffer;
}

// Helper to convert NDArray to constant data pointer. If the NDArray is a
// scalar, replicate it.
static size_t DLDataTypeBytes(std::string& data_type)
{
  if (data_type == "uint8" || data_type == "int8")  return 1;
  if (data_type == "float32")                       return 4;
  if (data_type == "int32")                         return 4;
  if (data_type == "int64")                         return 8;
  TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Unknown DLDataType size in NDArray: %s", data_type.c_str());
  return 0;
}

long long int* NDArrtoInt64(const runtime::NDArray &ndArr, int nelem)
{
  auto shape = ndArr.Shape();
  auto data_type = runtime::DLDataType2String(ndArr->dtype);
  size_t elem_bytes = DLDataTypeBytes(data_type);

  int32_t size = nelem * sizeof(long long int );
  int32_t ndarr_size = nelem * elem_bytes;
  long long int * data = (long long int  *) my_malloc(ndarr_size);
  ndArr.CopyToBytes(data, ndarr_size);

  if (data_type == "int64")
  {
    void* tmp_data = data;
    data = (long long int  *) my_malloc(size);
    for (int i = 0; i < nelem; i++)
    data[i] = (((long long int *) tmp_data)[i] );

    my_free(tmp_data);
  }

  return data;

}

float* NDArrtoFloat(const runtime::NDArray &ndArr, int nelem,
                           int ndarr_zp, float ndarr_scale)
{
  auto shape = ndArr.Shape();
  auto data_type = runtime::DLDataType2String(ndArr->dtype);
  size_t elem_bytes = DLDataTypeBytes(data_type);
  if (shape.empty())
  {
    // replicate scalar value
    float value;
    ndArr.CopyToBytes(&value, elem_bytes);

    if (data_type == "uint8")
      value = (*((uint8_t *) &value) - ndarr_zp) * ndarr_scale;
    else if (data_type == "int8")
      value = (*((int8_t *)  &value) - ndarr_zp) * ndarr_scale;
    else if (data_type == "int32")
      value = (*((int32_t *) &value) - ndarr_zp) * ndarr_scale;

    return floatConstant(value, nelem);
  }

  int32_t size = nelem * sizeof(float);
  int32_t ndarr_size = nelem * elem_bytes;
  float* data = (float *) my_malloc(ndarr_size);
  ndArr.CopyToBytes(data, ndarr_size);

  if (data_type == "uint8" || data_type == "int8" || data_type == "int32")
  {
    void* tmp_data = data;
    data = (float *) my_malloc(size);

    if (data_type == "uint8")
      for (int i = 0; i < nelem; i++)
        data[i] = (((uint8_t *) tmp_data)[i] - ndarr_zp) * ndarr_scale;
    else if (data_type == "int8")
      for (int i = 0; i < nelem; i++)
        data[i] = (((int8_t *)  tmp_data)[i] - ndarr_zp) * ndarr_scale;
    else if (data_type == "int32")
      for (int i = 0; i < nelem; i++)
        data[i] = (((int32_t *) tmp_data)[i] - ndarr_zp) * ndarr_scale;

    my_free(tmp_data);
  }

  return data;
}

std::string TIDL_relayGetOpName(relay::Call call)
{
  std::string op_name = "";
  if(const auto* op_node = call->op.as<tvm::OpNode>()){
    op_name = GetRef<tvm::Op>(op_node)->name;
  }
  else if(const auto* fn_node = call->op.as<tvm::relay::FunctionNode>())
  {
    auto fn = GetRef<tvm::relay::Function>(fn_node);
    if(fn->attrs.defined()) {
      if (const auto* attrs = fn->attrs.as<DictAttrsNode>()) {
        if (attrs->dict.count("Composite")) {
          op_name = Downcast<String>(attrs->dict.at("Composite"));
        }
      }
    }
  }
  return op_name;
}

std::string TIDL_relayGetSpanName(relay::Call call)
{
  std::string span_name = "";
  if(call->span.defined() && call->span->source_name.defined()){
    span_name = call->span->source_name->name;
  }
  return span_name;
}

/*
   Helper function to verify that a constant matches the semantics for a
   broadcast on the channel axis.  Background: Relay's arithmetic operations
   use numpy broadcast semantics.  For example, a scalar or vector can be
   added to an array by broadcasting it along one or more axes. TIDL supports
   operations between tensors and constants as channel-wise normalization layers
   (biasAdd, batchNorm).  This function is used when we see a relay operator
   between a tensor and a constant, to make sure the constant matches this form.
*/
bool TIDL_RelayChannelBroadcast(Expr tensor, Expr constant, int32_t &numCh)
{
  Array<PrimExpr> inShape = tensor->checked_type().as<TensorTypeNode>()->shape;
  int32_t dims = inShape.size();

  if (dims != 4)
  {
    // TIDL channel-wise operations (e.g. BatchNorm, Bias) require 4D tensors
    RelayDebugPrint("Illegal broadcast: input must be 4D\n");
    return false;
  }

  // Determine channel axis, and number of channels of the input tensor
  int chIndex = gloab_data_format == 0 ? 3 : 1;
  numCh = inShape[chIndex].as<IntImmNode>()->value;

  // Analyze the dimensions of the constant
  auto constExpr = constant.as<tvm::relay::ConstantNode>()->data;
  Array<PrimExpr> constShape = constant->checked_type().as<TensorTypeNode>()->shape;

  // There are two valid shapes for the constant: scalar, or a vector which
  // broadcasts onto the channel axis.
  if (constShape.size() == 0)
    return true;

  // For a non-scalar constant, make sure it broadcasts onto the channel
  // axis. Relay uses numpy broadcast semantics.  The constant must have
  // dimension '1' along all axes except for the channel axis, which must
  // agree with the input tensor. Dimensions are compared from right to left.
  for (int32_t j = 1; j <= constShape.size(); ++j)
  {
     int32_t kIndex = constShape.size() - j;
     int32_t tIndex = inShape.size() - j;
     int32_t kdim = constShape[kIndex].as<IntImmNode>()->value;

     bool ok = (tIndex == chIndex) ? kdim == numCh : kdim == 1;
     if (!ok)
     {
       RelayDebugPrint("illegal broadcast\n");
       return false;
     }
  }
  return true;
}


/*
   Helper function to populate AllowlistingMetaData from a Relay IR Call node
*/
static void TIDL_relaySaveAllowlistingMetaData(sTIDL_LayerPC_t& layer, const relay::Call &call,
                                               int32_t numInputs = 0, int32_t numOutputs = 1)
{
  // Array<Expr> inputs = call->args or call->args[0].fields
  // for inputs:  check if VarNode, what about ConstantNode?
  // Set numOutputs to 1 for now, use fields[0] for size if Tuple node?

  auto & almd = layer.allowlistingMetaData;
  Array<Expr> inputs = call->args;
  if (call->args[0].as<TupleNode>())
  {
    inputs = call->args[0].as<TupleNode>()->fields;
  }
  if (numInputs == 0)
  {
    numInputs = inputs.size();
  }

  almd.numInputs = almd.numValidInputs = numInputs;
  almd.numOutputs = numOutputs;
  almd.validInputIndices = {};
  almd.varTensorIndices.clear();
  almd.constTensorIndices.clear();
  almd.varTensorsDims.clear();
  almd.constTensorsDims.clear();
  almd.outputTensorDims.clear();
  for (int i = 0; i < numInputs; i++)
  {
    std::vector<int32_t> dims = TIDL_relayNormalizeShape(inputs[i]);
    if(call->args[i].as<ConstantNode>())
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

  Array<Expr> outputs = { call };
  if (numOutputs > 1 && outputs.as<TupleNode>())
  {
    outputs = call.as<TupleNode>()->fields;
  }
  for (int i = 0; i < numOutputs; i++)
  {
    almd.outputTensorDims.emplace_back(TIDL_relayNormalizeShape(outputs[i]));
  }
}


/* Helper function to get scalar value from Relay scalar Constant */
template<typename Tvalue>
Tvalue TIDL_relayGetConstantVal(const relay::Expr &expr)
{
  Tvalue value;
  expr.as<ConstantNode>()->data.CopyToBytes(&value, sizeof(Tvalue));
  return value;
}

/* Helper function to get NDArray value from Relay NDArray Constant */
template<typename Tvalue>
Tvalue* TIDL_relayGetConstantArr(const relay::Expr &expr, int32_t &len)
{
  const DLTensor *dltensor = expr.as<ConstantNode>()->data.operator->();
  len = tvm::runtime::GetDataSize(*dltensor) / sizeof(Tvalue);
  return (Tvalue*) dltensor->data;
}

/*
  Infer the (float) min and max of a quantized tensor, assuming that all
  available bits are used to the full extent.  i.e. (float) min and max of
  the tensor map to the min and max of the quantized range.
  - Only 8-bit quantized model is supported for now.  We should only set
    activation for layers that output 8-bit tensors, not intermediate layers
    that output int32 tensors.
*/
void TIDL_relaySetActivation(int zp_len, int *zp, int scale_len, float *scale,
                             std::string out_type, sTIDL_LayerPC_t &layer)
{
  if (out_type != "int8" && out_type != "uint8")  return;
  if (zp_len != 1 || scale_len != 1)  return;

  float q_min = (out_type == "uint8") ?   0.0f : -128.0f;
  float q_max = (out_type == "uint8") ? 255.0f :  127.0f;

  layer.actParams.actType = TIDL_Clip;
  layer.actParams.clipMin = (q_min - zp[0]) * scale[0];
  layer.actParams.clipMax = (q_max - zp[0]) * scale[0];

  // When importing 8-bit pre-quantized models with asymmetric quantization
  // to 8-bit TIDL formats, we often need advanced quantization because
  // TIDL only supports symmetric weights and feature maps (for now).
  //     e.g. TFLite MobileNetV2
  // Note: Importing 8-bit pre-quantized model to 16-bit TIDL format does NOT
  //     require calibration, we can represent precisely
  // 8-bit case: temporarily forcing calibration with RelU6 actType
  // Keep ClipLayer's actType, so that it can be merged later
  // TODO: Use a flag to force calibration for network w/ all TIDL_Clip actType
  if (gParams.numParamBits <= 8)
    if (layer.actParams.clipMin == 0.0f &&
        layer.actParams.clipMax - 5.999 >= 0 &&
        layer.actParams.clipMax - 5.999 <= 0.01 &&
        layer.layerType != TIDL_ClipLayer)
    {
      layer.actParams.actType = TIDL_RelU6;
    }
}

// Initialize global config for TVM annotation or import
int32_t TIDL_relayInit(int32_t is_nchw, tvm::Map<tvm::String, tvm::String> options_dict)
{
  bool status;
  TIDL_setDefaultOptions(relayOptions);

  // Iterate through the dictionary and parse each option
  for (const auto& kv : options_dict) {
    std::string key = std::string(kv.first);
    std::string value = std::string(kv.second);
    status = TIDL_readInterfaceOptions(relayOptions, key, value, TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY);
    if(!status) return -1;
  }

  TIDL_derivedInterfaceOptions(relayOptions);
  status = TIDL_checkInterfaceOptions(relayOptions);
  if(!status) return -1;
    /* Set global import config parameters */
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runtimesGparamsInit(relayOptions, TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY), "");

  /* Set global tensor data layout format */
  gloab_data_format = is_nchw;  // 1 for NCHW, 0 for NHWC

  char *relay_import_debug = getenv("TIDL_RELAY_IMPORT_DEBUG");
  if (relay_import_debug != nullptr)
  {
    tidl_relay_debuglevel = atoi(relay_import_debug);
    std::stringstream(relay_import_debug) >> relayOptions->m_debug_level;
    relayOptions->osrtDebugPrintLevel = (relayOptions->m_debug_level == 0) ? 0 : 1;
    gParams.debugTraceLevel = std::min(tidl_relay_debuglevel, 3);
    gParams.writeTraceLevel = (tidl_relay_debuglevel > 3) ? 3 : 0;
  }

  TIDL_printCompilationStages();
  TIDL_printVersionTable(TIDL_TOOLS_VERSION, (char*)gParams.c7xFirmwareVersion);
  TIDL_printInterfaceOptions(relayOptions);

  /* Device specific allow-listing checks require deviceName to be populated */
  if (TIDL_readDeviceConfigParams((char*)gParams.perfSimConfig) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("");
  }

  relay_import_init_params = gParams;
  return 0;
}

// Mimic setup code in main() in tidl_import_main.cpp
int32_t TIDL_relayImportInit(int32_t subgraph_id,
                             int32_t num_inputs, int32_t num_outputs,
                             void* inout_dscrs,
                             int32_t is_nchw,
                             std::string tidl_tools_path,
                             std::string artifacts_temp_dir,
                             bool isSubgraphOD)
{
  RelayDebugPrint("In TIDL_relayImportInit subgraph_id=%d\n", subgraph_id);
  /* Reset all the memories to to NULL, there could be multiple subgraphs */
  memset(&orgTIDLNetStructure, 0, sizeof(sTIDL_OrgNetwork_t));
  memset(&tIDLNetStructure,    0, sizeof(sTIDL_Network_t));
  runtimes_import_state.layerIndex = 0;
  runtimes_import_state.dataIndex  = 0;

  gParams = relay_import_init_params;

  TensorDescriptor_t * input_descriptors = (TensorDescriptor_t *)inout_dscrs;
  for(int i = 0; i < num_inputs; i++)
  {
    gParams.inLayout[i] = (is_nchw == 1) ? TIDL_LT_NCHW : TIDL_LT_NHWC;
    gParams.inElementType[i] = input_descriptors[i].element_type;
    gParams.inQuantFactor[i] = input_descriptors[i].scale;
    gParams.inZeroPoint[i] = input_descriptors[i].zp;
    int32_t layerIndex = TIDL_addInputDataLayer6D(i, input_descriptors[i].dimValues[TIDL_DIM_BATCH], input_descriptors[i].dimValues[TIDL_DIM_DIM1], input_descriptors[i].dimValues[TIDL_DIM_DIM2], input_descriptors[i].dimValues[TIDL_DIM_NUMCH], input_descriptors[i].dimValues[TIDL_DIM_HEIGHT], input_descriptors[i].dimValues[TIDL_DIM_WIDTH],
                                                     (char*)input_descriptors[i].name);

    RelayDebugPrint("in[%d]: scale=%f zp=%d t=%d n=%d dim1=%d dim2=%d c=%d h=%d w=%d nchw=%d\n", i,
                    input_descriptors[i].scale, input_descriptors[i].zp,
                    input_descriptors[i].element_type,
                    input_descriptors[i].dimValues[TIDL_DIM_BATCH], input_descriptors[i].dimValues[TIDL_DIM_DIM1],
                    input_descriptors[i].dimValues[TIDL_DIM_DIM2], input_descriptors[i].dimValues[TIDL_DIM_NUMCH],
                    input_descriptors[i].dimValues[TIDL_DIM_HEIGHT], input_descriptors[i].dimValues[TIDL_DIM_WIDTH], is_nchw);

    sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
    layer.outData[0].tensorScale  = input_descriptors[i].scale;
    RelayDebugPrint("Layer %d, subgraph input %d, name=%s\n", layerIndex, i,
                    (char*)layer.outDataNames[0]);
    layer.outConsumerCnt[0] = 0;

    //if the input is int64 to skip quantization
    if(input_descriptors[i].element_type==TIDL_SignedDoubleWord){
       gParams.inElementType[i] =TIDL_SignedWord;
    }
    TIDL_updateNamesList ((char*)gParams.inDataNamesList, i, (char *)input_descriptors[i].name);
  }
  gNumInputDataLayers = num_inputs;

  TensorDescriptor_t * output_descriptors = input_descriptors + num_inputs;
  if(!isSubgraphOD)
  {
    for (int i = 0; i < num_outputs; i++)
    {
      gParams.outLayout[i] = (is_nchw == 1) ? TIDL_LT_NCHW : TIDL_LT_NHWC;
      gParams.outElementType[i] = output_descriptors[i].element_type;
      gParams.outTensorScale[i] = output_descriptors[i].scale;
      gParams.outZeroPoint[i] = output_descriptors[i].zp;
      RelayDebugPrint("out[%d]: scale=%f zp=%d t=%d nchw=%d\n", i,
                      output_descriptors[i].scale, output_descriptors[i].zp,
                      output_descriptors[i].element_type, is_nchw);
    }
  }
  else
  {
    for (int i = 0; i < num_outputs; i++)
    {
      gParams.outLayout[i] = (is_nchw == 1) ? TIDL_LT_NCHW : TIDL_LT_NHWC;
      gParams.outTensorScale[i] = 1.0;
      gParams.outZeroPoint[i] = 0.0;
      gParams.outElementType[i] = TIDL_SinglePrecFloat;
      RelayDebugPrint("out[%d]: scale=%f zp=%d t=%d nchw=%d\n", i,
                      output_descriptors[i].scale, output_descriptors[i].zp,
                      output_descriptors[i].element_type, is_nchw);
    }
  }

  gParams.addDataConvertToNet = (ADD_DC_LAYER_AT_INPUT | ADD_DC_LAYER_AT_OUTPUT);
  gParams.inFileFormat = 1;  // raw data

  sprintf(inConfigFilename,
          "%s/tidl_import_subgraph%d.txt", artifacts_temp_dir.c_str(), subgraph_id);
  sprintf((char *)gParams.outputNetFile,
          "%s/subgraph%d_net.bin", artifacts_temp_dir.c_str(), subgraph_id);
  sprintf((char *)gParams.outputParamsFile,
          "%s/subgraph%d_params_", artifacts_temp_dir.c_str(), subgraph_id);
  // TODO: windows path
  sprintf((char *)gParams.inData, "%s/calib_raw_data%d.bin", artifacts_temp_dir.c_str(), subgraph_id);

  strcpy((char*)gParams.modelDumpTool, tidl_tools_path.c_str());
  strcat((char*)gParams.modelDumpTool, "/tidl_dump.out");

  /* Initialize rest of the layers */
  for (int i = num_inputs; i < TIDL_NUM_MAX_PC_LAYERS; i++)
  {
    sTIDL_LayerPC_t& layer_i = orgTIDLNetStructure.TIDLPCLayers[i];
    layer_i.actParams.actType  = TIDL_NoAct;
    layer_i.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
  }

  if (tidlValidateImportParams(&gParams) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Validation of TIDL relay import config parameters failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

// TIDL does not support asymmetric padding, so in the case of
// asymmetric padding we take the larger of the two values. If asymmetric
// padding is introduced by the "SAME" padding option in TFLite, the extra
// column/row is on the right/bottom. In this case we need to change
// TIDL's strideOffsetMethod from "Left" to "Center".
int32_t balancePadding(sTIDL_LayerPC_t &layer,
                              int32_t before, int32_t after)
{
  if (after > before)
  {
    if (after == before + 1)
      layer.strideOffsetMethod = TIDL_StrideOffsetCenter;
    return after;
  }
  else return before;
}

// Helper for conv2d and conv2dTranspose
int32_t convImportHelper(sTIDL_LayerPC_t &layer, std::string kernel_layout,
                         std::string data_layout, std::string out_layout,
                         const int &groups,
                         const Array<IndexExpr> &strides,
                         const Array<IndexExpr> &padding,
                         const Array<IndexExpr> &dilation,
                         const Array<IndexExpr> &kernel_size,
                         const relay::Call &call,
                         int32_t w_zp_len = 1, int32_t *w_zp_ptr = nullptr,
                         int32_t w_scale_len = 1, float *w_scale_ptr = nullptr)
{
  sTIDL_ConvParams_t &convParams = layer.layerParams.convParams;
  convParams.numGroups =  groups;
  convParams.strideH = strides[0].as<IntImmNode>()->value;
  convParams.strideW = strides[1].as<IntImmNode>()->value;
  convParams.dilationH = dilation[0].as<IntImmNode>()->value;
  convParams.dilationW = dilation[1].as<IntImmNode>()->value;
  convParams.kernelH = kernel_size[0].as<IntImmNode>()->value;
  convParams.kernelW = kernel_size[1].as<IntImmNode>()->value;

  //layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
  //RelayDebugPrint("Weight dtype is %d\n", num);
  //layer.strideOffsetMethod = TIDL_StrideOffsetTopLeft;

  // Padding attr array [top, left, bottom, right]
  int32_t pad_t, pad_l, pad_b, pad_r = 0;
  if(padding.size() == 1)
  {
    pad_t = pad_l = pad_b = pad_r = padding[0].as<IntImmNode>()->value;
  }
  else if(padding.size() == 2)
  {
    pad_t = pad_b = padding[0].as<IntImmNode>()->value;
    pad_l = pad_r = padding[1].as<IntImmNode>()->value;
  }
  else
  {
    pad_t = padding[0].as<IntImmNode>()->value;
    pad_l = padding[1].as<IntImmNode>()->value;
    pad_b = padding[2].as<IntImmNode>()->value;
    pad_r = padding[3].as<IntImmNode>()->value;
  }

  convParams.padW = balancePadding(layer, pad_l, pad_r);
  convParams.padH = balancePadding(layer, pad_t, pad_b);
  convParams.padL = -1;
  convParams.padR = -1;
  convParams.padT = -1;
  convParams.padB = -1;
  if (convParams.padW == 0 && convParams.padH == 0)
  {
   convParams.padL = 0;
   convParams.padR = 0;
   convParams.padT = 0;
   convParams.padB = 0;
  }

  // Set input/output channels
  auto in_shape  = call->args[0]->checked_type().as<TensorTypeNode>()->shape;
  auto out_shape = call->checked_type().as<TensorTypeNode>()->shape;
  if (data_layout == "NCHW")
  {
    convParams.numInChannels  = in_shape[1].as<IntImmNode>()->value;
    convParams.numOutChannels = out_shape[1].as<IntImmNode>()->value;
  }
  else if (data_layout == "NHWC")
  {
    convParams.numInChannels  = in_shape[3].as<IntImmNode>()->value;
    convParams.numOutChannels = out_shape[3].as<IntImmNode>()->value;
  }
  else
  {
    RelayDebugPrint("Data layout %s not supported\n", data_layout.c_str());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  // Set weights
  auto weight = call->args[1];
  auto weight_checked_type = weight->checked_type().as<TensorTypeNode>();
  Array<PrimExpr> weight_shape = weight_checked_type->shape;
  auto weight_type = DLDataType2String(weight_checked_type->dtype);
  auto weight_data = weight.as<tvm::relay::ConstantNode>()->data;

  int shape0 = weight_shape[0].as<IntImmNode>()->value;
  int shape1 = weight_shape[1].as<IntImmNode>()->value;
  int shape2 = weight_shape[2].as<IntImmNode>()->value;
  int shape3 = weight_shape[3].as<IntImmNode>()->value;

  layer.weights.bufSize = shape0 * shape1 * shape2 * shape3;
  // Copy NDArray to Array pointer, scale later on output channel
  layer.weights.ptr     = NDArrtoFloat(weight_data, layer.weights.bufSize);

  // TIDL natively uses "OIHW" kernel/weights layout
  int num_o_chs = shape0;
  if (kernel_layout == "HWIO")
  {
    TIDL_kernelReshape((float*)layer.weights.ptr,
                       shape1, shape0, shape2, shape3);
    num_o_chs = shape3;
  }
  else if (kernel_layout == "HWOI")
  {
    TIDL_kernelReshapeHWOI((float*)layer.weights.ptr,
                           shape1, shape0, shape3, shape2);
    num_o_chs = shape2;
  }
  else if (kernel_layout == "IOHW")
  {
    TIDL_kernelReshapeIOHW((float*)layer.weights.ptr,
                           shape3, shape2, shape0, shape1);
    num_o_chs = shape1;
  }
  else if (kernel_layout != "OIHW")
  {
    RelayDebugPrint("Kernel layout %s not supported\n", kernel_layout.c_str());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  // scale on TIDL kernel weights "OIHW" layout
  float *weights = (float *) layer.weights.ptr;
  if (w_scale_ptr != nullptr)
  {
    for (int o_ch = 0; o_ch < num_o_chs; o_ch++)
    {
      int32_t zp = (w_zp_len == 1) ? w_zp_ptr[0] : w_zp_ptr[o_ch];
      float scale = (w_scale_len == 1) ? w_scale_ptr[0] : w_scale_ptr[o_ch];
      for (int i = 0; i < layer.weights.bufSize / num_o_chs; i++)
      {
        int id = o_ch * (layer.weights.bufSize / num_o_chs) + i;
        weights[id] = (weights[id] - zp) * scale;
      }
    }
  }

  convParams.enableBias      = 0;
  convParams.enableEWRelU    = 0;
  convParams.enablePooling   = 0;

  //strcpy((char*)layer.inDataNames[0], "???");   // can we get name?
  //strcpy((char*)layer.outDataNames[0], "???");  // can we get name?
  layer.inData[0].dimValues[TIDL_DIM_NUMCH] = convParams.numInChannels;
  layer.outData[0].dimValues[TIDL_DIM_NUMCH] = convParams.numOutChannels;

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_relayImportNode(relay::Call call, int zp_len, void *zp_ptr,
                             int scale_len, void *scale_ptr)
{
  RelayDebugPrint("In TIDL_relayImportNode: \n");
  std::string op_name = TIDL_relayGetOpName(call);
  int         num_args = call->args.size();
  int32_t     *zp = (int32_t *)zp_ptr;
  float       *scale = (float *)scale_ptr;
  RelayDebugPrint("node name: %s, op name: %s, num_args: %d\n", call->span->source_name->name.c_str(), op_name.c_str(), num_args);
  for (int i = 0; i < num_args; i++)
  {
    if (auto tensor = call->args[i]->checked_type().as<TensorTypeNode>())
    {
      std::stringstream dims_stream;
      dims_stream << tensor->shape;
      RelayDebugPrint("  args[%d] dims: %s\n", i, dims_stream.str().c_str());
    }
  }
  RelayDebugPrint("  zp_len=%d, scale_len=%d\n", zp_len, scale_len);
  RelayDebugPrint("  zp[0]=%d, scale[0]=%f\n", zp[0], scale[0]);

  // Get new layerIndex, dataIndex
  int32_t layerIndex = TIDL_relayImportGetNewLayerIndex();
  if (layerIndex < 0)  return -1;
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
  // set layer defaults
  layer.numInBufs         = 1;


  layer.allowlistingMetaData.numInputs = 0;

  int num_outputs = 0;

  Type ty = call->checked_type();
  if (const auto* tt = ty.as<TensorTypeNode>())
  {
    num_outputs = 1;
  } else if (const auto* tup = ty.as<TupleTypeNode>())
  {
    num_outputs = tup->fields.size();
  }
  // TIDL supports only single output for batch_norm, but TVM gives a tuple with 3 outputs
  if (op_name == "nn.batch_norm")
  {
    num_outputs = 1;
  }
  layer.numOutBufs        = num_outputs;
  for(int i=0;i<num_outputs;i++){
    layer.outData[i].dataId = TIDL_relayImportGetNewDataIndex();
  }

  static int default_zp = 0;
  static float default_scale = 1.0f;
  if (zp == nullptr) zp = &default_zp;
  if (scale == nullptr) scale = &default_scale;
  // intialize the const_input with name and dataID with -1
  int32_t numInputs = call->args.size();
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
  for (int i = 0; i < numInputs; i++)
  {
    if(inputs[i].as<ConstantNode>())
    {
        //name the constants
      sprintf((char *)layer.inDataNames[i], "const_%d_%d",layer.outData[0].dataId,i);
      // initialize the connections to -1
      layer.inData[i].dataId = -1;
    }
  }
  auto result = TidlParseTVM(call,0,layer);
  layer = result.layer;

  if (auto out_tensor_type = call->checked_type().as<TensorTypeNode>())
  {
    auto out_type = DLDataType2String(out_tensor_type->dtype);
    if (out_type != "float32")
        TIDL_relaySetActivation(zp_len, zp, scale_len, scale, out_type, layer);
  }
  return result.status;
  
}


// Import output data layer when a subgraph has multiple outputs.
// This import must be followed by TIDL_relayImportLinkNode()
int32_t TIDL_relayImportOutDataLayer(int num_inputs)
{
  RelayDebugPrint("In TIDL_relayImportOutDataLayer: %d inputs\n", num_inputs);
  // Get new layerIndex, dataIndex
  int32_t layerIndex = TIDL_relayImportGetNewLayerIndex();
  if (layerIndex < 0)  return -1;
  //int32_t dataIndex  = TIDL_relayImportGetNewDataIndex();
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];

  layer.layerType         = TIDL_DataLayer;
  layer.numInBufs         = num_inputs;
  layer.numOutBufs        = -1;
  //layer.outData[0].dataId = dataIndex;

  return 0;
}

// use node name in the subgraph as the unique data/tensor name for each node
static void TIDL_relayFillTensorName(int8_t *name, char * node_name,
                                     int suffix_index = 0)
{
  char str[MAX_NAME_LEN];
  if (suffix_index <= 0)
    snprintf(str, MAX_NAME_LEN, "%s", node_name);
  else
    snprintf(str, MAX_NAME_LEN, "%s:%d", node_name, suffix_index);
  strcpy((char *)name, str);
}

/* This function is equivalent to the following tfImport sequence:
*  - tidl_tfLayerFillTensorNames()
*  - tidl_tfLayerUpdateConsumerCount()
*  - tidl_linkInputTensors()
*  - tidl_linkOutputTensors()
*/
int32_t TIDL_relayImportLinkNode(void *in_out_indices)
{
  RelayDebugPrint("In TIDL_relayImportLinkNode: \n");
  int32_t layerIndex = runtimes_import_state.layerIndex - 1;  // current layer
  sTIDL_LayerPC_t& layer  = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
  InOutNodes_t *in_out = (InOutNodes_t *) in_out_indices;
  char **in_nodes  = (char **) in_out->in_nodes;
  char **out_nodes = (char **) in_out->out_nodes;
  char    str[MAX_NAME_LEN];

  RelayDebugPrint("Layer %d, name: %s, num_ins: %d, num_outs: %d\n", layerIndex,
           in_out->this_node, in_out->num_in_nodes, in_out->num_out_nodes);
  for (int i = 0; i < in_out->num_in_nodes; i++)
    RelayDebugPrint(" in: %s\n", in_nodes[i]);
  for (int i = 0; i < in_out->num_out_nodes; i++)
    RelayDebugPrint(" out: %s\n", out_nodes[i]);

  // Fill in layer data/tensor name. If this node is the last node with single
  // output tensor, "this_node" has the output tensor name provided by Python
  // wrapper.
  TIDL_relayFillTensorName(layer.name, in_out->this_node);

  // fill in input data/tensor names
  if (in_out->num_in_nodes > 0)
  { int k=0;
    for (int i = 0; i < in_out->num_in_nodes; i++)
    { k=i;
      while(layer.inDataNames[k][0]!=0&&k<TIDL_NUM_IN_BUFS) k++;
      TIDL_relayFillTensorName(layer.inDataNames[k], in_nodes[i]);
      RelayDebugPrint("Layer %d, input %d name %s\n",
                      layerIndex, i, (char*)layer.inDataNames[i]);
      layer.inData[i].dataId = -1;

      // update inputDataLayer's outConsumerCnt if this node is an input tensor
      for(int j = 0; j < gNumInputDataLayers; j++)
      {
        sTIDL_LayerPC_t& inputLayer = orgTIDLNetStructure.TIDLPCLayers[j];
        if(strcmp((const char*)layer.inDataNames[i],
                  (const char*)inputLayer.outDataNames[0]) == 0)
          inputLayer.outConsumerCnt[0]++;
      }
    }
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Layer %d doesn't have any inputs", layerIndex);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  // Fill in output data/tensor names - "this_node" has the name of
  // output data/tensor.
  TIDL_relayFillTensorName(layer.outDataNames[0], in_out->this_node);
  if (in_out->num_out_nodes > 0)
  {
    if(layer.numOutBufs != -1)
    {
      // if multiple outputs, subsequent ones named with suffix ":%d"
      for (int i = 1; i < layer.numOutBufs; i++)
        TIDL_relayFillTensorName(layer.outDataNames[i], in_out->this_node, i);
    }
    else
    {
      // This is a tuple node and is the last node of the subgraph, and
      // layer.numOutBufs is set to -1 by TIDL_relayImportOutDataLayer().
      RelayDebugPrint("Layer %d, tuple output node of subgraph\n", layerIndex);
      if(in_out->num_out_nodes != layer.numInBufs)
      {
        // num_out_nodes is the number of output tensors and must be equal to
        // layer.numInBufs, which is the number of inputs to this tuple node and
        // is set by TIDL_relayImportOutDataLayer().
        /* Update message if it is not relevant to user */
        TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Numbers of inputs and outputs of the last and tuple "
               "node are different");
      }

      // Fill in outDataNames with output tensor names in right order:
      // - this_node is not used for outDataNames as other layers,
      // - out_nodes[] holds the output tensor names provided by Python wrapper,
      // - outDataNames[] are to be used by tidl_relayFillOutputTensorNames()
      for(int i=0; i<in_out->num_out_nodes; i++)
        TIDL_relayFillTensorName(layer.outDataNames[i], out_nodes[i]);
    }

    memset(layer.outConsumerLinked, 0, TIDL_NUM_OUT_BUFS * sizeof(int32_t));
  }
  else
  {
    // This is the last node of the subgraph and it is not a tuple node
    RelayDebugPrint("Layer %d, single output node of subgraph\n", layerIndex);
  }

  layer.weightsElementSizeInBits = NUM_WHGT_BITS;
  // If output is tuple, assume each consumer uses all the outputs
  for (int i = 0; i < layer.numOutBufs; i++)
     layer.outConsumerCnt[i] = in_out->num_out_nodes;

  tidl_linkInputTensors( &orgTIDLNetStructure, layerIndex);
  tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

static void tidl_relayAddSingleOutDataLayer()
{
  int32_t layerIndex = runtimes_import_state.layerIndex;
  int32_t addOneLayer = 0;
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];

  for (int32_t i = 0; i < layerIndex; i++)
  {
    sTIDL_LayerPC_t& layer_i = orgTIDLNetStructure.TIDLPCLayers[i];
    if (layer_i.layerType != TIDL_DataLayer)
    {
      if (layer_i.outConsumerCnt[0] == 0)
      {
        RelayDebugPrint("AddSingleOutDataLayer for layer %d\n", i);
        layer.layerType  = TIDL_DataLayer;
        layer.numInBufs = 1;
        layer.numOutBufs = -1;
        layer.inData[0].dataId = layer_i.outData[0].dataId;
        memcpy(layer.inDataNames[0], layer_i.outDataNames[0], TIDL_STRING_SIZE);
        memcpy(layer.outDataNames[0], layer.inDataNames[0], TIDL_STRING_SIZE);
        layer_i.outConsumerCnt[0] = 1;
        layer_i.outConsumerLinked[0] = 1;
        addOneLayer = 1;
      }
    }
  }

  runtimes_import_state.layerIndex += addOneLayer;
}

// This function populates output tensor names to the network structure.
// For each dataId of the output DataLayer's output, replace all inDataNames
// and outDataNames associated with the dataId with the official output
// tensor name stored in output DataLayer's outDataNames
static void tidl_relayFillOutputTensorNames()
{
  int32_t numLayers = runtimes_import_state.layerIndex;

  for (int i = 0; i < numLayers; i++)
  {
    // Look for any output data layer added by TIDL_relayImportOutDataLayer
    sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[i];
    if (layer.layerType == TIDL_DataLayer && layer.numOutBufs == -1)
    {
      RelayDebugPrint("Found tuple output data layer at layer %d.\n", i);

      // For this output data layer, inDataNames[] contains the old names of
      // input nodes and outDataNames[] contains the new (official) names of
      // output tensors.  Find other layers that has inDataName or outDataName
      // with the same dataId, replace those old names with the new names:
      // OutDataLayer.outDataName (official) --> OutDataLayer.inDataName
      //   --> layer.{inDataName, outDataName} if same dataId
      for(int ind = 0; ind < layer.numInBufs; ind++)
      {
        uint32_t dataId = layer.inData[ind].dataId;
        char *new_name  = (char *) layer.outDataNames[ind];
        strcpy((char *)layer.inDataNames[ind], new_name);

        for(int j = 0; j < numLayers; j++)
        {
          sTIDL_LayerPC_t& layer_j = orgTIDLNetStructure.TIDLPCLayers[j];
          if (layer_j.layerType != TIDL_DataLayer || layer_j.numOutBufs != -1)
          {
            for(int ind_j = 0; ind_j < layer_j.numInBufs; ind_j++)
            {
              if(layer_j.inData[ind_j].dataId == dataId)
                strcpy((char *)layer_j.inDataNames[ind_j], new_name);
            }
            for(int ind_j = 0; ind_j < layer_j.numOutBufs; ind_j++)
            {
              if(layer_j.outData[ind_j].dataId == dataId)
                strcpy((char *)layer_j.outDataNames[ind_j], new_name);
            }
          }
        }
      }
    }
  }
}

// return names of heads, number of graph outputs and correponding shapes and types
int TIDL_relayGetODMetaArchInfo(int m_meta_arch_type, int m_num_graph_outputs,
                                    std::string m_meta_layers_names_list, void *postproc_info)
{

  RelayDebugPrint("In TIDL_relayGetODMetaArchInfo: \n");
  ODPostProcInfo_t *out = (ODPostProcInfo_t *) postproc_info;
  int status=0, inNodeIdx=0, numOutNodes=0;
  TIDL_odParams_t odParams = {0};
  TensorDescriptor_t *output_tensors = NULL;

  vector<string> odInNodeNames = TIDL_readMetaArchInfo(m_meta_layers_names_list, m_num_graph_outputs, m_meta_arch_type, &odParams);

  for(inNodeIdx = 0; inNodeIdx < odInNodeNames.size(); inNodeIdx++)
  {
    strcpy(out->in_node_names[inNodeIdx], odInNodeNames[inNodeIdx].c_str());
  }
  out->num_in_nodes = inNodeIdx;

  status = TIDL_getOdOutNodeInfo(m_meta_arch_type, &odParams, &numOutNodes, &output_tensors);
  if(status != 0) TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Fetching output node info failed");
  out->num_out_nodes = numOutNodes;
  if(output_tensors!=NULL)
  {
    memcpy(out->out_nodes, output_tensors, sizeof(TensorDescriptor_t)*numOutNodes);
    free(output_tensors);
  }
  else
  {
    status = -1;
  }

  return status;
}


// Setup TIDL OD PostProc, real detection layers are added during OptimizeNet()
int32_t TIDL_relaySetupODPostProc(int m_meta_arch_type, int m_num_graph_outputs,
                                  std::string m_meta_layers_names_list)
{
  int32_t status = 0;
  RelayDebugPrint("In TIDL_relaySetupODPostProc: \n");
  gParams.metaArchType = m_meta_arch_type;
  strcpy((char *) gParams.metaLayersNamesList, m_meta_layers_names_list.c_str());
  status = tidl_metaArch_import(&gParams);
  for (int i = 0; i < m_num_graph_outputs; i++)
    sprintf((char *)tidl_OdOutputNames[i], "tidl_0_o%d", i);
  return status;
}

int32_t tidl_checkCastConstraint()
{
    // Validate Cast layer connections: Cast layers must connect to data layers
  int32_t numLayers = runtimes_import_state.layerIndex;

  // Create a list to store all cast layer names
  std::vector<std::string> castLayersList;

  for (int layerIndex = 0; layerIndex < numLayers; layerIndex++)
  {
    sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];

    if (layer.layerType == TIDL_CastLayer)
    {
      // Add cast layer name to the list
      castLayersList.push_back(std::string((char*)layer.name));

      bool hasDataLayerConnection = false;

      // Check if any input layer is a data layer
      for (int i = 0; i < layer.numInBufs; i++)
      {
        int32_t inputDataId = layer.inData[i].dataId;
        if (inputDataId >= 0)
        {
          // Find the layer that produces this input
          for (int j = 0; j < numLayers; j++)
          {
            sTIDL_LayerPC_t& inputLayer = orgTIDLNetStructure.TIDLPCLayers[j];
            for (int k = 0; k < inputLayer.numOutBufs; k++)
            {
              if (inputLayer.outData[k].dataId == inputDataId)
              {
                if (inputLayer.layerType == TIDL_DataLayer)
                {
                  hasDataLayerConnection = true;
                  castLayersList.pop_back();
                  break;
                }
              }
            }
            if (hasDataLayerConnection) break;
          }
        }
        if (hasDataLayerConnection) break;
      }

      // Check if this Cast layer feeds into a data layer (output)
      if (!hasDataLayerConnection)
      {
        // Check if any consumer of this layer's output is a data layer
        for (int i = 0; i < layer.numOutBufs; i++)
        {
          int32_t outputDataId = layer.outData[i].dataId;
          // Search for layers that consume this output
          for (int j = 0; j < numLayers; j++)
          {
            sTIDL_LayerPC_t& consumerLayer = orgTIDLNetStructure.TIDLPCLayers[j];
            for (int k = 0; k < consumerLayer.numInBufs; k++)
            {
              if (consumerLayer.inData[k].dataId == outputDataId)
              {
                if (consumerLayer.layerType == TIDL_DataLayer)
                {
                  hasDataLayerConnection = true;
                  castLayersList.pop_back();
                  break;
                }
              }
            }
            if (hasDataLayerConnection) break;
          }
          if (hasDataLayerConnection) break;
        }
      }

    }
  }

  if (!castLayersList.empty())
  {
    TIDL_GLOBAL_REPORT_ERROR("Cast layers are only supported at input and output of networks");

    std::string denyListMsg = "Please specify the compilation options 'deny_list:layer_name': '";
    for (size_t i = 0; i < castLayersList.size(); i++)
    {
      denyListMsg += castLayersList[i];
      if(i != castLayersList.size()-1)
      {
        denyListMsg += ", ";
      }
    }
    denyListMsg += "' and rerun compilation for this network";
    TIDL_GLOBAL_REPORT_NOT_PERFORMANT("%s", denyListMsg.c_str());
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_relayOptimizeNet(int32_t subgraph_id)
{
  int32_t status = 0;
  RelayDebugPrint("In TIDL_relayOptimizeNet: \n");

  // Populate output tensor names for the case of multiple output tensors
  tidl_relayFillOutputTensorNames();

  // tidl_optimizeNet() assumes that a DataLayer always exist.  Add an
  // outDataLayer for the single output tensor case here.  OutDataLayer for
  // multiple output tensors case are already added by importTupleNode()
  tidl_relayAddSingleOutDataLayer();

// TIDL Only supported at the terminal nodes (Input/Output) of the network this function checks it:
// remove this function if constraint is removed
  status = tidl_checkCastConstraint();
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) return status;

  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization for subgraph_%d started]", subgraph_id);
  status = TIDL_runtimesOptimizeNet(tidl_relay_debuglevel);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) return status;
  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization for subgraph_%d completed]", subgraph_id);

  // Prevent OD PostProc layers from being added to non-postprocessing subgraphs
  gParams.metaArchType = -1;

  return status;
}

int32_t TIDL_relayPostProcessNet(int32_t numFrames)
{
  RelayDebugPrint("In TIDL_relayPostProcessNet: \n");
  /* These values are not used, keeping it to use the common code with minimal changes */
  float quantFactor[16];
  std::fill(quantFactor, quantFactor+16, 1.0f);

  return TIDL_runtimesPostProcessNet(relayOptions, numFrames, relayOptions->m_num_param_bits, NULL, NULL, quantFactor, NULL, "");
}


void TIDL_relayUpdateDenyList(std::string span_name)
{
  deny_list_layer_name_composite.push_back(span_name);
}

// Called externally from TVM annotation pass to identify nodes that are supported by TIDL.
int32_t TIDL_relayAllowNode(relay::Call call)
{
  TIDL_osrtDebugPrint(tidl_relay_debuglevel, "In TIDL_relayAllowNode: ");
  std::string op_name = TIDL_relayGetOpName(call);
  std::string span_name = TIDL_relayGetSpanName(call);

  TIDL_osrtDebugPrint(tidl_relay_debuglevel, "op_name - %s\n", op_name.c_str());

  if(std::find(deny_list_layer_name_composite.begin(), deny_list_layer_name_composite.end(), span_name) != deny_list_layer_name_composite.end())
  {
    return 0;
  }

  if((std::find(relayOptions->m_deny_list.begin(), relayOptions->m_deny_list.end(), op_name) != relayOptions->m_deny_list.end())||
     (std::find(relayOptions->m_deny_list_layer_type.begin(), relayOptions->m_deny_list_layer_type.end(), op_name) != relayOptions->m_deny_list_layer_type.end())||
     (std::find(relayOptions->m_deny_list_layer_name.begin(), relayOptions->m_deny_list_layer_name.end(), span_name) != relayOptions->m_deny_list_layer_name.end()))
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Node %s added to unsupported nodes as specified in deny list", span_name.c_str());
    if (relayOptions->osrtDebugPrintLevel)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }


  /* If allowlist is present, only nodes present in the allow list are supported, others are delegated to ARM */
  if((relayOptions->m_allow_list_layer_name.size() > 0) && span_name!="")
  {
    /* If layer name not present in the list, return 0 */
    if(std::find(relayOptions->m_allow_list_layer_name.begin(), relayOptions->m_allow_list_layer_name.end(), span_name) == relayOptions->m_allow_list_layer_name.end())
    {
      TIDL_LOG_INFO(gDiags.gDiagList, "Node %s added to unsupported nodes since it is not part of allow list specified in runtime options", span_name.c_str());
      if (relayOptions->osrtDebugPrintLevel)
      {
        gDiags.reportLastModeDiag();
      }
      return 0;
    }
  }

  sTIDL_LayerPC_t layer;
  memset(&layer, 0, sizeof(sTIDL_LayerPC_t));
  strncpy((char *)layer.name, op_name.c_str(), MAX_NAME_LEN);


  int32_t status = 0;
  /* will use it to check the logs which got pushed while checking allowlisting for current layer */
  int32_t diagListSize = gDiags.gDiagList.size();
  layer = TidlParseTVM(call,0,layer).layer;
  status = tidlCheckAllowlistingConstraints(layer);
  if (status == TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Layer type not supported by TIDL --- layer type - %s,  Node name - %s", " ", span_name.c_str());
    if (relayOptions->osrtDebugPrintLevel > 0)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }
  else if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    if (relayOptions->osrtDebugPrintLevel > 0)
    {
      for(int j=diagListSize;j<gDiags.gDiagList.size();j++)
      {
        auto diag = gDiags.gDiagList[j];
        gDiags.modelDiag(diag);
      }
      TIDL_GLOBAL_REPORT_UNSUPPORTED("Layer is not supported by TIDL --- layer type - %s, Node name - %s", " ", span_name.c_str());
    }
    return 0;
  }

  TIDL_LOG_SUPPORTED(gDiags.gDiagList,  "Layers type supported by TIDL --- layer type - %d,  Node name - %s", layer.layerType, span_name.c_str());
  if (relayOptions->osrtDebugPrintLevel > 0)
  {
    gDiags.reportLastModeDiag();
  }
  return 1;
}

// Globally registered functions so that TVM can find and call
TVM_REGISTER_GLOBAL("TIDL_relayInit")
.set_body_typed(TIDL_relayInit);

TVM_REGISTER_GLOBAL("TIDL_relayImportInit")
.set_body_typed(TIDL_relayImportInit);

TVM_REGISTER_GLOBAL("TIDL_relayImportNode")
.set_body_typed(TIDL_relayImportNode);

TVM_REGISTER_GLOBAL("TIDL_relayImportOutDataLayer")
.set_body_typed(TIDL_relayImportOutDataLayer);

TVM_REGISTER_GLOBAL("TIDL_relayImportLinkNode")
.set_body_typed(TIDL_relayImportLinkNode);

TVM_REGISTER_GLOBAL("TIDL_relayGetODMetaArchInfo")
.set_body_typed(TIDL_relayGetODMetaArchInfo);

TVM_REGISTER_GLOBAL("TIDL_relaySetupODPostProc")
.set_body_typed(TIDL_relaySetupODPostProc);

TVM_REGISTER_GLOBAL("TIDL_relayOptimizeNet")
.set_body_typed(TIDL_relayOptimizeNet);

TVM_REGISTER_GLOBAL("TIDL_relayPostProcessNet")
.set_body_typed(TIDL_relayPostProcessNet);

TVM_REGISTER_GLOBAL("TIDL_relayAllowNode")
.set_body_typed(TIDL_relayAllowNode);

TVM_REGISTER_GLOBAL("TIDL_relayUpdateDenyList")
.set_body_typed(TIDL_relayUpdateDenyList);