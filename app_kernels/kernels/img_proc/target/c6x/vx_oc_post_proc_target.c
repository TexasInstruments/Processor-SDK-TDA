/*
*
* Copyright (c) 2020-2026 Texas Instruments Incorporated
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
#include <float.h>
#include <limits.h>
#include <math.h>

#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_target_utils.h"

#include <tivx_oc_post_proc_host.h>

#include <utils/mem/include/app_mem.h>

static vx_size getTensorDataType(vx_int32 tidl_type);

static tivx_target_kernel vx_oc_post_proc_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelOCPostProcCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    tivxOCPostProcParams * prms = NULL;

    prms = tivxMemAlloc(sizeof(tivxOCPostProcParams), TIVX_MEM_EXTERNAL);

    tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxOCPostProcParams));

    return(VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelOCPostProcDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if (VX_SUCCESS == status)
    {
        uint32_t size;
        tivxOCPostProcParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            tivxMemFree(prms, sizeof(tivxOCPostProcParams), TIVX_MEM_EXTERNAL);
        }

    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelOCPostProcProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    tivxOCPostProcParams *prms = NULL;
    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    if(status==VX_SUCCESS)
    {
        uint32_t size;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);
        if ((VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxOCPostProcParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr;

        tivx_obj_desc_user_data_object_t *in_args_desc;
        void* in_args_target_ptr;

        tivx_obj_desc_tensor_t *in_tensor_desc;
        void *in_tensor_target_ptr;

        tivx_obj_desc_user_data_object_t *results_desc;
        void *results_target_ptr;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_OC_POST_PROC_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_args_desc  = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_OC_POST_PROC_IN_ARGS_IDX];
        in_args_target_ptr  = tivxMemShared2TargetPtr(&in_args_desc->mem_ptr);
        tivxMemBufferMap(in_args_target_ptr, in_args_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        in_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_OC_POST_PROC_INPUT_TENSOR_IDX];
        in_tensor_target_ptr = tivxMemShared2TargetPtr(&in_tensor_desc->mem_ptr);
        tivxMemBufferMap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        results_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_OC_POST_PROC_RESULTS_IDX];
        results_target_ptr = tivxMemShared2TargetPtr(&results_desc->mem_ptr);
        tivxMemBufferMap(results_target_ptr, results_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        tivxOCPostProcParams *params = (tivxOCPostProcParams *)config_target_ptr;
        sTIDL_IOBufDesc_t *ioBufDesc = (sTIDL_IOBufDesc_t *)&params->ioBufDesc;

        vx_uint32 label_offset = 0;

        if(ioBufDesc->outWidth[0] == 1001)
        {
            label_offset = 0;
        }
        else if(ioBufDesc->outWidth[0] == 1000)
        {
            label_offset = 1;
        }

        vx_size data_type = getTensorDataType(ioBufDesc->outElementType[0]);
        vx_uint32 classid[TIVX_OC_MAX_CLASSES];
        vx_int32 i, j;

        for(i = 0; i < TIVX_OC_MAX_CLASSES; i++)
        {
            classid[i] = (vx_uint32)-1;
        }

        if(data_type == VX_TYPE_FLOAT32)
        {
            vx_float32 score[TIVX_OC_MAX_CLASSES];
            vx_float32 *pIn;

            pIn = (vx_float32 *)in_tensor_target_ptr + (ioBufDesc->outPadT[0] * in_tensor_desc->dimensions[0]) + ioBufDesc->outPadL[0];

            for(i = 0; i < params->num_top_results; i++)
            {
                score[i] = FLT_MIN;
                classid[i] = (vx_uint32)-1;

                for(j = 0; j < ioBufDesc->outWidth[0]; j++)
                {
                    if(pIn[j] > score[i])
                    {
                        score[i] = pIn[j];
                        classid[i] = j;
                    }
                }
                if(classid[i] < ioBufDesc->outWidth[0])
                {
                    pIn[classid[i]] = FLT_MIN;
                }
                else
                {
                    classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                }
            }
        }
        else if(data_type == VX_TYPE_INT16)
        {
            vx_int16 score[TIVX_OC_MAX_CLASSES];
            vx_int16 *pIn;

            pIn = (vx_int16 *)in_tensor_target_ptr + (ioBufDesc->outPadT[0] * in_tensor_desc->dimensions[0]) + ioBufDesc->outPadL[0];

            for(i = 0; i < params->num_top_results; i++)
            {
                score[i] = INT16_MIN;
                classid[i] = (vx_uint32)-1;

                for(j = 0; j < ioBufDesc->outWidth[0]; j++)
                {
                    if(pIn[j] > score[i])
                    {
                        score[i] = pIn[j];
                        classid[i] = j;
                    }
                }
                if(classid[i] < ioBufDesc->outWidth[0])
                {
                    pIn[classid[i]] = INT16_MIN;
                }
                else
                {
                    classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                }
            }
        }
        else if(data_type == VX_TYPE_INT8)
        {
            vx_int8 score[TIVX_OC_MAX_CLASSES];
            vx_int8 *pIn;

            pIn = (vx_int8 *)in_tensor_target_ptr + (ioBufDesc->outPadT[0] * in_tensor_desc->dimensions[0]) + ioBufDesc->outPadL[0];

            for(i = 0; i < params->num_top_results; i++)
            {
                score[i] = INT8_MIN;
                classid[i] = (vx_uint32)-1;

                for(j = 0; j < ioBufDesc->outWidth[0]; j++)
                {
                    if(pIn[j] > score[i])
                    {
                        score[i] = pIn[j];
                        classid[i] = j;
                    }
                }
                if(classid[i] < ioBufDesc->outWidth[0])
                {
                    pIn[classid[i]] = INT8_MIN;
                }
                else
                {
                    classid[i] = 0; /* invalid class ID, ideally it should not reach here */
                }
            }
        }

        tivxOCPostProcOutput *pResults = (tivxOCPostProcOutput *)results_target_ptr;
        pResults->num_top_results = params->num_top_results;
        for(i = 0; i < pResults->num_top_results; i++)
        {
            pResults->class_id[i] = classid[i] + label_offset;
        }

        for(; i < (TIVX_OC_MAX_CLASSES - pResults->num_top_results); i++)
        {
            pResults->class_id[i] = INT_MAX;
        }

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(in_args_target_ptr, in_args_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(results_target_ptr, results_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
    }

    return (status);
}
void tivxAddTargetKernelOCPostProc()
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_oc_post_proc_target_kernel = tivxAddTargetKernelByName
                                        (
                                            TIVX_KERNEL_OC_POST_PROC_NAME,
                                            target_name,
                                            tivxKernelOCPostProcProcess,
                                            tivxKernelOCPostProcCreate,
                                            tivxKernelOCPostProcDelete,
                                            NULL,
                                            NULL
                                        );
    }
}

void tivxRemoveTargetKernelOCPostProc()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_oc_post_proc_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_oc_post_proc_target_kernel = NULL;
    }
}

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
