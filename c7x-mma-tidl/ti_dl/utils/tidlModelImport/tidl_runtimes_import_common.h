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

#ifndef TIDL_RUNTIMES_IMPORT_COMMON_H_
#define TIDL_RUNTIMES_IMPORT_COMMON_H_

#include <vector>
#include <string>
#include "tidl_runtimes_infer_common.h"
#include "itidl_ti.h"

#define DEFAULT_COMPILE_CONSTRAINT_NC_FLAGS (0x1 | 0x40 | 0x200 | 0x400)
#define TIDL_FLOW_CTRL_COMPILE (-1)


// Global data structure for importing
struct sTIDL_runtimesImportState_t {
  int32_t                 dataIndex;
  int32_t                 layerIndex;
};

extern "C"
{

void TIDL_nodeTraversal(std::vector<std::vector<int>> adjacencyList, int startIdx, std::vector<int> &odBackboneNodes, std::vector<bool> &visited);
std::vector<int> TIDL_callNodeTraversal(std::vector<std::vector<int>> adjacencyList, std::vector<int> postProcInputIds, int graphSize);
void TIDL_runGraphvizToolRuntimes(std::string tidlToolsPath, std::string artifactsFolderPath, int32_t debugLevel);
std::vector<std::string> TIDL_readMetaArchInfo(std::string filePath,int32_t graphOutputSize,int metaType,void *odParams=NULL);

/** This structure has all the parameters required for graph visualization through graphviz */
typedef struct{
  int32_t nodeModelIdx;
  std::string nodeName;
  std::vector<int> inputAdjNodes;
  std::vector<int> outputAdjNodes;
  std::string opType;
  std::string diagnosticInfo;
  std::string graphInputNames;
  std::string graphOutputNames;
} sTIDL_runtimesVisualisationInfo_t;

/** Parameters used for Object Detection **/
typedef struct
{
    float confidence_threshold;
    int nms_type;
    float nms_threshold;
    int top_k;
    int keep_top_k;
} sTIDL_odParameters_t;

/* All options provided through interface are part of this class */
class TIDL_osrtOptions
{
public:
  TIDL_osrtOptions() :
  m_debug_level(0),
  m_num_param_bits(8),
  m_num_tidl_subgraphs(16),
  m_num_tidl_subgraph_max_node(0),
  m_enable_rt_multi_subgraph_support(0),
  m_calibration_frames(20),
  m_calibration_iterations(50),
  m_mixed_precision_factor(-1),
  m_model_group_id(0),
  m_tidl_calibration_flags(7),
  m_quantization_scale_type(TIDL_QuantStyleNP2Fixed),
  m_prequantized_model(0),
  m_partial_init_during_compile(0),
  m_packetize_mode(0),
  m_high_resolution_optimization(0),
  m_compileConstraintsFlag(DEFAULT_COMPILE_CONSTRAINT_NC_FLAGS),
  m_pre_batchnorm_fold(1),
  m_activation_clipping(1),
  m_weight_clipping(1),
  m_bias_calibration(1),
  m_channel_wise_quantization(0),
  m_bias_clipping(0),
  m_add_data_convert_ops(0),
  m_enable_custom_layers(0),
  m_meta_arch_type(-1),
  m_inference_mode(TIDL_inferenceModeDefault),
  m_num_cores(1),
  m_quant_params_proto_path(""),
  m_c7x_firmware_version(C7X_FIRMWARE_VERSION),
  m_network_name(""),
  m_log_file_name(""),
  m_batch_mode(0),
  m_reserved_ctrl_2(0),
  m_nc_temp_info_dir("/tmp"),
  m_temp_buffer_dir("/dev/shm"),
  m_enable_tfr_optimization(-1),
  m_enable_shape_folding(0),
  m_optimize_batchnorm_higherdims(0),
  m_softmax_16bit_scale_update(0)
  {}

  int m_debug_level;
  int m_num_param_bits;
  int m_num_tidl_subgraphs;
  int m_num_tidl_subgraph_max_node;
  int m_enable_rt_multi_subgraph_support;
  int m_calibration_frames;
  int m_calibration_iterations;
  float m_mixed_precision_factor;
  int8_t m_model_group_id;
  int m_tidl_calibration_flags;
  int m_quantization_scale_type;
  int m_prequantized_model;
  int m_partial_init_during_compile;
  int m_packetize_mode;
  int m_high_resolution_optimization;
  int m_compileConstraintsFlag;
  int m_pre_batchnorm_fold;
  int m_add_data_convert_ops;
  int m_meta_arch_type;
  int m_inference_mode;
  int m_num_cores;
  int m_enable_custom_layers;
  int m_enable_shape_folding;
  int m_optimize_batchnorm_higherdims;

  int m_activation_clipping;
  int m_weight_clipping;
  int m_bias_calibration;
  int m_channel_wise_quantization;
  int m_bias_clipping;
  int m_batch_mode;
  int m_reserved_ctrl_2;

  std::string m_quant_params_proto_path;
  std::string m_c7x_firmware_version;
  std::string m_artifacts_folder;
  std::string m_temp_folder;
  std::string m_tidl_tools_path;
  std::vector<int> m_deny_list_int; //TODO: This should be removed when we take deny list as string for tflite runtime
  std::string m_model_type;
  std::vector<std::string> m_deny_list;             //option for deny_list
  std::vector<std::string> m_deny_list_layer_type;  //option for deny_list_type
  std::vector<std::string> m_deny_list_layer_name;  //option for deny_list_name
  std::vector<std::string> m_allow_list_layer_name;  //option for allow_list_name
  std::vector<std::string> m_custom_layers_names_list; //option for custom_layers_names_list.
  std::vector<int> m_net_inelement_type;
  std::string m_output_feature_16bit_names_list;
  std::string m_params_16bit_names_list;
  std::string m_meta_layers_names_list;
  std::string m_log_file_name;  // option to give log file name
  std::string m_nc_temp_info_dir;  // option to give path for dumping nc temp info files
  std::string m_temp_buffer_dir;  // option to give path to redirect temporary buffers

  std::vector<int> m_reserved_ctrl_1; /* Layers forced to DDR */
  std::string m_single_core_layers_names_list; /* Layers forced to single core in multi core inference */
  std::string m_spatial_split_layers_names_list; /* Layers forced to spatial split in multi core inference */
  std::string m_channel_split_layers_names_list; /* Layers forced to channel split in multi core inference */
  std::string m_network_name;

  sTIDL_odParameters_t odUserParams;

  int osrtDebugPrintLevel;
  int m_enable_tfr_optimization;
  int m_softmax_16bit_scale_update;
  int m_use_16bit_for_topk;

};


bool TIDL_checkIsEmpty(const char *path);

std::vector<std::string> TIDL_readCommaSeparatedString(char * deny_list);

bool TIDL_readInterfaceOptions(TIDL_osrtOptions * options, std::string option_name, std::string option_value, int model_type);
bool TIDL_checkInterfaceOptions(TIDL_osrtOptions * options);
void TIDL_derivedInterfaceOptions(TIDL_osrtOptions * options);
void TIDL_printInterfaceOptions(TIDL_osrtOptions * options);

void TIDL_copyFile(std::string basename, std::string dstdir, std::string srcdir);
bool TIDL_OdOptionChecks(TIDL_osrtOptions * options, std::vector<int> valid_meta_arch_type, bool hasDetectionPostprocLayer = false);
void TIDL_updateNamesList (char *tidlNamesList, int32_t index, char* onnxName);

} //extern "C"

void TIDL_setDefaultOptions(TIDL_osrtOptions * osrt_options);

float TIDL_findMaxQuantizationScale(float min, float max, int32_t elementSizeInBits);

void TIDL_runtimesFindRange( float * src, int32_t n, int32_t d1, int32_t d2, int32_t c, int32_t h, int32_t w, float src_scale, int32_t zero, float &min, float &max);

int32_t TIDL_runtimesGparamsInit(TIDL_osrtOptions * osrt_options, int32_t modelType);

int32_t TIDL_addOutputDataLayer(int32_t idx, const char * layerName);
int32_t TIDL_addInputDataLayer(int32_t idx, int32_t numBatches, int32_t inNumCh, int32_t inHeight, int32_t inWidth, const char * layerName);
int32_t TIDL_addInputDataLayer6D(int32_t idx, int32_t numBatches, int32_t numDim1, int32_t numDim2, int32_t inNumCh, int32_t inHeight, int32_t inWidth, const char * layerName);


int32_t TIDL_runtimesOptimizeNet(int32_t debugLevel);

int32_t TIDL_runtimesPostProcessNet(TIDL_osrtOptions * osrt_options, int32_t calibrationFrames, int32_t numParamBits, void * subGraphPtr, void * configParams, float * inQuantFactor, int32_t * zp, char* subGraphName);

void TIDL_saveTidlSubGraph(void ** subGraphPtr);

void TIDL_saveGparams(void ** configParams);

void TIDL_broadcastFloatTensor(sBuffer_t &buf, int32_t bufSize);

int32_t TIDL_readDeviceConfigParams(char * fileName);

void TIDL_printParseTable(std::string supportedNodes, std::string offloadSubGraph, std::string unsupportedNodes, std::vector<std::vector<std::string>> denylistData);

bool TIDL_isVersionInRange(const char *version, const char *minVersion, const char *maxVersion);

#endif /*TIDL_RUNTIMES_IMPORT_COMMON_H_ */