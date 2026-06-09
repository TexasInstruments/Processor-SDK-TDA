/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#include "tivx_hwa_vpac_msc_priv.h"
#include <vx_vpac_multi_scale_output_target_priv.h>
#include <vhwa/include/vhwa_m2mMsc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* OPENVX Callback functions */
static vx_status VX_CALLBACK tivxVpacMscScaleProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscScaleCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscScaleDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscScaleControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static vx_status tivxVpacMscScaleSetCoeffsCmd(tivxVpacMscScaleObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscScaleSetInputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscScaleSetOutputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[], uint16_t num_params);
static vx_status tivxVpacMscScaleSetCropParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[], uint16_t num_params);
static vx_status tivxVpacMscMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxVpacMscMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);
static vx_status tivxVpacMscGetErrStatusCmd(const tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxVpacMscGetPsaStatusCmd(const tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscMultiScaleEnableErrorEventsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);

/* Local Functions */
static tivxVpacMscScaleObj *tivxVpacMscScaleAllocObject(
    tivxVpacMscScaleInstObj *instObj);
static void tivxVpacMscScaleFreeObject(tivxVpacMscScaleInstObj *instObj,
    tivxVpacMscScaleObj *msc_obj);
/* Driver Callback */
int32_t tivxVpacMscMultiScaleFrameComplCb(Fvid2_Handle handle, void *appData);
static void tivxVpacMscMultiScaleErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxVpacMscMultiScaleWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);

/* Config register readback callback */
#if !defined(VPAC3L)
int32_t tivxVpacMscScaleConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms);

/* Buffer allocation/free functions */
static vx_status tivxVpacMscScaleAllocReadbackBuffers(tivxVpacMscScaleObj *mscObj);
static void tivxVpacMscScaleFreeReadbackBuffers(tivxVpacMscScaleObj *mscObj);
static vx_status tivxEnableVpacMscSafetyMechanisms(
    tivxVpacMscScaleObj *mscObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
#endif

/* Function prototype for link skip function */
uint32_t tivxVpacMscScaleDoLinkSkip(tivx_obj_desc_image_t *in_img_desc, tivx_obj_desc_image_t *out_img_desc[], uint32_t num_outputs);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

tivxVpacMscScaleInstObj gTivxVpacMscMScaleInstObj[TIVX_VPAC_MSC_NUM_INST];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacMscMultiScale(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    tivxVpacMscScaleInstObj *inst_obj;
    uint32_t                inst_start;

    inst_start = TIVX_VPAC_MSC_M_SCALE_START_IDX;
    self_cpu = tivxGetSelfCpuId();

    /* Reset all values to 0 */
    memset(&gTivxVpacMscMScaleInstObj[inst_start], 0x0,
        sizeof(tivxVpacMscScaleInstObj) * VHWA_M2M_MSC_MAX_INST);

    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        if(status != (vx_status)VX_SUCCESS)
        {
            break;
        }
        inst_obj = &gTivxVpacMscMScaleInstObj[inst_start + cnt];

        if (0u == cnt)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
            /* LDRA_JUSTIFY_END */

            {
                strncpy(target_name, TIVX_TARGET_VPAC_MSC1,
                    TIVX_TARGET_MAX_NAME);
            }
            #if (VPAC_COUNT > 1)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */ 
            else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
            /* LDRA_JUSTIFY_END */
            {
                strncpy(target_name, TIVX_TARGET_VPAC2_MSC1,
                    TIVX_TARGET_MAX_NAME);
            }
            #endif
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Unsupported CPU ID */
                VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }
        else
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
            /* LDRA_JUSTIFY_END */
            {
                strncpy(target_name, TIVX_TARGET_VPAC_MSC2,
                    TIVX_TARGET_MAX_NAME);
            }
            #if (VPAC_COUNT > 1)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
            /* LDRA_JUSTIFY_END */
            {
                strncpy(target_name, TIVX_TARGET_VPAC2_MSC2,
                    TIVX_TARGET_MAX_NAME);
            }
            #endif
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Unsupported CPU ID */
                VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }

        if((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = tivxAddTargetKernelByName(
                                TIVX_KERNEL_VPAC_MSC_MULTI_SCALE_NAME,
                                target_name,
                                tivxVpacMscScaleProcess,
                                tivxVpacMscScaleCreate,
                                tivxVpacMscScaleDelete,
                                tivxVpacMscScaleControl,
                                NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != inst_obj->target_kernel)
            /* LDRA_JUSTIFY_END */

            {
                /* Allocate lock mutex */ 
                status = tivxMutexCreate(&inst_obj->lock);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
                <justification end> */ 
                if ((vx_status)VX_SUCCESS != status)
                {
                    (void)tivxRemoveTargetKernel(inst_obj->target_kernel);
                    inst_obj->target_kernel = NULL;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to create Mutex\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    /* Initialize Instance Object */
                    if (0u == cnt)
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                            However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
                        /* LDRA_JUSTIFY_END */

                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC0;
                        }
                        #if (VPAC_COUNT > 1)
                        /* LDRA_JUSTIFY_START
                        <metric start>  branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                            However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
                        /* LDRA_JUSTIFY_END */
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC0;
                        }
                        #endif
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else
                        {
                            /* Unsupported CPU ID */
                            VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                            status = (vx_status)VX_FAILURE;
                        }
                        /* LDRA_JUSTIFY_END */
                        inst_obj->alloc_sc_fwd_dir = 1U;
                    }
                    else
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                            However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
                        /* LDRA_JUSTIFY_END */
                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC1;
                        }
                        #if (VPAC_COUNT > 1)
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                            However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
                        /* LDRA_JUSTIFY_END */
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC1;
                        }
                        #endif
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else
                        {
                            /* Unsupported CPU ID */
                            VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                            status = (vx_status)VX_FAILURE;
                        }
                        /* LDRA_JUSTIFY_END */
                        inst_obj->alloc_sc_fwd_dir = 0U;
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
                status = (vx_status)VX_FAILURE;

                /* TODO: how to handle this condition */
                VX_PRINT(VX_ZONE_ERROR, "Failed to Add MSC TargetKernel\n");
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Clean up allocated resources */
    if ((vx_status)VX_SUCCESS != status)
    {
        for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
        {
            inst_obj = &gTivxVpacMscMScaleInstObj[cnt];
            if (inst_obj->target_kernel != NULL)
            {
                (void)tivxRemoveTargetKernel(inst_obj->target_kernel);
            }
            if (inst_obj->lock != NULL)
            {
                (void)tivxMutexDelete(&inst_obj->lock);
            }
        }
    }
    /* LDRA_JUSTIFY_END */
}
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelVpacMscMultiScale(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    tivxVpacMscScaleInstObj *inst_obj;
    uint32_t                inst_start;

    inst_start = TIVX_VPAC_MSC_M_SCALE_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscMScaleInstObj[inst_start + cnt];

        status = tivxRemoveTargetKernel(inst_obj->target_kernel);
        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = NULL;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Msc TargetKernel\n");
        }

        if (NULL != inst_obj->lock)
        {
            (void)tivxMutexDelete(&inst_obj->lock);
        }
    }
}
/* LDRA_JUSTIFY_END */


void tivxAddTargetKernelVpacMscMultiOut(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;
    tivxVpacMscScaleInstObj *inst_obj;
    uint32_t                inst_start;

    inst_start = TIVX_VPAC_MSC_M_SCALE2_START_IDX;
    self_cpu = tivxGetSelfCpuId();

    /* Reset all values to 0 */
    memset(&gTivxVpacMscMScaleInstObj[inst_start], 0x0,
        sizeof(tivxVpacMscScaleInstObj) * VHWA_M2M_MSC_MAX_INST);

    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        if(status != (vx_status)VX_SUCCESS)
        {
            break;
        }
        inst_obj = &gTivxVpacMscMScaleInstObj[inst_start + cnt];

        if (0u == cnt)
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
            /* LDRA_JUSTIFY_END */
            {
                strncpy(target_name, TIVX_TARGET_VPAC_MSC1,
                    TIVX_TARGET_MAX_NAME);
            }
            #if (VPAC_COUNT > 1)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
            /* LDRA_JUSTIFY_END */
            {
                strncpy(target_name, TIVX_TARGET_VPAC2_MSC1,
                    TIVX_TARGET_MAX_NAME);
            }
            #endif
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Unsupported CPU ID */
                VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }
        else
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
            /* LDRA_JUSTIFY_END */

            {
                strncpy(target_name, TIVX_TARGET_VPAC_MSC2,
                    TIVX_TARGET_MAX_NAME);
            }
            #if (VPAC_COUNT > 1)
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
            /* LDRA_JUSTIFY_END */
            {
                strncpy(target_name, TIVX_TARGET_VPAC2_MSC2,
                    TIVX_TARGET_MAX_NAME);
            }
            #endif
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Unsupported CPU ID */
                VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }

        if((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = tivxAddTargetKernelByName(
                                TIVX_KERNEL_VPAC_MSC_MULTI_SCALE2_NAME,
                                target_name,
                                tivxVpacMscScaleProcess,
                                tivxVpacMscScaleCreate,
                                tivxVpacMscScaleDelete,
                                tivxVpacMscScaleControl,
                                NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != inst_obj->target_kernel)
            /* LDRA_JUSTIFY_END */            
            {
                /* Allocate lock mutex */

                status = tivxMutexCreate(&inst_obj->lock);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    (void)tivxRemoveTargetKernel(inst_obj->target_kernel);
                    inst_obj->target_kernel = NULL;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to create Mutex\n");
                }
                else
                /* LDRA_JUSTIFY_END */

                {
                    /* Initialize Instance Object */
                    if (0u == cnt)
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
                        /* LDRA_JUSTIFY_END */
                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC0;
                        }
                        #if (VPAC_COUNT > 1)
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
                        /* LDRA_JUSTIFY_END */
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_0;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC0;
                        }
                        #endif
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else
                        {
                            /* Unsupported CPU ID */
                            VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                            status = (vx_status)VX_FAILURE;
                        }
                        /* LDRA_JUSTIFY_END */
                        inst_obj->alloc_sc_fwd_dir = 1U;
                    }
                    else
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
                        /* LDRA_JUSTIFY_END */

                        {
                            inst_obj->msc_drv_inst_id = VPAC_MSC_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC1_MSC1;
                        }
                        #if (VPAC_COUNT > 1)
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
                        /* LDRA_JUSTIFY_END */
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC1;
                        }
                        #endif
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The CPU ID is validated by the framework and only supported CPU IDs (TIVX_CPU_ID_VPAC1, TIVX_CPU_ID_VPAC2) are assigned to this target.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to log the error message for unsupported CPU ID.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else
                        {
                            /* Unsupported CPU ID */
                            VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                            status = (vx_status)VX_FAILURE;
                        }
                        /* LDRA_JUSTIFY_END */
                        inst_obj->alloc_sc_fwd_dir = 0U;
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
                status = (vx_status)VX_FAILURE;

                /* TODO: how to handle this condition */
                VX_PRINT(VX_ZONE_ERROR, "Failed to Add MSC TargetKernel\n");
            }
            /* LDRA_JUSTIFY_END */           
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    /* Clean up allocated resources */
    if ((vx_status)VX_SUCCESS != status)
    {
        for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
        {
            inst_obj = &gTivxVpacMscMScaleInstObj[cnt];
            if (inst_obj->target_kernel != NULL)
            {
                (void)tivxRemoveTargetKernel(inst_obj->target_kernel);
            }
            if (inst_obj->lock != NULL)
            {
                (void)tivxMutexDelete(&inst_obj->lock);
            }
        }
    }
    /* LDRA_JUSTIFY_END */           
}
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelVpacMscMultiOut(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t cnt;
    tivxVpacMscScaleInstObj *inst_obj;
    uint32_t                inst_start;

    inst_start = TIVX_VPAC_MSC_M_SCALE2_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscMScaleInstObj[inst_start + cnt];

        status = tivxRemoveTargetKernel(inst_obj->target_kernel);
        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = NULL;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Msc TargetKernel\n");
        }

        if (NULL != inst_obj->lock)
        {
            (void)tivxMutexDelete(&inst_obj->lock);
        }
    }
}
/* LDRA_JUSTIFY_END */           


/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support. 
This API skips lines in input to improve the performance but this affects output quality 
Effect on this unit: There is no impact, as the feature is unused, control cannot reach this condition.
<justification end> */
uint32_t tivxVpacMscScaleDoLinkSkip(tivx_obj_desc_image_t *in_img_desc, tivx_obj_desc_image_t *out_img_desc[], uint32_t num_outputs)
{
    uint32_t in_h, cnt;
    uint32_t out_max_h = 0;
    uint32_t do_line_skip = 0;

    in_h = in_img_desc->imagepatch_addr[0].dim_y;

    for (cnt = 0u; cnt < num_outputs; cnt ++)
    {
        if (NULL != out_img_desc[cnt])
        {
            if(out_img_desc[cnt]->imagepatch_addr[0].dim_y > out_max_h)
            {
                out_max_h = out_img_desc[cnt]->imagepatch_addr[0].dim_y;
            }
        }
    }

    /* if downscale ratio is more than 2.5x then skip alternate lines */
    if( ((out_max_h*5U)/2U) < in_h )
    {
        do_line_skip = 1U;
    }
    return do_line_skip;
}
/* LDRA_JUSTIFY_END */           

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacMscScaleCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    int32_t                  fvid2_status = FVID2_SOK;
    vx_uint32                cnt;
    Vhwa_M2mMscParams       *msc_prms = NULL;
    tivxVpacMscScaleObj     *msc_obj = NULL;
    tivx_obj_desc_image_t   *in_img_desc0 = NULL;
    tivx_obj_desc_image_t   *in_img_desc1 = NULL;
    tivx_obj_desc_image_t   *out_img_desc
        [MSC_MAX_OUTPUT] = {NULL};
    tivxVpacMscScaleInstObj *inst_obj = NULL;
    tivx_target_kernel       target_kernel = NULL;
    /* LDRA_JUSTIFY_START
    <metric start>  statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN0_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX]))) &&
        ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_IN0_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT0_IMG_IDX]))))
    {
        status = (vx_status)VX_FAILURE;
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
        target_kernel = tivxTargetKernelInstanceGetKernel(kernel);
        /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
        if (NULL == target_kernel)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get Target Kernel\n");
            status = (vx_status)VX_ERROR_INVALID_NODE;
        }
        else
        /* LDRA_JUSTIFY_END */           
        {
            inst_obj = NULL;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            for (cnt = 0u; cnt < TIVX_VPAC_MSC_NUM_INST; cnt ++)
            /* LDRA_JUSTIFY_END */           
            {
                if (target_kernel == gTivxVpacMscMScaleInstObj[cnt].target_kernel)
                {
                    inst_obj = &gTivxVpacMscMScaleInstObj[cnt];
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
            if (NULL == inst_obj)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Target Kernel\n");
                status = (vx_status)VX_ERROR_NOT_SUPPORTED;
            }
            /* LDRA_JUSTIFY_END */           

        }
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
        msc_obj = tivxVpacMscScaleAllocObject(inst_obj);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != msc_obj)
        /* LDRA_JUSTIFY_END */           

        {
            in_img_desc0 = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN0_IMG_IDX];
            if(num_params == TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS)
            {
                in_img_desc0 = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_IN0_IMG_IDX];
                in_img_desc1 = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_IN1_IMG_IDX];
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != in_img_desc0)
            /* LDRA_JUSTIFY_END */                     
            {
                msc_obj->in_img0_format = in_img_desc0->format;
            }
            msc_obj->in_img_desc0 = in_img_desc0;
            msc_obj->num_params = num_params;
            
            if(NULL == in_img_desc1)
            {
                for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
                {                    
                    out_img_desc[cnt] = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];                    
                }
            }
            else
            {
                for (cnt = 0u; cnt < (TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT/2U); cnt++)
                {
                    out_img_desc[(2U*cnt)] = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT0_IMG_IDX + cnt];
                    out_img_desc[(2U*cnt)+1U] = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT5_IMG_IDX + cnt];
                }
            }
            for (cnt = 0u; cnt < MSC_MAX_OUTPUT; cnt ++)
            {
                msc_obj->out_img_format[cnt] = out_img_desc[cnt]->format;
                msc_obj->out_img_desc[cnt] = out_img_desc[cnt];
            }

            /* Initialize Msc object */
            msc_obj->inst_obj = inst_obj;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(NULL != in_img_desc0)
            /* LDRA_JUSTIFY_END */           
            {
                msc_obj->in0_height = in_img_desc0->imagepatch_addr[0].dim_y;
            }
            msc_obj->yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
            msc_obj->enable_error_events = 0u;
            #if defined(VPAC3) || defined(VPAC3L)
            msc_obj->in_img0_yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
            msc_obj->in_img1_yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;
            if(in_img_desc1 != NULL)
            {
                msc_obj->in1_height = in_img_desc1->imagepatch_addr[0].dim_y;
                msc_obj->in_img1_format  = in_img_desc1->format;
                msc_obj->in_img_desc1 = in_img_desc1;
            }
            if((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->out_img_desc[0u]->format)
            {
                msc_obj->msc_prms.isEnableSimulProcessing = 0;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if((NULL != in_img_desc0) && (NULL != in_img_desc1))
            /* LDRA_JUSTIFY_END */
            {
                msc_obj->msc_prms.isEnableSimulProcessing = 1;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Unsupported input/output formats are rejected during OpenVX graph verification stage before process execution.
            The kernel validator function ensures only supported format combinations reach this code.
            Effect on this unit: This defensive else block is architecturally unreachable due to framework format validation.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else
            {
                /* Unsupported configuration */
            }
            /* LDRA_JUSTIFY_END */
            #endif
            #if defined VPAC1
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(NULL != in_img_desc1)
            {
                VX_PRINT(VX_ZONE_ERROR, "input1 should be NULL for VPAC1\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            /* LDRA_JUSTIFY_END */
            #endif
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Handle Object, increase VHWA_M2M_MSC_MAX_HANDLES macro in PDK driver\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
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
            Vhwa_M2mMscCreatePrmsInit(&msc_obj->createArgs);

            status = tivxEventCreate(&msc_obj->wait_for_compl);
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
                msc_obj->cbPrms.cbFxn   = tivxVpacMscMultiScaleFrameComplCb;
                msc_obj->cbPrms.appData = msc_obj;

                msc_obj->handle = Fvid2_create(FVID2_VHWA_M2M_MSC_DRV_ID,
                    inst_obj->msc_drv_inst_id, &msc_obj->createArgs,
                    NULL, &msc_obj->cbPrms);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL == msc_obj->handle)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Fvid2_create failed\n");
                    status = (vx_status)VX_ERROR_NO_RESOURCES;
                }
                else
                /* LDRA_JUSTIFY_END */ 

                {
                    Fvid2Frame_init(&msc_obj->inFrm);
                    for (cnt = 0u; cnt < MSC_MAX_OUTPUT; cnt ++)
                    {
                        Fvid2Frame_init(&msc_obj->outFrm[cnt]);
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
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
            msc_obj->errEvtPrms.errEvents =
            VHWA_MSC_VBUSM_RD_ERR | VHWA_MSC_SL2_WR_ERR;
            msc_obj->errEvtPrms.cbFxn     = tivxVpacMscMultiScaleErrorCb;
            msc_obj->errEvtPrms.appData   = msc_obj;

            fvid2_status = Fvid2_control(msc_obj->handle,
                VHWA_M2M_IOCTL_MSC_REGISTER_ERR_CB, &msc_obj->errEvtPrms, NULL);
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
                VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Register Error Callback\n");
                status = (vx_status)VX_FAILURE;
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
            msc_obj->wdTimerErrEvtPrms.wdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
            /* Disable Watchdog Timer Error Events by default */
            msc_obj->wdTimerErrEvtPrms.wdTimerErrEvents = 0u;
            msc_obj->wdTimerErrEvtPrms.cbFxn     = tivxVpacMscMultiScaleWdTimerErrorCb;
            msc_obj->wdTimerErrEvtPrms.appData   = msc_obj;

            fvid2_status = Fvid2_control(msc_obj->handle,
                VHWA_M2M_IOCTL_MSC_REGISTER_WDTIMER_ERR_CB, &msc_obj->wdTimerErrEvtPrms, NULL);
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
                VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Register Watchdog timer Error Callback\n");
                status = (vx_status)VX_FAILURE;
            }
        /* LDRA_JUSTIFY_END */           

        }
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
        uint32_t do_line_skip = 0;

        /* not enabling this as of now, this skips lines in input to improve the performance but this
         * affects output quality */
        /* do_line_skip = tivxVpacMscScaleDoLinkSkip(in_img_desc, out_img_desc, TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT); */

        msc_prms = &msc_obj->msc_prms;

        Vhwa_m2mMscParamsInit(msc_prms);

        tivxVpacMscMultiScaleSetFmt(&msc_prms->inFmt, in_img_desc0, do_line_skip);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL != in_img_desc0)
        /* LDRA_JUSTIFY_END */           

        {
            if (((vx_df_image)VX_DF_IMAGE_NV12 != out_img_desc[0u]->format) &&
            ((vx_df_image)VX_DF_IMAGE_UYVY != in_img_desc0->format) &&
            ((vx_df_image)VX_DF_IMAGE_YUYV != in_img_desc0->format))
            {
            /* Luma-only mode when the input format is NV12,
                * but the output format is not NV12.
                * Also, luma-only mode if the input format is not YUV422I (UYVY or YUYV) */
                msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
            }
        }
        /* set sclar parameters and output format for each level*/
        tivxVpacMscScaleSetScParamsForEachLevel(msc_obj,msc_prms);

        fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_SET_PARAMS,
            msc_prms, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */           
        {
            /* Set up Frame List */
            msc_obj->inFrmList.frames[0u] = &msc_obj->inFrm;

            for (cnt = 0u; cnt < MSC_MAX_OUTPUT; cnt ++)
            {
                msc_obj->outFrmList.frames[cnt] = &msc_obj->outFrm[cnt];
            }
        }
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
        status = tivxSetTargetKernelInstanceContext(kernel, msc_obj,
            sizeof(tivxVpacMscScaleObj));
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS != status)
    {
        if (NULL != msc_obj)
        {
            if (NULL != msc_obj->handle)
            {
                (void)Fvid2_delete(msc_obj->handle, NULL);
                msc_obj->handle = NULL;
            }

            if (NULL != msc_obj->wait_for_compl)
            {
                (void)tivxEventDelete(&msc_obj->wait_for_compl);
            }

            tivxVpacMscScaleFreeObject(inst_obj, msc_obj);
        }
    }
    /* LDRA_JUSTIFY_END */           
    return status;
}

static vx_status VX_CALLBACK tivxVpacMscScaleDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxVpacMscScaleObj *msc_obj = NULL;
    tivxVpacMscScaleInstObj *inst_obj = NULL;
    /* LDRA_JUSTIFY_START
    <metric start>  statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN0_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX]))) &&
        ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_IN0_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT0_IMG_IDX]))))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
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
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The kernel parameter is expected to be pre-validated by the OpenVX framework before kernel callback invocation.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, msc_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            msc_obj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */ 
        if ((((vx_status)VX_SUCCESS == status) && (NULL != msc_obj)) && (sizeof(tivxVpacMscScaleObj) == size))
        /* LDRA_JUSTIFY_END */           
        {
            inst_obj = msc_obj->inst_obj;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if (NULL != msc_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(msc_obj->handle, NULL);
                msc_obj->handle = NULL;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != msc_obj->wait_for_compl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&msc_obj->wait_for_compl);
            }

#if !defined(VPAC3L)
            /* Free config register readback buffers if allocated */
            if ((msc_obj->readback_mem_ptr_phys != 0u) || (msc_obj->golden_reg_mem_ptr_phys != 0u))
            {
                tivxVpacMscScaleFreeReadbackBuffers(msc_obj);
            }
#endif
            tivxVpacMscScaleFreeObject(inst_obj, msc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscScaleProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    int32_t                  fvid2_status = FVID2_SOK;
    uint32_t                 size;
    uint32_t                 cnt;
    uint32_t                 plane_cnt;
    uint32_t                 idx;
    Fvid2_Frame             *frm = NULL;
    tivx_obj_desc_image_t   *in_img_desc0 = NULL;
    tivx_obj_desc_image_t   *in_img_desc1 = NULL;
    tivx_obj_desc_image_t   *out_img_desc[MSC_MAX_OUTPUT];
    tivxVpacMscScaleObj     *msc_obj = NULL;
    Fvid2_FrameList         *inFrmList;
    Fvid2_FrameList         *outFrmList;
    Msc_ScConfig            *sc_cfg;
    uint64_t                cur_time;
    tivxVpacMscScaleInstObj *inst_obj = NULL;
    uint32_t                div_fact = 0;
    uint32_t                res = 0;
    tivx_obj_desc_t         *out_base_desc = NULL;
    #if !defined(VPAC3L)
    vx_status               validate_reg_status = (vx_status)VX_SUCCESS;
    #endif

    /* LDRA_JUSTIFY_START
    <metric start>  statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((num_params != TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN0_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX]))) &&
        ((num_params != TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_IN0_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT0_IMG_IDX]))))
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL Params check failed\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */           


    {
        in_img_desc0 = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_IN0_IMG_IDX];
        if(num_params == TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS)
        {
            in_img_desc0 = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_IN0_IMG_IDX];
            in_img_desc1 = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_IN1_IMG_IDX];
        }
        if(NULL == in_img_desc1)
        {
            for (cnt = 0u; cnt < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT; cnt ++)
            {                    
                out_img_desc[cnt] = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX + cnt];                    
            }
        }
        else
        {
            for (cnt = 0u; cnt < ((uint32_t)TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT/2U); cnt++)
            {
                out_img_desc[(2U*cnt)] = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT0_IMG_IDX + cnt];
                out_img_desc[((2U*cnt)+1U)] = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_MSC_SCALE2_OUT5_IMG_IDX + cnt];
            }
        }
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
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The kernel parameter is expected to be pre-validated by the OpenVX framework before kernel callback invocation.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, msc_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            msc_obj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((((vx_status)VX_SUCCESS == status) && (NULL != msc_obj)) && (sizeof(tivxVpacMscScaleObj) == size))
        /* LDRA_JUSTIFY_END */  
        {
            #if defined(VPAC3) || defined(VPAC3L)
            if(msc_obj->msc_prms.isEnableSimulProcessing == 1U)
            {
                if(num_params == TIVX_KERNEL_VPAC_MSC_SCALE_MAX_PARAMS)
                {
                    div_fact = 1;
                }
            }
            #endif
            for (cnt = 0u; cnt < msc_obj->num_outputs; cnt ++)
            {
                res = cnt >> div_fact;
                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> Rationale: The negative test framework and test apps cannot reach this portion. The current implementation enables sc_cfg only when out_img_desc is not NULL.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (((uint32_t)UTRUE == sc_cfg->enable) &&
                    (NULL == out_img_desc[res]))
                {
                    VX_PRINT(VX_ZONE_ERROR, "Null Descriptor for Enabled Optional Output\n");
                    status = (vx_status)VX_ERROR_INVALID_VALUE;
                    break;
                }
                /* LDRA_JUSTIFY_END */ 

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
            status = (vx_status)VX_ERROR_NO_RESOURCES;
            VX_PRINT(VX_ZONE_ERROR, "Invalid Target Instance Context\n");
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
        inFrmList = &msc_obj->inFrmList;
        outFrmList = &msc_obj->outFrmList;

        inFrmList->numFrames = 1U;

        frm = &msc_obj->inFrm;
        if((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->in_img0_format)
        {
            if(msc_obj->msc_prms.inFmt.dataFormat == FVID2_DF_LUMA_ONLY)
            {
                frm->addr[0] = tivxMemShared2PhysPtr(
                in_img_desc0->mem_ptr[0].shared_ptr,
                (int32_t)in_img_desc0->mem_ptr[0].mem_heap_region);
            }
            else if(msc_obj->msc_prms.inFmt.dataFormat == FVID2_DF_CHROMA_ONLY)
            {
                frm->addr[0] = tivxMemShared2PhysPtr(
                in_img_desc0->mem_ptr[1].shared_ptr,
                (int32_t)in_img_desc0->mem_ptr[1].mem_heap_region);
            }
            else
            {
                for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES; plane_cnt ++)
                {
                    frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                        in_img_desc0->mem_ptr[plane_cnt].shared_ptr,
                        (int32_t)in_img_desc0->mem_ptr[plane_cnt].mem_heap_region);
                }
            }
        }
        else
        {
            for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES; plane_cnt ++)
            {
                frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                    in_img_desc0->mem_ptr[plane_cnt].shared_ptr,
                    (int32_t)in_img_desc0->mem_ptr[plane_cnt].mem_heap_region);
            }
        }
        #if defined(VPAC3) || defined(VPAC3L)
        if(NULL != in_img_desc1)
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(((((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format))) &&
                (msc_obj->msc_prms.isEnableSimulProcessing == 1U))
            /* LDRA_JUSTIFY_END */

            {
                frm->addr[0] = tivxMemShared2PhysPtr(
                        in_img_desc0->mem_ptr[0].shared_ptr,
                        (int32_t)in_img_desc0->mem_ptr[0].mem_heap_region);
                frm->addr[1] = tivxMemShared2PhysPtr(
                        in_img_desc1->mem_ptr[0].shared_ptr,
                        (int32_t)in_img_desc1->mem_ptr[0].mem_heap_region);
            }
        }
        #endif     

        out_base_desc = (tivx_obj_desc_t *)out_img_desc[0u];
        msc_obj->timestamp = out_base_desc->timestamp;
        outFrmList->numFrames = MSC_MAX_OUTPUT;
        for (cnt = 0u; cnt < msc_obj->num_outputs; cnt ++)
        {
            idx = msc_obj->sc_map_idx[cnt];
            frm = &msc_obj->outFrm[idx];
            idx = cnt;// + TIVX_KERNEL_VPAC_MSC_SCALE_OUT0_IMG_IDX;

            #if defined(VPAC3) || defined(VPAC3L)
                res = cnt >> div_fact;
                if ((((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_NV12 == out_img_desc[res]->format)) &&
                    (msc_obj->msc_prms.isEnableSimulProcessing == 1u))
                {
                    if((cnt % 2U) == 0u)
                    {
                        frm->addr[0] = tivxMemShared2PhysPtr(
                            out_img_desc[res]->mem_ptr[0].shared_ptr,
                            (int32_t)out_img_desc[res]->mem_ptr[0].
                            mem_heap_region);
                    }
                    else
                    {
                        frm->addr[0] = tivxMemShared2PhysPtr(
                            out_img_desc[res]->mem_ptr[1].shared_ptr,
                            (int32_t)out_img_desc[res]->mem_ptr[1].
                            mem_heap_region);
                    }
                }
                else if(((((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format))) &&
                    (msc_obj->msc_prms.isEnableSimulProcessing == 1U))
                {
                    if((cnt % 2U) == 0u)
                    {
                        frm->addr[0] = tivxMemShared2PhysPtr(
                            out_img_desc[idx]->mem_ptr[0].shared_ptr,
                            (int32_t)out_img_desc[idx]->mem_ptr[0].
                            mem_heap_region);
                    }
                    else
                    {
                        frm->addr[0] = tivxMemShared2PhysPtr(
                            out_img_desc[idx]->mem_ptr[0].shared_ptr,
                            (int32_t)out_img_desc[idx]->mem_ptr[0].
                            mem_heap_region);
                    }
                }
                else
                {
                    for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES;
                        plane_cnt ++)
                    {
                        frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                            out_img_desc[idx]->mem_ptr[plane_cnt].shared_ptr,
                            (int32_t)out_img_desc[idx]->mem_ptr[plane_cnt].
                            mem_heap_region);
                    }
                }


            #else
            for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES;
                        plane_cnt ++)
            {
                frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                    out_img_desc[idx]->mem_ptr[plane_cnt].shared_ptr,
                    (int32_t)out_img_desc[idx]->mem_ptr[plane_cnt].
                    mem_heap_region);
            }
            #endif
        }

        /* Clearing error status from previous frame
        *  User should consume the error status before
        *  submitting next frame for processing
        */
        msc_obj->wdTimerErrStatus = 0u;
        msc_obj->err_stat = 0u;

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit MSC Request*/
        fvid2_status = Fvid2_processRequest(msc_obj->handle, inFrmList,
            outFrmList, FVID2_TIMEOUT_FOREVER);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Submit Request\n");
            status = (vx_status)VX_FAILURE;
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
        /* Wait for Frame Completion */
        (void)tivxEventWait(msc_obj->wait_for_compl, VX_TIMEOUT_WAIT_FOREVER);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale:
        This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
       if(0u == msc_obj->wdTimerErrStatus)
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
            if(0u != msc_obj->err_stat)
            {
                VX_PRINT(VX_ZONE_INFO, "Error interrupt: MSC non-stalling error interrupt triggered \n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            fvid2_status = Fvid2_getProcessedRequest(msc_obj->handle,
                inFrmList, outFrmList, 0);
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to Get Processed Request\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale:
        This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "IP stall: Watchdog timer error interrupt triggered \n");
            status = (vx_status)VX_ERROR_TIMEOUT;
        }
        /* LDRA_JUSTIFY_END */
#if !defined(VPAC3L)
        /* Call the control command for statusreg/configReg validate */
        {
            fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_VALIDATE_REG, NULL, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Register validation failure requires hardware corruption or fault injection not available in test environment.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Register validation failed (Fvid2_control returned %d)\n", fvid2_status);
                validate_reg_status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }
#endif
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
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;
        #if defined(TEST_MSC_PERFORMANCE_LOGGING)
        VX_PRINT(VX_ZONE_INFO, "timestamp value of process function is: %llu\n", cur_time);
        #endif

        inst_obj = msc_obj->inst_obj;

        if ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc0->format)
        {
            if(msc_obj->msc_prms.inFmt.dataFormat == FVID2_DF_LUMA_ONLY)
            {
                size = in_img_desc0->imagepatch_addr[0].dim_x*in_img_desc0->imagepatch_addr[0].dim_y;
            }
            else if(msc_obj->msc_prms.inFmt.dataFormat == FVID2_DF_CHROMA_ONLY)
            {
                size = in_img_desc0->imagepatch_addr[0].dim_x*in_img_desc0->imagepatch_addr[0].dim_y/2U;
            }
            else
            {
                size = (in_img_desc0->imagepatch_addr[0].dim_x*in_img_desc0->imagepatch_addr[0].dim_y) + \
                   ((in_img_desc0->imagepatch_addr[0].dim_x*in_img_desc0->imagepatch_addr[0].dim_y)/2U);
            }

        }
        else
        {
            size = (in_img_desc0->imagepatch_addr[0].dim_x)*(in_img_desc0->imagepatch_addr[0].dim_y);
        }

        appPerfStatsHwaUpdateLoad(inst_obj->hwa_perf_id,
            (uint32_t)cur_time,
            size /* pixels processed */
            );
    }

    #if !defined(VPAC3L)
    if(((vx_status)VX_SUCCESS != status) || ((vx_status)VX_SUCCESS != validate_reg_status))
    {
        status = (vx_status)VX_FAILURE;
    }
    #endif

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscScaleControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status            status = (vx_status)VX_SUCCESS;
    uint32_t             size;
    tivxVpacMscScaleObj *msc_obj = NULL;

    void *temp_kernel_context = NULL;


    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The kernel parameter is expected to be pre-validated by the OpenVX framework before kernel callback invocation.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, msc_obj would not be assigned and subsequent NULL checks would catch the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        msc_obj = temp_kernel_context;
    }
        /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Get Target Kernel Instance Context\n");
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
    else if ((NULL == msc_obj) ||
        (sizeof(tivxVpacMscScaleObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Size\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */ 
    else     
    {
        /* do nothing */
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
        switch (node_cmd_id)
        {
            case TIVX_VPAC_MSC_CMD_SET_COEFF:
            {
                status = tivxVpacMscScaleSetCoeffsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS:
            {
                status = tivxVpacMscScaleSetInputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS:
            {
                status = tivxVpacMscScaleSetOutputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U], num_params);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_CROP_PARAMS:
            {
                status = tivxVpacMscScaleSetCropParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U], num_params);
                break;
            }
            case TIVX_VPAC_MSC_CMD_GET_ERR_STATUS:
            {
                status = tivxVpacMscGetErrStatusCmd(msc_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_GET_PSA_STATUS:
            {
                status = tivxVpacMscGetPsaStatusCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxVpacMscMultiScaleEnableErrorEventsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            /* MSC: Add case to control callback for safety mechanism */
#if !defined(VPAC3L)
            case TIVX_VPAC_MSC_CMD_ENABLE_SAFETY_MECHANISM:
            {
                status = tivxEnableVpacMscSafetyMechanisms(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0]);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, the else branch would handle successful enablement without error logging.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to enable MSC safety mechanisms\n");
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#endif
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Command Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
        }
    }

    return status;
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacMscScaleObj *tivxVpacMscScaleAllocObject(tivxVpacMscScaleInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacMscScaleObj *msc_obj = NULL;
    vx_status status;


    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Mutex lock acquisition failure requires system-level resource exhaustion or mutex corruption that cannot be simulated in the test environment.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the else block would handle the mutex lock failure.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale:
        The test framework and test apps can reach till 8 only but not 16. The current kernel test app cannot support all 16 handles
        Effect on this unit:
        It will utilize all the 16 handles
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */

        {
            if (0U == instObj->msc_obj[cnt].isAlloc)
            {
                msc_obj = &instObj->msc_obj[cnt];
                memset(msc_obj, 0x0, sizeof(tivxVpacMscScaleObj));
                instObj->msc_obj[cnt].isAlloc = 1U;
                break;
            }
        }

        /* Release instance mutex */
        (void)tivxMutexUnlock(instObj->lock);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Mutex lock acquisition failure requires system-level resource exhaustion or mutex corruption that cannot be simulated in the test environment.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to log the error message for debugging purposes.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }
    /* LDRA_JUSTIFY_END */
    return (msc_obj);
}

static void tivxVpacMscScaleFreeObject(tivxVpacMscScaleInstObj *instObj,
    tivxVpacMscScaleObj *msc_obj)
{
    uint32_t cnt;
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Mutex lock acquisition failure requires system-level resource exhaustion or mutex corruption that cannot be simulated in the test environment.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to log the error message for debugging purposes.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale:
        The test framework and test apps can reach till 8 only but not 16. The current kernel test app cannot support all 16 handles
        Effect on this unit:
        It will utilize all the 16 handles
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_MSC_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */

        {
            if (msc_obj == &instObj->msc_obj[cnt])
            {
                msc_obj->isAlloc = 0U;
                break;
            }
        }
        /* Release instance mutex */
        (void)tivxMutexUnlock(instObj->lock);
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Mutex lock acquisition failure requires system-level resource exhaustion or mutex corruption that cannot be simulated in the test environment.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to log the error message for debugging purposes.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }
    /* LDRA_JUSTIFY_END */
}

void tivxVpacMscMultiScaleSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc, uint32_t do_line_skip)
{

     /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != img_desc)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the host kernel before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to print the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        switch (img_desc->format)
        {
        /* LDRA_JUSTIFY_END */

            case (vx_df_image)VX_DF_IMAGE_NV12:
            {
                fmt->dataFormat = FVID2_DF_YUV420SP_UV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U8:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U16:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                break;
            }
            case (vx_df_image)TIVX_DF_IMAGE_P12:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_UYVY:
            {
                fmt->dataFormat = FVID2_DF_YUV422I_UYVY;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to set input/output dataFormat and ccsFormat accordingly.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            case (vx_df_image)VX_DF_IMAGE_YUYV:
            {
                fmt->dataFormat = FVID2_DF_YUV422I_YUYV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Vx Image Format\n");
                break;
            }
            /* LDRA_JUSTIFY_END */

        }


        fmt->width      = img_desc->imagepatch_addr[0].dim_x;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support. 
        This API skips lines in input to improve the performance but this affects output quality. 
        Effect on this unit: There is no impact, as the feature is unused, control cannot reach this condition.
        <justification end> */
        if(do_line_skip != 0U)
        {
            fmt->height     = img_desc->imagepatch_addr[0].dim_y/2U;
            fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y*2U;
            fmt->pitch[1]   = (uint32_t)img_desc->imagepatch_addr[1].stride_y*2U;
        }
        else
        /* LDRA_JUSTIFY_END */           

        {
            fmt->height     = img_desc->imagepatch_addr[0].dim_y;
            fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
            fmt->pitch[1]   = (uint32_t)img_desc->imagepatch_addr[1].stride_y;
        }
    }
}

void tivxVpacMscMultiScaleSetScParams(Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivx_obj_desc_image_t *out_img_desc, uint32_t do_line_skip)
{
     /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
    if ((NULL != in_img_desc) && (NULL != out_img_desc))
    /* LDRA_JUSTIFY_END */           

    {
        sc_cfg->enable = UTRUE;
        sc_cfg->outWidth = out_img_desc->imagepatch_addr[0].dim_x;
        sc_cfg->outHeight = out_img_desc->imagepatch_addr[0].dim_y;
        sc_cfg->inRoi.cropStartX = 0u;
        sc_cfg->inRoi.cropStartY = 0u;
        sc_cfg->inRoi.cropWidth = in_img_desc->imagepatch_addr[0].dim_x;
         /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support. 
        This API skips lines in input to improve the performance but this affects output quality. 
        Effect on this unit: There is no impact, as the feature is unused, control cannot reach this condition.
        <justification end> */
        if(do_line_skip != 0U)
        {
            sc_cfg->inRoi.cropHeight = in_img_desc->imagepatch_addr[0].dim_y/2U;
        }
        else
        /* LDRA_JUSTIFY_END */           

        {
            sc_cfg->inRoi.cropHeight = in_img_desc->imagepatch_addr[0].dim_y;
        }
    }
}

void tivxVpacMscScaleCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    const tivx_vpac_msc_output_params_t *out_prms)
{
    sc_cfg->isSignedData = out_prms->signed_data;

    sc_cfg->coeffShift = out_prms->coef_shift;
    sc_cfg->isEnableFiltSatMode = out_prms->saturation_mode;

    /* Single Phase Coefficients */
    if (0u == out_prms->filter_mode)
    {
        sc_cfg->filtMode = MSC_FILTER_MODE_SINGLE_PHASE;

        /* Select one of the dedicated single phase coefficient */
        if (0u == out_prms->single_phase.horz_coef_src)
        {
            if (0u == out_prms->single_phase.horz_coef_sel)
            {
                sc_cfg->hsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;
            }
            else
            {
                sc_cfg->hsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_1;
            }
        }
        else /* Select one from multi-phase coefficients */
        {
            /* Value of vert_coef_sel is from 0-31,
             * but driver requires 2 to 17, so adding lowest value here */
            sc_cfg->hsSpCoeffSel = out_prms->single_phase.horz_coef_sel +
                MSC_SINGLE_PHASE_MP_COEFF0_0;
        }

        /* Select one of the dedicated single phase coefficient */
        if (0u == out_prms->single_phase.vert_coef_src)
        {
            if (0u == out_prms->single_phase.vert_coef_sel)
            {
                sc_cfg->vsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_0;
            }
            else
            {
                sc_cfg->vsSpCoeffSel = MSC_SINGLE_PHASE_SP_COEFF_1;
            }
        }
        else /* Select one from multi-phase coefficients */
        {
            /* Value of vert_coef_sel is from 0-31,
             * but driver requires 2 to 17, so adding lowest value here */
            sc_cfg->vsSpCoeffSel = out_prms->single_phase.vert_coef_sel +
                MSC_SINGLE_PHASE_MP_COEFF0_0;
        }
    }
    else /* Multi Phase Coefficients */
    {
        sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;

        /* 64 Phase Coefficients */
        if (0u == out_prms->multi_phase.phase_mode)
        {
            sc_cfg->phaseMode = MSC_PHASE_MODE_64PHASE;

            /* Used Coefficent 0 for horizontal scaling */
            if (0u == out_prms->multi_phase.horz_coef_sel)
            {
                sc_cfg->hsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
            }
            else /* Used Coefficent 1 for horizontal scaling */
            {
                sc_cfg->hsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_2;
            }

            /* Used Coefficent 0 for vertical scaling */
            if (0u == out_prms->multi_phase.vert_coef_sel)
            {
                sc_cfg->vsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_0;
            }
            else /* Used Coefficent 2 for vertical scaling */
            {
                sc_cfg->vsMpCoeffSel = MSC_MULTI_64PHASE_COEFF_SET_2;
            }
        }
        else /* 32 Phase Coefficients */
        {
            sc_cfg->phaseMode = MSC_PHASE_MODE_32PHASE;

            switch (out_prms->multi_phase.horz_coef_sel)
            {
                case 0:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
                case 1:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
                    break;
                case 2:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_2;
                    break;
                case 3:
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Incorrect multi-phase horz coeff, defaulting to set 0\n");
                    sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
            }

            switch (out_prms->multi_phase.vert_coef_sel)
            {
                case 0:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
                case 1:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_1;
                    break;
                case 2:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_2;
                    break;
                case 3:
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_3;
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Incorrect multi-phase horz coeff, defaulting to set 0\n");
                    sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
                    break;
            }
        }
    }
}

void tivxVpacMscScaleUpdateStartPhases(const tivxVpacMscScaleObj *msc_obj,
    Msc_ScConfig *sc_cfg, uint32_t cnt)
{
    vx_float32 temp_horzAccInit, temp_vertAccInit;
    uint32_t int_horzAccInit, int_vertAccInit;
    uint32_t temp_cropStartX, temp_cropStartY;

    if((TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_offset_x[cnt]) ||
       (TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_init_phase_x[cnt]))
    {
        temp_horzAccInit = (((((vx_float32)sc_cfg->inRoi.cropWidth/(vx_float32)sc_cfg->outWidth) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        int_horzAccInit = (uint32_t)temp_horzAccInit;
        temp_cropStartX = 0;
        if(int_horzAccInit > 4095U)
        {
            int_horzAccInit -= 4096U;
            temp_cropStartX = 1U;
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_init_phase_x[cnt])
        {
            sc_cfg->horzAccInit = int_horzAccInit;
        }
        else
        {
            sc_cfg->horzAccInit = msc_obj->user_init_phase_x[cnt];
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_offset_x[cnt])
        {
            sc_cfg->inRoi.cropStartX = msc_obj->user_crop_start_x[cnt] + temp_cropStartX;
        }
        else
        {
            sc_cfg->inRoi.cropStartX = msc_obj->user_crop_start_x[cnt] + msc_obj->user_offset_x[cnt];
        }

        /* TIOVX-1129: If NV12, x_offset should be an even number to not flip the chroma channels */
        if ((msc_obj->msc_prms.outFmt[msc_obj->sc_map_idx[cnt]].dataFormat == FVID2_DF_YUV420SP_UV) &&
            ((sc_cfg->inRoi.cropStartX & 1U) == 1U))
        {
            sc_cfg->inRoi.cropStartX--;
            sc_cfg->horzAccInit = 4095U;
        }
    }
    else
    {
        sc_cfg->horzAccInit = msc_obj->user_init_phase_x[cnt];
        sc_cfg->inRoi.cropStartX = msc_obj->user_crop_start_x[cnt] + msc_obj->user_offset_x[cnt];
    }

    if((TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_offset_y[cnt]) ||
       (TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_init_phase_y[cnt]))
    {
        temp_vertAccInit = (((((vx_float32)sc_cfg->inRoi.cropHeight/(vx_float32)sc_cfg->outHeight) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        int_vertAccInit = (uint32_t)temp_vertAccInit;
        temp_cropStartY = 0;
        if(int_vertAccInit > 4095U)
        {
            int_vertAccInit -= 4096U;
            temp_cropStartY = 1U;
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_init_phase_y[cnt])
        {
            sc_cfg->vertAccInit = int_vertAccInit;
        }
        else
        {
            sc_cfg->vertAccInit = msc_obj->user_init_phase_y[cnt];
        }

        if(TIVX_VPAC_MSC_AUTOCOMPUTE == msc_obj->user_offset_y[cnt])
        {
            sc_cfg->inRoi.cropStartY = msc_obj->user_crop_start_y[cnt] + temp_cropStartY;
        }
        else
        {
            sc_cfg->inRoi.cropStartY = msc_obj->user_crop_start_y[cnt] + msc_obj->user_offset_y[cnt];
        }
    }
    else
    {
        sc_cfg->vertAccInit = msc_obj->user_init_phase_y[cnt];
        sc_cfg->inRoi.cropStartY = msc_obj->user_crop_start_y[cnt] + msc_obj->user_offset_y[cnt];
    }
    if(msc_obj->yc_mode == TIVX_VPAC_MSC_MODE_CHROMA_ONLY)
    {
        sc_cfg->inRoi.cropStartY = sc_cfg->inRoi.cropStartY>>1;
    }
}
void tivxVpacMscScaleSetScParamsForEachLevel(tivxVpacMscScaleObj *msc_obj,Vhwa_M2mMscParams *msc_prms)
{
    uint32_t                          idx;
    uint32_t                          cnt;
    Msc_ScConfig                     *sc_cfg = NULL;
    Fvid2_Format                      *fmt = NULL;
    uint32_t do_line_skip = 0, numIter, div_fact, res;

    numIter = TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT;
    div_fact = 0;
    #if defined(VPAC3) || defined(VPAC3L)
    if(TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_PARAMS == msc_obj->num_params)
    {
        numIter = TIVX_KERNEL_VPAC_MSC_SCALE2_MAX_OUTPUT;
    }
    if ((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->out_img_desc[0u]->format)
    {
        if(msc_obj->msc_prms.isEnableSimulProcessing == 1U)
        {
            numIter = MSC_MAX_OUTPUT;
            div_fact = 1;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != msc_obj->in_img_desc0)
    /* LDRA_JUSTIFY_END */
    {
        if (((vx_df_image)VX_DF_IMAGE_UYVY == msc_obj->in_img_desc0->format) ||
            ((vx_df_image)VX_DF_IMAGE_YUYV == msc_obj->in_img_desc0->format))
        {
            numIter = TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT - 1U;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(((NULL != msc_obj->in_img_desc0) && (NULL != msc_obj->in_img_desc1)) && (msc_obj->msc_prms.isEnableSimulProcessing == 1U))
    /* LDRA_JUSTIFY_END */
    {
        if(msc_obj->in_img_desc0->format !=  msc_obj->in_img_desc1->format)
        {
            if((((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)))
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale:
                The test framework and test apps cannot reach this portion as it pre-validated in tivxVpacMscScaleSetInputParamsCmd()
                Effect on this unit:
                None, control will not reach here
                <justification end> */
                if(msc_obj->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                /* LDRA_JUSTIFY_END */           
                {
                    if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_2PLANES;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_VU_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_VU;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_UV;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_VU_CHROMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_VU;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        /* Do nothing */
                    }
                    /* LDRA_JUSTIFY_END */
                }
                if((((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)))
                {
                    msc_prms->inFmt.ccsFormat = FVID2_CCSF_BITS8_PACKED;
                    msc_prms->secChPrms.ccsf = FVID2_CCSF_BITS12_PACKED;
                }
                else
                {
                    msc_prms->inFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
                    msc_prms->secChPrms.ccsf = FVID2_CCSF_BITS8_PACKED;
                }
            }
            else if((((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)))
            {
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start> 
                Rationale:
                The test framework and test apps cannot reach this portion as it pre-validated in tivxVpacMscScaleSetInputParamsCmd()
                Effect on this unit:
                None, control will not reach here
                <justification end> */
                if(msc_obj->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                /* LDRA_JUSTIFY_END */           
                {
                    if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_2PLANES;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_VU_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_VU;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_UV;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_VU_CHROMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_VU;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        /* Do nothing */
                    }
                    /* LDRA_JUSTIFY_END */
                }
                if((((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)))
                {
                    msc_prms->inFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
                    msc_prms->secChPrms.ccsf = FVID2_CCSF_BITS12_UNPACKED16;
                }
                else
                {
                    msc_prms->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                    msc_prms->secChPrms.ccsf = FVID2_CCSF_BITS12_PACKED;
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start> 
            Rationale:
            The test framework and test apps cannot reach this portion as it pre-validated in tivxVpacMscScaleValidate()
            Effect on this unit:
            None, control will not reach here
            <justification end> */
            else if((((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)))
            /* LDRA_JUSTIFY_END */           

            {
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start> 
                Rationale:
                The test framework and test apps cannot reach this portion as it pre-validated in tivxVpacMscScaleSetInputParamsCmd()
                Effect on this unit:
                None, control will not reach here
                <justification end> */
                if(msc_obj->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                /* LDRA_JUSTIFY_END */           
                {
                    if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_2PLANES;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_VU_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_VU;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_UV;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_VU_CHROMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_VU;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        /* Do nothing */
                    }     
                    /* LDRA_JUSTIFY_END */  
                }
                if((((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)))
                {
                    msc_prms->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                    msc_prms->secChPrms.ccsf = FVID2_CCSF_BITS8_PACKED;
                }
                else
                {
                    msc_prms->inFmt.ccsFormat = FVID2_CCSF_BITS8_PACKED;
                    msc_prms->secChPrms.ccsf = FVID2_CCSF_BITS12_UNPACKED16;
                }
            }
            else
            {
                /* Unsupported format combination */
            }
            msc_prms->inFmt.pitch[0] = (uint32_t)msc_obj->in_img_desc0->imagepatch_addr[0].stride_y;
            msc_prms->inFmt.pitch[1] = (uint32_t)msc_obj->in_img_desc1->imagepatch_addr[0].stride_y;
            msc_prms->inFmt.width = (uint32_t)msc_obj->in_img_desc0->imagepatch_addr[0].dim_x;
            msc_prms->inFmt.height = (uint32_t)msc_obj->in_img_desc0->imagepatch_addr[0].dim_y;
            msc_prms->secChPrms.enable = (uint32_t)true;
            msc_prms->secChPrms.pitch = (uint32_t)msc_obj->in_img_desc1->imagepatch_addr[0].stride_y;
        }
        else
        {   /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start> 
            Rationale:
            The test framework and test apps cannot reach this portion as it pre-validated in tivxVpacMscScaleValidate()
            Effect on this unit:
            None, control will not reach here
            <justification end> */
            if((((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)))
            /* LDRA_JUSTIFY_END */           

            {
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start> 
                Rationale:
                The test framework and test apps cannot reach this portion as it pre-validated in tivxVpacMscScaleSetInputParamsCmd()
                Effect on this unit:
                None, control will not reach here
                <justification end> */
                if(msc_obj->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                /* LDRA_JUSTIFY_END */           
                {
                    if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_2PLANES;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_VU_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV420SP_VU;
                    }
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_UV_CHROMA_ONLY)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_UV;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_VU_CHROMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_YUV422SP_VU;
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        /* Do nothing */
                    }       
                    /* LDRA_JUSTIFY_END */
                }
                msc_prms->inFmt.pitch[1] = (uint32_t)msc_obj->in_img_desc1->imagepatch_addr[0].stride_y;
            }
        }
    }
    #endif
    for (cnt = 0; cnt < numIter ; cnt ++)
    {
        res = cnt >> div_fact;
        if((msc_obj->out_img_desc[res] != NULL) && (msc_obj->in_img_desc0 != NULL))
        {
            if (1U == msc_obj->inst_obj->alloc_sc_fwd_dir)
            {
                #if defined(VPAC3) || defined(VPAC3L)
                    if (((vx_df_image)VX_DF_IMAGE_UYVY == msc_obj->in_img_desc0->format) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == msc_obj->in_img_desc0->format))
                    {
                        msc_obj->msc_prms.isEnableSimulProcessing = 1U;
                        if(cnt == 0U)
                        {
                            idx = cnt;
                        }
                        else 
                        {
                            idx += 2u;
                        }
                    }
                    else
                    {
                        idx = cnt;
                    }
                #else
                idx = cnt;
                #endif
            }
            else
            {
                #if defined(VPAC3) || defined(VPAC3L)
                    if (((vx_df_image)VX_DF_IMAGE_UYVY == msc_obj->in_img_desc0->format) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == msc_obj->in_img_desc0->format))
                    {
                        msc_obj->msc_prms.isEnableSimulProcessing = 1U;
                        if(cnt == 0U)
                        {
                            idx = MSC_MAX_OUTPUT - 4U - cnt;
                        }
                        else
                        {
                            idx -= 2u;
                        }
                    }
                    else
                    {
                        idx = MSC_MAX_OUTPUT - 1U - cnt;
                    }
                #else
                idx = MSC_MAX_OUTPUT - 1U - cnt;
                #endif
            }
            fmt = &msc_prms->outFmt[idx];
            sc_cfg = &msc_prms->mscCfg.scCfg[idx];
            msc_obj->sc_map_idx[cnt] = idx;
            sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE; //MSC_FILTER_MODE_SINGLE_PHASE;
            tivxVpacMscMultiScaleSetScParams(sc_cfg, msc_obj->in_img_desc0, msc_obj->out_img_desc[res], do_line_skip);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if((NULL != msc_obj->in_img_desc0) && (NULL != msc_obj->in_img_desc1))
            /* LDRA_JUSTIFY_END */           

            {
                if((cnt % 2U) == 0U)
                {
                    tivxVpacMscMultiScaleSetScParams(sc_cfg, msc_obj->in_img_desc0, msc_obj->out_img_desc[res], do_line_skip);
                }
                else
                {
                    tivxVpacMscMultiScaleSetScParams(sc_cfg, msc_obj->in_img_desc1, msc_obj->out_img_desc[res], do_line_skip);
                }
            }
            
            tivxVpacMscMultiScaleSetFmt(fmt, msc_obj->out_img_desc[res], 0U);
            if ((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->in_img0_format)
            {
                if((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->out_img_format[cnt])
                {
                    if((uint32_t)TIVX_VPAC_MSC_MODE_LUMA_ONLY == msc_obj->yc_mode)
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
                        msc_prms->outFmt[idx].dataFormat = FVID2_DF_LUMA_ONLY;
                    }
                    else
                    {
                        msc_prms->inFmt.dataFormat = FVID2_DF_CHROMA_ONLY;
                        msc_prms->outFmt[idx].dataFormat = FVID2_DF_CHROMA_ONLY;
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(NULL != msc_obj->in_img_desc0)
                        /* LDRA_JUSTIFY_END */
                        {
                            msc_prms->mscCfg.scCfg[idx].inRoi.cropHeight = msc_obj->in_img_desc0->imagepatch_addr[0].dim_y/2U;
                            msc_prms->inFmt.height = msc_obj->in_img_desc0->imagepatch_addr[0].dim_y/2U;
                        }
                    }
                }
            }
            #if defined(VPAC3) || defined(VPAC3L)

            if((((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->in_img_desc0->format) &&
                ((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->out_img_desc[res]->format)) &&
                (msc_obj->msc_prms.isEnableSimulProcessing == 1u))
            {
                if((cnt % 2U) == 0u)
                {
                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_CHROMA_ONLY;
                    msc_prms->mscCfg.scCfg[idx].inRoi.cropHeight = (msc_prms->mscCfg.scCfg[idx].inRoi.cropHeight)>>1;
                    msc_prms->mscCfg.scCfg[idx].outHeight = (msc_prms->mscCfg.scCfg[idx].outHeight) >> 1;
                    msc_obj->in0_height = (msc_obj->in0_height) >> 1;
                }
            }
            if(NULL != msc_obj->in_img_desc1)
            {
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start> Rationale:
                The test framework and test apps cannot reach this portion as it pre-validated in tivxVpacMscScaleValidate()
                Effect on this unit:
                None, control will not reach here
                <justification end> */
                if((((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->in_img_desc1->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc0->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == msc_obj->in_img_desc1->format)))
                {
                    if(msc_obj->msc_prms.isEnableSimulProcessing == 1u)
                /* LDRA_JUSTIFY_END */

                    {
                        if((cnt % 2U) == 0u)
                        {
                            if(msc_obj->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                            {
                                if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                                {
                                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_PLANE_1;
                                }
                                else
                                {
                                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_LUMA_ONLY;
                                }
                            }
                        }
                        else
                        {
                            if(msc_obj->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                            {
                                if(msc_obj->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                                {
                                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_PLANE_2;
                                }
                                else
                                {
                                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_CHROMA_ONLY;
                                }
                            }
                        }
                    }
                }
            }
            #endif

            msc_obj->user_init_phase_x[cnt] = TIVX_VPAC_MSC_AUTOCOMPUTE;
            msc_obj->user_init_phase_y[cnt] = TIVX_VPAC_MSC_AUTOCOMPUTE;
            msc_obj->user_offset_x[cnt] =     TIVX_VPAC_MSC_AUTOCOMPUTE;
            msc_obj->user_offset_y[cnt] =     TIVX_VPAC_MSC_AUTOCOMPUTE;
            msc_obj->user_crop_start_x[cnt] = 0;
            msc_obj->user_crop_start_y[cnt] = 0;

            /* If rescale is smaller than 1/4, it might be because of crop, so don't set init yet */
            if(((sc_cfg->outWidth * 4U) < sc_cfg->inRoi.cropWidth) ||
                ((sc_cfg->outHeight * 4U) < sc_cfg->inRoi.cropHeight))
            {
                sc_cfg->horzAccInit = 0;
                sc_cfg->vertAccInit = 0;
            }
            else
            {
                tivxVpacMscScaleUpdateStartPhases(msc_obj, sc_cfg, cnt);
            }
        }
        else
        {
            break;
        }      
    }
    msc_obj->num_outputs = cnt; 
}

/**
 * \brief Map VHWA error interrupt flags to TIVX error interrupt flags
 *
 * \param vhwa_err_events   VHWA error event flags (input)
 * \param wd_timer_err      VHWA watchdog timer error flags (input)
 * \param tivx_err_events   Pointer to TIVX error event flags (output)
 *
 * \return VX_SUCCESS on success, VX_FAILURE if null pointer,
 *         VX_ERROR_INVALID_PARAMETERS if invalid error event mask
 */
static vx_status tivxVpacMscMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t mapped_tivx_err_events = 0u;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == tivx_err_events)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((0u != (vhwa_err_events & ~(VHWA_MSC_VBUSM_RD_ERR | VHWA_MSC_SL2_WR_ERR))) ||
             (0u != (wd_timer_err & ~(VHWA_MSC0_WDTIMER_ERR | VHWA_MSC1_WDTIMER_ERR))))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask provided\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, the else branch would map VHWA error flags to TIVX error flags for valid hardware error events.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Map VHWA flags to TIVX flags for regular error interrupts */
        if (0u != (vhwa_err_events & VHWA_MSC_VBUSM_RD_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_MSC_VBUSM_RD_ERR;
        }
        if (0u != (vhwa_err_events & VHWA_MSC_SL2_WR_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_MSC_SL2_WR_ERR;
        }

        /* Map VHWA flags to TIVX flags for watchdog timer errors */
        if (0u != (wd_timer_err & VHWA_MSC0_WDTIMER_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_MSC0_WDTIMER_ERR;
        }
        if (0u != (wd_timer_err & VHWA_MSC1_WDTIMER_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_MSC1_WDTIMER_ERR;
        }
        /* LDRA_JUSTIFY_END_DISABLED */

        *tivx_err_events = mapped_tivx_err_events;
    }

    return status;
}

/**
 * \brief Map TIVX error interrupt flags to VHWA error interrupt flags
 *
 * \param tivx_err_events   TIVX error event flags (input)
 * \param vhwa_err_events   Pointer to VHWA error event flags (output)
 * \param wd_timer_err      Pointer to VHWA watchdog timer error flags (output)
 *
 * \return VX_SUCCESS on success, VX_FAILURE if null pointers,
 *         VX_ERROR_INVALID_PARAMETERS if invalid error event mask
 */
static vx_status tivxVpacMscMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
                                               uint32_t *vhwa_err_events,
                                               uint32_t *wd_timer_err)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t enableErrorEventCheckMask = (TIVX_VPAC_MSC0_ALL_ERR_EVENTS | TIVX_VPAC_MSC1_ALL_ERR_EVENTS);

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL == vhwa_err_events) || (NULL == wd_timer_err))
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (0u != (tivx_err_events & ~enableErrorEventCheckMask))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask provided\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        *vhwa_err_events = 0u;
        *wd_timer_err = 0u;
        
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, the else branch would map VHWA error flags to TIVX error flags for valid hardware error events.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Map TIVX flags to VHWA flags for regular error interrupts */
        if (0u != (tivx_err_events & TIVX_VPAC_MSC_VBUSM_RD_ERR))
        {
            *vhwa_err_events |= VHWA_MSC_VBUSM_RD_ERR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_MSC_SL2_WR_ERR))
        {
            *vhwa_err_events |= VHWA_MSC_SL2_WR_ERR;
        }

        /* Map TIVX flags to VHWA flags for watchdog timer errors */
        if (0u != (tivx_err_events & TIVX_VPAC_MSC0_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_MSC0_WDTIMER_ERR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_MSC1_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_MSC1_WDTIMER_ERR;
        }
        /* LDRA_JUSTIFY_END_DISABLED */
    }

    return status;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxVpacMscScaleSetCoeffsCmd(tivxVpacMscScaleObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          cnt;
    tivx_vpac_msc_coefficients_t     *coeffs = NULL;
    void                             *target_ptr;
    Msc_Coeff                        *coeffCfg = NULL;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_msc_coefficients_t) ==
                usr_data_obj->mem_size)
        {
            coeffs = (tivx_vpac_msc_coefficients_t *)target_ptr;
            coeffCfg = &msc_obj->coeffCfg;

            Msc_coeffInit(coeffCfg);

            for (cnt = 0u; cnt < MSC_MAX_SP_COEFF_SET; cnt ++)
            {
                coeffCfg->spCoeffSet[cnt] = &coeffs->single_phase[cnt][0u];
            }

            for (cnt = 0u; cnt < MSC_MAX_MP_COEFF_SET; cnt ++)
            {
                coeffCfg->mpCoeffSet[cnt] = &coeffs->multi_phase[cnt][0u];
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Data Object is NULL\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        fvid2_status = Fvid2_control(msc_obj->handle, VHWA_M2M_IOCTL_MSC_SET_COEFF,
            coeffCfg, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to create coefficients\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */           

    }

    return (status);
}

static vx_status tivxVpacMscScaleSetOutputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[], uint16_t num_params)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt, idx, res, div_fact = 0;
    tivx_vpac_msc_output_params_t    *out_prms = NULL;
    void                             *target_ptr;
    Msc_ScConfig                     *sc_cfg = NULL;

    uint32_t loop_params = (num_params < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT) ? \
                            num_params : TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT;

    if ((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->out_img_desc[0u]->format)
    {
        if(msc_obj->msc_prms.isEnableSimulProcessing == 1U)
        {
            loop_params = 2U * loop_params;
            div_fact = 1U;
        }
    }

    for (cnt = 0u; cnt < loop_params; cnt ++)
    {
        res = cnt >> div_fact;
        if (NULL != usr_data_obj[res])
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[res]->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[res]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            if ((uint32_t)sizeof(tivx_vpac_msc_output_params_t) ==
                    usr_data_obj[res]->mem_size)
            {
                out_prms = (tivx_vpac_msc_output_params_t *)target_ptr;

                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                tivxVpacMscScaleCopyOutPrmsToScCfg(sc_cfg, out_prms);

                msc_obj->user_init_phase_x[cnt] = out_prms->multi_phase.init_phase_x;
                msc_obj->user_init_phase_y[cnt] = out_prms->multi_phase.init_phase_y;
                msc_obj->user_offset_x[cnt] =     out_prms->offset_x;
                msc_obj->user_offset_y[cnt] =     out_prms->offset_y;

                tivxVpacMscScaleUpdateStartPhases(msc_obj, sc_cfg, cnt);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Mem Size for Output Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[res]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = Fvid2_control(msc_obj->handle,
            VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_obj->msc_prms, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Set Output Params\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */           
    }

    return (status);
}


static vx_status tivxVpacMscScaleSetInputParamsCmd(tivxVpacMscScaleObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_vpac_msc_input_params_t     *in_prms = NULL;
    void                             *target_ptr;
    Vhwa_M2mMscParams                *msc_prms = NULL;
    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_msc_input_params_t) ==
                usr_data_obj->mem_size)
        {
            in_prms = (tivx_vpac_msc_input_params_t *)target_ptr;

            msc_obj->msc_prms.enableLineSkip = in_prms->src_ln_inc_2;
            msc_prms = &msc_obj->msc_prms;
            msc_obj->yc_mode = in_prms->yc_mode;
            if(1U < in_prms->yc_mode)
            {
                VX_PRINT(VX_ZONE_ERROR, "yc_mode value should be either 0 or 1\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            msc_obj->enable_error_events = in_prms->enable_error_events;
            msc_prms->enablePsa = in_prms->enable_psa;

            #if defined(VPAC3) || defined(VPAC3L)
            if((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->out_img_desc[0u]->format)
            {
                msc_obj->msc_prms.isEnableSimulProcessing = in_prms->is_enable_simul_processing;
                
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            else if((NULL != msc_obj->in_img_desc0) && (NULL != msc_obj->in_img_desc1))
            /* LDRA_JUSTIFY_END */
            {
                msc_obj->msc_prms.isEnableSimulProcessing = 1U;
                msc_obj->in_img0_yc_mode = in_prms->in_img0_yc_mode;
                msc_obj->in_img1_yc_mode = in_prms->in_img1_yc_mode;
                if(in_prms->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_CHROMA_ONLY)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Input0 should be luma only\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }
                if(in_prms->in_img0_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                {
                    if(in_prms->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_LUMA_ONLY)
                    {
                        if(msc_obj->in0_height != (msc_obj->in1_height))
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Incase of luma-luma plane processing height of both planes should be same\n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                    }
                    else if((in_prms->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_UV_CHROMA_ONLY) ||
                            (in_prms->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_420SP_VU_CHROMA_ONLY))
                    {
                        if(msc_obj->in0_height != ((msc_obj->in1_height) * 2U))
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Incase of YUV420SP processing Chroma plane height should be half of luma plane height\n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else if((in_prms->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_UV_CHROMA_ONLY) ||
                            (in_prms->in_img1_yc_mode == TIVX_VPAC_MSC_MODE_422SP_VU_CHROMA_ONLY))
                    /* LDRA_JUSTIFY_END */
                    {
                        if(msc_obj->in0_height != (msc_obj->in1_height))
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Incase of YUV422SP processing Chroma plane height should be same as luma plane height\n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications CAN reach this portion.
                    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    else
                    {
                        /* Do nothing */
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
            else
            {
                /* Unsupported configuration */
            }
            #endif
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to set tapSel accordingly.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            switch (in_prms->kern_sz)
            {
            /* LDRA_JUSTIFY_END */
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to set tapSel accordingly.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                case 3:
                    msc_obj->msc_prms.mscCfg.tapSel = MSC_TAP_SEL_3TAPS;
                    break;
                case 4:
                    msc_obj->msc_prms.mscCfg.tapSel = MSC_TAP_SEL_4TAPS;
                    break;
                /* LDRA_JUSTIFY_END */
                case 5:
                    msc_obj->msc_prms.mscCfg.tapSel = MSC_TAP_SEL_5TAPS;
                    break;
                default:
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Kernel Size\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    break;
            }
            tivxVpacMscScaleSetScParamsForEachLevel(msc_obj,msc_prms);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = Fvid2_control(msc_obj->handle,
            VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_obj->msc_prms, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Set Input Params\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

    }

    return (status);
}

static vx_status tivxVpacMscScaleSetCropParamsCmd(tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[], uint16_t num_params)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt, idx,res, div_fact = 0;
    tivx_vpac_msc_crop_params_t      *out_prms = NULL;
    void                             *target_ptr;
    Msc_ScConfig                     *sc_cfg = NULL;

    uint32_t loop_params = (num_params < TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT) ? \
                            num_params : TIVX_KERNEL_VPAC_MSC_SCALE_MAX_OUTPUT;

    if ((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->out_img_desc[0u]->format)
    {
        if(msc_obj->msc_prms.isEnableSimulProcessing == 1U)
        {
            loop_params = 2U * loop_params;
            div_fact = 1U;
        }
    }          
    for (cnt = 0u; cnt < loop_params; cnt ++)
    {
        res = cnt >> div_fact;
        if (NULL != usr_data_obj[res])          
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[res]->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[res]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            if (sizeof(tivx_vpac_msc_crop_params_t) ==
                    usr_data_obj[res]->mem_size)
            {
                out_prms = (tivx_vpac_msc_crop_params_t *)target_ptr;

                idx = msc_obj->sc_map_idx[cnt];
                sc_cfg = &msc_obj->msc_prms.mscCfg.scCfg[idx];

                sc_cfg->inRoi.cropStartX = out_prms->crop_start_x;
                sc_cfg->inRoi.cropStartY = out_prms->crop_start_y;
                sc_cfg->inRoi.cropWidth = out_prms->crop_width;
                sc_cfg->inRoi.cropHeight = out_prms->crop_height;
                msc_obj->user_crop_start_x[cnt] = out_prms->crop_start_x;
                msc_obj->user_crop_start_y[cnt] = out_prms->crop_start_y;
                if ((vx_df_image)VX_DF_IMAGE_NV12 == msc_obj->out_img_desc[0u]->format)
                {
                    if(((msc_obj->msc_prms.isEnableSimulProcessing == 1U) && ((cnt % 2U) == 1U)))
                    {
                        sc_cfg->inRoi.cropHeight = out_prms->crop_height/2U;
                        msc_obj->user_crop_start_y[cnt] = out_prms->crop_start_y/2U;
                        sc_cfg->inRoi.cropStartY = out_prms->crop_start_y/2U;
                    }
                }

                tivxVpacMscScaleUpdateStartPhases(msc_obj, sc_cfg, cnt);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Mem Size for Crop Params\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }

            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[res]->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }

        if ((vx_status)VX_SUCCESS != status)
        {
            break;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = (vx_status)Fvid2_control(msc_obj->handle,
            VHWA_M2M_IOCTL_MSC_SET_PARAMS, &msc_obj->msc_prms, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)FVID2_SOK != status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Set Crop Params\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static vx_status tivxVpacMscGetErrStatusCmd(const tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat = 0u;

    if (NULL != scalar_obj_desc)
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxVpacMscMapVhwaToTivxErrEvents(msc_obj->err_stat, msc_obj->wdTimerErrStatus, &combined_err_stat);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            scalar_obj_desc->data.u32 = combined_err_stat;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, the code would print error message about invalid hardware error mask.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask from hardware\n");
        }
        /* LDRA_JUSTIFY_END */
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status tivxVpacMscGetPsaStatusCmd(const tivxVpacMscScaleObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    Vhwa_M2mMscPsaSign                  psa;
    int32_t                             fvid2_status = FVID2_SOK;
    void                                *target_ptr;
    tivx_vpac_msc_psa_timestamp_data_t  *psa_status_ptr;


    if (NULL != usr_data_obj)
    {
        if (sizeof(tivx_vpac_msc_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            fvid2_status = (vx_status)Fvid2_control(msc_obj->handle,
                    VHWA_M2M_IOCTL_MSC_GET_PSA_SIGN, &psa, NULL);
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
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscGetPsa: Fvid2_control Failed\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            /* Map the user data object to access the memory */
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            psa_status_ptr = (tivx_vpac_msc_psa_timestamp_data_t *)target_ptr;

            /* Copy driver data (PSA values) into the structure */
            tivx_obj_desc_memcpy(&psa_status_ptr->psa_values, psa.psaSign, sizeof(psa.psaSign));

            /* Add timestamp to the structure */
            psa_status_ptr->timestamp = msc_obj->timestamp;

            /* Unmap the memory after copying */
            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null argument. Expected pointer to tivx_obj_desc_user_data_object_t\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status tivxVpacMscMultiScaleEnableErrorEventsCmd(tivxVpacMscScaleObj *msc_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t intr_en = 0u;
    uint32_t wd_timer_intr_en = 0u;
    void *target_ptr;
    tivx_vpac_msc_input_params_t *params_ptr = NULL;
    uint32_t enableErrorEventsMask = 0u;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != usr_data_obj)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(sizeof(tivx_vpac_msc_input_params_t) != usr_data_obj->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVpacMscMultiScaleEnableErrorEventsCmd: Incorrect Data Object Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Memory buffer mapping failure requires system-level resource exhaustion or invalid memory configuration that cannot be simulated in the test environment.
            This defensive check handles extreme system failure scenarios that are architecturally prevented in normal operation.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, the code would detect memory mapping failure and log an error message.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxVpacMscMultiScaleEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_vpac_msc_input_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->enable_error_events;
                /* Update WDTimer configuration from user parameters */
                msc_obj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->wdtimer_timeout_cycles;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxVpacMscMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, the code would detect invalid error event mask and log an error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacMscMultiScaleEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    msc_obj->errEvtPrms.errEvents = intr_en;
                    fvid2_status = Fvid2_control(msc_obj->handle,
                        VHWA_M2M_IOCTL_MSC_REGISTER_ERR_CB, &msc_obj->errEvtPrms, NULL);

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK == fvid2_status)
                    /* LDRA_JUSTIFY_END */
                    {
                        msc_obj->wdTimerErrEvtPrms.wdTimerErrEvents = wd_timer_intr_en;
                        fvid2_status = Fvid2_control(msc_obj->handle,
                            VHWA_M2M_IOCTL_MSC_REGISTER_WDTIMER_ERR_CB, &msc_obj->wdTimerErrEvtPrms, NULL);
                    }
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
                        VX_PRINT(VX_ZONE_ERROR,
                            "tivxVpacMscMultiScaleEnableErrorEventsCmd: Fvid2_control Failed \n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacMscMultiScaleEnableErrorEventsCmd: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}
/* ========================================================================== */
/*                    Config Register Readback Functions                     */
/* ========================================================================== */

#if !defined(VPAC3L)
static vx_status tivxVpacMscScaleAllocReadbackBuffers(tivxVpacMscScaleObj *mscObj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t   fvid2_status = FVID2_SOK; 

    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == mscObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxVpacMscScaleAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (NULL == mscObj->handle)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxVpacMscScaleAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Valid pointers */
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
        uint32_t readback_size = mscObj->config_reg_mem_size;

        /* Allocate readback buffer */
        vx_status alloc_status = tivxMemBufferAlloc(&mscObj->readback_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory buffer allocation failure for readback buffers requires system-level resource exhaustion that cannot be reliably simulated in the test environment.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            mscObj->readback_mem_ptr_phys = tivxMemShared2PhysPtr(mscObj->readback_mem_ptr_virt.shared_ptr, (int32_t)mscObj->readback_mem_ptr_virt.mem_heap_region);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memset((void *)(uintptr_t)mscObj->readback_mem_ptr_virt.shared_ptr, 0, readback_size);
        }

        /* Allocate golden register buffer */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            alloc_status = tivxMemBufferAlloc(&mscObj->golden_reg_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);

            mscObj->golden_reg_mem_ptr_phys = tivxMemShared2PhysPtr(mscObj->golden_reg_mem_ptr_virt.shared_ptr, (int32_t)mscObj->golden_reg_mem_ptr_virt.mem_heap_region);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memset((void *)(uintptr_t)mscObj->golden_reg_mem_ptr_virt.shared_ptr, 0, readback_size);
        }

        /* Make FVID2 control call to update the register readback structure */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_M2mMscConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ptr   = (void *)(uintptr_t)mscObj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)mscObj->readback_mem_ptr_virt.shared_ptr;

            /* Register the callback for config register memory comparison */
            reg_ptrs.cbFxn = tivxVpacMscScaleConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)mscObj;

            fvid2_status = Fvid2_control(
                mscObj->handle,
                VHWA_M2M_IOCTL_MSC_SET_CONFIG_REG_PTRS,
                (void *)&reg_ptrs,
                NULL
            );

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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set MSC register readback/golden pointers\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This cleanup path is reached only when setting register pointers fails, which requires hardware driver errors.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, the code would free allocated readback buffers to prevent memory leaks.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                tivxVpacMscScaleFreeReadbackBuffers(mscObj);
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
            tivxVpacMscScaleFreeReadbackBuffers(mscObj);
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static void tivxVpacMscScaleFreeReadbackBuffers(tivxVpacMscScaleObj *mscObj)
{
    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != mscObj)
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;
        uint32_t readback_size = mscObj->config_reg_mem_size;

        /* Free readback buffer if allocated */
        if (mscObj->readback_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&mscObj->readback_mem_ptr_virt, readback_size);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Memory buffer deallocation failure requires system-level resource management errors that cannot be simulated in the test environment.
            This defensive check handles extreme system failure scenarios that are architecturally prevented in normal operation.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, the code would detect memory free failure and log an error message.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free readback buffer\n");
            }
            /* LDRA_JUSTIFY_END */
            mscObj->readback_mem_ptr_phys = 0u;
        }

        /* Free golden register buffer if allocated */
        if (mscObj->golden_reg_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&mscObj->golden_reg_mem_ptr_virt, readback_size);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Memory buffer deallocation failure requires system-level resource management errors that cannot be simulated in the test environment.
            This defensive check handles extreme system failure scenarios that are architecturally prevented in normal operation.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, the code would detect memory free failure and log an error message.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free golden register buffer\n");
            }
            /* LDRA_JUSTIFY_END */
            mscObj->golden_reg_mem_ptr_phys = 0u;
        }

        mscObj->config_reg_mem_size = 0u;
    }
}
/* MSC: Enable safety mechanism function */
static vx_status tivxEnableVpacMscSafetyMechanisms(
    tivxVpacMscScaleObj *mscObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *params_ptr = NULL;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t enable;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == mscObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "mscObj is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else if (NULL != usr_data_obj)
    {
        params_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Shared memory pointer conversion failure requires invalid memory descriptor or system-level memory management errors that cannot be simulated in the test environment.
        This defensive check handles extreme system failure scenarios that are architecturally prevented in normal operation.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code would detect NULL shared memory pointer and return VX_FAILURE status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL == params_ptr)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to get shared memory pointer\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
        else
        {
            tivxCheckStatus(&status, tivxMemBufferMap(params_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to configure safety mechanisms.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                if (sizeof(tivx_vpac_msc_safety_mechanism_params_t) == usr_data_obj->mem_size)
                {
                    tivx_vpac_msc_safety_mechanism_params_t *safety_params =
                        (tivx_vpac_msc_safety_mechanism_params_t *)params_ptr;

                    /* Enable or disable reconfigReinitReg based on input parameter */
                    if ((vx_bool)vx_true_e == (vx_bool)safety_params->enable_reconfig_and_reinit_Reg)
                    {
                        enable = 1U;
                    }
                    else
                    {
                        enable = 0U;
                    }
                    fvid2_status = Fvid2_control(mscObj->handle, VHWA_M2M_IOCTL_MSC_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This FVID2 control failure requires hardware driver errors or invalid hardware state that cannot be simulated in the test environment.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to log the error and set failure status.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set reconfigReinitReg\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                    safety_params->enable_reconfig_and_reinit_Reg = (uint32_t)vx_false_e;

                    /* Enable or disable status register validation based on input parameter */
                    if ((vx_bool)vx_true_e == (vx_bool)safety_params->enable_status_reg_validation)
                    {
                        enable = 1U;
                    }
                    else
                    {
                        enable = 0U;
                    }
                    fvid2_status = Fvid2_control(mscObj->handle, VHWA_M2M_IOCTL_MSC_ENABLE_STATUS_REG_VALIDATE, &enable, NULL);
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
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set status register validation\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                    safety_params->enable_status_reg_validation = (uint32_t)vx_false_e;

                    /* Enable or disable config register readback based on input parameter */
                    if ((vx_bool)vx_true_e == (vx_bool)safety_params->enable_readback_config_registers)
                    {
                        uint32_t readback_size = 0U;

                        /* Fetch the readback size from the low level driver */
                        if ((vx_status)VX_SUCCESS == status)
                        {
                            fvid2_status = Fvid2_control(
                                mscObj->handle,
                                VHWA_M2M_IOCTL_MSC_GET_READBACK_SIZE,
                                &readback_size,
                                NULL
                            );

                            mscObj->config_reg_mem_size = readback_size;

                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case is out of scope for the imaging test framework.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if (((int32_t)FVID2_SOK == fvid2_status) && (readback_size > 0U))
                            /* LDRA_JUSTIFY_END */
                            {
                                VX_PRINT(VX_ZONE_INFO, "MSC readback size allocation successful: %u bytes\n", readback_size);
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
                                VX_PRINT(VX_ZONE_ERROR, "Failed to fetch MSC readback size from driver\n");
                                status = (vx_status)VX_FAILURE;
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
                        if(((vx_status)VX_SUCCESS == status) && (readback_size > 0U))
                        /* LDRA_JUSTIFY_END */
                        {
                            /* Before allocation, check if any of the buffers are already allocated */
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This condition checks if buffers are already allocated to avoid double allocation.
                            Effect on this unit: If the control reaches here, allocation is skipped to prevent memory leak.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((mscObj->readback_mem_ptr_phys != 0U) && (mscObj->golden_reg_mem_ptr_phys != 0U))
                            {
                                VX_PRINT(VX_ZONE_INFO, "Readback or golden register buffer already allocated, skipping allocation\n");
                            }
                            /* LDRA_JUSTIFY_END */
                            else
                            {
                                status = tivxVpacMscScaleAllocReadbackBuffers(mscObj);
                            }
                        }

                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to enable config register readback mechanism.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if ((vx_status)VX_SUCCESS == status)
                        /* LDRA_JUSTIFY_END */
                        {
                            enable = 1U;
                            fvid2_status = Fvid2_control(mscObj->handle, VHWA_M2M_IOCTL_MSC_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case is out of scope for the imaging test framework.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((int32_t)FVID2_SOK != fvid2_status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                                tivxVpacMscScaleFreeReadbackBuffers(mscObj);
                                status = (vx_status)VX_FAILURE;
                            }
                            /* LDRA_JUSTIFY_END */
                        }
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case requires buffer allocation failure that cannot be simulated in test environment.
                        Effect on this unit: If the control reaches here, error is logged and config register validation is skipped.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "MSC readback buffer allocation failed, skipping config register validation enable\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    else
                    {
                        enable = 0U;
                        fvid2_status = Fvid2_control(mscObj->handle, VHWA_M2M_IOCTL_MSC_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if ((int32_t)FVID2_SOK != fvid2_status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                            status = (vx_status)VX_FAILURE;
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    safety_params->enable_readback_config_registers = (uint32_t)vx_false_e;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.nale, this is not tested.
                <justification end> */
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */

                tivxCheckStatus(&status, tivxMemBufferUnmap(params_ptr, usr_data_obj->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
            }
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}
#endif

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacMscMultiScaleFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacMscScaleObj *msc_obj = (tivxVpacMscScaleObj *)appData;
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
    This behaviour is part of the application design. An error accumulation statement can be added in a future release.
    <justification end> */
    if (NULL != msc_obj)
    /* LDRA_JUSTIFY_END */

    {
        (void)tivxEventPost(msc_obj->wait_for_compl);
    }

    return FVID2_SOK;
}

#if !defined(VPAC3L)
int32_t tivxVpacMscScaleConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mMscConfigRegMemParams *msc_config_reg_prms = (Vhwa_M2mMscConfigRegMemParams *)configRegPrms;
    tivxVpacMscScaleObj *msc_obj = NULL;

    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This callback is invoked by the hardware driver with pre-validated parameters.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != msc_config_reg_prms) && (NULL != handle))
    /* LDRA_JUSTIFY_END */
    {
        msc_obj = (tivxVpacMscScaleObj *)msc_config_reg_prms->appData;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This callback is invoked by the hardware driver with pre-validated parameters.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != msc_config_reg_prms) && (NULL != handle) && (NULL != msc_obj))
    /* LDRA_JUSTIFY_END */
    {
        /* Compare the golden register memory and readback register memory */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are set up during buffer allocation and are expected to be valid if safety mechanisms are enabled.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != msc_config_reg_prms->golden_ptr) &&
            (NULL != msc_config_reg_prms->readback_ptr) &&
            (msc_config_reg_prms->readback_mem_size > 0U))
        /* LDRA_JUSTIFY_END */
        {
            int32_t cmp_result = memcmp(
                msc_config_reg_prms->golden_ptr,
                msc_config_reg_prms->readback_ptr,
                msc_config_reg_prms->readback_mem_size
            );

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Register memory comparison mismatch detection requires hardware register corruption or configuration errors during operation.
            This is a safety mechanism callback that compares golden register values with readback values to detect hardware faults.
            Testing this failure path requires injecting hardware faults or memory corruption that cannot be simulated in the test environment.
            This defensive check handles hardware-level fault scenarios that cannot be tested without hardware fault injection.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, the code would detect register configuration mismatch and return FVID2_EFAIL status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (cmp_result != 0)
            {
                VX_PRINT(VX_ZONE_ERROR, "MSC Config Register Validation Failed - Mismatch detected\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                VX_PRINT(VX_ZONE_INFO, "MSC Config Register Validation Passed\n");
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid parameters for MSC config register validation\n");
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer in MSC config register validation callback\n");
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}
#endif

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale:
This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacMscMultiScaleErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacMscScaleObj *msc_obj = (tivxVpacMscScaleObj *)appData;

    if (NULL != msc_obj)
    {
        msc_obj->err_stat = errEvents;
    }
}
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale:
This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacMscMultiScaleWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxVpacMscScaleObj *msc_obj = (tivxVpacMscScaleObj *)appData;

    if (NULL != msc_obj)
    {
        msc_obj->wdTimerErrStatus = msc_obj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents;

        if(0u != msc_obj->wdTimerErrStatus)
        {
            (void)tivxEventPost(msc_obj->wait_for_compl);
        }
    }
}

/* LDRA_JUSTIFY_END */
BUILD_ASSERT((sizeof(Vhwa_M2mMscPsaSign) == sizeof(((tivx_vpac_msc_psa_timestamp_data_t *)0)->psa_values))? 1U : 0U);

