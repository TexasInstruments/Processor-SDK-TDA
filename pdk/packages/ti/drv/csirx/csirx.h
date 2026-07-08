/*
 *  Copyright (c) Texas Instruments Incorporated 2018-2019
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 *  \defgroup DRV_CSIRX_MODULE CSIRX Driver
 *
 *  @{
 *
 * The CSI2 RX module provides the logic to for handling
 * CSI2 protocol based camera sensor or sensor data stream for capture.
 * This is CSIRX FVID2 driver documentation.
 */
/* @} */

/**
 *  \ingroup DRV_CSIRX_MODULE
 *  \defgroup CSIRX_TOP_LEVEL CSIRX Driver Header
 *            This is CSI2 Rx's top level include for applications.
 *
 *  @{
 */

/**
 *  \file csirx.h
 *
 *  \brief CSIRX Driver API/interface file.
 *
 *  Requirement: NA
 */

#ifndef CSIRX_H_
#define CSIRX_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/csirx/src/csirx/csl_csirx.h>
#include <ti/drv/fvid2/fvid2.h>
#include <ti/drv/csirx/soc/csirx_soc.h>
#include <ti/drv/csirx/include/csirx_cfg.h>
#include <ti/drv/csirx/include/csirx_event.h>
#include <ti/drv/udma/udma.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief CSIRX capture driver ID used at the time of FVID2 create. */
#define CSIRX_CAPT_DRV_ID               (FVID2_CSIRX_DRV_BASE + 0x00000000U)

/* Capture IOCTL's */
/**
 *  \brief Get capture status IOCTL.
 *
 *  This IOCTL can be used to get the capture status like number of frames
 *  queued/dequeued.
 *  Note: These counters will be reset either at the time of driver create or
 *  while starting the capture operation. See respective counter comments for
 *  details.
 *
 *  \param cmdArgs       [OUT] Csirx_InstStatus *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_GET_INST_STATUS     (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0000U)

/**
 *  \brief Print capture debug logs.
 *
 *  This IOCTL can be used to print the capture debug logs if enabled through
 *  CSIRX_DRV_ENABLE_DEBUG.
 *  Note: These logs will be reset at the time of driver create.
 *
 *  \param cmdArgs       [OUT] NULL
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_PRINT_DEBUG_LOGS    (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0100U)

/**
 *  \brief Set CSIRX D-PHY Configuration Parameters.
 *
 *  This IOCTL can be used to set CSIRX D-PHY configuration parameters.
 *
 *  \param cmdArgs       [IN]  Csirx_DPhyCfg *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_SET_DPHY_CONFIG     (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0200U)

/**
 *  \brief Register CSIRX events.
 *
 *  This IOCTL can be used to set enable/register CSIRX events.
 *  Note: This IOCTL should be called after calling Fvid2_create() for
 *        particular instance.
 *
 *  \param cmdArgs       [IN]  Fdrv_Handle
 *  \param cmdArgs       [IN]  CsirxDrv_EventPrms *
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_REGISTER_EVENT     (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0300U)

/**
 *  \brief Un-register CSIRX events.
 *
 *  This IOCTL can be used to set disable/un-register CSIRX events.
 *  Note: This IOCTL should be called after calling Fvid2_delete() for
 *        particular instance.
 *
 *  \param cmdArgs       [IN]  Fdrv_Handle
 *  \param cmdArgs       [IN]  eventGroup
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_UNREGISTER_EVENT   (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0400U)

/**
 *  \brief Get number of channels created for a given driver instance.
 *
 *  This IOCTL can be used to get number of channels created for given instance.
 *  Note: This IOCTL should be called after calling Fvid2_create() for
 *        particular instance.
 *
 *  \param cmdArgs       [IN]  Fdrv_Handle
 *  \param cmdArgs       [IN]  (uint32_t *)Pointer to variable where number of
 *                             channels created for given instance has to stored
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_GET_INST_CH_NUM    (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0500U)

/**
 *  \brief Start a channel for capture. This will start up PSIL thread, UDMA channel.
 *
 *  This IOCTL can be used to start/enable capture channel.
 *  Note: This IOCTL should be called after calling 'Fvid2_create()' and before
 *        'Fvid2_delete()' for particular instance.
 *
 *  \param cmdArgs       [IN]  Fdrv_Handle
 *  \param cmdArgs       [IN]  channel ID
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_CH_START           (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0600U)

/**
 *  \brief Stop a channel for capture. This will stop a PSIL thread, UDMA channel.
 *
 *  This IOCTL can be used to stop/disable capture channel.
 *  Note: This IOCTL should be called after calling 'Fvid2_create()' and before
 *        'Fvid2_delete()' for particular instance.
 *
 *  \param cmdArgs       [IN]  Fdrv_Handle
 *  \param cmdArgs       [IN]  channel ID
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_CH_STOP            (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0700U)
/* @} */

/**
 *  \brief Set testAsfIntr registers to trigger asf events for testing purposes.
 *
 *  This IOCTL can be used to write to testAsfIntr registers to test trigger asf events.
 *  Note: This IOCTL should be called after calling 'Fvid2_create()' and before
 *        'Fvid2_delete()' for particular instance.
 *
 *  \param cmdArgs       [IN]  Fdrv_Handle
 *  \param cmdArgs       [IN]  NULL
 *  \param cmdArgsStatus [OUT] NULL
 *
 *  \return FVID2_SOK on success, else failure
 *
 */
#define IOCTL_CSIRX_TRIG_ASF_EVENT     (FVID2_CSIRX_DRV_IOCTL_BASE + 0x0800U)
/* @} */

/**
 *  \anchor CSIRX_ChType
 *  \name   CSIRX Channel Type
 *  \brief  Id for CSIRX Channel/Stream type.
 *
 *  @{
 */
/** \brief Stream type: capture mode, frames will captured to
  *        internal memory or DDR */
#define CSIRX_CH_TYPE_CAPT                        ((uint32_t) 0x0U)
/** \brief Stream type: OTF mode,
  *        frames will be forwarded to Video Port/VISS */
#define CSIRX_CH_TYPE_OTF                         ((uint32_t) 0x1U)
/** \brief Stream type: Loopback mode,
  *        frames will be forwarded to CSI Tx trans PADs */
#define CSIRX_CH_TYPE_LPBK                        ((uint32_t) 0x2U)
/* @} */

/**
 *  \anchor CSIRX_StreamID
 *  \name   CSIRX Stram ID
 *  \brief  Id for CSIRX Stream
 *
 *  @{
 */
/** \brief Stream type: capture mode, frames will captured to
  *        internal memory or DDR */
#define CSIRX_CAPT_STREAM_ID                       ((uint32_t) 0x0U)
/** \brief Stream type: Loopback mode,
  *        frames will be forwarded to CSI Tx trans PADs */
#define CSIRX_LPBK_STREAM_ID                       ((uint32_t) 0x1U)
/** \brief Stream type: OTF mode,
  *        frames will be forwarded to Video Port/VISS */
#define CSIRX_OTF_STREAM0_ID                       ((uint32_t) 0x2U)
/** \brief Stream type: OTF mode,
  *        frames will be forwarded to Video Port/VISS */
#define CSIRX_OTF_STREAM1_ID                       ((uint32_t) 0x3U)

/* @} */
/** \brief Buffer memory alignment with cache line size. */
#define CSIRX_BUF_ALIGNMENT                       (UDMA_CACHELINE_ALIGNMENT)

/**
 *  \anchor CSIRX_AsfEsmIntrPriority
 *  \name   CSIRX ASF ESM Interrupt Priority
 *  \brief  CSIRX ASF ESM Interrupt Priority
 *
 *  @{
 */
/** \brief ASF ESM Interrupt Type Config */
#define CSIRX_ASF_ESM_INTR_PRIORITY_LEVEL_LOW               ((uint32_t) 0x0U)
/** \brief ASF ESM Interrupt Type Low */
#define CSIRX_ASF_ESM_INTR_PRIORITY_LEVEL_HIGH              ((uint32_t) 0x1U)
/* @} */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \brief Structure containing Channels configurations.
 */
typedef struct
{
    uint32_t chId;
    /**< Channel ID.
     *   It should be unique and start with '0' and continue without any gaps.
     *   It should be less than 'CSIRX_NUM_CH'. */
    uint32_t chType;
    /**< Channel type
     *   See \ref CSIRX_ChType for details. */
    uint32_t vcNum;
    /**< Specify the virtual channel number to be used.
     *   Valid rage 0 - 3 when v2p0 support is disabled and
     *   0 - 15 when v2p0 support is enabled */
    uint32_t inCsiDataType;
    /**< Data Type as per \ref FVID2_CSI2DF */
    Fvid2_Format outFmt;
    /**< Specify the characteristics of streams that has to be received.
     *   Output format described using FVID2 frame format #Fvid2_Format
     *      Valid member of this structure are
     *      <b>width</b>:<br>
     *      Input width, in pixels.
     *      WARNING - Sufficient buffer should be allocated to
     *      accommodate max line length.<br>
     *
     *      <b>height</b>:<br>
     *      Expected number of lines, 0x0 for unknown
     *      WARNING - Sufficient buffer should be allocated to
     *      accommodate max lines.<br>
     *
     *      <b>pitch</b>:<br>
     *      Pitch in bytes between two lines.<br>
     *
     *      <b>dataFormat</b>:<br>
     *      Output Data format, valid options are:<br>
     *      FVID2_DF_YUV422I_UYVY<br>
     *      FVID2_DF_YUV422I_YUYV<br>
     *      FVID2_DF_YUV422I_YVYU<br>
     *      FVID2_DF_YUV422I_VYUY<br>
     *
     *      <b>ccsFormat</b>:<br>
     *      Valid values are :''
     *      FVID2_CCSF_BITS12_PACKED<br>
     *      FVID2_CCSF_BITS12_UNPACKED16<br>
     *      This valid only when inCsiDataType is 'FVID2_CSI2_DF_RAW12' and
     *      when chType is CSIRX_CH_TYPE_CAPT.
     *      It should be 'FVID2_CCSF_BITS12_UNPACKED16' for all
     *      other data types.
     *
     *      <b>Unused Parameters</b>:<br>
     *      .bpp    - unused parameter for CSI2RX<br>
     *      For OTF mode, following parameters are not used:
     *      .bpp, .pitch<br>
     *      For loop-back mode, only .dataFormat is used. Rest of the
     *      parameters are unused.<br> */
    Udma_ChRxPrms rxChParams;
    /**< UDMA Rx channel configuration parameters */
    uint8_t ringOrderId;
    /**<  Ring bus order ID value to be programmed into the orderid field of
     *    the ring's RING_ORDERID register. */
} Csirx_ChCfg;

/**
 *  \brief Structure for transaction timeout and enable/disable config
 *         Used as part of instcfg to be set by the application
 */
typedef struct
{
    uint8_t transToEnable;
    /**<  Asf transaction monitor enable/disable */
    uint16_t transToCtrl;
    /**<  Asf transaction monitor timeout value (in pclk units) */
} Csirx_transToMonCfg;

/**
 * \brief Structure for test trigger asf event
 *        Used as part of instcfg to be set by the application
 */
typedef struct
{
    uint8_t asfIntegrityErrTest;
    /**< Test trigger for ASF integrity error interrupt */
    uint8_t asfProtocolErrTest;
    /**< Test trigger for ASF protocol error interrupt */
    uint8_t asfTransToErrTest;
    /**< Test trigger for ASF transaction timeout error interrupt */
    uint8_t asfCsrErrTest;
    /**< Test trigger for ASF configuration and status registers error interrupt */
    uint8_t asfDapErrTest;
    /**< Test trigger for ASF data and address paths error interrupt */
    uint8_t asfSramUncorrErrTest;
    /**< Test trigger for ASF SRAM uncorrectable error interrupt */
    uint8_t asfSramCorrErrTest;
    /**< Test trigger for ASF SRAM correctable error interrupt */
} Csirx_trigAsfEvtCfg;

/**
 * \brief Structure for test trigger asf event
 *        Used as part of instcfg to be set by the application
 */
typedef struct
{
    uint8_t asfIntegrityErr;
    /**< Test trigger for ASF integrity error interrupt */
    uint8_t asfProtocolErr;
    /**< Test trigger for ASF protocol error interrupt */
    uint8_t asfTransToErr;
    /**< Test trigger for ASF transaction timeout error interrupt */
    uint8_t asfCsrErr;
    /**< Test trigger for ASF configuration and status registers error interrupt */
    uint8_t asfDapErr;
    /**< Test trigger for ASF data and address paths error interrupt */
    uint8_t asfSramUncorrErr;
    /**< Test trigger for ASF SRAM uncorrectable error interrupt */
    uint8_t asfSramCorrErr;
    /**< Test trigger for ASF SRAM correctable error interrupt */
} Csirx_asfFatalNonFatalCfg;

/**
 *  \brief Structure containing csirx module initialization arguments
 */
typedef struct
{
    uint32_t enableCsiv2p0Support;
    /**< Enable support for CSI2 v2p0 protocol
     *   UTRUE: Enables support for CSI2 v2p0 protocol
     *   UFALSE: Disables support for CSI2 v2p0 protocol
     *   In v2p0, support extended for VC, up to 16 virtual channels [4-bits]
     *   and RAW16/20.
     *   Default is up to 4 virtual channels [3-bits] as per CSI2RX v1p3.
     */
    uint32_t numDataLanes;
    /**< Number of data to be used for receiving data */
    uint32_t dataLanesMap[CSIRX_CAPT_DATA_LANES_MAX];
    /**< physiCsirx mapping of logiCsirx data lanes
     *   dlMap[0U]: physiCsirx mapping of logiCsirx data lane 0
     *   dlMap[1U]: physiCsirx mapping of logiCsirx data lane 1
     *   dlMap[2U]: physiCsirx mapping of logiCsirx data lane 2
     *   dlMap[3U]: physiCsirx mapping of logiCsirx data lane 3
     */
    uint32_t enableErrbypass;
    /**< Enable Error Bypass mode. Data will forwarded to stream for further
     *   even after data id, CRC, ECC errors.
     *   UTRUE: Enables error bypass mode.
     *   UFALSE: Disables error bypass mode.
     */
    uint32_t numPixelsStrm0;
    /**< Number of pixels to output per clock cycle from the stream0.
     *   This is stream specific configuration and this is specific to "stream0"
     *   i.e. it is common across all the channels going to DDR/Memory and
     *   all the opened driver instances for given CSI-RX port/instance.
     *
     *   The width of the pixel interface (32 bits) and the bits per pixel for
     *   the selected datatype will determine how many pixels can be output in
     *   a single cycle.
     *
     *   For example:
     *   Case 1: 2 channels - each RAW12 (bpp = 12 bits) capture,
     *           valid values are 0 (1 pixel per clock) & 1 (2 pixels per clock).
     *           12 bits x 2 pixels per clock  = 24 bits which is less
     *           than pixel interface bus width (32 bits)
     *   Case 2: 2 channels- one RAW12 (bpp = 12 bits) channel &
     *                       one RGB888 (bpp = 24 bits) channel capture,
     *           valid values are 0 i.e. 1 pixel per clock (lowest of the two).
     *           RAW12: 12 bits x 2 pixels per clock  = 24 bits which is less
     *           than pixel interface bus width (32 bits)
     *           RGB888: 24 bits x 1 pixel per clock  = 24 bits which is less
     *           than pixel interface bus width (32 bits)
     *           Lowest of the two i.e. 1 pixel per clock is selected in this
     *           case to ensure proper operation of the module.
     *
     *   Valid values are 0, 1, 2.
     *   Default will be 1 pixel per clock (value of '00').
     *   00 -> 1 pixel per clock
     *   01 -> 2 pixels per clock
     *   10 -> 4 pixels per clock
     */
    uint32_t enableStrm[CSIRX_NUM_STREAM];
    /**< Enable/Disable stream interfaces for this CSIRX instance.
     * set enableStrm[stream_id] to 1 to enable the stream. 0 to disable the stream.
     * stream_id could be CSIRX_CAPT_STREAM_ID/CSIRX_LPBK_STRAM_ID/
     *                    CSIRX_OTF_STREAM0_ID/CSIRX_OTF_STREAM1_ID
     * By default, all streams are disabled. Application needs to choose which
     * streams needs to be enabled*/
    Csirx_transToMonCfg transToMonCfg;
    /**< Asf transaction monitor configuration.*/
    Csirx_trigAsfEvtCfg trigAsfTestIntr;
    /**< Trigger ASF test interrupt */
    Csirx_asfFatalNonFatalCfg asfFatalNonFatalCfg;
    /**< ASF fatal non fatal configuration */
} Csirx_InstCfg;

/**
 *  \brief Capture driver create arguments, used when calling Fvid2_create().
 *         Structure containing Streams configurations.
 */
typedef struct
{
    uint32_t        numCh;
    /**< Number of channels to be configured/processed
     *   It should be less that 'CSIRX_NUM_CH'. */
    Csirx_ChCfg     chCfg[CSIRX_NUM_CH];
    /**< Configuration for channels to be processed */
    Csirx_InstCfg   instCfg;
    /**< CSI Rx module configuration.
      *  See \ref Csirx_InstCfg for details */
    uint64_t        frameDropBuf;
    /**< Address of Frame Drop buffer,
      *  used when application has not queued any buffers
      *  If set to NULL (0), then driver will hold
      *  the last buffer for frame repeat */
    uint32_t        frameDropBufLen;
    /**< Frame Drop buffer length in bytes,
      *  used when application has not queued any buffers */
    uint32_t enableChDq;
    /**< Enable/Disable De-queue of frames per channel,
      *  0: Disabled
      *  1:Enabled
      *  When disabled, 'Fvid2_dequeue()' call will de-queue frames for all
      *  channels configured for given instance.
      *  When enabled, 'Fvid2_dequeue()' call will de-queue frames for given
      *  channel ID provided through 'streamId' parameter. */
} Csirx_CreateParams;

/**
 *  \brief Capture driver create arguments, used when calling Fvid2_create().
 *         Structure containing Streams configurations.
 */
typedef struct
{
    Udma_DrvHandle  drvHandle;
    /**< UDMA Driver handle */
} Csirx_InitParams;

/**
 *  \brief Capture driver create status. Returned after calling Fvid2_create().
 */
typedef struct
{
    int32_t retVal;
    /**< Create status, FVID2_SOK on success, else failure. */
} Csirx_CreateStatus;

/**
 *  \brief Capture status structure used to get the current status.
 */
typedef struct
{
    uint32_t queueCount[CSIRX_NUM_CH];
    /**< Counter to keep track of how many requests are queued to the
     *   driver.
     *   Note: This counter will be reset at the time of driver init. */
    uint32_t dequeueCount[CSIRX_NUM_CH];
    /**< Counter to keep track of how many requests are dequeued from the
     *   driver.
     *   Note: This counter will be reset at the time of driver init. */
    uint32_t dropCount[CSIRX_NUM_CH];
    /**< Counter to keep track of how many frames are dropped from the
     *   driver when no buffers are queued by the application.
     *   Note: This counter will be reset at the time of driver init. */
    uint32_t errorFrameCount[CSIRX_NUM_CH];
    /**< Counter to keep track number of error or incomplete frames
     *   from the driver.
     *   Note: This counter will be reset at the time of driver init. */
    uint32_t overflowCount;
    /**< Counter to keep track of the occurrence of overflow error.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t spuriousUdmaIntrCount;
    /**< Counter to keep track of the occurrences of spurious UDMA interrupts.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t frontFIFOOvflCount;
    /**< Counter to keep track of the occurrences of Front FIFO Overflow.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t crcCount;
    /**< Counter to keep track of the occurrences of CRC errors.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t eccCount;
    /**< Counter to keep track of the occurrences of un-corrected ECC errors.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t correctedEccCount;
    /**< Counter to keep track of the occurrences of corrected ECC errors.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t dataIdErrorCount;
    /**< Counter to keep track of the occurrences of Data ID errors.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t invalidAccessCount;
    /**< Counter to keep track of the occurrences of Invalid accesses.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t invalidSpCount;
    /**< Counter to keep track of the occurrences of reception of
     *   invalid short packet.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t strmFIFOOvflCount[CSIRX_NUM_STREAM];
    /**< Counter to keep track of the occurrences of Front FIFO Overflow.
     *   Note: This counter will be reset at the time of driver create and
     *   during driver start. */
    uint32_t vp0ErrLnFrm;
    /**< Counter to keepp track of the occurrences of Line/Frame errors in Video
     * port 0. */
    uint32_t vp1ErrLnFrm;
    /**< Counter to keepp track of the occurrences of Line/Frame errors in Video
     * port 1. */
    uint32_t psilFifoOverflow;
    /**< Counter to keepp track of the occurrences of PSIL fifo overflow errors */
    uint32_t deSkewEntryCount;
    /**< Counter to keep track of the occurrences of De-skew calibration event */
    uint32_t asfTransToEvent;
    /**< Additional Safety Feature to detect transaction monitor timeout event */
} Csirx_InstStatus;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/**
 *  \brief CSIRX Driver Init function.
 *
 *  \param initParams       [IN] Pointer to #Csirx_InitParams structure.
 *
 *  \return status CSIRX Initialization status. 'FVID2_SOK' is successful.
 */
int32_t Csirx_init(const Csirx_InitParams *initParams);

/**
 *  \brief CSIRX Driver de-Init function.
 *
 *  \return status CSIRX Initialization status. 'FVID2_SOK' is successful.
 */
int32_t Csirx_deInit(void);
/* ========================================================================== */
/*      Internal Function Declarations (Needed for other static inlines)      */
/* ========================================================================== */

/**
 *  \brief Csirx_InitParams structure init function.
 *
 *  \param initPrms         [IN] Pointer to #Csirx_InitParams structure.
 *
 */
static inline void Csirx_initParamsInit(Csirx_InitParams *initPrms);

/**
 *  \brief Csirx_CreateParams structure init function.
 *
 *  \param createPrms   [IN] Pointer to #Csirx_CreateParams structure.
 *
 */
static inline void Csirx_createParamsInit(Csirx_CreateParams *createPrms);

/**
 *  \brief Csirx_ChCfg structure init function.
 *
 *  \param chCfg            [IN] Pointer to #Csirx_ChCfg structure.
 *
 */
static inline void Csirx_chCfgInit(Csirx_ChCfg *chCfg);

/**
 *  \brief Csirx_InstCfg structure init function.
 *
 *  \param instCfg          [IN] Pointer to #Csirx_InstCfg structure.
 *
 */
static inline void Csirx_instCfgInit(Csirx_InstCfg *instCfg);

/**
 *  \brief Csirx_InstStatus structure init function.
 *
 *  \param status           [IN] Pointer to #Csirx_InstStatus structure.
 *
 */
static inline void Csirx_instStatusInit(Csirx_InstStatus *status);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline void Csirx_initParamsInit(Csirx_InitParams *initPrms)
{
    initPrms->drvHandle = NULL;
}

static inline void Csirx_createParamsInit(Csirx_CreateParams *createPrms)
{
    uint32_t loopCnt;

    if (NULL != createPrms)
    {
        createPrms->numCh = 0U;
        for (loopCnt = 0U ; loopCnt < CSIRX_NUM_CH ; loopCnt++)
        {
            Csirx_chCfgInit(&createPrms->chCfg[loopCnt]);
        }
        Csirx_instCfgInit(&createPrms->instCfg);
        createPrms->frameDropBufLen = 0U;
        createPrms->frameDropBuf    = 0U;
        createPrms->enableChDq      = 0U;
    }
}

static inline void Csirx_chCfgInit(Csirx_ChCfg *chCfgPrms)
{
    chCfgPrms->chId = CSIRX_CH_ID_DEFAULT;
    chCfgPrms->chType = CSIRX_CH_TYPE_CAPT;
    chCfgPrms->vcNum = CSIRX_VC_NUM_DEFAULT;
    chCfgPrms->inCsiDataType = CSIRX_IN_DT_DEFAULT;
    Fvid2Format_init(&chCfgPrms->outFmt);
    chCfgPrms->outFmt.width = 0U;
    chCfgPrms->outFmt.height = 0U;
    Fvid2Utils_memset(&chCfgPrms->outFmt.pitch[0U],
           0x0,
           sizeof (chCfgPrms->outFmt.pitch));

    /* Initialization of UDMA channel params */
    chCfgPrms->ringOrderId = UDMA_DEFAULT_RING_ORDER_ID;
    UdmaChRxPrms_init(&chCfgPrms->rxChParams, UDMA_CH_TYPE_RX);
    chCfgPrms->rxChParams.chanType    = (uint8_t)CSL_UDMAP_CHAN_TYPE_REF_TR_RING;
    /* Make CSI2RX DMA channels as highest priority,
       it is assumed that order 8-15 is mapped as RT at system level */
    chCfgPrms->rxChParams.busPriority = 0U;
    chCfgPrms->rxChParams.busQos      = 0U;
#if defined (SOC_J721E)
    chCfgPrms->rxChParams.busOrderId  = 8U;
#elif defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
    chCfgPrms->rxChParams.busOrderId  = 10U;
#endif
    chCfgPrms->rxChParams.dmaPriority =
                                TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_HIGH;
}

static inline void Csirx_instCfgInit(Csirx_InstCfg *modCfgPrms)
{
    uint32_t loopCnt;
    Csirx_transToMonCfg* transToMonitorCfg;
    Csirx_trigAsfEvtCfg* trigAsfTestIntrCfg;
    Csirx_asfFatalNonFatalCfg* asfFatalNonFatalCfg;

    transToMonitorCfg = &modCfgPrms->transToMonCfg;
    trigAsfTestIntrCfg = &modCfgPrms->trigAsfTestIntr;
    asfFatalNonFatalCfg = &modCfgPrms->asfFatalNonFatalCfg;

    modCfgPrms->enableCsiv2p0Support = UTRUE;
    modCfgPrms->numDataLanes         = CSIRX_CAPT_DATA_LANES_MAX;
    modCfgPrms->enableErrbypass      = UTRUE;
    modCfgPrms->numPixelsStrm0       = 0U;
    for(loopCnt = 0U ; loopCnt < CSIRX_CAPT_DATA_LANES_MAX ; loopCnt++)
    {
        modCfgPrms->dataLanesMap[loopCnt] = (loopCnt + 1U);
    }
    for(loopCnt = 0U ; loopCnt < CSIRX_NUM_STREAM ; loopCnt++)
    {
        modCfgPrms->enableStrm[loopCnt] = 0U;
    }

    transToMonitorCfg->transToCtrl   = 0U;
    transToMonitorCfg->transToEnable = 0U;

    trigAsfTestIntrCfg->asfIntegrityErrTest  = 0U;
    trigAsfTestIntrCfg->asfProtocolErrTest   = 0U;
    trigAsfTestIntrCfg->asfTransToErrTest    = 0U;
    trigAsfTestIntrCfg->asfCsrErrTest        = 0U;
    trigAsfTestIntrCfg->asfDapErrTest        = 0U;
    trigAsfTestIntrCfg->asfSramUncorrErrTest = 0U;
    trigAsfTestIntrCfg->asfSramCorrErrTest   = 0U;

    asfFatalNonFatalCfg->asfIntegrityErr     = 0U;
    asfFatalNonFatalCfg->asfProtocolErr      = 0U;
    asfFatalNonFatalCfg->asfTransToErr       = 0U;
    asfFatalNonFatalCfg->asfCsrErr           = 0U;
    asfFatalNonFatalCfg->asfDapErr           = 0U;
    asfFatalNonFatalCfg->asfSramUncorrErr    = 0U;
    asfFatalNonFatalCfg->asfSramCorrErr      = 0U;
}

static inline void Csirx_instStatusInit(Csirx_InstStatus *captStatus)
{
    uint32_t loopCnt, strmIdx;

    captStatus->overflowCount         = 0U;
    captStatus->spuriousUdmaIntrCount = 0U;
    captStatus->spuriousUdmaIntrCount = 0U;
    captStatus->frontFIFOOvflCount    = 0U;
    captStatus->crcCount              = 0U;
    captStatus->eccCount              = 0U;
    captStatus->correctedEccCount     = 0U;
    captStatus->deSkewEntryCount      = 0U;
    captStatus->dataIdErrorCount      = 0U;
    captStatus->invalidAccessCount    = 0U;
    captStatus->invalidSpCount        = 0U;
    for (strmIdx = 0U ; strmIdx < CSIRX_NUM_STREAM ; strmIdx++)
    {
        captStatus->strmFIFOOvflCount[strmIdx]= 0U;
    }
    for(loopCnt = 0U ; loopCnt < CSIRX_NUM_CH ; loopCnt++)
    {
        captStatus->queueCount[loopCnt]      = 0U;
        captStatus->dequeueCount[loopCnt]    = 0U;
        captStatus->dropCount[loopCnt]       = 0U;
        captStatus->errorFrameCount[loopCnt] = 0U;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef CSIRX_H_ */
/* @} */
