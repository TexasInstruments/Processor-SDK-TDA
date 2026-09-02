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

#include "TI/tivx.h"
#include <TI/tivx_img_proc.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_host_utils.h"
#include "tivx_sfm_host.h"

static vx_kernel vx_sfm_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelSFMValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelSFMInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelSFMValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image input = (vx_image)parameters[TIVX_KERNEL_SFM_INPUT_IMAGE_IDX];
    vx_uint32 input_w;
    vx_uint32 input_h;
    vx_df_image input_fmt;

    vx_image flow = (vx_image)parameters[TIVX_KERNEL_SFM_FLOW_VECTOR_IDX];
    vx_uint32 flow_w;
    vx_uint32 flow_h;
    vx_df_image flow_fmt;

    vx_image output = (vx_image)parameters[TIVX_KERNEL_SFM_OUTPUT_PTCLD_IMAGE_IDX];
    vx_uint32 output_w;
    vx_uint32 output_h;
    vx_df_image output_fmt;

    if ( (num != TIVX_KERNEL_SFM_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SFM_CONFIG_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_CREATE_PARAMS_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_IN_ARGS_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUT_ARGS_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_FLOW_VECTOR_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUTPUT_PTCLD_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUTPUT_OCPGRD_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUTPUT_FEATURE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_WIDTH, &input_w, sizeof(input_w)));
        tivxCheckStatus(&status, vxQueryImage(input, (vx_enum)VX_IMAGE_HEIGHT, &input_h, sizeof(input_h)));

        tivxCheckStatus(&status, vxQueryImage(flow, (vx_enum)VX_IMAGE_FORMAT, &flow_fmt, sizeof(flow_fmt)));
        tivxCheckStatus(&status, vxQueryImage(flow, (vx_enum)VX_IMAGE_WIDTH,  &flow_w,   sizeof(flow_w)));
        tivxCheckStatus(&status, vxQueryImage(flow, (vx_enum)VX_IMAGE_HEIGHT, &flow_h,   sizeof(flow_h)));

        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_WIDTH,  &output_w,   sizeof(output_w)));
        tivxCheckStatus(&status, vxQueryImage(output, (vx_enum)VX_IMAGE_HEIGHT, &output_h,   sizeof(output_h)));

    }

    /* CUSTOM PARAMETER CHECKING */

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelSFMInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_SFM_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_SFM_CONFIG_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_CREATE_PARAMS_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_IN_ARGS_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUT_ARGS_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_FLOW_VECTOR_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUTPUT_PTCLD_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUTPUT_OCPGRD_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_SFM_OUTPUT_FEATURE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    return status;
}

vx_status tivxAddKernelSFM(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t index;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_SFM_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_SFM_MAX_PARAMS,
                    tivxAddKernelSFMValidate,
                    tivxAddKernelSFMInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /*TIVX_KERNEL_SFM_OUTPUT_PTCLD_IMAGE_IDX*/
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /*TIVX_KERNEL_SFM_OUTPUT_OCPGRD_IMAGE_IDX*/
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /*TIVX_KERNEL_SFM_OUTPUT_FEATURE_IDX*/
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxAddKernelTarget(kernel, TIVX_TARGET_DSP_C7_1);
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        if (status != (vx_status)VX_SUCCESS)
        {
            vxReleaseKernel(&kernel);
            kernel = NULL;
        }
    }
    else
    {
        kernel = NULL;
    }
    vx_sfm_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelSFM(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_sfm_kernel;

    status = vxRemoveKernel(kernel);
    vx_sfm_kernel = NULL;

    return status;
}


