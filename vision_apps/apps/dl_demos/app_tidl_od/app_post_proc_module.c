/*
 *
 * Copyright (c) 2024-2026 Texas Instruments Incorporated
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

#include "app_post_proc_module.h"
#include "tivx_dl_post_proc_host.h"

tivxDLPostProcParams *local_postproc_config = NULL;

vx_status app_init_post_proc(vx_context context, PostProcObj *postProcObj, char *objName, vx_int32 bufq_depth)
{
    vx_status status = VX_SUCCESS;

    local_postproc_config = tivxMemAlloc(sizeof(tivxDLPostProcParams), TIVX_MEM_EXTERNAL);
    if(local_postproc_config == NULL) {
        printf("ERROR: Unable to allocate memory for local_postproc_config\n");
        status = -1;
        return status;
    }

    local_postproc_config = &postProcObj->params;
    local_postproc_config->od_prms.ioBufDesc = &postProcObj->ioBufDesc; 

    local_postproc_config->task_type = TIVX_DL_POST_PROC_DETECTION_TASK_TYPE;
    #if defined(SOC_AM62A)
    local_postproc_config->od_prms.viz_th = 0.2;
    #else
    local_postproc_config->od_prms.viz_th = 0.6;
    #endif
    local_postproc_config->num_input_tensors = postProcObj->num_input_tensors;


    postProcObj->config = vxCreateUserDataObject(context, "PostProcConfig", sizeof(tivxDLPostProcParams), local_postproc_config);
    status = vxGetStatus((vx_reference)postProcObj->config);

    vx_image output_img = vxCreateImage(context, postProcObj->output_img_width, postProcObj->output_img_height, VX_DF_IMAGE_NV12);
    postProcObj->output_image_arr = vxCreateObjectArray(context, (vx_reference)output_img, NUM_CH);
    status = vxGetStatus((vx_reference)postProcObj->output_image_arr);
    vxReleaseImage(&output_img);
    return status;
}


vx_status app_update_post_proc(vx_context context, PostProcObj *postProcObj, vx_user_data_object config)
{
    vx_status status = VX_SUCCESS;

    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                    (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    memcpy(&postProcObj->ioBufDesc, ioBufDesc, sizeof(sTIDL_IOBufDesc_t));

    postProcObj->num_input_tensors = ioBufDesc->numInputBuf;
    postProcObj->num_output_tensors = ioBufDesc->numOutputBuf;

    postProcObj->kernel = tivxAddKernelDLPostProc(context, postProcObj->num_input_tensors);
    status = vxGetStatus((vx_reference)postProcObj->kernel);

    vxUnmapUserDataObject(config, map_id_config);
    return status;
}

void app_deinit_post_proc(vx_context context, PostProcObj *postProcObj, vx_int32 bufq_depth)
{
    vxReleaseUserDataObject(&postProcObj->config);
    vxReleaseObjectArray(&postProcObj->output_image_arr);
    tivxRemoveKernelDLPostProc(context);

}

void app_delete_post_proc(PostProcObj *postProcObj)
{
    if(postProcObj->node != NULL)
    {
        vxReleaseNode(&postProcObj->node);
    }
}

vx_status app_create_graph_post_proc(vx_graph graph, PostProcObj *postProcObj,vx_object_array out_tensor_arr, vx_object_array input_img_arr)
{
    vx_status status = VX_SUCCESS;
    uint32_t i = 0;
    vx_bool replicate[8];
    vx_image in_args   = (vx_image)vxGetObjectArrayItem((vx_object_array)input_img_arr, 0);
    vx_image result  = (vx_image)vxGetObjectArrayItem((vx_object_array)postProcObj->output_image_arr, 0);

    vx_tensor input_tensors[APP_MODULES_MAX_TENSORS];
    for(i = 0; i < postProcObj->num_input_tensors; i++)
    {
        input_tensors[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)out_tensor_arr, i);
    }

    postProcObj->node = tivxDLPostProcNode(graph,
                                           postProcObj->kernel,
                                           postProcObj->config,
                                           in_args,
                                           input_tensors,
                                           result);

    APP_ASSERT_VALID_REF(postProcObj->node);
    status = vxSetNodeTarget(postProcObj->node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    vxSetReferenceName((vx_reference)postProcObj->node, "post_proc_node");

    replicate[TIVX_DL_POST_PROC_CONFIG_IDX] = vx_false_e;
    replicate[TIVX_DL_POST_PROC_INPUT_IMAGE_IDX] = vx_true_e;
    replicate[TIVX_DL_POST_PROC_OUTPUT_IMAGE_IDX] = vx_true_e;
    for(i = 0; i < postProcObj->num_input_tensors; i++)
    {
        replicate[TIVX_DL_POST_PROC_INPUT_TENSOR_START_IDX + i] = vx_true_e;
    }
    vxReplicateNode(graph, postProcObj->node, replicate,
                    TIVX_DL_POST_PROC_BASE_PARAMS + postProcObj->num_input_tensors);

    vxReleaseImage(&in_args);
    for(i = 0; i < postProcObj->num_input_tensors; i++)
    {
        vxReleaseTensor(&input_tensors[i]);
    }
    vxReleaseImage(&result);
    return(status);
}
