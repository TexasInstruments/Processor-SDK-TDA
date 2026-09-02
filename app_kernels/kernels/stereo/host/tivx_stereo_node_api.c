/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include <TI/tivx_stereo.h>

VX_API_ENTRY vx_node VX_API_CALL tivxPointCloudCreationNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_image,
                                      vx_image             input_sdedisparity,
                                      vx_tensor            input_tensor,
                                      vx_user_data_object  point_cloud_out)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_image,
            (vx_reference)input_sdedisparity,
            (vx_reference)input_tensor,
            (vx_reference)point_cloud_out
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_POINT_CLOUD_CREATION_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxOccupancyGridDetectionNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_user_data_object  point_cloud_in,
                                      vx_user_data_object  bound_box_3d_out)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)point_cloud_in,
            (vx_reference)bound_box_3d_out
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_OCCUPANCY_GRID_DETECTION_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxDisparityMergeNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             low_input_disparity,
                                      vx_image             high_input_disparity,
                                      vx_image             output_disparity)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)low_input_disparity,
            (vx_reference)high_input_disparity,
            (vx_reference)output_disparity
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_DISPARITY_MERGE_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxMedianFilterNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_disparity,
                                      vx_image             output_disparity)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_disparity,
            (vx_reference)output_disparity
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_MEDIAN_FILTER_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxHoleFillingNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_disparity)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_disparity
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_HOLE_FILLING_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}


VX_API_ENTRY vx_node VX_API_CALL tivxSdeHistogramVisualizeNode(vx_graph graph,
                                      vx_distribution      histogram,
                                      vx_image             histogram_image)
{
    vx_reference prms[] = {
            (vx_reference)histogram,
            (vx_reference)histogram_image
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_SDE_HISTOGRAM_VISUALIZE_NAME,
                                           prms,
                                           dimof(prms));

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxSdeTriangulationNode(vx_graph graph,
                                      vx_user_data_object  stereo_cam_config,
                                      vx_user_data_object  stereo_pc_config,
                                      vx_image             rgbImage,
                                      vx_image             disparity,
                                      vx_user_data_object  pointcloud)
{
    vx_reference prms[] = {
            (vx_reference)stereo_cam_config,
            (vx_reference)stereo_pc_config,
            (vx_reference)rgbImage,
            (vx_reference)disparity,
            (vx_reference)pointcloud
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_SDE_TRIANGULATION_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxExtractDisparityConfidenceNode(vx_graph graph,
                                      vx_image             input_sdedisparity,
                                      vx_array             output_disparity,
                                      vx_array             output_confidence)
{
    vx_reference prms[] = {
            (vx_reference)input_sdedisparity,
            (vx_reference)output_disparity,
            (vx_reference)output_confidence
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxGroundEstimationNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_image,
                                      vx_image             input_sdedisparity,
                                      vx_image             output_disparity,
                                      vx_user_data_object  output_ground_model)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_image,
            (vx_reference)input_sdedisparity,
            (vx_reference)output_disparity,
            (vx_reference)output_ground_model
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                              TIVX_KERNEL_GROUND_ESTIMATION_NAME,
                                              prms,
                                              dimof(prms));
    return node;
}


VX_API_ENTRY vx_node VX_API_CALL tivxObstacleDetectionNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_image,
                                      vx_image             input_disparity,
                                      vx_user_data_object  input_ground_model,
                                      vx_array             output_obstacle_pos,
                                      vx_scalar            output_num_obstacles,
                                      vx_array             output_freespace_boundary,
                                      vx_user_data_object  output_drivable_space)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)input_image,
            (vx_reference)input_disparity,
            (vx_reference)input_ground_model,
            (vx_reference)output_obstacle_pos,
            (vx_reference)output_num_obstacles,
            (vx_reference)output_freespace_boundary,
            (vx_reference)output_drivable_space
    };
    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_OBSTACLE_DETECTION_NAME,
                                           prms,
                                           dimof(prms));
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL tivxSdeDisparityVisualizeNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             disparity,
                                      vx_image             disparity_rgb)
{
    vx_reference prms[] = {
            (vx_reference)configuration,
            (vx_reference)disparity,
            (vx_reference)disparity_rgb
    };

    vx_node node = tivxCreateNodeByKernelName(graph,
                                           TIVX_KERNEL_SDE_DISPARITY_VISUALIZE_NAME,
                                           prms,
                                           dimof(prms));

    return node;
}
