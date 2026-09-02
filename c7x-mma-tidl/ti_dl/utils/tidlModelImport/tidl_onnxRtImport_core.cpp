/*
*
* Copyright (c) {2015 - 2017} Texas Instruments Incorporated
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
using namespace std;
using ::google::protobuf::Message;
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <cmath>
#include <stdarg.h>

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "tidl_import_config.h"
#include "onnx/onnx-ml.proto3.pb.h"
#include "tidl_custom_import.h"
#include "tidl_meta_arch.pb.h"

using namespace std;
using namespace onnx;

#include "tidl_onnxImport.h"
#include "tidl_onnxRtImport_core.h"
#include "tidl_import_common.h"
#include "tidl_import_common_model_check.h"
#include "itidl_ti.h"
#include "tidl_onnxRtImport_EP.h"
#include "tidl_onnxrt_common.h"
#include "tidl_import_diag.h"

#define __MODULE__ "[PARSER]"

#include "tidlParseOnnx/tidl_parse_onnx.h"
#include "tidlAllowlistingConstraints/tidl_constraint.h"

extern uint8_t tidl_OdOutputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_STRING_SIZE];



extern sTIDL_runtimesImportState_t runtimes_import_state;

/**
 * Map indicating the optional inputs for an onnx operator
 * We expect shape for all the inputs of an ONNX node, we can relax this constraint if the
 * ONNX node doesn't have shape for an optional input
*/
static const std::unordered_map<std::string, vector<int32_t>> ONNXNodeOptionalInputs =
{
  {"Resize", {1, 2, 3}},
  {"LSTM",   {TIDL_RecurrentInputB, TIDL_RecurrentInputSequenceLens, TIDL_RecurrentInputInitialH, TIDL_RecurrentInputInitialC, TIDL_RecurrentInputPeepholes}},
  {"GRU",    {TIDL_RecurrentInputB, TIDL_RecurrentInputSequenceLens, TIDL_RecurrentInputInitialH}},
  {"RNN",    {TIDL_RecurrentInputB, TIDL_RecurrentInputSequenceLens, TIDL_RecurrentInputInitialH}},
  {"CastLike", {1}},
};

/**
 * Map indicating the optional outputs for an onnx operator
 * We expect shape for all the outputs of an ONNX node, we can relax this constraint if the
 * ONNX node doesn't have shape for an optional output
 */
static const std::unordered_map<std::string, vector<int32_t>> ONNXNodeOptionalOutputs =
{
  {"LSTM", {0, 1, 2}},
  {"GRU",  {0, 1}},
  {"RNN",  {0, 1}},
  {"SkipSimplifiedLayerNormalization", {1,2,3}},
};

/*
  Set of operators which can be individally supported,
  maybe unconditionally or with some conditions imposed
  (For e.g., Div is not supported with 2 Var inputs)
  Whatever operator are not in here, are not supported
  indiviually no matter any specifics of the layer
  itself. They might be supported via fusion of a block,
  but in that case this set should not be checked for
  presence of that operator
*/
static const std::unordered_set<std::string> individualSupportedOnnxOps (
  {
    "Conv",
    "Cast",
    "CastLike",
    "Relu",
    "PRelu",
    "LeakyRelu",
    "Identity",
    "Div",
    "Clip",
    "Sigmoid",
    "Logistic",
    "Tanh",
    "HardSigmoid",
    "Elu",
    "Concat",
    "Add",
    "Sum",
    "MaxPool",
    "GlobalAveragePool",
    "AveragePool",
    "Upsample",
    "Resize",
    "ConvTranspose",
    "Reshape",
    "Transpose",
    "ArgMax",
    "ArgMin",
    "BatchNormalization",
    "Gemm",
    "Softmax",
    "Split",
    "Slice",
    "Flatten",
    "Pad",
    "Mul",
    "DepthToSpace",
    "DropOut",
    "Squeeze",
    "ReduceMax",
    "ReduceMin",
    "ScatterElements",
    "ScatterND",
    "Gather",
    "MatMul",
    "DequantizeLinear",
    "QuantizeLinear",
    "Sub",
    "Max",
    "Min",
    "GridSample",
    "DeformConv",
    "LayerNormalization",
    "Asin",
    "Asinh",
    "HardSwish",
    "Mish",
    "Log",
    "Abs",
    "Floor",
    "Exp",
    "Sin",
    "TopK",
    "Erf",
    "Sqrt",
    "Pow",
    "InstanceNormalization",
    "Unsqueeze",
    "SpaceToDepth",
    "Acos",
    "Atan",
    "Cos",
    "Cosh",
    "Neg",
    "Sinh",
    "Tan",
    "ReduceMean",
    "ReduceSum",
    "Expand",
    "Swish",
    "Tile",
    "And",
    "Or",
    "Xor",
    "Equal",
    "Greater",
    "GreaterOrEqual",
    "Less",
    "LessOrEqual",
    "Not",
    "IsInf",
    "IsNaN",
    "Where",
    "RMSNormalization",
    "Softplus",
    "Softsign",
    "Ceil",
    "Celu",
    "Selu",
    "Round",
    "Sign",
    "Mod",
    "GroupNormalization",
    "LSTM",
    "GRU",
    "RNN",
    "GatherND",
    "GatherElements",
    "Shape",
    "Size",
    "Attention",
    "NonZero",
    "SimplifiedLayerNormalization",
    "SkipSimplifiedLayerNormalization"
  }
);

  /*  List of layer types that support dynamic shapes, allowing their output dimensions to vary
   *  at runtime based on input data rather than being fixed during import */

  static const std::unordered_set<std::string> dynamicShapeSupportedLayers(
   {
    "NonZero",
    "Transpose",
    "GatherND" ,
    "Shape",
    "Size"
    }
  );

/*
  Set of operators which do not perform any operations
  (For e.g., Reshape). If a subgraph contains just these
  operators, the whole subgraph will not be allow listed
*/
static const std::unordered_set<std::string> onnxNoOps (
  {
    "Reshape",
    "Squeeze",
    "Unsqueeze",
    "Flatten",
    "CastLike"
    // Think of adding Cast too but then Unit test models will have to change
  }
);

// Functions

/** This function checks if current subgraph has OD post processing happening as part of it or not */
bool TIDL_checkIsSubgraphOd(GraphProto& onnxGraph)
{
  for (int i = 0; i < onnxGraph.output_size(); i++)
  {
    if(strcmp((char *)tidl_OdOutputNames[0], onnxGraph.output(i).name().c_str()) == 0)
    {
      return true;
    }
  }
  return false;
}

/**
 * Identifies isolated no-op nodes in an ONNX graph.
 * A node is considered isolated if:
 * 1. It's a recognized no-op operation (e.g., Identity, Cast with no effect, etc.)
 * 2. All its input connections come from outside the current subgraph
 * 3. All its output connections go to outside the current subgraph
 *
 * Isolated no-op nodes are typically safe to remove during graph optimization as they
 * don't affect the computation but add unnecessary overhead.
 *
 * @param onnxGraph Reference to the ONNX graph protobuf
 * @param nodes List of node indices to analyze (representing a subgraph)
 * @param inputAdjacencyList Input adjacency list for all nodes (maps node to its inputs)
 * @param outputAdjacencyList Output adjacency list for all nodes (maps node to its outputs)
 * @return Vector of node indices that are isolated no-ops in the subgraph
 
 */

std::vector<int> getIsolatedNoOpNodes(GraphProto& onnxGraph,std::vector<int32_t> nodes,std::vector<std::vector<int>> inputAdjacencyList,std::vector<std::vector<int>> outputAdjacencyList)
{
  std::vector<int> result;
  std::unordered_set<int32_t> nodeSet(nodes.begin(), nodes.end()); // O(1) lookup
  
  for (auto i : nodes)
  {
      bool isInputFromSameSubgraph = false;
      bool isOutputFromSameSubgraph = false;
      
      if (onnxNoOps.find(onnxGraph.node(i).op_type()) != onnxNoOps.end())
      {
          std::vector<int> inputs = inputAdjacencyList[i];
          std::vector<int> outputs = outputAdjacencyList[i];
          for (auto input : inputs)
          {
              if (nodeSet.find(input) != nodeSet.end())
              {
                  isInputFromSameSubgraph = true;
                  break;
              }
          }
          for (auto output : outputs)
          {
            if (nodeSet.find(output) != nodeSet.end())
            {
              isOutputFromSameSubgraph = true;
            }
          }
          
          if ((isInputFromSameSubgraph || isOutputFromSameSubgraph) == false)
          {
              result.push_back(i);
          }
      }
  }
  
  return result;
}

/** This function checks if current subgraph only no ops */
bool TIDL_checkIsSubgraphNoOps(GraphProto& onnxGraph, std::vector<int32_t> nodes)
{
  for (auto i : nodes)
  {
    if (onnxNoOps.find(onnxGraph.node(i).op_type()) == onnxNoOps.end())
    {
      return false;
    }
  }
  return true;
}

/* This function finds graph indices corresponding to names of OD heads */
std::vector<int> TIDL_getPostProcInputIds(GraphProto& onnxGraph, std::vector<std::string> odPostProcIndataNames)
{
  std::vector<int> odPostProcIndataIds = {};
  for(int i = 0; i < odPostProcIndataNames.size(); i++)
  {
    for(int j = 0; j < onnxGraph.node_size(); j++)
    {
      if((strcmp(odPostProcIndataNames[i].c_str(), onnxGraph.node(j).output(0).c_str())) == 0)
      {
        odPostProcIndataIds.push_back(j);
      }
    }
  }
  return odPostProcIndataIds;
}


/* Function to return status if a onnx operator is individually supported */
static int32_t TIDL_onnxRtIndividualOpSupport(GraphProto&   onnxGraph, int32_t nodeIdx, sTIDL_LayerPC_t &layer, TIDL_osrtOptions* osrtOptions)
{
  for(int32_t i = 0; i < osrtOptions->m_custom_layers_names_list.size(); i++)
  {
    if(strcmp(onnxGraph.node(nodeIdx).output(0).c_str(), osrtOptions->m_custom_layers_names_list[i].c_str()) == 0)
    {
      return 0;
    }
  }

  if (individualSupportedOnnxOps.find(onnxGraph.node(nodeIdx).op_type()) != individualSupportedOnnxOps.end())
  {
    return 0;
  }
  return TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED;
}

// Convert a onnx operator to a TIDL layer
static int32_t TIDL_onnxRtMapNode(GraphProto&   onnxGraph, int32_t nodeIdx, sTIDL_LayerPC_t &layer)
{
  int32_t status = 0;
  bool isModelShapeInfered = true;
  /* Populating layer name here, it will be used in allowlisting check */
  if(onnxGraph.node(nodeIdx).name() != "")
  {
    strcpy((char*)layer.name, onnxGraph.node(nodeIdx).name().c_str());
  }
  else
  {
    strcpy((char*)layer.name, onnxGraph.node(nodeIdx).output(0).c_str());
  }

  /*** TODO : Add layer mapping using table as in tfliteImport ***/
  layer = TidlParseOnnx(onnxGraph, nodeIdx, layer, isModelShapeInfered).layer;
  status = tidlCheckAllowlistingConstraints(layer); 
  return status;
}
extern int64_t onnxOpSetVersion;

/* For a layer, TIDL does not support if input has more than TIDL_DIM_MAX dimensions */
int32_t TIDL_checkLayerInputDimConstraints(GraphProto& onnGraph, int32_t layerIdx)
{
  std::vector<int32_t> nodeInputDims = getNodeInputShape(onnGraph,  onnGraph.node(layerIdx).input(0), 0);
  if(nodeInputDims.size() > TIDL_DIM_MAX) //layer supported only if number of dimensions of input <= TIDL_DIM_MAX
  {
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

int32_t TIDL_checkRequiredInput (std::string node, int32_t inputIdx)
{
  /* if some specific inputs are defined for this node then check for inputIdx in that list */
  if (ONNXNodeOptionalInputs.find(node) != ONNXNodeOptionalInputs.end())
  {
    vector<int32_t> inputList = ONNXNodeOptionalInputs.at(node);

    if (std::find(inputList.begin(), inputList.end(), inputIdx) != inputList.end())
    {
      /**
       * inputIdx is optional input
       */
      return 0;
    }
    else
    {
      return 1;
    }
  }

  /* if not defined then all inputs are necessary */
  return 1;
}

int32_t TIDL_checkRequiredOutput (std::string node, int32_t outputIdx)
{
  /* if some specific outputs are defined for this node then check for outputIdx in that list */
  if (ONNXNodeOptionalOutputs.find(node) != ONNXNodeOptionalOutputs.end())
  {
    vector<int32_t> outputList = ONNXNodeOptionalOutputs.at(node);

    if (std::find(outputList.begin(), outputList.end(), outputIdx) != outputList.end())
    {
      /**
       * outputIdx is optional ouput
       */
      return 0;
    }
    else
    {
      return 1;
    }
  }

  /* if not defined then all outputs are necessary */
  return 1;
}

/** This functions checks if the ONNX tensor is a scalar*/
int32_t TIDL_checkOnnxTensorIsScalar (GraphProto& onnxGraph, int32_t nodeId)
{
  int32_t isScalar = 0;
  // Iterate through the node's outputs to check shape inference
  for (const auto& output : onnxGraph.node(nodeId).output()) 
  {
    // Check if the output has shape information
    for (const auto& valueInfo : onnxGraph.value_info()) 
    {
      if (valueInfo.name() == output) 
      {
        if (valueInfo.type().has_tensor_type() && 
            valueInfo.type().tensor_type().has_shape()) 
        {
          if (valueInfo.type().tensor_type().shape().dim().size() == 0)
          {
            isScalar = 1;
          }
        }
      }
    }
  }  
  return isScalar;
}

/* For a layer, TIDL does not support if node does not have shape information of input/output */
int32_t TIDL_checkLayerInputOutputDimsExist(GraphProto& onnGraph, int32_t layerIdx)
{
  sTIDL_LayerPC_t tidlLayer;
  TIDL_onnxSaveAllowlistingMetaData(onnGraph, layerIdx, tidlLayer);
  sTIDL_allowlistingMetaData md = tidlLayer.allowlistingMetaData;

  int32_t shapeFound = 1;
  /* all inputs should have non-zero shape */
  for (int32_t i = 0; i < md.numVarInputs; i++)
  {
    std::vector<int32_t> inputDims = md.varTensorsDims[i];
    if(inputDims.size()==0 && TIDL_checkOnnxTensorIsScalar (onnGraph, layerIdx) == 0)
    {
      shapeFound = 0;
    }
    for(auto dim: inputDims)
    {
      if(dim == 0)
      {
        shapeFound = 0;
      }
    }
    if(shapeFound == 0)
    {
      /* shape not found, check if this input is necessary for processing */
      if(TIDL_checkRequiredInput (onnGraph.node(layerIdx).op_type(), md.varTensorIndices[i]) == 1)
      {
        break;
      }
      else
      {
        /* not necessary - ignore this input */
        shapeFound = 1;
      }
    }
  }

  if(shapeFound == 1)
  {
    /* all outputs should have non-zero shape */
    for (int32_t i = 0; i < md.numOutputs; i++)
    {
      std::vector<int32_t> outputDims = md.outputTensorDims[i];
      if(outputDims.size()==0 && TIDL_checkOnnxTensorIsScalar (onnGraph, layerIdx) == 0)
      {
        shapeFound = 0;
      }
      for(auto dim: outputDims)
      {
        if(dim == 0)
        {
          shapeFound = 0;
        }
      }
      if(shapeFound == 0)
      {
        /* shape not found, check if this output is necessary for processing */
        if(TIDL_checkRequiredOutput(onnGraph.node(layerIdx).op_type(), i) == 1)
        {
          break;
        }
        else
        {
          /* not necessary - ignore this output */
          shapeFound = 1;
        }
      }
    }
  }

  if(shapeFound == 0)
  {
    if(strcmp(onnGraph.node(layerIdx).op_type().c_str(), "DequantizeLinear") == 0 && (md.numVarInputs == 0 || onnGraph.node(layerIdx).name().find("/duplicated") != std::string::npos)){
      // checking if the dequantizeLinear layer is used as initializer
      // in this case this node won't have any shape details
      return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
    }
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  /* shape is there, return success */
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/* Identify nodes that are supported by TIDL */
int32_t TIDL_onnxAllowlistNode(GraphProto&   onnxGraph, int32_t i, TIDL_osrtOptions * osrtOptions, int32_t opSetVersion, bool isObjectDetectionNetwork, std::vector<int> odProcNodesComplement, int64_t *cost)
{
  int32_t debugLevel = osrtOptions->m_debug_level;
  int32_t osrtDebugPrintLevel = osrtOptions->osrtDebugPrintLevel;
  std::vector<std::string> denyList = osrtOptions->m_deny_list;
  std::vector<std::string> denyList_layer_type = osrtOptions->m_deny_list_layer_type;
  std::vector<std::string> denyList_layer_name = osrtOptions->m_deny_list_layer_name;
  std::vector<std::string> allowList_layer_name = osrtOptions->m_allow_list_layer_name;

  int32_t status = -1;
  sTIDL_LayerPC_t layer;
  memset(&layer, 0, sizeof(sTIDL_LayerPC_t));

  /* If allowlist is specified, all layers which are not in allow-list must be delegated to ARM */
  if(allowList_layer_name.size() > 0)
  {
    int32_t isNodeInAllowListName = 0;
    for(int k = 0; k < allowList_layer_name.size(); k++)
    {
      std::string onnxNodeName = "";
      if(onnxGraph.node(i).name() != "")
      {
        onnxNodeName = onnxGraph.node(i).name();
      }
      else
      {
        onnxNodeName = onnxGraph.node(i).output(0);
      }
      if(onnxNodeName == allowList_layer_name[k])
      {
        isNodeInAllowListName = 1;
        break;
      }
    }
    if(isNodeInAllowListName == 0)
    {
      TIDL_LOG_INFO(gDiags.gDiagList, "Node %s added to unsupported nodes since it is not part of allow list specified in runtime options", onnxGraph.node(i).op_type().c_str());
      if (osrtDebugPrintLevel)
      {
        gDiags.reportLastModeDiag();
      }
      return 0;
    }
  }

  int32_t isNodeInDenyList = 0;                      //option for Deny List
  for(int j = 0; j < denyList.size(); j++)
  {
    if(onnxGraph.node(i).op_type().c_str() == denyList[j])
    {
      isNodeInDenyList = 1;
      break;
    }
  }


  int32_t isNodeInDenyListName = 0;                   //option for Deny List Name
  for(int k = 0; k < denyList_layer_name.size(); k++)
  {
    std::string onnxNodeName = "";
    if(onnxGraph.node(i).name() != "")
    {
      onnxNodeName = onnxGraph.node(i).name();
    }
    else
    {
      onnxNodeName = onnxGraph.node(i).output(0);
    }
    if(onnxNodeName == denyList_layer_name[k])
    {
      isNodeInDenyListName = 1;
      break;
    }
  }
  int32_t isNodeInDenyListLayer = 0;                  //option for Deny List Type
  for(int j = 0; j < denyList_layer_type.size(); j++)
  {
    if(onnxGraph.node(i).op_type().c_str() == denyList_layer_type[j])
    {
      isNodeInDenyListLayer = 1;
      break;
    }
  }

  if(isNodeInDenyList || isNodeInDenyListLayer || isNodeInDenyListName)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Node (%s) - OP (%s) : Explicitly denied via deny list", onnxGraph.node(i).name().c_str(), onnxGraph.node(i).op_type().c_str());
    if (osrtDebugPrintLevel)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }
  if(isObjectDetectionNetwork)
  {
    if(odProcNodesComplement.size() == 0) //means ARM mode, allowlist based on numDims != 4
    {
      std::vector<int32_t> nodeInputDims = getNodeInputShape(onnxGraph,  onnxGraph.node(i).input(0), debugLevel);

      int32_t numDims = nodeInputDims.size();
      if(TIDL_checkLayerInputDimConstraints(onnxGraph, i) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Node (%s) - OP (%s) : Input has %d dims, exceeds maximum of %d", onnxGraph.node(i).name().c_str(), onnxGraph.node(i).op_type().c_str(), numDims, TIDL_DIM_MAX);
        if (osrtDebugPrintLevel)
        {
          gDiags.reportLastModeDiag();
        }
        return 0;
      }
    }
    else if(std::find(odProcNodesComplement.begin(), odProcNodesComplement.end(), i) == odProcNodesComplement.end()) //if node is not part of backbone network, mark it supported
    {
      TIDL_LOG_INFO(gDiags.gDiagList, "Not part of backbone network, will be delegated in post-processing");
      return 1;
    }
  }

  if (onnxGraph.node(i).input_size() <= 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Node (%s) - OP (%s) : No inputs found", onnxGraph.node(i).name().c_str(), onnxGraph.node(i).op_type().c_str());
    if (osrtDebugPrintLevel)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }

  std::vector<int32_t> nodeInputDims = getNodeInputShape(onnxGraph,  onnxGraph.node(i).input(0), debugLevel);
  if(nodeInputDims.size() == 0 && onnxGraph.node(i).input(0).find("/duplicated") != std::string::npos){
    /*
    Form onnx 1.15, if DQ layer might be duplicated. The node name of duplicated
    node is appended with "/duplicate". Get metadata from original dq layer since
    shape inference wouldn't have run on the duplicated.
    */
    std::string _origName = onnxGraph.node(i).input(0);
    _origName = _origName.substr(0, _origName.find("/duplicated")); 
    nodeInputDims = getNodeInputShape(onnxGraph, _origName, debugLevel);
  }
  int32_t numDims = nodeInputDims.size();
  if(TIDL_checkLayerInputDimConstraints(onnxGraph, i) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL) //if layer input has more than accepted dimensions, not supported
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Node (%s) - OP (%s) : Input has %d dims, exceeds maximum of %d", onnxGraph.node(i).name().c_str(), onnxGraph.node(i).op_type().c_str(), numDims, TIDL_DIM_MAX);
    if (osrtDebugPrintLevel)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }

  if(TIDL_checkLayerInputOutputDimsExist(onnxGraph, i) == -1 && (std::find(dynamicShapeSupportedLayers.begin(), dynamicShapeSupportedLayers.end(), onnxGraph.node(i).op_type()) == dynamicShapeSupportedLayers.end()) ) //if layer input is non-existent, not supported
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Node (%s) - OP (%s) : Input/output shape unknown", onnxGraph.node(i).name().c_str(), onnxGraph.node(i).op_type().c_str());
    if (osrtDebugPrintLevel)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }
  onnxOpSetVersion = opSetVersion;

  /* Populating gParams required by model checker */
  gParams.modelType = TIDL_IMPORT_MODEL_FORMAT_ONNX_RT;
  gParams.numParamBits = osrtOptions->m_num_param_bits;
  gParams.enableCustomLayers = osrtOptions->m_enable_custom_layers;
  gParams.inferenceMode = osrtOptions->m_inference_mode;
  char * tidl_tools_path = const_cast<char *>(osrtOptions->m_tidl_tools_path.c_str());
  if(tidl_tools_path)
  {
    strcpy((char*)gParams.perfSimConfig, tidl_tools_path);
    strcat((char*)gParams.perfSimConfig,"/device_config.cfg");
  }
  else
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Please provide TIDL tools path");
  }

  /* Device specific allow-listing checks require deviceName to be populated */
  if (TIDL_readDeviceConfigParams((char*)gParams.perfSimConfig) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("");
  }

  /*C7x Firmware version needs to be copied before model check is run:*/
  strcpy((char*)gParams.c7xFirmwareVersion, osrtOptions->m_c7x_firmware_version.c_str());

  status = TIDL_onnxRtIndividualOpSupport(onnxGraph, i, layer, osrtOptions);
  /* will use it to check the logs which got pushed while checking allowlisting for current layer */
  int32_t diagListSize = gDiags.gDiagList.size();
  
  if (status != TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED)
  {
    status = TIDL_onnxRtMapNode(onnxGraph, i, layer);
  }

  if (status == TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Layer type not supported by TIDL --- layer type - %s,  Node name - %s", onnxGraph.node(i).op_type().c_str(), onnxGraph.node(i).name().c_str());
    if (osrtDebugPrintLevel > 0)
    {
      gDiags.reportLastModeDiag();
    }
    return 0;
  }
  else if(status == TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    if (osrtDebugPrintLevel > 0)
    {
      // TODO: Change variable here
      for(int j=diagListSize;j<gDiags.gDiagList.size();j++)
      {
        auto diag = gDiags.gDiagList[j];
        gDiags.modelDiag(diag);
      }
      TIDL_GLOBAL_REPORT_UNSUPPORTED("Layer is not supported by TIDL --- layer type - %s, Node name - %s", onnxGraph.node(i).op_type().c_str(), onnxGraph.node(i).name().c_str());
    }
    return 0;
  }

  // TODO: Get cost function from numMacs for each layer.
  *cost = 1;

  TIDL_LOG_SUPPORTED(gDiags.gDiagList,  "Layers type supported by TIDL --- layer type - %s,  Node name - %s", onnxGraph.node(i).op_type().c_str(), onnxGraph.node(i).name().c_str());
  if (osrtDebugPrintLevel > 0)
  {
    gDiags.reportLastModeDiag();
  }

  return 1;
}

/** Create adjacency list of inputs of all nodes in graph
 * Returns list of form < (input node x1, input node y1...), (input node x2, input node y2...) > for nodes < node 1, node 2....>
 * */
std::vector<std::vector<int>> TIDL_createInputAdjacencyList(GraphProto&   onnxGraph)
{
  std::vector<int> inputAdjacentNodes = {};
  std::vector<std::vector<int>> adjacencyList;
  for(int i = 0; i < onnxGraph.node_size(); i++)
  {
    inputAdjacentNodes.clear();
    for(int j = 0; j < onnxGraph.node(i).input_size(); j++)
    {
      for(int k = 0; k < onnxGraph.node_size(); k++)
      {
        for(int l = 0; l < onnxGraph.node(k).output_size(); l++)
        {
          if(strcmp(onnxGraph.node(i).input(j).c_str(), onnxGraph.node(k).output(l).c_str()) == 0)
          {
            inputAdjacentNodes.push_back(k);
          }
        }
      }
    }
    adjacencyList.push_back(inputAdjacentNodes);
  }
  return adjacencyList;
}

/** Create adjacency list of outputs of all nodes in graph
 * Returns list of form < (input node x1, input node y1...), (input node x2, input node y2...) > for nodes < node 1, node 2....>
 * */
std::vector<std::vector<int>> TIDL_createOutputAdjacencyList(GraphProto& onnxGraph)
{
  std::vector<int> outputAdjacentNodes = {};
  std::vector<std::vector<int>> adjacencyList;
  for(int i = 0; i < onnxGraph.node_size(); i++)
  {
    outputAdjacentNodes.clear();
    int foundMatch =0;
    for(int j = 0; j < onnxGraph.node(i).output_size(); j++)
    {
      for(int k = 0; k < onnxGraph.node_size(); k++)
      {
        for(int l = 0; l < onnxGraph.node(k).input_size(); l++)
        {
          if(strcmp(onnxGraph.node(i).output(j).c_str(), onnxGraph.node(k).input(l).c_str()) == 0)
          {
            outputAdjacentNodes.push_back(k);
            foundMatch++;
            break;
          }
        }
      }
      //PC-- catching missed outputs - Case: overall output which is also input to another node(s)
      for(int m = 0; m < onnxGraph.output().size(); m++)
      {
        if((strcmp(onnxGraph.output(m).name().c_str(), onnxGraph.node(i).output(j).c_str()) == 0) && (foundMatch != 0))
        {
          //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "PC-- graph output onnxGraph.node(%d).output(%d).c_str() = %s.",i,j,onnxGraph.node(i).output(j).c_str());
          outputAdjacentNodes.push_back(-1);
        }
      }
    }
    adjacencyList.push_back(outputAdjacentNodes);
  }
  return adjacencyList;
}

void TIDL_onnxRTUpdateSubgraphStringBuf(std::string * string_buf)
{
  gParams.onnxProto = string_buf;
}

/** This function adds input and output data layers to subgraphs and sets their properties
Also validates the import paramters and does meta architecture import for OD models with meta arch file provided*/
int32_t TIDL_onnxRtImportInit(GraphProto& onnxGraph, onnxRtParams_t *onnxRtParams, char* subgraph_name, TIDL_osrtOptions * osrt_options, int32_t opSetVersion,
                                std::vector<std::string>outDataOd, bool isSubgraphOD, std::string * string_buf)
{
  int32_t i;
  int32_t status, elementSize;
  TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "In TIDL_onnxRtImportInit subgraph_name=%s", subgraph_name);

  // Reset all the memories to to NULL, there could be multiple subgraphs
  memset(&orgTIDLNetStructure, 0, sizeof(sTIDL_OrgNetwork_t));
  memset(&tIDLNetStructure,    0, sizeof(sTIDL_Network_t));
  memset(inDataNames,          0, sizeof(inDataNames));
  memset(outDataNames,         0, sizeof(outDataNames));

  runtimes_import_state.layerIndex = 0;
  runtimes_import_state.dataIndex  = 0;

  // Initialize layer independent parameters of gParams
  onnxOpSetVersion = opSetVersion;
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runtimesGparamsInit(osrt_options, TIDL_IMPORT_MODEL_FORMAT_ONNX_RT), "");

  sprintf((char *)gParams.metaLayersNamesList, osrt_options->m_meta_layers_names_list.c_str());

  if((outDataOd.size() == 0) || (! isSubgraphOD)) //Not an OD network / Subgaph does not contain OD post processing part
  {
    gParams.metaArchType = -1;
    numNetOutData = onnxRtParams->numNetOutData ;
    for (i = 0; i < numNetOutData; i++)
    {
      int32_t layerIndex = TIDL_addOutputDataLayer(i, (char *)onnxRtParams->outDataNames[i]);
      TIDL_updateNamesList ((char*)gParams.outDataNamesList, i, (char *)onnxRtParams->outDataNames[i]);
      TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "Layer %d, subgraph id %s, name=%s", layerIndex, subgraph_name, (char*)onnxRtParams->outDataNames[i]);

      status = TIDL_ortGetType(onnxRtParams->outputTensorElementType[i], &gParams.outElementType[i], &elementSize);
      gParams.modelOutElementType[i] = gParams.outElementType[i];
      if(status != 0)
      {
        return status;
      }

      if(gParams.addDataConvertToNet & ADD_DC_LAYER_AT_OUTPUT)
      {
        gParams.outTensorScale[i] = 1.0;
        gParams.outZeroPoint[i] = 0.0;
      }
      else
      {
        if(gParams.numFeatureBits == 32)
        {
          gParams.outElementType[i] = TIDL_SinglePrecFloat;
        }
        else if (osrt_options->m_num_param_bits > 8)
        {
          if(gParams.inferencePrecisionMode == TIDL_InferencePrecisionModeFixedPoint) 
          {
            if(gParams.outElementType[i] == TIDL_UnsignedChar || gParams.outElementType[i] == TIDL_UnsignedShort)
            {
              gParams.outElementType[i] = TIDL_UnsignedShort;
            }
            else
            {
              gParams.outElementType[i] = TIDL_SignedShort;
            }
          }
          else if(gParams.inferencePrecisionMode == TIDL_InferencePrecisionModeFloatingPoint)
          {
            gParams.outElementType[i] = TIDL_BFloat16;
          }
          else {
            TIDL_GLOBAL_REPORT_WARNING("Unsupported output element type for subgraph %s, layer %d", subgraph_name, i);
          }
        }
        else
        {
          if(gParams.outElementType[i] == TIDL_UnsignedChar || gParams.outElementType[i] == TIDL_UnsignedShort)
          {
            gParams.outElementType[i] = TIDL_UnsignedChar;
          }
          else
          {
            gParams.outElementType[i] = TIDL_SignedChar;
          }
        }
      }
    }
  }
  else
  {
    gParams.metaArchType = osrt_options->m_meta_arch_type;
    numNetOutData = outDataOd.size() ; //The OD heads are treated as outputs, since meta arch import assumes output data layers at heads
    for (i = 0; i < numNetOutData; i++)
    {
      int32_t layerIndex = TIDL_addOutputDataLayer(i, (char*)outDataOd[i].c_str());

      TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "Layer %d, subgraph id %s, name=%s", layerIndex, subgraph_name, (char*)outDataOd[i].c_str());
      if(gParams.addDataConvertToNet & ADD_DC_LAYER_AT_OUTPUT)
      {
        gParams.outTensorScale[i] = 1.0;
        gParams.outZeroPoint[i] = 0.0;
        gParams.outElementType[i] = TIDL_SinglePrecFloat; /* conv heads will have float output */
      }
      else
      {
        if(gParams.numFeatureBits == 32)
        {
          gParams.outElementType[i] = TIDL_SinglePrecFloat;
        }
        else if (osrt_options->m_num_param_bits > 8)
        {
          if(gParams.inferencePrecisionMode == TIDL_InferencePrecisionModeFloatingPoint)
          {
            gParams.outElementType[i] = TIDL_BFloat16;
          }
          else
          {
            gParams.outElementType[i] = TIDL_SignedShort;
          }
        }
        else
        {
          gParams.outElementType[i] = TIDL_SignedChar;
        }
      }
    }
  }

  for (i = 0; i < onnxRtParams->numNetInData; i++)
  {
    status = TIDL_ortGetType(onnxRtParams->inputTensorElementType[i], &gParams.inElementType[i], &elementSize);
    gParams.modelInElementType[i] = gParams.inElementType[i];

    /* Build the 6-D shape for TIDL_addInputDataLayer6D.
     * tensorShape from ORT is always a concrete data shape.
     * For named dynamic dims (has_dim_param): mark dynDimMask and optionally
     * overwrite with user-supplied max value from max_dynamic_dims.
     * dynDimMask: bit N set means dimValues[N] can vary at runtime. */
    int32_t shape6d[TIDL_DIM_MAX];
    uint8_t dynDimMask = 0;

    for (int32_t d = 0; d < TIDL_DIM_MAX; d++)
    {
      shape6d[d] = onnxRtParams->tensorShape[i][d];
    }

    std::string inName(reinterpret_cast<const char*>(onnxRtParams->inDataNames[i]));

    /* Find this input in the ONNX graph to identify dynamic dims. */
    const ::onnx::TensorShapeProto* onnxShape = nullptr;
    for (int k = 0; k < onnxGraph.input_size(); k++)
    {
      if (onnxGraph.input(k).name() == inName &&
          onnxGraph.input(k).type().has_tensor_type() &&
          onnxGraph.input(k).type().tensor_type().has_shape())
      {
        onnxShape = &onnxGraph.input(k).type().tensor_type().shape();
        break;
      }
    }

    if (onnxShape)
    {
      const std::map<std::string, int32_t>& dim_map = osrt_options->m_max_dynamic_dims;
      int32_t ndims = onnxShape->dim_size();
      for (int32_t d = 0; d < ndims; d++)
      {
        int32_t pos = TIDL_DIM_MAX - ndims + d;
        if (onnxShape->dim(d).has_dim_param())
        {
          dynDimMask |= (1u << pos);
          auto dim_it = dim_map.find(onnxShape->dim(d).dim_param());
          if (dim_it != dim_map.end())
          {
            shape6d[pos] = dim_it->second;
          }
          /* else: keep tensorShape (concrete data shape from ORT) */
        }
      }

      if (dynDimMask)
      {
        TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel,
          "Input '%s': shape [%d,%d,%d,%d,%d,%d] dynDimMask=0x%x", inName.c_str(),
          shape6d[TIDL_DIM_BATCH], shape6d[TIDL_DIM_DIM1], shape6d[TIDL_DIM_DIM2],
          shape6d[TIDL_DIM_NUMCH], shape6d[TIDL_DIM_HEIGHT], shape6d[TIDL_DIM_WIDTH],
          dynDimMask);
      }
    }

    int32_t layerIndex = TIDL_addInputDataLayer6D(i,
      shape6d[TIDL_DIM_BATCH], shape6d[TIDL_DIM_DIM1], shape6d[TIDL_DIM_DIM2],
      shape6d[TIDL_DIM_NUMCH], shape6d[TIDL_DIM_HEIGHT], shape6d[TIDL_DIM_WIDTH],
      (char*)onnxRtParams->inDataNames[i]);

    /* Mark which 6-D dims are dynamic so the runtime propagates shapes correctly. */
    orgTIDLNetStructure.TIDLPCLayers[layerIndex].outData[0].dynDimMask = dynDimMask;

    TIDL_updateNamesList ((char*)gParams.inDataNamesList, i, (char *)onnxRtParams->inDataNames[i]);
  
    if(status != 0)
    {
      return status;
    }

    int8_t inputDCAdded = 0;

    if(gParams.addDataConvertToNet & ADD_DC_LAYER_AT_INPUT)
    {
      /*
       * Dataconvert layer does not supoort these input element type.
       * In this case, use arm data conversion
       */
      if (gParams.inElementType[i] == TIDL_UnsignedWord ||
          gParams.inElementType[i] == TIDL_SignedWord ||
          gParams.inElementType[i] == TIDL_UnsignedDoubleWord ||
          gParams.inElementType[i] == TIDL_SignedDoubleWord ||
          gParams.inElementType[i] == TIDL_Bool)
      {
        inputDCAdded = 0;
      }
      else
      {
        gParams.inQuantFactor[i] = 1.0;
        gParams.inZeroPoint[i] = 0.0;
        inputDCAdded = 1;
      }
    }

    if (inputDCAdded == 0)
    {
      if(gParams.inElementType[i] != TIDL_SignedWord &&
         gParams.inElementType[i] != TIDL_SignedDoubleWord && //For pointpillars, keep the inElementType same (int32) for indices
         gParams.inElementType[i] != TIDL_Bool &&
         gParams.inElementType[i] != TIDL_UnsignedDoubleWord &&
         gParams.inElementType[i] != TIDL_UnsignedWord)
      {
        if(gParams.numFeatureBits == 32)
        {
          gParams.inElementType[i] = TIDL_SinglePrecFloat;
        }
        else if (osrt_options->m_num_param_bits > 8)
        {
          if(gParams.inElementType[i] == TIDL_UnsignedChar || gParams.inElementType[i] == TIDL_UnsignedShort)
          {
            gParams.inElementType[i] = TIDL_UnsignedShort;
          }
          else
          {
            gParams.inElementType[i] = TIDL_SignedShort;
          }
        }
        else
        {
          if(gParams.inElementType[i] == TIDL_UnsignedChar || gParams.inElementType[i] == TIDL_UnsignedShort)
          {
            gParams.inElementType[i] = TIDL_UnsignedChar;
          }
          else
          {
            gParams.inElementType[i] = TIDL_SignedChar;
          }
        }
      }
    }

    tidl_onnxLayerUpdateConsumerCount(&orgTIDLNetStructure, layerIndex, layerIndex, onnxGraph);
    tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);
    TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "Layer %d, subgraph id %s, name=%s", layerIndex, subgraph_name, (char*)onnxRtParams->inDataNames[i]);
  }

  // Initialize rest of the layers
  for (int i = onnxRtParams->numNetInData; i < TIDL_NUM_MAX_PC_LAYERS; i++)
  {
    sTIDL_LayerPC_t& layer_i = orgTIDLNetStructure.TIDLPCLayers[i];
    layer_i.actParams.actType  = TIDL_NoAct;
    layer_i.strideOffsetMethod = TIDL_StrideOffsetTopLeft;
  }

  /* Subgraph inputs are saved in raw file in float data format with corresponding quantization scales */
  for (int i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++)
  {
    gParams.rawDataInElementType[i]   = TIDL_SinglePrecFloat;
  }

  if (tidlValidateImportParams(&gParams) == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Validation of TIDL ONNX runtime import config parameters failed");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  if(gParams.metaArchType != -1)
  {
    TIDL_IMPORT_CHECK_AND_RETURN(tidl_metaArch_import(&gParams), "");
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** This function maps ONNX nodes to TIDL nodes and links them to form TIDL imported graph */
int32_t TIDL_onnxRtImportAndLinkNode(GraphProto&   onnxGraph, int32_t nodeIdx, int32_t debugLevel)
{
  int32_t status = 0;
  bool isModelShapeInfered = true;
  // Get new layerIndex, dataIndex
  int32_t layerIndex = runtimes_import_state.layerIndex++;
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
  // set layer defaults
  layer.numInBufs  = 1;
  layer.numOutBufs = onnxGraph.node(nodeIdx).output_size();
  for (int j = 0; j < layer.numOutBufs; j++)
  {
    int32_t dataIndex  = runtimes_import_state.dataIndex++;
    layer.outData[j].dataId = dataIndex;
  }

  if(onnxGraph.node(nodeIdx).name() != "")
  {
    strcpy((char*)layer.name, onnxGraph.node(nodeIdx).name().c_str());
  }
  else
  {
    strcpy((char*)layer.name, onnxGraph.node(nodeIdx).output(0).c_str());
  }

  /* Parse the node */
  layer = TidlParseOnnx(onnxGraph, nodeIdx, layer, isModelShapeInfered).layer;
  TIDL_IMPORT_CHECK_AND_RETURN(tidl_onnxLayerFillTensorNames(&orgTIDLNetStructure, nodeIdx, layerIndex, onnxGraph), "");
  tidl_onnxLayerUpdateConsumerCount(&orgTIDLNetStructure, nodeIdx, layerIndex, onnxGraph);
  tidl_linkInputTensors(&orgTIDLNetStructure, layerIndex);
  tidl_linkOutputTensors(&orgTIDLNetStructure, layerIndex);

  return status;
}

void TIDL_concatOutNetPrototxtIntoUserProtoOnnx()
{
  if(gParams.isQuantParamsToBeExported)
  {
    TIDL_IMPORT_CHECK_AND_ABORT(TIDL_exportQuantParamsIntoUserPath(&gParams), "");
  }
}

