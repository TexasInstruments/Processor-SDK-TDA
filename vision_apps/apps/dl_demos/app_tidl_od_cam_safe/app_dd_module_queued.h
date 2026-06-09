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
#ifndef _APP_DRAW_DETECTIONS_MODULE
#define _APP_DRAW_DETECTIONS_MODULE

#include "app_common.h"
#include "app_modules.h"
#include "itidl_ti.h"

/** \brief Draw Detections module object
 *
 * Holds OpenVX objects and configuration required to create, run and manage the
 * Draw Detections node, including per-buffer-queue resources and graph parameter
 * indices used for pipelining.
 *
 */
typedef struct
{
    /*! Draw detections node handle */
    vx_node node;
    /*! Configuration user data object for the node */
    vx_user_data_object config;
    /*! Draw detections parameters used to populate/initialize \ref config */
    tivxDrawBoxDetectionsParams params;
    /*! Name of this module instance (used for OpenVX object naming/debug) */
    vx_char objName[APP_MAX_FILE_PATH];
    /*! Buffer queue depth (must be <= APP_MODULES_MAX_BUFQ_DEPTH) */
    vx_uint32 bufq_depth;
    /*! Flag to create input pools (tensor/image) inside this module */
    vx_bool create_input_pools;
    /*! Input tensor object arrays (one per pipeline slot) */
    vx_object_array input_tensor_arr[APP_MODULES_MAX_BUFQ_DEPTH];
    /*! Input image object arrays (one per pipeline slot) */
    vx_object_array input_image_arr[APP_MODULES_MAX_BUFQ_DEPTH];
    /*! Input tensor handles (one per pipeline slot) */
    vx_tensor input_tensors[APP_MODULES_MAX_BUFQ_DEPTH];
    /*! Input image handles (one per pipeline slot) */
    vx_image  input_images[APP_MODULES_MAX_BUFQ_DEPTH];
    /*! Output image object arrays (one per pipeline slot) */
    vx_object_array output_image_arr[APP_MODULES_MAX_BUFQ_DEPTH];
    /*! Output image handles (one per pipeline slot) */
    vx_image        output_images[APP_MODULES_MAX_BUFQ_DEPTH];
    /*! Graph parameter index for output image queue */
    vx_int32 graph_parameter_out_image_index;
    /*! Graph parameter index for input tensor queue */
    vx_int32 graph_parameter_in_tensor_index;
    /*! Graph parameter index for input image queue */
    vx_int32 graph_parameter_in_image_index;

} DrawDetectionsObj;


/** \brief Draw Detections module init helper function
 *
 * This function creates all data objects required to instantiate the Draw Detections node.
 *
 * \param [in]  context               OpenVX context created using \ref vxCreateContext
 * \param [out] drawDetectionsObj     Draw Detections module object populated with node data objects
 * \param [in]  objName               Name of this module instance
 * \param [in]  num_cameras           Number of cameras (used for object array sizing if applicable)
 * \param [in]  bufq_depth            Buffer queue depth (must be <= APP_MODULES_MAX_BUFQ_DEPTH)
 * \param [in]  create_input_pools    Flag to create input pools (tensor/image) inside this module
 * \param [in]  input_tensor_exemplar Exemplar tensor used to create input tensor objects (if applicable)
 *
 */
vx_status app_init_dd_queued(vx_context context,
                            DrawDetectionsObj *drawDetectionsObj,
                            char *objName,
                            vx_int32 num_cameras,
                            vx_uint32 bufq_depth,
                            vx_bool create_input_pools,
                            vx_tensor input_tensor_exemplar);


/** \brief Draw Detections module update helper function
 *
 * Updates the configuration user data object used by the Draw Detections node.
 *
 * \param [in,out] drawDetectionsObj  Draw Detections module object
 * \param [in]     config             New configuration user data object
 *
 */
vx_status app_update_dd_queued(DrawDetectionsObj *drawDetectionsObj, vx_user_data_object config);


/** \brief Draw Detections module deinit helper function
 *
 * Releases all data objects created during \ref app_init_dd_queued.
 *
 * \param [in,out] drawDetectionsObj  Draw Detections module object
 *
 */
void app_deinit_dd_queued(DrawDetectionsObj *drawDetectionsObj);


/** \brief Draw Detections module delete helper function
 *
 * Deletes the Draw Detections node created during \ref app_create_graph_dd_queued.
 *
 * \param [in,out] drawDetectionsObj  Draw Detections module object
 *
 */
void app_delete_dd_queued(DrawDetectionsObj *drawDetectionsObj);


/** \brief Draw Detections module create helper function
 *
 * Creates the Draw Detections node in the given graph using objects created during
 * \ref app_init_dd_queued.
 *
 * \param [in]     graph            OpenVX graph where the node is created
 * \param [in,out] drawDetectionsObj Draw Detections module object containing node data
 * \param [in]     input_tensor_arr Input tensor object array (detection results)
 * \param [in]     input_image_arr  Input image object array
 * \param [in]     target_string    Target on which the node will run (e.g. DSP/CPU)
 *
 */
vx_status app_create_graph_dd_queued(vx_graph graph,
                                    DrawDetectionsObj *drawDetectionsObj,
                                    vx_object_array input_tensor_arr,
                                    vx_object_array input_image_arr,
                                    const char *target_string);

#endif
