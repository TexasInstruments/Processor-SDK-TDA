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
#include "TI/hwa_vpac_msc.h"
#include "tivx_hwa_kernels.h"
#include "tivx_kernel_vpac_msc.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_hwa_vpac_msc_priv.h"
#include "TI/tivx_event.h"
#include "TI/tivx_mutex.h"

#include "vhwa/include/vhwa_m2mMsc.h"

#include "utils/perf_stats/include/app_perf_stats.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* There are two target kernels supported by this implemenation,
 * one is MSC Pyramid node
 * two is OpenVX Gaussian Pyramid node
 */
#define TIVX_VPAC_MSC_NUM_INST                  (VHWA_M2M_MSC_MAX_INST * 2u)

#define TIVX_VPAC_MSC_PMD_START_IDX             (0u)
#define TIVX_VPAC_MSC_G_PMG_START_IDX           (2u)
#define TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE        (4096u)
/* #define TEST_MSC_PERFORMANCE_LOGGING */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct tivxVpacMscPmdInstObj_t tivxVpacMscPmdInstObj;

typedef struct
{
    /*!< Index of the input from which this pyramid is generated
     *   For the 0th Octave, it will be generated from the source image.
     *   This is used only if the multiple octaves are required
     *   This typically points to the last output from the
     *   previeus pyramid set. */
    uint32_t                input_idx;
    /*!< Index from which the Pyramid level starts in the vx_pyramid object */
    uint32_t                out_start_idx;
    /*!< Number of pyramid levels within this pyramid set */
    uint32_t                num_levels;
    /*!< This maps output index to scaler index */
    uint32_t                sc_map_idx[MSC_MAX_OUTPUT];
} tivxVpacMscPmdSubSetInfo;

typedef struct
{
    /*! Flag to indicate if it msc object is allocated or not */
    uint32_t                 isAlloc;

    /*! MSC Driver Create Arguments */
    Vhwa_M2mMscCreatePrms    createArgs;
    /*! MSC Driver parameters, locally storing it for each subset pyramid */
    Vhwa_M2mMscParams        msc_prms[TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO];
    /*! MSC Driver Coefficients */
    Msc_Coeff                coeffCfg;
    /*! Driver Handle */
    Fvid2_Handle             handle;
    /*! Mutext to wait for completion */
    tivx_event               wait_for_compl;

    /*! Input Fvid2 Frame List */
    Fvid2_FrameList          inFrmList;
    /*! Input Fvid2 Frame List */
    Fvid2_FrameList          outFrmList;
    /*! Input Fvid2 Frame */
    Fvid2_Frame              inFrm;
    /*! List of FVID2 Output Frames */
    Fvid2_Frame              outFrm[MSC_MAX_OUTPUT];
    /*! FVID2 Callback parameters */
    Fvid2_CbParams           cbPrms;

    /*! Locally storing pointer to instance object for easy access. */
    tivxVpacMscPmdInstObj   *inst_obj;

    /*! Input image descriptor, locally storing for easy access. */
    tivx_obj_desc_image_t   *in_img_desc;
    /*! Output image descriptor, locally storing for easy access. */
    tivx_obj_desc_image_t   *out_img_desc[TIVX_PYRAMID_MAX_LEVEL_OBJECTS];


    /*! Pyramid Subset information, it stores information like
     *  number of levels in this subset, input start index, output
     *  start index etc.. */
    tivxVpacMscPmdSubSetInfo ss_info[TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO];

    /*! Number of pyramid subsets, essentially valid entries
     *  in ss_info array */
    uint32_t                 num_pmd_subsets;

    /*! Total number of pyramid levels, locally storing for easy access. */
    uint32_t                 num_pmd_levels;
    /*! yc_mode, 0: Luma only mode, 1: Chroma Only mode. Locally storing for easy access.*/
    uint32_t                 yc_mode;
    /*max down scaling factor. Locally storing for easy access.*/
    uint32_t                max_ds_factor;
    /* Structure for error event parameters
     * Used to register callback for the given set of events.*/
    Msc_ErrEventParams      errEvtPrms;
    /* Stores the status of error interrupt */
    uint32_t                err_stat;
    /* Structure for watchdog timer error event parameters
     * Used to register callback for the given set of events.*/
    Msc_WdTimerErrEventParams      wdTimerErrEvtPrms;
    /* Stores the status of watchdog timer error interrupt */
    uint32_t                       wdTimerErrStatus;
    /* Store a 32-bit bitmask used to specify which error events
     * should be reported */
    uint32_t                enable_error_events;
    /*! Store the psa_sign values received from the hardware.
     *
     * Each psa_sign is an array of uint32_t which contains the PSA Signature
     * values for each pyramid subset */
    uint32_t                psaSign[TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS][TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO];
    /*! Store the time stamp of the last psa_sign received from the hardware */
    uint64_t                timestamp;

    /* Physical address for register readback memory */
    uint64_t                           readback_mem_ptr_phys;

    /* Virtual address for register readback memory */
    tivx_shared_mem_ptr_t              readback_mem_ptr_virt;

    /* Physical address for golden register memory */
    uint64_t                           golden_reg_mem_ptr_phys;

    /* Virtual address for golden register memory */
    tivx_shared_mem_ptr_t              golden_reg_mem_ptr_virt;

    /* Size of the configuration register memory */
    uint32_t                           config_reg_mem_size;
} tivxVpacMscPmdObj;


struct tivxVpacMscPmdInstObj_t
{
    /*! Mutex protecting msc objects allocation and free */
    tivx_mutex              lock;

    /*! MSC Objects */
    tivxVpacMscPmdObj       msc_obj[VHWA_M2M_MSC_MAX_HANDLES];

    /*! Locally storing target kernel, this is used to identify
     *  the target kernel instance */
    tivx_target_kernel      target_kernel;

    /*! Flag to indicate whether to allocate scaler from top ie from
     *  instance 0 or from bottom instance ie instance 9.
     *  Instance0 of the target kernels allocates scaler in forward direction,
     *  starting from scaler0, whereas Instance1 of the target kernels
     *  allocate in backward direction from scaler9. */
    uint32_t                alloc_sc_fwd_dir;

    /*! Instance ID of the MSC driver */
    uint32_t                msc_drv_inst_id;

    /*! HWA Performance ID */
    app_perf_hwa_id_t       hwa_perf_id;
} ;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* OPENVX Callback functions */
static vx_status VX_CALLBACK tivxVpacMscPmdProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscPmdCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscPmdDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxVpacMscPmdControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);

/* Local Functions */
static tivxVpacMscPmdObj *tivxVpacMscPmdAllocObject(tivxVpacMscPmdInstObj *instObj);
static void tivxVpacMscPmdFreeObject(tivxVpacMscPmdInstObj *instObj,
    tivxVpacMscPmdObj *msc_obj);
static void tivxVpacMscPmdSetScParams(Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivx_obj_desc_image_t *out_img_desc,
    uint32_t level,
    tivx_target_kernel_instance kernel, tivxVpacMscPmdObj *msc_obj, uint32_t subset_num);
static void tivxVpacMscPmdSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc);
static vx_status tivxVpacMscPmdCalcSubSetInfo(tivxVpacMscPmdObj *msc_obj, tivx_target_kernel_instance kernel);
static vx_status tivxVpacMscPmdSetMscParams(tivxVpacMscPmdObj *msc_obj,
    tivxVpacMscPmdSubSetInfo *ss_info, uint32_t num_oct,
    tivx_target_kernel_instance kernel);
static void tivxVpacMscPmdCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    const tivx_vpac_msc_output_params_t *out_prms);

/* Control Command Implementation */
static vx_status tivxVpacMscPmdSetCoeffsCmd(tivxVpacMscPmdObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscPmdSetInputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj, tivx_target_kernel_instance kernel);
static vx_status tivxVpacMscPmdSetOutputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[]);
static vx_status tivxVpacMscPmdGetErrStatusCmd(const tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc);
static vx_status tivxVpacMscPmdGetPsaStatusCmd(const tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscPmdEnableErrorEventsCmd(tivxVpacMscPmdObj *msc_obj,
                    tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxVpacMscPmdMapVhwaToTivxErrEvents(uint32_t vhwa_err_events,
    uint32_t wd_timer_err, uint32_t *tivx_err_events);
static vx_status tivxVpacMscPmdMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
    uint32_t *vhwa_err_events, uint32_t *wd_timer_err);

/* Driver Callback */
int32_t tivxVpacMscPmdFrameComplCb(Fvid2_Handle handle, void *appData);
static void tivxVpacMscPmdErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData);
static void tivxVpacMscPmdWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData);

#if !defined(VPAC3L)
/* Config register readback callback */
int32_t tivxVpacMscPmdConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms);

/* Buffer allocation/free functions */
static vx_status tivxVpacMscPmdAllocReadbackBuffers(tivxVpacMscPmdObj *mscObj);
static void tivxVpacMscPmdFreeReadbackBuffers(tivxVpacMscPmdObj *mscObj);

static vx_status tivxEnableVpacMscSafetyMechanisms(
    tivxVpacMscPmdObj *mscObj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

tivxVpacMscPmdInstObj gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_NUM_INST];

static int32_t gmsc_32_phase_gaussian_filter[] =
{
    #include "../host/msc_32_phase_gaussian_filter.txt"
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void tivxAddTargetKernelVpacMscGaussianPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    char                    target_name[TIVX_TARGET_MAX_NAME];
    vx_enum                 self_cpu;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_G_PMG_START_IDX;
    self_cpu = tivxGetSelfCpuId();

    /* Reset all values to 0 */
    memset(&gTivxVpacMscPmdInstObj[inst_start], 0x0,
        sizeof(tivxVpacMscPmdInstObj) * VHWA_M2M_MSC_MAX_INST);

    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        if(status != (vx_status)VX_SUCCESS)
        {
            break;
        }
        inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

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
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                However, due to the stated rationale, this is not tested.
            <justification end> */
            else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
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
            <metric start> statement branch <metric end>
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
            inst_obj->target_kernel = tivxAddTargetKernel(
                                (vx_enum)VX_KERNEL_GAUSSIAN_PYRAMID,
                                target_name,
                                tivxVpacMscPmdProcess,
                                tivxVpacMscPmdCreate,
                                tivxVpacMscPmdDelete,
                                tivxVpacMscPmdControl,
                                NULL);
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != inst_obj->target_kernel)
            /* LDRA_JUSTIFY_END */ 
            {
                /* Allocate lock semaphore */
                status = tivxMutexCreate(&inst_obj->lock);
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
                    (void)tivxRemoveTargetKernel(inst_obj->target_kernel);
                    inst_obj->target_kernel = NULL;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to create Semaphore\n");
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
                        <metric start> statement branch <metric end>
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
            inst_obj = &gTivxVpacMscPmdInstObj[cnt];
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
Rationale: The component level test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelVpacMscGaussianPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_G_PMG_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

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


void tivxAddTargetKernelVpacMscPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    char                    target_name[TIVX_TARGET_MAX_NAME];
    vx_enum                 self_cpu;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_PMD_START_IDX;
    self_cpu = tivxGetSelfCpuId();

    /* Reset all values to 0 */
    memset(&gTivxVpacMscPmdInstObj[inst_start], 0x0,
        sizeof(tivxVpacMscPmdInstObj) * VHWA_M2M_MSC_MAX_INST);

    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        if(status != (vx_status)VX_SUCCESS)
        {
            break;
        }
        inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

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
            <metric start> statement branch <metric end>
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

        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = tivxAddTargetKernelByName(
                            TIVX_KERNEL_VPAC_MSC_PYRAMID_NAME,
                            target_name,
                            tivxVpacMscPmdProcess,
                            tivxVpacMscPmdCreate,
                            tivxVpacMscPmdDelete,
                            tivxVpacMscPmdControl,
                            NULL);
            /* LDRA_JUSTIFY_START
            <metric start>  branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != inst_obj->target_kernel)
            /* LDRA_JUSTIFY_END */

            {
                /* Allocate lock semaphore */
                status = tivxMutexCreate(&inst_obj->lock);

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
                    (void)tivxRemoveTargetKernel(inst_obj->target_kernel);
                    inst_obj->target_kernel = NULL;
                    VX_PRINT(VX_ZONE_ERROR, "Failed to create Semaphore\n");
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
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case is out of scope for the imaging test framework.
                        Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        else if ((vx_enum)TIVX_CPU_ID_VPAC2 == self_cpu)
                        {
                            inst_obj->msc_drv_inst_id = VHWA_M2M_VPAC_1_MSC_DRV_INST_ID_1;
                            inst_obj->hwa_perf_id     = APP_PERF_HWA_VPAC2_MSC1;
                        }
                        /* LDRA_JUSTIFY_END */

                        #endif
                        else
                        {
                            /* Unsupported CPU ID */
                            VX_PRINT(VX_ZONE_ERROR, "Unsupported CPU ID\n");
                            status = (vx_status)VX_FAILURE;
                        }
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
            inst_obj = &gTivxVpacMscPmdInstObj[cnt];
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
Rationale: The component level test framework and test applications cannot reach this portion.
This failure case is out of scope for the imaging test framework.
Effect on this unit: The unit is NOT expected to result in an error because the branch statement is pre-validated by the application.
This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelVpacMscPyramid(void)
{
    vx_status               status = (vx_status)VX_SUCCESS;
    uint32_t                cnt;
    uint32_t                inst_start;
    tivxVpacMscPmdInstObj  *inst_obj;

    inst_start = TIVX_VPAC_MSC_PMD_START_IDX;
    for (cnt = 0u; cnt < VHWA_M2M_MSC_MAX_INST; cnt ++)
    {
        inst_obj = &gTivxVpacMscPmdInstObj[inst_start + cnt];

        status = tivxRemoveTargetKernel(inst_obj->target_kernel);
        if ((vx_status)VX_SUCCESS == status)
        {
            inst_obj->target_kernel = NULL;
        }

        if (NULL != inst_obj->lock)
        {
            (void)tivxMutexDelete(&inst_obj->lock);
        }
    }
}
/* LDRA_JUSTIFY_END */           

/* ========================================================================== */
/*                              OPENVX Callbacks                              */
/* ========================================================================== */

static vx_status VX_CALLBACK tivxVpacMscPmdCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    int32_t                  fvid2_status = FVID2_SOK;
    vx_uint32                cnt;
    tivxVpacMscPmdObj       *msc_obj = NULL;
    tivx_obj_desc_image_t   *in_img_desc = NULL;
    tivx_obj_desc_pyramid_t *out_pmd_desc = NULL;
    tivxVpacMscPmdInstObj   *inst_obj = NULL;
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
    if ((TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS != num_params) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
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
        target_kernel = tivxTargetKernelInstanceGetKernel(kernel);
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
            No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            for (cnt = 0u; cnt < TIVX_VPAC_MSC_NUM_INST; cnt ++)
            /* LDRA_JUSTIFY_END */
            {
                if (target_kernel == gTivxVpacMscPmdInstObj[cnt].target_kernel)
                {
                    inst_obj = &gTivxVpacMscPmdInstObj[cnt];
                    break;
                }
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
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
        msc_obj = tivxVpacMscPmdAllocObject(inst_obj);
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
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
            out_pmd_desc = (tivx_obj_desc_pyramid_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

            /* Initialize Msc object */
            msc_obj->inst_obj = inst_obj;
            msc_obj->in_img_desc = in_img_desc;
            msc_obj->num_pmd_levels = out_pmd_desc->num_levels;
            msc_obj->max_ds_factor = TIVX_VPAC_MSC_DEFAULT_MAX_DS_FACTOR;
            msc_obj->enable_error_events = 0u;
            msc_obj->yc_mode = TIVX_VPAC_MSC_MODE_LUMA_ONLY;

            /* Get the Image Descriptors from the Pyramid Object */
            tivxGetObjDescList(out_pmd_desc->obj_desc_id,
                (tivx_obj_desc_t **)msc_obj->out_img_desc,
                out_pmd_desc->num_levels);

            for (cnt = 0U; cnt < out_pmd_desc->num_levels; cnt ++)
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL == msc_obj->out_img_desc[cnt])
                {
                    VX_PRINT(VX_ZONE_ERROR, "Null Output Descriptor\n");
                    status = (vx_status)VX_FAILURE;
                    break;
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
                /* Based on input and number of output images,
                 * create and initialize msc driver parametes */
                status = tivxVpacMscPmdCalcSubSetInfo(msc_obj, kernel);
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
            VX_PRINT(VX_ZONE_ERROR, "Failed to allocate Handle Object, increase VHWA_M2M_MSC_MAX_HANDLES macro in PDK driver\n");
            status = (vx_status)VX_ERROR_NO_RESOURCES;
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
        Vhwa_M2mMscCreatePrmsInit(&msc_obj->createArgs);

        status = tivxEventCreate(&msc_obj->wait_for_compl);
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
            msc_obj->cbPrms.cbFxn   = tivxVpacMscPmdFrameComplCb;
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
            status = (vx_status)VX_ERROR_NO_RESOURCES;
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
        (VHWA_MSC_VBUSM_RD_ERR | VHWA_MSC_SL2_WR_ERR);
        msc_obj->errEvtPrms.cbFxn     = tivxVpacMscPmdErrorCb;
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
        msc_obj->wdTimerErrEvtPrms.cbFxn     = tivxVpacMscPmdWdTimerErrorCb;
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

    /* Setting coefficients and calling IOCTL */
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
        Msc_Coeff  *coeffCfg;
        int32_t  single_phase[TIVX_VPAC_MSC_MAX_SP_COEFF_SET][TIVX_VPAC_MSC_MAX_TAP];

        coeffCfg = &msc_obj->coeffCfg;

        Msc_coeffInit(coeffCfg);

        cnt = 0u;
        single_phase[0][cnt] = 0;
        cnt++;
        single_phase[0][cnt] = 0;
        cnt++;
        single_phase[0][cnt] = 256;
        cnt++;
        single_phase[0][cnt] = 0;
        cnt++;
        single_phase[0][cnt] = 0;
        cnt = 0u;
        single_phase[1][cnt] = 16;
        cnt++;
        single_phase[1][cnt] = 64;
        cnt++;
        single_phase[1][cnt] = 96;
        cnt++;
        single_phase[1][cnt] = 64;
        cnt++;
        single_phase[1][cnt] = 16;

        for (cnt = 0u; cnt < MSC_MAX_SP_COEFF_SET; cnt ++)
        {
            coeffCfg->spCoeffSet[cnt] = &single_phase[cnt][0u];
        }

        /* Coefficients for Gaussian filter */
        for (cnt = 0u; cnt < MSC_MAX_MP_COEFF_SET; cnt ++)
        {
            coeffCfg->mpCoeffSet[cnt] = &gmsc_32_phase_gaussian_filter[0];
        }

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
        /* Calling Set_Params for all Octaves, to verify the parameters
         * for all octaves */
        for (cnt = 0U; cnt < msc_obj->num_pmd_subsets; cnt ++)
        {
            fvid2_status = Fvid2_control(msc_obj->handle,
                VHWA_M2M_IOCTL_MSC_SET_PARAMS,
                &msc_obj->msc_prms[cnt], NULL);
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
                break;
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
            status = (vx_status)VX_SUCCESS;

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
            sizeof(tivxVpacMscPmdObj));
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

            tivxVpacMscPmdFreeObject(inst_obj, msc_obj);
        }
    }
    /* LDRA_JUSTIFY_END */

    return status;
}

static vx_status VX_CALLBACK tivxVpacMscPmdDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status       status = (vx_status)VX_SUCCESS;
    uint32_t        size;
    tivxVpacMscPmdObj *msc_obj = NULL;
    tivxVpacMscPmdInstObj *inst_obj = NULL;
    /* LDRA_JUSTIFY_START
    <metric start>  statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameter is expected to be pre-validated from a software layer above imaging.
    Therefore, this failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS != num_params) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
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
        if ((((vx_status)VX_SUCCESS == status) && (NULL != msc_obj)) &&
            (sizeof(tivxVpacMscPmdObj) == size))
        /* LDRA_JUSTIFY_END */ 
        {
            inst_obj = msc_obj->inst_obj;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The negative test framework and test applications cannot reach this portion.
            Handle is expected to be non-null as it is pre-validated in the code flow.
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
            This failure case is out of scope for the imaging test framework.
            Effect on this unit: If the control reaches here, our code base is expected to perform clean-up operations.
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
                tivxVpacMscPmdFreeReadbackBuffers(msc_obj);
            }
#endif
            tivxVpacMscPmdFreeObject(inst_obj, msc_obj);
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
        VX_PRINT(VX_ZONE_ERROR, "Invalid Descriptor\n");
    }
    /* LDRA_JUSTIFY_END */


    return status;
}

static vx_status VX_CALLBACK tivxVpacMscPmdProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status                 status = (vx_status)VX_SUCCESS;
    int32_t                   fvid2_status = FVID2_SOK;
    uint32_t                  size;
    uint32_t                  out_cnt;
    uint32_t                  plane_cnt;
    uint32_t                  in_idx, sc_idx, out_img_idx;
    uint32_t                  oct_cnt;
    Fvid2_Frame              *frm = NULL;
    tivx_obj_desc_image_t    *in_img_desc;
    tivx_obj_desc_image_t    *img_desc;
    tivx_obj_desc_pyramid_t  *out_pmd_desc;
    tivxVpacMscPmdObj        *msc_obj = NULL;
    Fvid2_FrameList          *inFrmList;
    Fvid2_FrameList          *outFrmList;
    tivxVpacMscPmdSubSetInfo *ss_info;
    uint64_t                 cur_time;
    tivxVpacMscPmdInstObj    *inst_obj = NULL;
    Vhwa_M2mMscPsaSign       psa;
    uint32_t                psa_store_idx = 0u;
    tivx_obj_desc_t         *out_pmd_base_desc;
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
    if ((TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PARAMS != num_params) ||
        ((NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX]) ||
        (NULL == obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX])))
    {
        VX_PRINT(VX_ZONE_ERROR, "NULL Params check failed\n");
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
        <metric start>  branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
        Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((((vx_status)VX_SUCCESS == status) && (NULL != msc_obj)) &&
            (sizeof(tivxVpacMscPmdObj) == size))
            /* LDRA_JUSTIFY_END */
        {
            in_img_desc = (tivx_obj_desc_image_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_IN_IMG_IDX];
            out_pmd_desc = (tivx_obj_desc_pyramid_t *)
                obj_desc[TIVX_KERNEL_VPAC_MSC_PYRAMID_OUT_PMD_IDX];

            out_pmd_base_desc = (tivx_obj_desc_t *)out_pmd_desc;
            msc_obj->timestamp = out_pmd_base_desc->timestamp;
            /* Get the Image Descriptors from the Pyramid Object */
            tivxGetObjDescList(out_pmd_desc->obj_desc_id,
                (tivx_obj_desc_t **)msc_obj->out_img_desc,
                out_pmd_desc->num_levels);

            for (out_cnt = 0U; out_cnt < out_pmd_desc->num_levels; out_cnt ++)
            {
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start> 
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, the code base is expected to accumulate and return the error.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL == msc_obj->out_img_desc[out_cnt])
                {
                    VX_PRINT(VX_ZONE_ERROR, "Null Descriptor for Output\n");
                    status = (vx_status)VX_FAILURE;
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
            VX_PRINT(VX_ZONE_ERROR, "Invalid Target Instance Context\n");
            status = (vx_status)VX_ERROR_INVALID_NODE;
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
        inFrmList = &msc_obj->inFrmList;
        outFrmList = &msc_obj->outFrmList;
        inst_obj = msc_obj->inst_obj;

        /* Clearing error status from previous frame
        *  User should consume the error status before
        *  submitting next frame for processing
        */
        msc_obj->wdTimerErrStatus = 0u;
        msc_obj->err_stat = 0u;

        /* Number of input frames is fixed to 1 */
        inFrmList->numFrames = 1U;

        cur_time = tivxPlatformGetTimeInUsecs();

        /*!
         * Determines the index of the psa_sign array where the PSA values will be stored.
         * If the alloc_sc_fwd_dir is 1 (i.e. MSC0) then the PSA values will be stored from the
         * beginning of the psa_sign array else the PSA values will be stored from the end
         * of the psa_sign array.
         */
        if(inst_obj->alloc_sc_fwd_dir == 1u)
        {
            psa_store_idx = 0u;
        }
        else
        {
            psa_store_idx = MSC_MAX_OUTPUT - 1u;
        }

        for (oct_cnt = 0u; oct_cnt < msc_obj->num_pmd_subsets; oct_cnt ++)
        {
            if(status != (vx_status)VX_SUCCESS)
            {
                break;
            }
            ss_info = &msc_obj->ss_info[oct_cnt];

            /* MSC Parameters requires to be set in the driver,
             * if the number of octaves are more than 1.
             * If it is only one, params are already set in the driver as
             * part of create, so no need to set again. */
            if (1u < msc_obj->num_pmd_subsets)
            {
                fvid2_status = Fvid2_control(msc_obj->handle,
                    VHWA_M2M_IOCTL_MSC_SET_PARAMS,
                    &msc_obj->msc_prms[oct_cnt], NULL);
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
                    VX_PRINT(VX_ZONE_ERROR, "Failed to set params\n");
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
            if(status == (vx_status)VX_SUCCESS)
            /* LDRA_JUSTIFY_END */
            {
                frm = &msc_obj->inFrm;
                /* For the first octave, input is from the actual input image */
                if (0u == oct_cnt)
                {
                    /* When input is NV12,  buffer pointer should point to Y plane incase of luma output and 
                     * UV plane incase of chroma output. */
                    if((vx_df_image)VX_DF_IMAGE_NV12 ==in_img_desc->format)
                    {
                        if(msc_obj->msc_prms[oct_cnt].inFmt.dataFormat == FVID2_DF_LUMA_ONLY)
                        {
                            frm->addr[0] = tivxMemShared2PhysPtr(
                                in_img_desc->mem_ptr[0].shared_ptr,
                                (int32_t)in_img_desc->mem_ptr[0].mem_heap_region);
                        }
                        else if(msc_obj->msc_prms[oct_cnt].inFmt.dataFormat == FVID2_DF_CHROMA_ONLY)
                        {
                            frm->addr[0] = tivxMemShared2PhysPtr(
                                in_img_desc->mem_ptr[1].shared_ptr,
                                (int32_t)in_img_desc->mem_ptr[1].mem_heap_region);
                        }
                        else
                        {
                            for (plane_cnt = 0u; plane_cnt < in_img_desc->planes;
                                plane_cnt ++)
                            {
                                frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                                    in_img_desc->mem_ptr[plane_cnt].shared_ptr,
                                    (int32_t)in_img_desc->mem_ptr[plane_cnt].mem_heap_region);
                            }
                        }
                    }
                    else
                    {
                        for (plane_cnt = 0u; plane_cnt < in_img_desc->planes;
                                plane_cnt ++)
                        {
                            frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                                in_img_desc->mem_ptr[plane_cnt].shared_ptr,
                                (int32_t)in_img_desc->mem_ptr[plane_cnt].mem_heap_region);
                        }
                    }
                }
                else
                {
                    /* For the rest octaves, Use the last output
                     * from the previous octave, as an input */
                    in_idx = ss_info->input_idx;
                    img_desc = msc_obj->out_img_desc[in_idx];
                    for (plane_cnt = 0u; plane_cnt < img_desc->planes;
                            plane_cnt ++)
                    {
                        frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                            img_desc->mem_ptr[plane_cnt].shared_ptr,
                            (int32_t)img_desc->mem_ptr[plane_cnt].mem_heap_region);
                    }
                }

                outFrmList->numFrames = MSC_MAX_OUTPUT;
                out_img_idx = ss_info->out_start_idx;
                for (out_cnt = 0u; out_cnt < ss_info->num_levels; out_cnt ++)
                {
                    img_desc = msc_obj->out_img_desc[out_img_idx];
                    sc_idx = ss_info->sc_map_idx[out_cnt];
                    frm = &msc_obj->outFrm[sc_idx];

                    for (plane_cnt = 0u; plane_cnt < img_desc->planes;
                            plane_cnt ++)
                    {
                        frm->addr[plane_cnt] = tivxMemShared2PhysPtr(
                            img_desc->mem_ptr[plane_cnt].shared_ptr,
                            (int32_t)img_desc->mem_ptr[plane_cnt].
                            mem_heap_region);
                    }
                    out_img_idx ++;
                }

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
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(status == (vx_status)VX_SUCCESS)
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
                        if(msc_obj->msc_prms[oct_cnt].enablePsa == 1u)
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
                                    "tivxVpacMscPmdGetPsa: Fvid2_control Failed\n");
                                status = (vx_status)VX_FAILURE;
                            }
                            /* LDRA_JUSTIFY_END */
                            for (out_cnt = 0u; out_cnt < ss_info->num_levels; out_cnt ++)
                            {
                                if(msc_obj->inst_obj->alloc_sc_fwd_dir == 1u)
                                {
                                    msc_obj->psaSign[0][psa_store_idx] = psa.psaSign[0][out_cnt];
                                    msc_obj->psaSign[1][psa_store_idx] = psa.psaSign[1][out_cnt];
                                    psa_store_idx ++;
                                }
                                else
                                {
                                    msc_obj->psaSign[0][psa_store_idx] = psa.psaSign[0][9U-out_cnt];
                                    msc_obj->psaSign[1][psa_store_idx] = psa.psaSign[1][9U-out_cnt];
                                    psa_store_idx --;
                                }
                            }
                        }
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
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
            }
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
        #if defined(TEST_MSC_PERFORMANCE_LOGGING)
        VX_PRINT(VX_ZONE_INFO, "timestamp value of process function is: %llu\n", cur_time);
        #endif

        if ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format)
        {
            if(msc_obj->msc_prms[0].inFmt.dataFormat == FVID2_DF_LUMA_ONLY)
            {
                size = in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y;
            }
            else if(msc_obj->msc_prms[0].inFmt.dataFormat == FVID2_DF_CHROMA_ONLY)
            {
                size = in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y/2U;
            }
            else
            {
                 size = (in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y) + \
                   ((in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y)/2U);
            }
        }
        else
        {
            size = in_img_desc->imagepatch_addr[0].dim_x*in_img_desc->imagepatch_addr[0].dim_y;
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

    return (status);
}

static vx_status VX_CALLBACK tivxVpacMscPmdControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    vx_status            status = (vx_status)VX_SUCCESS;
    uint32_t             size;
    tivxVpacMscPmdObj *msc_obj = NULL;

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
    else if ((NULL == msc_obj) ||
        (sizeof(tivxVpacMscPmdObj) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Size\n");
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
            case TIVX_VPAC_MSC_CMD_SET_COEFF:
            {
                status = tivxVpacMscPmdSetCoeffsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_INPUT_PARAMS:
            {
                status = tivxVpacMscPmdSetInputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U], kernel);
                break;
            }
            case TIVX_VPAC_MSC_CMD_SET_OUTPUT_PARAMS:
            {
                status = tivxVpacMscPmdSetOutputParamsCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t **)&obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_GET_ERR_STATUS:
            {
                status = tivxVpacMscPmdGetErrStatusCmd(msc_obj,
                    (tivx_obj_desc_scalar_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_GET_PSA_STATUS:
            {
                status = tivxVpacMscPmdGetPsaStatusCmd(msc_obj,
                    (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                break;
            }
            case TIVX_VPAC_MSC_CMD_ENABLE_ERROR_EVENTS:
            {
                status = tivxVpacMscPmdEnableErrorEventsCmd(msc_obj,
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

    return (status);
}

/* ========================================================================== */
/*                          Local Functions                                   */
/* ========================================================================== */

static tivxVpacMscPmdObj *tivxVpacMscPmdAllocObject(
    tivxVpacMscPmdInstObj *instObj)
{
    uint32_t        cnt;
    tivxVpacMscPmdObj *msc_obj = NULL;
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
        <justification start> 
        Rationale:
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
                memset(msc_obj, 0x0, sizeof(tivxVpacMscPmdObj));
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

static void tivxVpacMscPmdFreeObject(tivxVpacMscPmdInstObj *instObj,
    tivxVpacMscPmdObj *msc_obj)
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
        <justification start> 
        Rationale:
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

static void tivxVpacMscPmdSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc)
{
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the host kernel before the control reaches here.
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
            case (vx_df_image)VX_DF_IMAGE_YUYV:
            {
                fmt->dataFormat = FVID2_DF_YUV422I_YUYV;
                fmt->ccsFormat = FVID2_CCSF_BITS8_PACKED;
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

        fmt->width      = img_desc->imagepatch_addr[0].dim_x;
        fmt->height     = img_desc->imagepatch_addr[0].dim_y;
        fmt->pitch[0]   = (uint32_t)img_desc->imagepatch_addr[0].stride_y;
        fmt->pitch[1]   = (uint32_t)img_desc->imagepatch_addr[1].stride_y;
    }
}

static void tivxVpacMscPmdSetScParams(Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivx_obj_desc_image_t *out_img_desc,
    uint32_t level,
    tivx_target_kernel_instance kernel, tivxVpacMscPmdObj *msc_obj, uint32_t subset_num)
{
    tivx_target_kernel       target_kernel = NULL;

    target_kernel = tivxTargetKernelInstanceGetKernel(kernel);
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    The parameters are pre-validated by the host kernel before the control reaches here.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != in_img_desc) && (NULL != out_img_desc))
    /* LDRA_JUSTIFY_END */ 

    {
        vx_float32 temp;
        sc_cfg->enable = UTRUE;
        sc_cfg->filtMode = MSC_FILTER_MODE_SINGLE_PHASE;

        sc_cfg->outWidth = out_img_desc->imagepatch_addr[0].dim_x;
        sc_cfg->outHeight = out_img_desc->imagepatch_addr[0].dim_y;
        sc_cfg->inRoi.cropStartX = 0u;
        sc_cfg->inRoi.cropStartY = 0u;
        sc_cfg->inRoi.cropWidth = in_img_desc->imagepatch_addr[0].dim_x;
        sc_cfg->inRoi.cropHeight = in_img_desc->imagepatch_addr[0].dim_y;
        if ((vx_df_image)VX_DF_IMAGE_NV12 ==in_img_desc->format)
        {
            if((vx_df_image)VX_DF_IMAGE_U8 == out_img_desc->format)
            {
                if(TIVX_VPAC_MSC_MODE_CHROMA_ONLY == msc_obj->yc_mode)
                {
                    /* For 1st subset if input is NV12 and output is chroma only then, cropHeight and cropStartY will be half */
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    This failure case is out of scope for the imaging test framework.
                    Effect on this unit: The unit is NOT expected to result in an error.
                    This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(subset_num==0U)
                    /* LDRA_JUSTIFY_END */ 

                    {
                        sc_cfg->inRoi.cropHeight = in_img_desc->imagepatch_addr[0].dim_y>>1;
                        sc_cfg->inRoi.cropStartY = sc_cfg->inRoi.cropStartY>>1;
                    }
                }
            }
        }
        temp = (((((vx_float32)sc_cfg->inRoi.cropWidth/(vx_float32)sc_cfg->outWidth) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        sc_cfg->horzAccInit = (uint32_t)temp;
        /* The valid range for horzAccInit is 0-4095. If it exceeds this range, it will wrap around accordingly. */
        if(sc_cfg->horzAccInit > (TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE - 1U))
        {
            sc_cfg->horzAccInit = sc_cfg->horzAccInit % TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE;
        }
        temp = (((((vx_float32)sc_cfg->inRoi.cropHeight/(vx_float32)sc_cfg->outHeight) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        sc_cfg->vertAccInit = (uint32_t)temp;
        /* The valid range for vertAccInit is 0-4095. If it exceeds this range, it will wrap around accordingly. */
        if(sc_cfg->vertAccInit > (TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE - 1U))
        {
            sc_cfg->vertAccInit = sc_cfg->vertAccInit % TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE;
        }
        sc_cfg->hsSpCoeffSel = 1u;
        sc_cfg->vsSpCoeffSel = 1u;

        /* Note: in the case that it is using a Gaussian pyramid, select the first set of coefficients for first level */
        if ( (0U == level) &&
               ((gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_G_PMG_START_IDX].target_kernel == target_kernel) ||
                (gTivxVpacMscPmdInstObj[TIVX_VPAC_MSC_G_PMG_START_IDX+1U].target_kernel == target_kernel)) )
        {
            sc_cfg->hsSpCoeffSel = 0u;
            sc_cfg->vsSpCoeffSel = 0u;
        }
        else
        {
            if(!((sc_cfg->outWidth == sc_cfg->inRoi.cropWidth) ||
                ((sc_cfg->outWidth*2u) == sc_cfg->inRoi.cropWidth) ||
                ((sc_cfg->outWidth*4u) == sc_cfg->inRoi.cropWidth)) )
            {
                sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;
                sc_cfg->phaseMode = MSC_PHASE_MODE_32PHASE;
                sc_cfg->hsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            }

            if(!((sc_cfg->outHeight == sc_cfg->inRoi.cropHeight) ||
                ((sc_cfg->outHeight*2u) == sc_cfg->inRoi.cropHeight) ||
                ((sc_cfg->outHeight*4u) == sc_cfg->inRoi.cropHeight)) )
            {
                sc_cfg->filtMode = MSC_FILTER_MODE_MULTI_PHASE;
                sc_cfg->phaseMode = MSC_PHASE_MODE_32PHASE;
                sc_cfg->vsMpCoeffSel = MSC_MULTI_32PHASE_COEFF_SET_0;
            }
        }
    }
}

static vx_status tivxVpacMscPmdCalcSubSetInfo(tivxVpacMscPmdObj *msc_obj, tivx_target_kernel_instance kernel)
{
    vx_status                   status = (vx_status)VX_SUCCESS;
    uint32_t                    cnt;
    uint32_t                    num_pmd_levels;
    uint32_t                    num_subsets;
    tivx_obj_desc_image_t      *in_img_desc;
    tivx_obj_desc_image_t      *out_img_desc;
    tivxVpacMscPmdSubSetInfo   *ss_info;
    uint32_t                    max_ds_factor;
    /*! If input is NV12 and output is chroma only, then input height should be divided by half 
     *  from input image descriptor. div_factor stores that information*/
    uint32_t                    div_factor = TIVX_VPAC_MSC_INIT_DIV_FACTOR;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != msc_obj)
    /* LDRA_JUSTIFY_END */ 

    {
        max_ds_factor = msc_obj->max_ds_factor;

        num_subsets = 0U;
        in_img_desc = msc_obj->in_img_desc;
        out_img_desc = msc_obj->out_img_desc[0u];
        num_pmd_levels = msc_obj->num_pmd_levels;

        ss_info = &msc_obj->ss_info[0U];

        if ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format)
        {
            if((vx_df_image)VX_DF_IMAGE_U8 == out_img_desc->format)
            {
                if(TIVX_VPAC_MSC_MODE_CHROMA_ONLY == msc_obj->yc_mode)
                {
                    div_factor = 2u;
                }
                else
                {
                    div_factor = 1u;
                }
            }
        }
        /* Atleast, for the first level,
         * the scaling factor cannot be less than 1/max_ds_factor */
        if (((in_img_desc->imagepatch_addr[0u].dim_x /
                max_ds_factor) >
                out_img_desc->imagepatch_addr[0u].dim_x) ||
            (((in_img_desc->imagepatch_addr[0u].dim_y /div_factor) /
                max_ds_factor) >
                out_img_desc->imagepatch_addr[0u].dim_y))
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Scaling Factor\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        else
        {
            ss_info->input_idx = 0u;
            ss_info->out_start_idx = 0u;
            ss_info->num_levels = 0u;

            /* Atleast, one subset is required */
            num_subsets ++;

            for (cnt = 0u; cnt < num_pmd_levels; cnt ++)
            {
                if(status != (vx_status)VX_SUCCESS)
                {
                    break;
                }
                out_img_desc = msc_obj->out_img_desc[cnt];

                if ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format)
                {
                    if((vx_df_image)VX_DF_IMAGE_U8 == out_img_desc->format)
                    {
                        if(TIVX_VPAC_MSC_MODE_CHROMA_ONLY == msc_obj->yc_mode)
                        {
                            div_factor = 2u;
                        }
                        else
                        {
                            div_factor = 1u;
                        }
                    }
                }
                /* if input is U8 and output also U8, then no need to half the input height from input image descriptor*/
                else if ((vx_df_image)VX_DF_IMAGE_U8 == in_img_desc->format)
                {
                    div_factor = 1u;
                }
                else
                {
                    /* Unsupported format */
                }
                /* Need to change pyramid subset,
                 * if input to output ratio is more than max_ds_factor
                 */
                if (((in_img_desc->imagepatch_addr[0].dim_x /
                        max_ds_factor) >
                        out_img_desc->imagepatch_addr[0].dim_x) ||
                    (((in_img_desc->imagepatch_addr[0].dim_y /div_factor) /
                        max_ds_factor) >
                        out_img_desc->imagepatch_addr[0].dim_y))
                {
                    /* Get the next pyramid subset */
                    ss_info = &msc_obj->ss_info[num_subsets];

                    /* Input image for the this pyramid subset is the
                     * last output from previous pyramid subset */
                    in_img_desc = msc_obj->out_img_desc[cnt - 1u];

                    /* Initialize input and output indices */
                    ss_info->input_idx = cnt - 1u;
                    ss_info->out_start_idx = cnt;

                    /* Atleast, this level is required for
                     * this pyramid subset */
                    ss_info->num_levels = 1u;

                    num_subsets ++;
                    if (TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO
                        < num_subsets)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Pyramid Subsets required are more than TIVX_KERNEL_VPAC_MSC_PYRAMID_MAX_PMD_INFO\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
                else
                {
                    ss_info->num_levels ++;
                }
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(status == (vx_status)VX_SUCCESS)
                /* LDRA_JUSTIFY_END */

                {
                    if (MSC_MAX_OUTPUT < ss_info->num_levels)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Max 10 outputs supported in subset\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                    }
                }
            }
        }

        msc_obj->num_pmd_subsets = num_subsets;
    }
    /* LDRA_JUSTIFY_START
    <metric start>  branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    This failure case is out of scope for the imaging test framework.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (((vx_status)VX_SUCCESS == status) && (NULL != msc_obj))
    /* LDRA_JUSTIFY_END */
    {
        /* Now, set the scaler information for each pyramid subset */
        for (cnt = 0u; cnt < msc_obj->num_pmd_subsets; cnt ++)
        {
            status = tivxVpacMscPmdSetMscParams(msc_obj, &msc_obj->ss_info[cnt], cnt, kernel);
        }
    }

    return (status);
}

static vx_status tivxVpacMscPmdSetMscParams(tivxVpacMscPmdObj *msc_obj,
    tivxVpacMscPmdSubSetInfo *ss_info, uint32_t num_oct,
    tivx_target_kernel_instance kernel)
{
    vx_status                status = (vx_status)VX_SUCCESS;
    uint32_t                 out_cnt;
    uint32_t                 idx;
    uint32_t                 out_start_idx;
    Vhwa_M2mMscParams       *msc_prms = NULL;
    tivx_obj_desc_image_t   *in_img_desc;
    tivx_obj_desc_image_t   *img_desc;

    msc_prms = &msc_obj->msc_prms[num_oct];
    out_start_idx = ss_info->out_start_idx;

    if (0U == num_oct)
    {
        in_img_desc = msc_obj->in_img_desc;
    }
    else
    {
        in_img_desc = msc_obj->out_img_desc[ss_info->input_idx];
    }

    img_desc = msc_obj->out_img_desc[out_start_idx];

    /* Initialize MSC Parameters with the default configuration */
    Vhwa_m2mMscParamsInit(msc_prms);

    /* Set the input format */
    tivxVpacMscPmdSetFmt(&msc_prms->inFmt, in_img_desc);

    if ((((vx_df_image)VX_DF_IMAGE_NV12 != img_desc->format) &&
        ((vx_df_image)VX_DF_IMAGE_U8 != img_desc->format)) &&
        (((vx_df_image)VX_DF_IMAGE_UYVY != in_img_desc->format) &&
        ((vx_df_image)VX_DF_IMAGE_YUYV != in_img_desc->format)))
    {
        /* Luma-only mode when the input format is NV12,
         * but the output format is not NV12 or U8.
         * Also, luma-only mode if the input format is not YUV422I (UYVY or YUYV) */
        msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
    }

    /* set input format to chroma only when input is NV12, output is U8 and yc_mode is 1*/
    if ((vx_df_image)VX_DF_IMAGE_NV12 ==in_img_desc->format)
    {
        if((vx_df_image)VX_DF_IMAGE_U8 == img_desc->format)
        {
            if(TIVX_VPAC_MSC_MODE_LUMA_ONLY == msc_obj->yc_mode)
            {
                msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
            }
            else
            {
                msc_prms->inFmt.dataFormat = FVID2_DF_CHROMA_ONLY;
                /* LDRA_JUSTIFY_START
                <metric start>  branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot reach this portion.
                This failure case is out of scope for the imaging test framework.
                Effect on this unit: The unit is NOT expected to result in an error.
                This behaviour is part of the application design. However, due to the stated rationale, this is not tested.
                <justification end> */
                if(num_oct == 0U)
                /* LDRA_JUSTIFY_END */

                {
                    /* For 1st subset if input is NV12 and output is chroma only ,then input height will be half */
                    msc_prms->inFmt.height = in_img_desc->imagepatch_addr[0].dim_y>>1;
                }
            }
        }
    }
    else if((vx_df_image)VX_DF_IMAGE_U8 == in_img_desc->format)
    {
        if(TIVX_VPAC_MSC_MODE_LUMA_ONLY == msc_obj->yc_mode)
        {
            msc_prms->inFmt.dataFormat = FVID2_DF_LUMA_ONLY;
        }
        else
        {
            msc_prms->inFmt.dataFormat = FVID2_DF_CHROMA_ONLY;
        }
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this portion.
    Unsupported input formats are rejected during OpenVX graph verification stage before process execution.
    The kernel validator function ensures only supported formats reach this code.
    This defensive else block is architecturally unreachable due to framework format validation.
    Effect on this unit: If the control reaches here, this is an unsupported configuration that would be caught earlier by framework validation.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        /* Unsupported format */
    }
    /* LDRA_JUSTIFY_END */

    for (out_cnt = 0u; out_cnt < ss_info->num_levels; out_cnt ++)
    {
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start> 
        Rationale: The component level negative test framework and test applications CAN reach this portion.
        No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format) ||
            ((vx_df_image)VX_DF_IMAGE_YUYV == in_img_desc->format)) && (out_cnt > 3u)))
        {
            VX_PRINT(VX_ZONE_ERROR, "At most four YUYV or UYVY outputs can generate with 10 scalars\n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }
        /* LDRA_JUSTIFY_END */
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
            if (1U == msc_obj->inst_obj->alloc_sc_fwd_dir)
            {
                #if defined(VPAC3) || defined(VPAC3L)
                    if (((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == in_img_desc->format))
                    {
                        msc_prms->isEnableSimulProcessing = 1u;
                        if(out_cnt == 0u)
                        {
                            idx = out_cnt;
                        }
                        else
                        {
                            idx += 2u;
                        }
                    }
                    else
                    {
                        idx = out_cnt;
                    }
                #else
                    idx = out_cnt;
                #endif
            }
            else
            {
                #if defined(VPAC3) || defined(VPAC3L)
                    if (((vx_df_image)VX_DF_IMAGE_UYVY == in_img_desc->format) ||
                        ((vx_df_image)VX_DF_IMAGE_YUYV == in_img_desc->format))
                    {
                        msc_prms->isEnableSimulProcessing = 1u;
                        if(out_cnt == 0u)
                        {
                            idx = MSC_MAX_OUTPUT - 4U - out_cnt;
                        }
                        else
                        {
                            idx -= 2u;
                        }
                    }
                    else
                    {
                        idx = MSC_MAX_OUTPUT - 1U - out_cnt;
                    }
                #else
                    idx = MSC_MAX_OUTPUT - 1U - out_cnt;
                #endif
            }

            ss_info->sc_map_idx[out_cnt] = idx;

            tivxVpacMscPmdSetScParams(&msc_prms->mscCfg.scCfg[idx],
                in_img_desc, msc_obj->out_img_desc[out_start_idx], out_start_idx, kernel, msc_obj, num_oct);

            tivxVpacMscPmdSetFmt(&msc_prms->outFmt[idx],
                msc_obj->out_img_desc[out_start_idx]);

            if ((vx_df_image)VX_DF_IMAGE_NV12 == in_img_desc->format)
            {
                if((vx_df_image)VX_DF_IMAGE_U8 == msc_obj->out_img_desc[out_start_idx]->format)
                {
                    if(TIVX_VPAC_MSC_MODE_LUMA_ONLY == msc_obj->yc_mode)
                    {
                        msc_prms->outFmt[idx].dataFormat = FVID2_DF_LUMA_ONLY;
                    }
                    else
                    {
                        msc_prms->outFmt[idx].dataFormat = FVID2_DF_CHROMA_ONLY;
                    }
                }
            }
            else if((vx_df_image)VX_DF_IMAGE_U8 == in_img_desc->format)
            {
                if(TIVX_VPAC_MSC_MODE_LUMA_ONLY == msc_obj->yc_mode)
                {
                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_LUMA_ONLY;
                }
                else
                {
                    msc_prms->outFmt[idx].dataFormat = FVID2_DF_CHROMA_ONLY;
                }
            }
            else
            {
                /* Unsupported format */
            }
        }
        out_start_idx ++;
    }
    return status;
}

static void tivxVpacMscPmdCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    const tivx_vpac_msc_output_params_t *out_prms)
{
    vx_float32 temp;
    sc_cfg->isSignedData = out_prms->signed_data;

    sc_cfg->coeffShift = out_prms->coef_shift;
    sc_cfg->isEnableFiltSatMode = out_prms->saturation_mode;

    sc_cfg->inRoi.cropStartX = out_prms->offset_x;
    sc_cfg->inRoi.cropStartY = out_prms->offset_y;

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

    sc_cfg->horzAccInit = out_prms->multi_phase.init_phase_x;
    sc_cfg->vertAccInit = out_prms->multi_phase.init_phase_y;
    if(out_prms->offset_x == TIVX_VPAC_MSC_AUTOCOMPUTE)
    {
        sc_cfg->inRoi.cropStartX = 0u;
    }
    if(out_prms->offset_y == TIVX_VPAC_MSC_AUTOCOMPUTE)
    {
        sc_cfg->inRoi.cropStartY = 0u;
    }
    if(out_prms->multi_phase.init_phase_x == TIVX_VPAC_MSC_AUTOCOMPUTE)
    {
        temp = (((((vx_float32)sc_cfg->inRoi.cropWidth/(vx_float32)sc_cfg->outWidth) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        sc_cfg->horzAccInit = (uint32_t)temp;
        if(sc_cfg->horzAccInit > (TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE - 1U))
        {
            sc_cfg->horzAccInit = sc_cfg->horzAccInit % TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE;
        }
    }
    if(out_prms->multi_phase.init_phase_y == TIVX_VPAC_MSC_AUTOCOMPUTE)
    {
        temp = (((((vx_float32)sc_cfg->inRoi.cropHeight/(vx_float32)sc_cfg->outHeight) * 0.5f) - 0.5f) * 4096.0f) + 0.5f;
        sc_cfg->vertAccInit = (uint32_t)temp;
        if(sc_cfg->vertAccInit > (TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE - 1U))
        {
            sc_cfg->vertAccInit = sc_cfg->vertAccInit % TIVX_VPAC_MSC_MAX_ACC_INIT_VALUE;
        }
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
static vx_status tivxVpacMscPmdMapVhwaToTivxErrEvents(uint32_t vhwa_err_events, uint32_t wd_timer_err, uint32_t *tivx_err_events)
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
    else if ((0u != (vhwa_err_events & (~(VHWA_MSC_VBUSM_RD_ERR | VHWA_MSC_SL2_WR_ERR)))) ||
             (0u != (wd_timer_err & (~(VHWA_MSC0_WDTIMER_ERR | VHWA_MSC1_WDTIMER_ERR)))))
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
 * \return VX_SUCCESS on success, VX_FAILURE if null pointer,
 *         VX_ERROR_INVALID_PARAMETERS if invalid error event mask
 */
static vx_status tivxVpacMscPmdMapTivxToVhwaErrEvents(uint32_t tivx_err_events,
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

static vx_status tivxVpacMscPmdSetCoeffsCmd(tivxVpacMscPmdObj *msc_obj,
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

static vx_status tivxVpacMscPmdSetOutputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj[])
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt, ss_idx, sc_idx, out_cnt, level_count;
    tivx_vpac_msc_output_params_t    *out_prms = NULL;
    void                             *target_ptr;
    tivxVpacMscPmdSubSetInfo         *ss_info = NULL;
    Msc_ScConfig                     *sc_cfg = NULL;

    level_count = 0u;
    for (cnt = 0u; cnt < msc_obj->num_pmd_subsets; cnt ++)
    {
        ss_info = &msc_obj->ss_info[cnt];
        for(out_cnt = 0u; out_cnt< ss_info->num_levels; out_cnt++)
        {
            if (NULL != usr_data_obj[level_count])
            {
                target_ptr = tivxMemShared2TargetPtr(&usr_data_obj[level_count]->mem_ptr);

                tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj[level_count]->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

                if (sizeof(tivx_vpac_msc_output_params_t) ==
                        usr_data_obj[level_count]->mem_size)
                {
                    out_prms = (tivx_vpac_msc_output_params_t *)target_ptr;
                
                    ss_idx = cnt;

                    /* Scaler index within subset pyramid can be calculated
                    * by just modulo operation. */
                    sc_idx = level_count % ss_info->num_levels;

                    /* Now map scaler index to scaler config index */
                    sc_idx = ss_info->sc_map_idx[sc_idx];

                    sc_cfg = &msc_obj->msc_prms[ss_idx].mscCfg.scCfg[sc_idx];

                    tivxVpacMscPmdCopyOutPrmsToScCfg(sc_cfg, out_prms);
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Invalid Mem Size for Output Params\n");
                    status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                }

                tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj[level_count]->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Null User Data Object\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }


            if ((vx_status)VX_SUCCESS != status)
            {
                break;
            }
            level_count++;
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

static vx_status tivxVpacMscPmdSetInputParamsCmd(tivxVpacMscPmdObj *msc_obj,
    const tivx_obj_desc_user_data_object_t *usr_data_obj, tivx_target_kernel_instance kernel)
{
    vx_status                         status = (vx_status)VX_SUCCESS;
    uint32_t                          cnt;
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

            msc_obj->yc_mode = in_prms->yc_mode;
            if(1U < in_prms->yc_mode)
            {
                VX_PRINT(VX_ZONE_ERROR, "yc_mode value should be either 0 or 1\n");
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
            }
            msc_obj->max_ds_factor = in_prms->max_ds_factor;
            msc_obj->enable_error_events = in_prms->enable_error_events;
            for (cnt = 0u; cnt < msc_obj->num_pmd_subsets; cnt ++)
            {
                msc_prms = &msc_obj->msc_prms[cnt];
                msc_prms->enablePsa = in_prms->enable_psa;

                msc_prms->enableLineSkip = in_prms->src_ln_inc_2;
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
                        msc_prms->mscCfg.tapSel = MSC_TAP_SEL_3TAPS;
                        break;
                    case 4:
                        msc_prms->mscCfg.tapSel = MSC_TAP_SEL_4TAPS;
                        break;
                    /* LDRA_JUSTIFY_END */
                    case 5:
                        msc_prms->mscCfg.tapSel = MSC_TAP_SEL_5TAPS;
                        break;
                    default:
                        VX_PRINT(VX_ZONE_ERROR, "Invalid Kernel Size\n");
                        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                        break;
                }
            }
            /* Recalculate subset information for different values of max_ds_factor and yc_mode */
            tivxCheckStatus(&status, tivxVpacMscPmdCalcSubSetInfo(msc_obj, kernel));
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

static vx_status tivxVpacMscPmdGetErrStatusCmd(const tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_scalar_t *scalar_obj_desc)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    uint32_t                            combined_err_stat;

    if (NULL != scalar_obj_desc)
    {
        /* Use the mapping function to convert VHWA error flags to TIVX error flags */
        status = tivxVpacMscPmdMapVhwaToTivxErrEvents(msc_obj->err_stat, msc_obj->wdTimerErrStatus, &combined_err_stat);
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

static vx_status tivxVpacMscPmdGetPsaStatusCmd(const tivxVpacMscPmdObj *msc_obj,
    tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                           status = (vx_status)VX_SUCCESS;
    void                                *target_ptr;
    tivx_vpac_msc_psa_timestamp_data_t  *psa_status_ptr;

    if (NULL != usr_data_obj)
    {
        if (sizeof(tivx_vpac_msc_psa_timestamp_data_t) ==
                usr_data_obj->mem_size)
        {
            /* Map the user data object to access the memory */
            target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

            tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
                (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

            psa_status_ptr = (tivx_vpac_msc_psa_timestamp_data_t *)target_ptr;

            /* Copy driver data (PSA values) into the structure */
            tivx_obj_desc_memcpy(&psa_status_ptr->psa_values, (void *)&msc_obj->psaSign, sizeof(msc_obj->psaSign));

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

    return (status);
}

static vx_status tivxVpacMscPmdEnableErrorEventsCmd(tivxVpacMscPmdObj *msc_obj,
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
            VX_PRINT(VX_ZONE_ERROR, "tivxVpacMscPmdEnableErrorEventsCmd: Incorrect Data Object Size \n");
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
                    "tivxVpacMscPmdEnableErrorEventsCmd: tivxMemBufferMap Failed \n");
            }
            /* LDRA_JUSTIFY_END */
            else
            {
                params_ptr = (tivx_vpac_msc_input_params_t *)target_ptr;
                enableErrorEventsMask = params_ptr->enable_error_events;
                /* Update WDTimer configuration from user parameters */
                msc_obj->wdTimerErrEvtPrms.wdTimeoutCycles = params_ptr->wdtimer_timeout_cycles;
                /* Use the mapping function to convert TIVX error flags to VHWA error flags */
                status = tivxVpacMscPmdMapTivxToVhwaErrEvents(enableErrorEventsMask, &intr_en, &wd_timer_intr_en);

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
                        "tivxVpacMscPmdEnableErrorEventsCmd: Invalid error event mask or null pointer\n");
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
                        VX_PRINT(VX_ZONE_ERROR, "tivxVpacMscPmdEnableErrorEventsCmd: Fvid2_control Failed \n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }

                tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
                    (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));
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
        VX_PRINT(VX_ZONE_ERROR, "tivxVpacMscPmdEnableErrorEventsCmd: User data object is NULL\n");
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    return status;
}
#if !defined(VPAC3L)
/* MSC: Enable safety mechanism function */
static vx_status tivxEnableVpacMscSafetyMechanisms(
    tivxVpacMscPmdObj *mscObj,
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
            Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                if (sizeof(tivx_vpac_msc_safety_mechanism_params_t) == usr_data_obj->mem_size)
                {
                    tivx_vpac_msc_safety_mechanism_params_t *safety_params =
                        (tivx_vpac_msc_safety_mechanism_params_t *)params_ptr;

                    /* Enable or disable reconfigReg based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_reconfig_and_reinit_Reg;
                    fvid2_status = Fvid2_control(mscObj->handle, VHWA_M2M_IOCTL_MSC_ENABLE_RECONFIG_REINIT_REG, &enable, NULL);
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
                        VX_PRINT(VX_ZONE_ERROR, "Failed to set reconfigReg\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */

                    /* Enable or disable status register validation based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    enable = (uint32_t)safety_params->enable_status_reg_validation;
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

                    /* Enable or disable config register readback based on input parameter */
                    /* Pass mode directly: 0=disabled, 1=one-shot, 2=continuous */
                    if (0U != safety_params->enable_readback_config_registers)
                    {
                        uint32_t readback_size = 0U;

                        /* Fetch the readback size from the low level driver */
                        if(((vx_status)VX_SUCCESS == status))
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
                            if ((FVID2_SOK == fvid2_status) && (readback_size > 0U))
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
                                status = tivxVpacMscPmdAllocReadbackBuffers(mscObj);
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
                            fvid2_status = Fvid2_control(mscObj->handle, VHWA_M2M_IOCTL_MSC_ENABLE_CONFIG_REG_READBACK, &enable, NULL);
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test applications cannot reach this portion.
                            This condition checks if buffers are already allocated to avoid double allocation.
                            Effect on this unit: If the control reaches here, allocation is skipped to prevent memory leak.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if (FVID2_SOK != fvid2_status)
                            {
                                VX_PRINT(VX_ZONE_ERROR, "Failed to set config register validation\n");
                                tivxVpacMscPmdFreeReadbackBuffers(mscObj);
                                status = (vx_status)VX_FAILURE;
                            }
                            /* LDRA_JUSTIFY_END */
                        }
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications cannot reach this portion.
                        This failure case requires buffer allocation failure that cannot be simulated in test environment.
                        Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
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
                        This condition checks if buffers are already allocated to avoid double allocation.
                        Effect on this unit: If the control reaches here, allocation is skipped to prevent memory leak.
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

static vx_status tivxVpacMscPmdAllocReadbackBuffers(tivxVpacMscPmdObj *mscObj)
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
    if (NULL == mscObj)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null pointer passed to tivxVpacMscPmdAllocReadbackBuffers\n");
        status = (vx_status)VX_FAILURE;
    }
    else if (NULL == mscObj->handle)
    {
        VX_PRINT(VX_ZONE_ERROR, "Null handle in tivxVpacMscPmdAllocReadbackBuffers\n");
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
        if((vx_status)VX_SUCCESS == alloc_status)
        /* LDRA_JUSTIFY_END */
        {
            Vhwa_M2mMscConfigRegMemParams reg_ptrs;

            reg_ptrs.golden_ptr   = (void *)(uintptr_t)mscObj->golden_reg_mem_ptr_virt.shared_ptr;
            reg_ptrs.readback_ptr = (void *)(uintptr_t)mscObj->readback_mem_ptr_virt.shared_ptr;

            /* Register the callback for config register memory comparison */
            reg_ptrs.cbFxn = tivxVpacMscPmdConfigRegMemCompareCb;
            reg_ptrs.appData = (void *)mscObj;

            int32_t fvid2_status = Fvid2_control(
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
                tivxVpacMscPmdFreeReadbackBuffers(mscObj);
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
            tivxVpacMscPmdFreeReadbackBuffers(mscObj);
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static void tivxVpacMscPmdFreeReadbackBuffers(tivxVpacMscPmdObj *mscObj)
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
#endif
/* ========================================================================== */
/*                              Driver Callbacks                              */
/* ========================================================================== */

int32_t tivxVpacMscPmdFrameComplCb(Fvid2_Handle handle, void *appData)
{
    tivxVpacMscPmdObj *msc_obj = (tivxVpacMscPmdObj *)appData;
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
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacMscPmdErrorCb(Fvid2_Handle handle, uint32_t errEvents, void *appData)
{
    tivxVpacMscPmdObj *msc_obj = (tivxVpacMscPmdObj *)appData;

    if (NULL != msc_obj)
    {
        msc_obj->err_stat = errEvents;
    }
}
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start>
Rationale: This is covered in a test framework of type negative build since this requires altering the source code macros to trigger the API.
Effect on this unit: If control reaches here, the code base is expected to prevent undefined behaviour by avoiding dereferencing a NULL pointer.
However, due to the stated rationale, this is not tested.
<justification end> */
static void tivxVpacMscPmdWdTimerErrorCb(Fvid2_Handle handle, uint32_t wdTimerErrEvents, void *appData)
{
    tivxVpacMscPmdObj *msc_obj = (tivxVpacMscPmdObj *)appData;
    
    if (NULL != msc_obj)
    {
        msc_obj->wdTimerErrStatus = (msc_obj->wdTimerErrEvtPrms.wdTimerErrEvents & wdTimerErrEvents);

        if(0u != msc_obj->wdTimerErrStatus)
        {
            (void)tivxEventPost(msc_obj->wait_for_compl);
        }
    }
}
/* LDRA_JUSTIFY_END */

#if !defined(VPAC3L)
int32_t tivxVpacMscPmdConfigRegMemCompareCb(Fvid2_Handle handle, void *configRegPrms)
{
    Vhwa_M2mMscConfigRegMemParams *msc_config_reg_prms = (Vhwa_M2mMscConfigRegMemParams *)configRegPrms;
    tivxVpacMscPmdObj *msc_obj = NULL;

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
        msc_obj = (tivxVpacMscPmdObj *)msc_config_reg_prms->appData;
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications CAN reach this portion.
    No existing test case can reach this portion. A test case will be added to cover this gap in a future release.
    Effect on this unit: If the control reaches here, our code base is expected to accumulate the error status and return the same to the application.
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
        Rationale: The component level negative test framework and test applications cannot reach this portion.
        This failure case is out of scope for the imaging test framework.
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
    <metric start> branch <metric end>
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
BUILD_ASSERT((sizeof(Vhwa_M2mMscPsaSign) == sizeof(((tivx_vpac_msc_psa_timestamp_data_t *)0)->psa_values))? 1U : 0U);


