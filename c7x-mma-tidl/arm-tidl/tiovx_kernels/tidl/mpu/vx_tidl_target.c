/*
*
* Copyright (c) 2023-2025 Texas Instruments Incorporated
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

#include <stdio.h>
#include <string.h>
#include <TI/tivx.h>
#include <TI/tivx_config.h>
#include <TI/dl_kernels.h>
#include <TI/j7_tidl.h>
#include <TI/tivx_target_kernel.h>
#include <tivx_alg_ivision_if.h>
#include "itidl_ti.h"
#include "tivx_tidl_trace.h"
#include "tivx_kernels_target_utils.h"
#include "app_fileio.h"
#include "tivx_tidl_kernels.h"
#include "tidl_rt_force_negative_test.h"

/* #define TIVX_TIDL_MPU_TARGET_DEBUG */


typedef struct
{
    vx_user_data_object tidlNodeParams[TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS];
    vx_tensor  inputTensors[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_IN_BUFS];
    vx_tensor  outputTensors[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_OUT_BUFS];
    vx_object_array  input_tensor_list[TIDL_MAX_NUM_CORES];
    vx_object_array  output_tensor_list[TIDL_MAX_NUM_CORES];
    vx_user_data_object outArgs[TIDL_MAX_NUM_CORES];

    uint32_t num_input_tensors;
    uint32_t num_output_tensors;
    uint32_t num_virtual_cores;
    uint32_t num_cores;
    int32_t  core_start_idx;
    uint32_t num_super_batches;
    uint32_t inference_mode;
    uint32_t trace_log_level;

    vx_graph graph;
    vx_node node[TIDL_MAX_NUM_CORES];
    vx_kernel kernel;
    int32_t forceNegativeTest;
    void     *addrInput[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_IN_BUFS][MAX_TENSOR_DIMS];
    uint32_t sizeInput[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_IN_BUFS][MAX_TENSOR_DIMS];
    uint32_t numEntriesInput[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_IN_BUFS];

    void     *addrOutput[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_OUT_BUFS][MAX_TENSOR_DIMS];
    uint32_t sizeOutput[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_OUT_BUFS][MAX_TENSOR_DIMS];
    uint32_t numEntriesOutput[TIDL_MAX_NUM_CORES][TIDL_MAX_ALG_OUT_BUFS];

} tivxTIDLNestedKernelObj;

/* Max Targets in current target to be registered */
#define TIDL_MAX_TARGETS (3U)

/* Target names used to register current target */
static char target_name[TIDL_MAX_TARGETS][TIVX_TARGET_MAX_NAME] =
{
    TIVX_TARGET_MPU_1,
    TIVX_TARGET_MPU_2,
    TIVX_TARGET_MPU_3,
};

/* Target names for nodes created on current target */
static const char* child_target_name[] = {
            TIVX_TARGET_DSP_C7_1_PRI_1,
            TIVX_TARGET_DSP_C7_1_PRI_2,
            TIVX_TARGET_DSP_C7_1_PRI_3,
            TIVX_TARGET_DSP_C7_1_PRI_4,
            TIVX_TARGET_DSP_C7_1_PRI_5,
            TIVX_TARGET_DSP_C7_1_PRI_6,
            TIVX_TARGET_DSP_C7_1_PRI_7,
            TIVX_TARGET_DSP_C7_1_PRI_8,
#if defined(SOC_J784S4) || defined (SOC_J722S) || defined (SOC_TDA54)  || defined(SOC_J742S2)
            TIVX_TARGET_DSP_C7_2_PRI_1,
            TIVX_TARGET_DSP_C7_2_PRI_2,
            TIVX_TARGET_DSP_C7_2_PRI_3,
            TIVX_TARGET_DSP_C7_2_PRI_4,
            TIVX_TARGET_DSP_C7_2_PRI_5,
            TIVX_TARGET_DSP_C7_2_PRI_6,
            TIVX_TARGET_DSP_C7_2_PRI_7,
            TIVX_TARGET_DSP_C7_2_PRI_8,
#endif
#if defined(SOC_J784S4)
            TIVX_TARGET_DSP_C7_3_PRI_1,
            TIVX_TARGET_DSP_C7_3_PRI_2,
            TIVX_TARGET_DSP_C7_3_PRI_3,
            TIVX_TARGET_DSP_C7_3_PRI_4,
            TIVX_TARGET_DSP_C7_3_PRI_5,
            TIVX_TARGET_DSP_C7_3_PRI_6,
            TIVX_TARGET_DSP_C7_3_PRI_7,
            TIVX_TARGET_DSP_C7_3_PRI_8,
            TIVX_TARGET_DSP_C7_4_PRI_1,
            TIVX_TARGET_DSP_C7_4_PRI_2,
            TIVX_TARGET_DSP_C7_4_PRI_3,
            TIVX_TARGET_DSP_C7_4_PRI_4,
            TIVX_TARGET_DSP_C7_4_PRI_5,
            TIVX_TARGET_DSP_C7_4_PRI_6,
            TIVX_TARGET_DSP_C7_4_PRI_7,
            TIVX_TARGET_DSP_C7_4_PRI_8,
#endif
    };

static tivx_target_kernel vx_tidl_nested_target_kernel[TIDL_MAX_TARGETS] = {NULL};

/* OpenVX Node callbacks */
static vx_status VX_CALLBACK tivxKernelTIDLCreate(tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLProcess(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLDelete(tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxKernelTIDLControl(tivx_target_kernel_instance kernel,
    uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[], uint16_t num_params, void *priv_arg);
#if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
static vx_object_array createObjArray(vx_context context, vx_tensor *tensors, uint32_t num_tensors);
#endif
static vx_status copyTensorHandles(vx_tensor src, vx_tensor dst);
static vx_status removeTensorHandles(vx_tensor tensor);

/* Copy data pointer of the source vx_tensor data object to destination vx_tensor object*/
static vx_status copyTensorHandles(vx_tensor src, vx_tensor dst)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void            *addr[MAX_TENSOR_DIMS] = {NULL};
    uint32_t        size[MAX_TENSOR_DIMS];
    uint32_t        num_entries;

    status = tivxReferenceExportHandle((vx_reference)src,
                                         addr,
                                         size,
                                         MAX_TENSOR_DIMS,
                                         &num_entries);

    if(status == (vx_status)VX_SUCCESS)
    {
        status = tivxReferenceImportHandle((vx_reference)dst,
        (const void **)addr,
        (const uint32_t *)size,
        num_entries);
    }

    return status;
}

static vx_status removeTensorHandles(vx_tensor tensor)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void            *addr[MAX_TENSOR_DIMS] = {NULL};
    uint32_t        size[MAX_TENSOR_DIMS];
    uint32_t        num_entries;

    status = tivxReferenceExportHandle((vx_reference)tensor,
                                         addr,
                                         size,
                                         MAX_TENSOR_DIMS,
                                         &num_entries);

    for (uint32_t i = 0U; i < num_entries; i++)
    {
        addr[i] = NULL;
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        status = tivxReferenceImportHandle((vx_reference)tensor,
        (const void **)addr,
        (const uint32_t *)size,
        num_entries);
    }

    return status;
}

#if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
static vx_object_array createObjArray(vx_context context, vx_tensor *tensors, uint32_t num_tensors)
{
    uint32_t tensorIndex = 0U ;
    vx_reference refList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
    for(tensorIndex = 0; tensorIndex < num_tensors ; tensorIndex++){
        refList[tensorIndex] = (vx_reference)tensors[tensorIndex];
    }
    return tivxCreateObjectArrayFromList(context, refList, num_tensors);
}
#endif

/* Updates outArgs performance profile points to report MAX latency across all cores */
static vx_status tivxUpdateOutArgs(tivxTIDLNestedKernelObj * prms, tivx_obj_desc_t *obj_desc[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_context context = vxCreateContext();
    vx_user_data_object outArgs = (vx_user_data_object)obj_desc[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX]->host_ref;

    /* Map copy of outArgs */
    vx_map_id map_id_out_args, map_id_prms_out_args;
    TIDL_outArgs *outArgsBuffer = NULL, *prmsOutArgsBuffer = NULL;
    status = vxMapUserDataObject(outArgs, 0, sizeof(TIDL_outArgs), &map_id_out_args,
                                    (void **)&outArgsBuffer,  (vx_enum)VX_READ_ONLY,  (vx_enum)VX_MEMORY_TYPE_HOST, 0);

    if(status == (vx_status)VX_SUCCESS)
    {
        (void)memset(outArgsBuffer, 0, sizeof(TIDL_outArgs)); /* Initialize all data values in outArgs to 0 */

        /*
         * outArgs will be different for different cores. Most of the values
         * in outArgs will be same across cores except the profile points.
         * For profile points, we take max for each layer across each cores.
         * If profile points is not enabled, we simply copy the outArgs from
         * first core.
         */
        if (prms->trace_log_level <= 0U)
        {
            status = vxMapUserDataObject(prms->outArgs[0], 0, sizeof(TIDL_outArgs), &map_id_prms_out_args,
                                         (void **)&prmsOutArgsBuffer,  (vx_enum)VX_READ_ONLY,  (vx_enum)VX_MEMORY_TYPE_HOST, 0);

            if(status == (vx_status)VX_SUCCESS)
            {
                /* outArgs from RT application obj_desc is not populated yet, so copy over first DSP node's outArgs, and then update profile points later based on all cores */
                (void)memcpy(outArgsBuffer, prmsOutArgsBuffer, sizeof(TIDL_outArgs));
            }

            (void)vxUnmapUserDataObject(prms->outArgs[0], map_id_prms_out_args);
        }
        else
        {
            for(int i = 0; i < (int)prms->num_cores; i++)
            {
                if(status == (vx_status)VX_SUCCESS)
                {
                    status = vxMapUserDataObject(prms->outArgs[i], 0, sizeof(TIDL_outArgs), &map_id_prms_out_args,
                                                 (void **)&prmsOutArgsBuffer,  (vx_enum)VX_READ_ONLY,  (vx_enum)VX_MEMORY_TYPE_HOST, 0);
                }
                if(status == (vx_status)VX_SUCCESS)
                {
                    if(i == 0)
                    {
                        /* outArgs from RT application obj_desc is not populated yet, so copy over first DSP node's outArgs, and then update profile points later based on all cores */
                        (void)memcpy(outArgsBuffer, prmsOutArgsBuffer, sizeof(TIDL_outArgs));
                    }

                    for(int j = 0; j < prmsOutArgsBuffer->numLayers; j++)
                    {
                        for(int k = 0; k < TIDL_PROFILE_MAX; k++)
                        {
                            /* MAX of profile points across all cores */
                            outArgsBuffer->metaDataLayer[j].profilePoint[k] =
                                (outArgsBuffer->metaDataLayer[j].profilePoint[k] > prmsOutArgsBuffer->metaDataLayer[j].profilePoint[k])
                                ?  outArgsBuffer->metaDataLayer[j].profilePoint[k] : prmsOutArgsBuffer->metaDataLayer[j].profilePoint[k];
                        }
                    }

                    (void)vxUnmapUserDataObject(prms->outArgs[i], map_id_prms_out_args);
                }
            }
        }

        (void)vxUnmapUserDataObject(outArgs, map_id_out_args);
    }

    (void)vxReleaseContext(&context);
    return status;
}

static vx_status VX_CALLBACK tivxKernelTIDLProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    uint16_t num_params,
    void *priv_arg
)
{
    vx_status status = (vx_status)VX_SUCCESS;

    VX_PRINT(VX_ZONE_INFO, "MPU_1 tivxKernelTIDLProcess - numParams = %d \n", num_params);

    tivxTIDLNestedKernelObj *prms = NULL;

    uint32_t size;

    status = tivxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if (((vx_status)VX_SUCCESS != status) || (NULL == prms) || (sizeof(tivxTIDLNestedKernelObj) != size))
    {
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if(status == (vx_status)VX_SUCCESS)
    {
        /* LDRA_JUSTIFY_END */
        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_PROC1_FNT)
        {
          prms->inference_mode =  3 ; // invalid inference mode
        }
        if(prms->inference_mode == (uint32_t)TIDL_inferenceModeHighThroughput)
        {
            uint32_t numBatches = prms->num_virtual_cores * prms->num_super_batches;
            uint32_t numInputsPerCore = prms->num_input_tensors/numBatches;
            uint32_t numOutputsPerCore = prms->num_output_tensors/numBatches;
            tivx_obj_desc_t *objDescTensors[(TIDL_MAX_ALG_IN_BUFS + TIDL_MAX_ALG_OUT_BUFS)] = {NULL};
            if(obj_desc[TIVX_KERNEL_TIDL_IN_FIRST_TENSOR]->type == (uint16_t)TIVX_OBJ_DESC_OBJARRAY)
            {
                tivx_obj_desc_object_array_t * inputArrayDesc;
                tivx_obj_desc_object_array_t * outputArrayDesc;
                tivx_obj_desc_t *inputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
                tivx_obj_desc_t *outputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
                uint32_t inputElementSize, outputElementSize;


                inputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_INPUT_IDX];
                outputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_OUTPUT_IDX];
                inputElementSize = inputArrayDesc->num_items;
                outputElementSize = outputArrayDesc->num_items;

                tivxGetObjDescList(inputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)inputTensorDescList, inputElementSize);
                tivxGetObjDescList(outputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)outputTensorDescList, outputElementSize);

                /* Extracting input tensors */ 
                for(uint32_t i = 0U; i < prms->num_input_tensors; i++)
                {
                    objDescTensors[i] = inputTensorDescList[i];
                }
                /* Extracting output tensors */ 
                for(uint32_t i = 0U; i < prms->num_output_tensors; i++)
                {
                    objDescTensors[(i+prms->num_input_tensors)] = outputTensorDescList[i];
                }
            }
            for(uint32_t k = 0U; k < prms->num_super_batches; k++)
            {
                for(uint32_t j = 0U; j <  prms->num_virtual_cores; j++)
                {
                    if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_PROC2_FNT)
                    {
                       prms->inputTensors[0][0] =  NULL ;
                    }
                    for(uint32_t i = 0U; i < numInputsPerCore; i++)
                    {
                        if(obj_desc[TIVX_KERNEL_TIDL_IN_FIRST_TENSOR]->type == (uint16_t)TIVX_OBJ_DESC_OBJARRAY)
                        {
                            status |= copyTensorHandles((vx_tensor)(objDescTensors[((k * prms->num_virtual_cores * numInputsPerCore) + (j * numInputsPerCore) + i)]->host_ref),
                                                prms->inputTensors[j][i]);
                        }
                        else
                        {
                            status |= copyTensorHandles((vx_tensor)(obj_desc[(TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + (k * prms->num_virtual_cores * numInputsPerCore) + (j * numInputsPerCore) + i)]->host_ref),
                                                prms->inputTensors[j][i]);
                        }
                    }
                    for(uint32_t i = 0U; i < numOutputsPerCore; i++)
                    {
                        if(obj_desc[TIVX_KERNEL_TIDL_IN_FIRST_TENSOR]->type == (uint16_t)TIVX_OBJ_DESC_OBJARRAY)
                        {
                            status |= copyTensorHandles((vx_tensor)(objDescTensors[((prms->num_input_tensors) + (k * prms->num_virtual_cores * numOutputsPerCore) + (j * numOutputsPerCore) + i)]->host_ref),
                                                prms->outputTensors[j][i]);
                        }
                        else
                        {
                            status |= copyTensorHandles((vx_tensor)(obj_desc[(TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + (prms->num_input_tensors) + (k * prms->num_virtual_cores * numOutputsPerCore) + (j * numOutputsPerCore) + i)]->host_ref),
                                                prms->outputTensors[j][i]);
                        }
                    }
                }

                if(status == (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_INFO, "MPU_1 vxProcessGraph call to DSP target - super batch - %d \n", prms->num_super_batches);
                    status = vxProcessGraph(prms->graph);  /* Run graph processing num_super_batches times to infer all batches */
                }

                if(status != (vx_status)VX_SUCCESS)
                {
                    break;
                }
            }
        }

        else if(prms->inference_mode == (uint32_t)TIDL_inferenceModeLowLatency)
        {
            /* Copy over the input and output tensor pointers from application to all nodes - TIDL expects single input buffer and 
            writes single output buffer irrespective of number of cores used for processing in this mode  */
            uint32_t num_input_tensors = prms->num_input_tensors/prms->num_virtual_cores; /* For LowLatency mode numVirtualCore will be 1 */
            uint32_t num_output_tensors = prms->num_output_tensors/prms->num_virtual_cores;
            for(uint32_t j = 0U; j < prms->num_cores; j++)
            {
                if(prms->num_cores <= (uint32_t)TIDL_MAX_NUM_CORES) /* To avoid -Werror=array-bounds warning*/
                {
                    if(obj_desc[TIVX_KERNEL_TIDL_IN_FIRST_TENSOR]->type == (uint16_t)TIVX_OBJ_DESC_OBJARRAY)
                    {

                        tivx_obj_desc_object_array_t * inputArrayDesc;
                        tivx_obj_desc_object_array_t * outputArrayDesc;
                        tivx_obj_desc_t *inputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
                        tivx_obj_desc_t *outputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
                        uint32_t inputElementSize, outputElementSize;

                        inputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_INPUT_IDX];
                        outputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_OUTPUT_IDX];
                        inputElementSize = inputArrayDesc->num_items;
                        outputElementSize = outputArrayDesc->num_items;

                        tivxGetObjDescList(inputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)inputTensorDescList, inputElementSize);
                        tivxGetObjDescList(outputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)outputTensorDescList, outputElementSize);

                        /* Extracting and Copy input tensors */ 
                        for(uint32_t i = 0U; i < num_input_tensors; i++)
                        {
                            vx_tensor input = (vx_tensor)inputTensorDescList[i]->host_ref;
                            status |= copyTensorHandles((vx_tensor)input, prms->inputTensors[j][i]);

                        }
                        /* Extracting and Copy output tensors */ 
                        for(uint32_t i = 0U; i < num_output_tensors; i++)
                        {
                            vx_tensor output = (vx_tensor)outputTensorDescList[i]->host_ref;
                            status |= copyTensorHandles((vx_tensor)output, prms->outputTensors[j][i]);
                        }
                    }
                    else
                    {
                        for(uint32_t i = 0U; i < (prms->num_input_tensors/prms->num_virtual_cores); i++)
                        {
                            vx_tensor input = (vx_tensor)obj_desc[TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + i]->host_ref;
                            status |= copyTensorHandles((vx_tensor)input, prms->inputTensors[j][i]);
                        }
                        for(uint32_t i = 0U; i < (prms->num_output_tensors/prms->num_virtual_cores); i++)
                        {
                            vx_tensor output = (vx_tensor)obj_desc[TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + prms->num_input_tensors + i]->host_ref;
                            status |= copyTensorHandles((vx_tensor)output, prms->outputTensors[j][i]);
                        }
                    }
                }
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Inference Mode\n");
            status = (vx_status)VX_FAILURE;
        }
    }

    if((status == (vx_status)VX_SUCCESS) && (prms->inference_mode != (uint32_t)TIDL_inferenceModeHighThroughput))
    {
        /* TIDL_inferenceModeHighThroughput processing called multiple times as part of above conditions */
        VX_PRINT(VX_ZONE_INFO, "MPU_1 vxProcessGraph call to DSP target \n");
        status = vxProcessGraph(prms->graph);
    }

    if((status == (vx_status)VX_SUCCESS) && (prms != NULL))
    {
        status = tivxUpdateOutArgs(prms, obj_desc);
    }

    return status;
}

static vx_status tivxGetDspPtrsCmd(tivxTIDLNestedKernelObj * prms, vx_user_data_object controlGetArgs, vx_user_data_object * controlSetArgs)
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_map_id  map_id_get, map_id_set;
    void *controlGetArgsBuffer = NULL;
    void *controlSetArgsBuffer = NULL;

    for(int i = 0; i < (int32_t)prms->num_cores; i++)
    {
        vx_reference refs[2];
        refs[0] = (vx_reference)(controlGetArgs);
        
        /* Map individually for each core - controlGetArgs are read from the same memory location for all cores
            Need to invalidate MPU cache before reading the controlGetArgs set by each DSP */
        status = vxMapUserDataObject(controlGetArgs, 0, sizeof(TIDL_controlGetArgs), &map_id_get,
                                (void **)&controlGetArgsBuffer,  (vx_enum)VX_READ_ONLY,  (vx_enum)VX_MEMORY_TYPE_HOST, 0);
        status |= vxMapUserDataObject(*controlSetArgs, 0, sizeof(TIDL_controlSetArgs), &map_id_set,
                                        (void **)&controlSetArgsBuffer,  (vx_enum)VX_READ_AND_WRITE,  (vx_enum)VX_MEMORY_TYPE_HOST, 0);

        if(status == (vx_status)VX_SUCCESS)
        {
            status = tivxNodeSendCommand(prms->node[i], 0, TIVX_TIDL_CMD_GET_C7X_PTRS, refs, 1u);

            if(status == (vx_status)VX_SUCCESS)
            {
                TIDL_controlGetArgs * getArgs = (TIDL_controlGetArgs *)(controlGetArgsBuffer);
                TIDL_controlSetArgs * setArgs = (TIDL_controlSetArgs *)(controlSetArgsBuffer);
                setArgs->ctrlSetArgs[i].syncBufferPtr[TIDL_Physical] = getArgs->ctrlGetArgs.syncBufferPtr[TIDL_Physical];   
                setArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_L1] = getArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_L1];
                setArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_L2] = getArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_L2];
                setArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_MSMC] = getArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_MSMC];
                setArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_DDR] = getArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_DDR];
                setArgs->ctrlSetArgs[i].ncScratchPtr[TIDL_Physical][TIDL_DDR_PERSIST] = getArgs->ctrlGetArgs.ncScratchPtr[TIDL_Physical][TIDL_DDR_PERSIST];

                (void)vxUnmapUserDataObject(controlGetArgs, map_id_get);
                (void)vxUnmapUserDataObject(*controlSetArgs, map_id_set);
            }
        }
    }
    return status;
}

static vx_status tivxSetDspPtrsCmd(tivxTIDLNestedKernelObj * prms, vx_user_data_object controlSetArgs)
{
    vx_status status = (vx_status)VX_SUCCESS;

    for(uint32_t i = 0; (i < prms->num_cores) && (i < (uint32_t)TIDL_MAX_NUM_CORES); i++)
    {
        vx_reference refs[2];
        refs[0] = (vx_reference)controlSetArgs;
        status |= tivxNodeSendCommand(prms->node[i], 0, TIVX_TIDL_CMD_SET_C7X_PTRS, refs, 1u);
    }
    return status;
}

static vx_status tivxInitCmd(tivxTIDLNestedKernelObj * prms)
{
    vx_status status = (vx_status)VX_SUCCESS;
    for(int i = 0; i < (int32_t)prms->num_cores; i++)
    {
        status |= tivxNodeSendCommand(prms->node[i], 0, TIVX_TIDL_CMD_INIT, NULL, 0u);
    }
    return status;
}

static vx_status VX_CALLBACK tivxKernelTIDLCreate
(
  tivx_target_kernel_instance kernel,
  tivx_obj_desc_t *obj_desc[],
  uint16_t num_params,
  void *priv_arg
)
{
    #ifdef TIVX_TIDL_MPU_TARGET_DEBUG
    tivx_set_debug_zone(VX_ZONE_INFO);
    #endif

    vx_status status = (vx_status)VX_SUCCESS;
    vx_reference nestedNodeParams[16];
    uint32_t num_input_tensors_per_core = 0;
    uint32_t num_output_tensors_per_core = 0;

    TIDL_CreateParams  *createParams = NULL;
    vx_user_data_object create_params;
    vx_map_id map_id_create_params;

    VX_PRINT(VX_ZONE_INFO, "MPU_1 tivxKernelTIDLCreate - num_params = %d\n", num_params);

    for (uint16_t i = 0U; i < num_params; i ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data. obj_desc[] is sent as argument for callback.
        <justification end> */
        if ((NULL == obj_desc[i]) && (i != TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX)) /* i = 5 corresponds to trace data dump which can be NULL */
        {
            status = (vx_status)VX_FAILURE;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxTIDLNestedKernelObj * prms = NULL;
        vx_context context;

        prms = (tivxTIDLNestedKernelObj *)tivxMemAlloc((uint32_t)(sizeof(tivxTIDLNestedKernelObj)), (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        <justification end> */
        if(prms == NULL)
        {
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        <justification end> */
        if(status == (vx_status)VX_SUCCESS)
        {
            /* LDRA_JUSTIFY_END */
            context = vxCreateContext();
            prms->graph = vxCreateGraph(context);
            status = vxGetStatus((vx_reference)prms->graph);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        <justification end> */
        if(status == (vx_status)VX_SUCCESS)
        {
            /* LDRA_JUSTIFY_END */
            /* Read vx_user_data_object parameters passed from host */
            prms->tidlNodeParams[0] = (vx_user_data_object)(obj_desc[TIVX_KERNEL_TIDL_IN_CONFIG_IDX]->host_ref);
            prms->tidlNodeParams[1] =  (vx_user_data_object)(obj_desc[TIVX_KERNEL_TIDL_IN_NETWORK_IDX]->host_ref);
            prms->tidlNodeParams[2] =  (vx_user_data_object)(obj_desc[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX]->host_ref);
            prms->tidlNodeParams[3] =  (vx_user_data_object)(obj_desc[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX]->host_ref);
            prms->tidlNodeParams[4] =  (vx_user_data_object)(obj_desc[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX]->host_ref);
            if(obj_desc[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX] != NULL)
            {
                prms->tidlNodeParams[5] =  (vx_user_data_object)(obj_desc[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX]->host_ref);
            }
            else
            {
                prms->tidlNodeParams[5] = (vx_user_data_object)NULL;
            }

            /******  Find number of input and output tensors from TIDL io buffer descriptor ******/
            tivxTIDLJ7Params  *tidlConfigParams = NULL;
            sTIDL_IOBufDesc_t *ioBufDesc = NULL;
            vx_user_data_object config = prms->tidlNodeParams[0];
            vx_map_id map_id_config;

            status = vxMapUserDataObject(config, 0, sizeof(tivxTIDLJ7Params), &map_id_config,
                (void **)&tidlConfigParams, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            <justification end> */
            if(status == (vx_status)VX_SUCCESS)
            {
                /* LDRA_JUSTIFY_END */
                ioBufDesc = (sTIDL_IOBufDesc_t *)&tidlConfigParams->ioBufDesc;

                prms->num_input_tensors  = (uint32_t)ioBufDesc->numInputBuf;
                prms->num_output_tensors = (uint32_t)ioBufDesc->numOutputBuf;
                prms->num_virtual_cores = (uint32_t)ioBufDesc->numVirtualCores;
                prms->num_cores = (uint32_t)ioBufDesc->numCores;
                prms->num_super_batches = (uint32_t)ioBufDesc->numSuperBatches;
                prms->inference_mode = (uint32_t)ioBufDesc->inferenceMode;

                /*status check can be skipped as status is already success and
                 can be unmapped */
                (void)vxUnmapUserDataObject(config, map_id_config);
            }
            num_input_tensors_per_core = prms->num_input_tensors / prms->num_virtual_cores;
            num_output_tensors_per_core = prms->num_output_tensors / prms->num_virtual_cores;

            /***********************************************************************************/
            createParams = NULL;
            create_params = prms->tidlNodeParams[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX];
            status = vxMapUserDataObject(create_params, 0, sizeof(TIDL_CreateParams), &map_id_create_params,
                (void **)&createParams, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);
            if(status == (vx_status)VX_SUCCESS)
            {
                /* Read coreStartIdx from createParams*/
                prms->core_start_idx = (int32_t)createParams->coreStartIdx;
                prms->trace_log_level = (uint32_t)createParams->traceLogLevel;
                prms->forceNegativeTest = createParams->forceNegativeTest;
                
                /*status check can be skipped as status is already success and
                 can be unmapped */
                (void)vxUnmapUserDataObject(create_params, map_id_create_params);
            }
            /***********************************************************************************/

            /* Save inputs and outputs vx_tensors to be used for node creation */
            if(obj_desc[TIVX_KERNEL_TIDL_IN_FIRST_TENSOR]->type == (uint16_t)TIVX_OBJ_DESC_OBJARRAY)
            {
                tivx_obj_desc_object_array_t * inputArrayDesc;
                tivx_obj_desc_object_array_t * outputArrayDesc;
                tivx_obj_desc_t *inputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
                tivx_obj_desc_t *outputTensorDescList[TIVX_OBJECT_ARRAY_MAX_ITEMS];
                uint32_t inputElementSize, outputElementSize;

                inputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_INPUT_IDX];
                outputArrayDesc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_TIDLV2_OUTPUT_IDX];
                inputElementSize = inputArrayDesc->num_items;
                outputElementSize = outputArrayDesc->num_items;
                
                tivxGetObjDescList(inputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)inputTensorDescList, inputElementSize);
                tivxGetObjDescList(outputArrayDesc->obj_desc_id, (tivx_obj_desc_t **)outputTensorDescList, outputElementSize);


                for(uint32_t j = 0U; j < prms->num_virtual_cores; j++)
                {
                    /*Extracting I/P tensors*/ 
                    for(uint32_t i = 0U; i < num_input_tensors_per_core; i++)
                    {
                        prms->inputTensors[j][i] = (vx_tensor)(inputTensorDescList[(j * num_input_tensors_per_core) + i]->host_ref);
                    }
                    /*Extracting O/P tensors*/ 
                    for(uint32_t i = 0U; i < num_output_tensors_per_core; i++)
                    {
                        prms->outputTensors[j][i] = (vx_tensor)(outputTensorDescList[(j * num_output_tensors_per_core) + i]->host_ref);
                    }
                }
            }
            else
            {
                for(uint32_t j = 0U; j < prms->num_virtual_cores; j++)
                {
                    for(uint32_t i = 0U; i < num_input_tensors_per_core; i++)
                    {   
                        prms->inputTensors[j][i] = (vx_tensor)(obj_desc[(TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + (j * num_input_tensors_per_core) + i)]->host_ref);
                    }
                    for(uint32_t i = 0U; i < num_output_tensors_per_core; i++)
                    {
                        prms->outputTensors[j][i] = (vx_tensor)(obj_desc[(TIVX_KERNEL_TIDL_IN_FIRST_TENSOR + prms->num_input_tensors + (j * num_output_tensors_per_core) + i)]->host_ref);
                    }
                }
            }

            if(prms->inference_mode == (uint32_t)TIDL_inferenceModeLowLatency) /* Create num_cores (4) copies from num_virtual_cores (1) input/output tensors for individual cores */
            {
                for(uint32_t j = 1U; j < prms->num_cores ; j++)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data. num_core is fixed for given SOC and cannot be changed
                    <justification end> */
                    if(prms->num_cores <= (uint32_t)TIDL_MAX_NUM_CORES) /* To avoid -Werror=array-bounds warning*/
                    {
                        /* LDRA_JUSTIFY_END */
                        for(uint32_t i = 0U; i < num_input_tensors_per_core; i++)
                        {
                            prms->inputTensors[j][i] = (vx_tensor)tivxCreateReferenceFromExemplar(context, (vx_reference)prms->inputTensors[0][i]);
                            status |= copyTensorHandles(prms->inputTensors[0][i], prms->inputTensors[j][i]);
                        }
                        for(uint32_t i = 0U; i < num_output_tensors_per_core; i++)
                        {
                            prms->outputTensors[j][i] = (vx_tensor)tivxCreateReferenceFromExemplar(context, (vx_reference)prms->outputTensors[0][i]);
                            status |= copyTensorHandles(prms->outputTensors[0][i], prms->outputTensors[j][i]);
                        }
                    }
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            <justification end> */
            if(status == (vx_status)VX_SUCCESS)
            {
                /* LDRA_JUSTIFY_END */
                /* Typecast params to vx_reference for node creation API */
                for(uint32_t i = 0U; i < TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS; i++)
                {
                    nestedNodeParams[i] = (vx_reference)prms->tidlNodeParams[i];
                }
                
                /* vx_kernel kernel = vxGetKernelByName(context, TIVX_KERNEL_TIDL_NAME); */
                #if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
                if ((prms->num_input_tensors + prms->num_output_tensors) > (TIVX_KERNEL_MAX_PARAMS - TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS))
                {
                    prms->kernel = NULL;
                }
                else
                #endif
                {
                    prms->kernel = tivxAddKernelTIDL(context, num_input_tensors_per_core, num_output_tensors_per_core);
                    VX_PRINT(VX_ZONE_INFO, "Kernel created for nested graph on MPU_1 -- %p \n", prms->kernel);
                }

                create_params = prms->tidlNodeParams[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX];
                createParams = NULL;
                status = vxMapUserDataObject(create_params, 0, sizeof(TIDL_CreateParams), &map_id_create_params,
                    (void **)&createParams, (vx_enum)VX_READ_AND_WRITE, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

                if(status == (vx_status)VX_SUCCESS)
                {
#if defined (HOST_EMULATION)
                    char traceBaseNameOrig[TIDL_STRING_SIZE], traceBaseName[TIDL_STRING_SIZE];
                    (void)strncpy(traceBaseNameOrig, (char *)createParams->traceBaseName, TIDL_STRING_SIZE);
#endif
                    vx_user_data_object create_params_copy[TIDL_MAX_NUM_CORES];
                    TIDL_CreateParams * mappedObjects[TIDL_MAX_NUM_CORES];
                    vx_map_id mappedIds[TIDL_MAX_NUM_CORES];

                    for(uint32_t i = 0U; i < prms->num_cores; i++)
                    {
                        /* create copies of create Params to pass different parameters to different cores, e.g. coreId, traceBaseName, etc.*/
                        create_params_copy[i] = vxCreateUserDataObject(context, "TIDL_CreateParams", sizeof(TIDL_CreateParams), NULL);
                        /* Create out_args copy for each core, it is not yet populated so no need to copy data from existing outArgs to the copied ones */
                        prms->outArgs[i] = (vx_user_data_object)tivxCreateReferenceFromExemplar(context, (vx_reference)prms->tidlNodeParams[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX]);
                    }

                    for(uint32_t i = 0U; i < prms->num_cores; i++)
                    {
                        VX_PRINT(VX_ZONE_INFO, "Creating node %d with DSP target \n", i);

                        /************* Update createParams with current core's id **********************/

                        /* Better way to do this would be use of tivxGetSelfCpuId() as part of DSP create callbacks to get coreId but
                        that would not work for host emulation mode of TIDL 
                        Hence, creating copies of createParams for all cores, where each core's id is populated in its own copy of createParams */
                        createParams->coreId = (uint32_t)prms->core_start_idx + i;

                        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INIT1_FNT)
                        {
                          create_params_copy[0] =  NULL ;
                        }

                        status = vxCopyUserDataObject(create_params_copy[i], 0, sizeof(TIDL_CreateParams), createParams, (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST);
                        if(status == (vx_status)VX_SUCCESS)
                        {
                            if(prms->forceNegativeTest == -TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INIT2_FNT)
                            {
                              create_params_copy[0] =  NULL;
                            }
                            status = vxMapUserDataObject(create_params_copy[i], 0, sizeof(TIDL_CreateParams), &mappedIds[i],
                            (void **)&mappedObjects[i], (vx_enum)VX_WRITE_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);
                            if(status == (vx_status)VX_SUCCESS)
                            {
#if defined (HOST_EMULATION)
                                /* Update trace name with core id for host emulation mode. For target mode, this is done by the remote file io utility in app_utils */
                                char fileName[TIDL_STRING_SIZE], dirName[TIDL_STRING_SIZE];
                                (void)strcpy(fileName, traceBaseNameOrig);
                                (void)strcpy(dirName, traceBaseNameOrig);
                                char * fileNamePtr = getFileNameFromPath(fileName);
                                getDirFromPath(dirName);
                                (void)snprintf((char *)traceBaseName, TIDL_STRING_SIZE, "%s/C7x_%d_%s", dirName, (prms->core_start_idx + i + 1), (char *)fileNamePtr);
                                (void)strncpy(mappedObjects[i]->traceBaseName, (char *)traceBaseName, TIDL_STRING_SIZE);
#endif
                                /*status check can be skipped as status is already
                                success and can be unmapped */
                                (void)vxUnmapUserDataObject(create_params_copy[i], mappedIds[i]);
                            }
                        nestedNodeParams[TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX] = (vx_reference)create_params_copy[i];
                        nestedNodeParams[TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX] = (vx_reference)prms->outArgs[i];
                        
                        #if !defined (ENABLE_SDK_9_2_COMPATIBILITY) && !defined (ENABLE_SDK_10_0_COMPATIBILITY) && !defined (ENABLE_SDK_10_1_COMPATIBILITY) && !defined (ENABLE_SDK_11_0_COMPATIBILITY) && !defined (ENABLE_SDK_11_1_COMPATIBILITY)
                        if ((prms->num_input_tensors + prms->num_output_tensors) > (TIVX_KERNEL_MAX_PARAMS - TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS))
                        {
                            /* Packing Input tensors in vx_object_array */
                            prms->input_tensor_list[i] = createObjArray(context, prms->inputTensors[i], num_input_tensors_per_core);

                            /* Packing Output tensors in vx_object_array */
                            prms->output_tensor_list[i] = createObjArray(context, prms->outputTensors[i], num_output_tensors_per_core);

                            /*
                             * FORCE NEGATIVE TEST START
                             *
                             * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_OBJARRAY_CREATION_FAIL
                             * forces object array creation failure by setting to NULL
                             */
                            if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_OBJARRAY_CREATION_FAIL)
                            {
                                prms->input_tensor_list[i] = NULL;
                                VX_PRINT(VX_ZONE_INFO, "MPU: Force negative test - object array creation failure\n");
                            }
                            /*
                             * FORCE NEGATIVE TEST END
                             */

                            if((prms->input_tensor_list[i] == NULL) || (prms->output_tensor_list[i] == NULL))
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Object array creation failed from MPU side\n");
                                status = (vx_status)VX_FAILURE;
                            }
                            
                            if(status == (vx_status)VX_SUCCESS)
                            {
                                prms->node[i] = tivxTIDLNodeV2(prms->graph, 
                                                                    prms->tidlNodeParams[TIVX_KERNEL_TIDL_IN_CONFIG_IDX],
                                                                    prms->tidlNodeParams[TIVX_KERNEL_TIDL_IN_NETWORK_IDX],
                                                                    create_params_copy[i], /*TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX*/ 
                                                                    prms->tidlNodeParams[TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX],
                                                                    prms->outArgs[i], /*TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX*/
                                                                    prms->tidlNodeParams[TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX],
                                                                    prms->input_tensor_list[i], 
                                                                    prms->output_tensor_list[i]
                                                                );
                            }
                        }
                        else
                        #endif
                        {
                            prms->input_tensor_list[i] = NULL;
                            prms->output_tensor_list[i] = NULL;
                            prms->node[i] = tivxTIDLNode(prms->graph, prms->kernel, nestedNodeParams, prms->inputTensors[i], prms->outputTensors[i]);
                        }

                        /*
                         * FORCE NEGATIVE TEST START
                         *
                         * TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INIT3_FNT
                         * forces node to be NULL to test node creation failure path
                         */
                        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_INIT3_FNT)
                        {
                            prms->node[i] = NULL;
                            VX_PRINT(VX_ZONE_INFO, "MPU: Force negative test - INIT3 node NULL\n");
                        }
                        /*
                         * FORCE NEGATIVE TEST END
                         */

                        if(prms->node[i]==NULL)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Node creation failed from MPU side\n");
                            status = (vx_status)VX_FAILURE;
                        }

                        if(status == (vx_status)VX_SUCCESS)
                        {
                            (void)vxSetNodeTarget(prms->node[i], (vx_enum)VX_TARGET_STRING, child_target_name[((uint32_t)prms->core_start_idx + (uint32_t)i) * 8U]); /* setting target to DSP cores - multiple priorities currently not supported */
                            status = vxGetStatus((vx_reference)prms->node[i]);
                        }

                        if(status == (vx_status)VX_SUCCESS)
                        {
                            VX_PRINT(VX_ZONE_INFO, "Node successfully created for target %s \n", child_target_name[(uint32_t)i * 8U]);
                        }

                        /*status check can be skipped as status is already
                        success and can be released independent of map/unmapping
                        */
                        (void)vxReleaseUserDataObject(&create_params_copy[i]);
                        }
                    }

                    /*status check can be skipped as status is already success 
                     and can be unmapped */
                    (void)vxUnmapUserDataObject(create_params, map_id_create_params);
                }
                (void)vxReleaseContext(&context);
            }
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            status = vxVerifyGraph(prms->graph);
        }

        if(status == (vx_status)VX_SUCCESS)
        {
            (void)tivxSetTargetKernelInstanceContext(kernel, prms,  (uint32_t)(sizeof(tivxTIDLNestedKernelObj)));
        }

        if((status == (vx_status)VX_SUCCESS) && (prms->inference_mode == (uint32_t)TIDL_inferenceModeLowLatency))
        {
                vx_status innerContextStatus;

                vx_user_data_object  controlGetArgs, controlSetArgs;

                vx_context innerContext = vxCreateContext();

                controlGetArgs = vxCreateUserDataObject(innerContext, "TIDL_controlGetArgs", sizeof(TIDL_controlGetArgs), NULL);
                controlSetArgs = vxCreateUserDataObject(innerContext, "TIDL_controlSetArgs", sizeof(TIDL_controlSetArgs), NULL);

                innerContextStatus = vxGetStatus((vx_reference)controlGetArgs);
                innerContextStatus |= vxGetStatus((vx_reference)controlSetArgs);

                if (innerContextStatus == (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_INFO, "Calling get c7x ptrs cmd \n");
                    innerContextStatus = tivxGetDspPtrsCmd(prms, controlGetArgs, &controlSetArgs);
                }

                if(innerContextStatus == (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_INFO, "Calling set c7x ptrs cmd \n");
                    innerContextStatus = tivxSetDspPtrsCmd(prms, controlSetArgs);
                }

                if(innerContextStatus == (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_INFO, "Calling Init cmd \n");
                    innerContextStatus = tivxInitCmd(prms);
                }
                (void)vxReleaseUserDataObject(&controlGetArgs);
                (void)vxReleaseUserDataObject(&controlSetArgs);
                (void)vxReleaseContext(&innerContext);
                status = innerContextStatus;
        }
        VX_PRINT(VX_ZONE_INFO, "MPU_1 tivxKernelTIDLCreate -- done \n");
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelTIDLDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    uint16_t num_params, void *priv_arg)
{
    VX_PRINT(VX_ZONE_INFO, "MPU_1 tivxKernelTIDLDelete \n");
    
    vx_status status = (vx_status)VX_SUCCESS;

    uint32_t size = (uint32_t)(sizeof(tivxTIDLNestedKernelObj));
    tivxTIDLNestedKernelObj *prms = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);

    if(status == (vx_status)VX_SUCCESS)
    {
        if(prms->kernel != NULL)
        {
            status = vxRemoveKernel(prms->kernel);
        }
        status |= vxReleaseGraph(&prms->graph);
    }

    if(status == (vx_status)VX_SUCCESS)
    {
        uint32_t num_input_tensors_per_core = (prms->num_input_tensors/prms->num_virtual_cores);
        uint32_t num_output_tensors_per_core = (prms->num_output_tensors/prms->num_virtual_cores);


        for(uint32_t j = 0U; j < prms->num_cores; j++)
        {
            
            if(prms->input_tensor_list[j] != NULL)
            {
                status |= vxReleaseObjectArray(&prms->input_tensor_list[j]);
            }
        
            if(prms->output_tensor_list[j] != NULL)
            {
                status |= vxReleaseObjectArray(&prms->output_tensor_list[j]);
            }
            
            if((prms->inference_mode == (uint32_t)TIDL_inferenceModeLowLatency) && (j != 0U))
            {
                for(uint32_t i = 0U; i < num_input_tensors_per_core; i++)
                {
                    status |= removeTensorHandles(prms->inputTensors[j][i]);
                    status |= vxReleaseTensor(&prms->inputTensors[j][i]);
                }
                for(uint32_t i = 0U; i < num_output_tensors_per_core; i++)
                {
                    status |= removeTensorHandles(prms->outputTensors[j][i]);
                    status |= vxReleaseTensor(&prms->outputTensors[j][i]);
                }
            }
            status |= vxReleaseUserDataObject(&prms->outArgs[j]);
        }
    }

    return (status);
}

/* Public Functions */

void tivxAddTargetKernelTIDL_mpu(void)
{
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    Here tivxGetSelfCpuId() function always returns TIVX_CPU_ID_MPU_0 for this SOC.
    <justification end> */
    if (self_cpu == (vx_enum)TIVX_CPU_ID_MPU_0)
    {
        /* LDRA_JUSTIFY_END */
        uint32_t i;

        for (i = 0; i < TIDL_MAX_TARGETS; i++)
        {
            vx_tidl_nested_target_kernel[i] = tivxAddTargetKernelByName
                                    (
                                      TIVX_KERNEL_TIDL_NAME,
                                      target_name[i],
                                      tivxKernelTIDLProcess,
                                      tivxKernelTIDLCreate,
                                      tivxKernelTIDLDelete,
                                      tivxKernelTIDLControl,
                                      NULL
                                    );
        }
    }
}

void tivxRemoveTargetKernelTIDL_mpu(void)
{
    uint32_t i;
    VX_PRINT(VX_ZONE_INFO, "Removing MPU_1 target kernel \n");
    for (i = 0; i < TIDL_MAX_TARGETS; i++)
    {
        (void)tivxRemoveTargetKernel(vx_tidl_nested_target_kernel[i]);
    }
    VX_PRINT(VX_ZONE_INFO, "Removing MPU_1 target kernel  -- done \n");
}

static vx_status VX_CALLBACK tivxKernelTIDLControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status status = (vx_status)VX_SUCCESS;

    uint32_t size = (uint32_t)(sizeof(tivxTIDLNestedKernelObj));
    tivxTIDLNestedKernelObj *prms = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel,
        (void **)&prms, &size);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if(status == (vx_status)VX_SUCCESS)
    {
        /* LDRA_JUSTIFY_END */
        if(prms->forceNegativeTest == TIDL_SAFETY_FLAG_TIDL_RT_FORCE_CTRL_NFNT)
        {
            for(int32_t i = 0; i < (int32_t)prms->num_cores; i++) // Sending cmd data for only first DSP core 
            {
              status |= tivxNodeSendCommand(prms->node[0], 0, node_cmd_id, NULL, 0u);                
            }
        }
        else
        {
            status |= tivxNodeSendCommand(prms->node[0], 0, node_cmd_id, NULL, 0u);
        }
    }
  
    return status;
    
}
