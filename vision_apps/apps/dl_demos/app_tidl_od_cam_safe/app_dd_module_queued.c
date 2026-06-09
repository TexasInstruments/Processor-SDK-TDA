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

#include "app_dd_module_queued.h"


vx_status app_init_dd_queued(vx_context context,
                            DrawDetectionsObj *drawDetectionsObj,
                            char *objName,
                            vx_int32 num_cameras,
                            vx_uint32 bufq_depth,
                            vx_bool create_input_pools,
                            vx_tensor input_tensor_exemplar)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i;
    vx_image exemplar_img = NULL;

    vx_size nd = 0U;
    vx_size dims[APP_MAX_TENSOR_DIMS] = {0U};
    vx_enum dt = 0;

    if (bufq_depth > APP_MODULES_MAX_BUFQ_DEPTH)
    {
        bufq_depth = APP_MODULES_MAX_BUFQ_DEPTH;
    }

    drawDetectionsObj->bufq_depth = bufq_depth;
    drawDetectionsObj->create_input_pools = create_input_pools;

    drawDetectionsObj->graph_parameter_out_image_index = -1;
    drawDetectionsObj->graph_parameter_in_tensor_index = -1;
    drawDetectionsObj->graph_parameter_in_image_index = -1;

    snprintf(drawDetectionsObj->objName, APP_MAX_FILE_PATH, "%s", objName);

    for (i = 0U; i < APP_MODULES_MAX_BUFQ_DEPTH; i++)
    {
        drawDetectionsObj->input_tensor_arr[i] = NULL;
        drawDetectionsObj->input_image_arr[i] = NULL;
        drawDetectionsObj->input_tensors[i] = NULL;
        drawDetectionsObj->input_images[i] = NULL;

        drawDetectionsObj->output_image_arr[i] = NULL;
        drawDetectionsObj->output_images[i] = NULL;
    }

    drawDetectionsObj->config = vxCreateUserDataObject(context, "", sizeof(tivxDrawBoxDetectionsParams), NULL);
    status = vxGetStatus((vx_reference)drawDetectionsObj->config);

    if (status == VX_SUCCESS)
    {
        status = vxCopyUserDataObject(drawDetectionsObj->config,
                                      0,
                                      sizeof(tivxDrawBoxDetectionsParams),
                                      &drawDetectionsObj->params,
                                      VX_WRITE_ONLY,
                                      VX_MEMORY_TYPE_HOST);
    }

    if (status == VX_SUCCESS)
    {
        exemplar_img = vxCreateImage(context,
                                     drawDetectionsObj->params.width,
                                     drawDetectionsObj->params.height,
                                     VX_DF_IMAGE_NV12);
        status = vxGetStatus((vx_reference)exemplar_img);
    }

    if (status == VX_SUCCESS)
    {
        for (i = 0U; i < drawDetectionsObj->bufq_depth; i++)
        {
            drawDetectionsObj->output_image_arr[i] =
                vxCreateObjectArray(context, (vx_reference)exemplar_img, num_cameras);
            status = vxGetStatus((vx_reference)drawDetectionsObj->output_image_arr[i]);

            if (status == VX_SUCCESS)
            {
                drawDetectionsObj->output_images[i] =
                    (vx_image)vxGetObjectArrayItem(drawDetectionsObj->output_image_arr[i], 0);
                status = vxGetStatus((vx_reference)drawDetectionsObj->output_images[i]);
            }

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if ((status == VX_SUCCESS) && (drawDetectionsObj->create_input_pools == vx_true_e))
    {
        for (i = 0U; i < drawDetectionsObj->bufq_depth; i++)
        {
            drawDetectionsObj->input_image_arr[i] =
                vxCreateObjectArray(context, (vx_reference)exemplar_img, num_cameras);
            status = vxGetStatus((vx_reference)drawDetectionsObj->input_image_arr[i]);

            if (status == VX_SUCCESS)
            {
                drawDetectionsObj->input_images[i] =
                    (vx_image)vxGetObjectArrayItem(drawDetectionsObj->input_image_arr[i], 0);
                status = vxGetStatus((vx_reference)drawDetectionsObj->input_images[i]);
            }

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if ((status == VX_SUCCESS) && (drawDetectionsObj->create_input_pools == vx_true_e))
    {
        if (input_tensor_exemplar == NULL)
        {
            status = VX_FAILURE;
        }
    }

    if ((status == VX_SUCCESS) && (drawDetectionsObj->create_input_pools == vx_true_e))
    {
        status = vxQueryTensor(input_tensor_exemplar, VX_TENSOR_NUMBER_OF_DIMS, &nd, sizeof(nd));
        if ((status == VX_SUCCESS) && ((nd == 0U) || (nd > (vx_size)APP_MAX_TENSOR_DIMS)))
        {
            status = VX_FAILURE;
        }
    }

    if ((status == VX_SUCCESS) && (drawDetectionsObj->create_input_pools == vx_true_e))
    {
        status = vxQueryTensor(input_tensor_exemplar, VX_TENSOR_DIMS, dims, nd * sizeof(vx_size));
    }

    if ((status == VX_SUCCESS) && (drawDetectionsObj->create_input_pools == vx_true_e))
    {
        status = vxQueryTensor(input_tensor_exemplar, VX_TENSOR_DATA_TYPE, &dt, sizeof(dt));
    }

    if ((status == VX_SUCCESS) && (drawDetectionsObj->create_input_pools == vx_true_e))
    {
        for (i = 0U; i < drawDetectionsObj->bufq_depth; i++)
        {
            vx_tensor exemplar_tensor = vxCreateTensor(context, nd, dims, dt, 0);
            status = vxGetStatus((vx_reference)exemplar_tensor);

            if (status == VX_SUCCESS)
            {
                drawDetectionsObj->input_tensor_arr[i] =
                    vxCreateObjectArray(context, (vx_reference)exemplar_tensor, num_cameras);
                status = vxGetStatus((vx_reference)drawDetectionsObj->input_tensor_arr[i]);
            }

            if (status == VX_SUCCESS)
            {
                drawDetectionsObj->input_tensors[i] =
                    (vx_tensor)vxGetObjectArrayItem(drawDetectionsObj->input_tensor_arr[i], 0);
                status = vxGetStatus((vx_reference)drawDetectionsObj->input_tensors[i]);
            }

            vxReleaseTensor(&exemplar_tensor);

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (exemplar_img != NULL)
    {
        vxReleaseImage(&exemplar_img);
    }

    if (status == VX_SUCCESS)
    {
        vx_char ref_name[APP_MAX_FILE_PATH];

        snprintf(ref_name, APP_MAX_FILE_PATH, "%s_config", objName);
        status = vxSetReferenceName((vx_reference)drawDetectionsObj->config, ref_name);
    }

    return status;
}

vx_status app_update_dd_queued(DrawDetectionsObj *drawDetectionsObj, vx_user_data_object config)
{
    vx_status status = VX_SUCCESS;

    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    status = vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                                 (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    if (status == VX_SUCCESS)
    {
        ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
        memcpy(&drawDetectionsObj->params.ioBufDesc, ioBufDesc, sizeof(sTIDL_IOBufDesc_t));

        status = vxUnmapUserDataObject(config, map_id_config);
    }

    return status;
}

void app_deinit_dd_queued(DrawDetectionsObj *drawDetectionsObj)
{
    vx_uint32 i;

    vxReleaseUserDataObject(&drawDetectionsObj->config);

    for (i = 0U; i < drawDetectionsObj->bufq_depth; i++)
    {
        if (drawDetectionsObj->create_input_pools == vx_true_e)
        {
            if (drawDetectionsObj->input_tensors[i] != NULL)
            {
                vxReleaseTensor(&drawDetectionsObj->input_tensors[i]);
            }

            if (drawDetectionsObj->input_tensor_arr[i] != NULL)
            {
                vxReleaseObjectArray(&drawDetectionsObj->input_tensor_arr[i]);
            }

            if (drawDetectionsObj->input_images[i] != NULL)
            {
                vxReleaseImage(&drawDetectionsObj->input_images[i]);
            }

            if (drawDetectionsObj->input_image_arr[i] != NULL)
            {
                vxReleaseObjectArray(&drawDetectionsObj->input_image_arr[i]);
            }
        }

        if (drawDetectionsObj->output_images[i] != NULL)
        {
            vxReleaseImage(&drawDetectionsObj->output_images[i]);
        }

        if (drawDetectionsObj->output_image_arr[i] != NULL)
        {
            vxReleaseObjectArray(&drawDetectionsObj->output_image_arr[i]);
        }
    }
}

void app_delete_dd_queued(DrawDetectionsObj *drawDetectionsObj)
{
    if (drawDetectionsObj->node != NULL)
    {
        vxReleaseNode(&drawDetectionsObj->node);
    }
}

vx_status app_create_graph_dd_queued(vx_graph graph,
                                    DrawDetectionsObj *drawDetectionsObj,
                                    vx_object_array input_tensor_arr,
                                    vx_object_array input_image_arr,
                                    const char *target_string)
{
    vx_status status = VX_SUCCESS;
    vx_tensor input_tensor = NULL;
    vx_image input_image = NULL;
    vx_image output_image = NULL;

    if (drawDetectionsObj->create_input_pools == vx_true_e)
    {
        input_tensor = drawDetectionsObj->input_tensors[0];
        input_image = drawDetectionsObj->input_images[0];
    }
    else
    {
        input_tensor = (vx_tensor)vxGetObjectArrayItem(input_tensor_arr, 0);
        input_image = (vx_image)vxGetObjectArrayItem(input_image_arr, 0);
    }

    output_image = drawDetectionsObj->output_images[0];

    drawDetectionsObj->node = tivxDrawBoxDetectionsNode(graph,
                                                        drawDetectionsObj->config,
                                                        input_tensor,
                                                        input_image,
                                                        output_image);

    APP_ASSERT_VALID_REF(drawDetectionsObj->node);

    status = vxSetNodeTarget(drawDetectionsObj->node, VX_TARGET_STRING, target_string);
    if (status == VX_SUCCESS)
    {
        status = vxSetReferenceName((vx_reference)drawDetectionsObj->node, "DrawBoxDetectionsNode");
    }

    {
        vx_bool replicate[] = {vx_false_e, vx_true_e, vx_true_e, vx_true_e};
        vxReplicateNode(graph, drawDetectionsObj->node, replicate, 4);
    }

    if (drawDetectionsObj->create_input_pools == vx_false_e)
    {
        if (input_tensor != NULL)
        {
            vxReleaseTensor(&input_tensor);
        }

        if (input_image != NULL)
        {
            vxReleaseImage(&input_image);
        }
    }

    return status;
}
