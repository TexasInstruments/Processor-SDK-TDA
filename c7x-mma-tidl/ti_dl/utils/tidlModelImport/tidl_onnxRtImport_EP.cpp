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

#define ONNX_ML

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
#include <unistd.h>
#include <fstream>
#include <queue>

#include "onnx/onnx-ml.proto3.pb.h"
#include "tidl_onnxRtImport_EP.h"
#include "tidl_runtimes_infer_common.h"
#include "tidl_runtimes_import_common.h"
#include "tidl_onnxrt_common.h"
#include "ti_dl.h"
#include "tidl_import_diag.h"
#include "tidl_import_api.h"

#define __MODULE__ "[PARSER]"

using namespace std;
using namespace onnx;

static TIDL_OnnxrtEPData * data_ = new TIDL_OnnxrtEPData;



extern "C"
{

GraphCache buildGraphCache(const onnx::GraphProto& graph)
{
  GraphCache cache;
  
  for(const auto &init : graph.initializer())
  {
    cache.initializer_names.insert(init.name());
  }

  for(const auto &out : graph.output())
  {
    cache.graph_output_names.insert(out.name());
  }

  for (const auto &node : graph.node())
  {
    for(const auto &in : node.input())
    {
      if (!in.empty())
      {
        cache.tensor_consumer_count[in]++;
      }
    }
  }

  return cache;
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

/* Read interface options and populate to TIDL runtimes option class */
bool TIDL_populateOptions(std::vector<std::pair<std::string,std::string>> interface_options)
{
  bool status;
  data_->infer_ops.lib = dlopen("libvx_tidl_rt.so", RTLD_LAZY | RTLD_GLOBAL);
  if(! data_->infer_ops.lib)
  {
    TIDL_GLOBAL_REPORT_ERROR("%s ", dlerror());
    status = false;
    return status;
  }
  assert(data_->infer_ops.lib);

  TIDL_rtLoadSymbols(&data_->infer_ops);

  TIDL_OnnxrtEPData * options = data_;
  

  /* This is needed only for ONNX runtime since data_ is a global variable and gets reused when networks are run back to back -
    can result in incorrect option values being used. The default values in the constructor are set only in the global definition and not here
    TODO : This will work but check if there is a better way to handle this */
  TIDL_setDefaultOptions(&options->osrt_options);

  for(auto option : interface_options)
  {
    status = false;
    auto key = option.first;
    auto value = option.second;

    status = TIDL_readInterfaceOptions(&options->osrt_options, key, value, TIDL_IMPORT_MODEL_FORMAT_ONNX_RT);
    if(!status)
    {
      delete options;
      return status;
    }
  }

  TIDL_derivedInterfaceOptions(&options->osrt_options);
  status = TIDL_checkInterfaceOptions(&options->osrt_options);
  if(!status)
  {
    delete options;
    return status;
  }

  TIDL_printInterfaceOptions(&options->osrt_options);

  return status;
}
} //extern "C"


/* This function returns the input/output info of subgraph along with vector of node indices included in the subgraph
/* vector( (subgraph1, (inputs_1, outputs_1)), (subgraph2, (inputs_2, outputs_2)), ... ) */
std::vector<std::pair<std::vector<int>, std::pair<std::vector<std::string>, std::vector<std::string>>>> getSubgraphInfo(GraphProto& onnxGraph, std::vector<std::vector<int>> suportedNodeGroups)
{
  std::vector<std::string> nodeOutputs;
  std::vector<std::string> nodeInputs;
  std::pair<std::vector<std::string>, std::vector<std::string>> nodeInputsOutputs;
  std::vector<std::pair<std::vector<int>, std::pair<std::vector<std::string>, std::vector<std::string>>>> info;
  //vector( (subgraph1, (inputs_1, outputs_1)), (subgraph2, (inputs_2, outputs_2)) )

  for(int i = 0; i < suportedNodeGroups.size(); i++)
  {
    std::vector<int> subgraph = suportedNodeGroups[i];
    //Original graph inputs are not read as nodes, so read them separately - check if any of the input node names of subgraph match the original entire graph input names
    // Need this for loop to check for the actual input of the model graph
    for(int m = 0; m < onnxGraph.input_size(); m++)
    {
      //In some OD networks, initializer can be part of graph inputs - 'isInputInitializer' not needed if user is mandated to run onnxruntime/tools/python/remove_initializer_from_input.py
      bool isInputInitializer = false;
      for(int k = 0; k < onnxGraph.initializer_size(); k++)
      {
        if(strcmp(onnxGraph.initializer(k).name().c_str(), onnxGraph.input(m).name().c_str()) == 0)
        {
          isInputInitializer = true;
          break;
        }
      }
      if(! isInputInitializer)
      {
        for(int j = 0; j < subgraph.size(); j++)
        {
          for(int l = 0; l < onnxGraph.node(subgraph[j]).input_size(); l++)
          {
            if(strcmp(onnxGraph.node(subgraph[j]).input(l).c_str(), onnxGraph.input(m).name().c_str()) == 0)
            {
              nodeInputs.push_back(onnxGraph.input(m).name());
            }
          }
        }
      }
    }

    //save all other inputs and outputs of each subgraph
    for(int j = 0; j < subgraph.size(); j++)
    {
      for(int l = 0; l < onnxGraph.node(subgraph[j]).input_size(); l++)
      {
        for (int k = 0; k < onnxGraph.node_size(); k++)
        {
          for(int m = 0; m < onnxGraph.node(k).output_size(); m++)
          {
            if((strcmp(onnxGraph.node(k).output(m).c_str(), onnxGraph.node(subgraph[j]).input(l).c_str()) == 0)) //check for edge inputs(coming from previous layer) and not constant(e.g. weights, biases, etc.)
            {
              nodeInputs.push_back(onnxGraph.node(subgraph[j]).input(l));
            }
          }
        }
      }
    }
    //outputs
    for(int j = 0; j < subgraph.size(); j++)
    {
      for(int l = 0; l < onnxGraph.node(subgraph[j]).output_size(); l++)
      {
        nodeOutputs.push_back(onnxGraph.node(subgraph[j]).output(l));
      }
    }
#if 0
    //delete common elements in inputs and outputs - this removes all intermediate linking inputs/outputs, what is left gives subgraph inputs/outputs
    std::sort(nodeInputs.begin(), nodeInputs.end());
    nodeInputs.erase(std::unique(nodeInputs.begin(), nodeInputs.end()), nodeInputs.end());
    std::sort(nodeOutputs.begin(), nodeOutputs.end());
    nodeOutputs.erase(std::unique(nodeOutputs.begin(), nodeOutputs.end()), nodeOutputs.end());

    bool match;
    for(int i = 0; i < nodeInputs.size(); i++)
    {
      match = false;
      for(int j = 0; j < nodeOutputs.size(); j++)
      {
        if(nodeInputs[i].compare(nodeOutputs[j]) == 0)
        {
          match = true;
          auto itr = std::find(nodeInputs.begin(), nodeInputs.end(), nodeInputs[i]);
          if (itr != nodeInputs.end()) nodeInputs.erase(itr);
          itr = std::find(nodeOutputs.begin(), nodeOutputs.end(), nodeOutputs[j]);
          if (itr != nodeOutputs.end()) nodeOutputs.erase(itr);
          j--;
        }
      }
      if(match)
      {
        i--;
      }
    }
#endif
    nodeInputsOutputs = std::make_pair(nodeInputs, nodeOutputs);
    info.push_back(std::make_pair(subgraph, nodeInputsOutputs));

    nodeInputs.clear();
    nodeOutputs.clear();
  }
#if 0
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "info.size() = %d ", info.size());
  for(int i = 0; i < info.size(); i++)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "**** Subgraph %d *****", i);
    std::vector<int> subgraph = info[i].first;
    std::vector<std::string> inputs = info[i].second.first;
    std::vector<std::string> outputs = info[i].second.second;
    std::string subgraphInfo="";
    for(int j = 0; j < subgraph.size(); j++){
      subgraphInfo+=std::to_string(subgraph[j]);
      subgraphInfo+=" ";
    }
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s", subgraphInfo.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Inputs --- ");
    for(int j = 0; j < inputs.size(); j++) TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s ", inputs[j].c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Outputs --- ");
    for(int j = 0; j < outputs.size(); j++) TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s ", outputs[j].c_str());
  }
#endif
  return info;
}

/** This function takes the preliminary subgraphs created by onnx runtime and checks for further possible optimizations
 * For given subgraph, check if any of the other subgraphs can be merged into it (if all inputs of other subgraph come from outputs of current subgraph)
 * There is no direct way to get actual inputs of a subgraph in onnx runtime, so find inputs of all the nodes, and all of them should either be outputs of the
 * particular subgraph to be merged into, or among node outputs of same subgraph (edges within subgraph)
 * If subgraph can be merged, merge the node indices of this subgraph into the parent subgraph, and delete the merged subgraph
 * Recursively keep doing this till either only 1 subgraph remains or no further subgraphs can be merged
 * */
std::vector<std::vector<int>> optimizeGraphPartition(GraphProto& onnxGraph, std::vector<std::vector<int>> suportedNodeGroups)
{
  std::vector<std::pair<std::vector<int>, std::pair<std::vector<std::string>, std::vector<std::string>>>> info;

  std::vector<int> subgraph_i, subgraph_j;
  std::vector<std::string> inputs_i, inputs_j;
  std::vector<std::string> outputs_i, outputs_j;
  bool canMergeInput, canMergeSubgraph, mergeDone;
  mergeDone = false;
  canMergeSubgraph = false;

  while(mergeDone == false)
  {
    info = getSubgraphInfo(onnxGraph, suportedNodeGroups);
    for(int i = 0; i < info.size(); i++)
    {
      canMergeSubgraph = false;
      subgraph_i = info[i].first;
      inputs_i = info[i].second.first;
      outputs_i = info[i].second.second;
      for(int j = 0; j < info.size(); j++)
      {
        if(j == i) continue; //do not check subgraph with itself
        canMergeSubgraph = true;
        subgraph_j = info[j].first;
        inputs_j = info[j].second.first;
        outputs_j = info[j].second.second;
        for(int k = 0; k < inputs_j.size(); k++)
        {
          canMergeInput = false;
          for(int l = 0; l < outputs_i.size(); l++)
          {
            if(inputs_j[k].compare(outputs_i[l]) == 0)  // "all" inputs should be output of another subgraph, else cannot merge subgraphs
            {
              canMergeInput = true;
              break;
            }
          }
          for(int l = 0; l < outputs_j.size(); l++)
          {
            if(inputs_j[k].compare(outputs_j[l]) == 0)  // if node input is not the output of another subgraph, check if it is part of edge within the subgraph
            {
              canMergeInput = true;
              break;
            }
          }
          if(outputs_j.size() == 0) canMergeInput = false;
          if(canMergeInput == false)
          {
            canMergeSubgraph = false;  //even if any one of the inputs cannot be merged, subgraph can't be merged
            break;
          }
        }
        if(inputs_j.size() == 0) canMergeSubgraph = false;
        if(canMergeSubgraph)
        {
          suportedNodeGroups.clear();
          //put all supported nodes in subgraph_i, then delete subgraph_j
          subgraph_i.insert(subgraph_i.end(), subgraph_j.begin(), subgraph_j.end());
          info[i].first = subgraph_i;
          info.erase(std::find(info.begin(), info.end(), info[j]));
          for(int m = 0; m < info.size(); m++)
          {
            suportedNodeGroups.push_back(info[m].first); //form new set of subgraphs after merging which will be tested recursively
          }
          break;
        }
      }
      if(canMergeSubgraph) break;  //if merge occurs, stop testing current set of subgraphs, and start afresh on new set of merged subgraphs
    }
    if(canMergeSubgraph == false)
    {
      mergeDone = true;   //if none of the subgraphs can be merged into each other, merge is complete
    }
  }
  return suportedNodeGroups;
}

/** Evict any node whose scalar (0-dim) output is consumed by a non-TIDL (CPU) node.
 * Scalar tensors flowing between TIDL nodes (same or different subgraph) are allowed.
 * Only when the consumer runs on CPU does the scalar cross a real boundary, since TIDL
 * coerces 0-dim tensors to shape [1] at import time which would corrupt the CPU consumer.
 *
 * Runs as a fixpoint: evicting a node may expose further nodes whose consumers are now
 * on CPU, so passes repeat until no evictions occur.
 */
void TIDL_removeScalarBoundaryNodes(GraphProto& onnxGraph,
                                    std::vector<std::vector<int>>& suportedNodeGroups,
                                    const std::vector<int>& sortedNodeIndices)
{
  bool anyEviction = true;
  while (anyEviction)
  {
    anyEviction = false;

    /* rebuild allTidlNodes at the start of each pass to reflect prior evictions */
    std::set<int> allTidlNodes;
    for (const auto& sg : suportedNodeGroups)
      for (int idx : sg)
        allTidlNodes.insert(idx);

    for (auto& subgraph : suportedNodeGroups)
    {
      std::vector<int> toRemove;

      for (int nodeIdx : subgraph)
      {
        for (int outIdx = 0; outIdx < onnxGraph.node(nodeIdx).output_size(); outIdx++)
        {
          const std::string& outputName = onnxGraph.node(nodeIdx).output(outIdx);

          /* check if this output is a confirmed 0-dim scalar via shape inference */
          bool isScalar = false;
          for (const auto& vi : onnxGraph.value_info())
          {
            if (vi.name() == outputName &&
                vi.type().has_tensor_type() &&
                vi.type().tensor_type().has_shape() &&
                vi.type().tensor_type().shape().dim().size() == 0)
            {
              isScalar = true;
              break;
            }
          }
          if (!isScalar) continue;

          /* scalar output: check whether any consumer is a non-TIDL (CPU) node */
          bool hasNonTidlConsumer = false;
          for (int k = 0; k < onnxGraph.node_size() && !hasNonTidlConsumer; k++)
          {
            if (allTidlNodes.count(k)) continue; /* TIDL node – OK */
            for (int l = 0; l < onnxGraph.node(k).input_size(); l++)
            {
              if (onnxGraph.node(k).input(l) == outputName)
              {
                hasNonTidlConsumer = true;
                break;
              }
            }
          }

          if (hasNonTidlConsumer)
          {
            toRemove.push_back(nodeIdx);
            allTidlNodes.erase(nodeIdx);
            int diagIndex = std::find(sortedNodeIndices.begin(), sortedNodeIndices.end(), nodeIdx)
                            - sortedNodeIndices.begin();
            TIDL_LOG_UNSUPPORTED_AT(gDiags.gDiagList, diagIndex,
              "Node (%s) - OP (%s) : Scalar (0-dim) output consumed by non-TIDL node, denying",
              onnxGraph.node(nodeIdx).name().c_str(),
              onnxGraph.node(nodeIdx).op_type().c_str());
            break; /* no need to check remaining outputs of this node */
          }
        }
      }

      for (int idx : toRemove)
      {
        subgraph.erase(std::find(subgraph.begin(), subgraph.end(), idx));
        anyEviction = true;
      }
    }
  }

  /* drop subgraphs emptied by evictions */
  suportedNodeGroups.erase(
    std::remove_if(suportedNodeGroups.begin(), suportedNodeGroups.end(),
                   [](const std::vector<int>& sg){ return sg.empty(); }),
    suportedNodeGroups.end());
}

/** This function takes the optimized subgraphs created by onnx runtime and forcefully divides it into chunks
 * of multiple subgraphs evenly if number of nodes in the subgraph is > numMaxNodes. Size of each chunk is <= numMaxNodes.
 * TODO: Implement a cost function to optimally find points of breakage
 * */
std::vector<std::vector<int>> divideGraphPartition(GraphProto& onnxGraph, std::vector<std::vector<int>> suportedNodeGroups, int32_t numMaxNodes, std::vector<std::string> supportedFusedNodesNames)
{
  std::vector<std::vector<int>> suportedNodeGroupsSubdivided = {};
  for (int i = 0; i < suportedNodeGroups.size(); i++)
  {
    if (suportedNodeGroups[i].size() <= numMaxNodes)
    {
      suportedNodeGroupsSubdivided.push_back(suportedNodeGroups[i]);
    }
    else
    {
      std::vector<int> tempNodeGroups = {};
      int x = 0;
      int y;

      // Calculate number of nodes to divide evenly
      float numChuncks = (float)(suportedNodeGroups[i].size())/(float)(numMaxNodes);
      numChuncks = std::ceil(numChuncks);
      int temp = (int)(suportedNodeGroups[i].size()/numChuncks);

      while (x < suportedNodeGroups[i].size())
      {
        y = x + temp;
        if (y >= suportedNodeGroups[i].size())
        {
          y = suportedNodeGroups[i].size() - 1;
        }
        else
        {
          // Avoid breaking at nodes which is part of supportedFusedCombination
          std::string nodeName;
          while(y >= x)
          {
            nodeName = onnxGraph.node(suportedNodeGroups[i][y]).name();
            if (std::find(supportedFusedNodesNames.begin(), supportedFusedNodesNames.end(), nodeName) == supportedFusedNodesNames.end())
            {
              break;
            }
            y--;
          }
        }

        for (int j = x; j <= y; j++)
        {
          tempNodeGroups.push_back(suportedNodeGroups[i][j]);
        }
        suportedNodeGroupsSubdivided.push_back(tempNodeGroups);
        tempNodeGroups.clear();
        x = y + 1;
      }
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nNumber of nodes in subgraph exceeds max_num_subgraph_nodes(%d). Dividing evenly into %ld subgraphs.", numMaxNodes, suportedNodeGroupsSubdivided.size());
    }
  }
  return suportedNodeGroupsSubdivided;
}

std::pair<int,int> computeSubgraphIO(const onnx::GraphProto& graph,const GraphCache& graph_cache,const std::vector<int>& subgraph_node_ids)
{
  std::unordered_set<std::string> produced_inside;
  std::unordered_map<std::string, int> internal_consumer_count;

  //Collect produced tensors
  for(int nid : subgraph_node_ids)
  {
    const auto &node = graph.node(nid);
    for (const auto &out : node.output())
    {
      if(!out.empty())
      {
        produced_inside.insert(out);
      }
    }
  }

  //Collect internal consumers
  for(int nid : subgraph_node_ids)
  {
    const auto &node = graph.node(nid);
    for(const auto &in : node.input())
    {
      if(in.empty())
      {
        continue;
      }
      if(produced_inside.count(in))
      {
        internal_consumer_count[in]++;
      }
    }
  }

  std::vector<std::string> subgraph_inputs;
  std::vector<std::string> subgraph_outputs;

  //Compute subgraph inputs
  for(int nid : subgraph_node_ids)
  {
    const auto &node = graph.node(nid);
    for(const auto &in : node.input())
    {
      if (in.empty())
      {
        continue;
      }
      //node is graph input if is not produced inside and it is not initializer
      if(produced_inside.count(in) == 0 && graph_cache.initializer_names.count(in) == 0)
      {
        subgraph_inputs.push_back(in);
      }
    }
  }

  std::sort(subgraph_inputs.begin(),subgraph_inputs.end());
  subgraph_inputs.erase(std::unique(subgraph_inputs.begin(), subgraph_inputs.end()),subgraph_inputs.end());

  //Compute subgraph outputs

  for(const auto& tensor : produced_inside)
  {
    int total_consumers = 0;
    auto it = graph_cache.tensor_consumer_count.find(tensor);
    if (it != graph_cache.tensor_consumer_count.end())
    {
      total_consumers = it->second;
    }
    int internal_consumers = internal_consumer_count[tensor];

    bool used_outside = total_consumers > internal_consumers;
    bool is_graph_output = graph_cache.graph_output_names.count(tensor);

    if(used_outside || is_graph_output)
    {
      subgraph_outputs.push_back(tensor);
    }
  }
  return {subgraph_inputs.size(),subgraph_outputs.size()};
}

std::vector<std::vector<int>> divideGraphPartitionIO(GraphProto& onnxGraph,const GraphCache& graph_cache, std::vector<std::vector<int>> supportedNodeGroups, std::vector<std::string> supportedFusedNodesNames)
{
  std::vector<std::vector<int>> suportedNodeGroupsSubdivided = {};
  for (int i = 0; i < supportedNodeGroups.size(); i++)
  {
    std::pair<int,int> graph_io_size = computeSubgraphIO(onnxGraph,graph_cache,supportedNodeGroups[i]);
    if ( graph_io_size.first <= TIDL_MAX_ALG_IN_BUFS && graph_io_size.second <= TIDL_MAX_ALG_OUT_BUFS)
    {
      suportedNodeGroupsSubdivided.push_back(supportedNodeGroups[i]);
    }
    else
    {
      std::vector<int> tempNodeGroups = {};
      int partitionStart = 0;
      int currentIdx = 0;
      while (currentIdx < supportedNodeGroups[i].size())
      {
        tempNodeGroups.push_back(supportedNodeGroups[i][currentIdx]);
        std::pair<int,int> temp_graph_io_size = computeSubgraphIO(onnxGraph,graph_cache,tempNodeGroups);
        if ( temp_graph_io_size.first > TIDL_MAX_ALG_IN_BUFS || temp_graph_io_size.second > TIDL_MAX_ALG_OUT_BUFS)
        {
          tempNodeGroups.pop_back();
          std::string nodeName;
          currentIdx -= 1;
          while(currentIdx >= partitionStart)
          {
            nodeName = onnxGraph.node(supportedNodeGroups[i][currentIdx]).name();
            if (std::find(supportedFusedNodesNames.begin(), supportedFusedNodesNames.end(), nodeName) == supportedFusedNodesNames.end())
            {
              break;
            }
            tempNodeGroups.pop_back();
            currentIdx--;
          }
          if (tempNodeGroups.size() > 0)
          {
            suportedNodeGroupsSubdivided.push_back(tempNodeGroups);
          }
          tempNodeGroups.clear();
          partitionStart = currentIdx + 1;
          currentIdx = currentIdx + 1;
        }
        else
        {
          currentIdx += 1;
        }
      }
      if(tempNodeGroups.size() > 0)
      {
        suportedNodeGroupsSubdivided.push_back(tempNodeGroups);
      }
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nSubgraph %d further subdivided to satisfy max input/output constraints", i);
    }
  }
  return suportedNodeGroupsSubdivided;
}


std::vector<int> TIDL_getInputNodes(int idx, GraphProto& onnxGraph);
std::vector<int> TIDL_getOutputNodes(int idx, GraphProto& onnxGraph);
bool TIDL_checkShuffleOperation(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices);
bool TIDL_checkGELUOperation(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices);
bool TIDL_checkLNOperation(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices);
bool TIDL_checkPatchMerging(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices);
bool TIDL_checkMSDAMulMatmul(GraphProto& onnxGraph, std::vector<int>supportedNodesIndices);


/* Supported fused combinations in TIDL - {Operation name : {Layers as part of operation} } */
std::vector<std::pair<std::string, std::vector<std::string>>> fusedCombinations =
{
  {"Shuffle",                           {"Reshape", "Transpose", "Reshape"}},
  {"Pointpillar",                       {"ScatterElements", "Reshape"}},
  {"GELU",                              {"Div", "Erf", "Add", "Mul", "Mul"}},
  {"Logit",                             {"Clip", "Sub", "Clip", "Clip", "Div", "Log"}},
  {"LayerNormalization",                {"ReduceMean", "Sub", "Pow", "ReduceMean", "Add", "Sqrt", "Div"}},
  {"PatchMerging",                      {"Reshape", "Slice", "Slice", "Slice", "Slice", "Slice", "Slice", "Slice", "Slice", "Concat"}},
  {"DeformConv",                        {"Concat","Unsqueeze","Div","Add","Slice","Unsqueeze","Div","Add","Slice","Mul","Sub","Reshape","Gridsample","Mul"}},
  {"MSDAMulMatmul",                     {"Mul","MatMul"}},
};

/** After identifying a sequence of layers which can possibly be converted to a supported fused operator
 * add checks for individual layers in this combination to check if they satisfy constraints imposed by the fused operator
  */
std::vector<std::vector<int>> TIDL_checkFusedCombinationSupported(std::vector<std::pair<int, std::vector<int>>> supportedFusedCombinations, GraphProto& onnxGraph)
{
  std::vector<std::vector<int>> supportedNodes = {};
  bool isSupported;
  for(auto & supportedFusedCombination : supportedFusedCombinations)
  {
    isSupported = false;
    if(fusedCombinations[supportedFusedCombination.first].first.compare("Shuffle") == 0) /* Checks for shuffle*/
    {
      isSupported = TIDL_checkShuffleOperation(onnxGraph, supportedFusedCombination.second); /*onnxGraph, supportedNodesIndices*/
    }
    else if (fusedCombinations[supportedFusedCombination.first].first.compare("GELU") == 0)  /* checks for GELU*/
    {
      isSupported = TIDL_checkGELUOperation(onnxGraph, supportedFusedCombination.second);
    }
    else if (fusedCombinations[supportedFusedCombination.first].first.compare("LayerNormalization") == 0)  /* Check for LayerNormalization*/
    {
      isSupported = TIDL_checkLNOperation(onnxGraph, supportedFusedCombination.second);
    }
    else if (fusedCombinations[supportedFusedCombination.first].first.compare("PatchMerging") == 0)  /* Check for Patch Merging*/
    {
      isSupported = TIDL_checkPatchMerging(onnxGraph, supportedFusedCombination.second);
    }
    else if (fusedCombinations[supportedFusedCombination.first].first.compare("DeformConv") == 0)  /* Check for DCNv2*/
    {
      /*TBD: Fill in parameter filters:*/
      isSupported = true;
    }
    else if (fusedCombinations[supportedFusedCombination.first].first.compare("MSDAMulMatmul") == 0)  /* Check for MSDAMulMatmul*/
    {
      isSupported = TIDL_checkMSDAMulMatmul(onnxGraph, supportedFusedCombination.second);
    }
    else if (fusedCombinations[supportedFusedCombination.first].first.compare("Logit") == 0)  /* Check for Logit*/
    {
      /*TBD: Fill in parameter filters:*/
      isSupported = true;
    }
    else /* Replace this else condition with pointpillars specific check */
    {
      isSupported = true;
    }

    /* If current combination is supported, add corresponding nodes to final supported nodes list */
    if(isSupported)
    {
      supportedNodes.push_back(supportedFusedCombination.second);
    }
  }
  return supportedNodes;
}

/* TIDL fused combinations:
 * Purpose of using shared pointer instead of normal pointer is that we are pointing more then one pointer to same location
 * and since we must delete the dynamically allocated memory, we may face issues in deleting such pointers. shared_ptr gives us 
 * a reference count which tells how many pointers are pointing to same location at that time. reset() method in such pointers 
 * decrese this count by 1 and once it becomes 0 then the actual memory gets destroyed.
*/
std::vector<std::pair<std::string,shared_ptr<TIDL_fusedNode>>> TIDL_createFusedCombinations()
{
  /* {"Shuffle",                           {"Reshape", "Transpose", "Reshape"}} */
    shared_ptr<TIDL_fusedNode> shuffle(new TIDL_fusedNode("Reshape"));
    shared_ptr<TIDL_fusedNode> transpose(new TIDL_fusedNode("Transpose"));
    shared_ptr<TIDL_fusedNode> reshape(new TIDL_fusedNode("Reshape"));
    shuffle->inputs = {};
    shuffle->outputs = {transpose};
    
    transpose->inputs = {shuffle};
    transpose->outputs = {reshape};

    reshape->inputs = {transpose};
    reshape->outputs = {};

    /* delete dummy pointers, reset will decrease the reference count to the location by 1 and when this count becomes 0 then the memory gets destroyed */
    transpose.reset();
    reshape.reset();

  /* {"Pointpillar",                       {"ScatterElements", "Reshape"}} */
    shared_ptr<TIDL_fusedNode> pointpillar(new TIDL_fusedNode("ScatterElements"));
    reshape = shared_ptr<TIDL_fusedNode>(new TIDL_fusedNode("Reshape"));
    pointpillar->inputs = {};
    pointpillar->outputs = {reshape};

    reshape->inputs = {pointpillar};
    reshape->outputs = {};

    /* delete dummy pointers, reset will decrease the reference count to the location by 1 and when this count becomes 0 then the memory gets destroyed */
    reshape.reset();

  /* {"GELU",                              {"Div", "Erf", "Add", "Mul"}} */
    shared_ptr<TIDL_fusedNode> gelu(new TIDL_fusedNode("Div"));
    shared_ptr<TIDL_fusedNode> erf(new TIDL_fusedNode("Erf"));
    shared_ptr<TIDL_fusedNode> add(new TIDL_fusedNode("Add"));
    shared_ptr<TIDL_fusedNode> mul(new TIDL_fusedNode("Mul"));
    shared_ptr<TIDL_fusedNode> mul2(new TIDL_fusedNode("Mul"));

    gelu->inputs = {};
    gelu->outputs = {erf};
    
    erf->inputs = {gelu};
    erf->outputs = {add};

    add->inputs = {erf};
    add->outputs = {mul};

    mul->inputs = {add, NULL};  /* NULL means this input is not defined in the pattern but it can be present in onnx graph */
    mul->outputs = {mul2};

    mul2->inputs = {mul, NULL};
    mul2->outputs = {};
    
    /* delete dummy pointers, reset will decrease the reference count to the location by 1 and when this count becomes 0 then the memory gets destroyed */
    erf.reset();
    add.reset();
    mul.reset();
    mul2.reset();

  /* {"LayerNormalization",                {"ReduceMean", "Sub", "Pow", "ReduceMean", "Add", "Sqrt", "Div"}} */
    shared_ptr<TIDL_fusedNode> layerNormalization(new TIDL_fusedNode("ReduceMean"));
    shared_ptr<TIDL_fusedNode> sub(new TIDL_fusedNode("Sub"));
    shared_ptr<TIDL_fusedNode> pow(new TIDL_fusedNode("Pow"));
    shared_ptr<TIDL_fusedNode> reduceMean(new TIDL_fusedNode("ReduceMean"));
    add = shared_ptr<TIDL_fusedNode>(new TIDL_fusedNode("Add"));
    shared_ptr<TIDL_fusedNode> sqrt(new TIDL_fusedNode("Sqrt"));
    shared_ptr<TIDL_fusedNode> div(new TIDL_fusedNode("Div"));

    layerNormalization->inputs = {};
    layerNormalization->outputs = {sub};

    sub->inputs = {NULL, layerNormalization};
    sub->enforceInputOrder = true;
    sub->outputs = {pow, div};

    pow->inputs = {sub};
    pow->outputs = {reduceMean};
    
    reduceMean->inputs = {pow};
    reduceMean->outputs = {add};

    add->inputs = {reduceMean};
    add->outputs = {sqrt};

    sqrt->inputs = {add};
    sqrt->outputs = {div};

    div->inputs = {sub, sqrt};
    div->enforceInputOrder = true;
    div->outputs = {};

    /* delete dummy pointers, reset will decrease the reference count to the location by 1 and when this count becomes 0 then the memory gets destroyed */
    sub.reset();
    pow.reset();
    reduceMean.reset();
    add.reset();
    sqrt.reset();
    div.reset();

  /* {"PatchMerging",                      {"Reshape", "Slice", "Slice", "Slice", "Slice", "Slice", "Slice", "Slice", "Slice", "Concat"}} */
    shared_ptr<TIDL_fusedNode> patchMerging(new TIDL_fusedNode("Reshape"));
    patchMerging->inputs = {};
    patchMerging->outputs = {shared_ptr<TIDL_fusedNode>(new TIDL_fusedNode("Slice")), shared_ptr<TIDL_fusedNode>(new TIDL_fusedNode("Slice")), 
                              shared_ptr<TIDL_fusedNode>(new TIDL_fusedNode("Slice")), shared_ptr<TIDL_fusedNode>(new TIDL_fusedNode("Slice"))};
    shared_ptr<TIDL_fusedNode> concat(new TIDL_fusedNode("Concat"));
    for(int i=0;i<4;i++)
    {
      shared_ptr<TIDL_fusedNode> slice = patchMerging->outputs[i];
      slice->inputs = {patchMerging};
      slice->outputs = {shared_ptr<TIDL_fusedNode>(new TIDL_fusedNode("Slice"))};
      slice->outputs[0]->inputs = {slice};
      slice->outputs[0]->outputs = {concat};
      concat->inputs.push_back(slice->outputs[0]);
      slice.reset();
    }
    concat->outputs = {};

    /* {"MSDAMulMatmul", {"Mul", "MatMul"}} */
    shared_ptr<TIDL_fusedNode> msdaMulMatmul(new TIDL_fusedNode("Mul"));
    shared_ptr<TIDL_fusedNode> matmul(new TIDL_fusedNode("MatMul"));
    msdaMulMatmul->inputs = {};
    msdaMulMatmul->outputs = {matmul};

    matmul->inputs = {msdaMulMatmul};
    matmul->outputs = {};

    matmul.reset();

    /* {"DeformConv"
      Note: Only the unsupported slice branches are being detected as the current mechanism does not support multi-input 
            pattern matches.
      INPUT---->CONV_OFFSET->Slice0->Add0->Div0->Unsq0->Concat0->Mul0->Sub0->Reshape0->grid_0->mul_1->Reshape_2->Conv1
        \  \                 \Slice1->Add1->Div1->Unsq1/                               /       /
        \  --->PAD--------------------------------------------------------------------       /
          ----->CONV_MASK->Sigmoid->Reshape_1------------------------------------------------/
      Becomes:
      INPUT---->CONV_OFFSET---->DCN
          \--->PAD------------/ |
          \--->CONV_Mask->Sigmoid 
     */
    /*Only detecting the unsupported branch Slice->add->div_unsq->concat->mul->sub->gridsample:*/
    
    shared_ptr<TIDL_fusedNode> deformConv(new TIDL_fusedNode("Concat")); 

    shared_ptr<TIDL_fusedNode> dcn_slice_0(new TIDL_fusedNode("Slice")); 
    shared_ptr<TIDL_fusedNode> dcn_add_0(new TIDL_fusedNode("Add"));
    shared_ptr<TIDL_fusedNode> dcn_div_0(new TIDL_fusedNode("Div"));
    shared_ptr<TIDL_fusedNode> dcn_unsqueeze_0(new TIDL_fusedNode("Unsqueeze"));

    shared_ptr<TIDL_fusedNode> dcn_slice_1(new TIDL_fusedNode("Slice")); 
    shared_ptr<TIDL_fusedNode> dcn_add_1(new TIDL_fusedNode("Add"));
    shared_ptr<TIDL_fusedNode> dcn_div_1(new TIDL_fusedNode("Div"));
    shared_ptr<TIDL_fusedNode> dcn_unsqueeze_1(new TIDL_fusedNode("Unsqueeze"));

    shared_ptr<TIDL_fusedNode> dcn_mul_0(new TIDL_fusedNode("Mul"));
    shared_ptr<TIDL_fusedNode> dcn_sub(new TIDL_fusedNode("Sub"));
    shared_ptr<TIDL_fusedNode> dcn_reshape(new TIDL_fusedNode("Reshape"));
    shared_ptr<TIDL_fusedNode> dcn_grid(new TIDL_fusedNode("GridSample"));
    shared_ptr<TIDL_fusedNode> dcn_mul_1(new TIDL_fusedNode("Mul"));

    dcn_slice_0->inputs = {};
    dcn_slice_0->outputs = {dcn_add_0};

    dcn_slice_1->inputs = {};
    dcn_slice_1->outputs = {dcn_add_1};

    dcn_add_0->inputs = {dcn_slice_0};
    dcn_add_0->outputs = {dcn_div_0};

    dcn_add_1->inputs = {dcn_slice_1};
    dcn_add_1->outputs = {dcn_div_1};

    dcn_div_0->inputs = {dcn_add_0};
    dcn_div_0->outputs = {dcn_unsqueeze_0};

    dcn_div_1->inputs = {dcn_add_1};
    dcn_div_1->outputs = {dcn_unsqueeze_1};

    dcn_unsqueeze_0->inputs = {dcn_div_0};
    dcn_unsqueeze_0->outputs = {deformConv};

    dcn_unsqueeze_1->inputs = {dcn_div_1};
    dcn_unsqueeze_1->outputs = {deformConv};

    deformConv->inputs = {dcn_unsqueeze_0, dcn_unsqueeze_1};
    deformConv->outputs = {dcn_mul_0};

    dcn_mul_0->inputs = {deformConv};
    dcn_mul_0->outputs = {dcn_sub};

    dcn_sub->inputs = {dcn_mul_0};
    dcn_sub->outputs = {dcn_reshape};

    dcn_reshape->inputs = {dcn_sub};
    dcn_reshape->outputs = {dcn_grid};

    dcn_grid->inputs = {NULL, dcn_reshape};
    dcn_grid->enforceInputOrder = true;
    dcn_grid->outputs = {dcn_mul_1};

    dcn_mul_1->inputs = {dcn_grid, NULL};
    dcn_mul_1->outputs = {};

    /* delete dummy pointers, reset will decrease the reference count to the location by 1 and when this count becomes 0 then the memory gets destroyed */
    dcn_slice_0.reset();
    dcn_add_0.reset();
    dcn_div_0.reset();
    dcn_unsqueeze_0.reset();
    dcn_slice_1.reset();
    dcn_add_1.reset();
    dcn_div_1.reset();
    dcn_unsqueeze_1.reset();
    dcn_mul_0.reset();
    dcn_sub.reset();
    dcn_reshape.reset();
    dcn_grid.reset();
    dcn_mul_1.reset();

  /* {"Logit",                              {"Clip", "Sub", "Clip", "Clip", "Div", "Log"}} */
    shared_ptr<TIDL_fusedNode> logit(new TIDL_fusedNode("Clip"));
    shared_ptr<TIDL_fusedNode> sub_logit(new TIDL_fusedNode("Sub"));
    shared_ptr<TIDL_fusedNode> clip1_logit(new TIDL_fusedNode("Clip"));
    shared_ptr<TIDL_fusedNode> clip2_logit(new TIDL_fusedNode("Clip"));
    shared_ptr<TIDL_fusedNode> div_logit(new TIDL_fusedNode("Div"));
    shared_ptr<TIDL_fusedNode> log_logit(new TIDL_fusedNode("Log"));

    logit->inputs = {};
    logit->outputs = {sub_logit, clip1_logit};
    
    sub_logit->inputs = {logit};
    sub_logit->outputs = {clip2_logit};

    clip1_logit->inputs = {logit};
    clip1_logit->outputs = {div_logit};

    clip2_logit->inputs = {sub_logit};  
    clip2_logit->outputs = {div_logit};

    div_logit->inputs = {clip1_logit, clip2_logit};
    div_logit->outputs = {log_logit};

    log_logit->inputs = {div_logit};
    log_logit->outputs = {};
    
    /* delete dummy pointers, reset will decrease the reference count to the location by 1 and when this count becomes 0 then the memory gets destroyed */
    sub_logit.reset();
    clip1_logit.reset();
    clip2_logit.reset();
    div_logit.reset();
    log_logit.reset();

  std::vector<std::pair<std::string,shared_ptr<TIDL_fusedNode>>> fusedCombinations = 
  {
      {"Shuffle",                           shuffle},
      {"Pointpillar",                       pointpillar},
      {"GELU",                              gelu},
      {"Logit",                             logit},
      {"LayerNormalization",                layerNormalization},
      {"PatchMerging",                      patchMerging},
      {"DeformConv",                        deformConv},
      {"MSDAMulMatmul",                     msdaMulMatmul}
  };
  return fusedCombinations;
}

void TIDL_deleteFusedNode(shared_ptr<TIDL_fusedNode> pattern)
{
  for(int i=0;i<pattern->outputs.size();i++)
  {
    TIDL_deleteFusedNode(pattern->outputs[i]);
  }
  pattern.reset();
}

bool TIDL_checkFusedPattern(int idx, shared_ptr<TIDL_fusedNode> node, std::vector<int>&fusedNodes, GraphProto& onnxGraph)
{
  if(node == NULL)
  {
    return false;
  }

  if(std::find(fusedNodes.begin(), fusedNodes.end(),idx) != fusedNodes.end())
  {
    return true;
  }

  /* adding this check here also in case if some in-between node does not have shapes(that node will not go under check present in TIDL_onnxAllowlistFusedLayers func) */
  if(TIDL_checkLayerInputOutputDimsExist(onnxGraph, idx) == -1)
  {
    return false;
  }

  fusedNodes.push_back(idx);
  std::vector<int> inputs = TIDL_getInputNodes(idx, onnxGraph);
  std::vector<int> outputs = TIDL_getOutputNodes(idx, onnxGraph);

  /* each input of node in the pattern must be present in inputs of onnx node */
  if(node->inputs.size()>0)
  {
    if(node->inputs.size() != inputs.size())
    {
      return false;
    }
    for(int i = 0;i < node->inputs.size(); i++)
    {
      /* NULL means this input is not defined in the pattern but it can be present in onnx graph, for example sub node in layerNorm pattern */
      if(node->inputs[i]==NULL)
      {
        continue;
      }

      bool found = false;

      if (node->enforceInputOrder == true)
      {
        /* Skiping inputs which are coming from network main inputs */
        if(inputs[i] == -1)
        {
          continue;
        }
        if (onnxGraph.node(inputs[i]).op_type().compare(node->inputs[i]->opType) == 0)
        {
          found = true;
        }
      }
      else
      {
        for(int j = 0; j < inputs.size(); j++)
        {
          /* Skiping inputs which are coming from network main inputs */
          if(inputs[j] == -1)
          {
            continue;
          }
          if(onnxGraph.node(inputs[j]).op_type().compare(node->inputs[i]->opType) == 0)
          {
            found = true;
            break;
          }
        }
      }

      if(found == false)
      {
        return false;
      }
    }
  }
  /* each output of node in the pattern must be present in outputs of onnx node */
  if(node->outputs.size()>0)
  {
    if(node->outputs.size() != outputs.size())
    {
      return false;
    }

    for(int i=0;i<node->outputs.size();i++)
    {
      bool found = false;
      for(int j=0;j<outputs.size();j++)
      {
        /* Skiping outputs which are coming from network main outputs */
        if(outputs[j] == -1)
        {
          continue;
        }
        if(onnxGraph.node(outputs[j]).op_type().compare(node->outputs[i]->opType) == 0)
        {
          found = true;
          break;
        }
      }
      if(found == false)
      {
        return false;
      }
    }
  }

  for(int i = 0; i < node->inputs.size(); i++)
  {
    if(node->inputs[i] == NULL)
    {
      continue;
    }

    bool found = false;

    if (node->enforceInputOrder == true)
    {
      /* Skiping inputs which are coming from network main inputs */
      if(inputs[i] == -1)
      {
        continue;
      }
      if (TIDL_checkFusedPattern(inputs[i],node->inputs[i],fusedNodes,onnxGraph))
      {
        found = true;
      }
    }
    else
    {
      for(int j = 0; j < inputs.size(); j++)
      {
        /* Skiping inputs which are coming from network main inputs */
        if(inputs[j] == -1)
        {
          continue;
        }
        if(std::find(fusedNodes.begin(), fusedNodes.end(),inputs[j]) != fusedNodes.end())
        {
          found = true;
          continue;
        }

        if(TIDL_checkFusedPattern(inputs[j],node->inputs[i],fusedNodes,onnxGraph))
        {
          found = true;
          break;
        }
      }
    }

    if(found == false)
    {
      return false;
    }
  }

  /* last node in the pattern */
  if(node->outputs.size() == 0)
  {
    return true;
  }

  /* now verify all outputs of curr node matches with our pattern node */
  for(int i1=0;i1<outputs.size();i1++)
  {
    /* Skiping outputs which are coming from network main outputs */
    if(outputs[i1] == -1)
    {
      continue;
    }

    bool found = false;
    for(int i2=0;i2<node->outputs.size();i2++)
    {
      if(TIDL_checkFusedPattern(outputs[i1],node->outputs[i2],fusedNodes,onnxGraph))
      {
        found = true;
      }
      if(found)
        break;
    }
    if(found == false)
    {
      return false;
    }
  }

  

  return true;
}

/** Logic for allowlisting possible fusable combinations of layers:
 * first check if the curr node is matching with any pattern present in our fusedCombinations
 * if the node matches with one of the fusedCombinations then check if that combination is present in the onnx graph or not:
 * - all inputs/outputs of curr node in onnx graph should match with corresponding inputs/outputs in our defined pattern
 * - for each output follow the same procedure recursively
 * - if all nodes in the pattern got matched then we store indices of those nodes in fusedNodes and keep it in supportedFusedCombinations as pair of {patternIdx, fusedNodes}
 * - if any of the node does not fulfill the condition in our pattern then we say pattern is not matched and return false
 * - if we find the pattern then the nodes in that pattern will not be traversed again as we added them to supportedNodes - means don't check fused allowlisting
 *   for these nodes
 * 
 * Once we get all our fused patterns then we do some pattern specific checks in TIDL_checkFusedCombinationSupported function
 **/
std::vector<std::vector<int>> TIDL_onnxAllowlistFusedLayers(GraphProto& onnxGraph)
{
  std::vector<std::pair<std::string, shared_ptr<TIDL_fusedNode>>> fusedCombinations = TIDL_createFusedCombinations();
  std::vector<std::pair<int, std::vector<int>>> supportedFusedCombinations = {}; /* < Combination idx, {Node indices in combination} > */
  std::set<int> supportedNodes;

  for(int i=0;i<onnxGraph.node_size();i++)
  {
    /* check input/output shapes - if shapes are not there then do not check fused combinations, this will be denied */
    if(TIDL_checkLayerInputOutputDimsExist(onnxGraph, i) == -1)
    {
      continue;
    }
    /* if already matched with a pattern */
    if(supportedNodes.count(i))
    {
      continue;
    }
    /* check if any pattern starts with curr operator */
    for(int j=0;j<fusedCombinations.size();j++)
    {
      auto pattern = fusedCombinations[j];
      if(pattern.second != NULL && onnxGraph.node(i).op_type().compare(pattern.second->opType) == 0)
      {
        std::vector<int> fusedNodes;
        bool status = TIDL_checkFusedPattern(i, pattern.second, fusedNodes, onnxGraph);
        if(status)
        {
          supportedNodes.insert(fusedNodes.begin(), fusedNodes.end());
          supportedFusedCombinations.push_back({j,fusedNodes});
          break;
        }
      }
    }
  } 

  /* delete dynamically allocated patterns */
  for(auto pattern: fusedCombinations)
  {
    TIDL_deleteFusedNode(pattern.second);
  }

  return TIDL_checkFusedCombinationSupported(supportedFusedCombinations, onnxGraph);;
}

/* Generates and prints the parse table */
void TIDL_generateParseTable(GraphProto &onnxGraph, std::vector<int> &sortedNodeIndices, int32_t numSuportedNodes, int32_t numOffloadSubGraphCreated)
{
  std::string supportedNodes = std::to_string(numSuportedNodes);
  std::string offloadSubGraph = std::to_string(numOffloadSubGraphCreated);
  std::string unsupportedNodes = std::to_string(onnxGraph.node_size() - numSuportedNodes);
  std::vector<std::vector<std::string>> denylistData = {};

  // Use common diagnostic parsing function with ONNXRT-specific node info extractor
  TIDL_buildParseTableFromDiags(
    gDiags.gDiagList,
    denylistData,
    [&](int index) -> std::pair<std::string, std::string> {
      // Extract opType and nodeName from onnxGraph (ONNXRT has full graph context)
      int i = sortedNodeIndices[index];
      std::string opType = onnxGraph.node(i).op_type();
      std::string nodeName = "";
      if(!onnxGraph.node(i).name().empty())
      {
        nodeName = onnxGraph.node(i).name();
      }
      else
      {
        nodeName = onnxGraph.node(i).output(0);
      }
      return {opType, nodeName};
    }
  );

  TIDL_printParseTable(supportedNodes, offloadSubGraph, unsupportedNodes, denylistData);
}

int64_t calculateSubgraphCost(GraphProto& onnxGraph, std::vector<int> &subgraph, std::map<std::string,int64_t> &costMap)
{
  int64_t cost = 0;
  for (int i = 0; i < subgraph.size(); i++)
  {
    std::string key = onnxGraph.node(subgraph[i]).name();
    if (costMap.find(key) != costMap.end())
    {
      cost += costMap[key];
    }
    else
    {
      //TODO : Think about it
    }
  }
  return cost;
}

/** Store all information needed for graph visualization like input/output adjacent nodes, node names, etc and
  * write to graphvizInfo.txt, which will be used by graphviz executable for visualization */
void TIDL_getGraphVisualizationInfo(GraphProto& onnxGraph, std::vector<int> &sortedNodeIndices, std::vector<std::vector<int>> outputAdjacencyList, std::vector<std::vector<int>> inputAdjacencyList)
{
  sTIDL_runtimesVisualisationInfo_t * visInfo = new sTIDL_runtimesVisualisationInfo_t[onnxGraph.node_size()];
  for(int index = 0; index < sortedNodeIndices.size(); index++)
  {
    int i = sortedNodeIndices[index];
    visInfo[i].nodeModelIdx = i;
    if(! onnxGraph.node(i).name().empty())
    {
      visInfo[i].nodeName = onnxGraph.node(i).name();
    }
    else
    {
      visInfo[i].nodeName = onnxGraph.node(i).output(0);
    }
    visInfo[i].outputAdjNodes = outputAdjacencyList[i];
    visInfo[i].inputAdjNodes = inputAdjacencyList[i];
    visInfo[i].opType = onnxGraph.node(i).op_type();
    if(!gDiags.gDiagList.empty())
    {
      visInfo[i].diagnosticInfo = gDiags.gDiagList[index].getString();
    }
  }

  std::ofstream outfile;
  std::string graphvizFileName = data_->osrt_options.m_artifacts_folder + "/tempDir/graphvizInfo.txt";
  outfile.open(graphvizFileName);
  if(outfile.is_open())
  {
    outfile << std::to_string(onnxGraph.node_size()) <<endl;
    for(int i = 0; i < onnxGraph.node_size(); i++)
    {
      outfile << std::to_string(visInfo[i].nodeModelIdx) << " " << visInfo[i].nodeName << " " << visInfo[i].opType << " ";
      int outputsAdjNodesSize = 0;
      int add_output_cnt = 0;
      std::vector<std::string> add_output_name;
      for(auto& adjNode : visInfo[i].outputAdjNodes)
      {
        //Mark -1 added to identify the case in wich an nodes' output is also an input to other nodes and missed as an overall output
        if(adjNode == -1)
        {
          //Total outputAdjNodes size could include mark "-1" and could confuse graphviz. Reduced by one to keep current graphviz code flow
          outputsAdjNodesSize = visInfo[i].outputAdjNodes.size() -1;
        }
        else
        {
          outputsAdjNodesSize = visInfo[i].outputAdjNodes.size();
        }
      }
      if(visInfo[i].outputAdjNodes.size() == 0)
      {
        outfile << "outputAdjNodes " << visInfo[i].outputAdjNodes.size() << " ";
        std::string output_name = onnxGraph.node(i).output(0);
        outfile << output_name << " " ;
      }
      else
      {
        outfile << "outputAdjNodes " << outputsAdjNodesSize << " ";
      }
      //Run twice
      for(auto& adjNode : visInfo[i].outputAdjNodes)
      {
        if(adjNode != -1)
        {
          outfile << adjNode << " ";
        }
        else
        {
          //If a node's output is the input to other nodes and also an overall output, it should be only "1".
          add_output_name.push_back(onnxGraph.node(i).output(0));
          add_output_cnt++;
        }
      }
      outfile << "inputAdjNodes " << visInfo[i].inputAdjNodes.size() << " ";
      if(visInfo[i].inputAdjNodes.size() == 0)
      {
        std::string input_name = "";
        if (onnxGraph.node(i).input_size() > 0)
        {
          input_name = onnxGraph.node(i).input(0);
        }
        outfile << input_name << " " ;
      }
      for(auto& adjNode : visInfo[i].inputAdjNodes)
      {
        outfile << adjNode << " ";
      }
      size_t pos=0;
      std::string x="\n";
      std::string y=" ";
      while ((pos=visInfo[i].diagnosticInfo.find(x))!=std::string::npos)
      {
        visInfo[i].diagnosticInfo.replace(pos,x.length(),y);
      }
      outfile << "additionalOutput " << add_output_cnt << " ";
      if(add_output_name.size() != 0)
      {
        for(int i; i<add_output_name.size(); i++)
        {
          outfile << add_output_name[i] << " ";
        }
      }
      outfile << "diagInfo " << visInfo[i].diagnosticInfo;
      outfile << endl;
    }
    outfile.close();
  }
  else
  {
    TIDL_GLOBAL_REPORT_WARNING("Cannot open %s - graph visualisation will not work", graphvizFileName.c_str());
  }
  delete [] visInfo;
}

/** Check if input is constant/initializer */
int32_t TIDL_isInputConstInGraph(GraphProto& onnGraph, const string name)
{
  int i;
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


int32_t TIDL_isInputConst(std::string * string_buf, const string name)
{
  ModelProto model_proto;
  model_proto.ParseFromString(*string_buf);
  auto onnxGraph = model_proto.graph();
  return (TIDL_isInputConstInGraph(onnxGraph, name));
}

/** Assignes a unique name to the subgraph based on the subgraph id
 *  Concatenates subgraphname with the subgraph id (serial number)
 */
int32_t TIDL_updateSubGraphName (GraphProto& onnxGraph, char* stateSubGraphName, char* subgraphName, const char* serialNumber)
{
  /* Check for any user provided network name*/
  if (strcmp(data_->osrt_options.m_network_name.c_str(), "") == 0)
  {
    /* Check if model has any network name*/
    if (strcmp(onnxGraph.name().c_str(),"") == 0)
    {
      /* Use the default subgraph name*/
      strcpy(subgraphName, "subgraph_");
      strcat((char*)subgraphName, (char*)serialNumber);
    }
    else
    {
      strcpy(subgraphName, onnxGraph.name().c_str());
      strcat(subgraphName, "_");
      strcat(subgraphName, serialNumber);
    }
  }
  else
  {
    strcpy(subgraphName, data_->osrt_options.m_network_name.c_str());
    strcat(subgraphName, "_");
    strcat((char*)subgraphName, (char*)serialNumber);
  }

  strcpy ((char*)stateSubGraphName, (char*)subgraphName);
  return 0;
}

/** Find the names of ONNX graph outputs and concatenate them as part of outList */
int32_t TIDL_findOnnxRtOutputNames(GraphProto&   onnxGraph, char * outList)
{
  int i, j, k, l;
  char tensorName[TIDL_STRING_SIZE];
  char inTensorName[TIDL_STRING_SIZE];
  int outPutSize = 0;
  int node_idx = 0;

  for (i = 0; i < onnxGraph.node_size(); i++)
  {
    outPutSize = onnxGraph.node(i).output_size();
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
        node_idx = i;
        strcat(outList, tensorName);
      }
    }
  }
  return (node_idx);
}

/** Print properties of ONNX graph nodes */
int32_t TIDL_printOnnxGraphProperties(GraphProto&   onnxGraph)
{
  int32_t i;
  for (i = 0; i < onnxGraph.node_size(); i++)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%3d, %15s, %d, %d, %s, %s", i,
    onnxGraph.node(i).op_type().c_str(),
    onnxGraph.node(i).input_size(), onnxGraph.node(i).output_size(),
    onnxGraph.node(i).input(0).c_str(), onnxGraph.node(i).output(0).c_str());
  }
  return 0;
}

char* TIDL_replaceChar(char* string, char c1, char c2, int length)
{
  for (int32_t i = 0; i < length; i++)
  {
    if (string[i] == c1)
        string[i] = c2;
  }
  return string;
}

std::vector<int> sortOnnxGraphInTopologicalOrder(GraphProto& onnxGraph)
{
  std::vector<int> sortedIndices;
  std::queue<int> queuedNodes;
  std::queue<int> resetNodeIndices;
  std::vector<int> inDeg(onnxGraph.node_size(), 0);

  for (int i = 0; i < onnxGraph.node_size(); i++)
  {
    std::vector<std::string> uniqueInputNames = {};
    for (int j = 0; j < onnxGraph.node(i).input_size(); j++)
    {
      std::string inputName = onnxGraph.node(i).input(j);

      // Handle case in case same input is present
      if((strcmp(inputName.c_str(), "") == 0) ||
         std::find(uniqueInputNames.begin(),uniqueInputNames.end(),inputName) != uniqueInputNames.end())
      {
        continue;
      }
      uniqueInputNames.push_back(inputName);

      for(int k = 0; k < onnxGraph.node_size(); k++)
      {
        for(int l = 0; l < onnxGraph.node(k).output_size(); l++)
        {
          std::string outputName = onnxGraph.node(k).output(l);
          if(inputName == outputName)
          {
            inDeg[i]++;
            break;
          }
        }
      }
    }
  }

  for (int i = 0; i < onnxGraph.node_size(); i++)
  {
    if (inDeg[i] == 0)
    {
      if(queuedNodes.empty())
      {
        queuedNodes.push(i);
      }
      else
      {
        resetNodeIndices.push(i);
      }
    }
  }

  while(!queuedNodes.empty())
  {
    int i = queuedNodes.front();
    sortedIndices.push_back(i);
    queuedNodes.pop();

    // Get all consumers
    for (int j = 0; j < onnxGraph.node(i).output_size(); j++)
    {
      std::string outputName = onnxGraph.node(i).output(j);
      for(int k = 0; k < onnxGraph.node_size(); k++)
      {
        std::vector<std::string> uniqueInputNames = {};
        for(int l = 0; l < onnxGraph.node(k).input_size(); l++)
        {
          std::string inputName = onnxGraph.node(k).input(l);

          // Handle case in case same input is present
          if((strcmp(inputName.c_str(), "") == 0) ||
              std::find(uniqueInputNames.begin(),uniqueInputNames.end(),inputName) != uniqueInputNames.end())
          {
            continue;
          }
          uniqueInputNames.push_back(inputName);

          if(outputName == inputName)
          {
            inDeg[k]--;
            if(inDeg[k] == 0)
            {
              queuedNodes.push(k);
            }
            break;
          }
        }
      }
    }

    if(queuedNodes.empty() && !resetNodeIndices.empty() && sortedIndices.size() < onnxGraph.node_size())
    {
      i = resetNodeIndices.front();
      queuedNodes.push(i);
      resetNodeIndices.pop();
    }

  }

  return sortedIndices;
}

extern "C"
{

/* Called directly from ONNX runtime : run allowlisting API and return a vector of TIDL supported node groups */
int32_t TIDL_getSupportedNodesImport(std::string& data, std::vector<std::vector<int>> &nodeGroups)
{
  int32_t status = 0;
  ModelProto model_proto;
  model_proto.ParseFromString(data);

  // Read graph context embedded in model metadata by the OnnxRuntime EP.
  std::string ortVersion;
  int32_t opSetVersion = 0;
  for (int i = 0; i < model_proto.metadata_props_size(); i++)
  {
    const auto& prop = model_proto.metadata_props(i);
    if (prop.key() == "is_subgraph" && prop.value() == "1")
    {
      TIDL_GLOBAL_REPORT_WARNING("Graph is an ONNX subgraph (Loop/If/Scan body) -- TIDL does not support subgraph offload, delegating all nodes to CPU.");
      nodeGroups = {{}};
      return status;
    }
    else if (prop.key() == "ortVersion")
    {
      ortVersion = prop.value();
    }
    else if (prop.key() == "opsetVersion")
    {
      std::stringstream ss(prop.value());
      ss >> opSetVersion;
    }
  }

  auto onnxGraph = model_proto.graph();

  GraphCache graph_cache = buildGraphCache(onnxGraph);

  std::vector<int> sortedNodeIndices = sortOnnxGraphInTopologicalOrder(onnxGraph);
  if (sortedNodeIndices.size() != onnxGraph.node_size())
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Could not open sort onnx graph in topological order.")
  }

  if ((data_->osrt_options.m_log_file_name.c_str() != NULL) && (data_->osrt_options.m_log_file_name.c_str()[0] != '\0'))
  {
    gDiags.setLogFile(data_->osrt_options.m_log_file_name.c_str());
  }
  gDiags.gDiagList.clear();
  std::vector<std::vector<int>> outputAdjacencyList = {{}};
  std::vector<std::vector<int>> inputAdjacencyList = {{}};
  outputAdjacencyList = TIDL_createOutputAdjacencyList(onnxGraph);
  inputAdjacencyList = TIDL_createInputAdjacencyList(onnxGraph);


  bool isObjectDetectionNetwork = false;

  TIDL_printCompilationStages();
  TIDL_printVersionTable(TIDL_TOOLS_VERSION, data_->osrt_options.m_c7x_firmware_version, ortVersion, std::to_string(opSetVersion));

  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Parsing Started]");

  for (int i = 0; i < onnxGraph.node_size(); i++)
  {
    /** This condition assumes OD network has either "NonMaxSuppression" layer or "TopK" layer
     * If network doesn't have these layers, network will still be identified as OD network and will function fine if meta arch file is provided
     * else it will result in error(this is the case when post processing has to be run on ARM) - this particular case is covered in below Warning message  */
    if((strcmp(onnxGraph.node(i).op_type().c_str(), "NonMaxSuppression") == 0) || (strcmp(onnxGraph.node(i).op_type().c_str(), "TopK") == 0)
      || (data_->osrt_options.m_meta_arch_type != -1)||(strcmp(data_->osrt_options.m_model_type.c_str(),"OD")==0))
    {
      isObjectDetectionNetwork = true;
      break;
    }
  }
  if(! isObjectDetectionNetwork)
  {
    TIDL_GLOBAL_REPORT_WARNING("Network not identified as Object Detection network : (1) Ignore if network is not Object Detection network (2) If network is Object Detection network, please specify \"model_type\":\"OD\" as part of OSRT compilation options");
  }

  std::vector<int> odBackboneNodeIds = {};
  data_->odBackboneNodeNames = {};
  if(isObjectDetectionNetwork)
  {
    bool odCheckStatus = TIDL_OdOptionChecks(&data_->osrt_options, {TIDL_metaArchTIDLSSD, TIDL_metaArchTIDLYolo, TIDL_metaArchTIDLRetinaNet, TIDL_metaArchTIDLYoloV5, TIDL_metaArchTIDLYoloV8, TIDL_metaArchTIDL3DOD,TIDL_metaArchTIDLCenterPoint, TIDL_metaArchTIDLBEVFormer});  //These checks make sense only if network is OD
    if(! odCheckStatus)
    {
      delete data_;
      TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("");
    }
    data_->odPostProcHeadNames = TIDL_readMetaArchInfo(data_->osrt_options.m_meta_layers_names_list, onnxGraph.output_size(),data_->osrt_options.m_meta_arch_type);

    std::vector<int> postProcInputIds = TIDL_getPostProcInputIds(onnxGraph, data_->odPostProcHeadNames);
    
    /*
     * Find the stray output that does not belong on any on the od heads 
     */
    std::vector<int> strayOutputIds = {};
    for(int i = 0; i < onnxGraph.output_size(); i++)
    {
      for(int j = 0; j < onnxGraph.node_size(); j++)
      {
        if((strcmp(onnxGraph.output(i).name().c_str(), onnxGraph.node(j).output(0).c_str())) == 0)
        {
          std::vector<int32_t> inputs = TIDL_callNodeTraversal(inputAdjacencyList, {j}, onnxGraph.node_size());
          bool headFound = 0;
          for (int k = 0; k < postProcInputIds.size(); k++)
          {
            if (std::find(inputs.begin(),inputs.end(),postProcInputIds[k]) != inputs.end())
            {
              headFound = 1;
              break;
            }
          }
          if(!headFound)
          {
            strayOutputIds.push_back(j);
          }
        }
      }
    }

    for(int i = 0; i < strayOutputIds.size(); i++)
    {
      postProcInputIds.push_back(strayOutputIds[i]);
      data_->odPostProcHeadNames.push_back(onnxGraph.node(strayOutputIds[i]).output(0));
    }

    // Run DFS on the graph with OD "heads" as root and traversal towards the network input to get nodes of the backbone network
    odBackboneNodeIds =  TIDL_callNodeTraversal(inputAdjacencyList, postProcInputIds, onnxGraph.node_size());

    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Number of OD backbone nodes = %d ", odBackboneNodeIds.size());
    for(int i = 0; i < odBackboneNodeIds.size(); i++)
    {
      data_->odBackboneNodeNames.push_back(onnxGraph.node(odBackboneNodeIds[i]).name()); //convert node indices to corresponding names since indices change after graph partition
    }
  }
  else
  {
    data_->odBackboneNodeNames = {};
    data_->odPostProcHeadNames = {};
    data_->osrt_options.m_meta_arch_type = -1;
    data_->osrt_options.m_meta_layers_names_list = "";
  }


  std::vector<std::vector<int>> suportedNodeGroups;
  std::vector<int> nodeGroup;

  FILE *fp;
  char fileName[500];

  sprintf((char *)fileName, "%s/allowedNode.txt", data_->osrt_options.m_artifacts_folder.c_str());

  fp = fopen(fileName, "w+");
  if(fp == NULL)
  {
    TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Could not open %s for writing, exiting", fileName);
  }

  std::vector<std::vector<int>> supportedFusedNodes = TIDL_onnxAllowlistFusedLayers(onnxGraph);
  std::vector<int> addedFusedNodes = {};
  std::vector<std::string> supportedFusedNodesNames = {};
  std::map<std::string,int64_t> nodeCostMap;
  
  bool isSupported = false;
  int64_t cost = -1;
  int32_t i, j, num_subGraphs = 0;
  bool skip;

  for (int index = 0; index < sortedNodeIndices.size(); index++)
  {
    i = sortedNodeIndices[index];
    skip = 0;

    // Skip if already added in case of fused nodes
    for (int j = 0; j < addedFusedNodes.size(); j++)
    {
      if (i == addedFusedNodes[j])
      {
        TIDL_LOG_INFO(gDiags.gDiagList, "Layer %s, Op type %s -- supported via fusion, individually might not be supported",
                      onnxGraph.node(i).name().c_str(), onnxGraph.node(i).op_type().c_str());
        if (data_->osrt_options.osrtDebugPrintLevel)
        {
          gDiags.reportLastModeDiag();
        }
        skip = 1;
        break;
      }
    }

    if(skip)
    {
      continue;
    }

    cost = -1;
    isSupported = false;
    for(int j = 0; j < supportedFusedNodes.size(); j++)
    {
      if (std::find(supportedFusedNodes[j].begin(), supportedFusedNodes[j].end(), i) != supportedFusedNodes[j].end())
      {
        for(int k = 0; k < supportedFusedNodes[j].size(); k++)
        {
          int32_t idx = supportedFusedNodes[j][k];
          nodeGroup.push_back(idx);
          addedFusedNodes.push_back(idx);
          nodeCostMap[onnxGraph.node(idx).name()] = 0;
          supportedFusedNodesNames.push_back(onnxGraph.node(idx).name());
        }
        isSupported = true;
        TIDL_LOG_INFO(gDiags.gDiagList, "Layer %s, Op type %s -- supported via fusion, individually might not be supported",
                      onnxGraph.node(i).name().c_str(), onnxGraph.node(i).op_type().c_str());
        if (data_->osrt_options.osrtDebugPrintLevel)
        {
          gDiags.reportLastModeDiag();
        }
        nodeCostMap[onnxGraph.node(i).name()] = 1;
        break;
      }
    }

    if(!isSupported)
    {
      isSupported = TIDL_onnxAllowlistNode(onnxGraph, i, &data_->osrt_options, opSetVersion, isObjectDetectionNetwork, odBackboneNodeIds, &cost);
      if (isSupported)
      {
        nodeGroup.push_back(i);
        nodeCostMap[onnxGraph.node(i).name()] = cost;
      }
    }

    // Denied
    if(!isSupported)
    {
      if(!nodeGroup.empty())
      {
        if(!TIDL_checkIsSubgraphNoOps(onnxGraph, nodeGroup))
        {
          // Remove NoOp layers in the node group which do not have both inputs and outputsin the current node group
          std::vector<int> isolatedNoopNodeIndices=getIsolatedNoOpNodes(onnxGraph,nodeGroup,inputAdjacencyList,outputAdjacencyList);
          for (int j = 0; j < isolatedNoopNodeIndices.size(); j++)
          {
            if (!gDiags.gDiagList.empty())
            {
              int index = std::find(sortedNodeIndices.begin(),sortedNodeIndices.end(),isolatedNoopNodeIndices[j]) - sortedNodeIndices.begin();
              TIDL_LOG_UNSUPPORTED_AT(gDiags.gDiagList,index, "Subgraph have isolated non compute node --- layer type - %s,  Node name - %s", onnxGraph.node(isolatedNoopNodeIndices[j]).op_type().c_str(), onnxGraph.node(isolatedNoopNodeIndices[j]).name().c_str());
            }
            auto it = std::find(nodeGroup.begin(),nodeGroup.end(),isolatedNoopNodeIndices[j]);
            if (it != nodeGroup.end())
            {
              nodeGroup.erase(it);
            }
          }
          
          suportedNodeGroups.push_back(nodeGroup);
          num_subGraphs++;
        }
        else
        {
          if (!gDiags.gDiagList.empty())
          {
            auto last = gDiags.gDiagList[gDiags.gDiagList.size() - 1];
            gDiags.gDiagList.pop_back();
            for (j = 0; j < nodeGroup.size(); j++)
            {
              gDiags.gDiagList.pop_back();
            }
            for (j = 0; j < nodeGroup.size(); j++)
            {
              TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Subgraph does not have any compute node --- layer type - %s,  Node name - %s", onnxGraph.node(nodeGroup[j]).op_type().c_str(), onnxGraph.node(nodeGroup[j]).name().c_str());
              if (data_->osrt_options.osrtDebugPrintLevel > 0)
              {
                gDiags.reportLastModeDiag();
              }
            }
            gDiags.gDiagList.emplace_back(last);
          }
        }
        nodeGroup.clear();
      }
    }
  }

  if(!nodeGroup.empty())
  {
    if(!TIDL_checkIsSubgraphNoOps(onnxGraph, nodeGroup))
    {

      std::vector<int> isolatedNoopNodeIndices=getIsolatedNoOpNodes(onnxGraph,nodeGroup,inputAdjacencyList,outputAdjacencyList);
      for (int j = 0; j < isolatedNoopNodeIndices.size(); j++)
      { 
        if (!gDiags.gDiagList.empty())
        {
          int index = std::find(sortedNodeIndices.begin(),sortedNodeIndices.end(),isolatedNoopNodeIndices[j]) - sortedNodeIndices.begin();
          TIDL_LOG_UNSUPPORTED_AT(gDiags.gDiagList,index, "Subgraph have isolated non compute node --- layer type - %s,  Node name - %s", onnxGraph.node(isolatedNoopNodeIndices[j]).op_type().c_str(), onnxGraph.node(isolatedNoopNodeIndices[j]).name().c_str());
        }
        auto it = std::find(nodeGroup.begin(),nodeGroup.end(),isolatedNoopNodeIndices[j]);
        if (it != nodeGroup.end())
        {
          nodeGroup.erase(it);
        }
      }
      suportedNodeGroups.push_back(nodeGroup);
      num_subGraphs++;
    }
    else
    {
      if (!gDiags.gDiagList.empty())
      {
        for (j = 0; j < nodeGroup.size(); j++)
        {
          gDiags.gDiagList.pop_back();
        }
        for (j = 0; j < nodeGroup.size(); j++)
        {
          TIDL_LOG_UNSUPPORTED(gDiags.gDiagList, "Subgraph does not have any compute node --- layer type - %s,  Node name - %s", onnxGraph.node(nodeGroup[j]).op_type().c_str(), onnxGraph.node(nodeGroup[j]).name().c_str());
          if (data_->osrt_options.osrtDebugPrintLevel > 0)
          {
            gDiags.reportLastModeDiag();
          }
        }
      }
    }
    nodeGroup.clear();
  }

  /* Optimize the subgraph partitions */
  std::vector<std::vector<int>> suportedNodeGroupsOptimized = optimizeGraphPartition(onnxGraph, suportedNodeGroups);

  /* Evict nodes with scalar (0-dim) outputs that cross a subgraph boundary
   * since TIDL cannot handle 0-dim output tensors */
  TIDL_removeScalarBoundaryNodes(onnxGraph, suportedNodeGroupsOptimized, sortedNodeIndices);

  /* Sub-divide the subgraphs if needed*/
  int numMaxNodes;
  if (data_->osrt_options.m_num_tidl_subgraph_max_node <= 0)
  {
    if(data_->osrt_options.m_prequantized_model == 0)
    {
      numMaxNodes = TIDL_NUM_MAX_SUBGRAPH_NODES;
    }
    else
    {
      numMaxNodes = TIDL_NUM_MAX_SUBGRAPH_NODES_QDQ;
    }
    // Can't accept negative, raise a warning
    if (data_->osrt_options.m_num_tidl_subgraph_max_node < 0)
    {
      TIDL_GLOBAL_REPORT_WARNING("max_num_subgraph_nodes(%d) cannot be <= 0, defaulting to %d", data_->osrt_options.m_num_tidl_subgraph_max_node, numMaxNodes);
    }
  }
  else
  {
    numMaxNodes = data_->osrt_options.m_num_tidl_subgraph_max_node;
  }

  std::vector<std::vector<int>> suportedNodeGroupsSubdivided1 = divideGraphPartition(onnxGraph, suportedNodeGroupsOptimized, numMaxNodes, supportedFusedNodesNames);
  std::vector<std::vector<int>> suportedNodeGroupsSubdivided = divideGraphPartitionIO(onnxGraph,graph_cache,suportedNodeGroupsSubdivided1,supportedFusedNodesNames);
  if(suportedNodeGroupsSubdivided.size() > data_->osrt_options.m_num_tidl_subgraphs)
  {
    TIDL_GLOBAL_REPORT_WARNING("Number of subgraphs generated (%d) > max_num_subgraphs(%d) provided in options. Best %d subgraphs will be created. Additional subgraphs will be delegated to ARM", suportedNodeGroupsSubdivided.size(), data_->osrt_options.m_num_tidl_subgraphs, data_->osrt_options.m_num_tidl_subgraphs);

    std::vector<int64_t> subgraphCost;
    std::vector<int64_t> subgraphCostMax;
    std::vector<std::vector<int>> bestSubgraphs;

    // Calculate cost of each subgrapgh
    for (int i = 0; i < suportedNodeGroupsSubdivided.size(); i++)
    {
      int64_t cost = calculateSubgraphCost(onnxGraph, suportedNodeGroupsSubdivided[i], nodeCostMap);
      subgraphCost.push_back(cost);
      subgraphCostMax.push_back(cost);
    }

    // Sort the cost in descending order to get max m_num_tidl_subgraphs costs
    std::sort(subgraphCostMax.begin(), subgraphCostMax.end());
    std::reverse(subgraphCostMax.begin(), subgraphCostMax.end());
    while(subgraphCostMax.size() > data_->osrt_options.m_num_tidl_subgraphs)
    {
      subgraphCostMax.pop_back();
    }

    // Use subgraph if its cost is in maxCost
    for (int i = 0; i < suportedNodeGroupsSubdivided.size(); i++)
    {
      if(std::find(subgraphCostMax.begin(), subgraphCostMax.end(), subgraphCost[i]) != subgraphCostMax.end())
      {
        bestSubgraphs.push_back(suportedNodeGroupsSubdivided[i]);
      }
    }
    while(bestSubgraphs.size() > data_->osrt_options.m_num_tidl_subgraphs)
    {
      bestSubgraphs.pop_back();
    }

    suportedNodeGroupsSubdivided = bestSubgraphs;
  }

  /* Total nodes supported by C7x */
  int32_t numSuportedNodes = 0;
  fprintf(fp, "%d\n", suportedNodeGroupsSubdivided.size());
  for(int i = 0; i < suportedNodeGroupsSubdivided.size(); i++)
  {
    std::vector<int> subgraph = suportedNodeGroupsSubdivided[i];
    fprintf(fp, "%d\n", subgraph.size());
    for(int j = 0; j < subgraph.size(); j++)
    {
      fprintf(fp, "%d\n", subgraph[j]);
      numSuportedNodes++;
    }
  }

  fclose(fp);

  /* Total subgraphs offloaded to C7x*/
  int32_t numOffloadSubGraphCreated = suportedNodeGroupsSubdivided.size();

  data_->m_num_subgraphs_created = numOffloadSubGraphCreated;

  std::fstream onnxrtMetaData(data_->osrt_options.m_artifacts_folder + "/onnxrtMetaData.txt", std::ios::app);
  onnxrtMetaData << "numGraphNodes=" << std::to_string(onnxGraph.node_size()) << endl;

  //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nTotal Nodes = %d", onnxGraph.node_size());
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
  TIDL_GLOBAL_REPORT_SUBHEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_NoColor, "Subgraph Information Summary", onnxGraph.node_size());

  TIDL_generateParseTable(onnxGraph, sortedNodeIndices, numSuportedNodes, numOffloadSubGraphCreated);

  TIDL_getGraphVisualizationInfo(onnxGraph, sortedNodeIndices, outputAdjacencyList, inputAdjacencyList);

  TIDL_runGraphvizToolRuntimes(data_->osrt_options.m_tidl_tools_path, data_->osrt_options.m_artifacts_folder, data_->osrt_options.m_debug_level);

  TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Parsing Completed]");

  if(suportedNodeGroupsSubdivided.empty())
  {
    nodeGroups = {{}};
  }
  else
  {
    nodeGroups = suportedNodeGroupsSubdivided;
  }

  return status;
}

} //extern C

/**
 * Reads inData of any type & stores it in float type for calibration
*/
template <class dataType>
int32_t TIDL_convertInDataToFloat (float* dstPtr, dataType *srcPtr, int32_t bufferSize)
{
  int32_t status = 0;

  for( int j = 0; j < bufferSize; j++)
  {
    dstPtr[j] = srcPtr[j];
  }

  return status;
}

/** Write inputs for each subgraph corresponding to all input frames in float mode in bin file
  * and also store corresponding quantFactors for quantization information */
int32_t TIDL_writeQuantizedInput(onnxRtParams_t * onnxRtParams, char * inputName,
                              int32_t isCurrFrameIdx1, int32_t numParamBits, int32_t inferenceMode, float ** inQuantFactorInput)
{
  /* Derive companion dims file path: replace _calib_raw_data.bin with _calib_input_dims.txt */
  char dimsFileName[FILE_NAME_SIZE];
  strncpy(dimsFileName, inputName, FILE_NAME_SIZE - 1);
  dimsFileName[FILE_NAME_SIZE - 1] = '\0';
  char * suffix = strstr(dimsFileName, "_calib_raw_data.bin");
  if (suffix != NULL)
  {
    strcpy(suffix, "_calib_input_dims.txt");
  }

  if(isCurrFrameIdx1) //remove file at the beginning if it exists, in order to avoid appending contents from previous run
  {
    remove(inputName);
    remove(dimsFileName);
  }
  FILE* fp = fopen(inputName, "ab+");

  int32_t w[TIDL_MAX_ALG_IN_BUFS];
  int32_t h[TIDL_MAX_ALG_IN_BUFS];
  int32_t c[TIDL_MAX_ALG_IN_BUFS];
  int32_t d2[TIDL_MAX_ALG_IN_BUFS];
  int32_t d1[TIDL_MAX_ALG_IN_BUFS];
  int32_t n[TIDL_MAX_ALG_IN_BUFS];

  float * inQuantFactor = *inQuantFactorInput;
  float * scratch_mem = NULL;

  if (fp == NULL)
  {
    TIDL_GLOBAL_REPORT_ERROR("Could not open file to save the input tensors - %s, aborting", inputName);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  for(int i = 0; i < onnxRtParams->numNetInData; i++)
  {
    //TODO: Need to put if based on tensor element type for quantized models
    if( (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT)  &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32) &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)  &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8)  &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8)  &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16) &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16)  &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)  &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64) &&
        (onnxRtParams->inputTensorElementType[i] != ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL))

    {
      TIDL_GLOBAL_REPORT_ERROR("Input ONNX tensor element type - %d. Only FLOAT, BOOL, UINT8, INT8, INT16, UINT32, INT32, UINT64, INT64 and UINT16 inputs supported for ONNX runtime", onnxRtParams->inputTensorElementType[i]);
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    float* input = (float *)onnxRtParams->inputTensorData[i];

    auto& tensor_shape = onnxRtParams->tensorShape[i];

    /* !Update this condition in the outReshape functions */
    if(inferenceMode == TIDL_inferenceModeHighThroughput)
    {
      tensor_shape[0] = 1;
    }
    
    w[i] = tensor_shape[TIDL_DIM_WIDTH];
    h[i] = tensor_shape[TIDL_DIM_HEIGHT];
    c[i] = tensor_shape[TIDL_DIM_NUMCH];
    d1[i] = tensor_shape[TIDL_DIM_DIM2];
    d2[i] = tensor_shape[TIDL_DIM_DIM1];
    n[i] = tensor_shape[TIDL_DIM_BATCH];

    size_t tensorSize = w[i] * h[i] * c[i] * n[i] * d1[i] * d2[i];

    if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8 ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64 ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32 ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64 ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32 ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16 ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8 ||
        onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16)
    {
      scratch_mem = (float*)malloc(sizeof(float)*tensorSize);
      if (scratch_mem == NULL)
      {
        TIDL_GLOBAL_REPORT_ERROR("Could not allocate memory of size %d", sizeof(float)*tensorSize);
        return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
      }

      if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((uint8_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
      else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((uint32_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
       else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((uint64_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
      else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((int64_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
      else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((int32_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
      else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((bool *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
      else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((int16_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
      else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((int8_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }
      else if (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16)
      {
        TIDL_convertInDataToFloat (scratch_mem, ((uint16_t *)onnxRtParams->inputTensorData[i]), tensorSize);
      }

      input = (float *)scratch_mem;
    }

    float min, max;
    TIDL_runtimesFindRange((float *)(input), tensor_shape[0], tensor_shape[1], tensor_shape[2], tensor_shape[3], tensor_shape[4], tensor_shape[5], 1.0, 0, min, max);

    fwrite(input, 1, tensorSize * (32 / 8), fp);

    inQuantFactor[i] = TIDL_findMaxQuantizationScale(min, max, numParamBits);

    /* For pointpillars, index input is passed as it is */
    if((onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32) ||
       (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32) ||
       (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64) ||
       (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8) ||
       (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64) ||
       (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16) ||
       (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8) ||
       (onnxRtParams->inputTensorElementType[i] == ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16))
    {
      inQuantFactor[i] = 1.0;
    }
    free(scratch_mem);
    scratch_mem=NULL;   // to avoid double free error
  }
  fclose(fp);

  /* Write one line of per-input dims to the companion dims file.
   * Format: b_0 d1_0 d2_0 c_0 h_0 w_0;  b_1 d1_1 d2_1 c_1 h_1 w_1;  ...
   * This is consumed by the inference via inDimsFile for dynamic input models.
   */
  if (suffix != NULL)
  {
    FILE* dimsFp = fopen(dimsFileName, "a");
    if (dimsFp != NULL)
    {
      for (int i = 0; i < onnxRtParams->numNetInData; i++)
      {
        if (i > 0)
        {
          fprintf(dimsFp, "; ");
        }

        for (int d = 0; d < TIDL_DIM_MAX; d++)
        {
          fprintf(dimsFp, "%d", onnxRtParams->tensorShape[i][d]);
          if (d < TIDL_DIM_MAX - 1)
          {
            fprintf(dimsFp, " ");
          }
        }
      }
      fprintf(dimsFp, "\n");
      fclose(dimsFp);
    }
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** First save the inputs to the subgraph corresponding to "m_calibration_frames" number of frames.
  * For the last frame, given all the inputs are now available, run postProcessNet, which runs calibration on the subgraph
  * After the final calibrated and quantized subgraph is saved in net.bin and io.bin files, copy these files from
  * tempDir to the artifacts folder for inference
  */
int32_t TIDL_subgraphImport(onnxRtParams_t * onnxRtParams, TIDL_OnnxrtEPData* options,
                          void * subGraphPtr, char* subGraphName, int32_t currFrameIdx)
{
  if(currFrameIdx <= options->osrt_options.m_calibration_frames) //need to copy input of subgraphs only before calibration is done
  {
    std::string inputName = options->osrt_options.m_temp_folder + "/" + subGraphName + "_calib_raw_data.bin";

    int32_t isCurrFrameIdx1 = (currFrameIdx == 1) ? 1 : 0;
    int32_t numParamBits = options->osrt_options.m_num_param_bits;
    int32_t inferenceMode = options->osrt_options.m_inference_mode;

    float * inQuantFactorCurrTensor = (float *)malloc(TIDL_MAX_ALG_IN_BUFS * sizeof(float));
    if (inQuantFactorCurrTensor == NULL)
    {
      TIDL_GLOBAL_REPORT_ERROR("Could not allocate memory of size %d", sizeof(float)*TIDL_MAX_ALG_IN_BUFS);
      return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
    }
    memset(inQuantFactorCurrTensor, 0, TIDL_MAX_ALG_IN_BUFS * sizeof(float));
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_writeQuantizedInput(onnxRtParams, const_cast<char *>(inputName.c_str()), isCurrFrameIdx1, numParamBits, inferenceMode, &inQuantFactorCurrTensor), "");

    if((currFrameIdx == options->osrt_options.m_calibration_frames))
    {
      if(numParamBits == 32)
      {
        //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n ************ Frame index %d : Running float inference **************** \n", currFrameIdx);
      }
      else
      {
        //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n**********  Frame Index %d : Running fixed point mode for calibration **********\n", currFrameIdx);
        TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runtimesPostProcessNet(&options->osrt_options, options->osrt_options.m_calibration_frames, options->osrt_options.m_num_param_bits, subGraphPtr, NULL,
                                      inQuantFactorCurrTensor, NULL, subGraphName), "");
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n");
      }
    }

    if(currFrameIdx == options->osrt_options.m_calibration_frames)
    {
      std::string subGraphId;
      std::stringstream(subGraphName) >> subGraphId;
      TIDL_copyFile(subGraphId + "_tidl_net.bin", options->osrt_options.m_artifacts_folder, options->osrt_options.m_temp_folder);
      TIDL_copyFile(subGraphId + "_tidl_io_1.bin", options->osrt_options.m_artifacts_folder, options->osrt_options.m_temp_folder);
    }
    free(inQuantFactorCurrTensor);
    inQuantFactorCurrTensor = NULL;
  }
  else
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n**********  Frame Index %d Running inference - currFrameIdx > numFramesCalibration **********\n", currFrameIdx);
    //No need to run postProcessNet, run inference directly on the saved graph
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** Write metadata to be read during inference in order to prevent protobuf read */
void TIDL_writeOnnxRtMetaDataForInference(GraphProto& onnxGraph, OnnxTIDLSubGraphParams * state_subGraph, std::string artifactsFolder)
{
  std::string serialNumber = std::to_string(state_subGraph->serialNumber);

  std::fstream onnxrtMetaData(artifactsFolder + "/onnxrtMetaData.txt", std::ios::app);
  onnxrtMetaData << serialNumber + ":subGraphName=" <<  state_subGraph->subGraphName_ << endl;
  onnxrtMetaData << serialNumber + ":numInputs=" << state_subGraph->numInputs << endl;
  onnxrtMetaData << serialNumber + ":numOutputs=" << state_subGraph->numOutputs << endl;
  onnxrtMetaData << serialNumber + ":inputIdx=";
  for(int i = 0; i < state_subGraph->numInputs; i++)
  {
    if(i > 0) onnxrtMetaData << ",";
    onnxrtMetaData << state_subGraph->inputIdx[i];
  }
  onnxrtMetaData << endl;

  onnxrtMetaData << serialNumber + ":inDataNames=";
  for(int i = 0; i < state_subGraph->numInputs; i++)
  {
    if(i > 0) onnxrtMetaData << ",";
    onnxrtMetaData << onnxGraph.input(state_subGraph->inputIdx[i]).name().c_str();
  }
  onnxrtMetaData << endl;

  onnxrtMetaData << serialNumber + ":outDataNames=";
  for(int i = 0; i < state_subGraph->numOutputs; i++)
  {
    if(i > 0) onnxrtMetaData << ",";
    onnxrtMetaData << onnxGraph.output(i).name().c_str();
  }
  onnxrtMetaData << endl;

  onnxrtMetaData << serialNumber + ":outDataTypes=";
  for(int i = 0; i < state_subGraph->numOutputs; i++)
  {
    if(i > 0) onnxrtMetaData << ",";
    onnxrtMetaData << onnxGraph.output(i).type().tensor_type().elem_type();
  }
  onnxrtMetaData << endl;
}

std::vector<std::string> TIDL_splitStringWithDelimiter(std::string inpStr, std::string del)
{
  int32_t start = 0;
  int32_t end = inpStr.find(del);
  std::vector<std::string> splitStr;
  while(end != -1)
  {
    splitStr.push_back(inpStr.substr(start, end - start));
    start = end + del.size();
    end = inpStr.find(del, start);
  }
  splitStr.push_back(inpStr.substr(start, end - start));

  return splitStr;
}

/*
* Merge net and io files for multi-subgraph support in TIDL-RT.
* Also modify allowedNode.txt and onnxrtMetaData.txt to represent single subgraph.
*/
int32_t TIDL_combileArtifactsForTIDLRTMultiSubgraph(std::string artifactsFolder, int32_t numGraphNodes)
{

  int32_t status = -1;
  std::vector<std::string> subgraphNames;

  std::string onnxrtMetaDataPath = artifactsFolder + "/onnxrtMetaData.txt";
  std::ifstream onnxrtMetaData(onnxrtMetaDataPath);
  if (!onnxrtMetaData.is_open())
  {
    return status;
  }
  std::string line;
  std::vector<std::string> dict;
  while(std::getline(onnxrtMetaData, line))
  {
    dict = TIDL_splitStringWithDelimiter(line, "=");
    if(dict[0].find(":subGraphName") != std::string::npos)
    {
      subgraphNames.push_back(dict[1]);
    }
  }

  onnxrtMetaData.clear();
  onnxrtMetaData.seekg (0, ios::beg);

  int32_t numSubgraphs = subgraphNames.size();
  std::vector<std::vector<int32_t>> inputIdx(numSubgraphs);
  std::vector<std::vector<std::string>> inDataNames(numSubgraphs);
  std::vector<std::vector<std::string>> outDataNames(numSubgraphs);

  for (int i = 0; i < subgraphNames.size(); i++)
  {
    std::string line;
    std::vector<std::string> dict;
    while(std::getline(onnxrtMetaData, line))
    {
      dict = TIDL_splitStringWithDelimiter(line, "=");
      if(dict[0].find(":inputIdx") != std::string::npos)
      {
        std::vector<std::string> idxStr = TIDL_splitStringWithDelimiter(dict[0], ":");
        int32_t index;
        std::stringstream ss(idxStr[0]);
        ss >> index;
        int32_t val;
        std::vector<std::int32_t> data;
        std::vector<std::string> idx = TIDL_splitStringWithDelimiter(dict[1], ",");
        for(int j = 0; j < idx.size(); j++)
        {
          std::stringstream num(idx[j]);
          num >> val;
          data.push_back(val);
        }
        inputIdx[index] = data;
      }
      else if(dict[0].find(":inDataNames") != std::string::npos)
      {
        std::vector<std::string> idxStr = TIDL_splitStringWithDelimiter(dict[0], ":");
        int32_t index;
        std::stringstream ss(idxStr[0]);
        ss >> index;
        inDataNames[index] = TIDL_splitStringWithDelimiter(dict[1], ",");
      }
      else if(dict[0].find(":outDataNames") != std::string::npos)
      {
        std::vector<std::string> idxStr = TIDL_splitStringWithDelimiter(dict[0], ":");
        int32_t index;
        std::stringstream ss(idxStr[0]);
        ss >> index;
        outDataNames[index] = TIDL_splitStringWithDelimiter(dict[1], ",");
      }
    }
  }

  onnxrtMetaData.close();


  uint32_t HEADER = TIDL_NET_MULTI_SUBGRAPH_MAGIC_NUMBER;
  uint32_t netOffset = (2 + (4 * numSubgraphs)) * 4;
  uint32_t ioOffset = (2 + (4 * numSubgraphs)) * 4;
  uint32_t reservedBitVal = 0;

  /****************************************************************************/
  // Combile the network file
  std::string combinedNetPath = artifactsFolder + "/combined_subgraph_tidl_net.bin";
  std::ofstream combinedNet(combinedNetPath, std::ofstream::binary);
  if (!combinedNet.is_open())
  {
    return status;
  }

  for (int i = 0; i < numSubgraphs; i++)
  {
    uint32_t netFileSize = 0;
    std::string netFilePath = artifactsFolder + "/" + subgraphNames[i] +"_tidl_net.bin";
    std::ifstream netFile(netFilePath, std::ios::binary | std::ios::in);
    if (!netFile.is_open())
    {
      return status;
    }
    netFile.seekg(0,std::ios::end);
    netFileSize = netFile.tellg();
    netFile.seekg(0,std::ios::beg);

    if(i == 0)
    {
      // Write the header meta-data
      uint32_t netVersion;
      netFile.read((char *)&netVersion,sizeof(uint32_t));
      combinedNet.write((char*)&netVersion,sizeof(uint32_t));
      combinedNet.write((char*)&HEADER,sizeof(uint32_t));
      combinedNet.write((char*)&numSubgraphs,sizeof(uint32_t));
    }
    netFile.close();

    combinedNet.write((char*)&netOffset,sizeof(uint32_t));
    combinedNet.write((char*)&netFileSize,sizeof(uint32_t));
    combinedNet.write((char*)&reservedBitVal,sizeof(uint32_t)); // Reserved bits
    combinedNet.write((char*)&reservedBitVal,sizeof(uint32_t)); // Reserved bits
    netOffset = netFileSize;
  }
  // Copy contents to the combinet bin file
  for (int i = 0; i < numSubgraphs; i++)
  {
    std::string netFilePath = artifactsFolder + "/" + subgraphNames[i] +"_tidl_net.bin";
    std::ifstream netFile(netFilePath, std::ios::binary | std::ios::in);
    if (!netFile.is_open())
    {
      return status;
    }
    combinedNet<<netFile.rdbuf();
    netFile.close();
    std::remove(netFilePath.c_str());
  }
  combinedNet.close();
  std::string tempName = artifactsFolder + "/" + subgraphNames[0] + "_tidl_net.bin";
  std::rename(combinedNetPath.c_str(), tempName.c_str());
  /****************************************************************************/


  /****************************************************************************/
  // Create combined IOBufDesc
  std::vector<int32_t> combinedInputIdx;
  sTIDL_IOBufDesc_t combinedIoBufDesc;
  memset(&combinedIoBufDesc, 0, sizeof(sTIDL_IOBufDesc_t));
  int32_t inCount = 0;
  int32_t outCount = 0;
  std::vector<sTIDL_IOBufDesc_t> ioBufDescVec;
  ioBufDescVec.reserve(numSubgraphs);

  for (int i = 0; i < numSubgraphs; i++)
  {
    sTIDL_IOBufDesc_t ioBufDesc;
    std::string ioFilePath = artifactsFolder + "/" + subgraphNames[i] +"_tidl_io_1.bin";
    FILE *ioBufFile = fopen(ioFilePath.c_str(), "rb");
    if (ioBufFile == NULL)
    {
      return status;
    }
    fread(&ioBufDesc, sizeof(sTIDL_IOBufDesc_t), 1, ioBufFile);
    ioBufDescVec.push_back(ioBufDesc);
    fclose(ioBufFile);
  }

  // Get descriptors that are actual input and outputs i.e hanging IO
  for (int i = 0; i < numSubgraphs; i++)
  {
    sTIDL_IOBufDesc_t *ioBufDesc = &(ioBufDescVec[i]);

    combinedIoBufDesc.numCores = ioBufDesc->numCores;
    combinedIoBufDesc.numVirtualCores = ioBufDesc->numVirtualCores;
    combinedIoBufDesc.numSuperBatches = ioBufDesc->numSuperBatches;
    combinedIoBufDesc.inferenceMode = ioBufDesc->inferenceMode;

    for (int p = 0; p < ioBufDesc->numInputBuf; p++)
    {
      int32_t isIntermediateInput = 0;
      for (int j = 0; j < numSubgraphs; j++)
      {
        sTIDL_IOBufDesc_t *tIoBufDesc = &(ioBufDescVec[j]);
        for (int q = 0; q < tIoBufDesc->numOutputBuf; q++)
        {
          if (strcmp((char *)ioBufDesc->inDataName[p], (char *)tIoBufDesc->outDataName[q]) == 0
              ||
              (inDataNames[i][p] == outDataNames[j][q]))
          {
            isIntermediateInput = 1;
            break;
          }
        }
        if(isIntermediateInput)
        {
          break;
        }
      }

      if(isIntermediateInput == 0)
      {
        combinedIoBufDesc.numInputBuf++;
        combinedIoBufDesc.inWidth[inCount] = ioBufDesc->inWidth[p];
        combinedIoBufDesc.inHeight[inCount] = ioBufDesc->inHeight[p];
        combinedIoBufDesc.inNumChannels[inCount] = ioBufDesc->inNumChannels[p];
        combinedIoBufDesc.inDIM2[inCount] = ioBufDesc->inDIM2[p];
        combinedIoBufDesc.inDIM1[inCount] = ioBufDesc->inDIM1[p];
        combinedIoBufDesc.inNumBatches[inCount] =   ioBufDesc->inNumBatches[p];
        combinedIoBufDesc.inPadL[inCount] = ioBufDesc->inPadL[p];
        combinedIoBufDesc.inPadT[inCount] = ioBufDesc->inPadT[p];
        combinedIoBufDesc.inPadR[inCount] = ioBufDesc->inPadR[p];
        combinedIoBufDesc.inPadB[inCount] = ioBufDesc->inPadB[p];
        combinedIoBufDesc.inPadCh[inCount] = ioBufDesc->inPadCh[p];
        combinedIoBufDesc.inChannelPitch[inCount] = ioBufDesc->inChannelPitch[p];
        combinedIoBufDesc.inTensorScale[inCount] = ioBufDesc->inTensorScale[p];
        combinedIoBufDesc.inElementType[inCount] = ioBufDesc->inElementType[p];
        combinedIoBufDesc.rawDataInElementType[inCount] = ioBufDesc->rawDataInElementType[p];
        combinedIoBufDesc.inZeroPoint[inCount] = ioBufDesc->inZeroPoint[p];
        combinedIoBufDesc.inLayout[inCount] = ioBufDesc->inLayout[p];
        combinedIoBufDesc.inDataId[inCount] = ioBufDesc->inDataId[p];
        combinedIoBufDesc.inDataFormat[inCount] = ioBufDesc->inDataFormat[p];
        combinedIoBufDesc.inResizeType[inCount] = ioBufDesc->inResizeType[p];
        combinedIoBufDesc.resizeWidth[inCount] = ioBufDesc->resizeWidth[p];
        combinedIoBufDesc.resizeHeight[inCount] = ioBufDesc->resizeHeight[p];
        combinedIoBufDesc.inBufSize[inCount] = ioBufDesc->inBufSize[p];
        strcpy((char *)combinedIoBufDesc.inDataName[inCount], inDataNames[i][p].c_str());
        inCount++;

        combinedInputIdx.push_back(inputIdx[i][p]);
      }
    }
    for (int p = 0; p < ioBufDesc->numOutputBuf; p++)
    {
      int32_t isIntermediateOutput = 0;
      for (int j = 0; j < numSubgraphs; j++)
      {
        sTIDL_IOBufDesc_t *tIoBufDesc = &(ioBufDescVec[j]);
        for (int q = 0; q < tIoBufDesc->numInputBuf; q++)
        {
          if (strcmp((char *)ioBufDesc->outDataName[p], (char *)tIoBufDesc->inDataName[q]) == 0
              ||
              (outDataNames[i][p] == inDataNames[j][q]))
          {
            isIntermediateOutput = 1;
            break;
          }
        }
        if(isIntermediateOutput)
        {
          break;
        }
      }

      if(isIntermediateOutput == 0)
      {
        combinedIoBufDesc.numOutputBuf++;
        combinedIoBufDesc.outWidth[outCount] = ioBufDesc->outWidth[p];
        combinedIoBufDesc.outHeight[outCount] = ioBufDesc->outHeight[p];
        combinedIoBufDesc.outNumChannels[outCount] = ioBufDesc->outNumChannels[p];
        combinedIoBufDesc.outDIM2[outCount] = ioBufDesc->outDIM2[p];
        combinedIoBufDesc.outDIM1[outCount] = ioBufDesc->outDIM1[p];
        combinedIoBufDesc.outNumBatches[outCount] =   ioBufDesc->outNumBatches[p];
        combinedIoBufDesc.outPadL[outCount] = ioBufDesc->outPadL[p];
        combinedIoBufDesc.outPadT[outCount] = ioBufDesc->outPadT[p];
        combinedIoBufDesc.outPadR[outCount] = ioBufDesc->outPadR[p];
        combinedIoBufDesc.outPadB[outCount] = ioBufDesc->outPadB[p];
        combinedIoBufDesc.outPadCh[outCount] = ioBufDesc->outPadCh[p];
        combinedIoBufDesc.outChannelPitch[outCount] = ioBufDesc->outChannelPitch[p];
        combinedIoBufDesc.outTensorScale[outCount] = ioBufDesc->outTensorScale[p];
        combinedIoBufDesc.outElementType[outCount] = ioBufDesc->outElementType[p];
        combinedIoBufDesc.outZeroPoint[outCount] = ioBufDesc->outZeroPoint[p];
        combinedIoBufDesc.outLayout[outCount] = ioBufDesc->outLayout[p];
        combinedIoBufDesc.outDataId[outCount] = ioBufDesc->outDataId[p];
        combinedIoBufDesc.outBufSize[outCount] = ioBufDesc->outBufSize[p];
        combinedIoBufDesc.numValidTensorDims[outCount] = ioBufDesc->numValidTensorDims[p];
        strcpy((char *)combinedIoBufDesc.outDataName[outCount], outDataNames[i][p].c_str());
        outCount++;
      }
    }
  }
  /****************************************************************************/


  /****************************************************************************/
  // Combile the io file
  std::string combinedIOFilePath = artifactsFolder + "/combined_subgraph_tidl_io_1.bin";
  std::ofstream combinedIO(combinedIOFilePath, std::ofstream::binary);
  if (!combinedIO.is_open())
  {
    return status;
  }

  // Write combined iobuf at start of file
  combinedIO.write((char*)&combinedIoBufDesc,sizeof(sTIDL_IOBufDesc_t));

  // Write the header meta-data
  combinedIO.write((char*)&HEADER,sizeof(uint32_t));
  combinedIO.write((char*)&numSubgraphs,sizeof(uint32_t));

  for (int i = 0; i < numSubgraphs; i++)
  {
    uint32_t ioFileSize = 0;
    std::string ioFilePath = artifactsFolder + "/" + subgraphNames[i] +"_tidl_io_1.bin";
    std::ifstream ioFile(ioFilePath, std::ios::binary | std::ios::in);
    if (!ioFile.is_open())
    {
      return status;
    }
    ioFile.seekg(0,std::ios::end);
    ioFileSize = ioFile.tellg();
    ioFile.seekg(0,std::ios::beg);
    ioFile.close();

    combinedIO.write((char*)&ioOffset,sizeof(uint32_t));
    combinedIO.write((char*)&ioFileSize,sizeof(uint32_t));
    combinedIO.write((char*)&reservedBitVal,sizeof(uint32_t)); // Reserved bits
    combinedIO.write((char*)&reservedBitVal,sizeof(uint32_t)); // Reserved bits
    ioOffset = ioFileSize;
  }
  // Copy contents to the combinet bin file
  for (int i = 0; i < numSubgraphs; i++)
  {
    std::string ioFilePath = artifactsFolder + "/" + subgraphNames[i] +"_tidl_io_1.bin";
    std::ifstream ioFile(ioFilePath, std::ios::binary | std::ios::in);
    if (!ioFile.is_open())
    {
      return status;
    }
    combinedIO << ioFile.rdbuf();
    ioFile.close();
    std::remove(ioFilePath.c_str());
  }
  combinedIO.close();
  tempName = artifactsFolder + "/" + subgraphNames[0] + "_tidl_io_1.bin";
  std::rename(combinedIOFilePath.c_str(), tempName.c_str());
  /****************************************************************************/

  /****************************************************************************/
  // Change onnxrt metadata according to the combined iobuf
  std::ofstream onnxrtMetaDataModified(artifactsFolder + "/onnxrtMetaData.txt", std::ios::trunc);
  if (!onnxrtMetaDataModified.is_open())
  {
    return status;
  }

  onnxrtMetaDataModified << "numGraphNodes=" << numGraphNodes << endl;
  onnxrtMetaDataModified << "0:subGraphName=" <<  subgraphNames[0] << endl; //change this
  onnxrtMetaDataModified << "0:numInputs=" << combinedIoBufDesc.numInputBuf << endl;
  onnxrtMetaDataModified << "0:numOutputs=" << combinedIoBufDesc.numOutputBuf << endl;
  onnxrtMetaDataModified << "0:inputIdx=";
  for(int i = 0; i < combinedIoBufDesc.numInputBuf; i++)
  {
    if(i > 0) onnxrtMetaDataModified << ",";
    onnxrtMetaDataModified << combinedInputIdx[i];
  }
  onnxrtMetaDataModified << endl;

  onnxrtMetaDataModified << "0:inDataNames=";
  for(int i = 0; i < combinedIoBufDesc.numInputBuf; i++)
  {
    if(i > 0) onnxrtMetaDataModified << ",";
    onnxrtMetaDataModified << (char *)combinedIoBufDesc.inDataName[i];
  }
  onnxrtMetaDataModified << endl;

  onnxrtMetaDataModified << "0:outDataNames=";
  for(int i = 0; i < combinedIoBufDesc.numOutputBuf; i++)
  {
    if(i > 0) onnxrtMetaDataModified << ",";
    onnxrtMetaDataModified << (char *)combinedIoBufDesc.outDataName[i];
  }
  onnxrtMetaDataModified << endl;
  onnxrtMetaDataModified.close();
  /****************************************************************************/

  /****************************************************************************/
  // Change allowNode.txt
  std::ofstream allowedNodeModified(artifactsFolder + "/allowedNode.txt", std::ios::trunc);
  if (!allowedNodeModified.is_open())
  {
    return status;
  }
  allowedNodeModified << 1 << endl;
  allowedNodeModified << numGraphNodes << endl;
  for (int i = 0; i < numGraphNodes; i++)
  {
    allowedNodeModified << i << endl;
  }
  allowedNodeModified <<  endl;
  allowedNodeModified.close();
  status = 0;
  return status;
}


/* Check final compatibility of model for direct inference with TIDL-RT */
void TIDL_checkCompatibilityWithTIDLRT(OnnxTIDLSubGraphParams * state_subGraph, std::string artifactsFolder)
{
  int32_t status = 0;
  int32_t numGraphNodes = 0;
  int32_t numSubgraphs = 0;
  int32_t numOffloadedNodes = 0;
  std::string line;
  std::vector<std::string> dict;

  // Get total graph nodes from onnxrtMetaData
  std::string onnxrtMetaDataPath = artifactsFolder + "/onnxrtMetaData.txt";
  std::ifstream onnxrtMetaData(onnxrtMetaDataPath);
  if (!onnxrtMetaData.is_open())
  {
    TIDL_GLOBAL_REPORT_WARNING("Could not open %s. Skipping compatibility check for direct inference with TIDL-RT." , onnxrtMetaDataPath.c_str());
    status = -1;
  }

  if(status == 0)
  {
    while(std::getline(onnxrtMetaData, line))
    {
      dict = TIDL_splitStringWithDelimiter(line, "=");
      if(dict[0] == "numGraphNodes")
      {
        std::stringstream num(dict[1]);
        num >> numGraphNodes;
        break;
      }
    }
    onnxrtMetaData.close();

    // Get total offloaded nodes from allowedNode
    std::string allowedNodePath = artifactsFolder + "/allowedNode.txt";
    std::ifstream allowedNode(allowedNodePath);
    if (!allowedNode.is_open())
    {
      TIDL_GLOBAL_REPORT_WARNING("Could not open %s. Skipping compatibility check for direct inference with TIDL-RT." , allowedNodePath.c_str());
      status = -1;
    }
    if(status == 0)
    {
      std::getline(allowedNode, line);
      std::stringstream num(line);
      num >> numSubgraphs;

      int32_t line_no = 1;
      int32_t count = 1;
      while(std::getline(allowedNode, line))
      {
        int32_t data;
        std::stringstream num(line);
        num >> data;
        if(count == line_no)
        {
          numOffloadedNodes += data;
          line_no += data + 1;
        }
        else
        {
          // Store offloaded node index;
        }
        count++;
      }
      allowedNode.close();

      if(numGraphNodes == 0 || numOffloadedNodes == 0 || numSubgraphs == 0)
      {
        // pass
      }
      else if(numGraphNodes != numOffloadedNodes)
      {
        TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Yellow, "Number of offloaded nodes (%d) != Total nodes (%d). Some nodes are delegated to ARM and cannot directly run with TIDL-RT" , numOffloadedNodes, numGraphNodes);
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
      }
      else
      {
        if(numSubgraphs != 1)
        {
          TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%d subgraphs detected, merging them.", numSubgraphs);
          status = TIDL_combileArtifactsForTIDLRTMultiSubgraph(artifactsFolder, numGraphNodes);
          if(status == -1)
          {
            TIDL_GLOBAL_REPORT_ERROR("Subgraph merger failed.");
          }
        }
        TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Green, "Number of offloaded nodes (%d) == Total nodes (%d). All nodes are delegated to C7x and can directly run with TIDL-RT" , numOffloadedNodes, numGraphNodes);
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
      }
    }
  }
}

extern "C"
{

bool TIDLEP_checkCompatibility(const char *ortVersion)
{
  bool status = false;
  if (TIDL_onnxCompatibilityMap.find(ortVersion) == TIDL_onnxCompatibilityMap.end())
  {
    TIDL_GLOBAL_REPORT_WARNING("ONNX version %s not found in compatibility map. Skipping compatibility check." , ortVersion);
    return true;
  }

  std::pair<std::string, std::string> versionRange;
  std::string minVersion, maxVersion;

  versionRange= TIDL_onnxCompatibilityMap[ortVersion].first;
  minVersion = versionRange.first;
  maxVersion = versionRange.second;

  status = TIDL_isVersionInRange(TIDL_TOOLS_VERSION, minVersion.c_str(), maxVersion.c_str());
  if(!status)
  {
    TIDL_GLOBAL_REPORT_ERROR("TIDL TOOLS version %s is not supported by ONNX version %s. Only versions %s to %s is supported" ,
      TIDL_TOOLS_VERSION, ortVersion, minVersion.c_str(), maxVersion.c_str());
  }

  return status;
}

int32_t TIDLEP_getDdrStats(uint64_t * read, uint64_t * write)
{
  return(data_->infer_ops.TIDLRT_getDdrStats(read, write));
}

/** This function populates onnxRtParams_t and OnnxTIDLSubGraphParams structures
  * Also finds index of input/output in original ONNX graph, save it and save corresponding input/output layer names
  * as part of onnxRtParams_t
  * This function is Init kind of function, and is called only once for each subgraph
  */
int32_t TIDL_createStateImportFunc(OnnxTIDLSubGraphParams * state_subGraph, std::string * string_buf, const std::string node_name)
{
  int32_t status = 0;
  TIDL_GLOBAL_REPORT_INFO(data_->osrt_options.osrtDebugPrintLevel, "TIDL_createStateImportFunc Started:");
  onnxRtParams_t * onnxRtParams = &state_subGraph->onnxRtParams;
  state_subGraph->currFrameIdx_ = 0;
  state_subGraph->subGraphPtr_ = NULL;
  state_subGraph->string_buf = string_buf;
  std::string serialNumber = std::to_string(state_subGraph->serialNumber);

  ModelProto model_proto;
  model_proto.ParseFromString(*string_buf);
  auto onnxGraph = model_proto.graph();

  for(int i = 0; i < onnxGraph.output_size(); i++)
  {
    onnxRtParams->outputTensorElementType[i] = onnxGraph.output(i).type().tensor_type().elem_type();
  }

  if(data_->osrt_options.osrtDebugPrintLevel)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Compute on node : %s",  node_name.c_str());
    status = TIDL_printOnnxGraphProperties(onnxGraph);
  }

  int8_t subgraphName[TIDLRT_STRING_SIZE];
  status = TIDL_updateSubGraphName (onnxGraph, state_subGraph->subGraphName_, (char*)subgraphName, serialNumber.c_str());
  // TIDL_replaceChar mutates the buffer in place and returns the same pointer,
  // so an additional strcpy(dst, dst) was an overlapping self-copy (UB).
  TIDL_replaceChar((char*)state_subGraph->subGraphName_, '/', '_', strlen((const char*)state_subGraph->subGraphName_));

  int32_t currIdx = 0;
  for (int i = 0; i < onnxGraph.input_size(); i++)
  {
    if (TIDL_isInputConst(string_buf, onnxGraph.input(i).name()))
    {
      continue;
    }
    state_subGraph->inputIdx[currIdx++] = i;
  }
  state_subGraph->numInputs = currIdx;
  state_subGraph->numOutputs = onnxGraph.output_size();

  for (int i = 0; i < state_subGraph->numInputs; i++)
  {
    TIDL_GLOBAL_REPORT_INFO(data_->osrt_options.osrtDebugPrintLevel, "\nInput tensor name -  %s ", onnxGraph.input(state_subGraph->inputIdx[i]).name().c_str());
    strcpy((char *)onnxRtParams->inDataNames[i],  (char*)onnxGraph.input(state_subGraph->inputIdx[i]).name().c_str());
  }
  for (int i = 0; i < state_subGraph->numOutputs; i++)
  {
    TIDL_GLOBAL_REPORT_INFO(data_->osrt_options.osrtDebugPrintLevel, "Output tensor name - %s ", onnxGraph.output(i).name().c_str());
    strcpy((char *)onnxRtParams->outDataNames[i],  onnxGraph.output(i).name().c_str());
  }

  TIDL_writeOnnxRtMetaDataForInference(onnxGraph, state_subGraph, data_->osrt_options.m_artifacts_folder);

  return status;
}

/** This function is responsbile for complete import of a subgraph including following steps:
  * Import init - Adding input/output data layers for subgraph and setting their properties, setting default import parameters
  * Mapping framework nodes to TIDL layers and linking all the layers
  * Performing layer level optimization of the network
  * Calibration, quantization and saving final subgraph to net.bin and io.bin
  * ** IMPORTANT POINT TO NOTE **: This function is called for each frame, but subgraph import is done only for the first frame with floating point mode,
  * followed by TIDL RT handle creation for float mode, the reason being we want to save intermediate outputs of all subgraphs in
  * float mode only, so the TIDL RT float mode handle created here (for first frame) is invoked for each frame in "TIDL_computeInvokeFunc"
 */
int32_t TIDL_computeImportFunc(OnnxTIDLSubGraphParams * state_subGraph, std::string * string_buf,int32_t opSetVersion)
{
  ModelProto model_proto;
  model_proto.ParseFromString(*string_buf);
  std::string serialNumber = std::to_string(state_subGraph->serialNumber);

  auto onnxGraph = model_proto.graph();

  // Check if subgraph contains OD post processing part
  bool isSubgraphOd = false;

  /** We need to check whether subgraph is OD only when post processing happens on TIDL(which is when meta arch file is provided)
   * else the post processing part is delegated to ARM and not seen as TIDL subgraph here */
  if(data_->osrt_options.m_meta_arch_type != -1) //meta arch provided
  {
    isSubgraphOd = TIDL_checkIsSubgraphOd(onnxGraph);
  }

  onnxRtParams_t * onnxRtParams = &state_subGraph->onnxRtParams;

  if(onnxRtParams->numNetInData > TIDL_MAX_ALG_IN_BUFS)
  {
    TIDL_GLOBAL_REPORT_ERROR("Number of inputs to the graph is %d,maximum supported is %d", onnxRtParams->numNetInData,TIDL_MAX_ALG_IN_BUFS);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  if(onnxRtParams->numNetOutData > TIDL_MAX_ALG_OUT_BUFS)
  {
    TIDL_GLOBAL_REPORT_ERROR("Number of outputs to the graph is %d,maximum supported is %d", onnxRtParams->numNetOutData,TIDL_MAX_ALG_OUT_BUFS);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  int32_t status = 0;
  state_subGraph->currFrameIdx_++;
    /* [TIDL-4702] : Update the subgraph string_buf for each frame */
  TIDL_onnxRTUpdateSubgraphStringBuf((std::string* )state_subGraph->string_buf);
  // Update inDataNamesList for each frame
  for (int i = 0; i < onnxRtParams->numNetInData; i++)
  {
    TIDL_updateNamesList ((char*)gParams.inDataNamesList, i, (char *)onnxRtParams->inDataNames[i]);
  }
  
  if ((state_subGraph->currFrameIdx_ == 1))
  {
    int8_t subgraphName[TIDLRT_STRING_SIZE];
    TIDL_updateSubGraphName (onnxGraph, state_subGraph->subGraphName_, (char*)subgraphName, serialNumber.c_str());
    // In-place replace; previous strcpy(dst, dst) was an overlapping self-copy (UB).
    TIDL_replaceChar((char*)state_subGraph->subGraphName_, '/', '_', strlen((const char*)state_subGraph->subGraphName_));

    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_onnxRtImportInit(onnxGraph, onnxRtParams, (char*)state_subGraph->subGraphName_, &data_->osrt_options, opSetVersion,
                              data_->odPostProcHeadNames, isSubgraphOd, (std::string* )state_subGraph->string_buf), "Onnx RT import failed");

    for (int i = 0; i < onnxGraph.node_size(); i++)
    {
      if((data_->odBackboneNodeNames.size() == 0)  //non OD network
          || (std::find(data_->odBackboneNodeNames.begin(), data_->odBackboneNodeNames.end(), onnxGraph.node(i).name()) != data_->odBackboneNodeNames.end()))
      {
        //Map all nodes for non OD network. For OD network, map nodes only if they are part of backbone, do not map the post proc nodes
        TIDL_IMPORT_CHECK_AND_RETURN(TIDL_onnxRtImportAndLinkNode(onnxGraph, i, data_->osrt_options.m_debug_level), "Onnx RT mapping to TIDL nodes failed");
      }
    }

    if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
    {
      const char * artifacts_folder = const_cast<char *>(data_->osrt_options.m_temp_folder.c_str());
      snprintf((char *)gParams.outputNetFile, FILE_NAME_SIZE, "%s/%s_tidl_net.bin", artifacts_folder, state_subGraph->subGraphName_);
      snprintf((char *)gParams.outputParamsFile, FILE_NAME_SIZE, "%s/%s_tidl_io_", artifacts_folder, state_subGraph->subGraphName_);
    }

    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization for %s Started]" , state_subGraph->subGraphName_);
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runtimesOptimizeNet(data_->osrt_options.osrtDebugPrintLevel), "");
    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Optimization for %s Completed]" , state_subGraph->subGraphName_);

    TIDL_saveTidlSubGraph(&state_subGraph->subGraphPtr_);

    std::vector<float32_tidl> qfloat(TIDL_MAX_ALG_IN_BUFS);
    //TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\n ************** Frame index 1 : Running float import ************* \n");

    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_runtimesPostProcessNet(&data_->osrt_options, 1, 32, state_subGraph->subGraphPtr_, NULL, qfloat.data(), NULL, state_subGraph->subGraphName_), "");

    sTIDL_tidlRtParams_t * subgraphParams = reinterpret_cast<sTIDL_tidlRtParams_t *>(&state_subGraph->tidlRtParams);

    /* Setting options used in TIDL_subgraphRtCreate function */
    sTIDL_subgraphRtCreateOptions_t* subgraphRtCreateOptions = (sTIDL_subgraphRtCreateOptions_t*)malloc(sizeof(sTIDL_subgraphRtCreateOptions_t));
    strcpy(subgraphRtCreateOptions->netPath, data_->osrt_options.m_temp_folder.c_str());
    subgraphRtCreateOptions->debugLevel = data_->osrt_options.m_debug_level;
    subgraphRtCreateOptions->osrtDebugPrintLevel = data_->osrt_options.osrtDebugPrintLevel;
    subgraphRtCreateOptions->maxPreEmptDelay = FLT_MAX;
    subgraphRtCreateOptions->targetPriority = 0;
    subgraphRtCreateOptions->coreNum = 1;
    subgraphRtCreateOptions->coreStartIdx = 1;
    subgraphRtCreateOptions->inferenceMode = TIDL_inferenceModeDefault;
    subgraphRtCreateOptions->flowCtrl = TIDL_FLOW_CTRL_COMPILE;
    strcpy(subgraphRtCreateOptions->tempBufferDir, data_->osrt_options.m_temp_buffer_dir.c_str());

    /* Force inference mode to default for floating point compilation pass in OSRT. Fixed point pass indirectly calls invoke through standalone 
    PC_dsp_test_dl_algo.out exe instead of calling the RT executable directly, so this is not required for fixed point pass */
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_subgraphRtCreate(subgraphRtCreateOptions, &data_->infer_ops, state_subGraph->subGraphName_, subgraphParams),"Unable to create TIDL subgraph : Please ensure %s has sufficient free space", (char*)subgraphRtCreateOptions->tempBufferDir);
    
    if(subgraphRtCreateOptions != NULL)
    {
      free(subgraphRtCreateOptions);
    }
    
    TIDL_GLOBAL_REPORT_HEADING(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, TIDL_ModelDiagnostic::DK_Purple, "[Quantization & Calibration for %s Started]",state_subGraph->subGraphName_);
  }

  return status;
}


/** This function calls TIDL RT invoke to perform graph processing - this function is called for each subgraph each frame
  * "TIDL_subgraphRtInvoke" processes the subgraph and computes outputs
  * "TIDL_subgraphImport" utilises these outputs to be saved as inputs for the consumer subgraphs and only for the last frame,
  * runs calibration, performs quantization and saves the final quantized subgraph
  */
int32_t TIDL_computeInvokeFunc(OnnxTIDLSubGraphParams * state_subGraph)
{
  int32_t status = 0;
  if((data_->osrt_options.m_num_param_bits != 32) && !(data_->osrt_options.m_num_param_bits == 16 && data_->osrt_options.m_inference_precision_mode == TIDL_InferencePrecisionModeFloatingPoint) && (data_->osrt_options.m_prequantized_model == 0))
  {
    /* Floating point pass for subgraph input collection not required in case of execution with numParamBits = 32 and BF16 inference mode */
    TIDL_IMPORT_CHECK_AND_RETURN(TIDL_subgraphRtInvoke(data_->osrt_options.osrtDebugPrintLevel, state_subGraph, &data_->infer_ops), "");
  }
  else if((data_->osrt_options.m_num_param_bits == 32) || (data_->osrt_options.m_num_param_bits == 16 && data_->osrt_options.m_inference_precision_mode == TIDL_InferencePrecisionModeFloatingPoint))
  {
    /* In 32-bit float mode the floating point invoke is skipped, so output buffers
     * are zeroed.  Exception: if the output node in subgraph is Shape or Size with
     * static input, fill the buffer with the correct int64 values so that downstream
     * no-offload consumers (Gather, Mul, Concat, Reshape …) receive correct data. */
    onnxRtParams_t * onnxRtParams = &state_subGraph->onnxRtParams;

    /* parse the subgraph to locate Shape/Size producers of each output tensor */
    ModelProto subgraphModel;
    std::string *subgraphStringBuf = reinterpret_cast<std::string *>(state_subGraph->string_buf);
    subgraphModel.ParseFromString(*subgraphStringBuf);
    const GraphProto& subgraph = subgraphModel.graph();

    auto findVI = [&](const std::string& name) -> const TypeProto* {
      for (int n = 0; n < subgraph.input_size(); n++)
        if (subgraph.input(n).name() == name) return &subgraph.input(n).type();
      for (int n = 0; n < subgraph.value_info_size(); n++)
        if (subgraph.value_info(n).name() == name) return &subgraph.value_info(n).type();
      for (int n = 0; n < subgraph.output_size(); n++)
        if (subgraph.output(n).name() == name) return &subgraph.output(n).type();
      return nullptr;
    };

    for (int i = 0; i < onnxRtParams->numNetOutData; i++)
    {
      const std::string outName((char*)onnxRtParams->outDataNames[i]);
      bool filled = false;

      const NodeProto* prod = nullptr;
      for (int n = 0; n < subgraph.node_size() && !prod; n++)
        for (const auto& o : subgraph.node(n).output())
          if (o == outName) { prod = &subgraph.node(n); break; }

      if (prod != nullptr)
      {
        const bool isShape = (prod->op_type() == "Shape");
        const bool isSize  = (prod->op_type() == "Size");

        if ((isShape || isSize) && prod->input_size() > 0)
        {
          const TypeProto* viPtr = findVI(prod->input(0));
          if (viPtr != nullptr)
          {
            const TypeProto& tp = *viPtr;
            if (tp.has_tensor_type() && tp.tensor_type().has_shape())
            {
              const TensorShapeProto& tsp = tp.tensor_type().shape();

              /* check all dims are known static values */
              bool isStatic = true;
              for (int d = 0; d < tsp.dim_size(); d++)
              {
                if (!tsp.dim(d).has_dim_value() || tsp.dim(d).dim_value() < 0)
                {
                  isStatic = false;
                  break;
                }
              }

              if (isStatic)
              {
                int64_t* outPtr = (int64_t*)onnxRtParams->outputTensorData[i];
                if (isShape)
                {
                  /* respect optional start/end attributes */
                  int32_t start = 0;
                  int32_t end   = tsp.dim_size();
                  for (const auto& attr : prod->attribute())
                  {
                    if (attr.name() == "start")
                      start = (attr.i() < 0) ? tsp.dim_size() + (int32_t)attr.i()
                                             : (int32_t)attr.i();
                    else if (attr.name() == "end")
                      end = (attr.i() < 0) ? tsp.dim_size() + (int32_t)attr.i()
                                           : (int32_t)attr.i();
                  }
                  start = (start < 0) ? 0 : (start > tsp.dim_size()) ? tsp.dim_size() : start;
                  end   = (end < start) ? start : (end > tsp.dim_size()) ? tsp.dim_size() : end;
                  for (int d = start; d < end; d++)
                    outPtr[d - start] = (int64_t)tsp.dim(d).dim_value();
                }
                else /* isSize */
                {
                  int64_t total = 1;
                  for (int d = 0; d < tsp.dim_size(); d++)
                    total *= tsp.dim(d).dim_value();
                  outPtr[0] = total;
                }
                filled = true;
              }
            }
          }
        }
      }

      if (!filled)
      {
        std::vector<int64_t> nchw_shape{};
        std::vector<int64_t> pitch{};
        int32_t s = TIDL_getOutputShapeAndPitch(state_subGraph->tidlRtParams.ioBufDesc, NULL, onnxRtParams->outDataNames[i], nchw_shape, pitch);
        (void)pitch; // Unused
        if (s == 0)
        {
          uint32_t bufferSize = 1;
          for(auto &data : nchw_shape)
            bufferSize *= data;
          int32_t elementType = 0;
          int32_t elementSize = 0;
          TIDL_ortGetType(onnxRtParams->outputTensorElementType[i], &elementType, &elementSize);
          bufferSize *= elementSize;
          memset((void *)onnxRtParams->outputTensorData[i], 0, bufferSize);
        }
      }
    }
  }

  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_subgraphImport(&state_subGraph->onnxRtParams, data_, state_subGraph->subGraphPtr_, state_subGraph->subGraphName_, state_subGraph->currFrameIdx_), "");

  if((state_subGraph->serialNumber == (data_->m_num_subgraphs_created-1)) && (state_subGraph->currFrameIdx_ == (data_->osrt_options.m_calibration_frames)))
  {
    TIDL_concatOutNetPrototxtIntoUserProtoOnnx();
    if(data_->osrt_options.m_enable_rt_multi_subgraph_support == 1)
    {
      TIDL_checkCompatibilityWithTIDLRT(state_subGraph, data_->osrt_options.m_artifacts_folder);
    }
  }

  return status;
}

/** This function is used to free any allocated memory and delete any allocated TIDL RT handles */
int32_t TIDL_releaseRtFunc(OnnxTIDLSubGraphParams * state_subGraph)
{
  int32_t status = 0;
  delete state_subGraph->subGraphPtr_;
  sTIDL_tidlRtParams_t * subgraphParams = reinterpret_cast<sTIDL_tidlRtParams_t *>(&state_subGraph->tidlRtParams);
  status = TIDL_subgraphRtDelete(data_->osrt_options.osrtDebugPrintLevel, &data_->infer_ops, subgraphParams);
  return status;
}

} //extern C
