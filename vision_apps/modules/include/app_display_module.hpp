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
#ifndef _APP_DISPLAY_MODULE
#define _APP_DISPLAY_MODULE

/**
 * \defgroup group_vision_apps_modules_display Display Node Module
 *
 * \brief This section contains module APIs for the TIOVX Display Node tivxHwaDisplay
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#include "app_modules.h"
#include "app_modules_types.h"
#include <TI/video_io_display.h>

/** \brief Physical screen resolution the display window is centered within. */
constexpr vx_int32 DISPLAY_SCREEN_WIDTH  = 1920;
constexpr vx_int32 DISPLAY_SCREEN_HEIGHT = 1080;

/* DISPLAY_WIDTH/DISPLAY_HEIGHT are also independently defined (same values)
 * in several demo-local app_common.h files; guarded so whichever header is
 * included first wins. */
/** \brief Display width value, can be modified as needed
 *
 */
#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH  (1920)
#endif

/** \brief Display height value, can be modified as needed
 *
 */
#ifndef DISPLAY_HEIGHT
#define DISPLAY_HEIGHT (1080)
#endif

/** \brief Display Module Data Structure
 *
 * Contains the data objects required to use the tivxHwaDisplay node
 *
 */
class DisplayObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Display; }
    /*! Input image (set by app before create) */
    vx_image input_image;

    /*! Display node object */
    vx_node  disp_node;

    /*! Display node user data object for configuration of display node */
    vx_user_data_object disp_params_obj;

    /*! Display node params structure to initialize disp_params_obj object */
    tivx_display_params_t disp_params;

    /*! Display option value; must be set to 1 to use the display */
    vx_int32 display_option;

    vx_int32 out_width;
    vx_int32 out_height;

    /*! Display node graph parameter index */
    vx_int32 graph_parameter_index;

    /*! Name of display module */
    vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! \brief Constructor — sets safe defaults. */
    DisplayObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    DisplayObj(const DisplayObj&)            = delete;
    DisplayObj& operator=(const DisplayObj&) = delete;
    DisplayObj(DisplayObj&&)                 = delete;
    DisplayObj& operator=(DisplayObj&&)      = delete;

    /*! \brief Initialize the module (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the display node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the display node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Whether the display node should actually be created/used —
     *  true iff the app requested display_option==1 AND the target build
     *  has the display node compiled in. Every lifecycle method is a
     *  no-op when this is false (Null Object). */
    bool isDisplayEnabled() const;
};

/* @} */

#endif
