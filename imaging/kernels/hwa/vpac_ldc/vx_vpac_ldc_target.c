/*
 *
 * Copyright (c) 2017-2021 Texas Instruments Incorporated
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

#include <vx_vpac_ldc_target_priv.h>
#include <vhwa/include/vhwa_m2mLdc.h>
#if defined(LDRA_COVERAGE)
#include <kernels/coverage_files/include/ldra_remote_core_coverage_main.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static tivxVpacLdcObj *tivxVpacLdcAllocObject(tivxVpacLdcInstObj *instObj);
static void tivxVpacLdcFreeObject(tivxVpacLdcInstObj *instObj,
    tivxVpacLdcObj *ldc_obj);
static void tivxVpacLdcSetRegionParams(Ldc_Config *cfg,
    const tivx_obj_desc_user_data_object_t *reg_prms_desc);
static vx_status tivxVpacLdcSetFmt(const tivx_vpac_ldc_params_t *ldc_prms,
    Fvid2_Format *fmt, const tivx_obj_desc_image_t *img_desc);
static void tivxVpacLdcSetAffineConfig(Ldc_PerspectiveTransformCfg *cfg,
    const tivx_obj_desc_matrix_t *warp_matrix_desc);
static vx_status tivxVpacLdcSetMeshParams(Ldc_Config *ldc_cfg,
    const tivx_obj_desc_user_data_object_t *mesh_prms_desc,
    const tivx_obj_desc_image_t *mesh_img_desc);
static vx_status tivxVpacLdcSetLutParamsCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_user_data_object_t *luma_user_desc,
    tivx_obj_desc_user_data_object_t *chroma_user_desc);
static vx_status tivxVpacLdcGetErrStatusCmd(const tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxVpacLdcSetRdBwLimitCmd(tivxVpacLdcObj *ldc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacLdcSetParams(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_t *obj_desc[]);
static vx_status tivxVpacLdcGetPsaStatusCmd(const tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacLdcEnableErrorEventsCmd(tivxVpacLdcObj *ldc_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacLdcMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxVpacLdcMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);

static int32_t tivxVpacLdcFrameComplCb(Fvid2_Handle handle, void *appData);
static void tivxVpacLdcErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxVpacLdcWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);
int32_t tivxVpacLdcConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms);

static vx_status vpacLdcInstObjInit(void);
static void vpacLdcInstObjDeinit(void);

static vx_status tivxEnableVpacLdcSafetyMechanisms(
    tivxVpacLdcObj *ldcObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);

static vx_status tivxVpacLdcAllocReadbackBuffers(tivxVpacLdcObj *ldcObj);

static void tivxVpacLdcFreeReadbackBuffers(tivxVpacLdcObj *ldcObj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static tivx_target_kernel vx_vpac_ldc_target_kernel = NULL;
static tivx_target_kernel vx_vpac_ldc2_target_kernel = NULL;
tivxVpacLdcInstObj gTivxVpacLdcInstObj;
static uint32_t gTivxVpacLdcInstObj_is_initialized = 0U;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static vx_status vpacLdcInstObjInit(void)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    if(0U == gTivxVpacLdcInstObj_is_initialized)
    {
        /* Allocate lock mutex */
        status = tivxMutexCreate(&gTivxVpacLdcInstObj.lock);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to create Mutex\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            memset(&gTivxVpacLdcInstObj.ldc_obj, 0x0,
                sizeof(tivxVpacLdcObj) * VHWA_M2M_LDC_MAX_HANDLES);
        }
    }
    gTivxVpacLdcInstObj_is_initialized ++;

    return (status);
}
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
static void vpacLdcInstObjDeinit(void)
{
    if(gTivxVpacLdcInstObj_is_initialized > 0U)
    {
        gTivxVpacLdcInstObj_is_initialized --;
        if(0U == gTivxVpacLdcInstObj_is_initialized)
        {
            if (NULL != gTivxVpacLdcInstObj.lock)
            {
                (void)tivxMutexDelete(&gTivxVpacLdcInstObj.lock);
            }
        }
    }
}
/* LDRA_JUSTIFY_END */
void tivxAddTargetKernelVpacLdc(void)
{
    vx_status status;
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
    if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
    /* LDRA_JUSTIFY_END */
    {
        strncpy(target_name, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #if (VPAC_COUNT > 1)
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_LDC1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_END */
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
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        vx_vpac_ldc_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_LDC_NAME,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            tivxVpacLdcControl,
                            NULL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vx_vpac_ldc_target_kernel)
        /* LDRA_JUSTIFY_END */
        {
            status = vpacLdcInstObjInit();
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
    if((vx_status)VX_SUCCESS != status)
    {
        /* TODO: how to handle this condition */
        VX_PRINT(VX_ZONE_ERROR, "Failed to Add LDC TargetKernel\n");
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
void tivxRemoveTargetKernelVpacLdc(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_ldc_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_ldc_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Ldc TargetKernel\n");
    }
    vpacLdcInstObjDeinit();
}
/* LDRA_JUSTIFY_END */
void tivxAddTargetKernelVpacLdc2(void)
{
    vx_status status;
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
    if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
    /* LDRA_JUSTIFY_END */
    {
        strncpy(target_name, TIVX_TARGET_VPAC_LDC1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #if (VPAC_COUNT > 1)
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
    {
        strncpy(target_name, TIVX_TARGET_VPAC2_LDC1, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_END */
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
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */
    {
        vx_vpac_ldc2_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_LDC2_NAME,
                            target_name,
                            tivxVpacLdcProcess,
                            tivxVpacLdcCreate,
                            tivxVpacLdcDelete,
                            tivxVpacLdcControl,
                            NULL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vx_vpac_ldc2_target_kernel)
        /* LDRA_JUSTIFY_END */
        {
           status = vpacLdcInstObjInit();
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
    if((vx_status)VX_SUCCESS != status)
    {
        /* TODO: how to handle this condition */
        VX_PRINT(VX_ZONE_ERROR, "Failed to Add LDC TargetKernel\n");
    }
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelVpacLdc2(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_ldc2_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_ldc2_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Ldc TargetKernel\n");
    }
    vpacLdcInstObjDeinit();
}
/* LDRA_JUSTIFY_END */

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacLdcProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status              status = (vx_status)VX_SUCCESS;
    int32_t                fvid2_status = FVID2_SOK;
    uint32_t               size;
    uint32_t               out_cnt;
    uint32_t               plane_cnt;
    Fvid2_Frame           *frm = NULL;
    tivx_obj_desc_image_t *in_frm_desc[2U] = {NULL};
    tivx_obj_desc_image_t *out_frm_desc[4U] = {NULL};
    tivxVpacLdcObj        *ldc_obj = NULL;
    Fvid2_FrameList       *inFrmList;
    Fvid2_FrameList       *outFrmList;
    uint64_t               cur_time = 0ULL;
    tivx_obj_desc_t         *out_base_desc = NULL;
    vx_status               validate_reg_status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( ((num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
        && ((num_params != TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX])))
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
        Effect on this unit: If the control reaches here, ldc_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            ldc_obj = temp_kernel_context;
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to get Target Kernel\n");
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if ((NULL == ldc_obj) ||
            (sizeof(tivxVpacLdcObj) != size))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Ldc Object\n");
            status = (vx_status)VX_ERROR_INVALID_NODE;
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
        else if ((1u == ldc_obj->ldc_cfg.enableOutput[1U]) &&
                (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX]))
        {
            VX_PRINT(VX_ZONE_ERROR, "Null Desc for output1\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* do nothing */
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch  <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        inFrmList = &ldc_obj->inFrmList;
        outFrmList = &ldc_obj->outFrmList;
        if(num_params == TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        {
            in_frm_desc[0u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX];
            out_frm_desc[0u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
            out_frm_desc[1u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];
        }
        if(num_params == TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS)
        {
            in_frm_desc[0u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX];
            in_frm_desc[1u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC2_IN1_IMG_IDX];
            out_frm_desc[0u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX];
            out_frm_desc[1u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT1_IMG_IDX];
            out_frm_desc[2u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT2_IMG_IDX];
            out_frm_desc[3u] = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT3_IMG_IDX];
            /* LDRA_JUSTIFY_START
            <metric start> branch  <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(NULL != in_frm_desc[1])
            /* LDRA_JUSTIFY_END */
            {
                out_frm_desc[1u] = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT2_IMG_IDX];
                out_frm_desc[2u] = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT1_IMG_IDX];
            }
        }

        inFrmList->frames[0U] = &ldc_obj->inFrm;
        inFrmList->numFrames = 1U;
        outFrmList->frames[0U] = &ldc_obj->outFrm[0U];
        outFrmList->frames[1U] = &ldc_obj->outFrm[1U];
        outFrmList->numFrames = 0U;

        frm = &ldc_obj->inFrm;
        #if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> branch  <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((in_frm_desc[0] != NULL) && (in_frm_desc[1] != NULL))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch  <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */
            if(((((vx_df_image)VX_DF_IMAGE_U8 == in_frm_desc[0]->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == in_frm_desc[1]->format)) ||
                (((vx_df_image)TIVX_DF_IMAGE_P12 == in_frm_desc[0]->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == in_frm_desc[1]->format)) ||
                (((vx_df_image)TIVX_DF_IMAGE_P12 ==in_frm_desc[0]->format) &&
                ((vx_df_image)VX_DF_IMAGE_U16 == in_frm_desc[1]->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U16 == in_frm_desc[0]->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == in_frm_desc[1]->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U16 == in_frm_desc[0]->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == in_frm_desc[1]->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U8 == in_frm_desc[0]->format) &&
                ((vx_df_image)VX_DF_IMAGE_U16 == in_frm_desc[1]->format)) ||
                (((vx_df_image)TIVX_DF_IMAGE_P12 == in_frm_desc[0]->format) &&
                ((vx_df_image)TIVX_DF_IMAGE_P12 == in_frm_desc[1]->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U8 == in_frm_desc[0]->format) &&
                ((vx_df_image)VX_DF_IMAGE_U8 == in_frm_desc[1]->format)) ||
                (((vx_df_image)VX_DF_IMAGE_U16 == in_frm_desc[0]->format) &&
                ((vx_df_image)VX_DF_IMAGE_U16 == in_frm_desc[1]->format))))
                /* LDRA_JUSTIFY_END */
            {
                frm->addr[0] = tivxMemShared2PhysPtr(
                        in_frm_desc[0]->mem_ptr[0].shared_ptr,
                        (int32_t)in_frm_desc[0]->mem_ptr[0].mem_heap_region);
                frm->addr[1] = tivxMemShared2PhysPtr(
                        in_frm_desc[1]->mem_ptr[0].shared_ptr,
                        (int32_t)in_frm_desc[1]->mem_ptr[0].mem_heap_region);
            }
        }
        else
        #endif
        if(in_frm_desc[0] != NULL)
        {
            for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES; plane_cnt ++)
            {
                frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                in_frm_desc[0]->mem_ptr[plane_cnt].shared_ptr,
                (int32_t)in_frm_desc[0]->mem_ptr[plane_cnt].mem_heap_region);
            }
        }
        else
        {
            /* Do Nothing */
            VX_PRINT(VX_ZONE_ERROR, "Null frame descriptor for inputs \n");
            status = (vx_status)VX_FAILURE;
        }

        out_base_desc = (tivx_obj_desc_t *)out_frm_desc[0];
        ldc_obj->timestamp = out_base_desc->timestamp;
        if((vx_status)VX_SUCCESS == status)
        {
            for (out_cnt = 0u; out_cnt < ldc_obj->num_output; out_cnt ++)
            {
                frm = &ldc_obj->outFrm[out_cnt];
                for (plane_cnt = 0u; plane_cnt < TIVX_IMAGE_MAX_PLANES;
                    plane_cnt ++)
                {
                    frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                        out_frm_desc[out_cnt]->mem_ptr[plane_cnt].shared_ptr,
                        (int32_t)out_frm_desc[out_cnt]->mem_ptr[plane_cnt].mem_heap_region);
                }
                #if defined(VPAC3) || defined(VPAC3L)
                if(out_cnt == 0U)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch  <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameter is expected to be pre-validated from a software layer above imaging.
                    Therefore, this failure case is out of scope for the imaging test framework.
                    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if((out_frm_desc[0] != NULL) && (out_frm_desc[1] != NULL) && (NULL != in_frm_desc[1]))
                    /* LDRA_JUSTIFY_END */
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch  <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        The parameters are pre-validated by the host kernel before the control reaches here.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                        This behaviour is part of the application design. An error print statement can be added in a future release if required.
                        <justification end> */
                        if(((((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[0]->format) &&
                            ((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[0]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)TIVX_DF_IMAGE_P12 ==out_frm_desc[0]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[0]->format) &&
                            ((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[0]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[0]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[0]->format) &&
                            ((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[0]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[1]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[0]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[1]->format))))
                        /* LDRA_JUSTIFY_END */
                        {
                            frm->addr[0] = tivxMemShared2PhysPtr(
                                    out_frm_desc[0]->mem_ptr[0].shared_ptr,
                                    (int32_t)out_frm_desc[0]->mem_ptr[0].mem_heap_region);
                            frm->addr[1] = tivxMemShared2PhysPtr(
                                    out_frm_desc[1]->mem_ptr[0].shared_ptr,
                                    (int32_t)out_frm_desc[1]->mem_ptr[0].mem_heap_region);
                        }
                    }
                }
                else
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameter is expected to be pre-validated from a software layer above imaging.
                    Therefore, this failure case is out of scope for the imaging test framework.
                    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if((out_frm_desc[2] != NULL) && (out_frm_desc[3] != NULL) && (NULL != in_frm_desc[1]))
                    {
                        if(((((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[2]->format) &&
                            ((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[2]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)TIVX_DF_IMAGE_P12 ==out_frm_desc[2]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[2]->format) &&
                            ((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[2]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[2]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[2]->format) &&
                            ((vx_df_image)TIVX_DF_IMAGE_P12 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[2]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U8 == out_frm_desc[3]->format)) ||
                            (((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[2]->format) &&
                            ((vx_df_image)VX_DF_IMAGE_U16 == out_frm_desc[3]->format))))
                        {
                            frm->addr[0] = tivxMemShared2PhysPtr(
                                    out_frm_desc[2]->mem_ptr[0].shared_ptr,
                                    (int32_t)out_frm_desc[2]->mem_ptr[0].mem_heap_region);
                            frm->addr[1] = tivxMemShared2PhysPtr(
                                    out_frm_desc[3]->mem_ptr[0].shared_ptr,
                                    (int32_t)out_frm_desc[3]->mem_ptr[0].mem_heap_region);
                        }
                    }
                    /* LDRA_JUSTIFY_END */
                }
                #endif        
                outFrmList->numFrames ++;
            }

            /* Clearing error status from previous frame
            *  User should consume the error status before
            *  submitting next frame for processing
            */
            ldc_obj->wdTimerErrStatus = 0u;
            ldc_obj->err_stat = 0u;

            cur_time = tivxPlatformGetTimeInUsecs();

            /* Submit LDC Request*/
            fvid2_status = Fvid2_processRequest(ldc_obj->handle, inFrmList,
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
        (void)tivxEventWait(ldc_obj->waitForProcessCmpl, VX_TIMEOUT_WAIT_FOREVER);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale:
        This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(0u == ldc_obj->wdTimerErrStatus)
        /* LDRA_JUSTIFY_END */
        {
            if(0u != ldc_obj->err_stat)
            {
                VX_PRINT(VX_ZONE_INFO, "Error interrupt: LDC non-stalling error interrupt triggered \n");
                status = (vx_status)VX_FAILURE;
            }

            fvid2_status = Fvid2_getProcessedRequest(ldc_obj->handle,
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
        <metric start> branch statement <metric end>
        <justification start> 
        Rationale:
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

        /* Call the control command for statusReg/configReg validate */
        {
            fvid2_status = Fvid2_control(ldc_obj->handle, VHWA_M2M_IOCTL_LDC_VALIDATE_REG, NULL, NULL);
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

    }

    /* LDRA_JUSTIFY_START
    <metric start> branch statement <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS != status)
    /* LDRA_JUSTIFY_END */
    {
        VX_PRINT(VX_ZONE_INFO, "LDC process completed with status: 0x%x (performance statistics are calculated for this request even if it failed)\n", status);
    }
    else
    {
        /* Do  Nothing */
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The ldc_obj parameter is expected to be pre-validated by the calling context (callback framework).
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base prevents undefined behaviour by avoiding dereferencing a NULL pointer, skipping performance statistics update.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((NULL != ldc_obj) && (cur_time != 0ULL))
    /* LDRA_JUSTIFY_END */
    {
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        appPerfStatsHwaUpdateLoad(ldc_obj->hwa_perf_id,
            (uint32_t)cur_time,
            (ldc_obj->ldc_cfg.outputFrameWidth*ldc_obj->ldc_cfg.outputFrameHeight) /* pixels processed */
            );
    }

    if(((vx_status)VX_SUCCESS != status) || ((vx_status)VX_SUCCESS != validate_reg_status))
    {
        status = (vx_status)VX_FAILURE;
    }
    return (status);
}

static vx_status VX_CALLBACK tivxVpacLdcCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    tivx_vpac_ldc_params_t           *ldc_prms = NULL;
    Ldc_Config                       *ldc_cfg = NULL;
    tivxVpacLdcObj                   *ldc_obj = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_matrix_t           *warp_matrix_desc = NULL;
    tivx_obj_desc_user_data_object_t *reg_prms_desc = NULL;
    tivx_obj_desc_user_data_object_t *mesh_prms_desc = NULL;
    tivx_obj_desc_image_t            *mesh_img_desc = NULL;
    tivx_obj_desc_image_t            *in0_img_desc = NULL;
    tivx_obj_desc_image_t            *in1_img_desc = NULL;
    tivx_obj_desc_image_t            *out0_img_desc = NULL;
    tivx_obj_desc_image_t            *out1_img_desc = NULL;
    tivx_obj_desc_image_t            *out2_img_desc = NULL;
    tivx_obj_desc_image_t            *out3_img_desc = NULL;
    void                             *target_ptr;
    tivx_obj_desc_user_data_object_t *dcc_buf_desc = NULL;
#if defined(VPAC3) || defined(VPAC3L)
    uint32_t                            in0_height = 0U;
    uint32_t                            in1_height = 0U;
    uint32_t                            out0_height = 0U;
    uint32_t                            out1_height = 0U;
    uint32_t                            out2_height = 0U;
    uint32_t                            out3_height = 0U;
#endif

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( ((num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
        && ((num_params != TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL Params check failed\n");
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
        ldc_obj = tivxVpacLdcAllocObject(&gTivxVpacLdcInstObj);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != ldc_obj)
        /* LDRA_JUSTIFY_END */
        {
            if(num_params == TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
            {
                config_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX];
                warp_matrix_desc = (tivx_obj_desc_matrix_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_WARP_MATRIX_IDX];
                reg_prms_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_REGION_PRMS_IDX];
                mesh_prms_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_PRMS_IDX];
                mesh_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_MESH_IMG_IDX];
                in0_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX];
                out0_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX];
                out1_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_OUT1_IMG_IDX];
                dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC_DCC_DB_IDX];
            }
            if(num_params == TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS)
            {
                config_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_CONFIGURATION_IDX];
                warp_matrix_desc = (tivx_obj_desc_matrix_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_WARP_MATRIX_IDX];
                reg_prms_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_REGION_PRMS_IDX];
                mesh_prms_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_MESH_PRMS_IDX];
                mesh_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_MESH_IMG_IDX];
                in0_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX];
                in1_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_IN1_IMG_IDX];
                out0_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX];
                out1_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT1_IMG_IDX];
                out2_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT2_IMG_IDX];
                out3_img_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT3_IMG_IDX];
                dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_VPAC_LDC2_DCC_DB_IDX];
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(NULL != in1_img_desc)
                /* LDRA_JUSTIFY_END */
                {
                    out1_img_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT2_IMG_IDX];
                    out2_img_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT1_IMG_IDX];
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Handle Object, increase VHWA_M2M_LDC_MAX_HANDLES macro in PDK driver\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
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

        Vhwa_M2mLdcCreateArgsInit(&ldc_obj->createArgs);

        ldc_cfg = &ldc_obj->ldc_cfg;

        target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, config_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        ldc_prms = (tivx_vpac_ldc_params_t *)target_ptr;
        ldc_obj->createArgs.enablePsa = ldc_prms->enable_psa;

        status = tivxEventCreate(&ldc_obj->waitForProcessCmpl);

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
            ldc_obj->cbPrms.cbFxn   = tivxVpacLdcFrameComplCb;
            ldc_obj->cbPrms.appData = ldc_obj;

            ldc_obj->handle = Fvid2_create(FVID2_VHWA_M2M_LDC_DRV_ID,
                ldc_obj->ldc_drv_inst_id, (void *)&ldc_obj->createArgs,
                NULL, &ldc_obj->cbPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == ldc_obj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Fvid2_create failed\n");
                status = (vx_status)VX_FAILURE;
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Event\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Register Error Callback */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        ldc_obj->errEvtPrms.errEvents =
            (VHWA_LDC_PIX_IBLK_OUTOFBOUND_ERR | VHWA_LDC_MESH_IBLK_OUTOFBOUND |
            VHWA_LDC_PIX_IBLK_MEMOVF | VHWA_LDC_MESH_IBLK_MEMOVF |
            VHWA_LDC_IFR_OUTOFBOUND | VHWA_LDC_INT_SZOVF |
            VHWA_LDC_SL2_WR_ERR | VHWA_LDC_VBUSM_RD_ERR);
        ldc_obj->errEvtPrms.cbFxn     = tivxVpacLdcErrorCb;
        ldc_obj->errEvtPrms.appData   = ldc_obj;

        fvid2_status = Fvid2_control(ldc_obj->handle,
            IOCTL_VHWA_M2M_LDC_REGISTER_ERR_CB, &ldc_obj->errEvtPrms, NULL);
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
        /* Set default 64K cycle timeout during registration */
        ldc_obj->wdTimerErrEvtPrms.wdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        /* Disable Watchdog Timer Error Event by default */
        ldc_obj->wdTimerErrEvtPrms.wdTimerErrEvents = 0u;
        ldc_obj->wdTimerErrEvtPrms.cbFxn     = tivxVpacLdcWdTimerErrorCb;
        ldc_obj->wdTimerErrEvtPrms.appData   = ldc_obj;
        fvid2_status = Fvid2_control(ldc_obj->handle,
            VHWA_M2M_IOCTL_LDC_REGISTER_WDTIMER_ERR_CB, &ldc_obj->wdTimerErrEvtPrms, NULL);
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
        #if defined VPAC1
            if(NULL != in1_img_desc)
            {
                VX_PRINT(VX_ZONE_ERROR, " 'in1_img' should be NULL for VPAC1\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
        #endif
        #if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((NULL != in0_img_desc) && (NULL != in1_img_desc))
        /* LDRA_JUSTIFY_END */
        {
            in0_height = in0_img_desc->imagepatch_addr[0].dim_y;
            in1_height = in1_img_desc->imagepatch_addr[0].dim_y;
            out0_height = out0_img_desc->imagepatch_addr[0].dim_y;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging.
            Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if(NULL != out1_img_desc)
            /* LDRA_JUSTIFY_END */
            {
                out1_height = out1_img_desc->imagepatch_addr[0].dim_y;
            }
            if(NULL != out2_img_desc)
            {
                out2_height = out2_img_desc->imagepatch_addr[0].dim_y;
            }
            if(NULL != out3_img_desc)
            {
                out3_height = out3_img_desc->imagepatch_addr[0].dim_y;
            }
            if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
            {
                if((NULL != out2_img_desc) && (NULL != out3_img_desc))
                {
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    VX_PRINT(VX_ZONE_ERROR, "'out1_img' and 'out3_img' should be NULL when 'in_img1_yc_mode is TIVX_VPAC_LDC_MODE_LUMA_ONLY \n");
                }
            }
            if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_CHROMA_ONLY)
            {
                VX_PRINT(VX_ZONE_ERROR, "'in0_img' should be luma only\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
            {
                if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                {
                    if(in0_height != in1_height)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Incase of luma-luma plane processing 'in_height' of both planes should be same\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    if(out0_height != out1_height)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Incase of luma-luma plane processing 'out_height' of both planes should be same\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    if(in0_height != (in1_height * 2U))
                    {
                        VX_PRINT(VX_ZONE_ERROR, " 'in_height' of chroma plane should be half of luma plane \n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    if(out0_height != (out1_height * 2U))
                    {
                        VX_PRINT(VX_ZONE_ERROR, "'out_height' of chroma plane should be half of luma plane \n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                    if((NULL != out2_img_desc) && (NULL != out3_img_desc))
                    {
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(out2_height != ((out3_height * 2U)))
                        /* LDRA_JUSTIFY_END */
                        {
                            VX_PRINT(VX_ZONE_ERROR, "'out_height' of chroma plane should be half of luma plane \n");
                            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        }
                    }

                }
            }
        }
        #endif

        if((vx_status)VX_SUCCESS == status)
        {
            /* Initialize LDC Config with defaults */
            Ldc_ConfigInit(ldc_cfg);

            /* Set up input and output image formats */
            ldc_obj->num_output = 1U;
            status = tivxVpacLdcSetFmt(ldc_prms, &ldc_cfg->inFmt, in0_img_desc);
        }
        #if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
       if((in0_img_desc != NULL) && (in1_img_desc != NULL))
       /* LDRA_JUSTIFY_END */
        {
            if(in0_img_desc->format != in1_img_desc->format)
            {
                if(((((vx_df_image)VX_DF_IMAGE_U8 == in0_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == in1_img_desc->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == in0_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == in1_img_desc->format))))
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)VX_DF_IMAGE_U8 == in0_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == in1_img_desc->format)))
                    {
                        ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS8_PACKED;
                        ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS12_PACKED;
                    }
                    else
                    {
                        ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
                        ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS8_PACKED;
                    }
                }
                else if(((((vx_df_image)TIVX_DF_IMAGE_P12 ==in0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == in1_img_desc->format)) ||
                        (((vx_df_image)VX_DF_IMAGE_U16 == in0_img_desc->format) &&
                        ((vx_df_image)TIVX_DF_IMAGE_P12 == in1_img_desc->format))))
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)TIVX_DF_IMAGE_P12 == in0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == in1_img_desc->format)))
                    {
                        ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_PACKED;
                        if(TIVX_VPAC_LDC_ALIGN_MSB == ldc_prms->input_align_12bit)
                        {
                            ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
                        }
                        else
                        {
                            ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS12_UNPACKED16;
                        }
                    }
                    else
                    {
                        if(TIVX_VPAC_LDC_ALIGN_MSB == ldc_prms->input_align_12bit)
                        {
                            ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
                        }
                        else
                        {
                            ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                        }
                        ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS12_PACKED;
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the host kernel before the control reaches here.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                else if(((((vx_df_image)VX_DF_IMAGE_U16 == in0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U8 == in1_img_desc->format)) ||
                        (((vx_df_image)VX_DF_IMAGE_U8 == in0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == in1_img_desc->format))))
                {
                    /* LDRA_JUSTIFY_END */
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)VX_DF_IMAGE_U16 == in0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U8 == in1_img_desc->format)))
                    {
                        if(TIVX_VPAC_LDC_ALIGN_MSB == ldc_prms->input_align_12bit)
                        {
                            ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
                        }
                        else
                        {
                            ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                        }
                        ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS8_PACKED;
                    }
                    else
                    {
                        ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS8_PACKED;
                        if(TIVX_VPAC_LDC_ALIGN_MSB == ldc_prms->input_align_12bit)
                        {
                            ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
                        }
                        else
                        {
                            ldc_cfg->indChPrms.ccsf = FVID2_CCSF_BITS12_UNPACKED16;
                        }
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                Unsupported input formats are rejected during OpenVX graph verification stage before process execution.
                The kernel validator function ensures only supported format combinations reach this code.
                Effect on this unit: This defensive else block is architecturally unreachable due to framework format validation.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    /* Unsupported input format combination */
                }
                /* LDRA_JUSTIFY_END */
                ldc_cfg->inFmt.pitch[0] = (uint32_t)in0_img_desc->imagepatch_addr[0].stride_y;
                ldc_cfg->inFmt.pitch[1] = (uint32_t)in1_img_desc->imagepatch_addr[0].stride_y;
                ldc_cfg->inFmt.width = in0_img_desc->imagepatch_addr[0].dim_x;
                ldc_cfg->inFmt.height = in0_img_desc->imagepatch_addr[0].dim_y;
                ldc_cfg->indChPrms.enable = (uint32_t)true;
                ldc_cfg->indChPrms.pitch = (uint32_t)in1_img_desc->imagepatch_addr[0].stride_y;
            }
            else
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the host kernel before the control reaches here.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(((((vx_df_image)TIVX_DF_IMAGE_P12 == in0_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == in1_img_desc->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == in0_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == in1_img_desc->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == in0_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == in1_img_desc->format))))
                    /* LDRA_JUSTIFY_END */
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->inFmt.dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((vx_df_image)VX_DF_IMAGE_U16 == in0_img_desc->format)
                    {
                        if(TIVX_VPAC_LDC_ALIGN_MSB == ldc_prms->input_align_12bit)
                        {
                            ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
                        }
                        else
                        {
                            ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                        }
                    }
                    ldc_cfg->inFmt.pitch[1] = (uint32_t)in1_img_desc->imagepatch_addr[0].stride_y;
                }
            }
        }
        #endif
        ldc_obj->sensor_dcc_id = ldc_prms->dcc_camera_id;
    }
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != in0_img_desc)
        /* LDRA_JUSTIFY_END */
        {
            if (((vx_df_image)VX_DF_IMAGE_U16 == in0_img_desc->format) &&
                (TIVX_VPAC_LDC_ALIGN_MSB == ldc_prms->input_align_12bit))
            {
                ldc_cfg->inFmt.ccsFormat = FVID2_CCSF_BITS12_UNPACKED16_MSB_ALIGNED;
            }
        }

        ldc_cfg->enableOutput[0U] = (uint32_t)UTRUE;
        status = tivxVpacLdcSetFmt(ldc_prms, &ldc_cfg->outFmt[0u], out0_img_desc);
        #if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((out0_img_desc != NULL) && (out1_img_desc != NULL) && (in1_img_desc != NULL))
        /* LDRA_JUSTIFY_END */
        {
            if(out0_img_desc->format != out1_img_desc->format)
            {
                if(((((vx_df_image)VX_DF_IMAGE_U8 == out0_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == out1_img_desc->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == out0_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == out1_img_desc->format))))
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)VX_DF_IMAGE_U8 == out0_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == out1_img_desc->format)))
                    {
                        ldc_cfg->outFmt[0].ccsFormat = FVID2_CCSF_BITS8_PACKED;
                        ldc_cfg->indOutChCcsf[0] = FVID2_CCSF_BITS12_PACKED;
                    }
                    else
                    {
                        ldc_cfg->outFmt[0].ccsFormat = FVID2_CCSF_BITS12_PACKED;
                        ldc_cfg->indOutChCcsf[0] = FVID2_CCSF_BITS8_PACKED;
                    }
                }
                else if(((((vx_df_image)TIVX_DF_IMAGE_P12 ==out0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == out1_img_desc->format)) ||
                        (((vx_df_image)VX_DF_IMAGE_U16 == out0_img_desc->format) &&
                        ((vx_df_image)TIVX_DF_IMAGE_P12 == out1_img_desc->format))))
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)TIVX_DF_IMAGE_P12 == out0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == out1_img_desc->format)))
                    {
                        ldc_cfg->outFmt[0].ccsFormat = FVID2_CCSF_BITS12_PACKED;
                        ldc_cfg->indOutChCcsf[0] = FVID2_CCSF_BITS12_UNPACKED16;
                    }
                    else
                    {
                        ldc_cfg->outFmt[0].ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                        ldc_cfg->indOutChCcsf[0] = FVID2_CCSF_BITS12_PACKED;
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the host kernel before the control reaches here.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                else if(((((vx_df_image)VX_DF_IMAGE_U16 == out0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U8 == out1_img_desc->format)) ||
                        (((vx_df_image)VX_DF_IMAGE_U8 == out0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == out1_img_desc->format))))
                {
                    /* LDRA_JUSTIFY_END */
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start> 
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The parameters are pre-validated by the caller before the control reaches here.
                    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    /* LDRA_JUSTIFY_END */
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)VX_DF_IMAGE_U16 == out0_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U8 == out1_img_desc->format)))
                    {
                        ldc_cfg->outFmt[0].ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                        ldc_cfg->indOutChCcsf[0] = FVID2_CCSF_BITS8_PACKED;
                    }
                    else
                    {
                        ldc_cfg->outFmt[0].ccsFormat = FVID2_CCSF_BITS8_PACKED;
                        ldc_cfg->indOutChCcsf[0] = FVID2_CCSF_BITS12_UNPACKED16;
                    }
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                Unsupported output formats are rejected during OpenVX graph verification stage before process execution.
                The kernel validator function ensures only supported format combinations reach this code.
                Effect on this unit: This defensive else block is architecturally unreachable due to framework format validation.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    /* Unsupported output format combination */
                }
                /* LDRA_JUSTIFY_END */
                ldc_cfg->outFmt[0].pitch[0] = (uint32_t)out0_img_desc->imagepatch_addr[0].stride_y;
                ldc_cfg->outFmt[0].pitch[1] = (uint32_t)out1_img_desc->imagepatch_addr[0].stride_y;
                ldc_cfg->outFmt[0].width = out0_img_desc->imagepatch_addr[0].dim_x;
                ldc_cfg->outFmt[0].height = out0_img_desc->imagepatch_addr[0].dim_y;
            }
            else
            {
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the host kernel before the control reaches here.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(((((vx_df_image)TIVX_DF_IMAGE_P12 == out0_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == in1_img_desc->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == out0_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == in1_img_desc->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == out0_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == in1_img_desc->format))))
                    /* LDRA_JUSTIFY_END */
                {
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_2PLANES;
                        }
                        else
                        {
                            ldc_cfg->outFmt[0].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    ldc_cfg->outFmt[0].pitch[1] = (uint32_t)out1_img_desc->imagepatch_addr[0].stride_y;
                }
            }
        }
        #endif
    }

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        if ((FVID2_DF_LUMA_ONLY == ldc_cfg->outFmt[0u].dataFormat) ||
            (FVID2_DF_CHROMA_ONLY == ldc_cfg->outFmt[0u].dataFormat))
        {
            ldc_cfg->inFmt.dataFormat = ldc_cfg->outFmt[0u].dataFormat;
        }

        if ((NULL != out1_img_desc) && (in1_img_desc == NULL))
        {
            ldc_cfg->enableOutput[1U] = (uint32_t)UTRUE;
            status = tivxVpacLdcSetFmt(ldc_prms, &ldc_cfg->outFmt[1u], out1_img_desc);
            ldc_obj->num_output = 2U;
        }
        #if defined(VPAC3) || defined(VPAC3L)
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((out2_img_desc != NULL) && (out3_img_desc != NULL) && (in1_img_desc != NULL))
        /* LDRA_JUSTIFY_END */
        {
            ldc_cfg->enableOutput[1U] = (uint32_t)UTRUE;
            status = tivxVpacLdcSetFmt(ldc_prms, &ldc_cfg->outFmt[1u], out2_img_desc);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if(out2_img_desc->format != out3_img_desc->format)
            {
                if(((((vx_df_image)VX_DF_IMAGE_U8 == out2_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == out3_img_desc->format)) ||
                    (((vx_df_image)TIVX_DF_IMAGE_P12 == out2_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == out3_img_desc->format))))
                {
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_CHROMA_ONLY)
                        {
                            ldc_cfg->outFmt[1].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)VX_DF_IMAGE_U8 == out2_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == out3_img_desc->format)))
                    {
                        ldc_cfg->outFmt[1].ccsFormat = FVID2_CCSF_BITS8_PACKED;
                        ldc_cfg->indOutChCcsf[1] = FVID2_CCSF_BITS12_PACKED;
                    }
                    else
                    {
                        ldc_cfg->outFmt[1].ccsFormat = FVID2_CCSF_BITS12_PACKED;
                        ldc_cfg->indOutChCcsf[1] = FVID2_CCSF_BITS8_PACKED;
                    }
                }
                else if(((((vx_df_image)TIVX_DF_IMAGE_P12 ==out2_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == out3_img_desc->format)) ||
                        (((vx_df_image)VX_DF_IMAGE_U16 == out2_img_desc->format) &&
                        ((vx_df_image)TIVX_DF_IMAGE_P12 == out3_img_desc->format))))
                {
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_CHROMA_ONLY)
                        {
                            ldc_cfg->outFmt[1].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)TIVX_DF_IMAGE_P12 == out2_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == out3_img_desc->format)))
                    {
                        ldc_cfg->outFmt[1].ccsFormat = FVID2_CCSF_BITS12_PACKED;
                        ldc_cfg->indOutChCcsf[1] = FVID2_CCSF_BITS12_UNPACKED16;
                    }
                    else
                    {
                        ldc_cfg->outFmt[1].ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                        ldc_cfg->indOutChCcsf[1] = FVID2_CCSF_BITS12_PACKED;
                    }
                }
                else if(((((vx_df_image)VX_DF_IMAGE_U16 == out2_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U8 == out3_img_desc->format)) ||
                        (((vx_df_image)VX_DF_IMAGE_U8 == out2_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U16 == out3_img_desc->format))))
                {
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_CHROMA_ONLY)
                        {
                            ldc_cfg->outFmt[1].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    if((((vx_df_image)VX_DF_IMAGE_U16 == out2_img_desc->format) &&
                        ((vx_df_image)VX_DF_IMAGE_U8 == out3_img_desc->format)))
                    {
                        ldc_cfg->outFmt[1].ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                        ldc_cfg->indOutChCcsf[1] = FVID2_CCSF_BITS8_PACKED;
                    }
                    else
                    {
                        ldc_cfg->outFmt[1].ccsFormat = FVID2_CCSF_BITS8_PACKED;
                        ldc_cfg->indOutChCcsf[1] = FVID2_CCSF_BITS12_UNPACKED16;
                    }
                }
                else
                {
                    /* Unsupported output format combination for second output */
                }
                ldc_cfg->outFmt[1].pitch[0] = (uint32_t)out2_img_desc->imagepatch_addr[0].stride_y;
                ldc_cfg->outFmt[1].pitch[1] = (uint32_t)out3_img_desc->imagepatch_addr[0].stride_y;
                ldc_cfg->outFmt[1].width = out2_img_desc->imagepatch_addr[0].dim_x;
                ldc_cfg->outFmt[1].height = out3_img_desc->imagepatch_addr[0].dim_y;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                /* LDRA_JUSTIFY_START
                <metric start>  statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The parameters are pre-validated by the host kernel before the control reaches here.
                Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(((((vx_df_image)TIVX_DF_IMAGE_P12 == out2_img_desc->format) &&
                    ((vx_df_image)TIVX_DF_IMAGE_P12 == out3_img_desc->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U8 == out2_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U8 == out3_img_desc->format)) ||
                    (((vx_df_image)VX_DF_IMAGE_U16 == out2_img_desc->format) &&
                    ((vx_df_image)VX_DF_IMAGE_U16 == out3_img_desc->format))))
                /* LDRA_JUSTIFY_END */
                {
                    if(ldc_prms->in_img0_yc_mode == TIVX_VPAC_LDC_MODE_LUMA_ONLY)
                    {
                        if(ldc_prms->in_img1_yc_mode == TIVX_VPAC_LDC_MODE_CHROMA_ONLY)
                        {
                            ldc_cfg->outFmt[1].dataFormat = FVID2_DF_YUV420SP_UV;
                        }
                    }
                    ldc_cfg->outFmt[1].pitch[1] = (uint32_t)out3_img_desc->imagepatch_addr[0].stride_y;
                }
            }
            ldc_obj->num_output = 2U;
        }
        #endif
    }
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* By default back mapping is disabled */
        ldc_cfg->enableBackMapping = (uint32_t)UFALSE;

        /* Set Luma interpolation type */
        if (TIVX_VPAC_LDC_INTERPOLATION_BICUBIC == ldc_prms->luma_interpolation_type)
        {
            ldc_cfg->lumaIntrType = VHWA_LDC_LUMA_INTRP_BICUBIC;
        }
        else
        {
            ldc_cfg->lumaIntrType = VHWA_LDC_LUMA_INTRP_BILINEAR;
        }
        ldc_cfg->outputStartX = ldc_prms->init_x;
        ldc_cfg->outputStartY = ldc_prms->init_y;
#if defined(VPAC3) || defined(VPAC3L)
        /* Set continuous mode from TIVX params */
        ldc_cfg->continuousMode = ldc_prms->continuous_mode;
#endif
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != out0_img_desc)
        /* LDRA_JUSTIFY_END */
        {
            ldc_cfg->outputFrameWidth = out0_img_desc->imagepatch_addr[0U].dim_x;
            ldc_cfg->outputFrameHeight = out0_img_desc->imagepatch_addr[0U].dim_y;
        }

        if (NULL != dcc_buf_desc)
        {
            status = tivxVpacLdcSetParamsFromDcc(ldc_obj, dcc_buf_desc);
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
                fvid2_status = Fvid2_control(ldc_obj->handle,
                    IOCTL_VHWA_M2M_LDC_SET_PARAMS, (void*)&ldc_obj->ldc_cfg, NULL);
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
                    VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params \n");
                    status = (vx_status)VX_FAILURE;
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
                VX_PRINT(VX_ZONE_ERROR, "TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS returned 0x%x\n", status);
            }
            /* LDRA_JUSTIFY_END */ 
        }
        else
        {
            status = tivxVpacLdcSetMeshParams(ldc_cfg, mesh_prms_desc, mesh_img_desc);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to skip LDC configuration setup and propagate the error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                tivxVpacLdcSetRegionParams(ldc_cfg, reg_prms_desc);

                tivxVpacLdcSetAffineConfig(&ldc_cfg->perspTrnsformCfg,
                        warp_matrix_desc);
            }
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to skip LDC configuration setup and propagate the error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            fvid2_status = Fvid2_control(ldc_obj->handle,
                IOCTL_VHWA_M2M_LDC_SET_PARAMS, &ldc_obj->ldc_cfg, NULL);
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
                VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params \n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
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
        status = tivxSetTargetKernelInstanceContext(kernel, ldc_obj,
            sizeof(tivxVpacLdcObj));
    }
    if ((vx_status)VX_SUCCESS != status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != ldc_obj)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != ldc_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(ldc_obj->handle, NULL);
                ldc_obj->handle = NULL;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != ldc_obj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&ldc_obj->waitForProcessCmpl);
            }

            tivxVpacLdcFreeObject(&gTivxVpacLdcInstObj, ldc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxVpacLdcObj *ldc_obj = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( ((num_params != TIVX_KERNEL_VPAC_LDC_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_IN0_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC_OUT0_IMG_IDX]))
        && ((num_params != TIVX_KERNEL_VPAC_LDC2_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_IN0_IMG_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_VPAC_LDC2_OUT0_IMG_IDX])))
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
        Effect on this unit: If the control reaches here, ldc_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            ldc_obj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
        This behaviour is part of the application design. An error accumulation statement can be added in a future release.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) && (NULL != ldc_obj) &&
            (sizeof(tivxVpacLdcObj) == size))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging.
            Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != ldc_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(ldc_obj->handle, NULL);
                ldc_obj->handle = NULL;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging.
            Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != ldc_obj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&ldc_obj->waitForProcessCmpl);
            }

            /* Free config register readback buffers if allocated */
            if ((ldc_obj->readback_mem_ptr_phys != 0u) || (ldc_obj->golden_reg_mem_ptr_phys != 0u))
            {
                tivxVpacLdcFreeReadbackBuffers(ldc_obj);
            }

            tivxVpacLdcFreeObject(&gTivxVpacLdcInstObj, ldc_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacLdcControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacLdcObj                   *ldc_obj = NULL;
    int32_t                          fvid2_status = FVID2_SOK;

    void *temp_kernel_context = NULL;
    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The kernel parameter is expected to be pre-validated by the OpenVX framework before kernel callback invocation.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, ldc_obj would not be assigned and subsequent NULL checks would catch the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        ldc_obj = temp_kernel_context;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((vx_status)VX_SUCCESS == status) && (NULL != ldc_obj) &&
        (sizeof(tivxVpacLdcObj) == size))
    /* LDRA_JUSTIFY_END */
    {
        status = (vx_status)VX_SUCCESS;
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
        VX_PRINT(VX_ZONE_ERROR, "Failed to get Target Kernel\n");
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
        switch (node_cmd_id)
        {
            case TIVX_VPAC_LDC_CMD_SET_READ_BW_LIMIT_PARAMS:
            {
                status = tivxVpacLdcSetRdBwLimitCmd(ldc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_LDC_CMD_SET_BIT_DEPTH_CONV_LUT_PARAMS:
            {
                status = tivxVpacLdcSetLutParamsCmd(ldc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U],
                    (tivx_obj_desc_user_data_object_t *)obj_desc[1U]);
                break;
            }
            case TIVX_VPAC_LDC_CMD_GET_ERR_STATUS:
            {
                status = tivxVpacLdcGetErrStatusCmd(ldc_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_LDC_CMD_SET_LDC_PARAMS:
            {
                status = tivxVpacLdcSetParams(ldc_obj, obj_desc);
                break;
            }
            case TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS:
            {
                status = (vx_status)VX_SUCCESS;
                tivx_obj_desc_user_data_object_t *dcc_buf_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[0];
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != dcc_buf_desc)
                /* LDRA_JUSTIFY_END */
                {
                    status = tivxVpacLdcSetParamsFromDcc(ldc_obj, dcc_buf_desc);
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(status == (vx_status) (vx_status) VX_SUCCESS)
                    /* LDRA_JUSTIFY_END */
                    {
                        fvid2_status = Fvid2_control(ldc_obj->handle,
                            IOCTL_VHWA_M2M_LDC_SET_PARAMS, (void*)&ldc_obj->ldc_cfg, NULL);
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
                            VX_PRINT(VX_ZONE_ERROR, "Fvid2_control Failed: Set Params \n");
                            status = (vx_status)VX_FAILURE;
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
                        VX_PRINT(VX_ZONE_ERROR, "tivxVpacLdcSetParamsFromDcc returned 0x%x\n", status);
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
                    VX_PRINT(VX_ZONE_ERROR, "TIVX_VPAC_LDC_CMD_SET_LDC_DCC_PARAMS : dcc_buf_desc is NULL\n", status);
                    status = (vx_status)VX_FAILURE;
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            case TIVX_VPAC_LDC_CMD_GET_PSA_STATUS:
            {
                status = tivxVpacLdcGetPsaStatusCmd(ldc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_LDC_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxVpacLdcEnableErrorEventsCmd(ldc_obj,
                  (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
#if defined(LDRA_COVERAGE)
            case TIVX_VPAC_CMD_GET_COVERAGE_START:
            {
                #if defined (C7X_FAMILY) || defined(R5F)
                VX_PRINT(VX_ZONE_ERROR, "################### DYNAMIC CODE COVERAGE INITIALIZATION STARTED ######################\n");
                ldra_initialize();
                VX_PRINT(VX_ZONE_ERROR, "################### DYNAMIC CODE COVERAGE INITIALIZATION FINISHED ######################\n");
                #endif  
                break;
            }
#endif

            case TIVX_VPAC_LDC_CMD_ENABLE_VPAC_SAFETY_MECHANISM:
            {
                status = tivxEnableVpacLdcSafetyMechanisms(ldc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0]);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to enable VPAC LDC safety mechanisms\n");
                }
                /* LDRA_JUSTIFY_END */
                break;
            }

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            All valid control commands are explicitly defined in the switch cases above.
            The OpenVX framework validates control command IDs against registered kernel capabilities before invoking the kernel control callback.
            Unregistered or invalid command IDs are rejected at the framework dispatcher level, preventing them from reaching this default case.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Node Cmd Id\n");
                status = (vx_status)VX_FAILURE;
                break;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacLdcObj *tivxVpacLdcAllocObject(tivxVpacLdcInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacLdcObj *ldc_obj = NULL;
    vx_enum self_cpu;
    vx_status status;

    self_cpu = tivxGetSelfCpuId();

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
        <metric start>  branch <metric end>
        <justification start> 
        Rationale:The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 8 LDC handles
        Effect on this unit: It will utilise all the 8 LDC handles which are validated in the LDC driver.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (0U == instObj->ldc_obj[cnt].isAlloc)
            {
                ldc_obj = &instObj->ldc_obj[cnt];
                memset(ldc_obj, 0x0, sizeof(tivxVpacLdcObj));
                instObj->ldc_obj[cnt].isAlloc = 1U;
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
                /* LDRA_JUSTIFY_END */
                {
                    instObj->ldc_obj[cnt].ldc_drv_inst_id = VHWA_M2M_LDC_DRV_INST_ID;
                    instObj->ldc_obj[cnt].hwa_perf_id     = APP_PERF_HWA_VPAC1_LDC;
                }
                #if (VPAC_COUNT > 1)
                else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
                {
                    instObj->ldc_obj[cnt].ldc_drv_inst_id = VHWA_M2M_VPAC_1_LDC_DRV_INST_ID_0;
                    instObj->ldc_obj[cnt].hwa_perf_id     = APP_PERF_HWA_VPAC2_LDC;
                }
                #endif
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    /* Unsupported CPU ID */
                    VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                    ldc_obj = NULL;
                }
                /* LDRA_JUSTIFY_END */
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
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to log the mutex lock failure for diagnostic purposes.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }
    /* LDRA_JUSTIFY_END */

    return (ldc_obj);
}

static void tivxVpacLdcFreeObject(tivxVpacLdcInstObj *instObj,
    tivxVpacLdcObj *ldc_obj)
{
    uint32_t cnt;
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
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
        <metric start>  branch <metric end>
        <justification start>
        Rationale:The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 8 LDC handles
        Effect on this unit: It will free all the 8 LDC handles which are validated in the LDC driver.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_LDC_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (ldc_obj == &instObj->ldc_obj[cnt])
            {
                ldc_obj->isAlloc = 0U;
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

static vx_status tivxVpacLdcSetFmt(const tivx_vpac_ldc_params_t *ldc_prms,
    Fvid2_Format *fmt, const tivx_obj_desc_image_t *img_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
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
            case (vx_df_image)VX_DF_IMAGE_UYVY:
            {
                fmt->dataFormat = FVID2_DF_YUV422I_UYVY;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_YUYV:
            {
                fmt->dataFormat = FVID2_DF_YUV422I_YUYV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_NV12:
            {
                fmt->dataFormat = FVID2_DF_YUV420SP_UV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U8:
            {
                if (TIVX_VPAC_LDC_MODE_LUMA_ONLY == ldc_prms->yc_mode)
                {
                    fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    fmt->dataFormat = FVID2_DF_CHROMA_ONLY;
                }
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
                break;
            }
            case (vx_df_image)VX_DF_IMAGE_U16:
            {
                if (TIVX_VPAC_LDC_MODE_LUMA_ONLY == ldc_prms->yc_mode)
                {
                    fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    fmt->dataFormat = FVID2_DF_CHROMA_ONLY;
                }
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                break;
            }
            case (vx_df_image)TIVX_DF_IMAGE_P12:
            {
                if (TIVX_VPAC_LDC_MODE_LUMA_ONLY == ldc_prms->yc_mode)
                {
                    fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    fmt->dataFormat = FVID2_DF_CHROMA_ONLY;
                }
                fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
                break;
            }
            case (vx_df_image)TIVX_DF_IMAGE_NV12_P12:
            {
                fmt->dataFormat = FVID2_DF_YUV420SP_UV;
                fmt->ccsFormat = FVID2_CCSF_BITS12_PACKED;
                break;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to print the error.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            default:
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, "Invalid Vx Image Format\n");
                break;
            }
            /* LDRA_JUSTIFY_END */
        }

        fmt->width      = img_desc->imagepatch_addr[0].dim_x;
        fmt->height     = img_desc->imagepatch_addr[0].dim_y;

        /*  In case of chroma only mode, LDC assumes chroma is from
         *  YUV420 input and expects the frame size
         *  to be for YUV420 frame, ie frame size of luma, where chroma
         *  height is half of luma height.
         *  Node internally takes care of multiplying height by 2. */
        if (FVID2_DF_CHROMA_ONLY == fmt->dataFormat)
        {
            fmt->height = fmt->height * 2U;
        }

        fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
        fmt->pitch[1]   = (uint32_t)img_desc->imagepatch_addr[1].stride_y;
    }

    return status;
}

static void tivxVpacLdcSetAffineConfig(Ldc_PerspectiveTransformCfg *cfg,
    const tivx_obj_desc_matrix_t *warp_matrix_desc)
{
    void *warp_matrix_target_ptr;
    vx_float32 *mat_addr;
    vx_status                    status = (vx_status)VX_SUCCESS;

    if (NULL != warp_matrix_desc)
    {
        warp_matrix_target_ptr = tivxMemShared2TargetPtr(&warp_matrix_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        /* Direct pass to HW registers */
        if ((vx_enum)VX_TYPE_INT16 == warp_matrix_desc->data_type)
        {
            int16_t *mat_addr_int16;

            mat_addr_int16 = (int16_t *)warp_matrix_target_ptr;

            if(3U == warp_matrix_desc->columns)
            {
                cfg->coeffA     = mat_addr_int16[0];
                cfg->coeffB     = mat_addr_int16[3];
                cfg->coeffC     = mat_addr_int16[6];
                cfg->coeffD     = mat_addr_int16[1];
                cfg->coeffE     = mat_addr_int16[4];
                cfg->coeffF     = mat_addr_int16[7];
                cfg->coeffG     = mat_addr_int16[2];
                cfg->coeffH     = mat_addr_int16[5];
                cfg->enableWarp = 1U;
            }
            else
            {
                cfg->coeffA     = mat_addr_int16[0];
                cfg->coeffB     = mat_addr_int16[2];
                cfg->coeffC     = mat_addr_int16[4];
                cfg->coeffD     = mat_addr_int16[1];
                cfg->coeffE     = mat_addr_int16[3];
                cfg->coeffF     = mat_addr_int16[5];
                cfg->coeffG     = 0;
                cfg->coeffH     = 0;
                cfg->enableWarp = 0U;
            }
        }
        else /* Compute HW registers from floating point warp matrix */
        {
            mat_addr = (vx_float32 *)warp_matrix_target_ptr;

            if(3u == warp_matrix_desc->columns)
            {
                vx_float32 temp_coeffA = (mat_addr[0] / mat_addr[8]) * 4096.0f;
                cfg->coeffA       = (int16_t)temp_coeffA;
                vx_float32 temp_coeffB = (mat_addr[3] / mat_addr[8]) * 4096.0f;
                cfg->coeffB       = (int16_t)temp_coeffB;
                vx_float32 temp_coeffC = (mat_addr[6] / mat_addr[8]) * 8.0f;
                cfg->coeffC       = (int16_t)temp_coeffC;
                vx_float32 temp_coeffD = (mat_addr[1] / mat_addr[8]) * 4096.0f;
                cfg->coeffD       = (int16_t)temp_coeffD;
                vx_float32 temp_coeffE = (mat_addr[4] / mat_addr[8]) * 4096.0f;
                cfg->coeffE       = (int16_t)temp_coeffE;
                vx_float32 temp_coeffF = (mat_addr[7] / mat_addr[8]) * 8.0f;
                cfg->coeffF       = (int16_t)temp_coeffF;
                vx_float32 temp_coeffG = (mat_addr[2] / mat_addr[8]) * 8388608.0f;
                cfg->coeffG       = (int16_t)temp_coeffG;
                vx_float32 temp_coeffH = (mat_addr[5] / mat_addr[8]) * 8388608.0f;
                cfg->coeffH       = (int16_t)temp_coeffH;
                cfg->enableWarp   = 1U;
            }
            else
            {
                vx_float32 temp_coeffA = mat_addr[0] * 4096.0f;
                cfg->coeffA       = (int16_t)temp_coeffA;
                vx_float32 temp_coeffB = mat_addr[2] * 4096.0f;
                cfg->coeffB       = (int16_t)temp_coeffB;
                vx_float32 temp_coeffC = mat_addr[4] * 8.0f;
                cfg->coeffC       = (int16_t)temp_coeffC;
                vx_float32 temp_coeffD = mat_addr[1] * 4096.0f;
                cfg->coeffD       = (int16_t)temp_coeffD;
                vx_float32 temp_coeffE = mat_addr[3] * 4096.0f;
                cfg->coeffE       = (int16_t)temp_coeffE;
                vx_float32 temp_coeffF = mat_addr[5] * 8.0f;
                cfg->coeffF       = (int16_t)temp_coeffF;
                cfg->coeffG       = 0;
                cfg->coeffH       = 0;
                cfg->enableWarp   = 0U;
            }
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(warp_matrix_target_ptr, warp_matrix_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    }
    else
    {
        cfg->coeffA     = 4096;
        cfg->coeffB     = 0;
        cfg->coeffC     = 0;
        cfg->coeffD     = 0;
        cfg->coeffE     = 4096;
        cfg->coeffF     = 0;
        cfg->coeffG     = 0;
        cfg->coeffH     = 0;
        cfg->enableWarp = 0U;
    }
}

static vx_status tivxVpacLdcSetMeshParams(Ldc_Config *ldc_cfg,
    const tivx_obj_desc_user_data_object_t *mesh_prms_desc,
    const tivx_obj_desc_image_t *mesh_img_desc)
{
    vx_status                    status = (vx_status)VX_SUCCESS;
    tivx_vpac_ldc_mesh_params_t *mesh_prms = NULL;
    void                        *target_ptr;
    Ldc_LutCfg                  *lut_cfg = NULL;

    if ((NULL != mesh_prms_desc) && (NULL != mesh_img_desc))
    {
        target_ptr = tivxMemShared2TargetPtr(&mesh_prms_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, mesh_prms_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The parameters are pre-validated by the host kernel before the control reaches here.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
        if (sizeof(tivx_vpac_ldc_mesh_params_t) == mesh_prms_desc->mem_size)
        /* LDRA_JUSTIFY_END */
        {
            mesh_prms = (tivx_vpac_ldc_mesh_params_t *)target_ptr;
            lut_cfg   = &ldc_cfg->lutCfg;

            lut_cfg->address    = tivxMemShared2PhysPtr(mesh_img_desc->mem_ptr[0].shared_ptr,
                (int32_t)mesh_img_desc->mem_ptr[0].mem_heap_region);
            lut_cfg->lineOffset = (uint32_t)mesh_img_desc->imagepatch_addr[0].stride_y;
            lut_cfg->dsFactor   = mesh_prms->subsample_factor;
            lut_cfg->width      = mesh_prms->mesh_frame_width;
            lut_cfg->height     = mesh_prms->mesh_frame_height;

            /* Back mapping is enabled, if the mesh params & mesh
             * image are non-null */
            ldc_cfg->enableBackMapping = (uint32_t)UTRUE;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are pre-validated by the host kernel before the control reaches here.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, mesh_prms_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        /* Disable back mapping if one of the mesh params or mesh
         * image is null */
        ldc_cfg->enableBackMapping = (uint32_t)UFALSE;
    }

    return (status);
}

static void tivxVpacLdcSetRegionParams(Ldc_Config *cfg,
    const tivx_obj_desc_user_data_object_t *reg_prms_desc)
{
    void                                *target_ptr;
    uint32_t                             cnt1, cnt2;
    tivx_vpac_ldc_region_params_t       *reg_prms = NULL;
    tivx_vpac_ldc_multi_region_params_t *mreg_prms = NULL;
    vx_status                    status = (vx_status)VX_SUCCESS;

    if (NULL != reg_prms_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&reg_prms_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, reg_prms_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_ldc_region_params_t) ==
                reg_prms_desc->mem_size)
        {
            reg_prms = (tivx_vpac_ldc_region_params_t*)target_ptr;

            cfg->enableMultiRegions = (uint32_t)UFALSE;

            cfg->outputBlockWidth  = reg_prms->out_block_width;
            cfg->outputBlockHeight = reg_prms->out_block_height;
            cfg->pixelPad          = reg_prms->pixel_pad;
        }
        else
        {
            mreg_prms = (tivx_vpac_ldc_multi_region_params_t*)target_ptr;

            cfg->enableMultiRegions = (uint32_t)UTRUE;

            for (cnt1 = 0u; cnt1 < LDC_MAX_HORZ_REGIONS; cnt1 ++)
            {
                cfg->regCfg.width[cnt1] = mreg_prms->reg_width[cnt1];
            }

            for (cnt1 = 0u; cnt1 < LDC_MAX_VERT_REGIONS; cnt1 ++)
            {
               cfg->regCfg.height[cnt1] = mreg_prms->reg_height[cnt1];
            }

            for (cnt1 = 0u; cnt1 < LDC_MAX_VERT_REGIONS; cnt1 ++)
            {
                for (cnt2 = 0u; cnt2 < LDC_MAX_HORZ_REGIONS; cnt2 ++)
                {
                    reg_prms = &mreg_prms->reg_params[cnt1][cnt2];

                    cfg->regCfg.enable[cnt1][cnt2] = reg_prms->enable;

                    cfg->regCfg.blockWidth[cnt1][cnt2]  =
                        reg_prms->out_block_width;
                    cfg->regCfg.blockHeight[cnt1][cnt2] =
                        reg_prms->out_block_height;
                    cfg->regCfg.pixelPad[cnt1][cnt2]    = reg_prms->pixel_pad;
                }
            }
        }
    }
    else
    {
        cfg->outputBlockWidth = TIVX_VPAC_LDC_DEF_BLOCK_WIDTH;
        cfg->outputBlockHeight = TIVX_VPAC_LDC_DEF_BLOCK_HEIGHT;
        cfg->pixelPad = TIVX_VPAC_LDC_DEF_PIXEL_PAD;
    }
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
static vx_status tivxVpacLdcMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
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
    else if ((0u != (vhwa_err_events & ~VHWA_LDC_ALL_ERR_EVENTS)) ||
             (0u != (wd_timer_err & ~VHWA_LDC_WDTIMER_ERR)))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask provided\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Map VHWA flags to TIVX flags for regular error interrupts */
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, the else branch would map VHWA error flags to TIVX error flags for valid hardware error events.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_LDC_PIX_IBLK_OUTOFBOUND_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_PIX_BLK_OUTOFBOUND_ERR_STATUS;
        }
        if (0u != (vhwa_err_events & VHWA_LDC_MESH_IBLK_OUTOFBOUND))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_MESH_BLK_OUTOFBOUND_ERR_STATUS;
        }
        if (0u != (vhwa_err_events & VHWA_LDC_PIX_IBLK_MEMOVF))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_PIX_MEM_OVF_ERR_STATUS;
        }
        if (0u != (vhwa_err_events & VHWA_LDC_MESH_IBLK_MEMOVF))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_MESH_MEM_OVF_ERR_STATUS;
        }
        /* LDRA_JUSTIFY_END_DISABLED */
        if (0u != (vhwa_err_events & VHWA_LDC_IFR_OUTOFBOUND))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_PIX_FRM_OUTOFBOUND_ERR_STATUS;
        }
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, the else branch would map VHWA error flags to TIVX error flags for valid hardware error events.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_LDC_INT_SZOVF))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_SZ_OVF_ERR_STATUS;
        }
        if (0u != (vhwa_err_events & VHWA_LDC_SL2_WR_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_SL2_WR_ERR_STATUS;
        }
        if (0u != (vhwa_err_events & VHWA_LDC_VBUSM_RD_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_RD_ERR_STATUS;
        }

        /* Map VHWA flags to TIVX flags for watchdog timer errors */
        if (0u != (wd_timer_err & VHWA_LDC_WDTIMER_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_LDC_WDTIMER_ERR;
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
static vx_status tivxVpacLdcMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
                                               uint32_t *vhwa_err_events,
                                               uint32_t *wd_timer_err)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t enableErrorEventCheckMask = TIVX_VPAC_LDC_ALL_ERR_EVENTS;

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
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_PIX_BLK_OUTOFBOUND_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_PIX_IBLK_OUTOFBOUND_ERR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_MESH_BLK_OUTOFBOUND_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_MESH_IBLK_OUTOFBOUND;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_PIX_MEM_OVF_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_PIX_IBLK_MEMOVF;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_MESH_MEM_OVF_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_MESH_IBLK_MEMOVF;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_PIX_FRM_OUTOFBOUND_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_IFR_OUTOFBOUND;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_SZ_OVF_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_INT_SZOVF;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_SL2_WR_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_SL2_WR_ERR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_RD_ERR_STATUS))
        {
            *vhwa_err_events |= VHWA_LDC_VBUSM_RD_ERR;
        }

        /* Map TIVX flags to VHWA flags for watchdog timer errors */
        if (0u != (tivx_err_events & TIVX_VPAC_LDC_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_LDC_WDTIMER_ERR;
        }
        /* LDRA_JUSTIFY_END_DISABLED */
    }

    return status;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */


static vx_status tivxVpacLdcSetRdBwLimitCmd(tivxVpacLdcObj *ldc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    tivx_vpac_ldc_bandwidth_params_t *bwPrms = NULL;
    void                             *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_ldc_bandwidth_params_t) ==
                usr_data_obj->mem_size)
        {
            bwPrms = (tivx_vpac_ldc_bandwidth_params_t *)target_ptr;

            ldc_obj->rdBwLimitCfg.rdBwLimit = bwPrms->bandwidth_control;
            ldc_obj->rdBwLimitCfg.rdTagCnt = bwPrms->tag_count;
            ldc_obj->rdBwLimitCfg.rdMaxBurstLength =
                bwPrms->max_burst_length;

            status = Fvid2_control(ldc_obj->handle,
                IOCTL_VHWA_M2M_LDC_SET_RD_BW_LIMIT,
                &ldc_obj->rdBwLimitCfg, NULL);
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set BW Params \n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                status = (vx_status)VX_SUCCESS;
            }
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxVpacLdcSetLutParamsCmd(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_user_data_object_t *luma_user_desc,
    tivx_obj_desc_user_data_object_t *chroma_user_desc)
{
    vx_status                         status = (vx_status)VX_SUCCESS;

    int32_t                           fvid2_status = FVID2_SOK;
    tivx_vpac_ldc_bit_depth_conv_lut_params_t *lutPrms = NULL;
    void                             *target_ptr;

    if (NULL != luma_user_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&luma_user_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, luma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t) ==
                luma_user_desc->mem_size)
        {
            lutPrms = (tivx_vpac_ldc_bit_depth_conv_lut_params_t *)target_ptr;

            ldc_obj->lut_cfg.enable = 1U;
            ldc_obj->lut_cfg.inputBits = lutPrms->input_bits;
            ldc_obj->lut_cfg.outputBits = lutPrms->output_bits;
            ldc_obj->lut_cfg.tableAddr = lutPrms->lut;

            fvid2_status = Fvid2_control(ldc_obj->handle,
                IOCTL_LDC_SET_LUMA_TONEMAP_LUT_CFG, &ldc_obj->lut_cfg, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((int32_t)FVID2_SOK != (int32_t)fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set Luma Lut\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, luma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    if ((NULL != chroma_user_desc) && ((vx_status)VX_SUCCESS == status))
    {
        target_ptr = tivxMemShared2TargetPtr(&chroma_user_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, chroma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_ldc_bit_depth_conv_lut_params_t) ==
                chroma_user_desc->mem_size)
        {
            lutPrms = (tivx_vpac_ldc_bit_depth_conv_lut_params_t *)target_ptr;

            ldc_obj->lut_cfg.enable = 1U;
            ldc_obj->lut_cfg.inputBits = lutPrms->input_bits;
            ldc_obj->lut_cfg.outputBits = lutPrms->output_bits;
            ldc_obj->lut_cfg.tableAddr = lutPrms->lut;

            fvid2_status = Fvid2_control(ldc_obj->handle,
                IOCTL_LDC_SET_CHROMA_TONEMAP_LUT_CFG, &ldc_obj->lut_cfg, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((int32_t)FVID2_SOK != (int32_t)fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set Chroma Lut\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, chroma_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    return (status);
}

static vx_status tivxVpacLdcGetErrStatusCmd(const tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat = 0u;

    if (NULL != scalar_obj_desc)
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxVpacLdcMapVhwaToTivxErrEvents(ldc_obj->err_stat, ldc_obj->wdTimerErrStatus, &combined_err_stat);
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
        VX_PRINT(VX_ZONE_ERROR, "Null argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return (status);
}

static vx_status tivxVpacLdcSetParams(tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_t *obj_desc[])
{
    int32_t fvid2_status = FVID2_SOK;
    vx_status status = (vx_status)VX_FAILURE;
    tivx_obj_desc_matrix_t *warp_matrix_desc = NULL;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != obj_desc[TIVX_VPAC_LDC_SET_PARAMS_WARP_MATRIX_IDX])
    /* LDRA_JUSTIFY_END */
    {
        warp_matrix_desc = (tivx_obj_desc_matrix_t *)
            obj_desc[TIVX_VPAC_LDC_SET_PARAMS_WARP_MATRIX_IDX];

        tivxVpacLdcSetAffineConfig(&ldc_obj->ldc_cfg.perspTrnsformCfg,
                warp_matrix_desc);

        status = (vx_status)VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    justification start> Passing only VX_SUCCESS case for the status. 
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        fvid2_status = Fvid2_control(ldc_obj->handle,
            IOCTL_VHWA_M2M_LDC_SET_PARAMS, &ldc_obj->ldc_cfg, NULL);
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
                "tivxVpacLdcSetParams: Fvid2_control Failed: Set Params \n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
}

static vx_status tivxVpacLdcGetPsaStatusCmd(const tivxVpacLdcObj *ldc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    Vhwa_M2mLdcPsaSign                  psa;
    int32_t                             fvid2_status = FVID2_SOK;
    void                                *target_ptr;
    tivx_vpac_ldc_psa_timestamp_data_t  *psa_status_ptr;

    if (NULL != usr_data_obj)
    {
        if (sizeof(tivx_vpac_ldc_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            fvid2_status = Fvid2_control(ldc_obj->handle,
                VHWA_M2M_IOCTL_LDC_GET_PSA_SIGN, &psa, NULL);
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
                    "tivxVpacLdcGetPsa: Fvid2_control Failed\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            /* Map the user data object to access the memory */
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            psa_status_ptr = (tivx_vpac_ldc_psa_timestamp_data_t *)target_ptr;

            /* Copy driver data (PSA values) into the structure */
            tivx_obj_desc_memcpy(&psa_status_ptr->psa_values, psa.psaSign, sizeof(psa.psaSign));

            /* Add timestamp to the structure */
            psa_status_ptr->timestamp = ldc_obj->timestamp;
            
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

    return (status);
}

static vx_status tivxVpacLdcEnableErrorEventsCmd(tivxVpacLdcObj *ldc_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t intr_en = 0u;
    uint32_t wd_timer_intr_en = 0u;
    void *target_ptr;
    tivx_vpac_ldc_params_t *params_ptr = NULL;
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
        if(sizeof(tivx_vpac_ldc_params_t) != usr_data_obj->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVpacLdcEnableErrorEventsCmd: Incorrect Data Object Size \n");
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
                    "tivxVpacLdcEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_vpac_ldc_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->enable_error_events;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxVpacLdcMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

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
                        "tivxVpacLdcEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    ldc_obj->errEvtPrms.errEvents = intr_en;
                    fvid2_status = Fvid2_control(ldc_obj->handle,
                        IOCTL_VHWA_M2M_LDC_REGISTER_ERR_CB, &ldc_obj->errEvtPrms, NULL);

                    if (FVID2_SOK == fvid2_status)
                    {
                        /* Update WDTimer configuration from application */
                        ldc_obj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->wdtimer_timeout_cycles;
                        ldc_obj->wdTimerErrEvtPrms.wdTimerErrEvents = wd_timer_intr_en;
                        fvid2_status = Fvid2_control(ldc_obj->handle,
                            VHWA_M2M_IOCTL_LDC_REGISTER_WDTIMER_ERR_CB, &ldc_obj->wdTimerErrEvtPrms, NULL);
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case requires hardware driver failure that cannot be simulated in test environment.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR,
                            "tivxVpacLdcEnableErrorEventsCmd: Fvid2_control Failed \n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
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
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacLdcEnableErrorEventsCmd: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

/* Callback for config register and golden register memory comparison */
int32_t tivxVpacLdcConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mLdcConfigRegMemParams *ldc_config_reg_prms = (Vhwa_M2mLdcConfigRegMemParams *)configRegPrms;
    tivxVpacLdcObj *ldc_obj = NULL;

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
    if ((NULL != ldc_config_reg_prms) && (NULL != handle))
    /* LDRA_JUSTIFY_END */
    {
        ldc_obj = (tivxVpacLdcObj *)ldc_config_reg_prms->appData;
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
    if ((NULL != ldc_config_reg_prms) && (NULL != handle) && (NULL != ldc_obj))
    /* LDRA_JUSTIFY_END */
    {
        /* Compare the golden register memory and readback register memory using ldc_config_reg_prms */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are set up during buffer allocation and are expected to be valid if safety mechanisms are enabled.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != ldc_config_reg_prms->golden_ptr) &&
            (NULL != ldc_config_reg_prms->readback_ptr) &&
            (ldc_config_reg_prms->readback_mem_size > 0U))
        /* LDRA_JUSTIFY_END */
        {
            int32_t cmp_result = memcmp(
                ldc_config_reg_prms->golden_ptr,
                ldc_config_reg_prms->readback_ptr,
                ldc_config_reg_prms->readback_mem_size);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This error condition detects hardware configuration corruption which requires fault injection not available in test environment.
            Effect on this unit: If the control reaches here, our code base is expected to detect and report the safety mechanism violation.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != cmp_result)
            {
                VX_PRINT(VX_ZONE_ERROR, "LDC config register mismatch detected!\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                VX_PRINT(VX_ZONE_INFO, "LDC config register match successful.\n");
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are set up during buffer allocation and are expected to be valid if safety mechanisms are enabled.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid parameters for LDC config register validation\n");
            status = FVID2_EBADARGS;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This callback is invoked by the hardware driver with pre-validated parameters.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer in LDC config register validation callback\n");
        status = FVID2_EBADARGS;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

static vx_status tivxVpacLdcAllocReadbackBuffers(tivxVpacLdcObj *ldcObj)
{
    vx_status status = (vx_status)VX_SUCCESS;

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
    if (NULL == ldcObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxVpacLdcAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (NULL == ldcObj->handle)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxVpacLdcAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Valid ldcObj and handle */
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
        uint32_t readback_size = ldcObj->config_reg_mem_size;

        vx_status alloc_status = tivxMemBufferAlloc(&ldcObj->readback_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory allocation is managed by framework with sufficient heap size configured.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            ldcObj->readback_mem_ptr_phys = tivxMemShared2PhysPtr(ldcObj->readback_mem_ptr_virt.shared_ptr, (int32_t)ldcObj->readback_mem_ptr_virt.mem_heap_region);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory allocation is managed by framework with sufficient heap size configured.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memset((void *)(uintptr_t)ldcObj->readback_mem_ptr_virt.shared_ptr, 0, readback_size);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory allocation is managed by framework with sufficient heap size configured.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            alloc_status = tivxMemBufferAlloc(&ldcObj->golden_reg_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);

            ldcObj->golden_reg_mem_ptr_phys = tivxMemShared2PhysPtr(ldcObj->golden_reg_mem_ptr_virt.shared_ptr, (int32_t)ldcObj->golden_reg_mem_ptr_virt.mem_heap_region);
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory allocation is managed by framework with sufficient heap size configured.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Fvid2Utils_memset((void *)(uintptr_t)ldcObj->golden_reg_mem_ptr_virt.shared_ptr, 0, readback_size);
        }

        /* Make FVID2 control call to update the register readback structure */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory allocation is managed by framework with sufficient heap size configured.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_M2mLdcConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ptr   = (void *)(uintptr_t)ldcObj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)ldcObj->readback_mem_ptr_virt.shared_ptr;

            /* Register the callback for config register memory comparison */
            reg_ptrs.cbFxn = tivxVpacLdcConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)ldcObj;

            int32_t fvid2_status = Fvid2_control(
                ldcObj->handle,
                VHWA_M2M_IOCTL_LDC_SET_CONFIG_REG_PTRS,
                (void *)&reg_ptrs,
                NULL
            );

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case requires hardware driver failure that cannot be simulated in test environment.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to set LDC register readback/golden pointers\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case requires hardware driver failure that cannot be simulated in test environment.
            Effect on this unit: If the control reaches here, our code base is expected to free allocated resources.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                tivxVpacLdcFreeReadbackBuffers(ldcObj);
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory allocation is managed by framework with sufficient heap size configured.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to free allocated resources and return error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            tivxVpacLdcFreeReadbackBuffers(ldcObj);
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static void tivxVpacLdcFreeReadbackBuffers(tivxVpacLdcObj *ldcObj)
{
    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != ldcObj)
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;
        uint32_t readback_size = ldcObj->config_reg_mem_size;

        /* Free readback buffer if allocated */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (ldcObj->readback_mem_ptr_phys != 0u)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxMemBufferFree(&ldcObj->readback_mem_ptr_virt, readback_size);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free readback buffer\n");
            }
            /* LDRA_JUSTIFY_END */
            ldcObj->readback_mem_ptr_phys = 0u;
        }

        /* Free golden register buffer if allocated */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
        <justification end> */
        if (ldcObj->golden_reg_mem_ptr_phys != 0u)
        /* LDRA_JUSTIFY_END */
        {
            status = tivxMemBufferFree(&ldcObj->golden_reg_mem_ptr_virt, readback_size);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free golden register buffer\n");
            }
            /* LDRA_JUSTIFY_END */
            ldcObj->golden_reg_mem_ptr_phys = 0u;
        }
        ldcObj->config_reg_mem_size = 0u;
    }
}

static vx_status tivxEnableVpacLdcSafetyMechanisms(
    tivxVpacLdcObj *ldcObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *params_ptr = NULL;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t enable;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == ldcObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "ldcObj is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the final else branch would handle NULL usr_data_obj case.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (NULL != usr_data_obj)
    /* LDRA_JUSTIFY_END */
    {
        params_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
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
                if (sizeof(tivx_vpac_ldc_safety_mechanism_params_t) == usr_data_obj->mem_size)
                {
                    tivx_vpac_ldc_safety_mechanism_params_t *safety_params =
                        (tivx_vpac_ldc_safety_mechanism_params_t *)params_ptr;

                    /* Enable or disable reconfigMMR based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_reconfig_and_reinit_Reg;
                    fvid2_status = Fvid2_control(ldcObj->handle, VHWA_M2M_IOCTL_LDC_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case requires hardware driver failure that cannot be simulated in test environment.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set reconfigMMR\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */

                    /* Enable or disable status register validation based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_status_reg_validation;
                    fvid2_status = Fvid2_control(ldcObj->handle, VHWA_M2M_IOCTL_LDC_ENABLE_STATUS_REG_VALIDATE, &enable, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case requires hardware driver failure that cannot be simulated in test environment.
                    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set status register validation\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */

                    /* Enable or disable config register validation based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    if (0U != safety_params->enable_readback_config_registers)
                    {
                        uint32_t readback_size = 0U;

                        /* Fetch the TIVX_VPAC_LDC_READBACK_SIZE from the low level driver */
                        if(((vx_status)VX_SUCCESS == status))
                        {
                            fvid2_status = Fvid2_control(
                                ldcObj->handle,
                                VHWA_M2M_IOCTL_LDC_GET_READBACK_SIZE,
                                &readback_size,
                                NULL
                            );

                            ldcObj->config_reg_mem_size = readback_size;

                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case requires hardware driver failure that cannot be simulated in test environment.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((FVID2_SOK == fvid2_status) && (readback_size > 0U))
                            {
                                VX_PRINT(VX_ZONE_INFO, "LDC readback size allocation successful: %u bytes\n", readback_size);
                            }
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to fetch LDC readback size from driver\n");
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
                            if ((ldcObj->readback_mem_ptr_phys != 0U) && (ldcObj->golden_reg_mem_ptr_phys != 0U))
                            {
                                VX_PRINT(VX_ZONE_INFO, "Readback or golden register buffer already allocated, skipping allocation\n");
                            }
                            /* LDRA_JUSTIFY_END */
                            else
                            {
                                status = tivxVpacLdcAllocReadbackBuffers(ldcObj);
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
                            enable = (uint32_t)safety_params->enable_readback_config_registers;
                            fvid2_status = Fvid2_control(ldcObj->handle, VHWA_M2M_IOCTL_LDC_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case requires hardware driver failure that cannot be simulated in test environment.
                            Effect on this unit: If the control reaches here, our code base is expected to free allocated buffers and return error.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if (FVID2_SOK != fvid2_status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                                tivxVpacLdcFreeReadbackBuffers(ldcObj);
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
                            VX_PRINT(VX_ZONE_ERROR, "LDC readback buffer allocation failed, skipping config register validation enable\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    else
                    {
                        enable = 0U;
                        fvid2_status = Fvid2_control(ldcObj->handle, VHWA_M2M_IOCTL_LDC_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case requires hardware driver failure that cannot be simulated in test environment.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != fvid2_status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                            status = (vx_status)VX_FAILURE;
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
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
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

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

static int32_t tivxVpacLdcFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacLdcObj *ldc_obj = (tivxVpacLdcObj *)appData;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != ldc_obj)
    /* LDRA_JUSTIFY_END */
    {
        (void)tivxEventPost(ldc_obj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}
/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start> Rationale:
This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacLdcErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacLdcObj *ldc_obj = (tivxVpacLdcObj *)appData;

    if (NULL != ldc_obj)
    {
        ldc_obj->err_stat = errEvents;
    }
}
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start> Rationale:
This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacLdcWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxVpacLdcObj *ldc_obj = (tivxVpacLdcObj *)appData;
    if (NULL != ldc_obj)
    {
        ldc_obj->wdTimerErrStatus = ldc_obj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents;

        if(0u != ldc_obj->wdTimerErrStatus)
        {
            (void)tivxEventPost(ldc_obj->waitForProcessCmpl);
        }
    }
}

/* LDRA_JUSTIFY_END */
BUILD_ASSERT((sizeof(Vhwa_M2mLdcPsaSign) == sizeof(((tivx_vpac_ldc_psa_timestamp_data_t *)0)->psa_values))? 1U : 0U);
