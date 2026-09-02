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

/**
 * \addtogroup group_vision_apps_utils_dctrl
 *
 * @{
 */

#ifndef APP_DSS_DEFAULTS_H_
#define APP_DSS_DEFAULTS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <utils/dss/include/app_dss.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief On-chip eDP/DP display */
#define APP_DSS_DEFAULT_DISPLAY_TYPE_EDP         (0u)

/** \brief Off-chip HDMI display using DPI output from SoC */
#define APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI    (1u)

/** \brief Enables DSI output on AOU LCD Display */
#define APP_DSS_DEFAULT_DISPLAY_TYPE_DSI         (2u)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 * \brief Timing Parameters for display
 */
typedef struct {

    uint32_t width; /**< Width in pixels */
    uint32_t height; /**< Height in lines */
    uint32_t hFrontPorch; /**< Hsync front porch in pixels */
    uint32_t hBackPorch; /**< Hsync back porch in pixels */
    uint32_t hSyncLen; /**< Hsync width in pixels */
    uint32_t vFrontPorch; /**< Vsync front porch in lines */
    uint32_t vBackPorch; /**< Vsync back porch in lines */
    uint32_t vSyncLen; /**< Vsync width in lines */
    uint64_t pixelClock; /**< Pixel clock in Hz */

} app_dss_default_timings_prm_t;

/**
 * \brief Parameters to use to init display
 */
typedef struct {

    uint32_t display_type; /**< value of type, APP_DSS_DEFAULT_DISPLAY_TYPE_* */
    app_dss_default_timings_prm_t timings; /**< Default timings imformation */

    uint32_t enableM2m;
} app_dss_default_prm_t;


/**
 * \brief Parameters to use to init display in dual display mode
 */
typedef struct {

    app_dss_default_prm_t display[2]; /**< parameters of each display */

    uint32_t vid_pipe_to_display_map[APP_DSS_VID_PIPE_ID_MAX]; /**< 0: vid pipe mapped to display 0, 1: vid pipe mapped to display 1 */

    uint32_t enableM2m;
} app_dss_dual_display_default_prm_t;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief Set default parameters to use for appDssDefaultInit()
 */
void appDssDefaultSetDefaultPrm(app_dss_default_prm_t *prm);

/**
 *  \brief DSS initialization wrapper function.
 */
int32_t appDssDefaultInit(app_dss_default_prm_t *prm);

/**
 *  \brief DSS de-initialization wrapper function.
 */
int32_t appDssDefaultDeInit(void);

/**
 *  \brief Set default parameters to use for appDssDualDisplayDefaultInit()
 */
void appDssDualDisplayDefaultSetDefaultPrm(app_dss_dual_display_default_prm_t *prm);

/**
 *  \brief DSS initialization wrapper function for dual display mode.
 */
int32_t appDssDualDisplayDefaultInit(app_dss_dual_display_default_prm_t *prm);

/**
 *  \brief DSS de-initialization wrapper function for dual display mode.
 */
int32_t appDssDualDisplayDefaultDeInit(void);


/* @} */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef APP_DSS_DEFAULTS_H_ */
