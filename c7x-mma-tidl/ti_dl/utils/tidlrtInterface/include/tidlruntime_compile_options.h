/*
*
* Copyright (c) 2025 Texas Instruments Incorporated
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

#ifndef TIDL_COMPILE_OPTIONS_H
#define TIDL_COMPILE_OPTIONS_H

#ifdef BUILD_COMPILE

#include "tidlruntime_utility.h"
#include "tidl_import_config.h"

#include <string>
#include <map>

class TIDLRT_CompileSession_Options
{
public:
    TIDLRT_CompileSession_Options();
    int32_t TIDLRT_CompileSession_Options_Parse(const std::map<std::string, std::string> &options);
    void TIDLRT_CompileSession_Options_Dump();

private:
    int32_t TIDLRT_CompileSession_Options_Translate(const std::map<std::string, std::string> &options);

public:
    tidl_import_config m_options;

    sOptionMapping m_optionMap[MAX_ITEMS_TO_PARSE] =
    {
		// { {TIDL_KEY,                    OSRT_KEY},                                              DATA,                                    TYPE,        SIZE,                                           CATEGORY,      TRANSLATION,  PARSE_TYPE,    PARSE_COUNT}
        {{"randParams",                   "rand_params"},                                          &m_options.randParams,                   INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"modelType",                    "model_type"},                                           &m_options.modelType,                    INT_32,      0,                                              SINGLE,        true,         -1,            0},
        {{"quantizationStyle",            "advanced_options:quantization_scale_type"},             &m_options.quantizationStyle,            INT_32,      0,                                              SINGLE,        true,         -1,            0},
        {{"calibrationOption",            "accuracy_level"},                                       &m_options.calibrationOption,            INT_32,      0,                                              SINGLE,        true,         -1,            0},
        {{"preQuantizedModel",            "advanced_options:prequantized_model"},                  &m_options.preQuantizedModel,            INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"enableBiasFusion",             "advanced_options:enable_bias_fusion"},                  &m_options.enableBiasFusion,             INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"softmax16BitScaleUpdate",      "ti_internal_reserved_2"},                               &m_options.softmax16BitScaleUpdate,      INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"activationRangeMethod",        "activation_range_method"},                              &m_options.activationRangeMethod,        INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"weightRangeMethod",            "weight_range_method"},                                  &m_options.weightRangeMethod,            INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"percentileActRangeShrink",     "percentile_act_range_shrink"},                          &m_options.percentileActRangeShrink,     FLOAT,       0,                                              SINGLE,        false,        -1,            0},
        {{"percentileWtRangeShrink",      "percentile_wt_range_shrink"},                           &m_options.percentileWtRangeShrink,      FLOAT,       0,                                              SINGLE,        false,        -1,            0},
        {{"biasCalibrationFactor",        "bias_calibration_factor"},                              &m_options.biasCalibrationFactor,        FLOAT,       0,                                              SINGLE,        false,        -1,            0},
        {{"biasCalibrationIterations",    "advanced_options:calibration_iterations"},              &m_options.biasCalibrationIterations,    INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"quantRoundAdd",                "quant_round_add"},                                      &m_options.quantRoundAdd,                INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"numParamBits",                 "tensor_bits"},                                          &m_options.numParamBits,                 INT_32,      0,                                              SINGLE,        true,         -1,            0},
        {{"numFeatureBits",               "tensor_bits"},                                          &m_options.numFeatureBits,               INT_32,      0,                                              SINGLE,        true,         -1,            0},
        {{"inFileFormat",                 "in_file_format"},                                       &m_options.inFileFormat,                 INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"postProcType",                 "post_proc_type"},                                       &m_options.postProcType,                 INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"postProcDataId",               "post_proc_data_id"},                                    &m_options.postProcDataId,               INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"numFrames",                    "advanced_options:calibration_frames"},                  &m_options.numFrames,                    INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"numFramesBiasCalibration",     "num_frames_bias_calibration"},                          &m_options.numFramesBiasCalibration,     INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"addDataConvertToNet",          "advanced_options:add_data_convert_ops"},                &m_options.addDataConvertToNet,          INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"foldBnInConv2D",               "fold_bn_in_conv2d"},                                    &m_options.foldBnInConv2D,               INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"foldEltWiseInConv2D",          "fold_elt_wise_in_conv2d"},                              &m_options.foldEltWiseInConv2D,          INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"foldMaxPoolInConv2D",          "fold_max_pool_in_conv2d"},                              &m_options.foldMaxPoolInConv2D,          INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"foldDepthToSpaceInConv2D",     "fold_depth_to_space_in_conv2d"},                        &m_options.foldDepthToSpaceInConv2D,     INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"metaArchType",                 "object_detection:meta_arch_type"},                      &m_options.metaArchType,                 INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"foldPreBnConv2D",              "advanced_options:pre_batchnorm_fold"},                  &m_options.foldPreBnConv2D,              INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"debugTraceLevel",              "debug_trace_level"},                                    &m_options.debugTraceLevel,              INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"writeTraceLevel",              "write_trace_level"},                                    &m_options.writeTraceLevel,              INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"compileConstraintsFlag",       "ti_internal_nc_flag"},                                  &m_options.compileConstraintsFlag,       INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"executeNetworkCompiler",       "execute_network_compiler"},                             &m_options.executeNetworkCompiler,       INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"quantRangeUpdateFactor",       "quant_range_update_factor"},                            &m_options.quantRangeUpdateFactor,       FLOAT,       0,                                              SINGLE,        false,        -1,            0},
        {{"quantRangeExpansionFactor",    "quant_range_expansion_factor"},                         &m_options.quantRangeExpansionFactor,    FLOAT,       0,                                              SINGLE,        false,        -1,            0},
        {{"inElementType",                "in_element_type"},                                      &m_options.inElementType,                INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"NetInElementType",             "advanced_options:net_inelement_type"},                  &m_options.NetInElementType,             INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"rawDataInElementType",         "raw_data_in_element_type"},                             &m_options.rawDataInElementType,         INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inZeroPoint",                  "in_zero_point"},                                        &m_options.inZeroPoint,                  INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inLayout",                     "in_layout"},                                            &m_options.inLayout,                     INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inTensorScale",                "in_tensor_scale"},                                      &m_options.inQuantFactor,                FLOAT,       TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"outElementType",               "out_element_type"},                                     &m_options.outElementType,               INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"outZeroPoint",                 "out_zero_point"},                                       &m_options.outZeroPoint,                 INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"outLayout",                    "out_layout"},                                           &m_options.outLayout,                    INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"outTensorScale",               "out_tensor_scale"},                                     &m_options.outTensorScale,               FLOAT,       TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"outElementSize",               "out_element_size"},                                     &m_options.outElementSize,               INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inQuantFactor",                "in_quant_factor"},                                      &m_options.inQuantFactor,                FLOAT,       TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inMean",                       "in_mean"},                                              &m_options.inMean,                       FLOAT,       TIDL_MAX_ALG_IN_BUFS*TIDL_IN_NUF_MAX_CH,        MULTI,         false,        -1,            0},
        {{"inScale",                      "in_scale"},                                             &m_options.inScale,                      FLOAT,       TIDL_MAX_ALG_IN_BUFS*TIDL_IN_NUF_MAX_CH,        MULTI,         false,        -1,            0},
        {{"inDataNorm",                   "in_data_norm"},                                         &m_options.inDataNorm,                   INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inDataPadInTIDL",              "in_data_pad_in_tidl"},                                  &m_options.inDataPadInTIDL,              INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inWidth",                      "in_width"},                                             &m_options.inWidth,                      INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inHeight",                     "in_height"},                                            &m_options.inHeight,                     INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inNumChannels",                "in_num_channels"},                                      &m_options.inNumChannels,                INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inDIM2",                       "in_dim2"},                                              &m_options.inDIM2,                       INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inDIM1",                       "in_dim1"},                                              &m_options.inDIM1,                       INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"numBatches",                   "num_batches"},                                          &m_options.numBatches,                   INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inferenceMode",                "advanced_options:inference_mode"},                      &m_options.inferenceMode,                INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"numCores",                     "advanced_options:num_cores"},                           &m_options.numCores,                     INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"inDataFormat",                 "in_data_format"},                                       &m_options.inDataFormat,                 INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inResizeType",                 "in_resize_type"},                                       &m_options.inResizeType,                 INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"resizeWidth",                  "resize_width"},                                         &m_options.resizeWidth,                  INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"resizeHeight",                 "resize_height"},                                        &m_options.resizeHeight,                 INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"reserved",                     "reserved"},                                             &m_options.reserved,                     INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"inData",                       "in_data"},                                              &m_options.inData,                       STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"inDataNamesList",              "in_data_names_list"},                                   &m_options.inDataNamesList,              STRING,      TIDL_MAX_ALG_IN_BUFS*FILE_NAME_SIZE,            SINGLE,        false,        -1,            0},
        {{"outDataNamesList",             "out_data_names_list"},                                  &m_options.outDataNamesList,             STRING,      TIDL_MAX_ALG_OUT_BUFS*FILE_NAME_SIZE,           SINGLE,        false,        -1,            0},
        {{"metaLayersNamesList",          "object_detection:meta_layers_names_list"},              &m_options.metaLayersNamesList,          STRING,      TIDL_MAX_ALG_IN_BUFS*FILE_NAME_SIZE,            SINGLE,        false,        -1,            0},
        {{"perfSimTool",                  "tidl_tools_path"},                                      &m_options.perfSimTool,                  STRING,      FILE_NAME_SIZE,                                 SINGLE,        true,         -1,            0},
        {{"graphVizTool",                 "tidl_tools_path"},                                      &m_options.graphVizTool,                 STRING,      FILE_NAME_SIZE,                                 SINGLE,        true,         -1,            0},
        {{"tidlStatsTool",                "tidl_tools_path"},                                      &m_options.tidlStatsTool,                STRING,      FILE_NAME_SIZE,                                 SINGLE,        true,         -1,            0},
        {{"perfSimConfig",                "tidl_tools_path"},                                      &m_options.perfSimConfig,                STRING,      FILE_NAME_SIZE,                                 SINGLE,        true,         -1,            0},
        {{"modelDumpTool",                "tidl_tools_path"},                                      &m_options.modelDumpTool,                STRING,      FILE_NAME_SIZE,                                 SINGLE,        true,         -1,            0},
        {{"msmcSizeKB",                   "msmc_size_kb"},                                         &m_options.msmcSizeKB,                   INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"deviceName",                   "device_name"},                                          &m_options.deviceName,                   INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"inputNetFile",                 "input_net_file"},                                       &m_options.inputNetFile,                 STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"inputParamsFile",              "input_params_file"},                                    &m_options.inputParamsFile,              STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"outputNetFile",                "artifacts_folder"},                                     &m_options.outputNetFile,                STRING,      FILE_NAME_SIZE,                                 SINGLE,        true,         -1,            0},
        {{"quantParamsPrototxtFile",      "advanced_options:quant_params_proto_path"},             &m_options.quantParamsPrototxtFile,      STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"outputNetQuantParamsPrototxtFile", "output_net_quant_params_prototxt_file"},            &m_options.outputNetQuantParamsPrototxtFile, STRING,  FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"fileNameGrpInfo",              "file_name_grp_info"},                                   &m_options.fileNameGrpInfo,              STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"outputParamsFile",             "artifacts_folder"},                                     &m_options.outputParamsFile,             STRING,      FILE_NAME_SIZE,                                 SINGLE,        true,         -1,            0},
        {{"configLine",                   "config_line"},                                          &m_options.configLine,                   STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"negativeTestUpdate",           "negative_test_update"},                                 &m_options.negativeTestUpdate,           STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"configParam",                  "config_param"},                                         &m_options.configParam,                  STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"executeQuantsTool",            "execute_quants_tool"},                                  &m_options.executeQuantsTool,            INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"enableHighResOptimization",    "advanced_options:high_resolution_optimization"},        &m_options.enableHighResOptimization,    INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"enableCustomLayers",           "advanced_options:enable_custom_layers"},                &m_options.enableCustomLayers,           INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"inYuvFormat",                  "in_yuv_format"},                                        &m_options.inYuvFormat,                  INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         false,        -1,            0},
        {{"ddrLayers",                    "ti_internal_reserved_1"},                               &m_options.ddrLayers,                    INT_32,      TIDL_MAX_ALG_IN_BUFS,                           MULTI,         true,         -1,            0},
        {{"singleCoreLayersNames",        "advanced_options:single_core_layers_names_list"},       &m_options.singleCoreLayersNames,        STRING,      TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE,             SINGLE,        false,        -1,            0},
        {{"spatialSplitLayersNames",      "advanced_options:m_spatial_split_layers_names_list"},   &m_options.spatialSplitLayersNames,      STRING,      TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE,             SINGLE,        false,        -1,            0},
        {{"channelSplitLayersNames",      "advanced_options:m_channel_split_layers_names_list"},   &m_options.channelSplitLayersNames,      STRING,      TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE,             SINGLE,        false,        -1,            0},
        {{"outputFeature16bitNamesList",  "advanced_options:output_feature_16bit_names_list"},     &m_options.outputFeature16bitNamesList,  STRING,      TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE,             SINGLE,        false,        -1,            0},
        {{"networkName",                  "advanced_options:network_name"},                        &m_options.networkName,                  STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"c7xFirmwareVersion",           "advanced_options:c7x_firmware_version"},                &m_options.c7xFirmwareVersion,           STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"params16bitNamesList",         "advanced_options:params_16bit_names_list"},             &m_options.params16bitNamesList,         STRING,      TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE,             SINGLE,        false,        -1,            0},
        {{"nativeLayerNamesList",         "advanced_options:native_layer_names_list"},             &m_options.nativeLayerNamesList,         STRING,      TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE,             SINGLE,        false,        -1,            0},
        {{"mixedPrecisionFactor",         "advanced_options:mixed_precision_factor"},              &m_options.mixedPrecisionFactor,         FLOAT,       0,                                              SINGLE,        false,        -1,            0},
        {{"modelGroupId",                 "advanced_options:model_group_id"},                      &m_options.modelGroupId,                 INT_8,       0,                                              SINGLE,        false,        -1,            0},
        {{"logFileName",                  "advanced_options:log_file_name"},                       &m_options.logFileName,                  STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"partialInitDuringCompile",     "advanced_options:partial_init_during_compile"},         &m_options.partialInitDuringCompile,     INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"batchMode",                    "advanced_options:batch_mode"},                          &m_options.batchMode,                    INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"graphOptimizationLevel",       "advanced_options:graph_optimization_level"},            &m_options.graphOptimizationLevel,       INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"enableShapeFolding",           "advanced_options:enable_shape_folding"},                &m_options.enableShapeFolding,           INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"optimizeBatchNormHigherDims",  "advanced_options:optimize_batchnorm_higherdims"},       &m_options.optimizeBatchNormHigherDims,  INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"packetizeMode",                "advanced_options:packetize_mode"},                      &m_options.packetizeMode,                INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"ncTempInfoDir",                "advanced_options:nc_temp_info_dir"},                    &m_options.ncTempInfoDir,                STRING,      FILE_NAME_SIZE,                                 SINGLE,        false,        -1,            0},
        {{"enableTFROptimization",        "advanced_options:enable_tfr_optimization"},             &m_options.enableTFROptimization,        INT_32,      0,                                              SINGLE,        false,        -1,            0},
        {{"mp16bitResolvedFeatureNamesList",   "advanced_options:mp_16bit_resolved_feature_names_list"},&m_options.mp16bitResolvedFeatureNamesList,  STRING,      TIDL_NUM_MAX_LAYERS*FILE_NAME_SIZE,             SINGLE,        false,        -1,            0},
        {{"enableConcatNoOp",             "advanced_options:enable_concat_no_op"},                 &m_options.enableConcatNoOp,             INT_32,      0,                                              SINGLE,        false,        -1,            0},
    };
};
#endif /* BUILD_COMPILE */

#endif  /* TIDL_COMPILE_OPTIONS_H */