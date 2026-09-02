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

#include <stdio.h>

#include "TI/tivx.h"
#include <TI/tivx_img_proc.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_host_utils.h"
#include "tivx_od_postprocessing_host.h"

static vx_kernel vx_ODPostProc_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelImgODPostProcValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxAddKernelImgODPostProcInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelImgODPostProcValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_array configuration = NULL;
    vx_size configuration_item_size;

    vx_tensor in_points;
    vx_enum in_points_data_type;
    vx_size in_points_dims;


    vx_tensor out_points;
    vx_enum out_points_data_type;
    vx_size out_points_dims;

    vx_tensor out_valid_flag;
    vx_enum out_valid_flag_data_type;
    vx_size out_valid_flag_dims;

    if ( (num != TIVX_KERNEL_OD_POSTPROCESS_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_INPUT_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_INPUT_TABLE_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_INPUT_REV_TABLE_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_OUTPUT_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_POINTS_VALID_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (vx_array)parameters[TIVX_KERNEL_OD_POSTPROCESS_CONFIGURATION_IDX];
        in_points = (vx_tensor)parameters[TIVX_KERNEL_OD_POSTPROCESS_INPUT_POINTS_IDX];
        out_points = (vx_tensor)parameters[TIVX_KERNEL_OD_POSTPROCESS_OUTPUT_POINTS_IDX];
        out_valid_flag = (vx_tensor)parameters[TIVX_KERNEL_OD_POSTPROCESS_POINTS_VALID_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryArray(configuration, VX_ARRAY_ITEMSIZE, &configuration_item_size, sizeof(configuration_item_size)));

        tivxCheckStatus(&status, vxQueryTensor(in_points, VX_TENSOR_DATA_TYPE, &in_points_data_type, sizeof(in_points_data_type)));
        tivxCheckStatus(&status, vxQueryTensor(in_points, VX_TENSOR_NUMBER_OF_DIMS, &in_points_dims, sizeof(in_points_dims)));

        tivxCheckStatus(&status, vxQueryTensor(out_points, VX_TENSOR_DATA_TYPE, &out_points_data_type, sizeof(out_points_data_type)));
        tivxCheckStatus(&status, vxQueryTensor(out_points, VX_TENSOR_NUMBER_OF_DIMS, &out_points_dims, sizeof(out_points_dims)));

        tivxCheckStatus(&status, vxQueryTensor(out_valid_flag, VX_TENSOR_DATA_TYPE, &out_valid_flag_data_type, sizeof(out_valid_flag_data_type)));
        tivxCheckStatus(&status, vxQueryTensor(out_valid_flag, VX_TENSOR_NUMBER_OF_DIMS, &out_valid_flag_dims, sizeof(out_valid_flag_dims)));

    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ( configuration_item_size != sizeof(tivxODPostProcParams))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Configuration item size is not as per expectation %d, %d \n", configuration_item_size, sizeof(tivxODPostProcParams));
        }
        if ( (out_points_data_type != VX_TYPE_UINT16) || (out_points_dims != 2))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "out_points_data_type is not as per expectation \n");
        }
        if ( (in_points_data_type != VX_TYPE_FLOAT32) || (in_points_dims != 3))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "in_points_data_type is not as per expectation %d, %d \n", in_points_data_type, in_points_dims);
        }
        if ( (out_valid_flag_data_type != VX_TYPE_UINT8) || (out_valid_flag_dims != 1))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            printf("out_valid_flag_data_type %d \n", out_valid_flag_data_type);
            printf("out_valid_flag_dims %d \n", (unsigned int)out_valid_flag_dims);

            VX_PRINT(VX_ZONE_ERROR, "out_valid_flag_data_type or dimension is not as per expectation \n");
        }

    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelImgODPostProcInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_OD_POSTPROCESS_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_INPUT_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_OUTPUT_POINTS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OD_POSTPROCESS_POINTS_VALID_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelODPostProc(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_OD_POSTPROCESS_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_OD_POSTPROCESS_MAX_PARAMS,
                    tivxAddKernelImgODPostProcValidate,
                    tivxAddKernelImgODPostProcInitialize,
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
                        VX_TYPE_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_TENSOR,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_TENSOR,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_TENSOR,
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
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
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
    vx_ODPostProc_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelODPostProc(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_ODPostProc_kernel;

    status = vxRemoveKernel(kernel);
    vx_ODPostProc_kernel = NULL;

    return status;
}
