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

#include "TI/tivx.h"
#include "TI/hwa_vpac_nf.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_nf_bilateral.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_nf_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include <math.h>
#include "vhwa/include/vhwa_m2mNf.h"
#include "utils/perf_stats/include/app_perf_stats.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define LUT_ROWS 5U

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                            isAlloc;
    tivx_vpac_nf_bilateral_params_t     nfBilateralParams;
    Vhwa_M2mNfCreatePrms                createPrms;
    Vhwa_M2mNfConfig                    nf_cfg;
    Nf_WgtTableConfig                   wgtTbl;
    Fvid2_Handle                        handle;
    tivx_event                          waitForProcessCmpl;
    /* Structure for error event parameters
     * Used to register callback for the given set of events.*/
    Nf_ErrEventParams                   errEvtPrms;

    Fvid2_Frame                         inFrm;
    Fvid2_Frame                         outFrm;
    Fvid2_CbParams                      cbPrms;
    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;

    /* Stores the status of error interrupt */
    uint32_t                            err_stat;
    /* Structure for watchdog timer error event parameters
     * Used to register callback for the given set of events.*/
    Nf_WdTimerErrEventParams            wdTimerErrEvtPrms;
    /* Stores the status of watchdog timer error interrupt */
    uint32_t                            wdTimerErrStatus;

    /*! Instance ID of the NF driver */
    uint32_t                            nf_drv_inst_id;
    /*! HWA Performance ID */
    app_perf_hwa_id_t                   hwa_perf_id;
    /* Timestamp for the most recent frame processed. */
    uint64_t                            timestamp;
    
    uint64_t                           readback_mem_ptr_phys;
    /* Physical address for register readback memory */
    tivx_shared_mem_ptr_t              readback_mem_ptr_virt;
    /* Virtual address for register readback memory */
    uint64_t                           golden_reg_mem_ptr_phys;
    /* Physical address for register written memory */
    tivx_shared_mem_ptr_t              golden_reg_mem_ptr_virt;
    /* Virtual address for register written memory */
    uint32_t                           config_reg_mem_size;
    /* Size of the configuration register memory */
} tivxVpacNfBilateralObj;

typedef struct
{
    tivx_mutex lock;
    tivxVpacNfBilateralObj nfBilateralObj[VHWA_M2M_NF_MAX_HANDLES];

} tivxVpacNfBilateralInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacNfBilateralProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfBilateralControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static tivxVpacNfBilateralObj *tivxVpacNfBilateralAllocObject(
       tivxVpacNfBilateralInstObj *instObj);
static void tivxVpacNfBilateralFreeObject(
       tivxVpacNfBilateralInstObj *instObj, tivxVpacNfBilateralObj *nf_bilateral_obj);
static void tivxVpacNfSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static void tivxVpacNfBilateralGenerateLut(uint8_t subRangeBits, const vx_float64 *sigma_s,
    const vx_float64 *sigma_r, uint32_t *i_lut);
static uint32_t tivxVpacNfBilateralGenerateLutCoeffs(uint8_t mode,uint8_t inp_bitw,
    uint8_t filtSize, vx_float64 sigma_s, vx_float64 sigma_r, vx_float64 *f_wt_lut, uint8_t out_bitw,
    uint32_t *i_wt_lut_spatial, uint32_t *i_wt_lut_full);
static void tivxVpacNfBilateralInterleaveTables(uint32_t * const *i_lut, uint8_t numTables,
    uint32_t rangeLutEntries);
static uint32_t getSubRangeBits(uint16_t i);
static vx_status tivxVpacNfBilateralSetHtsLimitCmd(
    tivxVpacNfBilateralObj *nf_bilateral_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfBilateralSetCoeff(tivxVpacNfBilateralObj *nf_bilateral_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfBilateralGetErrStatusCmd(const tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxVpacNfBilateralGetPsaStatusCmd(const tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfBilateralEnableErrorEventsCmd(tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfBilateralMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxVpacNfBilateralMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);

int32_t tivxVpacNfBilateralFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxVpacNfBilateralErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxVpacNfBilateralWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);
static vx_status tivxEnableVpacNfBilateralSafetyMechanisms(tivxVpacNfBilateralObj *nf_bilateral_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);

int32_t tivxVpacNfBilateralConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms);

static vx_status tivxVpacNfBilateralAllocReadbackBuffers(tivxVpacNfBilateralObj *nf_bilateral_obj);

static void tivxVpacNfBilateralFreeReadbackBuffers(tivxVpacNfBilateralObj *nf_bilateral_obj);
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_vpac_nf_bilateral_target_kernel = NULL;

tivxVpacNfBilateralInstObj gTivxVpacNfBilateralInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacNfBilateral(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();

    /* LDRA_JUSTIFY_START
    <metric start>statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
    {
        strncpy(target_name, TIVX_TARGET_VPAC_NF, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_END */
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
        strncpy(target_name, TIVX_TARGET_VPAC2_NF, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    #endif
    /* LDRA_JUSTIFY_END */
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if (status == (vx_status)VX_SUCCESS)
    {
    /* LDRA_JUSTIFY_END */
        vx_vpac_nf_bilateral_target_kernel = tivxAddTargetKernelByName(
                    TIVX_KERNEL_VPAC_NF_BILATERAL_NAME,
                    target_name,
                    tivxVpacNfBilateralProcess,
                    tivxVpacNfBilateralCreate,
                    tivxVpacNfBilateralDelete,
                    tivxVpacNfBilateralControl,
                    NULL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vx_vpac_nf_bilateral_target_kernel)
        /* LDRA_JUSTIFY_END */
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxVpacNfBilateralInstObj.lock);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                                        This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "Failed to create Mutex\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                memset(&gTivxVpacNfBilateralInstObj.nfBilateralObj, 0x0,
                    sizeof(tivxVpacNfBilateralObj) * VHWA_M2M_NF_MAX_HANDLES);
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
            /* TODO: how to handle this condition */
            VX_PRINT(VX_ZONE_ERROR,
                "Failed to Add NF Bilateral TargetKernel\n");
        }
        /* LDRA_JUSTIFY_END */ 
    }
}
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void tivxRemoveTargetKernelVpacNfBilateral.* <function end>
<justification start> 
Rationale: The component level test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelVpacNfBilateral(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_nf_bilateral_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_nf_bilateral_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Nf TargetKernel\n");
    }
    if (NULL != gTivxVpacNfBilateralInstObj.lock)
    {
        (void)tivxMutexDelete(&gTivxVpacNfBilateralInstObj.lock);
    }
}

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacNfBilateralProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          size;
    tivxVpacNfBilateralObj           *nf_bilateral_obj = NULL;
    tivx_obj_desc_image_t            *src;
    tivx_obj_desc_image_t            *dst;
    Fvid2_FrameList                  *inFrmList;
    Fvid2_FrameList                  *outFrmList;
    uint64_t                         cur_time;
    tivx_obj_desc_t         *out_base_desc = NULL;
    vx_status                        validate_reg_status = (vx_status)VX_SUCCESS;


    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to print the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. Passing only valid types to tivxAddKernelVpacNfBilateralValidate function. 
                        Error cases pre-validating at tiovx.
    Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach to the false condition.
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
        Effect on this unit: If the control reaches here, nf_bilateral_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            nf_bilateral_obj = temp_kernel_context;
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
            VX_PRINT(VX_ZONE_ERROR, "Null Desc\n");
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
        else if (sizeof(tivxVpacNfBilateralObj) != size)
        {
            VX_PRINT(VX_ZONE_ERROR, "Incorrect object size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            /* do nothing */
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        inFrmList = &nf_bilateral_obj->inFrmList;
        outFrmList = &nf_bilateral_obj->outFrmList;
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX];

        out_base_desc = (tivx_obj_desc_t *)dst;
        nf_bilateral_obj->timestamp = out_base_desc->timestamp;

        /* Set Weight Coefficients for NF */
        fvid2_status = Fvid2_control(nf_bilateral_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
            &nf_bilateral_obj->wgtTbl, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set coeff request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        /* Initialize NF Input Frame List */
        inFrmList->frames[0U] =
            &nf_bilateral_obj->inFrm;
        inFrmList->numFrames = 1U;

        nf_bilateral_obj->inFrm.addr[0U] = tivxMemShared2PhysPtr(
            src->mem_ptr[0].shared_ptr,
            (int32_t)src->mem_ptr[0].mem_heap_region);

        /* Initialize NF Output Frame List */
        outFrmList->frames[0U] = &nf_bilateral_obj->outFrm;
        outFrmList->numFrames = 1U;

        nf_bilateral_obj->outFrm.addr[0U] = tivxMemShared2PhysPtr(
            dst->mem_ptr[0].shared_ptr,
            (int32_t)dst->mem_ptr[0].mem_heap_region);

        /* Clearing error status from previous frame
        *  User should consume the error status before
        *  submitting next frame for processing
        */
        nf_bilateral_obj->wdTimerErrStatus = 0u;
        nf_bilateral_obj->err_stat = 0u;

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit NF Request*/
        fvid2_status = Fvid2_processRequest(nf_bilateral_obj->handle, inFrmList,
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
    <metric start>  branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Wait for Frame Completion */
        (void)tivxEventWait(nf_bilateral_obj->waitForProcessCmpl, VX_TIMEOUT_WAIT_FOREVER);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale:
        This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(0u == nf_bilateral_obj->wdTimerErrStatus)
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
            if(0u != nf_bilateral_obj->err_stat)
            {
                VX_PRINT(VX_ZONE_INFO, "Error interrupt: NF non-stalling error interrupt triggered \n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            fvid2_status = Fvid2_getProcessedRequest(nf_bilateral_obj->handle,
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
        /* Call the control command for statusReg/configReg validate */
        {
            fvid2_status = Fvid2_control(nf_bilateral_obj->handle, VHWA_M2M_IOCTL_NF_VALIDATE_REG, NULL, NULL);
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
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        appPerfStatsHwaUpdateLoad(nf_bilateral_obj->hwa_perf_id,
            (uint32_t)cur_time,
            dst->imagepatch_addr[0U].dim_x*dst->imagepatch_addr[0U].dim_y /* pixels processed */
            );
    }

    if(((vx_status)VX_SUCCESS != status) || ((vx_status)VX_SUCCESS != validate_reg_status))
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    tivxVpacNfBilateralObj           *nf_bilateral_obj = NULL;
    Vhwa_M2mNfConfig                 *nf_cfg = NULL;
    tivx_vpac_nf_bilateral_params_t  *params = NULL;
    tivx_vpac_nf_bilateral_sigmas_t  *sigmas = NULL;
    tivx_obj_desc_user_data_object_t *params_array = NULL;
    tivx_obj_desc_user_data_object_t *sigmas_array = NULL;
    tivx_obj_desc_image_t            *src;
    tivx_obj_desc_image_t            *dst;
    void                             *params_array_target_ptr = NULL;
    void                             *sigmas_array_target_ptr = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);
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
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
        status = (vx_status)VX_FAILURE;
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
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nf_bilateral_obj = tivxVpacNfBilateralAllocObject(&gTivxVpacNfBilateralInstObj);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != nf_bilateral_obj)
        /* LDRA_JUSTIFY_END */
        {
            params_array = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_CONFIGURATION_IDX];
            sigmas_array = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_SIGMAS_IDX];
            src = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_INPUT_IDX];
            dst = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_BILATERAL_OUTPUT_IDX];
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Nf Bilateral Object\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
        }
        /* LDRA_JUSTIFY_END */
    }
   
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        Vhwa_M2mNfCreatePrmsInit(&nf_bilateral_obj->createPrms);

        nf_cfg = &nf_bilateral_obj->nf_cfg;

        params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);
        sigmas_array_target_ptr = tivxMemShared2TargetPtr(&sigmas_array->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferMap(sigmas_array_target_ptr, sigmas_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        params = (tivx_vpac_nf_bilateral_params_t *)params_array_target_ptr;
        sigmas = (tivx_vpac_nf_bilateral_sigmas_t *)sigmas_array_target_ptr;

        /* Initialize NF Config with defaults */
        Nf_ConfigInit(&nf_cfg->nfCfg);
        nf_bilateral_obj->createPrms.enablePsa = params->params.enable_psa;

        status = tivxEventCreate(&nf_bilateral_obj->waitForProcessCmpl);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            nf_bilateral_obj->cbPrms.cbFxn   = tivxVpacNfBilateralFrameComplCb;
            nf_bilateral_obj->cbPrms.appData = nf_bilateral_obj;

            nf_bilateral_obj->handle = Fvid2_create(FVID2_VHWA_M2M_NF_DRV_ID,
                nf_bilateral_obj->nf_drv_inst_id, (void *)&nf_bilateral_obj->createPrms,
                NULL, &nf_bilateral_obj->cbPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == nf_bilateral_obj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Nf Bilateral Object\n");
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
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nf_bilateral_obj->errEvtPrms.errEvents = VHWA_NF_RD_ERR | VHWA_NF_WR_ERR;
        nf_bilateral_obj->errEvtPrms.cbFxn     = tivxVpacNfBilateralErrorCb;
        nf_bilateral_obj->errEvtPrms.appData   = nf_bilateral_obj;

        fvid2_status = Fvid2_control(nf_bilateral_obj->handle,
            IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB, &nf_bilateral_obj->errEvtPrms, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */ 
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to Register Error Callback\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Set default 64K cycle timeout during registration */
        nf_bilateral_obj->wdTimerErrEvtPrms.wdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        /* Disable Watchdog Timer Error Events by default */
        nf_bilateral_obj->wdTimerErrEvtPrms.wdTimerErrEvents = 0u;
        nf_bilateral_obj->wdTimerErrEvtPrms.cbFxn     = tivxVpacNfBilateralWdTimerErrorCb;
        nf_bilateral_obj->wdTimerErrEvtPrms.appData   = nf_bilateral_obj;
        fvid2_status = Fvid2_control(nf_bilateral_obj->handle,
            VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB, &nf_bilateral_obj->wdTimerErrEvtPrms, NULL);
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
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Set NF Config parameters - centralPixelWeight set in tivxVpacNfBilateralProcess */
        nf_cfg->nfCfg.filterMode = NF_FILTER_MODE_BILATERAL;
        nf_cfg->nfCfg.tableMode = params->adaptive_mode;
        nf_cfg->nfCfg.skipMode = params->params.output_pixel_skip;
        nf_cfg->nfCfg.interleaveMode = params->params.input_interleaved;
        nf_cfg->nfCfg.outputShift = params->params.output_downshift;
        nf_cfg->nfCfg.outputOffset = params->params.output_offset;
        nf_cfg->nfCfg.numSubTables = getSubRangeBits(sigmas->num_sigmas);
        nf_cfg->nfCfg.subTableIdx = params->sub_table_select;
        nf_cfg->nfCfg.centralPixelWeight = 255;
        nf_bilateral_obj->wgtTbl.filterMode = NF_FILTER_MODE_BILATERAL;

        tivxVpacNfBilateralGenerateLut((uint8_t)getSubRangeBits(sigmas->num_sigmas), sigmas->sigma_space, sigmas->sigma_range,
            nf_bilateral_obj->wgtTbl.blFilterLut);

        tivxVpacNfSetFmt(&nf_cfg->inFmt, src);
        tivxVpacNfSetFmt(&nf_cfg->outFmt, dst);

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           VPAC NF parameters needs to be reconfigured */

        memcpy(&nf_bilateral_obj->nfBilateralParams, params, sizeof(tivx_vpac_nf_bilateral_params_t));

        fvid2_status = Fvid2_control(nf_bilateral_obj->handle,
            IOCTL_VHWA_M2M_NF_SET_PARAMS, &nf_bilateral_obj->nf_cfg, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Set parameters request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        /* Set NF coeff */
        fvid2_status = Fvid2_control(nf_bilateral_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
            &nf_bilateral_obj->wgtTbl, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Set coeffs request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        tivxCheckStatus(&status, tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        tivxCheckStatus(&status, tivxMemBufferUnmap(sigmas_array_target_ptr, sigmas_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxSetTargetKernelInstanceContext(kernel, nf_bilateral_obj,
            sizeof(tivxVpacNfBilateralObj));
    }
    if ((vx_status)VX_SUCCESS != status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != nf_bilateral_obj)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested..
            <justification end> */
            if (NULL != nf_bilateral_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(nf_bilateral_obj->handle, NULL);
                nf_bilateral_obj->handle = NULL;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != nf_bilateral_obj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&nf_bilateral_obj->waitForProcessCmpl);
            }

            tivxVpacNfBilateralFreeObject(&gTivxVpacNfBilateralInstObj, nf_bilateral_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 size;
    tivxVpacNfBilateralObj    *nf_bilateral_obj = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_BILATERAL_MAX_PARAMS);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
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
        Effect on this unit: If the control reaches here, nf_bilateral_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            nf_bilateral_obj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((((vx_status)VX_SUCCESS == status) && (NULL != nf_bilateral_obj)) &&
            (sizeof(tivxVpacNfBilateralObj) == size))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != nf_bilateral_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(nf_bilateral_obj->handle, NULL);
                nf_bilateral_obj->handle = NULL;
            }

           /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != nf_bilateral_obj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&nf_bilateral_obj->waitForProcessCmpl);
            }

            /* Free the readback and golden register buffers */
            tivxVpacNfBilateralFreeReadbackBuffers(nf_bilateral_obj);

            tivxVpacNfBilateralFreeObject(&gTivxVpacNfBilateralInstObj, nf_bilateral_obj);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, "Invalid Target Instance Context\n");
        }
        /* LDRA_JUSTIFY_END */
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
    }
    /* LDRA_JUSTIFY_END */
    return status;
}

static vx_status VX_CALLBACK tivxVpacNfBilateralControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacNfBilateralObj             *nf_bilateral_obj = NULL;

    void *temp_kernel_context = NULL;
    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The kernel parameter is expected to be pre-validated by the OpenVX framework before kernel callback invocation.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, nf_bilateral_obj would not be assigned and subsequent NULL checks would catch the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nf_bilateral_obj = temp_kernel_context;
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
        VX_PRINT(VX_ZONE_ERROR, "Failed to get Target Kernel Instance Context\n");
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
    else if ((NULL == nf_bilateral_obj) ||
        (sizeof(tivxVpacNfBilateralObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Wrong Size for Nf Bilateral Obj\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */
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
            case TIVX_VPAC_NF_CMD_SET_HTS_LIMIT:
            {
                status = tivxVpacNfBilateralSetHtsLimitCmd(nf_bilateral_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_SET_COEFF:
            {
                status = tivxVpacNfBilateralSetCoeff(nf_bilateral_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_GET_ERR_STATUS:
            {
                status = tivxVpacNfBilateralGetErrStatusCmd(nf_bilateral_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_GET_PSA_STATUS:
            {
                status = tivxVpacNfBilateralGetPsaStatusCmd(nf_bilateral_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxVpacNfBilateralEnableErrorEventsCmd(nf_bilateral_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_ENABLE_VPAC_SAFETY_MECHANISM:
            {
                status = tivxEnableVpacNfBilateralSafetyMechanisms(nf_bilateral_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0]);
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, the else branch would handle successful enablement without error logging.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (status != (vx_status)VX_SUCCESS)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to enable VPAC NF safety mechanisms\n");
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Node Command Id\n");
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

static tivxVpacNfBilateralObj *tivxVpacNfBilateralAllocObject(
       tivxVpacNfBilateralInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacNfBilateralObj *nf_bilateral_obj = NULL;
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
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
        <justification start> 
        Rationale:The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 4 NF handles
        Effect on this unit: It will utilise all the 4 NF handles which are validated in the NF driver.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (0U == instObj->nfBilateralObj[cnt].isAlloc)
            {
                nf_bilateral_obj = &instObj->nfBilateralObj[cnt];
                memset(nf_bilateral_obj, 0x0, sizeof(tivxVpacNfBilateralObj));
                instObj->nfBilateralObj[cnt].isAlloc = 1U;

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
                    instObj->nfBilateralObj[cnt].nf_drv_inst_id = VHWA_M2M_NF_DRV_INST_ID;
                    instObj->nfBilateralObj[cnt].hwa_perf_id    = APP_PERF_HWA_VPAC1_NF;
                }
                #if (VPAC_COUNT > 1)
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */ 
                else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
                /* LDRA_JUSTIFY_END */
                {
                    instObj->nfBilateralObj[cnt].nf_drv_inst_id = VHWA_M2M_VPAC_1_NF_DRV_INST_ID_0;
                    instObj->nfBilateralObj[cnt].hwa_perf_id    = APP_PERF_HWA_VPAC2_NF;
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
                    nf_bilateral_obj = NULL;
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
    return (nf_bilateral_obj);
}

static void tivxVpacNfBilateralFreeObject(tivxVpacNfBilateralInstObj *instObj,
    tivxVpacNfBilateralObj *nf_bilateral_obj)
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
    Effect on this unit: If the control reaches here, the else block would handle the mutex lock failure.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The test framework does not support the configuration required to trigger this scenario.
        Effect on this unit: If the control reaches here, the code is expected to free the allocated handle for NF.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (nf_bilateral_obj == &instObj->nfBilateralObj[cnt])
            {
                nf_bilateral_obj->isAlloc = 0U;
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

static void tivxVpacNfSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc)
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
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications CAN reach this portion.
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to set input/output dataFormat and ccsFormat accordingly.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            case (vx_df_image)VX_DF_IMAGE_S16:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
                fmt->ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
                break;
            }
            /* LDRA_JUSTIFY_END */
            case (vx_df_image)TIVX_DF_IMAGE_P12:
            {
                fmt->dataFormat = FVID2_DF_LUMA_ONLY;
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
                VX_PRINT(VX_ZONE_ERROR, "Invalid Vx Image Format\n");
                break;
            }
            /* LDRA_JUSTIFY_END */
        }

        fmt->width     = img_desc->imagepatch_addr[0U].dim_x;
        fmt->height    = img_desc->imagepatch_addr[0U].dim_y;
        fmt->pitch[0U] = (uint32_t)img_desc->imagepatch_addr[0U].stride_y;
    }
}

static void tivxVpacNfBilateralGenerateLut(uint8_t subRangeBits, const vx_float64 *sigma_s,
    const vx_float64 *sigma_r, uint32_t *i_lut)
{
    uint32_t numTables = (uint32_t)1U << (uint32_t)subRangeBits;
    uint8_t tableNum;
    uint32_t rangeLutEntries = (uint32_t)256U >> (uint32_t)subRangeBits;
    uint32_t f_lut_size = LUT_ROWS * 256U * sizeof(vx_float64);
    vx_float64   *f_lut = tivxMemAlloc(f_lut_size, (vx_enum)TIVX_MEM_EXTERNAL);
    uint32_t ret_val;

    for (tableNum = 0U; tableNum < numTables; tableNum++)
    {
        vx_float64 s_sigma = sigma_s[tableNum];
        vx_float64 r_sigma = sigma_r[tableNum];

        /*-----------------------------------------------------------------*/
        /* Generate fixed point LUT values, with index to LUT being        */
        /* pixel differences.                                              */
        /*-----------------------------------------------------------------*/
        ret_val = tivxVpacNfBilateralGenerateLutCoeffs
            (
            0U,
            8U - (uint8_t)subRangeBits,
            5U,
            s_sigma,
            r_sigma,
            f_lut,
            8U,
            NULL,
            &i_lut[(uint32_t)tableNum * LUT_ROWS * rangeLutEntries]
            );

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        LUT coefficient generation failure (ret_val == 1U) requires invalid sigma parameters or mathematical computation errors.
        Effect on this unit: If the control reaches here, the code would log LUT coefficients failure but continue execution.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (1U == ret_val)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to generate LUT coefficients\n");
        }
        /* LDRA_JUSTIFY_END */
    }

    vx_status free_status = tivxMemFree(f_lut, f_lut_size, (vx_enum)TIVX_MEM_EXTERNAL);
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Memory free failure requires system-level resource management errors or memory corruption that cannot be simulated in the test environment.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code would log memory free failure but continue execution.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS != free_status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to free memory\n");
    }
    /* LDRA_JUSTIFY_END */

    if (numTables > 1U)
    {
        tivxVpacNfBilateralInterleaveTables(&i_lut, (uint8_t)numTables, rangeLutEntries);
    }
}

static uint32_t tivxVpacNfBilateralGenerateLutCoeffs(uint8_t mode,uint8_t inp_bitw,
    uint8_t filtSize, vx_float64 sigma_s, vx_float64 sigma_r, vx_float64 *f_wt_lut, uint8_t out_bitw,
    uint32_t *i_wt_lut_spatial, uint32_t *i_wt_lut_full)
{
    int32_t row, col;
    int32_t lut_w, lut_h;
    uint32_t temp_lut_w;
    uint32_t temp_lutSize = (uint32_t)1U << inp_bitw;
    int32_t lutSize = (int32_t)temp_lutSize;
    int32_t referenceSize = 12;
    uint8_t numspatialDistances;
    uint32_t returnVal = 0U;

    vx_float64 wt_s;
    vx_float64 wt_r;
    vx_float64 wt_sum;
    vx_float64 max = 0.0f;

    /* Translate filter size (5x5, 3x3, 1x1) into spatialDistances */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to set numspatialDistances accordingly.
    However, due to the stated rationale, this is not tested.
    <justification end> */      
    if (filtSize == 3U)
    {
        numspatialDistances = 2U;
    }
    else if (filtSize == 1U)
    {
        numspatialDistances = 0U;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        numspatialDistances = 5U;
    }

    /*---------------------------------------------------------------------*/
    /* Compute LUT width and height.                                       */
    /*---------------------------------------------------------------------*/
    memset(f_wt_lut, 0, LUT_ROWS * (uint32_t)lutSize * sizeof(vx_float64));

    temp_lut_w = (uint32_t)1U << inp_bitw;
    lut_w = (int32_t)temp_lut_w;
    lut_h = (int32_t)numspatialDistances;

    /*---------------------------------------------------------------------*/
    /* Actual doubleing pt. LUT creation for Bilateral filter.              */
    /*---------------------------------------------------------------------*/

    /* If the space sigma is 0, then table should remain all zeros */
    if (sigma_s != 0.0f)
    {
        /* Index of the space distance from the center pixel, 0-4 */
        const uint8_t spaceWeightIndex[25] = {  4, 3, 2, 3, 4,
                                                3, 1, 0, 1, 3,
                                                2, 0, 0, 0, 2,
                                                3, 1, 0, 1, 3,
                                                4, 3, 2, 3, 4  };

        /* This distance-squared mapping of each index (above) in a 5x5 to the center pixel */
        const vx_float64 distanceValuesSquared[] =
        {
            1.0, // 0: 1^2 + 0^2
            2.0, // 1: 1^2 + 1^2
            4.0, // 2: 2^2 + 0^2
            5.0, // 3: 2^2 + 1^2
            8.0  // 4: 2^2 + 2^2
        };

        /* If the range sigma is 0, bilateral doesn't make sense, so instead generic mode weights should be generated */
        if (sigma_r == 0.0f)
        {
            /* Generate generic mode weights using gaussian curve from sigma_s */
            for (row = 0; row < 25; row++)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
                <justification end> */  
                if (spaceWeightIndex[row] < numspatialDistances)
                /* LDRA_JUSTIFY_END */
                {   // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                    wt_s = distanceValuesSquared[spaceWeightIndex[row]] / (2.0f * sigma_s * sigma_s);
                    f_wt_lut[row] = exp(-wt_s);
                }
            }
            for (row = 13; row < 25; row++)
            {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start> 
                Rationale: The test framework and test apps cannot reach this portion.
                The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
                Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
                <justification end> */
                if (spaceWeightIndex[row] < numspatialDistances)
                /* LDRA_JUSTIFY_END */
                {   // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                    wt_s = distanceValuesSquared[spaceWeightIndex[row]] / (2.0f * sigma_s * sigma_s);
                    f_wt_lut[row-1] = exp(-wt_s);
                }
            }
            /* Overwrite center pixel weight to be 1 */
            f_wt_lut[12] = 1.0;
            max = 1.0;
        }
        else {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */   
            if (mode == 0u) /* Bilateral Filter Weights */
            /* LDRA_JUSTIFY_END */
            {
                for (col = 0; col < lut_w; col++)
                {
                    // Only generate the lut values across the range that are needed
                    uint32_t temp = (uint32_t)1U << ((uint32_t)referenceSize - (uint32_t)inp_bitw);
                    int32_t col_mod = col * (int32_t)temp;
                    // gaussian_r = exp(-(x^2 / (2*sigma_r^2))
                    wt_r = ((vx_float64)col_mod * (vx_float64)col_mod) / (2.0f * sigma_r * sigma_r);

                    for (row = 0; row < lut_h; row++)
                    {
                        // gaussian_s = exp(-(x^2 / (2*sigma_s^2))
                        wt_s = distanceValuesSquared[row] / (2.0f * sigma_s * sigma_s);
                        wt_sum = wt_s + wt_r;
                        f_wt_lut[(row * lut_w) + col] = exp(-wt_sum);
                        //printf("%f\n", f_wt_lut[(row * lut_w) + col]);
                    }
                }
                max = 1.0;
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */   
            else if ((mode == 1u) || (mode == 2u)) /* Passthrough Weights */
            {
                f_wt_lut[0] = 1.0;
                max = 1.0;
            }
            else if (mode == 3u) /* Highest Value Weights */
            {
                for (col = 0; col < lut_w; col++)
                {
                    for (row = 0; row < lut_h; row++)
                    {
                        f_wt_lut[(row * lut_w) + col] = 1.0;
                    }
                }
                max = 1.0;
            }
            else
            {
                returnVal = 1U;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    /*---------------------------------------------------------------------*/
    /* Fixed point LUT creation here.                                      */
    /*---------------------------------------------------------------------*/
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale:
    The value of returnVal is set to 1U only if an unsupported or invalid mode is passed to the function. 
    In all valid usage scenarios within this codebase, the mode is hardcoded to 0, so returnVal is never set to 1U.
    Effect on this unit:
    If returnVal == 1U, the function exits early and skips LUT initialization and output population. 
    As a result, no LUT data is generated or written, which may cause subsequent processing to operate on uninitialized or invalid LUT data, 
    potentially leading to incorrect filter behavior or undefined results.
    <justification end> */  
    if(returnVal != 1U)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale: The test framework and test apps cannot reach this portion.
        The function is always called with a valid pointer for i_wt_lut_full in the current implementation. 
        Effect on this unit:
        If i_wt_lut_full is NULL, the function skips LUT initialization and output population, resulting in no LUT data being generated or written. 
        This may cause subsequent processing to operate on uninitialized or invalid LUT data, potentially leading to incorrect filter behavior or undefined results.
        <justification end> */ 
        if (i_wt_lut_full != NULL)
        /* LDRA_JUSTIFY_END */
        {
            int32_t i;
            memset(i_wt_lut_full, 0, LUT_ROWS * (uint32_t)lutSize * sizeof(uint32_t));
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The test framework and test apps cannot reach this portion.
            The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
            Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
            <justification end> */
            if (mode == 2u)
            {
                /* do nothing */
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                for (i = 0; i < ((int32_t)LUT_ROWS * lutSize); i++) {
                    if (0.0f != max)
                    {
                        uint32_t temp_u32 = ((uint32_t)1U << out_bitw) - 1U;
                        int32_t one_lsl_out_bitw_minus_one = (int32_t)temp_u32;
                        vx_float64 temp_f64 = ((f_wt_lut[i] / max) * (vx_float64)one_lsl_out_bitw_minus_one) + 0.5f;
                        i_wt_lut_full[i] = (uint32_t)temp_f64;
                    }
                }
            }
        }

        /* In case spatial lut needs to be generated separatly */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The test framework and test apps cannot reach this portion.
        The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
        Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
        <justification end> */ 
        if ((i_wt_lut_spatial) && (i_wt_lut_full))
        {
            memset(i_wt_lut_spatial, 0, LUT_ROWS * sizeof(i_wt_lut_spatial[0]));
            for (row = 0; row < lut_h; row++)
            {
                i_wt_lut_spatial[row] = i_wt_lut_full[row * lut_w];
            }
        }
        /* LDRA_JUSTIFY_END */
    }
    return returnVal;
}

static void tivxVpacNfBilateralInterleaveTables(uint32_t * const *i_lut, uint8_t numTables,
    uint32_t rangeLutEntries)
{
    uint32_t *const oldLut = *i_lut;
    uint32_t newLutSize = LUT_ROWS * 256U * sizeof(uint32_t);
    uint32_t *newLut = tivxMemAlloc(newLutSize, (vx_enum)TIVX_MEM_EXTERNAL);
    uint32_t i, j;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to print the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != newLut)
    /* LDRA_JUSTIFY_END */
    {
        memset(newLut, 0, LUT_ROWS * 256U * sizeof(uint32_t));

        for (j = 0U; j < numTables; j++)
        {
            for (i = 0U; i < (LUT_ROWS * rangeLutEntries); i++)
            {
                newLut[(numTables * i) + j] = oldLut[((j * LUT_ROWS) * rangeLutEntries) + i];
            }
        }

        memcpy((uint32_t *)oldLut, newLut, LUT_ROWS*256U*sizeof(uint32_t));

        vx_status free_status = tivxMemFree(newLut, newLutSize, (vx_enum)TIVX_MEM_EXTERNAL);
        if ((vx_status)VX_SUCCESS != free_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Failed to free memory\n");
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to print the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Nf newLut\n");
    }
    /* LDRA_JUSTIFY_END */
}

static uint32_t getSubRangeBits(uint16_t i)
{
    uint32_t out = 0U;
    uint16_t local_i = i;  /* Local copy to avoid modifying function parameter */
    local_i >>= 1;
    while (local_i != 0U)
    {
        out++;
        local_i >>= 1;
    }
    return out;
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
static vx_status tivxVpacNfBilateralMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
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
    else if ((0u != (vhwa_err_events & ~VHWA_NF_ALL_ERR_EVENTS)) ||
             (0u != (wd_timer_err & ~VHWA_NF_WDTIMER_ERR)))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask provided\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* LDRA_JUSTIFY_START_DISABLED
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, the else branch would map VHWA error flags to TIVX error flags for valid hardware error events.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Map VHWA flags to TIVX flags for regular error interrupts */
        if (0u != (vhwa_err_events & VHWA_NF_RD_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_NF_RD_ERR;
        }
        if (0u != (vhwa_err_events & VHWA_NF_WR_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_NF_WR_ERR;
        }

        /* Map VHWA flags to TIVX flags for watchdog timer errors */
        if (0u != (wd_timer_err & (uint32_t)VHWA_NF_WDTIMER_ERR))
        {
            mapped_tivx_err_events |= TIVX_VPAC_NF_WDTIMER_ERR;
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
static vx_status tivxVpacNfBilateralMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
                                                      uint32_t *vhwa_err_events,
                                                      uint32_t *wd_timer_err)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t enableErrorEventCheckMask = TIVX_VPAC_NF_ALL_ERR_EVENTS;

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
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, the else branch would map VHWA error flags to TIVX error flags for valid hardware error events.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Map TIVX flags to VHWA flags for regular error interrupts */
        if (0u != (tivx_err_events & TIVX_VPAC_NF_RD_ERR))
        {
            *vhwa_err_events |= VHWA_NF_RD_ERR;
        }
        if (0u != (tivx_err_events & TIVX_VPAC_NF_WR_ERR))
        {
            *vhwa_err_events |= VHWA_NF_WR_ERR;
        }

        /* Map TIVX flags to VHWA flags for watchdog timer errors */
        if (0u != (tivx_err_events & TIVX_VPAC_NF_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_NF_WDTIMER_ERR;
        }
        /* LDRA_JUSTIFY_END_DISABLED */
    }

    return status;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxVpacNfBilateralSetHtsLimitCmd(
    tivxVpacNfBilateralObj *nf_bilateral_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                                status = (vx_status)VX_SUCCESS;
    int32_t                                  fvid2_status = FVID2_SOK;
    Vhwa_HtsLimiter                          hts_limit;
    tivx_vpac_nf_hts_bw_limit_params_t      *app_hts_prms;
    void                                    *target_ptr;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_vpac_nf_hts_bw_limit_params_t) ==
                usr_data_obj->mem_size)
        {
            app_hts_prms = (tivx_vpac_nf_hts_bw_limit_params_t *)target_ptr;

            hts_limit.enableBwLimit = app_hts_prms->enable_hts_bw_limit;
            hts_limit.cycleCnt = app_hts_prms->cycle_cnt;
            hts_limit.tokenCnt = app_hts_prms->token_cnt;

            fvid2_status = Fvid2_control(nf_bilateral_obj->handle,
                IOCTL_VHWA_M2M_NF_SET_HTS_LIMIT, &hts_limit, NULL);
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
                VX_PRINT(VX_ZONE_ERROR, "Set HTS limit request failed\n");
                status = (vx_status)VX_FAILURE;
            }
            else
             /* LDRA_JUSTIFY_END */
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

static vx_status tivxVpacNfBilateralSetCoeff(tivxVpacNfBilateralObj *nf_bilateral_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    Nf_WgtTableConfig                *wgtTbl = NULL;
    void                             *target_ptr;

    if (NULL != usr_data_obj)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(Nf_WgtTableConfig) ==
                usr_data_obj->mem_size)
        {
            wgtTbl = (Nf_WgtTableConfig *)target_ptr;
            fvid2_status = Fvid2_control(nf_bilateral_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
                           wgtTbl, NULL);
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
                VX_PRINT(VX_ZONE_ERROR, "Set coeff request failed\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
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

static vx_status tivxVpacNfBilateralGetErrStatusCmd(const tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat = 0u;

    if (NULL != scalar_obj_desc)
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxVpacNfBilateralMapVhwaToTivxErrEvents(nf_bilateral_obj->err_stat, nf_bilateral_obj->wdTimerErrStatus, &combined_err_stat);
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
        <metric start> branch <metric end>
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

static vx_status tivxVpacNfBilateralGetPsaStatusCmd(const tivxVpacNfBilateralObj *nf_bilateral_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    Vhwa_M2mNfPsaSign                 psa;
    int32_t                           fvid2_status = FVID2_SOK;
    void                              *target_ptr;
    tivx_vpac_nf_psa_timestamp_data_t *psa_status_ptr;
    
    if (NULL != usr_data_obj)
    {
        if (sizeof(tivx_vpac_nf_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            fvid2_status = Fvid2_control(nf_bilateral_obj->handle,
                IOCTL_VHWA_M2M_NF_GET_PSA_SIGN, &psa, NULL);
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
                    "tivxVpacNfBilateralGetPsa: Fvid2_control Failed\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
            
            /* Map the user data object to access the memory */
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            psa_status_ptr = (tivx_vpac_nf_psa_timestamp_data_t *)target_ptr;

            /* Copy driver data (PSA values) into the structure */
            tivx_obj_desc_memcpy(&psa_status_ptr->psa_values, psa.psaSign, sizeof(psa.psaSign));

            /* Add timestamp to the structure */
            psa_status_ptr->timestamp = nf_bilateral_obj->timestamp;

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
        VX_PRINT(VX_ZONE_ERROR, "Null argument. Expected pointer to tivx_obj_desc_user_data_object_t\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    return (status);
}

static vx_status tivxVpacNfBilateralEnableErrorEventsCmd(tivxVpacNfBilateralObj *nf_bilateral_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t  fvid2_status = FVID2_SOK;
    uint32_t intr_en = 0u;
    uint32_t wd_timer_intr_en = 0u;
    void *target_ptr;
    tivx_vpac_nf_bilateral_params_t *params_ptr = NULL;
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
        if(sizeof(tivx_vpac_nf_bilateral_params_t) != usr_data_obj->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVpacNfBilateralEnableErrorEventsCmd: Incorrect Data Object Size \n");
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
                    "tivxVpacNfBilateralEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_vpac_nf_bilateral_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->params.enable_error_events;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxVpacNfBilateralMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                The error event mask is pre-validated by the application and only valid event masks are provided.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, the code would detect invalid error event mask and log an error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxVpacNfBilateralEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    nf_bilateral_obj->errEvtPrms.errEvents = intr_en;
                    fvid2_status = Fvid2_control(nf_bilateral_obj->handle,
                        IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB, &nf_bilateral_obj->errEvtPrms, NULL);

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
                        /* Update WDTimer configuration from application */
                        nf_bilateral_obj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->params.wdtimer_timeout_cycles;
                        nf_bilateral_obj->wdTimerErrEvtPrms.wdTimerErrEvents = wd_timer_intr_en;
                        fvid2_status = Fvid2_control(nf_bilateral_obj->handle,
                            VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB, &nf_bilateral_obj->wdTimerErrEvtPrms, NULL);
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
                            "tivxVpacNfBilateralEnableErrorEventsCmd: Fvid2_control Failed \n");
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code would detect NULL user data object and return VX_FAILURE.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxVpacNfBilateralEnableErrorEventsCmd: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

static vx_status tivxEnableVpacNfBilateralSafetyMechanisms(
    tivxVpacNfBilateralObj *nf_bilateral_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    void *params_ptr = NULL;
    int32_t  fvid2_status = FVID2_SOK;
    uint32_t enable;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == nf_bilateral_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "nf_bilateral_obj is NULL\n");
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

            if ((vx_status)VX_SUCCESS == status)
            {
                if (sizeof(tivx_vpac_nf_safety_mechanism_params_t) == usr_data_obj->mem_size)
                {
                    tivx_vpac_nf_safety_mechanism_params_t *safety_params =
                        (tivx_vpac_nf_safety_mechanism_params_t *)params_ptr;

                    /* Enable or disable reconfigMMR based on input parameter */
                    if ((vx_bool)vx_true_e == (vx_bool)safety_params->enable_reconfig_and_reinit_Reg)
                    {
                        enable = 1U;
                    }
                    else
                    {
                        enable = 0U;
                    }
                    fvid2_status = Fvid2_control(nf_bilateral_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
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
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set reconfigMMR\n");
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
                    fvid2_status = Fvid2_control(nf_bilateral_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_STATUS_REG_VALIDATE, &enable, NULL);
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

                    /* Enable or disable config register validation based on input parameter */
                    if ((vx_bool)vx_true_e == (vx_bool)safety_params->enable_readback_config_registers)
                    {
                        uint32_t readback_size = 0U;

                        /* Fetch the readback size from the low level driver */
                        if ((vx_status)VX_SUCCESS == status)
                        {
                            fvid2_status = Fvid2_control(
                                nf_bilateral_obj->handle,
                                VHWA_M2M_IOCTL_NF_GET_READBACK_SIZE,
                                &readback_size,
                                NULL
                            );

                            nf_bilateral_obj->config_reg_mem_size = readback_size;

                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case is out of scope for the imaging test framework.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if (((FVID2_SOK == fvid2_status) && (readback_size > 0U)))
                            /* LDRA_JUSTIFY_END */
                            {
                                VX_PRINT(VX_ZONE_INFO, "NF readback size allocation successful: %u bytes\n", readback_size);
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
                                VX_PRINT(VX_ZONE_ERROR, "Failed to fetch NF readback size from driver\n");
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
                        if ((((vx_status)VX_SUCCESS == status) && (readback_size > 0U)))
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
                            if (((nf_bilateral_obj->readback_mem_ptr_phys != 0U) && (nf_bilateral_obj->golden_reg_mem_ptr_phys != 0U)))
                            {
                                VX_PRINT(VX_ZONE_INFO, "Readback or golden register buffer already allocated, skipping allocation\n");
                            }
                            /* LDRA_JUSTIFY_END */
                            else
                            {
                                status = tivxVpacNfBilateralAllocReadbackBuffers(nf_bilateral_obj);
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
                            enable = 1U;
                            fvid2_status = Fvid2_control(nf_bilateral_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                                VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                                tivxVpacNfBilateralFreeReadbackBuffers(nf_bilateral_obj);
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
                            VX_PRINT(VX_ZONE_ERROR, "NF readback buffer allocation failed, skipping config register validation enable\n");
                        }
                        /* LDRA_JUSTIFY_END */
                    }
                    else
                    {
                        enable = 0U;
                        fvid2_status = Fvid2_control(nf_bilateral_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
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

static vx_status tivxVpacNfBilateralAllocReadbackBuffers(tivxVpacNfBilateralObj *nf_bilateral_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t   fvid2_status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == nf_bilateral_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxVpacNfBilateralAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    if ((((vx_status)VX_SUCCESS == status) && (NULL == nf_bilateral_obj->handle)))
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxVpacNfBilateralAllocReadbackBuffers\n");
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
        uint32_t readback_size = nf_bilateral_obj->config_reg_mem_size;

        vx_status alloc_status = tivxMemBufferAlloc(&nf_bilateral_obj->readback_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);
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
            nf_bilateral_obj->readback_mem_ptr_phys = tivxMemShared2PhysPtr(
                nf_bilateral_obj->readback_mem_ptr_virt.shared_ptr,
                (int32_t)nf_bilateral_obj->readback_mem_ptr_virt.mem_heap_region);
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
            Fvid2Utils_memset((void *)(uintptr_t)nf_bilateral_obj->readback_mem_ptr_virt.shared_ptr, 0, readback_size);
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
            alloc_status = tivxMemBufferAlloc(&nf_bilateral_obj->golden_reg_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);

            nf_bilateral_obj->golden_reg_mem_ptr_phys = tivxMemShared2PhysPtr(
                nf_bilateral_obj->golden_reg_mem_ptr_virt.shared_ptr,
                (int32_t)nf_bilateral_obj->golden_reg_mem_ptr_virt.mem_heap_region);
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
            Fvid2Utils_memset((void *)(uintptr_t)nf_bilateral_obj->golden_reg_mem_ptr_virt.shared_ptr, 0, readback_size);
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
            Vhwa_M2mNfConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ref_ptr   = (void *)(uintptr_t)nf_bilateral_obj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)nf_bilateral_obj->readback_mem_ptr_virt.shared_ptr;

            reg_ptrs.cbFxn = tivxVpacNfBilateralConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)nf_bilateral_obj;

            fvid2_status = Fvid2_control(
                nf_bilateral_obj->handle,
                VHWA_M2M_IOCTL_NF_SET_CONFIG_REG_PTRS,
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set NF Bilateral register readback/golden pointers\n");
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
                tivxVpacNfBilateralFreeReadbackBuffers(nf_bilateral_obj);
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
            tivxVpacNfBilateralFreeReadbackBuffers(nf_bilateral_obj);
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static void tivxVpacNfBilateralFreeReadbackBuffers(tivxVpacNfBilateralObj *nf_bilateral_obj)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != nf_bilateral_obj)
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;
        uint32_t readback_size = nf_bilateral_obj->config_reg_mem_size;

        if (nf_bilateral_obj->readback_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&nf_bilateral_obj->readback_mem_ptr_virt, readback_size);
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
            nf_bilateral_obj->readback_mem_ptr_phys = 0u;
        }

        if (nf_bilateral_obj->golden_reg_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&nf_bilateral_obj->golden_reg_mem_ptr_virt, readback_size);
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
            nf_bilateral_obj->golden_reg_mem_ptr_phys = 0u;
        }

        nf_bilateral_obj->config_reg_mem_size = 0u;
    }
}


/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacNfBilateralFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacNfBilateralObj *nf_bilateral_obj = (tivxVpacNfBilateralObj *)appData;
    /* LDRA_JUSTIFY_START
    <metric start> branch statement <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
    This behaviour is part of the application design. An error accumulation statement can be added in a future release.
    <justification end> */
    if (NULL != nf_bilateral_obj)
    {
        (void)tivxEventPost(nf_bilateral_obj->waitForProcessCmpl);
    }
    /* LDRA_JUSTIFY_END */

    return FVID2_SOK;
}
/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start> Rationale:
This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
void tivxVpacNfBilateralErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacNfBilateralObj *nf_bilateral_obj = (tivxVpacNfBilateralObj *)appData;

    if (NULL != nf_bilateral_obj)
    {
        nf_bilateral_obj->err_stat = errEvents;
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
static void tivxVpacNfBilateralWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxVpacNfBilateralObj *nf_bilateral_obj = (tivxVpacNfBilateralObj *)appData;
    if (NULL != nf_bilateral_obj)
    {
        nf_bilateral_obj->wdTimerErrStatus = nf_bilateral_obj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents;

        if(0u != nf_bilateral_obj->wdTimerErrStatus)
        {
            (void)tivxEventPost(nf_bilateral_obj->waitForProcessCmpl);
        }
    }
}

/* LDRA_JUSTIFY_END */
int32_t tivxVpacNfBilateralConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mNfConfigRegMemParams *nf_config_reg_prms = (Vhwa_M2mNfConfigRegMemParams *)configRegPrms;
    tivxVpacNfBilateralObj *nf_bilateral_obj = NULL;

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
    if (((NULL != nf_config_reg_prms) && (NULL != handle)))
    /* LDRA_JUSTIFY_END */
    {
        nf_bilateral_obj = (tivxVpacNfBilateralObj *)nf_config_reg_prms->appData;
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
    if ((((NULL != nf_config_reg_prms) && (NULL != handle)) && (NULL != nf_bilateral_obj)))
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameters are set up during buffer allocation and are expected to be valid if safety mechanisms are enabled.
        Therefore, this failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((((NULL != nf_config_reg_prms->golden_ref_ptr) &&
            (NULL != nf_config_reg_prms->readback_ptr)) &&
            (nf_config_reg_prms->readback_mem_size > 0U)))
        /* LDRA_JUSTIFY_END */
        {
            int32_t cmp_result = memcmp(
                nf_config_reg_prms->golden_ref_ptr,
                nf_config_reg_prms->readback_ptr,
                nf_config_reg_prms->readback_mem_size);

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
            if (0 != cmp_result)
            {
                VX_PRINT(VX_ZONE_ERROR, "NF config register mismatch detected!\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                VX_PRINT(VX_ZONE_INFO, "NF config register match successful.\n");
            }
        }
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
    else
    {
        status = FVID2_EFAIL;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

BUILD_ASSERT(((VHWA_NF_RD_ERR == TIVX_VPAC_NF_RD_ERR)? 1U : 0U));
BUILD_ASSERT(((VHWA_NF_WR_ERR == TIVX_VPAC_NF_WR_ERR)? 1U : 0U));
BUILD_ASSERT((sizeof(Vhwa_M2mNfPsaSign) == sizeof(((tivx_vpac_nf_psa_timestamp_data_t *)0)->psa_values))? 1U : 0U);
