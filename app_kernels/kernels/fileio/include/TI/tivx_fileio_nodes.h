/*
 *
 * Copyright (c) 2020-2026 Texas Instruments Incorporated
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

#ifndef TIVX_FILEIO_NODES_H_
#define TIVX_FILEIO_NODES_H_

#include <VX/vx.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief Creates a vx_array file write Node.
 *
 * Node accepts a vx_array and optional parameters of path and file name
 * prefix and writes the contents of the vx_array to a file.
 *
 *
 * \param [in] graph             The reference to the graph.
 * \param [in] in_array          input array
 * \param [in] file_path         path where file will be written
 *
 * \param [in] file_prefix       prefix to the file name
 * \ingroup group_vision_apps_kernels_fileio
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxWriteArrayNode(vx_graph   graph,
                                                    vx_array   in_array,
                                                    vx_array   file_path,
                                                    vx_array   file_prefix);

/*! \brief Creates a vx_image file write Node.
 *
 * Node accepts a vx_image and optional parameters of path and file name
 * prefix and writes the contents of the vx_image to a file.
 *
 *
 * \param [in] graph             The reference to the graph.
 * \param [in] in_image          input image
 * \param [in] file_path         path where file will be written
 * \param [in] file_prefix       prefix to the file name
 *
 * \ingroup group_vision_apps_kernels_fileio
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxWriteImageNode(vx_graph   graph,
                                                    vx_image   in_image,
                                                    vx_array   file_path,
                                                    vx_array   file_prefix);

/*! \brief Creates a tivx_raw_image file write Node.
 *
 * Node accepts a vx_image and optional parameters of path and file name
 * prefix and writes the contents of the vx_image to a file.
 *
 *
 * \param [in] graph             The reference to the graph.
 * \param [in] in_raw_img        input raw image
 * \param [in] file_path         path where file will be written
 * \param [in] file_prefix       prefix to the file name
 *
 * \ingroup group_vision_apps_kernels_fileio
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxWriteRawImageNode(vx_graph   graph,
                                                       tivx_raw_image   in_raw_image,
                                                       vx_array   file_path,
                                                       vx_array   file_prefix);

/*! \brief Creates a vx_tensor file write Node.
 *
 * Node accepts a vx_image and optional parameters of path and file name
 * prefix and writes the contents of the vx_tensor to a file.
 *
 *
 * \param [in] graph             The reference to the graph.
 * \param [in] in_tensor         input tensor
 * \param [in] file_path         path where file will be written
 * \param [in] file_prefix       prefix to the file name
 *
 * \ingroup group_vision_apps_kernels_fileio
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxWriteTensorNode(vx_graph   graph,
                                                     vx_tensor  in_tensor,
                                                     vx_array   file_path,
                                                     vx_array   file_prefix);

/*! \brief Creates a vx_user_data_object file write Node.
 *
 * Node accepts a vx_image and optional parameters of path and file name
 * prefix and writes the contents of the vx_user_data_object to a file.
 *
 *
 * \param [in] graph                 The reference to the graph.
 * \param [in] in_user_data_object   input user_data_object
 * \param [in] file_path             path where file will be written
 * \param [in] file_prefix           prefix to the file name
 *
 * \ingroup group_vision_apps_kernels_fileio
 *
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>vxGetStatus</tt>
 */
VX_API_ENTRY vx_node VX_API_CALL tivxWriteUserDataObjectNode(vx_graph   graph,
                                                             vx_user_data_object  in_user_data_object,
                                                             vx_array   file_path,
                                                             vx_array   file_prefix);

#ifdef __cplusplus
}
#endif

#endif /* TIVX_FILEIO_NODES_H_ */
