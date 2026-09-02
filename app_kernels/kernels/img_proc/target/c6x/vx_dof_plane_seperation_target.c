/*
*
* Copyright (c) 2018-2026 Texas Instruments Incorporated
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



#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <TI/tivx_target_kernel.h>
#include "tivx_kernels_target_utils.h"

#include <tivx_dof_plane_seperation_host.h>
#include "tiadalg_interface.h"

static tivx_target_kernel vx_dof_plane_seperation_target_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDofPlaneSepCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    return(VX_SUCCESS);
}

static vx_status VX_CALLBACK tivxKernelDofPlaneSepDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    uint32_t i;

    for (i = 0U; i < (num_params -1); i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDofPlaneSepProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    tivxDofPlaneSepParams *prms = NULL;
    tivx_obj_desc_user_data_object_t* config_desc;
    void * config_target_ptr;
    vx_int32 i;

    for (i = 0U; i < num_params; i ++)
    {
        if ((NULL == obj_desc[i]) && (i != TIVX_KERNEL_DOF_PLANE_SEPERATION_YUV_OUTPUT_TENSOR_IDX))
        {
            status = VX_FAILURE;
            break;
        }
    }

    config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DOF_PLANE_SEPERATION_CONFIG_IDX];
    config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
    tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
    prms = (tivxDofPlaneSepParams *)config_target_ptr;

    if ((VX_SUCCESS == status) && (prms->skip_flag != 1))
    {
        tivx_obj_desc_image_t *in_dof_desc;
        void* in_image_target_ptr;

        tivx_obj_desc_tensor_t *out_img_desc;
        void *out_tensor_target_ptr;

        tivx_obj_desc_tensor_t *out_yuv_img_desc;
        void *out_yuv_tensor_target_ptr = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_DOF_PLANE_SEPERATION_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        prms = (tivxDofPlaneSepParams *)config_target_ptr;

        in_dof_desc  = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DOF_PLANE_SEPERATION_INPUT_IMAGE_IDX];
        in_image_target_ptr  = tivxMemShared2TargetPtr(&in_dof_desc->mem_ptr[0]);
        tivxMemBufferMap(in_image_target_ptr, in_dof_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        out_img_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DOF_PLANE_SEPERATION_OUTPUT_TENSOR_IDX];
        out_tensor_target_ptr = tivxMemShared2TargetPtr(&out_img_desc->mem_ptr);
        tivxMemBufferMap(out_tensor_target_ptr, out_img_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        out_yuv_img_desc = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_KERNEL_DOF_PLANE_SEPERATION_YUV_OUTPUT_TENSOR_IDX];

        if(out_yuv_img_desc != NULL){
            out_yuv_tensor_target_ptr = tivxMemShared2TargetPtr(&out_yuv_img_desc->mem_ptr);
            tivxMemBufferMap(out_yuv_tensor_target_ptr, out_yuv_img_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }

        vx_int32 data_type = TIADALG_DATA_TYPE_U08;

        if(prms->tidl_8bit_16bit_flag == 0)
        {
            data_type = TIADALG_DATA_TYPE_U08;
        }
        else if(prms->tidl_8bit_16bit_flag == 1)
        {
            data_type = TIADALG_DATA_TYPE_U16;
        }

        status = tiadalg_dof_plane_seperation_c66
                (
                    (uint32_t*)in_image_target_ptr,
                    prms->width,
                    prms->height,
                    data_type,
                    prms->pad_pixel,
                    out_tensor_target_ptr,
                    out_yuv_tensor_target_ptr
                );

        if(status!=TIADALG_PROCESS_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "tiadalg failed !!!\n");
        }

        tivxMemBufferUnmap(in_image_target_ptr, in_dof_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(out_tensor_target_ptr, out_img_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);

        if(out_yuv_img_desc != NULL)
        {
            tivxMemBufferUnmap(out_yuv_tensor_target_ptr, out_yuv_img_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }
    }

    tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

    return (status);
}
void tivxAddTargetKernelDofPlaneSep()
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {

        vx_dof_plane_seperation_target_kernel = tivxAddTargetKernelByName
                                (
                                  TIVX_KERNEL_DOF_PLANE_SEPERATION_NAME,
                                  target_name,
                                  tivxKernelDofPlaneSepProcess,
                                  tivxKernelDofPlaneSepCreate,
                                  tivxKernelDofPlaneSepDelete,
                                  NULL,
                                  NULL
                                );
    }
}

void tivxRemoveTargetKernelDofPlaneSep()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dof_plane_seperation_target_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dof_plane_seperation_target_kernel = NULL;
    }
}
