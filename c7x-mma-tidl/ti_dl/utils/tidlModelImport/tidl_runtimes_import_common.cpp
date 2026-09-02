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
#include <bits/stdc++.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <utility>
#include <regex>

using namespace std;
using ::google::protobuf::Message;
#include "object_detection/protos/ssd.pb.h"
#include "object_detection/protos/image_resizer.pb.h"
#include "object_detection/protos/box_coder.pb.h"
#include "object_detection/protos/faster_rcnn_box_coder.pb.h"
#include "object_detection/protos/anchor_generator.pb.h"
#include "object_detection/protos/ssd_anchor_generator.pb.h"
#include "object_detection/protos/post_processing.pb.h"

#include "ti_dl.h"
#include "tidl_import_api.h"
#include "schema_generated.h"
#include "flatbuffers/flexbuffers.h"
#include "tidl_import_config.h"
#include "tidl_runtimes_infer_common.h"
#include "tidl_runtimes_import_common.h"
#include "tidl_import_common.h"
#include "tidl_meta_arch.pb.h"
#include "tidl_quant_params.pb.h"
#include "object_detection/protos/pipeline.pb.h"
#include "tidl_import_common_tidlrt_osrt.h"
#include "tidl_import_diag.h"

#define __MODULE__ ""

#define MAX_NUM_TIDL_SUBGRAPHS (16)

sTIDL_runtimesImportState_t runtimes_import_state;

extern uint8_t tidl_OdOutputNames[TIDL_MAX_TF_SSD_LAYERS][TIDL_STRING_SIZE];
extern "C"
{

/** This function read the names of the OD heads from meta arch file and returns them as a vector of strings
 * */
std::vector<std::string> TIDL_readMetaArchInfo(std::string filePath,int32_t graphOutputSize,int metaType, void* odOutParams)
{
  int32_t  i, j, k, l;
  std::vector<std::string> inOdDataNames = {};
  TIDL_odParams_t *odParams = (TIDL_odParams_t*)odOutParams;
  if(odParams != NULL)
  {
    odParams->codeType = 0;
    odParams->subCodeType = 0;
    odParams->keepTopK = 0;
    odParams->num_keypoint = 0;
    odParams->keypoint_confidence = 0;
    odParams->tidl_OdFramework = "";
  }

  if(metaType != -1)
  {
    if(metaType ==  TIDL_metaArchTFSSD)
    {
      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "TIDL Meta pipeLine (proto) file  : %s  ", filePath.c_str());
      object_detection::protos::TrainEvalPipelineConfig tfOdpipeLineDef;
      bool success = TIDL_readProtoFromTextFile((const char *)filePath.c_str(), &tfOdpipeLineDef);
      if(!success) TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Reading prototext file failed");

      if (tfOdpipeLineDef.has_model() == true)
      {
        if (tfOdpipeLineDef.model().has_ssd() == true)
        {
          auto &ssdDef = tfOdpipeLineDef.model().ssd();
          if(ssdDef.box_input_size() == ssdDef.class_input_size())
          {
            for (j = 0; j < ssdDef.box_input_size(); j++)
            {
              inOdDataNames.push_back(ssdDef.box_input(j).c_str());
              inOdDataNames.push_back(ssdDef.class_input(j).c_str());
            }
            // TODO: To populate odParams for TFSSD if required to support in TVM

          }
          else
          {
            TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nBox input size(%d) and Class input size(%d) should match, double check the input names", ssdDef.box_input_size(), ssdDef.class_input_size());
          }
        }
      }
    }
    else
    {
      tidl_meta_arch::TIDLMetaArch  tidlMetaArch;
      bool success = TIDL_readProtoFromTextFile((const char *)filePath.c_str(), &tidlMetaArch);
      if(!success) TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Reading prototext file failed");

      TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s is meta arch name ",tidlMetaArch.name().c_str());

      for (j = 0; j < tidlMetaArch.caffe_ssd_size(); j++)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.caffe_ssd(j).name().c_str());

        for(k =0; k < tidlMetaArch.caffe_ssd(j).box_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.caffe_ssd(j).box_input(k));
        }
        for(k =0; k < tidlMetaArch.caffe_ssd(j).class_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.caffe_ssd(j).class_input(k));
        }
        if(tidlMetaArch.caffe_ssd(j).output_size() != graphOutputSize)
        {
          TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Number of output names provided in prototxt - %d - do not match actual number of outputs of OD network - %d ", tidlMetaArch.caffe_ssd(j).output_size(), graphOutputSize);
        }
        for(k =0; k < tidlMetaArch.caffe_ssd(j).output_size(); k++)
        {
          strcpy((char *)tidl_OdOutputNames[k],tidlMetaArch.caffe_ssd(j).output(k).c_str());
        }
        if(odParams != NULL) {
          if(tidlMetaArch.caffe_ssd(j).detection_output_param().has_keep_top_k())
          {
            odParams->keepTopK = tidlMetaArch.caffe_ssd(j).detection_output_param().keep_top_k();
          }
          odParams->tidl_OdFramework = tidlMetaArch.caffe_ssd(j).has_framework() ? tidlMetaArch.caffe_ssd(j).framework() : "";
        }
      }

      for (j = 0; j < tidlMetaArch.centernet_size(); j++)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.centernet(j).name().c_str());

        for(k =0; k < tidlMetaArch.centernet(j).box_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.centernet(j).box_input(k).name().c_str());
        }
        for(k =0; k < tidlMetaArch.centernet(j).class_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.centernet(j).class_input(k).name().c_str());
        }

        for(k =0; k < tidlMetaArch.centernet(j).reg_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.centernet(j).reg_input(k).name().c_str());
        }

        if(tidlMetaArch.centernet(j).output_size() != graphOutputSize)
        {
          TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Number of output names provided in prototxt - %d - do not match actual number of outputs of OD network - %d ", tidlMetaArch.centernet(j).output_size(), graphOutputSize);
        }
        for(k =0; k < tidlMetaArch.centernet(j).output_size(); k++)
        {
          strcpy((char *)tidl_OdOutputNames[k],tidlMetaArch.centernet(j).output(k).c_str());
        }
        if(odParams != NULL) {
          if(tidlMetaArch.centernet(j).detection_output_param().has_keep_top_k())
          {
            odParams->keepTopK = tidlMetaArch.centernet(j).detection_output_param().keep_top_k();
          }
          odParams->tidl_OdFramework = tidlMetaArch.centernet(j).has_framework() ? tidlMetaArch.centernet(j).framework() : "";
        }
      }

      for (j = 0; j < tidlMetaArch.tidl_retinanet_size(); j++)        //tidl_retinanet_size()
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.tidl_retinanet(j).name().c_str());

        if(tidlMetaArch.tidl_retinanet(j).box_input_size() == tidlMetaArch.tidl_retinanet(j).class_input_size())
        {
          for(k =0; k < tidlMetaArch.tidl_retinanet(j).box_input_size(); k++)
          {
            inOdDataNames.push_back(tidlMetaArch.tidl_retinanet(j).box_input(k).c_str());
          }
          for(k =0; k < tidlMetaArch.tidl_retinanet(j).class_input_size(); k++)
          {
            inOdDataNames.push_back(tidlMetaArch.tidl_retinanet(j).class_input(k).c_str());
          }
        }
        else
        {
          TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "\nBox input size(%d) and Class input size(%d) should match, double check the input names", tidlMetaArch.tidl_retinanet(j).box_input_size(), tidlMetaArch.tidl_retinanet(j).class_input_size());
        }
        if(tidlMetaArch.tidl_retinanet(j).output_size() != graphOutputSize)
        {
          TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Number of output names provided in prototxt - %d - do not match actual number of outputs of OD network - %d ", tidlMetaArch.tidl_retinanet(j).output_size(), graphOutputSize);
        }
        for(k =0; k < tidlMetaArch.tidl_retinanet(j).output_size(); k++)
        {
          strcpy((char *)tidl_OdOutputNames[k],tidlMetaArch.tidl_retinanet(j).output(k).c_str());
        }
        if(odParams != NULL) {
          if(tidlMetaArch.tidl_retinanet(j).detection_output_param().has_keep_top_k())
          {
            odParams->keepTopK = tidlMetaArch.tidl_retinanet(j).detection_output_param().keep_top_k();
          }
          odParams->tidl_OdFramework = tidlMetaArch.tidl_retinanet(j).has_framework() ? tidlMetaArch.tidl_retinanet(j).framework() : "";
        }
      }

      for (j = 0; j < tidlMetaArch.tidl_yolo_size(); j++)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.tidl_yolo(j).name().c_str());
        for(k =0; k < tidlMetaArch.tidl_yolo(j).yolo_param_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.tidl_yolo(j).yolo_param(k).input().c_str());
        }
        if(tidlMetaArch.tidl_yolo(j).output_size() != graphOutputSize)
        {
          TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Number of output names provided in prototxt - %d - do not match actual number of outputs of OD network - %d", tidlMetaArch.tidl_yolo(j).output_size(), graphOutputSize);
        }
        for(int k = 0; k < tidlMetaArch.tidl_yolo(j).output_size(); k++)
        {
          strcpy((char *)tidl_OdOutputNames[k],tidlMetaArch.tidl_yolo(j).output(k).c_str());
        }

        if(odParams != NULL) {
          if (tidlMetaArch.tidl_yolo(j).detection_output_param().has_num_keypoint())
          {
            odParams->num_keypoint = tidlMetaArch.tidl_yolo(j).detection_output_param().num_keypoint();
          }

          if (tidlMetaArch.tidl_yolo(j).detection_output_param().has_keypoint_confidence())
          {
            odParams->keypoint_confidence = tidlMetaArch.tidl_yolo(j).detection_output_param().keypoint_confidence();
          }

          if (tidlMetaArch.tidl_yolo(j).detection_output_param().has_keep_top_k())
          {
            odParams->keepTopK = tidlMetaArch.tidl_yolo(j).detection_output_param().keep_top_k();
          }

          if (tidlMetaArch.tidl_yolo(j).detection_output_param().has_sub_code_type())
          {
            odParams->subCodeType = tidlMetaArch.tidl_yolo(j).detection_output_param().sub_code_type();
          }
          odParams->tidl_OdFramework = tidlMetaArch.tidl_yolo(j).has_framework() ? tidlMetaArch.tidl_yolo(j).framework() : "";
        }
      }

      for (j = 0; j < tidlMetaArch.tidl_3dod_size(); j++)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.tidl_3dod(j).name().c_str());
        for(k =0; k < tidlMetaArch.tidl_3dod(j).box_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.tidl_3dod(j).box_input(k).c_str());
        }
        for(k =0; k < tidlMetaArch.tidl_3dod(j).class_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.tidl_3dod(j).class_input(k).c_str());
        }
        for(k =0; k < tidlMetaArch.tidl_3dod(j).dir_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.tidl_3dod(j).dir_input(k).c_str());
        }
        if(tidlMetaArch.tidl_3dod(j).output_size() != graphOutputSize)
        {
          TIDL_GLOBAL_REPORT_FATAL_AND_ABORT("Number of output names provided in prototxt - %d - do not match actual number of outputs of OD network - %d ", tidlMetaArch.tidl_yolo(j).output_size(), graphOutputSize);
        }
        for(k =0; k < tidlMetaArch.tidl_3dod(j).output_size(); k++)
        {
          strcpy((char *)tidl_OdOutputNames[k],tidlMetaArch.tidl_3dod(j).output(k).c_str());
        }

        if(odParams != NULL) {
          if (tidlMetaArch.tidl_3dod(j).detection_output_param().has_keep_top_k())
          {
            odParams->keepTopK = tidlMetaArch.tidl_3dod(j).detection_output_param().keep_top_k();
          }

          if (tidlMetaArch.tidl_3dod(j).detection_output_param().has_code_type())
          {
            odParams->codeType = tidlMetaArch.tidl_3dod(j).detection_output_param().code_type();
          }
          odParams->tidl_OdFramework = "";
        }
      }

      for (j = 0; j < tidlMetaArch.tidl_bevformer_size(); j++)
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "%s",tidlMetaArch.tidl_bevformer(j).name().c_str());

        for(k =0; k < tidlMetaArch.tidl_bevformer(j).box_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.tidl_bevformer(j).box_input(k).name().c_str());
        }
        for(k =0; k < tidlMetaArch.tidl_bevformer(j).class_input_size(); k++)
        {
          inOdDataNames.push_back(tidlMetaArch.tidl_bevformer(j).class_input(k).name().c_str());
        }

        for(k =0; k < tidlMetaArch.tidl_bevformer(j).output_size(); k++)
        {
          strcpy((char *)tidl_OdOutputNames[k],tidlMetaArch.tidl_bevformer(j).output(k).c_str());
        }
        if(odParams != NULL) {
          if (tidlMetaArch.tidl_bevformer(j).detection_output_param().has_keep_top_k())
          {
            odParams->keepTopK = tidlMetaArch.tidl_bevformer(j).detection_output_param().keep_top_k();
          }

          odParams->tidl_OdFramework = tidlMetaArch.tidl_bevformer(j).has_framework() ? tidlMetaArch.tidl_bevformer(j).framework() : "";
        }
      }
    }
  }
  return inOdDataNames;
}

void TIDL_updateNamesList (char *tidlNamesList, int32_t index, char* onnxName)
{
  if (index == 0)
  {
    strcpy ((char*)(tidlNamesList), onnxName);
  }
  else if (index > 0)
  {
    strcat((char*)tidlNamesList, ",");
    strcat((char*)tidlNamesList, onnxName);
  }
}

/** Iterative implementation of DFS on graph */
void TIDL_nodeTraversal(std::vector<std::vector<int>> adjacencyList, int startIdx, std::vector<int> &odBackboneNodes, std::vector<bool> &visited)
{
  // Mark the current node as visited
  visited[startIdx] = true;
  odBackboneNodes.push_back(startIdx);

  // Create a stack for DFS
  std::stack<int> stack;
  // Push the current source node
  stack.push(startIdx);

  while (!stack.empty())
  {
    // Pop the topmost node from stack
    startIdx = stack.top();
    stack.pop();

    if (!visited[startIdx])
    {
      odBackboneNodes.push_back(startIdx);
      visited[startIdx] = true;
    }

    // Get all adjacent nodes of the popped node
    // If an adjacent has not been visited, then push it
    // to the stack.
    for (int i = 0; i < adjacencyList[startIdx].size(); i++)
    {
      if (visited[adjacencyList[startIdx][i]] == false)
            stack.push(adjacencyList[startIdx][i]);
    }
  }
}

/** This function calls DFS with each of the OD heads as root node */
std::vector<int> TIDL_callNodeTraversal(std::vector<std::vector<int>> adjacencyList, std::vector<int> postProcInputIds, int graphSize)
{
  std::vector<bool> visited;
  visited.assign(graphSize, false);

  std::vector<int> odBackboneNodes = {};
  for(auto inIds : postProcInputIds)  //Trace back from each of the OD post proc inputs/ heads
  {
    TIDL_nodeTraversal(adjacencyList, inIds, odBackboneNodes, visited);
  }
  return odBackboneNodes;
}

/** This function calls the exe for runtimes visualization */
void TIDL_runGraphvizToolRuntimes(std::string tidlToolsPath, std::string artifactsFolderPath, int32_t debugLevel)
{
  int systemReturnCode;
  std::string allowlistPath = artifactsFolderPath + "/allowedNode.txt";
  std::string graphvizInfoPath = artifactsFolderPath + "/tempDir/graphvizInfo.txt";
  std::string outputPath = artifactsFolderPath + "/tempDir/runtimes_visualization.svg";

  std::string sysCommandStr = tidlToolsPath + "/tidl_graphVisualiser_runtimes.out " + allowlistPath + " " + graphvizInfoPath + " " + outputPath;

  char * sysCommand = new char[sysCommandStr.length() + 1];
  strcpy(sysCommand, sysCommandStr.c_str());

  if(debugLevel > 0)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Running Runtimes GraphViz - %s ", sysCommand);
  }
  systemReturnCode = system(sysCommand);
  if(systemReturnCode != 0)
  {
    TIDL_GLOBAL_REPORT_WARNING("System command failed with return code : %d. Skipping Runtimes GraphViz.", systemReturnCode);
  }

  delete [] sysCommand;
}

/* Check if directory is empty */
bool TIDL_checkIsEmpty(const char *path) {
    if (!TIDL_checkIsDir(path))
        return false;

    struct dirent *e;
    DIR *d = opendir(path);

    if(!d)
        return false;

    errno = 0;
    while(e = readdir(d)) {
        /* do not recurse into . and .. */
        if(!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
            continue;
        return false;
    }

    if(errno)
        return false;

    return true;
}

/** Read individual elements of a comma separated string
  * This function is used to read interface options which are provided as comma separated strings */
std::vector<std::string> TIDL_readCommaSeparatedString(char * deny_list)
{
    std::vector<std::string> ret;
    char * token = strtok(deny_list, ",");
    while( token != NULL )
    {
        for(int i = 0; i < strlen(token); i++)
        {
            if(token[i] ==  ' ')
            {
                memmove(token+i, token+i+1, strlen(token) - i);
            }
        }
        std::string itoken;
        std::stringstream(token) >> itoken;
        ret.push_back(itoken);
        token = strtok(NULL, ",");
    }
    return ret;
}

/** This function can be used to add a particular layer to deny list if needed */
std::vector<int32_t> tidl_fillDenyListOption(std::string deny_list)
{
    std::vector<int32_t> ret;
    char * deny_list_char =  new char[deny_list.length() + 1];
    strcpy(deny_list_char, deny_list.c_str());

    char * token = strtok(deny_list_char, ",");

    while( token != NULL )
    {
        for(int i = 0; i < strlen(token); i++)
        {
            if(token[i] ==  ' ')
            {
                memmove(token+i, token+i+1, strlen(token) - i);
            }
        }
        int32_t itoken;
        sscanf(token, "%d", &itoken);
        ret.push_back(itoken);
        token = strtok(NULL, ",");
    }
    delete[] deny_list_char;
    return ret;
}

/* Read interface options and populate to TIDL runtimes option class */
bool TIDL_readInterfaceOptions(TIDL_osrtOptions * options, std::string option_name, std::string option_value, int model_type)
{
  if(!strcmp("advanced_options:network_name", option_name.c_str()))
  {
    options->m_network_name = option_value;
  }
  if(!strcmp("advanced_options:c7x_firmware_version", option_name.c_str()))
  {
    options->m_c7x_firmware_version = option_value;
  }
  if(!strcmp("advanced_options:quant_params_proto_path", option_name.c_str()))
  {
    options->m_quant_params_proto_path = option_value;
  }
  if(!strcmp("advanced_options:log_file_name", option_name.c_str()))
  {
    options->m_log_file_name = option_value;
  }
  if(!strcmp("advanced_options:nc_temp_info_dir", option_name.c_str()))
  {
    options->m_nc_temp_info_dir = option_value;
  }
  if(!strcmp("advanced_options:temp_buffer_dir", option_name.c_str()))
  {
    options->m_temp_buffer_dir = option_value;
    if(!TIDL_checkIsDir(options->m_temp_buffer_dir.c_str()))
    {
      TIDL_GLOBAL_REPORT_WARNING("Directory to redirect buffers (temp_buffer_dir - %s) does not exist, /dev/shm will be used instead\n", options->m_temp_buffer_dir.c_str());
      options->m_temp_buffer_dir = "/dev/shm";
    }
    if(strlen(options->m_temp_buffer_dir.c_str()) > 64U)
    {
      TIDL_GLOBAL_REPORT_WARNING("Directory to redirect temporary buffers (temp_buffer_dir - %s) exceeds the maximum path length (64), /dev/shm will be used instead\n", options->m_temp_buffer_dir.c_str());
      options->m_temp_buffer_dir = "/dev/shm";
    }
  }
  if(!strcmp("advanced_options:batch_mode", option_name.c_str()))
  {
    std::stringstream(option_value) >> options->m_batch_mode;
    if(options->m_batch_mode != 0 && options->m_batch_mode != 1)
    {
      TIDL_GLOBAL_REPORT_WARNING("Invalid batch mode %d given. Defaulting to 0.", options->m_batch_mode);
      options->m_batch_mode = 0;
    }
  }
  if (!strcmp("tidl_tools_path", option_name.c_str()))
    {
      options->m_tidl_tools_path = option_value;
      if(!TIDL_checkIsDir(options->m_tidl_tools_path.c_str()))
      {
        TIDL_GLOBAL_REPORT_ERROR("tidl_tools_path directory does not exist");
        return false;
      }
      // TODO: maybe check for the libs, quants tools, GC tool are contained inside
    }
    if (!strcmp("artifacts_folder", option_name.c_str()))
    {
      options->m_artifacts_folder = option_value;
      if(!TIDL_checkIsDir(options->m_artifacts_folder.c_str()))
      {
        TIDL_GLOBAL_REPORT_ERROR("artifacts_folder directory does not exist");
        return false;
      }
      if(!TIDL_checkIsEmpty(options->m_artifacts_folder.c_str()) && (model_type != TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY))
      {
        // For TVM, artifacts_folder can be populated in case of REUSE_TIDL_ARTIFACTS flag set
        TIDL_GLOBAL_REPORT_ERROR("artifacts_folder is not empty");
        return false;
      }
    }

    if (!strcmp("model_type", option_name.c_str()))
    {
      options->m_model_type = option_value;
      if(options->m_model_type.c_str()=="OD")
      {
        TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "The Model is classified as OD as mentioned by the user assumptions so considering as OD ");
      }
    }

    if (!strcmp("debug_level", option_name.c_str()))
    {
      std::stringstream(option_value) >> options->m_debug_level;
      // TODO: any invalid option_values? like negative, or beyond supported range?
    }

    options->osrtDebugPrintLevel = (options->m_debug_level == 0) ? 0 : 1;

    if (!strcmp("tensor_bits", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_num_param_bits;

        std::vector<int> valid_num_params{8, 16, 32};
        if(std::find(valid_num_params.begin(), valid_num_params.end(), options->m_num_param_bits) == valid_num_params.end())
        {
            TIDL_GLOBAL_REPORT_ERROR("Unsupported tensor_bits");
            return false;
        }
        if(options->m_num_param_bits == 32)
        {
          TIDL_GLOBAL_REPORT_WARNING("tensor_bits = 32, Compiling for floating point - target execution is not supported for 32 bit compilation");
        }
    }

    if (!strcmp("inference_precision_mode", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_inference_precision_mode;
        std::vector<int> valid_inference_precision_modes{TIDL_InferencePrecisionModeFixedPoint, TIDL_InferencePrecisionModeFloatingPoint};
        if(std::find(valid_inference_precision_modes.begin(), valid_inference_precision_modes.end(), options->m_inference_precision_mode) == valid_inference_precision_modes.end())
        {
            TIDL_GLOBAL_REPORT_ERROR("Unsupported inference_precision_mode");
            return false;
        }        
    }

    if (!strcmp("max_num_subgraphs", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_num_tidl_subgraphs;

        if(options->m_num_tidl_subgraphs > MAX_NUM_TIDL_SUBGRAPHS)
        {
            TIDL_GLOBAL_REPORT_ERROR("max_num_subgraphs(%d) > %d is not allowed", options->m_num_tidl_subgraphs, (int32_t)MAX_NUM_TIDL_SUBGRAPHS);
            return false;
        }
    }

    if (!strcmp("advanced_options:max_num_subgraph_nodes", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_num_tidl_subgraph_max_node;
    }

    if (!strcmp("advanced_options:enable_rt_multi_subgraph_support", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_enable_rt_multi_subgraph_support;
    }

    // TODO: fix denylist
    if (strcmp("deny_list", option_name.c_str()) == 0)        //option for Deny_List
    {
        try
        {
          std::string str = option_value;
          char *cstr = new char[str.length() + 1];
          strcpy(cstr, str.c_str());
          options->m_deny_list = TIDL_readCommaSeparatedString(cstr);
          delete[] cstr;
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed deny_list option");
            return false;
        }
    }

    if (strcmp("advanced_options:custom_layers_names_list", option_name.c_str()) == 0)        //option for custom_layer_list
    {
        try
        {
          std::string str = option_value;
          char *cstr = new char[str.length() + 1];
          strcpy(cstr, str.c_str());
          options->m_custom_layers_names_list = TIDL_readCommaSeparatedString(cstr);
          delete[] cstr;
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed custom_layers_names_list option");
            return false;
        }
    }

    if (strcmp("advanced_options:net_inelement_type", option_name.c_str()) == 0)        //option to set net_inelement_type
    {
        try
        {
          std::string str = option_value;
          char *cstr = new char[str.length() + 1];
          strcpy(cstr, str.c_str());
          std::vector<string> string_types = TIDL_readCommaSeparatedString(cstr);
          std::vector<int> int_types;
          for (string str: string_types)
          {
            int_types.push_back(stoi(str));
          }
          options->m_net_inelement_type = int_types;
          delete[] cstr;
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed net_inelement_type option");
            return false;
        }
    }

    if (strcmp("deny_list:layer_type", option_name.c_str()) == 0)  //option for Deny_List_type
    {
        try
        {
          std::string str = option_value;
          char *cstr = new char[str.length() + 1];
          strcpy(cstr, str.c_str());
          options->m_deny_list_layer_type = TIDL_readCommaSeparatedString(cstr);
          delete[] cstr;
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed deny_list option");
            return false;
        }
    }
    if (strcmp("deny_list:layer_name", option_name.c_str()) == 0)     //option for Deny_List_name
    {
        try
        {
          std::string str = option_value;
          char *cstr = new char[str.length() + 1];
          strcpy(cstr, str.c_str());
          options->m_deny_list_layer_name = TIDL_readCommaSeparatedString(cstr);
          delete[] cstr;
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed deny_list option");
            return false;
        }
    }

    if (strcmp("allow_list:layer_name", option_name.c_str()) == 0)     //option for allow_List_name
    {
        try
        {
          std::string str = option_value;
          char *cstr = new char[str.length() + 1];
          strcpy(cstr, str.c_str());
          options->m_allow_list_layer_name = TIDL_readCommaSeparatedString(cstr);
          delete[] cstr;
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed allow_list option");
            return false;
        }
    }

    if (strcmp("deny_list", option_name.c_str()) == 0)  {
        try {
            options->m_deny_list_int = tidl_fillDenyListOption(option_value);
        } catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed 'deny_list' option");
            return false;
        }
    }

    if (!strcmp("accuracy_level", option_name.c_str()))
    {
        std::map<std::string, int> valid_calibs {{"0", 64}, {"1", 7},{"2", 132}, {"9", 9}};  // 9 will be mapped to suitable flag based on advanced options
        //printf("Accuracy Level chosen : %d\n",valid_calibs.find(option_value));
        if(valid_calibs.find(option_value) == valid_calibs.end())
        {
            TIDL_GLOBAL_REPORT_ERROR("Unsupported accuracy_level, supported values are '0', '1' and '9' ");
            return false;
        }
        options->m_tidl_calibration_flags = valid_calibs[option_value];
    }

    if (!strcmp("advanced_options:calibration_frames", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_calibration_frames;
        // TODO: any invalid option_values? like negative, or too many frames?
    }

    if (!strcmp("advanced_options:calibration_iterations", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_calibration_iterations;
        // TODO: any invalid option_values? like negative, or too many iters?
    }

    if(!strcmp("advanced_options:mixed_precision_factor",option_name.c_str()))
    {
      std::stringstream(option_value) >> options->m_mixed_precision_factor;
    }
    if(!strcmp("advanced_options:model_group_id",option_name.c_str()))
    {
      std::stringstream(option_value) >> options->m_model_group_id;
    }

    if(!strcmp("advanced_options:prequantized_model", option_name.c_str()))
    {
        std::map<std::string, int> prequantized_model_mapping {{"0", 0}, {"1", 1}};
        if(prequantized_model_mapping.find(option_value) == prequantized_model_mapping.end())
        {
            TIDL_GLOBAL_REPORT_ERROR("Unsupported onnx QDQ mode, specify either '0' or '1' ");
            return false;
        }
        options->m_prequantized_model = prequantized_model_mapping[option_value];
    }

    if(!strcmp("advanced_options:partial_init_during_compile", option_name.c_str()))
    {
      std::stringstream(option_value) >> options->m_partial_init_during_compile;
    }

    if(!strcmp("advanced_options:packetize_mode", option_name.c_str()))
    {
      std::stringstream(option_value) >> options->m_packetize_mode;
    }

    if (!strcmp("advanced_options:quantization_scale_type", option_name.c_str()))
    {
        //std::map<std::string, int> quantization_scale_type_mapping {{"0", TIDL_QuantStyleNP2Fixed}, {"1", TIDL_QuantStyleAsymNP2}, {"3", TIDL_QuantStyleAsymNP2},{"4",TIDL_QuantStyleAsymNP2_TFL}};
        std::map<std::string, int> quantization_scale_type_mapping {{"0", TIDL_QuantStyleNP2Fixed}, {"1", TIDL_QuantStyleP2Dynamic}, {"3", TIDL_QuantStyleAsymNP2_TFL}, {"4", TIDL_QuantStyleAsymNP2}};
        if(quantization_scale_type_mapping.find(option_value) == quantization_scale_type_mapping.end())
        {
            TIDL_GLOBAL_REPORT_ERROR("Unsupported quantization_scale_type, specify either '0', '1', '3' or '4' ");
            return false;
        }
        options->m_quantization_scale_type = quantization_scale_type_mapping[option_value];
    }

    if (!strcmp("advanced_options:enable_tfr_optimization", option_name.c_str()))
    {
        /* 0 -> disable tfr optimization, 1 -> preserve the float range during calibration, 2 -> use global histogram for range calibration */
        std::map<std::string, int> tfr_optimization_mapping {{"0", 0}, {"1", 1}, {"2", 2}};
        if(tfr_optimization_mapping.find(option_value) == tfr_optimization_mapping.end())
        {
            TIDL_GLOBAL_REPORT_ERROR("Unsupported tfr_optimization_type, specify either '0', '1', '2'");
            return false;
        }
        options->m_enable_tfr_optimization = tfr_optimization_mapping[option_value];
    }

    if (!strcmp("advanced_options:enable_concat_no_op", option_name.c_str()))
    {
        /* 0 -> disable concat no op optimization, 1 -> enable concat no op optimization (Default) */
        std::map<std::string, int> concat_no_op_mapping {{"0", 0}, {"1", 1}};
        if(concat_no_op_mapping.find(option_value) == concat_no_op_mapping.end())
        {
            TIDL_GLOBAL_REPORT_ERROR("Unsupported concat_no_op_type, specify either '0' or '1'");
            return false;
        }
        options->m_enable_concat_no_op = concat_no_op_mapping[option_value];
    }

    if (!strcmp("advanced_options:high_resolution_optimization", option_name.c_str()))
    {
      std::stringstream(option_value) >> options->m_high_resolution_optimization;
    }

    if (!strcmp("advanced_options:pre_batchnorm_fold", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_pre_batchnorm_fold;
    }
    if (!strcmp("advanced_options:add_data_convert_ops", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_add_data_convert_ops;
    }

    if (!strcmp("advanced_options:enable_custom_layers", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_enable_custom_layers;
    }

    if (!strcmp("advanced_options:graph_optimization_level", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_graph_optimization_level;
    }

    if (!strcmp("advanced_options:enable_shape_folding", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_enable_shape_folding;
    }

    if (!strcmp("advanced_options:optimize_batchnorm_higherdims", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_optimize_batchnorm_higherdims;
    }

    if (!strcmp("bias_calibration_factor", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_bias_calibration_factor;
    }

    if (!strcmp("ti_internal_nc_flag", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_compileConstraintsFlag;
    }

    if (strcmp("ti_internal_reserved_1", option_name.c_str()) == 0)
    {
        try
        {
          std::string str = option_value;
          char *cstr = new char[str.length() + 1];
          strcpy(cstr, str.c_str());
          std::vector<std::string> ctrl = TIDL_readCommaSeparatedString(cstr);
          delete[] cstr;

          for(auto& s : ctrl)
          {
            int x;
            std::stringstream(s) >> x;
            options->m_reserved_ctrl_1.push_back(x);
          }
          options->m_reserved_ctrl_1.push_back(-1); //Read of this particular option is terminated with (-1)
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed ti_internal_reserved_1 option");
            return false;
        }

    }

    if (strcmp("ti_internal_reserved_2", option_name.c_str()) == 0)
    {
      std::stringstream(option_value) >> options->m_reserved_ctrl_2;
    }

    if (strcmp("advanced_options:softmax16BitScaleUpdate", option_name.c_str()) == 0)
    {
      std::stringstream(option_value) >> options->m_softmax_16bit_scale_update;
    }

    if (strcmp("advanced_options:use16BitForTopK", option_name.c_str()) == 0)
    {
      std::stringstream(option_value) >> options->m_use_16bit_for_topk;
    }

    if (!strcmp("advanced_options:output_feature_16bit_names_list", option_name.c_str()))
    {
      options->m_output_feature_16bit_names_list = option_value;
    }
    if (!strcmp("advanced_options:mp_16bit_resolved_feature_names_list", option_name.c_str()))
    {
      options->m_mp_16bit_resolved_feature_names_list = option_value;
    }
    if (!strcmp("advanced_options:params_16bit_names_list", option_name.c_str()))
    {
      options->m_params_16bit_names_list = option_value;
    }
    if (!strcmp("advanced_options:native_layer_names_list", option_name.c_str()))
    {
      options->m_native_layer_names_list = option_value;
    }
    if (!strcmp("advanced_options:single_core_layers_names_list", option_name.c_str()))
    {
      options->m_single_core_layers_names_list = option_value;
    }
    if (!strcmp("advanced_options:m_spatial_split_layers_names_list", option_name.c_str()))
    {
      options->m_spatial_split_layers_names_list = option_value;
    }
    if (!strcmp("advanced_options:m_channel_split_layers_names_list", option_name.c_str()))
    {
      options->m_channel_split_layers_names_list = option_value;
    }
    if (!strcmp("object_detection:meta_layers_names_list", option_name.c_str()))
    {
      options->m_meta_layers_names_list = option_value;
    }
    if (!strcmp("object_detection:meta_arch_type", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_meta_arch_type;
    }
    if (!strcmp("advanced_options:inference_mode", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_inference_mode;
#if defined (SOC_J784S4) || defined (SOC_J722S) || defined(SOC_J742S2) || defined(SOC_TDA54)
        std::vector<int> valid_inference_modes{0, 1, 2};
#else
        std::vector<int> valid_inference_modes{0};
#endif
        if(std::find(valid_inference_modes.begin(), valid_inference_modes.end(), options->m_inference_mode) == valid_inference_modes.end())
        {
            std::string values="";
            for(int i=0;i<valid_inference_modes.size();i++)
            {
              values+=std::to_string(valid_inference_modes[i]);
              if(i!=valid_inference_modes.size()-1)
                values+=", ";
            }
            TIDL_GLOBAL_REPORT_ERROR("Unsupported inference mode(%d), supported modes are %s", options->m_inference_mode, values.c_str());
            return false;
        }
    }
    if (!strcmp("advanced_options:num_cores", option_name.c_str()))
    {
        std::stringstream(option_value) >> options->m_num_cores;
#if defined (SOC_J784S4) || defined(SOC_TDA54)
        std::vector<int> valid_num_cores{1,2,3,4};
#elif defined (SOC_J722S) || defined(SOC_J742S2)
        std::vector<int> valid_num_cores{1,2};
#else
        std::vector<int> valid_num_cores{1};
#endif
        if(std::find(valid_num_cores.begin(), valid_num_cores.end(), options->m_num_cores) == valid_num_cores.end())
        {
            std::string cores="";
            for(int i=0;i<valid_num_cores.size();i++)
            {
              cores+=std::to_string(valid_num_cores[i]);
              if(i!=valid_num_cores.size()-1)
                cores+=", ";
            }
            TIDL_GLOBAL_REPORT_ERROR("Unsupported num_cores(%d), number of cores allowed are %s", options->m_num_cores, cores.c_str());
            return false;
        }
    }

    if (strcmp("advanced_options:copy_tensor_quant_spec", option_name.c_str()) == 0)
    {
        // Option for copy_tensor_quant_spec given as string of tuple "(tesor1,tensor2),(tensor8,tensor5)..."
        try
        {
          std::string str = option_value;

          // Regular expression to match tuples like (abc, xyz)
          std::regex tupleRegex(R"(\(\s*([^,]+)\s*,\s*([^)]+)\s*\))");
          std::smatch match;
          std::string::const_iterator searchStart(str.cbegin());

          while (std::regex_search(searchStart, str.cend(), match, tupleRegex))
          {
              std::string first = match[1].str();
              std::string second = match[2].str();

              // Remove leading whitespace
              first.erase(first.begin(), std::find_if(first.begin(), first.end(), [](unsigned char ch) {
                return !std::isspace(ch);
              }));

              second.erase(second.begin(), std::find_if(second.begin(), second.end(), [](unsigned char ch) {
                return !std::isspace(ch);
              }));

              // Remove trailing whitespace
              first.erase(std::find_if(first.rbegin(), first.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
              }).base(), first.end());

              second.erase(std::find_if(second.rbegin(), second.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
              }).base(), second.end());

              options->m_copy_tensor_quant_spec.emplace_back(first, second);

              searchStart = match.suffix().first;
          }
        }
        catch(std::string &e) {
            TIDL_GLOBAL_REPORT_ERROR("Could not parse malformed same_scale_tensors option");
            return false;
        }
    }

    if (!strcmp("object_detection:confidence_threshold", option_name.c_str())) {
        std::stringstream(option_value) >> options->odUserParams.confidence_threshold;
    }
    if (!strcmp("object_detection:nms_type", option_name.c_str())) {
        std::stringstream(option_value) >> options->odUserParams.nms_type;
    }
    if (!strcmp("object_detection:nms_threshold", option_name.c_str())) {
        std::stringstream(option_value) >> options->odUserParams.nms_threshold;
    }
    if (!strcmp("object_detection:top_k", option_name.c_str())) {
        std::stringstream(option_value) >> options->odUserParams.top_k;
    }
    if (!strcmp("object_detection:keep_top_k", option_name.c_str())) {
        std::stringstream(option_value) >> options->odUserParams.keep_top_k;
    }

    // below options will be used only if accuracy_level = 9
    if (!strcmp("advanced_options:activation_clipping", option_name.c_str())) {
        std::stringstream(option_value) >> options->m_activation_clipping;
    }
    if (!strcmp("advanced_options:weight_clipping", option_name.c_str())) {
        std::stringstream(option_value) >> options->m_weight_clipping;
    }
    if (!strcmp("advanced_options:bias_calibration", option_name.c_str())) {
        std::stringstream(option_value) >> options->m_bias_calibration;
    }
    if (!strcmp("advanced_options:channel_wise_quantization", option_name.c_str())) {
        std::stringstream(option_value) >> options->m_channel_wise_quantization;
    }
    if (!strcmp("advanced_options:bias_clipping", option_name.c_str())) {
        std::stringstream(option_value) >> options->m_bias_clipping;
    }

    if (!strcmp("max_dynamic_dims", option_name.c_str()))
    {
        /*
         * Format: "dimParam=maxVal,dimParam=maxVal,..."
         * Comma-separated dim_param=value pairs. dim_param strings are
         * model-level symbols shared across all inputs.
         * Example: "batch_size=4, seq_len=128"
         */
        auto trim = [](const std::string& s) -> std::string {
            size_t start = s.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return "";
            size_t end = s.find_last_not_of(" \t\r\n");
            return s.substr(start, end - start + 1);
        };

        options->m_max_dynamic_dims.clear();
        std::stringstream ss(option_value);
        std::string dim_token;
        while (std::getline(ss, dim_token, ','))
        {
            if (trim(dim_token).empty()) continue;
            size_t eq_pos = dim_token.find('=');
            if (eq_pos == std::string::npos)
            {
                TIDL_GLOBAL_REPORT_ERROR("max_dynamic_dims: expected "
                    "'dimParam=maxVal' but got '%s'", trim(dim_token).c_str());
                return false;
            }
            std::string dim_name = trim(dim_token.substr(0, eq_pos));
            std::string val_str  = trim(dim_token.substr(eq_pos + 1));
            int32_t max_val = 0;
            std::stringstream(val_str) >> max_val;
            if (max_val <= 0)
            {
                TIDL_GLOBAL_REPORT_ERROR("max_dynamic_dims: max value "
                    "must be a positive integer (got '%s' for dim '%s')",
                    val_str.c_str(), dim_name.c_str());
                return false;
            }
            options->m_max_dynamic_dims[dim_name] = max_val;
        }
    }

    return true;
}

/** Validated conditions on interface options if any are met */
bool TIDL_checkInterfaceOptions(TIDL_osrtOptions * options)
{
  if (options->m_tidl_tools_path.empty())
  {
    TIDL_GLOBAL_REPORT_ERROR("tidl_tools_path must be provided");
    return false;
  }

  if (options->m_artifacts_folder.empty())
  {
    TIDL_GLOBAL_REPORT_ERROR("artifacts_folder must be provided");
    return false;
  }

  if(mkdir(options->m_temp_folder.c_str(), 0755) != 0)
  {
    if(errno != EEXIST) // Should not throw error if already exists, equivalent to "mkdir -p"
    {
      TIDL_GLOBAL_REPORT_ERROR("mkdir tempDir failed");
      return false;
    }
  }

  if(((options->m_deny_list_layer_name.size() > 0) || (options->m_deny_list_layer_type.size() > 0) || (options->m_deny_list.size() > 0) || (options->m_deny_list_int.size() > 0))
       && (options->m_allow_list_layer_name.size() > 0))
  {
    TIDL_GLOBAL_REPORT_ERROR("Specify either allow list or deny list - cannot specify both simultaneously");
    return false;
  }

  return true;
}

/** Options derived from the basic options read from interface */
void TIDL_derivedInterfaceOptions(TIDL_osrtOptions * options)
{
  if(options->m_num_param_bits == 8 && options->m_inference_precision_mode == TIDL_InferencePrecisionModeFloatingPoint)
  {
    TIDL_GLOBAL_REPORT_WARNING("8 bit Float is not supported. Switching to 8 bit INT");
    options->m_inference_precision_mode = TIDL_InferencePrecisionModeFixedPoint;
  }
  else if(options->m_num_param_bits == 32 && options->m_inference_precision_mode == TIDL_InferencePrecisionModeFixedPoint)
  {
    TIDL_GLOBAL_REPORT_WARNING("32 bit INT is not supported. Switching to 32 bit Float");
    options->m_inference_precision_mode = TIDL_InferencePrecisionModeFloatingPoint;
  }
  if(options->m_quantization_scale_type == TIDL_QuantStyleAsymNP2)
  {
    if((options->m_tidl_calibration_flags & TIDL_CalibNoOutlier) == 0)
    {
      options->m_tidl_calibration_flags = options->m_bias_calibration * TIDL_CalibOptionBiasCalibration;
    }
    if((options->m_tidl_calibration_flags & TIDL_CalibNoOutlier) != 0) {
      if(options->m_enable_tfr_optimization == -1)
      {
        options->m_enable_tfr_optimization = 1; // enable the tfr optimization by default for AsymNP2 and NoOutlier mode
      }
    }
  }
  if(options->m_tidl_calibration_flags == 9) //user defined accuracy level
  {
    options->m_tidl_calibration_flags = options->m_activation_clipping * TIDL_CalibOptionActivationRange +     //default 1
                                        options->m_weight_clipping * TIDL_CalibOptionWeightRange +     //default 1
                                        options->m_bias_calibration * TIDL_CalibOptionBiasCalibration +    //default 1
                                        options->m_channel_wise_quantization * TIDL_CalibOptionPerChannelWeightQuantization + //default 0
                                        options->m_bias_clipping * TIDL_CalibOptionBiasRange;   //default 0
  }

  options->m_temp_folder = options->m_artifacts_folder + "/tempDir";
}

/** Print interface options for debug purposes */
void TIDL_printInterfaceOptions(TIDL_osrtOptions * options)
{
  if(options->m_debug_level)
  {
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_tools_path                                 = %s ", options->m_tidl_tools_path.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "artifacts_folder                                = %s ", options->m_artifacts_folder.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_tensor_bits                                = %d ", options->m_num_param_bits);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "debug_level                                     = %d ", options->m_debug_level);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "num_tidl_subgraphs                              = %d ", options->m_num_tidl_subgraphs);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "num_tidl_subgraph_max_node                      = %d ", options->m_num_tidl_subgraph_max_node);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "enable_rt_multi_subgraph_support                = %d ", options->m_enable_rt_multi_subgraph_support);
    std::string tidl_denylist="";
    for(int i = 0; i < options->m_deny_list.size(); i++)
    {
      tidl_denylist+=options->m_deny_list[i].c_str();
      if(i!=options->m_deny_list.size()-1)
        tidl_denylist+=", ";
    }
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_denylist                                   = %s", tidl_denylist.c_str());  //option for Deny_List
    tidl_denylist="";
    for(int i = 0; i < options->m_deny_list_layer_name.size(); i++)
    {
      tidl_denylist+=options->m_deny_list_layer_name[i].c_str();
      if(i!=options->m_deny_list_layer_name.size()-1)
        tidl_denylist+=", ";
    }
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_denylist_layer_name                        = %s", tidl_denylist.c_str());  //option for Deny_List_Name
    tidl_denylist="";
    for(int i = 0; i < options->m_deny_list_layer_type.size(); i++)
    {
      tidl_denylist+=options->m_deny_list_layer_type[i].c_str();
      if(i!=options->m_deny_list_layer_type.size()-1)
        tidl_denylist+=", ";
    }
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_denylist_layer_type                        = %s", tidl_denylist.c_str());  //option for Deny_List_Type
    tidl_denylist="";
    for(int i = 0; i < options->m_allow_list_layer_name.size(); i++)
    {
      tidl_denylist+=options->m_allow_list_layer_name[i].c_str();
      if(i!=options->m_allow_list_layer_name.size())
        tidl_denylist+=", ";
    }
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_allowlist_layer_name                       = %s", tidl_denylist.c_str());  //option for allow_List_Name
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "model_type                                      = %s ",options->m_model_type.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_calibration_accuracy_level                 = %d ", options->m_tidl_calibration_flags);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_calibration_options:num_frames_calibration = %d ", options->m_calibration_frames);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "tidl_calibration_options:bias_calibration_iterations = %d ", options->m_calibration_iterations);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "mixed_precision_factor = %f ",options->m_mixed_precision_factor);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "model_group_id = %d ",options->m_model_group_id);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "power_of_2_quantization                         = %d ", options->m_quantization_scale_type);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "ONNX QDQ Enabled                                = %d ", options->m_prequantized_model);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "enable_high_resolution_optimization             = %d ", options->m_high_resolution_optimization);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "pre_batchnorm_fold                              = %d ", options->m_pre_batchnorm_fold);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "add_data_convert_ops                            = %d ", options->m_add_data_convert_ops);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "output_feature_16bit_names_list                 = %s ", options->m_output_feature_16bit_names_list.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "mp_16bit_resolved_feature_names_list            = %s ", options->m_mp_16bit_resolved_feature_names_list.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "m_params_16bit_names_list                       = %s ", options->m_params_16bit_names_list.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "m_native_layer_names_list                       = %s ", options->m_native_layer_names_list.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "m_single_core_layers_names_list                 = %s ", options->m_single_core_layers_names_list.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "m_spatial_split_layers_names_list               = %s ", options->m_spatial_split_layers_names_list.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "m_channel_split_layers_names_list               = %s ", options->m_channel_split_layers_names_list.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Inference mode                                  = %d ", options->m_inference_mode);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "Number of cores                                 = %d ", options->m_num_cores);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "reserved_compile_constraints_flag               = %d ", options->m_compileConstraintsFlag);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "partial_init_during_compile                     = %d ", options->m_partial_init_during_compile);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "packetize_mode                                  = %d ", options->m_packetize_mode);
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "enable_tfr_optimization                         = %d ", options->m_enable_tfr_optimization);
    std::string ti_internal_reserved_1="";
    for(auto& x : options->m_reserved_ctrl_1)
    {
      ti_internal_reserved_1+=std::to_string(x);
      ti_internal_reserved_1+="  ";
    }
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "ti_internal_reserved_1                          = %s", ti_internal_reserved_1.c_str());
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL, "");
  }
}

/** Copy data from src file to dst file */
void TIDL_copyFile(std::string basename, std::string dstdir, std::string srcdir) {
    std::string src_fname = srcdir + "/" + basename;
    std::string dst_fname = dstdir + "/" + basename;
    int src_fd = open(src_fname.c_str(), O_RDONLY);
    int dst_fd = open(dst_fname.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ssize_t size = lseek(src_fd, 0, SEEK_END); lseek(src_fd, 0, SEEK_SET);
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(size);

    {
        auto done = 0;
        auto remaining = size;
        while(remaining) {
            int ret = read(src_fd, buffer.get() + done, remaining);
            done += ret;
            remaining -= ret;
        }
    }
    {
        auto done = 0;
        auto remaining = size;
        while(remaining) {
            int ret = write(dst_fd, buffer.get() + done, remaining);
            done += ret;
            remaining -= ret;
        }
    }

    close(src_fd);
    close(dst_fd);
}

/* Check for OD related options read from interface */
bool TIDL_OdOptionChecks(TIDL_osrtOptions * options, std::vector<int> valid_meta_arch_type, bool hasDetectionPostprocLayer)
{
  bool status = false;
  if(options->m_meta_layers_names_list.empty() && (options->m_meta_arch_type != TIDL_metaArchTfliteRtSsdDetectionLayer))
  {
    options->m_meta_arch_type = -1;
    TIDL_GLOBAL_REPORT_WARNING("'meta_layers_names_list' is not provided - running OD post processing in ARM mode");
  }
  else if(options->m_meta_arch_type != TIDL_metaArchTfliteRtSsdDetectionLayer)
  {
    //Check if meta_layers_names_list file exists
    std::ifstream ifile(options->m_meta_layers_names_list.c_str());
    if(! ifile.good())
    {
      TIDL_GLOBAL_REPORT_ERROR("File provided in meta_layers_names_list : %s does not exist", options->m_meta_layers_names_list.c_str());
      return false;
    }
    //Check if metaArchType is valid
    if(std::find(valid_meta_arch_type.begin(), valid_meta_arch_type.end(), options->m_meta_arch_type) == valid_meta_arch_type.end())
    {
      TIDL_GLOBAL_REPORT_ERROR("'meta_arch_type' = %d not valid. Please provide valid 'meta_arch_type'", options->m_meta_arch_type);
      return false;
    }
  }
  return true;
}

} //extern "C"

/** find quantization scale based on min/max range of data */
float TIDL_findMaxQuantizationScale(float min, float max, int32_t elementSizeInBits)
{
    float absRange = (fabs(max) > fabs(min)) ? fabs(max) : fabs(min);
    absRange = (float)ceil(log((double)absRange) / log((double)2));
    absRange = pow(2.0, (double)absRange);
    float quantPrec;
    if (absRange != 0)
    {
        quantPrec = ((1.0*(1 << (elementSizeInBits - 1))) / absRange);
    }
    else
    {
        quantPrec = 1;
    }

    return quantPrec;
}

/** Convert data to float and find range of data */
void TIDL_runtimesFindRange( float * src, int32_t n, int32_t d1, int32_t d2, int32_t c, int32_t h, int32_t w, float src_scale, int32_t zero, float &min, float &max)
{
    float curr;
    min = FLT_MAX;
    max = -FLT_MAX;
    int32_t i0, i1, i2, i3, i4, i5;
    for(i0 = 0; i0 < n; i0++)
    {
      for (i1 = 0; i1 < d1; i1++ )
      {
        for (i2 = 0; i2 < d2; i2++)
        {
          for (i3 = 0; i3 < c; i3++)
          {
              for (i4 = 0; i4 < h; i4++)
              {
                  for (i5 = 0; i5 < w; i5++)
                  {
                      curr = ((src[ i0*(d1 * d2 * c * h * w) + i1*(d2 * c * h * w) + i2*(c * h * w) + i3*(h * w) + i4*w + i5] - zero)*src_scale);
                      min = curr < min ? curr : min;
                      max = curr > max ? curr : max;
                  }
              }
          }
        }
      }
    }
}

/* Read parameters from device config file */
int32_t TIDL_readDeviceConfigParams(char * fileName)
{
  std::ifstream ifile(fileName);
  if(! ifile.good())
  {
    TIDL_GLOBAL_REPORT_ERROR("Device config file does not exist : %s", fileName);
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }
  std::string line;
  while(std::getline(ifile, line))
  {
    int equalPos = line.find("=");
    if(equalPos != -1) /* if = exists in line */
    {
      std::string tokenName = line.substr(0, equalPos);
      std::string tokenValue = line.substr(equalPos + 1, line.length());
      tokenName.erase(remove(tokenName.begin(), tokenName.end(), ' '), tokenName.end()); /* remove spaces */
      tokenValue.erase(remove(tokenValue.begin(), tokenValue.end(), ' '), tokenValue.end()); /* remove spaces */
      std::istringstream tokenValSs(tokenValue);
      int tokenVal;
      tokenValSs >> tokenVal;

      /* Parse import tool required device config parameters here */
      if(strcmp(tokenName.c_str(), "DEVICE_NAME") == 0)
      {
        gParams.deviceName = tokenVal;
      }
      if(strcmp(tokenName.c_str(), "NUMCORES") == 0)
      {
        gParams.numCores = tokenVal;
      }
    }
  }
  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/* Set default values for options for ONNX runtime */
void TIDL_setDefaultOptions(TIDL_osrtOptions * osrt_options)
{
  osrt_options->m_debug_level                                  = 0;
  osrt_options->m_num_param_bits                               = 8;
  osrt_options->m_inference_precision_mode                     = TIDL_InferencePrecisionModeFixedPoint;
  osrt_options->m_num_tidl_subgraphs                           = 16;
  osrt_options->m_num_tidl_subgraph_max_node                   = 0;
  osrt_options->m_enable_rt_multi_subgraph_support             = 0;
  osrt_options->m_calibration_frames                           = 20;
  osrt_options->m_calibration_iterations                       = 50;
  osrt_options->m_mixed_precision_factor                       = -1;
  osrt_options->m_model_group_id                               = 0;
  osrt_options->m_tidl_calibration_flags                       = 7;
  osrt_options->m_quantization_scale_type                      = TIDL_QuantStyleNP2Fixed;
  osrt_options->m_prequantized_model                           = 0;
  osrt_options->m_high_resolution_optimization                 = 0;
  osrt_options->m_compileConstraintsFlag                       = DEFAULT_COMPILE_CONSTRAINT_NC_FLAGS;
  osrt_options->m_pre_batchnorm_fold                           = 1;
  osrt_options->m_activation_clipping                          = 1;
  osrt_options->m_weight_clipping                              = 1;
  osrt_options->m_bias_calibration                             = 1;
  osrt_options->m_bias_calibration_factor                      = 0.05f;
  osrt_options->m_channel_wise_quantization                    = 0;
  osrt_options->m_bias_clipping                                = 0;
  osrt_options->m_add_data_convert_ops                         = 0;
  osrt_options->m_meta_arch_type                               = -1;
  osrt_options->m_inference_mode                               = TIDL_inferenceModeDefault;
  osrt_options->m_num_cores                                    = 1;
  osrt_options->m_quant_params_proto_path                      = "";
  osrt_options->m_artifacts_folder                             = "";
  osrt_options->m_temp_folder                                  = "";
  osrt_options->m_tidl_tools_path                              = "";
  osrt_options->m_model_type                                   = "";
  osrt_options->m_output_feature_16bit_names_list              = "";
  osrt_options->m_mp_16bit_resolved_feature_names_list         = "";
  osrt_options->m_params_16bit_names_list                      = "";
  osrt_options->m_native_layer_names_list                      = "";
  osrt_options->m_meta_layers_names_list                       = "";
  osrt_options->m_deny_list                                    = {};
  osrt_options->m_deny_list_layer_type                         = {};
  osrt_options->m_deny_list_layer_name                         = {};
  osrt_options->m_allow_list_layer_name                        = {};
  osrt_options->m_deny_list_int                                = {};
  osrt_options->m_reserved_ctrl_1                              = {};
  osrt_options->m_single_core_layers_names_list                = "";
  osrt_options->m_spatial_split_layers_names_list              = "";
  osrt_options->m_channel_split_layers_names_list              = "";
  osrt_options->m_network_name                                 = "";
  osrt_options->m_c7x_firmware_version                         = C7X_FIRMWARE_VERSION;
  osrt_options->m_log_file_name                                = "";
  osrt_options->m_batch_mode                                   = 0;
  osrt_options->m_partial_init_during_compile                  = 0;
  osrt_options->m_packetize_mode                               = 0;
  osrt_options->m_reserved_ctrl_2                              = 0;
  osrt_options->m_nc_temp_info_dir                             = "/tmp";
  osrt_options->m_temp_buffer_dir                              = "/dev/shm";
  osrt_options->m_enable_custom_layers                         = 0;
  osrt_options->m_use_16bit_for_topk                           = 1;
  osrt_options->m_enable_tfr_optimization                      = -1;
  osrt_options->m_graph_optimization_level                     = TIDL_OPTIMIZE_LEVEL_BASIC;
  osrt_options->m_enable_shape_folding                         = -1;
  osrt_options->m_optimize_batchnorm_higherdims                = 0;
  osrt_options->m_softmax_16bit_scale_update                   = 0;
  osrt_options->m_enable_concat_no_op                          = 1;
}


/* Set global import config parameters - parameters which are needed by optimizeNet to decide optimization of layers must be put here */
int32_t TIDL_runtimesGparamsInit(TIDL_osrtOptions * osrt_options, int32_t modelType)
{
  setDefaultParams(&gParams);
  gParams.numParamBits = osrt_options->m_num_param_bits;
  gParams.inferencePrecisionMode = osrt_options->m_inference_precision_mode;
  gParams.numFeatureBits = osrt_options->m_num_param_bits;
  gParams.foldPreBnConv2D = osrt_options->m_pre_batchnorm_fold;
  gParams.addDataConvertToNet = osrt_options->m_add_data_convert_ops;
  gParams.inFileFormat = 1;  // raw data
  gParams.modelType = modelType;   // tflite runtime - 5, ONNX runtime - 6
  gParams.quantizationStyle = osrt_options->m_quantization_scale_type;
  gParams.enableTFROptimization = osrt_options->m_enable_tfr_optimization;
  gParams.preQuantizedModel = osrt_options->m_prequantized_model;
  gParams.partialInitDuringCompile = osrt_options->m_partial_init_during_compile;
  gParams.packetizeMode = osrt_options->m_packetize_mode;
  gParams.inferenceMode = osrt_options->m_inference_mode;
  gParams.graphOptimizationLevel = osrt_options->m_graph_optimization_level;
  gParams.enableShapeFolding = osrt_options->m_enable_shape_folding;
  gParams.optimizeBatchNormHigherDims = osrt_options->m_optimize_batchnorm_higherdims;
  gParams.enableHighResOptimization = osrt_options->m_high_resolution_optimization;
  gParams.batchMode = osrt_options->m_batch_mode;
  gParams.enableCustomLayers = osrt_options->m_enable_custom_layers;
  gParams.use16BitForTopK = osrt_options->m_use_16bit_for_topk;
  gParams.enableConcatNoOp = osrt_options->m_enable_concat_no_op;
  gParams.biasCalibrationFactor = osrt_options->m_bias_calibration_factor;
  strcpy((char*)gParams.quantParamsPrototxtFile, osrt_options->m_quant_params_proto_path.c_str());
  strcpy((char*)gParams.networkName, osrt_options->m_network_name.c_str());
  strcpy((char*)gParams.c7xFirmwareVersion, osrt_options->m_c7x_firmware_version.c_str());
  strcpy((char*)gParams.logFileName, osrt_options->m_log_file_name.c_str());
  strcpy((char *)&gParams.outputFeature16bitNamesList[0], const_cast<char *>(osrt_options->m_output_feature_16bit_names_list.c_str()));
  strcpy((char *)&gParams.mp16bitResolvedFeatureNamesList[0], const_cast<char *>(osrt_options->m_mp_16bit_resolved_feature_names_list.c_str()));
  strcpy((char *)&gParams.params16bitNamesList[0], const_cast<char *>(osrt_options->m_params_16bit_names_list.c_str()));
  strcpy((char *)&gParams.nativeLayerNamesList[0], const_cast<char *>(osrt_options->m_native_layer_names_list.c_str()));
  strcpy((char *)&gParams.singleCoreLayersNames[0], const_cast<char *>(osrt_options->m_single_core_layers_names_list.c_str()));
  strcpy((char*)gParams.ncTempInfoDir, osrt_options->m_nc_temp_info_dir.c_str());
  strcpy((char *)&gParams.spatialSplitLayersNames[0], const_cast<char *>(osrt_options->m_spatial_split_layers_names_list.c_str()));
  strcpy((char *)&gParams.channelSplitLayersNames[0], const_cast<char *>(osrt_options->m_channel_split_layers_names_list.c_str()));

  std::string quantParamsPrototxtFile((char*)gParams.quantParamsPrototxtFile);

  // file is provided by user
  if(quantParamsPrototxtFile != "" )
  {
    FILE* fd = fopen((char*)gParams.quantParamsPrototxtFile, "r");

    // if file doesnt exist
    if(fd == NULL)
    {
      // user wants the quant params prototxt file to be written
      gParams.isQuantParamsToBeExported = 1;
    }
    // file exists, treat it as read only
    else
    {
      gParams.isQuantParamsToBeExported = 0;
      fclose(fd);
    }
  }
  // file isnt provided by user, quant params wont be exported
  else
  {
    gParams.isQuantParamsToBeExported = 0;
  }

  // populate net_inelement_type
  for (int i = 0; i < osrt_options->m_net_inelement_type.size(); i++)
  {
    gParams.NetInElementType[i] = osrt_options->m_net_inelement_type[i];
  }

  char * tidl_tools_path = const_cast<char *>(osrt_options->m_tidl_tools_path.c_str());
  if(tidl_tools_path)
  {
    strcpy((char*)gParams.tidlStatsTool, tidl_tools_path);
    strcat((char*)gParams.tidlStatsTool,"/PC_dsp_test_dl_algo.out");
    strcpy((char*)gParams.perfSimTool, tidl_tools_path);
    strcat((char*)gParams.perfSimTool,"/ti_cnnperfsim.out");
    strcpy((char*)gParams.graphVizTool, tidl_tools_path);
    strcat((char*)gParams.graphVizTool,"/tidl_graphVisualiser.out");
    strcpy((char*)gParams.perfSimConfig, tidl_tools_path);
    strcat((char*)gParams.perfSimConfig,"/device_config.cfg");
  }
  else
  {
    TIDL_GLOBAL_REPORT_ERROR("Please provide TIDL tools path, Aborting");
    return TIDL_IMPORT_DIAGNOSIS_RETURN_FAIL;
  }

  /* Device Name is manipulated as part of import, so need to read from device config file and populate in gParams */
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_readDeviceConfigParams((char*)gParams.perfSimConfig), "Reading device config params failed");

  if(osrt_options->m_num_cores != -1)
  {
    gParams.numCores = osrt_options->m_num_cores;
  }

  if(gParams.graphOptimizationLevel == TIDL_OPTIMIZE_LEVEL_EXTENDED)
  {
    if(gParams.enableShapeFolding == -1)
    {
      gParams.enableShapeFolding = 1;
    }
    // Other higher optimization flag can be added here in future
  }
  else
  {
    if(gParams.enableShapeFolding == -1)
    {
      gParams.enableShapeFolding = 0;
    }
    // Other higher optimization flag can be added here in future
  }

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** Add output data layer to subgraph */
int32_t TIDL_addOutputDataLayer(int32_t idx, const char * layerName)
{
  int32_t layerIndex = runtimes_import_state.layerIndex++;
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
  layer.layerType         = TIDL_DataLayer;
  layer.numInBufs         = 1;
  layer.numOutBufs        = -1;
  layer.outData[0].dataId = 0;
  strcpy((char*)layer.name, layerName);
  strcpy((char*)layer.inDataNames[0], layerName);
  strcpy((char*)layer.outDataNames[0], layerName);
  layer.outConsumerCnt[0] = 0;
  strcpy((char*)outDataNames[idx], layerName);
  return layerIndex;
}

/** Add input data layer to subgraph (6D)*/
int32_t TIDL_addInputDataLayer6D(int32_t idx, int32_t numBatches, int32_t numDim1, int32_t numDim2, int32_t inNumCh, int32_t inHeight, int32_t inWidth, const char * layerName)
{
  gParams.numBatches[idx] = numBatches;
  gParams.inDIM1[idx] = numDim1;
  gParams.inDIM2[idx] = numDim2;
  gParams.inNumChannels[idx] = inNumCh;
  gParams.inHeight[idx] = inHeight;
  gParams.inWidth[idx] = inWidth;
  int32_t layerIndex = runtimes_import_state.layerIndex++;
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
  layer.layerType         = TIDL_DataLayer;
  layer.numInBufs         = -1;
  layer.numOutBufs        = 1;
  layer.outData[0].dataId = runtimes_import_state.dataIndex++;
  layer.outData[0].elementType  = gParams.inElementType[idx];
  layer.outData[0].numDim       = TIDL_DIM_MAX;
  layer.outData[0].dimValues[TIDL_DIM_BATCH] = gParams.numBatches[idx];
  layer.outData[0].dimValues[TIDL_DIM_DIM1]=gParams.inDIM1[idx];
  layer.outData[0].dimValues[TIDL_DIM_DIM2]=gParams.inDIM2[idx];
  layer.outData[0].dimValues[TIDL_DIM_NUMCH] = gParams.inNumChannels[idx];
  layer.outData[0].dimValues[TIDL_DIM_HEIGHT] = gParams.inHeight[idx];
  layer.outData[0].dimValues[TIDL_DIM_WIDTH ] = gParams.inWidth[idx];
  strcpy((char *)layer.name,  layerName);
  strcpy((char *)layer.outDataNames[0],  layerName);
  strcpy((char*)inDataNames[idx], layerName);
  layer.outConsumerCnt[0] = 1;
  layer.outConsumerLinked[0] = 0;

  return layerIndex;
}

/** Add input data layer to subgraph (Deprecated) */
int32_t TIDL_addInputDataLayer(int32_t idx,  int32_t numBatches, int32_t inNumCh, int32_t inHeight, int32_t inWidth, const char * layerName)
{
  gParams.numBatches[idx] = numBatches;
  gParams.inNumChannels[idx] = inNumCh;
  gParams.inHeight[idx] = inHeight;
  gParams.inWidth[idx] = inWidth;
  int32_t layerIndex = runtimes_import_state.layerIndex++;
  sTIDL_LayerPC_t& layer = orgTIDLNetStructure.TIDLPCLayers[layerIndex];
  layer.layerType         = TIDL_DataLayer;
  layer.numInBufs         = -1;
  layer.numOutBufs        = 1;
  layer.outData[0].dataId = runtimes_import_state.dataIndex++;
  layer.outData[0].elementType  = gParams.inElementType[idx];
  layer.outData[0].numDim       = TIDL_DIM_MAX;
  layer.outData[0].dimValues[TIDL_DIM_BATCH] = gParams.numBatches[idx];
  layer.outData[0].dimValues[TIDL_DIM_DIM1]=gParams.inDIM1[idx];
  layer.outData[0].dimValues[TIDL_DIM_DIM2]=gParams.inDIM2[idx];
  layer.outData[0].dimValues[TIDL_DIM_NUMCH] = gParams.inNumChannels[idx];
  layer.outData[0].dimValues[TIDL_DIM_HEIGHT] = gParams.inHeight[idx];
  layer.outData[0].dimValues[TIDL_DIM_WIDTH ] = gParams.inWidth[idx];
  strcpy((char *)layer.name,  layerName);
  strcpy((char *)layer.outDataNames[0],  layerName);
  layer.outConsumerCnt[0] = 1;
  layer.outConsumerLinked[0] = 0;

  return layerIndex;
}

/** This runtimes function calls "tidl_optimizeNet" in the backend for layer level graph optimization */
int32_t TIDL_runtimesOptimizeNet(int32_t osrtDebugPrintLevel)
{
  TIDL_GLOBAL_REPORT_INFO(osrtDebugPrintLevel, "In TIDL_runtimesOptimizeNet: LayerIndex = %d, dataIndex = %d ", runtimes_import_state.layerIndex, runtimes_import_state.dataIndex);

  TIDL_IMPORT_CHECK_AND_RETURN(tidl_optimizeNet(orgTIDLNetStructure, runtimes_import_state.layerIndex, runtimes_import_state.dataIndex), "Network Optimization failed");

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** Since we call postProcessNet multiple times, some of the variables may be overwritten, so restore them to
 * original values in this function to be used again */
int32_t tidl_setParamsForPostProcessNet(int32_t tensor_bits)
{
  gParams.numParamBits = tensor_bits;
  gParams.numFeatureBits = tensor_bits;

  gParams.executeNetworkCompiler = 1;

  return 0;
}

/** This function calls "TIDL_import_backend" to run calibration, quantization and save the final network files */
int32_t TIDL_runtimesPostProcessNet(TIDL_osrtOptions * osrt_options, int32_t calibrationFrames, int32_t numParamBits, void * subGraphPtr, void * configParams, float * inQuantFactor, int32_t * zp, char* subGraphName)
{
  TIDL_GLOBAL_REPORT_INFO(osrt_options->osrtDebugPrintLevel, "In TIDL_runtimesPostProcessNet ");
  int32_t status = 0;
  if(subGraphPtr != NULL){
    sTIDL_OrgNetwork_t * netPtr = (sTIDL_OrgNetwork_t *)subGraphPtr;
    orgTIDLNetStructure = *netPtr;
    TIDL_allocAndCopyModelParams(&orgTIDLNetStructure, (sTIDL_OrgNetwork_t *)subGraphPtr, orgTIDLNetStructure.numLayers);
  }
  /* Restore gParams saved during init */
  if(configParams != NULL)
  {
    /* This backup is needed since gParams populated during init for a subgraph won't be preserved during invoke for the same subgraph
      Specifically required for parameters like inQuantFactor and inZeroPoint which will vary across subgraphs */
    tidl_import_config * configParamsCast = (tidl_import_config *)configParams;
    gParams = *configParamsCast;
  }

  memset(&tIDLNetStructure, 0, sizeof(sTIDL_Network_t));

  tidl_setParamsForPostProcessNet(numParamBits);
  gParams.numFramesBiasCalibration = calibrationFrames;
  gParams.calibrationOption = osrt_options->m_tidl_calibration_flags;
  gParams.biasCalibrationIterations = osrt_options->m_calibration_iterations;
  gParams.biasCalibrationFactor = osrt_options->m_bias_calibration_factor;
  gParams.mixedPrecisionFactor = osrt_options->m_mixed_precision_factor;
  gParams.modelGroupId = osrt_options->m_model_group_id;
  gParams.enableHighResOptimization = osrt_options->m_high_resolution_optimization;
  gParams.compileConstraintsFlag = osrt_options->m_compileConstraintsFlag;
  gParams.foldPreBnConv2D = osrt_options->m_pre_batchnorm_fold;
  strcpy((char *)&gParams.outputFeature16bitNamesList[0], const_cast<char *>(osrt_options->m_output_feature_16bit_names_list.c_str()));
  strcpy((char *)&gParams.mp16bitResolvedFeatureNamesList[0], const_cast<char *>(osrt_options->m_mp_16bit_resolved_feature_names_list.c_str()));
  strcpy((char *)&gParams.params16bitNamesList[0], const_cast<char *>(osrt_options->m_params_16bit_names_list.c_str()));
  strcpy((char *)&gParams.nativeLayerNamesList[0], const_cast<char *>(osrt_options->m_native_layer_names_list.c_str()));
  strcpy((char *)&gParams.singleCoreLayersNames[0], const_cast<char *>(osrt_options->m_single_core_layers_names_list.c_str()));
  strcpy((char *)&gParams.spatialSplitLayersNames[0], const_cast<char *>(osrt_options->m_spatial_split_layers_names_list.c_str()));
  strcpy((char *)&gParams.channelSplitLayersNames[0], const_cast<char *>(osrt_options->m_channel_split_layers_names_list.c_str()));

  for(int i = 0; i < osrt_options->m_reserved_ctrl_1.size(); i++)
  {
    gParams.ddrLayers[i] = osrt_options->m_reserved_ctrl_1[i];
  }

  gParams.copyTensorQuantSpec = osrt_options->m_copy_tensor_quant_spec;
  gParams.softmax16BitScaleUpdate = osrt_options->m_softmax_16bit_scale_update;
  gParams.use16BitForTopK = osrt_options->m_use_16bit_for_topk;
  gParams.graphOptimizationLevel = osrt_options->m_graph_optimization_level;
  gParams.enableShapeFolding = osrt_options->m_enable_shape_folding;
  gParams.optimizeBatchNormHigherDims = osrt_options->m_optimize_batchnorm_higherdims;

  if((gParams.addDataConvertToNet & ADD_DC_LAYER_AT_INPUT) == 0)
  {
    /** If data convert layer not present, need to get the input tensor scale separately using representative input frames
     * For data convert layer:
     * This scale is computed as part of calibration process similar to other layers, so no need to specifically populate here
     * It is populated in runtimes import init to model tensor scale (same as that in RT expected tensor scale - set as part of RT invoke) so ARM conversion is bypassed
     */
    for(int i = 0; i < TIDL_MAX_ALG_IN_BUFS; i++)
    {
      gParams.inQuantFactor[i] = inQuantFactor[i];
      if((gParams.quantizationStyle == TIDL_QuantStyleAsymNP2_TFL) && (zp != NULL)) // Zp required only for asym quant flow
      {
        gParams.inZeroPoint[i] = zp[i];
      }
      // Treating BF16 same as FP32 to update quant factor to 1 and zero point to 0, as no calibration/quantization is performed for BF16 
      if(numParamBits == 32 || (numParamBits == 16 && osrt_options->m_inference_precision_mode == TIDL_InferencePrecisionModeFloatingPoint))
      {
        gParams.inQuantFactor[i] = 1.0;
        gParams.inZeroPoint[i] = 0.0;
      }
    }
  }

  int netInputIdx = 0;
  for(int i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    if((orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_DataLayer) && (orgTIDLNetStructure.TIDLPCLayers[i].numInBufs == -1))
    {
      int32_t outDataId;
      int32_t currInputIdx = netInputIdx;
      orgTIDLNetStructure.TIDLPCLayers[i].outData[0].tensorScale = gParams.inQuantFactor[currInputIdx];
      orgTIDLNetStructure.TIDLPCLayers[i].outData[0].tensorZeroPoint = gParams.inZeroPoint[currInputIdx];
      outDataId = orgTIDLNetStructure.TIDLPCLayers[i].outData[0].dataId;
      netInputIdx++;
      /* Identify all the layers which are consumer of input data layer and update tensorScale and tensorZeroPoint
      in their inDataParams*/
      for (int k = 0; k < orgTIDLNetStructure.numLayers; k++ )
      {
        for ( int inBufIdx = 0;inBufIdx < orgTIDLNetStructure.TIDLPCLayers[k].numInBufs; inBufIdx++)
        {
          if ( orgTIDLNetStructure.TIDLPCLayers[k].inData[inBufIdx].dataId == outDataId)
          {
            orgTIDLNetStructure.TIDLPCLayers[k].inData[inBufIdx].tensorScale     = orgTIDLNetStructure.TIDLPCLayers[i].outData[0].tensorScale;
            orgTIDLNetStructure.TIDLPCLayers[k].inData[inBufIdx].tensorZeroPoint = orgTIDLNetStructure.TIDLPCLayers[i].outData[0].tensorZeroPoint;
          }
        }
      }
    }
  }
  //tidl_updateWeightElemSize(&orgTIDLNetStructure, &gParams, orgTIDLNetStructure.numLayers);
  for(int i = 0; i < orgTIDLNetStructure.numLayers; i++)
  {
    if(orgTIDLNetStructure.TIDLPCLayers[i].layerType != TIDL_DataLayer)
      orgTIDLNetStructure.TIDLPCLayers[i].weightsElementSizeInBits = gParams.numParamBits;
  }

  /* Relay import uses different names for these files, avoid re-initializating the names here */
  if(gParams.modelType != TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
  {
    const char * artifacts_folder = const_cast<char *>(osrt_options->m_temp_folder.c_str());
    snprintf((char *)inConfigFilename, FILE_NAME_SIZE, "%s/%s_tidl_io_", artifacts_folder, subGraphName);
    snprintf((char *)gParams.outputNetFile, FILE_NAME_SIZE, "%s/%s_tidl_net.bin", artifacts_folder, subGraphName);
    snprintf((char *)gParams.outputParamsFile, FILE_NAME_SIZE, "%s/%s_tidl_io_", artifacts_folder, subGraphName);
    sprintf((char *)gParams.inData, "%s/%s_calib_raw_data.bin", artifacts_folder, subGraphName);
    /* Set inDimsFile when any network input has dynamic dimensions. */
    memset((char *)gParams.inDimsFile, 0, FILE_NAME_SIZE);
    for (int i = 0; i < orgTIDLNetStructure.numLayers; i++)
    {
      if ((orgTIDLNetStructure.TIDLPCLayers[i].layerType == TIDL_DataLayer) &&
          (orgTIDLNetStructure.TIDLPCLayers[i].numInBufs == -1) &&
          (orgTIDLNetStructure.TIDLPCLayers[i].outData[0].dynDimMask != 0))
      {
        snprintf((char *)gParams.inDimsFile, FILE_NAME_SIZE, "%s/%s_calib_input_dims.txt", artifacts_folder, subGraphName);
        break;
      }
    }
  }
  TIDL_IMPORT_CHECK_AND_RETURN(TIDL_import_backend(orgTIDLNetStructure.numLayers, runtimes_import_state.dataIndex), "");

  return TIDL_IMPORT_DIAGNOSIS_RETURN_OK;
}

/** We need to call postProcessNet multiple times, so save the optimized subgraph, which can be copied
 * during postProcessNet and used multiple times */
void TIDL_saveTidlSubGraph(void ** subGraphPtr)
{
  *(sTIDL_OrgNetwork_t**)subGraphPtr = new sTIDL_OrgNetwork_t;
  if ( *subGraphPtr == NULL )
  {
    TIDL_GLOBAL_REPORT_ERROR("Unable to allocate memory to save the subgraph ");
    return;
  }
  **((sTIDL_OrgNetwork_t**)subGraphPtr) = orgTIDLNetStructure;
}

void TIDL_saveGparams(void ** configParams)
{
  *(tidl_import_config**)configParams = new tidl_import_config;
  if ( *configParams == NULL )
  {
    TIDL_GLOBAL_REPORT_ERROR("Unable to allocate memory to save the import config parameters ");
    return;
  }
  **((tidl_import_config**)configParams) = gParams;
}


/** Broadcast vector buffer to have a modified dimension */
void TIDL_broadcastFloatTensor(sBuffer_t &buf, int32_t bufSize)
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

/** Prints the parse table */
std::string TIDL_parseReasonFromDiagString(const std::string& diagString)
{
  // Use the exact same parsing mechanism as ONNXRT flow
  std::string reason = diagString;
  reason = reason.substr(reason.rfind(':') + 1);
  reason = reason.substr(0, reason.find("--"));
  reason.erase(reason.find_last_not_of(' ') + 1);
  reason.erase(0, reason.find_first_not_of(' '));
  return reason;
}

void TIDL_buildParseTableFromDiags(
  const DiagList_t& diagList,
  std::vector<std::vector<std::string>>& denylistData,
  std::function<std::pair<std::string, std::string>(int)> nodeInfoExtractor
)
{
  if (diagList.empty())
  {
    return;
  }

  // Track nodes already added to prevent duplicates
  std::set<std::string> addedNodes;

  for (int index = 0; index < diagList.size(); index++)
  {
    // Only process unsupported/error diagnostics
    if (diagList[index].getKind() != TIDL_ModelDiagnostic::DK_NotSupported &&
        diagList[index].getKind() != TIDL_ModelDiagnostic::DK_Error &&
        diagList[index].getKind() != TIDL_ModelDiagnostic::DK_NotVerified)
    {
      continue;
    }

    // Get node information from runtime-specific extractor
    auto nodeInfo = nodeInfoExtractor(index);
    std::string opType = nodeInfo.first;
    std::string nodeName = nodeInfo.second;

    // Skip if this node has already been added
    if (addedNodes.find(nodeName) != addedNodes.end())
    {
      continue;
    }

    // Parse reason from diagnostic string using common logic (same as ONNXRT)
    std::string diag = diagList[index].getString();
    std::string reason = TIDL_parseReasonFromDiagString(diag);

    // Use defaults if extraction failed
    if (opType.empty()) opType = "Unknown";
    if (nodeName.empty()) nodeName = "N/A";
    if (reason.empty()) reason = diag; // fallback to full diagnostic

    denylistData.push_back({opType, nodeName, reason});
    addedNodes.insert(nodeName);
  }
}

void TIDL_printParseTable(std::string supportedNodes, std::string offloadSubGraph, std::string unsupportedNodes, std::vector<std::vector<std::string>> denylistData)
{
  std::stringstream tableStream;
  std::string tableString;

  std::vector<std::string> header;
  std::vector<std::vector<std::string>> data;
  std::vector<TIDL_table_align_t> columnAlignment;

  if (gParams.modelType == TIDL_IMPORT_MODEL_FORMAT_TVM_RELAY)
  {
    header = {"Offload type","No. of Nodes","Number of Subgraphs"};
    data =  {
              {"TIDL",supportedNodes,offloadSubGraph},
              {"TVM generated code (C7x)",unsupportedNodes,"x"}
            };
  }
  else
  {
    header = {"Core","No. of Nodes","Number of Subgraphs"};
    data =  {
              {"C7x",supportedNodes,offloadSubGraph},
              {"CPU",unsupportedNodes,"x"}
            };
  }
  columnAlignment = {ALIGN_LEFT,ALIGN_RIGHT,ALIGN_RIGHT};
  TIDL_createTable(tableStream,header,data,1,columnAlignment);
  tableString = tableStream.str();
  TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"%s",tableString.c_str());

  tableStream.str("");
  header.clear();
  data.clear();
  if(!denylistData.empty())
  {
    header = {"Node","Node Name","Reason"};
    TIDL_createTable(tableStream,header,denylistData,1,{});
    tableString = tableStream.str();
    TIDL_GLOBAL_REPORT_INFO(TIDL_IMPORT_DIAGNOSIS_DEBUG_LEVEL,"%s",tableString.c_str());
  }
}

/* Check compatiblity minVersion <= version <= maxVersion */
bool TIDL_isVersionInRange(const char *version, const char *minVersion, const char *maxVersion)
{
  std::string _version = version;
  std::string _minVersion = minVersion;
  std::string _maxVersion = maxVersion;

  if(_version == _minVersion || _version == _maxVersion)
  {
    return true;
  }

  std::vector<std::string> versionSplit, minVersionSplit, maxVersionSplit;
  versionSplit = TIDL_splitString(_version,"_");
  minVersionSplit = TIDL_splitString(_minVersion,"_");
  maxVersionSplit = TIDL_splitString(_maxVersion,"_");

  if(versionSplit.size() != TIDL_VERSION_SPLITS || minVersionSplit.size() != TIDL_VERSION_SPLITS || maxVersionSplit.size() != TIDL_VERSION_SPLITS)
  {
    return false;
  }

  int32_t i;
  bool statusMin = false;
  bool statusMax = false;
  for(i = 0; i < TIDL_VERSION_SPLITS; i++)
  {
    if(versionSplit[i] > minVersionSplit[i])
    {
      statusMin = true;
      break;
    }
  }

  for(i = 0; i < TIDL_VERSION_SPLITS; i++)
  {
    if(versionSplit[i] < maxVersionSplit[i])
    {
      statusMax = true;
      break;
    }
  }
  return (statusMin & statusMax);
}
