/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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
#include "tivx_kernel_ground_estimation.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_ground_estimation_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelGroundEstimationValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelGroundEstimationInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelGroundEstimation(vx_context context);
vx_status tivxRemoveKernelGroundEstimation(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelGroundEstimationValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;

    vx_image input_image = NULL;
    vx_df_image input_image_fmt;

    vx_image input_sdedisparity = NULL;
    vx_df_image input_sdedisparity_fmt;

    vx_image output_disparity = NULL;
    vx_df_image output_disparity_fmt;

    vx_user_data_object output_ground_model = NULL;
    vx_char output_ground_model_name[VX_MAX_REFERENCE_NAME];
    vx_size output_ground_model_size;

    if ( (num != TIVX_KERNEL_GROUND_ESTIMATION_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX];
        input_image = (vx_image)parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX];
        input_sdedisparity = (vx_image)parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX];
        output_disparity = (vx_image)parameters[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX];
        output_ground_model = (vx_user_data_object)parameters[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        tivxCheckStatus(&status, vxQueryImage(input_image, VX_IMAGE_FORMAT, &input_image_fmt, sizeof(input_image_fmt)));

        tivxCheckStatus(&status, vxQueryImage(input_sdedisparity, VX_IMAGE_FORMAT, &input_sdedisparity_fmt, sizeof(input_sdedisparity_fmt)));

        tivxCheckStatus(&status, vxQueryImage(output_disparity, VX_IMAGE_FORMAT, &output_disparity_fmt, sizeof(output_disparity_fmt)));

        tivxCheckStatus(&status, vxQueryUserDataObject(output_ground_model, VX_USER_DATA_OBJECT_NAME, &output_ground_model_name, sizeof(output_ground_model_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(output_ground_model, VX_USER_DATA_OBJECT_SIZE, &output_ground_model_size, sizeof(output_ground_model_size)));
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_ground_estimation_params_t)) ||
            (strncmp(configuration_name, "tivx_ground_estimation_params_t", sizeof(configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_ground_estimation_params_t \n");
        }

        if (VX_DF_IMAGE_U8 != input_image_fmt && VX_DF_IMAGE_NV12 != input_image_fmt && VX_DF_IMAGE_UYVY != input_image_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_image' should be an image of type:\n VX_DF_IMAGE_U8, VX_DF_IMAGE_NV12 or VX_DF_IMAGE_UYVY \n");
        }

        if (VX_DF_IMAGE_S16 != input_sdedisparity_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_sdedisparity' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

        if (VX_DF_IMAGE_S16 != output_disparity_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_disparity_fmt' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

        if ((output_ground_model_size != sizeof(tivx_ground_model_params_t)) ||
            (strncmp(output_ground_model_name, "tivx_ground_model_params_t", sizeof(output_ground_model_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_ground_model' should be a user_data_object of type:\n tivx_ground_model_params_t \n");
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    /* < DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not */
    /*                   covered by the code-generation script.) > */

    /* < DEVELOPER_TODO: (Optional) If intending to use a virtual data object, set metas using appropriate TI API. */
    /*                   For a code example, please refer to the validate callback of the follow file: */
    /*                   tiovx/kernels/openvx-core/host/vx_absdiff_host.c. For further information regarding metas, */
    /*                   please refer to the OpenVX 1.1 spec p. 260, or search for vx_kernel_validate_f. > */

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelGroundEstimationInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_GROUND_ESTIMATION_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_DISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_GROUND_ESTIMATION_OUTPUT_GROUND_MODEL_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_IMAGE_IDX];
        prms.in_img[1U] = (vx_image)parameters[TIVX_KERNEL_GROUND_ESTIMATION_INPUT_SDEDISPARITY_IDX];

        prms.num_input_images = 2;
        prms.num_output_images = 0;

        /* < DEVELOPER_TODO: (Optional) Set padding values based on valid region if border mode is */
        /*                    set to VX_BORDER_UNDEFINED and remove the #if 0 and #endif lines. */
        /*                    Else, remove this entire #if 0 ... #endif block > */
        #if 0
        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;
        prms.border_mode = VX_BORDER_UNDEFINED;
        #endif

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelGroundEstimation(vx_context context)
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
                    TIVX_KERNEL_GROUND_ESTIMATION_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_GROUND_ESTIMATION_MAX_PARAMS,
                    tivxAddKernelGroundEstimationValidate,
                    tivxAddKernelGroundEstimationInitialize,
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
        if (status == (vx_status)VX_SUCCESS)
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
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_USER_DATA_OBJECT,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            /* add supported target's */
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
    vx_ground_estimation_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelGroundEstimation(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_ground_estimation_kernel;

    status = vxRemoveKernel(kernel);
    vx_ground_estimation_kernel = NULL;

    return status;
}


