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
 * \defgroup group_vision_apps_modules_capture Capture Node Module
 *
 * \brief This section contains module APIs for the TIOVX Capture node tivxHwaCapture
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#ifndef _APP_CAPTURE_MODULE_HPP_
#define _APP_CAPTURE_MODULE_HPP_

#include "app_modules_types.h"
#include "app_sensor_module.hpp"
#include <TI/video_io_capture.h>

/** \brief Number of capture channels that are used by this module.  Can be modified as needed
 *
 */
#define NUM_CAPT_CHANNELS   (4u)

// Note: ImgObj class is defined in app_sensor_module.hpp (included above)

/** \brief Capture Module Data Structure
 *
 * Contains the data objects required to use the tivxHwaCapture node
 *
 */
class CaptureObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Capture; }

    /*! Capture node object */
    vx_node node;

    /*! User data object for config parameter, used as node parameter of Capture node */
    vx_user_data_object config;

    /*! Capture node params structure to initialize config object */
    tivx_capture_params_t params;

    /*! Capture node object array output */
    vx_object_array raw_image_arr[APP_MODULES_MAX_BUFQ_DEPTH];

    /*! Capture node graph parameter index */
    vx_int32 graph_parameter_index;

    /*! Flag to indicate whether or not the intermediate output is written */
    vx_int32 en_out_capture_write;

    /*! Flag to enable test mode */
    vx_int32 test_mode;

    /*! Node used to write capture output */
    vx_node write_node;

    /*! File path used to write capture node output */
    vx_array file_path;

    /*! File path prefix used to write capture node output */
    vx_array file_prefix;

    /*! User data object containing write cmd parameters */
    vx_user_data_object write_cmd;

    /*! Output file path for capture node output */
    vx_char output_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

    /*! Name of capture module */
    vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! Raw image used when camera gets disconnected */
    tivx_raw_image error_frame_raw_image;

    /*! Flag to enable detection of camera disconnection */
    vx_uint8 enable_error_detection;

    /*! Capture node format, taken from sensor_out_format */
    vx_uint32 capture_format;

    /*! Buffer queue depth */
    vx_int32 bufq_depth;

    /*! \brief Constructor — sets safe defaults (no OpenVX objects created). */
    CaptureObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    CaptureObj(const CaptureObj&)            = delete;
    CaptureObj& operator=(const CaptureObj&) = delete;
    CaptureObj(CaptureObj&&)                 = delete;
    CaptureObj& operator=(CaptureObj&&)      = delete;

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the capture node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the capture node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Send write command to capture write node */
    vx_status sendCmdWriteNode(vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

    /*! \brief Send error frame to capture node */
    vx_status sendErrorFrame();

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Configure capture parameters (helper). */
    vx_status configureParams(vx_context context, SensorObj *sensorObj);

    /*! \brief Create capture output objects (helper). */
    vx_status createOutput(vx_context context, SensorObj *sensorObj);

    /*! \brief Create error detection frame (helper). */
    vx_status createErrorDetectionFrame(vx_context context, SensorObj *sensorObj);

    /*! \brief Configure capture output write (helper). */
    vx_status configureOutputWrite(vx_context context);

    /*! \brief Create capture write output node (helper). */
    vx_status createGraphWriteOutput(vx_graph graph);
};

/** \brief Capture module utility function
 *
 * This capture helper function takes a raw_image that is unpopulated and populates it with the provided path
 *
 * \param [in]  context       OpenVX context which must be created using \ref vxCreateContext
 * \param [in]  sensorInfo    Capture sensor information
 * \param [in]  file_path     Path to image
 * \param [out] bytes_read    Number of bytes read from file
 *
 */
tivx_raw_image read_error_image_raw(vx_context context, IssSensor_Info *sensorInfo, char file_path[], vx_int32 *bytes_read);

/* @} */

#endif //_APP_CAPTURE_MODULE_HPP_