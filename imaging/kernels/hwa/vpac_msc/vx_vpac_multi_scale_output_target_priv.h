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

#ifndef VX_VPAC_MSC_MULTI_SCALE_TARGET_FVID2_PRIV_H_
#define VX_VPAC_MSC_MULTI_SCALE_TARGET_FVID2_PRIV_H_

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

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* #define TEST_MSC_PERFORMANCE_LOGGING */
#define TIVX_VPAC_MSC_NUM_INST                  (VHWA_M2M_MSC_MAX_INST * 2u)

#define TIVX_VPAC_MSC_M_SCALE_START_IDX             (0u)
#define TIVX_VPAC_MSC_M_SCALE2_START_IDX            (2u)

/*********************************
 *      MSC MULTISCALE STRUCTURES
 *********************************/

typedef struct tivxVpacMscScaleInstObj_t tivxVpacMscScaleInstObj;
typedef struct
{
    uint32_t                isAlloc;
    Vhwa_M2mMscCreatePrms   createArgs;
    Vhwa_M2mMscParams       msc_prms;
    Fvid2_Handle            handle;
    tivx_event              wait_for_compl;
    uint32_t                num_output;
    Fvid2_FrameList         inFrmList;
    Fvid2_FrameList         outFrmList;
    Fvid2_Frame             inFrm;
    Fvid2_Frame             outFrm[MSC_MAX_OUTPUT];
    Fvid2_CbParams          cbPrms;
    tivxVpacMscScaleInstObj *inst_obj;
    uint32_t                sc_map_idx[MSC_MAX_OUTPUT];
    Msc_Coeff               coeffCfg;
    uint32_t                num_outputs;
    vx_df_image             in_img0_format;
    vx_df_image             in_img1_format;
    vx_df_image             out_img_format[MSC_MAX_OUTPUT];
    uint32_t                in0_height;
    uint32_t                in1_height;
    /*! yc_mode, 0: Luma only mode, 1: Chroma Only mode. Locally storing for easy access.*/
    uint32_t                 yc_mode;
    tivx_obj_desc_image_t   *in_img_desc0;
    tivx_obj_desc_image_t   *in_img_desc1;
    tivx_obj_desc_image_t   *out_img_desc[MSC_MAX_OUTPUT];
    uint32_t                 in_img0_yc_mode;
    uint32_t                 in_img1_yc_mode;
    uint32_t                 num_params;
    /* State from user commands to override auto mode or not */
    uint32_t                user_init_phase_x[MSC_MAX_OUTPUT];
    uint32_t                user_init_phase_y[MSC_MAX_OUTPUT];
    uint32_t                user_offset_x[MSC_MAX_OUTPUT];
    uint32_t                user_offset_y[MSC_MAX_OUTPUT];
    uint32_t                user_crop_start_x[MSC_MAX_OUTPUT];
    uint32_t                user_crop_start_y[MSC_MAX_OUTPUT];
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
    /* Timestamp for the most recent frame processed. */
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
} tivxVpacMscScaleObj;

struct tivxVpacMscScaleInstObj_t
{
    tivx_mutex              lock;
    tivxVpacMscScaleObj     msc_obj[VHWA_M2M_MSC_MAX_HANDLES];
    tivx_target_kernel      target_kernel;
    uint32_t                alloc_sc_fwd_dir;
    uint32_t                msc_drv_inst_id;
    /*! HWA Performance ID */
    app_perf_hwa_id_t       hwa_perf_id;
} ;

/*********************************
 *      Function Prototypes
 *********************************/

void tivxVpacMscMultiScaleSetScParams(Msc_ScConfig *sc_cfg,
    const tivx_obj_desc_image_t *in_img_desc,
    const tivx_obj_desc_image_t *out_img_desc, uint32_t do_line_skip);
void tivxVpacMscMultiScaleSetFmt(Fvid2_Format *fmt,
    const tivx_obj_desc_image_t *img_desc, uint32_t do_line_skip);
void tivxVpacMscScaleCopyOutPrmsToScCfg(Msc_ScConfig *sc_cfg,
    const tivx_vpac_msc_output_params_t *out_prms);
void tivxVpacMscScaleUpdateStartPhases(const tivxVpacMscScaleObj *msc_obj,
    Msc_ScConfig *sc_cfg, uint32_t cnt);
void tivxVpacMscScaleSetScParamsForEachLevel(tivxVpacMscScaleObj *msc_obj,
    Vhwa_M2mMscParams *msc_prms);

#ifdef __cplusplus
}
#endif

#endif /* VX_VPAC_MSC_MULTI_SCALE_TARGET_FVID2_PRIV_H_ */

