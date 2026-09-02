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

#ifndef TIVX_STEREO_NODES_H_
#define TIVX_STEREO_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief [Graph] Creates a POINT_CLOUD_CREATION Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] input_image
 * \param [in] input_sdedisparity
 * \param [in] input_tensor
 * \param [out] point_cloud_out
 * \see <tt>TIVX_KERNEL_POINT_CLOUD_CREATION_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxPointCloudCreationNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_image,
                                      vx_image             input_sdedisparity,
                                      vx_tensor            input_tensor,
                                      vx_user_data_object  point_cloud_out);

/*! \brief [Graph] Creates a OCCUPANCY_GRID_DETECTION Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] point_cloud_in
 * \param [out] bound_box_3d_out
 * \see <tt>TIVX_KERNEL_OCCUPANCY_GRID_DETECTION_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxOccupancyGridDetectionNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_user_data_object  point_cloud_in,
                                      vx_user_data_object  bound_box_3d_out);


/*! \brief Creates a Stereo Confidence Histogram Visualization node.
 *
 * \param [in] graph             The reference to the graph.
 * \param [in] histogram         Confidence distribution output from DMPAC SDE node
 * \param [out] histogram_image  Confidence histogram as U8 grayscale image
 *
 * \ingroup group_vision_apps_kernels_stereo
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxSdeHistogramVisualizeNode(vx_graph graph,
                                      vx_distribution      histogram,
                                      vx_image             histogram_image);


/*! \brief Creates a Stereo Triangulation node.
 *
 * \param [in]  graph             The reference to the graph.
 * \param [in]  stereo_cam_config Stereo camera configuration. User data object element type \ref tivx_stereo_cam_params_t
 * \param [in]  stereo_pc_config  Stereo pointcloud configuration. User data object element type \ref tivx_stereo_pointcloud_params_t
 * \param [in]  rgbImage          rectified image in RGB, on which disaprity is based
 * \param [in]  disparity         16-bit disparity output from DMPSC SDE node
 * \param [out] pointcloud        User data object of 3D points. See tivx_perception_pointcloud_t
 *
 * \ingroup group_vision_apps_kernels_stereo
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxSdeTriangulationNode(vx_graph graph,
                                      vx_user_data_object  stereo_cam_config,
                                      vx_user_data_object  stereo_pc_config,
                                      vx_image             rgbImage,
                                      vx_image             disparity,
                                      vx_user_data_object  pointcloud);

/*! \brief [Graph] Creates a DISPARITY_MERGE Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] low_input_disparity
 * \param [in] high_input_disparity
 * \param [out] output_disparity
 * \see <tt>TIVX_KERNEL_DISPARITY_MERGE_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDisparityMergeNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             low_input_disparity,
                                      vx_image             high_input_disparity,
                                      vx_image             output_disparity);

/*! \brief [Graph] Creates a MEDIAN_FILTER Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] input_disparity
 * \param [out] output_disparity
 * \see <tt>TIVX_KERNEL_MEDIAN_FILTER_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxMedianFilterNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_disparity,
                                      vx_image             output_disparity);

/*! \brief [Graph] Creates a HOLE_FILLING Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] input_disparity
 * \see <tt>TIVX_KERNEL_HOLE_FILLING_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxHoleFillingNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_disparity);

/*! \brief [Graph] Creates a EXTRACT_DISPARITY_CONFIDENCE Node.
 * \param [in] graph The reference to the graph.
 * \param [in] input_sdedisparity
 * \param [out] output_disparity
 * \param [out] output_confidence
 * \see <tt>TIVX_KERNEL_EXTRACT_DISPARITY_CONFIDENCE_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxExtractDisparityConfidenceNode(vx_graph graph,
                                      vx_image             input_sdedisparity,
                                      vx_array             output_disparity,
                                      vx_array             output_confidence);

/*! \brief [Graph] Creates a GROUND_ESTIMATION Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] input_image
 * \param [in] input_sdedisparity
 * \param [out] output_disparity
 * \param [out] output_ground_model
 * \see <tt>TIVX_KERNEL_GROUND_ESTIMATION_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxGroundEstimationNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_image,
                                      vx_image             input_sdedisparity,
                                      vx_image             output_disparity,
                                      vx_user_data_object  output_ground_model);

/*! \brief [Graph] Creates a OBSTACLE_DETECTION Node.
 * \param [in] graph The reference to the graph.
 * \param [in] configuration
 * \param [in] input_image
 * \param [in] input_disparity
 * \param [in] input_ground_model
 * \param [out] output_obstacle_pos
 * \param [out] output_num_obstacles
 * \param [out] output_freespace_boundary
 * \param [out] output_drivable_space
 * \see <tt>TIVX_KERNEL_OBSTACLE_DETECTION_NAME</tt>
 * \ingroup group_vision_apps_kernels_stereo
 * \return <tt>vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxObstacleDetectionNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             input_image,
                                      vx_image             input_disparity,
                                      vx_user_data_object  input_ground_model,
                                      vx_array             output_obstacle_pos,
                                      vx_scalar            output_num_obstacles,
                                      vx_array             output_freespace_boundary,
                                      vx_user_data_object  output_drivable_space);

/*! \brief Creates a Stereo Disparity Visualization node.
 *
 * \param [in] graph          The reference to the graph.
 * \param [in] configuration  Parameters for the node of. Array element type \ref tivx_stereo_cam_params_t
 * \param [in] disparity      16-bit disparity output from DMPAC SDE node
 * \param [out] disparity_rgb Disparity map representated as 24 RGB image
 *
 * \ingroup group_vision_apps_kernels_stereo
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxSdeDisparityVisualizeNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_image             disparity,
                                      vx_image             disparity_rgb);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_STEREO_NODES_H_ */


