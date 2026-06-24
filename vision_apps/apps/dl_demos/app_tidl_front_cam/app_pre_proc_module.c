/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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

#include "app_pre_proc_module.h"
#include <vx_internal.h>

static void createOutputTensors(vx_context context, vx_user_data_object config, vx_tensor output_tensors[]);

static vx_size getTensorDataType(vx_int32 tidl_type)
{
    vx_size openvx_type = VX_TYPE_INVALID;

    if (tidl_type == TIDL_UnsignedChar)
    {
        openvx_type = VX_TYPE_UINT8;
    }
    else if(tidl_type == TIDL_SignedChar)
    { 
        openvx_type = VX_TYPE_INT8;
    }
    else if(tidl_type == TIDL_UnsignedShort)
    {
        openvx_type = VX_TYPE_UINT16;
    } 
    else if(tidl_type == TIDL_SignedShort)
    {
        openvx_type = VX_TYPE_INT16;
    }
    else if(tidl_type == TIDL_UnsignedWord)
    {
        openvx_type = VX_TYPE_UINT32;
    }
    else if(tidl_type == TIDL_SignedWord)
    {
        openvx_type = VX_TYPE_INT32;
    }
    else if(tidl_type == TIDL_SinglePrecFloat)
    {
        openvx_type = VX_TYPE_FLOAT32;
    }

    return openvx_type;
}

/**
 * @brief Initialize the pre-processing object.
 *
 * @param[in] context The OpenVX context for the pre-processing node.
 * @param[in] preProcObj The pre-processing object to be initialized.
 * @param[in] objName The name of the pre-processing object.
 *
 * @return The status of the initialization.
 *
 * @details Initialize the pre-processing object and set the default parameters.
 * The default parameters are:
 *   skip_flag = 0
 *   scale = 1
 *   mean = 0
 *   channel_order = 0 (NCHW)
 *   tensor_format = 0 (RGB) or 1 (BGR) depending on the input data type.
 *   crop = 0
 */
vx_status app_init_pre_proc(vx_context context, PreProcObj *preProcObj, char *objName)
{
    vx_status status = VX_SUCCESS;
    tivxDLPreProcArmv8Params *local_preproc_config = &preProcObj->params;

    sTIDL_IOBufDesc_t *ioBufDesc = &preProcObj->ioBufDesc;
    vx_size data_type = getTensorDataType(ioBufDesc->inElementType[0]);

    if((data_type == VX_TYPE_INT8) || (data_type == VX_TYPE_UINT8))
    {
        if(ioBufDesc->inDataFormat[0] == 1) {
            local_preproc_config->tensor_format = 0; /* RGB */
        } else {
            local_preproc_config->tensor_format = 1; /* BGR */
        }
    }
    else if((data_type == VX_TYPE_INT16) || (data_type == VX_TYPE_UINT16))
    {
        if(ioBufDesc->inDataFormat[0] == 1) {
            local_preproc_config->tensor_format = 0; /* RGB */
        } else {
            local_preproc_config->tensor_format = 1; /* BGR */
        }
    }

    /* Use inChannelPitch from TIDL IO buf desc so the preproc kernel writes
     * each channel at the same stride the TIDL inference kernel reads with.
     * TIDL may apply DDR alignment that makes inChannelPitch != paddedW * paddedH.*/
    local_preproc_config->channel_pitch = (vx_uint32)ioBufDesc->inChannelPitch[0];

    preProcObj->config = vxCreateUserDataObject(context, "pre_proc_config", sizeof(tivxDLPreProcArmv8Params), local_preproc_config);
    status = vxGetStatus((vx_reference)preProcObj->config);

    vx_char ref_name[APP_MAX_FILE_PATH];
    snprintf(ref_name, APP_MAX_FILE_PATH, "%s_config", objName);
    vxSetReferenceName((vx_reference)preProcObj->config, ref_name);
    return status;
}

/**
 * @brief Updates the PreProcObj with the config data.
 *
 * This function updates the PreProcObj with the config data from the
 * tidl config. It updates the num_input_tensors, num_output_tensors,
 * and output_tensor_arr fields of the PreProcObj.
 *
 * @param[in] context The OpenVX context.
 * @param[in] preProcObj The PreProcObj to update.
 * @param[in] config The vx_user_data_object containing the config data.
 *
 * @returns The status of the operation.
 */
vx_status app_update_pre_proc(vx_context context, PreProcObj *preProcObj, vx_user_data_object config)
{
    vx_status status = VX_SUCCESS;

    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;
    vx_tensor output_tensors[APP_MAX_TENSORS];
    vx_int32 i;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                    (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    memcpy(&preProcObj->ioBufDesc, ioBufDesc, sizeof(sTIDL_IOBufDesc_t));

    vxUnmapUserDataObject(config, map_id_config);

    preProcObj->num_input_tensors = ioBufDesc->numInputBuf;
    preProcObj->num_output_tensors = ioBufDesc->numOutputBuf;

    createOutputTensors(context, config, output_tensors);
    
    for(i = 0; i < preProcObj->num_input_tensors; i++)
    {
        preProcObj->output_tensor_arr[i] = vxCreateObjectArray(context, (vx_reference)output_tensors[i], NUM_CH);
        vxReleaseTensor(&output_tensors[i]);
    }
    return status;
}

/**
 * @brief Releases resources allocated by app_init_pre_proc_od
 *
 * @param[in] preProcObj - A pointer to the pre-processing object which contains
 *                         the resources to be released.
 */
void app_deinit_pre_proc(PreProcObj *preProcObj)
{
    vxReleaseUserDataObject(&preProcObj->config);

    vx_int32 i;    
    for(i = 0; i < preProcObj->num_input_tensors; i++)
    {
        vxReleaseObjectArray(&preProcObj->output_tensor_arr[i]);
    }    
}

/**
 * @brief Deletes the pre-processing node from the graph.
 *
 * @param[in] preProcObj - A pointer to the pre-processing object which contains
 *                         the node to be deleted.
 */
void app_delete_pre_proc(PreProcObj *preProcObj)
{
    if(preProcObj->node != NULL)
    {
        vxReleaseNode(&preProcObj->node);
    }
}

/**
 * @brief This function creates a node in the graph to perform pre-processing
 *        steps for DL model execution.
 *
 * @param[in] graph - The graph to which the pre-processing node is to be added.
 * @param[in] preProcObj - A pointer to the pre-processing object which contains
 *                         the user data object containing the model parameters.
 * @param[in] input_arr - An array of input objects to the pre-processing node.
 * @param[in] node_name - The name to be given to the pre-processing node.
 *
 * @return VX_SUCCESS if the node is successfully created and added to the graph.
 *         VX_FAILURE otherwise.
 */
vx_status app_create_graph_pre_proc(vx_graph graph, PreProcObj *preProcObj, vx_object_array input_arr, char *node_name)
{
    vx_status status = VX_SUCCESS;

    vx_image  input   = (vx_image)vxGetObjectArrayItem((vx_object_array)input_arr, 0);
    vx_tensor output  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)preProcObj->output_tensor_arr[0], 0);
    
    vxSetReferenceName((vx_reference)input, "PreProc_Input_Image");
    vxSetReferenceName((vx_reference)output, "PreProc_Output_Tensor"); 

    // A72 for j784s4 A53 for j722s
    preProcObj->node = tivxDLPreProcArmv8Node(graph,
                                              preProcObj->config,
                                              input,
                                              output);

    APP_ASSERT_VALID_REF(preProcObj->node);
    status = vxSetNodeTarget(preProcObj->node, VX_TARGET_STRING, TIVX_TARGET_MPU_0);

    vxSetReferenceName((vx_reference)preProcObj->node, node_name);

    vx_bool replicate[] = {vx_false_e, vx_true_e, vx_true_e};
    vxReplicateNode(graph, preProcObj->node, replicate, 3);

    vxReleaseImage(&input);
    vxReleaseTensor(&output);

    return(status);
}

vx_status writePreProcOutput(char* file_name, vx_object_array output_arr)
{
    vx_status status = VX_SUCCESS;

    vx_tensor output;
    vx_size numCh;
    vx_int32 ch;

    vxQueryObjectArray((vx_object_array)output_arr, VX_OBJECT_ARRAY_NUMITEMS, &numCh, sizeof(vx_size));

    for(ch = 0; ch < numCh; ch++)
    {
        vx_size num_dims;
        void *data_ptr;
        vx_map_id map_id;

        vx_size    start[APP_MAX_TENSOR_DIMS];
        vx_size    tensor_strides[APP_MAX_TENSOR_DIMS];
        vx_size    tensor_sizes[APP_MAX_TENSOR_DIMS];

        output  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)output_arr, ch);

        vxQueryTensor(output, VX_TENSOR_NUMBER_OF_DIMS, &num_dims, sizeof(vx_size));

        if(num_dims != 3)
        {
            printf("Number of dims are != 3! exiting.. \n");
            break;
        }

        vxQueryTensor(output, VX_TENSOR_DIMS, tensor_sizes, 3 * sizeof(vx_size));

        start[0] = start[1] = start[2] = 0;

        tensor_strides[0] = 1;
        tensor_strides[1] = tensor_strides[0];
        tensor_strides[2] = tensor_strides[1] * tensor_strides[1];

        status = tivxMapTensorPatch(output, num_dims, start, tensor_sizes, &map_id, tensor_strides, &data_ptr, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);

        if(VX_SUCCESS == status)
        {
            vx_char new_name[APP_MAX_FILE_PATH];
            snprintf(new_name, APP_MAX_FILE_PATH, "%s_%dx%d_ch%d.rgb", file_name, (uint32_t)tensor_sizes[0], (uint32_t)tensor_sizes[1], ch);

            FILE *fp = fopen(new_name, "wb");
            if(NULL == fp)
            {
                printf("Unable to open file %s for writing!\n", new_name);
                break;
            }

            fwrite(data_ptr, 1, tensor_sizes[0] * tensor_sizes[1] * tensor_sizes[2], fp);
            fclose(fp);

            tivxUnmapTensorPatch(output, map_id);
        }

        vxReleaseTensor(&output);
    }

    return(status);
}

static void createOutputTensors(vx_context context, vx_user_data_object config, vx_tensor output_tensors[])
{
    vx_size   input_sizes[APP_MAX_TENSOR_DIMS];
    vx_map_id map_id_config;

    vx_uint32 id;

    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    for(id = 0; id < ioBufDesc->numInputBuf; id++)
    {
        input_sizes[0] = ioBufDesc->inWidth[id]  + ioBufDesc->inPadL[id] + ioBufDesc->inPadR[id];
        input_sizes[1] = ioBufDesc->inHeight[id] + ioBufDesc->inPadT[id] + ioBufDesc->inPadB[id];
        input_sizes[2] = ioBufDesc->inNumChannels[id];

        vx_enum data_type = get_vx_tensor_datatype(ioBufDesc->inElementType[id]);
        output_tensors[id] = vxCreateTensor(context, 3, input_sizes, data_type, 0);
    }

    vxUnmapUserDataObject(config, map_id_config);

    return;
}
