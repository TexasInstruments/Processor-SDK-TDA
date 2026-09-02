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
#include "TI/tivx_srv.h"
#include "tivx_srv_kernels_priv.h"
#include "tivx_kernel_point_detect.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_point_detect_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelPointDetectValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelPointDetectInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelPointDetect(vx_context context);
vx_status tivxRemoveKernelPointDetect(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelPointDetectValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_image in = NULL;
    vx_df_image in_fmt;

    vx_user_data_object in_configuration = NULL;
    vx_char in_configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size in_configuration_size;

    vx_user_data_object in_ldclut = NULL;
    vx_char in_ldclut_name[VX_MAX_REFERENCE_NAME];
    vx_size in_ldclut_size;

    vx_user_data_object out_configuration = NULL;
    vx_char out_configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size out_configuration_size;

    vx_image buf_bwluma_frame = NULL;
    vx_df_image buf_bwluma_frame_fmt;

    if ( (num != TIVX_KERNEL_POINT_DETECT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_IN_LDCLUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_OUT_CONFIGURATION_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        in = (vx_image)parameters[TIVX_KERNEL_POINT_DETECT_IN_IDX];
        in_configuration = (vx_user_data_object)parameters[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX];
        in_ldclut = (vx_user_data_object)parameters[TIVX_KERNEL_POINT_DETECT_IN_LDCLUT_IDX];
        out_configuration = (vx_user_data_object)parameters[TIVX_KERNEL_POINT_DETECT_OUT_CONFIGURATION_IDX];
        buf_bwluma_frame = (vx_image)parameters[TIVX_KERNEL_POINT_DETECT_BUF_BWLUMA_FRAME_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(in_configuration, VX_USER_DATA_OBJECT_NAME, &in_configuration_name, sizeof(in_configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_configuration, VX_USER_DATA_OBJECT_SIZE, &in_configuration_size, sizeof(in_configuration_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(in_ldclut, VX_USER_DATA_OBJECT_NAME, &in_ldclut_name, sizeof(in_ldclut_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_ldclut, VX_USER_DATA_OBJECT_SIZE, &in_ldclut_size, sizeof(in_ldclut_size)));

        tivxCheckStatus(&status, vxQueryImage(in, VX_IMAGE_FORMAT, &in_fmt, sizeof(in_fmt)));

        tivxCheckStatus(&status, vxQueryUserDataObject(out_configuration, VX_USER_DATA_OBJECT_NAME, &out_configuration_name, sizeof(out_configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(out_configuration, VX_USER_DATA_OBJECT_SIZE, &out_configuration_size, sizeof(out_configuration_size)));

        if (NULL != buf_bwluma_frame)
        {
            tivxCheckStatus(&status, vxQueryImage(buf_bwluma_frame, VX_IMAGE_FORMAT, &buf_bwluma_frame_fmt, sizeof(buf_bwluma_frame_fmt)));
        }
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ((in_configuration_size != sizeof(svPointDetect_t)) ||
            (strncmp(in_configuration_name, "svPointDetect_t", sizeof(in_configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_configuration' should be a user_data_object of type:\n svPointDetect_t \n");
        }

        if ((in_ldclut_size != sizeof(svLdcLut_t)) ||
            (strncmp(in_ldclut_name, "svLdcLut_t", sizeof(in_ldclut_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_ldclut' should be a user_data_object of type:\n svLdcLut_t \n");
        }

        if ((VX_DF_IMAGE_U8 != in_fmt) && (VX_DF_IMAGE_NV12 != in_fmt))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in' should be an image of type:\n VX_DF_IMAGE_U8 \n");
        }

        if ((out_configuration_size != sizeof(svACDetectStructFinalCorner_t)) ||
            (strncmp(out_configuration_name, "svACDetectStructFinalCorner_t", sizeof(out_configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out_configuration' should be a user_data_object of type:\n svACDetectStructFinalCorner_t \n");
        }

        if (NULL != buf_bwluma_frame)
        {
            if (VX_DF_IMAGE_U8 != buf_bwluma_frame_fmt)
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'buf_bwluma_frame' should be an image of type:\n VX_DF_IMAGE_U8 \n");
            }
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

static vx_status VX_CALLBACK tivxAddKernelPointDetectInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_POINT_DETECT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_IN_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_IN_LDCLUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_IN_IDX])
        || (NULL == parameters[TIVX_KERNEL_POINT_DETECT_OUT_CONFIGURATION_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_POINT_DETECT_IN_IDX];
        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_POINT_DETECT_BUF_BWLUMA_FRAME_IDX];

        prms.num_input_images = 1;
        prms.num_output_images = 1;

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

vx_status tivxAddKernelPointDetect(vx_context context)
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
                    TIVX_KERNEL_POINT_DETECT_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_POINT_DETECT_MAX_PARAMS,
                    tivxAddKernelPointDetectValidate,
                    tivxAddKernelPointDetectInitialize,
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
                        VX_TYPE_USER_DATA_OBJECT,
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
                        VX_PARAMETER_STATE_OPTIONAL
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
    vx_point_detect_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelPointDetect(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_point_detect_kernel;

    status = vxRemoveKernel(kernel);
    vx_point_detect_kernel = NULL;

    return status;
}


