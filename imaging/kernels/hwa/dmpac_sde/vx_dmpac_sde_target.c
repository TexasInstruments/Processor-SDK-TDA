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
#include "TI/hwa_dmpac_sde.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_sde.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_dmpac_sde_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include <math.h>

#include "vhwa/include/vhwa_m2mSde.h"

#include "utils/perf_stats/include/app_perf_stats.h"
#if defined(LDRA_COVERAGE)
#include <kernels/coverage_files/include/ldra_remote_core_coverage_main.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                            isAlloc;
    tivx_dmpac_sde_params_t             sdeParams;
    Vhwa_M2mSdeCreateArgs               createPrms;
    Vhwa_M2mSdePrms                     sdePrms;
    Fvid2_Handle                        handle;
    tivx_event                          waitForProcessCmpl;
    /* Structure for error event parameters
     * Used to register callback for the given set of events.*/
    Sde_ErrEventParams                  errEvtPrms;

    Fvid2_Frame                         inFrm[VHWA_M2M_SDE_MAX_IN_BUFFER];
    Fvid2_Frame                         outFrm;
    Fvid2_CbParams                      cbPrms;
    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;
    uint32_t                            csHistogram[128U];

    /* Stores the status of error interrupt */
    uint32_t                            err_stat;
    /* Structure for watchdog timer error event parameters
     * Used to register callback for the given set of events.*/
    Sde_WdTimerErrEventParams           wdTimerErrEvtPrms;
    /* Stores the status of watchdog timer error interrupt */
    uint32_t                            wdTimerErrStatus;
    /* Timestamp for the most recent frame processed. */
    uint64_t                            timestamp;

    /* Physical address for register readback memory */
    uint64_t                           readback_mem_ptr_phys;

    /* Virtual address for register readback memory */
    tivx_shared_mem_ptr_t              readback_mem_ptr_virt;

    /* Physical address for register written memory */
    uint64_t                           golden_reg_mem_ptr_phys;

    /* Virtual address for register written memory */
    tivx_shared_mem_ptr_t              golden_reg_mem_ptr_virt;

    /* Size of the configuration register memory */
    uint32_t                           config_reg_mem_size;

} tivxDmpacSdeObj;

typedef struct
{
    tivx_mutex lock;
    tivxDmpacSdeObj sdeObj[VHWA_M2M_SDE_MAX_HANDLES];
} tivxDmpacSdeInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxDmpacSdeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacSdeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacSdeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacSdeControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static tivxDmpacSdeObj *tivxDmpacSdeAllocObject(
       tivxDmpacSdeInstObj *instObj);
static void tivxDmpacSdeFreeObject(
       tivxDmpacSdeInstObj *instObj,
       tivxDmpacSdeObj *sde_obj);
static void tivxDmpacSdeSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static vx_status tivxDmpacSdeGetErrStatusCmd(
       const tivxDmpacSdeObj *sde_obj,
       tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxDmpacSdeGetPsaStatusCmd(
       const tivxDmpacSdeObj *sde_obj,
       tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacSdeEnableErrorEventsCmd(
       tivxDmpacSdeObj *sde_obj,
       tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacSdeMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxDmpacSdeMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);

int32_t tivxDmpacSdeFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxDmpacSdeErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxDmpacSdeWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);
static vx_status tivxEnableDmpacSdeSafetyMechanisms(
    tivxDmpacSdeObj *sdeObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacSdeAllocReadbackBuffers(tivxDmpacSdeObj *sdeObj);
static void tivxDmpacSdeFreeReadbackBuffers(tivxDmpacSdeObj *sdeObj);
static int32_t tivxDmpacSdeConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_dmpac_sde_target_kernel = NULL;

tivxDmpacSdeInstObj gTivxDmpacSdeInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelDmpacSde(void)
{
    vx_status status = (vx_status)VX_FAILURE;
    char target_name[TIVX_TARGET_MAX_NAME];
    vx_enum self_cpu;

    self_cpu = tivxGetSelfCpuId();
    /* LDRA_JUSTIFY_START 
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
    <justification end> */
    if (self_cpu == (vx_enum)TIVX_CPU_ID_DMPAC)
    /* LDRA_JUSTIFY_END */
    {
        strncpy(target_name, TIVX_TARGET_DMPAC_SDE, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        vx_dmpac_sde_target_kernel = tivxAddTargetKernelByName(
                    TIVX_KERNEL_DMPAC_SDE_NAME,
                    target_name,
                    tivxDmpacSdeProcess,
                    tivxDmpacSdeCreate,
                    tivxDmpacSdeDelete,
                    tivxDmpacSdeControl,
                    NULL);
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vx_dmpac_sde_target_kernel)
        /* LDRA_JUSTIFY_END */
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxDmpacSdeInstObj.lock);
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
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                memset(&gTivxDmpacSdeInstObj.sdeObj, 0x0,
                    sizeof(tivxDmpacSdeObj) * VHWA_M2M_SDE_MAX_HANDLES);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Add SDE TargetKernel\n");
        }
        /* LDRA_JUSTIFY_END */
    }
}
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The test framework and test apps cannot reach this portion.
The function is unused because of an un-implemented feature, this code base is not being used/enabled with the current kernel support.
Effect on this unit: There is no impact, as the feature is unused, cannot be enabled, and control cannot reach this condition.
<justification end> */
void tivxRemoveTargetKernelDmpacSde(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dmpac_sde_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_dmpac_sde_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Sde TargetKernel\n");
    }
    if (NULL != gTivxDmpacSdeInstObj.lock)
    {
        (void)tivxMutexDelete(&gTivxDmpacSdeInstObj.lock);
    }
}
/* LDRA_JUSTIFY_END */


/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxDmpacSdeProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          size;
    void                             *confidence_histogram_target_ptr = NULL;
    tivxDmpacSdeObj                  *sde_obj = NULL;
    tivx_obj_desc_image_t            *left_desc;
    tivx_obj_desc_image_t            *right_desc;
    tivx_obj_desc_image_t            *output_desc;
    tivx_obj_desc_distribution_t     *confidence_histogram_desc = NULL;
    Fvid2_FrameList                  *inFrmList;
    Fvid2_FrameList                  *outFrmList;
    uint64_t                         cur_time;
    tivx_obj_desc_t                  *out_base_desc = NULL;
    vx_status                        validate_reg_status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
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
            sde_obj = temp_kernel_context;
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
            VX_PRINT(VX_ZONE_ERROR, "Null Desc\n");
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
        else if (sizeof(tivxDmpacSdeObj) != size)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Object Size\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        {
            /* do nothing */
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
        inFrmList                 = &sde_obj->inFrmList;
        outFrmList                = &sde_obj->outFrmList;

        left_desc                 = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
        right_desc                = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
        output_desc               = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
        confidence_histogram_desc = (tivx_obj_desc_distribution_t *)obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIDENCE_HISTOGRAM_IDX];
        
        out_base_desc = (tivx_obj_desc_t *)output_desc;
        sde_obj->timestamp = out_base_desc->timestamp;

        if( confidence_histogram_desc != NULL)
        {
            confidence_histogram_target_ptr = tivxMemShared2TargetPtr(&confidence_histogram_desc->mem_ptr);
            tivxCheckStatus(&status, tivxMemBufferMap(confidence_histogram_target_ptr,
                confidence_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }

        /* Initialize SDE Input Frame List */
        inFrmList->frames[SDE_INPUT_BASE_IMG] =
            &sde_obj->inFrm[SDE_INPUT_BASE_IMG];
        inFrmList->frames[SDE_INPUT_REFERENCE_IMG] =
            &sde_obj->inFrm[SDE_INPUT_REFERENCE_IMG];
        inFrmList->numFrames = 2U;

        sde_obj->inFrm[SDE_INPUT_BASE_IMG].addr[0] = tivxMemShared2PhysPtr(
                left_desc->mem_ptr[0].shared_ptr,
                (int32_t)left_desc->mem_ptr[0].mem_heap_region);
        sde_obj->inFrm[SDE_INPUT_REFERENCE_IMG].addr[0] = tivxMemShared2PhysPtr(
                right_desc->mem_ptr[0].shared_ptr,
                (int32_t)right_desc->mem_ptr[0].mem_heap_region);

        /* Initialize SDE Output Frame List */
        outFrmList->frames[0U] = &sde_obj->outFrm;
        outFrmList->numFrames = 1U;

        sde_obj->outFrm.addr[0] = tivxMemShared2PhysPtr(
                output_desc->mem_ptr[0].shared_ptr,
                (int32_t)output_desc->mem_ptr[0].mem_heap_region);

        /* Clearing error status from previous frame
        *  User should consume the error status before
        *  submitting next frame for processing
        */
        sde_obj->wdTimerErrStatus = 0u;
        sde_obj->err_stat = 0u;

        cur_time = tivxPlatformGetTimeInUsecs();

        /* Submit SDE Request*/
        fvid2_status = Fvid2_processRequest(sde_obj->handle, inFrmList,
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
        (void)tivxEventWait(sde_obj->waitForProcessCmpl, VX_TIMEOUT_WAIT_FOREVER);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is covered in VHWA_NEGATIVE_TEST build.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(0u == sde_obj->wdTimerErrStatus)
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
            if(0u != sde_obj->err_stat)
            {
                VX_PRINT(VX_ZONE_INFO, "Error interrupt: SDE non-stalling error interrupt triggered \n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            fvid2_status = Fvid2_getProcessedRequest(sde_obj->handle,
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
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is covered in VHWA_NEGATIVE_TEST build.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "IP stall: Watchdog timer error interrupt triggered \n");
            status = (vx_status)VX_ERROR_TIMEOUT;
        }
        /* LDRA_JUSTIFY_END */

        /* Call the control command for statusreg/configreg validate */
        {
            fvid2_status = Fvid2_control(sde_obj->handle, VHWA_M2M_IOCTL_SDE_VALIDATE_REG, NULL, NULL);
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Register validation failed (Fvid2_control returned %d)\n", fvid2_status);
                validate_reg_status = (vx_status)VX_FAILURE;
            }
        }

        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Get Histogram */
        if(NULL != confidence_histogram_desc)
        {
            fvid2_status = Fvid2_control(sde_obj->handle,
                VHWA_M2M_IOCTL_SDE_GET_HISTOGRAM,
                (uint32_t *) confidence_histogram_target_ptr, NULL);
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
                VX_PRINT(VX_ZONE_ERROR, "Histogram Request failed\n");
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
        Vhwa_M2mSdePrms                  *sdePrms = NULL;

        sdePrms = &sde_obj->sdePrms;

        

        appPerfStatsHwaUpdateLoad(APP_PERF_HWA_SDE,
            (uint32_t)cur_time,
            sdePrms->sdeCfg.width*sdePrms->sdeCfg.height /* pixels processed */
            );
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
        if( confidence_histogram_desc != NULL)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(confidence_histogram_target_ptr,
                confidence_histogram_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
                (vx_enum)VX_WRITE_ONLY));
        }
    }

    if(((vx_status)VX_SUCCESS != status) || ((vx_status)VX_SUCCESS != validate_reg_status))
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    uint32_t                          i;
    uint32_t                          aligned_width;
    uint32_t                          aligned_height;
    tivxDmpacSdeObj                  *sde_obj = NULL;
    Vhwa_M2mSdePrms                  *sdePrms = NULL;
    tivx_dmpac_sde_params_t          *params = NULL;
    tivx_obj_desc_user_data_object_t *params_array = NULL;
    tivx_obj_desc_image_t            *left_desc;
    tivx_obj_desc_image_t            *right_desc;
    tivx_obj_desc_image_t            *output_desc;
    void                             *params_array_target_ptr = NULL;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        sde_obj = tivxDmpacSdeAllocObject(&gTivxDmpacSdeInstObj);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release..
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != sde_obj)
        /* LDRA_JUSTIFY_END */
        {
            params_array = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX];
            left_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX];
            right_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX];
            output_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX];
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Sde Bilateral Object\n");
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
        Vhwa_M2mSdeCreateArgsInit(&sde_obj->createPrms);
        sdePrms = &sde_obj->sdePrms;

        params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        params = (tivx_dmpac_sde_params_t *)params_array_target_ptr;

        sde_obj->createPrms.enablePsa = params->enable_psa;

        /* Initialize SDE Config with defaults */
        Sde_ConfigInit(&sdePrms->sdeCfg);

        status = tivxEventCreate(&sde_obj->waitForProcessCmpl);
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
            sde_obj->cbPrms.cbFxn   = tivxDmpacSdeFrameComplCb;
            sde_obj->cbPrms.appData = sde_obj;

            sde_obj->handle = Fvid2_create(FVID2_VHWA_M2M_SDE_DRV_ID,
                VHWA_M2M_SDE_DRV_INST_ID, (void *)&sde_obj->createPrms,
                NULL, &sde_obj->cbPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == sde_obj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Sde Object\n");

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
        sde_obj->errEvtPrms.errEvents = VHWA_SDE_ALL_ERR_EVENTS;
        sde_obj->errEvtPrms.cbFxn     = tivxDmpacSdeErrorCb;
        sde_obj->errEvtPrms.appData   = sde_obj;

        fvid2_status = Fvid2_control(sde_obj->handle,
            VHWA_M2M_IOCTL_SDE_REGISTER_ERR_CB, &sde_obj->errEvtPrms, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Register Error Callback\n");
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
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Set default 64K cycle timeout during registration */
        sde_obj->wdTimerErrEvtPrms.wdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        sde_obj->wdTimerErrEvtPrms.focoWdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        /* Disable Watchdog Timer Error Event by default */
        sde_obj->wdTimerErrEvtPrms.wdTimerErrEvents = 0u;
        sde_obj->wdTimerErrEvtPrms.cbFxn     = tivxDmpacSdeWdTimerErrorCb;
        sde_obj->wdTimerErrEvtPrms.appData   = sde_obj;
        fvid2_status = Fvid2_control(sde_obj->handle,
            VHWA_M2M_IOCTL_SDE_REGISTER_WDTIMER_ERR_CB, &sde_obj->wdTimerErrEvtPrms, NULL);
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
        sdePrms = &sde_obj->sdePrms;

        params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        params = (tivx_dmpac_sde_params_t *)params_array_target_ptr;

        /* Initialize SDE Config with defaults */
        Sde_ConfigInit(&sdePrms->sdeCfg);

        /* Set SDE Config parameters */
        aligned_width = left_desc->imagepatch_addr[0].dim_x;
        aligned_height = left_desc->imagepatch_addr[0].dim_y;
        if (aligned_width < 128U) {
            aligned_width = 128U;    /* Minimum width = 128 */
        }
        if (((aligned_width & 15U) != 0U)) {
            aligned_width += 16U;
            aligned_width &= (~15U);   /* Must be multiple of 16 */
        }
        if (aligned_height < 64U) {
            aligned_height = 64U;    /* Minimum height = 64 */
        }
        if (((aligned_height & 15U)!= 0U)) {
            aligned_height += 16U;
            aligned_height &= (~15U);   /* Must be multiple of 16 */
        }
        if (SDE_MAX_IMAGE_WIDTH < aligned_width)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Aligned width should be no greater than %d\n", SDE_MAX_IMAGE_WIDTH);
        }
        if (SDE_MAX_IMAGE_HEIGHT < aligned_height)
        {
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            VX_PRINT(VX_ZONE_ERROR, "Parameter height should be no greater than %d\n", SDE_MAX_IMAGE_HEIGHT);
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        sdePrms->sdeCfg.enableSDE = 1U;
        sdePrms->sdeCfg.medianFilter = (uint32_t) (params->median_filter_enable);
        sdePrms->sdeCfg.width = aligned_width;
        sdePrms->sdeCfg.height = aligned_height;
        sdePrms->sdeCfg.minDisparity = (uint32_t) (params->disparity_min);
        sdePrms->sdeCfg.searchRange = (uint32_t) (params->disparity_max);
        sdePrms->sdeCfg.lrThreshold = (uint32_t)  (params->threshold_left_right);
        sdePrms->sdeCfg.enableTextureFilter = (uint32_t) (params->texture_filter_enable);
        sdePrms->sdeCfg.textureFilterThreshold = (uint32_t) (params->threshold_texture);
        sdePrms->sdeCfg.penaltyP1 = (uint32_t) (params->aggregation_penalty_p1);
        sdePrms->sdeCfg.penaltyP2 = (uint32_t) (params->aggregation_penalty_p2);
        for(i = 0U; i < DMPAC_SDE_NUM_SCORE_MAP; i++)
        {
            sdePrms->sdeCfg.confScoreMap[i] = (uint32_t) (params->confidence_score_map[i]);
        }

        tivxDmpacSdeSetFmt(&sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG],
            left_desc);
        tivxDmpacSdeSetFmt(&sdePrms->inOutImgFmt[SDE_INPUT_REFERENCE_IMG],
            right_desc);
        tivxDmpacSdeSetFmt(&sdePrms->inOutImgFmt[SDE_OUTPUT],
            output_desc);

        sdePrms->focoPrms.shiftM1 = 0u;
        sdePrms->focoPrms.dir = 0u;
        sdePrms->focoPrms.round = 0u;

        if(sdePrms->inOutImgFmt[SDE_INPUT_BASE_IMG].ccsFormat
            == (uint32_t)FVID2_CCSF_BITS8_PACKED)
        {
            sdePrms->focoPrms.shiftM1 = 4u;
        }

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           DMPAC SDE parameters needs to be reconfigured */

        memcpy(&sde_obj->sdeParams, params, sizeof(tivx_dmpac_sde_params_t));

        fvid2_status = Fvid2_control(sde_obj->handle,
            VHWA_M2M_IOCTL_SDE_SET_PARAMS, &sde_obj->sdePrms, NULL);
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
            if (FVID2_EALLOC == fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Not enough SL2 memory for this configuration\n");
            }
        }
        /* LDRA_JUSTIFY_END */

        tivxCheckStatus(&status, tivxMemBufferUnmap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        status = tivxSetTargetKernelInstanceContext(kernel, sde_obj,
            sizeof(tivxDmpacSdeObj));
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
        if (NULL != sde_obj)
        {
            if (NULL != sde_obj->handle)
            {
                (void)Fvid2_delete(sde_obj->handle, NULL);
                sde_obj->handle = NULL;
            }

            if (NULL != sde_obj->waitForProcessCmpl)
            {
                (void)tivxEventDelete(&sde_obj->waitForProcessCmpl);
            }

            tivxDmpacSdeFreeObject(&gTivxDmpacSdeInstObj, sde_obj);
        }
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 size;
    tivxDmpacSdeObj    *sde_obj = NULL;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_DMPAC_SDE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_LEFT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_RIGHT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_SDE_OUTPUT_IDX])
    )
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        void *temp_kernel_context = NULL;
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
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
           sde_obj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) && (NULL != sde_obj) &&
            (sizeof(tivxDmpacSdeObj) == size))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
                                             False condition can't be evoked from test as no control over sde_object
                                  Effect on this unit: No effect on this unit as this is a cleanup code.
            <justification end> */
            if (NULL != sde_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(sde_obj->handle, NULL);
                sde_obj->handle = NULL;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
                                             False condition can't be evoked from test as no control over sde_object
                                  Effect on this unit: No effect on this unit as this is a cleanup code.
            <justification end> */
            if (NULL != sde_obj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&sde_obj->waitForProcessCmpl);
            }

            /* Free the readback and golden register buffers */
            tivxDmpacSdeFreeReadbackBuffers(sde_obj);

            tivxDmpacSdeFreeObject(&gTivxDmpacSdeInstObj, sde_obj);
        }
    }


    return status;
}

static vx_status VX_CALLBACK tivxDmpacSdeControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxDmpacSdeObj                  *sde_obj = NULL;

    void *temp_kernel_context = NULL;

    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
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
        sde_obj = temp_kernel_context;
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
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((NULL == sde_obj) ||
        (sizeof(tivxDmpacSdeObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Wrong Size for Sde Obj\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /* do nothing */
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
            case TIVX_DMPAC_SDE_CMD_GET_ERR_STATUS:
            {
                status = tivxDmpacSdeGetErrStatusCmd(sde_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_SDE_CMD_GET_PSA_STATUS:
            {
                status = tivxDmpacSdeGetPsaStatusCmd(sde_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_SDE_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxDmpacSdeEnableErrorEventsCmd(sde_obj,
                  (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
#if defined(LDRA_COVERAGE)
            case TIVX_DMPAC_CMD_GET_COVERAGE_END:
            {
                #if defined (C7X_FAMILY) || defined(R5F)
                VX_PRINT(VX_ZONE_ERROR, "################### DYNAMIC CODE COVERAGE TERMINATION STARTED ######################\n");
                ldra_terminate();
                VX_PRINT(VX_ZONE_ERROR, "################### DYNAMIC CODE COVERAGE TERMINATION FINISHED ######################\n");
                #endif
                break;
            }
#endif
            case TIVX_DMPAC_SDE_CMD_ENABLE_DMPAC_SAFETY_MECHANISM:
            {
                status = tivxEnableDmpacSdeSafetyMechanisms(sde_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0]);
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to enable DMPAC SDE safety mechanisms\n");
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

static tivxDmpacSdeObj *tivxDmpacSdeAllocObject(
       tivxDmpacSdeInstObj *instObj)
{
    uint32_t        cnt;
    tivxDmpacSdeObj *sde_obj = NULL;
    vx_status status;


    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> 
    Rationale: The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 4 SDE handles
    Effect on this unit: It will utilise all the 4 SDE handles which are validated in the SDE driver.
    <justification end> */
    for (cnt = 0U; cnt < VHWA_M2M_SDE_MAX_HANDLES; cnt ++)
    /* LDRA_JUSTIFY_END */

    {
        if (0U == instObj->sdeObj[cnt].isAlloc)
        {
            sde_obj = &instObj->sdeObj[cnt];
            memset(sde_obj, 0x0, sizeof(tivxDmpacSdeObj));
            instObj->sdeObj[cnt].isAlloc = 1U;
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
    return (sde_obj);
}

static void tivxDmpacSdeFreeObject(tivxDmpacSdeInstObj *instObj,
    tivxDmpacSdeObj *sde_obj)
{
    uint32_t cnt;
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> 
        Rationale:The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 4 SDE handles
        Effect on this unit: It will free all the 4 SDE handles which are validated in the SDE driver.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_SDE_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (sde_obj == &instObj->sdeObj[cnt])
            {
                sde_obj->isAlloc = 0U;
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

static void tivxDmpacSdeSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != img_desc)
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
        switch (img_desc->format)
        {
        /* LDRA_JUSTIFY_END */
            case (vx_df_image)VX_DF_IMAGE_U8:
            case (vx_df_image)VX_DF_IMAGE_NV12:
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
            case (vx_df_image)VX_DF_IMAGE_S16:
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
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
                                             Default case can't be entered as formats already filtered from host kernel.
                                  Effect on this unit: Prints an error message due to invalid Vx Image Format.
            <justification end> */
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Vx Image Format\n");
                break;
            }
            /* LDRA_JUSTIFY_END */
        }

        fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
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
static vx_status tivxDmpacSdeMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (((0u != (vhwa_err_events & (~VHWA_SDE_ALL_ERR_EVENTS)))) ||
             ((0u != (wd_timer_err & (~VHWA_SDE_ALL_WDTIMER_ERR_EVENTS)))))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask provided\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Map VHWA flags to TIVX flags for regular error interrupts */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_SDE_RD_ERR))
        {
            mapped_tivx_err_events |= TIVX_DMPAC_SDE_RD_ERR;
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
        if (0u != (vhwa_err_events & VHWA_SDE_WR_ERR))
        {
            mapped_tivx_err_events |= TIVX_DMPAC_SDE_WR_ERR;
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
        if (0u != (vhwa_err_events & VHWA_SDE_FOCO0_SL2_WR_ERR))
        {
            mapped_tivx_err_events |= TIVX_DMPAC_SDE_FOCO0_SL2_WR_ERR;
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
        if (0u != (vhwa_err_events & VHWA_SDE_FOCO0_VBUSM_RD_ERR))
        {
            mapped_tivx_err_events |= TIVX_DMPAC_SDE_FOCO0_VBUSM_RD_ERR;
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
        /* Map VHWA flags to TIVX flags for watchdog timer errors */
        if (0u != (wd_timer_err & VHWA_SDE_WDTIMER_ERR))
        {
            mapped_tivx_err_events |= TIVX_DMPAC_SDE_WDTIMER_ERR;
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
        if (0u != (wd_timer_err & VHWA_SDE_FOCO1_WDTIMER_ERR))
        {
            mapped_tivx_err_events |= TIVX_DMPAC_SDE_FOCO1_WDTIMER_ERR;
        }
		/* LDRA_JUSTIFY_END */

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
static vx_status tivxDmpacSdeMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
                                                uint32_t *vhwa_err_events,
                                                uint32_t *wd_timer_err)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t enableErrorEventCheckMask = (uint32_t)TIVX_DMPAC_SDE_ALL_ERR_EVENTS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated the caller.
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
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated the caller.
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

        /* Map TIVX flags to VHWA flags for regular error interrupts */
        if (0u != (tivx_err_events & TIVX_DMPAC_SDE_RD_ERR))
        {
            *vhwa_err_events |= VHWA_SDE_RD_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_SDE_WR_ERR))
        {
            *vhwa_err_events |= VHWA_SDE_WR_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_SDE_FOCO0_SL2_WR_ERR))
        {
            *vhwa_err_events |= VHWA_SDE_FOCO0_SL2_WR_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_SDE_FOCO0_VBUSM_RD_ERR))
        {
            *vhwa_err_events |= VHWA_SDE_FOCO0_VBUSM_RD_ERR;
        }

        /* Map TIVX flags to VHWA flags for watchdog timer errors */
        if (0u != (tivx_err_events & TIVX_DMPAC_SDE_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_SDE_WDTIMER_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_SDE_FOCO1_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_SDE_FOCO1_WDTIMER_ERR;
        }
    }

    return status;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxDmpacSdeGetErrStatusCmd(const tivxDmpacSdeObj *sde_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat = 0u;

    if (NULL != scalar_obj_desc)
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxDmpacSdeMapVhwaToTivxErrEvents(sde_obj->err_stat, sde_obj->wdTimerErrStatus, &combined_err_stat);
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
            scalar_obj_desc->data.u32 = combined_err_stat;
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask from hardware\n");
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxDmpacSdeGetPsaStatusCmd(const tivxDmpacSdeObj *sde_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            psa;
    int32_t                             fvid2_status = FVID2_SOK;
    void                                *target_ptr;
    tivx_dmpac_sde_psa_timestamp_data_t *psa_status_ptr;
    
    if (NULL != usr_data_obj)
    {
        if (sizeof(tivx_dmpac_sde_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            fvid2_status = Fvid2_control(sde_obj->handle,
                VHWA_M2M_IOCTL_SDE_GET_PSA_SIGN, &psa, NULL);
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
                    "tivxDmpacSdeGetPsa: Fvid2_control Failed \n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
            /* Map the user data object to access the memory */
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            psa_status_ptr = (tivx_dmpac_sde_psa_timestamp_data_t *)target_ptr;

            /* Copy driver data (PSA values) into the structure */
            tivx_obj_desc_memcpy(&psa_status_ptr->psa_values, &psa, sizeof(psa));

            /* Add timestamp to the structure */
            psa_status_ptr->timestamp = sde_obj->timestamp;

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

static vx_status tivxDmpacSdeEnableErrorEventsCmd(tivxDmpacSdeObj *sde_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t intr_en = 0u;
    uint32_t wd_timer_intr_en = 0u;
    void *target_ptr;
    tivx_dmpac_sde_params_t *params_ptr = NULL;
    uint32_t enableErrorEventsMask = 0u;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL != usr_data_obj)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(sizeof(tivx_dmpac_sde_params_t) != usr_data_obj->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxDmpacSdeEnableErrorEventsCmd: Incorrect Data Object Size \n");
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
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR,
                    "tivxDmpacSdeEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_dmpac_sde_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->enable_error_events;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxDmpacSdeMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

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
                    VX_PRINT(VX_ZONE_ERROR,
                        "tivxDmpacSdeEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    sde_obj->errEvtPrms.errEvents = intr_en;
                    fvid2_status = Fvid2_control(sde_obj->handle,
                        VHWA_M2M_IOCTL_SDE_REGISTER_ERR_CB, (void *)&sde_obj->errEvtPrms, NULL);

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
                        sde_obj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->wdtimer_timeout_cycles;
                        sde_obj->wdTimerErrEvtPrms.focoWdTimeoutCycles = params_ptr->foco_wdtimer_timeout_cycles;
                        sde_obj->wdTimerErrEvtPrms.wdTimerErrEvents = wd_timer_intr_en;
                        fvid2_status = Fvid2_control(sde_obj->handle,
                            VHWA_M2M_IOCTL_SDE_REGISTER_WDTIMER_ERR_CB, (void *)&sde_obj->wdTimerErrEvtPrms, NULL);
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
                            "tivxDmpacSdeEnableErrorEventsCmd: Fvid2_control Failed \n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR,
            "tivxDmpacSdeEnableErrorEventsCmd: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status tivxEnableDmpacSdeSafetyMechanisms(
    tivxDmpacSdeObj *sdeObj,
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == sdeObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "sdeObj is NULL\n");
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
        The parameter is expected to be pre-validated from a software layer above imaging.
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
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                if (sizeof(tivx_dmpac_sde_safety_mechanism_params_t) == usr_data_obj->mem_size)
                {
                    tivx_dmpac_sde_safety_mechanism_params_t *safety_params =
                        (tivx_dmpac_sde_safety_mechanism_params_t *)params_ptr;

                    /* Enable or disable reconfigMMR based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_reconfig_and_reinit_Reg;
                    fvid2_status = Fvid2_control(sdeObj->handle, VHWA_M2M_IOCTL_SDE_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
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

                    /* Enable or disable status register validation based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_status_reg_validation;
                    fvid2_status = Fvid2_control(sdeObj->handle, VHWA_M2M_IOCTL_SDE_ENABLE_STATUS_REG_VALIDATE, &enable, NULL);
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

                    /* Enable or disable config register validation based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    if (0U != safety_params->enable_readback_config_registers)
                    {
                        uint32_t readback_size = 0u;

                        /* Fetch the TIVX_DMPAC_SDE_READBACK_SIZE from the low level driver */
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if(((vx_status)VX_SUCCESS == status))
                        /* LDRA_JUSTIFY_END */
                        {
                            fvid2_status = Fvid2_control(
                                sdeObj->handle,
                                VHWA_M2M_IOCTL_SDE_GET_READBACK_SIZE,
                                &readback_size,
                                NULL
                            );

                            sdeObj->config_reg_mem_size = readback_size;

                            if (((FVID2_SOK == fvid2_status) && (readback_size > 0u)))
                            {
                                VX_PRINT(VX_ZONE_INFO, "SDE readback size allocation successful: %u bytes\n", readback_size);
                            }
                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case is out of scope for the imaging test framework.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            else
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to fetch SDE readback size from driver\n");
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
                        if((((vx_status)VX_SUCCESS == status) && (readback_size > 0u)))
                        /* LDRA_JUSTIFY_END */
                        {
                            /* Before allocation, check if any of the buffers are already allocated */
                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            The buffers are expected to be unallocated during first-time configuration.
                            Effect on this unit: If the control reaches here, our code base is expected to skip buffer allocation to prevent memory leaks.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((sdeObj->readback_mem_ptr_phys != 0u) && (sdeObj->golden_reg_mem_ptr_phys != 0u))
                            {
                                VX_PRINT(VX_ZONE_INFO, "Readback or golden register buffer already allocated, skipping allocation\n");
                            }
                            /* LDRA_JUSTIFY_END */
                            else
                            {
                                status = tivxDmpacSdeAllocReadbackBuffers(sdeObj);
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
                            fvid2_status = Fvid2_control(sdeObj->handle, VHWA_M2M_IOCTL_SDE_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                                tivxDmpacSdeFreeReadbackBuffers(sdeObj);
                                status = (vx_status)VX_FAILURE;
                            }
                            /* LDRA_JUSTIFY_END */
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "SDE readback buffer allocation failed, skipping config register validation enable\n");
                        }
                    }
                    else
                    {
                        enable = 0u;
                        fvid2_status = Fvid2_control(sdeObj->handle, VHWA_M2M_IOCTL_SDE_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
                    status = (vx_status)VX_FAILURE;
                }

                tivxCheckStatus(&status, tivxMemBufferUnmap(params_ptr, usr_data_obj->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
            }
        }
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status tivxDmpacSdeAllocReadbackBuffers(tivxDmpacSdeObj *sdeObj)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == sdeObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxDmpacSdeAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The handle is expected to be initialized during object creation.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if (NULL == sdeObj->handle)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxDmpacSdeAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Valid sdeObj and handle */
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
        uint32_t readback_size = sdeObj->config_reg_mem_size;

        vx_status alloc_status = tivxMemBufferAlloc(&sdeObj->readback_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);
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
            sdeObj->readback_mem_ptr_phys = tivxMemShared2PhysPtr(sdeObj->readback_mem_ptr_virt.shared_ptr, (int32_t)sdeObj->readback_mem_ptr_virt.mem_heap_region);
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
            Fvid2Utils_memset((void *)(uintptr_t)sdeObj->readback_mem_ptr_virt.shared_ptr, 0u, readback_size);
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
            alloc_status = tivxMemBufferAlloc(&sdeObj->golden_reg_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);

            sdeObj->golden_reg_mem_ptr_phys = tivxMemShared2PhysPtr(sdeObj->golden_reg_mem_ptr_virt.shared_ptr, (int32_t)sdeObj->golden_reg_mem_ptr_virt.mem_heap_region);
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
            Fvid2Utils_memset((void *)(uintptr_t)sdeObj->golden_reg_mem_ptr_virt.shared_ptr, 0u, readback_size);
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
        if((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_M2mSdeConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ptr   = (void *)(uintptr_t)sdeObj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)sdeObj->readback_mem_ptr_virt.shared_ptr;

            /* Register the callback for config register memory comparison */
            reg_ptrs.cbFxn = tivxDmpacSdeConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)sdeObj;

            int32_t fvid2_status = Fvid2_control(
                sdeObj->handle,
                VHWA_M2M_IOCTL_SDE_SET_CONFIG_REG_PTRS,
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set SDE register readback/golden pointers\n");
                status = (vx_status)VX_FAILURE;
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
            if ((vx_status)VX_SUCCESS != status)
            {
                tivxDmpacSdeFreeReadbackBuffers(sdeObj);
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory allocation is expected to succeed in normal test conditions.
        Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations and return error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            tivxDmpacSdeFreeReadbackBuffers(sdeObj);
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static void tivxDmpacSdeFreeReadbackBuffers(tivxDmpacSdeObj *sdeObj)
{
    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != sdeObj)
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;
        uint32_t readback_size = sdeObj->config_reg_mem_size;

        /* Free readback buffer if allocated */
        if (sdeObj->readback_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&sdeObj->readback_mem_ptr_virt, readback_size);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Memory free operations are expected to succeed in normal conditions.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free readback buffer\n");
            }
            /* LDRA_JUSTIFY_END */
            sdeObj->readback_mem_ptr_phys = 0u;
        }

        /* Free golden register buffer if allocated */
        if (sdeObj->golden_reg_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&sdeObj->golden_reg_mem_ptr_virt, readback_size);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Memory free operations are expected to succeed in normal conditions.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free golden register buffer\n");
            }
            /* LDRA_JUSTIFY_END */
            sdeObj->golden_reg_mem_ptr_phys = 0u;
        }
    }
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxDmpacSdeFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxDmpacSdeObj *sde_obj = (tivxDmpacSdeObj *)appData;
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != sde_obj)
    /* LDRA_JUSTIFY_END */
    {
        (void)tivxEventPost(sde_obj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is covered in VHWA_NEGATIVE_TEST build.
Effect on this unit: If this portion is reached, our application is expected to throw an error.
However, due to the stated rationale, this is not tested for normal build.
<justification end> */
void tivxDmpacSdeErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxDmpacSdeObj *sde_obj = (tivxDmpacSdeObj *)appData;

    if (NULL != sde_obj)
    {
        sde_obj->err_stat = errEvents;
    }
}
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: The component level negative test framework and test applications cannot reach this portion.
This failure case is covered in VHWA_NEGATIVE_TEST build.
Effect on this unit: If this portion is reached, our application is expected to throw an error.
However, due to the stated rationale, this is not tested for normal build.
<justification end> */
static void tivxDmpacSdeWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxDmpacSdeObj *sde_obj = (tivxDmpacSdeObj *)appData;
    if (NULL != sde_obj)
    {
        sde_obj->wdTimerErrStatus = sde_obj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents;
        
        if(0u != sde_obj->wdTimerErrStatus)
        {
            (void)tivxEventPost(sde_obj->waitForProcessCmpl);
        }
    }
}

/* LDRA_JUSTIFY_END */
/* Callback for config register and golden register memory comparison */
static int32_t tivxDmpacSdeConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mSdeConfigRegMemParams *sde_config_reg_prms = (Vhwa_M2mSdeConfigRegMemParams *)configRegPrms;
    tivxDmpacSdeObj *sde_obj = NULL;

    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != sde_config_reg_prms) && (NULL != handle))
    /* LDRA_JUSTIFY_END */
    {
        sde_obj = (tivxDmpacSdeObj *)sde_config_reg_prms->appData;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != sde_config_reg_prms) && (NULL != handle) && (NULL != sde_obj))
    /* LDRA_JUSTIFY_END */
    {
        /* Compare the golden register memory and readback register memory using sde_config_reg_prms */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The memory pointers are expected to be pre-allocated and validated.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != sde_config_reg_prms->golden_ptr) &&
            (NULL != sde_config_reg_prms->readback_ptr) &&
            (sde_config_reg_prms->readback_mem_size > 0u))
        /* LDRA_JUSTIFY_END */
        {
            int32_t cmp_result = memcmp(
                sde_config_reg_prms->golden_ptr,
                sde_config_reg_prms->readback_ptr,
                sde_config_reg_prms->readback_mem_size);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Memory content comparison mismatch indicates hardware register corruption which is not expected in normal operation.
            Effect on this unit: If the control reaches here, our code base is expected to report safety violation and return error status.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0 != cmp_result)
            {
                VX_PRINT(VX_ZONE_ERROR, "SDE config register mismatch detected!\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                VX_PRINT(VX_ZONE_INFO, "SDE config register match successful.\n");
            }
        }
    }
    else
    {
        status = FVID2_EFAIL;
    }

    return status;
}

BUILD_ASSERT((((VHWA_SDE_RD_ERR == TIVX_DMPAC_SDE_RD_ERR))? 1U : 0U));
BUILD_ASSERT((((VHWA_SDE_WR_ERR ==TIVX_DMPAC_SDE_WR_ERR))? 1U : 0U));
BUILD_ASSERT((((VHWA_SDE_FOCO0_SL2_WR_ERR == TIVX_DMPAC_SDE_FOCO0_SL2_WR_ERR))? 1U : 0U));
BUILD_ASSERT((((VHWA_SDE_FOCO0_VBUSM_RD_ERR == TIVX_DMPAC_SDE_FOCO0_VBUSM_RD_ERR))? 1U : 0U));
BUILD_ASSERT((sizeof(uint32_t) == sizeof(((tivx_dmpac_sde_psa_timestamp_data_t *)0)->psa_values))? 1U : 0U);
