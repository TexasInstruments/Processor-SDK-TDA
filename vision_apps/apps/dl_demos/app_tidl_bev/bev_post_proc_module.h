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
#ifndef _BEV_POST_PROC_MODULE
#define _BEV_POST_PROC_MODULE

#include "app_common.h"
#include "itidl_ti.h"
#include <string>

#include <app_modules_types.h>

#define APP_MODULES_MAX_TENSORS         (8u)

/**
 * BEV PostProc module class
 *
 * Custom post-processing for BEV with dual modes (BEV/camera).
 * Inherits from AppModuleBase for module system integration.
 */
class BEVPostProcObj : public AppModuleBase {
public:
    BEVPostProcObj();

    /* AppModuleBase interface */
    AppModuleType getType() const override { return AppModuleType::BevPostProc; }
    vx_status initModule(AppGraphObj *graph, vx_int32 module_index) override;
    vx_status createNode(AppGraphObj *graph, vx_int32 module_index) override;
    vx_status deleteNode(AppGraphObj *graph, vx_int32 module_index) override;
    vx_status deinitModule(AppGraphObj *graph, vx_int32 module_index) override;

    /* Public data members */
    vx_node node;
    vx_user_data_object config;
    tivxDLBEVPostProcParams params;
    sTIDL_IOBufDesc_t ioBufDesc;
    /*! TIDL config user data object - set by app (from tidlObj.config) before addNode(). */
    vx_user_data_object tidl_config;
    vx_uint32 num_input_tensors;
    vx_uint32 num_output_tensors;
    vx_uint32 output_img_width;
    vx_uint32 output_img_height;
    vx_size lidar_points[1];
    vx_size lidar2cam_points[1];
    vx_size cam_2img_points[1];
    vx_float32 viz_th;
    vx_uint32 num_top_results;
    vx_kernel kernel;
    vx_tensor input_temp;
    vx_object_array output_image_arr;
    vx_object_array output_arr[APP_MAX_BUFQ_DEPTH];
    vx_image output_image[APP_MAX_BUFQ_DEPTH];
    vx_int32 graph_parameter_index;
    vx_int32 num_ch;        /* Number of channels (set before addNode) */
    vx_int32 bufq_depth;    /* Buffer queue depth (set before addNode) */
    std::string objName;
    vx_object_array Lidar_2_img_tensor_arr;
    vx_object_array input_lidar_arr[APP_MAX_BUFQ_DEPTH];
    vx_tensor input_lidar[APP_MAX_BUFQ_DEPTH];
    vx_object_array input_lidar2cam_arr[APP_MAX_BUFQ_DEPTH];
    vx_tensor input_lidar2cam[APP_MAX_BUFQ_DEPTH];
    vx_object_array input_cam2img_arr[APP_MAX_BUFQ_DEPTH];
    vx_tensor input_cam2img[APP_MAX_BUFQ_DEPTH];
    std::string Lidar_2_img_file_path;

    /* Module-specific state (set via helpers before createNode) */
    bool is_camera_mode;
    vx_object_array input_tensor_arr;
    vx_object_array input_image_arr;

private:
    void setDefaults();
};

/* Helper functions */
vx_status writebevPostProcOutput(char* file_name, BEVPostProcObj *postProcObj);
vx_status readCameraIntrinsicInput(vx_object_array tensor_arr);

#endif /* _BEV_POST_PROC_MODULE */
