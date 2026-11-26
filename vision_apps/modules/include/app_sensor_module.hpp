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
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 *       works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 *       and any resulting derivative works, are licensed by TI for use only with TI Devices.
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
 * \defgroup group_vision_apps_modules_sensor Sensor Module
 *
 * \brief This section contains module APIs for the image sensor framework
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#ifndef _APP_SENSOR_MODULE_HPP_
#define _APP_SENSOR_MODULE_HPP_

#include "app_modules_types.h"

extern "C" {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"
#include <utils/iss/include/app_iss.h>
#pragma GCC diagnostic pop
}

/** \brief Use case config value corresponding to ISS_SENSOR_FEATURE_CFG_UC0
 *
 */
#define APP_SENSOR_FEATURE_CFG_UC0 (0)

/** \brief Use case config value corresponding to ISS_SENSOR_FEATURE_CFG_UC1
 *
 */
#define APP_SENSOR_FEATURE_CFG_UC1 (1)

/** \brief Use case config value corresponding to ISS_SENSOR_FEATURE_CFG_UC2
 *
 */
#define APP_SENSOR_FEATURE_CFG_UC2 (2)

/** \brief Sensor Module Data Structure
 *
 * Contains the data objects required to use sensor framework API's
 *
 */
class SensorObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Sensor; }

    /*! Sensor parameters structure */
    IssSensor_CreateParams sensorParams;

    /*! DCC configuration data structure */
    vx_user_data_object dcc_config;

    /*! Array of strings of available sensor names */
    char availableSensorNames[ISS_SENSORS_MAX_SUPPORTED_SENSOR][ISS_SENSORS_MAX_NAME];

    /*! String of sensor name being used */
    vx_char sensor_name[ISS_SENSORS_MAX_NAME];

    /*! Number of sensors detected by sensor framework */
    uint8_t num_sensors_found;

    /*! Number of sensors detected by sensor framework */
    uint32_t sensor_features_enabled;

    /*! Supported sensor features */
    uint32_t sensor_features_supported;

    /*! Flag to indicate if sensor DCC is enabled */
    uint32_t sensor_dcc_enabled;

    /*! Flag to indicate if sensor WDR is enabled */
    uint32_t sensor_wdr_enabled;

    /*! Flag to indicate if sensor exposure control is enabled */
    uint32_t sensor_exp_control_enabled;

    /*! Flag to indicate if sensor gain control is enabled */
    uint32_t sensor_gain_control_enabled;

    /*! Number of cameras enabled */
    uint32_t num_cameras_enabled;

    uint32_t ch_mask;

    /*! Flag to whether or not interactive mode is enabled */
    vx_int32 is_interactive;

    /*! Flag to indicate if LDC is enabled */
    vx_int32 enable_ldc;

    /*! Sensor index value */
    vx_int32 sensor_index;

    /*! Use case option flag */
    vx_int32 usecase_option;

    /*! Image width value */
    vx_int32 image_width;

    /*! Image height value */
    vx_int32 image_height;

    /*! Output format : 0=RAW, 1=YUV */
    uint32_t sensor_out_format;

    /*! \brief Constructor — sets safe defaults (no OpenVX objects created). */
    SensorObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    SensorObj(const SensorObj&)            = delete;
    SensorObj& operator=(const SensorObj&) = delete;
    SensorObj(SensorObj&&)                 = delete;
    SensorObj& operator=(SensorObj&&)      = delete;

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the sensor node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the sensor node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Query sensor and populate sensor parameters
     *
     * This function queries the user for which sensor to use and populates sensor object
     * parameters based on the chosen sensor; also supports automatic detection of available sensors
     */
    vx_status querrySensor();

    /*! \brief Start image sensor streaming. Call after verifyGraph(). */
    vx_status startSensor();

    /*! \brief Stop image sensor streaming. Call before deinit. */
    vx_status stopSensor();

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();
};

/* @} */

#endif //_APP_SENSOR_MODULE_HPP_