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

#include "TI/tivx.h"
#include "TI/tivx_stereo.h"
#include "tivx_stereo_kernels_priv.h"
#include "tivx_kernel_extract_disparity_confidence.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_extract_disparity_confidence_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelExtractDisparityConfidenceValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelExtractDisparityConfidenceInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelExtractDisparityConfidence(vx_context context);
vx_status tivxRemoveKernelExtractDisparityConfidence(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelExtractDisparityConfidenceValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_image input_sdedisparity = NULL;
    vx_df_image input_sdedisparity_fmt;

    vx_array output_disparity = NULL;
    vx_enum output_disparity_item_type;
    vx_size output_disparity_item_size;

    vx_array output_confidence = NULL;
    vx_enum output_confidence_item_type;
    vx_size output_confidence_item_size;

    if ( (num != TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_INPUT_SDEDISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_OUTPUT_DISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_OUTPUT_CONFIDENCE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        input_sdedisparity = (vx_image)parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_INPUT_SDEDISPARITY_IDX];
        output_disparity = (vx_array)parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_OUTPUT_DISPARITY_IDX];
        output_confidence = (vx_array)parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_OUTPUT_CONFIDENCE_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryImage(input_sdedisparity, VX_IMAGE_FORMAT, &input_sdedisparity_fmt, sizeof(input_sdedisparity_fmt)));

        tivxCheckStatus(&status, vxQueryArray(output_disparity, VX_ARRAY_ITEMTYPE, &output_disparity_item_type, sizeof(output_disparity_item_type)));
        tivxCheckStatus(&status, vxQueryArray(output_disparity, VX_ARRAY_ITEMSIZE, &output_disparity_item_size, sizeof(output_disparity_item_size)));

        tivxCheckStatus(&status, vxQueryArray(output_confidence, VX_ARRAY_ITEMTYPE, &output_confidence_item_type, sizeof(output_confidence_item_type)));
        tivxCheckStatus(&status, vxQueryArray(output_confidence, VX_ARRAY_ITEMSIZE, &output_confidence_item_size, sizeof(output_confidence_item_size)));
    }

    /* PARAMETER CHECKING */

    if ((vx_status)VX_SUCCESS == status)
    {
        if (VX_DF_IMAGE_S16 != input_sdedisparity_fmt)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_sdedisparity' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

        if ( output_disparity_item_size != sizeof(float))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_disparity' should be an array of type:\n float \n");
        }

        if ( output_confidence_item_size != sizeof(uint8_t))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_confidence' should be an array of type:\n uint8_t \n");
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

static vx_status VX_CALLBACK tivxAddKernelExtractDisparityConfidenceInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_INPUT_SDEDISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_OUTPUT_DISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_OUTPUT_CONFIDENCE_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_INPUT_SDEDISPARITY_IDX];

        prms.num_input_images = 1;
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

vx_status tivxAddKernelExtractDisparityConfidence(vx_context context)
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
                    TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_MAX_PARAMS,
                    tivxAddKernelExtractDisparityConfidenceValidate,
                    tivxAddKernelExtractDisparityConfidenceInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    if (status == (vx_status)VX_SUCCESS)
    {
        index = 0;

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
                        VX_TYPE_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxKernelsHostUtilsAddKernelTargetDsp(kernel);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_0);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_1);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_2);
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_3);
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
    vx_extract_disparity_confidence_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelExtractDisparityConfidence(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_extract_disparity_confidence_kernel;

    status = vxRemoveKernel(kernel);
    vx_extract_disparity_confidence_kernel = NULL;

    return status;
}


