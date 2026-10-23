/*
*
* Copyright (c) 2021 Texas Instruments Incorporated
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


/**
@file      itvm_rt.h
@brief     This file defines the public interface for TVM Runtime API
           This same API supports a client on a different CPU,
           out-of-process on the same CPU, or in-process
@version 0.1 June 2021 : Initial Code

*/

/** @ingroup    iTI_DL_RT */
/*@{*/
#ifndef ITVM_RT_H_
#define ITVM_RT_H_

#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TVMRT_MAX_INPUT_TENSORS                  (16U)
#define TVMRT_MAX_TOTAL_INPUT_TENSOR_NAMES_SIZE  (512U)
#define TVMRT_MAX_TOTAL_TENSORS                  (32U)
#define TVMRT_MAX_TENSOR_DIM                     (8)

/*! \brief Parameters describing a TVMRT Tensor */
typedef struct {
  uint32_t size_in_bytes;
} sTVMRT_TensorParams_t;

/*! \brief TVMRT Tensor for TVMRT_invoke */
typedef struct {
  /*! \brief Pointer to raw tensor data */
  void *data;
} sTVMRT_Tensor_t;

typedef struct {
    uint64_t cpIn_time_start;
    uint64_t cpIn_time_end;
    uint64_t proc_time_start;
    uint64_t proc_time_end;
    uint64_t cpOut_time_start;
    uint64_t cpOut_time_end;
} sTVMRT_PerfStats_t;

/*! \brief TVMRT configuration parameters */
typedef struct {
  /** Pointer to C7x TVM deployable module */
  void * deploy_mod;
  int32_t deploy_mod_size;
  /** Input/Output tensors description */
  int32_t num_input_tensors;
  int32_t num_output_tensors;
  /** TVM input tensors names are packed 0-terminated strings, offset array
   *     points to the start of each name */
  uint32_t input_names_offset[TVMRT_MAX_INPUT_TENSORS];
  uint8_t  input_names[TVMRT_MAX_TOTAL_INPUT_TENSOR_NAMES_SIZE];
  /** TVM tensors info for recreating DLTensors to interface with TVM runtime */
  sTVMRT_TensorParams_t tensors_params[TVMRT_MAX_TOTAL_TENSORS];

  /** Flags to control TVM RT debug level */
  int32_t tvm_rt_debug_level;
  /** Flags to control TIDL debugTrace Level*/
  int32_t traceLogLevel;
  /** Level for debug trace dumps of intermediate tensors and
   *      other data buffers into file */
  int32_t traceWriteLevel;
  /**  trace files base Name */
  void * traceBaseName;
  /** Pointer to structure holding perf data */
  sTVMRT_PerfStats_t *stats;
  /** Call back Function pointer to Write Log*/
  int32_t(*TVMVprintf)(const char * format, va_list arg);
  /** Call back Function pointer to Write Binary data to a file*/
  int32_t(*TVMWriteBinToFile)(const char *fileName, void *addr, int32_t size);
  /** Call back Function pointer to read data from a binary file */
  int32_t(*TVMReadBinFromFile)(const char *fileName, void *addr, int32_t size);

  /** Maximum Tolerated delay for TIDL pre-emption in milliSecond */
  float maxPreEmptDelay;
  /** Int value of priority for selcting the target name 0(highest prior)-7(lowest prior) */
  int targetPriority;
  /** Dump all output tensors for a specified TVM node */
  int32_t tvm_rt_trace_node;
  /** Increase the size of TVM trace buffer */
  int32_t tvm_rt_trace_size;
  /** Specify core number for execution - 1 for J721E, J721S2 and any one of 1,2,3,4 for J784S4 */
  uint32_t coreNum;
  /** Force a negative test case */
  int32_t  forceNegativeTest;
  /**  File-based trace write level (0=disabled, 1=single node, 2=all nodes) */
  int32_t tvm_rt_trace_write_level;
  /**  File-based trace base path prefix  */
  char tvm_rt_trace_base_path[128];
  /**  File-based trace node filter */
  char tvm_rt_trace_write_node[128];
} sTVMRT_Params_t;


int32_t TVMRT_create(sTVMRT_Params_t *prms, void **handle);
int32_t TVMRT_invoke(void *handle,
                     sTVMRT_Tensor_t *in[], sTVMRT_Tensor_t *out[]);
/* Activate will be done by the invoke if the hanlde is not active. */
int32_t TVMRT_deactivate(void *handle);
int32_t TVMRT_delete(void *handle);

int32_t TVMRT_setParamsDefault(sTVMRT_Params_t *prms);

#ifdef __cplusplus
}
#endif

/*@}*/
/* iTI_DL_RT */

#endif /*ITVM_RT_H_ */

