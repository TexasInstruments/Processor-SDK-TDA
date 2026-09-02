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
#include "VX/vx.h"
#include "tivx_srv_kernels_priv.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_kernel_generate_3dbowl.h"

#include <math.h>
#include <float.h>



static vx_kernel vx_generate_3dbowl_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelGenerate3DbowlValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelGenerate3DbowlInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelGenerate3Dbowl(vx_context context);
vx_status tivxRemoveKernelGenerate3Dbowl(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelGenerate3DbowlValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object in_configuration = NULL;
    vx_char in_configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size in_configuration_size;

    vx_user_data_object in_calmat = NULL;
    vx_char in_calmat_name[VX_MAX_REFERENCE_NAME];
    vx_size in_calmat_size;

    vx_user_data_object in_offset = NULL;
    vx_char in_offset_name[VX_MAX_REFERENCE_NAME];
    vx_size in_offset_size;

    vx_array out_lut3dxyz = NULL;
    vx_enum out_lut3dxyz_item_type;
    vx_size out_lut3dxyz_item_size;

    vx_user_data_object out_calmat_scaled = NULL;
    vx_char out_calmat_scaled_name[VX_MAX_REFERENCE_NAME];
    vx_size out_calmat_scaled_size;

    if ( (num != TIVX_KERNEL_GENERATE_3DBOWL_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_CALMAT_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_OFFSET_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_OUT_LUT3DXYZ_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_OUT_CALMAT_SCALED_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        in_configuration = (vx_user_data_object)parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_CONFIGURATION_IDX];
        in_calmat = (vx_user_data_object)parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_CALMAT_IDX];
        in_offset = (vx_user_data_object)parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_OFFSET_IDX];
        out_lut3dxyz = (vx_array)parameters[TIVX_KERNEL_GENERATE_3DBOWL_OUT_LUT3DXYZ_IDX];
        out_calmat_scaled = (vx_user_data_object)parameters[TIVX_KERNEL_GENERATE_3DBOWL_OUT_CALMAT_SCALED_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(in_configuration, VX_USER_DATA_OBJECT_NAME, &in_configuration_name, sizeof(in_configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_configuration, VX_USER_DATA_OBJECT_SIZE, &in_configuration_size, sizeof(in_configuration_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(in_calmat, VX_USER_DATA_OBJECT_NAME, &in_calmat_name, sizeof(in_calmat_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_calmat, VX_USER_DATA_OBJECT_SIZE, &in_calmat_size, sizeof(in_calmat_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(in_offset, VX_USER_DATA_OBJECT_NAME, &in_offset_name, sizeof(in_offset_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_offset, VX_USER_DATA_OBJECT_SIZE, &in_offset_size, sizeof(in_offset_size)));

        tivxCheckStatus(&status, vxQueryArray(out_lut3dxyz, VX_ARRAY_ITEMTYPE, &out_lut3dxyz_item_type, sizeof(out_lut3dxyz_item_type)));
        tivxCheckStatus(&status, vxQueryArray(out_lut3dxyz, VX_ARRAY_ITEMSIZE, &out_lut3dxyz_item_size, sizeof(out_lut3dxyz_item_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(out_calmat_scaled, VX_USER_DATA_OBJECT_NAME, &out_calmat_scaled_name, sizeof(out_calmat_scaled_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(out_calmat_scaled, VX_USER_DATA_OBJECT_SIZE, &out_calmat_scaled_size, sizeof(out_calmat_scaled_size)));
    }

    /* PARAMETER CHECKING */

    if (VX_SUCCESS == status)
    {
        if ((in_configuration_size != sizeof(svGpuLutGen_t)) ||
            (strncmp(in_configuration_name, "svGpuLutGen_t", sizeof(in_configuration_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_configuration' should be a user_data_object of type:\n svGpuLutGen_t \n");
        }

        if ((in_calmat_size != sizeof(svACCalmatStruct_t)) ||
            (strncmp(in_calmat_name, "svACCalmatStruct_t", sizeof(in_calmat_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_calmat' should be a user_data_object of type:\n svACCalmatStruct_t \n");
        }

        if ((in_offset_size != sizeof(svGeometric_t)) ||
            (strncmp(in_offset_name, "svGeometric_t", sizeof(in_offset_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_offset' should be a user_data_object of type:\n svGeometric_t \n");
        }

        if ( out_lut3dxyz_item_type != VX_TYPE_FLOAT32)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out_lut3dxyz' should be an array of type:\n VX_TYPE_FLOAT32 \n");
        }

        if ((out_calmat_scaled_size != sizeof(svACCalmatStruct_t)) ||
            (strncmp(out_calmat_scaled_name, "svACCalmatStruct_t", sizeof(out_calmat_scaled_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out_calmat_scaled' should be a user_data_object of type:\n svACCalmatStruct_t \n");
        }
    }


    /* CUSTOM PARAMETER CHECKING */

    /* < DEVELOPER_TODO: (Optional) Add any custom parameter type or range checking not */
    /*                   covered by the code-generation script.) > */

    /* < DEVELOPER_TODO: (Optional) If intending to use a virtual data object, set metas using appropriate TI API. */
    /*                   For a code example, please refer to the validate callback of the follow file: */
    /*                   tiovx/kernels/openvx-core/host/vx_absdiff_host.c. For further information regarding metas, */
    /*                   please refer to the OpenVX 1.1 spec p. 260, or search for vx_kernel_validate_f. > */

    if (VX_SUCCESS == status)
    {
        svGpuLutGen_t         in_params;
        vxCopyUserDataObject(in_configuration, 0, sizeof(svGpuLutGen_t), &in_params, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
        if (SKIP != in_params.subsampleratio)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameters 'subsampleratio' should be the same as the #define SKIP\n");

        }
    }
    
    return status;
}

static vx_status VX_CALLBACK tivxAddKernelGenerate3DbowlInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_GENERATE_3DBOWL_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_CALMAT_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_IN_OFFSET_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_OUT_LUT3DXYZ_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_3DBOWL_OUT_CALMAT_SCALED_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelGenerate3Dbowl(vx_context context)
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
                    TIVX_KERNEL_GENERATE_3DBOWL_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_GENERATE_3DBOWL_MAX_PARAMS,
                    tivxAddKernelGenerate3DbowlValidate,
                    tivxAddKernelGenerate3DbowlInitialize,
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
                        VX_TYPE_USER_DATA_OBJECT,
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
    vx_generate_3dbowl_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelGenerate3Dbowl(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_generate_3dbowl_kernel;

    status = vxRemoveKernel(kernel);
    vx_generate_3dbowl_kernel = NULL;

    return status;
}


