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

#include "TI/tivx.h"
#include "TI/tivx_stereo.h"
#include "tivx_stereo_kernels_priv.h"
#include "tivx_kernel_sde_histogram_visualize.h"
#include "TI/tivx_target_kernel.h"

#include <stdio.h>


static vx_kernel vx_sde_histogram_visualize_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelSdeHistogramVisualizeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelSdeHistogramVisualizeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelSdeHistogramVisualizeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_image img = {NULL};
    vx_df_image fmt = {0};
    vx_uint32 w, h;

    status = tivxKernelValidateParametersNotNull(parameters, TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        img = (vx_image)parameters[TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_HISTOGRAM_IMAGE_IDX];
    }

    if (VX_SUCCESS == status)
    {
        /* Get the image width/height and format */
        status  = vxQueryImage(img, VX_IMAGE_FORMAT, &fmt, sizeof(fmt));
        status |= vxQueryImage(img, VX_IMAGE_WIDTH, &w, sizeof(w));
        status |= vxQueryImage(img, VX_IMAGE_HEIGHT, &h, sizeof(h));
    }

    if (VX_SUCCESS == status)
    {
        if (VX_DF_IMAGE_U8 != fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "SDE_HISTOGRAM_VISUALIZE: Invalid data format. Histogram image MUST be VX_DF_IMAGE_U8\n");
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelSdeHistogramVisualizeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if (num_params != TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_status tivxAddKernelSdeHistogramVisualize(vx_context context)
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
                    TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_MAX_PARAMS,
                    tivxAddKernelSdeHistogramVisualizeValidate,
                    tivxAddKernelSdeHistogramVisualizeInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }

    if (status == VX_SUCCESS)
    {
        index = 0;

        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_INPUT,
                        VX_TYPE_DISTRIBUTION,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_IMAGE,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }

        if (status == VX_SUCCESS)
        {
            /* add supported target's */
        #if defined(SOC_TDA54)
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU1);
        #else
            tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_0);
        #endif
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
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
    vx_sde_histogram_visualize_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelSdeHistogramVisualize(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_sde_histogram_visualize_kernel;

    status = vxRemoveKernel(kernel);
    vx_sde_histogram_visualize_kernel = NULL;

    return status;
}


