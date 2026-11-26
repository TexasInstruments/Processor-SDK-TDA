/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
 *
 * All rights not granted herein.
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
 *       permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 *       licensed and provided to you in object code.
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 *       works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 *       and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
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
 */

/**
 * \defgroup group_vision_apps_modules_ldc LDC Node Module
 *
 * \brief This section contains module APIs for the TIOVX LDC node tivxVpacLdcNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#ifndef _APP_LDC_MODULE_HPP_
#define _APP_LDC_MODULE_HPP_

#include "app_modules_types.h"
#include "app_sensor_module.hpp"
#include <TI/hwa_vpac_ldc.h>
#include "app_modules.h"

/** \brief Default LDC table width.  Can be modified as needed for use case
 *
 */
#define LDC_TABLE_WIDTH     (1920)

/** \brief Default LDC table height.  Can be modified as needed for use case
 *
 */
#define LDC_TABLE_HEIGHT    (1080)

/** \brief Default LDC downscale factor.  Can be modified as needed for use case
 *
 */
#define LDC_DS_FACTOR       (2)

/** \brief Default LDC block width.  Can be modified as needed for use case
 *
 */
#define LDC_BLOCK_WIDTH     (64)

/** \brief Default LDC block height.  Can be modified as needed for use case
 *
 */
#define LDC_BLOCK_HEIGHT    (32)

/** \brief Default LDC pixel padding.  Can be modified as needed for use case
 *
 */
#define LDC_PIXEL_PAD       (1)

/** \brief LDC Module Data Structure
 *
 * Contains the data objects required to use tivxVpacLdcNode
 *
 */
class LDCObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Ldc; }

    /*! LDC node object */
    vx_node node;

    /*! LDC mesh image */
    vx_image mesh_img;

    /*! LDC node params structure to initialize config object */
    tivx_vpac_ldc_params_t params;

    /*! User data object for config parameter, used as node parameter of LDC node */
    vx_user_data_object config;

    /*! LDC mesh params structure to initialize mesh config object */
    tivx_vpac_ldc_mesh_params_t   mesh_params;

    /*! User data object for mesh config, used as node parameter of LDC node */
    vx_user_data_object mesh_config;

    /*! LDC region params structure to initialize region params config object */
    tivx_vpac_ldc_region_params_t region_params;

    /*! User data object for region config, used as node parameter of LDC node */
    vx_user_data_object region_config;

    /*! User data object for DCC config parameter, used as node parameter of LDC node */
    vx_user_data_object dcc_config;

    /*! LDC table width, set to \ref LDC_TABLE_WIDTH  */
    vx_uint32 table_width;

    /*! LDC table height, set to \ref LDC_TABLE_HEIGHT  */
    vx_uint32 table_height;

    /*! LDC downscale factor, set to \ref LDC_DS_FACTOR  */
    vx_uint32 ds_factor;

    /*! Output object array of LDC node  */
    vx_object_array output_arr;

    /*! Flag to enable writing LDC output  */
    vx_int32 en_out_ldc_write;

    /*! Node used to write LDC output */
    vx_node write_node;

    /*! File path used to write LDC node output */
    vx_array file_path;

    /*! File path prefix used to write LDC node output */
    vx_array file_prefix;

    /*! User data object containing write cmd parameters */
    vx_user_data_object write_cmd;

    /*! Output file path for LDC node output */
    vx_char output_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

    /*! Name of LDC module */
    vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! Number of cameras enabled */
    uint32_t num_cameras_enabled;

    /*! Input object array to LDC node (set by graph module before createNode) */
    vx_object_array input_arr;

    /*! Target string for LDC node (set by graph module before createNode) */
    vx_char target[TIVX_TARGET_MAX_NAME];

    /*! \brief Constructor — sets safe defaults (no OpenVX objects created). */
    LDCObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    LDCObj(const LDCObj&)            = delete;
    LDCObj& operator=(const LDCObj&) = delete;
    LDCObj(LDCObj&&)                 = delete;
    LDCObj& operator=(LDCObj&&)      = delete;

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the LDC node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the LDC node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Send write command to LDC write node */
    vx_status sendCmdWriteNode(vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

    /*! \brief LDC lookup table - shared by all instances */
    static const uint8_t ldc_lut[];

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Initialize LDC module (helper). */
    vx_status initLdc(vx_context context, SensorObj *sensorObj);

    /*! \brief Deinitialize LDC module (helper). */
    void deinitLdc();

    /*! \brief Delete LDC node (helper). */
    void deleteLdc();

    /*! \brief Create LDC graph (helper). */
    vx_status createGraphLdc(vx_graph graph, vx_object_array input_arr, const char *target);

    /*! \brief Create LDC write output graph (helper). */
    vx_status createGraphLdcWriteOutput(vx_graph graph);

    /*! \brief Configure DCC parameters (helper). */
    vx_status configureDccParams(vx_context context, SensorObj *sensorObj);

    /*! \brief Configure mesh parameters (helper). */
    vx_status configureMeshParams(vx_context context, SensorObj *sensorObj);

    /*! \brief Configure region parameters (helper). */
    vx_status configureRegionParams(vx_context context, SensorObj *sensorObj);

    /*! \brief Configure LDC parameters (helper). */
    vx_status configureLdcParams(vx_context context, SensorObj *sensorObj);

    /*! \brief Create LDC outputs (helper). */
    vx_status createLdcOutputs(vx_context context, SensorObj *sensorObj);
};

/* @} */

#endif //_APP_LDC_MODULE_HPP_