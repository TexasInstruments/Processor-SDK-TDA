/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include <TI/tivx.h>
#include <TI/tivx_img_proc.h>
#include <tivx_pixel_visualization_host.h>
#include <tivx_img_mosaic_host.h>
#include <tivx_dl_color_blend_host.h>
#include <tivx_dl_draw_box_host.h>
#include <tivx_dl_bev_post_proc_host.h>
#include <tivx_dl_bev_cam_post_proc_host.h>
#include <tivx_dl_post_proc_host.h>

VX_API_ENTRY vx_node VX_API_CALL tivxDLPreProcArmv8Node(vx_graph             graph,
                                                        vx_user_data_object  config,
                                                        vx_image             input_image,
                                                        vx_tensor            output_tensor)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)input_image,
            (vx_reference)output_tensor
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                                TIVX_KERNEL_DL_PRE_PROC_ARMV8_NAME,
                                                prms,
                                                dimof(prms));

    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxDLPostProcNode(vx_graph             graph,
                                                    vx_kernel            kernel,
                                                    vx_user_data_object  config,
                                                    vx_image             input_image,
                                                    vx_tensor            input_tensor[],
                                                    vx_image             output_image)
{
    vx_reference            prms[TIVX_DL_POST_PROC_MAX_PARAMS];
    vx_map_id               map_id_config;
    tivxDLPostProcParams    *postProcParams;
    vx_uint32               num_input_tensors, num_params, i;

    vxMapUserDataObject(config, 0, sizeof(tivxDLPostProcParams), &map_id_config,
        (void **)&postProcParams, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

    num_input_tensors = postProcParams->num_input_tensors;

    vxUnmapUserDataObject(config, map_id_config);

    num_params = (vx_uint32)TIVX_DL_POST_PROC_BASE_PARAMS + (vx_uint32)num_input_tensors;

    prms[0] = (vx_reference)config;
    prms[1] = (vx_reference)input_image;
    prms[2] = (vx_reference)output_image;

    for(i = 0; i < num_input_tensors; i++){
        prms[TIVX_DL_POST_PROC_INPUT_TENSOR_START_IDX + i] = (vx_reference)input_tensor[i];
    }

    vx_node node = tivxCreateNodeByKernelRef(graph,
                                             kernel,
                                             prms,
                                             num_params);
    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxDLColorConvertArmv8Node(vx_graph  graph,
                                                             vx_image  input_image,
                                                             vx_image  output_image)
{
    vx_reference prms[] = {
            (vx_reference)input_image,
            (vx_reference)output_image
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                             TIVX_KERNEL_DL_COLOR_CONVERT_ARMV8_NAME,
                                             prms,
                                             dimof(prms));

    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxODPostProcNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_tensor            input_key_points,
                                      vx_tensor            fwd_table_tensor,
                                      vx_tensor            rev_table_tensor,
                                      vx_tensor            kp_tensor,
                                      vx_tensor            kp_valid)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_key_points,
            (vx_reference)fwd_table_tensor,
            (vx_reference)rev_table_tensor,
            (vx_reference)kp_tensor,
            (vx_reference)kp_valid
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_OD_POSTPROCESS_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}
VX_API_ENTRY vx_node VX_API_CALL tivxImgPreProcNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_image             in_img,                                     
                                      vx_tensor            out_img)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)in_img,            
            (vx_reference)out_img
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_IMG_PREPROCESS_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxDL4DPreProcArmv8Node(vx_graph graph,
                                      vx_user_data_object            configuration,
                                      /* check if Array or vx_user_data_object*/
                                      vx_image            in_img,
                                      vx_image            in_img2,
                                      vx_image            in_img3,
                                      vx_image            in_img4,  
                                      vx_image            in_img5,
                                      vx_image            in_img6,
                                      vx_tensor           out_img)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)in_img,
            (vx_reference)in_img2,
            (vx_reference)in_img3,
            (vx_reference)in_img4,
            (vx_reference)in_img5,
            (vx_reference)in_img6,
            (vx_reference)out_img
    };
    /*#####################################################################################
        MOdification to take 6 Camera Images and create 4D Tensor.

    ###################################################################################*/
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DL_PRE_PROC_ARMV8_4D_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxOCPreProcNode(vx_graph             graph,
                                                   vx_user_data_object  config,
                                                   vx_image             in_img,
                                                   vx_tensor            out_tensor)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)in_img,
            (vx_reference)out_tensor
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_OC_PRE_PROC_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxOCPostProcNode(vx_graph             graph,
                                                    vx_user_data_object  config,
                                                    vx_user_data_object  in_args,
                                                    vx_tensor            in_tensor,
                                                    vx_user_data_object  results)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)in_args,
            (vx_reference)in_tensor,
            (vx_reference)results
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_OC_POST_PROC_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxDofPlaneSepNode(vx_graph             graph,
                                                     vx_user_data_object  config,
                                                     vx_image             dof_flow,
                                                     vx_tensor            out_planes)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)dof_flow,
            (vx_reference)out_planes
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DOF_PLANE_SEPERATION_NAME,
                                           prms,
                                           dimof(prms));
    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxPixelVizNode(vx_graph             graph,
                                                  vx_kernel            kernel,
                                                  vx_user_data_object  configuration,
                                                  vx_user_data_object  tidl_out_args,
                                                  vx_image             in_img,
                                                  vx_int32             num_output_tensors,
                                                  vx_tensor            detection[],
                                                  vx_image             out_img[])
{
    vx_reference prms[TIVX_KERNEL_PIXEL_VISUALIZATION_MAX_PARAMS];
    vx_int32 i;

    vx_int32 num_params = TIVX_KERNEL_PIXEL_VISUALIZATION_BASE_PARAMS + (num_output_tensors * 2);

    prms[0] = (vx_reference)configuration;
    prms[1] = (vx_reference)tidl_out_args;
    prms[2] = (vx_reference)in_img;

    for(i = 0; i < num_output_tensors; i++){
        prms[TIVX_KERNEL_PIXEL_VISUALIZATION_OUTPUT_TENSOR_IDX + i] = (vx_reference)detection[i];
        prms[TIVX_KERNEL_PIXEL_VISUALIZATION_OUTPUT_TENSOR_IDX + num_output_tensors + i] = (vx_reference)out_img[i];
    }

    vx_node node = tivxCreateNodeByKernelRef(graph,
                                             kernel,
                                             prms,
                                             num_params);
    return(node);

}

VX_API_ENTRY vx_node VX_API_CALL tivxPoseVizNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_image             background_image,
                                      vx_matrix            pose_tensor,
                                      vx_image             output_image)
{
  vx_reference prms[] = {
          (vx_reference)configuration,
          (vx_reference)background_image,
          (vx_reference)pose_tensor,
          (vx_reference)output_image
  };

  vx_node node = tivxCreateNodeByKernelName(graph,
                                          TIVX_KERNEL_POSE_VISUALIZATION_NAME,
                                          prms,
                                          dimof(prms));
  return(node);

}


VX_API_ENTRY vx_node VX_API_CALL tivxVisualLocalizationNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_tensor            voxel_info,
                                      vx_tensor            map_3d_points,
                                      vx_tensor            map_desc,
                                      vx_tensor            ext_feat_pt,
                                      vx_tensor            ext_feat_desc,
                                      vx_tensor            up_samp_wt,
                                      vx_tensor            lens_dist_table,
                                      vx_user_data_object  tidl_out_args,
                                      vx_matrix            pose_tensor)
{
  vx_reference prms[] = {
          (vx_reference)configuration,
          (vx_reference)voxel_info,
          (vx_reference)map_3d_points,
          (vx_reference)map_desc,
          (vx_reference)ext_feat_pt,
          (vx_reference)ext_feat_desc,
          (vx_reference)up_samp_wt,
          (vx_reference)lens_dist_table,
          (vx_reference)tidl_out_args,
          (vx_reference)pose_tensor
  };

  vx_node node = tivxCreateNodeByKernelName(graph,
                                          TIVX_KERNEL_VISUAL_LOCALIZATION_NAME,
                                          prms,
                                          dimof(prms));
  return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxImgMosaicNode(vx_graph             graph,
                                                   vx_kernel            kernel,
                                                   vx_user_data_object  config,
                                                   vx_image             output_image,
                                                   vx_image             background_image,
                                                   vx_object_array      input_arr[],
                                                   vx_uint32            num_inputs)
{
    vx_reference prms[TIVX_IMG_MOSAIC_MAX_PARAMS];
    vx_int32 i;

    vx_int32 num_params = TIVX_IMG_MOSAIC_BASE_PARAMS + num_inputs;

    prms[0] = (vx_reference)config;
    prms[1] = (vx_reference)output_image;
    prms[2] = (vx_reference)background_image;

    for(i = 0; i < num_inputs; i++){
        prms[TIVX_IMG_MOSAIC_INPUT_START_IDX + i] = (vx_reference)input_arr[i];
    }

    vx_node node = tivxCreateNodeByKernelRef(graph,
                                             kernel,
                                             prms,
                                             num_params);
    return(node);

}

VX_API_ENTRY vx_node VX_API_CALL tivxDrawKeypointDetectionsNode(vx_graph             graph,
                                                                vx_user_data_object  configuration,
                                                                vx_tensor            kp_tensor,
                                                                vx_tensor            kp_valid,
                                                                vx_tensor            input_tensor,
                                                                vx_image             input_image,
                                                                vx_image             output_image)
{
  vx_reference prms[] = {
          (vx_reference)configuration,
          (vx_reference)kp_tensor,
          (vx_reference)kp_valid,
          (vx_reference)input_tensor,
          (vx_reference)input_image,
          (vx_reference)output_image
  };

  vx_node node = tivxCreateNodeByKernelName(graph,
                                            TIVX_KERNEL_DRAW_KEYPOINT_DETECTIONS_NAME,
                                            prms,
                                            dimof(prms));

    return(node);

}

VX_API_ENTRY vx_node VX_API_CALL tivxDrawBoxDetectionsNode(vx_graph             graph,
                                                           vx_user_data_object  configuration,
                                                           vx_tensor            input_tensor,
                                                           vx_image             input_image,
                                                           vx_image             output_image)
{
  vx_reference prms[] = {
          (vx_reference)configuration,
          (vx_reference)input_tensor,
          (vx_reference)input_image,
          (vx_reference)output_image
  };

  vx_node node = tivxCreateNodeByKernelName(graph,
                                            TIVX_KERNEL_DRAW_BOX_DETECTIONS_NAME,
                                            prms,
                                            dimof(prms));

    return(node);

}

VX_API_ENTRY vx_node VX_API_CALL tivxDLBEVPostProcNode(vx_graph             graph,
                                                    vx_kernel            kernel,
                                                    vx_user_data_object  config,
                                                    vx_image             input_image,
                                                    vx_tensor            Lidar_2_Img_tensor,
                                                    vx_tensor            input_tensor[],
                                                    vx_image             output_image)
{   
    vx_reference            prms[TIVX_KERNEL_DRAW_BOX_DETECTIONS_MAX_PARAMS];
    vx_map_id               map_id_config;
    tivxDLBEVPostProcParams    *postProcParams;
    vx_uint32               num_input_tensors, num_params, i;

    vxMapUserDataObject(config, 0, sizeof(tivxDLBEVPostProcParams), &map_id_config,
        (void **)&postProcParams, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

    num_input_tensors = postProcParams->num_input_tensors;

    vxUnmapUserDataObject(config, map_id_config);

    num_params = (vx_uint32)TIVX_KERNEL_DRAW_BOX_DETECTIONS_BASE_PARAMS + (vx_uint32)num_input_tensors;
    
    prms[0] = (vx_reference)config;
    prms[1] = (vx_reference)input_image;
    prms[2] = (vx_reference)output_image;
    prms[3] = (vx_reference)Lidar_2_Img_tensor;

    for(i = 0; i < num_input_tensors; i++){
        prms[TIVX_KERNEL_DRAW_BOX_DETECTIONS_INPUT_TENSOR_START_IDX + i] = (vx_reference)input_tensor[i];
    }
    VX_PRINT(VX_ZONE_INFO, "TIVX_KERNEL_DRAW_BOX_DETECTIONS_BASE_PARAMS %d \n",TIVX_KERNEL_DRAW_BOX_DETECTIONS_BASE_PARAMS);
    VX_PRINT(VX_ZONE_INFO, "num_input_tensors- %d  ------- num_params - %d \n",num_input_tensors, num_params);
            
    vx_node node = tivxCreateNodeByKernelRef(graph,
                                             kernel,
                                             prms,
                                             num_params);
    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxDLBEVCamPostProcNode(vx_graph             graph,
                                                    vx_kernel            kernel,
                                                    vx_user_data_object  config,
                                                    vx_image             input_image,
                                                    vx_tensor            Lidar_to_Cam_tensor,
                                                    vx_tensor        Cam_to_Img_tensor,
                                                    vx_tensor            input_tensor[],
                                                    vx_image             output_image)
{   
    vx_reference            prms[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_MAX_PARAMS];
    vx_map_id               map_id_config;
    tivxDLBEVPostProcParams    *postProcParams;
    vx_uint32               num_input_tensors, num_params, i;

    vxMapUserDataObject(config, 0, sizeof(tivxDLBEVPostProcParams), &map_id_config,
        (void **)&postProcParams, (vx_enum)VX_READ_ONLY, (vx_enum)VX_MEMORY_TYPE_HOST, 0);

    num_input_tensors = postProcParams->num_input_tensors;

    vxUnmapUserDataObject(config, map_id_config);

    num_params = (vx_uint32)TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_BASE_PARAMS + (vx_uint32)num_input_tensors;
    
    prms[0] = (vx_reference)config;
    prms[1] = (vx_reference)input_image;
    prms[2] = (vx_reference)output_image;
    prms[3] = (vx_reference)Lidar_to_Cam_tensor;
    prms[4] = (vx_reference)Cam_to_Img_tensor;

    for(i = 0; i < num_input_tensors; i++){
        prms[TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_INPUT_TENSOR_START_IDX + i] = (vx_reference)input_tensor[i];
    }
    VX_PRINT(VX_ZONE_INFO, "TIVX_KERNEL_DRAW_BOX_DETECTIONS_BASE_PARAMS %d \n",TIVX_KERNEL_DRAW_BEV_BOX_DETECTIONS_BASE_PARAMS);
    VX_PRINT(VX_ZONE_INFO, "num_input_tensors- %d  ------- num_params - %d \n",num_input_tensors, num_params);
            
    vx_node node = tivxCreateNodeByKernelRef(graph,
                                             kernel,
                                             prms,
                                             num_params);
    return(node);
}


VX_API_ENTRY vx_node VX_API_CALL tivxImgHistNode(vx_graph             graph,
                                                 vx_image             input_image,
                                                 vx_distribution      output_histogram)
{
    vx_reference prms[] = {
            (vx_reference)input_image,
            (vx_reference)output_histogram
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                                TIVX_KERNEL_IMG_HIST_NAME,
                                                prms,
                                                dimof(prms));

    return(node);

}

VX_API_ENTRY vx_node VX_API_CALL tivxSFMNode(vx_graph            graph,
                                            vx_user_data_object  config,
                                            vx_user_data_object  create_params,
                                            vx_user_data_object  in_args,
                                            vx_user_data_object  out_args,
                                            vx_image             input_image,
                                            vx_image             flow_vectors,
                                            vx_image             output_ptcld_image,
                                            vx_image             output_og_image,
                                            vx_user_data_object  output_feat)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)create_params,
            (vx_reference)in_args,
            (vx_reference)out_args,
            (vx_reference)input_image,
            (vx_reference)flow_vectors,
            (vx_reference)output_ptcld_image,
            (vx_reference)output_og_image,
            (vx_reference)output_feat,
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                                TIVX_KERNEL_SFM_NAME,
                                                prms,
                                                dimof(prms));

    return(node);

}

VX_API_ENTRY vx_node VX_API_CALL tivxDLPreProcNode(vx_graph             graph,
                                                   vx_user_data_object  config,
                                                   vx_image             input_image,
                                                   vx_tensor            output_tensor)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)input_image,
            (vx_reference)output_tensor
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                                TIVX_KERNEL_DL_PRE_PROC_NAME,
                                                prms,
                                                dimof(prms));

    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxDLColorBlendNode(vx_graph             graph,
                                                      vx_user_data_object  config,
                                                      vx_image             input_image,
                                                      vx_tensor            input_tensor,
                                                      vx_image             output_image)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)input_image,
            (vx_reference)input_tensor,
            (vx_reference)output_image
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                             TIVX_KERNEL_DL_COLOR_BLEND_NAME,
                                             prms,
                                             dimof(prms));

    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxDLDrawBoxNode(vx_graph             graph,
                                                   vx_user_data_object  config,
                                                   vx_image             input_image,
                                                   vx_tensor            input_tensor,
                                                   vx_image             output_image)
{
    vx_reference prms[] = {
            (vx_reference)config,
            (vx_reference)input_image,
            (vx_reference)input_tensor,
            (vx_reference)output_image
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                             TIVX_KERNEL_DL_DRAW_BOX_NAME,
                                             prms,
                                             dimof(prms));

    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxDLColorConvertNode(vx_graph  graph,
                                                        vx_image  input_image,
                                                        vx_image  output_image)
{
    vx_reference prms[] = {
            (vx_reference)input_image,
            (vx_reference)output_image
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                             TIVX_KERNEL_DL_COLOR_CONVERT_NAME,
                                             prms,
                                             dimof(prms));

    return(node);
}

VX_API_ENTRY vx_node VX_API_CALL tivxAddImgNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference refs[] = {
            (vx_reference)in1, 
            (vx_reference)in2, 
            (vx_reference)out
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                              TIVX_C7X_KERNEL_IMG_ADD_NAME,
                                              refs, 
                                              sizeof(refs)/sizeof(refs[0]));

    vxSetReferenceName((vx_reference)node, TIVX_C7X_KERNEL_IMG_ADD_NAME);

    return node;
}
