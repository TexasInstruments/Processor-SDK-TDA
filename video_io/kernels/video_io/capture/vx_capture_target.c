/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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
#include "TI/video_io_capture.h"
#include "TI/tivx_event.h"
#include "tivx_video_io_kernels.h"
#include "tivx_kernel_capture.h"
#include "TI/tivx_target_kernel.h"
#include "tivx_kernels_target_utils.h"
#include "tivx_video_io_capture_priv.h"

#include <TI/tivx_queue.h>
#include <fvid2.h>
#include <csirx.h>
#include <tivx_obj_desc_priv.h>
#include <vx_reference.h>
#include <vx_internal.h>

#if !defined(SOC_J722S) && defined(ASF_TEST_ENABLED_CAPTURE)
#include <ti/csl/csl_esm.h>
#include <ti/csl/soc.h>
#endif

#if defined(LDRA_COVERAGE_ENABLED_VIDEO_IO)
#include "ldra_remote_core_coverage_main.h"
#endif

#define CAPTURE_FRAME_DROP_LEN                          (4096U*4U)

#define CAPTURE_INST_ID_INVALID                         (0xFFFFU)

#define CAPTURE_IN_CSI_DT_INVALID                       (0xFFFFFFFFU)

#define CAPTURE_TIMEOUT_VALID                           (0U)
#define CAPTURE_TIMEOUT_EXCEEDED                        (1U)

#define CAPTURE_MS_TO_US                                (1000U)

static char target_name[][TIVX_TARGET_MAX_NAME] =
{
    TIVX_TARGET_CAPTURE1,
    TIVX_TARGET_CAPTURE2,
    TIVX_TARGET_CAPTURE3,
    TIVX_TARGET_CAPTURE4,
#if !defined(SOC_AM62A) && !defined(SOC_J722S)
    TIVX_TARGET_CAPTURE5,
    TIVX_TARGET_CAPTURE6,
    TIVX_TARGET_CAPTURE7,
    TIVX_TARGET_CAPTURE8,
#if defined(SOC_J784S4) || defined(SOC_J742S2)
    TIVX_TARGET_CAPTURE9,
    TIVX_TARGET_CAPTURE10,
    TIVX_TARGET_CAPTURE11,
    TIVX_TARGET_CAPTURE12,
#endif
#endif
};

#define CAPTURE_NUM_TARGETS                             (sizeof(target_name)/sizeof(target_name[0]))

typedef struct tivxCaptureParams_t tivxCaptureParams;

typedef struct
{
    uint32_t instId;
    /**< Csirx Drv Instance ID. */
    uint8_t numCh;
    /**< Number of channels processed on given CSIRX DRV instance. */
    uint32_t chVcMap[TIVX_CAPTURE_MAX_CH];
    /**< Virtual ID for channels for current capture instance. */
    Fvid2_Handle drvHandle;
    /**< FVID2 capture driver handle. */
    Csirx_CreateParams createPrms;
    /**< Csirx create time parameters */
    Csirx_CreateStatus createStatus;
    /**< Csirx create time status */
    Fvid2_CbParams drvCbPrms;
    /**< Capture callback params */
    uint8_t raw_capture;
    /**< flag indicating raw capture */
    Csirx_InstStatus captStatus;
    /**< CSIRX Capture status. */
    Csirx_DPhyCfg dphyCfg;
    /**< CSIRX DPHY configuration. */
    tivxCaptureParams *captParams;
    /**< Reference to capture node parameters. */
} tivxCaptureInstParams;

struct tivxCaptureParams_t
{
    tivxCaptureInstParams instParams[TIVX_CAPTURE_MAX_INST];
    /**< Capture Instance parameters */
    uint32_t numOfInstUsed;
    /**< Number of CSIRX DRV instances used in current TIOVX Node. */
    uint8_t numCh;
    /**< Number of channels processed on given capture node instance. */
    tivx_obj_desc_t *img_obj_desc[TIVX_CAPTURE_MAX_CH];
    /**< Captured Images */
    uint8_t steady_state_started;
    /**< Flag indicating whether or not steady state has begun. */
    tivx_event  frame_available;
    /**< Following Queues i.e. freeFvid2FrameQ, pendingFrameQ, fvid2_free_q_mem,
     *   fvid2Frames, and pending_frame_free_q_mem are for given instance of the
     *   Node. If Node instance contains more than 1 instances of the CSIRX DRV
     *   instances, then first 'n' channels are for first instance of the driver
     *   then n channels for next driver and so on... */
    /**< Event indicating when a frame is available. */
    tivx_queue freeFvid2FrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal FVID2 queue */
    tivx_queue pendingFrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal pending frame queue */
    tivx_queue pendingObjArrayQ;
    /**< Internal pending obj arr queue */
    tivx_queue pendingFrameTimestampLoQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal queue tracking lower 32 bits of time stamp of pending frames */
    tivx_queue pendingFrameTimestampHiQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal queue tracking upper 32 bits of time stamp of pending frames */
    uintptr_t fvid2_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< FVID2 queue mem */
    Fvid2_Frame fvid2Frames[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< FVID2 frame structs */
    uintptr_t pending_frame_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending frame queue mem */
    uintptr_t pending_obj_arr_q_mem[TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending obj arr queue mem */
    uintptr_t pending_frame_timestamp_lo_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending timestamp lo queue mem */
    uintptr_t pending_frame_timestamp_hi_free_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< pending timestamp hi queue mem */
    uint32_t timeout;
    /**< Total timeout to check for dead camera; taken directly from
     *   tivx_capture_params_t input */
    uint32_t timeoutInitial;
    /**< Initial timeout to check for dead camera; taken directly from
     *   tivx_capture_params_t input */
    uint64_t timeoutRemaining;
    /**< Remaining timeout for dead camera */
    uint8_t activeChannelMask;
    /**< Mask for active channels; bit 0 maps to channel 0, bit N maps to bit N;
     *   1 indicates active, 0 indicates inactive */
    tivx_queue errorFrameQ[TIVX_CAPTURE_MAX_CH];
    /**< Internal error frame queue; contains descriptor ID's of invalid frames */
    uintptr_t error_frame_q_mem[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< error frame queue mem */
    tivx_obj_desc_t *error_obj_desc[TIVX_CAPTURE_MAX_CH][TIVX_CAPTURE_MAX_NUM_BUFS];
    /**< Error Image Object Descriptors; allocated in control callback and points to
     *   memory from descriptor sent from application */
    uint8_t enableErrorFrameTimeout;
    /**< Flag indicating if error frame has been sent and can use error timeout
     *   Error timeout is only used if this error frame is sent */

    /* Make frame list instance specific */
    Fvid2_FrameList frmList;
    tivx_capture_error_per_channel_t local_error_info[TIVX_CAPTURE_ERROR_INFO_NUM_CHANNELS + 1U];
    /**< Pending CSIRX errors detected: indices [0, numCh) are per-channel,
     *   frame-correlated errors; index [numCh] holds errors that cannot be
     *   correlated to a channel or frame (its timestamp is always 0). */
};

static tivx_target_kernel vx_capture_target_kernel[CAPTURE_NUM_TARGETS] = {NULL};

static vx_status captDrvCallback(Fvid2_Handle handle, void *appData);
static void captDrvErrorCallback(Csirx_EventStatus eventStatus, void *appData);
#if !defined(SOC_J722S)
static void captDrvAsfCallback(Csirx_EventStatus eventStatus, void *appData);
#endif
static uint32_t tivxCaptureExtractInCsiDataType(uint32_t format);
static uint32_t tivxCaptureExtractCcsFormat(uint32_t format);
static uint32_t tivxCaptureExtractDataFormat(uint32_t format);
static vx_status tivxCaptureEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *output_desc,
       tivxCaptureParams *prms);
static vx_status tivxCaptureSetCreateParams(
       tivxCaptureParams *prms,
       const tivx_obj_desc_user_data_object_t *obj_desc);
static vx_status VX_CALLBACK tivxCaptureProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCaptureCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCaptureDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status VX_CALLBACK tivxCaptureControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg);
static vx_status tivxCaptureGetStatistics(tivxCaptureParams *prms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj);
static vx_status tivxCaptureAllocErrorDesc(tivxCaptureParams *prms,
    tivx_obj_desc_t *obj_desc);
static void tivxCaptureCopyStatistics(tivxCaptureParams *prms,
    tivx_capture_statistics_t *capt_status_prms);
static void tivxCaptureGetChannelIndices(const tivxCaptureParams *prms,
                                         uint32_t instId,
                                         uint32_t *startChIdx,
                                         uint32_t *endChIdx);
static uint32_t tivxCaptureGetNodeChannelNum(const tivxCaptureParams *prms,
                                             uint32_t instId,
                                             uint32_t chId);
static uint32_t tivxCaptureGetDrvInstIndex(const tivxCaptureParams *prms,
                                           uint32_t instId);
static uint32_t tivxCaptureMapInstId(uint32_t instId);
static void tivxCapturePrintStatus(tivxCaptureInstParams *prms);
static vx_status tivxCaptureStart(tivxCaptureParams *prms);
static void tivxCaptureSetTimeout(tivxCaptureParams *prms);
static vx_status tivxCaptureTimeout(tivxCaptureParams *prms);
static void tivxCaptureGetObjDesc(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        tivx_obj_desc_object_array_t *output_desc,
        uint64_t *timestamp);
static vx_status tivxCaptureDequeueFrameFromDriver(tivxCaptureParams *prms);
static uint32_t tivxCaptureIsAllChFrameAvailable(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        uint8_t timeoutExceeded);

/**
 *******************************************************************************
 *
 * \brief Wrapper function to get time in microseconds
 *
 * This function is a simple wrapper around the tivxPlatformGetTimeInUsecs
 * API and is used in contexts where a function pointer with the signature
 * uint64_t (*)(void *) is required.
 *
 * \param  args   [IN] Argument not used in this implementation.
 *                      It is explicitly cast to void to avoid
 *                      unused-parameter compiler warnings.
 *
 * \return Current time in microseconds as returned by
 *         tivxPlatformGetTimeInUsecs().
 *
 *******************************************************************************
 */     
static uint64_t tivxPlatformGetTimeInUsecsWrapper(void *args)
{
    (void)args;

    return tivxPlatformGetTimeInUsecs();
}   

/**
 *******************************************************************************
 *
 * \brief Callback function from driver to application
 *
 * Callback function gets called from Driver to application on reception of
 * a frame
 *
 * \param  handle       [IN] Driver handle for which callback has come.
 * \param  appData      [IN] Application specific data which is registered
 *                           during the callback registration.
 *
 * \return  SYSTEM_LINK_STATUS_SOK on success
 *
 *******************************************************************************
 */
static vx_status captDrvCallback(Fvid2_Handle handle, void *appData)
{
    (void)handle;
    vx_status status;

    tivxCaptureParams *prms = (tivxCaptureParams*)appData;

    status = tivxEventPost(prms->frame_available);

    return status;
}

/**
*******************************************************************************
*
* \brief Callback for CSIRX error events
*
* Gets called by the CSIRX driver when an enabled error event is detected.
* The detected errors are stored for the corresponding CSIRX instance and
* reported by the Capture node during processing.
*
* \param eventStatus [IN] CSIRX event status containing the detected errors
* \param appData     [IN] Capture parameters (tivxCaptureParams *)
*
*******************************************************************************
*/
static void captDrvErrorCallback(Csirx_EventStatus eventStatus, void *appData)
{
    tivxCaptureParams *prms;
    uint16_t errors = 0U;
    uintptr_t key;

    prms = (tivxCaptureParams *)appData;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach else
    portion.
    This callback is invoked only with the parameters this component supplied when the event was
    registered, which cannot be altered through the video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: If the callback parameters were invalid, the detected errors would not be
    decoded or stored.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((NULL != prms) &&
        (CSIRX_EVENT_GROUP_ERROR == eventStatus.eventGroup))
    /* LDRA_JUSTIFY_END */
    {
        if ((eventStatus.eventMasks &
             CSIRX_EVENT_TYPE_ERR_HEADER_ECC) != 0U)
        {
            errors |= TIVX_CAPTURE_ERR_HEADER_ECC;
        }

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        This error is reported by the capture driver only in the event of a physical CSI-2 link
        level fault, which cannot be stimulated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: errors is OR'd with TIVX_CAPTURE_ERR_PAYLOAD_CRC, which is later
        reported to the application through the Capture node's error-event mechanism.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((eventStatus.eventMasks &
             CSIRX_EVENT_TYPE_ERR_PAYLOAD_CRC) != 0U)
        {
            errors |= TIVX_CAPTURE_ERR_PAYLOAD_CRC;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        This error is reported by the capture driver only in the event of a hardware level data
        overflow, which cannot be stimulated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: errors is OR'd with TIVX_CAPTURE_ERR_DATA_OVERFLOW, which is later
        reported to the application through the Capture node's error-event mechanism.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((eventStatus.eventMasks &
             (CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_FRONT |
              CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM0 |
              CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM1 |
              CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM2 |
              CSIRX_EVENT_TYPE_ERR_FIFO_OVERFLOW_STRM3)) != 0U)
        {
            errors |= TIVX_CAPTURE_ERR_DATA_OVERFLOW;
        }
        /* LDRA_JUSTIFY_END */

        if (0U != errors)
        {
            key = HwiP_disable();

            /* Not correlated to a specific frame or channel: stored in the
             * one reserved slot past the per-channel entries, timestamp left
             * at 0. */
            prms->local_error_info[prms->numCh].error_bitfield |= errors;

            HwiP_restore(key);
        }
    }
}

#if !defined(SOC_J722S)
/**
*******************************************************************************
*
* \brief Callback for CSIRX ASF events
*
* Gets called by the CSIRX driver when ASF events are detected. These faults
* carry no frame, channel, or instance context, so they are stored in the
* one reserved slot past the per-channel entries.
*
* \param eventStatus [IN] CSIRX event status containing the detected errors
* \param appData     [IN] Capture parameters (tivxCaptureParams *)
*
*******************************************************************************
*/
static void captDrvAsfCallback(Csirx_EventStatus eventStatus, void *appData)
{
    tivxCaptureParams *prms;
    uint16_t errors = 0U;
    uintptr_t key;
    uint32_t masks;

    prms = (tivxCaptureParams *)appData;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach else
    portion.
    This callback is invoked only with the parameters this component supplied when the event was
    registered, which cannot be altered through the video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: If prms were NULL, the detected ASF errors would not be decoded or stored.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != prms)
    /* LDRA_JUSTIFY_END */
    {
        masks = eventStatus.eventMasks &
                (CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR |
                 CSIRX_EVENT_TYPE_ASF_CSR_ERR |
                 CSIRX_EVENT_TYPE_ASF_DAP_ERR);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        the false outcome of this branch.
        This branch outcome is determined by the set of error events reported by the capture driver
        and cannot be selected through the video_io component interface.
        Therefore, this case is out of scope for the video_io test framework.
        Effect on this unit: errors is OR'd with TIVX_CAPTURE_ERR_ASF_TRANS_TIMEOUT.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((masks & CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR) != 0U)
        {
            errors |= TIVX_CAPTURE_ERR_ASF_TRANS_TIMEOUT;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        the false outcome of this branch.
        This branch outcome is determined by the set of error events reported by the capture driver
        and cannot be selected through the video_io component interface.
        Therefore, this case is out of scope for the video_io test framework.
        Effect on this unit: errors is OR'd with TIVX_CAPTURE_ERR_ASF_CSR_PARITY.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((masks & CSIRX_EVENT_TYPE_ASF_CSR_ERR) != 0U)
        {
            errors |= TIVX_CAPTURE_ERR_ASF_CSR_PARITY;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        the false outcome of this branch.
        This branch outcome is determined by the set of error events reported by the capture driver
        and cannot be selected through the video_io component interface.
        Therefore, this case is out of scope for the video_io test framework.
        Effect on this unit: errors is OR'd with TIVX_CAPTURE_ERR_ASF_DAP_PARITY.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((masks & CSIRX_EVENT_TYPE_ASF_DAP_ERR) != 0U)
        {
            errors |= TIVX_CAPTURE_ERR_ASF_DAP_PARITY;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        the false outcome of this branch.
        This branch outcome is determined by the set of error events reported by the capture driver
        and cannot be selected through the video_io component interface.
        Therefore, this case is out of scope for the video_io test framework.
        Effect on this unit: The detected errors are stored in the uncorrelated error-info slot.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (0U != errors)
        {
            key = HwiP_disable();

            /* Not correlated to a specific frame or channel: stored in the
             * one reserved slot past the per-channel entries, timestamp left
             * at 0. */
            prms->local_error_info[prms->numCh].error_bitfield |= errors;

            HwiP_restore(key);
        }
        /* LDRA_JUSTIFY_END */
    }
}
#endif

/* Waiting on capture frame available event based on timeout
 *    Log the wait time and subtract from the remaining time
 *    Use remaining time as the wait time */
static vx_status tivxCaptureTimeout(tivxCaptureParams *prms)
{
    vx_status status;
    uint64_t timestamp = 0;

    timestamp = tivxPlatformGetTimeInUsecs();

    status = tivxEventWait(prms->frame_available, (uint32_t)prms->timeoutRemaining);

    /* Calculate time that the tivxEventWait waited */
    timestamp = tivxPlatformGetTimeInUsecs() - timestamp;
    if (1U == prms->enableErrorFrameTimeout)
    {
        /* Rounding up so that the timeout does not get clipped for each subsequent camera */
        if (timestamp > (CAPTURE_MS_TO_US * prms->timeoutRemaining))
        {
            prms->timeoutRemaining = 0u;
        }
        else
        {
            /* Update timeoutRemaining based on amount of time already waited */
            prms->timeoutRemaining = (((CAPTURE_MS_TO_US * prms->timeoutRemaining + CAPTURE_MS_TO_US) - timestamp) / CAPTURE_MS_TO_US);
        }
    }
    
    return status;
}

static vx_status tivxCaptureEnqueueFrameToDriver(
       tivx_obj_desc_object_array_t *output_desc,
       tivxCaptureParams *prms)
{
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint64_t captured_frame;
    uint32_t chId = 0U;
    Fvid2_FrameList *frmList;
    Fvid2_Frame *fvid2Frame;
    uint32_t startChIdx, endChIdx, instIdx;
    tivxCaptureInstParams *instParams;
    uint16_t obj_desc_id;
    uintptr_t temp_fvid2_frame;

    frmList = &prms->frmList;
    tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                       prms->numCh);

    (void)tivxQueuePut(&prms->pendingObjArrayQ, (uintptr_t)output_desc, TIVX_EVENT_TIMEOUT_NO_WAIT);

    /* Prepare and queue frame-list for each instance */
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];
        tivxCaptureGetChannelIndices(prms, instIdx, &startChIdx, &endChIdx);
        frmList->numFrames = 0;

        for (chId = startChIdx ; chId < endChIdx ; chId++)
        {
            /* Only enqueue the frame if it is a valid frame */
            if (tivxFlagIsBitSet(prms->img_obj_desc[chId]->flags, TIVX_REF_FLAG_IS_INVALID) == (vx_bool)vx_false_e)
            {
                if ((uint32_t)TIVX_OBJ_DESC_RAW_IMAGE == prms->img_obj_desc[chId]->type)
                {
                    tivx_obj_desc_raw_image_t *raw_image;

                    raw_image = (tivx_obj_desc_raw_image_t *)prms->img_obj_desc[chId];

                    obj_desc_id = prms->img_obj_desc[chId]->obj_desc_id;

                    captured_frame = tivxMemShared2PhysPtr(
                        (raw_image->mem_ptr[0].shared_ptr + (uint64_t)tivxComputePatchOffset(0, 0, &raw_image->imagepatch_addr[0U])),
                        (vx_enum)TIVX_MEM_EXTERNAL);
                }
                else
                {
                    tivx_obj_desc_image_t *image;
                    image = (tivx_obj_desc_image_t *)prms->img_obj_desc[chId];

                    obj_desc_id = prms->img_obj_desc[chId]->obj_desc_id;

                    captured_frame = tivxMemShared2PhysPtr(
                        (image->mem_ptr[0].shared_ptr + (uint64_t)tivxComputePatchOffset(0, 0, &image->imagepatch_addr[0U])),
                        (vx_enum)TIVX_MEM_EXTERNAL);
                }

                (void)tivxQueueGet(&prms->freeFvid2FrameQ[chId], &temp_fvid2_frame, TIVX_EVENT_TIMEOUT_NO_WAIT);
                fvid2Frame = (Fvid2_Frame *)temp_fvid2_frame;

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach else portion.
                This condition depends on internal state of this component, which cannot be altered
                through the video_io component interface.
                Therefore, this case is out of scope for the video_io test framework.
                Effect on this unit: A NULL entry would prevent the frame from being added to the
                driver frame list and cause an error message to be printed.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != fvid2Frame)
                /* LDRA_JUSTIFY_END */
                {
                    uintptr_t obj_desc_id_u32 = (uintptr_t)obj_desc_id;

                    /* Put into frame list as it is for same driver instance */
                    frmList->frames[frmList->numFrames]           = fvid2Frame;
                    frmList->frames[frmList->numFrames]->chNum    = (chId - startChIdx);
                    frmList->frames[frmList->numFrames]->addr[0U] = captured_frame;
                    frmList->frames[frmList->numFrames]->appData  = (void *)obj_desc_id_u32;
                    frmList->numFrames++;
                }
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                This condition depends on internal state of this component, which cannot be altered
                through the video_io component interface.
                Therefore, this case is out of scope for the video_io test framework.
                Effect on this unit: The frame is not added to the driver frame list and an error
                message is printed.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, " CAPTURE: Could not retrieve buffer from buffer queue!!!\n");
                }
                /* LDRA_JUSTIFY_END */
            }
            else
            {
                (void)tivxQueuePut(&prms->errorFrameQ[chId], (uintptr_t)output_desc->obj_desc_id[chId], TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
        }

        if (frmList->numFrames > 0U)
        {
            fvid2_status = Fvid2_queue(instParams->drvHandle, frmList, 0);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            This failure is reported by the capture driver only in the event of a driver or hardware
            level fault, which cannot be stimulated through the video_io component interface.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: The function returns VX_FAILURE, prints an error message, and stops
            queueing frames for the remaining Capture instances.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Frame could not be queued for frame %d !!!\n", chId);
                break;
            }
            /* LDRA_JUSTIFY_END */
        }
        
    }

    return status;
}

static uint32_t tivxCaptureExtractInCsiDataType(uint32_t format)
{
    uint32_t inCsiDataType;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach the
    uncovered outcomes of this switch.
    This configuration is rejected by the Capture host side validation before this unit is invoked.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: Supported image formats are mapped to the corresponding driver CSI-2 data
    types.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    switch (format)
    {
    /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level test framework and test applications cannot reach this
        portion.
        This configuration is rejected by the Capture host side validation before this unit is
        invoked.
        Therefore, this case is out of scope for the video_io test framework.
        Effect on this unit: If reached, the input CSI data type would be set to
        FVID2_CSI2_DF_RGB888.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        case (vx_df_image)VX_DF_IMAGE_RGB:
            inCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        /* LDRA_JUSTIFY_END */
        case (vx_df_image)VX_DF_IMAGE_RGBX:
        case (vx_df_image)TIVX_DF_IMAGE_BGRX:
            inCsiDataType = FVID2_CSI2_DF_RGB888;
            break;
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (uint32_t)TIVX_RAW_IMAGE_P12_BIT:
            inCsiDataType = FVID2_CSI2_DF_RAW12;
            break;
        case (vx_df_image)VX_DF_IMAGE_UYVY:
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            inCsiDataType = FVID2_CSI2_DF_YUV422_8B;
            break;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level test framework and test applications cannot reach this
        portion.
        This configuration is rejected by the Capture host side validation before this unit is
        invoked.
        Therefore, this case is out of scope for the video_io test framework.
        Effect on this unit: The input CSI data type would be marked invalid, causing
        tivxCaptureCreate() to return VX_FAILURE.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        default:
            inCsiDataType = CAPTURE_IN_CSI_DT_INVALID;
            break;
        /* LDRA_JUSTIFY_END */
    }

    return inCsiDataType;
}

static uint32_t tivxCaptureExtractInCsiDataTypeFromRawImg(tivx_obj_desc_raw_image_t *raw_img)
{
    uint32_t inCsiDataType = CAPTURE_IN_CSI_DT_INVALID;
    tivx_raw_image_create_params_t *params = &raw_img->params;

    if ((uint32_t)TIVX_RAW_IMAGE_16_BIT == params->format[0].pixel_container)
    {
        switch (params->format[0].msb)
        {
            case 9u:
                inCsiDataType = FVID2_CSI2_DF_RAW10;
            break;
            case 11u:
                inCsiDataType = FVID2_CSI2_DF_RAW12;
            break;
            case 13u:
                inCsiDataType = FVID2_CSI2_DF_RAW14;
            break;
            case 15u:
                inCsiDataType = FVID2_CSI2_DF_RAW16;
            break;
            default:
                break;
        }
    }
    else if ((uint32_t)TIVX_RAW_IMAGE_8_BIT == params->format[0].pixel_container)
    {
        switch (params->format[0].msb)
        {
            case 5u:
                inCsiDataType = FVID2_CSI2_DF_RAW6;
            break;
            case 6u:
                inCsiDataType = FVID2_CSI2_DF_RAW7;
            break;
            case 7u:
                inCsiDataType = FVID2_CSI2_DF_RAW8;
            break;
            default:
                break;
        }
    }
    else if ((uint32_t)TIVX_RAW_IMAGE_P12_BIT == params->format[0].pixel_container)
    {
        if (11u == params->format[0].msb)
        {
            inCsiDataType = FVID2_CSI2_DF_RAW12;
        }   
    }
    else
    {
        /* Don Nothing */
    }

    return (inCsiDataType);
}

static uint32_t tivxCaptureExtractCcsFormat(uint32_t format)
{
    uint32_t ccsFormat = FVID2_CCSF_BITS12_PACKED;

    switch (format)
    {
        case (uint32_t)TIVX_RAW_IMAGE_P12_BIT:
            ccsFormat = FVID2_CCSF_BITS12_PACKED;
            break;
        case (vx_enum)TIVX_RAW_IMAGE_16_BIT:
        case (vx_df_image)VX_DF_IMAGE_U16:
        case (vx_df_image)VX_DF_IMAGE_UYVY:
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            ccsFormat = FVID2_CCSF_BITS12_UNPACKED16;
            break;
        default:
            ccsFormat = FVID2_CCSF_MAX;
            break;
    }

    return ccsFormat;
}

static uint32_t tivxCaptureMapInstId(uint32_t instId)
{
    uint32_t drvInstId = 0xFFFF;

    switch (instId)
    {
        case 0:
            drvInstId = CSIRX_INSTANCE_ID_0;
            break;
#if !defined(SOC_AM62A)
        case 1:
            drvInstId = CSIRX_INSTANCE_ID_1;
            break;
#if defined(SOC_J784S4) || defined(SOC_J742S2)
        case 2:
            drvInstId = CSIRX_INSTANCE_ID_2;
            break;
#endif
#endif
        default:
            /* do nothing */
            break;
    }

    return (drvInstId);
}

/* TODO: Complete this case statement */
static uint32_t tivxCaptureExtractDataFormat(uint32_t format)
{
    uint32_t dataFormat = FVID2_DF_BGRX32_8888;

    switch (format)
    {
        case (vx_df_image)VX_DF_IMAGE_UYVY:
            dataFormat = FVID2_DF_YUV422I_UYVY;
        break;
        case (vx_df_image)VX_DF_IMAGE_YUYV:
            dataFormat = FVID2_DF_YUV422I_YUYV;
        break;
        default:
            dataFormat = FVID2_DF_BGRX32_8888;
        break;
    }

    return dataFormat;
}

static vx_status tivxCaptureSetCreateParams(
       tivxCaptureParams *prms,
       const tivx_obj_desc_user_data_object_t *obj_desc)
{
    vx_status status = (vx_status)VX_SUCCESS;
    uint32_t loopCnt = 0U, i, format, width, height, planes, stride[TIVX_IMAGE_MAX_PLANES];
    void *capture_config_target_ptr;
    tivx_capture_params_t *params;
    uint32_t chIdx, instId = 0U, instIdx;
    Csirx_CreateParams *createParams;
    tivx_obj_desc_raw_image_t *raw_image = NULL;

    capture_config_target_ptr = tivxMemShared2TargetPtr(&obj_desc->mem_ptr);

    tivxCheckStatus(&status, tivxMemBufferMap(capture_config_target_ptr, obj_desc->mem_size,
        (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_READ_ONLY));

    params = (tivx_capture_params_t *)capture_config_target_ptr;

    /* Scan through all the instances provided in the Node instance and prepare CSIRX DRV instance data/cfg */
    for (instIdx = 0U ; instIdx < params->numInst ; instIdx++)
    {
        /* set instance to be used for capture */
        prms->instParams[instIdx].instId = tivxCaptureMapInstId(params->instId[instIdx]);
        prms->numOfInstUsed++;
    }
    /* Scan through all the channels provided in the Node instance and prepare CSIRX DRV instance data/cfg */
    for (chIdx = 0U ; chIdx < params->numCh ; chIdx++)
    {
        instId = tivxCaptureGetDrvInstIndex(prms, params->chInstMap[chIdx]);
        if (instId >= prms->numOfInstUsed)
        {
            status = (vx_status)VX_FAILURE;
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Wrong Instance ID provided: %d !!!\n", params->chInstMap[chIdx]);
            break;
        }
        else
        {
            prms->instParams[instId].chVcMap[prms->instParams[instId].numCh] =
                                                        params->chVcNum[chIdx];
            prms->instParams[instId].numCh++;
        }
    }

    if (status == (vx_status)VX_SUCCESS)
    {
        if ((vx_enum)TIVX_OBJ_DESC_RAW_IMAGE == (vx_enum)prms->img_obj_desc[0]->type)
        {
            raw_image = (tivx_obj_desc_raw_image_t *)prms->img_obj_desc[0];
            format = raw_image->params.format[0].pixel_container; /* TODO: Question: what should be done when this is different per exposure */
            width = raw_image->params.width;
            height = raw_image->params.height + (raw_image->params.meta_height_before + raw_image->params.meta_height_after);
            planes = raw_image->params.num_exposures;
            for (i = 0; i < planes; i++)
            {
                stride[i] = (uint32_t)raw_image->imagepatch_addr[i].stride_y;
            }
            prms->instParams[instId].raw_capture = 1U;
        }
        else
        {
            tivx_obj_desc_image_t *image;
            image = (tivx_obj_desc_image_t *)prms->img_obj_desc[0];
            format = image->format;
            width = image->imagepatch_addr[0].dim_x;
            height = image->imagepatch_addr[0].dim_y;
            planes = image->planes;
            for (i = 0; i < planes; i++)
            {
                stride[i] = (uint32_t)image->imagepatch_addr[i].stride_y;
            }
            prms->instParams[instId].raw_capture = 0U;
        }

        /* Copying timeout values from user to local structure */
        prms->timeout        = params->timeout;
        prms->timeoutInitial = params->timeoutInitial;

        /* Do following for each CSIRX DRV instance in the current Node */
        for (instIdx = 0U ; instIdx < params->numInst ; instIdx++)
        {
            prms->instParams[instIdx].captParams = prms;
            /* set instance configuration parameters */
            createParams = &prms->instParams[instIdx].createPrms;
            Csirx_createParamsInit(createParams);
            /* Set CSIRX D-PHY configuration parameters */
            Csirx_initDPhyCfg(&prms->instParams[instIdx].dphyCfg);
            prms->instParams[instIdx].dphyCfg.inst               = params->instId[instIdx];
            prms->instParams[instIdx].dphyCfg.rightLaneBandSpeed = params->instCfg[instIdx].laneBandSpeed;
            prms->instParams[instIdx].dphyCfg.leftLaneBandSpeed  = params->instCfg[instIdx].laneBandSpeed;

            /* set module configuration parameters */
            createParams->instCfg.enableCsiv2p0Support = params->instCfg[instIdx].enableCsiv2p0Support;
            createParams->instCfg.enableErrbypass      = (uint32_t)UFALSE;
            createParams->instCfg.numPixelsStrm0       = params->instCfg[instIdx].numPixels;
            createParams->instCfg.enableStrm[CSIRX_CAPT_STREAM_ID] = 1U;
            createParams->instCfg.numDataLanes = params->instCfg[instIdx].numDataLanes;
#if !defined(SOC_J722S) && defined(ASF_TEST_ENABLED_CAPTURE)
            createParams->instCfg.trigAsfTestIntr.asfTransToErrTest = 1U;
            createParams->instCfg.trigAsfTestIntr.asfCsrErrTest     = 1U;
            createParams->instCfg.trigAsfTestIntr.asfDapErrTest     = 1U;
#endif
            for (loopCnt = 0U ;
                 loopCnt < createParams->instCfg.numDataLanes ;
                 loopCnt++)
            {
                createParams->instCfg.dataLanesMap[loopCnt] = params->instCfg[instIdx].dataLanesMap[loopCnt];
            }

            createParams->numCh = prms->instParams[instIdx].numCh;
            for (loopCnt = 0U ; loopCnt < createParams->numCh ; loopCnt++)
            {
                createParams->chCfg[loopCnt].chId = loopCnt;
                createParams->chCfg[loopCnt].chType = CSIRX_CH_TYPE_CAPT;
                createParams->chCfg[loopCnt].vcNum = prms->instParams[instIdx].chVcMap[loopCnt];

                if ((uint32_t)TIVX_OBJ_DESC_RAW_IMAGE == prms->img_obj_desc[0]->type)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach else portion.
                    This condition depends on internal state of this component, which cannot be
                    altered through the video_io component interface.
                    Therefore, this case is out of scope for the video_io test framework.
                    Effect on this unit: If raw_image were NULL, the CSI data type extraction
                    function would not be called and the channel data type would not be assigned by
                    this block.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (NULL != raw_image)
                    /* LDRA_JUSTIFY_END */
                    {
                        createParams->chCfg[loopCnt].inCsiDataType =
                            tivxCaptureExtractInCsiDataTypeFromRawImg(raw_image);
                    }
                    
                }
                else
                {
                    createParams->chCfg[loopCnt].inCsiDataType =
                        tivxCaptureExtractInCsiDataType(format);
                }
                if (CAPTURE_IN_CSI_DT_INVALID ==
                                    createParams->chCfg[loopCnt].inCsiDataType)
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR,
                        " CAPTURE: ERROR: Un-supported Capture Data-type!!!\n");
                    break;
                }

                createParams->chCfg[loopCnt].outFmt.width =
                    width;
                createParams->chCfg[loopCnt].outFmt.height =
                    height;
                for (i = 0; i < planes; i ++)
                {
                    createParams->chCfg[loopCnt].outFmt.pitch[i] =
                        stride[i];
                }

                createParams->chCfg[loopCnt].outFmt.dataFormat =
                    tivxCaptureExtractDataFormat(format);
                createParams->chCfg[loopCnt].outFmt.ccsFormat =
                    tivxCaptureExtractCcsFormat(format);
            }
            /* set frame drop buffer parameters */
            createParams->frameDropBufLen = CAPTURE_FRAME_DROP_LEN;
            void *p_drop_buf = NULL;
            p_drop_buf = tivxMemAlloc(createParams->frameDropBufLen, (vx_enum)TIVX_MEM_EXTERNAL);
	        createParams->frameDropBuf = (uint64_t)(uintptr_t)p_drop_buf;

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            Reaching this portion requires an allocation from a shared memory region to fail, which
            the video_io test framework cannot induce deterministically.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: The code sets status to VX_ERROR_NO_MEMORY, prints an error
            message, and stops configuring the remaining Capture instances.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (0U == createParams->frameDropBuf)
            {
                status = (vx_enum)VX_ERROR_NO_MEMORY;
                VX_PRINT(VX_ZONE_ERROR,
                    " CAPTURE: ERROR: Insufficient memory for frameDropBuf!!!\n");
                break;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    tivxCheckStatus(&status, tivxMemBufferUnmap(capture_config_target_ptr,
       obj_desc->mem_size, (vx_enum)VX_MEMORY_TYPE_HOST,
       (vx_enum)VX_READ_ONLY));

    return status;
}

static vx_status tivxCaptureStart(tivxCaptureParams *prms)
{
    vx_status status = (vx_enum)VX_SUCCESS;
    uint32_t instIdx;
    int32_t fvid2_status = FVID2_SOK;

    if (0U == prms->steady_state_started)
    {
        /* start all driver instances in the node */
        for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
        {
            fvid2_status = Fvid2_start(prms->instParams[instIdx].drvHandle, NULL);
            
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            This failure is reported by the capture driver only in the event of a driver or hardware
            level fault, which cannot be stimulated through the video_io component interface.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: The function returns VX_FAILURE, prints an error message, and stops
            starting the remaining Capture instances.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could not start FVID2 !!!\n");
                break;
            }
            /* LDRA_JUSTIFY_END */
        }
    }

    return status;
}

/* Setting timeout based on capture params and whether user has provided error frame */
static void tivxCaptureSetTimeout(tivxCaptureParams *prms)
{
    /* Only apply timeout if error frame has been sent; otherwise wait forever */
    if (1U == prms->enableErrorFrameTimeout)
    {
        /* Using timeoutInitial if all channels are active, else using timeout */
        if ((uint8_t)((1U << prms->numCh) - 1U) == prms->activeChannelMask)
        {
            prms->timeoutRemaining = prms->timeoutInitial;
        }
        else
        {
            prms->timeoutRemaining = prms->timeout;
        }
    }
    else
    {
        prms->timeoutRemaining = VX_TIMEOUT_WAIT_FOREVER;

        if (0U == prms->steady_state_started)
        {
            /* Notifying user with a warning that the error frame was not provided if the timeout is not FOREVER */
            if ( (VX_TIMEOUT_WAIT_FOREVER != prms->timeoutInitial) ||
                 (VX_TIMEOUT_WAIT_FOREVER != prms->timeout) )
            {
                VX_PRINT(VX_ZONE_WARNING, " CAPTURE: WARNING: Error frame not provided using tivxCaptureRegisterErrorFrame, defaulting to waiting forever !!!\n");
            }
        }
    }
}

/* Determines if a frame has been received from each active channel */
static uint32_t tivxCaptureIsAllChFrameAvailable(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        uint8_t timeoutExceeded)
{
    uint32_t is_all_ch_frame_available = 1;
    vx_uint32 chId = 0U;

    /* Initial loop through channel to check for inactive and active channels */
    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        uintptr_t temp_desc_id;

        if ((vx_status)VX_SUCCESS == tivxQueuePeek(&prms->pendingFrameQ[chId], &temp_desc_id))
        {
            recv_obj_desc_id[chId] = (uint16_t*)temp_desc_id;
        }

        if (NULL==recv_obj_desc_id[chId])
        {
            /* Handle case that capture node timed out; set associated bit in activeChannelMask to 0 */
            if (CAPTURE_TIMEOUT_EXCEEDED == timeoutExceeded)
            {
                prms->activeChannelMask &= (uint8_t)(~(1U<<chId));
                VX_PRINT(VX_ZONE_INFO,
                    " Channel %d not received!!!\n", chId);
            }
        }
        else
        {
            /* Handle the case that a camera came back up and set associated bit in activeChannelMask to 1 */
            prms->activeChannelMask |= (uint8_t)(1U<<chId);
        }
    }

    /* Loop through channels when capture timeout has not been exceeded to
     * check if all frames have been received */
    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        uintptr_t temp_desc_id;

        if ((vx_status)VX_SUCCESS == tivxQueuePeek(&prms->pendingFrameQ[chId], &temp_desc_id))
        {
            recv_obj_desc_id[chId] = (uint16_t*)temp_desc_id;
        }

        /* Handle case that capture node timed out */
        if (CAPTURE_TIMEOUT_VALID == timeoutExceeded)
        {
            /* Marks that a frame is not available only if this is an active channel; i.e., channel has died
             * or if no channel are active */
            if( ( (NULL==recv_obj_desc_id[chId]) &&
                  ((1U<<chId) & prms->activeChannelMask) != 0U) ||
                (0U == prms->activeChannelMask) )
            {
                is_all_ch_frame_available = 0;
            }
        }
    }

    return is_all_ch_frame_available;
}

static vx_status tivxCaptureDequeueFrameFromDriver(tivxCaptureParams *prms)
{
    vx_status status = (vx_enum)VX_SUCCESS;
    uint32_t instIdx, tmp_obj_desc_id = 0U, tmp_timestamp_lo = 0U, tmp_timestamp_hi = 0U;
    uint64_t tmp_timestamp = 0U;
    tivxCaptureInstParams *instParams;
    Fvid2_Frame *fvid2Frame;
    vx_uint32 frmIdx = 0U, chId = 0U;
    int32_t fvid2_status = FVID2_SOK;
    Fvid2_FrameList *frmList;

    frmList = &prms->frmList;
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];

        frmList->numFrames = 0;
        fvid2_status = Fvid2_dequeue(instParams->drvHandle,
                                     frmList,
                                     0,
                                     FVID2_TIMEOUT_NONE);

        if(FVID2_SOK == fvid2_status)
        {
            for(frmIdx=0; frmIdx < frmList->numFrames; frmIdx++)
            {
                uint16_t frameErrors = 0U;
                uintptr_t key;
                fvid2Frame = frmList->frames[frmIdx];

                if (FVID2_FRAME_STATUS_COMPLETED != fvid2Frame->status)
                {
                    VX_PRINT(VX_ZONE_ERROR,
                        " CAPTURE: ERROR: Incomplete Frame for Ch%d !!!\n", fvid2Frame->chNum);
                }

                chId = tivxCaptureGetNodeChannelNum(
                                    prms,
                                    instIdx,
                                    fvid2Frame->chNum);

                tmp_obj_desc_id = (uintptr_t)fvid2Frame->appData;
                tmp_timestamp = fvid2Frame->timeStamp64;

                tivx_uint64_to_uint32(
                    tmp_timestamp,
                    &tmp_timestamp_hi,
                    &tmp_timestamp_lo
                );

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach the uncovered outcomes of this switch.
                This frame status is reported by the capture driver only in the event of a DMA level
                fault, which cannot be stimulated through the video_io component interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: The frame completion status reported by the driver is mapped to
                the corresponding Capture error bit.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                switch (fvid2Frame->status)
                {
                /* LDRA_JUSTIFY_END */
                    case FVID2_FRAME_STATUS_TRUNCATED:
                        frameErrors |= TIVX_CAPTURE_ERR_SHORT_FRAME;
                        break;

                    case FVID2_FRAME_STATUS_ELONGATED:
                        frameErrors |= TIVX_CAPTURE_ERR_LONG_FRAME;
                        break;

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This frame status is reported by the capture driver only in the event of a DMA
                    level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: frameErrors is OR'd with TIVX_CAPTURE_ERR_DMA_SUBMISSION,
                    which is later reported to the application through the Capture node's
                    error-event mechanism.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    case FVID2_FRAME_STATUS_SUBMISSION_ERROR:
                        frameErrors |= TIVX_CAPTURE_ERR_DMA_SUBMISSION;
                        break;
                    /* LDRA_JUSTIFY_END */

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This frame status is reported by the capture driver only in the event of a DMA
                    level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: frameErrors is OR'd with TIVX_CAPTURE_ERR_DMA_ABORTED,
                    which is later reported to the application through the Capture node's
                    error-event mechanism.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    case FVID2_FRAME_STATUS_ABORTED:
                        frameErrors |= TIVX_CAPTURE_ERR_DMA_ABORTED;
                        break;
                    /* LDRA_JUSTIFY_END */

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This frame status is reported by the capture driver only in the event of a DMA
                    level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: frameErrors is OR'd with TIVX_CAPTURE_ERR_DMA_ERROR, which
                    is later reported to the application through the Capture node's error-event
                    mechanism.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    case FVID2_FRAME_STATUS_ERROR:
                        frameErrors |= TIVX_CAPTURE_ERR_DMA_ERROR;
                        break;
                    /* LDRA_JUSTIFY_END */

                    case FVID2_FRAME_STATUS_COMPLETED:
                    default:
                        /* Status is not reported through Capture error information. */
                        break;
                }

                if (0U != frameErrors)
                {
                    key = HwiP_disable();

                    prms->local_error_info[chId].error_bitfield |= frameErrors;

                    HwiP_restore(key);
                    prms->local_error_info[chId].timestamp = (vx_uint16)tmp_timestamp;
                }

                (void)tivxQueuePut(&prms->freeFvid2FrameQ[chId], (uintptr_t)fvid2Frame, TIVX_EVENT_TIMEOUT_NO_WAIT);
                (void)tivxQueuePut(&prms->pendingFrameQ[chId], (uintptr_t)tmp_obj_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
                (void)tivxQueuePut(&prms->pendingFrameTimestampLoQ[chId], tmp_timestamp_lo, TIVX_EVENT_TIMEOUT_NO_WAIT);
                (void)tivxQueuePut(&prms->pendingFrameTimestampHiQ[chId], tmp_timestamp_hi, TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level test framework and test applications cannot reach this
        portion.
        This failure is reported by the capture driver only in the event of a driver or hardware
        level fault, which cannot be stimulated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: No error is recorded, and processing continues with the next Capture
        instance.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if (fvid2_status == FVID2_ENO_MORE_BUFFERS)
        {
            /* continue: move onto next driver instance
              within node as current driver instance did
              not generate this CB */
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level test framework and test applications cannot reach this
        portion.
        This failure is reported by the capture driver only in the event of a driver or hardware
        level fault, which cannot be stimulated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: FVID2_EAGAIN is ignored, while any other status is converted to
        VX_FAILURE and reported with an error message.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            /* TIOVX-687: Note: disabling for now until investigated further */
            if (FVID2_EAGAIN != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR,
                    " CAPTURE: ERROR: FVID2 Dequeue failed !!!\n");
            }
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

/* Populating capture output with object descriptors based on if camera is enabled */
static void tivxCaptureGetObjDesc(tivxCaptureParams *prms,
        uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH],
        tivx_obj_desc_object_array_t *output_desc,
        uint64_t *timestamp)
{
    uintptr_t tmp_timestamp_hi = 0U, tmp_timestamp_lo = 0U;
    uint64_t tmp_timestamp = 0U;
    vx_uint32 chId = 0U;

    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        uintptr_t temp_desc_id;

        if ((vx_status)VX_SUCCESS == tivxQueueGet(&prms->pendingFrameQ[chId], &temp_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT))
        {
            recv_obj_desc_id[chId] = (uint16_t*)temp_desc_id;
        }
    }

    for(chId = 0U ; chId < prms->numCh ; chId++)
    {
        uintptr_t tmp_desc_id_32;
        uint16_t tmp_obj_desc_16;
        tivx_obj_desc_t *tmp_obj_desc;

        if (NULL!=recv_obj_desc_id[chId])
        {
            tmp_desc_id_32 = (uintptr_t)recv_obj_desc_id[chId];
        }
        else
        {
            (void)tivxQueueGet(&prms->errorFrameQ[chId], (uintptr_t*)&tmp_desc_id_32, TIVX_EVENT_TIMEOUT_NO_WAIT);
        }

        tmp_obj_desc_16 = (uint16_t)tmp_desc_id_32;
        tivxGetObjDescList(&tmp_obj_desc_16, &tmp_obj_desc, 1);

        output_desc->obj_desc_id[chId] = (uint16_t)tmp_obj_desc_16;

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This condition depends on internal state of this component, which cannot be altered through
        the video_io component interface.
        Therefore, this case is out of scope for the video_io test framework.
        Effect on this unit: If the descriptor is not found, its scope and timestamp are not
        updated; this function does not return an error status.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(tmp_obj_desc!=NULL)
        /* LDRA_JUSTIFY_END */
        {
            tmp_obj_desc->scope_obj_desc_id = (uint16_t)output_desc->base.obj_desc_id;
            (void)tivxQueueGet(&prms->pendingFrameTimestampLoQ[chId], (uintptr_t*)&tmp_timestamp_lo, TIVX_EVENT_TIMEOUT_NO_WAIT);
            (void)tivxQueueGet(&prms->pendingFrameTimestampHiQ[chId], (uintptr_t*)&tmp_timestamp_hi, TIVX_EVENT_TIMEOUT_NO_WAIT);

            tivx_uint32_to_uint64(
                    &tmp_timestamp,
                    (uint32_t)tmp_timestamp_hi,
                    (uint32_t)tmp_timestamp_lo
                );

            tmp_obj_desc->timestamp = tmp_timestamp;

            /* Setting the timestamp for object array to largest value of object array elements */
            if (tmp_timestamp > *timestamp)
            {
                *timestamp = tmp_timestamp;
            }
        }
        
    }
}

static vx_status VX_CALLBACK tivxCaptureProcess(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    (void)priv_arg;
    vx_status status = (vx_status)VX_SUCCESS;
    tivxCaptureParams *prms = NULL;
    tivx_obj_desc_object_array_t *output_desc;
    vx_uint32 size, chId = 0U;
    vx_enum state;
    uint64_t timestamp = 0U;
    uint8_t timeoutExceeded = CAPTURE_TIMEOUT_VALID;

    /* LDRA_JUSTIFY_START
    <metric start>  statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this
    portion.
    The callback arguments are supplied and pre-validated by the framework before this unit is
    invoked, and cannot be altered through the video_io component interface.
    Therefore, this failure case is out of scope for the video_io test framework.
    Effect on this unit: Invalid callback parameters cause the function to return VX_FAILURE without
    processing a Capture frame.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this
    portion.
    This outcome requires an earlier step in this unit to have failed, which is itself out of scope
    for the video_io test framework as justified at its own location.
    Effect on this unit: If parameter validation fails, the target kernel context and node state are
    not retrieved.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        void *temp_prms = NULL;
        output_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

        status = tivxGetTargetKernelInstanceContext(kernel,
            &temp_prms, &size);
        prms = (tivxCaptureParams *)temp_prms;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        The kernel instance context used here is the one this component stored during Capture
        creation, and cannot be replaced or invalidated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: An invalid kernel instance context would set the status to VX_FAILURE
        and skip the node state retrieval below.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((vx_status)VX_SUCCESS != status) || (NULL == prms) ||
            (sizeof(tivxCaptureParams) != size))
        {
            status = (vx_status)VX_FAILURE;
        }
        else
        /* LDRA_JUSTIFY_END */
        {
            status = tivxGetTargetKernelInstanceState(kernel, &state);
        }
    }

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach else
    portion.
    This outcome requires an earlier step in this unit to have failed, which is itself out of scope
    for the video_io test framework as justified at its own location.
    Effect on this unit: If the retrieval failed, no frame would be enqueued to or dequeued from the
    capture driver and the failure status would be returned.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        /* Steady state: receives a buffer and returns a buffer */
        if ((vx_enum)VX_NODE_STATE_STEADY == state)
        {
            /* Providing buffers to capture source */
            status = tivxCaptureEnqueueFrameToDriver(output_desc, prms);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            This failure is reported by the capture driver only in the event of a driver or hardware
            level fault, which cannot be stimulated through the video_io component interface.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: The status is set to VX_FAILURE and an error message is printed,
            and the capture driver is not started for this frame.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Enqueue Frame to Driver failed !!!\n");
            }
            /* LDRA_JUSTIFY_END */

            /* Starts FVID2 on initial frame */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach else portion.
            This outcome requires an earlier step in this unit to have failed, which is itself out
            of scope for the video_io test framework as justified at its own location.
            Effect on this unit: If the enqueue failed, tivxCaptureStart() would be skipped and the
            enqueue failure status would be returned to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                status = tivxCaptureStart(prms);
            }

            /* Pends until a frame is available then dequeue frames from capture driver */

            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach else portion.
            This outcome requires an earlier step in this unit to have failed, which is itself out
            of scope for the video_io test framework as justified at its own location.
            Effect on this unit: If the start failed, frame dequeuing would be skipped and the start
            failure status would be returned to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS == status)
            /* LDRA_JUSTIFY_END */
            {
                uint16_t *recv_obj_desc_id[TIVX_CAPTURE_MAX_CH];
                tivx_obj_desc_object_array_t *recv_obj_arr_desc;
                uintptr_t temp_arr_desc_id;

                uint32_t is_all_ch_frame_available = 0;
                vx_bool errorReported = (vx_bool)vx_false_e;

                for(chId = 0U ; chId < TIVX_CAPTURE_MAX_CH ; chId++)
                {
                    recv_obj_desc_id[chId] = NULL;
                }

                tivxCaptureSetTimeout(prms);
                while(is_all_ch_frame_available == 0U)
                {
                    /* Upon first entry into this while loop, tivxCaptureIsAllChFrameAvailable is called to determine if
                     * any pending object descriptors are already available to use */
                    is_all_ch_frame_available = tivxCaptureIsAllChFrameAvailable(prms, recv_obj_desc_id, timeoutExceeded);

                    /* In the case that no object descriptors are available, entering the logic to check first if timeout
                     * has been exceeded and if not, to dequeue frames from driver for all channels */
                    if(is_all_ch_frame_available == 0U)
                    {
                        uint32_t status_bits = (uint32_t)status | (uint32_t)tivxCaptureTimeout(prms);
                        status = (vx_status)status_bits;
                        if (status != (vx_status)VX_SUCCESS)
                        {
                            prms->timeoutRemaining = 0;
                            timeoutExceeded = CAPTURE_TIMEOUT_EXCEEDED;
                        }
                        else
                        {
                            /* This function will pend on an event posted from the capture driver to then dequeue the frame
                             * from the capture driver */
                            status = tivxCaptureDequeueFrameFromDriver(prms);
                        }
                    }

                    {
                        uint32_t has_error = 0U;
                        for (chId = 0U; chId <= prms->numCh; chId++)
                        {
                            if (0U != prms->local_error_info[chId].error_bitfield)
                            {
                                has_error = 1U;
                                break;
                            }
                        }

                        if (0U != has_error)
                        {
                            uintptr_t key;

                            key = HwiP_disable();
                            (void)tivxSetTargetKernelInstanceErrorInfo(
                                kernel,
                                prms->local_error_info,
                                (vx_uint16)(((uint32_t)prms->numCh + 1U) * sizeof(tivx_capture_error_per_channel_t)));
                            (void)memset(prms->local_error_info, 0, TIVX_MAX_ERROR_INFO_SIZE);
                            HwiP_restore(key);

                            errorReported = (vx_bool)vx_true_e;
                        }
                    }
                }

                if ((vx_bool)vx_true_e == errorReported)
                {
                    status = (vx_status)VX_FAILURE;
                }

                /* Getting next obj arr obj desc from queue to populate with the latest dequeued frames */
                (void)tivxQueueGet(&prms->pendingObjArrayQ, &temp_arr_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
                recv_obj_arr_desc = (tivx_obj_desc_object_array_t *)temp_arr_desc_id;

                obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX] = (tivx_obj_desc_t *)recv_obj_arr_desc;

                output_desc = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

                tivxCaptureGetObjDesc(prms, recv_obj_desc_id, output_desc, &timestamp);

                obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX]->timestamp = timestamp;

                if (0U == prms->steady_state_started)
                {
                    prms->steady_state_started = 1;
                }
            }
    
        }
        /* Pipe-up state: only receives a buffer; does not return a buffer */
        else
        {
            status = tivxCaptureEnqueueFrameToDriver(output_desc, prms);
        }
    }

    return status;
}

static vx_status VX_CALLBACK tivxCaptureCreate(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    (void)priv_arg;
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    tivx_obj_desc_user_data_object_t *input_obj_desc;
    tivx_obj_desc_object_array_t *output_desc;
    tivxCaptureParams *prms = NULL;
    uint32_t chId, bufId, instIdx;
    tivxCaptureInstParams *instParams = NULL;
    Csirx_CreateParams *createParams;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this
    portion.
    The callback arguments are supplied and pre-validated by the framework before this unit is
    invoked, and cannot be altered through the video_io component interface.
    Therefore, this failure case is out of scope for the video_io test framework.
    Effect on this unit: Invalid callback parameters cause the function to return VX_FAILURE without
    creating a Capture instance.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        input_obj_desc = (tivx_obj_desc_user_data_object_t *)obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX];
        output_desc    = (tivx_obj_desc_object_array_t *)obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX];

        prms = tivxMemAlloc(sizeof(tivxCaptureParams), (vx_enum)TIVX_MEM_EXTERNAL);

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        Reaching this portion requires an allocation from a shared memory region to fail, which the
        video_io test framework cannot induce deterministically.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: If the allocation failed, the Capture parameter structure would not be
        zero-initialized and the else block below reports VX_ERROR_NO_MEMORY.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (NULL != prms)
        /* LDRA_JUSTIFY_END */
        {
            (void)memset(prms, 0, sizeof(tivxCaptureParams));
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        Reaching this portion requires an allocation from a shared memory region to fail, which the
        video_io test framework cannot induce deterministically.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: An error message is printed and VX_ERROR_NO_MEMORY is returned to the
        application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could allocate memory !!!\n");
            status = (vx_status)VX_ERROR_NO_MEMORY;
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If the allocation failed, the steady-state, error-timeout and
        channel-count fields of prms would be left uninitialized and every later Capture
        configuration step would be skipped.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            /* Initialize steady_state_started to 0 */
            prms->steady_state_started = 0;

            /* Black frame has not been sent */
            prms->enableErrorFrameTimeout = 0U;

            /* Initialize raw capture to 0 */
            for (instIdx = 0U ; instIdx < TIVX_CAPTURE_MAX_INST ; instIdx++)
            {
                prms->instParams[instIdx].raw_capture = 0;
            }

            /* Set number of channels to number of items in object array */
            prms->numCh = (uint8_t)output_desc->num_items;

            prms->activeChannelMask = (uint8_t)((1U<<(prms->numCh))-1U);

            if (prms->numCh > TIVX_CAPTURE_MAX_CH)
            {
                status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
                VX_PRINT(VX_ZONE_ERROR, "Object descriptor number of channels exceeds max value allowed by capture!!!\r\n");
            }
        }

        /* Setting CSIRX capture parameters */
        if ((vx_status)VX_SUCCESS == status)
        {
            tivxGetObjDescList(output_desc->obj_desc_id, (tivx_obj_desc_t **)prms->img_obj_desc,
                           prms->numCh);

            status = tivxCaptureSetCreateParams(prms, input_obj_desc);
        }

        /* Creating frame available event */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxEventCreate(&prms->frame_available);

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            Reaching this portion requires the creation of a shared OS resource to fail, which the
            video_io test framework cannot induce deterministically.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: An error message is printed and the event creation status is
            propagated, skipping the capture driver creation below.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "Event creation failed in capture!!!\r\n");
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Creating FVID2 handle */
        if ((vx_status)VX_SUCCESS == status)
        {
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                Fvid2CbParams_init(&instParams->drvCbPrms);

                instParams->drvCbPrms.cbFxn   = (Fvid2_CbFxn) &captDrvCallback;
                instParams->drvCbPrms.appData = prms;

                instParams->drvHandle = Fvid2_create(CSIRX_CAPT_DRV_ID,
                                                     instParams->instId,
                                                     &instParams->createPrms,
                                                     &instParams->createStatus,
                                                     &instParams->drvCbPrms);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                This failure is reported by the capture driver only in the event of a driver or
                hardware level fault, which cannot be stimulated through the video_io component
                interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: An error message is printed and the status is set to
                VX_FAILURE, skipping the D-PHY and error-event configuration for this instance.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((NULL == instParams->drvHandle) ||
                    (instParams->createStatus.retVal != FVID2_SOK))
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture Create Failed!!!\r\n");
                    status = (vx_status)VX_FAILURE;
                }
                else
                /* LDRA_JUSTIFY_END */
                {
                    fvid2_status = Fvid2_control(
                        instParams->drvHandle, IOCTL_CSIRX_SET_DPHY_CONFIG,
                        &instParams->dphyCfg, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This failure is reported by the capture driver only in the event of a driver or
                    hardware level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: The status is set to VX_FAILURE and an error message is
                    printed, skipping the error-event registration for this instance.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, ": Failed to set PHY Parameters!!!\r\n");
                    }
                    else
                    /* LDRA_JUSTIFY_END */
                    {
                        /* Register Error Events */
                        Csirx_EventPrms eventPrms;
                        Csirx_eventPrmsInit(&eventPrms);
                        eventPrms.eventCb = &captDrvErrorCallback;
                        eventPrms.appData = prms;
                        fvid2_status = Fvid2_control(instParams->drvHandle,
                                               IOCTL_CSIRX_REGISTER_EVENT,
                                               &eventPrms,
                                               NULL);
                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications
                        cannot reach this portion.
                        This failure is reported by the capture driver only in the event of a driver
                        or hardware level fault, which cannot be stimulated through the video_io
                        component interface.
                        Therefore, this failure case is out of scope for the video_io test
                        framework.
                        Effect on this unit: The status is set to VX_FAILURE and an error message is
                        printed for this Capture instance.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != fvid2_status)
                        {
                            status = (vx_status)VX_FAILURE;
                            VX_PRINT(VX_ZONE_ERROR, ": Failed to set Event Parameters!!!\r\n");
                        }
                        /* LDRA_JUSTIFY_END */

#if !defined(SOC_J722S)
                        if ((0U == instIdx) && ((vx_status)VX_SUCCESS == status))
                        {
                            Csirx_EventPrms asfPrms;

                            Csirx_eventPrmsInit(&asfPrms);
                            asfPrms.eventGroup = CSIRX_ESM_LOW_EVENT_GROUP_ASF;
                            asfPrms.eventMasks = CSIRX_EVENT_TYPE_ASF_TRANS_TO_ERR |
                                                CSIRX_EVENT_TYPE_ASF_CSR_ERR |
                                                CSIRX_EVENT_TYPE_ASF_DAP_ERR;
                            asfPrms.eventCb    = &captDrvAsfCallback;
                            asfPrms.appData    = prms;

                            fvid2_status = Fvid2_control(instParams->drvHandle,
                                                        IOCTL_CSIRX_REGISTER_EVENT,
                                                        &asfPrms,
                                                        NULL);
                            /* LDRA_JUSTIFY_START
                            <metric start> statement branch <metric end>
                            <justification start>
                            Rationale: The component level negative test framework and test
                            applications cannot reach this portion.
                            This failure is reported by the capture driver only in the event of a
                            driver or hardware level fault, which cannot be stimulated through the
                            video_io component interface.
                            Therefore, this failure case is out of scope for the video_io test
                            framework.
                            Effect on this unit: The status is set to VX_FAILURE and an error
                            message is printed for this Capture instance.
                            However, due to the stated rationale, this is not tested.
                            <justification end> */
                            if (FVID2_SOK != fvid2_status)
                            {
                                status = (vx_status)VX_FAILURE;
                                VX_PRINT(VX_ZONE_ERROR,
                                        "Failed to register CSIRX ASF events !!!\r\n");
                            }
                            /* LDRA_JUSTIFY_END */
                        }
#endif
                    }
                }
            }
        }

        if (((vx_status)VX_SUCCESS == status) && (NULL != instParams))
        {
            Fvid2_TimeStampParams tsParams;

            tsParams.timeStampFxn = (Fvid2_TimeStampFxn) &tivxPlatformGetTimeInUsecsWrapper;
            /* register time stamping function */
            fvid2_status = Fvid2_control(instParams->drvHandle,
                                   FVID2_REGISTER_TIMESTAMP_FXN,
                                   &tsParams,
                                   NULL);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            This failure is reported by the capture driver only in the event of a driver or hardware
            level fault, which cannot be stimulated through the video_io component interface.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: The status is set to VX_FAILURE and an error message is printed,
            skipping the driver frame queue creation below.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (FVID2_SOK != fvid2_status)
            {
                status = (vx_status)VX_FAILURE;
                VX_PRINT(VX_ZONE_ERROR, ": Failed to set PHY Parameters!!!\r\n");
            }
            /* LDRA_JUSTIFY_END */
        }

        /* Creating FVID2 frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0u ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->freeFvid2FrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->fvid2_free_q_mem[chId], 0);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                The arguments supplied to this call are fixed by this unit, so the failure outcome
                cannot be produced through the video_io component interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: An error message is printed and the queue creation loop is
                terminated, returning the failure status to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture queue create failed!!!\n");
                    break;
                }
                /* LDRA_JUSTIFY_END */

                for(bufId = 0u ; bufId < (TIVX_CAPTURE_MAX_NUM_BUFS) ; bufId++)
                {
                    (void)tivxQueuePut(&prms->freeFvid2FrameQ[chId], (uintptr_t)&prms->fvid2Frames[chId][bufId], TIVX_EVENT_TIMEOUT_NO_WAIT);
                }
            }
        }

        /* Creating pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->pendingFrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_frame_free_q_mem[chId], 0);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                The arguments supplied to this call are fixed by this unit, so the failure outcome
                cannot be produced through the video_io component interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: An error message is printed and the queue creation loop is
                terminated, returning the failure status to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
                /* LDRA_JUSTIFY_END */
            }
        }

        /* Creating pending frame obj arr Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxQueueCreate(&prms->pendingObjArrayQ, TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_obj_arr_q_mem, 0);
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            The arguments supplied to this call are fixed by this unit, so the failure outcome
            cannot be produced through the video_io component interface.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: An error message is printed and the queue creation status is
            returned to the application.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
            }
            /* LDRA_JUSTIFY_END */
        }

        /* TODO: Should there be a flag to determine whether or not to create this? */
        /* Creating pending frame Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->errorFrameQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->error_frame_q_mem[chId], 0);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                The arguments supplied to this call are fixed by this unit, so the failure outcome
                cannot be produced through the video_io component interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: An error message is printed and the queue creation loop is
                terminated, returning the failure status to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
                /* LDRA_JUSTIFY_END */
            }
        }

        /* Creating pending timestamp Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->pendingFrameTimestampLoQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_frame_timestamp_lo_free_q_mem[chId], 0);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                The arguments supplied to this call are fixed by this unit, so the failure outcome
                cannot be produced through the video_io component interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: An error message is printed and the queue creation loop is
                terminated, returning the failure status to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
                /* LDRA_JUSTIFY_END */
            }
        }

        /* Creating pending timestamp Q */
        if ((vx_status)VX_SUCCESS == status)
        {
            for(chId = 0U ; chId < prms->numCh ; chId++)
            {
                status = tivxQueueCreate(&prms->pendingFrameTimestampHiQ[chId], TIVX_CAPTURE_MAX_NUM_BUFS, prms->pending_frame_timestamp_hi_free_q_mem[chId], 0);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                The arguments supplied to this call are fixed by this unit, so the failure outcome
                cannot be produced through the video_io component interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: An error message is printed and the queue creation loop is
                terminated, returning the failure status to the application.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS != status)
                {
                    VX_PRINT(VX_ZONE_ERROR, ": Capture create failed!!!\r\n");
                    break;
                }
                /* LDRA_JUSTIFY_END */
            }
        }

        if ((vx_status)VX_SUCCESS == status)
        {
            status = tivxSetTargetKernelInstanceContext(kernel, prms, sizeof(tivxCaptureParams));
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot reach this portion.
            This failure case is out of scope for the video_io test framework.
            Effect on this unit: If the control reaches here, the code base is NOT expected
            to accumulate and return an error. However, due to the stated rationale, this is not tested.
            <justification end> */
            if ((vx_status)VX_SUCCESS != status)
            {
                VX_PRINT(VX_ZONE_ERROR, "tivxSetTargetKernelInstanceContext failed %d \n", status);
            }
            /* LDRA_JUSTIFY_END */
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If prms were NULL, no Capture instance teardown would be performed and
        the existing failure status would be returned unchanged.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else if (NULL != prms)
        /* LDRA_JUSTIFY_END */
        {
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level test framework and test applications cannot reach
                this portion.
                This failure is reported by the capture driver only in the event of a driver or
                hardware level fault, which cannot be stimulated through the video_io component
                interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: The capture driver error events are unregistered and the driver
                handle of this Capture instance is deleted and cleared.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (NULL != instParams->drvHandle)
                {
                    /* Disable Error Events */

                    fvid2_status = Fvid2_control(instParams->drvHandle,
                                           IOCTL_CSIRX_UNREGISTER_EVENT,
                                           (void *)CSIRX_EVENT_GROUP_ERROR,
                                           NULL);
                    if(FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, ": Capture Event Unregister failed!!!\r\n");
                    }

#if !defined(SOC_J722S)
                    /* Disable ASF Events -- see the matching unregister call
                     * in tivxCaptureDelete() for why this is needed. */
                    if (0U == instIdx)
                    {
                        fvid2_status = Fvid2_control(instParams->drvHandle,
                                               IOCTL_CSIRX_UNREGISTER_EVENT,
                                               (void *)CSIRX_ESM_LOW_EVENT_GROUP_ASF,
                                               NULL);
                        if(FVID2_SOK != fvid2_status)
                        {
                            status = (vx_status)VX_FAILURE;
                            VX_PRINT(VX_ZONE_ERROR, ": Capture ASF Event Unregister failed!!!\r\n");
                        }
                    }
#endif
                    (void)Fvid2_delete(instParams->drvHandle, NULL);
                    instParams->drvHandle = NULL;
                }
                /* LDRA_JUSTIFY_END */

                /* Freeing memory used for frame drop buf */
                createParams = &instParams->createPrms;
                (void)tivxMemFree((void*)(uintptr_t)createParams->frameDropBuf, createParams->frameDropBufLen, (vx_enum)TIVX_MEM_EXTERNAL);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale: The component level negative test framework and test applications cannot
            reach this portion.
            This failure is reported by the capture driver only in the event of a driver or hardware
            level fault, which cannot be stimulated through the video_io component interface.
            Therefore, this failure case is out of scope for the video_io test framework.
            Effect on this unit: The frame-available event created earlier in this function is
            deleted before the Capture parameter structure is freed.
            However, due to the stated rationale, this is not tested.
            <justification end> */
            if (NULL != prms->frame_available)
            {
                (void)tivxEventDelete(&prms->frame_available);
            }
            /* LDRA_JUSTIFY_END */

            (void)tivxMemFree(prms, sizeof(tivxCaptureParams), (vx_enum)TIVX_MEM_EXTERNAL);
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        Reaching this portion requires an allocation from a shared memory region to fail, which the
        video_io test framework cannot induce deterministically.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: No Capture resource is released, because the allocation that would have
        created them never succeeded.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            /* do nothing */
        }
        /* LDRA_JUSTIFY_END */
    }

    return status;
}

static void tivxCapturePrintStatus(tivxCaptureInstParams *prms)
{
    int32_t fvid2_status;
    uint32_t cnt;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach else
    portion.
    The only call site within this component supplies parameters that are always valid, so this
    outcome cannot be produced through the video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: If the argument is NULL, no driver status is queried or printed; this void
    function does not return an error status.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != prms)
    /* LDRA_JUSTIFY_END */
    {
        fvid2_status = Fvid2_control(prms->drvHandle,
                                IOCTL_CSIRX_GET_INST_STATUS,
                                &prms->captStatus,
                                NULL);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This failure is reported by the capture driver only in the event of a driver or hardware
        level fault, which cannot be stimulated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: If the status query fails, the Capture statistics are not printed and
        control proceeds to the error-reporting block.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (FVID2_SOK == fvid2_status)
        /* LDRA_JUSTIFY_END */
        {
            VX_PRINT(VX_ZONE_INFO,   "==========================================================\r\n");
            VX_PRINT(VX_ZONE_INFO,   " Capture Status: Instance|%d\r\n", prms->instId);
            VX_PRINT(VX_ZONE_INFO,   "==========================================================\r\n");
            VX_PRINT(VX_ZONE_INFO,   " overflowCount: %d\r\n", prms->captStatus.overflowCount);
            VX_PRINT(VX_ZONE_INFO,   " spuriousUdmaIntrCount: %d\r\n", prms->captStatus.spuriousUdmaIntrCount);
            VX_PRINT(VX_ZONE_INFO,   " frontFIFOOvflCount: %d\r\n", prms->captStatus.frontFIFOOvflCount);
            VX_PRINT(VX_ZONE_INFO,   " crcCount: %d\r\n", prms->captStatus.crcCount);
            VX_PRINT(VX_ZONE_INFO,   " eccCount: %d\r\n", prms->captStatus.eccCount);
            VX_PRINT(VX_ZONE_INFO,   " correctedEccCount: %d\r\n", prms->captStatus.correctedEccCount);
            VX_PRINT(VX_ZONE_INFO,   " dataIdErrorCount: %d\r\n", prms->captStatus.dataIdErrorCount);
            VX_PRINT(VX_ZONE_INFO,   " invalidAccessCount: %d\r\n", prms->captStatus.invalidAccessCount);
            VX_PRINT(VX_ZONE_INFO,   " invalidSpCount: %d\r\n", prms->captStatus.invalidSpCount);
            for(cnt = 0U ; cnt < CSIRX_NUM_STREAM ; cnt ++)
            {
                VX_PRINT(VX_ZONE_INFO,   " strmFIFOOvflCount[%d]: %d\r\n", cnt, prms->captStatus.strmFIFOOvflCount[cnt]);
            }
            VX_PRINT(VX_ZONE_INFO,   " Channel Num | Frame Queue Count | Frame De-queue Count | Frame Drop Count | Error Frame Count |\r\n");
            for(cnt = 0U ; cnt < prms->numCh ; cnt ++)
            {
                VX_PRINT(VX_ZONE_INFO,
                      " %11d | %17d | %20d | %16d | %17d |\r\n",
                      cnt,
                      prms->captStatus.queueCount[cnt],
                      prms->captStatus.dequeueCount[cnt],
                      prms->captStatus.dropCount[cnt],
                      prms->captStatus.errorFrameCount[cnt]);
            }
        }
        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        This failure is reported by the capture driver only in the event of a driver or hardware
        level fault, which cannot be stimulated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: An error message is printed and no status is propagated because
        tivxCapturePrintStatus() has no return value.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        else
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Control failed !!!\n");
        }
        /* LDRA_JUSTIFY_END */
    }
}

static vx_status VX_CALLBACK tivxCaptureDelete(
       tivx_target_kernel_instance kernel,
       tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    (void)priv_arg;
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    tivxCaptureParams *prms = NULL;
    Fvid2_FrameList *frmList;
    uint32_t size, chId, bufId, instIdx;
    tivxCaptureInstParams *instParams;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this
    portion.
    The callback arguments are supplied and pre-validated by the framework before this unit is
    invoked, and cannot be altered through the video_io component interface.
    Therefore, this failure case is out of scope for the video_io test framework.
    Effect on this unit: Invalid callback parameters cause the function to return VX_FAILURE without
    releasing any Capture resource.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ( (num_params != TIVX_KERNEL_CAPTURE_MAX_PARAMS)
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_INPUT_ARR_IDX])
        || (NULL == obj_desc[TIVX_KERNEL_CAPTURE_OUTPUT_IDX])
    )
    {
        status = (vx_status)VX_FAILURE;
    }
    else
    /* LDRA_JUSTIFY_END */
    {
        void *temp_prms = NULL; 
        status = tivxGetTargetKernelInstanceContext(kernel, &temp_prms, &size); 
        prms = (tivxCaptureParams *)temp_prms;

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        The kernel instance context used here is the one this component stored during Capture
        creation, and cannot be replaced or invalidated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: An error message is printed and the context retrieval status is carried
        into the teardown condition below.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS != status)
        {
            VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: Could not obtain kernel instance context !!!\n");
        }
        /* LDRA_JUSTIFY_END */

        /* LDRA_JUSTIFY_START
        <metric start> statement branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        this portion.
        The kernel instance context used here is the one this component stored during Capture
        creation, and cannot be replaced or invalidated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: An error message is printed and the status is set to VX_FAILURE,
        skipping the whole Capture teardown below.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if(NULL == prms)
        {
            VX_PRINT(VX_ZONE_ERROR, "Kernel instance context is NULL!!!\n");
            status = (vx_status)VX_FAILURE;
        }
        /* LDRA_JUSTIFY_END */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If the context retrieval failed, all Capture instance teardown below
        would be skipped and the failure status returned to the application.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            frmList = &prms->frmList;
            for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
            {
                instParams = &prms->instParams[instIdx];

                /* Stopping FVID2 Capture */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach else portion.
                This outcome requires an earlier step in this unit to have failed, which is itself
                out of scope for the video_io test framework as justified at its own location.
                Effect on this unit: If a previous instance failed, the driver stop request for this
                instance would be skipped.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    fvid2_status = Fvid2_stop(instParams->drvHandle, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This failure is reported by the capture driver only in the event of a driver or
                    hardware level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: The status is set to VX_FAILURE and an error message is
                    printed, skipping the frame drain for this instance.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Capture not stopped !!!\n");
                    }
                    /* LDRA_JUSTIFY_END */
                }

                /* Dequeue all the request from the driver */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach else portion.
                This outcome requires an earlier step in this unit to have failed, which is itself
                out of scope for the video_io test framework as justified at its own location.
                Effect on this unit: If the stop request failed, the queued frames of this instance
                would not be drained from the driver.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    Fvid2FrameList_init(frmList);
                    do
                    {
                        fvid2_status = Fvid2_dequeue(
                            instParams->drvHandle,
                            frmList,
                            0,
                            FVID2_TIMEOUT_NONE);
                    } while (FVID2_SOK == fvid2_status);

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This failure is reported by the capture driver only in the event of a driver or
                    hardware level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: An error message is printed and the status is set to
                    VX_FAILURE, skipping the Capture statistics printout for this instance.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_ENO_MORE_BUFFERS != fvid2_status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Capture Dequeue Failed !!!\n");
                        status = (vx_status)VX_FAILURE;
                    }
                    /* LDRA_JUSTIFY_END */
                }

                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach else portion.
                This outcome requires an earlier step in this unit to have failed, which is itself
                out of scope for the video_io test framework as justified at its own location.
                Effect on this unit: If the drain failed, the Capture statistics of this instance
                would not be printed.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    tivxCapturePrintStatus(instParams);
                }

                /* Freeing memory used for frame drop buf */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach else portion.
                This outcome requires an earlier step in this unit to have failed, which is itself
                out of scope for the video_io test framework as justified at its own location.
                Effect on this unit: If an earlier step failed, the frame drop buffer of this
                instance would not be freed.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    Csirx_CreateParams *createParams;

                    createParams = &instParams->createPrms;

                    uintptr_t frame_drop_buf = (uintptr_t) createParams->frameDropBuf;
                    void *p_drop_buf = (void *) frame_drop_buf;
                    status = tivxMemFree(p_drop_buf, createParams->frameDropBufLen, (vx_enum)TIVX_MEM_EXTERNAL);

                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications cannot reach this portion.
                    The test framework does not support the configuration required to trigger this error scenario.
                    Effect on this unit: If the control reaches here, our code base is expected to print the error status.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Capture tivxMemFree failed %d \n", status);
                    }
                    /* LDRA_JUSTIFY_END */
                }

                /* Disable Error Events */

                fvid2_status = Fvid2_control(instParams->drvHandle,
                                       IOCTL_CSIRX_UNREGISTER_EVENT,
                                       (void *)CSIRX_EVENT_GROUP_ERROR,
                                       NULL);
                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                This failure is reported by the capture driver only in the event of a driver or
                hardware level fault, which cannot be stimulated through the video_io component
                interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: The status is set to VX_FAILURE and an error message is
                printed, skipping the driver handle deletion for this instance.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if(FVID2_SOK != fvid2_status)
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, ": Capture Event Unregister failed!!!\r\n");
                }
                /* LDRA_JUSTIFY_END */

#if !defined(SOC_J722S)
                /* Disable ASF Events. Only registered for instIdx==0 at
                 * create time, so only unregister it there too. Without
                 * this, CsirxDrv_eventGroupRegister()'s eventInitDone latch
                 * for CSIRX_ESM_LOW_EVENT_GROUP_ASF never clears, so a later
                 * Csirx_create() in the same boot silently keeps this
                 * instance's (now torn down) appData wired into
                 * captDrvAsfCallback instead of the new instance's. */
                if (0U == instIdx)
                {
                    fvid2_status = Fvid2_control(instParams->drvHandle,
                                           IOCTL_CSIRX_UNREGISTER_EVENT,
                                           (void *)CSIRX_ESM_LOW_EVENT_GROUP_ASF,
                                           NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This failure is reported by the capture driver only in the event of a driver or
                    hardware level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: The status is set to VX_FAILURE and an error message is
                    printed for this Capture instance.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, ": Capture ASF Event Unregister failed!!!\r\n");
                    }
                    /* LDRA_JUSTIFY_END */
                }
#endif
                /* Deleting FVID2 handle */
                /* Freeing memory used for frame drop buf */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach else portion.
                This outcome requires an earlier step in this unit to have failed, which is itself
                out of scope for the video_io test framework as justified at its own location.
                Effect on this unit: If the event unregistration failed, the driver handle of this
                instance would not be deleted.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ((vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    fvid2_status = Fvid2_delete(instParams->drvHandle, NULL);
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This failure is reported by the capture driver only in the event of a driver or
                    hardware level fault, which cannot be stimulated through the video_io component
                    interface.
                    Therefore, this failure case is out of scope for the video_io test framework.
                    Effect on this unit: The status is set to VX_FAILURE and an error message is
                    printed, leaving the driver handle of this instance unchanged.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if (FVID2_SOK != fvid2_status)
                    {
                        status = (vx_status)VX_FAILURE;
                        VX_PRINT(VX_ZONE_ERROR, " CAPTURE: ERROR: FVID2 Delete Failed !!!\n");
                    }
                    /* LDRA_JUSTIFY_END */
                }

                /* Free-ing kernel instance params */
                /* Freeing memory used for frame drop buf */
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach else portion.
                This outcome requires an earlier step in this unit to have failed, which is itself
                out of scope for the video_io test framework as justified at its own location.
                Effect on this unit: If the deletion failed, the driver handle of this instance
                would not be cleared.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if ( (vx_status)VX_SUCCESS == status)
                /* LDRA_JUSTIFY_END */
                {
                    instParams->drvHandle = NULL;
                }
            }
        }

        /* Deleting FVID2 frame Q */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If an instance teardown step failed, the per-channel driver frame
        queues would not be deleted.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            for(chId = 0U; chId < prms->numCh ; chId++)
            {
                (void)tivxQueueDelete(&prms->freeFvid2FrameQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If a preceding cleanup step failed, the per-channel pending frame
        queues would not be deleted.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                (void)tivxQueueDelete(&prms->pendingFrameQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If a preceding cleanup step failed, the pending object array queue
        would not be deleted.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            (void)tivxQueueDelete(&prms->pendingObjArrayQ);
        }

        /* Freeing error object descriptors if they have been allocated */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If a preceding cleanup step failed, the error object descriptors
        allocated for the registered error frame would not be freed.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (((vx_status)VX_SUCCESS == status) &&
            (1U == prms->enableErrorFrameTimeout))
        /* LDRA_JUSTIFY_END */
        {
            for (chId = 0U; chId < prms->numCh; chId++)
            {
                for (bufId = 0U; bufId < TIVX_CAPTURE_MAX_NUM_BUFS; bufId++)
                {
                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach else portion.
                    This condition depends on internal state of this component, which cannot be
                    altered through the video_io component interface.
                    Therefore, this case is out of scope for the video_io test framework.
                    Effect on this unit: A NULL entry would be skipped, leaving the error object
                    descriptor for that channel and buffer unfreed.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if(prms->error_obj_desc[chId][bufId]!=NULL)
                    /* LDRA_JUSTIFY_END */
                    {
                        status = ownObjDescFree((tivx_obj_desc_t**)&prms->error_obj_desc[chId][bufId]);
                    }
                    /* LDRA_JUSTIFY_START
                    <metric start> statement branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach this portion.
                    This condition depends on internal state of this component, which cannot be
                    altered through the video_io component interface.
                    Therefore, this case is out of scope for the video_io test framework.
                    Effect on this unit: The remaining buffers of this channel are skipped and the
                    per-channel loop continues.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((vx_status)VX_SUCCESS != status)
                    {
                        break;
                    }
                    /* LDRA_JUSTIFY_END */
                }
            }
        }

        /* Deleting pending frame Q */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If a preceding cleanup step failed, the per-channel error frame queues
        would not be deleted.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                (void)tivxQueueDelete(&prms->errorFrameQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If a preceding cleanup step failed, the per-channel low timestamp
        queues would not be deleted.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                (void)tivxQueueDelete(&prms->pendingFrameTimestampLoQ[chId]);
            }
        }

        /* Deleting pending frame Q */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If a preceding cleanup step failed, the per-channel high timestamp
        queues would not be deleted.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            for(chId= 0U ; chId < prms->numCh ; chId++)
            {
                (void)tivxQueueDelete(&prms->pendingFrameTimestampHiQ[chId]);
            }
        }

        /* Deleting event */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        This outcome requires an earlier step in this unit to have failed, which is itself out of
        scope for the video_io test framework as justified at its own location.
        Effect on this unit: If a preceding cleanup step failed, the frame-available event would not
        be deleted.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if ((vx_status)VX_SUCCESS == status)
        /* LDRA_JUSTIFY_END */
        {
            (void)tivxEventDelete(&prms->frame_available);
        }
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale: The component level negative test framework and test applications cannot reach
        else portion.
        The kernel instance context used here is the one this component stored during Capture
        creation, and cannot be replaced or invalidated through the video_io component interface.
        Therefore, this failure case is out of scope for the video_io test framework.
        Effect on this unit: If the size did not match, the Capture parameter structure would not be
        freed.
        However, due to the stated rationale, this is not tested.
        <justification end> */
        if (sizeof(tivxCaptureParams) == size)
        /* LDRA_JUSTIFY_END */
        {
            (void)tivxMemFree(prms, sizeof(tivxCaptureParams), (vx_status)TIVX_MEM_EXTERNAL);
        }
    }

    return status;
}

static void tivxCaptureCopyStatistics(tivxCaptureParams *prms,
    tivx_capture_statistics_t *capt_status_prms)
{
    uint32_t i, instIdx, strmIdx;
    tivxCaptureInstParams *instParams;

    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        instParams = &prms->instParams[instIdx];
        for (i = 0U ; i < instParams->numCh ; i++)
        {
            capt_status_prms->queueCount[instIdx][i]      = instParams->captStatus.queueCount[i];
            capt_status_prms->dequeueCount[instIdx][i]    = instParams->captStatus.dequeueCount[i];
            capt_status_prms->dropCount[instIdx][i]       = instParams->captStatus.dropCount[i];
            capt_status_prms->errorFrameCount[instIdx][i] = instParams->captStatus.errorFrameCount[i];
        }
        capt_status_prms->overflowCount[instIdx]         = instParams->captStatus.overflowCount;
        capt_status_prms->spuriousUdmaIntrCount[instIdx] = instParams->captStatus.spuriousUdmaIntrCount;
        capt_status_prms->frontFIFOOvflCount[instIdx]    = instParams->captStatus.frontFIFOOvflCount;
        capt_status_prms->crcCount[instIdx]              = instParams->captStatus.crcCount;
        capt_status_prms->eccCount[instIdx]              = instParams->captStatus.eccCount;
        capt_status_prms->correctedEccCount[instIdx]     = instParams->captStatus.correctedEccCount;
        capt_status_prms->dataIdErrorCount[instIdx]      = instParams->captStatus.dataIdErrorCount;
        capt_status_prms->invalidAccessCount[instIdx]    = instParams->captStatus.invalidAccessCount;
        capt_status_prms->invalidSpCount[instIdx]        = instParams->captStatus.invalidSpCount;
        (void)memset(capt_status_prms->strmFIFOOvflCount[instIdx],
                     0, sizeof(capt_status_prms->strmFIFOOvflCount[instIdx]));
        for (strmIdx = 0U ; strmIdx < CSIRX_NUM_STREAM ; strmIdx++)
        {
            capt_status_prms->strmFIFOOvflCount[instIdx][strmIdx] =
                            instParams->captStatus.strmFIFOOvflCount[strmIdx];
        }
    }
    capt_status_prms->activeChannelMask  = prms->activeChannelMask;
}

static vx_status tivxCaptureGetStatistics(tivxCaptureParams *prms,
    const tivx_obj_desc_user_data_object_t *usr_data_obj)
{
    vx_status                             status = (vx_status)VX_SUCCESS;
    tivx_capture_statistics_t                 *capt_status_prms = NULL;
    void                                  *target_ptr;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach else
    portion.
    This condition depends on internal state of this component, which cannot be altered through the
    video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: A non-NULL user data object is mapped and its size is validated before the
    Capture statistics are copied.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (NULL != usr_data_obj) 
    /* LDRA_JUSTIFY_END */
    {
        target_ptr = tivxMemShared2TargetPtr(&usr_data_obj->mem_ptr);

        tivxCheckStatus(&status, tivxMemBufferMap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));

        if (sizeof(tivx_capture_statistics_t) ==
                usr_data_obj->mem_size)
        {
            capt_status_prms = (tivx_capture_statistics_t *)target_ptr;

            tivxCaptureCopyStatistics(prms, capt_status_prms);
        }
        else
        {
            VX_PRINT(VX_ZONE_ERROR, "Invalid Size \n");
            status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
        }

        tivxCheckStatus(&status, tivxMemBufferUnmap(target_ptr, usr_data_obj->mem_size,
            (vx_enum)VX_MEMORY_TYPE_HOST, (vx_enum)VX_WRITE_ONLY));
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this
    portion.
    This condition depends on internal state of this component, which cannot be altered through the
    video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: An error message is printed and VX_ERROR_INVALID_PARAMETERS is returned.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else
    {
        VX_PRINT(VX_ZONE_ERROR, "User Data Object is NULL \n");
        status = (vx_status)VX_ERROR_INVALID_PARAMETERS;
    }
    /* LDRA_JUSTIFY_END */
    return (status);
}

static uint64_t ownReferenceGetHostRefFromObjDescId(uint16_t obj_desc_id)
{
    tivx_obj_desc_t *obj_desc = ownObjDescGet(obj_desc_id);
    uint64_t ref = 0;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach else
    portion.
    This condition depends on internal state of this component, which cannot be altered through the
    video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: If the object descriptor is not found, the function returns zero instead of
    its host reference.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(obj_desc!=NULL)
    /* LDRA_JUSTIFY_END */
    {
        ref = (uint64_t)obj_desc->host_ref;
    }

    return ref;
}

static vx_status tivxCaptureAllocErrorDesc(tivxCaptureParams *prms,
    tivx_obj_desc_t *obj_desc)
{
    vx_status                             status = (vx_status)VX_SUCCESS;
    uint16_t obj_desc_id = 0U;
    vx_reference ref = NULL;
    uint64_t ref64 = 0;
    uint32_t bufId, chId, planeId;

    ref64 = ownReferenceGetHostRefFromObjDescId(obj_desc->obj_desc_id);
    tivxFlagBitSet(&obj_desc->flags, TIVX_REF_FLAG_IS_INVALID);

    /* Allocate object descriptors */
    for (chId = 0U; chId < prms->numCh; chId++)
    {
        for (bufId = 0U; bufId < TIVX_CAPTURE_MAX_NUM_BUFS; bufId++)
        {
            /* Passing a NULL pointer as "ref" then overwriting it the next line w/ the 64 bit value */
            prms->error_obj_desc[chId][bufId] = ownObjDescAlloc((vx_enum)obj_desc->type, ref);


            if (NULL != prms->error_obj_desc[chId][bufId]) 
            {
                /* Since vx_reference is a 32 bit address, this needs to use the 64 bit value of the host_ref */
                prms->error_obj_desc[chId][bufId]->host_ref = ref64;

                tivxFlagBitSet(&prms->error_obj_desc[chId][bufId]->flags, TIVX_REF_FLAG_IS_INVALID);

                if ((uint32_t)TIVX_OBJ_DESC_RAW_IMAGE == (uint32_t)obj_desc->type)
                {
                    tivx_obj_desc_raw_image_t *tmp_raw_image;
                    tivx_obj_desc_raw_image_t *ref_raw_image;

                    tmp_raw_image = (tivx_obj_desc_raw_image_t *)prms->error_obj_desc[chId][bufId];
                    ref_raw_image = (tivx_obj_desc_raw_image_t *)obj_desc;

                    tmp_raw_image->params.num_exposures      = ref_raw_image->params.num_exposures;
                    tmp_raw_image->params.width              = ref_raw_image->params.width;
                    tmp_raw_image->params.height             = ref_raw_image->params.height;
                    tmp_raw_image->params.line_interleaved   = ref_raw_image->params.line_interleaved;
                    tmp_raw_image->params.meta_height_before = ref_raw_image->params.meta_height_before;
                    tmp_raw_image->params.meta_height_after  = ref_raw_image->params.meta_height_after;

                    tmp_raw_image->create_type       = ref_raw_image->create_type;

                    for (planeId = 0U; planeId < tmp_raw_image->params.num_exposures; planeId++)
                    {
                        tmp_raw_image->mem_ptr[planeId].host_ptr        = ref_raw_image->mem_ptr[planeId].host_ptr;
                        tmp_raw_image->mem_ptr[planeId].mem_heap_region = ref_raw_image->mem_ptr[planeId].mem_heap_region;
                        tmp_raw_image->mem_ptr[planeId].shared_ptr      = ref_raw_image->mem_ptr[planeId].shared_ptr;
                        tmp_raw_image->mem_ptr[planeId].dma_buf_fd      = ref_raw_image->mem_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->img_ptr[planeId].host_ptr        = ref_raw_image->img_ptr[planeId].host_ptr;
                        tmp_raw_image->img_ptr[planeId].mem_heap_region = ref_raw_image->img_ptr[planeId].mem_heap_region;
                        tmp_raw_image->img_ptr[planeId].shared_ptr      = ref_raw_image->img_ptr[planeId].shared_ptr;
                        tmp_raw_image->img_ptr[planeId].dma_buf_fd      = ref_raw_image->img_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->meta_before_ptr[planeId].host_ptr        = ref_raw_image->meta_before_ptr[planeId].host_ptr;
                        tmp_raw_image->meta_before_ptr[planeId].mem_heap_region = ref_raw_image->meta_before_ptr[planeId].mem_heap_region;
                        tmp_raw_image->meta_before_ptr[planeId].shared_ptr      = ref_raw_image->meta_before_ptr[planeId].shared_ptr;
                        tmp_raw_image->meta_before_ptr[planeId].dma_buf_fd      = ref_raw_image->meta_before_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->meta_after_ptr[planeId].host_ptr        = ref_raw_image->meta_after_ptr[planeId].host_ptr;
                        tmp_raw_image->meta_after_ptr[planeId].mem_heap_region = ref_raw_image->meta_after_ptr[planeId].mem_heap_region;
                        tmp_raw_image->meta_after_ptr[planeId].shared_ptr      = ref_raw_image->meta_after_ptr[planeId].shared_ptr;
                        tmp_raw_image->meta_after_ptr[planeId].dma_buf_fd      = ref_raw_image->meta_after_ptr[planeId].dma_buf_fd;

                        tmp_raw_image->params.format[planeId]            = ref_raw_image->params.format[planeId];
                        tmp_raw_image->mem_size[planeId]                 = ref_raw_image->mem_size[planeId];
                        tmp_raw_image->imagepatch_addr[planeId].dim_x    = ref_raw_image->imagepatch_addr[planeId].dim_x;
                        tmp_raw_image->imagepatch_addr[planeId].dim_y    = ref_raw_image->imagepatch_addr[planeId].dim_y;
                        tmp_raw_image->imagepatch_addr[planeId].stride_x = ref_raw_image->imagepatch_addr[planeId].stride_x;
                        tmp_raw_image->imagepatch_addr[planeId].stride_y = ref_raw_image->imagepatch_addr[planeId].stride_y;
                        tmp_raw_image->imagepatch_addr[planeId].scale_x  = ref_raw_image->imagepatch_addr[planeId].scale_x;
                        tmp_raw_image->imagepatch_addr[planeId].scale_y  = ref_raw_image->imagepatch_addr[planeId].scale_y;
                        tmp_raw_image->imagepatch_addr[planeId].step_x   = ref_raw_image->imagepatch_addr[planeId].step_x;
                        tmp_raw_image->imagepatch_addr[planeId].step_y   = ref_raw_image->imagepatch_addr[planeId].step_y;
                    }
                }
                else
                {
                    tivx_obj_desc_image_t *tmp_image;
                    tivx_obj_desc_image_t *ref_image;

                    tmp_image = (tivx_obj_desc_image_t *)prms->error_obj_desc[chId][bufId];
                    ref_image   = (tivx_obj_desc_image_t *)obj_desc;

                    tmp_image->planes        = ref_image->planes;
                    tmp_image->uniform_image_pixel_value = ref_image->uniform_image_pixel_value;
                    tmp_image->width                     = ref_image->width;
                    tmp_image->height                    = ref_image->height;
                    tmp_image->format                    = ref_image->format;
                    tmp_image->color_range               = ref_image->color_range;

                    tmp_image->valid_roi.start_x = ref_image->valid_roi.start_x;
                    tmp_image->valid_roi.start_y = ref_image->valid_roi.start_x;
                    tmp_image->valid_roi.end_x   = ref_image->valid_roi.end_x;
                    tmp_image->valid_roi.end_y   = ref_image->valid_roi.end_y;
                    tmp_image->color_space       = ref_image->color_space;
                    tmp_image->create_type       = ref_image->create_type;

                    for (planeId = 0U; planeId < ref_image->planes; planeId++)
                    {
                        tmp_image->mem_ptr[planeId].host_ptr        = ref_image->mem_ptr[planeId].host_ptr;
                        tmp_image->mem_ptr[planeId].mem_heap_region = ref_image->mem_ptr[planeId].mem_heap_region;
                        tmp_image->mem_ptr[planeId].shared_ptr      = ref_image->mem_ptr[planeId].shared_ptr;
                        tmp_image->mem_ptr[planeId].dma_buf_fd      = ref_image->mem_ptr[planeId].dma_buf_fd;
                        tmp_image->mem_size[planeId]                = ref_image->mem_size[planeId];

                        tmp_image->imagepatch_addr[planeId].dim_x    = ref_image->imagepatch_addr[planeId].dim_x;
                        tmp_image->imagepatch_addr[planeId].dim_y    = ref_image->imagepatch_addr[planeId].dim_y;
                        tmp_image->imagepatch_addr[planeId].stride_x = ref_image->imagepatch_addr[planeId].stride_x;
                        tmp_image->imagepatch_addr[planeId].stride_y = ref_image->imagepatch_addr[planeId].stride_y;
                        tmp_image->imagepatch_addr[planeId].scale_x  = ref_image->imagepatch_addr[planeId].scale_x;
                        tmp_image->imagepatch_addr[planeId].scale_y  = ref_image->imagepatch_addr[planeId].scale_y;
                        tmp_image->imagepatch_addr[planeId].step_x   = ref_image->imagepatch_addr[planeId].step_x;
                        tmp_image->imagepatch_addr[planeId].step_y   = ref_image->imagepatch_addr[planeId].step_y;
                    }
                }

                obj_desc_id = prms->error_obj_desc[chId][bufId]->obj_desc_id;
                (void)tivxQueuePut(&prms->errorFrameQ[chId], (uintptr_t)obj_desc_id, TIVX_EVENT_TIMEOUT_NO_WAIT);
            }
            else
            {
                VX_PRINT(VX_ZONE_ERROR, "Object descriptor allocation failed\n");
                status = (vx_status)VX_FAILURE;
            }
        }
    }

    if ((vx_status)VX_SUCCESS == status)
    {
        prms->enableErrorFrameTimeout = 1U;
    }

    return (status);
}

static vx_status VX_CALLBACK tivxCaptureControl(
       tivx_target_kernel_instance kernel,
       uint32_t node_cmd_id, tivx_obj_desc_t *obj_desc[],
       uint16_t num_params, void *priv_arg)
{
    (void)num_params;
	(void)priv_arg;
    vx_status status = (vx_status)VX_SUCCESS;
    int32_t fvid2_status = FVID2_SOK;
    uint32_t             size, instIdx;
    tivxCaptureParams *prms = NULL;
    tivxCaptureInstParams *instParams;

    void *temp_prms = NULL;
    status = tivxGetTargetKernelInstanceContext(kernel, &temp_prms, &size);
	prms = (tivxCaptureParams *)temp_prms;

    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach this
    portion.
    The kernel instance context used here is the one this component stored during Capture creation,
    and cannot be replaced or invalidated through the video_io component interface.
    Therefore, this failure case is out of scope for the video_io test framework.
    Effect on this unit: The context retrieval error is returned and an error message is printed.
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
    Rationale: The component level negative test framework and test applications cannot reach this
    portion.
    The kernel instance context used here is the one this component stored during Capture creation,
    and cannot be replaced or invalidated through the video_io component interface.
    Therefore, this failure case is out of scope for the video_io test framework.
    Effect on this unit: The function status is set to VX_FAILURE and an error message is printed.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    else if ((NULL == prms) ||
        (sizeof(tivxCaptureParams) != size))
    {
        VX_PRINT(VX_ZONE_ERROR, "Invalid Object Size\n");
        status = (vx_status)VX_FAILURE;
    }
    else
    {
        /*Do nothing*/
    }
    /* LDRA_JUSTIFY_END */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level negative test framework and test applications cannot reach else
    portion.
    This outcome requires an earlier step in this unit to have failed, which is itself out of scope
    for the video_io test framework as justified at its own location.
    Effect on this unit: If context retrieval or validation failed, the requested control command
    would be skipped and the existing failure status returned.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if ((vx_status)VX_SUCCESS == status)
    /* LDRA_JUSTIFY_END */
    {
        switch (node_cmd_id)
        {
            case TIVX_CAPTURE_PRINT_STATISTICS:
            {
                for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                {
                    instParams = &prms->instParams[instIdx];
                    tivxCapturePrintStatus(instParams);
                }
                break;
            }
            case TIVX_CAPTURE_GET_STATISTICS:
            {
                if (NULL != obj_desc[0])
                {
                    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
                    {
                        instParams = &prms->instParams[instIdx];
                        fvid2_status = Fvid2_control(instParams->drvHandle,
                                                IOCTL_CSIRX_GET_INST_STATUS,
                                                &instParams->captStatus,
                                                NULL);

                        /* LDRA_JUSTIFY_START
                        <metric start> statement branch <metric end>
                        <justification start>
                        Rationale: The component level negative test framework and test applications
                        cannot reach this portion.
                        This failure is reported by the capture driver only in the event of a driver
                        or hardware level fault, which cannot be stimulated through the video_io
                        component interface.
                        Therefore, this failure case is out of scope for the video_io test
                        framework.
                        Effect on this unit: The function status is set to VX_FAILURE, an error
                        message is printed, and status retrieval stops for the remaining Capture
                        instances.
                        However, due to the stated rationale, this is not tested.
                        <justification end> */
                        if (FVID2_SOK != fvid2_status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Get status returned failure\n");
                            status = (vx_status)VX_FAILURE;
                            break;
                        }
                        /* LDRA_JUSTIFY_END */
                    }

                    /* LDRA_JUSTIFY_START
                    <metric start> branch <metric end>
                    <justification start>
                    Rationale: The component level negative test framework and test applications
                    cannot reach else portion.
                    This outcome requires an earlier step in this unit to have failed, which is
                    itself out of scope for the video_io test framework as justified at its own
                    location.
                    Effect on this unit: If the driver status query failed, Capture statistics would
                    not be copied to the user data object and the command would return VX_FAILURE.
                    However, due to the stated rationale, this is not tested.
                    <justification end> */
                    if ((vx_status)VX_SUCCESS == status)
                    /* LDRA_JUSTIFY_END */
                    {
                        status = tivxCaptureGetStatistics(prms,
                            (tivx_obj_desc_user_data_object_t *)obj_desc[0U]);
                        if ((vx_status)VX_SUCCESS != status)
                        {
                            VX_PRINT(VX_ZONE_ERROR, "Get status failed\n");
                            status = (vx_status)VX_FAILURE;
                        }
                    }
                    
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "User data object was NULL\n");
                    status = (vx_status)VX_FAILURE;
                }
                break;
            }
            case TIVX_CAPTURE_REGISTER_ERROR_FRAME:
            {
                if ( NULL != obj_desc[0] )
                {
                    if (0U == prms->enableErrorFrameTimeout) 
                    {
                        status = tivxCaptureAllocErrorDesc(prms, obj_desc[0U]);
                    }
                    else
                    {
                        VX_PRINT(VX_ZONE_ERROR, "Reference frame already provided\n");
                        status = (vx_status)VX_FAILURE;
                    }
                }
                else
                {
                    VX_PRINT(VX_ZONE_ERROR, "Provided reference was NULL\n");
                    status = (vx_status)VX_FAILURE;
                }
                break;
            }
#if defined(LDRA_COVERAGE_ENABLED_VIDEO_IO)
            case TIVX_CAPTURE_COVERAGE_START:
            {
                VX_PRINT(VX_ZONE_ERROR,"################## LDRA INIT START  ##################\n");
                ldra_initialize();
                VX_PRINT(VX_ZONE_ERROR,"################## LDRA INIT FINISH ##################\n");
                break;
            }

            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale:  The component level test framework and test applications CAN reach this portion.
            The existing application can reach this part of the code, but given the nature of the LDRA framework
            itself, this part of the code cannot be included in the final coverage.
            Effect on this unit: However, due to the internal behavior and shutdown sequence of the LDRA
            framework, this portion of the code cannot be captured by the LDRA coverage
            instrumentation. Therefore, although executed, these statements are not
            included in the final coverage results.
            <justification end> */
            case TIVX_CAPTURE_COVERAGE_END:
            {
                VX_PRINT(VX_ZONE_ERROR,"################## LDRA TERMINATION  START ##################\n");
                ldra_terminate();
                VX_PRINT(VX_ZONE_ERROR,"################## LDRA TERMINATION FINISH ##################\n");
                break;
            }
            /* LDRA_JUSTIFY_END */
#endif
#if !defined(SOC_J722S) && defined(ASF_TEST_ENABLED_CAPTURE)
            case TIVX_CAPTURE_INJECT_ASF_ERR:
            {
                /* CSIRX0 fatal/non-fatal (ASF) faults route through MAIN ESM
                 * (bits 200/201) before reaching CsirxDrv_asfEsmLowEventIsrFxn
                 * via CSIRX_ESM_LOW_EVENT_GROUP_ASF. Nothing else in the boot
                 * chain enables this forwarding, so it must be armed here
                 * before the test trigger can ever be observed. */
                uint32_t esmBase = (uint32_t)CSL_ESM0_CFG_BASE;
                uint32_t esmBit;

                for (esmBit = 200U; esmBit <= 201U; esmBit++)
                {
                    (void)ESMClearIntrStatus(esmBase, esmBit);
                    (void)ESMSetIntrPriorityLvl(esmBase, esmBit, ESM_INTR_PRIORITY_LEVEL_LOW);
                    (void)ESMEnableIntr(esmBase, esmBit);
                }
                (void)ESMEnableGlobalIntr(esmBase);

                fvid2_status = Fvid2_control(prms->instParams[0U].drvHandle,
                            IOCTL_CSIRX_TRIG_ASF_EVENT,
                            NULL,
                            NULL);

                /* LDRA_JUSTIFY_START
                <metric start> statement branch <metric end>
                <justification start>
                Rationale: The component level negative test framework and test applications cannot
                reach this portion.
                This failure is reported by the capture driver only in the event of a driver or
                hardware level fault, which cannot be stimulated through the video_io component
                interface.
                Therefore, this failure case is out of scope for the video_io test framework.
                Effect on this unit: The status is set to VX_FAILURE and an error message is
                printed, and that status is returned as the result of the
                TIVX_CAPTURE_INJECT_ASF_ERR control command.
                However, due to the stated rationale, this is not tested.
                <justification end> */
                if (FVID2_SOK != fvid2_status)
                {
                    status = (vx_status)VX_FAILURE;
                    VX_PRINT(VX_ZONE_ERROR, ": Failed to trigger ASF event!!!\r\n");
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

void tivxAddTargetKernelCapture(void)
{
    vx_enum self_cpu;
    vx_uint32 i = 0;

    self_cpu = tivxGetSelfCpuId();

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The test framework and test apps cannot reach else portion.
    The CPU identifier evaluated here is obtained from the platform configuration and is not
    controlled by Capture node parameters; covering the other outcome would require executing this
    registration on a different CPU.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: On a different CPU, no Capture target kernels would be registered; this
    void function would not report an error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if(self_cpu == (vx_enum)TIVX_CPU_ID_VIDEO_IO)
    /* LDRA_JUSTIFY_END */
    {
        for (i = 0; i < CAPTURE_NUM_TARGETS; i++)
        {
            vx_capture_target_kernel[i] = tivxAddTargetKernelByName(
                                TIVX_KERNEL_CAPTURE_NAME,
                                target_name[i],
                                tivxCaptureProcess,
                                tivxCaptureCreate,
                                tivxCaptureDelete,
                                tivxCaptureControl,
                                NULL);
        }
    }

}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void tivxRemoveTargetKernelCapture.* <function end>
<justification start>
Rationale: The test framework and test apps cannot reach this portion.
This function is called by the framework during the target kernel de-initialization sequence,
which the test framework does not drive; it cannot be invoked through the video_io component
interface.
Effect on this unit: If the control reaches here, our code base is expected to remove the
registered target kernels and report any failure to the application.
However, due to the stated rationale, this is not tested.
<justification end> */
void tivxRemoveTargetKernelCapture(void)
{
    vx_status status = (vx_status)VX_SUCCESS;
    vx_uint32 i = 0;
    
    for (i = 0; i < CAPTURE_NUM_TARGETS; i++)
    {
        status = tivxRemoveTargetKernel(vx_capture_target_kernel[i]);

        if(status == (vx_status)VX_SUCCESS)
        {
            vx_capture_target_kernel[i] = NULL;
        }
    }
}

static void tivxCaptureGetChannelIndices(const tivxCaptureParams *prms,
                                         uint32_t instId,
                                         uint32_t *startChIdx,
                                         uint32_t *endChIdx)
{
    uint32_t instIdx;

    *startChIdx = 0U;
    *endChIdx   = 0U;

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level test framework and test applications cannot reach this portion.
    The only call site within this component supplies parameters that are always valid, so this
    outcome cannot be produced through the video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: The loop calculates the first node channel index by accumulating the
    channel counts of the preceding Capture instances.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    /* LDRA_JUSTIFY_END */
    {
        /* get start channel ID here */
        if (instIdx == instId)
        {
            break;
        }
        else
        {
            *startChIdx += prms->instParams[instIdx].numCh;
        }
    }

    /* Get last channel ID here */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level test framework and test applications cannot reach else portion.
    The only call site within this component supplies parameters that are always valid, so this
    outcome cannot be produced through the video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: If the instance index were invalid, endChIdx would remain at its
    initialized value of zero; this void function would not report an error.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    if (instIdx < prms->numOfInstUsed)
    /* LDRA_JUSTIFY_END */
    {
        *endChIdx = *startChIdx + prms->instParams[instIdx].numCh;
    }
    
}

static uint32_t tivxCaptureGetNodeChannelNum(const tivxCaptureParams *prms,
                                             uint32_t instId,
                                             uint32_t chId)
{
    uint32_t instIdx, chIdx = 0U;

    /* Get addition of all the channels processed on all previous driver instances */

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale: The component level test framework and test applications cannot reach this portion.
    The only call site within this component supplies parameters that are always valid, so this
    outcome cannot be produced through the video_io component interface.
    Therefore, this case is out of scope for the video_io test framework.
    Effect on this unit: The function returns the node channel index calculated from the channel
    counts of preceding Capture instances and the driver channel index.
    However, due to the stated rationale, this is not tested.
    <justification end> */
    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    /* LDRA_JUSTIFY_END */
    {
        if (instIdx == instId)
        {
            break;
        }
        else
        {
            chIdx += prms->instParams[instIdx].numCh;
        }
    }
    chIdx += chId;

    return (chIdx);
}

static uint32_t tivxCaptureGetDrvInstIndex(const tivxCaptureParams *prms,
                                           uint32_t instId)
{
    uint32_t instIdx, instVal;

    instVal = tivxCaptureMapInstId(instId);

    for (instIdx = 0U ; instIdx < prms->numOfInstUsed ; instIdx++)
    {
        if (prms->instParams[instIdx].instId == instVal)
        {
            /* Found out the index for required instance */
            break;
        }
    }

    return instIdx;
}