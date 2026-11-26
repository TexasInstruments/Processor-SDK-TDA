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

#include "bev_post_proc_module.h"
#include <app_tidl_module.hpp>

/* ========================================================================== */
/*                    BEVPostProcObj Class Implementation                     */
/* ========================================================================== */

BEVPostProcObj::BEVPostProcObj()
{
    setDefaults();
}

void BEVPostProcObj::setDefaults()
{
    this->node = NULL;
    this->config = NULL;
    this->tidl_config = NULL;
    memset(&this->params, 0, sizeof(tivxDLBEVPostProcParams));
    memset(&this->ioBufDesc, 0, sizeof(sTIDL_IOBufDesc_t));
    this->num_input_tensors = 0;
    this->num_output_tensors = 0;
    this->output_img_width = 0;
    this->output_img_height = 0;
    this->lidar_points[0] = 0;
    this->lidar2cam_points[0] = 0;
    this->cam_2img_points[0] = 0;
    this->viz_th = 0.0f;
    this->num_top_results = 0;
    this->kernel = NULL;
    this->input_temp = NULL;
    this->output_image_arr = NULL;
    this->Lidar_2_img_tensor_arr = NULL;
    this->graph_parameter_index = 0;
    this->num_ch = 0;       /* 0 means use graph->num_ch */
    this->bufq_depth = 0;   /* 0 means use APP_MODULES_BUFFER_Q_DEPTH */

    for (int i = 0; i < APP_MAX_BUFQ_DEPTH; i++) {
        this->output_arr[i] = NULL;
        this->output_image[i] = NULL;
        this->input_lidar_arr[i] = NULL;
        this->input_lidar[i] = NULL;
        this->input_lidar2cam_arr[i] = NULL;
        this->input_lidar2cam[i] = NULL;
        this->input_cam2img_arr[i] = NULL;
        this->input_cam2img[i] = NULL;
    }

    this->objName = "";
    this->Lidar_2_img_file_path = "";

    this->is_camera_mode = false;
    this->input_tensor_arr = NULL;
    this->input_image_arr = NULL;
}

/* NOTE: The post_proc module implementation is complex and dual-mode.
 * For brevity, keeping original C functions and adding wrapper methods that call them.
 * Full refactoring would move all logic into class methods. */

tivxDLBEVPostProcParams *local_postproc_config;

/* ========================================================================== */
/*                    C-Style Functions (Legacy)                              */
/* ========================================================================== */

vx_status app_init_bev_post_proc(vx_context context, BEVPostProcObj *postProcObj, char *objName, vx_int32 bufq_depth, vx_int32 num_ch)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 q;

    local_postproc_config = (tivxDLBEVPostProcParams *)tivxMemAlloc(sizeof(tivxDLBEVPostProcParams), TIVX_MEM_EXTERNAL);
    if(local_postproc_config == NULL) {
        printf("ERROR: Unable to allocate memory for local_postproc_config\n");
        status = -1;
        return status;
    }

    local_postproc_config = &postProcObj->params;
    local_postproc_config->od_prms.ioBufDesc = &postProcObj->ioBufDesc;

    local_postproc_config->task_type = TIVX_DL_BEV_POST_PROC_DETECTION_TASK_TYPE;
    local_postproc_config->od_prms.viz_th = postProcObj->params.od_prms.viz_th;
    local_postproc_config->num_input_tensors = postProcObj->num_input_tensors;

    postProcObj->config = vxCreateUserDataObject(context, "PostProcConfig", sizeof(tivxDLBEVPostProcParams), local_postproc_config);
    status = vxGetStatus((vx_reference)postProcObj->config);

    vx_image output_img = vxCreateImage(context, postProcObj->output_img_width, postProcObj->output_img_height, VX_DF_IMAGE_NV12);
    
   postProcObj->output_arr[0]   = vxCreateObjectArray(context, (vx_reference)output_img, num_ch);

    vx_tensor input_lidar_tensor=vxCreateTensor(context, 1, postProcObj->lidar_points, VX_TYPE_INT32 , 0 );

    for(q = 0; q < bufq_depth; q++)
    {
        postProcObj->input_lidar_arr[q]   = vxCreateObjectArray(context, (vx_reference)input_lidar_tensor, NUM_CH);
        status = vxGetStatus((vx_reference)postProcObj->input_lidar_arr[q]);
        if(status != VX_SUCCESS)
        {
            printf("FAilure in creating input object Array index--%d \n",q);
        }
        postProcObj->input_lidar[q]  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)postProcObj->input_lidar_arr[q], 0);
    }
    vxReleaseImage(&output_img);
    vxReleaseTensor(&input_lidar_tensor);

    return status;
}

vx_status app_init_bev_cam_post_proc(vx_context context, BEVPostProcObj *postProcObj, char *objName, vx_int32 bufq_depth, vx_int32 num_ch)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 q;

    local_postproc_config = (tivxDLBEVPostProcParams *)tivxMemAlloc(sizeof(tivxDLBEVPostProcParams), TIVX_MEM_EXTERNAL);
    if(local_postproc_config == NULL) {
        printf("ERROR: Unable to allocate memory for local_postproc_config\n");
        status = -1;
        return status;
    }
    /*Reading 3*3 Matrix i.e 9 */
    postProcObj->cam_2img_points[0]=CAM_2_IMG_MATRIX_SIZE;

    local_postproc_config = &postProcObj->params;
    local_postproc_config->od_prms.ioBufDesc = &postProcObj->ioBufDesc;

    local_postproc_config->task_type = TIVX_DL_BEV_POST_PROC_DETECTION_TASK_TYPE;
    local_postproc_config->od_prms.viz_th = postProcObj->params.od_prms.viz_th;
    local_postproc_config->num_input_tensors = postProcObj->num_input_tensors;

    postProcObj->config = vxCreateUserDataObject(context, "PostProcConfig", sizeof(tivxDLBEVPostProcParams), local_postproc_config);
    status = vxGetStatus((vx_reference)postProcObj->config);

    vx_image output_img = vxCreateImage(context, postProcObj->output_img_width, postProcObj->output_img_height, VX_DF_IMAGE_NV12);
   postProcObj->output_arr[0]   = vxCreateObjectArray(context, (vx_reference)output_img, num_ch);

    vx_tensor input_lidar2cam_tensor=vxCreateTensor(context, 1, postProcObj->lidar2cam_points, VX_TYPE_INT32 , 0 );

    for(q = 0; q < bufq_depth; q++)
    {
        postProcObj->input_lidar2cam_arr[q]   = vxCreateObjectArray(context, (vx_reference)input_lidar2cam_tensor, num_ch);
        status = vxGetStatus((vx_reference)postProcObj->input_lidar2cam_arr[q]);
        if(status != VX_SUCCESS)
        {
            printf("FAilure in creating input object Array index--%d \n",q);
        }
        postProcObj->input_lidar2cam[q]  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)postProcObj->input_lidar2cam_arr[q], 0);
    }
    //intrinsic_parameter is a 3*3 Matrix.

    vx_tensor input_camera_intr_tensor=vxCreateTensor(context, 1, postProcObj->cam_2img_points, VX_TYPE_INT32 , 0 );
    for(q = 0; q < bufq_depth; q++)
    {
        postProcObj->input_cam2img_arr[q]   = vxCreateObjectArray(context, (vx_reference)input_camera_intr_tensor, num_ch);
        status = vxGetStatus((vx_reference)postProcObj->input_cam2img_arr[q]);
        if(status != VX_SUCCESS)
        {
            printf("FAilure in creating input object Array index--%d \n",q);
        }
        postProcObj->input_cam2img[q]  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)postProcObj->input_cam2img_arr[q], 0);
    }

    readCameraIntrinsicInput(postProcObj->input_cam2img_arr[0]);

    vxReleaseImage(&output_img);
    vxReleaseTensor(&input_lidar2cam_tensor);
    vxReleaseTensor(&input_camera_intr_tensor);

    return status;
}

vx_status app_update_bev_post_proc(vx_context context, BEVPostProcObj *postProcObj, vx_user_data_object config)
{
    vx_status status = VX_SUCCESS;

    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                    (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    memcpy(&postProcObj->ioBufDesc, ioBufDesc, sizeof(sTIDL_IOBufDesc_t));

    postProcObj->num_input_tensors = ioBufDesc->numOutputBuf;
    postProcObj->num_output_tensors = ioBufDesc->numOutputBuf;

    postProcObj->kernel = tivxAddKernelDLBEVPostProc(context, postProcObj->num_input_tensors);
    status = vxGetStatus((vx_reference)postProcObj->kernel);

    vxUnmapUserDataObject(config, map_id_config);

    /* CRITICAL FIX: Update the config with the correct num_input_tensors */
    if (status == VX_SUCCESS && postProcObj->config != NULL) {
        vx_map_id map_id_local;
        tivxDLBEVPostProcParams *local_config;

        vxMapUserDataObject(postProcObj->config, 0, sizeof(tivxDLBEVPostProcParams), &map_id_local,
                        (void **)&local_config, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

        local_config->num_input_tensors = postProcObj->num_input_tensors;
        local_config->od_prms.ioBufDesc = &postProcObj->ioBufDesc;
        local_config->od_prms.viz_th = postProcObj->params.od_prms.viz_th;

        vxUnmapUserDataObject(postProcObj->config, map_id_local);
    }

    return status;
}

vx_status app_update_bev_cam_post_proc(vx_context context, BEVPostProcObj *postProcObj, vx_user_data_object config)
{
    vx_status status = VX_SUCCESS;

    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                    (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    memcpy(&postProcObj->ioBufDesc, ioBufDesc, sizeof(sTIDL_IOBufDesc_t));

    postProcObj->num_input_tensors = ioBufDesc->numOutputBuf;
    postProcObj->num_output_tensors = ioBufDesc->numOutputBuf;

    postProcObj->kernel = tivxAddKernelDLBEVCAMPostProc(context, postProcObj->num_input_tensors);
    status = vxGetStatus((vx_reference)postProcObj->kernel);

    vxUnmapUserDataObject(config, map_id_config);

    /* CRITICAL FIX: Update the config with the correct num_input_tensors */
    if (status == VX_SUCCESS && postProcObj->config != NULL) {
        vx_map_id map_id_local;
        tivxDLBEVPostProcParams *local_config;

        vxMapUserDataObject(postProcObj->config, 0, sizeof(tivxDLBEVPostProcParams), &map_id_local,
                        (void **)&local_config, VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

        local_config->num_input_tensors = postProcObj->num_input_tensors;
        local_config->od_prms.ioBufDesc = &postProcObj->ioBufDesc;
        local_config->od_prms.viz_th = postProcObj->params.od_prms.viz_th;

        vxUnmapUserDataObject(postProcObj->config, map_id_local);
    }

    return status;
}

void app_deinit_bev_post_proc(vx_context context, BEVPostProcObj *postProcObj, vx_int32 bufq_depth)
{
    vx_uint32 q;
    for(q = 0; q < bufq_depth; q++)
    {
        if(postProcObj->input_lidar[q] != NULL)
            vxReleaseTensor(&postProcObj->input_lidar[q]);
        if(postProcObj->input_lidar_arr[q] != NULL)
            vxReleaseObjectArray(&postProcObj->input_lidar_arr[q]);
    }
    if(postProcObj->output_arr[0] != NULL)
        vxReleaseObjectArray(&postProcObj->output_arr[0]);
    if(postProcObj->config != NULL)
        vxReleaseUserDataObject(&postProcObj->config);
    tivxRemoveKernelDLBEVPostProc(context);

}

void app_deinit_bev_cam_post_proc(vx_context context, BEVPostProcObj *postProcObj, vx_int32 bufq_depth)
{
    vx_uint32 q;
    for(q = 0; q < bufq_depth; q++)
    {
        if(postProcObj->input_lidar2cam[q] != NULL)
            vxReleaseTensor(&postProcObj->input_lidar2cam[q]);
        if(postProcObj->input_lidar2cam_arr[q] != NULL)
            vxReleaseObjectArray(&postProcObj->input_lidar2cam_arr[q]);
        if(postProcObj->input_cam2img[q] != NULL)
            vxReleaseTensor(&postProcObj->input_cam2img[q]);
        if(postProcObj->input_cam2img_arr[q] != NULL)
            vxReleaseObjectArray(&postProcObj->input_cam2img_arr[q]);
    }
    if(postProcObj->output_arr[0] != NULL)
        vxReleaseObjectArray(&postProcObj->output_arr[0]);
    if(postProcObj->config != NULL)
        vxReleaseUserDataObject(&postProcObj->config);
    tivxRemoveKernelDLBEVCAMPostProc(context);

}

void app_delete_bev_post_proc(BEVPostProcObj *postProcObj)
{
    if(postProcObj->node != NULL)
    {
        vxReleaseNode(&postProcObj->node);
    }
}

vx_status app_create_graph_bev_post_proc(vx_graph graph, BEVPostProcObj *postProcObj,vx_object_array out_tensor_arr, vx_object_array input_img_arr)
{
    vx_status status = VX_SUCCESS;
    uint32_t i = 0;
    vx_bool replicate[8];
    vx_image in_args   = (vx_image)vxGetObjectArrayItem((vx_object_array)input_img_arr, 0);
    vx_image result  = (vx_image)vxGetObjectArrayItem((vx_object_array)postProcObj->output_arr[0], 0);
    vx_tensor Lidar_pointcloud_tensor = (vx_tensor)vxGetObjectArrayItem((vx_object_array)postProcObj->input_lidar_arr[0],0);


    vx_tensor input_tensors[APP_MODULES_MAX_TENSORS];
    for(i = 0; i < postProcObj->num_input_tensors; i++)
    {
        input_tensors[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)out_tensor_arr, i);
    }
    postProcObj->node = tivxDLBEVPostProcNode(graph,
                                           postProcObj->kernel,
                                           postProcObj->config,
                                           in_args,
                                           Lidar_pointcloud_tensor,
                                           input_tensors,
                                           result);

    APP_ASSERT_VALID_REF(postProcObj->node);
    status = vxSetNodeTarget(postProcObj->node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    vxSetReferenceName((vx_reference)postProcObj->node, "post_proc_node");
    
    replicate[0] = vx_false_e;
    replicate[1] = vx_true_e;
    replicate[2] = vx_true_e;
    replicate[3] = vx_false_e;
    replicate[4] = vx_false_e;

    vxReplicateNode(graph, postProcObj->node, replicate,
                     5);

    vxReleaseImage(&in_args);
    for(i = 0; i < postProcObj->num_input_tensors; i++)
    {
        vxReleaseTensor(&input_tensors[i]);
    }
    vxReleaseTensor(&Lidar_pointcloud_tensor);
    vxReleaseImage(&result);
    return(status);
}
vx_status app_create_graph_bev_cam_post_proc(vx_graph graph, BEVPostProcObj *postProcObj,vx_object_array out_tensor_arr, vx_object_array input_img_arr)
{
    vx_status status = VX_SUCCESS;
    uint32_t i = 0;
    vx_bool replicate[8];
    vx_image in_args   = (vx_image)vxGetObjectArrayItem((vx_object_array)input_img_arr, 0);
    vx_image result  = (vx_image)vxGetObjectArrayItem((vx_object_array)postProcObj->output_arr[0], 0);
    vx_tensor Lidar_to_Cam_tensor = (vx_tensor)vxGetObjectArrayItem((vx_object_array)postProcObj->input_lidar2cam_arr[0],0);
    vx_tensor Cam_to_Img_tensor = (vx_tensor)vxGetObjectArrayItem((vx_object_array)postProcObj->input_cam2img_arr[0],0);


    vx_tensor input_tensors[APP_MODULES_MAX_TENSORS];
    for(i = 0; i < postProcObj->num_input_tensors; i++)
    {
        input_tensors[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)out_tensor_arr, i);
    }
    postProcObj->node = tivxDLBEVCamPostProcNode(graph,
                                           postProcObj->kernel,
                                           postProcObj->config,
                                           in_args,
                                           Lidar_to_Cam_tensor,
                                           Cam_to_Img_tensor,
                                           input_tensors,
                                           result);

    APP_ASSERT_VALID_REF(postProcObj->node);
    status = vxSetNodeTarget(postProcObj->node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    vxSetReferenceName((vx_reference)postProcObj->node, "post_proc_node_cam");
    
    replicate[0] = vx_false_e;
    replicate[1] = vx_true_e;
    replicate[2] = vx_true_e;
    replicate[3] = vx_true_e;
    replicate[4] = vx_true_e;
    replicate[5] = vx_false_e;

    vxReplicateNode(graph, postProcObj->node, replicate,
                     6);

    vxReleaseImage(&in_args);
    for(i = 0; i < postProcObj->num_input_tensors; i++)
    {
        vxReleaseTensor(&input_tensors[i]);
    }
    vxReleaseTensor(&Lidar_to_Cam_tensor);
    vxReleaseTensor(&Cam_to_Img_tensor);
    vxReleaseImage(&result);
    return(status);
}


vx_status readCameraIntrinsicInput( vx_object_array tensor_arr)
{
    vx_status status;

    status = vxGetStatus((vx_reference)tensor_arr);
    vx_size start[1];
    vx_size strides[1];
    vx_size input_dims_num=1;
    vx_size input_dims[1];

    /*Focal length is hard coded for the given camera property. It can be exposed by cfg*/
    vx_float32 Camera_intrinsic[6][9]={{342.27112333333326, 0.0, 328.7053733333333,0.0, 342.7143133333333, 115.99709,0.0, 0.0, 1.0},
    {341.1655133333333, 0.0, 363.6190433333333, 0.0, 341.03267, 128.5554233333333, 0.0, 0.0, 1.0},
    {341.01492333333334, 0.0, 353.852583333333, 0.0, 341.01188, 99.88590333333332,0.0, 0.0, 1.0},
    {340.94239666666664, 0.0, 356.46578, 0.0, 341.02170666666666, 116.23542333333332,0.0, 0.0, 1.0},
    {338.26705, 0.0, 346.5209, 0.0, 338.22173, 119.74416666666667,0.0, 0.0, 1.0},
    {722.3381366666666, 0.0, 355.66673999999995, 0.0, 722.3366699999999, 107.17622666666666,0.0, 0.0, 1.0},
    };

    if(status == VX_SUCCESS)
    {
        vx_size tensor_len;
        vx_int32 i, j;
        vx_float32 *Cam2img_tensor_buffer = NULL;

        vxQueryObjectArray(tensor_arr, VX_OBJECT_ARRAY_NUMITEMS, &tensor_len, sizeof(vx_size));

        for (i = 0; i < tensor_len; i++)
        {
            vx_tensor   in_tensor;

            in_tensor = (vx_tensor)vxGetObjectArrayItem(tensor_arr, i);

            Cam2img_tensor_buffer = (vx_float32*)malloc(sizeof(vx_float32)*9);

            for( j =0; j<9;j++)
            {
                if(j<3)
                {
                    Camera_intrinsic[i][j] = Camera_intrinsic[i][j];
                }
                if(j>=3 && j<6)
                {
                    Camera_intrinsic[i][j] = Camera_intrinsic[i][j];
                }

            }
            memcpy(Cam2img_tensor_buffer, &Camera_intrinsic[i][0], (sizeof(vx_float32)*9));
            if (Cam2img_tensor_buffer == NULL) {
                // Handle memory allocation error
                printf("Memory allocation failed\n");
                vxReleaseTensor(&in_tensor);
                return VX_FAILURE;
            }
            strides[0] =  sizeof(vx_float32);
            input_dims[0] = 9;
            start[0] = 0;

            status = vxCopyTensorPatch(in_tensor, input_dims_num, start, input_dims, strides, Cam2img_tensor_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST);

            vxReleaseTensor(&in_tensor);
        }

        free(Cam2img_tensor_buffer);
    }

    return(status);
}

/* ========================================================================== */
/*                    Class Method Implementations                            */
/* ========================================================================== */

vx_status BEVPostProcObj::initModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_status status = VX_SUCCESS;
    vx_context context = graph->context;
    vx_int32 bufq_depth = (this->bufq_depth > 0) ? this->bufq_depth : APP_MODULES_BUFFER_Q_DEPTH;
    vx_int32 num_ch = (this->num_ch > 0) ? this->num_ch : 1;  /* Default to 1 if not set */

    this->objName = "bev_post_proc_module_" + std::to_string(module_index);

    /* TIDL config must be set by the app (from tidlObj.config) before this
     * module is added to the graph. */
    if (this->tidl_config == NULL) {
        printf("[BEV-POSTPROC] ERROR: tidl_config not set\n");
        return VX_FAILURE;
    }

    vx_user_data_object tidl_config = this->tidl_config;

    /* Call init function */
    if (this->is_camera_mode) {
        status = app_init_bev_cam_post_proc(context, this, (char*)this->objName.c_str(), bufq_depth, num_ch);
    } else {
        status = app_init_bev_post_proc(context, this, (char*)this->objName.c_str(), bufq_depth, num_ch);
    }

    if (status != VX_SUCCESS) {
        return status;
    }

    /* Call update function (merged from updateModule) */
    if (this->is_camera_mode) {
        status = app_update_bev_cam_post_proc(context, this, tidl_config);
    } else {
        status = app_update_bev_post_proc(context, this, tidl_config);
    }

    return status;
}

vx_status BEVPostProcObj::createNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_graph graph = appGraph->graph;

    if (this->input_tensor_arr == NULL || this->input_image_arr == NULL) {
        printf("[BEV-POSTPROC] ERROR: input arrays are NULL!\n");
        return VX_FAILURE;
    }

    if (this->is_camera_mode) {
        return app_create_graph_bev_cam_post_proc(graph, this, this->input_tensor_arr, this->input_image_arr);
    } else {
        return app_create_graph_bev_post_proc(graph, this, this->input_tensor_arr, this->input_image_arr);
    }
}

vx_status BEVPostProcObj::deleteNode(AppGraphObj *graph, vx_int32 module_index)
{
    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }
    return VX_SUCCESS;
}

vx_status BEVPostProcObj::deinitModule(AppGraphObj *graph, vx_int32 module_index)
{
    vx_context context = graph->context;
    vx_int32 bufq_depth = (this->bufq_depth > 0) ? this->bufq_depth : APP_MODULES_BUFFER_Q_DEPTH;

    if (this->is_camera_mode) {
        app_deinit_bev_cam_post_proc(context, this, bufq_depth);
    } else {
        app_deinit_bev_post_proc(context, this, bufq_depth);
    }
    return VX_SUCCESS;
}
