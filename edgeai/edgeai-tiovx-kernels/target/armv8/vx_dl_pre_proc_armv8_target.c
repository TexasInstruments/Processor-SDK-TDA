/*
*
* Copyright (c) 2022 Texas Instruments Incorporated
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

#include <edgeai_tiovx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_target_utils.h"
#include <tivx_dl_pre_proc_armv8_host.h>
#include <edgeai_dl_pre_proc_armv8_utils.h>

#define DL_PRE_PROC_MAX_KERNELS 4

static tivx_target_kernel vx_dl_pre_proc_armv8_target_kernel[DL_PRE_PROC_MAX_KERNELS] = {NULL};

static vx_status VX_CALLBACK tivxKernelDLPreProcArmv8Create
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    /* tivx_set_debug_zone(VX_ZONE_INFO); */

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    tivxDLPreProcArmv8Params * kernelParams = NULL;

    kernelParams = tivxMemAlloc(sizeof(tivxDLPreProcArmv8Params), TIVX_MEM_EXTERNAL);
    if (NULL == kernelParams)
    {
        status = VX_FAILURE;
    }
    else
    {
        tivxSetTargetKernelInstanceContext(kernel, kernelParams,  sizeof(tivxDLPreProcArmv8Params));
    }

    /* tivx_clr_debug_zone(VX_ZONE_INFO); */

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLPreProcArmv8Delete(
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
        tivxDLPreProcArmv8Params *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            tivxMemFree(prms, sizeof(tivxDLPreProcArmv8Params), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLPreProcArmv8Process
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxDLPreProcArmv8Params *prms = NULL;
    vx_int32 i;

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
            (sizeof(tivxDLPreProcArmv8Params) != size))
        {
            status = VX_FAILURE;
        }
    }

    if (VX_SUCCESS == status)
    {
        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr;

        tivx_obj_desc_image_t *in_img_desc;
        void* in_img_target_ptr[2];

        tivx_obj_desc_tensor_t *out_tensor_desc;
        void *out_tensor_target_ptr;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_img_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE_IDX];
        in_img_target_ptr[0]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[0]);
        tivxMemBufferMap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        in_img_target_ptr[1]  = NULL;
        if(in_img_desc->mem_ptr[1].shared_ptr != 0)
        {
            in_img_target_ptr[1]  = tivxMemShared2TargetPtr(&in_img_desc->mem_ptr[1]);
            tivxMemBufferMap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        out_tensor_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DL_PRE_PROC_ARMV8_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_tensor_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        VX_PRINT(VX_ZONE_INFO, "Image channel 0\n");
        VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[0].dim_x);
        VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[0].dim_y);
        VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[0].stride_y);
        VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[0].stride_x);
        VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[0].step_x);
        VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[0].step_y);

        if(in_img_target_ptr[1] != NULL)
        {
            VX_PRINT(VX_ZONE_INFO, "Image channel 1\n");
            VX_PRINT(VX_ZONE_INFO, "dim_x = %d\n", in_img_desc->imagepatch_addr[1].dim_x);
            VX_PRINT(VX_ZONE_INFO, "dim_y = %d\n", in_img_desc->imagepatch_addr[1].dim_y);
            VX_PRINT(VX_ZONE_INFO, "stride_y = %d\n", in_img_desc->imagepatch_addr[1].stride_y);
            VX_PRINT(VX_ZONE_INFO, "stride_x = %d\n", in_img_desc->imagepatch_addr[1].stride_x);
            VX_PRINT(VX_ZONE_INFO, "step_x = %d\n", in_img_desc->imagepatch_addr[1].step_x);
            VX_PRINT(VX_ZONE_INFO, "step_y = %d\n", in_img_desc->imagepatch_addr[1].step_y);
        }

        VX_PRINT(VX_ZONE_INFO, "Tensor input \n");
        VX_PRINT(VX_ZONE_INFO, "stride[0] = %d\n", out_tensor_desc->stride[0]);
        VX_PRINT(VX_ZONE_INFO, "stride[1] = %d\n", out_tensor_desc->stride[1]);
        VX_PRINT(VX_ZONE_INFO, "stride[2] = %d\n", out_tensor_desc->stride[2]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[0] = %d\n", out_tensor_desc->dimensions[0]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[1] = %d\n", out_tensor_desc->dimensions[1]);
        VX_PRINT(VX_ZONE_INFO, "dimensions[2] = %d\n", out_tensor_desc->dimensions[2]);

        tivxDLPreProcArmv8Params *dlParams = (tivxDLPreProcArmv8Params *)config_target_ptr;
        dlPreProcessImageParams params;

        params.channel_order            = dlParams->channel_order;
        params.tensor_format            = dlParams->tensor_format;
        params.tensor_data_type         = out_tensor_desc->data_type;
        params.input_width              = in_img_desc->imagepatch_addr[0].dim_x;
        params.input_height             = in_img_desc->imagepatch_addr[0].dim_y;
        params.in_stride_y              = in_img_desc->imagepatch_addr[0].stride_y;
        params.in_img_target_ptr[0]     = in_img_target_ptr[0];
        params.in_img_target_ptr[1]     = in_img_target_ptr[1];
        params.output_dimensions[0]     = out_tensor_desc->dimensions[0];
        params.output_dimensions[1]     = out_tensor_desc->dimensions[1];
        params.output_dimensions[2]     = out_tensor_desc->dimensions[2];
        params.out_tensor_target_ptr    = out_tensor_target_ptr;
        params.mean[0]                  = dlParams->mean[0];
        params.mean[1]                  = dlParams->mean[1];
        params.mean[2]                  = dlParams->mean[2];
        params.scale[0]                 = dlParams->scale[0];
        params.scale[1]                 = dlParams->scale[1];
        params.scale[2]                 = dlParams->scale[2];
        params.channel_pitch            = dlParams->channel_pitch;

        vx_df_image image_format = in_img_desc->format;

        if(image_format == VX_DF_IMAGE_RGB)
        {
            dlPreProcess_RGB_image(&params);
        }
        else if(image_format == VX_DF_IMAGE_NV12)
        {
            dlPreProcess_NV12_image(&params);
        }

        /* Write DL pre proc operation here */
        tivxMemBufferUnmap(out_tensor_target_ptr, out_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        tivxMemBufferUnmap(in_img_target_ptr[0], in_img_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if (in_img_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(in_img_target_ptr[1], in_img_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }
        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
    }

    return (status);
}

void tivxAddTargetKernelDLPreProcArmv8()
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[DL_PRE_PROC_MAX_KERNELS][TIVX_TARGET_MAX_NAME];

    strncpy(target_name[0], TIVX_TARGET_MPU_0, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[1], TIVX_TARGET_MPU_1, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[2], TIVX_TARGET_MPU_2, TIVX_TARGET_MAX_NAME);
    strncpy(target_name[3], TIVX_TARGET_MPU_3, TIVX_TARGET_MAX_NAME);
    status = (vx_status)VX_SUCCESS;

    if( (vx_status)VX_SUCCESS == status)
    {
        int i;

        for (i = 0; i < DL_PRE_PROC_MAX_KERNELS; i++)
        {
            vx_dl_pre_proc_armv8_target_kernel[i] = tivxAddTargetKernelByName
                                                    (
                                                        TIVX_KERNEL_DL_PRE_PROC_ARMV8_NAME,
                                                        target_name[i],
                                                        tivxKernelDLPreProcArmv8Process,
                                                        tivxKernelDLPreProcArmv8Create,
                                                        tivxKernelDLPreProcArmv8Delete,
                                                        NULL,
                                                        NULL
                                                    );
        }
    }
}

void tivxRemoveTargetKernelDLPreProcArmv8()
{
    vx_status status = VX_SUCCESS;
    int i;

    for (i = 0; i < DL_PRE_PROC_MAX_KERNELS; i++)
    {
        status = tivxRemoveTargetKernel(vx_dl_pre_proc_armv8_target_kernel[i]);
        if (status == VX_SUCCESS)
        {
            vx_dl_pre_proc_armv8_target_kernel[i] = NULL;
        }
    }
}
