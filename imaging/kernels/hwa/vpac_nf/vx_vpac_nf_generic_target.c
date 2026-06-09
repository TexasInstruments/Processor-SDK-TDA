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
 * Redistributions must preserve existing copyright prmnotices and reproduce this license
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
#include "tivx_kernel_vpac_nf_generic.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_nf_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include "vhwa/include/vhwa_m2mNf.h"
#include "utils/perf_stats/include/app_perf_stats.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct
{
    uint32_t                            isAlloc;
    tivx_vpac_nf_common_params_t        nfGenericParams;
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
} tivxVpacNfGenericObj;

typedef struct
{
    tivx_mutex lock;
    tivxVpacNfGenericObj nfGenericObj[VHWA_M2M_NF_MAX_HANDLES];
} tivxVpacNfGenericInstObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacNfGenericProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfGenericCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfGenericDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacNfGenericControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static tivxVpacNfGenericObj *tivxVpacNfGenericAllocObject(
       tivxVpacNfGenericInstObj *instObj);
static void tivxVpacNfGenericFreeObject(
       tivxVpacNfGenericInstObj *instObj, tivxVpacNfGenericObj *nf_generic_obj);
static void tivxVpacNfSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static vx_status tivxVpacNfGenericSetHtsLimitCmd(
    tivxVpacNfGenericObj *nf_generic_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfGenericSetCoeff(tivxVpacNfGenericObj *nf_generic_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfGenericGetErrStatusCmd(const tivxVpacNfGenericObj *nf_generic_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxVpacNfGenericGetPsaStatusCmd(const tivxVpacNfGenericObj *nf_generic_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfGenericEnableErrorEventsCmd(tivxVpacNfGenericObj *nf_generic_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacNfGenericMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxVpacNfGenericMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);

int32_t tivxVpacNfGenericFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxVpacNfGenericErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxVpacNfGenericWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);
static vx_status tivxEnableVpacNfGenericSafetyMechanisms(tivxVpacNfGenericObj *nf_generic_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);

int32_t tivxVpacNfGenericConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms);

static vx_status tivxVpacNfGenericAllocReadbackBuffers(tivxVpacNfGenericObj *nf_generic_obj);

static void tivxVpacNfGenericFreeReadbackBuffers(tivxVpacNfGenericObj *nf_generic_obj);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_vpac_nf_generic_target_kernel = NULL;

tivxVpacNfGenericInstObj gTivxVpacNfGenericInstObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacNfGeneric(void)
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
    if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC1)
    /* LDRA_JUSTIFY_END */
    {
        strncpy(target_name, TIVX_TARGET_VPAC_NF, TIVX_TARGET_MAX_NAME);
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
        strncpy(target_name, TIVX_TARGET_VPAC2_NF, TIVX_TARGET_MAX_NAME);
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */  
    if (status == (vx_status)VX_SUCCESS)
    {
    /* LDRA_JUSTIFY_END */
        vx_vpac_nf_generic_target_kernel = tivxAddTargetKernelByName(
                    TIVX_KERNEL_VPAC_NF_GENERIC_NAME,
                    target_name,
                    tivxVpacNfGenericProcess,
                    tivxVpacNfGenericCreate,
                    tivxVpacNfGenericDelete,
                    tivxVpacNfGenericControl,
                    NULL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vx_vpac_nf_generic_target_kernel)
        /* LDRA_JUSTIFY_END */
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxVpacNfGenericInstObj.lock);
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
                    "Failed to create Mutex\n");
            }
            else
            /* LDRA_JUSTIFY_END */
            {
                memset(&gTivxVpacNfGenericInstObj.nfGenericObj, 0x0,
                    sizeof(tivxVpacNfGenericObj) * VHWA_M2M_NF_MAX_HANDLES);
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
                "Failed to Add NF Generic TargetKernel\n");
        }
        /* LDRA_JUSTIFY_END */ 
    }
}
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> 
Rationale: The component level test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelVpacNfGeneric(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_vpac_nf_generic_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_vpac_nf_generic_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Nf TargetKernel\n");
    }
    if (NULL != gTivxVpacNfGenericInstObj.lock)
    {
        (void)tivxMutexDelete(&gTivxVpacNfGenericInstObj.lock);
    }
}
/* LDRA_JUSTIFY_END */

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacNfGenericProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                    status = (vx_status)VX_SUCCESS;
    int32_t                      fvid2_status = FVID2_SOK;
    uint32_t                     size;
    int32_t                      k;
    int32_t                      m;
    int16_t                      temp_lut[25];
    int16_t                     *pConv;
    void                        *conv_target_ptr;
    tivxVpacNfGenericObj        *nf_generic_obj = NULL;
    tivx_obj_desc_convolution_t *conv;
    tivx_obj_desc_image_t       *src;
    tivx_obj_desc_image_t       *dst;
    Fvid2_FrameList             *inFrmList;
    Fvid2_FrameList             *outFrmList;
    uint64_t                    cur_time;
    tivx_obj_desc_t             *out_base_desc = NULL;
    vx_status                   validate_reg_status = (vx_status)VX_SUCCESS;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS);

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> Rationale: The test framework and test apps cannot reach this portion. Passing only valid types to tivxAddKernelVpacNfGenericValidate function. 
                        Error cases pre-validating at tiovx.
    Effect on this unit: None;Unused feature, cannot be enabled, control cannot reach to this condition.
    <justification end> */
    if ((vx_status)VX_SUCCESS != status)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
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
        Effect on this unit: If the control reaches here, nf_generic_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            nf_generic_obj = temp_kernel_context;
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
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if ((uint32_t)sizeof(tivxVpacNfGenericObj) != size)
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        inFrmList = &nf_generic_obj->inFrmList;
        outFrmList = &nf_generic_obj->outFrmList;
        src = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_INPUT_IDX];
        conv = (tivx_obj_desc_convolution_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_CONV_IDX];
        dst = (tivx_obj_desc_image_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_OUTPUT_IDX];

        out_base_desc = (tivx_obj_desc_t *)dst;
        nf_generic_obj->timestamp = out_base_desc->timestamp;

        conv_target_ptr = tivxMemShared2TargetPtr(&conv->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(conv_target_ptr, conv->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        pConv = conv_target_ptr;

        /* Centers the given matrix in the 5x5 */
        {
            for (m = -2; m < 3; m++)
            {
                for (k = -2; k < 3; k++)
                {
                    if ( (m < (-((int32_t)conv->rows/2))) || (m > ((int32_t)conv->rows/2)))
                    {
                        /* Force to zero */
                        temp_lut[((m + 2) * 5) + (k + 2)] = 0;
                    }
                    else if ( (k < (-((int32_t)conv->columns/2))) || (k > ((int32_t)conv->columns/2)))
                    {
                        /* Force to zero */
                        temp_lut[((m + 2) * 5) + (k + 2)] = 0;
                    }
                    else
                    {
                        int32_t conv_cols = (int32_t)conv->columns;
                        temp_lut[((m + 2) * 5) + (k + 2)] = pConv[((m + ((int32_t)conv->rows/2)) * conv_cols) +
                                                                       (k + (conv_cols/2))];
                    }
                }
            }
        }

        /* Since it is convolution, flip the matrix (decide later to do this or not) */
        for (k = 0; k < 12; k++) {
            nf_generic_obj->wgtTbl.genFilterCoeffs[k] = (int32_t) temp_lut[25 - 1 - k];
        }
        for (k = 13; k < 25; k++) {
            nf_generic_obj->wgtTbl.genFilterCoeffs[k - 1] = (int32_t) temp_lut[25 - 1 - k];
        }

        nf_generic_obj->nf_cfg.nfCfg.centralPixelWeight = temp_lut[12];
        nf_generic_obj->wgtTbl.filterMode      = NF_FILTER_MODE_GENERIC_2D_FILTER;

        /* Update NF params */
        fvid2_status = Fvid2_control(nf_generic_obj->handle,
            IOCTL_VHWA_M2M_NF_SET_PARAMS, &nf_generic_obj->nf_cfg, NULL);
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
        else
        /* LDRA_JUSTIFY_END */
        {
            status = (vx_status)VX_SUCCESS;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Set NF coeff */
        fvid2_status = Fvid2_control(nf_generic_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
            &nf_generic_obj->wgtTbl, NULL);
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

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Initialize NF Input Frame List */
        inFrmList->frames[0U] =
            &nf_generic_obj->inFrm;
        inFrmList->numFrames = 1U;

        nf_generic_obj->inFrm.addr[0U] = tivxMemShared2PhysPtr(
            src->mem_ptr[0].shared_ptr,
            (int32_t)src->mem_ptr[0].mem_heap_region);

        /* Initialize NF Output Frame List */
        outFrmList->frames[0U] = &nf_generic_obj->outFrm;
        outFrmList->numFrames = 1U;

        nf_generic_obj->outFrm.addr[0U] = tivxMemShared2PhysPtr(
            dst->mem_ptr[0].shared_ptr,
            (int32_t)dst->mem_ptr[0].mem_heap_region);

        /* Clearing error status from previous frame
        *  User should consume the error status before
        *  submitting next frame for processing
        */
        nf_generic_obj->wdTimerErrStatus = 0u;
        nf_generic_obj->err_stat = 0u;
            
        cur_time = tivxPlatformGetTimeInUsecs();    

        /* Submit NF Request*/
        fvid2_status = Fvid2_processRequest(nf_generic_obj->handle, inFrmList,
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Wait for Frame Completion */
        (void)tivxEventWait(nf_generic_obj->waitForProcessCmpl, VX_TIMEOUT_WAIT_FOREVER);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start> Rationale:
        This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(0u == nf_generic_obj->wdTimerErrStatus)
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
            if(0u != nf_generic_obj->err_stat)
            {
                VX_PRINT(VX_ZONE_INFO, "Error interrupt: NF non-stalling error interrupt triggered \n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

            fvid2_status = Fvid2_getProcessedRequest(nf_generic_obj->handle,
                inFrmList, outFrmList, 0U);
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
            int32_t ctrl_status;
            ctrl_status = Fvid2_control(nf_generic_obj->handle, VHWA_M2M_IOCTL_NF_VALIDATE_REG, NULL, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Register validation failure requires hardware corruption or fault injection not available in test environment.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != ctrl_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Register validation failed (Fvid2_control returned %d)\n", ctrl_status);
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        appPerfStatsHwaUpdateLoad(nf_generic_obj->hwa_perf_id,
            (uint32_t)cur_time,
            dst->imagepatch_addr[0U].dim_x*dst->imagepatch_addr[0U].dim_y /* pixels processed */
            );
    }

    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        tivxCheckStatus(&status, tivxMemBufferUnmap(conv_target_ptr, conv->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    if(((vx_status)VX_SUCCESS != status) || ((vx_status)VX_SUCCESS != validate_reg_status))
    {
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfGenericCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    int32_t                           k;
    int32_t                           m;
    int16_t                           temp_lut[25];
    int16_t                          *pConv;
    tivx_obj_desc_convolution_t      *conv;
    void                             *conv_target_ptr;
    tivxVpacNfGenericObj             *nf_generic_obj = NULL;
    Vhwa_M2mNfConfig                 *nf_cfg = NULL;
    tivx_vpac_nf_common_params_t     *params = NULL;
    tivx_obj_desc_user_data_object_t *params_array = NULL;
    tivx_obj_desc_image_t            *src;
    tivx_obj_desc_image_t            *dst;
    void                             *params_array_target_ptr = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS);

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
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
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
        nf_generic_obj = tivxVpacNfGenericAllocObject(&gTivxVpacNfGenericInstObj);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != nf_generic_obj)
        /* LDRA_JUSTIFY_END */
        {
            params_array = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_CONFIGURATION_IDX];
            src = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_INPUT_IDX];
            dst = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_OUTPUT_IDX];
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Nf Generic Object\n");
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
        Vhwa_M2mNfCreatePrmsInit(&nf_generic_obj->createPrms);

        nf_cfg = &nf_generic_obj->nf_cfg;

        params_array_target_ptr = tivxMemShared2TargetPtr(&params_array->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(params_array_target_ptr, params_array->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        params = (tivx_vpac_nf_common_params_t *)params_array_target_ptr;
        nf_generic_obj->createPrms.enablePsa = params->enable_psa;
        /* Initialize NF Config with defaults */
        Nf_ConfigInit(&nf_cfg->nfCfg);


        status = tivxEventCreate(&nf_generic_obj->waitForProcessCmpl);

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
            nf_generic_obj->cbPrms.cbFxn   = tivxVpacNfGenericFrameComplCb;
            nf_generic_obj->cbPrms.appData = nf_generic_obj;

            nf_generic_obj->handle = Fvid2_create(FVID2_VHWA_M2M_NF_DRV_ID,
                nf_generic_obj->nf_drv_inst_id, (void *)&nf_generic_obj->createPrms,
                NULL, &nf_generic_obj->cbPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == nf_generic_obj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to Alloc Nf Generic Object\n");
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nf_generic_obj->errEvtPrms.errEvents = (VHWA_NF_RD_ERR | VHWA_NF_WR_ERR);
        nf_generic_obj->errEvtPrms.cbFxn     = tivxVpacNfGenericErrorCb;
        nf_generic_obj->errEvtPrms.appData   = nf_generic_obj;

        fvid2_status = Fvid2_control(nf_generic_obj->handle,
            IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB, &nf_generic_obj->errEvtPrms, NULL);
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
        nf_generic_obj->wdTimerErrEvtPrms.wdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        /* Disable Watchdog Timer Error Events by default */
        nf_generic_obj->wdTimerErrEvtPrms.wdTimerErrEvents = 0u;
        nf_generic_obj->wdTimerErrEvtPrms.cbFxn     = tivxVpacNfGenericWdTimerErrorCb;
        nf_generic_obj->wdTimerErrEvtPrms.appData   = nf_generic_obj;
        fvid2_status = Fvid2_control(nf_generic_obj->handle,
            VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB, &nf_generic_obj->wdTimerErrEvtPrms, NULL);
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
        conv = (tivx_obj_desc_convolution_t *)obj_desc[TIVX_KERNEL_VPAC_NF_GENERIC_CONV_IDX];
        conv_target_ptr = tivxMemShared2TargetPtr(&conv->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(conv_target_ptr, conv->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        pConv = conv_target_ptr;

        /* Centers the given matrix in the 5x5 */
        {
            for (m = -2; m < 3; m++)
            {
                for (k = -2; k < 3; k++)
                {
                    if ( (m < (-((int32_t)conv->rows/2))) || (m > ((int32_t)conv->rows/2)))
                    {
                        /* Force to zero */
                        temp_lut[((m + 2) * 5) + (k + 2)] = 0;
                    }
                    else if ( (k < (-((int32_t)conv->columns/2))) || (k > ((int32_t)conv->columns/2)))
                    {
                        /* Force to zero */
                        temp_lut[((m + 2) * 5) + (k + 2)] = 0;
                    }
                    else
                    {
                        int32_t conv_cols = (int32_t)conv->columns;
                        temp_lut[((m + 2) * 5) + (k + 2)] = pConv[((m + ((int32_t)conv->rows/2)) * conv_cols) +
                                                                       (k + (conv_cols/2))];
                    }
                }
            }
        }

        /* Since it is convolution, flip the matrix (decide later to do this or not) */
        for (k = 0; k < 12; k++) {
            nf_generic_obj->wgtTbl.genFilterCoeffs[k] = (int32_t) temp_lut[25 - 1 - k];
        }
        for (k = 13; k < 25; k++) {
            nf_generic_obj->wgtTbl.genFilterCoeffs[k - 1] = (int32_t) temp_lut[25 - 1 - k];
        }

        /* Set NF Config parameters - centralPixelWeight set in tivxVpacNfGenericProcess */
        nf_cfg->nfCfg.filterMode = NF_FILTER_MODE_GENERIC_2D_FILTER;
        nf_cfg->nfCfg.tableMode = 0u;
        nf_cfg->nfCfg.skipMode = params->output_pixel_skip;
        nf_cfg->nfCfg.interleaveMode = params->input_interleaved;
        nf_cfg->nfCfg.outputShift = params->output_downshift;
        nf_cfg->nfCfg.outputOffset = params->output_offset;
        nf_cfg->nfCfg.numSubTables = 0u;
        nf_cfg->nfCfg.subTableIdx = 0u;
        nf_cfg->nfCfg.centralPixelWeight = temp_lut[12];
        nf_generic_obj->wgtTbl.filterMode      = NF_FILTER_MODE_GENERIC_2D_FILTER;

        tivxVpacNfSetFmt(&nf_cfg->inFmt, src);
        tivxVpacNfSetFmt(&nf_cfg->outFmt, dst);

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           VPAC NF parameters needs to be reconfigured */
        memcpy(&nf_generic_obj->nfGenericParams, params, sizeof(tivx_vpac_nf_common_params_t));

        fvid2_status = Fvid2_control(nf_generic_obj->handle,
            IOCTL_VHWA_M2M_NF_SET_PARAMS, &nf_generic_obj->nf_cfg, NULL);
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set parameters request failed\n");
            status = (vx_status)VX_FAILURE;
        }

        /* Set NF coeff */
        fvid2_status = Fvid2_control(nf_generic_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
            &nf_generic_obj->wgtTbl, NULL);
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
        tivxCheckStatus(&status, tivxMemBufferUnmap(conv_target_ptr, conv->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        status = tivxSetTargetKernelInstanceContext(kernel, nf_generic_obj,
            sizeof(tivxVpacNfGenericObj));
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
        if (NULL != nf_generic_obj)
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
            if (NULL != nf_generic_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(nf_generic_obj->handle, NULL);
                nf_generic_obj->handle = NULL;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != nf_generic_obj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&nf_generic_obj->waitForProcessCmpl);
            }

            tivxVpacNfGenericFreeObject(&gTivxVpacNfGenericInstObj, nf_generic_obj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxVpacNfGenericDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 size;
    tivxVpacNfGenericObj    *nf_generic_obj = NULL;

    status = tivxCheckNullParams(obj_desc, num_params,
                TIVX_KERNEL_VPAC_NF_GENERIC_MAX_PARAMS);
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
        Effect on this unit: If the control reaches here, nf_generic_obj would not be assigned and subsequent NULL checks would catch the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            nf_generic_obj = temp_kernel_context;
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) && (NULL != nf_generic_obj) &&
            ((uint32_t)sizeof(tivxVpacNfGenericObj) == size))
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
            if (NULL != nf_generic_obj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(nf_generic_obj->handle, NULL);
                nf_generic_obj->handle = NULL;
            }

             /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != nf_generic_obj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&nf_generic_obj->waitForProcessCmpl);
            }
            
            /* Free the readback and golden register buffers */
            tivxVpacNfGenericFreeReadbackBuffers(nf_generic_obj);

            tivxVpacNfGenericFreeObject(&gTivxVpacNfGenericInstObj, nf_generic_obj);
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

static vx_status VX_CALLBACK tivxVpacNfGenericControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxVpacNfGenericObj             *nf_generic_obj = NULL;

    void *temp_kernel_context = NULL;
    status = tivxGetTargetKernelInstanceContext(kernel, &temp_kernel_context, &size);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The kernel parameter is expected to be pre-validated by the OpenVX framework before kernel callback invocation.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, nf_generic_obj would not be assigned and subsequent NULL checks would catch the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        nf_generic_obj = temp_kernel_context;
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

    /* LDRA_JUSTIFY_END */ 
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((NULL == nf_generic_obj) ||
        ((uint32_t)sizeof(tivxVpacNfGenericObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Wrong Size for Nf Generic Obj\n");
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
    Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        switch (node_cmd_id)
        {
            case TIVX_VPAC_NF_CMD_SET_HTS_LIMIT:
            {
                status = tivxVpacNfGenericSetHtsLimitCmd(nf_generic_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_SET_COEFF:
            {
                status = tivxVpacNfGenericSetCoeff(nf_generic_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_GET_ERR_STATUS:
            {
                status = tivxVpacNfGenericGetErrStatusCmd(nf_generic_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_GET_PSA_STATUS:
            {
                status = tivxVpacNfGenericGetPsaStatusCmd(nf_generic_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxVpacNfGenericEnableErrorEventsCmd(nf_generic_obj,
                  (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_NF_CMD_ENABLE_VPAC_SAFETY_MECHANISM:
            {
                status = tivxEnableVpacNfGenericSafetyMechanisms(nf_generic_obj,
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

static tivxVpacNfGenericObj *tivxVpacNfGenericAllocObject(
       tivxVpacNfGenericInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacNfGenericObj *nf_generic_obj = NULL;
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale:The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 4 NF handles
        Effect on this unit: It will utilise all the 4 NF handles which are validated in the NF driver.
        <justification end> */ 
        for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this scenario.
            Effect on this unit: If the control reaches here, the code is expected to allocate another handle for NF.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if (0U == instObj->nfGenericObj[cnt].isAlloc)
            /* LDRA_JUSTIFY_END */
            {
                nf_generic_obj = &instObj->nfGenericObj[cnt];
                memset(nf_generic_obj, 0x0, sizeof(tivxVpacNfGenericObj));
                instObj->nfGenericObj[cnt].isAlloc = 1U;

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
                    instObj->nfGenericObj[cnt].nf_drv_inst_id = VHWA_M2M_NF_DRV_INST_ID;
                    instObj->nfGenericObj[cnt].hwa_perf_id    = APP_PERF_HWA_VPAC1_NF;
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */  
                #if (VPAC_COUNT > 1)
                else if (self_cpu == (vx_enum)TIVX_CPU_ID_VPAC2)
                /* LDRA_JUSTIFY_END */
                {
                    instObj->nfGenericObj[cnt].nf_drv_inst_id = VHWA_M2M_VPAC_1_NF_DRV_INST_ID_0;
                    instObj->nfGenericObj[cnt].hwa_perf_id    = APP_PERF_HWA_VPAC2_NF;
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
                    nf_generic_obj = NULL;
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to acquire mutex lock\n");
    }
    /* LDRA_JUSTIFY_END */
    return (nf_generic_obj);
}

static void tivxVpacNfGenericFreeObject(tivxVpacNfGenericInstObj *instObj,
    tivxVpacNfGenericObj *nf_generic_obj)
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale:The test framework and test apps cannot reach this portion. The current kernel test app cannot support all 4 NF handles
        Effect on this unit: It will free all the 4 NF handles which are validated in the NF driver.
        <justification end> */ 
        for (cnt = 0U; cnt < VHWA_M2M_NF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The test framework and test apps cannot reach this portion.
            The test framework does not support the configuration required to trigger this scenario.
            Effect on this unit: If the control reaches here, the code is expected to free the allocated handle for NF.
            However, due to the stated rationale, this is not tested.
            <justification end> */ 
            if (nf_generic_obj == &instObj->nfGenericObj[cnt])
            {
                nf_generic_obj->isAlloc = 0U;
                break;
            }
            /* LDRA_JUSTIFY_END */
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
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
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
        <metric start> statement branch <metric end>
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
static vx_status tivxVpacNfGenericMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
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
        <metric start> branch <metric end>
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
        if (0u != (wd_timer_err & VHWA_NF_WDTIMER_ERR))
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
static vx_status tivxVpacNfGenericMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
                                                     uint32_t *vhwa_err_events,
                                                     uint32_t *wd_timer_err)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t enableErrorEventCheckMask = (uint32_t)TIVX_VPAC_NF_ALL_ERR_EVENTS;

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

static vx_status tivxVpacNfGenericSetHtsLimitCmd(
    tivxVpacNfGenericObj *nf_generic_obj,
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

        if ((uint32_t)sizeof(tivx_vpac_nf_hts_bw_limit_params_t) ==
                usr_data_obj->mem_size)
        {
            app_hts_prms = (tivx_vpac_nf_hts_bw_limit_params_t *)target_ptr;

            hts_limit.enableBwLimit = app_hts_prms->enable_hts_bw_limit;
            hts_limit.cycleCnt = app_hts_prms->cycle_cnt;
            hts_limit.tokenCnt = app_hts_prms->token_cnt;

            fvid2_status = Fvid2_control(nf_generic_obj->handle,
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
    }

    return (status);
}

static vx_status tivxVpacNfGenericSetCoeff(tivxVpacNfGenericObj *nf_generic_obj,
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

        if ((uint32_t)sizeof(Nf_WgtTableConfig) ==
                usr_data_obj->mem_size)
        {
            wgtTbl = (Nf_WgtTableConfig *)target_ptr;
            fvid2_status = Fvid2_control(nf_generic_obj->handle, IOCTL_VHWA_M2M_NF_SET_FILTER_COEFF,
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

static vx_status tivxVpacNfGenericGetErrStatusCmd(const tivxVpacNfGenericObj *nf_generic_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat = 0u;

    if (NULL != scalar_obj_desc)
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxVpacNfGenericMapVhwaToTivxErrEvents(nf_generic_obj->err_stat, nf_generic_obj->wdTimerErrStatus, &combined_err_stat);
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
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
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

static vx_status tivxVpacNfGenericGetPsaStatusCmd(const tivxVpacNfGenericObj *nf_generic_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    Vhwa_M2mNfPsaSign                 psa;
    int32_t                           fvid2_status = FVID2_SOK;
    void                              *target_ptr;
    tivx_vpac_nf_psa_timestamp_data_t *psa_status_ptr;
    
    if (NULL != usr_data_obj)
    {
        if ((uint32_t)sizeof(tivx_vpac_nf_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            fvid2_status = Fvid2_control(nf_generic_obj->handle,
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
                    "tivxVpacNfGenericGetPsa: Fvid2_control Failed\n");
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
            psa_status_ptr->timestamp = nf_generic_obj->timestamp;

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

static vx_status tivxVpacNfGenericEnableErrorEventsCmd(tivxVpacNfGenericObj *nf_generic_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t intr_en = 0u;
    uint32_t wd_timer_intr_en = 0u;
    void *target_ptr;
    tivx_vpac_nf_common_params_t *params_ptr = NULL;
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
        if(sizeof(tivx_vpac_nf_common_params_t) != usr_data_obj->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxVpacNfGenericEnableErrorEventsCmd: Incorrect Data Object Size \n");
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
                    "tivxVpacNfGenericEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_vpac_nf_common_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->enable_error_events;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxVpacNfGenericMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

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
                        "tivxVpacNfGenericEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    nf_generic_obj->errEvtPrms.errEvents = intr_en;
                    fvid2_status = Fvid2_control(nf_generic_obj->handle,
                        IOCTL_VHWA_M2M_NF_REGISTER_ERR_CB, &nf_generic_obj->errEvtPrms, NULL);

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
                        nf_generic_obj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->wdtimer_timeout_cycles;
                        nf_generic_obj->wdTimerErrEvtPrms.wdTimerErrEvents = wd_timer_intr_en;
                        fvid2_status = Fvid2_control(nf_generic_obj->handle,
                            VHWA_M2M_IOCTL_NF_REGISTER_WDTIMER_ERR_CB, &nf_generic_obj->wdTimerErrEvtPrms, NULL);
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
                            "tivxVpacNfGenericEnableErrorEventsCmd: Fvid2_control Failed \n");
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
            "tivxVpacNfGenericEnableErrorEventsCmd: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}
static vx_status tivxEnableVpacNfGenericSafetyMechanisms(
    tivxVpacNfGenericObj *nf_generic_obj,
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
    if (NULL == nf_generic_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "nf_generic_obj is NULL\n");
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
                if ((uint32_t)sizeof(tivx_vpac_nf_safety_mechanism_params_t) == usr_data_obj->mem_size)
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
                    fvid2_status = Fvid2_control(nf_generic_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: If the control reaches here, our code base is expected to configure safety mechanisms.
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
                    fvid2_status = Fvid2_control(nf_generic_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_STATUS_REG_VALIDATE, &enable, NULL);
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
                                nf_generic_obj->handle,
                                VHWA_M2M_IOCTL_NF_GET_READBACK_SIZE,
                                &readback_size,
                                NULL
                            );

                            nf_generic_obj->config_reg_mem_size = readback_size;

                            /* LDRA_JUSTIFY_START
                            <metric start> branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This failure case is out of scope for the imaging test framework.
                            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if ((FVID2_SOK == fvid2_status) && (readback_size > 0U))
                            /* LDRA_JUSTIFY_END */
                            {
                                VX_PRINT(VX_ZONE_INFO, "NF readback size allocation successful: %u bytes\n", readback_size);
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
                        if (((vx_status)VX_SUCCESS == status) && (readback_size > 0U))
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
                            if ((nf_generic_obj->readback_mem_ptr_phys != 0U) && (nf_generic_obj->golden_reg_mem_ptr_phys != 0U))
                            {
                                VX_PRINT(VX_ZONE_INFO, "Readback or golden register buffer already allocated, skipping allocation\n");
                            }
                            /* LDRA_JUSTIFY_END */
                            else
                            {
                                status = tivxVpacNfGenericAllocReadbackBuffers(nf_generic_obj);
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
                            fvid2_status = Fvid2_control(nf_generic_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                                tivxVpacNfGenericFreeReadbackBuffers(nf_generic_obj);
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
                        fvid2_status = Fvid2_control(nf_generic_obj->handle, VHWA_M2M_IOCTL_NF_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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

static vx_status tivxVpacNfGenericAllocReadbackBuffers(tivxVpacNfGenericObj *nf_generic_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == nf_generic_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxVpacNfGenericAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (NULL == nf_generic_obj->handle)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxVpacNfGenericAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Valid nf_generic_obj and handle */
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
        uint32_t readback_size = nf_generic_obj->config_reg_mem_size;

        vx_status alloc_status = tivxMemBufferAlloc(&nf_generic_obj->readback_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);
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
            nf_generic_obj->readback_mem_ptr_phys = tivxMemShared2PhysPtr(nf_generic_obj->readback_mem_ptr_virt.shared_ptr, (int32_t)nf_generic_obj->readback_mem_ptr_virt.mem_heap_region);
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
            Fvid2Utils_memset((void *)(uintptr_t)nf_generic_obj->readback_mem_ptr_virt.shared_ptr, 0U, readback_size);
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
            alloc_status = tivxMemBufferAlloc(&nf_generic_obj->golden_reg_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);

            nf_generic_obj->golden_reg_mem_ptr_phys = tivxMemShared2PhysPtr(nf_generic_obj->golden_reg_mem_ptr_virt.shared_ptr, (int32_t)nf_generic_obj->golden_reg_mem_ptr_virt.mem_heap_region);
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
        {
            Fvid2Utils_memset((void *)(uintptr_t)nf_generic_obj->golden_reg_mem_ptr_virt.shared_ptr, 0U, readback_size);
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
        if ((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_M2mNfConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ref_ptr   = (void *)(uintptr_t)nf_generic_obj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)nf_generic_obj->readback_mem_ptr_virt.shared_ptr;

            reg_ptrs.cbFxn = tivxVpacNfGenericConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)nf_generic_obj;

            int32_t fvid2_status = Fvid2_control(
                nf_generic_obj->handle,
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set NF register readback/golden pointers\n");
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
                tivxVpacNfGenericFreeReadbackBuffers(nf_generic_obj);
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
            tivxVpacNfGenericFreeReadbackBuffers(nf_generic_obj);
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static void tivxVpacNfGenericFreeReadbackBuffers(tivxVpacNfGenericObj *nf_generic_obj)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != nf_generic_obj)
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;
        uint32_t readback_size = nf_generic_obj->config_reg_mem_size;

        if (nf_generic_obj->readback_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&nf_generic_obj->readback_mem_ptr_virt, readback_size);
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
            nf_generic_obj->readback_mem_ptr_phys = 0u;
        }

        if (nf_generic_obj->golden_reg_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&nf_generic_obj->golden_reg_mem_ptr_virt, readback_size);
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
            nf_generic_obj->golden_reg_mem_ptr_phys = 0u;
        }

        nf_generic_obj->config_reg_mem_size = 0u;
    }
}

/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacNfGenericFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacNfGenericObj *nf_generic_obj = (tivxVpacNfGenericObj *)appData;
    /* LDRA_JUSTIFY_START
    <metric start> branch statement <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, the code base is NOT expected to accumulate and return an error.
    This behaviour is part of the application design. An error accumulation statement can be added in a future release.
    <justification end> */
    if (NULL != nf_generic_obj)
    {
        (void)tivxEventPost(nf_generic_obj->waitForProcessCmpl);
    }
    /* LDRA_JUSTIFY_END */

    return FVID2_SOK;
}

/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start>
Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
However, due to the stated rationale, this is not tested.
<justification end> */
void tivxVpacNfGenericErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacNfGenericObj *nf_generic_obj = (tivxVpacNfGenericObj *)appData;

    if (NULL != nf_generic_obj)
    {
        nf_generic_obj->err_stat = errEvents;
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
static void tivxVpacNfGenericWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxVpacNfGenericObj *nf_generic_obj = (tivxVpacNfGenericObj *)appData;
    if (NULL != nf_generic_obj)
    {
        nf_generic_obj->wdTimerErrStatus = (nf_generic_obj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents);

        if(0u != nf_generic_obj->wdTimerErrStatus)
        {
            (void)tivxEventPost(nf_generic_obj->waitForProcessCmpl);
        }
    }
}

/* LDRA_JUSTIFY_END */
/* Callback for config register and golden register memory comparison for NF */
int32_t tivxVpacNfGenericConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mNfConfigRegMemParams *nf_config_reg_prms = (Vhwa_M2mNfConfigRegMemParams *)configRegPrms;
    tivxVpacNfGenericObj *nf_generic_obj = NULL;

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
    if ((NULL != nf_config_reg_prms) && (NULL != handle))
    /* LDRA_JUSTIFY_END */
    {
        nf_generic_obj = (tivxVpacNfGenericObj *)nf_config_reg_prms->appData;
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
    if ((NULL != nf_config_reg_prms) && (NULL != handle) && (NULL != nf_generic_obj))
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
        if ((NULL != nf_config_reg_prms->golden_ref_ptr) &&
            (NULL != nf_config_reg_prms->readback_ptr) &&
            (nf_config_reg_prms->readback_mem_size > 0U))
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
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
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
BUILD_ASSERT((sizeof(Vhwa_M2mNfPsaSign) == sizeof(((tivx_vpac_nf_psa_timestamp_data_t *)NULL)->psa_values))? 1U : 0U);
