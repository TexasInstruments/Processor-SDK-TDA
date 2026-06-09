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

#include "app_pre_proc_module_queued.h"
#include "app_modules.h"

#include <vx_internal.h>

static vx_status create_output_tensors(vx_context context,
                                       vx_user_data_object config,
                                       vx_tensor output_tensors[])
{
    vx_status status = VX_SUCCESS;
    vx_size input_sizes[APP_MAX_TENSOR_DIMS];
    vx_map_id map_id_config = 0;
    vx_bool is_mapped = vx_false_e;

    tivxTIDLJ7Params *tidlParams = NULL;
    sTIDL_IOBufDesc_t *ioBufDesc = NULL;

    vx_uint32 id;
    vx_uint32 j;

    if ((context == NULL) || (config == NULL) || (output_tensors == NULL))
    {
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxMapUserDataObject(config,
                                     0,
                                     sizeof(tivxTIDLJ7Params),
                                     &map_id_config,
                                     (void **)&tidlParams,
                                     VX_READ_ONLY,
                                     VX_MEMORY_TYPE_HOST,
                                     0);
        if (status == VX_SUCCESS)
        {
            is_mapped = vx_true_e;
        }
    }

    if (status == VX_SUCCESS)
    {
        ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

        for (id = 0U; id < ioBufDesc->numInputBuf; id++)
        {
            vx_enum data_type;

            input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
            input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
            input_sizes[2] = ioBufDesc->inNumChannels[id];

            data_type = get_vx_tensor_datatype(ioBufDesc->inElementType[id]);

            output_tensors[id] = vxCreateTensor(context, 3, input_sizes, data_type, 0);

            status = vxGetStatus((vx_reference)output_tensors[id]);
            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (is_mapped == vx_true_e)
    {
        (void)vxUnmapUserDataObject(config, map_id_config);
        is_mapped = vx_false_e;
    }

    if (status != VX_SUCCESS)
    {
        for (j = 0U; j < APP_MAX_TENSORS; j++)
        {
            if (output_tensors[j] != NULL)
            {
                vxReleaseTensor(&output_tensors[j]);
                output_tensors[j] = NULL;
            }
        }
    }

    return status;
}

static vx_size get_tensor_data_type(vx_int32 tidl_type)
{
    vx_size openvx_type = VX_TYPE_INVALID;

    if (tidl_type == TIDL_UnsignedChar)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    else if (tidl_type == TIDL_SignedChar)
    {
        openvx_type = VX_TYPE_INT8;
    }
    else if (tidl_type == TIDL_UnsignedShort)
    {
        openvx_type = VX_TYPE_UINT16;
    }
    else if (tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if (tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if (tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if (tidl_type == TIDL_SinglePrecFloat)
    {
        openvx_type = VX_TYPE_FLOAT32;
    }
    else
    {
        /* leave invalid */
    }

    return openvx_type;
}

static vx_status pre_proc_update_from_config(PreProcObj *preProcObj,
                                             vx_user_data_object config)
{
    vx_status status = VX_SUCCESS;

    vx_map_id map_id_config = 0;
    vx_bool   is_mapped     = vx_false_e;

    tivxTIDLJ7Params  *tidlParams = NULL;
    sTIDL_IOBufDesc_t *ioBufDesc  = NULL;

    if ((preProcObj == NULL) || (config == NULL))
    {
        status = VX_FAILURE;
    }
    else
    {
        status = vxMapUserDataObject(config,
                                     0,
                                     sizeof(tivxTIDLJ7Params),
                                     &map_id_config,
                                     (void **)&tidlParams,
                                     VX_READ_ONLY,
                                     VX_MEMORY_TYPE_HOST,
                                     0);
        if (status == VX_SUCCESS)
        {
            is_mapped = vx_true_e;

            ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
            (void)memcpy(&preProcObj->ioBufDesc, ioBufDesc, sizeof(sTIDL_IOBufDesc_t));

            preProcObj->num_input_tensors  = preProcObj->ioBufDesc.numInputBuf;
            preProcObj->num_output_tensors = preProcObj->ioBufDesc.numOutputBuf;

            (void)vxUnmapUserDataObject(config, map_id_config);
            is_mapped = vx_false_e;
        }

        if (is_mapped == vx_true_e)
        {
            (void)vxUnmapUserDataObject(config, map_id_config);
            is_mapped = vx_false_e;
        }
    }

    return status;
}

static vx_status create_input_images(vx_context context,
                                     PreProcObj *preProcObj,
                                     vx_object_array exemplar_input_arr,
                                     vx_uint32 bufq_depth)
{
    vx_status status       = VX_SUCCESS;
    vx_image  exemplar_img = NULL;

    vx_uint32 q = 0U;

    if ((context == NULL) || (preProcObj == NULL) || (exemplar_input_arr == NULL) || (bufq_depth == 0U))
    {
        status = VX_FAILURE;
    }
    else
    {
        exemplar_img = (vx_image)vxGetObjectArrayItem(exemplar_input_arr, 0);
        status = vxGetStatus((vx_reference)exemplar_img);

        if (status == VX_SUCCESS)
        {
            for (q = 0U; (q < bufq_depth) && (status == VX_SUCCESS); q++)
            {
                preProcObj->input_img_arr[q] =
                    vxCreateObjectArray(context, (vx_reference)exemplar_img, NUM_CH);

                status = vxGetStatus((vx_reference)preProcObj->input_img_arr[q]);
                if (status == VX_SUCCESS)
                {
                    preProcObj->input_img[q] =
                        (vx_image)vxGetObjectArrayItem(preProcObj->input_img_arr[q], 0);

                    {
                        char name[VX_MAX_REFERENCE_NAME];
                        snprintf(name, sizeof(name), "pp_in_img_q%u", (unsigned)q);
                        (void)vxSetReferenceName((vx_reference)preProcObj->input_img[q], name);
                    }

                    status = vxGetStatus((vx_reference)preProcObj->input_img[q]);
                }
            }
        }

        if (exemplar_img != NULL)
        {
            (void)vxReleaseImage(&exemplar_img);
            exemplar_img = NULL;
        }

        if (status != VX_SUCCESS)
        {
            for (q = 0U; q < APP_MODULES_MAX_BUFQ_DEPTH; q++)
            {
                if (preProcObj->input_img[q] != NULL)
                {
                    (void)vxReleaseImage(&preProcObj->input_img[q]);
                    preProcObj->input_img[q] = NULL;
                }

                if (preProcObj->input_img_arr[q] != NULL)
                {
                    (void)vxReleaseObjectArray(&preProcObj->input_img_arr[q]);
                    preProcObj->input_img_arr[q] = NULL;
                }
            }
        }
    }

    return status;
}

static vx_status create_output_tensor_arrays(vx_context context,
                                             PreProcObj *preProcObj,
                                             vx_user_data_object config,
                                             vx_tensor tmp_tensors[APP_MAX_TENSORS])
{
    vx_status status = VX_SUCCESS;

    vx_uint32 q = 0U;
    vx_int32  i = 0;

    if ((context == NULL) || (preProcObj == NULL) || (config == NULL) || (tmp_tensors == NULL))
    {
        status = VX_FAILURE;
    }
    else
    {
        status = create_output_tensors(context, config, tmp_tensors);

        if (status == VX_SUCCESS)
        {
            for (q = 0U; (q < preProcObj->bufq_depth) && (status == VX_SUCCESS); q++)
            {
                for (i = 0; (i < (vx_int32)preProcObj->num_input_tensors) && (status == VX_SUCCESS); i++)
                {
                    preProcObj->output_tensor_arr[q][i] =
                        vxCreateObjectArray(context, (vx_reference)tmp_tensors[i], NUM_CH);

                    status = vxGetStatus((vx_reference)preProcObj->output_tensor_arr[q][i]);
                    if (status == VX_SUCCESS)
                    {
                        preProcObj->output_tensor[i][q] =
                            (vx_tensor)vxGetObjectArrayItem(preProcObj->output_tensor_arr[q][i], 0);

                        status = vxGetStatus((vx_reference)preProcObj->output_tensor[i][q]);
                    }
                }
            }
        }
    }

    return status;
}

static void cleanup_pre_proc_outputs(PreProcObj *preProcObj)
{
    vx_uint32 q = 0U;
    vx_int32  i = 0;

    if (preProcObj != NULL)
    {
        for (i = 0; i < APP_MAX_TENSORS; i++)
        {
            for (q = 0U; q < APP_MODULES_MAX_BUFQ_DEPTH; q++)
            {
                if (preProcObj->output_tensor[i][q] != NULL)
                {
                    (void)vxReleaseTensor(&preProcObj->output_tensor[i][q]);
                    preProcObj->output_tensor[i][q] = NULL;
                }
            }
        }

        for (q = 0U; q < APP_MODULES_MAX_BUFQ_DEPTH; q++)
        {
            for (i = 0; i < APP_MAX_TENSORS; i++)
            {
                if (preProcObj->output_tensor_arr[q][i] != NULL)
                {
                    (void)vxReleaseObjectArray(&preProcObj->output_tensor_arr[q][i]);
                    preProcObj->output_tensor_arr[q][i] = NULL;
                }
            }
        }
    }
}

static void cleanup_pre_proc_inputs(PreProcObj *preProcObj)
{
    vx_uint32 q = 0U;

    if (preProcObj != NULL)
    {
        for (q = 0U; q < APP_MODULES_MAX_BUFQ_DEPTH; q++)
        {
            if (preProcObj->input_img[q] != NULL)
            {
                (void)vxReleaseImage(&preProcObj->input_img[q]);
                preProcObj->input_img[q] = NULL;
            }

            if (preProcObj->input_img_arr[q] != NULL)
            {
                (void)vxReleaseObjectArray(&preProcObj->input_img_arr[q]);
                preProcObj->input_img_arr[q] = NULL;
            }
        }
    }
}

vx_status app_update_pre_proc_queued(vx_context context,
                                     PreProcObj *preProcObj,
                                     vx_user_data_object config,
                                     vx_uint32 bufq_depth,
                                     vx_object_array exemplar_input_arr)
{
    vx_status status = VX_SUCCESS;

    vx_tensor  tmp_tensors[APP_MAX_TENSORS] = { NULL };
    vx_int32   i = 0;

    if ((context == NULL) ||
        (preProcObj == NULL) ||
        (config == NULL) ||
        (bufq_depth == 0U) ||
        (exemplar_input_arr == NULL))
    {
        status = VX_FAILURE;
    }
    else
    {
        if (bufq_depth > (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH)
        {
            preProcObj->bufq_depth = (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH;
        }
        else
        {
            preProcObj->bufq_depth = bufq_depth;
        }

        status = pre_proc_update_from_config(preProcObj, config);

        if (status == VX_SUCCESS)
        {
            status = create_input_images(context,
                                         preProcObj,
                                         exemplar_input_arr,
                                         preProcObj->bufq_depth);
        }

        if (status == VX_SUCCESS)
        {
            status = create_output_tensor_arrays(context,
                                                 preProcObj,
                                                 config,
                                                 tmp_tensors);
        }

        if (status != VX_SUCCESS)
        {
            cleanup_pre_proc_outputs(preProcObj);
            cleanup_pre_proc_inputs(preProcObj);
        }

        for (i = 0; i < APP_MAX_TENSORS; i++)
        {
            if (tmp_tensors[i] != NULL)
            {
                (void)vxReleaseTensor(&tmp_tensors[i]);
                tmp_tensors[i] = NULL;
            }
        }
    }

    return status;
}

vx_status app_init_pre_proc_queued(vx_context context,
                                  PreProcObj *preProcObj,
                                  char *objName)
{
    vx_status status = VX_SUCCESS;

    tivxDLPreProcArmv8Params *local_preproc_config = &preProcObj->params;
    sTIDL_IOBufDesc_t *ioBufDesc = &preProcObj->ioBufDesc;

    vx_size data_type = get_tensor_data_type(ioBufDesc->inElementType[0]);


    if (status == VX_SUCCESS)
    {
        if ((data_type == VX_TYPE_INT8) || (data_type == VX_TYPE_UINT8))
        {
            if (ioBufDesc->inDataFormat[0] == 1)
            {
                local_preproc_config->tensor_format = 0;
            }
            else
            {
                local_preproc_config->tensor_format = 1;
            }
        }
        else if ((data_type == VX_TYPE_INT16) || (data_type == VX_TYPE_UINT16))
        {
            if (ioBufDesc->inDataFormat[0] == 1)
            {
                local_preproc_config->tensor_format = 0;
            }
            else
            {
                local_preproc_config->tensor_format = 1;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        preProcObj->config = vxCreateUserDataObject(context,
                                                    "pre_proc_config",
                                                    sizeof(tivxDLPreProcArmv8Params),
                                                    local_preproc_config);

        status = vxGetStatus((vx_reference)preProcObj->config);
    }

    if (status == VX_SUCCESS)
    {
        vx_char ref_name[APP_MAX_FILE_PATH];

        snprintf(ref_name, APP_MAX_FILE_PATH, "%s_config", objName);
        vxSetReferenceName((vx_reference)preProcObj->config, ref_name);

        strncpy(preProcObj->objName, objName, APP_MAX_FILE_PATH - 1);
        preProcObj->objName[APP_MAX_FILE_PATH - 1] = 0;
    }

    return status;
}

void app_deinit_pre_proc_queued(PreProcObj *preProcObj)
{
    if (preProcObj == NULL)
    {
        return;
    }
    else
    {
        if (preProcObj->config != NULL)
        {
            (void)vxReleaseUserDataObject(&preProcObj->config);
            preProcObj->config = NULL;
        }

        cleanup_pre_proc_outputs(preProcObj);
        cleanup_pre_proc_inputs(preProcObj);

        preProcObj->num_input_tensors  = 0U;
        preProcObj->num_output_tensors = 0U;
        preProcObj->bufq_depth         = 0U;
    }
}

void app_delete_pre_proc_queued(PreProcObj *preProcObj)
{
    if (preProcObj == NULL)
    {
        return;
    }

    if (preProcObj->node != NULL)
    {
        (void)vxReleaseNode(&preProcObj->node);
        preProcObj->node = NULL;
    }
}

vx_status app_create_graph_pre_proc_queued(vx_graph graph,
                                           PreProcObj *preProcObj,
                                           vx_image input_img,
                                           const char *node_name,
                                           const char *target_string)
{
    vx_status status = VX_SUCCESS;
    vx_image  input  = NULL;
    vx_tensor output = NULL;

    if ((graph == NULL) ||
        (preProcObj == NULL) ||
        (input_img == NULL) ||
        (node_name == NULL) ||
        (target_string == NULL))
    {
        status = VX_FAILURE;
    }
    else
    {
        input = input_img;
        status = vxGetStatus((vx_reference)input);

        if (status == VX_SUCCESS)
        {
            output = preProcObj->output_tensor[0][0];
            status = vxGetStatus((vx_reference)output);
        }

        if (status == VX_SUCCESS)
        {
            (void)vxSetReferenceName((vx_reference)input,
                                     "PreProc_Input_Image");
            (void)vxSetReferenceName((vx_reference)output,
                                     "PreProc_Output_Tensor");

            preProcObj->node = tivxDLPreProcArmv8Node(graph,
                                                     preProcObj->config,
                                                     input,
                                                     output);

            status = vxGetStatus((vx_reference)preProcObj->node);
        }

        if (status == VX_SUCCESS)
        {
            status = vxSetNodeTarget(preProcObj->node,
                                     VX_TARGET_STRING,
                                     target_string);
        }

        if (status == VX_SUCCESS)
        {
            (void)vxSetReferenceName((vx_reference)preProcObj->node,
                                     node_name);

            {
                vx_bool replicate[] = { vx_false_e, vx_true_e, vx_true_e };
                status = vxReplicateNode(graph,
                                         preProcObj->node,
                                         replicate,
                                         3U);
            }
        }
    }

    return status;
}
