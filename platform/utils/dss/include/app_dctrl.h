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

#ifndef APP_DCTRL_H_
#define APP_DCTRL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**
 * \defgroup group_vision_apps_utils_dctrl Display controller remote service APIs (RTOS only)
 *
 * \brief This section contains APIs for display controller remote service
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/**
 * \brief Remote service for Display Controller
 */
#define APP_DCTRL_REMOTE_SERVICE_NAME         "com.ti.dctrl"

/**
 *  \anchor App_DctrlNodeId
 *  \name DCTRL Node Id
 *
 *  Node ids that are used by the set path to connect different modules and
 *  create a graph
 *
 *  @{
 */
#define APP_DCTRL_NODE_INVALID                (0x0U)
#define APP_DCTRL_NODE_VID1                   (0x1U)
#define APP_DCTRL_NODE_VIDL1                  (0x2U)
#define APP_DCTRL_NODE_VID2                   (0x3U)
#define APP_DCTRL_NODE_VIDL2                  (0x4U)
#define APP_DCTRL_NODE_OVERLAY1               (0x5U)
#define APP_DCTRL_NODE_OVERLAY2               (0x6U)
#define APP_DCTRL_NODE_OVERLAY3               (0x7U)
#define APP_DCTRL_NODE_OVERLAY4               (0x8U)
#define APP_DCTRL_NODE_VP1                    (0x9U)
#define APP_DCTRL_NODE_VP2                    (0xAU)
#define APP_DCTRL_NODE_VP3                    (0xBU)
#define APP_DCTRL_NODE_VP4                    (0xCU)
#define APP_DCTRL_NODE_DPI_DPI0               (0xDU)
#define APP_DCTRL_NODE_DPI_DPI1               (0xEU)
#define APP_DCTRL_NODE_EDP_DPI0               (0xFU)
#define APP_DCTRL_NODE_EDP_DPI1               (0x10U)
#define APP_DCTRL_NODE_EDP_DPI2               (0x11U)
#define APP_DCTRL_NODE_EDP_DPI3               (0x12U)
#define APP_DCTRL_NODE_EDP_MST                (0x13U)
#define APP_DCTRL_NODE_DSI_DPI1               (0x14U)
#define APP_DCTRL_NODE_DSI_DPI2               (0x15U)
#define APP_DCTRL_NODE_DSI_DPI3               (0x16U)
#define APP_DCTRL_NODE_DISCSYNC0              (0x17U)
#define APP_DCTRL_NODE_DISCSYNC1              (0x18U)
#define APP_DCTRL_NODE_WB                     (0x19U)
/* @} */

/** \brief Defines maximum number of edges for allocation. This is derived by
 *         looking at all possible DSS connections in the SoC.
 */
#define APP_DCTRL_MAX_EDGES                   (39U)

/**
 *  \anchor App_DctrlVidStandard
 *  \name Video standards
 *
 *  @{
 */
/** \brief 720x480 30FPS interlaced NTSC standard. */
#define APP_DCTRL_VID_STD_NTSC                      ((uint32_t) 0x00U)
/** \brief 720x576 30FPS interlaced PAL standard. */
#define APP_DCTRL_VID_STD_PAL                       ((uint32_t) 0x01U)
/** \brief 720x480 30FPS interlaced SD standard. */
#define APP_DCTRL_VID_STD_480I                      ((uint32_t) 0x02U)
/** \brief 720x576 30FPS interlaced SD standard. */
#define APP_DCTRL_VID_STD_576I                      ((uint32_t) 0x03U)
/** \brief Interlaced, 360x120 per field NTSC, 360x144 per field PAL. */
#define APP_DCTRL_VID_STD_CIF                       ((uint32_t) 0x04U)
/** \brief Interlaced, 360x240 per field NTSC, 360x288 per field PAL. */
#define APP_DCTRL_VID_STD_HALF_D1                   ((uint32_t) 0x05U)
/** \brief Interlaced, 720x240 per field NTSC, 720x288 per field PAL. */
#define APP_DCTRL_VID_STD_D1                        ((uint32_t) 0x06U)
/** \brief 720x480 60FPS progressive ED standard. */
#define APP_DCTRL_VID_STD_480P                      ((uint32_t) 0x07U)
/** \brief 720x576 60FPS progressive ED standard. */
#define APP_DCTRL_VID_STD_576P                      ((uint32_t) 0x08U)
/** \brief 1280x720 60FPS progressive HD standard. */
#define APP_DCTRL_VID_STD_720P_60                   ((uint32_t) 0x09U)
/** \brief 1280x720 50FPS progressive HD standard. */
#define APP_DCTRL_VID_STD_720P_50                   ((uint32_t) 0x0AU)
/** \brief 1920x1080 30FPS interlaced HD standard. */
#define APP_DCTRL_VID_STD_1080I_60                  ((uint32_t) 0x0BU)
/** \brief 1920x1080 50FPS interlaced HD standard. */
#define APP_DCTRL_VID_STD_1080I_50                  ((uint32_t) 0x0CU)
/** \brief 1920x1080 60FPS progressive HD standard. */
#define APP_DCTRL_VID_STD_1080P_60                  ((uint32_t) 0x0DU)
/** \brief 1920x1080 50FPS progressive HD standard. */
#define APP_DCTRL_VID_STD_1080P_50                  ((uint32_t) 0x0EU)
/** \brief 1920x1080 24FPS progressive HD standard. */
#define APP_DCTRL_VID_STD_1080P_24                  ((uint32_t) 0x0FU)
/** \brief 1920x1080 30FPS progressive HD standard. */
#define APP_DCTRL_VID_STD_1080P_30                  ((uint32_t) 0x10U)
/** \brief 640x480 60FPS VESA standard. */
#define APP_DCTRL_VID_STD_VGA_60                    ((uint32_t) 0x11U)
/** \brief 640x480 72FPS VESA standard. */
#define APP_DCTRL_VID_STD_VGA_72                    ((uint32_t) 0x12U)
/** \brief 640x480 75FPS VESA standard. */
#define APP_DCTRL_VID_STD_VGA_75                    ((uint32_t) 0x13U)
/** \brief 640x480 85FPS VESA standard. */
#define APP_DCTRL_VID_STD_VGA_85                    ((uint32_t) 0x14U)
/** \brief 800x480 60PFS WVGA */
#define APP_DCTRL_VID_STD_WVGA_60                   ((uint32_t) 0x15U)
/** \brief 800x600 60FPS VESA standard. */
#define APP_DCTRL_VID_STD_SVGA_60                   ((uint32_t) 0x16U)
/** \brief 800x600 72FPS VESA standard. */
#define APP_DCTRL_VID_STD_SVGA_72                   ((uint32_t) 0x17U)
/** \brief 800x600 75FPS VESA standard. */
#define APP_DCTRL_VID_STD_SVGA_75                   ((uint32_t) 0x18U)
/** \brief 800x600 85FPS VESA standard. */
#define APP_DCTRL_VID_STD_SVGA_85                   ((uint32_t) 0x19U)
/** \brief 1024x600 70FPS standard. */
#define APP_DCTRL_VID_STD_WSVGA_70                  ((uint32_t) 0x1AU)
/** \brief 1024x768 60FPS VESA standard. */
#define APP_DCTRL_VID_STD_XGA_60                    ((uint32_t) 0x1BU)
/** \brief 1024x768 60FPS VESA standard. Applicable for DSS in 8-bit TDM mode.*/
#define APP_DCTRL_VID_STD_XGA_DSS_TDM_60            ((uint32_t) 0x1CU)
/** \brief 1024x768 72FPS VESA standard. */
#define APP_DCTRL_VID_STD_XGA_70                    ((uint32_t) 0x1DU)
/** \brief 1024x768 75FPS VESA standard. */
#define APP_DCTRL_VID_STD_XGA_75                    ((uint32_t) 0x1EU)
/** \brief 1024x768 85FPS VESA standard. */
#define APP_DCTRL_VID_STD_XGA_85                    ((uint32_t) 0x1FU)
/** \brief 1368x768 60 PFS VESA. */
#define APP_DCTRL_VID_STD_1368_768_60               ((uint32_t) 0x20U)
/** \brief 1366x768 60 PFS VESA. */
#define APP_DCTRL_VID_STD_1366_768_60               ((uint32_t) 0x21U)
/** \brief 1360x768 60 PFS VESA. */
#define APP_DCTRL_VID_STD_1360_768_60               ((uint32_t) 0x22U)
/** \brief 1280x800 30FPS VESA standard. */
#define APP_DCTRL_VID_STD_WXGA_30                   ((uint32_t) 0x23U)
/** \brief 1280x800 60FPS VESA standard. */
#define APP_DCTRL_VID_STD_WXGA_60                   ((uint32_t) 0x24U)
/** \brief 1280x800 75FPS VESA standard. */
#define APP_DCTRL_VID_STD_WXGA_75                   ((uint32_t) 0x25U)
/** \brief 1280x800 85FPS VESA standard. */
#define APP_DCTRL_VID_STD_WXGA_85                   ((uint32_t) 0x26U)
/** \brief 1440x900 60 PFS VESA standard. */
#define APP_DCTRL_VID_STD_1440_900_60               ((uint32_t) 0x27U)
/** \brief 1280x1024 60FPS VESA standard. */
#define APP_DCTRL_VID_STD_SXGA_60                   ((uint32_t) 0x28U)
/** \brief 1280x1024 75FPS VESA standard. */
#define APP_DCTRL_VID_STD_SXGA_75                   ((uint32_t) 0x29U)
/** \brief 1280x1024 85FPS VESA standard. */
#define APP_DCTRL_VID_STD_SXGA_85                   ((uint32_t) 0x2AU)
/** \brief 1680x1050 60 PFS VESA standard. */
#define APP_DCTRL_VID_STD_WSXGAP_60                 ((uint32_t) 0x2BU)
/** \brief 1400x1050 60FPS VESA standard. */
#define APP_DCTRL_VID_STD_SXGAP_60                  ((uint32_t) 0x2CU)
/** \brief 1400x1050 75FPS VESA standard. */
#define APP_DCTRL_VID_STD_SXGAP_75                  ((uint32_t) 0x2DU)
/** \brief 1600x1200 60FPS VESA standard. */
#define APP_DCTRL_VID_STD_UXGA_60                   ((uint32_t) 0x2EU)
/** \brief Interlaced, 2Ch D1, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_2CH_D1                ((uint32_t) 0x2FU)
/** \brief Interlaced, 2ch half D1, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_2CH_HALF_D1           ((uint32_t) 0x30U)
/** \brief Interlaced, 2ch CIF, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_2CH_CIF               ((uint32_t) 0x31U)
/** \brief Interlaced, 4Ch D1, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_4CH_D1                ((uint32_t) 0x32U)
/** \brief Interlaced, 4Ch CIF, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_4CH_CIF               ((uint32_t) 0x33U)
/** \brief Interlaced, 4Ch Half-D1, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_4CH_HALF_D1           ((uint32_t) 0x34U)
/** \brief Interlaced, 8Ch CIF, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_8CH_CIF               ((uint32_t) 0x35U)
/** \brief Interlaced, 8Ch Half-D1, NTSC or PAL. */
#define APP_DCTRL_VID_STD_MUX_8CH_HALF_D1           ((uint32_t) 0x36U)
/** \brief WXGA standard (1280x768) with the aspect ratio 5:3 at 30FPS. */
#define APP_DCTRL_VID_STD_WXGA_5x3_30               ((uint32_t) 0x37U)
/** \brief WXGA resolution (1280x768) with the aspect ratio 5:3 at 60FPS. */
#define APP_DCTRL_VID_STD_WXGA_5x3_60               ((uint32_t) 0x38U)
/** \brief WXGA resolution (1280x768) with the aspect ratio 5:3 at 75FPS. */
#define APP_DCTRL_VID_STD_WXGA_5x3_75               ((uint32_t) 0x39U)
/** \brief Auto-detect standard. Used in capture mode. */
#define APP_DCTRL_VID_STD_AUTO_DETECT               ((uint32_t) 0x3AU)
/** \brief Custom standard used when connecting to external LCD etc...
 *         The video timing is provided by the application.
 */
#define APP_DCTRL_VID_STD_CUSTOM                    ((uint32_t) 0x3BU)
/** \brief Interlaced mode */
#define APP_DCTRL_VID_STD_SF_INTERLACED             ((uint32_t) 0x00U)
/** \brief Progressive mode */
#define APP_DCTRL_VID_STD_SF_PROGRESSIVE            ((uint32_t) 0x01U)
/** \brief Used by driver for validating the input parameters */
#define APP_DCTRL_VID_STD_SF_MAX                    ((uint32_t) 0x02U)

/* @} */

/**
 *  \anchor App_DctrlDVFormat
 *  \name Digital Video Format
 *
 *  @{
 */
/** \brief Video format is BT656 with embedded sync */
#define APP_DCTRL_DV_BT656_EMBSYNC                 ((uint32_t) 0x00U)
/** \brief Video format is BT1120 with embedded sync */
#define APP_DCTRL_DV_BT1120_EMBSYNC                ((uint32_t) 0x01U)
/** \brief Video format is for any discrete sync */
#define APP_DCTRL_DV_GENERIC_DISCSYNC              ((uint32_t) 0x02U)
/* @} */

/**
 *  \anchor App_DctrlVideoIfWidth
 *  \name   Digital video interface width
 *
 *  @{
 */
/** \brief 8-bit interface. */
#define APP_DCTRL_VIFW_8BIT                         ((uint32_t) 0x00U)
/** \brief 10-bit interface. */
#define APP_DCTRL_VIFW_10BIT                        ((uint32_t) 0x01U)
/** \brief 12-bit interface. */
#define APP_DCTRL_VIFW_12BIT                        ((uint32_t) 0x02U)
/** \brief 14-bit interface. */
#define APP_DCTRL_VIFW_14BIT                        ((uint32_t) 0x03U)
/** \brief 16-bit interface. */
#define APP_DCTRL_VIFW_16BIT                        ((uint32_t) 0x04U)
/** \brief 18-bit interface. */
#define APP_DCTRL_VIFW_18BIT                        ((uint32_t) 0x05U)
/** \brief 20-bit interface. */
#define APP_DCTRL_VIFW_20BIT                        ((uint32_t) 0x06U)
/** \brief 24-bit interface. */
#define APP_DCTRL_VIFW_24BIT                        ((uint32_t) 0x07U)
/** \brief 30-bit interface. */
#define APP_DCTRL_VIFW_30BIT                        ((uint32_t) 0x08U)
/** \brief 36-bit interface. */
#define APP_DCTRL_VIFW_36BIT                        ((uint32_t) 0x09U)
/** \brief CSI2 specific - 1 data lanes */
#define APP_DCTRL_VIFW_1LANES                       ((uint32_t) 0x0AU)
/** \brief CSI2 specific - 2 data lanes */
#define APP_DCTRL_VIFW_2LANES                       ((uint32_t) 0x0BU)
/** \brief CSI2 specific - 3 data lanes */
#define APP_DCTRL_VIFW_3LANES                       ((uint32_t) 0x0CU)
/** \brief CSI2 / LVDS specific - 4 data lanes */
#define APP_DCTRL_VIFW_4LANES                       ((uint32_t) 0x0DU)
/** \brief Maximum modes */
#define APP_DCTRL_VIFW_MAX                          ((uint32_t) 0x0EU)
/* @} */

/**
 *  \anchor App_DctrlPolarity
 *  \name   Polarity type
 *
 *  @{
 */
/** \brief Low Polarity. */
#define APP_DCTRL_POL_LOW                    ((uint32_t) 0U)
/** \brief High Polarity. */
#define APP_DCTRL_POL_HIGH                   ((uint32_t) 1U)
/** \brief Used by driver for validating the input parameters. */
#define APP_DCTRL_POL_MAX                    ((uint32_t) 2U)
/* @} */

/**
 *  \anchor App_DctrlEdgePolarity
 *  \name   Edge Polarity type
 *
 *  @{
 */
/** \brief Rising Edge. */
#define APP_DCTRL_EDGE_POL_RISING            ((uint32_t) 0U)
/** \brief Falling Edge. */
#define APP_DCTRL_EDGE_POL_FALLING           ((uint32_t) 1U)
/** \brief Used by driver for validating the input parameters. */
#define APP_DCTRL_EDGE_POL_MAX               ((uint32_t) 2U)
/* @} */

/**
 *  \anchor App_DctrlSyncAlign
 *  \name   HS/VS alignment types
 *
 *  @{
 */
/** \brief HS/VS not aligned. */
#define APP_DCTRL_HVSYNC_ALIGN_OFF           ((uint32_t) 0U)
/** \brief HS/VS aligned. */
#define APP_DCTRL_HVSYNC_ALIGN_ON            ((uint32_t) 1U)
/** \brief Used by driver for validating the input parameters. */
#define APP_DCTRL_HVSYNC_ALIGN_MAX           ((uint32_t) 2U)
/* @} */

/**
 *  \anchor App_DctrlSyncPclkControl
 *  \name   HS/VS pixel clock control types
 *
 *  @{
 */
/** \brief HSYNC and VSYNC are driven on opposite edges of the pixel clock
 *         than pixel data */
#define APP_DCTRL_HVCLK_CTRL_OFF             ((uint32_t) 0U)
/** \brief HSYNC and VSYNC are driven according to hVClkRiseFall value */
#define APP_DCTRL_HVCLK_CTRL_ON              ((uint32_t) 1U)
/** \brief Used by driver for validating the input parameters. */
#define APP_DCTRL_HVCLK_CTRL_MAX             ((uint32_t) 2U)
/* @} */

/**
 *  \anchor App_DctrlOverlayTransColor
 *  \name Overlay Transparency Color Key Selection
 *
 *  @{
 */
/** \brief Destination transparency color key selected */
#define APP_DCTRL_OVERLAY_TRANS_COLOR_DEST             ((uint32_t) 0U)
/** \brief Source transparency color key selected */
#define APP_DCTRL_OVERLAY_TRANS_COLOR_SRC              ((uint32_t) 1U)
/* @} */

/**
 *  \anchor App_DctrlOverlayLayerNum
 *  \name Overlay Layer Number
 *
 *  @{
 */
/** \brief Overlay Layer 0 */
#define APP_DCTRL_OVERLAY_LAYER_NUM_0                     ((uint32_t) 0x0U)
/** \brief Overlay Layer 1 */
#define APP_DCTRL_OVERLAY_LAYER_NUM_1                     ((uint32_t) 0x1U)
/** \brief Overlay Layer 2 */
#define APP_DCTRL_OVERLAY_LAYER_NUM_2                     ((uint32_t) 0x2U)
/** \brief Overlay Layer 3 */
#define APP_DCTRL_OVERLAY_LAYER_NUM_3                     ((uint32_t) 0x3U)
/** \brief Overlay Layer 3 */
#define APP_DCTRL_OVERLAY_LAYER_NUM_4                     ((uint32_t) 0x4U)
/** \brief Maximum overlay layers */
#define APP_DCTRL_OVERLAY_LAYER_MAX                       ((uint32_t) 0x5U)
/** \brief Invalid Overlay Layer */
#define APP_DCTRL_OVERLAY_LAYER_INVALID                   ((uint32_t) 0xFFU)
/* @} */

/**
 *  \anchor App_DctrlCmdId
 *  \name Display contorller CMD IDs to pass to appRemoteServiceRun()
 *
 *  @{
 */

/** \brief Base address for the display controller commands. */
#define APP_DCTRL_CMD_BASE                        ((uint32_t) 0x0U)

/**
 * \brief Command to register the FVID2 Handle for display controller.
 */
#define APP_DCTRL_CMD_REGISTER_HANDLE             (APP_DCTRL_CMD_BASE + 0x1U)

/**
 * \brief Command to unregister the FVID2 Handle for display controller.
 */
#define APP_DCTRL_CMD_DELETE_HANDLE               (APP_DCTRL_CMD_BASE + 0x2U)

/**
 * \brief Command to set the DSS display path configuration.
 */
#define APP_DCTRL_CMD_SET_PATH                    (APP_DCTRL_CMD_BASE + 0x3U)

/**
 * \brief Command to clear the DSS display path configuration.
 */
#define APP_DCTRL_CMD_CLEAR_PATH                  (APP_DCTRL_CMD_BASE + 0x4U)

/**
 * \brief Command to set Video Port configuration.
 */
#define APP_DCTRL_CMD_SET_VP_PARAMS               (APP_DCTRL_CMD_BASE + 0x5U)

/**
 * \brief Command to set configuration of the given overlay.
 */
#define APP_DCTRL_CMD_SET_OVERLAY_PARAMS          (APP_DCTRL_CMD_BASE + 0x6U)

/**
 * \brief Command to set layer/Z-order configuration of the given overlay.
 */
#define APP_DCTRL_CMD_SET_LAYER_PARAMS            (APP_DCTRL_CMD_BASE + 0x7U)

/**
 * \brief Command to disable Video Port.
 */
#define APP_DCTRL_CMD_STOP_VP                     (APP_DCTRL_CMD_BASE + 0x8U)

/**
 * \brief Command to to set advance Video Port configuration.
 */
#define APP_DCTRL_CMD_SET_ADV_VP_PARAMS           (APP_DCTRL_CMD_BASE + 0x9U)

/**
 * \brief Command to to set DSI parameters/configuration.
 */
#define APP_DCTRL_CMD_SET_DSI_PARAMS              (APP_DCTRL_CMD_BASE + 0xAU)

/**
 * \brief Command to query whether DP is connected.
 */
#define APP_DCTRL_CMD_IS_DP_CONNECTED             (APP_DCTRL_CMD_BASE + 0xBU)

/* @} */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief Structure containing edge information. Edge is a connection
 *  between two nodes i.e. two modules. DSS Hardware can be represented
 *  by a graph, where each module is node and edge is present between two
 *  nodes if they are connected.
 */
typedef struct
{
    uint32_t startNode;
    /**< Starting node of the edge */
    uint32_t endNode;
    /**< End node of the edge */
} app_dctrl_edge_info_t;

/**
 * \brief Structure containing DSS path information.
 */
typedef struct
{
    uint32_t numEdges;
    /**< Number edge in the edgeInfo array */
    app_dctrl_edge_info_t edgeInfo[APP_DCTRL_MAX_EDGES];
    /**< List of edges connecting DSS modules. Display controller parses these
     *   edges and enables/disables input/output path in the appropriate DSS
     *   module. This edge tells which module is connected to which module
     *   enabling output in edge start module and input in edge end module.*/
} app_dctrl_path_info_t;

/**
 * \brief Structure containing Video Port information.
 */
typedef struct
{
    uint32_t vpId;
    /**< Video port Id. See \ref App_DssVpId for values */
    uint32_t standard;
    /**< [IN] Standard for which to get the info.
     *   For valid values see \ref App_DctrlVidStandard */
    uint32_t width;
    /**< Active video frame width in pixels. */
    uint32_t height;
    /**< Active video frame height in lines. */
    uint32_t scanFormat;
    /**< Scan format of standard. For valid values see #Fvid2_ScanFormat. */
    uint32_t hFrontPorch;
    /**< Horizontal front porch. Same for both fields in case of interlaced
     *   display. */
    uint32_t hBackPorch;
    /**< Horizontal back porch. */
    uint32_t hSyncLen;
    /**< Horizontal sync length. Same for both fields in case of interlaced
     *   display. */
    uint32_t vFrontPorch;
    /**< Vertical front porch for each field or frame. */
    uint32_t vBackPorch;
    /**< Vertical back porch for each field or frame. */
    uint32_t vSyncLen;
    /**< Vertical sync length for each field. */
    uint32_t pixelClock;
    /**< Pixel clock */
    uint32_t dvoFormat;
    /**< Digital output format. For valid values see \ref App_DctrlDVFormat */
    uint32_t cscRange;
    /**< CSC Range. For valid values see \ref App_DssCscRange */
    uint32_t videoIfWidth;
    /**< Video interface Width, Valid options are<br>
     *   APP_DCTRL_VIFW_12BIT,<br>
     *   APP_DCTRL_VIFW_16BIT,<br>
     *   APP_DCTRL_VIFW_18BIT,<br>
     *   APP_DCTRL_VIFW_24BIT,<br>
     *   APP_DCTRL_VIFW_30BIT,<br>
     *   APP_DCTRL_VIFW_36BIT<br>
     *   For more details see \ref App_DctrlVideoIfWidth */
    uint32_t actVidPolarity;
    /**< Invert output enable
     *   For valid values see \ref App_DctrlPolarity */
    uint32_t pixelClkPolarity;
    /**< Invert pixel clock
     *   For valid values see \ref App_DctrlEdgePolarity */
    uint32_t hsPolarity;
    /**< HSYNC Polarity
     *   For valid values see \ref App_DctrlPolarity */
    uint32_t vsPolarity;
    /**< VSYNC Polarity
     *   For valid values see \ref App_DctrlPolarity */
} app_dctrl_vp_params_t;

/**
 *  \brief Advance Signal Configuration for the LCD
 */
typedef struct
{
    uint32_t vpId;
    /**< Video port Id. See \ref App_DssVpId for values */
    uint32_t hVAlign;
    /**< Defines the alignment between HSYNC and VSYNC assertion.
     *   For valid values see \ref App_DctrlSyncAlign */
    uint32_t hVClkControl;
    /**< HSYNC/VSYNC pixel clock control on/off.
     *   For valid values see \ref App_DctrlSyncPclkControl */
    uint32_t hVClkRiseFall;
    /**< Program HSYNC/VSYNC rise or fall
     *   For valid values see \ref App_DctrlEdgePolarity */
    uint32_t acBI;
    /**< AC bias pin transitions per interrupt.
     *   Value (from 0 to 15) used to specify the number of AC
     *   bias pin transitions */
    uint32_t acB;
    /**< AC bias pin frequency RW 0x00
     *   Value (from 0 to 255) used to specify the number of line
     *   clocks to count before transitioning the AC bias pin. This
     *   pin is used to periodically invert the polarity of the power
     *   supply to prevent DC charge buildup within the display */
    uint32_t vSyncGated;
    /**< VSYNC gated enabled
     *   UFALSE: VSYNC gated disabled
     *   UTRUE: VSYNC gated enabled */
    uint32_t hSyncGated;
    /**< HSYNC gated enabled
     *   UFALSE: HSYNC gated disabled
     *   UTRUE: HSYNC gated enabled */
    uint32_t pixelClockGated;
    /**< Pixel clock gated enabled
     *   UFALSE: Pixel clock gated disabled
     *   UTRUE: Pixel clock gated enabled */
    uint32_t pixelDataGated;
    /**< Pixel data gated enabled
     *   UFALSE: Pixel data gated disabled
     *   UTRUE: Pixel data gated enabled */
    uint32_t pixelGated;
    /**< Pixel gated enable (only for TFT)
     *   UFALSE: Pixel clock always toggles (only in TFT mode)
     *   UTRUE: Pixel clock toggles only when there is valid data to
     *   display (only in TFT mode) */
} app_dctrl_adv_vp_params_t;

/**
 * \brief Structure containing Overlay information.
 */
typedef struct
{
    uint32_t overlayId;
    /**< Overlay Id. See \ref App_DssOverlayId for values */
    uint32_t colorKeyEnable;
    /**< Enable Transparency Color Key
     *   false: Disable Color Key
     *   true:  Enable Color Key */
    uint32_t colorKeySel;
    /**< Transparency Color Key Selection. For valid values
     *   see \ref App_DctrlOverlayTransColor */
    uint32_t transColorKeyMin;
    /**< Minimum Transparency color key value in 24 bit RGB format
     *   31-----24 23-----16 15------8 7-------0
     *     Unused      R          G        B      */
    uint32_t transColorKeyMax;
    /**< Maximum Transparency color key value in 24 bit RGB format
     *   31-----24 23-----16 15------8 7-------0
     *     Unused      R          G        B      */
    uint32_t backGroundColor;
    /**< Background color value in 24 bit RGB format
     *   31-----24 23-----16 15------8 7-------0
     *     Unused      R          G        B      */
} app_dctrl_overlay_params_t;

/**
 * \brief Structure containing Overlay layer information.
 */
typedef struct
{
    uint32_t overlayId;
    /**< Overlay Id. See \ref App_DssOverlayId for values */
    uint32_t pipeLayerNum[APP_DSS_VID_PIPE_ID_MAX];
    /**< Layer to which a particular Video Pipe Id is connected.
     *   For valid values see \ref App_DctrlOverlayLayerNum */
} app_dctrl_layer_params_t;

/**
 * \brief Structure for dsi parameters
 */
typedef struct
{
    uint32_t num_lanes;
    uint32_t lane_speed_in_kbps;
    /**< Total number of output lanes */
} app_dctrl_dsi_params_t;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/**
 *  \brief app_dctrl_path_info_t structure init function.
 *
 *  \param  pathInfo  [IN]Pointer to #app_dctrl_path_info_t structure.
 *
 *  \return None
 */
static inline void appDctrlPathInfoInit(app_dctrl_path_info_t *pathInfo);

/**
 *  \brief app_dctrl_vp_params_t structure init function.
 *
 *  \param  vpParams  [IN] Pointer to #app_dctrl_vp_params_t structure.
 *
 *  \return None
 */
static inline void appDctrlVpParamsInit(app_dctrl_vp_params_t *vpParams);

/**
 *  \brief app_dctrl_adv_vp_params_t structure init function.
 *
 *  \param  advVpParams  [IN] Pointer to #app_dctrl_adv_vp_params_t structure.
 *
 *  \return None
 */
static inline void appDctrlAdvVpParamsInit(app_dctrl_adv_vp_params_t *advVpParams);

/**
 *  \brief app_dctrl_overlay_params_t structure init function.
 *
 *  \param  overlayParams  [IN] Pointer to #app_dctrl_overlay_params_t structure.
 *
 *  \return None
 */
static inline void appDctrlOverlayParamsInit(
    app_dctrl_overlay_params_t *overlayParams);

/**
 *  \brief app_dctrl_layer_params_t structure init function.
 *
 *  \param  layerParams  [IN] Pointer to #app_dctrl_layer_params_t structure.
 *
 *  \return None
 */
static inline void appDctrlLayerParamsInit(
    app_dctrl_layer_params_t *layerParams);

/**
 *  \brief app_dctrl_dsi_params_t structure init function.
 *
 *  \param  prms  [IN] Pointer to #app_dctrl_dsi_params_t structure.
 *
 *  \return None
 */
static inline void appDctrlDsiParamsInit(app_dctrl_dsi_params_t *prms);

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief DCTRL initialization function.
 */
int32_t appDctrlInit(void);

/**
 *  \brief DCTRL de-initialization function.
 */
int32_t appDctrlDeInit(void);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline void appDctrlPathInfoInit(app_dctrl_path_info_t *pathInfo)
{
    uint32_t i;
    if(NULL != pathInfo)
    {
        pathInfo->numEdges = 0U;
        for(i=0U; i<APP_DCTRL_MAX_EDGES; i++)
        {
            pathInfo->edgeInfo[i].startNode = 0U;
            pathInfo->edgeInfo[i].endNode = 0U;
        }
    }
}

static inline void appDctrlVpParamsInit(app_dctrl_vp_params_t *vpParams)
{
    if(NULL != vpParams)
    {
        vpParams->vpId = APP_DSS_VP_ID_1;
        vpParams->standard     = APP_DCTRL_VID_STD_1080P_60;
        vpParams->width        = 1920U;
        vpParams->height       = 1080U;
        vpParams->scanFormat   = APP_DCTRL_VID_STD_SF_PROGRESSIVE;
        vpParams->hFrontPorch  = 88U;
        vpParams->hBackPorch   = 148U;
        vpParams->hSyncLen     = 44U;
        vpParams->vFrontPorch  = 4U;
        vpParams->vBackPorch   = 36U;
        vpParams->vSyncLen     = 5U;
        vpParams->pixelClock   = 148500U;
        vpParams->dvoFormat    = APP_DCTRL_DV_GENERIC_DISCSYNC;
        vpParams->cscRange     = APP_DSS_CSC_RANGE_FULL;
        vpParams->videoIfWidth = APP_DCTRL_VIFW_12BIT;
        vpParams->actVidPolarity   = APP_DCTRL_POL_HIGH;
        vpParams->pixelClkPolarity = APP_DCTRL_EDGE_POL_RISING;
        vpParams->hsPolarity = APP_DCTRL_POL_HIGH;
        vpParams->vsPolarity = APP_DCTRL_POL_HIGH;
    }
}

static inline void appDctrlAdvVpParamsInit(app_dctrl_adv_vp_params_t *advVpParams)
{
    if(NULL != advVpParams)
    {
        advVpParams->vpId = APP_DSS_VP_ID_1;
        advVpParams->hVAlign = APP_DCTRL_HVSYNC_ALIGN_OFF;
        advVpParams->hVClkControl = APP_DCTRL_HVCLK_CTRL_OFF;
        advVpParams->hVClkRiseFall = APP_DCTRL_EDGE_POL_FALLING;
        advVpParams->acBI = 0x0U;
        advVpParams->acB = 0x0U;
        advVpParams->vSyncGated = false;
        advVpParams->hSyncGated = false;
        advVpParams->pixelClockGated = false;
        advVpParams->pixelDataGated = false;
        advVpParams->pixelGated = false;
    }
}

static inline void appDctrlOverlayParamsInit(
    app_dctrl_overlay_params_t *overlayParams)
{
    if(NULL != overlayParams)
    {
        overlayParams->overlayId = APP_DSS_OVERLAY_ID_1;
        overlayParams->colorKeyEnable = false;
        overlayParams->colorKeySel = APP_DCTRL_OVERLAY_TRANS_COLOR_DEST;
        overlayParams->transColorKeyMin = 0x0U;
        overlayParams->transColorKeyMax = 0x0U;
        overlayParams->backGroundColor = 0x0U;
    }
}

static inline void appDctrlLayerParamsInit(
    app_dctrl_layer_params_t *layerParams)
{
    uint32_t i;
    if(NULL != layerParams)
    {
        layerParams->overlayId = APP_DSS_OVERLAY_ID_1;
        for(i=0U; i<APP_DSS_VID_PIPE_ID_MAX; i++)
        {
            layerParams->pipeLayerNum[i] = APP_DCTRL_OVERLAY_LAYER_INVALID;
        }
    }
}

static inline void appDctrlDsiParamsInit(app_dctrl_dsi_params_t *prms)
{
    if (NULL != prms)
    {
        prms->num_lanes = 2u;
    }
}

/* @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef APP_DCTRL_H_ */
