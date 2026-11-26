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

#include "app_post_proc_module.hpp"
#include <string.h>   /* memset */

static char     s_img_labels[TIVX_DL_POST_PROC_MAX_NUM_CLASSNAMES][TIVX_DL_POST_PROC_MAX_SIZE_CLASSNAME] = {
    "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
    "fire hydrant", "street sign", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep",
    "cow", "elephant", "bear", "zebra", "giraffe", "hat", "backpack", "umbrella", "shoe", "eye glasses",
    "handbag", "tie", "suitcase", "frisbee", "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove",
    "skateboard", "surfboard", "tennis racket", "bottle", "plate", "wine glass", "cup", "fork", "knife", "spoon",
    "bowl", "banana", "apple", "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut",
    "cake", "chair", "couch", "potted plant", "bed", "mirror", "dining table", "window", "desk", "toilet",
    "door", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone", "microwave", "oven", "toaster",
    "sink", "refrigerator", "blender", "book", "clock", "vase", "scissors", "teddy bear", "hair drier", "toothbrush",
    "hair brush"
};
static int32_t  s_label_offset[TIVX_DL_POST_PROC_MAX_NUM_CLASSNAMES] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 26, 27, 30, 31, 32, 33,
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    66, 69, 71, 72, 73, 74, 75, 76, 77, 78,
    79, 80, 81, 83, 84, 85, 86, 87, 88, 89,
    90
};

PostProcObj::PostProcObj() { setDefaults(); }

void PostProcObj::setDefaults()
{
    for (int i = 0; i < (int)APP_MODULES_MAX_TENSORS; i++)
    {
        this->input_tensor_arr[i] = NULL;
    }
    this->input_arr           = NULL;
    this->node                = NULL;
    this->config              = NULL;
    memset(&this->params, 0, sizeof(this->params));
    memset(&this->ioBufDesc, 0, sizeof(this->ioBufDesc));
    this->num_input_tensors   = 0;
    this->num_output_tensors  = 0;
    this->output_img_width    = 768;
    this->output_img_height   = 384;
    this->viz_th              = 0.0f;
    this->num_top_results     = 0;
    this->kernel              = NULL;
    this->output_image_arr    = NULL;
    for (int i = 0; i < (int)APP_MODULES_MAX_BUFQ_DEPTH; i++)
    {
        this->output_image[i] = NULL;
    }
    this->graph_parameter_index = 0;
    memset(this->objName, 0, sizeof(this->objName));
    this->num_ch = NUM_CH;

    this->params.task_type            = TIVX_DL_POST_PROC_DETECTION_TASK_TYPE;
    this->params.od_prms.scaleX       = 1;
    this->params.od_prms.scaleY       = 1;
    this->params.od_prms.formatter[0] = 0;
    this->params.od_prms.formatter[1] = 1;
    this->params.od_prms.formatter[2] = 2;
    this->params.od_prms.formatter[3] = 3;
    this->params.od_prms.formatter[4] = 5;
    this->params.od_prms.formatter[5] = 4;

    this->params.od_prms.labelIndexOffset = 0;
    this->params.od_prms.labelOffset      = s_label_offset;
    this->params.od_prms.classnames       = s_img_labels;
    this->params.od_prms.viz_th           = 0.6;
    /* Placeholder; initModule() overrides this */
    this->params.od_prms.outputFormat     = TIVX_DL_POST_PROC_OD_OUTPUT_NATIVE;
}

#include "tivx_dl_post_proc_host.h"

/* ================================================================ */
/*  Uniform-signature public API                                    */
/* ================================================================ */

/* ---- Virtual dispatch methods ---- */

vx_status PostProcObj::initModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_status status = VX_SUCCESS;
    vx_context context = appGraph->context;

    if (this->ioBufDesc.numInputBuf == 0)
    {
        printf("app_post_proc: ERROR: ioBufDesc not set\n");
        return VX_FAILURE;
    }

    this->num_input_tensors = this->ioBufDesc.numOutputBuf;
    this->num_output_tensors = this->ioBufDesc.numOutputBuf;
    this->kernel = tivxAddKernelDLPostProc(context, this->num_input_tensors);
    status = vxGetStatus((vx_reference)this->kernel);
    if (status != VX_SUCCESS)
    {
        printf("app_post_proc: ERROR: tivxAddKernelDLPostProc failed\n");
        return status;
    }

    /* Initialize post-proc configuration */
    /* Set ioBufDesc pointer based on task type */
    if (this->params.task_type == TIVX_DL_POST_PROC_DETECTION_TASK_TYPE) {
        this->params.od_prms.ioBufDesc = &this->ioBufDesc;
        /* Set default threshold if not already configured by app */
        if (this->params.od_prms.viz_th == 0.0f) {
            this->params.od_prms.viz_th = 0.6;
        }
        /* TODO: no flag in iobuf yet tells us whether the compiled
         * model is TIDLRT or OSRT -- infer from tensor count for now. */
        if (this->num_input_tensors == 1)
        {
            this->params.od_prms.outputFormat = TIVX_DL_POST_PROC_OD_OUTPUT_NATIVE;
        }
        else
        {
            this->params.od_prms.outputFormat = TIVX_DL_POST_PROC_OD_OUTPUT_REFORMATTED;
        }
    } else if (this->params.task_type == TIVX_DL_POST_PROC_CLASSIFICATION_TASK_TYPE) {
        this->params.oc_prms.ioBufDesc = &this->ioBufDesc;
    } else if (this->params.task_type == TIVX_DL_POST_PROC_SEGMENTATION_TASK_TYPE) {
        this->params.ss_prms.ioBufDesc = &this->ioBufDesc;
        this->params.ss_prms.inDataWidth = this->ioBufDesc.outWidth[0];
        this->params.ss_prms.inDataHeight = this->ioBufDesc.outHeight[0];
        /* Set default alpha blend value if not configured */
        if (this->params.ss_prms.alpha == 0.0f) {
            this->params.ss_prms.alpha = 0.4f;
        }
    } else {
        printf("app_post_proc: ERROR: Unknown task_type %d (must be DETECTION, CLASSIFICATION, or SEGMENTATION)\n",
               this->params.task_type);
        return VX_FAILURE;
    }

    this->params.num_input_tensors = this->num_input_tensors;

    this->config = vxCreateUserDataObject(context, "PostProcConfig", sizeof(tivxDLPostProcParams), &this->params);
    status = vxGetStatus((vx_reference)this->config);
    if (status != VX_SUCCESS)
    {
        printf("app_post_proc: ERROR: vxCreateUserDataObject failed\n");
        return status;
    }

    vx_image output_img = vxCreateImage(context, this->output_img_width, this->output_img_height, VX_DF_IMAGE_NV12);
    this->output_image_arr = vxCreateObjectArray(context, (vx_reference)output_img, this->num_ch);
    status = vxGetStatus((vx_reference)this->output_image_arr);
    vxReleaseImage(&output_img);

    return status;
}

vx_status PostProcObj::deinitModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vxReleaseUserDataObject(&this->config);
    vxReleaseObjectArray(&this->output_image_arr);
    return VX_SUCCESS;
}

vx_status PostProcObj::deleteNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    if(this->node != NULL)
    {
        vxReleaseNode(&this->node);
    }
    if(this->kernel != NULL)
    {
        vxRemoveKernel(this->kernel);
    }
    return VX_SUCCESS;
}

vx_status PostProcObj::createNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_graph graph_vx = appGraph->graph;
    vx_status status = VX_SUCCESS;
    uint32_t i = 0;
    vx_bool replicate[8];
    vx_image in_img   = (vx_image)vxGetObjectArrayItem((vx_object_array)this->input_arr, 0);
    vx_image result   = (vx_image)vxGetObjectArrayItem((vx_object_array)this->output_image_arr, 0);

    vx_tensor input_tensors_local[APP_MODULES_MAX_TENSORS];
    for(i = 0; i < this->num_input_tensors; i++)
    {
        input_tensors_local[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)this->input_tensor_arr[i], 0);
    }

    this->node = tivxDLPostProcNode(graph_vx,
                              this->kernel,
                              this->config,
                              in_img,
                              input_tensors_local,
                              result);

    APP_ASSERT_VALID_REF(this->node);
    status = vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);
    vxSetReferenceName((vx_reference)this->node, "post_proc_node");

    replicate[TIVX_DL_POST_PROC_CONFIG_IDX] = vx_false_e;
    replicate[TIVX_DL_POST_PROC_INPUT_IMAGE_IDX] = vx_true_e;
    replicate[TIVX_DL_POST_PROC_OUTPUT_IMAGE_IDX] = vx_true_e;
    for(i = 0; i < this->num_input_tensors; i++)
    {
        replicate[TIVX_DL_POST_PROC_INPUT_TENSOR_START_IDX + i] = vx_true_e;
    }
    vxReplicateNode(graph_vx, this->node, replicate,
                    TIVX_DL_POST_PROC_BASE_PARAMS + this->num_input_tensors);

    vxReleaseImage(&in_img);
    for(i = 0; i < this->num_input_tensors; i++)
    {
        vxReleaseTensor(&input_tensors_local[i]);
    }
    vxReleaseImage(&result);
    return(status);
}
