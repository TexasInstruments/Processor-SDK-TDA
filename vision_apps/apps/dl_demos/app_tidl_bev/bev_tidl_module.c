/*
 *
 * Copyright (c) 2025-2026 Texas Instruments Incorporated
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
#include "bev_tidl_module.h"

#undef COMPUTE_CHECKSUM

static vx_user_data_object readConfig(vx_context context, vx_char *config_file, uint32_t *num_input_tensors, uint32_t *num_output_tensors, uint32_t *inferenceMode, uint32_t *num_cores, vx_uint8 *check_sum);
static vx_user_data_object readNetwork(vx_context context, vx_char *network_file, vx_uint8 *check_sum);
static vx_status setCreateParams(vx_context context, TIDLObj *tidlObj);
static vx_status setCreateParams_batch(vx_context context, vx_user_data_object createParams,vx_uint32 coreNum,vx_int32 coreStartIdx);
static vx_status setInArgs(vx_context context, vx_user_data_object inArgs);
static vx_status setOutArgs(vx_context context, vx_user_data_object outArgs);
static void createOutputTensors(vx_context context, vx_user_data_object config, vx_tensor output_tensors[]);
static void initParam(vx_reference params[], uint32_t _max_params);
static void addParam(vx_reference params[], vx_reference obj);
#ifdef COMPUTE_CHECKSUM
static void getQC(uint8_t *pIn, uint8_t *pOut, int32_t inSize);
#endif
static vx_status updateChecksums(vx_user_data_object config, vx_uint8 *config_checksum, vx_uint8 *network_checksum);

static uint32_t num_params;
static uint32_t max_params;

vx_status app_init_tidl_bev(vx_context context, TIDLObj *tidlObj, char *objName, vx_int32 bufq_depth)
{
    vx_status status = VX_SUCCESS;

    vx_uint32 num_input_tensors = 0;
    vx_uint32 num_output_tensors = 0;

    vx_tensor output_tensors[APP_MAX_TENSORS];
    vx_uint32 inferenceMode = TIDL_inferenceModeDefault;
    vx_uint32 num_cores = 1;
    vx_uint32 capacity;

    tidlObj->kernel = NULL;

    tidlObj->config = readConfig(context, &tidlObj->config_file_path[0], &num_input_tensors, &num_output_tensors, &inferenceMode, &num_cores, &tidlObj->config_checksum[0]);
    status = vxGetStatus((vx_reference)tidlObj->config);

    tidlObj->num_input_tensors  = num_input_tensors;
    tidlObj->num_output_tensors = num_output_tensors;
    tidlObj->inferenceMode = inferenceMode;
    
    tidlObj->coreNum = num_cores;
    tidlObj->coreStartIdx = 1;

    if ((tidlObj->num_input_tensors > APP_MAX_TENSORS) ||
        (tidlObj->num_output_tensors> APP_MAX_TENSORS))
    {
        status = VX_FAILURE;
    }

    if(status == VX_SUCCESS)
    {
        tidlObj->network = readNetwork(context, &tidlObj->network_file_path[0], &tidlObj->network_checksum[0]);
        status = vxGetStatus((vx_reference)tidlObj->network);
    }
    vx_tensor input_tensor=vxCreateTensor(context, 1, tidlObj->xycord_size, VX_TYPE_INT32 , 0 );
    vx_uint32 q;
    for(q = 0; q < bufq_depth; q++)
    {
        tidlObj->input_xycord_arr[q]   = vxCreateObjectArray(context, (vx_reference)input_tensor, NUM_CH);
        status = vxGetStatus((vx_reference)tidlObj->input_xycord_arr[q]);
        if(status != VX_SUCCESS)
        {
            printf("FAilure in creating input object Array index--%d \n",q);
        }
        tidlObj->input_xycord[q]  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)tidlObj->input_xycord_arr[q], 0);
    }
    vxReleaseTensor(&input_tensor);


    if(status == VX_SUCCESS)
    {
        status = updateChecksums(tidlObj->config, &tidlObj->config_checksum[0], &tidlObj->network_checksum[0]);

        if(status != VX_SUCCESS)
        {
            printf("Update Checksums failed\n");
        }
    }

    if(status == VX_SUCCESS)
     {
         capacity = sizeof(TIDL_CreateParams);
         tidlObj->createParams = vxCreateUserDataObject(context, "TIDL_CreateParams", capacity, NULL );
         if(tidlObj->inferenceMode ==0)
         {
            status = setCreateParams(context, tidlObj);
         }
         if(tidlObj->inferenceMode !=0)
         {
            status = setCreateParams_batch(context, tidlObj->createParams, tidlObj->coreNum, tidlObj->coreStartIdx);
         }
         
     }

    if(status == VX_SUCCESS)
    {
        vx_user_data_object inArgs;
        vx_int32 i;

        capacity = sizeof(TIDL_InArgs);
        inArgs = vxCreateUserDataObject(context, "TIDL_InArgs", capacity, NULL );
        tidlObj->in_args_arr  = vxCreateObjectArray(context, (vx_reference)inArgs, NUM_CH);
        vxReleaseUserDataObject(&inArgs);

        for(i = 0; i < NUM_CH; i++)
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
        vx_int32 i;

        capacity = sizeof(TIDL_outArgs);
        outArgs = vxCreateUserDataObject(context, "TIDL_outArgs", capacity, NULL );
        tidlObj->out_args_arr  = vxCreateObjectArray(context, (vx_reference)outArgs, NUM_CH);
        vxReleaseUserDataObject(&outArgs);

        for(i = 0; i < NUM_CH; i++)
        {
            vx_user_data_object outArgs;

            outArgs = (vx_user_data_object)vxGetObjectArrayItem(tidlObj->out_args_arr, i);
            setOutArgs(context, outArgs);
            vxReleaseUserDataObject(&outArgs);
        }
    }

    if(status == VX_SUCCESS)
    {
        vx_int32 i;
        createOutputTensors(context, tidlObj->config, output_tensors);
        tidlObj->output1_tensor_arr  = vxCreateObjectArray(context, (vx_reference)output_tensors[0], NUM_CH);

        for(i = 0; i < num_output_tensors; i++)
        {
            tidlObj->output_tensor_arr_batch[i]  = vxCreateObjectArray(context, (vx_reference)output_tensors[i], NUM_CH);
            vxSetReferenceName((vx_reference)tidlObj->output_tensor_arr_batch[i], "tidl_node_output_tensor_arr_batch");
            vxReleaseTensor(&output_tensors[i]);
        }
        

        tidlObj->kernel = tivxAddKernelTIDL(context, tidlObj->num_input_tensors, tidlObj->num_output_tensors);
        status = vxGetStatus((vx_reference)tidlObj->kernel);
    }

    if(status == VX_SUCCESS)
    {
        strcpy(tidlObj->objName, objName);
    }

    return status;
}

void app_deinit_tidl_bev(TIDLObj *tidlObj, vx_int32 bufq_depth)
{
    vxReleaseUserDataObject(&tidlObj->config);
    vxReleaseUserDataObject(&tidlObj->network);
    vxReleaseUserDataObject(&tidlObj->createParams);

    vx_uint32 q;
    for(q = 0; q < bufq_depth; q++)
    {
        vxReleaseObjectArray(&tidlObj->input_xycord_arr[q]);
        vxReleaseTensor(&tidlObj->input_xycord[q]);
    }
    vxReleaseObjectArray(&tidlObj->in_args_arr);
    vxReleaseObjectArray(&tidlObj->out_args_arr);

    vxReleaseObjectArray(&tidlObj->output1_tensor_arr);
    vx_int32 i;
    for(i = 0; i < tidlObj->num_output_tensors; i++)
    {
        vxReleaseObjectArray(&tidlObj->output_tensor_arr_batch[i]);
    }
}

void app_delete_tidl_bev(TIDLObj *tidlObj)
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

vx_status app_create_graph_tidl_4D(vx_context context, vx_graph graph, TIDLObj *tidlObj, vx_object_array input_tensor_arr1)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    vx_reference params[APP_TIDL_MAX_PARAMS];

    vx_tensor input_tensor[APP_MAX_TENSORS];

    vx_tensor output_tensor1[APP_MAX_TENSORS];

    tidlObj->node = NULL;

    /* Initialize param array */
    initParam(params, APP_TIDL_MAX_PARAMS);

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
    input_tensor[0]  = (vx_tensor)vxGetObjectArrayItem((vx_object_array)input_tensor_arr1, 0); 
    input_tensor[1] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)tidlObj->input_xycord_arr[0],0);
    for(i = 0; i < tidlObj->num_output_tensors; i++)
    {
        output_tensor1[i] = (vx_tensor)vxGetObjectArrayItem((vx_object_array)tidlObj->output_tensor_arr_batch[i], 0);
    }

    tidlObj->node = tivxTIDLNode(graph, tidlObj->kernel, params, input_tensor,output_tensor1);
    status = vxGetStatus((vx_reference)tidlObj->node);
    vxSetReferenceName((vx_reference)tidlObj->node, "BEVTIDLNode");
    
    #if defined(SOC_J784S4) || defined(SOC_J722S) || defined (SOC_J742S2)
    const char* mpuTargets[] = {
        TIVX_TARGET_MPU_1,
        TIVX_TARGET_MPU_2,
        TIVX_TARGET_MPU_3,
    };
    if((tidlObj->coreNum < 1) || (tidlObj->coreNum > TIDL_MAX_NUM_CORES))
    {
        printf("TIDL_RT_OVX: ERROR: Invalid core number specified - %d , please specify 1 <= coreNum <= %d\n", tidlObj->coreNum, TIDL_MAX_NUM_CORES);
        return VX_ERROR_INVALID_VALUE;
    }
    if((tidlObj->coreStartIdx < 1) || (tidlObj->coreStartIdx > TIDL_MAX_NUM_CORES-1))
    {
        printf("TIDL_RT_OVX: ERROR: Invalid coreStartIdx specified - %d , please specify 0 <= coreStartIdx <= %d\n", tidlObj->coreStartIdx, TIDL_MAX_NUM_CORES-1);
        return VX_ERROR_INVALID_VALUE;
    }
    if(tidlObj->inferenceMode == TIDL_inferenceModeDefault)
    {
        if(tidlObj->coreStartIdx != 1)
        {
            printf("TIDL_RT_OVX: WARNING : coreStartIdx is not applicable for inferenceMode = 0, please use coreNum to specify desired core for inference \n");
        }
    }
    if((tidlObj->inferenceMode == TIDL_inferenceModeHighThroughput) || (tidlObj->inferenceMode == TIDL_inferenceModeLowLatency))
    {
        APP_PRINTF("Assigning TIDL NOde Target to MPU as it is HighThrouput MOde\n");
      vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, mpuTargets[0]); /* setting target to mpu_1 */
    }
    else /* default mode - set DSP targets */
    {
      vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);
    }
    #else
    if(tidlObj->coreNum != 1U)
    {
        printf("TIDL_RT_OVX: ERROR: Invalid core number specified - %d , expected core number is 1\n", tidlObj->coreNum);
        return VX_ERROR_INVALID_VALUE;
    }
    if(tidlObj->coreStartIdx != 1)
    {
        printf("TIDL_RT_OVX: ERROR: Invalid coreStartIdx specified - %d , expected coreStartIdx is 1\n", tidlObj->coreStartIdx);
        return VX_ERROR_INVALID_VALUE;
    }
    vxSetNodeTarget(tidlObj->node, VX_TARGET_STRING, TIVX_TARGET_DSP_C7_1);
    #endif

    for(i = 0; i < tidlObj->num_input_tensors; i++)
    {
        vxReleaseTensor(&input_tensor[i]);
    }


    for(i = 0; i < tidlObj->num_output_tensors; i++)
    {
        vxReleaseTensor(&output_tensor1[i]);
    }
    return status;
}

static vx_user_data_object readConfig(vx_context context, vx_char *config_file,  vx_uint32 *num_input_tensors, vx_uint32 *num_output_tensors,vx_uint32 *inferenceMode, vx_uint32 *num_cores , vx_uint8 *check_sum)
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
                printf("Unable to read file!\n");
            }

            *num_input_tensors  = ioBufDesc->numInputBuf;
            *num_output_tensors = ioBufDesc->numOutputBuf;
            *inferenceMode = ioBufDesc->inferenceMode;
            *num_cores = ioBufDesc->numCores;

            APP_PRINTF("ioBufDesc->numVirtualCores--%d  ioBufDesc->num_superbatches -%d ioBufDesc->num_cores-%d \n ", ioBufDesc->numVirtualCores,ioBufDesc->numSuperBatches,ioBufDesc->numCores);
            APP_PRINTF("ioBufDesc->numInputBuf--%d  ioBufDesc->numOutputBuf -%d  \n ", ioBufDesc->numInputBuf,ioBufDesc->numOutputBuf);
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
            uint8_t *pPerfInfo = (uint8_t *)network_buffer + pNet->dataFlowInfo;
            printf("Computing checksum at 0x%016lX, size = %d\n", (uint64_t)pPerfInfo,  capacity - pNet->dataFlowInfo);
            getQC(pPerfInfo, check_sum, capacity - pNet->dataFlowInfo);
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
        }
        else
        {
            printf("Unable to allocate memory for create time params! %d bytes\n", capacity);
        }

        vxUnmapUserDataObject(tidlObj->createParams, map_id);
    }

    return status;
}

static vx_status setCreateParams_batch(vx_context context, vx_user_data_object createParams, vx_uint32 coreNum,vx_int32 coreStartIdx)
{
    vx_status status = VX_SUCCESS;
    vx_map_id  map_id;
    vx_uint32  capacity;
    void *createParams_buffer = NULL;

    status = vxGetStatus((vx_reference)createParams);

    if(VX_SUCCESS == status)
    {
        vxSetReferenceName((vx_reference)createParams, "tidl_node_createParams");

        capacity = sizeof(TIDL_CreateParams);
        vxMapUserDataObject(createParams, 0, capacity, &map_id,
              (void **)&createParams_buffer, VX_WRITE_ONLY, VX_MEMORY_TYPE_HOST, 0);

        if(createParams_buffer)
        {
            TIDL_CreateParams *prms = createParams_buffer;
            //write create params here
            TIDL_createParamsInit(prms);

            prms->isInbufsPaded                 = 1;
            prms->quantRangeExpansionFactor     = 1.0;
            prms->quantRangeUpdateFactor        = 0.0;
            prms->traceLogLevel                 = 0;
            prms->traceWriteLevel               = 0;
            // prms->coreId                     = 3;
            /*4D BEV Modfication*/
            prms->coreId                        = coreNum -1 ;
            prms->coreStartIdx                  = coreStartIdx -1 ;
        }
        else
        {
            printf("Unable to allocate memory for create time params! %d bytes\n", capacity);
        }

        vxUnmapUserDataObject(createParams, map_id);
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
    vx_size output_sizes[APP_MAX_TENSOR_DIMS];
    vx_map_id map_id_config;

    vx_uint32 id;

    tivxTIDLJ7Params *tidlParams;
    sTIDL_IOBufDesc_t *ioBufDesc;

    vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                      (void **)&tidlParams, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

    ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlParams->ioBufDesc;
    for(id = 0; id < ioBufDesc->numOutputBuf; id++)
    {
        output_sizes[0] = ioBufDesc->outWidth[id]  + ioBufDesc->outPadL[id] + ioBufDesc->outPadR[id];
        output_sizes[1] = ioBufDesc->outHeight[id] + ioBufDesc->outPadT[id] + ioBufDesc->outPadB[id];
        output_sizes[2] = ioBufDesc->outNumChannels[id];
        /*BEV Detections*/
        vx_enum data_type = get_vx_tensor_datatype(ioBufDesc->outElementType[id]);
        /*############################################          MODIFICATION FOR BEV*/
        if((ioBufDesc->outNumBatches[id]) ==1)
        {
            output_tensors[id] = vxCreateTensor(context, 3, output_sizes, data_type, 0);
        }
        else{
            /*############################################           MODIFICATION FOR BEV*/
            output_sizes[2] = (ioBufDesc->outNumChannels[id] + ioBufDesc->outPadCh[id]) * ioBufDesc->outDIM1[id] * ioBufDesc->outDIM2[id];
            output_sizes[3] = ioBufDesc->outNumBatches[id]; 
            output_tensors[id] = vxCreateTensor(context, 4 , output_sizes, data_type, 0);
            /*###############################################*/
        }
    }

    vxUnmapUserDataObject(config, map_id_config);

    return;
}

static void initParam(vx_reference params[], uint32_t _max_params)
{
    num_params  = 0;
    max_params = _max_params;
}

static void addParam(vx_reference params[], vx_reference obj)
{
    APP_ASSERT(num_params <= max_params);

    params[num_params] = obj;
    num_params++;
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

        if ((remSize - TIVX_TIDL_J7_CHECKSUM_SIZE) < 0)
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
