/******************************************************************************
Copyright (c) [2012 - 2019] Texas Instruments Incorporated

All rights reserved not granted herein.

Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 license under copyrights and patents it now or hereafter owns or controls to
 make,  have made, use, import, offer to sell and sell ("Utilize") this software
 subject to the terms herein.  With respect to the foregoing patent license,
 such license is granted  solely to the extent that any such patent is necessary
 to Utilize the software alone.  The patent license shall not apply to any
 combinations which include this software, other than combinations with devices
 manufactured by or for TI ("TI Devices").  No hardware patent is licensed
 hereunder.

 Redistributions must preserve existing copyright notices and reproduce this
 license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

 * No reverse engineering, decompilation, or disassembly of this software
   is permitted with respect to any software provided in binary form.

 * Any redistribution and use are licensed by TI for use only with TI Devices.

 * Nothing shall obligate TI to provide you with source code for the software
   licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution of
 the source code are permitted provided that the following conditions are met:

 * Any redistribution and use of the source code, including any resulting
   derivative works, are licensed by TI for use only with TI Devices.

 * Any redistribution and use of any object code compiled from the source code
   and any resulting derivative works, are licensed by TI for use only with TI
   Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this software
 without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI?S LICENSORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI?S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

/**
 *  \file csitx_transmit_test_cfg.h
 *
 *  \brief CSITX sample application configuration file.
 *
 */

#ifndef CSIRX_TX_TEST_CFG_H_
#define CSIRX_TX_TEST_CFG_H_
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdio.h>
#include <string.h>

#include "ti/osal/osal.h"
#include "ti/osal/TaskP.h"

#include <ti/osal/osal.h>
#include <ti/csl/tistdtypes.h>
#include <ti/csl/csl_types.h>
#include <ti/csl/soc.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/csirx/csirx.h>
#include <ti/drv/csitx/csitx.h>
#include <ti/board/src/devices/common/common.h>
#include <ti/board/board.h>
#include <ti/board/src/devices/board_devices.h>
#include <ti/drv/sciclient/sciclient.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/**< Application name */
#define APP_NAME                        "CSITX_TX_APP"

/**< Enable CSITX to CSITX loopback mode
     0: Disable loopback mode, CSITX will transmit frames continuously
     1: Enable loopback mode, CSITX will transmit and CSIRX will receive
        frames continuously */
#define APP_ENABLE_LOOPBACK                     (1U)
/**< CSITX instance to be used. Valid values are:
      0: For CSITX instance 0
      1: For CSITX instance 1 (Only for J721S2, as J721E has only instance 0) */
#define CSITX_INSTANCE                          (0U)
/**< Number of channels */
#define APP_TX_CH_NUM                           ((uint32_t)8U)
/**< Input Image Data format */
#define APP_TX_IMAGE_DT                         (FVID2_CSI2_DF_RAW12)
/**< Frame storage format. Only valid for RAW12 DT. */
#define APP_TX_IMAGE_STORAGE_FORMAT             (FVID2_CCSF_BITS12_UNPACKED16)
/**< Number of frames to transmit. Set it to '0' for indefinite transmit */
#define APP_TX_FRAMES_TX                        ((uint32_t)1000U)
/**< Number of frames per Tx channel */
#define APP_TX_FRAMES_PER_CH                    ((uint32_t)1U)
/**< Number of frames per Rx channel */
#define APP_RX_FRAMES_PER_CH                    ((uint32_t)4U)
/**< Frame Attribute: Width in pixels */
#define APP_TX_FRAME_WIDTH                      ((uint32_t)1920U)
/**< Frame Attribute: Height in pixels */
#define APP_TX_FRAME_HEIGHT                     ((uint32_t)1080U)
/**< Frame Attribute: Bytes per pixel */
#define APP_TX_FRAME_BPP                        ((uint32_t)2U)
/**< Frame storage format. Only valid for RAW12 DT. */
#define APP_CAPT_IMAGE_STORAGE_FORMAT           (FVID2_CCSF_BITS12_UNPACKED16)

/**< Frame Attribute: Pitch in bytes */
#define APP_TX_FRAME_PITCH                      ((uint32_t)\
                                (APP_TX_FRAME_WIDTH * APP_TX_FRAME_BPP))
/**< Frame Attribute: size in bytes */
#define APP_TX_FRAME_SIZE                                ((uint32_t)\
            (APP_TX_FRAME_HEIGHT * APP_TX_FRAME_WIDTH * APP_TX_FRAME_BPP))

/* Print buffer character limit for prints- UART or CCS Console */
#define APP_PRINT_BUFFER_SIZE                   ((uint32_t)4000)

/** \brief Log enable for CSITX Sample application */
#define CsitxAppTrace                       ((uint32_t) GT_INFO   |\
                                             (uint32_t) GT_TraceState_Enable)

/**< Maximum number of error frame logs to store.
     It stores most recent errors.*/
#define APP_ERR_FRAME_LOG_MAX                                  ((uint32_t)500U)

/**< Print Driver Logs. Set to '1' to enable printing. */
#define APP_PRINT_DRV_LOGS                                     (0U)
/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
/**
 *  \brief Capture application object.
 */
typedef struct
{
    uint32_t instId;
   /**< Csirx Drv Instance ID. */
    Csirx_InitParams initPrms;
   /**< Csirx init time parameters */
    Csirx_CreateParams createPrms;
   /**< Csirx create time parameters */
    Csirx_CreateStatus createStatus;
   /**< Csirx create time status */
    Fvid2_Handle drvHandle;
   /**< FVID2 capture driver handle. */
    Fvid2_CbParams cbPrms;
   /**< Callback params. */
    uint32_t numFramesToCapture;
   /**< Number of frames to receive for a given configuration */
    volatile uint32_t numFramesRcvd;
   /**< Number of frames received */
    uint32_t frameErrorCnt;
   /**< Number of erroneous frames received */
    Fvid2_Frame frames[APP_TX_FRAMES_PER_CH * APP_TX_CH_NUM];
   /**< FVID2 Frames that will be used for capture. */
    Csirx_InstStatus captStatus;
   /**< CSIRX Capture status. */
    uint32_t chFrmCnt[APP_TX_CH_NUM];
   /**< Number of frames captured per channel. */
    uint32_t errFrmCh[APP_ERR_FRAME_LOG_MAX];
   /**< Channel to which error frame belongs. */
    uint32_t errFrmNo[APP_ERR_FRAME_LOG_MAX];
   /**< Error frame number for the channel. */
    uint32_t errFrmTs[APP_ERR_FRAME_LOG_MAX];
   /**< TS in ms. */
} appCaptObj;

/**
 *  \brief Tx application object.
 */
typedef struct
{
    uint32_t instId;
   /**< Csitx Drv Instance ID. */
    Csitx_InitParams initPrms;
   /**< Csitx init time parameters */
    Csitx_CreateParams createPrms;
   /**< Csitx create time parameters */
    Csitx_CreateStatus createStatus;
   /**< Csitx create time status */
    Fvid2_Handle drvHandle;
   /**< FVID2 transmit driver handle. */
    Fvid2_CbParams cbPrms;
   /**< Callback params. */
    uint32_t numFramesToTx;
   /**< Number of frames to transmit for a given configuration */
    volatile uint32_t numFramesTx;
   /**< Number of frames transmitted */
    uint32_t frameErrorCnt;
   /**< Number of erroneous frames transmitted */
    uint32_t maxWidth;
   /**< Max width in pixels - used for buffer allocation for all instance. */
    uint32_t maxHeight;
   /**< Max height in lines - used for buffer allocation for all instance. */
    Fvid2_Frame frames[APP_TX_FRAMES_PER_CH * APP_TX_CH_NUM];
   /**< FVID2 Frames that will be used for transmit. */
    Csitx_InstStatus txStatus;
   /**< CSITX Tx status. */
    uint32_t chFrmCnt[APP_TX_CH_NUM];
   /**< Number of frames transmitted per channel. */
    uint32_t errFrmCh[APP_ERR_FRAME_LOG_MAX];
   /**< Channel to which error frame belongs. */
    uint32_t errFrmNo[APP_ERR_FRAME_LOG_MAX];
   /**< Error frame number for the channel. */
    uint32_t errFrmTs[APP_ERR_FRAME_LOG_MAX];
   /**< TS in ms. */
   appCaptObj captObj;
   /**< CSIRX Capture Object. */
}appTxObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

extern uint8_t gTxFrms[(APP_TX_FRAMES_PER_CH * APP_TX_CH_NUM)][APP_TX_FRAME_SIZE];
extern uint8_t gCompareFrame;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/* None */

#endif /* CSIRX_TX_TEST_CFG_H_ */
