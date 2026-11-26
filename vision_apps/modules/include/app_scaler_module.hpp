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
 #ifndef _APP_SCALER_MODULE
 #define _APP_SCALER_MODULE


/**
 * \defgroup group_vision_apps_modules_scaler Scaler Node Module
 *
 * \brief This section contains module APIs for the TIOVX Scaler node tivxVpacMscScaleNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#include "app_modules.h"
#include "app_modules_types.h"
#include <TI/hwa_vpac_msc.h>

/** \brief Maximum amount of values allowed from scaler node
 *
 */
#define APP_MODULES_MAX_SCALER_OUTPUTS (5)

/** \brief Maximum buffer queue depth for scaler input
 *
 */
#define APP_MODULES_MAX_SCALER_BUFFERS (8)

/** \brief Read mode for reading entire file
 *
 */
#define APP_MODULES_READ_FILE (0x333)

/** \brief Read mode for reading selected channel
 *
 */
#define APP_MODULES_READ_CHANNEL (0x444)


/** \brief Scaler Module Data Structure
 *
 * Contains the data objects required to use tivxVpacMscScaleNode
 *
 */
class ScalerObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::Scaler; }
    /*! Input object array (created by module) */
   vx_object_array input_arr[APP_MODULES_MAX_SCALER_BUFFERS];

    /*! Input images extracted from input_arr for graph pipelining */
   vx_image input_images[APP_MODULES_MAX_SCALER_BUFFERS];

    /*! Input dimensions (must be set before initModule) */
   vx_int32 input_width;
   vx_int32 input_height;

    /*! Buffer queue depth (must be > 0, typically 2-8 for pipelining) */
   vx_int32 bufq_depth;

    /*! Scaler node object */
   vx_node    node;

    /*! Scaler image output structure array */
   ImgObj output[APP_MODULES_MAX_SCALER_OUTPUTS];

    /*! Scaler node coefficient user data object */
   vx_user_data_object coeff_obj;

    /*! Scaler node graph parameter index */
   vx_int32 graph_parameter_index;

    /*! Flag to enable writing scaler output  */
   vx_int32 en_out_scaler_write;

    /*! File path used to write scaler node output */
   vx_array file_path;

    /*! File path prefix used to write scaler node output */
   vx_array file_prefix[APP_MODULES_MAX_SCALER_OUTPUTS];

    /*! Node used to write scaler output */
   vx_node write_node[APP_MODULES_MAX_SCALER_OUTPUTS];

    /*! User data object containing write cmd parameters */
   vx_user_data_object write_cmd[APP_MODULES_MAX_SCALER_OUTPUTS];

    /*! Output file path for scaler node output */
   vx_char output_file_path[TIVX_FILEIO_FILE_PATH_LENGTH];

    /*! Name of scaler module */
   vx_char objName[APP_MODULES_MAX_OBJ_NAME_SIZE];

    /*! Number of channels used by scaler module */
   vx_int32 num_ch;

    /*! Number of outputs used by scaler module */
   vx_int32 num_outputs;

    /*! Color format used by scaler node; supported values of \ref VX_DF_IMAGE_U8 and \ref VX_DF_IMAGE_NV12 */
   vx_int32 color_format;

    /*! Flag for external input mode: vx_true_e = input_arr provided externally, don't create/release */
   vx_bool external_input;

    /*! \brief Constructor — sets safe defaults (no OpenVX objects created). */
    ScalerObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    ScalerObj(const ScalerObj&)            = delete;
    ScalerObj& operator=(const ScalerObj&) = delete;
    ScalerObj(ScalerObj&&)                 = delete;
    ScalerObj& operator=(ScalerObj&&)      = delete;

    /*! \brief Initialize the module — create data objects (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the scaler node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the scaler node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module — release data objects (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Send MSC coefficients to the scaler node. */
    vx_status sendMscCoeff(vx_int32 module_index);

    /*! \brief Send write command to scaler write node */
    vx_status sendCmdWriteNode(vx_uint32 start_frame, vx_uint32 num_frames, vx_uint32 num_skip);

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Configure scaler coefficients (helper). */
    vx_status configureScalerCoeffs(vx_context context);

    /*! \brief Create scaler output objects (helper). */
    vx_status createScalerOutputs(vx_context context);

    /*! \brief Create scaler write output graph (helper). */
    vx_status createGraphScalerWriteOutput(vx_graph graph, vx_int32 idx);
};

/** \brief Scaler module helper function for setting MSC coefficients
 *
 * This Scaler helper function sets the MSC coefficients based on the type of interpolation
 *
 * \param [out] coeff          MSC coefficients set based on interpolation type
 * \param [in]  interpolation  Scaler interpolation type; valid values are \ref VX_INTERPOLATION_BILINEAR and \ref VX_INTERPOLATION_NEAREST_NEIGHBOR
 *
 */
void scale_set_coeff(tivx_vpac_msc_coefficients_t *coeff,  uint32_t interpolation);

/** \brief Helper function to read image from file
 *
 * This scaler create helper function will read in the image from file into the provided image array
 *
 * \param [in]  file_name  Full path to file to read
 * \param [out] img_arr    Object array containing images which file is read in to
 * \param [in]  read_mode  Read mode; options are \ref APP_MODULES_READ_FILE and \ref APP_MODULES_READ_CHANNEL
 * \param [in]  ch_num     Channel number of array to write
 *
 */
vx_status readScalerInput(char* file_name, vx_object_array img_arr, vx_int32 read_mode, vx_int32 ch_num);

/** \brief Helper function to write image from file
 *
 * This scaler create helper function will write the image from OpenVX object array to the provided file path
 *
 * \param [in]  file_name  Full path to file to write
 * \param [in]  img_arr    Object array containing images which file written from
 *
 */
vx_status writeScalerOutput(char* file_name, vx_object_array img_arr);


/* @} */

#endif
