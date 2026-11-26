/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
 *
 * All rights not granted herein.
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
 *       permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 *       licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 *       works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 *       and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
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
 */

/**
 * \defgroup group_vision_apps_modules_viss VISS Node Module
 *
 * \brief This section contains module APIs for the TIOVX VISS node tivxVpacVissNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#ifndef _APP_VISS_MODULE_HPP_
#define _APP_VISS_MODULE_HPP_

#include "app_modules_types.h"
#include "app_sensor_module.hpp"

/** \brief VISS Module Data Structure
 *
 * Contains the data objects required to use tivxVpacVissNode
 *
 */
class VISSObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Viss; }

    /*! VISS node object */
    vx_node node;

    /*! User data object for config parameter, used as node parameter of VISS node */
    vx_user_data_object config;

    /*! VISS node params structure to initialize config object */
    tivx_vpac_viss_params_t params;

    /*! User data object for DCC parameter, used as node parameter of VISS node */
    vx_user_data_object dcc_config;

    /*! Object array of output images, used as node parameter of VISS node */
    vx_object_array output_arr;

    /*! Object array of H3A objects, used as node parameter of VISS node */
    vx_object_array h3a_stats_arr;

    /*! Flag to indicate whether or not the intermediate output is written */
    vx_int32 en_out_viss_write;

    /*! Node used to write VISS output */
    vx_node img_write_node;

    /*! Node used to write H3A output */
    vx_node h3a_write_node;

    /*! File path used to write VISS node output */
    vx_array file_path;

    /*! File path prefix used to write VISS output */
    vx_array img_file_prefix;

    /*! File path prefix used to write H3A output */
    vx_array h3a_file_prefix;

    /*! User data object containing write cmd parameters */
    vx_user_data_object write_cmd;

    /*! Output file path for VISS node output */
    vx_char output_file_path[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! Name of VISS module */
    vx_char obj_name[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! Name of VISS module (duplicate for legacy compatibility - TODO: consolidate with obj_name) */
    vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! Number of cameras enabled */
    uint32_t num_cameras_enabled;

    /*! Target string for VISS node */
    vx_char target[TIVX_TARGET_MAX_NAME];

    /*! Object array of H3A stats objects, used as node parameter of VISS node.
     *  One object array per buffer queue depth.
     */
    vx_object_array h3a_stats_arr_q[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! H3A stats object for each buffer queue depth.
     *  These are items retrieved from h3a_stats_arr[q].
     */
    vx_user_data_object h3a_stats_q[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! VISS node graph parameter index */
    vx_int32 graph_parameter_index;

    /*! VISS module output buffer queue depth */
    vx_uint32 bufq_depth;

    /*! \brief Constructor — sets safe defaults (no OpenVX objects created). */
    VISSObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    VISSObj(const VISSObj&)            = delete;
    VISSObj& operator=(const VISSObj&) = delete;
    VISSObj(VISSObj&&)                 = delete;
    VISSObj& operator=(VISSObj&&)      = delete;

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the VISS node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the VISS node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Send write command to VISS write node */
    vx_status sendCmdWriteNode(vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Configure VISS parameters (helper). */
    vx_status configureParams(vx_context context, SensorObj *sensorObj);

    /*! \brief Configure VISS parameters for IR sensor (helper). */
    vx_status configureParamsIR(vx_context context, SensorObj *sensorObj);

    /*! \brief Configure DCC parameters (helper). */
    vx_status configureDccParams(vx_context context, SensorObj *sensorObj);

    /*! \brief Create VISS output objects (helper). */
    vx_status createOutputs(vx_context context, SensorObj *sensorObj);

    /*! \brief Create VISS output objects for IR sensor (helper). */
    vx_status createOutputsIR(vx_context context, SensorObj *sensorObj);

    /*! \brief Create VISS output objects for queued mode (helper). */
    vx_status createOutputsQueued(vx_context context, SensorObj *sensorObj);

    /*! \brief Create VISS output objects for IR sensor in queued mode (helper). */
    vx_status createOutputsIRQueued(vx_context context, SensorObj *sensorObj);

    /*! \brief Initialize VISS module in queued mode (helper). */
    vx_status initQueued(vx_context context, SensorObj *sensorObj, char *objName);

    /*! \brief Deinitialize VISS module in queued mode (helper). */
    void deinitQueued();

    /*! \brief Create VISS graph nodes in queued mode (helper). */
    vx_status createGraphQueued(vx_graph graph, vx_object_array raw_image_arr, const char *target);

    /*! \brief Create VISS write output node (helper). */
    vx_status createGraphWriteOutput(vx_graph graph);
};

/* @} */

#endif //_APP_VISS_MODULE_HPP_