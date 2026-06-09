/*
*
* Copyright (c) {2015 - 2025} Texas Instruments Incorporated
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

#ifndef _TIDL_RT_UTILS_H
#define _TIDL_RT_UTILS_H

#include "TI/tivx.h"
#include "TI/tivx_config.h"
#include "TI/tivx_task.h"
#include "TI/j7_tidl.h"
#include "../inc/itidl_rt.h"
#include "tidl_rt_profile.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TIDL_MAX_INTERMEDIATE_TENSOR (16)
#define TIDLRT_MAX_HANDLES (32)


typedef struct _AppObj
{

    uint32_t num_input_tensors;
    uint32_t num_output_tensors;

    bool is_tidl_node_tensor_list;

    int32_t shared_input_tensors[TIDL_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
    int32_t shared_output_tensors[TIDL_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];

    sTIDL_IOBufDesc_t   ioBufDesc;

    /* OpenVX references */
    vx_context context;
    vx_graph   graph;
    vx_kernel  kernel;
    vx_node    tidl_node;

    vx_user_data_object  config;
    vx_user_data_object  network;
    vx_user_data_object  createParams;
    vx_user_data_object  inArgs;
    vx_user_data_object  outArgs;
    vx_user_data_object  traceData;

    vx_tensor  input_tensors[TIDL_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_IN_BUFS];
    vx_tensor  output_tensors[TIDL_MAX_NUM_CORES * (uint32_t)TIDL_MAX_ALG_OUT_BUFS];

    vx_object_array  input_tensor_list;
    vx_object_array  output_tensor_list;

    //PC-- taks need it?
    tivx_task task;
    uint32_t stop_task;
    uint32_t stop_task_done;

    /* debug trace levels for TIDL subgraphs 0 - 3 */
    uint32_t traceLogLevel;
    /* trace file types for TIDL subgraphs layer level traces
     * 0 - no trace files
     * 1 - NCHW trace file
     * 2 - NCHW trace files with pad
     * 3 - NCHW trace files with pad + float by scale multiplication
     */
    uint32_t traceWriteLevel;
    /* directory and filename prefix for trace files
     * for example a value "/trace/trace_" would generate
     * files in /trace and will have a prefix trace_
     */
    char traceBaseName[TIDLRT_STRING_SIZE];

    uint32_t firstProcessCall;
    
    /** Int value of priority for selcting the target name */
    int targetPriority;

    /** Maximum Tolerated delay for TIDL pre-emption in milliSecond */
    float maxPreEmptDelay;

    /* Core number to be used for execution - 1 (c7x_1) for J721E and J721S2, any of 1,2,3,4, for J784S4 */
    uint32_t coreNum;

    /* Core number from which execution starts -- 1 indexed */
    int32_t coreStartIdx;

    /** TIDL RT profiling structure */
    sProfilePoints_t profilePoints;


} AppObj;

typedef struct _IntHandle_t
{
    /** Struct to an application object*/
    AppObj gAppObj;

    /** TIRL RT create time configuration parametes*/
    sTIDLRT_Params_t rtPrms;

    /** Pointer to next handle*/
    void *next;

    /** Number of intermediate inputs */
    int32_t numIntermediateIn;

    /** Pointer to intermediate input tensors*/
    sTIDLRT_Tensor_t intermediateInTensor[TIDL_MAX_INTERMEDIATE_TENSOR];

    /** Flag to indicate if the output tensor is intermediate  */
    int8_t  intermediateInTensorAllocated[TIDL_MAX_INTERMEDIATE_TENSOR];

    /** Number of intermediate outputs */
    int32_t numIntermediateOut;

    /** Pointer to intermediate output tensors*/
    sTIDLRT_Tensor_t intermediateOutTensor[TIDL_MAX_INTERMEDIATE_TENSOR];

    /** Flag to indicate if the output tensor is intermediate  */
    int8_t  intermediateOutTensorAllocated[TIDL_MAX_INTERMEDIATE_TENSOR];

} IntHandle_t;

//TIDLRT
int32_t tidl_rt_ovx_Init(void);
int32_t tidl_rt_ovx_DeInit(int memOnly);
vx_status is_tidlrt_in_tensor_same(sTIDL_IOBufDesc_t *ioBufDesc, sTIDLRT_Tensor_t *in[], uint32_t id, uint32_t tidlrt_id);
vx_status is_tidlrt_out_tensor_same(sTIDL_IOBufDesc_t *ioBufDesc, sTIDLRT_Tensor_t *in[], uint32_t id, uint32_t tidlrt_id);

vx_status cp_data_in_tidlrt_tensor_tiovx(AppObj *obj, sTIDLRT_Tensor_t *in[], void *input_buffer, uint32_t id, uint32_t tidlrt_id);
vx_status cp_data_out_tensor_tidlrt_tiovx(AppObj *obj, sTIDLRT_Tensor_t *out[], void *output_buffer, uint32_t id, uint32_t elementSize, vx_size data_type, float scale, uint32_t tidlrt_id);

#ifdef __cplusplus
}
#endif

#endif
