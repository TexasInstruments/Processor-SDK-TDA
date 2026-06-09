/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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
#include <vx_vpac_fc_target_priv.h>
#include "tivx_hwa_vpac_fc_priv.h"
#include <utils/ipc/include/app_ipc.h>
#include <stddef.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacFcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacFcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacFcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacFcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static tivxVpacFcObj *tivxVpacFcAllocObject(tivxVpacFcInstObj *instObj);
static void tivxVpacFcFreeObject(tivxVpacFcInstObj *instObj,
    tivxVpacFcObj *fcObj);

static vx_status tivxVpacFcCheckInputDesc(uint16_t num_params, tivx_obj_desc_t *obj_desc[]);
static vx_status tivxVpacFcMapUserDesc(void **target_ptr, tivx_obj_desc_user_data_object_t *desc, uint32_t size);
static vx_status tivxVpacFcVissMscSetParams(tivxVpacVissObj *vissObj, tivxVpacMscScaleObj *mscObj, const tivx_vpac_viss_params_t *tivxVissPrms,
    const tivx_vpac_fc_viss_msc_params_t *fcPrms, const tivx_obj_desc_raw_image_t *raw_img_desc, tivx_obj_desc_image_t *img_desc_fc_out[]);

static vx_status tivxVpacFcVissMscMapFormat(Fvid2_Format *Vissfmt, Fvid2_Format *Mscfmt, uint32_t out_start, uint32_t mux_val, uint32_t vxFmt, uint32_t *plane_count);
static vx_status tivxVpacFcUnmapUserDesc(void **target_ptr, tivx_obj_desc_user_data_object_t *desc);

static void tivxVpacFcMscScaleCopyOutPrmsToScCfg(Msc_ScConfig *scCfg, uint32_t cnt);

int32_t tivxVpacFcFrameComplCb(Fvid2_Handle handle, void *appData);


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_vpac_fc_target_kernel = NULL;

tivxVpacFcInstObj       gTivxVpacFcInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacFc(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Flexconnect Add Kernel */
    if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
    /* LDRA_JUSTIFY_END */
    {
        strncpy(target_name, TIVX_TARGET_VPAC_FC, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #if (VPAC_COUNT > 1)
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_FC, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #endif
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
        VX_PRINT(VX_ZONE_ERROR, "Invalid CPU ID\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        memset(&gTivxVpacFcInstObj, 0, sizeof(tivxVpacFcInstObj));

        status = tivxMutexCreate(&gTivxVpacFcInstObj.lock);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Allocate Flexconnect lock \n");
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        vx_vpac_fc_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_FC_NAME,
                            target_name,
                            tivxVpacFcProcess,
                            tivxVpacFcCreate,
                            tivxVpacFcDelete,
                            tivxVpacFcControl,
                            NULL);
    }
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start>void tivxRemoveTargetKernelVpacFc.* <function end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: None; Unused feature, cannot be enabled, control cannot reach to this condition.
<justification end> */
void tivxRemoveTargetKernelVpacFc(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    if (NULL != gTivxVpacFcInstObj.lock)
    {
        (void)tivxMutexDelete(&gTivxVpacFcInstObj.lock);
    }

    status = tivxRemoveTargetKernel(vx_vpac_fc_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_fc_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Flexconnect TargetKernel\n");
    }
}
/* LDRA_JUSTIFY_END */

/* Setting the output descriptor flags to invalid if received raw_image is invalid */
static void tivxVpacFcSetIsInvalidFlag(tivx_obj_desc_t *obj_desc[])
{
    uint32_t                   cnt;
    uint32_t                   out_start;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if ((uint32_t)tivxFlagIsBitSet(obj_desc[TIVX_KERNEL_VPAC_FC_VISS_RAW_IDX]->flags, TIVX_REF_FLAG_IS_INVALID) == 1U)
    {
        out_start = TIVX_KERNEL_VPAC_FC_MSC_SCALE_OUT0_IMG_IDX;
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
        {
            if (NULL != obj_desc[out_start])
            {
                tivxFlagBitSet(&obj_desc[out_start]->flags, TIVX_REF_FLAG_IS_INVALID);
            }
            out_start ++;
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_FC_VISS_H3A_AEW_AF_IDX])
        {
            tivxFlagBitSet(&obj_desc[TIVX_KERNEL_VPAC_FC_VISS_H3A_AEW_AF_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_FC_VISS_HISTOGRAM0_IDX])
        {
            tivxFlagBitSet(&obj_desc[TIVX_KERNEL_VPAC_FC_VISS_HISTOGRAM0_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        out_start = TIVX_KERNEL_VPAC_FC_MSC_SCALE_OUT0_IMG_IDX;
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            if (NULL != obj_desc[out_start])
            /* LDRA_JUSTIFY_END */
            {
                tivxFlagBitClear(&obj_desc[out_start]->flags, TIVX_REF_FLAG_IS_INVALID);
            }
            out_start ++;
        }

        if (NULL != obj_desc[TIVX_KERNEL_VPAC_FC_VISS_H3A_AEW_AF_IDX])
        {
            tivxFlagBitClear(&obj_desc[TIVX_KERNEL_VPAC_FC_VISS_H3A_AEW_AF_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (NULL != obj_desc[TIVX_KERNEL_VPAC_FC_VISS_HISTOGRAM0_IDX])
        {
            tivxFlagBitClear(&obj_desc[TIVX_KERNEL_VPAC_FC_VISS_HISTOGRAM0_IDX]->flags, TIVX_REF_FLAG_IS_INVALID);
        }
        /* LDRA_JUSTIFY_END */
    }
}
/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacFcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                  status = (vx_status)VX_SUCCESS;
    int32_t                    fvid2_status = FVID2_SOK;
    uint32_t                   out_start;
    uint32_t                   cnt;

    tivxVpacFcObj                       *fcObj = NULL;
    Vhwa_M2mFcGraphPathInfo             *fcPathInfo = NULL;
    tivx_vpac_fc_viss_msc_params_t      *fcPrms = NULL;

    tivxVpacVissObj                     *vissObj = NULL;
    Vhwa_M2mVissParams                  *vissDrvPrms = NULL;
    tivx_vpac_viss_params_t             *tivxVissPrms = NULL;

    tivxVpacMscScaleObj                 *msc_obj = NULL;
    Vhwa_M2mMscParams                   *msc_prms = NULL;

    tivx_obj_desc_raw_image_t           *raw_img_desc_fc_in = NULL;
    tivx_obj_desc_image_t               *img_desc_fc_out[TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT] = {NULL};
    tivx_obj_desc_image_t               *img_desc_viss_out[TIVX_KERNEL_VPAC_FC_VISS_MAX_IMAGE_OUTPUT] = {NULL};

    tivx_obj_desc_user_data_object_t    *config_desc = NULL;
    tivx_ae_awb_params_t                *ae_awb_result = NULL;
    tivx_obj_desc_user_data_object_t    *aewb_res_desc = NULL;
    tivx_obj_desc_user_data_object_t    *h3a_out_desc = NULL;
    tivx_obj_desc_user_data_object_t    *dcc_buf_desc = NULL;
    tivx_obj_desc_distribution_t        *raw_histogram_desc = NULL;

    tivxVpacMscArgs                     msc_args;

    /* Check for mandatory descriptor */
    status = tivxVpacFcCheckInputDesc(num_params, obj_desc);
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
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        fcObj = tivxVpacFcAllocObject(&gTivxVpacFcInstObj);
        msc_obj = &fcObj->msc_obj;
        vissObj = &fcObj->vissObj;
        fcPathInfo = &fcObj->pathInfo;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != fcObj)
        /* LDRA_JUSTIFY_END */
        {
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_FC_CONFIGURATION_IDX];
            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_FC_VISS_AE_AWB_RESULT_IDX];
            dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_FC_DCC_BUF_IDX];
            h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_FC_VISS_H3A_AEW_AF_IDX];
            raw_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[
                TIVX_KERNEL_VPAC_FC_VISS_RAW_HISTOGRAM_IDX];                
            raw_img_desc_fc_in = (tivx_obj_desc_raw_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_FC_VISS_RAW_IDX];

            /* Get All VISS output image object descriptors */

            out_start = TIVX_KERNEL_VPAC_FC_VISS_OUT0_IDX;

            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc_viss_out[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }

            /* Get All MSC output image object descriptors */
            out_start = TIVX_KERNEL_VPAC_FC_MSC_SCALE_OUT0_IMG_IDX;
            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc_fc_out[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Alloc Failed for Flexconnect Object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(config_desc->mem_size != sizeof(tivx_vpac_fc_viss_msc_params_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_vpac_fc_viss_msc_params_t, host size (%d) != target size (%d)\n",
                    config_desc->mem_size, sizeof(tivx_vpac_fc_viss_msc_params_t));
            }
            /* LDRA_JUSTIFY_END */
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != aewb_res_desc))
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(aewb_res_desc->mem_size != sizeof(tivx_ae_awb_params_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_ae_awb_params_t, host size (%d) != target size (%d)\n",
                    aewb_res_desc->mem_size, sizeof(tivx_ae_awb_params_t));
            }
            /* LDRA_JUSTIFY_END */
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) 
        && (NULL != raw_histogram_desc))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(raw_histogram_desc->mem_size != ((128U) * (sizeof(uint32_t))))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "raw_histogram_desc, host size (%d) != target size (%d)\n",
                    raw_histogram_desc->mem_size, ((128U) * (sizeof(uint32_t))));
            }
            /* LDRA_JUSTIFY_END */
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != h3a_out_desc))
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(h3a_out_desc->mem_size != sizeof(tivx_h3a_data_t))
            {
                status = (vx_status)VX_FAILURE;

                VX_PRINT(VX_ZONE_ERROR, "tivx_h3a_data_t, host size (%d) != target size (%d)\n",
                    h3a_out_desc->mem_size, sizeof(tivx_h3a_data_t));
            }
            /* LDRA_JUSTIFY_END */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != dcc_buf_desc)
            /* LDRA_JUSTIFY_END */
            {
                vissObj->h3a_out_enabled = (vx_bool)vx_true_e;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to print the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                VX_PRINT(VX_ZONE_WARNING,
                    "VISS H3A output is not generated due to DCC not being enabled\n");
            }
            /* LDRA_JUSTIFY_END */

            /* This is taking the app ipc cpu number, not the tivx mapped number, so it can
             * be used by AWB node for direct usage without translation */
            vissObj->cpu_id = appIpcGetSelfCpuId();

        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxMutexCreate(&fcObj->config_lock);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Now Map config Desc and get FC Parameters */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxVpacFcMapUserDesc(&fcObj->fc_prms_target_ptr,
                config_desc, sizeof(tivx_vpac_fc_viss_msc_params_t));
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                fcPrms = (tivx_vpac_fc_viss_msc_params_t *)
                    fcObj->fc_prms_target_ptr;

                tivxVissPrms = &fcPrms->tivxVissPrms;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Map Flexconnect Parameters Descriptor\n");
            }
            /* LDRA_JUSTIFY_END */      
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate mutex\n");
        }
        /* LDRA_JUSTIFY_END */
        /* Extract AEWB Result parameters, it might be needed in
         * setting some of the VISS configuration */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            if(NULL != aewb_res_desc)
            {
                status = tivxVpacFcMapUserDesc(&vissObj->aewb_res_target_ptr,
                    aewb_res_desc, sizeof(tivx_ae_awb_params_t));
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    ae_awb_result = (tivx_ae_awb_params_t *)
                        vissObj->aewb_res_target_ptr;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Map AEWB Result Descriptor\n");
                }
                /* LDRA_JUSTIFY_END */
            }
            else
            {
                uint32_t i, loop_break = 0;

                /* AEWB Result sent by the graph is NULL */
                /* VISS needs to use the results sent by AEWB node through VISS_CMD_SET_2A_PARAMS command */
                /* RemoteService command is supported only on target*/
                for(i=0;i<VHWA_M2M_VISS_MAX_HANDLES;i++)
                {
                    status = tivxMutexLock(viss_aewb_lock[i]);
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if((vx_status)VX_SUCCESS == status)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(0U == viss_aewb_channel[i])
                        {
                            viss_aewb_channel[i] = 1u;
                            vissObj->channel_id = i;
                            (void)memset(&viss_aewb_results[vissObj->channel_id], 0, sizeof(tivx_ae_awb_params_t));
                            loop_break = 1;
                        }
                        status = tivxMutexUnlock(viss_aewb_lock[i]);
                    }

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */                   
                    if(((vx_status)VX_SUCCESS != status) || (loop_break == 1U))
                    /* LDRA_JUSTIFY_END */
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "viss_aewb_lock[%d] failed\n", i);
                        }
                        /* LDRA_JUSTIFY_END */
                        break;
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(((vx_status)VX_SUCCESS == status) && (loop_break == 0U))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Number of instances has exceeded VHWA_M2M_VISS_MAX_HANDLES\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }
                /* LDRA_JUSTIFY_END */
            }
        }
    }

    /* VISS Create */
    vissDrvPrms = &vissObj->vissPrms;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Set number of inputs */
    if (NULL != raw_img_desc_fc_in)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vissDrvPrms)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (1U == raw_img_desc_fc_in->params.num_exposures)
            /* LDRA_JUSTIFY_END */
            {
            vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if (2U == raw_img_desc_fc_in->params.num_exposures)
            {
            vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE;
            }
            else if (3U == raw_img_desc_fc_in->params.num_exposures)
            {
            vissDrvPrms->inputMode = VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE;
            }
            else
            {
            /* do nothing */
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "vissDrvPrms is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "raw_img_desc_fc_in is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    msc_prms = &msc_obj->msc_prms;

    Vhwa_m2mMscParamsInit(msc_prms);

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Now allocate the require resources and open the FVID2 driver */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxEventCreate(&fcObj->waitForProcessCmpl);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
        }
        /* LDRA_JUSTIFY_END */

        if ((vx_status)VX_SUCCESS == status)
        {
            fcObj->cbPrms.cbFxn   = tivxVpacFcFrameComplCb;
            fcObj->cbPrms.appData = fcObj;

            fcObj->handle = Fvid2_create(FVID2_VHWA_M2M_FC_DRV_ID,
                fcObj->fc_drv_inst_id, &fcObj->createArgs,
                NULL, &fcObj->cbPrms);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == fcObj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Open Driver\n");
                status = (vx_status)VX_ERROR_NO_RESOURCES;
            }
            /* LDRA_JUSTIFY_END */
        }
    }


    /* Parse the inputs/outputs to update the path info */
    uint32_t edge_counter = 0U;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Parse the VISS inputs enabled to create DDR -> VISS_IN edges */
    if (((NULL != vissDrvPrms) && (NULL != fcPathInfo)))
    /* LDRA_JUSTIFY_END */
    {
        switch(vissDrvPrms->inputMode)
        {
            case VHWA_M2M_VISS_MODE_SINGLE_FRAME_INPUT:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_DDR;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_VISS_IN_0;
                edge_counter++;
                break;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            case VHWA_M2M_VISS_MODE_TWO_FRAME_MERGE:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_DDR;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_VISS_IN_0;
                edge_counter++;
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_DDR;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_VISS_IN_1;
                edge_counter++;
                break;
            case VHWA_M2M_VISS_MODE_THREE_FRAME_MERGE:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_DDR;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_VISS_IN_0;
                edge_counter++;
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_DDR;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_VISS_IN_1;
                edge_counter++;
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_DDR;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_VISS_IN_2;
                edge_counter++;
                break;
            default:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_DDR;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_VISS_IN_0;
                edge_counter++;
                break;
            /* LDRA_JUSTIFY_END */
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "vissDrvPrms or fcPathInfo is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    /* Parse the viss-msc mapping and update the path for all msc threads */
    for (uint32_t thread_id = 0U; thread_id < TIVX_KERNEL_VPAC_FC_MAX_MSC_INPUT_THREADS; thread_id++)
    {
        if (((NULL != fcPrms) && (NULL != fcPathInfo)))
        {
            switch (fcPrms->msc_in_thread_viss_out_map[thread_id])
            {
            case TIVX_VPAC_FC_VISS_OUT0:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_VISS_OUT_Y12;
                fcPathInfo->edgeInfo[edge_counter].endPort = (uint64_t)VHWA_FC_PORT_MSC0_IN_0 + (uint64_t)thread_id;
                edge_counter++;
                break;
            case TIVX_VPAC_FC_VISS_OUT1:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_VISS_OUT_UV12;
                fcPathInfo->edgeInfo[edge_counter].endPort = (uint64_t)VHWA_FC_PORT_MSC0_IN_0 + (uint64_t)thread_id;
                edge_counter++;
                break;
            case TIVX_VPAC_FC_VISS_OUT2:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_VISS_OUT_Y8;
                fcPathInfo->edgeInfo[edge_counter].endPort = (uint64_t)VHWA_FC_PORT_MSC0_IN_0 + (uint64_t)thread_id;
                edge_counter++;
                break;
            case TIVX_VPAC_FC_VISS_OUT3:
                fcPathInfo->edgeInfo[edge_counter].startPort = VHWA_FC_PORT_VISS_OUT_UV8;
                fcPathInfo->edgeInfo[edge_counter].endPort = (uint64_t)VHWA_FC_PORT_MSC0_IN_0 + (uint64_t)thread_id;
                edge_counter++;
                break;
            case TIVX_VPAC_FC_MSC_CH_INVALID:
                break;
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid VISS out and MSC%d mapping \n", thread_id);
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "fcPrms or fcPathInfo is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Parse the msc outputs enabled to create MSC_OUT -> DDR edges */
    uint32_t viss_output_msc0_cnt = 0U;
    uint32_t viss_output_msc1_cnt = 0U;
    for(cnt = 0U; cnt < TIVX_VPAC_MSC_MAX_OUTPUT; cnt ++)
    {
        if (((NULL != fcPrms) && (NULL != fcPathInfo)))
        {
            switch(fcPrms->msc_out_msc_in_map[cnt])
            {
            case TIVX_VPAC_FC_MSC0:
                fcPathInfo->edgeInfo[edge_counter].startPort = (uint64_t)VHWA_FC_PORT_MSC0_OUT_0 + (uint64_t)cnt;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_DDR;
                edge_counter++;
                viss_output_msc0_cnt++;
                break;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            case TIVX_VPAC_FC_MSC1:
                fcPathInfo->edgeInfo[edge_counter].startPort = (uint64_t)VHWA_FC_PORT_MSC1_OUT_0 + (uint64_t)cnt;
                fcPathInfo->edgeInfo[edge_counter].endPort = VHWA_FC_PORT_DDR;
                edge_counter++;
                viss_output_msc1_cnt++;
                break;
            /* LDRA_JUSTIFY_END */
            case TIVX_VPAC_FC_MSC_TH_INVALID:
                break;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            default:
                VX_PRINT(VX_ZONE_ERROR, "Invalid MSC output and MSC%d mapping \n", cnt);
                status = (vx_status)VX_FAILURE;
                break;
            /* LDRA_JUSTIFY_END */
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "fcPrms or fcPathInfo is NULL\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    if (fcPathInfo != NULL)
    {
        fcPathInfo->numEdges = edge_counter;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement<metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "fcPathInfo is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    /* Now allocate the require resources and open the FVID2 driver */
    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set Flex-connect path */
        status = Fvid2_control(fcObj->handle, IOCTL_VHWA_FC_SET_GRAPH,
                            (void*)fcPathInfo, NULL);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set Flex-connect path */
        status = Fvid2_control(fcObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                            NULL, NULL);
    }

    /* VISS params update */


    /* Extract the format information from the config descriptor
     * and output images and set the format in Driver using
     * SET_PARAMS ioctl  */

    /* Initialise VISS format; this shouldn't be changed */
    if (tivxVissPrms != NULL)
    {
        tivxVissPrms->fcp[0].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        tivxVissPrms->fcp[0].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;
        tivxVissPrms->fcp[0].mux_output2 = TIVX_VPAC_VISS_MUX2_Y8;
        tivxVissPrms->fcp[0].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;

    #if defined(VPAC3)
        tivxVissPrms->fcp[1].mux_output0 = TIVX_VPAC_VISS_MUX0_Y12;
        tivxVissPrms->fcp[1].mux_output1 = TIVX_VPAC_VISS_MUX1_UV12;
        tivxVissPrms->fcp[1].mux_output2 = TIVX_VPAC_VISS_MUX2_Y8;
        tivxVissPrms->fcp[1].mux_output3 = TIVX_VPAC_VISS_MUX3_UV8;
    #endif
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement<metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "tivxVissPrms is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    if ((vx_status)VX_SUCCESS == status)
    {
        tivxCheckStatus(&status, tivxVpacVissSetParams(vissDrvPrms, tivxVissPrms));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set the input image format and number of inputs from
        * raw image descriptor */
        tivxVpacVissSetInputParams(vissObj, raw_img_desc_fc_in);

        /* FC_TODO: Review this, set output params for VISS->DDR, Re-enable later */


        status = tivxVpacFcVissMscSetParams(vissObj, msc_obj,
                    tivxVissPrms, fcPrms, raw_img_desc_fc_in, img_desc_fc_out);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch<metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set VISS->MSC Output Params\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Set default values for ALL viss configuration parameters */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacVissSetDefaultParams(vissObj, tivxVissPrms, NULL);
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* Set H3A Source parameters, this mainly sets up the
         * input source to the h3a. */
        tivxVpacVissSetH3aSrcParams(vissObj, tivxVissPrms);
    }

    /* Extract individual module specific parameters from the DCC data
     * base and set it in the driver */
    if ((vx_status)VX_SUCCESS == status)
    {
        if (NULL != dcc_buf_desc)
        {
            vissObj->use_dcc = 1u;

            status = tivxVpacVissInitDcc(vissObj, tivxVissPrms);

            if ((vx_status)VX_SUCCESS == status)
            {
                /* Parse DCC Database and store the output in local variables */
                status = tivxVpacVissSetParamsFromDcc(
                    vissObj, dcc_buf_desc, h3a_out_desc, ae_awb_result);
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set DCC Params\n");
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set DCC Params\n");
            }
            /* LDRA_JUSTIFY_END */

            /* Enable DPC based in driver */
            if (((uint32_t)UTRUE == vissObj->vissCfg.dpcLutCfg.enable) ||
                ((uint32_t)UTRUE == vissObj->vissCfg.dpcOtfCfg.enable))
            {
                vissDrvPrms->enableDpc = (uint32_t)UTRUE;
            }
            else
            {
                vissDrvPrms->enableDpc = (uint32_t)UFALSE;
            }

            #ifdef VPAC3L
            /* Set the DPC OTF CFA mode */
            vissDrvPrms->cfa_mode = vissObj->vissCfg.dpcOtfCfg.cfa_mode;
            #endif

        }
    }

    /* before writing configuration set application buffer */
    if ((vx_status) VX_SUCCESS == status)
    {
        status = tivxVpacFcVissSetConfigBuffer(fcObj);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to set ConfigBuf in driver\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        /* All Formats, frame size, module enables are set in
         * viss parameters, call this ioctl to set validate and set
         * them in the driver */
        fvid2_status = Fvid2_control(fcObj->handle,
            IOCTL_VHWA_M2M_VISS_SET_PARAMS, (void *)vissDrvPrms, NULL);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to set Params in driver\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Now Set the parsed parameters in the VISS Driver,
     * This is required even for the non-DCC parameters, like
     * H3A Input source */
    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxVpacFcVissSetConfigInDrv(fcObj);

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Reset this flag, as the config is already applied to driver */
            vissObj->isConfigUpdated = 0U;
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Parse and Set non-DCC Params\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* MSC params and config update */
    if ((vx_status)VX_SUCCESS == status)
    {


        /* set sclar parameters and output format for each level*/


        if(viss_output_msc0_cnt > 0U)
        {
            msc_args.mscInstId = VPAC_MSC_INST_ID_0;
            msc_args.mscArgs   = (Ptr) msc_prms;
            fvid2_status = Fvid2_control(fcObj->handle, VHWA_M2M_IOCTL_MSC_SET_PARAMS,
                &msc_args, NULL);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(viss_output_msc1_cnt > 0U)
        {
            msc_args.mscInstId = VPAC_MSC_INST_ID_1;
            msc_args.mscArgs   = (Ptr) msc_prms;
            fvid2_status = Fvid2_control(fcObj->handle, VHWA_M2M_IOCTL_MSC_SET_PARAMS,
                &msc_args, NULL);
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            /* Set up the VISS output frame list */
            for (cnt = 0U; cnt < VHWA_M2M_VISS_MAX_OUTPUTS; cnt++)
            {
                fcObj->outFrmList.frames[VHWA_FC_VISS_DST_BUFF_IDX_START + cnt] =
                         &fcObj->outFrm[VHWA_FC_VISS_DST_BUFF_IDX_START + cnt];
            }

            if(viss_output_msc0_cnt > 0U)
            {
                /* Set up the MSC0 output frame list */
                for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
                {
                    fcObj->outFrmList.frames[VHWA_FC_MSC0_DST_BUFF_IDX_START + cnt] = &fcObj->outFrm[VHWA_FC_MSC0_DST_BUFF_IDX_START + cnt];
                    fcObj->num_out_buf_addr[VHWA_FC_MSC0_DST_BUFF_IDX_START + cnt] = 1U;
                }
                fcObj->outFrmList.numFrames = TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(viss_output_msc1_cnt > 0U)
            {
                /* Set up the MSC1 output frame list */
                for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
                {
                    fcObj->outFrmList.frames[VHWA_FC_MSC1_DST_BUFF_IDX_START + cnt] = &fcObj->outFrm[VHWA_FC_MSC1_DST_BUFF_IDX_START + cnt];
                    fcObj->num_out_buf_addr[VHWA_FC_MSC1_DST_BUFF_IDX_START + cnt] = 1U;
                }
                fcObj->outFrmList.numFrames = TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* Configure Flexconnect for VISS and MSC */
    if(FVID2_SOK == status)
    {
        status = Fvid2_control(fcObj->handle, IOCTL_VHWA_FC_SET_CONFIG,
                                NULL, NULL);
    }

    /* Unmap descriptor memories */
    if ((vx_status)VX_SUCCESS == status)
    {
        /* If the target pointer is non null, descriptor is also non null,
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
        if ((NULL != aewb_res_desc) && (NULL != fcObj->vissObj.aewb_res_target_ptr))
        {
            status = tivxVpacVissUnmapUserDesc(&fcObj->vissObj.aewb_res_target_ptr,
                aewb_res_desc);
        }

        /* If the target pointer is non null, descriptor is also non null
         * Even if there is any error, if this pointer is non-null,
         * unmap must be called */
        if (NULL != fcObj->fc_prms_target_ptr)
        {
            status = tivxVpacFcUnmapUserDesc(&fcObj->fc_prms_target_ptr,
                config_desc);
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: FC set Config \n");
        status = (vx_status)VX_FAILURE;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxSetTargetKernelInstanceContext(
            kernel, fcObj, sizeof(tivxVpacFcObj));

        if ((vx_status)VX_SUCCESS == status)
        {
            /* Set up the input frame list */
            for (cnt = 0U; cnt < vissObj->num_in_buf; cnt ++)
            {
                fcObj->inFrmList.frames[cnt] = &fcObj->inFrm[cnt];
            }
            fcObj->inFrmList.numFrames = vissObj->num_in_buf;
        }
    }

    if ((vx_status)VX_SUCCESS != status)
    {
        if (NULL != fcObj)
        {
            tivxVpacVissDeInitDcc(&fcObj->vissObj);

            if (NULL != fcObj->handle)
            {
                (void)Fvid2_delete(fcObj->handle, NULL);
                fcObj->handle = NULL;
            }

            if (NULL != fcObj->waitForProcessCmpl)
            {
                (void)tivxEventDelete(&fcObj->waitForProcessCmpl);
            }

            if (NULL != fcObj->config_lock)
            {
                (void)tivxMutexDelete(&fcObj->config_lock);
            }

            tivxVpacFcFreeObject(&gTivxVpacFcInstObj, fcObj);

        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacFcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 size;
    tivxVpacFcObj         *fcObj = NULL;

    /* Check for mandatory descriptor */
    status = tivxVpacFcCheckInputDesc(num_params, obj_desc);
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
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
        if ((vx_status)VX_SUCCESS == status)
        {
            fcObj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check the validity of context object */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect kernel instance context\n");
        }
        else if ((NULL == fcObj) ||
            (sizeof(tivxVpacFcObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {

            tivx_obj_desc_user_data_object_t *aewb_res_desc = NULL;

            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_VISS_AE_AWB_RESULT_IDX];

            if(NULL == aewb_res_desc)
            {
                status = tivxMutexLock(viss_aewb_lock[fcObj->vissObj.channel_id]);
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    if(0U != viss_aewb_channel[fcObj->vissObj.channel_id])
                    {
                        viss_aewb_channel[fcObj->vissObj.channel_id] = 0u;
                    }
                    status = tivxMutexUnlock(viss_aewb_lock[fcObj->vissObj.channel_id]);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "viss_aewb_lock[%d] failed\n", fcObj->vissObj.channel_id);
                }
                /* LDRA_JUSTIFY_END */
            }

            tivxVpacVissDeInitDcc(&fcObj->vissObj);

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error. This behaviour is part of the application design.
            However, due to the stated rationale, this is not tested.
            <justification end> */            
            if (true == fcObj->vissObj.configurationBuffer.configThroughUdmaFlag)
            /* LDRA_JUSTIFY_END */
            {
                tivxVpacFcVissDeleteConfigBuffer(fcObj);
            }

            (void)Fvid2_delete(fcObj->handle, NULL);
            fcObj->handle = NULL;

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */ 
            if (NULL != fcObj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&fcObj->waitForProcessCmpl);
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */ 
            if (NULL != fcObj->config_lock)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxMutexDelete(&fcObj->config_lock);
            }

            tivxVpacFcFreeObject(&gTivxVpacFcInstObj, fcObj);
        }
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacFcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                  status = (vx_status)VX_SUCCESS;
    int32_t                    fvid2_status = FVID2_SOK;
    uint32_t                   cnt;
    uint32_t                   out_start;
    uint32_t                   buf_cnt;
    uint32_t                   size;
    uint64_t                   start_time, cur_time;

    tivxVpacFcObj                       *fcObj = NULL;
    tivx_vpac_fc_viss_msc_params_t      *fcPrms;

    tivxVpacVissObj                     *vissObj = NULL;
    tivx_vpac_viss_params_t             *tivxVissPrms;

    tivx_obj_desc_raw_image_t           *raw_img_desc_fc_in = NULL;
    tivx_obj_desc_image_t               *img_desc_fc_out[TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT];
    tivx_obj_desc_image_t               *img_desc_viss_out[TIVX_KERNEL_VPAC_FC_VISS_MAX_IMAGE_OUTPUT];

    tivx_obj_desc_user_data_object_t    *config_desc = NULL;
    tivx_ae_awb_params_t                *ae_awb_result = NULL;
    tivx_obj_desc_user_data_object_t    *aewb_res_desc = NULL;
    tivx_obj_desc_user_data_object_t    *h3a_out_desc = NULL;

    tivx_ae_awb_params_t                 aewb_params;
    tivx_h3a_data_t                     *h3a_out = NULL;

    /* Check for mandatory descriptor */
    status = tivxVpacFcCheckInputDesc(num_params, obj_desc);
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
        VX_PRINT(VX_ZONE_ERROR, "Input Descriptor Error\n");
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
        if ((vx_status)VX_SUCCESS == status)
        {
            fcObj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Check the validity of context object */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect kernel instance context\n");
        }
        else if ((NULL == fcObj) ||
            (sizeof(tivxVpacFcObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
            status = (vx_status)VX_FAILURE;
        }        
        else
        /* LDRA_JUSTIFY_END */
        {
            vissObj = &fcObj->vissObj;

            /* Assign object descriptors */
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_FC_CONFIGURATION_IDX];
            raw_img_desc_fc_in = (tivx_obj_desc_raw_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_FC_VISS_RAW_IDX];
            aewb_res_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_FC_VISS_AE_AWB_RESULT_IDX];
            h3a_out_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[
                TIVX_KERNEL_VPAC_FC_VISS_H3A_AEW_AF_IDX];
            /* Get All VISS output image object descriptors */
            out_start = TIVX_KERNEL_VPAC_FC_VISS_OUT0_IDX;
            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_VISS_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc_viss_out[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }
            /* Get All output image object descriptors */
            out_start = TIVX_KERNEL_VPAC_FC_MSC_SCALE_OUT0_IMG_IDX;
            for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
            {
                img_desc_fc_out[cnt] = (tivx_obj_desc_image_t *)obj_desc[out_start];
                out_start ++;
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ( (vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            tivxVpacFcSetIsInvalidFlag(obj_desc);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Now Map config Desc and get FC Parameters */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxVpacFcMapUserDesc(&fcObj->fc_prms_target_ptr,
                config_desc, sizeof(tivx_vpac_fc_viss_msc_params_t));
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                fcPrms = (tivx_vpac_fc_viss_msc_params_t *)
                    fcObj->fc_prms_target_ptr;

                tivxVissPrms = &fcPrms->tivxVissPrms;

            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Map Flexconnect Parameters Descriptor\n");
            }
            /* LDRA_JUSTIFY_END */
            
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            /* AEWB Result is optional parameter */
            if((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                if(NULL != aewb_res_desc)
                {
                    status = tivxVpacVissMapUserDesc(&vissObj->aewb_res_target_ptr,
                        aewb_res_desc, sizeof(tivx_ae_awb_params_t));
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((vx_status)VX_SUCCESS == status)
                    /* LDRA_JUSTIFY_END */
                    {
                        ae_awb_result = (tivx_ae_awb_params_t *)vissObj->aewb_res_target_ptr;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to Map AEWB Result Descriptor\n");
                    }
                    /* LDRA_JUSTIFY_END */
                }
                else
                {
                    /* AEWB Result sent by the graph is NULL */
                    /* VISS needs to use the results sent by AEWB node through VISS_CMD_SET_2A_PARAMS command */
                    /* RemoteService command is supported only on target*/
                    uint32_t chId = vissObj->channel_id;
                    status = tivxMutexLock(viss_aewb_lock[chId]);
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if((vx_status)VX_SUCCESS == status)
                    /* LDRA_JUSTIFY_END */
                    {
                        ae_awb_result = &aewb_params;
                        memcpy(ae_awb_result, &viss_aewb_results[chId], sizeof(tivx_ae_awb_params_t));
                        status = tivxMutexUnlock(viss_aewb_lock[chId]);
                    }
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                if (NULL != h3a_out_desc)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start>  branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (h3a_out_desc->mem_size == sizeof(tivx_h3a_data_t))
                    /* LDRA_JUSTIFY_END */
                    {
                        vissObj->h3a_out_target_ptr = tivxMemShared2TargetPtr(&h3a_out_desc->mem_ptr);

                        h3a_out = (tivx_h3a_data_t *)vissObj->h3a_out_target_ptr;

                        /* H3A output is special case, only need to map the header since rest is written by HW */
                        tivxCheckStatus(&status, tivxMemBufferMap(vissObj->h3a_out_target_ptr, offsetof(tivx_h3a_data_t, resv),
                            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
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
                        VX_PRINT(VX_ZONE_ERROR, "Failed to Map H3A Result Descriptor\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }

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
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate mutex\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Update Configuration in Driver */
        status = tivxMutexLock(fcObj->config_lock);
        if((vx_status)VX_SUCCESS == status)
        {
            /* Check if there is any change in H3A Input Source */
            tivxVpacVissSetH3aSrcParams(vissObj, tivxVissPrms);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if (NULL != ae_awb_result)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxVpacVissApplyAEWBParams(vissObj, ae_awb_result);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to apply AEWB Result\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) && (1u == vissObj->isConfigUpdated))
        /* LDRA_JUSTIFY_END */
        {
            status = tivxVpacFcVissSetConfigInDrv(fcObj);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Set Config in Driver\n");
            }
            /* LDRA_JUSTIFY_END */
        }
            (void)tivxMutexUnlock(fcObj->config_lock);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Set the buffer address in the input buffer */
        for (cnt = 0u; cnt < fcObj->vissObj.num_in_buf; cnt ++)
        {
            fcObj->inFrm[cnt].addr[0u] = tivxMemShared2PhysPtr(
                raw_img_desc_fc_in->img_ptr[cnt].shared_ptr,
                (int32_t)((int32_t)raw_img_desc_fc_in->img_ptr[cnt].mem_heap_region));
        }

        /* Set the buffer address in the output buffer */
        for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
        {
            if(NULL != img_desc_fc_out[cnt])
            {
                if(TIVX_VPAC_FC_MSC_TH_INVALID != fcPrms->msc_out_msc_in_map[cnt])
                {
                    if (TIVX_VPAC_FC_MSC0 == fcPrms->msc_out_msc_in_map[cnt])
                    {
                        for (buf_cnt = 0U; buf_cnt < fcObj->num_out_buf_addr[VHWA_FC_MSC0_DST_BUFF_IDX_START + cnt];
                            buf_cnt ++)
                        {
                            fcObj->outFrm[VHWA_FC_MSC0_DST_BUFF_IDX_START + cnt].addr[buf_cnt] = tivxMemShared2PhysPtr(
                                img_desc_fc_out[cnt]->mem_ptr[buf_cnt].shared_ptr,
                                (int32_t)((int32_t)img_desc_fc_out[cnt]->mem_ptr[buf_cnt].mem_heap_region));
                        }
                    }
                    else
                    {
                        for (buf_cnt = 0U; buf_cnt < fcObj->num_out_buf_addr[VHWA_FC_MSC1_DST_BUFF_IDX_START + cnt];
                            buf_cnt ++)
                        {
                            fcObj->outFrm[VHWA_FC_MSC1_DST_BUFF_IDX_START + cnt].addr[buf_cnt] = tivxMemShared2PhysPtr(
                                img_desc_fc_out[cnt]->mem_ptr[buf_cnt].shared_ptr,
                                (int32_t)((int32_t)img_desc_fc_out[cnt]->mem_ptr[buf_cnt].mem_heap_region));
                        }
                    }
                }
            }
        }

        if (NULL != h3a_out)
        {
            h3a_out->aew_af_mode = tivxVissPrms->h3a_aewb_af_mode;
            h3a_out->h3a_source_data = tivxVissPrms->h3a_in;
            h3a_out->cpu_id = vissObj->cpu_id;
            h3a_out->channel_id = vissObj->channel_id;
            h3a_out->size = vissObj->h3a_output_size;

            if(0U == tivxVissPrms->h3a_aewb_af_mode)
            {
                /* TI 2A Node may not need the aew config since it gets it from DCC, but this is copied
                 * in case third party 2A nodes which don't use DCC can easily see this information */
                memcpy(&h3a_out->aew_config, &vissObj->aew_config, sizeof(tivx_h3a_aew_config));
            }

            fcObj->outFrm[VHWA_M2M_VISS_OUT_H3A_IDX].addr[0u] = tivxMemShared2PhysPtr(
                (uint64_t)h3a_out->data,
                (vx_enum)TIVX_MEM_EXTERNAL);

            h3a_out_desc->valid_mem_size = vissObj->h3a_output_size + TIVX_VPAC_VISS_H3A_OUT_BUFF_ALIGN;

            /* Unmap even before processing since the ARM is done, rest of buffer is HW */
            tivxCheckStatus(&status, tivxMemBufferUnmap(vissObj->h3a_out_target_ptr, offsetof(tivx_h3a_data_t, resv), (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
            vissObj->h3a_out_target_ptr = NULL;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {

        start_time = tivxPlatformGetTimeInUsecs();

        tivxLogRtTraceKernelInstanceExeStartTimestamp(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA, start_time);

        /* Submit the request to the driver */
        fvid2_status = Fvid2_processRequest(fcObj->handle, &fcObj->inFrmList,
            &fcObj->outFrmList, FVID2_TIMEOUT_FOREVER);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* Wait for Frame Completion */
            (void)tivxEventWait(fcObj->waitForProcessCmpl,
                VX_TIMEOUT_WAIT_FOREVER);

            cur_time = tivxPlatformGetTimeInUsecs();

            tivxLogRtTraceKernelInstanceExeEndTimestamp(kernel, TIVX_KERNEL_VPAC_VISS_RT_TRACE_OFFSET_HWA, cur_time);

            fvid2_status = Fvid2_getProcessedRequest(fcObj->handle,
                &fcObj->inFrmList, &fcObj->outFrmList, 0);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* If the target pointer is non null, descriptor is also non null,
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    if (((vx_status)VX_SUCCESS == status) && (NULL != aewb_res_desc))
    {
        status = tivxVpacFcUnmapUserDesc(&vissObj->aewb_res_target_ptr, aewb_res_desc);
    }

    /* If the target pointer is non null, descriptor is also non null
     * Even if there is any error, if this pointer is non-null,
     * unmap must be called */
    if (((vx_status)VX_SUCCESS == status) && (NULL != config_desc))
    {
        status = tivxVpacFcUnmapUserDesc(&fcObj->fc_prms_target_ptr, config_desc);
    }

    return (status);
}

static vx_status VX_CALLBACK tivxVpacFcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacFcObj                  *FcObj = NULL;

    void *temp_kernel_context = NULL;


    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
    if ((vx_status)VX_SUCCESS == status)
    {
        FcObj = temp_kernel_context;
    }

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
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
    }
    else if ((NULL == FcObj) ||
        (sizeof(tivxVpacFcObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Incorrect Object Size\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        /* do nothing */
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        switch (node_cmd_id)
        {
            case TIVX_VPAC_FC_MSC_CMD_SET_COEFF:
            {
                status = tivxVpacFcMscScaleSetCoeffsCmd(FcObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U], (tivx_obj_desc_user_data_object_t *)obj_desc[1U]);
                break;
            }
            case TIVX_VPAC_FC_MSC_CMD_SET_INPUT_PARAMS:
            {
                status = tivxVpacFcMscScaleSetInputParamsCmd(FcObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U], (tivx_obj_desc_user_data_object_t *)obj_desc[1U]);
                break;
            }
            case TIVX_VPAC_FC_MSC_CMD_SET_OUTPUT_PARAMS:
            {
                status = tivxVpacFcMscScaleSetOutputParamsCmd(FcObj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U], num_params);
                break;
            }
            case TIVX_VPAC_FC_MSC_CMD_SET_CROP_PARAMS:
            {
                status = tivxVpacFcMscScaleSetCropParamsCmd(FcObj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U], num_params);
                break;
            }
            case TIVX_VPAC_FC_VISS_CMD_SET_DCC_PARAMS:
            {
                /* Update Configuration in Driver */
                status = tivxMutexLock(FcObj->config_lock);
                if((vx_status)VX_SUCCESS == status)
                {
                    status = tivxVpacVissSetParamsFromDcc(&FcObj->vissObj,
                        (tivx_obj_desc_user_data_object_t *)obj_desc[0U], NULL, NULL);
                    (void)tivxMutexUnlock(FcObj->config_lock);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
                }
                break;
            }
            case TIVX_VPAC_FC_DELETE_GRAPH:
            {
                /* Delete Flex-connect path */
                status = Fvid2_control(FcObj->handle, IOCTL_VHWA_FC_DELETE_GRAPH,
                                        NULL, NULL);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to Delete Flex-connect Path\n");
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }            
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacFcObj *tivxVpacFcAllocObject(tivxVpacFcInstObj *instObj)
{
    uint32_t         cnt;
    tivxVpacFcObj *fcObj = NULL;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {
        for (cnt = 0U; cnt < VHWA_FC_DRV_MAX_HANDLES; cnt ++)
        {
            if (0U == instObj->fcObj[cnt].isAlloc)
            {
                fcObj = &instObj->fcObj[cnt];
                (void)memset(fcObj, 0, sizeof(tivxVpacFcObj));
                instObj->fcObj[cnt].isAlloc = 1U;

                #ifdef SOC_AM62A
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU1_0)
                #else
                if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU2_0)
                #endif
                {
                    instObj->fcObj[cnt].fc_drv_inst_id = VHWA_M2M_FC_DRV_INST0;
                }
                #if (VPAC_COUNT > 1)
                else if (self_cpu == (vx_enum)TIVX_CPU_ID_MCU4_0)
                {
                    instObj->fcObj[cnt].fc_drv_inst_id = VHWA_M2M_VPAC_1_FC_DRV_INST_ID_0;
                }
                #endif
                else
                {
                    /* Unsupported CPU ID */
                    VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                    fcObj = NULL;
                }
                break;
            }
        }
        /* Release instance mutex */
        (void)tivxMutexUnlock(instObj->lock);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }

    return (fcObj);
}

static void tivxVpacFcFreeObject(tivxVpacFcInstObj *instObj, tivxVpacFcObj *fcObj)
{
    uint32_t cnt;
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {

        for (cnt = 0U; cnt < VHWA_FC_DRV_MAX_HANDLES; cnt ++)
        {
            if (fcObj == &instObj->fcObj[cnt])
            {
                fcObj->isAlloc = 0U;
                break;
            }
        }
        /* Release instance mutex */
        (void)tivxMutexUnlock(instObj->lock);
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }
}
static vx_status tivxVpacFcCheckInputDesc(uint16_t num_params, tivx_obj_desc_t *obj_desc[])
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    uint32_t out_start;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    If the control reaches here, our code base is expected to print the error. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (num_params != TIVX_KERNEL_VPAC_FC_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Num params incorrect, = %d\n", num_params);
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == obj_desc[TIVX_KERNEL_VPAC_FC_CONFIGURATION_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_FC_VISS_RAW_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;

        if (NULL == obj_desc[TIVX_KERNEL_VPAC_FC_CONFIGURATION_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR, "Configuration is NULL\n");
        }
        if (NULL == obj_desc[TIVX_KERNEL_VPAC_FC_VISS_RAW_IDX])
        {
            VX_PRINT(VX_ZONE_ERROR, "Raw input is NULL\n");
        }
    }
    /* LDRA_JUSTIFY_END */
    else /* At least one output must be enabled */
    {
        out_start = TIVX_KERNEL_VPAC_FC_MSC_SCALE_OUT0_IMG_IDX;
        for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
        {
            if (NULL != obj_desc[out_start])
            {
                break;
            }
            out_start ++;
        }

        if (cnt >= TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT)
        {
            VX_PRINT(VX_ZONE_ERROR, "Atleast one output must be enabled\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }

    return (status);
}

static vx_status tivxVpacFcMapUserDesc(void **target_ptr, tivx_obj_desc_user_data_object_t *desc, uint32_t size)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> branch<metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (desc->mem_size == size)
    /* LDRA_JUSTIFY_END */
    {
        *target_ptr = tivxMemShared2TargetPtr(&desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(*target_ptr, desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Incorrect descriptor\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

static vx_status tivxVpacFcUnmapUserDesc(void **target_ptr, tivx_obj_desc_user_data_object_t *desc)
{
    vx_status status = (vx_status)VX_SUCCESS;

    tivxCheckStatus(&status, tivxMemBufferUnmap(*target_ptr, desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
        (vx_enum)VX_READ_ONLY));

    *target_ptr = NULL;

    return status;
}

static vx_status tivxVpacFcVissMscSetParams(tivxVpacVissObj *vissObj, tivxVpacMscScaleObj *mscObj, const tivx_vpac_viss_params_t *tivxVissPrms,
    const tivx_vpac_fc_viss_msc_params_t *fcPrms, const tivx_obj_desc_raw_image_t *raw_img_desc, tivx_obj_desc_image_t *img_desc_fc_out[])
{
    vx_status                 status = (vx_status)VX_SUCCESS;
    uint32_t                  cnt;
    uint32_t                  out_cnt;
    uint32_t                  msc_inst;
    uint32_t                  vxFmt = (uint32_t)VX_DF_IMAGE_VIRT;
    uint32_t                  mux_val[TIVX_KERNEL_VPAC_VISS_MAX_IMAGE_OUTPUT];
    Vhwa_M2mVissOutputParams *outPrms = NULL;
    Vhwa_M2mVissParams       *vissDrvPrms = NULL;
    Vhwa_M2mMscParams        *msc_prms = NULL;
    Msc_ScConfig             *scCfg = NULL;
    uint32_t                  plane_count = 0U;

    status = tivxVpacVissMapMuxVal(mux_val, tivxVissPrms);

    vissDrvPrms = &vissObj->vissPrms;
    msc_prms    = &mscObj->msc_prms;

    for(cnt = 0 ; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch<metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        if(NULL != img_desc_fc_out[cnt])
        /* LDRA_JUSTIFY_END */
        {
            vxFmt = (uint32_t)img_desc_fc_out[cnt]->format;
            break;
        }
    }

    /* Disable all Outputs first */
    for (out_cnt = 0u; out_cnt < TIVX_KERNEL_VPAC_FC_VISS_MAX_IMAGE_OUTPUT; out_cnt ++)
    {
        vissDrvPrms->outPrms[out_cnt].enable = UFALSE;
    }

    for (msc_inst = 0U; msc_inst < TIVX_KERNEL_VPAC_FC_MAX_MSC_INPUT_THREADS; msc_inst ++)
    {
        for (out_cnt = 0u; out_cnt < TIVX_KERNEL_VPAC_FC_VISS_MAX_IMAGE_OUTPUT; out_cnt ++)
        {

            if (out_cnt == fcPrms->msc_in_thread_viss_out_map[msc_inst] )
            {
                outPrms = &vissDrvPrms->outPrms[out_cnt];
                                
                status = tivxVpacFcVissMscMapFormat(
                &outPrms->fmt, &msc_prms->inFmt, out_cnt,
                mux_val[out_cnt], vxFmt, &plane_count);
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    outPrms->enable = UTRUE;
                    outPrms->fmt.width = raw_img_desc->params.width;
                    outPrms->fmt.height = raw_img_desc->params.height;


                    for (cnt = 0u; cnt < TIVX_IMAGE_MAX_PLANES; cnt ++)
                    {
                        if(FVID2_CCSF_BITS8_PACKED == outPrms->fmt.ccsFormat)
                        {
                            outPrms->fmt.pitch[cnt] = outPrms->fmt.width;
                        }
                        else
                        {
                            outPrms->fmt.pitch[cnt] = outPrms->fmt.width * 2U;
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to map format for output%d\n", out_cnt);
                }
                /* LDRA_JUSTIFY_END */
            }

            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
        }
    }

    msc_prms->inFmt.width    = raw_img_desc->params.width;
    msc_prms->inFmt.height   = raw_img_desc->params.height;
    if(FVID2_CCSF_BITS8_PACKED == msc_prms->inFmt.ccsFormat)
    {
        msc_prms->inFmt.pitch[0] = raw_img_desc->params.width;
        msc_prms->inFmt.pitch[1] = raw_img_desc->params.width;
    }
    else
    {
        msc_prms->inFmt.pitch[0] = raw_img_desc->params.width * 2U;
        msc_prms->inFmt.pitch[1] = raw_img_desc->params.width * 2U;
    }

    msc_prms->mscCfg.tapSel = MSC_TAP_SEL_5TAPS;

    if(((0x01U == plane_count) || (0x04U == plane_count)))
    {
        /* Set MSC input params to luma only */
        msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
        msc_prms->isEnableSimulProcessing = (uint32_t)false;
        if(0x01U == plane_count)
        {
            if (vissDrvPrms->outPrms[0U].enable == UTRUE)
            {
                vissDrvPrms->outPrms[0U].fmt.dataFormat = FVID2_DF_LUMA_ONLY;
            }
        }
        else
        {
            if (vissDrvPrms->outPrms[2U].enable == UTRUE)
            {
                vissDrvPrms->outPrms[2U].fmt.dataFormat = FVID2_DF_LUMA_ONLY;
            }
        }
    }
#if defined (VPAC3) || defined (VPAC3L)
    else if(((0x02U == plane_count) || (0x08U == plane_count)))
    {
        /* set msc input as chroma only */
        msc_prms->inFmt.dataFormat = FVID2_DF_CHROMA_ONLY;
        msc_prms->isEnableSimulProcessing = (uint32_t)false;
        if(0x02U == plane_count)
        {
            if (vissDrvPrms->outPrms[1U].enable == UTRUE)
            {
                vissDrvPrms->outPrms[1U].fmt.dataFormat = FVID2_DF_CHROMA_ONLY;
            }
        }
        else
        {
            if (vissDrvPrms->outPrms[3U].enable == UTRUE)
            {
                vissDrvPrms->outPrms[3U].fmt.dataFormat = FVID2_DF_CHROMA_ONLY;
            }
        }
    }
    else if((((0x03U == plane_count) || (0x0CU == plane_count)) && (TIVX_VPAC_VISS_CHROMA_MODE_420 == tivxVissPrms->fcp[0].chroma_mode)))
    {
        /* set msc input as yuv420 */
        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
        msc_prms->isEnableSimulProcessing = (uint32_t)true;

        if(0x03U == plane_count)
        {
            if (vissDrvPrms->outPrms[0U].enable == UTRUE)
            {
                vissDrvPrms->outPrms[0U].fmt.dataFormat = FVID2_DF_YUV420SP_UV;
                vissDrvPrms->outPrms[1U].enable = UFALSE;
            }
        }
        else
        {
            if (vissDrvPrms->outPrms[2U].enable == UTRUE)
            {
                vissDrvPrms->outPrms[2U].fmt.dataFormat = FVID2_DF_YUV420SP_UV;
                vissDrvPrms->outPrms[3U].enable = UFALSE;
            }
        }
    }
    else if((((0x03U == plane_count) || (0x0CU == plane_count)) && (TIVX_VPAC_VISS_CHROMA_MODE_422 == tivxVissPrms->fcp[0].chroma_mode)))
    {
        /* set msc input as yuv422 */
        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_UV;
        msc_prms->isEnableSimulProcessing = (uint32_t)true;
        if(0x03U == plane_count)
        {
            if (vissDrvPrms->outPrms[0U].enable == UTRUE)
            {
                vissDrvPrms->outPrms[0U].fmt.dataFormat = FVID2_DF_YUV422SP_UV;
                vissDrvPrms->outPrms[1U].enable = UFALSE;
            }
        }
    }
#endif
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid MSC input Format\n");
        status = (vx_status)VX_FAILURE;
    }

    /* Map MSC output params and Scalar config  */
    for (cnt = 0U; cnt < TIVX_KERNEL_VPAC_FC_MAX_IMAGE_OUTPUT; cnt++)
    {
        scCfg = &msc_prms->mscCfg.scCfg[cnt];

        if (NULL != img_desc_fc_out[cnt])
        {
            scCfg->enable = UTRUE;

            /* Set MSC output format based on FC output */
            msc_prms->outFmt[cnt].width = img_desc_fc_out[cnt]->width;
            scCfg->outWidth = msc_prms->outFmt[cnt].width;

            msc_prms->outFmt[cnt].height = img_desc_fc_out[cnt]->height;
            scCfg->outHeight = msc_prms->outFmt[cnt].height;

            msc_prms->outFmt[cnt].pitch[0] = img_desc_fc_out[cnt]->width;
            msc_prms->outFmt[cnt].pitch[1] = img_desc_fc_out[cnt]->width;

            if(((cnt % 2U) == 0U))
            {
                msc_prms->outFmt[cnt].dataFormat = FVID2_DF_LUMA_ONLY;
                scCfg->inRoi.cropHeight = msc_prms->inFmt.height;
            }
    #if defined (VPAC3) || defined (VPAC3L)
            else
            {
                msc_prms->outFmt[cnt].dataFormat = FVID2_DF_CHROMA_ONLY;
                scCfg->inRoi.cropHeight = msc_prms->inFmt.height / 2U;

            }
    #endif
            msc_prms->outFmt[cnt].ccsFormat = msc_prms->inFmt.ccsFormat;

            scCfg->inRoi.cropWidth = msc_prms->inFmt.width;
            
            tivxVpacFcMscScaleCopyOutPrmsToScCfg(scCfg, cnt);
        }
        else
        {
            scCfg->enable = UFALSE;
        }
    }

    /* H3A is one of the output for the Driver,
     * Enable it if required */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps. Since no error is accumulated due to previously stated rationales, this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        if ((vx_bool)vx_true_e == vissObj->h3a_out_enabled)
        {
            outPrms = &vissDrvPrms->outPrms[VHWA_M2M_VISS_OUT_H3A_IDX];
            outPrms->enable = (uint32_t)UTRUE;
            outPrms->fmt.dataFormat = FVID2_DF_RAW;
        }
    }

    return (status);
}

static vx_status tivxVpacFcVissMscMapFormat(Fvid2_Format *Vissfmt, Fvid2_Format *Mscfmt, uint32_t out_start, uint32_t mux_val, uint32_t vxFmt, uint32_t *plane_count)
{
    vx_status status = (vx_status)VX_SUCCESS;

    uint32_t ccsFormat = 0U;

    switch(vxFmt)
    {
        case (uint32_t)VX_DF_IMAGE_U16:
            ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            break;
        case (uint32_t)VX_DF_IMAGE_U8:
            ccsFormat = FVID2_CCSF_BITS8_PACKED;
            break;
        case (uint32_t)TIVX_DF_IMAGE_P12:
            ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        case (uint32_t)VX_DF_IMAGE_NV12:
            ccsFormat = FVID2_CCSF_BITS8_PACKED;
            break;
        case (uint32_t)TIVX_DF_IMAGE_NV12_P12:
            ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        case (uint32_t)VX_DF_IMAGE_UYVY:
            ccsFormat = FVID2_CCSF_BITS8_PACKED;
            break;
        case (uint32_t)VX_DF_IMAGE_YUYV:
            ccsFormat = FVID2_CCSF_BITS8_PACKED;
            break;
        /* LDRA_JUSTIFY_START
        <metric start> statement branch<metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        default :
            VX_PRINT(VX_ZONE_ERROR, "Invalid Format %d\n", vxFmt);
            status = (vx_status)VX_FAILURE;
            break;
        /* LDRA_JUSTIFY_END */
    }

    switch(out_start)
    {
        case 0U:
                switch (mux_val)
                {
                case TIVX_VPAC_VISS_MUX0_Y12:
                    *plane_count |= 0x01U;
                    break;
                /* LDRA_JUSTIFY_START
                <metric start> statement<metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Mux Value %d\n", out_start);
                    status = (vx_status)VX_FAILURE;
                    break;
                /* LDRA_JUSTIFY_END */
                }
            break;
    #if defined (VPAC3) || defined (VPAC3L)
        case 1U:
                switch (mux_val)
                {
                case TIVX_VPAC_VISS_MUX1_UV12:
                    *plane_count |= 0x02U;
                    break;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch<metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Mux Value %d\n", out_start);
                    status = (vx_status)VX_FAILURE;
                    break;
                /* LDRA_JUSTIFY_END */
                }
            break;
    #endif
        case 2U:
                switch (mux_val)
                {
                case TIVX_VPAC_VISS_MUX2_Y8:
                    *plane_count |= 0x04U;
                    break;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch<metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */                                  
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Mux Value %d\n", out_start);
                    status = (vx_status)VX_FAILURE;
                    break;
                /* LDRA_JUSTIFY_END */
                }
            break;
    #if defined (VPAC3) || defined (VPAC3L)
        case 3U:
                switch (mux_val)
                {
                case TIVX_VPAC_VISS_MUX3_UV8:
                    *plane_count |= 0x08U;
                    break;
                /* LDRA_JUSTIFY_START
                <metric start> statement branch<metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The test framework does not support the configuration required to trigger this error scenario.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Mux Value %d\n", out_start);
                    status = (vx_status)VX_FAILURE;
                    break;
                /* LDRA_JUSTIFY_END */
                }
            break;
    #endif
        /* LDRA_JUSTIFY_START
        <metric start> statement branch<metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
            VX_PRINT(VX_ZONE_ERROR, "Invalid Output Index %d\n", out_start);
            status = (vx_status)VX_FAILURE;
            break;
        /* LDRA_JUSTIFY_END */
    }

    Vissfmt->ccsFormat  = ccsFormat;
    Mscfmt->ccsFormat   = ccsFormat;

    return status;
}

static void tivxVpacFcMscScaleCopyOutPrmsToScCfg(Msc_ScConfig *scCfg, uint32_t cnt)
{
    scCfg->inRoi.cropStartX = 0U;
    scCfg->inRoi.cropStartY = 0U;
    scCfg->horzAccInit = 0U;
    scCfg->vertAccInit = 0U;
    scCfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;
    scCfg->phaseMode = MSC_PHASE_MODE_32PHASE;
    scCfg->hsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;
    scCfg->vsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;

    scCfg->coeffShift = MSC_COEFF_SHIFT_8;
    scCfg->isSignedData = UFALSE;
    scCfg->isEnableFiltSatMode = UFALSE;
    
    switch(cnt)
    {
        case 0U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            break;
        case 1U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            break;
        case 2U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            break;
        case 3U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            break;
        case 4U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_2;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            break;
        case 5U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_2;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            break;
        case 6U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            break;
        case 7U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            break;
        case 8U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            break;
        case 9U:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            break;
        default:
            scCfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            scCfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
            break;
    }

}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacFcFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacFcObj *fcObj = (tivxVpacFcObj *)appData;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != fcObj)
    /* LDRA_JUSTIFY_END */
    {
        (void)tivxEventPost(fcObj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}
