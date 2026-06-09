/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#ifndef _APP_AEWB_MODULE
#define _APP_AEWB_MODULE

/**
 * \defgroup group_vision_apps_modules_aewb AEWB Node Module
 *
 * \brief This section contains module APIs for the AEWB node tivxAewbNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#include <TI/j7_imaging_aewb.h>

#include "app_modules.h"
#include "app_sensor_module.h"

/** \brief AEWB Module Data Structure
 *
 * Contains the data objects required to use tivxAewbNode
 *
 */
typedef struct {
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
}AEWBObj;

/** \brief AEWB module init helper function
 *
 * This AEWB init helper function will create all the data objects required to create the AEWB
 * node
 *
 * \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
 * \param [out] aewbObj    AEWB Module object which gets populated with AEWB node data objects
 * \param [in]  sensorObj  Sensor Module object used to initialize AEWB data object parameters;
 *                         must be initialized prior to passing to this function
 * \param [in]  objName    String of the name of this object
 * \param [in]  starting_channel     First channel enabled
 * \param [in]  num_cameras_enabled  Number of cameras enabled
 *
 */
vx_status app_init_aewb(vx_context context, AEWBObj *aewbObj, SensorObj *sensorObj, char *objName, uint32_t starting_channel, uint32_t num_cameras_enabled);

/** \brief AEWB module deinit helper function
 *
 * This AEWB deinit helper function will release all the data objects created during the \ref app_init_aewb call
 *
 * \param [in,out] aewbObj    AEWB Module object which contains AEWB node data objects which are released in this function
 *
 */
void app_deinit_aewb(AEWBObj *aewbObj);

/** \brief AEWB module delete helper function
 *
 * This AEWB delete helper function will delete the AEWB node that is created during the \ref app_create_graph_aewb call
 *
 * \param [in,out] aewbObj   AEWB Module object which contains AEWB node objects which are released in this function
 *
 */
void app_delete_aewb(AEWBObj *aewbObj);


/** \brief AEWB module create helper function
 *
 * This AEWB create helper function will create the node using the data objects created during the \ref app_init_aewb call.
 *
 * \param [in]     graph          OpenVX graph that has been created using \ref vxCreateGraph and where the AEWB node is created
 * \param [in,out] aewbObj        AEWB Module object which contains AEWB node which is created in this function
 * \param [in]     h3a_stats_arr  Object array of H3A stats; must already have been created prior to passing to this function
 *
 */
vx_status app_create_graph_aewb(vx_graph graph, AEWBObj *aewbObj, vx_object_array h3a_stats_arr);


/** \brief AEWB module init helper function
 *
 * This AEWB init helper function will create all the data objects required to create the
 * AEWB node
 *
 * \param [in]  context              OpenVX context which must be created using
 *                                  \ref vxCreateContext
 * \param [out] aewbObj              AEWB Module object which gets populated with AEWB node
 *                                  data objects
 * \param [in]  sensorObj            Sensor Module object used to initialize AEWB data
 *                                  object parameters; must be initialized prior to
 *                                  passing to this function
 * \param [in]  starting_channel     First channel enabled
 * \param [in]  num_cameras_enabled  Number of cameras enabled
 * \param [in]  bufq_depth           AEWB output buffer queue depth
 *
 */
vx_status app_init_aewb_queued(vx_context context,
                              AEWBObj *aewbObj,
                              SensorObj *sensorObj,
                              uint32_t starting_channel,
                              uint32_t num_cameras_enabled,
                              vx_uint32 bufq_depth);


/** \brief AEWB module deinit helper function
 *
 * This AEWB deinit helper function will release all the data objects created during the
 * \ref app_init_aewb_queued call
 *
 * \param [in,out] aewbObj    AEWB Module object which contains AEWB node data objects
 *                            which are released in this function
 *
 */
void app_deinit_aewb_queued(AEWBObj *aewbObj);


/** \brief AEWB module create helper function
 *
 * This AEWB create helper function will create the node using the data objects created
 * during the \ref app_init_aewb_queued call.
 *
 * \param [in]     graph          OpenVX graph that has been created using
 *                               \ref vxCreateGraph and where the AEWB node is created
 * \param [in,out] aewbObj        AEWB Module object which contains AEWB node which is
 *                               created in this function
 * \param [in]     h3a_stats_arr  Object array of H3A stats; must already have been created
 *                               prior to passing to this function
 * \param [in]     target_string  The target name ASCII string
 *
 */
vx_status app_create_graph_aewb_queued(vx_graph graph,
                                      AEWBObj *aewbObj,
                                      vx_object_array h3a_stats_arr[],
                                      const char *target_string);

/* @} */

#endif
