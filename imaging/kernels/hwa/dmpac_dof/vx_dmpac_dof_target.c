/*
 *
 * Copyright (c) 2017-2019 Texas Instruments Incorporated
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
#include "TI/hwa_dmpac_dof.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_dmpac_dof.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"
#include "tivx_hwa_dmpac_dof_priv.h"
#include "vhwa/include/vhwa_m2mDof.h"

#include "utils/perf_stats/include/app_perf_stats.h"
#if defined(LDRA_COVERAGE)
#include <kernels/coverage_files/include/ldra_remote_core_coverage_main.h>
#endif

#define TIVX_DMPAC_DOF_FLOW_VEC_QSIZE   (TIVX_DMPAC_DOF_MAX_FLOW_VECTOR_DELAY+1U)

typedef struct
{
    uint32_t                            isAlloc;
    uint32_t                            isFirstFrame;
    uint32_t                            total_pyr_lvl;

    uint64_t                            inter_buff1;
    uint64_t                            inter_buff2;
    uint32_t                            inter_buff_size;

    tivx_dmpac_dof_params_t             dofAppPrms;
    tivx_dmpac_dof_sof_params_t         sofAppPrms;

    Vhwa_M2mDofCreateArgs               createArgs;
    Vhwa_M2mDofPrms                     dofPrms;
    Dof_ConfScoreParam                  csPrms;
    Fvid2_Handle                        handle;
    /* Stores the status of error interrupt */
    uint32_t                            err_stat;
    tivx_event                          waitForProcessCmpl;
    /* Structure for error event parameters
     * Used to register callback for the given set of events.*/
    Dof_ErrEventParams                  errEvtPrms;
    Vhwa_HtsLimiter                     htsBwLimitCfg;

    Fvid2_FrameList                     inFrmList;
    Fvid2_FrameList                     outFrmList;
    Fvid2_Frame                         inFrm[VHWA_M2M_DOF_MAX_IN_BUFFER];
    Fvid2_Frame                         outFrm;
    Fvid2_CbParams                      cbPrms;

    /* Number of internal delay slots to use for applying previous flow vector
     * output to temporal predictor. After the create phase, this field will
     * either be 0 or a known validated non-zero value and will be used in
     * conjunction with 'flow_vector_in_desc' parameter, to determine whether
     * to use internal history buffer 'outFlowVecHistory' or not.
     *
     * A history buffer is considered for use if
     *  |outFlowVecWrIdx - outFlowVecRdIdx] >= flowVecIntDelay
     *
     * In practice |outFlowVecWrIdx - outFlowVecRdIdx] == flowVecIntDelay is
     * maintained under steady state conditions.
     */
    uint32_t                            flowVecIntDelay;

    /* Next write location in the history buffer. This is initialized to 0 and
     * should never take over 'outFlowVecRdIdx'. The check for takeover is
     * not checked explicitly.
     */
    int32_t                             outFlowVecWrIdx;

    /* Next read location in the history buffer. This is initialized to 0 and
     * should never takeover 'outFlowVecWrIdx'. The check for takeover is
     * not checked explicitly.
     */
    int32_t                             outFlowVecRdIdx;

    /* Space to hold the past flow vector information. This buffer is used only
     * if 'flowVecIntDelay' is non-zero and temporal prediction is enabled.
     */
    uint64_t                           outFlowVecHistory[TIVX_DMPAC_DOF_FLOW_VEC_QSIZE];

    /* Structure for watchdog timer error event parameters
     * Used to register callback for the given set of events.*/
    Dof_WdTimerErrEventParams           wdTimerErrEvtPrms;

    /* Stores the status of watchdog timer error interrupt */
    uint32_t                            wdTimerErrStatus;
    /* Timestamp for the most recent frame processed. */
    uint64_t                            timestamp;

#if !defined(VPAC3L)
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
#endif
} tivxDmpacDofObj;


typedef struct
{
    tivx_mutex      lock;
    tivxDmpacDofObj  dofObj[VHWA_M2M_DOF_MAX_HANDLES];
} tivxDmpacDofInstObj;


static vx_status VX_CALLBACK tivxDmpacDofProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxDmpacDofControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

static tivxDmpacDofObj *tivxDmpacDofAllocObject(tivxDmpacDofInstObj *instObj);
static void tivxDmpacDofFreeObject(tivxDmpacDofInstObj *instObj,
                                                tivxDmpacDofObj *dofObj);

static void tivxDmpacDofSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static void tivxDmpacDofSetCfgPrms(Vhwa_M2mDofPrms *dofPrms,
    const tivx_dmpac_dof_params_t *dofAppPrms, const tivx_dmpac_dof_sof_params_t *sofAppPrms,
    tivx_obj_desc_t *obj_desc[]);
static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacDofUpdateCfgPrms(tivxDmpacDofObj *dof_obj,
                                    const tivx_dmpac_dof_params_t *dofAppPrms, uint32_t output_format);
static vx_status tivxDmpacDofGetErrStatusCmd(const tivxDmpacDofObj *dof_obj,
                        tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxDmpacDofSetHtsBwLimit(tivxDmpacDofObj *dof_obj,
                                const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacDofUpdateSofPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_dmpac_dof_sof_params_t *sofAppPrms);
static vx_status tivxDmpacDofSetSofParams(tivxDmpacDofObj *dofObj,
    const tivx_obj_desc_user_data_object_t *dof_app_prms_sof_user_desc);
static vx_status tivxDmpacDofSetDofParams(tivxDmpacDofObj *dofObj,
    const tivx_obj_desc_user_data_object_t *dof_app_prms_user_desc);
static vx_status tivxDmpacDofGetPsaStatusCmd(const tivxDmpacDofObj *dof_obj,
                        tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacDofEnableErrorEventsCmd(tivxDmpacDofObj *dof_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxDmpacDofMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxDmpacDofMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);

int32_t tivxDmpacDofFrameComplCb(Fvid2_Handle handle, void *appData);
void tivxDmpacDofErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxDmpacDofWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);
#if !defined(SOC_J722S)
int32_t tivxDmpacDofConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms);

static vx_status tivxEnableDmpacDofSafetyMechanisms(
    tivxDmpacDofObj *dofObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);

static vx_status tivxDmpacDofAllocReadbackBuffers(tivxDmpacDofObj *dofObj);

static void tivxDmpacDofFreeReadbackBuffers(tivxDmpacDofObj *dofObj);
#endif
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
static tivx_target_kernel vx_dmpac_dof_target_kernel = NULL;

tivxDmpacDofInstObj gTivxDmpacDofInstObj;

/* Default Confidence score configuration, matching with cmodel */
static const Dof_ConfScoreParam gConfScPrms = {
    224U,
    {
        {{7U, 6U, 0U},{0x0000000bU, 0x00000010U, 0x00000000U}, {0x00000395U, 0xfffffc6bU, 0xfffffc6bU, 0xfffffc6bU}},
        {{7U, 5U, 7U},{0x0000001bU, 0x00000052U, 0x00000001U}, {0xffffec00U, 0x00001400U, 0x00001400U, 0xffffec00U}},
        {{7U, 5U, 0U},{0x0000001bU, 0x00000052U, 0x00000000U}, {0x0000127eU, 0xffffed82U, 0x0000127eU, 0x0000127eU}},
        {{0U, 5U, 5U},{0x00000254U, 0x000000afU, 0x000001efU}, {0xffffff23U, 0x000000ddU, 0xffffff23U, 0x000000ddU}},
        {{6U, 5U, 7U},{0x00000003U, 0x00000214U, 0x0000000bU}, {0x000000abU, 0xffffff55U, 0xffffff55U, 0x000000abU}},
        {{6U, 5U, 5U},{0x0000000aU, 0x0000022cU, 0x00000052U}, {0x000000a6U, 0xffffff5aU, 0x000000a6U, 0xffffff5aU}},
        {{6U, 7U, 6U},{0x00000010U, 0x0000000bU, 0x00000023U}, {0xffffff3dU, 0x000000c3U, 0x000000c3U, 0xffffff3dU}},
        {{7U, 5U, 7U},{0x00000006U, 0x0000023cU, 0x0000001bU}, {0x00000089U, 0xffffff77U, 0xffffff77U, 0x00000089U}},
        {{7U, 0U, 0U},{0x00000011U, 0x000002b4U, 0x0000053fU}, {0x00000072U, 0xffffff8eU, 0xffffff8eU, 0x00000072U}},
        {{6U, 7U, 0U},{0x00000010U, 0x0000000bU, 0x000002e5U}, {0xffffff7fU, 0x00000081U, 0x00000081U, 0xffffff7fU}},
        {{6U, 4U, 5U},{0x00000003U, 0x00001141U, 0x0000018fU}, {0xffffff85U, 0x0000007bU, 0xffffff85U, 0x0000007bU}},
        {{0U, 5U, 5U},{0x000001f4U, 0x000001e3U, 0x000001bbU}, {0x00000082U, 0xffffff7eU, 0xffffff7eU, 0x00000082U}},
        {{6U, 4U, 5U},{0x00000003U, 0x0000129cU, 0x00000123U}, {0xffffffb8U, 0x00000048U, 0xffffffb8U, 0x00000048U}},
        {{5U, 5U, 6U},{0x000000afU, 0x00000012U, 0x0000006aU}, {0xffffffb5U, 0x0000004bU, 0xffffffb5U, 0x0000004bU}},
        {{4U, 1U, 7U},{0x00002e5aU, 0x00000001U, 0x000000c6U}, {0xffffffb3U, 0x0000004dU, 0xffffffb3U, 0x0000004dU}},
        {{5U, 5U, 5U},{0x000000a3U, 0x000000a3U, 0x000002deU}, {0xffffc000U, 0x00000000U, 0x00000000U, 0xffffc000U}},
    }
};


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelDmpacDof(void)
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
    if (self_cpu == (vx_enum)TIVX_CPU_ID_DMPAC)
    /* LDRA_JUSTIFY_END */ 
    {
        strncpy(target_name, TIVX_TARGET_DMPAC_DOF, TIVX_TARGET_MAX_NAME);
        status = (vx_status)VX_SUCCESS;
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch statement <metric end>
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
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (status == (vx_status)VX_SUCCESS)
    /* LDRA_JUSTIFY_END */   
    {
        vx_dmpac_dof_target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_DMPAC_DOF_NAME,
                            target_name,
                            tivxDmpacDofProcess,
                            tivxDmpacDofCreate,
                            tivxDmpacDofDelete,
                            tivxDmpacDofControl,
                            NULL);
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != vx_dmpac_dof_target_kernel)
        /* LDRA_JUSTIFY_END */ 
        {
            /* Allocate lock mutex */
            status = tivxMutexCreate(&gTivxDmpacDofInstObj.lock);
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
                memset(&gTivxDmpacDofInstObj.dofObj, 0x0,
                    sizeof(tivxDmpacDofObj) * VHWA_M2M_DOF_MAX_HANDLES);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to Add DOF TargetKernel\n");
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
void tivxRemoveTargetKernelDmpacDof(void)
{
    vx_status status = (vx_status)VX_SUCCESS;

    status = tivxRemoveTargetKernel(vx_dmpac_dof_target_kernel);
    if (status == (vx_status)VX_SUCCESS)
    {
        vx_dmpac_dof_target_kernel = NULL;
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Failed to Remove Dof TargetKernel\n");
    }
    if (NULL != gTivxDmpacDofInstObj.lock)
    {
        (void)tivxMutexDelete(&gTivxDmpacDofInstObj.lock);
    }
}
/* LDRA_JUSTIFY_END */

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxDmpacDofProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                     status = (vx_status)VX_SUCCESS;
    int32_t                       fvid2_status = FVID2_SOK;
    uint32_t                      size;
    uint32_t                      pyr_lvl, pyr_cnt;
    uint32_t                      total_pyr_lvl;
    uint32_t                      isBaseImg;
    tivxDmpacDofObj              *dofObj = NULL;
    Fvid2_FrameList              *inFrmList;
    Fvid2_FrameList              *outFrmList;
    tivx_dmpac_dof_params_t      *dofAppPrms = NULL;
    tivx_dmpac_dof_sof_params_t  *sofAppPrms = NULL;
    void                         *target_ptr;

    tivx_obj_desc_user_data_object_t *config_desc;
    tivx_obj_desc_image_t        *input_current_base_desc = NULL;
    tivx_obj_desc_image_t        *input_reference_base_desc = NULL;
    tivx_obj_desc_pyramid_t      *input_current_desc;
    tivx_obj_desc_pyramid_t      *input_reference_desc;
    tivx_obj_desc_image_t        *flow_vector_in_desc = NULL;
    tivx_obj_desc_user_data_object_t *sparse_of_config_desc;
    tivx_obj_desc_image_t        *sparse_of_map_desc = NULL;
    tivx_obj_desc_image_t        *flow_vector_out_desc;
    tivx_obj_desc_distribution_t *confidence_histogram_desc = NULL;
    tivx_obj_desc_image_t    *img_current_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_image_t    *img_reference_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_t          *out_base_desc = NULL;
    #if !defined(SOC_J722S)
    vx_status                     validate_reg_status = (vx_status)VX_SUCCESS;
    #endif

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
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
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            dofObj = temp_kernel_context;
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
        /* point to descriptors with correct type */
        input_current_base_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];
        input_reference_base_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_BASE_IDX];
        input_current_desc = (tivx_obj_desc_pyramid_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];
        input_reference_desc = (tivx_obj_desc_pyramid_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX];
        flow_vector_in_desc = (tivx_obj_desc_image_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_IN_IDX];
        sparse_of_config_desc = (tivx_obj_desc_user_data_object_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_CONFIG_IDX];
        sparse_of_map_desc = (tivx_obj_desc_image_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];
        flow_vector_out_desc = (tivx_obj_desc_image_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];
        confidence_histogram_desc = (tivx_obj_desc_distribution_t *)
                            obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIDENCE_HISTOGRAM_IDX];

        tivxGetObjDescList(input_current_desc->obj_desc_id,
                           (tivx_obj_desc_t**)img_current_desc,
                           input_current_desc->num_levels);
        tivxGetObjDescList(input_reference_desc->obj_desc_id,
                           (tivx_obj_desc_t**)img_reference_desc,
                           input_reference_desc->num_levels);


        out_base_desc = (tivx_obj_desc_t *)flow_vector_out_desc;
        dofObj->timestamp = out_base_desc->timestamp;

        if(NULL != input_current_base_desc)
        {
            total_pyr_lvl = input_current_desc->num_levels + 1U;
            isBaseImg = 1U;
        }
        else
        {
            total_pyr_lvl = input_current_desc->num_levels;
            isBaseImg = 0U;
        }

        inFrmList = &dofObj->inFrmList;
        outFrmList = &dofObj->outFrmList;

        /* Initialize DOF Input Frame List */
        inFrmList->frames[DOF_INPUT_REFERENCE_IMG] =
                                &dofObj->inFrm[DOF_INPUT_REFERENCE_IMG];
        inFrmList->frames[DOF_INPUT_CURRENT_IMG] =
                                &dofObj->inFrm[DOF_INPUT_CURRENT_IMG];
        inFrmList->frames[DOF_INPUT_TEMPORAL_PRED] =
                                &dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED];
        inFrmList->frames[DOF_INPUT_PYRAMID_PRED] =
                                &dofObj->inFrm[DOF_INPUT_PYRAMID_PRED];
        inFrmList->frames[DOF_INPUT_SOF] = &dofObj->inFrm[DOF_INPUT_SOF];
        inFrmList->numFrames = 5U;

        /* Initialize DOF Output Frame List */
        outFrmList->frames[0U] = &dofObj->outFrm;
        outFrmList->numFrames = 1U;
        /* LDRA_JUSTIFY_START
        <metric start> branch statement <metric end>
        <justification start> Rationale: The negative test framework and test apps cannot reach this portion.
        The true branch of the if statement cannot be achieved because both variables are populated the same way.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(total_pyr_lvl != dofObj->total_pyr_lvl)
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Pyramid Count\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
        if(NULL != sparse_of_config_desc)
        {
            target_ptr = tivxMemShared2TargetPtr(&sparse_of_config_desc->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, sparse_of_config_desc->mem_size,
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            sofAppPrms = (tivx_dmpac_dof_sof_params_t *)target_ptr;

            /* Check if sof configuration changes */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            The test framework does not support the configuration required to trigger this error scenario.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
            This behaviour is part of the application design. An error print statement can be added in a future release if required.
            <justification end> */            
            if(memcmp(sofAppPrms, &dofObj->sofAppPrms,
                sizeof(tivx_dmpac_dof_sof_params_t)) != 0)
            {
                status = tivxDmpacDofUpdateSofPrms(dofObj, sofAppPrms);
            }
            /* LDRA_JUSTIFY_END */
            tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, sparse_of_config_desc->mem_size,
                                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
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
        VX_PRINT(VX_ZONE_ERROR, "Invalid Target Instance Context\n");
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
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Clearing error status from previous frame
        *  User should consume the error status before
        *  submitting next frame for processing
        */
        dofObj->wdTimerErrStatus = 0u;
        dofObj->err_stat = 0u;

        uint64_t cur_time;
        cur_time = tivxPlatformGetTimeInUsecs();

        for(pyr_cnt = total_pyr_lvl; pyr_cnt > 0U; pyr_cnt--)
        {
            pyr_lvl = pyr_cnt - 1U;
            if(0U == isBaseImg)
            {
                dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                    tivxMemShared2PhysPtr(
                        img_reference_desc[pyr_lvl]->mem_ptr[0].shared_ptr,
                        (int32_t)img_reference_desc[pyr_lvl]->mem_ptr[0].mem_heap_region);

                dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                    tivxMemShared2PhysPtr(
                        img_current_desc[pyr_lvl]->mem_ptr[0].shared_ptr,
                        (int32_t)img_current_desc[pyr_lvl]->mem_ptr[0].mem_heap_region);
            }
            else
            {
                if(pyr_lvl > 0U)
                {
                    dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            img_reference_desc[pyr_lvl-1U]->mem_ptr[0].shared_ptr,
                            (int32_t)img_reference_desc[pyr_lvl-1U]->mem_ptr[0].mem_heap_region);

                    dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            img_current_desc[pyr_lvl-1U]->mem_ptr[0].shared_ptr,
                            (int32_t)img_current_desc[pyr_lvl-1U]->mem_ptr[0].mem_heap_region);
                }
                else
                {
                    dofObj->inFrm[DOF_INPUT_REFERENCE_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            input_reference_base_desc->mem_ptr[0].shared_ptr,
                            (int32_t)input_reference_base_desc->mem_ptr[0].mem_heap_region);

                    dofObj->inFrm[DOF_INPUT_CURRENT_IMG].addr[0] =
                        tivxMemShared2PhysPtr(
                            input_current_base_desc->mem_ptr[0].shared_ptr,
                            (int32_t)input_current_base_desc->mem_ptr[0].mem_heap_region);
                }
            }

            dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] = (uint64_t)NULL;
            dofObj->inFrm[DOF_INPUT_SOF].addr[0] = (uint64_t)NULL;

            if(0U == pyr_lvl)
            {
                if(NULL != flow_vector_in_desc)
                {
                    dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] =
                        tivxMemShared2PhysPtr(
                            flow_vector_in_desc->mem_ptr[0].shared_ptr,
                            (int32_t)flow_vector_in_desc->mem_ptr[0].mem_heap_region);
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications CAN reach this portion.
                            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
                Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                        However, due to the stated rationale, this is not tested.
                <justification end> */
                else if (dofObj->flowVecIntDelay != 0U)
                {
                    int32_t diff;

                    /* Check if we have enough history to use a past buffer. */
                    diff = dofObj->outFlowVecWrIdx - dofObj->outFlowVecRdIdx;

                    if (diff < 0)
                    {
                        diff = (diff + (int32_t)TIVX_DMPAC_DOF_FLOW_VEC_QSIZE);
                    }

                    if (diff >= (int32_t)dofObj->flowVecIntDelay)
                    {
                        dofObj->inFrm[DOF_INPUT_TEMPORAL_PRED].addr[0] =
                            dofObj->outFlowVecHistory[dofObj->outFlowVecRdIdx];
                        dofObj->outFlowVecRdIdx = (dofObj->outFlowVecRdIdx + 1);

                        dofObj->outFlowVecRdIdx = (dofObj->outFlowVecRdIdx % (int32_t)TIVX_DMPAC_DOF_FLOW_VEC_QSIZE);
                    }
                }
                else
                {
                    /* No temporal prediction configured */
                }
                /* LDRA_JUSTIFY_END */

                if(NULL != sparse_of_map_desc)
                {
                    dofObj->inFrm[DOF_INPUT_SOF].addr[0] =
                        tivxMemShared2PhysPtr(
                            sparse_of_map_desc->mem_ptr[0].shared_ptr,
                            (int32_t)sparse_of_map_desc->mem_ptr[0].mem_heap_region);
                }
                dofObj->inFrm[DOF_INPUT_PYRAMID_PRED].addr[0] =
                                                            dofObj->inter_buff1;

                dofObj->outFrm.addr[0] =
                        tivxMemShared2PhysPtr(
                            flow_vector_out_desc->mem_ptr[0].shared_ptr,
                            (int32_t)flow_vector_out_desc->mem_ptr[0].mem_heap_region);
            }
            else
            {
                if((pyr_lvl % 2U) == 0U)
                {
                    dofObj->inFrm[DOF_INPUT_PYRAMID_PRED].addr[0] =
                                                            dofObj->inter_buff1;
                    dofObj->outFrm.addr[0] = dofObj->inter_buff2;
                }
                else
                {
                    dofObj->inFrm[DOF_INPUT_PYRAMID_PRED].addr[0] =
                                                            dofObj->inter_buff2;
                    dofObj->outFrm.addr[0] = dofObj->inter_buff1;
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
            if((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                /* Set pyramid level to be processed */
                fvid2_status = Fvid2_control(dofObj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_NEXT_PYR, &pyr_lvl, NULL);
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to set next pyramid\n");
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
            if((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                /* Submit DOF Request*/
                fvid2_status = Fvid2_processRequest(dofObj->handle, inFrmList,
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
                /* Wait for Frame Completion */
                (void)tivxEventWait(dofObj->waitForProcessCmpl, VX_TIMEOUT_WAIT_FOREVER);

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the error.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(0u == dofObj->wdTimerErrStatus)
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
                    if(0u != dofObj->err_stat)
                    {
                        VX_PRINT(VX_ZONE_INFO, "Error interrupt: DOF non-stalling error interrupt triggered \n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */

                    fvid2_status = Fvid2_getProcessedRequest(dofObj->handle,
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
#if !defined(SOC_J722S)
                /* Call the control command for statusReg/configReg validate */
                {
                    fvid2_status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_VALIDATE_REG, NULL, NULL);
                    if (FVID2_SOK != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Register validation failed (Fvid2_control returned %d)\n", fvid2_status);
                        validate_reg_status = (vx_status)VX_FAILURE;
                    }
                }
#endif
            }
        }

        cur_time = tivxPlatformGetTimeInUsecs() - cur_time;

        appPerfStatsHwaUpdateLoad(APP_PERF_HWA_DOF,
            (uint32_t)cur_time,
            dofObj->dofPrms.coreCfg.width*dofObj->dofPrms.coreCfg.height /* pixels processed */
            );
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
            /* Save the out flow vector. */
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> 
            Rationale: The component level negative test framework and test applications CAN reach this portion.
                        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
                    However, due to the stated rationale, this is not tested.
            <justification end> */
            if (dofObj->flowVecIntDelay != 0U)
            {
                uint64_t   ptr;

                ptr = tivxMemShared2PhysPtr(
                        flow_vector_out_desc->mem_ptr[0].shared_ptr,
                        (int32_t)flow_vector_out_desc->mem_ptr[0].mem_heap_region);

                dofObj->outFlowVecHistory[dofObj->outFlowVecWrIdx] = ptr;
                uint32_t outFlowVecWrIdx_temp = (uint32_t)dofObj->outFlowVecWrIdx + 1U;
                dofObj->outFlowVecWrIdx = (int32_t)outFlowVecWrIdx_temp;

                uint32_t outFlowVecWrIdx_mod = (uint32_t)dofObj->outFlowVecWrIdx % (uint32_t)TIVX_DMPAC_DOF_FLOW_VEC_QSIZE;
                dofObj->outFlowVecWrIdx = (int32_t)outFlowVecWrIdx_mod;
            }
            /* LDRA_JUSTIFY_END */
            /* Get Histogram */
            if(NULL != confidence_histogram_desc)
            {
                target_ptr = tivxMemShared2TargetPtr(&confidence_histogram_desc->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, confidence_histogram_desc->mem_size,
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));

                fvid2_status = Fvid2_control(dofObj->handle,
                                       VHWA_M2M_IOCTL_DOF_GET_HISTOGRAM,
                                       (uint32_t *)target_ptr, NULL);
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

                tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, confidence_histogram_desc->mem_size,
                                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_AND_WRITE));
            }

            if(1U == dofObj->isFirstFrame)
            {
                /* This was the first iteration, so the Temporal predictor was
                    turned off, Enable the Temporal predictor if configured */
                dofObj->isFirstFrame = 0u;

                config_desc = (tivx_obj_desc_user_data_object_t *)
                    obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];

                target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, config_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                dofAppPrms = (tivx_dmpac_dof_params_t *)target_ptr;
                status = tivxDmpacDofUpdateCfgPrms(dofObj, dofAppPrms, flow_vector_out_desc->format);

                tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
        }
    }

    #if !defined(SOC_J722S)
    if(((vx_status)VX_SUCCESS != status) || ((vx_status)VX_SUCCESS != validate_reg_status))
    {
        status = (vx_status)VX_FAILURE;
    }
    #endif
    return (status);
}

static vx_status VX_CALLBACK tivxDmpacDofCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    int32_t                           fvid2_status = FVID2_SOK;
    tivx_dmpac_dof_params_t          *dofAppPrms = NULL;
    tivx_dmpac_dof_sof_params_t      *sofAppPrms = NULL;
    Vhwa_M2mDofPrms                  *dofPrms = NULL;
    tivxDmpacDofObj                  *dofObj = NULL;
    tivx_obj_desc_user_data_object_t *config_desc = NULL;
    tivx_obj_desc_user_data_object_t *sof_config_desc = NULL;
    tivx_obj_desc_pyramid_t          *input_current_desc;
    tivx_shared_mem_ptr_t            tBuffPtr;
    void                             *target_ptr, *sof_target_ptr;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_REFERENCE_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX]))
    {
        VX_PRINT(VX_ZONE_ERROR, "Required input parameter set to NULL\n");
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
        dofObj = tivxDmpacDofAllocObject(&gTivxDmpacDofInstObj);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != dofObj)
        /* LDRA_JUSTIFY_END */
        {
            /* point to descriptors with correct type */
            config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_DOF_CONFIGURATION_IDX];

            sof_config_desc = (tivx_obj_desc_user_data_object_t *)
                obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_CONFIG_IDX];

            input_current_desc = (tivx_obj_desc_pyramid_t *)
                                obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];

            if(NULL != obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX])
            {
                dofObj->total_pyr_lvl = input_current_desc->num_levels + 1U;
            }
            else
            {
                dofObj->total_pyr_lvl = input_current_desc->num_levels;
            }

            dofObj->isFirstFrame = 1U;
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
            VX_PRINT(VX_ZONE_ERROR, "May need to increase the value of VHWA_M2M_DOF_MAX_HANDLES in pdk/packages/ti/drv/vhwa/include/vhwa_m2mDof.h\n");
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
        Vhwa_m2mDofCreateArgsInit(&dofObj->createArgs);

        dofPrms = &dofObj->dofPrms;

        target_ptr = tivxMemShared2TargetPtr(&config_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, config_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        dofAppPrms = (tivx_dmpac_dof_params_t *)target_ptr;
        dofObj->createArgs.enablePsa = dofAppPrms->enable_psa;

        /* Initialize DOF Config with 0 */
        DofCfg_init(&dofPrms->coreCfg);

        status = tivxEventCreate(&dofObj->waitForProcessCmpl);
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
            dofObj->cbPrms.cbFxn   = tivxDmpacDofFrameComplCb;
            dofObj->cbPrms.appData = dofObj;

            dofObj->handle = Fvid2_create(FVID2_VHWA_M2M_DOF_DRV_ID,
                VHWA_M2M_DOF_DRV_INST_ID, (void *)&dofObj->createArgs,
                NULL, &dofObj->cbPrms);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL == dofObj->handle)
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Handle\n");
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
        dofObj->errEvtPrms.errEvents =
            ((VHWA_DOF_RD_ERR | VHWA_DOF_WR_ERR) |
            (VHWA_DOF_MP0_RD_STATUS_ERR | VHWA_DOF_FOCO0_SL2_WR_ERR) |
            VHWA_DOF_FOCO0_VBUSM_RD_ERR);
        dofObj->errEvtPrms.cbFxn     = tivxDmpacDofErrorCb;
        dofObj->errEvtPrms.appData   = dofObj;

        fvid2_status = Fvid2_control(dofObj->handle,
            VHWA_M2M_IOCTL_DOF_REGISTER_ERR_CB, &dofObj->errEvtPrms, NULL);
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
            VX_PRINT(VX_ZONE_ERROR, "Error CB registration failed\n");
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
        dofObj->wdTimerErrEvtPrms.wdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        dofObj->wdTimerErrEvtPrms.focoWdTimeoutCycles = VHWA_WDTIMEOUT_64K_CYCLES;
        /* Disable Watchdog Timer Error Event by default */
        dofObj->wdTimerErrEvtPrms.wdTimerErrEvents = 0u;
        dofObj->wdTimerErrEvtPrms.cbFxn     = tivxDmpacDofWdTimerErrorCb;
        dofObj->wdTimerErrEvtPrms.appData   = dofObj;
        fvid2_status = Fvid2_control(dofObj->handle,
            VHWA_M2M_IOCTL_DOF_REGISTER_WDTIMER_ERR_CB, &dofObj->wdTimerErrEvtPrms, NULL);
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

    /* Set DOF Configuration Parameters */
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
        if(NULL != sof_config_desc)
        {
            sof_target_ptr = tivxMemShared2TargetPtr(&sof_config_desc->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(sof_target_ptr, sof_config_desc->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

            sofAppPrms = (tivx_dmpac_dof_sof_params_t *)sof_target_ptr;

            memcpy(&dofObj->sofAppPrms, sofAppPrms, sizeof(tivx_dmpac_dof_sof_params_t));
        }

        tivxDmpacDofSetCfgPrms(dofPrms, dofAppPrms, sofAppPrms, obj_desc);

        if((DOF_PREDICTOR_TEMPORAL == dofAppPrms->base_predictor[0]) ||
           (DOF_PREDICTOR_TEMPORAL == dofAppPrms->base_predictor[1]))
        {
            /* Store the flow vector delay parameter. */
            dofObj->flowVecIntDelay = dofAppPrms->flow_vector_internal_delay_num;
        }
        else
        {
            /* We should ignore the flow_vector_internal_delay_num' if temporal
             * predictor is OFF. Set it to 0 for ease of use in the process
             * function.
             */
            dofObj->flowVecIntDelay = 0U;
        }

        if(NULL != sof_config_desc)
        {
            tivxCheckStatus(&status, tivxMemBufferUnmap(sof_target_ptr, config_desc->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
        }

        /* Save the parameters in the object variable,
           This is used to compare with config in process request to check if
           DOF paramerters needs to be reconfigured */
        memcpy(&dofObj->dofAppPrms, dofAppPrms, sizeof(tivx_dmpac_dof_params_t));

        fvid2_status = Fvid2_control(dofObj->handle,
            VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dofObj->dofPrms, NULL);
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
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, config_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }

    /* Initialize and set the Confidence score parameters to default value */

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
        /* Initialize Confidence Score Parameters with 0 */
        Dof_DecisionTreesInit(&dofObj->csPrms);

        /* Initialize Confidence Score Parameters with default values */
        Vhwa_m2mConfScoreParamInit(&dofObj->csPrms);

        memcpy(&dofObj->csPrms, &gConfScPrms, sizeof(Dof_ConfScoreParam));

        fvid2_status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS,
                                &dofObj->csPrms, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set CS parameter reqeust failed\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    /* Allocate intermediate buffers for Pyramid processing */
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
        /* Size = base image width * height * 2 bytes/pixel / 4 (half scale is largest size needed) */
        dofObj->inter_buff_size = (dofObj->dofPrms.coreCfg.width *
                                   dofObj->dofPrms.coreCfg.height) / 2U;

        status = tivxMemBufferAlloc(&tBuffPtr, dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);
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
            dofObj->inter_buff1 =
                        tivxMemShared2PhysPtr(tBuffPtr.shared_ptr,
                                                (int32_t)tBuffPtr.mem_heap_region);

            status = tivxMemBufferAlloc(&tBuffPtr, dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch statement <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Buffer Alloc 1 Failed\n");
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
            dofObj->inter_buff2 =
                        tivxMemShared2PhysPtr(tBuffPtr.shared_ptr,
                                                (int32_t)tBuffPtr.mem_heap_region);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch statement <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Buffer Alloc 2 Failed\n");
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
        status = tivxSetTargetKernelInstanceContext(kernel, dofObj,
            sizeof(tivxDmpacDofObj));
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
        if (NULL != dofObj)
        {
            if (NULL != dofObj->handle)
            {
                (void)Fvid2_delete(dofObj->handle, NULL);
                dofObj->handle = NULL;
            }

            if (NULL != dofObj->waitForProcessCmpl)
            {
                (void)tivxEventDelete(&dofObj->waitForProcessCmpl);
            }

            tivxDmpacDofFreeObject(&gTivxDmpacDofInstObj, dofObj);
        }
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

static vx_status VX_CALLBACK tivxDmpacDofDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxDmpacDofObj *dofObj = NULL;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */ 
    if (num_params != TIVX_KERNEL_DMPAC_DOF_MAX_PARAMS)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
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
            dofObj = temp_kernel_context;
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) && (NULL != dofObj) &&
            (sizeof(tivxDmpacDofObj) == size))
        /* LDRA_JUSTIFY_END */
        {
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != dofObj->handle)
            /* LDRA_JUSTIFY_END */
            {
                (void)Fvid2_delete(dofObj->handle, NULL);
                dofObj->handle = NULL;
            }

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != dofObj->waitForProcessCmpl)
            /* LDRA_JUSTIFY_END */
            {
                (void)tivxEventDelete(&dofObj->waitForProcessCmpl);
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((int32_t)NULL != (int32_t)dofObj->inter_buff1)
            /* LDRA_JUSTIFY_END */
            {
                vx_status free_status = tivxMemFree( (void*)(uintptr_t)(dofObj->inter_buff1), dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                Memory free operations are expected to succeed in normal conditions.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != free_status)
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to free memory\n");
                }
                /* LDRA_JUSTIFY_END */
                dofObj->inter_buff1 = (uint64_t)NULL;
            }
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            Effect on this unit: If the control reaches here, then our code base is expected to prevent undefined behaviour due to dereferencing a NULL pointer.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((int32_t)NULL != (int32_t)dofObj->inter_buff2)
            /* LDRA_JUSTIFY_END */
            {
                vx_status free_status = tivxMemFree( (void*)(uintptr_t)(dofObj->inter_buff2), dofObj->inter_buff_size, (vx_enum)TIVX_MEM_EXTERNAL);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                Memory free operations are expected to succeed in normal conditions.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != free_status)
                /* LDRA_JUSTIFY_END */
                {
                    VX_PRINT(VX_ZONE_ERROR, "Failed to free memory\n");
                }
                dofObj->inter_buff2 = (uint64_t)NULL;
            }

#if !defined(SOC_J722S)
            /* Free the readback and golden register buffers */
            tivxDmpacDofFreeReadbackBuffers(dofObj);
#endif
            tivxDmpacDofFreeObject(&gTivxDmpacDofInstObj, dofObj);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxDmpacDofControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          size;
    tivxDmpacDofObj                   *dofObj = NULL;

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
        dofObj = temp_kernel_context;
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
    else if ((NULL == dofObj) ||
        (sizeof(tivxDmpacDofObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
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
            case TIVX_DMPAC_DOF_CMD_CS_PARAMS:
            {
                status = tivxDmpacDofSetCsPrms(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_SET_HTS_BW_LIMIT_PARAMS:
            {
                status = tivxDmpacDofSetHtsBwLimit(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_GET_ERR_STATUS:
            {
                status = tivxDmpacDofGetErrStatusCmd(dofObj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_GET_PSA_STATUS:
            {
                status = tivxDmpacDofGetPsaStatusCmd(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_SET_SOF_PARAMS:
            {
                if(((dofObj->sofAppPrms.sof_fv_height != 0U) && (dofObj->sofAppPrms.sof_max_pix_in_row != 0U)))
                {
                    status = tivxDmpacDofSetSofParams(dofObj,
                        (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "SOF Not Enabled\n");
                    status = (vx_status)VX_FAILURE;
                }
                break;
            }
            case TIVX_DMPAC_DOF_CMD_SET_DOF_PARAMS:
            {
                status = tivxDmpacDofSetDofParams(dofObj, 
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0]);
                break;
            }
            case TIVX_DMPAC_DOF_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxDmpacDofEnableErrorEventsCmd(dofObj,
                  (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
#if defined(LDRA_COVERAGE)
            case TIVX_DMPAC_CMD_GET_COVERAGE_START:
            {
                #if defined (C7X_FAMILY) || defined(R5F)
                VX_PRINT(VX_ZONE_ERROR, "################### DYNAMIC CODE COVERAGE INITIALIZATION STARTED ######################\n");
                ldra_initialize();
                VX_PRINT(VX_ZONE_ERROR, "################### DYNAMIC CODE COVERAGE INITIALIZATION FINISHED ######################\n");
                #endif  
                break;
            }
#endif
#if !defined(SOC_J722S)
            case TIVX_DMPAC_DOF_CMD_ENABLE_DMPAC_SAFETY_MECHANISM:
            {
                status = tivxEnableDmpacDofSafetyMechanisms(dofObj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0]);
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to enable DMPAC DOF safety mechanisms\n");
                }
                /* LDRA_JUSTIFY_END */
                break;
            }
#endif
            default:
            {
                VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
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

static tivxDmpacDofObj *tivxDmpacDofAllocObject(tivxDmpacDofInstObj *instObj)
{
    uint32_t        cnt;
    tivxDmpacDofObj *dofObj = NULL;
    vx_status       status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (0U == instObj->dofObj[cnt].isAlloc)
            {
                dofObj = &instObj->dofObj[cnt];
                memset(dofObj, 0x0, sizeof(tivxDmpacDofObj));
                instObj->dofObj[cnt].isAlloc = 1U;
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

    return (dofObj);
}

static void tivxDmpacDofFreeObject(tivxDmpacDofInstObj *instObj,
    tivxDmpacDofObj *dofObj)
{
    uint32_t cnt;
    vx_status status;

    /* Lock instance mutex */
    status = tivxMutexLock(instObj->lock);
    if((vx_status)VX_SUCCESS == status)
    {
        /* LDRA_JUSTIFY_START
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The test framework does not support the configuration required to trigger this error scenario.
        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
        This behaviour is part of the application design. An error print statement can be added in a future release if required.
        <justification end> */
        for (cnt = 0U; cnt < VHWA_M2M_DOF_MAX_HANDLES; cnt ++)
        /* LDRA_JUSTIFY_END */
        {
            if (dofObj == &instObj->dofObj[cnt])
            {
                dofObj->isAlloc = 0U;
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


static void tivxDmpacDofSetFmt(Fvid2_Format *fmt,
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
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The parameter is expected to be pre-validated from a software layer above imaging. Therefore, this failure case is out of scope for the imaging test framework.
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

        fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
    }
}

static void tivxDmpacDofSetCfgPrms(Vhwa_M2mDofPrms *dofPrms,
    const tivx_dmpac_dof_params_t *dofAppPrms, const tivx_dmpac_dof_sof_params_t *sofAppPrms,
    tivx_obj_desc_t *obj_desc[])
{
    uint32_t                  pyr_cnt;

    tivx_obj_desc_image_t    *input_curr_base_desc = NULL;
    tivx_obj_desc_pyramid_t  *input_curr_desc;
    tivx_obj_desc_image_t    *sof_map_desc = NULL;
    tivx_obj_desc_image_t    *img_current_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];
    tivx_obj_desc_image_t    *fv_out_desc;
    Fvid2_Format             *fmt;

    input_curr_desc = (tivx_obj_desc_pyramid_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_IDX];

    input_curr_base_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_DMPAC_DOF_INPUT_CURRENT_BASE_IDX];

    tivxGetObjDescList(input_curr_desc->obj_desc_id,
                       (tivx_obj_desc_t**)img_current_desc,
                       input_curr_desc->num_levels);

    Vhwa_m2mDofConfigInit(dofPrms);

    if(NULL != input_curr_base_desc)
    {
        /* Information is in base input */
        dofPrms->tPrmdLvl = input_curr_desc->num_levels + 1U;

        dofPrms->coreCfg.width = input_curr_base_desc->imagepatch_addr[0U].dim_x;
        dofPrms->coreCfg.height = input_curr_base_desc->imagepatch_addr[0U].dim_y;
        dofPrms->flowVectorHeight = input_curr_base_desc->imagepatch_addr[0U].dim_y;

    }
    else
    {
        /* Information is in pyramid input */

        dofPrms->tPrmdLvl = input_curr_desc->num_levels;
        dofPrms->coreCfg.width = img_current_desc[0]->imagepatch_addr[0U].dim_x;
        dofPrms->coreCfg.height = img_current_desc[0]->imagepatch_addr[0U].dim_y;
        dofPrms->flowVectorHeight = img_current_desc[0]->imagepatch_addr[0U].dim_y;
    }

    fv_out_desc = (tivx_obj_desc_image_t *)
                    obj_desc[TIVX_KERNEL_DMPAC_DOF_FLOW_VECTOR_OUT_IDX];

    dofPrms->coreCfg.enableDOF = 1u;
    dofPrms->tPredictor  = DOF_PREDICTOR_DELEY_LEFT;
    dofPrms->mPredictor1 = dofAppPrms->inter_predictor[0];
    dofPrms->mPredictor2 = dofAppPrms->inter_predictor[1];
    dofPrms->bPredictor1 = dofAppPrms->base_predictor[0];
    dofPrms->bPredictor2 = dofAppPrms->base_predictor[1];

    dofPrms->coreCfg.horizontalSearchRange = dofAppPrms->horizontal_search_range;
    dofPrms->coreCfg.topSearchRange = dofAppPrms->vertical_search_range[0];
    dofPrms->coreCfg.bottomSearchRange = dofAppPrms->vertical_search_range[1];
    dofPrms->coreCfg.medianFilter = dofAppPrms->median_filter_enable;

    if(0U == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 0U;
        dofPrms->coreCfg.referenceCensusTransform = 0U;
    }
    else if(1U == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 0U;
        dofPrms->coreCfg.referenceCensusTransform = 1U;
    }
    else if(2U == dofAppPrms->motion_direction)
    {
        dofPrms->coreCfg.currentCensusTransform = 1U;
        dofPrms->coreCfg.referenceCensusTransform = 0U;
    }
    else
    {
        dofPrms->coreCfg.currentCensusTransform = 1U;
        dofPrms->coreCfg.referenceCensusTransform = 1U;
    }

    dofPrms->coreCfg.motionSmoothnessFactor = dofAppPrms->motion_smoothness_factor;
    dofPrms->coreCfg.iirFilterAlpha = dofAppPrms->iir_filter_alpha;

    dofPrms->coreCfg.lkConfidanceScore = (fv_out_desc->format == (vx_df_image)VX_DF_IMAGE_U16) ? 0U : 1U;

    if((NULL != obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX]) &&
       (NULL != sofAppPrms))
    {
        sof_map_desc = (tivx_obj_desc_image_t *)
                        obj_desc[TIVX_KERNEL_DMPAC_DOF_SPARSE_OF_MAP_IDX];

        dofPrms->inOutImgFmt[0][DOF_INPUT_SOF].pitch[0U] =
                                (uint32_t)sof_map_desc->imagepatch_addr[0].stride_y;

        dofPrms->coreCfg.enableSof   = 1u;
        dofPrms->coreCfg.maxMVsInSof = sofAppPrms->sof_max_pix_in_row;
        dofPrms->flowVectorHeight = sofAppPrms->sof_fv_height;
    }
    else
    {
        dofPrms->coreCfg.enableSof = 0u;
        dofPrms->inOutImgFmt[0][DOF_INPUT_SOF].pitch[0U] = 0u;
    }

    if((DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor1) ||
       (DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor2))
    {
        dofPrms->inOutImgFmt[0][DOF_INPUT_TEMPORAL_PRED].pitch[0U] =
                                    (uint32_t)fv_out_desc->imagepatch_addr[0].stride_y;
        dofPrms->coreCfg.enableSof = 0u;
    }
    else
    {
        dofPrms->inOutImgFmt[0][DOF_INPUT_TEMPORAL_PRED].pitch[0U] = 0u;
    }

    dofPrms->inOutImgFmt[0][DOF_OUTPUT].pitch[0U] =
                                    (uint32_t)fv_out_desc->imagepatch_addr[0].stride_y;

    for(pyr_cnt = 0U; pyr_cnt < dofPrms->tPrmdLvl; pyr_cnt++)
    {
        /* Set for Temporal and SOF image */
        if(0U != pyr_cnt)
        {
            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_TEMPORAL_PRED].pitch[0U] = 0U;
            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_SOF].pitch[0U] = 0U;
        }

        /* Set for reference and current image */
        fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_CURRENT_IMG];

        if(NULL == input_curr_base_desc)
        {
            tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt]);

            fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
            tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt]);

            dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        (uint32_t)img_current_desc[pyr_cnt]->imagepatch_addr[0].stride_y;

            if(pyr_cnt > 0U)
            {
                dofPrms->inOutImgFmt[pyr_cnt][DOF_OUTPUT].pitch[0U] =
                    img_current_desc[pyr_cnt]->imagepatch_addr[0].dim_x * 2u;
            }
            if (pyr_cnt < (dofPrms->tPrmdLvl-1u))
            {
                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                    (uint32_t)img_current_desc[pyr_cnt+1u]->imagepatch_addr[0].dim_x * 2u;
            }
        }
        else
        {
            if(pyr_cnt > 0U)
            {
                tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt-1U]);

                fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
                tivxDmpacDofSetFmt(fmt, img_current_desc[pyr_cnt-1U]);

                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        (uint32_t)img_current_desc[pyr_cnt]->imagepatch_addr[0].dim_x * 2u;

                dofPrms->inOutImgFmt[pyr_cnt][DOF_OUTPUT].pitch[0U] =
                    (uint32_t)img_current_desc[pyr_cnt-1U]->imagepatch_addr[0].dim_x * 2u;
            }
            else
            {
                tivxDmpacDofSetFmt(fmt, input_curr_base_desc);

                fmt = &dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_REFERENCE_IMG];
                tivxDmpacDofSetFmt(fmt, input_curr_base_desc);

                dofPrms->inOutImgFmt[pyr_cnt][DOF_INPUT_PYRAMID_PRED].pitch[0U] =
                        (uint32_t)img_current_desc[0u]->imagepatch_addr[0].dim_x * 2u;
            }
        }

        dofPrms->focoPrms.shiftM1 = 0u;
        dofPrms->focoPrms.dir = 0u;
        dofPrms->focoPrms.round = 0u;

        /* For first frame temporal input is not available */
        if(DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor2)
        {
            dofPrms->bPredictor2 = DOF_PREDICTOR_NONE;
        }
        if(DOF_PREDICTOR_TEMPORAL == dofPrms->bPredictor1)
        {
            dofPrms->bPredictor1 = DOF_PREDICTOR_NONE;
        }
    }


    return;
}

static vx_status tivxDmpacDofUpdateCfgPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_dmpac_dof_params_t *dofAppPrms, uint32_t output_format)
{
    vx_status                    status = (vx_status)VX_SUCCESS;
    int32_t                      fvid2_status = FVID2_SOK;
    
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(NULL == dofAppPrms)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This branch statement checks for errors accumulated in previous steps.
    Since no error is accumulated due to previously stated rationales, some branches for this branch statement cannot be reached.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        dof_obj->dofPrms.mPredictor1 = dofAppPrms->inter_predictor[0];
        dof_obj->dofPrms.mPredictor2 = dofAppPrms->inter_predictor[1];
        dof_obj->dofPrms.bPredictor1 = dofAppPrms->base_predictor[0];
        dof_obj->dofPrms.bPredictor2 = dofAppPrms->base_predictor[1];
        dof_obj->dofPrms.coreCfg.horizontalSearchRange =
                                    dofAppPrms->horizontal_search_range;
        dof_obj->dofPrms.coreCfg.topSearchRange =
                                    dofAppPrms->vertical_search_range[0];
        dof_obj->dofPrms.coreCfg.bottomSearchRange =
                                    dofAppPrms->vertical_search_range[1];
        dof_obj->dofPrms.coreCfg.medianFilter =
                                    dofAppPrms->median_filter_enable;

        if(0U == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 0U;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 0U;
        }
        else if(1U == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 0U;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 1U;
        }
        else if(2U == dofAppPrms->motion_direction)
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 1U;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 0U;
        }
        else
        {
            dof_obj->dofPrms.coreCfg.currentCensusTransform = 1U;
            dof_obj->dofPrms.coreCfg.referenceCensusTransform = 1U;
        }

        dof_obj->dofPrms.coreCfg.motionSmoothnessFactor =
                                    dofAppPrms->motion_smoothness_factor;
        dof_obj->dofPrms.coreCfg.iirFilterAlpha = dofAppPrms->iir_filter_alpha;
        dof_obj->dofPrms.coreCfg.lkConfidanceScore =
                    (output_format == (vx_df_image)VX_DF_IMAGE_U16) ? 0U : 1U;

        fvid2_status = Fvid2_control(dof_obj->handle,
                    VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dof_obj->dofPrms, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set Parameter request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            memcpy(&dof_obj->dofAppPrms, dofAppPrms,
                                            sizeof(tivx_dmpac_dof_params_t));

            status = (vx_status)VX_SUCCESS;
        }
    }

    return (status);

}

static vx_status tivxDmpacDofUpdateSofPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_dmpac_dof_sof_params_t *sofAppPrms)
{
    vx_status                    status = (vx_status)VX_SUCCESS;
    int32_t                      fvid2_status = FVID2_SOK;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the caller before the control reaches here.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
    <justification end> */    
    if(NULL == sofAppPrms)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start> 
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The test framework does not support the configuration required to trigger this error scenario.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        dof_obj->dofPrms.coreCfg.maxMVsInSof = sofAppPrms->sof_max_pix_in_row;
        dof_obj->dofPrms.flowVectorHeight = sofAppPrms->sof_fv_height;

        fvid2_status = Fvid2_control(dof_obj->handle,
                    VHWA_M2M_IOCTL_DOF_SET_PARAMS, &dof_obj->dofPrms, NULL);
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK != fvid2_status)
        {
            VX_PRINT(VX_ZONE_ERROR, "Set Parameter request failed\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
        <justification end> */    
        else
        {
            memcpy(&dof_obj->sofAppPrms, sofAppPrms,
                                            sizeof(tivx_dmpac_dof_sof_params_t));
        }
        /* LDRA_JUSTIFY_END */
    }

    return (status);
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
static vx_status tivxDmpacDofMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t mapped_tivx_err_events = 0u;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: This function is not part of the API and is only called internally.
    The caller is expected to pass a valid pointer.
    Effect on this unit: The unit is NOT expected to result in an error because the parameter is internally controlled.
    <justification end> */
    if (NULL == tivx_err_events)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer argument\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else if ((0u != (vhwa_err_events & (~VHWA_DOF_ALL_ERR_EVENTS))) ||
             (0u != (wd_timer_err & (~VHWA_DOF_ALL_WDTIMER_ERR_EVENTS))))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid error event mask provided\n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Map VHWA flags to TIVX flags for regular error interrupts */
        if (0u != (vhwa_err_events & VHWA_DOF_RD_ERR))
        /* LDRA_JUSTIFY_END */
        {
            mapped_tivx_err_events |= TIVX_DMPAC_DOF_RD_ERR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_DOF_WR_ERR))
        /* LDRA_JUSTIFY_END */
        {
            mapped_tivx_err_events |= TIVX_DMPAC_DOF_WR_ERR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_DOF_MP0_RD_STATUS_ERR))
        /* LDRA_JUSTIFY_END */
        {
            mapped_tivx_err_events |= TIVX_DMPAC_DOF_MP0_RD_STATUS_ERR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_DOF_FOCO0_SL2_WR_ERR))
        /* LDRA_JUSTIFY_END */
        {
            mapped_tivx_err_events |= TIVX_DMPAC_DOF_FOCO0_SL2_WR_ERR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (vhwa_err_events & VHWA_DOF_FOCO0_VBUSM_RD_ERR))
        /* LDRA_JUSTIFY_END */
        {
            mapped_tivx_err_events |= TIVX_DMPAC_DOF_FOCO0_VBUSM_RD_ERR;
        }

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        /* Map VHWA flags to TIVX flags for watchdog timer errors */
        if (0u != (wd_timer_err & VHWA_DOF_WDTIMER_ERR))
        /* LDRA_JUSTIFY_END */
        {
            mapped_tivx_err_events |= TIVX_DMPAC_DOF_WDTIMER_ERR;
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0u != (wd_timer_err & VHWA_DOF_FOCO0_WDTIMER_ERR))
        /* LDRA_JUSTIFY_END */
        {
            mapped_tivx_err_events |= TIVX_DMPAC_DOF_FOCO0_WDTIMER_ERR;
        }

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
 * \return VX_SUCCESS on success, VX_FAILURE if null pointer,
 *         VX_ERROR_INVALID_PARAMETERS if invalid error event mask
 */
static vx_status tivxDmpacDofMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
                                                uint32_t *vhwa_err_events,
                                                uint32_t *wd_timer_err)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t enableErrorEventCheckMask = (uint32_t)TIVX_DMPAC_DOF_ALL_ERR_EVENTS;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: This function is not part of the API and is only called internally.
    The caller is expected to pass valid pointers.
    Effect on this unit: The unit is NOT expected to result in an error because the parameters are internally controlled.
    <justification end> */
    if ((NULL == vhwa_err_events) || (NULL == wd_timer_err))
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer argument\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (0u != (tivx_err_events & (~enableErrorEventCheckMask)))
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
        if (0u != (tivx_err_events & TIVX_DMPAC_DOF_RD_ERR))
        {
            *vhwa_err_events |= VHWA_DOF_RD_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_DOF_WR_ERR))
        {
            *vhwa_err_events |= VHWA_DOF_WR_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_DOF_MP0_RD_STATUS_ERR))
        {
            *vhwa_err_events |= VHWA_DOF_MP0_RD_STATUS_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_DOF_FOCO0_SL2_WR_ERR))
        {
            *vhwa_err_events |= VHWA_DOF_FOCO0_SL2_WR_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_DOF_FOCO0_VBUSM_RD_ERR))
        {
            *vhwa_err_events |= VHWA_DOF_FOCO0_VBUSM_RD_ERR;
        }

        /* Map TIVX flags to VHWA flags for watchdog timer errors */
        if (0u != (tivx_err_events & TIVX_DMPAC_DOF_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_DOF_WDTIMER_ERR;
        }
        if (0u != (tivx_err_events & TIVX_DMPAC_DOF_FOCO0_WDTIMER_ERR))
        {
            *wd_timer_err |= VHWA_DOF_FOCO0_WDTIMER_ERR;
        }
    }

    return status;
}

/* ========================================================================== */
/*                    Control Command Implementation                          */
/* ========================================================================== */

static vx_status tivxDmpacDofSetCsPrms(tivxDmpacDofObj *dof_obj,
                        const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    uint32_t                            idx;
    uint32_t                            cnt;
    vx_status                           status = (vx_status)VX_SUCCESS;
    int32_t                             fvid2_status = FVID2_SOK;
    tivx_dmpac_dof_cs_tree_params_t    *cs_prms;
    Dof_ConfScoreParam                  csPrms;
    void                               *target_ptr;

    if(NULL == usr_data_obj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Input\n");
        status = (vx_status)VX_FAILURE;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_dmpac_dof_cs_tree_params_t) ==
                usr_data_obj->mem_size)
        {
            cs_prms = (tivx_dmpac_dof_cs_tree_params_t *)target_ptr;

            csPrms.confidanceScoreGain = cs_prms->cs_gain;

            for(idx = 0U; idx < DOF_NUM_DECISION_TREES; idx++)
            {
                for(cnt = 0U; cnt < 3U; cnt++)
                {
                    csPrms.decisionTree[idx].index[cnt] =
                                    cs_prms->decision_tree_index[idx][cnt];
                    csPrms.decisionTree[idx].threshold[cnt] =
                                    cs_prms->decision_tree_threshold[idx][cnt];
                }
                for(cnt = 0U; cnt < 4U; cnt++)
                {
                    csPrms.decisionTree[idx].weight[cnt] =
                                    cs_prms->decision_tree_weight[idx][cnt];
                }
            }

            fvid2_status = Fvid2_control(dof_obj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_CONF_SCORE_PARAMS, &csPrms, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
                                This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Set CS parameter request failed\n");
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

static vx_status tivxDmpacDofGetErrStatusCmd(const tivxDmpacDofObj *dof_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat = 0u;

    if (NULL != scalar_obj_desc)
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxDmpacDofMapVhwaToTivxErrEvents(dof_obj->err_stat, dof_obj->wdTimerErrStatus, &combined_err_stat);
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

static vx_status tivxDmpacDofSetSofParams(tivxDmpacDofObj *dofObj,
    const tivx_obj_desc_user_data_object_t *dof_app_prms_sof_user_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_dmpac_dof_sof_params_t *sofAppPrms = NULL;
    void *target_ptr;

    if(NULL != dof_app_prms_sof_user_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&dof_app_prms_sof_user_desc->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, dof_app_prms_sof_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_dmpac_dof_sof_params_t) ==
                dof_app_prms_sof_user_desc->mem_size)
        {
            sofAppPrms = (tivx_dmpac_dof_sof_params_t *)target_ptr;
            status = tivxDmpacDofUpdateSofPrms(dofObj, sofAppPrms);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, dof_app_prms_sof_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxDmpacDofSetDofParams(tivxDmpacDofObj *dofObj,
    const tivx_obj_desc_user_data_object_t *dof_app_prms_user_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    tivx_dmpac_dof_params_t *dofAppPrms = NULL;
    vx_df_image output_format;
    void *target_ptr;

    if(NULL != dof_app_prms_user_desc)
    {
        target_ptr = tivxMemShared2TargetPtr(&dof_app_prms_user_desc->mem_ptr);
        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, dof_app_prms_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

        if (sizeof(tivx_dmpac_dof_params_t) ==
                dof_app_prms_user_desc->mem_size)
        {
            dofAppPrms = (tivx_dmpac_dof_params_t *)target_ptr;
            /* Output Image format cannot be updated across frames */
            output_format = (dofObj->dofPrms.coreCfg.lkConfidanceScore  == 0U) ?
                (vx_df_image)VX_DF_IMAGE_U16 : (vx_df_image)VX_DF_IMAGE_U32;
            status = tivxDmpacDofUpdateCfgPrms(dofObj, dofAppPrms, output_format);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
            status = (vx_status)VX_FAILURE;
        }
        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, dof_app_prms_user_desc->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
    }
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return (status);
}

static vx_status tivxDmpacDofGetPsaStatusCmd(const tivxDmpacDofObj *dof_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            psa;
    void                                *target_ptr;
    int32_t                             fvid2_status = FVID2_SOK;
    tivx_dmpac_dof_psa_timestamp_data_t *psa_status_ptr;

    if (NULL != usr_data_obj)
    {
        if (sizeof(tivx_dmpac_dof_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            fvid2_status = Fvid2_control(dof_obj->handle,
                VHWA_M2M_IOCTL_DOF_GET_PSA_SIGN, &psa, NULL);
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
                    "tivxDmpacDofGetPsa: Fvid2_control Failed\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */
            /* Map the user data object to access the memory */
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            psa_status_ptr = (tivx_dmpac_dof_psa_timestamp_data_t *)target_ptr;

            /* Copy driver data (PSA values) into the structure */
            tivx_obj_desc_memcpy(&psa_status_ptr->psa_values, &psa, sizeof(psa));

            /* Add timestamp to the structure */
            psa_status_ptr->timestamp = dof_obj->timestamp;

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

static vx_status tivxDmpacDofSetHtsBwLimit(tivxDmpacDofObj *dof_obj,
                               const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                                status = (vx_status)VX_SUCCESS;
    int32_t                                  fvid2_status = FVID2_SOK;
    Vhwa_HtsLimiter                          hts_limit;
    tivx_dmpac_dof_hts_bw_limit_params_t    *app_hts_prms;
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

        if (sizeof(tivx_dmpac_dof_hts_bw_limit_params_t) ==
                usr_data_obj->mem_size)
        {
            app_hts_prms = (tivx_dmpac_dof_hts_bw_limit_params_t *)target_ptr;

            hts_limit.enableBwLimit = app_hts_prms->enable_hts_bw_limit;
            hts_limit.cycleCnt = app_hts_prms->cycle_cnt;
            hts_limit.tokenCnt = app_hts_prms->token_cnt;

            fvid2_status = Fvid2_control(dof_obj->handle,
                        VHWA_M2M_IOCTL_DOF_SET_HTS_LIMIT, &hts_limit, NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> Rationale: The component level negative test framework and test applications cannot reach this portion.
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

#if !defined(SOC_J722S)
static vx_status tivxEnableDmpacDofSafetyMechanisms(
    tivxDmpacDofObj *dofObj,
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
    if (NULL == dofObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "dofObj is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
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
                if (sizeof(tivx_dmpac_dof_safety_mechanism_params_t) == usr_data_obj->mem_size)
                {
                    tivx_dmpac_dof_safety_mechanism_params_t *safety_params =
                        (tivx_dmpac_dof_safety_mechanism_params_t *)params_ptr;

                    /* Enable or disable reconfigMMR based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_reconfig_and_reinit_Reg;
                    fvid2_status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
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
                    fvid2_status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_ENABLE_STATUS_REG_VALIDATE, &enable, NULL);
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
                        uint32_t readback_size = 0U;

                        /* Fetch the TIVX_DMPAC_DOF_READBACK_SIZE from the low level driver */
                        /* LDRA_JUSTIFY_START
                        <metric start> branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if((vx_status)VX_SUCCESS == status)
                        /* LDRA_JUSTIFY_END */
                        {
                            fvid2_status = Fvid2_control(
                                dofObj->handle,
                                VHWA_M2M_IOCTL_DOF_GET_READBACK_SIZE,
                                &readback_size,
                                NULL
                            );

                            dofObj->config_reg_mem_size = readback_size;

                            if (((FVID2_SOK == fvid2_status) && (readback_size > 0U)))
                            {
                                VX_PRINT(VX_ZONE_INFO, "DOF readback size allocation successful: %u bytes\n", readback_size);
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
                                VX_PRINT(VX_ZONE_ERROR, "Failed to fetch DOF readback size from driver\n");
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
                            if ((dofObj->readback_mem_ptr_phys != 0U) && (dofObj->golden_reg_mem_ptr_phys != 0U))
                            {
                                VX_PRINT(VX_ZONE_INFO, "Readback or golden register buffer already allocated, skipping allocation\n");
                            }
                            else
                            {
                                status = tivxDmpacDofAllocReadbackBuffers(dofObj);
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
                            fvid2_status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                                tivxDmpacDofFreeReadbackBuffers(dofObj);
                                status = (vx_status)VX_FAILURE;
                            }
                            /* LDRA_JUSTIFY_END */
                        }
                        else
                        {
                            VX_PRINT(VX_ZONE_ERROR, "DOF readback buffer allocation failed, skipping config register validation enable\n");
                        }
                    }
                    else
                    {
                        enable = 0U;
                        fvid2_status = Fvid2_control(dofObj->handle, VHWA_M2M_IOCTL_DOF_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
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
                            VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                            status = (vx_status)VX_FAILURE;
                        }
                    }
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
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Argument\n");
                    status = (vx_status)VX_FAILURE;
                }

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
static vx_status tivxDmpacDofEnableErrorEventsCmd(tivxDmpacDofObj *dof_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t intr_en = 0u;
    uint32_t wd_timer_intr_en = 0u;
    void *target_ptr;
    tivx_dmpac_dof_params_t *params_ptr = NULL;
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
        if(sizeof(tivx_dmpac_dof_params_t) != usr_data_obj->mem_size)
        {
            VX_PRINT(VX_ZONE_ERROR, "tivxDmpacDofEnableErrorEventsCmd: Incorrect Data Object Size \n");
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
                    "tivxDmpacDofEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_dmpac_dof_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->enable_error_events;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxDmpacDofMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

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
                        "tivxDmpacDofEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
                }
                /* LDRA_JUSTIFY_END */
                else
                {
                    dof_obj->errEvtPrms.errEvents = intr_en;
                    fvid2_status = Fvid2_control(dof_obj->handle,
                        VHWA_M2M_IOCTL_DOF_REGISTER_ERR_CB, &dof_obj->errEvtPrms, NULL);

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
                        dof_obj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->wdtimer_timeout_cycles;
                        dof_obj->wdTimerErrEvtPrms.focoWdTimeoutCycles = params_ptr->foco_wdtimer_timeout_cycles;
                        dof_obj->wdTimerErrEvtPrms.wdTimerErrEvents = wd_timer_intr_en;
                        fvid2_status = Fvid2_control(dof_obj->handle,
                            VHWA_M2M_IOCTL_DOF_REGISTER_WDTIMER_ERR_CB, &dof_obj->wdTimerErrEvtPrms, NULL);
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
                            "tivxDmpacDofEnableErrorEventsCmd: Fvid2_control Failed \n");
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
            "tivxDmpacDofEnableErrorEventsCmd: Null Argument\n");
        status = (vx_status)VX_FAILURE;
    }

    return status;
}

#if !defined(SOC_J722S)
static vx_status tivxDmpacDofAllocReadbackBuffers(tivxDmpacDofObj *dofObj)
{
    vx_status status = (vx_status)VX_SUCCESS;

    /* Null pointer checks */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL == dofObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxDmpacDofAllocReadbackBuffers\n");
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
    else if (NULL == dofObj->handle)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxDmpacDofAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
        /* Valid dofObj and handle */
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
        uint32_t readback_size = dofObj->config_reg_mem_size;

        vx_status alloc_status = tivxMemBufferAlloc(&dofObj->readback_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);
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
            dofObj->readback_mem_ptr_phys = tivxMemShared2PhysPtr(dofObj->readback_mem_ptr_virt.shared_ptr, (int32_t)dofObj->readback_mem_ptr_virt.mem_heap_region);
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
            Fvid2Utils_memset((void *)(uintptr_t)dofObj->readback_mem_ptr_virt.shared_ptr, 0, readback_size);
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
            alloc_status = tivxMemBufferAlloc(&dofObj->golden_reg_mem_ptr_virt, readback_size, (vx_enum)TIVX_MEM_EXTERNAL);

            dofObj->golden_reg_mem_ptr_phys = tivxMemShared2PhysPtr(dofObj->golden_reg_mem_ptr_virt.shared_ptr, (int32_t)dofObj->golden_reg_mem_ptr_virt.mem_heap_region);
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
            Fvid2Utils_memset((void *)(uintptr_t)dofObj->golden_reg_mem_ptr_virt.shared_ptr, 0, readback_size);
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
            Vhwa_M2mDofConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ptr   = (void *)(uintptr_t)dofObj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)dofObj->readback_mem_ptr_virt.shared_ptr;

            /* Register the callback for config register memory comparison */
            reg_ptrs.cbFxn = tivxDmpacDofConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)dofObj;

            int32_t fvid2_status = Fvid2_control(
                dofObj->handle,
                VHWA_M2M_IOCTL_DOF_SET_CONFIG_REG_PTRS, /* <-- You must define this IOCTL in your driver */
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
                VX_PRINT(VX_ZONE_ERROR, "Failed to set DOF register readback/golden pointers\n");
                status = (vx_status)VX_FAILURE;
            }
            /* LDRA_JUSTIFY_END */

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
                tivxDmpacDofFreeReadbackBuffers(dofObj);
            }
            /* LDRA_JUSTIFY_END */
        }
        else
        {
            tivxDmpacDofFreeReadbackBuffers(dofObj);
            status = (vx_status)VX_FAILURE;
        }
    }

    return status;
}

static void tivxDmpacDofFreeReadbackBuffers(tivxDmpacDofObj *dofObj)
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
    if (NULL != dofObj)
    /* LDRA_JUSTIFY_END */
    {
        vx_status status;
        uint32_t readback_size = dofObj->config_reg_mem_size;

        /* Free readback buffer if allocated */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory free operations are expected to succeed in normal conditions.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (dofObj->readback_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&dofObj->readback_mem_ptr_virt, readback_size);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free readback buffer\n");
            }
            dofObj->readback_mem_ptr_phys = 0u;
        }
        /* LDRA_JUSTIFY_END */

        /* Free golden register buffer if allocated */
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        Memory free operations are expected to succeed in normal conditions.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (dofObj->golden_reg_mem_ptr_phys != 0u)
        {
            status = tivxMemBufferFree(&dofObj->golden_reg_mem_ptr_virt, readback_size);
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Failed to free golden register buffer\n");
            }
            dofObj->golden_reg_mem_ptr_phys = 0u;
        }
        /* LDRA_JUSTIFY_END */
    }
}
#endif
/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxDmpacDofFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxDmpacDofObj *dof_obj = (tivxDmpacDofObj *)appData;
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from the caller.
    Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
    This behaviour is part of the application design. An error print statement can be added in a future release if required.
    <justification end> */
    if (NULL != dof_obj)
    /* LDRA_JUSTIFY_END */
    {
        (void)tivxEventPost(dof_obj->waitForProcessCmpl);
    }

    return FVID2_SOK;
}

/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach the error portion. function covered in negative coverage build
Effect on this unit: If control reaches here, the code base is expected to call the imaging callback function to return an error status to the application.
<justification end> */
void tivxDmpacDofErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxDmpacDofObj *dof_obj = (tivxDmpacDofObj *)appData;

    if (NULL != dof_obj)
    {
        dof_obj->err_stat = errEvents;
    }
}
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch statement <metric end>
<justification start>
Rationale: The test framework and test apps cannot reach the error portion. function covered in negative coverage build
Effect on this unit: If control reaches here, the code base is expected to call the imaging callback function to return an error status to the application.
<justification end> */
static void tivxDmpacDofWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxDmpacDofObj *dof_obj = (tivxDmpacDofObj *)appData;
    if (NULL != dof_obj)
    {
        dof_obj->wdTimerErrStatus = dof_obj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents;

        if(0u != dof_obj->wdTimerErrStatus)
        {
            (void)tivxEventPost(dof_obj->waitForProcessCmpl);
        }
    }
}

/* LDRA_JUSTIFY_END */

#if !defined(SOC_J722S)
/* Callback for config register and golden register memory comparison */
int32_t tivxDmpacDofConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mDofConfigRegMemParams *dof_config_reg_prms = (Vhwa_M2mDofConfigRegMemParams *)configRegPrms;
    tivxDmpacDofObj *dof_obj = NULL;

    int32_t status = FVID2_SOK;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != dof_config_reg_prms) && (NULL != handle))
    /* LDRA_JUSTIFY_END */
    {
        dof_obj = (tivxDmpacDofObj *)dof_config_reg_prms->appData;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are expected to be pre-validated from the caller.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != dof_config_reg_prms) && (NULL != handle) && (NULL != dof_obj))
    /* LDRA_JUSTIFY_END */
    {
        /* Compare the golden register memory and readback register memory using dof_config_reg_prms */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        The memory pointers are expected to be pre-allocated and validated.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((NULL != dof_config_reg_prms->golden_ptr) &&
            (NULL != dof_config_reg_prms->readback_ptr) &&
            (dof_config_reg_prms->readback_mem_size > 0U))
        /* LDRA_JUSTIFY_END */
        {
            int32_t cmp_result = memcmp(
            dof_config_reg_prms->golden_ptr,
            dof_config_reg_prms->readback_ptr,
            dof_config_reg_prms->readback_mem_size);

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
                VX_PRINT(VX_ZONE_ERROR, "DOF config register mismatch detected!\n");
                status = FVID2_EFAIL;
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                VX_PRINT(VX_ZONE_INFO, "DOF config register match successful.\n");
            }
        }
    }
    else
    {
        status = FVID2_EFAIL;
    }

    return status;
}
#endif
BUILD_ASSERT(((VHWA_DOF_RD_ERR == TIVX_DMPAC_DOF_RD_ERR)? 1U : 0U));
BUILD_ASSERT(((VHWA_DOF_WR_ERR == TIVX_DMPAC_DOF_WR_ERR)? 1U : 0U));
BUILD_ASSERT(((VHWA_DOF_MP0_RD_STATUS_ERR == TIVX_DMPAC_DOF_MP0_RD_STATUS_ERR)? 1U : 0U));
BUILD_ASSERT(((VHWA_DOF_FOCO0_SL2_WR_ERR == TIVX_DMPAC_DOF_FOCO0_SL2_WR_ERR)? 1U : 0U));
BUILD_ASSERT(((VHWA_DOF_FOCO0_VBUSM_RD_ERR == TIVX_DMPAC_DOF_FOCO0_VBUSM_RD_ERR)? 1U : 0U));
BUILD_ASSERT((sizeof(uint32_t) == sizeof(((tivx_dmpac_dof_psa_timestamp_data_t *)0)->psa_values))? 1U : 0U);
