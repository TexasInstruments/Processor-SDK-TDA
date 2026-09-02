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
#include "TI/tivx_sample.h"
#include "tivx_sample_kernels_priv.h"
#include "tivx_kernel_opengl_mosaic.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_opengl_mosaic_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelOpenglMosaicValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelOpenglMosaicInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelOpenglMosaic(vx_context context);
vx_status tivxRemoveKernelOpenglMosaic(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelOpenglMosaicValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;

    vx_object_array input = NULL;
    vx_df_image input_fmt;
    vx_size num_items;
    vx_image input_image = NULL;
    vx_uint32 i;

    vx_image output = NULL;
    vx_df_image output_fmt;

    if ( (num != TIVX_KERNEL_OPENGL_MOSAIC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OPENGL_MOSAIC_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPENGL_MOSAIC_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_OPENGL_MOSAIC_CONFIGURATION_IDX];
        input = (vx_object_array)parameters[TIVX_KERNEL_OPENGL_MOSAIC_INPUT_IDX];
        output = (vx_image)parameters[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));
    }

        if (VX_SUCCESS == status)
        {
            vxQueryObjectArray(input, VX_OBJECT_ARRAY_NUMITEMS, &num_items, sizeof(num_items));

            for (i = 0; i < num_items; i++)
            {
                input_image = (vx_image)vxGetObjectArrayItem(input, i);

                vxQueryImage(input_image, VX_IMAGE_FORMAT, &input_fmt, sizeof(input_fmt));

                if (VX_DF_IMAGE_NV12 != input_fmt)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'input' should be an image of type:\n VX_DF_IMAGE_NV12 \n");
                }

                vxReleaseImage(&input_image);
            }

            tivxCheckStatus(&status, vxQueryImage(output, VX_IMAGE_FORMAT, &output_fmt, sizeof(output_fmt)));

        }
    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_opengl_mosaic_params_t)) ||
            (strncmp(configuration_name, "tivx_opengl_mosaic_params_t", sizeof(configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be an user_data_object of type:\n tivx_opengl_mosaic_params_t \n");
        }
    }

    if (VX_SUCCESS == status)
    {
        if (VX_DF_IMAGE_RGBX != output_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output' should be an image of type:\n VX_DF_IMAGE_RGBX \n");
        }
    }

    /* CUSTOM PARAMETER CHECKING if any */

    return status;
}

static vx_status VX_CALLBACK tivxAddKernelOpenglMosaicInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_OPENGL_MOSAIC_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OPENGL_MOSAIC_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPENGL_MOSAIC_INPUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.out_img[0U] = (vx_image)parameters[TIVX_KERNEL_OPENGL_MOSAIC_OUTPUT_IDX];

        prms.num_input_images = 0;
        prms.num_output_images = 1;

        prms.top_pad = 0;
        prms.bot_pad = 0;
        prms.left_pad = 0;
        prms.right_pad = 0;
        prms.border_mode = VX_BORDER_UNDEFINED;

        status = tivxKernelConfigValidRect(&prms);
    }

    return status;
}

vx_status tivxAddKernelOpenglMosaic(vx_context context)
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
                    TIVX_KERNEL_OPENGL_MOSAIC_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_OPENGL_MOSAIC_MAX_PARAMS,
                    tivxAddKernelOpenglMosaicValidate,
                    tivxAddKernelOpenglMosaicInitialize,
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
                        VX_TYPE_OBJECT_ARRAY,
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
            tivxAddKernelTarget(kernel, TIVX_TARGET_MPU_0);
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
    vx_opengl_mosaic_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelOpenglMosaic(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_opengl_mosaic_kernel;

    status = vxRemoveKernel(kernel);
    vx_opengl_mosaic_kernel = NULL;

    return status;
}


