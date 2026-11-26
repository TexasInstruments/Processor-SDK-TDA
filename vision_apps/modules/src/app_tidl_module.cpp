/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include "app_tidl_module.hpp"
#include <string.h>

TIDLObj::TIDLObj() { setDefaults(); }

void TIDLObj::setDefaults()
{
    for (int i = 0; i < APP_MODULE_TIDL_MAX_TENSORS; i++)
    {
        this->input_tensor_arr[i]  = NULL;
        this->output_tensor_arr[i] = NULL;
    }
    this->node          = NULL;
    this->kernel        = NULL;
    this->config        = NULL;
    this->network       = NULL;
    this->createParams  = NULL;
    this->in_args_arr   = NULL;
    this->out_args_arr  = NULL;
    this->trace_data_arr = NULL;
    this->num_input_tensors  = 0;
    this->num_output_tensors = 0;
    this->graph_parameter_index = 0;
    this->bufq_depth = 0;  
    this->inferenceMode = 0;
    this->coreNum = 1;
    this->coreStartIdx = 1;
    this->config_file_path.clear();
    this->network_file_path.clear();
    memset(this->objName, 0, sizeof(this->objName));
    this->num_cameras = 1;
    memset(this->config_checksum, 0, sizeof(this->config_checksum));
    memset(this->network_checksum, 0, sizeof(this->network_checksum));

    /* XYCoordinate support (for BEV networks) */
    this->enable_xycord = vx_false_e;
    this->xycord_size[0] = 0;
    for (int i = 0; i < (int)APP_MAX_BUFQ_DEPTH; i++)
    {
        this->input_xycord_arr[i] = NULL;
        this->input_xycord[i] = NULL;
    }

    memset(&this->ioBufDesc, 0, sizeof(this->ioBufDesc));

    #if defined(SOC_J784S4) || defined(SOC_J722S)
    this->core_id = 1;
    #else
    this->core_id = 0;
    #endif
}

sTIDL_IOBufDesc_t* TIDLObj::getIOBufDesc()
{
    return &this->ioBufDesc;
}

void TIDLObj::readConfigFile()
{
    FILE *fp = fopen(this->config_file_path.c_str(), "rb");
    if (fp != NULL)
    {
        size_t read_count = fread(&this->ioBufDesc, sizeof(sTIDL_IOBufDesc_t), 1, fp);
        if (read_count == 1)
        {
            this->num_input_tensors  = this->ioBufDesc.numInputBuf;
            this->num_output_tensors = this->ioBufDesc.numOutputBuf;
        }
        else
        {
            printf("[TIDL] Unable to read config file %s\n", this->config_file_path.c_str());
        }
        fclose(fp);
    }
    else
    {
        printf("[TIDL] Unable to open config file %s\n", this->config_file_path.c_str());
    }
}

#undef COMPUTE_CHECKSUM

vx_status TIDLObj::initModule(AppGraphObj *appGraph, vx_int32 module_index)
{
     vx_context context = appGraph->context;
     vx_int32 num_cameras_local = this->num_cameras;
     vx_status status = VX_SUCCESS;

     vx_uint32 num_input_tensors = 0;
     vx_uint32 num_output_tensors = 0;
     vx_uint32 inferenceMode = 0;
     vx_uint32 num_cores = 1;

     vx_tensor output_tensors_local[APP_MODULES_MAX_TENSORS];
     vx_uint32 capacity;
     vx_int32 i;

     this->kernel = NULL;

     /* Check if config file path is set */
     if(this->config_file_path.empty())
     {
         printf("[TIDL] ERROR: config_file_path is empty! Set it before calling addNode()\n");
         return VX_FAILURE;
     }

     this->config = this->readConfig(context, this->config_file_path.c_str(), &num_input_tensors, &num_output_tensors, &inferenceMode, &num_cores, &this->config_checksum[0]);
     status = vxGetStatus((vx_reference)this->config);

     this->num_input_tensors  = num_input_tensors;
     this->num_output_tensors = num_output_tensors;
     this->inferenceMode = inferenceMode;
     this->coreNum = num_cores;

     /* Populate this->ioBufDesc so downstream modules (PreProc/PostProc)
      * can access it via getModuleByType() without a separate readConfigFile(). */
     if(status == VX_SUCCESS)
     {
         vx_map_id map_id_iobuf;
         tivxTIDLJ7Params *tidlParams_iobuf = NULL;
         vxMapUserDataObject(this->config, 0, sizeof(tivxTIDLJ7Params), &map_id_iobuf,
                             (void **)&tidlParams_iobuf, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);
         if(tidlParams_iobuf != NULL)
         {
             memcpy(&this->ioBufDesc, &tidlParams_iobuf->ioBufDesc, sizeof(sTIDL_IOBufDesc_t));
         }
         else
         {
             printf("[TIDL] ERROR: vxMapUserDataObject failed, this->ioBufDesc stays zeroed\n");
         }
         vxUnmapUserDataObject(this->config, map_id_iobuf);
     }
     else
     {
         printf("[TIDL] ERROR: readConfig failed (status=%d), this->ioBufDesc stays zeroed\n", status);
     }

     if(status == VX_SUCCESS)
     {
         this->network = this->readNetwork(context, this->network_file_path.c_str(), &this->network_checksum[0]);
         status = vxGetStatus((vx_reference)this->network);
     }

     if(status == VX_SUCCESS)
     {
        status = this->updateChecksums(this->config, &this->config_checksum[0], &this->network_checksum[0]);
     }

     if(status == VX_SUCCESS)
     {
         capacity = sizeof(TIDL_CreateParams);
         this->createParams = vxCreateUserDataObject(context, "TIDL_CreateParams", capacity, NULL );
         status = this->setCreateParams(context);
     }

     if(status == VX_SUCCESS)
     {
         vx_user_data_object inArgsObj;

         capacity = sizeof(TIDL_InArgs);
         inArgsObj = vxCreateUserDataObject(context, "TIDL_InArgs", capacity, NULL );
         this->in_args_arr  = vxCreateObjectArray(context, (vx_reference)inArgsObj, num_cameras_local);
         vxReleaseUserDataObject(&inArgsObj);

         vxSetReferenceName((vx_reference)this->in_args_arr, "tidl_node_in_args_arr");

         for(i = 0; i < num_cameras_local; i++)
         {
            vx_user_data_object inArgsItem;

            inArgsItem = (vx_user_data_object)vxGetObjectArrayItem(this->in_args_arr, i);
            this->setInArgs(context, inArgsItem);
            vxReleaseUserDataObject(&inArgsItem);
         }
     }

     if(status == VX_SUCCESS)
     {
         vx_user_data_object outArgsObj;

         capacity = sizeof(TIDL_outArgs);
         outArgsObj = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL );
         this->out_args_arr  = vxCreateObjectArray(context, (vx_reference)outArgsObj, num_cameras_local);
         vxReleaseUserDataObject(&outArgsObj);

         vxSetReferenceName((vx_reference)this->out_args_arr, "tidl_node_out_args_arr");

         for(i = 0; i < num_cameras_local; i++)
         {
            vx_user_data_object outArgsItem;

            outArgsItem = (vx_user_data_object)vxGetObjectArrayItem(this->out_args_arr, i);
            this->setOutArgs(context, outArgsItem);
            vxReleaseUserDataObject(&outArgsItem);
         }
     }

     for(i = 0; i < (vx_int32)APP_MODULES_MAX_TENSORS; i++)
     {
         this->output_tensor_arr[i] = NULL;
     }

     this->createOutputTensors(context, this->config, output_tensors_local);

     for(i = 0; i < (vx_int32)num_output_tensors; i++)
     {
         this->output_tensor_arr[i]  = vxCreateObjectArray(context, (vx_reference)output_tensors_local[i], num_cameras_local);
         vxSetReferenceName((vx_reference)this->output_tensor_arr[i], "tidl_node_output_tensor_arr");
         vxReleaseTensor(&output_tensors_local[i]);
     }

     /* Create XYCoordinate object arrays if enabled (for BEV pipelining) */
     if(this->enable_xycord && status == VX_SUCCESS)
     {
         vx_int32 bufq_depth = (this->bufq_depth > 0) ? this->bufq_depth : APP_MODULES_BUFFER_Q_DEPTH;

         vx_tensor xy_template = vxCreateTensor(context, 1, this->xycord_size, VX_TYPE_FLOAT32, 0);
         status = vxGetStatus((vx_reference)xy_template);

         if(status == VX_SUCCESS)
         {
             for (vx_int32 q = 0; q < bufq_depth && status == VX_SUCCESS; q++)
             {
                 this->input_xycord_arr[q] = vxCreateObjectArray(context, (vx_reference)xy_template, num_cameras_local);
                 status = vxGetStatus((vx_reference)this->input_xycord_arr[q]);
                 if(status == VX_SUCCESS)
                 {
                     std::string name = "tidl_xycord_arr_" + std::to_string(q);
                     vxSetReferenceName((vx_reference)this->input_xycord_arr[q], name.c_str());
                     /* Note: Don't extract/store tensors here - they get corrupted by other modules.
                      * App extracts them fresh during pipeline setup instead. */
                 }
             }

             if(status == VX_SUCCESS)
             {
                 /* Connect first XYCoordinate array to input[1] */
                 this->input_tensor_arr[1] = this->input_xycord_arr[0];
             }
             else
             {
                 printf("[TIDL] ERROR: Failed to create XYCoordinate arrays\n");
             }

             vxReleaseTensor(&xy_template);
         }
         else
         {
             printf("[TIDL] ERROR: Failed to create XYCoordinate template tensor\n");
         }
     }

     if(status == VX_SUCCESS)
     {
         this->kernel = tivxAddKernelTIDL(context, this->num_input_tensors, this->num_output_tensors);
         status = vxGetStatus((vx_reference)this->kernel);
     }

     snprintf(this->objName, APP_MODULES_MAX_OBJ_NAME_SIZE, "tidl_obj");

     return status;
}

vx_status TIDLObj::deinitModule(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_int32 i;

    vxReleaseUserDataObject(&this->config);
    vxReleaseUserDataObject(&this->network);
    vxReleaseUserDataObject(&this->createParams);

    vxReleaseObjectArray(&this->in_args_arr);
    vxReleaseObjectArray(&this->out_args_arr);

    for(i = 0; i < (vx_int32)this->num_output_tensors; i++)
    {
        vxReleaseObjectArray(&this->output_tensor_arr[i]);
    }

    /* Release XYCoordinate arrays if created
     * Note: input_xycord[] tensors are not released here - they're extracted fresh by the app
     * during pipeline setup and managed by the app. */
    if(this->enable_xycord)
    {
        vx_int32 bufq_depth = (this->bufq_depth > 0) ? this->bufq_depth : APP_MODULES_BUFFER_Q_DEPTH;
        for (int i = 0; i < bufq_depth; i++)
        {
            if(this->input_xycord_arr[i] != NULL)
            {
                vxReleaseObjectArray(&this->input_xycord_arr[i]);
            }
        }
    }

    return VX_SUCCESS;
}

vx_status TIDLObj::deleteNode(AppGraphObj *appGraph, vx_int32 module_index)
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

vx_status TIDLObj::createNode(AppGraphObj *appGraph, vx_int32 module_index)
{
    vx_graph graph_vx = appGraph->graph;
    vx_status status = VX_SUCCESS;

    vx_reference params[APP_MODULES_MAX_PARAMS];

    vx_tensor input_tensor_local[APP_MODULES_MAX_TENSORS];
    vx_tensor output_tensor_local[APP_MODULES_MAX_TENSORS];

    vx_int32 i;

    this->node = NULL;

    /* Initialize param array */
    vx_uint32 param_idx = 0;

    /* The 1st param MUST be config array */
    params[param_idx++] = (vx_reference)this->config;

    /* The 2nd param MUST be network tensor */
    params[param_idx++] = (vx_reference)this->network;

    /* The 3rd param MUST be create params */
    params[param_idx++] = (vx_reference)this->createParams;

    /* The 4th param MUST be inArgs */
    vx_user_data_object inArgsItem = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)this->in_args_arr, 0);
    params[param_idx++] = (vx_reference)inArgsItem;
    vxReleaseUserDataObject(&inArgsItem);

    /* The 5th param MUST be outArgs */
    vx_user_data_object outArgsItem = (vx_user_data_object)vxGetObjectArrayItem((vx_object_array)this->out_args_arr, 0);
    params[param_idx++] = (vx_reference)outArgsItem;
    vxReleaseUserDataObject(&outArgsItem);

    /* The 6th param MUST be NULL if trace data dump is not enabled */
    params[param_idx++] = NULL;

    /* Create TIDL Node */
    for(i = 0; i < (vx_int32)this->num_input_tensors; i++)
    {
        input_tensor_local[i]  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)this->input_tensor_arr[i], 0);
    }

    for(i = 0; i < (vx_int32)this->num_output_tensors; i++)
    {
        output_tensor_local[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)this->output_tensor_arr[i], 0);
    }

    this->node = tivxTIDLNode(graph_vx, this->kernel, params, input_tensor_local, output_tensor_local);
    status = vxGetStatus((vx_reference)this->node);
    vxSetReferenceName((vx_reference)this->node, "tidl_node");

    /* Validate batch processing parameters */
    #if defined(SOC_J784S4) || defined(SOC_J722S)
    if((this->coreNum < 1) || (this->coreNum > TIDL_MAX_NUM_CORES))
    {
        printf("[TIDL] ERROR: Invalid coreNum %d, please specify 1 <= coreNum <= %d\n", this->coreNum, TIDL_MAX_NUM_CORES);
        return VX_ERROR_INVALID_VALUE;
    }
    if((this->coreStartIdx < 1) || (this->coreStartIdx > TIDL_MAX_NUM_CORES-1))
    {
        printf("[TIDL] ERROR: Invalid coreStartIdx %d, please specify 1 <= coreStartIdx <= %d\n", this->coreStartIdx, TIDL_MAX_NUM_CORES-1);
        return VX_ERROR_INVALID_VALUE;
    }
    if(this->inferenceMode == TIDL_inferenceModeDefault)
    {
        if(this->coreStartIdx != 1)
        {
            printf("[TIDL] WARNING: coreStartIdx is not applicable for inferenceMode = 0\n");
        }
    }
    #else
    if(this->coreNum != 1U)
    {
        printf("[TIDL] ERROR: Invalid coreNum %d, expected coreNum is 1\n", this->coreNum);
        return VX_ERROR_INVALID_VALUE;
    }
    if(this->coreStartIdx != 1)
    {
        printf("[TIDL] ERROR: Invalid coreStartIdx %d, expected coreStartIdx is 1\n", this->coreStartIdx);
        return VX_ERROR_INVALID_VALUE;
    }
    #endif

    /* Set node target based on inference mode (batch/high-throughput uses MPU) */
    #if defined(SOC_J784S4) || defined(SOC_J722S)
    const char* mpuTargets[] = {TIVX_TARGET_MPU_1, TIVX_TARGET_MPU_2, TIVX_TARGET_MPU_3};

    if((this->inferenceMode == TIDL_inferenceModeHighThroughput) || (this->inferenceMode == TIDL_inferenceModeLowLatency))
    {
        vxSetNodeTarget(this->node, VX_TARGET_STRING, mpuTargets[0]);
    }
    else /* Default mode - use DSP targets */
    {
        vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);

        #if defined(SOC_J784S4)
        if (this->core_id == 1)
        {
            vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_2);
        }
        else if (this->core_id == 2)
        {
            vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_3);
        }
        else if (this->core_id == 3)
        {
            vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_4);
        }
        #elif defined(SOC_J722S)
        if (this->core_id == 1)
        {
            vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_2);
        }
        #endif
    }
    #else
    vxSetNodeTarget(this->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);
    #endif

    if(this->inferenceMode == TIDL_inferenceModeDefault && !this->enable_xycord)
    {
        vx_bool replicate[16];
        replicate[TIVX_KERNEL_TIDL_IN_CONFIG_IDX] = vx_false_e;
        replicate[TIVX_KERNEL_TIDL_IN_NETWORK_IDX] = vx_false_e;
        replicate[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX] = vx_false_e;
        replicate[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX] = vx_true_e;
        replicate[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX] = vx_true_e;
        replicate[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX] = vx_false_e;

        for(i = 0; i < (vx_int32)this->num_input_tensors; i++)
        {
          replicate[TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + i] = vx_true_e;
        }

        for(i = 0; i < (vx_int32)this->num_output_tensors; i++)
        {
          replicate[TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + this->num_input_tensors + i] = vx_true_e;
        }

        vxReplicateNode(graph_vx, this->node, replicate, TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + this->num_input_tensors + this->num_output_tensors);
    }

    for(i = 0; i < (vx_int32)this->num_input_tensors; i++)
    {
        vxReleaseTensor(&input_tensor_local[i]);
    }

    for(i = 0; i < (vx_int32)this->num_output_tensors; i++)
    {
        vxReleaseTensor(&output_tensor_local[i]);
    }

    return status;
}

vx_status TIDLObj::writeOutput(char *file_name)
{
    vx_status status = VX_SUCCESS;

    vx_tensor output;
    vx_size numCh;
    vx_int32 ch, tensor_id;

    vx_map_id map_id_config;
    sTIDL_IOBufDesc_t *ioBufDesc;
    tivxTIDLJ7Params *tidlParams;

    vxMapUserDataObject(this->config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                    (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;

    for(tensor_id = 0; tensor_id < (vx_int32)this->num_output_tensors; tensor_id++)
    {
        vxQueryObjectArray((vx_object_array)this->output_tensor_arr[tensor_id], VX_OBJECT_ARRAY_NUMITEMS, &numCh, sizeof(vx_size));

        for(ch = 0; ch < (vx_int32)numCh; ch++)
        {
            vx_size num_dims;
            void *data_ptr;
            vx_map_id map_id;

            vx_size    start[APP_MODULES_MAX_TENSOR_DIMS];
            vx_size    tensor_strides[APP_MODULES_MAX_TENSOR_DIMS];
            vx_size    tensor_sizes[APP_MODULES_MAX_TENSOR_DIMS];

            output  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)this->output_tensor_arr[tensor_id], ch);

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

                for(k = 0; k < (vx_int32)tensor_sizes[2]; k++)
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

  vxUnmapUserDataObject(this->config, map_id_config);

  return(status);
}

vx_user_data_object TIDLObj::readConfig(vx_context context, const char *config_file, vx_uint32 *num_input_tensors, vx_uint32 *num_output_tensors, vx_uint32 *inferenceMode, vx_uint32 *num_cores, vx_uint8 *check_sum)
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
            *inferenceMode = ioBufDesc->inferenceMode;
            *num_cores = ioBufDesc->numCores;

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


vx_user_data_object TIDLObj::readNetwork(vx_context context, const char *network_file, vx_uint8 *check_sum)
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

vx_status TIDLObj::updateChecksums(vx_user_data_object config, vx_uint8 *config_checksum, vx_uint8 *network_checksum)
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
vx_status TIDLObj::setCreateParams(vx_context context)
{
    vx_status status = VX_SUCCESS;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *createParams_buffer = NULL;

    status = vxGetStatus((vx_reference)this->createParams);

    if(VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)this->createParams, "tidl_node_createParams");

        capacity = sizeof(TIDL_CreateParams);
        vxMapUserDataObject(this->createParams, 0, capacity, &map_id,
              (void **)&createParams_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if(createParams_buffer)
        {
            TIDL_CreateParams *prms = (TIDL_CreateParams *)createParams_buffer;
            TIDL_createParamsInit(prms);

            /* Batch mode (BEV): any non-default inferenceMode uses coreNum/coreStartIdx */
            if(this->inferenceMode != TIDL_inferenceModeDefault)
            {
                prms->coreId       = this->coreNum - 1;
                prms->coreStartIdx = this->coreStartIdx - 1;
            }
            else
            {
                prms->coreId = this->core_id;
            }

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

        vxUnmapUserDataObject(this->createParams, map_id);
    }

    return status;
}

vx_status TIDLObj::setInArgs(vx_context context, vx_user_data_object inArgs)
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
            TIDL_InArgs *prms = (TIDL_InArgs *)inArgs_buffer;
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

vx_status TIDLObj::setOutArgs(vx_context context, vx_user_data_object outArgs)
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
            TIDL_outArgs *prms = (TIDL_outArgs *)outArgs_buffer;
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

void TIDLObj::createOutputTensors(vx_context context, vx_user_data_object config, vx_tensor output_tensors[])
{
    vx_size output_sizes[APP_MODULES_MAX_TENSOR_DIMS];
    vx_map_id map_id_config;

    vx_uint32 id;

    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    for(id = 0; id < (vx_uint32)ioBufDesc->numOutputBuf; id++) {
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
