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
#include "tivx_kernel_obstacle_detection.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_obstacle_detection_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelObstacleDetectionValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelObstacleDetectionInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelObstacleDetection(vx_context context);
vx_status tivxRemoveKernelObstacleDetection(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelObstacleDetectionValidate(vx_node node,
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

    vx_image input_disparity = NULL;
    vx_df_image input_disparity_fmt;

    vx_user_data_object input_ground_model = NULL;
    vx_char input_ground_model_name[VX_MAX_REFERENCE_NAME];
    vx_size input_ground_model_size;

    vx_array output_obstacle_pos = NULL;
    vx_enum output_obstacle_pos_item_type;
    vx_size output_obstacle_pos_item_size;

    vx_scalar output_num_obstacles = NULL;
    vx_enum output_num_obstacles_scalar_type;

    vx_array output_freespace_boundary = NULL;
    vx_enum output_freespace_boundary_item_type;
    vx_size output_freespace_boundary_item_size;

    vx_user_data_object output_drivable_space = NULL;
    vx_char output_drivable_space_name[VX_MAX_REFERENCE_NAME];
    vx_size output_drivable_space_size;

    if ( (num != TIVX_KERNEL_OBSTACLE_DETECTION_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_DISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_GROUND_MODEL_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_OBSTACLE_POS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_NUM_OBSTACLES_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_FREESPACE_BOUNDARY_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_DRIVABLE_SPACE_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_CONFIGURATION_IDX];
        input_image = (vx_image)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_IMAGE_IDX];
        input_disparity = (vx_image)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_DISPARITY_IDX];
        input_ground_model = (vx_user_data_object)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_GROUND_MODEL_IDX];
        output_obstacle_pos = (vx_array)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_OBSTACLE_POS_IDX];
        output_num_obstacles = (vx_scalar)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_NUM_OBSTACLES_IDX];
        output_freespace_boundary = (vx_array)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_FREESPACE_BOUNDARY_IDX];
        output_drivable_space = (vx_user_data_object)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_DRIVABLE_SPACE_IDX];
    }

    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        tivxCheckStatus(&status, vxQueryImage(input_image, VX_IMAGE_FORMAT, &input_image_fmt, sizeof(input_image_fmt)));

        tivxCheckStatus(&status, vxQueryImage(input_disparity, VX_IMAGE_FORMAT, &input_disparity_fmt, sizeof(input_disparity_fmt)));

        tivxCheckStatus(&status, vxQueryUserDataObject(input_ground_model, VX_USER_DATA_OBJECT_NAME, &input_ground_model_name, sizeof(input_ground_model_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(input_ground_model, VX_USER_DATA_OBJECT_SIZE, &input_ground_model_size, sizeof(input_ground_model_size)));

        tivxCheckStatus(&status, vxQueryArray(output_obstacle_pos, VX_ARRAY_ITEMTYPE, &output_obstacle_pos_item_type, sizeof(output_obstacle_pos_item_type)));
        tivxCheckStatus(&status, vxQueryArray(output_obstacle_pos, VX_ARRAY_ITEMSIZE, &output_obstacle_pos_item_size, sizeof(output_obstacle_pos_item_size)));

        tivxCheckStatus(&status, vxQueryScalar(output_num_obstacles, VX_SCALAR_TYPE, &output_num_obstacles_scalar_type, sizeof(output_num_obstacles_scalar_type)));

        tivxCheckStatus(&status, vxQueryArray(output_freespace_boundary, (vx_enum)VX_ARRAY_ITEMTYPE, &output_freespace_boundary_item_type, sizeof(output_freespace_boundary_item_type)));
        tivxCheckStatus(&status, vxQueryArray(output_freespace_boundary, (vx_enum)VX_ARRAY_ITEMSIZE, &output_freespace_boundary_item_size, sizeof(output_freespace_boundary_item_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(output_drivable_space, (vx_enum)VX_USER_DATA_OBJECT_NAME, &output_drivable_space_name, sizeof(output_drivable_space_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(output_drivable_space, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &output_drivable_space_size, sizeof(output_drivable_space_size)));
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ((configuration_size != sizeof(tivx_obstacle_detection_params_t)) ||
            (strncmp(configuration_name, "tivx_obstacle_detection_params_t", sizeof(configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_obstacle_detection_params_t \n");
        }

        if (VX_DF_IMAGE_U8 != input_image_fmt && VX_DF_IMAGE_NV12 != input_image_fmt && VX_DF_IMAGE_UYVY != input_image_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_image' should be an image of type:\n VX_DF_IMAGE_U8, VX_DF_IMAGE_NV12 or VX_DF_IMAGE_UYVY \n");
        }

        if (VX_DF_IMAGE_S16 != input_disparity_fmt)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_disparity' should be an image of type:\n VX_DF_IMAGE_S16 \n");
        }

        if ((input_ground_model_size != sizeof(tivx_ground_model_params_t)) ||
            (strncmp(input_ground_model_name, "tivx_ground_model_params_t", sizeof(input_ground_model_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'input_ground_model' should be a user_data_object of type:\n tivx_ground_model_params_t \n");
        }

        if ( output_obstacle_pos_item_size != sizeof(tivx_obstacle_pos_t))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_obstacle_pos' should be an array of type:\n tivx_obstacle_pos_t \n");
        }

        if (output_num_obstacles_scalar_type != VX_TYPE_UINT32)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_num_obstacles' should be a scalar of type:\n VX_SCALAR_TYPE \n");
        }

        if ( output_freespace_boundary_item_size != sizeof(int32_t))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_freespace_boundary' should be an array of type:\n int32_t \n");
        }

        if ((output_drivable_space_size != sizeof(tivx_drivable_space_t)) ||
            (strncmp(output_drivable_space_name, "tivx_drivable_space_t", sizeof(output_drivable_space_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'output_drivable_space' should be a user_data_object of type:\n tivx_drivable_space_t \n");
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

static vx_status VX_CALLBACK tivxAddKernelObstacleDetectionInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;
    tivxKernelValidRectParams prms;

    if ( (num_params != TIVX_KERNEL_OBSTACLE_DETECTION_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_IMAGE_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_DISPARITY_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_GROUND_MODEL_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_OBSTACLE_POS_IDX])
        || (NULL == parameters[TIVX_KERNEL_OBSTACLE_DETECTION_OUTPUT_NUM_OBSTACLES_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    if (VX_SUCCESS == status)
    {
        tivxKernelValidRectParams_init(&prms);

        prms.in_img[0U] = (vx_image)parameters[TIVX_KERNEL_OBSTACLE_DETECTION_INPUT_IMAGE_IDX];

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

vx_status tivxAddKernelObstacleDetection(vx_context context)
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
                    TIVX_KERNEL_OBSTACLE_DETECTION_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_OBSTACLE_DETECTION_MAX_PARAMS,
                    tivxAddKernelObstacleDetectionValidate,
                    tivxAddKernelObstacleDetectionInitialize,
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
                        VX_OUTPUT,
                        VX_TYPE_ARRAY,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        VX_OUTPUT,
                        VX_TYPE_SCALAR,
                        VX_PARAMETER_STATE_REQUIRED
            );
            index++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        index,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_ARRAY,
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
    vx_obstacle_detection_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelObstacleDetection(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_obstacle_detection_kernel;

    status = vxRemoveKernel(kernel);
    vx_obstacle_detection_kernel = NULL;

    return status;
}


