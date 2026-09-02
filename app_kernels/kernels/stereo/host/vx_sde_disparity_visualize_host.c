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
#include "tivx_kernel_sde_disparity_visualize.h"
#include "TI/tivx_target_kernel.h"

#include <stdio.h>


static vx_kernel vx_sde_disparity_visualize_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelSdeDisparityVisualizeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelSdeDisparityVisualizeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);

static vx_status VX_CALLBACK tivxAddKernelSdeDisparityVisualizeValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;

    vx_image img[2U] = {NULL};
    vx_df_image fmt[2U] = {0};
    vx_uint32 w[2U], h[2U];


    //status = tivxKernelValidateParametersNotNull(parameters, TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_MAX_PARAMS);

    if (VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_CONFIGURATION_IDX];
    }

    if (VX_SUCCESS == status)
    {
        //img[0U] = (vx_image)parameters[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_DISPARITY_IDX];
        img[1U] = (vx_image)parameters[TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_DISPARITY_RGB_IDX];
    }

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
    }
#if 0
    if (VX_SUCCESS == status)
    {
        /* Get the image width/height and format */
        status = vxQueryImage(img[0U], VX_IMAGE_FORMAT, &fmt[0U],
            sizeof(fmt[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_WIDTH, &w[0U], sizeof(w[0U]));
        status |= vxQueryImage(img[0U], VX_IMAGE_HEIGHT, &h[0U], sizeof(h[0U]));
    }
#endif

    if (VX_SUCCESS == status)
    {
        /* Get the image width/height and format */
        status = vxQueryImage(img[1U], VX_IMAGE_FORMAT, &fmt[1U],
            sizeof(fmt[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_WIDTH, &w[1U], sizeof(w[1U]));
        status |= vxQueryImage(img[1U], VX_IMAGE_HEIGHT, &h[1U], sizeof(h[1U]));
    }

    /* PARAMETER CHECKING */
    if (VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_sde_disparity_vis_params_t)) ||
            (strncmp(configuration_name, "tivx_sde_disparity_vis_params_t", sizeof(configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_sde_disparity_vis_params_t \n");
        }
    }

    if (VX_SUCCESS == status)
    {
#if 0
        if (VX_DF_IMAGE_S16 != fmt[0U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "SDE_DISPARITY_VISUALIZE: Invalid data format. Disparity MUST be VX_DF_IMAGE_S16\n");
        }
#endif
        if (VX_DF_IMAGE_RGB != fmt[1U])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "SDE_DISPARITY_VISUALIZE: Invalid data format. Disparity image MUST be VX_DF_IMAGE_RGB\n");
        }

#if 0
        if( (w[0u] != w[1u]) || (h[0u] != h[1u]) )
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "SDE_DISPARITY_VISUALIZE: Invalid image dimensions. Disparity and disparity images MUST have same WxH\n");
        }
#endif
    }

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelSdeDisparityVisualizeInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if (num_params != TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
    }

    return status;
}

vx_status tivxAddKernelSdeDisparityVisualize(vx_context context)
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
                    TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_MAX_PARAMS,
                    tivxAddKernelSdeDisparityVisualizeValidate,
                    tivxAddKernelSdeDisparityVisualizeInitialize,
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
        #if defined (SOC_TDA54)
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
    vx_sde_disparity_visualize_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelSdeDisparityVisualize(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_sde_disparity_visualize_kernel;

    status = vxRemoveKernel(kernel);
    vx_sde_disparity_visualize_kernel = NULL;

    return status;
}


