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
#ifndef _APP_POST_PROC_MODULE
#define _APP_POST_PROC_MODULE

#include "app_modules.h"
#include "app_modules_types.h"
#include "itidl_ti.h"
#include "app_tidl_module.hpp"
#include "app_display_module.hpp"

/* Macros that may not be provided by app_modules.h but are needed by
 * this module.  Guarded so that if the including app already defines
 * them (e.g. via app_common.h / app_modules_types.h), there is no
 * conflict. */
#ifndef APP_ASSERT
#define APP_ASSERT(x)               assert((x))
#endif

#ifndef APP_ASSERT_VALID_REF
#define APP_ASSERT_VALID_REF(ref)   (APP_ASSERT(vxGetStatus((vx_reference)(ref))==VX_SUCCESS));
#endif

#ifndef APP_MAX_FILE_PATH
#define APP_MAX_FILE_PATH           (256u)
#endif

#ifndef NUM_CH
#define NUM_CH                      (1)
#endif

class PostProcObj : public AppModuleBase {
public:
    AppModuleType getType() const override { return AppModuleType::PostProc; }
    /*! Input tensor object array (set by app before create) */
    vx_object_array input_tensor_arr[APP_MODULES_MAX_TENSORS];

    /*! Input image object array (set by app before create) */
    vx_object_array input_arr;

    vx_node  node;

    vx_user_data_object config;

    tivxDLPostProcParams params;

    sTIDL_IOBufDesc_t ioBufDesc;

    vx_uint32 num_input_tensors;

    vx_uint32 num_output_tensors;


    vx_uint32 output_img_width;

    vx_uint32 output_img_height;


    vx_float32 viz_th;

    vx_uint32 num_top_results;

    vx_kernel kernel;

    vx_object_array output_image_arr;

    /*! Per-pipeline-slot output images for use as graph parameters */
    vx_image output_image[APP_MODULES_MAX_BUFQ_DEPTH];

    vx_int32 graph_parameter_index;

    /*! Number of channels (cameras). Defaults to NUM_CH; set before initModule(). */
    vx_int32 num_ch;

    vx_char objName[APP_MAX_FILE_PATH];

    /*! \brief Constructor — sets safe defaults. */
    PostProcObj();

    /*! Owns OpenVX handles that are released explicitly (deinitModule/deleteNode);
     *  copying would duplicate handles and risk a double-release or dangling use. */
    PostProcObj(const PostProcObj&)            = delete;
    PostProcObj& operator=(const PostProcObj&) = delete;
    PostProcObj(PostProcObj&&)                 = delete;
    PostProcObj& operator=(PostProcObj&&)      = delete;

    /*! \brief Initialize the module (override). */
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Create the post-proc node in the graph (override). */
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief Delete the post-proc node (override). */
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;

    /*! \brief De-initialize the module (override). */
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

private:
    /*! \brief Set safe defaults (called by constructor). */
    void setDefaults();
};

vx_status writePostProcOutput(char* file_name, PostProcObj *postProcObj);

#endif
