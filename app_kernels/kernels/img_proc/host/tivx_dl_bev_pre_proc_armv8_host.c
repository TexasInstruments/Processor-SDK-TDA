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


#include <TI/tivx_target_kernel.h>
#include <tivx_kernels_host_utils.h>
#include <tivx_dl_bev_pre_proc_armv8_host.h>


#include "TI/tivx.h"
#include "TI/tivx_img_proc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_host_utils.h"


static vx_kernel vx_DLPreProcArmv8_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelDLPreProc4DArmv8Validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelDLPreProc4DArmv8Initialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelDLPreProc4DArmv8Validate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object config = NULL;
    vx_size config_size;

    vx_image in_img;
    vx_uint32 in_img_width;
    vx_uint32 in_img_height;
    vx_size in_img_planes;
    vx_size in_img_size;

    vx_tensor out_tensor;
    vx_size out_tensor_dims;

/*BEV 4D Changes*/
    if ( (num != TIVX_KERNEL_DL_PRE_PROC_ARMV8_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_CONFIG_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE2_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE3_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE4_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE5_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE6_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_OUTPUT_TENSOR_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        config     = (vx_user_data_object)parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_CONFIG_IDX];
        in_img     = (vx_image)parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE_IDX];
        out_tensor = (vx_tensor)parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_OUTPUT_TENSOR_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(config, VX_USER_DATA_OBJECT_SIZE, &config_size, sizeof(config_size)));

        tivxCheckStatus(&status, vxQueryImage(in_img, VX_IMAGE_WIDTH, &in_img_width, sizeof(in_img_width)));
        tivxCheckStatus(&status, vxQueryImage(in_img, VX_IMAGE_HEIGHT, &in_img_height, sizeof(in_img_height)));
        tivxCheckStatus(&status, vxQueryImage(in_img, VX_IMAGE_PLANES, &in_img_planes, sizeof(in_img_planes)));
        tivxCheckStatus(&status, vxQueryImage(in_img, VX_IMAGE_SIZE, &in_img_size, sizeof(in_img_size)));

        tivxCheckStatus(&status, vxQueryTensor(out_tensor, VX_TENSOR_NUMBER_OF_DIMS, &out_tensor_dims, sizeof(out_tensor_dims)));
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ( config_size != sizeof(tivxDLPreProcArmv8Params4D))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'config' should be an user data object of type:\n tivxDLPreProcArmv8Params \n");
        }

        if (((out_tensor_dims != 3) && (out_tensor_dims != 4)))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Out image data type or dimension is not as per expectation %d\n", out_tensor_dims);
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    /* < DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not */
    /*                   covered by the code-generation script.) > */

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelDLPreProc4DArmv8Initialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_DL_PRE_PROC_ARMV8_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_CONFIG_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_DL_PRE_PROC_ARMV8_OUTPUT_TENSOR_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelDLPreProc4DArmv8(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;
    /*BEV 4D Change*/
    vx_char name[256] = TIVX_KERNEL_DL_PRE_PROC_ARMV8_4D_NAME;
    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    name,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_DL_PRE_PROC_ARMV8_MAX_PARAMS,
                    tivxAddKernelDLPreProc4DArmv8Validate,
                    tivxAddKernelDLPreProc4DArmv8Initialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        /*###########################################################################################################
        -------------------------------Modifications to take 6 BEV Image inputs into kernel---------------------------
         ############################################################################################################*/
         if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
         if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
         if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
         if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_TENSOR,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add Armv8 target */
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_0);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_2);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_3);
        }
        if (status == VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_DLPreProcArmv8_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelDLPreProc4DArmv8(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_DLPreProcArmv8_kernel;

    status = vxRemoveKernel(kernel);
    vx_DLPreProcArmv8_kernel = NULL;

    return status;
}
