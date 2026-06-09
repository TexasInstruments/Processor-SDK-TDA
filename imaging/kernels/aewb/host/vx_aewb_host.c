/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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
#include "TI/hwa_vpac_viss.h"
#include "TI/j7_imaging_aewb.h"
#include "tivx_imaging_kernels_priv.h"
#include "tivx_kernel_aewb.h"
#include "TI/tivx_target_kernel.h"

static vx_kernel vx_aewb_kernel = NULL;

static vx_status VX_CALLBACK tivxAddKernelAewbValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxAddKernelAewbInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params);
static vx_status VX_CALLBACK tivxAddKernelAewbValidate(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num,
            vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_user_data_object configuration = NULL;
    vx_char configuration_name[VX_MAX_REFERENCE_NAME];
    vx_size configuration_size;

    vx_user_data_object h3a_aew_af = NULL;
    vx_char h3a_aew_af_name[VX_MAX_REFERENCE_NAME];
    vx_size h3a_aew_af_size;
    vx_user_data_object ae_awb_result = NULL;
    vx_char ae_awb_result_name[VX_MAX_REFERENCE_NAME];
    vx_size ae_awb_result_size;

    vx_user_data_object dcc_param = NULL;
    vx_char dcc_param_name[VX_MAX_REFERENCE_NAME];
    vx_size dcc_param_size;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested. 
    <justification end> */
    if ( (num != TIVX_KERNEL_AEWB_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_AEWB_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_AEWB_H3A_AEW_AF_IDX])
        || (NULL == parameters[TIVX_KERNEL_AEWB_AE_AWB_RESULT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        configuration = (vx_user_data_object)parameters[TIVX_KERNEL_AEWB_CONFIGURATION_IDX];
        h3a_aew_af = (vx_user_data_object)parameters[TIVX_KERNEL_AEWB_H3A_AEW_AF_IDX];
        ae_awb_result = (vx_user_data_object)parameters[TIVX_KERNEL_AEWB_AE_AWB_RESULT_IDX];
        dcc_param = (vx_user_data_object)parameters[TIVX_KERNEL_AEWB_AE_AWB_DCC_IDX];
    }

    /* PARAMETER ATTRIBUTE FETCH */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_NAME, &configuration_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(configuration, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &configuration_size, sizeof(configuration_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(h3a_aew_af, (vx_enum)VX_USER_DATA_OBJECT_NAME, &h3a_aew_af_name, sizeof(configuration_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(h3a_aew_af, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &h3a_aew_af_size, sizeof(configuration_size)));

        tivxCheckStatus(&status, vxQueryUserDataObject(ae_awb_result, (vx_enum)VX_USER_DATA_OBJECT_NAME, &ae_awb_result_name, sizeof(ae_awb_result_name)));
        tivxCheckStatus(&status, vxQueryUserDataObject(ae_awb_result, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &ae_awb_result_size, sizeof(ae_awb_result_size)));

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != dcc_param)
        /* LDRA_JUSTIFY_END */
        {
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_param, (vx_enum)VX_USER_DATA_OBJECT_NAME, &dcc_param_name, sizeof(dcc_param_name)));
            tivxCheckStatus(&status, vxQueryUserDataObject(dcc_param, (vx_enum)VX_USER_DATA_OBJECT_SIZE, &dcc_param_size, sizeof(dcc_param_size)));
        }
    }

    /* PARAMETER CHECKING */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        if ((configuration_size != sizeof(tivx_aewb_config_t)) ||
            (strncmp(configuration_name, "tivx_aewb_config_t", sizeof(configuration_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'configuration' should be a user_data_object of type:\n tivx_aewb_config_t \n");
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((h3a_aew_af_size != sizeof(tivx_h3a_data_t)) ||
            (strncmp(h3a_aew_af_name, "tivx_h3a_data_t", sizeof(h3a_aew_af_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'h3a_aew_af' should be a user_data_object of type:\n tivx_h3a_data_t \n");
        }
        if ((ae_awb_result_size != sizeof(tivx_ae_awb_params_t)) ||
            (strncmp(ae_awb_result_name, "tivx_ae_awb_params_t", sizeof(ae_awb_result_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'ae_awb_result' should be a user_data_object of type:\n tivx_ae_awb_params_t \n");
        }

        if ((ae_awb_result_size != sizeof(tivx_ae_awb_params_t)) ||
            (strncmp(ae_awb_result_name, "tivx_ae_awb_params_t", sizeof(ae_awb_result_name)) != 0))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'ae_awb_result' should be a user_data_object of type:\n tivx_ae_awb_params_t \n");
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != dcc_param)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((dcc_param_size < 1U) ||
                (strncmp(dcc_param_name, "dcc_2a", sizeof(dcc_param_name)) != 0))
            /* LDRA_JUSTIFY_END */
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'dcc_param' should be a user_data_object of name:\n dcc_2a \n");
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

static vx_status VX_CALLBACK tivxAddKernelAewbInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_AEWB_MAX_PARAMS)
        || (NULL == parameters[TIVX_KERNEL_AEWB_CONFIGURATION_IDX])
        || (NULL == parameters[TIVX_KERNEL_AEWB_H3A_AEW_AF_IDX])
        || (NULL == parameters[TIVX_KERNEL_AEWB_AE_AWB_RESULT_IDX])
    )
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR, "One or more REQUIRED parameters are set to NULL\n");
    }
    /* LDRA_JUSTIFY_END */
    return status;
}

vx_status tivxAddKernelAewb(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t param_idx;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(status != (vx_status)VX_SUCCESS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Unable to allocate user kernel ID\n");
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
   if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_AEWB_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_AEWB_MAX_PARAMS,
                    tivxAddKernelAewbValidate,
                    tivxAddKernelAewbInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        param_idx = 0U;

        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_DISTRIBUTION,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if ((vx_status)VX_SUCCESS == status)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if ((vx_status)VX_SUCCESS == status)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if((vx_status)VX_SUCCESS == status)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if((vx_status)VX_SUCCESS == status)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_USER_DATA_OBJECT,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
        }
        if((vx_status)VX_SUCCESS == status)
        {
            /* add supported target's */
            #if !defined(SOC_AM62A)
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_0));
            #if !defined(SOC_J722S)
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_MCU2_1));
            #endif
            #else
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_MCU1_0));
            #endif
        }
        if((vx_status)VX_SUCCESS == status)
        {
            status = vxFinalizeKernel(kernel);
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status != (vx_status)VX_SUCCESS)
        {
            (void)vxReleaseKernel(&kernel);
            kernel = NULL;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        kernel = NULL;
    }
    /* LDRA_JUSTIFY_END */
    vx_aewb_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelAewb(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_aewb_kernel;

    status = vxRemoveKernel(kernel);
    vx_aewb_kernel = NULL;

    return status;
}