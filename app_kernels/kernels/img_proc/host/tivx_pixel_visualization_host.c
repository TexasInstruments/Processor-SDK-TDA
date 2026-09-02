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
#include <TI/tivx_img_proc.h>
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_host_utils.h"
#include "tivx_pixel_visualization_host.h"
#include <stdio.h>

static vx_status VX_CALLBACK tivxPixelVizValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);

static vx_status VX_CALLBACK tivxPixelVizValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;
    vx_int32 i;

    vx_user_data_object configuration = NULL;
    vx_size configuration_item_size;

    vx_tensor in_det;
    vx_size in_det_dims;

    vx_image out_det;
    vx_uint32 out_img_width;
    vx_uint32 out_img_height;
    vx_size out_img_size;
    vx_size out_img_planes;

    vx_int32 num_output_tensors = 0;

    tivxPixelVizParams     *config_buffer = NULL;
    vx_map_id map_id_config;

    if (num > TIVX_KERNEL_PIXEL_VISUALIZATION_MAX_PARAMS)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_PIXEL_VISUALIZATION_CONFIGURATION_IDX];
        if (configuration == NULL)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Configuration parameter is null \n");
        }
        else
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE , &configuration_item_size, sizeof(configuration_item_size)));
            if ( configuration_item_size != sizeof(tivxPixelVizParams))
            {
                status = VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Configuration item size is not as per expectation %d, %d \n", configuration_item_size, sizeof(tivxPixelVizParams));
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        vxMapUserDataObject(configuration, 0, sizeof(tivxPixelVizParams), &map_id_config, (void**)&config_buffer, VX_READ_ONLY, VX_MEMORY_TYPE_HOST, 0);

        num_output_tensors = config_buffer->num_output_tensors;
    }

    if(status == VX_SUCCESS)
    {
        if(NULL == parameters[TIVX_KERNEL_PIXEL_VISUALIZATION_TIDLOUTARGS_IDX])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Outargs reference is set to NULL\n");
        }
    }
    if(status == VX_SUCCESS)
    {
        if(NULL == parameters[TIVX_KERNEL_PIXEL_VISUALIZATION_INPUT_IMAGE_IDX])
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Input image reference is set to NULL\n");
        }
    }

    /* PARAMETER ATTRIBUTE FETCH */

    for(i = 0; i < num_output_tensors; i++)
    {
        if (VX_SUCCESS == status)
        {
            in_det = (vx_tensor)parameters[TIVX_KERNEL_PIXEL_VISUALIZATION_OUTPUT_TENSOR_IDX + i];
            if(in_det != NULL)
            {
                tivxCheckStatus(&status, vxQueryTensor(in_det, VX_TENSOR_NUMBER_OF_DIMS, &in_det_dims, sizeof(in_det_dims)));
                if (in_det_dims != 3)
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "out_points_data_type is not as per expectation \n");
                }
            }
        }
    }

    for(; i < (num_output_tensors * 2); i++)
    {
        if (VX_SUCCESS == status)
        {
            out_det = (vx_image)parameters[TIVX_KERNEL_PIXEL_VISUALIZATION_OUTPUT_TENSOR_IDX + i];
            if(out_det != NULL)
            {
                tivxCheckStatus(&status, vxQueryImage(out_det, VX_IMAGE_WIDTH, &out_img_width, sizeof(out_img_width)));
                tivxCheckStatus(&status, vxQueryImage(out_det, VX_IMAGE_HEIGHT, &out_img_height, sizeof(out_img_height)));
                tivxCheckStatus(&status, vxQueryImage(out_det, VX_IMAGE_PLANES, &out_img_planes, sizeof(out_img_planes)));
                tivxCheckStatus(&status, vxQueryImage(out_det, VX_IMAGE_SIZE, &out_img_size, sizeof(out_img_size)));
                if (((out_img_size != (out_img_width*out_img_height*out_img_planes*3))&&(config_buffer->op_rgb_or_yuv!=1)) ||
                    ((out_img_size != (out_img_width*out_img_height*1.5))&&(config_buffer->op_rgb_or_yuv==1)) )
                {
                    status = VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "out_points_data_type is not as per expectation \n");
                }
            }
        }
    }

    if (VX_SUCCESS == status)
    {
        vxUnmapUserDataObject(configuration, map_id_config);
    }

    return status;
}


vx_kernel tivxAddKernelPixelViz(vx_context context, vx_int32 num_output_tensors)
{
    vx_kernel kernel;
    vx_status status;
    vx_uint32 index;
    vx_enum kernel_id;
    vx_int32 i;

    vx_char kernel_name[VX_MAX_KERNEL_NAME];

    /* Create kernel name by concatonating kernel name with number of outputs to create a unique kernel */
    snprintf( kernel_name, VX_MAX_KERNEL_NAME, "%s:%d", TIVX_KERNEL_PIXEL_VISUALIZATION_NAME, num_output_tensors );

    kernel = vxGetKernelByName(context, kernel_name);

    if ( NULL == kernel)
    {
        status = vxAllocateUserKernelId(context, &kernel_id);
        if(status != VX_SUCCESS)
        {
            VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
        }


        if (status == VX_SUCCESS)
        {
            /* Number of parameters are config + outArgs + input image + (output tensors * 2) */
            uint32_t num_params = 3 + (num_output_tensors * 2);
            kernel = vxAddUserKernel(
                        context,
                        kernel_name,
                        kernel_id,
                        NULL,
                        num_params,
                        tivxPixelVizValidate,
                        NULL,
                        NULL);

            status = vxGetStatus((vx_reference)kernel);
        }

        index = 0;
        if (status == VX_SUCCESS)
        {
           status = vxAddParameterToKernel(kernel,
                            index,
                            VX_INPUT,
                            VX_TYPE_USER_DATA_OBJECT,
                            VX_PARAMETER_STATE_REQUIRED);
           index++;
        }
        if (status == VX_SUCCESS)
        {
           status = vxAddParameterToKernel(kernel,
                            index,
                            VX_INPUT,
                            VX_TYPE_USER_DATA_OBJECT,
                            VX_PARAMETER_STATE_REQUIRED);
           index++;
        }
        if (status == VX_SUCCESS)
        {
           status = vxAddParameterToKernel(kernel,
                            index,
                            VX_INPUT,
                            VX_TYPE_IMAGE,
                            VX_PARAMETER_STATE_REQUIRED);
           index++;
        }
        for(i = 0; i < num_output_tensors; i++)
        {
            if (status == VX_SUCCESS)
            {
                  status = vxAddParameterToKernel(kernel,
                              index,
                              VX_INPUT,
                              VX_TYPE_TENSOR,
                              VX_PARAMETER_STATE_REQUIRED);
                  index++;
            }
        }
        for(i = 0; i < num_output_tensors; i++)
        {
            if (status == VX_SUCCESS)
            {
                  status = vxAddParameterToKernel(kernel,
                              index,
                              VX_OUTPUT,
                              VX_TYPE_IMAGE,
                              VX_PARAMETER_STATE_REQUIRED);
                  index++;
            }
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

    return kernel;
}

