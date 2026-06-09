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
#include "app_tidl_module.h"

#undef COMPUTE_CHECKSUM

static vx_user_data_object readConfig(vx_context context, vx_char *config_file, uint32_t *num_input_tensors, uint32_t *num_output_tensors, vx_uint8 *check_sum);
static vx_user_data_object readNetwork(vx_context context, vx_char *network_file, vx_uint8 *check_sum);
static vx_status setCreateParams(vx_context context, TIDLObj *tidlObj);
static vx_status setInArgs(vx_context context, vx_user_data_object inArgs);
static vx_status setOutArgs(vx_context context, vx_user_data_object outArgs);
static void createOutputTensors(vx_context context, vx_user_data_object config, vx_tensor output_tensors[]);
static vx_status createOutputTensorsQ(vx_context context, vx_user_data_object config, vx_tensor output_tensors[]);
static void initParam(vx_reference params[], uint32_t _max_params);
static void addParam(vx_reference params[], vx_reference obj);
#ifdef COMPUTE_CHECKSUM
static void getQC(uint8_t *pIn, uint8_t *pOut, int32_t inSize);
#endif
static vx_status updateChecksums(vx_user_data_object config, vx_uint8 *config_checksum, vx_uint8 *network_checksum);

static uint32_t num_params;
static uint32_t max_params;

vx_status app_init_tidl(vx_context context, TIDLObj *tidlObj, char *objName, vx_int32 num_cameras)
{
     vx_status status = VX_SUCCESS;

     vx_uint32 num_input_tensors = 0;
     vx_uint32 num_output_tensors = 0;

     vx_tensor output_tensors[APP_MODULES_MAX_TENSORS];
     vx_uint32 capacity;
     vx_int32 i;

     tidlObj->kernel = NULL;

     tidlObj->config = readConfig(context, &tidlObj->config_file_path[0], &num_input_tensors, &num_output_tensors, &tidlObj->config_checksum[0]);
     status = vxGetStatus((vx_reference)tidlObj->config);

     tidlObj->num_input_tensors  = num_input_tensors;
     tidlObj->num_output_tensors = num_output_tensors;

     if(status == VX_SUCCESS)
     {
         tidlObj->network = readNetwork(context, &tidlObj->network_file_path[0], &tidlObj->network_checksum[0]);
         status = vxGetStatus((vx_reference)tidlObj->network);
     }

     if(status == VX_SUCCESS)
     {
        status = updateChecksums(tidlObj->config, &tidlObj->config_checksum[0], &tidlObj->network_checksum[0]);
     }

     if(status == VX_SUCCESS)
     {
         capacity = sizeof(TIDL_CreateParams);
         tidlObj->createParams = vxCreateUserDataObject(context, "TIDL_CreateParams", capacity, NULL );
         status = setCreateParams(context, tidlObj);
     }

     if(status == VX_SUCCESS)
     {
         vx_user_data_object inArgs;

         capacity = sizeof(TIDL_InArgs);
         inArgs = vxCreateUserDataObject(context, "TIDL_InArgs", capacity, NULL );
         tidlObj->in_args_arr  = vxCreateObjectArray(context, (vx_reference)inArgs, num_cameras);
         vxReleaseUserDataObject(&inArgs);

         vxSetReferenceName((vx_reference)tidlObj->in_args_arr, "tidl_node_in_args_arr");

         for(i = 0; i < num_cameras; i++)
         {
            vx_user_data_object inArgs;

            inArgs = (vx_user_data_object)vxGetObjectArrayItem(tidlObj->in_args_arr, i);
            setInArgs(context, inArgs);
            vxReleaseUserDataObject(&inArgs);
         }
     }

     if(status == VX_SUCCESS)
     {
         vx_user_data_object outArgs;

         capacity = sizeof(TIDL_outArgs);
         outArgs = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL );
         tidlObj->out_args_arr  = vxCreateObjectArray(context, (vx_reference)outArgs, num_cameras);
         vxReleaseUserDataObject(&outArgs);

         vxSetReferenceName((vx_reference)tidlObj->out_args_arr, "tidl_node_out_args_arr");

         for(i = 0; i < num_cameras; i++)
         {
            vx_user_data_object outArgs;

            outArgs = (vx_user_data_object)vxGetObjectArrayItem(tidlObj->out_args_arr, i);
            setOutArgs(context, outArgs);
            vxReleaseUserDataObject(&outArgs);
         }
     }

     for(i = 0; i < APP_MODULES_MAX_TENSORS; i++)
     {
         tidlObj->output_tensor_arr[i] = NULL;
     }

     createOutputTensors(context, tidlObj->config, output_tensors);

     for(i = 0; i < num_output_tensors; i++)
     {
         tidlObj->output_tensor_arr[i]  = vxCreateObjectArray(context, (vx_reference)output_tensors[i], num_cameras);
         vxSetReferenceName((vx_reference)tidlObj->output_tensor_arr[i], "tidl_node_output_tensor_arr");
         vxReleaseTensor(&output_tensors[i]);
     }

     if(status == VX_SUCCESS)
     {
         tidlObj->kernel = tivxAddKernelTIDL(context, tidlObj->num_input_tensors, tidlObj->num_output_tensors);
         status = vxGetStatus((vx_reference)tidlObj->kernel);
     }

     snprintf(tidlObj->objName, APP_MODULES_MAX_OBJ_NAME_SIZE, "%s", objName);

     return status;
}

void app_deinit_tidl(TIDLObj *tidlObj)
{
    vx_int32 i;

    vxReleaseUserDataObject(&tidlObj->config);
    vxReleaseUserDataObject(&tidlObj->network);
    vxReleaseUserDataObject(&tidlObj->createParams);

    vxReleaseObjectArray(&tidlObj->in_args_arr);
    vxReleaseObjectArray(&tidlObj->out_args_arr);

    for(i = 0; i < tidlObj->num_output_tensors; i++)
    {
        vxReleaseObjectArray(&tidlObj->output_tensor_arr[i]);
    }
}

void app_delete_tidl(TIDLObj *tidlObj)
{
  if(tidlObj->node != NULL)
  {
    vxReleaseNode(&tidlObj->node);
  }
  if(tidlObj->kernel != NULL)
  {
    vxRemoveKernel(tidlObj->kernel);
  }
}

vx_status app_create_graph_tidl(vx_context context, vx_graph graph, TIDLObj *tidlObj, vx_object_array input_tensor_arr[])
{
    vx_status status = VX_SUCCESS;

    vx_reference params[APP_MODULES_MAX_PARAMS];

    vx_tensor input_tensor[APP_MODULES_MAX_TENSORS];
    vx_tensor output_tensor[APP_MODULES_MAX_TENSORS];

    vx_int32 i;

    tidlObj->node = NULL;

    /* Initialize param array */
    initParam(params, APP_MODULES_MAX_PARAMS);

    /* The 1st param MUST be config array */
    addParam(params, (vx_reference)tidlObj->config);

    /* The 2nd param MUST be network tensor */
    addParam(params, (vx_reference)tidlObj->network);

    /* The 3rd param MUST be create params */
    addParam(params, (vx_reference)tidlObj->createParams);

    /* The 4th param MUST be inArgs */
    vx_user_data_object inArgs = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)tidlObj->in_args_arr, 0);
    addParam(params, (vx_reference)inArgs);
    vxReleaseUserDataObject(&inArgs);

    /* The 5th param MUST be outArgs */
    vx_user_data_object outArgs = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)tidlObj->out_args_arr, 0);
    addParam(params, (vx_reference)outArgs);
    vxReleaseUserDataObject(&outArgs);

    /* The 6th param MUST be NULL if trace data dump is not enabled */
    addParam(params, NULL);

    /* Create TIDL Node */
    for(i = 0; i < tidlObj->num_input_tensors; i++)
    {
        input_tensor[i]  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)input_tensor_arr[i], 0);
    }

    for(i = 0; i < tidlObj->num_output_tensors; i++)
    {
        output_tensor[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)tidlObj->output_tensor_arr[i], 0);
    }

    tidlObj->node = tivxTIDLNode(graph, tidlObj->kernel, params, input_tensor, output_tensor);
    status = vxGetStatus((vx_reference)tidlObj->node);
    vxSetReferenceName((vx_reference)tidlObj->node, "tidl_node");
    vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);

    #if defined(SOC_J784S4)
    if (tidlObj->core_id == 1)
    {
        vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_2);
    }
    else if (tidlObj->core_id == 2)
    {
        vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_3);
    }
    else if (tidlObj->core_id == 3)
    {
        vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_4);
    }
    #elif defined(SOC_J722S)
    if (tidlObj->core_id == 1)
    {
        vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_2);
    }
    #endif

    vx_bool replicate[16];
    replicate[TIVX_KERNEL_TIDL_IN_CONFIG_IDX] = vx_false_e;
    replicate[TIVX_KERNEL_TIDL_IN_NETWORK_IDX] = vx_false_e;
    replicate[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX] = vx_false_e;
    replicate[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX] = vx_true_e;
    replicate[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX] = vx_true_e;
    replicate[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX] = vx_false_e;

    for(i = 0; i < tidlObj->num_input_tensors; i++)
    {
      replicate[TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + i] = vx_true_e;
    }

    for(i = 0; i < tidlObj->num_output_tensors; i++)
    {
      replicate[TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + tidlObj->num_input_tensors + i] = vx_true_e;
    }

    vxReplicateNode(graph, tidlObj->node, replicate, TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + tidlObj->num_input_tensors + tidlObj->num_output_tensors);

    for(i = 0; i < tidlObj->num_input_tensors; i++)
    {
        vxReleaseTensor(&input_tensor[i]);
    }

    for(i = 0; i < tidlObj->num_output_tensors; i++)
    {
        vxReleaseTensor(&output_tensor[i]);
    }

    return status;
}

vx_status writeTIDLOutput(char *file_name, TIDLObj *tidlObj)
{
    vx_status status = VX_SUCCESS;

    vx_tensor output;
    vx_size numCh;
    vx_int32 ch, tensor_id;

    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    vxMapUserDataObject(tidlObj->config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                    (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

    for(tensor_id = 0; tensor_id < tidlObj->num_output_tensors; tensor_id++)
    {
        vxQueryObjectArray((vx_object_array)tidlObj->output_tensor_arr[tensor_id], VX_OBJECT_ARRAY_NUMITEMS, &numCh, sizeof(vx_size));

        for(ch = 0; ch < numCh; ch++)
        {
            vx_size num_dims;
            void *data_ptr;
            vx_map_id map_id;

            vx_size    start[APP_MODULES_MAX_TENSOR_DIMS];
            vx_size    tensor_strides[APP_MODULES_MAX_TENSOR_DIMS];
            vx_size    tensor_sizes[APP_MODULES_MAX_TENSOR_DIMS];

            output  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)tidlObj->output_tensor_arr[tensor_id], ch);

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
                vx_char new_name[APP_MODULES_MAX_FILE_PATH_SIZE];
                vx_int32 i,k;

                snprintf(new_name, APP_MODULES_MAX_FILE_PATH_SIZE, "%s_tensor_%d_%dx%dx%d_ch%d.bin", file_name, tensor_id, ioBufDesc->outWidth[tensor_id], ioBufDesc->outHeight[tensor_id], (uint32_t)tensor_sizes[2], ch);
                FILE *fp = fopen(new_name, "wb");
                if(NULL == fp)
                {
                    printf("Unable to open file %s for writing!\n", new_name);
                    break;
                }

                for(k = 0; k < tensor_sizes[2]; k++)
                {
                    uint8_t *pOut = (uint8_t *)data_ptr + (tensor_sizes[0] * tensor_sizes[1] * k) + (ioBufDesc->outPadT[0] * tensor_sizes[0]) + ioBufDesc->outPadL[0];

                    for(i = 0; i < ioBufDesc->outHeight[tensor_id]; i++)
                    {
                        fwrite(pOut, 1, ioBufDesc->outWidth[tensor_id], fp);
                        pOut += tensor_sizes[0];
                    }
                }

                fclose(fp);
                tivxUnmapTensorPatch(output, map_id);
            }
            vxReleaseTensor(&output);
        }
    }

  vxUnmapUserDataObject(tidlObj->config, map_id_config);

  return(status);
}

vx_status app_init_tidl_queued(vx_context context, TIDLObj *tidlObj, char *objName, vx_int32 num_cameras, vx_uint32 bufq_depth)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 num_input_tensors = 0U;
    vx_uint32 num_output_tensors = 0U;
    vx_tensor output_tensors[APP_MODULES_MAX_TENSORS] = {NULL};
    vx_uint32 capacity = 0U;
    vx_uint32 i = 0U;
    vx_uint32 q = 0U;

    if ((context == NULL) || (tidlObj == NULL) || (objName == NULL) || (num_cameras <= 0))
    {
        printf("Invalid argument passed to app_init_tidl_queued!\n");
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference)context);
    }

    if (status == VX_SUCCESS)
    {
        tidlObj->kernel = NULL;
        tidlObj->config = NULL;
        tidlObj->network = NULL;
        tidlObj->createParams = NULL;
        tidlObj->in_args_arr = NULL;
        tidlObj->out_args_arr = NULL;
        tidlObj->num_input_tensors = 0U;
        tidlObj->num_output_tensors = 0U;

        if (bufq_depth == 0U)
        {
            bufq_depth = 1U;
        }

        if (bufq_depth > (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH)
        {
            bufq_depth = (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH;
        }

        for (i = 0U; i < (vx_uint32)APP_MODULES_MAX_TENSORS; i++)
        {
            for (q = 0U; q < (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH; q++)
            {
                tidlObj->output_tensor_q[i][q] = NULL;
            }
        }

        for (i = 0U; i < (vx_uint32)APP_MODULES_MAX_TENSORS; i++)
        {
            for (q = 0U; q < (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH; q++)
            {
                tidlObj->output_tensor_arr_q[i][q] = NULL;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        tidlObj->config = readConfig(context,
                                     &tidlObj->config_file_path[0],
                                     &num_input_tensors,
                                     &num_output_tensors,
                                     &tidlObj->config_checksum[0]);
        status = vxGetStatus((vx_reference)tidlObj->config);
    }

    if (status == VX_SUCCESS)
    {
        tidlObj->num_input_tensors = num_input_tensors;
        tidlObj->num_output_tensors = num_output_tensors;

        tidlObj->network = readNetwork(context,
                                       &tidlObj->network_file_path[0],
                                       &tidlObj->network_checksum[0]);
        status = vxGetStatus((vx_reference)tidlObj->network);
    }

    if (status == VX_SUCCESS)
    {
        status = updateChecksums(tidlObj->config,
                                 &tidlObj->config_checksum[0],
                                 &tidlObj->network_checksum[0]);
    }

    if (status == VX_SUCCESS)
    {
        capacity = (vx_uint32)sizeof(TIDL_CreateParams);
        tidlObj->createParams = vxCreateUserDataObject(context, "TIDL_CreateParams", capacity, NULL);
        status = vxGetStatus((vx_reference)tidlObj->createParams);
    }

    if (status == VX_SUCCESS)
    {
        status = setCreateParams(context, tidlObj);
    }

    if (status == VX_SUCCESS)
    {
        vx_user_data_object inArgs = NULL;

        capacity = (vx_uint32)sizeof(TIDL_InArgs);
        inArgs = vxCreateUserDataObject(context, "TIDL_InArgs", capacity, NULL);
        status = vxGetStatus((vx_reference)inArgs);

        if (status == VX_SUCCESS)
        {
            tidlObj->in_args_arr = vxCreateObjectArray(context, (vx_reference)inArgs, (vx_uint32)num_cameras);
            status = vxGetStatus((vx_reference)tidlObj->in_args_arr);
        }

        vxReleaseUserDataObject(&inArgs);

        if (status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)tidlObj->in_args_arr, "tidl_node_in_args_arr");
        }

        if (status == VX_SUCCESS)
        {
            for (i = 0U; i < (vx_uint32)num_cameras; i++)
            {
                vx_user_data_object inArgsItem = NULL;

                inArgsItem = (vx_user_data_object)vxGetObjectArrayItem(tidlObj->in_args_arr, i);
                status = vxGetStatus((vx_reference)inArgsItem);

                if (status == VX_SUCCESS)
                {
                    status = setInArgs(context, inArgsItem);
                }

                if (inArgsItem != NULL)
                {
                    vxReleaseUserDataObject(&inArgsItem);
                }

                if (status != VX_SUCCESS)
                {
                    break;
                }
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        vx_user_data_object outArgs = NULL;

        capacity = (vx_uint32)sizeof(TIDL_outArgs);
        outArgs = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL);
        status = vxGetStatus((vx_reference)outArgs);

        if (status == VX_SUCCESS)
        {
            tidlObj->out_args_arr = vxCreateObjectArray(context, (vx_reference)outArgs, (vx_uint32)num_cameras);
            status = vxGetStatus((vx_reference)tidlObj->out_args_arr);
        }

        vxReleaseUserDataObject(&outArgs);

        if (status == VX_SUCCESS)
        {
            vxSetReferenceName((vx_reference)tidlObj->out_args_arr, "tidl_node_out_args_arr");
        }

        if (status == VX_SUCCESS)
        {
            for (i = 0U; i < (vx_uint32)num_cameras; i++)
            {
                vx_user_data_object outArgsItem = NULL;

                outArgsItem = (vx_user_data_object)vxGetObjectArrayItem(tidlObj->out_args_arr, i);
                status = vxGetStatus((vx_reference)outArgsItem);

                if (status == VX_SUCCESS)
                {
                    status = setOutArgs(context, outArgsItem);
                }

                if (outArgsItem != NULL)
                {
                    vxReleaseUserDataObject(&outArgsItem);
                }

                if (status != VX_SUCCESS)
                {
                    break;
                }
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        status = createOutputTensorsQ(context, tidlObj->config, output_tensors);
    }

    if (status == VX_SUCCESS)
    {
        for (i = 0U; i < tidlObj->num_output_tensors; i++)
        {
            for (q = 0U; q < bufq_depth; q++)
            {
                vx_char name[VX_MAX_REFERENCE_NAME];
                vx_tensor ch0 = NULL;

                tidlObj->output_tensor_arr_q[i][q] =
                    vxCreateObjectArray(context, (vx_reference)output_tensors[i], (vx_uint32)num_cameras);
                status = vxGetStatus((vx_reference)tidlObj->output_tensor_arr_q[i][q]);

                if (status == VX_SUCCESS)
                {
                    snprintf(name, VX_MAX_REFERENCE_NAME, "tidl_node_output_tensor_arr_%u_q%u", i, q);
                    vxSetReferenceName((vx_reference)tidlObj->output_tensor_arr_q[i][q], name);

                    ch0 = (vx_tensor)vxGetObjectArrayItem(tidlObj->output_tensor_arr_q[i][q], 0U);
                    status = vxGetStatus((vx_reference)ch0);
                }

                if (status == VX_SUCCESS)
                {
                    tidlObj->output_tensor_q[i][q] = ch0;
                    snprintf(name, VX_MAX_REFERENCE_NAME, "tidl_node_output_tensor_ch0_%u_q%u", i, q);
                    vxSetReferenceName((vx_reference)ch0, name);
                }

                if (status != VX_SUCCESS)
                {
                    break;
                }
            }

            if (output_tensors[i] != NULL)
            {
                vxReleaseTensor(&output_tensors[i]);
            }

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        tidlObj->kernel = tivxAddKernelTIDL(context,
                                            tidlObj->num_input_tensors,
                                            tidlObj->num_output_tensors);
        status = vxGetStatus((vx_reference)tidlObj->kernel);
    }

    if (status == VX_SUCCESS)
    {
        snprintf(tidlObj->objName, APP_MODULES_MAX_OBJ_NAME_SIZE, "%s", objName);
    }

    if (status != VX_SUCCESS)
    {
        for (i = 0U; i < (vx_uint32)APP_MODULES_MAX_TENSORS; i++)
        {
            if (output_tensors[i] != NULL)
            {
                vxReleaseTensor(&output_tensors[i]);
            }
        }
    }

    return status;
}

void app_deinit_tidl_queued(TIDLObj *tidlObj, vx_uint32 bufq_depth)
{
    vx_uint32 i = 0U;
    vx_uint32 q = 0U;

    if (tidlObj == NULL)
    {
        return;
    }

    if (bufq_depth == 0U)
    {
        bufq_depth = 1U;
    }

    if (bufq_depth > (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH)
    {
        bufq_depth = (vx_uint32)APP_MODULES_MAX_BUFQ_DEPTH;
    }

    for (i = 0U; i < tidlObj->num_output_tensors; i++)
    {
        for (q = 0U; q < bufq_depth; q++)
        {
            if (tidlObj->output_tensor_q[i][q] != NULL)
            {
                vxReleaseTensor(&tidlObj->output_tensor_q[i][q]);
            }
        }
    }

    if (tidlObj->config != NULL)
    {
        vxReleaseUserDataObject(&tidlObj->config);
    }

    if (tidlObj->network != NULL)
    {
        vxReleaseUserDataObject(&tidlObj->network);
    }

    if (tidlObj->createParams != NULL)
    {
        vxReleaseUserDataObject(&tidlObj->createParams);
    }

    if (tidlObj->in_args_arr != NULL)
    {
        vxReleaseObjectArray(&tidlObj->in_args_arr);
    }

    if (tidlObj->out_args_arr != NULL)
    {
        vxReleaseObjectArray(&tidlObj->out_args_arr);
    }

    for (i = 0U; i < tidlObj->num_output_tensors; i++)
    {
        for (q = 0U; q < bufq_depth; q++)
        {
            if (tidlObj->output_tensor_arr_q[i][q] != NULL)
            {
                vxReleaseObjectArray(&tidlObj->output_tensor_arr_q[i][q]);
            }
        }
    }
}

vx_status app_create_graph_tidl_queued(vx_context context, vx_graph graph, TIDLObj *tidlObj,
                                       vx_object_array input_tensor_arr[])
{
    vx_status status = VX_SUCCESS;
    vx_reference params[APP_MODULES_MAX_PARAMS];
    vx_tensor input_tensor[APP_MODULES_MAX_TENSORS] = {NULL};
    vx_tensor output_tensor[APP_MODULES_MAX_TENSORS] = {NULL};
    vx_user_data_object inArgs = NULL;
    vx_user_data_object outArgs = NULL;
    vx_uint32 i = 0U;

    (void)context;

    tidlObj->node = NULL;

    initParam(params, APP_MODULES_MAX_PARAMS);

    addParam(params, (vx_reference)tidlObj->config);
    addParam(params, (vx_reference)tidlObj->network);
    addParam(params, (vx_reference)tidlObj->createParams);

    if (status == VX_SUCCESS)
    {
        inArgs = (vx_user_data_object)vxGetObjectArrayItem(tidlObj->in_args_arr, 0U);
        status = vxGetStatus((vx_reference)inArgs);
    }

    if (status == VX_SUCCESS)
    {
        addParam(params, (vx_reference)inArgs);
    }

    if (status == VX_SUCCESS)
    {
        outArgs = (vx_user_data_object)vxGetObjectArrayItem(tidlObj->out_args_arr, 0U);
        status = vxGetStatus((vx_reference)outArgs);
    }

    if (status == VX_SUCCESS)
    {
        addParam(params, (vx_reference)outArgs);
        addParam(params, NULL);
    }

    if (status == VX_SUCCESS)
    {
        for (i = 0U; i < tidlObj->num_input_tensors; i++)
        {
            input_tensor[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)input_tensor_arr[i], 0U);
            status = vxGetStatus((vx_reference)input_tensor[i]);

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        for (i = 0U; i < tidlObj->num_output_tensors; i++)
        {
            output_tensor[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)tidlObj->output_tensor_arr_q[i][0U], 0U);
            status = vxGetStatus((vx_reference)output_tensor[i]);

            if (status != VX_SUCCESS)
            {
                break;
            }
        }
    }

    if (status == VX_SUCCESS)
    {
        tidlObj->node = tivxTIDLNode(graph, tidlObj->kernel, params, input_tensor, output_tensor);
        status = vxGetStatus((vx_reference)tidlObj->node);
    }

    if (status == VX_SUCCESS)
    {
        vx_bool replicate[16] = {vx_false_e};

        vxSetReferenceName((vx_reference)tidlObj->node, "tidl_node");
        vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);

#if defined(SOC_J784S4)
        if (tidlObj->core_id == 1U)
        {
            vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_2);
        }
        else if (tidlObj->core_id == 2U)
        {
            vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_3);
        }
        else if (tidlObj->core_id == 3U)
        {
            vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_4);
        }
#endif

        replicate[TIVX_KERNEL_TIDL_IN_CONFIG_IDX] = vx_false_e;
        replicate[TIVX_KERNEL_TIDL_IN_NETWORK_IDX] = vx_false_e;
        replicate[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX] = vx_false_e;
        replicate[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX] = vx_true_e;
        replicate[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX] = vx_true_e;
        replicate[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX] = vx_false_e;

        for (i = 0U; i < tidlObj->num_input_tensors; i++)
        {
            replicate[TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + i] = vx_true_e;
        }

        for (i = 0U; i < tidlObj->num_output_tensors; i++)
        {
            replicate[TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + tidlObj->num_input_tensors + i] = vx_true_e;
        }

        status = vxReplicateNode(graph,
                                 tidlObj->node,
                                 replicate,
                                 TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + tidlObj->num_input_tensors + tidlObj->num_output_tensors);
    }

    if (inArgs != NULL)
    {
        vxReleaseUserDataObject(&inArgs);
    }

    if (outArgs != NULL)
    {
        vxReleaseUserDataObject(&outArgs);
    }

    for (i = 0U; i < tidlObj->num_input_tensors; i++)
    {
        if (input_tensor[i] != NULL)
        {
            vxReleaseTensor(&input_tensor[i]);
        }
    }

    for (i = 0U; i < tidlObj->num_output_tensors; i++)
    {
        if (output_tensor[i] != NULL)
        {
            vxReleaseTensor(&output_tensor[i]);
        }
    }

    return status;
}

static vx_user_data_object readConfig(vx_context context, vx_char *config_file,  vx_uint32 *num_input_tensors, vx_uint32 *num_output_tensors, vx_uint8 *check_sum)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object config = NULL;
    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;
    vx_uint32  capacity;
    vx_map_id map_id;
    vx_size read_count;

    FILE *fp_config;

    fp_config = fopen(config_file, "rb");

    if(fp_config == NULL)
    {
        printf("Unable to open file! %s \n", config_file);
        return NULL;
    }

    fseek(fp_config, 0, SEEK_END);
    capacity = ftell(fp_config);
    fseek(fp_config, 0, SEEK_SET);

    if( capacity != sizeof(sTIDL_IOBufDesc_t))
    {
        printf("Config file size (%d bytes) does not match size of sTIDL_IOBufDesc_t (%d bytes)\n", capacity, (vx_uint32)sizeof(sTIDL_IOBufDesc_t));
        fclose(fp_config);
        return NULL;
    }

    status = vxGetStatus((vx_reference)context);

    if(VX_SUCCESS == status)
    {
        config = vxCreateUserDataObject(context, "tivxTIDLJ7Params", sizeof(tivxTIDLJ7Params), NULL );
        status = vxGetStatus((vx_reference)config);

        if (VX_SUCCESS == status)
        {
            vxSetReferenceName((vx_reference)config, "tidl_node_config");

            vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id,
                            (void **)&tidlParams, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

            if(tidlParams == NULL)
            {
                printf("Map of config object failed\n");
                fclose(fp_config);
                return NULL;
            }

            tivx_tidl_j7_params_init(tidlParams);

            ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

            read_count = fread(ioBufDesc, capacity, 1, fp_config);
            if(read_count != 1)
            {
                printf("Unable to read file %s \n", config_file);
            }

            *num_input_tensors  = ioBufDesc->numInputBuf;
            *num_output_tensors = ioBufDesc->numOutputBuf;

#ifdef COMPUTE_CHECKSUM
            tidlParams->compute_config_checksum  = 0;
            tidlParams->compute_network_checksum = 1;
#else
            tidlParams->compute_config_checksum  = 0;
            tidlParams->compute_network_checksum = 0;
#endif
            vxUnmapUserDataObject(config, map_id);
        }
    }
    fclose(fp_config);

    return config;
}


static vx_user_data_object readNetwork(vx_context context, vx_char *network_file, vx_uint8 *check_sum)
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object network;
    vx_map_id  map_id;
    vx_uint32  capacity;
    vx_size read_count;
    void  *network_buffer = NULL;

    FILE *fp_network;

    fp_network = fopen(network_file, "rb");

    if(fp_network == NULL)
    {
        printf("Unable to open file! %s \n", network_file);
        return NULL;
    }

    fseek(fp_network, 0, SEEK_END);
    capacity = ftell(fp_network);
    fseek(fp_network, 0, SEEK_SET);

    network = vxCreateUserDataObject(context, "TIDL_network", capacity, NULL );
    status = vxGetStatus((vx_reference)network);

    if (VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)network, "tidl_node_network");

        vxMapUserDataObject(network, 0, capacity, &map_id,
                        (void **)&network_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if(network_buffer)
        {
            read_count = fread(network_buffer, capacity, 1, fp_network);
            if(read_count != 1)
            {
                printf("Unable to read file!\n");
            }
            #ifdef COMPUTE_CHECKSUM
            sTIDL_Network_t *pNet = (sTIDL_Network_t *)network_buffer;
            if (pNet->dataFlowInfo < capacity)
            {
                uint8_t *pPerfInfo = (uint8_t *)network_buffer + pNet->dataFlowInfo;
                printf("Computing checksum at 0x%016lX, size = %d\n", (uint64_t)pPerfInfo,  capacity - pNet->dataFlowInfo);
                getQC(pPerfInfo, check_sum, capacity - pNet->dataFlowInfo);
            }
            else
            {
                printf("ERROR: pNet->dataFlowInfo should be less than %d\n", capacity);
            }
            #endif
        }
        else
        {
            printf("Unable to allocate memory for reading network! %d bytes\n", capacity);
        }

        vxUnmapUserDataObject(network, map_id);
    }

    fclose(fp_network);

    return network;
}

static vx_status updateChecksums(vx_user_data_object config, vx_uint8 *config_checksum, vx_uint8 *network_checksum)
{
  vx_status status = VX_SUCCESS;

  tivxTIDLJ7Params *tidlParams;
  vx_map_id  map_id;

  vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id,
                  (void **)&tidlParams, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

  if(tidlParams != NULL)
  {
    memcpy(tidlParams->config_checksum, config_checksum, TIVX_TIDL_J7_CHECKSUM_SIZE);
    memcpy(tidlParams->network_checksum, network_checksum, TIVX_TIDL_J7_CHECKSUM_SIZE);
  }
  else
  {
    printf("Unable to copy checksums!\n");
    status = VX_FAILURE;
  }

  vxUnmapUserDataObject(config, map_id);

  return status;
}
static vx_status setCreateParams(vx_context context, TIDLObj *tidlObj)
{
    vx_status status = VX_SUCCESS;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *createParams_buffer = NULL;

    status = vxGetStatus((vx_reference)tidlObj->createParams);

    if(VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)tidlObj->createParams, "tidl_node_createParams");

        capacity = sizeof(TIDL_CreateParams);
        vxMapUserDataObject(tidlObj->createParams, 0, capacity, &map_id,
              (void **)&createParams_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if(createParams_buffer)
        {
            TIDL_CreateParams *prms = createParams_buffer;
            //write create params here
            TIDL_createParamsInit(prms);

            prms->coreId                        = tidlObj->core_id;
            prms->isInbufsPaded                 = 1;
            prms->quantRangeExpansionFactor     = 1.0;
            prms->quantRangeUpdateFactor        = 0.0;
            prms->traceLogLevel                 = 0;
            prms->traceWriteLevel               = 0;
        }
        else
        {
            printf("Unable to allocate memory for create time params! %d bytes\n", capacity);
        }

        vxUnmapUserDataObject(tidlObj->createParams, map_id);
    }

    return status;
}

static vx_status setInArgs(vx_context context, vx_user_data_object inArgs)
{
    vx_status status = VX_SUCCESS;

    vx_map_id  map_id;
    vx_uint32  capacity;
    void *inArgs_buffer = NULL;

    status = vxGetStatus((vx_reference)inArgs);

    if(VX_SUCCESS == status)
    {
        capacity = sizeof(TIDL_InArgs);
        vxMapUserDataObject(inArgs, 0, capacity, &map_id,
                    (void **)&inArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if(inArgs_buffer)
        {
            TIDL_InArgs *prms = inArgs_buffer;
            prms->iVisionInArgs.size         = sizeof(TIDL_InArgs);
            prms->iVisionInArgs.subFrameInfo = 0;
        }
        else
        {
            printf("Unable to allocate memory for inArgs! %d bytes\n", capacity);
        }

        vxUnmapUserDataObject(inArgs, map_id);
    }

    return status;
}

static vx_status setOutArgs(vx_context context, vx_user_data_object outArgs)
{
    vx_status status = VX_SUCCESS;

    vx_map_id  map_id;
    vx_uint32  capacity;
    void *outArgs_buffer = NULL;

    status = vxGetStatus((vx_reference)outArgs);

    if(VX_SUCCESS == status)
    {
        capacity = sizeof(TIDL_outArgs);
        vxMapUserDataObject(outArgs, 0, capacity, &map_id,
                            (void **)&outArgs_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if(outArgs_buffer)
        {
            TIDL_outArgs *prms = outArgs_buffer;
            prms->iVisionOutArgs.size  = sizeof(TIDL_outArgs);
        }
        else
        {
            printf("Unable to allocate memory for outArgs! %d bytes\n", capacity);
        }

        vxUnmapUserDataObject(outArgs, map_id);
    }

    return status;
}

static void createOutputTensors(vx_context context, vx_user_data_object config, vx_tensor output_tensors[])
{
    vx_size output_sizes[APP_MODULES_MAX_TENSOR_DIMS];
    vx_map_id map_id_config;

    vx_uint32 id;

    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    for(id = 0; id < ioBufDesc->numOutputBuf; id++) {
        vx_char name[VX_MAX_REFERENCE_NAME];

        snprintf(name, VX_MAX_REFERENCE_NAME, "tidl_node_output_tensors_%d", id);

        output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
        output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
        output_sizes[2] = ioBufDesc->outNumChannels[id];

        vx_enum data_type = get_vx_tensor_datatype(ioBufDesc->outElementType[id]);
        output_tensors[id] = vxCreateTensor(context, 3, output_sizes, data_type, 0);
        vxSetReferenceName((vx_reference)output_tensors[id], name);
    }

  vxUnmapUserDataObject(config, map_id_config);

  return;
}

static vx_status createOutputTensorsQ(vx_context context, vx_user_data_object config, vx_tensor output_tensors[])
{
    vx_status status = VX_SUCCESS;
    vx_size output_sizes[APP_MODULES_MAX_TENSOR_DIMS] = {0U};
    vx_map_id map_id_config = 0U;
    vx_uint32 id = 0U;
    tivxTIDLJ7Params *tidlParams = NULL;
    sTIDL_IOBufDesc_t *ioBufDesc = NULL;
    vx_bool is_mapped = (vx_bool)vx_false_e;

    if ((context == NULL) || (config == NULL) || (output_tensors == NULL))
    {
        printf("Invalid argument passed to createOutputTensors!\n");
        status = VX_FAILURE;
    }

    if (status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference)context);
    }

    if (status == VX_SUCCESS)
    {
        status = vxGetStatus((vx_reference)config);
    }

    if (status == VX_SUCCESS)
    {
        status = vxMapUserDataObject(config,
                                     0U,
                                     sizeof(tivxTIDLJ7Params),
                                     &map_id_config,
                                     (void **)&tidlParams,
                                     VX_READ_ONLY,
                                     VX_MEMORY_TYPE_HOST,
                                     0U);
    }

    if (status == VX_SUCCESS)
    {
        if (tidlParams != NULL)
        {
            is_mapped = (vx_bool)vx_true_e;
            ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
        }
        else
        {
            printf("Unable to map config object!\n");
            status = VX_FAILURE;
        }
    }

    if (status == VX_SUCCESS)
    {
        for (id = 0U; id < ioBufDesc->numOutputBuf; id++)
        {
            vx_char name[VX_MAX_REFERENCE_NAME];
            vx_enum data_type;

            snprintf(name, VX_MAX_REFERENCE_NAME, "tidl_node_output_tensors_%u", id);

            output_sizes[0] = ioBufDesc->outWidth[id] + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
            output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
            output_sizes[2] = ioBufDesc->outNumChannels[id];

            data_type = get_vx_tensor_datatype(ioBufDesc->outElementType[id]);
            output_tensors[id] = vxCreateTensor(context, 3U, output_sizes, data_type, 0U);
            status = vxGetStatus((vx_reference)output_tensors[id]);

            if (status == VX_SUCCESS)
            {
                vxSetReferenceName((vx_reference)output_tensors[id], name);
            }
            else
            {
                printf("Unable to create output tensor %u!\n", id);
                break;
            }
        }
    }

    if (is_mapped == (vx_bool)vx_true_e)
    {
        vx_status unmap_status = vxUnmapUserDataObject(config, map_id_config);

        if ((status == VX_SUCCESS) && (unmap_status != VX_SUCCESS))
        {
            status = unmap_status;
        }
    }

    if (status != VX_SUCCESS)
    {
        for (id = 0U; id < APP_MODULES_MAX_TENSORS; id++)
        {
            if (output_tensors[id] != NULL)
            {
                vxReleaseTensor(&output_tensors[id]);
            }
        }
    }

    return status;
}

static void initParam(vx_reference params[], uint32_t _max_params)
{
   num_params  = 0;
   max_params = _max_params;
}

static void addParam(vx_reference params[], vx_reference obj)
{
    if(num_params <= max_params)
    {
        params[num_params] = obj;
        num_params++;
    }
    else
    {
        APP_ERROR("Error! num_params > max_params!\n");
    }
    
}

#ifdef COMPUTE_CHECKSUM
static void getQC(uint8_t *pIn, uint8_t *pOut, int32_t inSize)
{
  int32_t i, j;
  uint8_t vec[TIVX_TIDL_J7_CHECKSUM_SIZE];
  int32_t remSize;

  /* Initialize vector */
  for(j = 0; j < TIVX_TIDL_J7_CHECKSUM_SIZE; j++)
  {
     vec[j] = 0;
  }

  /* Create QC */
  remSize = inSize;
  for(i = 0; i < inSize; i+=TIVX_TIDL_J7_CHECKSUM_SIZE)
  {
    int32_t elems;

    if (remSize < TIVX_TIDL_J7_CHECKSUM_SIZE)
    {
      elems = TIVX_TIDL_J7_CHECKSUM_SIZE - remSize;
      remSize += TIVX_TIDL_J7_CHECKSUM_SIZE;
    }
    else
    {
      elems = TIVX_TIDL_J7_CHECKSUM_SIZE;
      remSize -= TIVX_TIDL_J7_CHECKSUM_SIZE;
    }

    for(j = 0; j < elems; j++)
    {
      vec[j] ^= pIn[i + j];
    }
  }

  /* Return QC */
  for(j = 0; j < TIVX_TIDL_J7_CHECKSUM_SIZE; j++)
  {
    pOut[j] = vec[j];
  }
}
#endif
