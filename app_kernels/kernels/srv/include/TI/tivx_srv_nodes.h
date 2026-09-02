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

#ifndef TIVX_SRV_NODES_H_
#define TIVX_SRV_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Creates a POINT_DETECT Node.
 *
 * The node is the first stage of camera calibration and takes in a luma image of a calibration chart. The node detects the
 * position of the 4 corners in each of the two regions of the chart and returns their coordinates. As such it returns 8  
 * entries (4 corners x 2 for x & y coordinates) per ROI for a total of 2 ROIs
 *
 * \param [in] graph The reference to the graph.
 * \param [in] in_configuration The input object of a single params structure of the type svPointDetect_t
 * \param [in] in_ldclut The input object of a single params structure of the type svLdcLut_t. Contanins lens parameters of all 4 cameras
 * \param [in] in the input image in <tt> VX_DF_IMAGE_U8</tt> format. Only Luma plane of the image is used
 * \param [out] out_configuration The computed output configuration, an object of a single params structure of the type svACDetectStructFinalCorner_t 
 * \param [out] buf_bwluma_frame (optional) Output image in <tt> VX_DF_IMAGE_U8</tt> format.
 * \see <tt>TIVX_KERNEL_POINT_DETECT_NAME</tt>
 * \ingroup group_vision_apps_kernels_srv
 * \return <tt> vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt> vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxPointDetectNode(vx_graph graph,
                                      vx_user_data_object  in_configuration,
                                      vx_user_data_object  in_ldclut,
                                      vx_image             in,
                                      vx_user_data_object  out_configuration,
                                      vx_image             buf_bwluma_frame);


/*! \brief Creates a POSE_ESTIMATION Node.
 *
 * The node is used for generating the pose of each of the cameras in the system. The node uses the information of real 
 * world points based on the calibration chart pattern and their projection on the screen and solves a set of linear
 * equations to determine the extrinsic pose of the camera(s)
 *
 * \param [in] graph The reference to the graph.
 * \param [in] in_configuration The input object of a single params structure of the type svPoseEstimation_t
 * \param [in] in_ldclut The input object of a single params structure of the type svLdcLut_t. Contains lens parameters of all 4 cameras
 * \param [in] in_corner_points Array of objects of a single params structure of the type svACDetectStructFinalCorner_t  
 * \param [out] out_calmat The Pose of the camera(s), an object of a single params structure of the type svACCalmatStruct_t
 * \see <tt>TIVX_KERNEL_POSE_ESTIMATION_NAME</tt>
 * \ingroup group_vision_apps_kernels_srv
 * \return <tt> vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt> vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxPoseEstimationNode(vx_graph graph,
                                      vx_user_data_object  in_configuration,
                                      vx_user_data_object  in_ldclut,
                                      vx_object_array      in_corner_points,
                                      vx_user_data_object  out_calmat);


/*! \brief  Creates a GENERATE_3DBOWL Node.
 *
 * The node creates a bowl surface for geometric transformations. The existing bowl is a rectiniliear surface with elevated
 * planes with non linear surace at the intersection of planes.
 *
 * \param [in] graph The reference to the graph.
 * \param [in] in_configuration The input object of a single params structure of the type svGpuLutGen_t
 * \param [in] in_calmat The input object of a single params structure of the type svACCalmatStruct_t
 * \param [in] in_offset The input object of a single params structure of the type svGeometric_t 
 * \param [out] out_lut3dxyz  Output Array of type <tt> VX_TYPE_FLOAT32</tt> 
 * \param [out] out_calmat_scaled  The output object of a single params structure of the type svACCalmatStruct_t
 * \see <tt>TIVX_KERNEL_GENERATE_3DBOWL_NAME</tt>
 * \ingroup group_vision_apps_kernels_srv
 * \return <tt> vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt> vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxGenerate3DbowlNode(vx_graph graph,
                                      vx_user_data_object  in_configuration,
                                      vx_user_data_object  in_calmat,
                                      vx_user_data_object  in_offset,
                                      vx_array             out_lut3dxyz,
                                      vx_user_data_object  out_calmat_scaled);

/*! \brief Creates a GENERATE_GPULUT Node.
 *
 * The node performs the bowl surface to the physical camera mapping which is used by the GPU for geometric transformations
 * and for rendering the surroundview output.
 *
 * \param [in] graph The reference to the graph.
 * \param [in] in_configuration in_configuration The input object of a single params structure of the type svGpuLutGen_t 
 * \param [in] in_ldclut  The input object of a single params structure of the type svLdcLut_t. Contanins lens parameters of all 4 cameras 
 * \param [in] in_calmat_scaled The input object of a single params structure of the type svACCalmatStruct_t
 * \param [in] in_lut3dxyz Input Array of type <tt> VX_TYPE_FLOAT32</tt>
 * \param [out] out_gpulut3d  Output Array of type <tt> VX_TYPE_UINT16</tt>
 * \see <tt>TIVX_KERNEL_GENERATE_GPULUT_NAME</tt>
 * \ingroup group_vision_apps_kernels_srv
 * \return <tt> vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt> vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxGenerateGpulutNode(vx_graph graph,
                                      vx_user_data_object  in_configuration,
                                      vx_user_data_object  in_ldclut,
                                      vx_user_data_object  in_calmat_scaled,
                                      vx_array             in_lut3dxyz,
                                      vx_array             out_gpulut3d);

/*! \brief [Graph] Creates a GL_SRV Node.
 *
 * The node integrates OpenGL calls to render a SRV image from four input images.
 *
 * \param [in] graph The reference to the graph.
 * \param [in] configuration An input object of type tivx_srv_params_t used to configure OpenGL SRV
 * \param [in] input An object array of input images from a capture source which are used by OpenGL SRV
 * \param [in] srv_views (optional) An array objects of type srv_coords_t used to define the SRV view
 * \param [in] galign_lut (optional) A LUT used for geometric alignment.  If not given, the node creates a default LUT
 * \param [out] output An output SRV image created by OpenGL
 * \see <tt>TIVX_KERNEL_GL_SRV_NAME</tt>
 * \ingroup group_vision_apps_kernels_srv
 * \return <tt> vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt> vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxGlSrvNode(vx_graph graph,
                                      vx_user_data_object  configuration,
                                      vx_object_array      input,
                                      vx_object_array      srv_views,
                                      vx_array             galign_lut,
                                      vx_image             output);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_SRV_NODES_H_ */


