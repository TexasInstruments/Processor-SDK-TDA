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
#ifndef _APP_IMG_MOSAIC_MODULE
#define _APP_IMG_MOSAIC_MODULE

/**
 * \defgroup group_vision_apps_modules_mosaic MSC HW Mosaic Node Module
 *
 * \brief This section contains module APIs for the MSC HW Mosaic Node tivxImgMosaicNode
 *
 * \ingroup group_vision_apps_modules
 *
 * @{
 */

#include "app_modules.h"
#include "app_modules_types.h"

/** \brief HW Mosaic Module Data Structure
 *
 * Contains the data objects required to use tivxImgMosaicNode
 *
 */
class ImgMosaicObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::ImgMosaic; }
  /*! Background image (set by app before create, or NULL for black) */
  vx_image background;

  /*! MSC HW Mosaic node object */
  vx_node  node;

  /*! MSC HW Mosaic kernel object */
  vx_kernel kernel;

  /*! MSC HW Mosaic node user data object for configuration of node */
  vx_user_data_object config;

  /*! Mosaic node params structure to initialize config object */
  tivxImgMosaicParams params;

  /*! Input object array to mosaic node */
  vx_object_array input_arr[TIVX_IMG_MOSAIC_MAX_INPUTS];

  /*! Buffer array of output images of mosaic node */
  vx_image output_image[APP_MODULES_MAX_BUFQ_DEPTH];

  /*! Mosaic node graph parameter index */
  vx_int32 graph_parameter_index;

  /*! Width of mosaic node output image */
  vx_int32 out_width;

  /*! Height of mosaic node output image */
  vx_int32 out_height;

  /*! Total number of inputs to mosaic node */
  vx_int32 num_inputs;

  /*! Buffer queue depth (defaults to 1 if not set before initModule) */
  vx_int32 bufq_depth;

    /*! \brief Constructor — sets safe defaults. */
    ImgMosaicObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    ImgMosaicObj(const ImgMosaicObj&)            = delete;
    ImgMosaicObj& operator=(const ImgMosaicObj&) = delete;
    ImgMosaicObj(ImgMosaicObj&&)                 = delete;
    ImgMosaicObj& operator=(ImgMosaicObj&&)      = delete;

    /*! \brief Initialize the module (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the mosaic node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the mosaic node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Write mosaic output image to file
     *
     * This helper function will write the contents of the provided image to the path provided
     *
     * \param [in] file_name  String of file path for where this output will be written
     * \param [in] out_img    Output image to write to file
     */
    vx_status writeMosaicOutput(char* file_name, vx_image out_img);

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();

    /*! \brief Configure mosaic parameters (helper). */
    vx_status configureParams(vx_context context);

    /*! \brief Create SW mosaic kernel (helper). */
    vx_status createSwMosaicKernel(vx_context context);

    /*! \brief Create output image (helper). */
    vx_status createOutputImage(vx_context context, vx_int32 bufq_depth);
};

/* @} */

#endif
