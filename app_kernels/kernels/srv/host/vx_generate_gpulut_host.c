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
#include "tivx_kernel_generate_gpulut.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_generate_gpulut_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelGenerateGpulutValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelGenerateGpulutInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
vx_status tivxAddKernelGenerateGpulut(vx_context context);
vx_status tivxRemoveKernelGenerateGpulut(vx_context context);

static vx_status VX_CALLBACK tivxAddKernelGenerateGpulutValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = VX_SUCCESS;

    vx_user_data_object in_configuration = NULL;
    vx_char in_configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size in_configuration_size;

    vx_user_data_object in_ldclut = NULL;
    vx_char in_ldclut_name[VX_MAX_REFERENCE_NAME];
    vx_size in_ldclut_size;

    vx_user_data_object in_calmat_scaled = NULL;
    vx_char in_calmat_scaled_name[VX_MAX_REFERENCE_NAME];
    vx_size in_calmat_scaled_size;

    vx_array in_lut3dxyz = NULL;
    vx_enum in_lut3dxyz_item_type;
    vx_size in_lut3dxyz_item_size;

    vx_array out_gpulut3d = NULL;
    vx_enum out_gpulut3d_item_type;
    vx_size out_gpulut3d_item_size;

    if ( (num != TIVX_KERNEL_GENERATE_GPULUT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_LDCLUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_CALMAT_SCALED_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_LUT3DXYZ_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_OUT_GPULUT3D_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }

    if (VX_SUCCESS == status)
    {
        in_configuration = (vx_user_data_object)parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX];
        in_ldclut = (vx_user_data_object)parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_LDCLUT_IDX];
        in_calmat_scaled = (vx_user_data_object)parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_CALMAT_SCALED_IDX];
        in_lut3dxyz = (vx_array)parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_LUT3DXYZ_IDX];
        out_gpulut3d = (vx_array)parameters[TIVX_KERNEL_GENERATE_GPULUT_OUT_GPULUT3D_IDX];
    }


    /* PARAMETER ATTRIBUTE FETCH */

    if (VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(in_configuration, VX_USER_DATA_OBJECT_NAME, &in_configuration_name, sizeof(in_configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_configuration, VX_USER_DATA_OBJECT_SIZE, &in_configuration_size, sizeof(in_configuration_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(in_ldclut, VX_USER_DATA_OBJECT_NAME, &in_ldclut_name, sizeof(in_ldclut_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_ldclut, VX_USER_DATA_OBJECT_SIZE, &in_ldclut_size, sizeof(in_ldclut_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(in_calmat_scaled, VX_USER_DATA_OBJECT_NAME, &in_calmat_scaled_name, sizeof(in_calmat_scaled_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(in_calmat_scaled, VX_USER_DATA_OBJECT_SIZE, &in_calmat_scaled_size, sizeof(in_calmat_scaled_size)));

        tivxCheckStatus(&status, vxQueryArray(in_lut3dxyz, VX_ARRAY_ITEMTYPE, &in_lut3dxyz_item_type, sizeof(in_lut3dxyz_item_type)));
        tivxCheckStatus(&status, vxQueryArray(in_lut3dxyz, VX_ARRAY_ITEMSIZE, &in_lut3dxyz_item_size, sizeof(in_lut3dxyz_item_size)));

        tivxCheckStatus(&status, vxQueryArray(out_gpulut3d, VX_ARRAY_ITEMTYPE, &out_gpulut3d_item_type, sizeof(out_gpulut3d_item_type)));
        tivxCheckStatus(&status, vxQueryArray(out_gpulut3d, VX_ARRAY_ITEMSIZE, &out_gpulut3d_item_size, sizeof(out_gpulut3d_item_size)));
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

        if ((in_ldclut_size != sizeof(svLdcLut_t)) ||
            (strncmp(in_ldclut_name, "svLdcLut_t", sizeof(in_ldclut_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_ldclut' should be a user_data_object of type:\n svLdcLut_t \n");
        }

        if ((in_calmat_scaled_size != sizeof(svACCalmatStruct_t)) ||
            (strncmp(in_calmat_scaled_name, "svACCalmatStruct_t", sizeof(in_calmat_scaled_name)) != 0))
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_calmat_scaled' should be a user_data_object of type:\n svACCalmatStruct_t \n");
        }

        if ( in_lut3dxyz_item_type != VX_TYPE_FLOAT32)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'in_lut3dxyz' should be an array of type:\n VX_TYPE_FLOAT32 \n");
        }

        if ( out_gpulut3d_item_type != VX_TYPE_UINT16)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'out_gpulut3d' should be an array of type:\n VX_TYPE_UINT16 \n");
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

static vx_status VX_CALLBACK tivxAddKernelGenerateGpulutInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = VX_SUCCESS;

    if ( (num_params != TIVX_KERNEL_GENERATE_GPULUT_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_LDCLUT_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_CALMAT_SCALED_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_IN_LUT3DXYZ_IDX])
        || (NULL == parameters[TIVX_KERNEL_GENERATE_GPULUT_OUT_GPULUT3D_IDX])
    )
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    return status;
}

vx_status tivxAddKernelGenerateGpulut(vx_context context)
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
                    TIVX_KERNEL_GENERATE_GPULUT_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_GENERATE_GPULUT_MAX_PARAMS,
                    tivxAddKernelGenerateGpulutValidate,
                    tivxAddKernelGenerateGpulutInitialize,
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
                        VX_INPUT,
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
                        VX_TYPE_ARRAY,
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
    vx_generate_gpulut_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelGenerateGpulut(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_generate_gpulut_kernel;

    status = vxRemoveKernel(kernel);
    vx_generate_gpulut_kernel = NULL;

    return status;
}


