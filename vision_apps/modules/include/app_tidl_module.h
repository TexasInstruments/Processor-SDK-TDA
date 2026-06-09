/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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
#include "itidl_ti.h"

/** \brief Maximum number of TIDL tensors
 *
 */
#define APP_MODULE_TIDL_MAX_TENSORS (8)

typedef struct {
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

  /*! TIDL number of output tensors */
  vx_uint32 num_output_tensors;

  /*! TIDL graph parameter index */
  vx_int32 graph_parameter_index;

  /*! Config structure file path */
  vx_char config_file_path[APP_MODULES_MAX_FILE_PATH_SIZE];

  /*! Network structure file path */
  vx_char network_file_path[APP_MODULES_MAX_FILE_PATH_SIZE];

  /*! Name of TIDL module */
  vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

  /*! Config structure checksum */
  vx_uint8 config_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /*! Network structure checksum */
  vx_uint8 network_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /*! TIDL output tensors (tensor x queue-depth) */
  vx_tensor output_tensor_q[APP_MODULES_MAX_TENSORS][APP_MODULES_MAX_BUFQ_DEPTH];
  vx_object_array output_tensor_arr_q[APP_MODULES_MAX_TENSORS][APP_MODULES_MAX_BUFQ_DEPTH];

  /*! TIDL graph parameter index (per tensor) */
  vx_int32 graph_parameter_idxs[APP_MODULES_MAX_TENSORS];

  /*! Buffer queue depth used by this instance (must be <= APP_MODULES_MAX_BUFQ_DEPTH) */
  vx_uint32 bufq_depth;

} TIDLObj;

/** \brief TIDL module init helper function
 *
 * This TIDL init helper function will create all the data objects required to create the TIDL
 * node
 *
 * \param [in]  context     OpenVX context which must be created using \ref vxCreateContext
 * \param [out] obj         TIDL Module object which gets populated with TIDL node data objects
 * \param [in]  objName     String of the name of this object
 * \param [in]  num_cameras Number of cameras used by TIDL
 *
 */
vx_status app_init_tidl(vx_context context, TIDLObj *obj, char *objName, vx_int32 num_cameras);

/** \brief TIDL module deinit helper function
 *
 * This TIDL deinit helper function will release all the data objects created during the \ref app_init_tidl call
 *
 * \param [in,out] obj    TIDL Module object which contains TIDL node data objects which are released in this function
 *
 */
void app_deinit_tidl(TIDLObj *obj);

/** \brief TIDL module delete helper function
 *
 * This TIDL delete helper function will delete the TIDL node that is created during the \ref app_create_graph_tidl call
 *
 * \param [in,out] obj   TIDL Module object which contains TIDL node objects which are released in this function
 *
 */
void app_delete_tidl(TIDLObj *obj);

/** \brief TIDL module create helper function
 *
 * This TIDL create helper function will create the node using all the data objects created during the \ref app_init_tidl call.
 *
 * \param [in]     context           OpenVX context which must be created using \ref vxCreateContext
 * \param [in]     graph             OpenVX graph that has been created using \ref vxCreateGraph and where the TIDL node is created
 * \param [in,out] tidlObj           TIDL Module object which contains TIDL node which is created in this function
 * \param [in,out] input_tensor_arr  Input tensors to TIDL node; must be created separately outside the TIDL module
 *
 */
vx_status app_create_graph_tidl(vx_context context, vx_graph graph, TIDLObj *tidlObj, vx_object_array input_tensor_arr[]);

/** \brief TIDL module write TIDL output helper function
 *
 * This TIDL helper function will write each element of the output_tensor_arr to file.
 *
 * \param [in]  file_name  Full path to file to write
 * \param [in]  tidlObj    TIDL Module object which contains the output_tensor_arr which is written to file in this function
 *
 */
vx_status writeTIDLOutput(char *file_name, TIDLObj *tidlObj);

/** \brief TIDL module init helper function
 *
 * This TIDL init helper function will create all the data objects required to create the TIDL
 * node.
 *
 * \param [in]  context      OpenVX context which must be created using \ref vxCreateContext
 * \param [out] tidlObj      TIDL Module object which gets populated with TIDL node data objects
 * \param [in]  objName      String of the name of this object
 * \param [in]  num_cameras  Number of cameras used by TIDL
 * \param [in]  bufq_depth   Buffer queue depth (must be <= APP_MODULES_MAX_BUFQ_DEPTH)
 *
 */
vx_status app_init_tidl_queued(vx_context context, TIDLObj *tidlObj, char *objName, vx_int32 num_cameras, vx_uint32 bufq_depth);

/** \brief TIDL module deinit helper function
 *
 * This TIDL deinit helper function will release all the data objects created during the
 * \ref app_init_tidl_queued call.
 *
 * \param [in,out] tidlObj      TIDL Module object which contains TIDL node data objects which are released in this function
 * \param [in]     bufq_depth   Buffer queue depth used for allocations in init
 *
 */
void app_deinit_tidl_queued(TIDLObj *tidlObj, vx_uint32 bufq_depth);

/** \brief TIDL module create helper function
 *
 * This TIDL create helper function will create the node using all the data objects created during the
 * \ref app_init_tidl_queued call.
 *
 * \param [in]     context           OpenVX context which must be created using \ref vxCreateContext
 * \param [in]     graph             OpenVX graph that has been created using \ref vxCreateGraph and where the TIDL node is created
 * \param [in,out] tidlObj           TIDL Module object which contains TIDL node which is created in this function
 * \param [in,out] input_tensor_arr  Input tensors to TIDL node; must be created separately outside the TIDL module
 *
 * \note input_tensor_arr size must match tidlObj->num_input_tensors (and any queueing/replication policy used by the pipeline).
 *
 */
vx_status app_create_graph_tidl_queued(vx_context context, vx_graph graph, TIDLObj *tidlObj, vx_object_array input_tensor_arr[]);


/* @} */

#endif
