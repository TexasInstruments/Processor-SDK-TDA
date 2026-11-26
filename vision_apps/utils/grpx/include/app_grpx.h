/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#ifndef APP_GRPX_H_
#define APP_GRPX_H_

/**
 * \defgroup group_vision_apps_utils_grpx Graphics overlay utility APIs
 *
 * \brief This section contains APIs to create and draw graphics overlays for demos
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

#include <stdint.h>
#include <utils/draw2d/include/draw2d.h>
#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief User callback that is invoked every 'update_interval' msecs
 *
 *        Note, screen is cleared by before calling this API in case update_type is 'Full update'
 *
 * \param [in] draw2dObj       Draw2D library handle, user can use this handle to draw text, bitmaps on the screen
 * \param [in] draw2dBufInfo   Buffer information into which the drawing happens
 * \param [in] update_type     0: Full update, i.e draw all content, 1: partial update, only draw content updated from last invocation
 *
 */
typedef void (*app_grpx_draw_f)(Draw2D_Handle *handle, Draw2D_BufInfo *draw2dBufInfo, uint32_t update_type);

/**
 * \brief Init parameters
 */
typedef struct {

    uint32_t width; /*!< width of buffer in memory as well as window width on screen, currently these are the same */
    uint32_t height; /*!< width of buffer in memory as well as window width on screen, currently these are the same */
    uint32_t pos_x; /*!< position of window on screen */
    uint32_t pos_y; /*!< position of window on screen */
    uint32_t display_pipe; /*!< 0:VID1, 1:VIDL1, 2:VID2 and 3:VIDL2 */
    uint32_t update_interval; /*!< graphics update interval in msecs */
    app_grpx_draw_f draw_callback; /*!< User callback that is invoked for every 'update_interval' msecs,
                                      when NULL a default graphics layout is drawn with TI logo, CPU load */
    vx_context  openvx_context; /*!< OpenVX context to use for creating display node */

} app_grpx_init_prms_t;

/**
 * \brief Init graphics overlay
 *
 *        This also statrts showing the graphics overlay on the screen
 *
 * \param [in] prm  Init parameters
 */
int32_t appGrpxInit(app_grpx_init_prms_t *prm);

/**
 * \brief Set default parameters for init
 *
 *        Recommend to call this before calling appGrpxInit() to set defaults and
 *        then override the parameters based on use-case.
 */
void appGrpxInitParamsInit(app_grpx_init_prms_t *prm, vx_context context);


/**
 * \brief Deinit graphics overlay
 *
 *        This also stops showing the graphics overlay on the screen
 *        Make sure OpenVX context is deleted only after this is called.
 */
int32_t appGrpxDeInit();


/**
 * \brief Get CPU load widget box dimensions
 *
 * \param [out] width  Width of the generated graphics, can be used by user to position the graphics
 * \param [out] height Height of the generated graphics, can be used by user to position the graphics
 *
 */
int32_t appGrpxGetDimCpuLoad(uint16_t *width, uint16_t *height);

/**
 * \brief Show CPU load
 *
 *        Utility API to get and draw CPU load on the graphics overlay.
 *
 *
 *        This API is provided as a convinience so that all users can
 *        use this and not have to contruct the CPU load graphics in their
 *        own way.
 *
 *        This API can be called in the draw callback function registered
 *        during init.
 *
 * \param [in] startx Position of the graphics
 * \param [in] starty Position of the graphics
 */
int32_t appGrpxShowCpuLoad(uint16_t startx, uint16_t starty);


/**
 * \brief Get Logo widget box dimensions
 *
 * \param [out] width  Width of the generated graphics, can be used by user to position the graphics
 * \param [out] height Height of the generated graphics, can be used by user to position the graphics
 *
 */
int32_t appGrpxGetDimLogo(uint16_t *width, uint16_t *height);

/**
 * \brief Show TI logo
 *
 *        Utility API to draw TI logo on the graphics overlay.
 *
 *        Other constraints and paramters same as appGrpxShowLogo()
 */
int32_t appGrpxShowLogo(uint16_t startx, uint16_t starty);

/**
 * \brief Get VHWA load widget box dimensions
 *
 * \param [out] width  Width of the generated graphics, can be used by user to position the graphics
 * \param [out] height Height of the generated graphics, can be used by user to position the graphics
 *
 */
int32_t appGrpxGetDimHwaLoad(uint16_t *width, uint16_t *height);

/**
 * \brief Show Vision HWA load
 *
 *        Utility API to get and draw Vision HWA load on the graphics overlay.
 *
 *        Other constraints and paramters same as appGrpxShowLogo()
 */
int32_t appGrpxShowHwaLoad(uint16_t startx, uint16_t starty);

/**
 * \brief Get DDR load widget box dimensions
 *
 * \param [out] width  Width of the generated graphics, can be used by user to position the graphics
 * \param [out] height Height of the generated graphics, can be used by user to position the graphics
 *
 */
int32_t appGrpxGetDimDdrLoad(uint16_t *width, uint16_t *height);

/**
 * \brief Show DDR load
 *
 *        Utility API to get and draw DDR load on the graphics overlay.
 *
 *        Other constraints and paramters same as appGrpxShowLogo()
 */
int32_t appGrpxShowDdrLoad(uint16_t startx, uint16_t starty);


/**
 * \brief Draw default layout
 *
 *        Utility API to draw basic layout of TI logo, CPU, HWA, DDR loads
 *
 *        Other constraints and paramters same as app_grpx_draw_f()
 */
void appGrpxDrawDefault(Draw2D_Handle *draw2d_obj, Draw2D_BufInfo *draw2d_buf_info, uint32_t update_type);

/* @} */

#ifdef __cplusplus
}
#endif

#endif
