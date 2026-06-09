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
#include "tidl_custom_import.h"
#include "tidl_parse_onnx_jumptable.h"

TidlParseOnnx::TidlParseOnnx(GraphProto& onnxGraph, int32_t index, sTIDL_LayerPC_t layerPC, bool& isModelShapeInfered) :
    graph(onnxGraph), index(index), layer(layerPC), isModelShapeInfered(isModelShapeInfered)
{
  status = saveAllowlistingMetaData();
  if(status != 0)
  {
    TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Allowlisting : Error while saving allowlisting meta data");
    layer.layerType = TIDL_UnsupportedLayer;
    layer.parseStatus = TIDL_ParseFailed;
    return;
  }

  string onnxOpStr = graph.node(index).op_type().c_str();
  auto jumpTableEntry = tidlParseOnnxJumpTable.find(onnxOpStr);
  const onnx::NodeProto& node = onnxGraph.node(index);
  
  int32_t customStatus = -1;
  
  if (gParams.enableCustomLayers)
  {
    /* A new generic API to map custom layer parameters and weights is used
    post 11.0 release instead of ONNX specific API.*/
#ifdef TIDL_CUSTOM_LAYER_API_v1
    customStatus = TIDL_MapCustomParamsOnnx(layer,
                                            index,
                                            &onnxGraph);
#elif defined(TIDL_CUSTOM_LAYER_API_v2)
    if(layer.weights.ptr == NULL)
    {
      customStatus = TIDL_mapCustomParams_v2(&layer,
                                          index,  
                                          &onnxGraph,
                                          NULL,
                                          gParams.modelType);
      status = customStatus;
    }
#endif
  }

  if ((customStatus == -1) && (jumpTableEntry != tidlParseOnnxJumpTable.end()))
  {
    status = (this->*(jumpTableEntry->second))(); // Call the parse function

    if (status == -1)
    {
      layer.parseStatus = TIDL_ParseFailed;
    }
    else
    {
      if(isModelShapeInfered &&
         checkShapeInferenceforOnnx(layer.allowlistingMetaData) == TIDL_IMPORT_DIAGNOSIS_RETURN_OK)
      {
        if(isLayerIODataTypesSupported(layer) && isLayerIONumSupported(layer))
        {
          layer.parseStatus = TIDL_ParsePassed;
        }
        else
        {
          layer.parseStatus = TIDL_ParseFailed;
        }
      }
      else
      {
        isModelShapeInfered = false;
      }
    }
  }

  if(status == -1)
  {
    layer.layerType = TIDL_UnsupportedLayer;
    status = TIDL_ALLOWLISTING_LAYER_TYPE_UNSUPPORTED;
  }

}

/** Function to save layer level tensor properties metadata to be used for allowlisting
 * Saves data for sTIDL_allowlistingMetaData structure
 */
int32_t TidlParseOnnx::saveAllowlistingMetaData()
{
  /*
   Form onnx 1.15, if DQ layer might be duplicated. The node name of duplicated
   node is appended with "/duplicate". Get metadata from original dq layer since
   shape inference wouldn't have run on the duplicated.
  */
  if (strcmp(graph.node(index).op_type().c_str(), "DequantizeLinear") == 0)
  {
    std::string name = graph.node(index).name();
    name = getOriginalNameFromDuplicateName(name);
    for (int j = 0; j < graph.node_size(); j++)
    {
      if(strcmp(graph.node(j).name().c_str(), name.c_str()) == 0)
      {
        index = j;
        break;
      }
    }
  }

  layer.allowlistingMetaData.numInputs = graph.node(index).input_size();
  layer.allowlistingMetaData.numOutputs = graph.node(index).output_size();
  layer.allowlistingMetaData.numValidInputs = 0;
  layer.allowlistingMetaData.validInputIndices = {};
   for (int j = 0; j < graph.node(index).input_size(); j++)
  {
    if (strcmp(graph.node(index).input(j).c_str(),"") != 0)
    {
      layer.allowlistingMetaData.numValidInputs++;
      layer.allowlistingMetaData.validInputIndices.push_back(j);
    }
  }
  layer.allowlistingMetaData.numVarInputs = numVariableLayerInputs(graph, index,  layer.allowlistingMetaData.varTensorIndices);
  layer.allowlistingMetaData.numConstInputs = layer.allowlistingMetaData.numValidInputs - layer.allowlistingMetaData.numVarInputs;

  //Find indices of constant tensors
  for(auto& idx : layer.allowlistingMetaData.validInputIndices)
  {
    if(find(layer.allowlistingMetaData.varTensorIndices.begin(), layer.allowlistingMetaData.varTensorIndices.end(), idx) == layer.allowlistingMetaData.varTensorIndices.end())
    //idx not found in variable tensors indices => constant tensor idx
    {
      layer.allowlistingMetaData.constTensorIndices.push_back(idx);
    }
  }
  //Populate dimensions of variable tensors
  for(auto& varIdx : layer.allowlistingMetaData.varTensorIndices)
  {
    std::string inputName = graph.node(index).input(varIdx);

    // If input at varIdx is duplicated DQ then use the original DQ to get dims
    if(inputName.find("/duplicated") != std::string::npos)
    {
      int32_t found = 0;
      for (int j = 0; j < graph.node_size(); j++)
      {
        if (strcmp(graph.node(j).op_type().c_str(), "DequantizeLinear") == 0)
        {
          for (int k = 0; k < graph.node(j).output_size(); k++)
          {
            if (strcmp(graph.node(j).output(k).c_str(), inputName.c_str()) == 0)
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

    layer.allowlistingMetaData.varTensorsDims.push_back(getNodeInputShape(graph,  inputName, 0));
  }
  //Populate dimensions of constant tensors
  for(auto& constIdx : layer.allowlistingMetaData.constTensorIndices)
  {
    int32_t status = 0;
    TensorProto tensor = getInitializerTensor(graph, graph.node(index).input(constIdx), index, status);
    if(status != 0)
    {
      TIDL_GLOBAL_REPORT_ERROR("Unable to find initializer at index - %d for node %s", constIdx, graph.node(index).name().c_str());
      return TIDL_ALLOWLISTING_LAYER_CHECK_FAILED;
    }
    layer.allowlistingMetaData.constTensorNames.push_back(graph.node(index).input(constIdx).c_str());
    vector<int32_t> dims;
    for(int j = 0; j < tensor.dims_size(); j++)
    {
      dims.push_back(tensor.dims(j));
    }
    layer.allowlistingMetaData.constTensorsDims.push_back(dims);
  }
  //Populate dimensions of output tensors
  for(int outputIdx=0; outputIdx<layer.allowlistingMetaData.numOutputs; outputIdx++)
  {
    layer.allowlistingMetaData.outputTensorDims.push_back(getNodeOutputShape(graph,  graph.node(index).output(outputIdx), 0));
  }

  //Populate data types of input & output tensors
  vector<vector<int32_t>> inputOutputDataTypes = getNodeInputOutputTypes(graph, index);

  layer.allowlistingMetaData.inputDataTypes = inputOutputDataTypes[0];
  layer.allowlistingMetaData.outputDataTypes = inputOutputDataTypes[1];

  return TIDL_ALLOWLISTING_LAYER_CHECK_PASSED;
}
