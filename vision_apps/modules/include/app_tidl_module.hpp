/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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
#ifndef _APP_TIDL_MODULE
#define _APP_TIDL_MODULE

/**
 * \defgroup group_vision_apps_modules_tidl TIDL Node Module
 *
 * \brief This section contains module APIs for the TIOVX TIDL node tivxTIDLNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#include "app_modules.h"
#include "app_modules_types.h"
#include "itidl_ti.h"

/** \brief Maximum number of TIDL tensors
 *
 */
#define APP_MODULE_TIDL_MAX_TENSORS (8)

class TIDLObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Tidl; }
  /*! Input tensor object arrays (set by app before create) */
  vx_object_array  input_tensor_arr[APP_MODULE_TIDL_MAX_TENSORS];

  /*! TIDL node object */
  vx_node    node;

  /*! TIDL kernel object */
  vx_kernel  kernel;

  /*! TIDL config user data object */
  vx_user_data_object  config;

  /*! TIDL network user data object */
  vx_user_data_object  network;

  /*! TIDL create params user data object */
  vx_user_data_object  createParams;

  /*! TIDL create params user data object */
  vx_object_array  in_args_arr;

  /*! TIDL object array of output args */
  vx_object_array  out_args_arr;

  /*! TIDL object array of trace data */
  vx_object_array trace_data_arr;

  /*! TIDL object array of output tensors */
  vx_object_array  output_tensor_arr[APP_MODULE_TIDL_MAX_TENSORS];

  /*! TIDL number of input tensors */
  vx_uint32 num_input_tensors;

  /*! TIDL CPU Core_ID */
  vx_uint32 core_id ;

  /*! TIDL inference mode (0=regular, 1=batch) - set by app before addNode */
  vx_uint32 inferenceMode;

  /*! TIDL core number (for batch processing) */
  vx_uint32 coreNum;

  /*! TIDL core start index (for batch processing) */
  vx_int32 coreStartIdx;

  /*! TIDL number of output tensors */
  vx_uint32 num_output_tensors;

  /*! TIDL graph parameter index */
  vx_int32 graph_parameter_index;

  /*! Buffer queue depth (0 = use default APP_MODULES_BUFFER_Q_DEPTH) */
  vx_int32 bufq_depth;

  /*! Config structure file path */
  std::string config_file_path;

  /*! Network structure file path */
  std::string network_file_path;

  /*! Name of TIDL module */
  vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

  /*! Number of cameras (set by app before init) */
  vx_int32 num_cameras;

  /*! Config structure checksum */
  vx_uint8 config_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /*! Network structure checksum */
  vx_uint8 network_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /*! Enable XYCoordinate input (for BEV networks) - set by app before addNode */
  vx_bool enable_xycord;

  /*! XYCoordinate tensor size - set by app before addNode */
  vx_size xycord_size[1];

  /*! XYCoordinate object arrays - one per buffer slot - created by module, loaded by app */
  vx_object_array input_xycord_arr[APP_MAX_BUFQ_DEPTH];

  /*! XYCoordinate tensors - extracted from object arrays for graph parameters */
  vx_tensor input_xycord[APP_MAX_BUFQ_DEPTH];

  /*! IO buffer descriptor — populated early by readConfigFile() */
  sTIDL_IOBufDesc_t ioBufDesc;

    /*! \brief Constructor — sets safe defaults. */
    TIDLObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    TIDLObj(const TIDLObj&)            = delete;
    TIDLObj& operator=(const TIDLObj&) = delete;
    TIDLObj(TIDLObj&&)                 = delete;
    TIDLObj& operator=(TIDLObj&&)      = delete;

    /*! \brief Read config binary file early (just file I/O).
     *  Populates ioBufDesc, num_input_tensors, num_output_tensors.
     */
    void readConfigFile();

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the TIDL node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the TIDL node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Get IO buffer descriptor. */
    sTIDL_IOBufDesc_t* getIOBufDesc();

    /*! \brief Write TIDL output to file */
    vx_status writeOutput(char *file_name);

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Read config binary file (helper). */
    vx_user_data_object readConfig(vx_context context, const char *config_file, vx_uint32 *num_input_tensors, vx_uint32 *num_output_tensors, vx_uint32 *inferenceMode, vx_uint32 *num_cores, vx_uint8 *check_sum);

    /*! \brief Read network binary file (helper). */
    vx_user_data_object readNetwork(vx_context context, const char *network_file, vx_uint8 *check_sum);

    /*! \brief Set TIDL create params (helper). */
    vx_status setCreateParams(vx_context context);

    /*! \brief Set TIDL input args (helper). */
    vx_status setInArgs(vx_context context, vx_user_data_object inArgs);

    /*! \brief Set TIDL output args (helper). */
    vx_status setOutArgs(vx_context context, vx_user_data_object outArgs);

    /*! \brief Create output tensors from config (helper). */
    void createOutputTensors(vx_context context, vx_user_data_object config, vx_tensor output_tensors[]);

    /*! \brief Update checksums in config (helper). */
    vx_status updateChecksums(vx_user_data_object config, vx_uint8 *config_checksum, vx_uint8 *network_checksum);
};

/* @} */

#endif
