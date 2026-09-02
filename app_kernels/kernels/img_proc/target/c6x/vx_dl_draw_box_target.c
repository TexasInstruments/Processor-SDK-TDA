/*
*
* Copyright (c) 2021-2026 Texas Instruments Incorporated
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
#include <tivx_kernels_target_utils.h>
#include <utils/mem/include/app_mem.h>

#include <tivx_dl_draw_box_host.h>

static tivx_target_kernel vx_dlDrawBox_kernel = NULL;

static vx_status VX_CALLBACK tivxKernelDLDrawBoxCreate
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

    for (i = 0U; i < num_params; i ++)
    {
        if (NULL == obj_desc[i])
        {
            status = VX_FAILURE;
            break;
        }
    }

    tivxDLDrawBoxParams * prms = NULL;

    prms = tivxMemAlloc(sizeof(tivxDLDrawBoxParams), TIVX_MEM_EXTERNAL);
    if (NULL == prms)
    {
        status = VX_FAILURE;
    }
    else
    {
        status = tivxSetTargetKernelInstanceContext(kernel, prms,  sizeof(tivxDLDrawBoxParams));
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLDrawBoxDelete(
    tivx_target_kernel_instance kernel, tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params, void *priv_arg)
{
    vx_status status = VX_SUCCESS;
    int32_t i;

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
        tivxDLDrawBoxParams *prms = NULL;

        status = tivxGetTargetKernelInstanceContext(kernel,
            (void **)&prms, &size);

        if (VX_SUCCESS == status)
        {
            tivxMemFree(prms, sizeof(tivxDLDrawBoxParams), TIVX_MEM_EXTERNAL);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxKernelDLDrawBoxProcess
(
    tivx_target_kernel_instance kernel,
    tivx_obj_desc_t *obj_desc[],
    vx_uint16 num_params,
    void *priv_arg
)
{
    vx_status status = VX_SUCCESS;

    tivxDLDrawBoxParams *prms = NULL;
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
            (sizeof(tivxDLDrawBoxParams) != size))
        {
            status = VX_FAILURE;
        }
    }

    if(VX_SUCCESS == status)
    {
        tivx_obj_desc_user_data_object_t* config_desc;
        void * config_target_ptr = NULL;

        tivx_obj_desc_tensor_t *in_tensor_desc;
        void* in_tensor_target_ptr;

        tivx_obj_desc_image_t* input_image_desc;
        void * input_image_target_ptr[2];

        input_image_target_ptr[0] = NULL;
        input_image_target_ptr[1] = NULL;

        tivx_obj_desc_image_t *output_image_desc;
        void * output_image_target_ptr[2];

        output_image_target_ptr[0] = NULL;
        output_image_target_ptr[1] = NULL;

        config_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_DL_DRAW_BOX_CONFIG_IDX];
        config_target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);
        tivxMemBufferMap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST,VX_READ_ONLY);

        in_tensor_desc  = (tivx_obj_desc_tensor_t *)obj_desc[TIVX_DL_DRAW_BOX_INPUT_TENSOR_IDX];
        in_tensor_target_ptr  = tivxMemShared2TargetPtr(&in_tensor_desc->mem_ptr);
        tivxMemBufferMap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        input_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_DRAW_BOX_INPUT_IMAGE_IDX];
        input_image_target_ptr[0] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[0]);
        tivxMemBufferMap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        if(input_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            input_image_target_ptr[1] = tivxMemShared2TargetPtr(&input_image_desc->mem_ptr[1]);
            tivxMemBufferMap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_READ_ONLY);
        }

        output_image_desc = (tivx_obj_desc_image_t *)obj_desc[TIVX_DL_DRAW_BOX_OUTPUT_IMAGE_IDX];
        output_image_target_ptr[0] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[0]);
        tivxMemBufferMap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST,VX_WRITE_ONLY);
        if(output_image_desc->mem_ptr[1].shared_ptr != 0)
        {
            output_image_target_ptr[1] = tivxMemShared2TargetPtr(&output_image_desc->mem_ptr[1]);
            tivxMemBufferMap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST,VX_WRITE_ONLY);
        }

        // implement here

        tivxMemBufferUnmap(config_target_ptr, config_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        tivxMemBufferUnmap(in_tensor_target_ptr, in_tensor_desc->mem_size, VX_MEMORY_TYPE_HOST, VX_READ_ONLY);

        tivxMemBufferUnmap(input_image_target_ptr[0], input_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        if(input_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(input_image_target_ptr[1], input_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_READ_ONLY);
        }

        tivxMemBufferUnmap(output_image_target_ptr[0], output_image_desc->mem_size[0], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        if(output_image_target_ptr[1] != NULL)
        {
            tivxMemBufferUnmap(output_image_target_ptr[1], output_image_desc->mem_size[1], VX_MEMORY_TYPE_HOST, VX_WRITE_ONLY);
        }
    }

    return (status);
}

void tivxAddTargetKernelDLDrawBox()
{
    char target_name[TIVX_TARGET_MAX_NAME];

    if( (vx_status)VX_SUCCESS == tivxKernelsTargetUtilsAssignTargetNameDsp(target_name))
    {
        vx_dlDrawBox_kernel = tivxAddTargetKernelByName
                              (
                                TIVX_KERNEL_DL_DRAW_BOX_NAME,
                                target_name,
                                tivxKernelDLDrawBoxProcess,
                                tivxKernelDLDrawBoxCreate,
                                tivxKernelDLDrawBoxDelete,
                                NULL,
                                NULL
                              );
    }
}

void tivxRemoveTargetKernelDLDrawBox()
{
    vx_status status = VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dlDrawBox_kernel);
    if (status == VX_SUCCESS)
    {
        vx_dlDrawBox_kernel = NULL;
    }
}
