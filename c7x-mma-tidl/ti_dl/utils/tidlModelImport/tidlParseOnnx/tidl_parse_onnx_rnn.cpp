/*
*
* Copyright (c) {2015 - 2026} Texas Instruments Incorporated
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
using namespace std;
using namespace onnx;

template<> int32_t TidlParseOnnx::parse<OnnxStr("RNN")> ()
{
  int32_t status = 0;

  sTIDL_RNNParams_t &rnnParams = layer.layerParams.rnnParams;
  sTIDL_RNNPCParams_t &rnnPCParams = layer.layerPCParams.rnnParams;
  sTIDL_allowlistingMetaData md  = layer.allowlistingMetaData;
  layer.layerType = TIDL_RNNLayer;
  layer.numInBufs = md.numVarInputs;

  NodeProto node = graph.node(index);

  /* Parse Attributes */
  rnnParams.isClipSet = 0;
  rnnParams.direction = TIDL_RNNForward;
  rnnParams.layout = 0;

  int32_t attrActivationAlphaIdx = getAttrIdx(node, "activation_alpha");
  if (attrActivationAlphaIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    int32_t numActivations = node.attribute(attrActivationAlphaIdx).floats_size();
    if(numActivations > TIDL_RNN_MAX_ACTIVATIONS)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RNN layer : Upto %d activation_alpha values are supported, provided %d for node %s", TIDL_RNN_MAX_ACTIVATIONS, numActivations, graph.node(index).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    for (int32_t idx = 0; idx < numActivations; idx++)
    {
      rnnParams.activation_alpha[idx] = node.attribute(attrActivationAlphaIdx).floats(idx);
    }
  }

  int32_t attrActivationBetaIdx = getAttrIdx(node, "activation_beta");
  if (attrActivationBetaIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    int32_t numActivations = node.attribute(attrActivationBetaIdx).floats_size();
    if(numActivations > TIDL_RNN_MAX_ACTIVATIONS)
    {
      TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RNN layer : Upto %d activation_beta values are supported, provided %d for node %s", TIDL_RNN_MAX_ACTIVATIONS, numActivations, graph.node(index).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    for (int32_t idx = 0; idx < numActivations; idx++)
    {
      rnnParams.activation_beta[idx] = node.attribute(attrActivationBetaIdx).floats(idx);
    }
  }

  /* activations: list of strings */
  int32_t availableActivations = 0;
  int32_t attrActivationsIdx = getAttrIdx(node, "activations");

  if(attrActivationsIdx != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    if(node.attribute(attrActivationsIdx).strings_size() > 0)
    {
      availableActivations = node.attribute(attrActivationsIdx).strings_size();
      if(availableActivations != 1 && availableActivations != 2)
      {
        TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RNN layer : Only 1 or 2 (if bidirectional) activations are supported, provided %d for node %s", availableActivations, graph.node(index).name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }

      for (int32_t idx = 0; idx < availableActivations; idx++)
      {
        std::string activation = node.attribute(attrActivationsIdx).strings(idx);
        if(activationMap.find(activation) == activationMap.end())
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RNN layer : Activation %s is not supported for node %s", activation.c_str(), graph.node(index).name().c_str());
          return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
        }
        rnnParams.activations[idx] = activationMap[activation];
      }
    }
  }

  float32_tidl clip = 0.0f;
  status = getFloatAttr(node, "clip", &clip, 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    rnnParams.clip = clip;
    rnnParams.isClipSet = 1;
  }

  char direction[50];
  status = getStringAttr(node, "direction", direction, 0);
  if (status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    if(strcmp(direction, "forward") == 0)
    {
      rnnParams.direction = TIDL_RNNForward;
    }
    else if(strcmp(direction, "reverse") == 0)
    {
      rnnParams.direction = TIDL_RNNReverse;
    }
    else if(strcmp(direction, "bidirectional") == 0)
    {
      rnnParams.direction = TIDL_RNNBidirectional;
    }
    else
    {
      rnnParams.direction = TIDL_RNNUnsupported;
    }
  }

  int32_t num_directions = 1;
  if(rnnParams.direction == TIDL_RNNBidirectional)
  {
    num_directions = 2;
  }

  /* Fill default activations if not provided based on numDirections */
  if(attrActivationsIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    /* Default activations */
    availableActivations = num_directions;
    for(int32_t idx = 0; idx < num_directions; idx++)
    {
      /* activation f */
      rnnParams.activations[idx] = TIDL_Tanh;
    }
  }

  /* Default alpha and beta values */
  if(attrActivationAlphaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL ||
     attrActivationBetaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    for(int32_t idx = 0; idx < availableActivations; idx++)
    {
      if(activationAlphaBetaMap.find(rnnParams.activations[idx]) != activationAlphaBetaMap.end())
      {
        if(attrActivationAlphaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          rnnParams.activation_alpha[idx] = activationAlphaBetaMap[rnnParams.activations[idx]].first;
        }
        if(attrActivationBetaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          rnnParams.activation_beta[idx] = activationAlphaBetaMap[rnnParams.activations[idx]].second;
        }
      }
      else
      {
        /* For activations which are not in the map, set alpha and beta to 0 */
        if(attrActivationAlphaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          rnnParams.activation_alpha[idx] = 0.0f;
        }
        if(attrActivationBetaIdx == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
        {
          rnnParams.activation_beta[idx] = 0.0f;
        }
      }
    }
  }

  int32_t hidden_size = 0;
  status = getIntAttr(node, "hidden_size", &hidden_size, 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    rnnParams.hidden_size = hidden_size;
  }

  int32_t layout = 0;
  status = getIntAttr(node, "layout", &layout, 0);
  if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    rnnParams.layout = layout;
  }

  /* Parse Inputs */
  status = copyFloatConst(graph, index, 1, layer.weights, INPUT_REQUIRED);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RNN layer : Unable to copy initializer at index %d for node %s", 1, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  else
  {
    /* Const layer will get added for this input */
    layer.numInBufs++;
  }

  status = copyFloatConst(graph, index, 2, rnnPCParams.recurrenceWeights, INPUT_REQUIRED);
  if(status == TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : RNN layer : Unable to copy initializer at index %d for node %s", 2, graph.node(index).name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }
  else
  {
    /* Const layer will get added for this input */
    layer.numInBufs++;
  }

  /* Bias tensor */
  if (graph.node(index).input_size() > 3)
  {
    status = copyFloatConst(graph, index, 3, layer.bias, INPUT_NOT_REQUIRED);
  }

  /* sequence_lens tensor */
  if (graph.node(index).input_size() > 4)
  {
    status = copyFloatConst(graph, index, 4, rnnPCParams.sequence_lens, INPUT_NOT_REQUIRED);
  }

  /* initial_h tensor */
  if (graph.node(index).input_size() > 5)
  {
    status = copyFloatConst(graph, index, 5, rnnPCParams.initial_h, INPUT_NOT_REQUIRED);
    if(status != TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL)
    {
      /* Const layer will get added for this input */
      layer.numInBufs++;
    }
  }

  for(int32_t varIdx: md.varTensorIndices)
  {
    if(varIdx == 5)
    {
      rnnParams.isInitialHPresent = 1;
    }
  }
  for(int32_t constIdx: md.constTensorIndices)
  {
    if(constIdx == 5)
    {
      rnnParams.isInitialHPresent = 1;
    }
  }

  /* Validate the input and weights dimensions */
  std::vector<int32_t> inputXDims = md.varTensorsDims[0];
  if (inputXDims.size() != 3)
  {
    TIDL_LOG_ERROR(gDiags.gDiagList, "Input X must be a 3D tensor for node %s", node.name().c_str());
    return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
  }

  /* Extract dimensions based on layout */
  int32_t seq_length, batch_size, input_size;
  if (rnnParams.layout == 0)
  {
    /* input shape: [seq_length, batch_size, input_size] */
    seq_length = inputXDims[0];
    batch_size = inputXDims[1];
    input_size = inputXDims[2];
  }
  else
  {
    /* input shape: [batch_size, seq_length, input_size] */
    batch_size = inputXDims[0];
    seq_length = inputXDims[1];
    input_size = inputXDims[2];
  }

  TensorProto tensor;
  /* W input */
  tensor = getInitializerTensor(graph, graph.node(index).input(1), index, status);
  if(status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    bool isTensorValid = true;
    if (tensor.dims_size() != 3)
    {
      isTensorValid = false;
    }
    
    if (getTensorDim(tensor, 0) != num_directions ||
        getTensorDim(tensor, 1) != rnnParams.hidden_size ||
        getTensorDim(tensor, 2) != input_size)
    {
      isTensorValid = false;
    }

    if(isTensorValid == false)
    {
      TIDL_LOG_ERROR(gDiags.gDiagList, "Weight tensor W must be a 3D tensor of shape [num_directions, hidden_size, input_size] for node %s", node.name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
  
  /* R input */
  tensor = getInitializerTensor(graph, graph.node(index).input(2), index, status);
  if(status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
  {
    bool isTensorValid = true;
    if (tensor.dims_size() != 3)
    {
      isTensorValid = false;
    }
    
    if (getTensorDim(tensor, 0) != num_directions ||
        getTensorDim(tensor, 1) != rnnParams.hidden_size ||
        getTensorDim(tensor, 2) != rnnParams.hidden_size)
    {
      isTensorValid = false;
    }

    if(isTensorValid == false)
    {
      TIDL_LOG_ERROR(gDiags.gDiagList, "Recurrence weight tensor R must be a 3D tensor of shape [num_directions, hidden_size, hidden_size] for node %s", node.name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
  }
  
  /* B input */
  if (node.input_size() > 3)
  {
    tensor = getInitializerTensor(graph, graph.node(index).input(3), index, status);
    if(status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      bool isTensorValid = true;
      if (tensor.dims_size() != 2)
      {
        isTensorValid = false;
      }
      
      if (getTensorDim(tensor, 0) != num_directions ||
          getTensorDim(tensor, 1) != 2 * rnnParams.hidden_size)
      {
        isTensorValid = false;
      }

      if(isTensorValid == false)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "Bias tensor B must be a 2D tensor of shape [num_directions, 2*hidden_size] for node %s", node.name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  /* sequence_lens input */
  if (node.input_size() > 4)
  {
    tensor = getInitializerTensor(graph, graph.node(index).input(4), index, status);
    if(status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      bool isTensorValid = true;
      if (tensor.dims_size() != 1)
      {
        isTensorValid = false;
      }
      
      if (getTensorDim(tensor, 0) != batch_size)
      {
        isTensorValid = false;
      }

      if(isTensorValid == false)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "sequence_lens must be a 1D tensor of shape [batch_size] for node %s", node.name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  /* initial_h input */
  if (node.input_size() > 5)
  {
    tensor = getInitializerTensor(graph, graph.node(index).input(5), index, status);
    if(status != TIDL_ALLOWLISTING_LAYER_CHECK_FAILED)
    {
      bool isTensorValid = true;
      if (tensor.dims_size() != 3)
      {
        isTensorValid = false;
      }
      
      if (getTensorDim(tensor, 0) != num_directions ||
          getTensorDim(tensor, 1) != batch_size ||
          getTensorDim(tensor, 2) != rnnParams.hidden_size)
      {
        isTensorValid = false;
      }

      if(isTensorValid == false)
      {
        TIDL_LOG_ERROR(gDiags.gDiagList, "initial_h must be a 3D tensor of shape [num_directions, batch_size, hidden_size] for node %s", node.name().c_str());
        return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
      }
    }
  }

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
