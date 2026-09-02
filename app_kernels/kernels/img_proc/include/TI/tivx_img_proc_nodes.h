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

#ifndef TIVX_IMG_PROC_NODES_H_
#define TIVX_IMG_PROC_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Creates a Image Pre Processing Node.
 *
 * Node does pre processing needed for conventional deep learning algorithm. Preprocessing steps
 * such as mean subtraction, scaling and plane deinterleaving is supported by this Node.
 *
 *
 * \param [in] graph             The reference to the graph.
 * \param [in] config            Array element type tivxImgPreProcParams, max number of elements = 1
 * \param [in] in_img            input image
 * \param [in] out_tensor        output tensor
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxImgPreProcNode(vx_graph   graph,
                                                    vx_array   config,
                                                    vx_image   in_img,
                                                    vx_tensor  out_tensor);

VX_API_ENTRY vx_node VX_API_CALL tivxDL4DPreProcArmv8Node(vx_graph             graph,
                                                        vx_user_data_object  config,
                                                        vx_image             input_image,
                                                        vx_image             input_image2,
                                                        vx_image             input_image3,
                                                        vx_image             input_image4,
                                                        vx_image             input_image5,
                                                        vx_image             input_image6,
                                                        vx_tensor            output_tensor);

/*! \brief Creates a OC Pre Processing Node.
 *
 * Node does pre processing needed for image classification operation
 * Performs only NV12 to BGR conversion
 *
 * \param [in] graph        The reference to the graph.
 * \param [in] config       User data object element type tivxOCPreProcParams
 * \param [in] in_img       input image
 * \param [out] out_tensor  output tensor
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxOCPreProcNode(vx_graph graph,
                                                   vx_user_data_object  config,
                                                   vx_image             in_img,
                                                   vx_tensor            out_tensor);

/*! \brief Creates a OC Post Processing Node.
  *
  * Node does post processing needed for conventional image classification algorithm.
  * Returns top N classes from a list of probabilities
  *
  * \param [in] graph        The reference to the graph.
  * \param [in] config       User data object element type tivxOCPostProcParams
  * \param [in] in_args      input user data object
  * \param [in] in_tensor    input tensor
  * \param [out] results     output user data object
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxOCPostProcNode(vx_graph graph,
                                                    vx_user_data_object  config,
                                                    vx_user_data_object  in_args,
                                                    vx_tensor            in_tensor,
                                                    vx_user_data_object  results);

/*! \brief Creates a Post processing Node for Parking Spot Detection Algorithm.
 *
 * Node does generates interpolated points on parking spot periphery from four corners points
 * detected from deep learning based algorithm
 *
 *
 * \param [in] graph                The reference to the graph.
 * \param [in] configuration        Array element type tivxODPostProcParams, max number of elements = 1
 * \param [in] input_key_points     Interleaved 4 corner points of multiple parking spots.
 * \param [in] fwd_table_tensor     Angular map table for rectilinear angle to fish eye angle
 * \param [in] rev_table_tensor     Angular map table for fish eye angle to rectilinear angle
 * \param [in] kp_tensor            output interpolated points on periphery of parking spot
 * \param [in] kp_valid             output flag of validity for each parking spot
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */

VX_API_ENTRY vx_node VX_API_CALL tivxODPostProcNode(vx_graph graph,
                                      vx_array             configuration,
                                      vx_tensor            input_key_points,
                                      vx_tensor            fwd_table_tensor,
                                      vx_tensor            rev_table_tensor,
                                      vx_tensor            kp_tensor,
                                      vx_tensor            kp_valid);

/*! \brief Creates a Dense Optical Flow (Dof) 3 plane creation Node.
 *
 * Node does creation of three planes (each of 8 bit) from the raw 32 bit output from Dof hardware accelerator
 * 3 planes corresponds to horizontal, vertical flow vectors and confidence
 *
 * \param [in] graph         The reference to the graph.
 * \param [in] config        Array element type tivxDofPlaneSepParams, max number of elements = 1
 * \param [in] dof_flow      input dof flow vectors
 * \param [out] out_planes    output planes.
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxDofPlaneSepNode(vx_graph             graph,
                                                     vx_user_data_object  config,
                                                     vx_image             dof_flow,
                                                     vx_tensor            out_planes);

/*! \brief Creates an image with pixel level detection/label overlayed.
 *
 * Node does blending of input image with pixel level detection/classification data for visualization purpose.
 *
 * \param [in] graph                The reference to the graph.
 * \param [in] kernel               The reference to openvx kernel
 * \param [in] configuration        Array element type tivxODPostProcParams, max number of elements = 1
 * \param [in] tidl_out_args        out args of tidl to receive the scale values of each output tensor
 * \param [in] in_img               Input image.
 * \param [in] num_output_tensors   Number of output tensors/images.
 * \param [in] detection            Detection planes
 * \param [out] out_img             ovelayed output image
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */

 VX_API_ENTRY vx_node VX_API_CALL tivxPixelVizNode(vx_graph             graph,
                                                   vx_kernel            kernel,
                                                   vx_user_data_object  configuration,
                                                   vx_user_data_object  tidl_out_args,
                                                   vx_image             in_img,
                                                   vx_int32             num_output_tensors,
                                                   vx_tensor            detection[],
                                                   vx_image             out_img[]);

/*! \brief Estimating the camera location from captured image and map information
 *
 * Node estimates the location in 3d world co-ordinate
 *
 * \param [in] graph                The reference to the graph.
 * \param [in] configuration        Array element type tivxODPostProcParams, max number of elements = 1
 * \param [in] voxel_info           Voxel info. voxels are cuboid in real world
 * \param [in] map_3d_points        Map 3D points
 * \param [in] map_desc             Map descriptors
 * \param [in] ext_feat_pt          Externally computed feature points(x,y)
 * \param [in] ext_feat_desc        Feature descriptor for each feature point
 * \param [in] up_samp_wt           Filter weights for upsampling
 * \param [in] lens_dist_table      Fish eye lens distortion table
 * \param [in] tidl_out_args        TIDL outatgs needed for scale
 * \param [out] pose_tensor         Output pose matrix of size 3x4
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */

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
                                      vx_matrix            pose_tensor);

  /*! \brief Visualization of the estimated location
 *
 * Node generated an image with visualization of X, Y location of 3D world on image plane
 *
 * \param [in] graph                The reference to the graph.
 * \param [in] configuration        Array element type tivxODPostProcParams, max number of elements = 1
 * \param [in] background_image     Background image on which pose information will be visualized
 * \param [in] pose_tensor          Input pose matrix created by tivxVisualLocalizationNode Node
 * \param [out] output_image        Output image
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxPoseVizNode(vx_graph graph,
                                      vx_array            configuration,
                                      vx_image            background_image,
                                      vx_matrix           pose_tensor,
                                      vx_image            output_image);


/*! \brief Image Mosiac Node
 *
 * Used to mosiac a list of images onto to a sigle image
 *
 * \param [in] graph             The reference to the graph.
 * \param [in] kernel            The reference to the openvx kernel
 * \param [in] config            Mosaic configuration of type tivxImgMosaicParams
 * \param [out] output_image     Output image of type vx_image, only VX_DF_IMAGE_NV12, VX_DF_IMAGE_U8, VX_DF_IMAGE_U16 supported
 * \param [in] background_image  Optional background image of type vx_image, only VX_DF_IMAGE_NV12, VX_DF_IMAGE_U8, VX_DF_IMAGE_U16 supported
 * \param [in] input_arr         Array of inputs of type vx_image, only VX_DF_IMAGE_NV12, VX_DF_IMAGE_U8, VX_DF_IMAGE_U16 supported
 * \param [in] num_inputs        Number of inputs, i.e number of elements in input_arr[]
 *
 * \ingroup group_vision_apps_kernels_img_proc
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxImgMosaicNode(vx_graph             graph,
                                                   vx_kernel            kernel,
                                                   vx_user_data_object  config,
                                                   vx_image             output_image,
                                                   vx_image             background_image,
                                                   vx_object_array      input_arr[],
                                                   vx_uint32            num_inputs);

/*! \brief Draw Keypoint Detections Node
  *
  * Used to draw keypoint detections on an image
  *
  * \param [in] graph            reference to the graph.
  * \param [in] configuration    configuration of type tivxDrawKeyPointDetectionParams
  * \param [in] kp_tensor        keypoint tensor from OD postprocessing node
  * \param [in] kp_valid         valid keypoints tensor from OD postprocessing node
  * \param [in] input_tensor     TIDL output tensor of detected key points
  * \param [in] input_image      Input image to draw detections
  * \param [out] output_image    Output image with detections drawn
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDrawKeypointDetectionsNode(vx_graph             graph,
                                                                vx_user_data_object  configuration,
                                                                vx_tensor            kp_tensor,
                                                                vx_tensor            kp_valid,
                                                                vx_tensor            input_tensor,
                                                                vx_image             input_image,
                                                                vx_image             output_image);

/*! \brief Draw Box Detections Node
  *
  * Used to draw box detections on an image
  *
  * \param [in] graph            reference to the graph.
  * \param [in] configuration    configuration of type tivxDrawBoxDetectionParams
  * \param [in] input_tensor     TIDL output tensor of detected key points
  * \param [in] input_image      Input image to draw detections
  * \param [out] output_image    Output image with detections drawn
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDrawBoxDetectionsNode(vx_graph             graph,
                                                           vx_user_data_object  configuration,
                                                           vx_tensor            input_tensor,
                                                           vx_image             input_image,
                                                           vx_image             output_image);



/*! \brief Draw BEV Post Proc  Node
  *
  * Used to draw box detections on an image
  *
  * \param [in] graph            reference to the graph.
  * \param [in] configuration    configuration of type tivxDrawBoxDetectionParams
  * \param [in] input_tensor     TIDL output tensor of detected key points
  * \param [in] input_image      Input image to draw detections
  * \param [out] output_image    Output image with detections drawn
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */

VX_API_ENTRY vx_node VX_API_CALL tivxDLBEVPostProcNode(vx_graph             graph,
                                                    vx_kernel            kernel,
                                                    vx_user_data_object  config,
                                                    vx_image             input_image,
                                                    vx_tensor            Lidar_2_Img_tensor,
                                                    vx_tensor            input_tensor[],
                                                    vx_image             output_image);

VX_API_ENTRY vx_node VX_API_CALL tivxDLBEVCamPostProcNode(vx_graph             graph,
                                                      vx_kernel            kernel,
                                                      vx_user_data_object  config,
                                                      vx_image             input_image,
                                                      vx_tensor            Lidar_2_Cam_tensor,
                                                      vx_tensor            Cam_2_Img_tensor,
                                                      vx_tensor            input_tensor[],
                                                      vx_image             output_image);
/*! \brief Image Histogram Node
  *
  * Used to get histogram of image intensity, done on A72
  *
  * \param [in] graph              reference to the graph.
  * \param [in] input_image        Input image to draw detections
  * \param [out] output_histogram  Output histogram of image intensities (luma only)
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxImgHistNode(vx_graph             graph,
                                                 vx_image             input_image,
                                                 vx_distribution      output_histogram);

/*! \brief Structure From Motion Node
  *
  * Structure From Motion (SFM), done on C71
  *
  * \param [in] graph              reference to the graph.
  * \param [in] config             SFM parameters defined in \ref tivxSFMParams
  * \param [in] create_params      SFM parameters defined in \ref tivxSFMParams
  * \param [in] in_args            SFM parameters defined in \ref tivxSFMParams
  * \param [in] out_args           SFM parameters defined in \ref tivxSFMParams
  * \param [in] input_image        Input image in NV12 format to overlay depth estimations
  * \param [in] flow_vectors       Flow vectors in VX_DF_IMAGE_U32 format
  * \param [out] output_ptcld_image Output point cloud image
  * \param [out] output_og_image    Output occupancy grid map
  * \param [out] output_feat        Output features
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxSFMNode(vx_graph            graph,
                                            vx_user_data_object  config,
                                            vx_user_data_object  create_params,
                                            vx_user_data_object  in_args,
                                            vx_user_data_object  out_args,
                                            vx_image             input_image,
                                            vx_image             flow_vectors,
                                            vx_image             output_ptcld_image,
                                            vx_image             output_og_image,
                                            vx_user_data_object  output_feat);

/*! \brief DL Pre-processing Node
  *
  * Transforms input image to a format as required for DL processing
  *
  * \param [in] graph              reference to the graph
  * \param [in] config             reference to tivxDLPreProcPrams
  * \param [in] input_image        Input image
  * \param [out] output_tensor     Output tensor
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDLPreProcNode(vx_graph             graph,
                                                   vx_user_data_object  config,
                                                   vx_image             input_image,
                                                   vx_tensor            output_tensor);

/*! \brief DL Color Blend Node
  *
  * Takes the DL output and blends the provided color-map to the input image
  *
  * \param [in] graph              reference to the graph
  * \param [in] config             reference to tivxDLPreProcPrams
  * \param [in] input_image        Input image
  * \param [in] input_tensor       Input tensor
  * \param [out] output_image      Output image
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDLColorBlendNode(vx_graph             graph,
                                                      vx_user_data_object  config,
                                                      vx_image             input_image,
                                                      vx_tensor            input_tensor,
                                                      vx_image             output_image);

/*! \brief DL Draw Box Node
  *
  * Takes the DL output and draws boxes around an object on input image
  *
  * \param [in] graph              reference to the graph
  * \param [in] config             reference to tivxDLPreProcPrams
  * \param [in] input_image        Input image
  * \param [in] input_tensor       Input tensor
  * \param [out] output_image      Output image
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDLDrawBoxNode(vx_graph             graph,
                                                   vx_user_data_object  config,
                                                   vx_image             input_image,
                                                   vx_tensor            input_tensor,
                                                   vx_image             output_image);

/*! \brief DL Color Convert Node
  *
  * Performs limited color conversions required for DL pipeline which is DMA optimized
  *
  * \param [in] graph              reference to the graph
  * \param [in] input_image        Input image
  * \param [out] output_image      Output image
  *
  * \ingroup group_vision_apps_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDLColorConvertNode(vx_graph             graph,
                                                        vx_image             input_image,
                                                        vx_image             output_image);

/*! \brief DL Pre-processing Armv8 Node
  *
  * Transforms input image to a format as required for DL processing
  *
  * \param [in] graph              reference to the graph
  * \param [in] config             reference to tivxDLPreProcArmv8Prams
  * \param [in] input_image        Input image
  * \param [out] output_tensor     Output tensor
  *
  * \ingroup group_app_tiovx_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDLPreProcArmv8Node(vx_graph             graph,
                                                        vx_user_data_object  config,
                                                        vx_image             input_image,
                                                        vx_tensor            output_tensor);

/*! \brief DL Post Process Node
  *
  * Takes the input_image and input_tensor and provides the post processed output image according to the inference type
  * (i.e. classification, object detection or segmentation)
  *
  * \param [in] graph              reference to the graph
  * \param [in] config             reference to tivxDLPostProcPrams
  * \param [in] input_image        Input image
  * \param [in] input_tensor       Input tensor
  * \param [out] output_image      Output image
  *
  * \ingroup group_app_tiovx_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDLPostProcNode(vx_graph             graph,
                                                    vx_kernel            kernel,
                                                    vx_user_data_object  config,
                                                    vx_image             input_image,
                                                    vx_tensor            input_tensor[],
                                                    vx_image             output_image);

/*! \brief DL Color Convert Armv8 Node
  *
  * Performs limited color conversions required for DL pipeline
  *
  * \param [in] graph              reference to the graph
  * \param [in] input_image        Input image
  * \param [out] output_image      Output image
  *
  * \ingroup group_app_tiovx_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxDLColorConvertArmv8Node(vx_graph             graph,
                                                             vx_image             input_image,
                                                             vx_image             output_image);

/*! \brief C7x Image Add Node
  *
  * Performs image addition on C7x core
  *
  * \param [in]  graph              reference to the graph
  * \param [in]  imp1               Input  image
  * \param [out] img2               Input  image
  * \param [out] img_out            Output image
  *
  * \ingroup group_app_tiovx_kernels_img_proc
  *
  * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
  */
VX_API_ENTRY vx_node VX_API_CALL tivxAddImgNode(vx_graph              graph, 
                                                vx_image              img1,
                                                vx_image              img2, 
                                                vx_image              img_out);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_IMG_PROC_NODES_H_ */
