/*
 *
 * Copyright (c) 2019-2025 Texas Instruments Incorporated
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

#ifndef J7_TIDL_H_
#define J7_TIDL_H_

#include <VX/vx.h>
#include <VX/vx_kernels.h>
#include <VX/vx_khr_user_data_object.h>
#include <TI/tivx_tensor.h>
#include "itidl_ti.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Total number of C7X DSPs with MMAs in the system
 *
 *  Note: This should be defined in tiovx/include/TI/soc/tivx_soc_<soc>.h,
 *  but defining again here in case using an older version of tiovx
 */
#ifndef C7XMMA_COUNT
#if defined(SOC_J784S4)
#define C7XMMA_COUNT  (4U)
#elif defined(SOC_J722S) || defined(SOC_J742S2)
#define C7XMMA_COUNT  (2U)
#else
#define C7XMMA_COUNT  (1U)
#endif
#endif

#define TIVX_TIDL_J7_CHECKSUM_SIZE (64U)

/* These correspond to parameter indices of the TIDL node
 * They can be useful to aid in setting replicate parameters if
 * vxReplicateNode is used. */
#define TIVX_KERNEL_TIDL_IN_CONFIG_IDX                  (0U)
#define TIVX_KERNEL_TIDL_IN_NETWORK_IDX                 (1U)
#define TIVX_KERNEL_TIDL_IN_CREATE_PARAMS_IDX           (2U)
#define TIVX_KERNEL_TIDL_IN_IN_ARGS_IDX                 (3U)
#define TIVX_KERNEL_TIDL_IN_OUT_ARGS_IDX                (4U)
#define TIVX_KERNEL_TIDL_IN_TRACE_DATA_IDX              (5U)
#define TIVX_KERNEL_TIDL_IN_FIRST_TENSOR                (6U)
#define TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS            (6U)
#define TIVX_KERNEL_TIDL_NUM_MIN_PARAMETERS             (TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + 2U)
#define TIVX_KERNEL_TIDLV2_NUM_PARAMETERS               (TIVX_KERNEL_TIDL_NUM_BASE_PARAMETERS + 2U)
#define TIVX_KERNEL_TIDLV2_INPUT_IDX                    (6U)
#define TIVX_KERNEL_TIDLV2_OUTPUT_IDX                   (TIVX_KERNEL_TIDLV2_INPUT_IDX + 1U)

#define MAX_TENSOR_DIMS         (4u)
#define TIDL_MAX_PARAMS         (16u) //PC-- need it?

/*!
 * \brief TIDL params structure
 * \ingroup group_vision_function_tidl
 */
typedef struct{
  /** Checksum placeholder for TIDL config params */
  vx_uint8 config_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /** Checksum placeholder for TIDL network params */
  vx_uint8 network_checksum[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /** Flag to indicate if config params checksum is to be computed or not */
  vx_uint32 compute_config_checksum;

  /** Flag to indicate if network params checksum is to be computed or not */
  vx_uint32 compute_network_checksum;

  /**TIDL input/output buffer descriptor*/
  sTIDL_IOBufDesc_t ioBufDesc;

  /** Flag to enable optimization ivision alg activate, default it is disabled */
  vx_uint32 optimize_ivision_activation;

}tivxTIDLJ7Params;

/*!
 * \brief Utility function to calculate quick checksum
 * \ingroup group_vision_function_tidl
 */
static inline void tivx_tidl_j7_get_qc(uint8_t *pIn, uint8_t *pOut, uint32_t inSize)
{
  uint32_t i, j, remSize;
  uint8_t vec[TIVX_TIDL_J7_CHECKSUM_SIZE];

  /* Initialize vector */
  for(j = 0U; j < TIVX_TIDL_J7_CHECKSUM_SIZE; j++)
  {
     vec[j] = 0U;
  }

  /* Create QC */
  remSize = inSize;
  for(i = 0U; i < inSize; i+=TIVX_TIDL_J7_CHECKSUM_SIZE)
  {
    uint32_t elems;

    if (remSize < TIVX_TIDL_J7_CHECKSUM_SIZE)
    {
      elems = TIVX_TIDL_J7_CHECKSUM_SIZE - remSize;
      remSize += TIVX_TIDL_J7_CHECKSUM_SIZE;
    }
    else
    {
      elems = TIVX_TIDL_J7_CHECKSUM_SIZE;
      remSize -= TIVX_TIDL_J7_CHECKSUM_SIZE;
    }

    for(j = 0U; j < elems; j++)
    {
      vec[j] ^= pIn[i + j];
    }
  }

  /* Return QC */
  for(j = 0U; j < TIVX_TIDL_J7_CHECKSUM_SIZE; j++)
  {
    pOut[j] = vec[j];
  }
}


/*!
 * \brief TIDL params initialization
 * \ingroup group_vision_function_tidl
 */
static inline void tivx_tidl_j7_params_init(tivxTIDLJ7Params *tidlParams)
{
  (void)memset(&tidlParams->config_checksum[0], 0, TIVX_TIDL_J7_CHECKSUM_SIZE);
  (void)memset(&tidlParams->network_checksum[0], 0, TIVX_TIDL_J7_CHECKSUM_SIZE);

  tidlParams->compute_config_checksum = 0;
  tidlParams->compute_network_checksum = 0;
  tidlParams->optimize_ivision_activation = 0;

  (void)memset(&tidlParams->ioBufDesc, 0, sizeof(sTIDL_IOBufDesc_t));
}

/*! \brief [Graph] Creates a TIDL Node.
 * \param [in] graph Reference to vx_graph.
 * \param [in] kernel Reference to vx_kernel.
 * \param [in,out] appParams is an array of 5 parameters:
 *             - config vx_user_data_object type corresponding to the configuration (named string: sTIDL_IOBufDesc_t)
 *             - network vx_user_data_object type corresponding to the network (named string: TIDL_network)
 *             - createParams: vx_user_data_object type corresponding to the structure TIDL_CreateParams.
 *                             This structure contains create-time parameters and are used only one time to initialize
 *                             the node.
 *                             Currently members quantHistoryParam1, quantHistoryParam2, quantMargin need to be initialized by
 *                             the application.
 *             - inArgs: vx_user_data_object type corresponding to the structure TIDL_InArgs. Used to pass input parameters
 *                       that goes in effect during the node's process call when graph is executed. These parameters
 *                       can be updated before each frame execution of a graph. Currently the list of parameters is empty.
 *                       So inArgs is just a placeholder for future extension.
 *             - outArgs: vx_user_data_object type corresponding to the structure TIDL_outArgs. Used to collect output parameters
 *                       that are returned by the node's process call when graph is executed. These parameters
 *                       are refreshed after each frame execution of a graph. Currently the list of parameters is empty.
 *                       So outArgs is just a placeholder for future extension.
 * \param [in] input_tensors Array of input tensors
 *             This parameter is ignored when the first layer of the network is a data layer, which is most of the time.
 *             Only networks that are dependent on the output of a previous networks have first layer that are not data layer.
 * \param [out] output_tensors Array of output tensors
 *
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 * \ingroup group_vision_function_tidl
 */
VX_API_ENTRY vx_node VX_API_CALL tivxTIDLNode(vx_graph  graph,
                                              vx_kernel kernel,
                                              vx_reference appParams[],
                                              vx_tensor input_tensors[],
                                              vx_tensor output_tensors[]);


/*! 
 * \brief [Graph] Creates a TIDL Node which uses vx_object_array created using \ref tivxCreateObjectArrayFromList for tensor inputs and outputs.
 * \param [in] graph Reference to vx_graph.
 * \param [in] config vx_user_data_object type corresponding to the configuration (named string: sTIDL_IOBufDesc_t)
 * \param [in] network vx_user_data_object type corresponding to the network (named string: TIDL_network)
 * \param [in] createParams: vx_user_data_object type corresponding to the structure TIDL_CreateParams.
 *                             This structure contains create-time parameters and are used only one time to initialize
 *                             the node.
 *                             Currently members quantHistoryParam1, quantHistoryParam2, quantMargin need to be initialized by
 *                             the application.
 * \param [in] inArgs: vx_user_data_object type corresponding to the structure TIDL_InArgs. Used to pass input parameters
 *                       that goes in effect during the node's process call when graph is executed. These parameters
 *                       can be updated before each frame execution of a graph. Currently the list of parameters is empty.
 *                       So inArgs is just a placeholder for future extension.
 * \param [in] outArgs: vx_user_data_object type corresponding to the structure TIDL_outArgs. Used to collect output parameters
 *                       that are returned by the node's process call when graph is executed. These parameters
 *                       are refreshed after each frame execution of a graph. Currently the list of parameters is empty.
 *                       So outArgs is just a placeholder for future extension.
 * \param [in] traceData (optional) vx_user_data_object type corresponding to saving the trace.
 * \param [in] input_obj_array Input object array which contains a list of tensors, refer \ref tivxCreateObjectArrayFromList on how to create object array
 * \param [out] output_obj_array Output object array which contains a list of tensors, refer \ref tivxCreateObjectArrayFromList on how to create object array
 * \return <tt>\ref vx_node</tt>.
 * \retval vx_node A node reference. Any possible errors preventing a successful creation should be checked using <tt>\ref vxGetStatus</tt>
 * \ingroup group_vision_function_tidl
 */
VX_API_ENTRY vx_node VX_API_CALL tivxTIDLNodeV2(vx_graph  graph,
                                                vx_user_data_object  config,
                                                vx_user_data_object  network,
                                                vx_user_data_object  createParams,
                                                vx_user_data_object  inArgs,
                                                vx_user_data_object  outArgs,
                                                vx_user_data_object  traceData,
                                                vx_object_array input_obj_array,
                                                vx_object_array output_obj_array);


/*!
 * \brief Used for the Application to create the tidl kernel from the context.
 * \ingroup group_vision_function_tidl
 *
 * \param [in]  context             OpenVX context which must be created using \ref vxCreateContext
 * \param [in]  num_input_tensors   Number of input vx_tensor objects to be created
 * \param [in]  num_output_tensors  Number of output vx_tensor objects to be created
 *
 * \returns Handle to vx_kernel object if successful, NULL otherwise
 *
 * \note The caller of this function should check status of the return vx_kernel handle
 *       and if found to be NULL must handle or propagate error appropriately.
 *
 */
vx_kernel tivxAddKernelTIDL(vx_context context,
                            uint32_t num_input_tensors,
                            uint32_t num_output_tensors);

#ifdef __cplusplus
}
#endif

#endif /* J7_TIDL_H_ */
