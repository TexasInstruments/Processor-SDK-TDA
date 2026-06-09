/*
 *
 * Copyright (c) 2017-2018 Texas Instruments Incorporated
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifdef BUILD_VPAC_MSC

#include "TI/tivx.h"
#include "TI/hwa_vpac_msc.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_msc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_hwa_host_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacMscScaleValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params, vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxVpacMscScaleInitialize(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params);
static vx_status VX_CALLBACK tivxVpacMscPyramidValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
static vx_status VX_CALLBACK tivxVpacMscPyramidInitialize(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static vx_kernel vx_vpac_msc_scale_kernel = NULL;
static vx_kernel vx_vpac_msc_pyramid_kernel = NULL;
static vx_kernel vx_vpac_msc_scale2_kernel = NULL;

static uint32_t gmsc_32_phase_gaussian_filter[] =
{
    #include "msc_32_phase_gaussian_filter.txt"
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

vx_status tivxAddKernelVpacMscMultiScale(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t param_idx;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS,
                    tivxVpacMscScaleValidate,
                    tivxVpacMscScaleInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        param_idx = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC1));
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC2));
            #if (VPAC_COUNT > 1)
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC1));
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC2));
            #endif
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        kernel = NULL;
    }
    /* LDRA_JUSTIFY_END */
    vx_vpac_msc_scale_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacMscMultiScale(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_msc_scale_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_msc_scale_kernel = NULL;

    return status;
}


vx_status tivxAddKernelVpacMscPyramid(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t param_idx;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS,
                    tivxVpacMscPyramidValidate,
                    tivxVpacMscPyramidInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        param_idx = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_PYRAMID,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC1));
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC2));
            #if (VPAC_COUNT > 1)
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC1));
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC2));
            #endif
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        kernel = NULL;
    }
    /* LDRA_JUSTIFY_END */
    vx_vpac_msc_pyramid_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacMscPyramid(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_msc_pyramid_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_msc_pyramid_kernel = NULL;

    return status;
}

vx_status tivxAddKernelVpacMscMultiOut(vx_context context)
{
    vx_kernel kernel;
    vx_status status;
    uint32_t param_idx;
    vx_enum kernel_id;

    status = vxAllocateUserKernelId(context, &kernel_id);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        kernel = vxAddUserKernel(
                    context,
                    TIVX_KERNEL_VPAC_MSC_MULTI_SCALE2_NAME,
                    kernel_id,
                    NULL,
                    TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS,
                    tivxVpacMscScaleValidate,
                    tivxVpacMscScaleInitialize,
                    NULL);

        status = vxGetStatus((vx_reference)kernel);
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        param_idx = 0;

        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_REQUIRED
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_INPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
            param_idx++;
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxAddParameterToKernel(kernel,
                        param_idx,
                        (vx_enum)VX_OUTPUT,
                        (vx_enum)VX_TYPE_IMAGE,
                        (vx_enum)VX_PARAMETER_STATE_OPTIONAL
            );
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            /* add supported target's */
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC1));
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC_MSC2));
            #if (VPAC_COUNT > 1)
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC1));
            tivxCheckStatus(&status, tivxAddKernelTarget(kernel, TIVX_TARGET_VPAC2_MSC2));
            #endif
        }
        if (status == (vx_status)VX_SUCCESS)
        {
            status = vxFinalizeKernel(kernel);
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        kernel = NULL;
    }
    /* LDRA_JUSTIFY_END */
    vx_vpac_msc_scale2_kernel = kernel;

    return status;
}

vx_status tivxRemoveKernelVpacMscMultiOut(vx_context context)
{
    vx_status status;
    vx_kernel kernel = vx_vpac_msc_scale2_kernel;

    status = vxRemoveKernel(kernel);
    vx_vpac_msc_scale2_kernel = NULL;

    return status;
}

void tivx_vpac_msc_coefficients_params_init(
    tivx_vpac_msc_coefficients_t *coeff,
    vx_enum interpolation)
{
    uint32_t i;
    uint32_t idx;
    uint32_t weight;

    /* Passthrough if using single phase */
    idx = 0;
    coeff->single_phase[0][idx] = 0;
    idx++;
    coeff->single_phase[0][idx] = 0;
    idx++;
    coeff->single_phase[0][idx] = (int32_t)256;
    idx++;
    coeff->single_phase[0][idx] = 0;
    idx++;
    coeff->single_phase[0][idx] = 0;
    idx = 0;
    coeff->single_phase[1][idx] = 0;
    idx++;
    coeff->single_phase[1][idx] = 0;
    idx++;
    coeff->single_phase[1][idx] = (int32_t)256;
    idx++;
    coeff->single_phase[1][idx] = 0;
    idx++;
    coeff->single_phase[1][idx] = 0;

    if((vx_enum)VX_INTERPOLATION_BILINEAR == interpolation)
    {
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = i<<2U;
            coeff->multi_phase[0][idx] = 0;
            idx++;
            coeff->multi_phase[0][idx] = 0;
            idx++;
            coeff->multi_phase[0][idx] = (int32_t)256-(int32_t)weight;
            idx++;
            coeff->multi_phase[0][idx] = (int32_t)weight;
            idx++;
            coeff->multi_phase[0][idx] = 0;
            idx++;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = (i+32u)<<2U;
            coeff->multi_phase[1][idx] = 0;
            idx++;
            coeff->multi_phase[1][idx] = 0;
            idx++;
            coeff->multi_phase[1][idx] = (int32_t)256-(int32_t)weight;
            idx++;
            coeff->multi_phase[1][idx] = (int32_t)weight;
            idx++;
            coeff->multi_phase[1][idx] = 0;
            idx++;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = i<<2U;
            coeff->multi_phase[2][idx] = 0;
            idx++;
            coeff->multi_phase[2][idx] = 0;
            idx++;
            coeff->multi_phase[2][idx] = (int32_t)256-(int32_t)weight;
            idx++;
            coeff->multi_phase[2][idx] = (int32_t)weight;
            idx++;
            coeff->multi_phase[2][idx] = 0;
            idx++;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            weight = (i+32u)<<2U;
            coeff->multi_phase[3][idx] = 0;
            idx++;
            coeff->multi_phase[3][idx] = 0;
            idx++;
            coeff->multi_phase[3][idx] = (int32_t)256-(int32_t)weight;
            idx++;
            coeff->multi_phase[3][idx] = (int32_t)weight;
            idx++;
            coeff->multi_phase[3][idx] = 0;
            idx++;
        }
    }
    else if((vx_enum)VX_INTERPOLATION_NEAREST_NEIGHBOR == interpolation)
    {
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[0][idx] = 0;
            idx++;
            coeff->multi_phase[0][idx] = 0;
            idx++;
            coeff->multi_phase[0][idx] = (int32_t)256;
            idx++;
            coeff->multi_phase[0][idx] = 0;
            idx++;
            coeff->multi_phase[0][idx] = 0;
            idx++;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[1][idx] = 0;
            idx++;
            coeff->multi_phase[1][idx] = 0;
            idx++;
            coeff->multi_phase[1][idx] = 0;
            idx++;
            coeff->multi_phase[1][idx] = (int32_t)256;
            idx++;
            coeff->multi_phase[1][idx] = 0;
            idx++;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[2][idx] = 0;
            idx++;
            coeff->multi_phase[2][idx] = 0;
            idx++;
            coeff->multi_phase[2][idx] = (int32_t)256;
            idx++;
            coeff->multi_phase[2][idx] = 0;
            idx++;
            coeff->multi_phase[2][idx] = 0;
            idx++;
        }
        idx = 0;
        for(i = 0; i < 32u; i++)
        {
            coeff->multi_phase[3][idx] = 0;
            idx++;
            coeff->multi_phase[3][idx] = 0;
            idx++;
            coeff->multi_phase[3][idx] = 0;
            idx++;
            coeff->multi_phase[3][idx] = (int32_t)256;
            idx++;
            coeff->multi_phase[3][idx] = 0;
            idx++;
        }
    }
    else if((vx_enum)TIVX_VPAC_MSC_INTERPOLATION_GAUSSIAN_32_PHASE == interpolation)
    {
        for(i = 0; i < 2u; i++)
        {
            idx = 0;
            coeff->single_phase[i][idx] = 16;
            idx++;
            coeff->single_phase[i][idx] = 64;
            idx++;
            coeff->single_phase[i][idx] = 96;
            idx++;
            coeff->single_phase[i][idx] = 64;
            idx++;
            coeff->single_phase[i][idx] = 16;
        }
        for(i = 0; i < (32u*5u); i++)
        {
            coeff->multi_phase[0][i] = (int32_t)gmsc_32_phase_gaussian_filter[i];
            coeff->multi_phase[1][i] = (int32_t)gmsc_32_phase_gaussian_filter[i];
            coeff->multi_phase[2][i] = (int32_t)gmsc_32_phase_gaussian_filter[i];
            coeff->multi_phase[3][i] = (int32_t)gmsc_32_phase_gaussian_filter[i];
        }
    }
    else
    {
        /* Do nothing */
    }
}

void tivx_vpac_msc_input_params_init(tivx_vpac_msc_input_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_msc_input_params_t));

        prms->kern_sz = 5u;
        prms->src_ln_inc_2 = 0u;
        prms->input_align_12bit = 0u;
        prms->yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
        prms->max_ds_factor = TIVX_VPAC_MSC_DEFAULT_MAX_DS_FACTOR;
        #if defined(VPAC3) || defined(VPAC3L)
        prms->is_enable_simul_processing = 0;
        prms->in_img0_yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
        prms->in_img1_yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
        #endif
        prms->enable_error_events = 0u;
        prms->enable_psa = 0u;
        prms->wdtimer_timeout_cycles = TIVX_VPAC_MSC_WDTIMER_64K_CYCLES;
    }
}

void tivx_vpac_msc_output_params_init(tivx_vpac_msc_output_params_t *prms)
{
    if (NULL != prms)
    {
        memset(prms, 0x0, sizeof(tivx_vpac_msc_output_params_t));

        prms->signed_data = 0u;
        prms->filter_mode = 1u;
        prms->coef_shift = 8u;
        prms->saturation_mode = 0u;
        prms->offset_x = TIVX_VPAC_MSC_AUTOCOMPUTE;
        prms->offset_y = TIVX_VPAC_MSC_AUTOCOMPUTE;
        prms->output_align_12bit = 0u;

        prms->single_phase.horz_coef_src = 0u;
        prms->single_phase.horz_coef_sel = 0u;
        prms->single_phase.vert_coef_src = 0u;
        prms->single_phase.vert_coef_sel = 0u;

        prms->multi_phase.phase_mode = 0u;
        prms->multi_phase.horz_coef_sel = 0u;
        prms->multi_phase.vert_coef_sel = 0u;
        prms->multi_phase.init_phase_x = TIVX_VPAC_MSC_AUTOCOMPUTE;
        prms->multi_phase.init_phase_y = TIVX_VPAC_MSC_AUTOCOMPUTE;
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacMscScaleValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params, vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;

    vx_uint32 cnt;

    vx_image in_img0 = NULL, in_img1 = NULL;
    vx_df_image in_img0_fmt, in_img1_fmt;
    vx_uint32 in_img0_w, in_img0_h, in_img1_w, in_img1_h;

    vx_image out_img[TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT] = {NULL};
    vx_df_image out_img_fmt[TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT] = {VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0'),
                                                                        VX_DF_IMAGE('0','0','0','0')};
    vx_uint32 out_img_w[TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT] = {0};
    vx_uint32 out_img_h[TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT] = {0};
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_MSC_SCALE_IN0_IMG_IDX]) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX])) &&
        ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_MSC_SCALE2_IN0_IMG_IDX]) ||
        (NULL == parameters[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT0_IMG_IDX])))
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        VX_PRINT(VX_ZONE_ERROR,
            "One or more REQUIRED parameters are set to NULL\n");
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        in_img0 = (vx_image)parameters[TIVX_KERNEL_VPAC_MSC_SCALE_IN0_IMG_IDX];
        if(TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS == num_params)
        {
            in_img0 = (vx_image)parameters[TIVX_KERNEL_VPAC_MSC_SCALE2_IN0_IMG_IDX];
            in_img1 = (vx_image)parameters[TIVX_KERNEL_VPAC_MSC_SCALE2_IN1_IMG_IDX];
        }
        if(NULL == in_img1)
        {
            for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
            {                
                out_img[cnt] = (vx_image)parameters
                    [TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];                
            }
        }
        else
        {
            for (cnt = 0u; cnt < (TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT/2U); cnt++)
            {
                out_img[(2U*cnt)] = (vx_image)parameters
                    [TIVX_KERNEL_VPAC_MSC_SCALE2_OUT0_IMG_IDX + cnt];
                out_img[(2U*cnt)+1U] = (vx_image)parameters
                    [TIVX_KERNEL_VPAC_MSC_SCALE2_OUT5_IMG_IDX + cnt];
            }
        }
    }

    /* PARAMETER ATTRIBUTE FETCH */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        tivxCheckStatus(&status, vxQueryImage(in_img0, (vx_enum)VX_IMAGE_FORMAT,
            &in_img0_fmt, sizeof(in_img0_fmt)));
        tivxCheckStatus(&status, vxQueryImage(in_img0, (vx_enum)VX_IMAGE_WIDTH,
            &in_img0_w, sizeof(in_img0_w)));
        tivxCheckStatus(&status, vxQueryImage(in_img0, (vx_enum)VX_IMAGE_HEIGHT,
            &in_img0_h, sizeof(in_img0_h)));
        if(NULL != in_img1)
        {
            tivxCheckStatus(&status, vxQueryImage(in_img1, (vx_enum)VX_IMAGE_FORMAT,
                &in_img1_fmt, sizeof(in_img1_fmt)));
            tivxCheckStatus(&status, vxQueryImage(in_img1, (vx_enum)VX_IMAGE_WIDTH,
                &in_img1_w, sizeof(in_img1_w)));
            tivxCheckStatus(&status, vxQueryImage(in_img1, (vx_enum)VX_IMAGE_HEIGHT,
                &in_img1_h, sizeof(in_img1_h)));
        }
    }

    /* PARAMETER ATTRIBUTE FETCH */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT; cnt ++)
        {
            if (NULL != out_img[cnt])
            {
                tivxCheckStatus(&status, vxQueryImage(out_img[cnt],
                    (vx_enum)VX_IMAGE_FORMAT, &out_img_fmt[cnt],
                    sizeof(out_img_fmt[cnt])));
                tivxCheckStatus(&status, vxQueryImage(out_img[cnt],
                    (vx_enum)VX_IMAGE_WIDTH, &out_img_w[cnt], sizeof(out_img_w[cnt])));
                tivxCheckStatus(&status, vxQueryImage(out_img[cnt],
                    (vx_enum)VX_IMAGE_HEIGHT, &out_img_h[cnt], sizeof(out_img_h[cnt])));
            }

            /* PARAMETER ATTRIBUTE FETCH */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != in_img0)
    /* LDRA_JUSTIFY_END */
    {
        if( ((vx_df_image)VX_DF_IMAGE_NV12 != in_img0_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U8 != in_img0_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != in_img0_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != in_img0_fmt)
            #if defined(VPAC3) || defined(VPAC3L)
            && 
            ((vx_df_image)VX_DF_IMAGE_UYVY != in_img0_fmt)
            #endif
            )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect in_img0 fmt \n");
        }
        if( ((vx_df_image)VX_DF_IMAGE_NV12 == in_img0_fmt) &&
            (NULL != in_img1))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "in_img1 should be null \n");
        }
    }
    if (NULL != in_img1)
    {
        if( ((vx_df_image)VX_DF_IMAGE_U8 != in_img1_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != in_img1_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != in_img1_fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect in_img1 fmt \n");
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if((NULL != in_img0) && (NULL == in_img1))
        /* LDRA_JUSTIFY_END */
        {
            vx_bool loop_exit = (vx_bool)vx_false_e;
            for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
            {
                if((status != (vx_status)VX_SUCCESS) || ((vx_bool)vx_true_e == loop_exit))
                {
                    break;
                }
                if (NULL == out_img[cnt])
                {
                    /* all images must be continuous */
                    loop_exit = (vx_bool)vx_true_e;
                }
                else
                {
                    if (((vx_df_image)VX_DF_IMAGE_NV12 != out_img_fmt[cnt]) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 != out_img_fmt[cnt]) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 != out_img_fmt[cnt]) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 != out_img_fmt[cnt])
                    #if defined(VPAC3) || defined(VPAC3L)
                    &&
                    ((vx_df_image)VX_DF_IMAGE_UYVY != out_img_fmt[cnt]) &&
                    ((vx_df_image)VX_DF_IMAGE_YUYV != out_img_fmt[cnt])
                    #endif
                    )
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' should be an image of type:\n VX_DF_IMAGE_NV12 or VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12 or VX_DF_IMAGE_UYVY or VX_DF_IMAGE_YUYV\n", cnt);
                        loop_exit = (vx_bool)vx_true_e;
                    }
                    #if defined(VPAC3) || defined(VPAC3L)
                    if (((vx_bool)vx_false_e == loop_exit) && (((vx_df_image)VX_DF_IMAGE_UYVY == out_img_fmt[cnt]) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == out_img_fmt[cnt])) &&
                        ((vx_df_image)VX_DF_IMAGE_UYVY != in_img0_fmt))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out0_img' is YUV422I but 'in_img' is not YUV422I \n");
                        loop_exit = (vx_bool)vx_true_e;
                    }
                    #endif

                    if (((vx_bool)vx_false_e == loop_exit) && ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_fmt[cnt]) &&
                        ((vx_df_image)VX_DF_IMAGE_NV12 != in_img0_fmt))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' should be an image of type:\n VX_DF_IMAGE_NV12 when in_img is VX_DF_IMAGE_NV12\n", cnt);
                        loop_exit = (vx_bool)vx_true_e;
                    }

                    if (((vx_bool)vx_false_e == loop_exit) && (out_img_w[cnt] > in_img0_w))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' width should not be larger than the input0 width\n", cnt);
                        loop_exit = (vx_bool)vx_true_e;
                    }

                    if (((vx_bool)vx_false_e == loop_exit) && (out_img_h[cnt] > in_img0_h))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' height should not be larger than the input0 height\n", cnt);
                        loop_exit = (vx_bool)vx_true_e;
                    }
                    #if defined(VPAC3) || defined(VPAC3L)
                    if (((vx_bool)vx_false_e == loop_exit) && (cnt > 3u))
                    {
                        if ((((vx_df_image)VX_DF_IMAGE_UYVY == out_img_fmt[cnt]) ||
                            ((vx_df_image)VX_DF_IMAGE_YUYV == out_img_fmt[cnt])) &&
                            ((vx_df_image)VX_DF_IMAGE_UYVY == in_img0_fmt))
                        {
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                            VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' format shouldn't be YUYV or UYVY\n", cnt);
                            VX_PRINT(VX_ZONE_ERROR, "At most four YUYV or UYVY outputs can generate with 10 scalars\n");
                            loop_exit = (vx_bool)vx_true_e;
                        }
                    }
                    #endif
                    if (((vx_bool)vx_false_e == loop_exit) && (cnt > 0u))
                    {
                        if (out_img_fmt[cnt] != out_img_fmt[cnt-1u])
                        {
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                            VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' format should be same as 'out_img[%d]' format\n", cnt, cnt-1u);
                            VX_PRINT(VX_ZONE_ERROR, "Format must be same for all outputs\n");
                            loop_exit = (vx_bool)vx_true_e;
                        }
                    }

                }
            }
        }
    }
    if ((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if((NULL != in_img0) && (NULL != in_img1))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((in_img1_w != in_img0_w))
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "'in_img1' width should be same as the in_img0 width\n");
            }
            /* LDRA_JUSTIFY_END */
            vx_bool loop_exit3 = (vx_bool)vx_false_e;
            for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT; cnt ++)
            {
                if((status != (vx_status)VX_SUCCESS) || ((vx_bool)vx_true_e == loop_exit3))
                {
                    break;
                }
                if (((vx_df_image)VX_DF_IMAGE_U8 != out_img_fmt[cnt]) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 != out_img_fmt[cnt]) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 != out_img_fmt[cnt]))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' should be an image of type: VX_DF_IMAGE_U8 or VX_DF_IMAGE_U16 or TIVX_DF_IMAGE_P12\n", cnt);
                    loop_exit3 = (vx_bool)vx_true_e;
                }
                if (((vx_bool)vx_false_e == loop_exit3) && ((out_img_w[cnt] > in_img0_w) || (out_img_w[cnt] > in_img1_w)))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' width should not be larger than the input0 or input1 width\n", cnt);
                    loop_exit3 = (vx_bool)vx_true_e;
                }
            }
            vx_bool loop_exit2 = (vx_bool)vx_false_e;
            for (cnt = 0u; cnt < (TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT/2U); cnt++)
            {
                if((status != (vx_status)VX_SUCCESS) || ((vx_bool)vx_true_e == loop_exit2))
                {
                    break;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. An error print statement can be added in a future release if required.
                <justification end> */
                if((NULL != out_img[(2U*cnt)]) && (NULL == out_img[(2U*cnt)+1U]))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' shouldn't be NULL, when out_img[%d] is not NULL\n", cnt+5U, cnt);
                    loop_exit2 = (vx_bool)vx_true_e;
                }
                /* LDRA_JUSTIFY_END */

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. An error print statement can be added in a future release if required.
                <justification end> */
                if(((vx_bool)vx_false_e == loop_exit2) && (NULL == out_img[(2U*cnt)]) && (NULL != out_img[(2U*cnt)+1U]))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' shouldn't be NULL, when out_img[%d] is not NULL\n", cnt+5U, cnt);
                    loop_exit2 = (vx_bool)vx_true_e;
                }
                /* LDRA_JUSTIFY_END */

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. An error print statement can be added in a future release if required.
                <justification end> */
                if(((vx_bool)vx_false_e == loop_exit2) && (NULL != out_img[(2U*cnt)]) && (NULL != out_img[(2U*cnt)+1U]))
                /* LDRA_JUSTIFY_END */
                {
                    if ((out_img_h[(2U*cnt)] > in_img0_h))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' height should not be larger than the input0 height\n", cnt);
                        loop_exit2 = (vx_bool)vx_true_e;
                    }
                    if (((vx_bool)vx_false_e == loop_exit2) && (out_img_h[(2U*cnt)+1U] > in_img1_h))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' height should not be larger than the input1 height\n", cnt+5U);
                        loop_exit2 = (vx_bool)vx_true_e;
                    }
                    if(((vx_bool)vx_false_e == loop_exit2) && (out_img_fmt[(2U*cnt)] != in_img0_fmt))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' format should be same as in_img0_fmt\n", cnt);
                        loop_exit2 = (vx_bool)vx_true_e;
                    }
                    if(((vx_bool)vx_false_e == loop_exit2) && (out_img_fmt[(2U*cnt)+1U] != in_img1_fmt))
                    {
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        VX_PRINT(VX_ZONE_ERROR, "'out_img[%d]' format should be same as in_img1_fmt\n", cnt+5U);
                        loop_exit2 = (vx_bool)vx_true_e;
                    }
                }
            }
            
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscScaleInitialize(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num_params)
{
    return (vx_status)VX_SUCCESS;
}

static vx_status VX_CALLBACK tivxVpacMscPyramidValidate(vx_node node,
    const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_image img;
    vx_pyramid pmd;
    vx_uint32 w, h, i;
    vx_uint32 p_w, p_h;
    vx_size num_levels;
    vx_df_image fmt, p_fmt;
    vx_float32 scale;
    vx_border_t border;

    /* Check only half scale pyramid is supported */
    /* Check NV12, U8, U16 and P12 data format support for input and output */
    /* Check to see all pyramid levels have same format */
    /* Atleast for the first level, scaling factor cannot be less than 1/4x */

    for (i = 0U; i < TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS; i ++)
    {
        /* Check for NULL */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        
        if (NULL == parameters[i])
        {
            status = (vx_status)VX_ERROR_NO_MEMORY;
            break;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        img = (vx_image)parameters[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
        pmd = (vx_pyramid)parameters[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

        /* Get the image width/heigh and format */
        tivxCheckStatus(&status, vxQueryImage(img, (vx_enum)VX_IMAGE_FORMAT,
            &fmt, sizeof(fmt)));
        tivxCheckStatus(&status, vxQueryImage(img, (vx_enum)VX_IMAGE_WIDTH,
            &w, sizeof(w)));
        tivxCheckStatus(&status, vxQueryImage(img, (vx_enum)VX_IMAGE_HEIGHT,
            &h, sizeof(h)));
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Check for validity of data format */
        if( ((vx_df_image)VX_DF_IMAGE_NV12 != fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U8 != fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != fmt) 
            #if defined(VPAC3) || defined(VPAC3L)
            &&
            ((vx_df_image)VX_DF_IMAGE_UYVY != fmt)
            #endif
            )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Input Image Format \n");
        }
    }

    if (((vx_status)VX_SUCCESS == status) &&
        ((vx_bool)vx_false_e == tivxIsReferenceVirtual((vx_reference)pmd)))
    {
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_WIDTH, &p_w, sizeof(p_w)));
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_HEIGHT, &p_h, sizeof(p_h)));
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_FORMAT, &p_fmt, sizeof(p_fmt)));

        /* Check for frame sizes */
        if (((w / TIVX_VPAC_MSC_MAX_DS_FACTOR) > p_w) ||
            ((h / TIVX_VPAC_MSC_MAX_DS_FACTOR) > p_h))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Scale out of range [1.0 downto 0.25]\n");
        }

        /* Check for validity of data format */
        if( ((vx_df_image)VX_DF_IMAGE_NV12 != p_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U8 != p_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_U16 != p_fmt) &&
            ((vx_df_image)TIVX_DF_IMAGE_P12 != p_fmt) 
            #if defined(VPAC3) || defined(VPAC3L)
            &&
            ((vx_df_image)VX_DF_IMAGE_UYVY != p_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_YUYV != p_fmt)
            #endif
            )
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Pyramid Format\n");
        }

        if (((vx_df_image)VX_DF_IMAGE_NV12 == p_fmt) &&
            ((vx_df_image)VX_DF_IMAGE_NV12 != fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Pyramid Format MisMatch\n");
        }
        #if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((((vx_df_image)VX_DF_IMAGE_UYVY == p_fmt) ||
            ((vx_df_image)VX_DF_IMAGE_YUYV == p_fmt)) &&
            ((vx_df_image)VX_DF_IMAGE_UYVY != fmt))
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "'Pyramid' is YUV422I but 'in_img' is not YUV422I \n");
        }
        /* LDRA_JUSTIFY_END */
        #endif
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_SCALE, &scale, sizeof(scale)));
        tivxCheckStatus(&status, vxQueryPyramid(pmd, (vx_enum)VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels)));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = vxQueryNode(node, (vx_enum)VX_NODE_BORDER, &border, sizeof(border));
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            if ((border.mode != (vx_enum)VX_BORDER_REPLICATE) &&
                (border.mode != (vx_enum)VX_BORDER_UNDEFINED))
            {
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
                VX_PRINT(VX_ZONE_ERROR, "Only replicate/Undefined border mode is supported \n");
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])
        /* LDRA_JUSTIFY_END */
        {
            tivxCheckStatus(&status, vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_WIDTH, &p_w, sizeof(p_w)));
            tivxCheckStatus(&status, vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_HEIGHT, &p_h, sizeof(p_h)));
            tivxCheckStatus(&status, vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_FORMAT, &p_fmt, sizeof(p_fmt)));
            tivxCheckStatus(&status, vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_LEVELS, &num_levels, sizeof(num_levels)));
            tivxCheckStatus(&status, vxSetMetaFormatAttribute(metas[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX],
                (vx_enum)VX_PYRAMID_SCALE, &scale, sizeof(scale)));
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacMscPyramidInitialize(vx_node node,
            const vx_reference parameters[ ],
            vx_uint32 num_params)
{
    vx_status status = (vx_status)VX_SUCCESS;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */

    if (num_params != TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS)
    {
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

#if !defined(VPAC3L)
/* MSC: Initialize safety mechanism parameters */
void tivx_vpac_msc_safety_mech_prms_init(tivx_vpac_msc_safety_mechanism_params_t *prms)
{
    if (NULL != prms)
    {
        prms->enable_reconfig_and_reinit_Reg   = (uint32_t)vx_false_e;
        prms->enable_status_reg_validation     = (uint32_t)vx_false_e;
        prms->enable_readback_config_registers = (uint32_t)vx_false_e;
    }
}
#endif
/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

#endif
