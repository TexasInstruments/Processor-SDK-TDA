/*
 *
 * Copyright (c) 2020-2024 Texas Instruments Incorporated
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
#ifndef _APP_VISS_MODULE
#define _APP_VISS_MODULE

/**
 * \defgroup group_vision_apps_modules_viss VISS Node Module
 *
 * \brief This section contains module APIs for the TIOVX VISS node tivxVpacVissNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#include "app_modules.h"
#include "app_sensor_module.h"

/** \brief VISS Module Data Structure
 *
 * Contains the data objects required to use tivxVpacVissNode
 *
 */
typedef struct {
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


}VISSObj;

/** \brief VISS module init helper function
 *
 * This VISS init helper function will create all the data objects required to create the VISS
 * node
 *
 * \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
 * \param [out] vissObj    VISS Module object which gets populated with VISS node data objects
 * \param [in]  sensorObj  Sensor Module object used to initialize VISS data object parameters;
 *                         must be initialized prior to passing to this function
 * \param [in]  objName    String of the name of this object
 * \param [in]  num_cameras_enabled  Number of cameras enabled
 *
 */
vx_status app_init_viss(vx_context context, VISSObj *vissObj, SensorObj *sensorObj, char *objName, uint32_t num_cameras_enabled);

/** \brief VISS module deinit helper function
 *
 * This VISS deinit helper function will release all the data objects created during the \ref app_init_viss call
 *
 * \param [in,out] vissObj    VISS Module object which contains VISS node data objects which are released in this function
 *
 */
void app_deinit_viss(VISSObj *vissObj);

/** \brief VISS module delete helper function
 *
 * This VISS delete helper function will delete the VISS node and write node that is created during the \ref app_create_graph_viss call
 *
 * \param [in,out] vissObj   VISS Module object which contains VISS node objects which are released in this function
 *
 */
void app_delete_viss(VISSObj *vissObj);

/** \brief VISS module create helper function for RGB frames
 *
 * This VISS create helper function will create the node using all the data objects created during the \ref app_init_viss call.
 * Internally calls \ref app_create_graph_viss_write_output if en_out_viss_write is set
 *
 * \param [in]     graph           OpenVX graph that has been created using \ref vxCreateGraph and where the VISS node is created
 * \param [in,out] vissObj         VISS Module object which contains VISS node and write node which are created in this function
 * \param [in]     raw_image_arr   Raw image input object array to VISS node.  Must be created separately, typically passed from output of capture node
 * \param [in]     target          The name of the target (ASCII string) on which the node executes.
 *
 */
vx_status app_create_graph_viss(vx_graph graph, VISSObj *vissObj, vx_object_array raw_image_arr, const char *target);

/** \brief VISS module write output helper function
 *
 * This VISS create helper function will create the node for writing the VISS output
 *
 * \param [in]     graph    OpenVX graph
 * \param [in,out] vissObj  VISS Module object which contains write node which is created in this function
 *
 */
vx_status app_create_graph_viss_write_output(vx_graph graph, VISSObj *vissObj);

/** \brief VISS module write output helper function
 *
 * This VISS create helper function will create the node for writing the VISS output
 *
 * \param [in] vissObj       VISS Module object which contains the write node used in this function
 * \param [in] start_frame   Starting frame to write
 * \param [in] num_frames    Total number of frames to write
 * \param [in] num_skip      Number of VISS frames to skip writing
 *
 */
vx_status app_send_cmd_viss_write_node(VISSObj *vissObj, vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

/** \brief VISS module init helper function
 *
 * This VISS init helper function will create all the data objects required to create the
 * VISS node
 *
 * \param [in]  context    OpenVX context which must be created using \ref vxCreateContext
 * \param [out] vissObj    VISS Module object which gets populated with VISS node data objects
 * \param [in]  sensorObj  Sensor Module object used to initialize VISS data object
 *                         parameters; must be initialized prior to passing to this function
 * \param [in]  objName    String of the name of this object
 * \param [in]  num_cameras_enabled  Number of cameras enabled
 * \param [in]  bufq_depth           VISS output buffer queue depth
 *
 */
vx_status app_init_viss_queued(vx_context context,
                              VISSObj *vissObj,
                              SensorObj *sensorObj,
                              char *objName,
                              uint32_t num_cameras_enabled,
                              vx_uint32 bufq_depth);

/** \brief VISS module deinit helper function
 *
 * This VISS deinit helper function will release all the data objects created during the
 * \ref app_init_viss_queued call
 *
 * \param [in,out] vissObj    VISS Module object which contains VISS node data objects which
 *                            are released in this function
 *
 */
void app_deinit_viss_queued(VISSObj *vissObj);

/** \brief VISS module create helper function for RGB frames
 *
 * This VISS create helper function will create the node using all the data objects created
 * during the \ref app_init_viss_queued call.
 *
 * \param [in]     graph           OpenVX graph that has been created using
 *                                \ref vxCreateGraph and where the VISS node is created
 * \param [in,out] vissObj         VISS Module object which contains VISS node and write
 *                                nodes which are created in this function
 * \param [in]     raw_image_arr   Raw image input object array to VISS node. Must be
 *                                created separately, typically passed from capture node
 * \param [in]     target          The name of the target (ASCII string) on which the node
 *                                executes.
 *
 */
vx_status app_create_graph_viss_queued(vx_graph graph,
                                      VISSObj *vissObj,
                                      vx_object_array raw_image_arr,
                                      const char *target);

/* @} */

#endif
