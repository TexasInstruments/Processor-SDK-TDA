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
 * \defgroup group_vision_apps_modules_aewb AEWB Node Module
 *
 * \brief This section contains module APIs for the AEWB node tivxAewbNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#ifndef _APP_AEWB_MODULE_HPP_
#define _APP_AEWB_MODULE_HPP_

#include "app_modules_types.h"
#include <TI/j7_imaging_aewb.h>
#include "app_sensor_module.hpp"

/** \brief AEWB Module Data Structure
 *
 * Contains the data objects required to use tivxAewbNode
 *
 */
class AEWBObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Aewb; }

    /*! AEWB node object */
    vx_node node;

    /*! AEWB node config param object array */
    vx_object_array config_arr;

    /*! AEWB node params structure to initialize config object */
    tivx_aewb_config_t params;

    /*! AEWB DCC config user data object */
    vx_user_data_object dcc_config;

    /*! AEWB histogram object array */
    vx_object_array histogram_arr;

    /*! AEWB output object array */
    vx_object_array aewb_output_arr;

    /*! AEWB module object name */
    vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! AEWB output object array (one object array per buffer queue depth) */
    vx_object_array aewb_output_arr_q[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! AEWB output object for each buffer queue depth.
     *  These are items retrieved from aewb_output_arr_q[q].
     */
    vx_user_data_object aewb_output[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! Input H3A stats object for each buffer queue depth.
     *  These are items retrieved from the corresponding H3A stats arrays.
     */
    vx_user_data_object h3a_stats[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! AEWB node graph parameter index (input) */
    vx_int32 graph_parameter_in_index;

    /*! AEWB node graph parameter index (output) */
    vx_int32 graph_parameter_out_index;

    /*! AEWB module output buffer queue depth */
    vx_uint32 bufq_depth;

    /*! Starting channel for AEWB */
    uint32_t starting_channel;

    /*! Number of cameras enabled */
    uint32_t num_cameras_enabled;

    /*! \brief Constructor — sets safe defaults (no OpenVX objects created). */
    AEWBObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    AEWBObj(const AEWBObj&)            = delete;
    AEWBObj& operator=(const AEWBObj&) = delete;
    AEWBObj(AEWBObj&&)                 = delete;
    AEWBObj& operator=(AEWBObj&&)      = delete;

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the AEWB node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the AEWB node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Configure DCC parameters (helper). */
    vx_status configureDcc(vx_context context, SensorObj *sensorObj);

    /*! \brief Configure AEWB parameters (helper). */
    vx_status configureAewb(vx_context context, SensorObj *sensorObj);

    /*! \brief Create histogram (helper). */
    vx_status createHistogram(vx_context context, SensorObj *sensorObj);

    /*! \brief Create AEWB output queue (helper). */
    vx_status createAewbOutputQ(vx_context context, SensorObj *sensorObj);

    /*! \brief Create AEWB output (helper). */
    vx_status createAewbOutput(vx_context context, SensorObj *sensorObj);

    /*! \brief Initialize queued mode (helper). */
    vx_status initQueued(vx_context context, SensorObj *sensorObj);

    /*! \brief Deinitialize queued mode (helper). */
    void deinitQueued();

    /*! \brief Create graph for queued mode (helper). */
    vx_status createGraphQueued(vx_graph graph, vx_object_array h3a_stats_arr[], const char *target_string);

    /*! \brief Initialize AEWB (helper). */
    vx_status initAewb(vx_context context, SensorObj *sensorObj);

    /*! \brief Deinitialize AEWB (helper). */
    void deinitAewb();

    /*! \brief Delete AEWB node (helper). */
    void deleteAewb();

    /*! \brief Create AEWB graph (helper). */
    vx_status createGraphAewb(vx_graph graph, vx_object_array h3a_stats_arr);
};

/* @} */

#endif //_APP_AEWB_MODULE_HPP_
