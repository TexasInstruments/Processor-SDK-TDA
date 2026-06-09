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
#ifndef _APP_PRE_PROC_MODULE
#define _APP_PRE_PROC_MODULE

#include "app_common.h"
#include "app_modules.h"
#include "tiadalg_interface.h"
#include "itidl_ti.h"

/** \brief Pre-Processing Module Data Structure
 *
 *  The structure maintains:
 *  - Node handle
 *  - Configuration object
 *  - Input image pools (object arrays + images)
 *  - Output tensor pools (object arrays + tensors)
 *  - Graph parameter indices for queue-based scheduling
 */
typedef struct
{
    /*! Pre-processing node object */
    vx_node node;

    /*! Configuration user data object passed to node */
    vx_user_data_object config;

    /*! Local copy of configuration parameters */
    tivxDLPreProcArmv8Params params;

    /*! TIDL I/O buffer descriptor */
    sTIDL_IOBufDesc_t ioBufDesc;

    /*! Number of input tensors (as defined by network) */
    vx_uint32 num_input_tensors;

    /*! Number of output tensors (as defined by network) */
    vx_uint32 num_output_tensors;

    /*! Pipeline buffer queue depth */
    vx_uint32 bufq_depth;

    /*! Output tensor object arrays per queue slot. */
    vx_object_array output_tensor_arr[APP_MODULES_MAX_BUFQ_DEPTH][APP_MAX_TENSORS];

    /*! Cached tensor handles per output and queue slot. */
    vx_tensor output_tensor[APP_MAX_TENSORS][APP_MODULES_MAX_BUFQ_DEPTH];

    /*! Input image object arrays per queue slot. */
    vx_object_array input_img_arr[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! Cached image per queue slot (channel 0). */
    vx_image input_img[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! Graph parameter index for output tensor queue */
    vx_int32 graph_parameter_out_index;

    /*! Graph parameter index for input image queue */
    vx_int32 graph_parameter_in_index;

    /*! Name of pre-processing module */
    vx_char objName[APP_MAX_FILE_PATH];

} PreProcObj;

/** \brief Pre-processing module update helper function
 *
 * This pre-processing update helper function updates the PreProcObj based on the
 * provided TIDL config. It updates ioBufDesc, num_input_tensors, num_output_tensors and
 * creates output tensor object arrays (output_tensor_arr).
 *
 * \param [in]      context             OpenVX context which must be created using
 *                                     \ref vxCreateContext.
 * \param [in,out]  preProcObj          Pre-processing Module object which is updated
 *                                     in this function.
 * \param [in]      config              vx_user_data_object containing TIDL config
 *                                     (tivxTIDLJ7Params).
 * \param [in]      bufq_depth          Output buffer queue depth. Must be non-zero and
 *                                     less than or equal to APP_MODULES_MAX_BUFQ_DEPTH.
 * \param [in]      exemplar_input_arr  Exemplar input object array used to derive/create
 *                                     input tensor objects as needed by the module.
 *
 * \return VX_SUCCESS on success, VX_FAILURE (or other error status) on failure.
 */
vx_status app_update_pre_proc_queued(vx_context context,
                                    PreProcObj *preProcObj,
                                    vx_user_data_object config,
                                    vx_uint32 bufq_depth,
                                    vx_object_array exemplar_input_arr);

/** \brief Pre-processing module init helper function
 *
 * This pre-processing init helper function creates and initializes the config object
 * using the parameters in PreProcObj and sets the object name. Buffer queue depth is
 * stored in the object and later used for creating object arrays in
 * \ref app_update_pre_proc_queued.
 *
 * \param [in]      context     OpenVX context which must be created using
 *                              \ref vxCreateContext.
 * \param [out]     preProcObj  Pre-processing Module object which gets populated with
 *                              module data objects and initialized internal members.
 * \param [in]      objName     String specifying the name of this module instance.
 *
 * \return VX_SUCCESS on success, VX_FAILURE (or other error status) on failure.
 */
vx_status app_init_pre_proc_queued(vx_context context,
                                   PreProcObj *preProcObj,
                                   char *objName);

/** \brief Pre-processing module deinit helper function
 *
 * This pre-processing deinit helper function will release all the data objects created
 * during the \ref app_init_pre_proc_queued and \ref app_update_pre_proc_queued calls
 *
 * \param [in,out] obj   Pre-processing Module object which contains module data objects
 *                       which are released in this function
 *
 */
void app_deinit_pre_proc_queued(PreProcObj *obj);

/** \brief Pre-processing module delete helper function
 *
 * This pre-processing delete helper function will delete the node that is created during
 * the \ref app_create_graph_pre_proc_queued call
 *
 * \param [in,out] obj   Pre-processing Module object which contains node objects which are
 *                       released in this function
 *
 */
void app_delete_pre_proc_queued(PreProcObj *obj);

/** \brief Pre-processing module create helper function
 *
 * This pre-processing create helper function creates the pre-processing node using the
 * data objects created during the \ref app_init_pre_proc_queued and
 * \ref app_update_pre_proc_queued calls.
 *
 * \param [in]      graph          OpenVX graph that has been created using
 *                                 \ref vxCreateGraph and where the pre-processing node
 *                                 is created.
 * \param [in,out]  preProcObj     Pre-processing Module object which stores the created
 *                                 node handle.
 * \param [in]      input_img      Input image to the pre-processing node (queue slot 0
 *                                 leaf handle).
 * \param [in]      node_name      The name to be given to the pre-processing node.
 * \param [in]      target_string  Target string passed to \ref vxSetNodeTarget
 *                                 (e.g. TIVX_TARGET_MPU_0).
 *
 * \return VX_SUCCESS on success, VX_FAILURE (or other error status) on failure.
 */
vx_status app_create_graph_pre_proc_queued(vx_graph graph,
                                           PreProcObj *preProcObj,
                                           vx_image input_img,
                                           const char *node_name,
                                           const char *target_string);

#endif
