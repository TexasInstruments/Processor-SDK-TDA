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


/**
@file      tidl_error.h
brief      This file defines the various errors for TI deep learning library. \n

@version 0.1 July 2025 : Initial Code
*/

/** @ingroup  TIDL_ERRORS */

/*@{*/
#ifndef TIDL_ERRORS_H_
#define TIDL_ERRORS_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Max number of error groups */
#define TIDL_ERROR_MAX_GROUPS                             (64U)

/** Each group gets 64-bits
    LSB bit [0] is reserved in each group
    MSB bit [63] indicates the MAX error type in each group  */
#define TIDL_ERROR_TYPES_PER_GROUP                        (64U)

/** Various error groups */
#define TIDL_ERROR_GROUP_COMMON                            ( 0U)
#define TIDL_ERROR_GROUP_IVISON                            ( 1U)
#define TIDL_ERROR_GROUP_WORKLOAD                          ( 2U)
#define TIDL_ERROR_GROUP_QUANTIZATION                      ( 3U)
#define TIDL_ERROR_GROUP_PADDING                           ( 4U)
#define TIDL_ERROR_GROUP_DEBUG_TRACE                       ( 5U)
#define TIDL_ERROR_GROUP_ALG_UTILS                         ( 6U)
#define TIDL_ERROR_GROUP_COMMON_UTILS                      ( 7U)
#define TIDL_ERROR_GROUP_XFR_LINK                          ( 8U)
#define TIDL_ERROR_GROUP_ALG_COMMON                        ( 9U)
#define TIDL_ERROR_GROUP_DATA                              (10U)
#define TIDL_ERROR_GROUP_CONV                              (11U)
#define TIDL_ERROR_GROUP_POOL                              (12U)
#define TIDL_ERROR_GROUP_RELU                              (13U)
#define TIDL_ERROR_GROUP_PRELU                             (14U)
#define TIDL_ERROR_GROUP_ELTWISE                           (15U)
#define TIDL_ERROR_GROUP_INNERPROD                         (16U)
#define TIDL_ERROR_GROUP_SOFTMAX                           (17U)
#define TIDL_ERROR_GROUP_BATCHNORM                         (18U)
#define TIDL_ERROR_GROUP_BIAS                              (19U)
#define TIDL_ERROR_GROUP_SCALE                             (20U)
#define TIDL_ERROR_GROUP_DECONV2D                          (21U)
#define TIDL_ERROR_GROUP_CONCAT                            (22U)
#define TIDL_ERROR_GROUP_SPLIT                             (23U)
#define TIDL_ERROR_GROUP_SLICE                             (24U)
#define TIDL_ERROR_GROUP_CROP                              (25U)
#define TIDL_ERROR_GROUP_FLATTEN                           (26U)
#define TIDL_ERROR_GROUP_DROPOUT                           (27U)
#define TIDL_ERROR_GROUP_ARGOP                             (28U)
#define TIDL_ERROR_GROUP_DETECTION                         (29U)
#define TIDL_ERROR_GROUP_SHUFFLE                           (30U)
#define TIDL_ERROR_GROUP_RESIZE                            (31U)
#define TIDL_ERROR_GROUP_ROIPOOL                           (32U)
#define TIDL_ERROR_GROUP_ODPOSTPROC                        (33U)
#define TIDL_ERROR_GROUP_DEPTHTOSPACE                      (34U)
#define TIDL_ERROR_GROUP_SIGMOID                           (35U)
#define TIDL_ERROR_GROUP_PAD                               (36U)
#define TIDL_ERROR_GROUP_COLORCONV                         (37U)
#define TIDL_ERROR_GROUP_ODOUTPUTREFORMAT                  (38U)
#define TIDL_ERROR_GROUP_DATACONVERT                       (39U)
#define TIDL_ERROR_GROUP_CUSTOM                            (40U)
#define TIDL_ERROR_GROUP_BATCHRESHAPE                      (41U)
#define TIDL_ERROR_GROUP_REDUCE                            (42U)
#define TIDL_ERROR_GROUP_SCATTERELEMENTS                   (43U)
#define TIDL_ERROR_GROUP_SQUEEZE                           (44U)
#define TIDL_ERROR_GROUP_TANH                              (45U)
#define TIDL_ERROR_GROUP_HARDSIGMOID                       (46U)
#define TIDL_ERROR_GROUP_ELU                               (47U)
#define TIDL_ERROR_GROUP_RESHAPE                           (48U)
#define TIDL_ERROR_GROUP_CONSTDATA                         (49U)
#define TIDL_ERROR_GROUP_GATHER                            (50U)
#define TIDL_ERROR_GROUP_TRANSPOSE                         (51U)
#define TIDL_ERROR_GROUP_LAYERNORM                         (52U)
#define TIDL_ERROR_GROUP_GRIDSAMPLE                        (53U)
#define TIDL_ERROR_GROUP_TOPK                              (54U)
#define TIDL_ERROR_GROUP_DEFORMCONV                        (55U)
#define TIDL_ERROR_GROUP_RMSNORM                           (56U)
#define TIDL_ERROR_GROUP_LSTM                              (57U)
#define TIDL_ERROR_GROUP_GRU                               (58U)
#define TIDL_ERROR_GROUP_RNN                               (59U)
#define TIDL_ERROR_GROUP_RSVD_BIT60                        (60U)
#define TIDL_ERROR_GROUP_RSVD_BIT61                        (61U)
#define TIDL_ERROR_GROUP_RSVD_BIT62                        (62U)
#define TIDL_ERROR_GROUP_RSVD_BIT63                        (63U)


/* Define more layer groups here ... */

/** TIDL_ERROR_GROUP_COMMON error types */
#define TIDL_ERROR_COMMON_RSVD                             ( 0U)
#define TIDL_ERROR_COMMON_UNSUPPORTED_LAYER                ( 1U)
#define TIDL_ERROR_COMMON_DATAFLOW_INFO_NULL               ( 2U)
#define TIDL_ERROR_COMMON_INVALID_NET_VERSION              ( 3U)
#define TIDL_ERROR_COMMON_INVALID_IO_LINE_PITCH            ( 4U)
#define TIDL_ERROR_COMMON_INVALID_DDR_INFO_FROM_GC         ( 5U)
#define TIDL_ERROR_COMMON_EXCEED_PARAMS_MEMTAB_REQUEST     ( 6U)
#define TIDL_ERROR_COMMON_EXCEED_DATA_MEMTAB_REQUEST       ( 7U)
#define TIDL_ERROR_COMMON_EXCEED_PRIORITY_LEVEL            ( 8U)
#define TIDL_ERROR_COMMON_EXCEED_OBJECTS_PER_LEVEL         ( 9U)
#define TIDL_ERROR_COMMON_EXCEED_OBJ_DET_MAX_HEADS         (10U)
#define TIDL_ERROR_COMMON_DATA_TYPE_NOT_SUPPORTED          (11U)
#define TIDL_ERROR_COMMON_UNSUPPORTED_AXIS                 (12U)

/** TIDL_ERROR_GROUP_IVISON error types */
#define TIDL_ERROR_IVISION_ALG_RSVD                        ( 0U)
#define TIDL_ERROR_IVISION_ALG_ALLOC_FAIL                  ( 1U)
#define TIDL_ERROR_IVISION_ALG_CREATE_FAIL                 ( 2U)
#define TIDL_ERROR_IVISION_ALG_DELETE_FAIL                 ( 3U)
#define TIDL_ERROR_IVISION_ALG_INIT_FAIL                   ( 4U)
#define TIDL_ERROR_IVISION_ALG_DEINIT_FAIL                 ( 5U)
#define TIDL_ERROR_IVISION_ALG_PROCESS_FAIL                ( 6U)

/** TIDL_ERROR_GROUP_WORKLOAD error types */
#define TIDL_ERROR_WORKLOAD_RSVD                           ( 0U)
#define TIDL_ERROR_WORKLOAD_LNKSUBTYPE_INVALID             ( 1U)
#define TIDL_ERROR_WORKLOAD_DIMNS_INVALID                  ( 2U)
#define TIDL_ERROR_WORKLOAD_PARENT_CHILD_MISMATCH          ( 3U)
#define TIDL_ERROR_WORKLOAD_DIMNS_ERROR                    ( 4U)
#define TIDL_ERROR_WORKLOAD_MEMSIZE_ERROR                  ( 5U)
#define TIDL_ERROR_WORKLOAD_LNKSUBTYPE_ERROR               ( 6U)

/** TIDL_ERROR_GROUP_PADDING error types */
#define TIDL_ERROR_PADDING_RSVD                            (0U)
#define TIDL_ERROR_PADDING_INPUT_BUF_NOT_SUPPORTED         (1U)
#define TIDL_ERROR_PADDING_INVALID_OTF                     (2U)

/** TIDL_ERROR_GROUP_QUANTIZATION error types */
#define TIDL_ERROR_QUANTIZATION_RSVD                       (0U)
#define TIDL_ERROR_QUANTIZATION_STATS_NOT_AVAILABALE       (1U)
#define TIDL_ERROR_QUANTIZATION_INVALID_ASYM               (2U)

/** TIDL_ERROR_GROUP_DEBUG_TRACE error types */
#define TIDL_ERROR_DEBUG_TRACE_RSVD                        (0U)
#define TIDL_ERROR_DEBUG_TRACE_INVALID_PARAM               (1U)

/** TIDL_ERROR_GROUP_ALG_UTILS error types */
#define TIDL_ERROR_ALGUTILS_RSVD                           (0U)
#define TIDL_ERROR_ALGUTILS_INVALID_GRPID_ERROR            (1U)
#define TIDL_ERROR_ALGUTILS_FAIL_ACTIVATE_ERROR            (2U)
#define TIDL_ERROR_ALGUTILS_INVALID_LNKTYPE_ERROR          (3U)

/** TIDL_ERROR_GROUP_COMMON_UTILS error types */
#define TIDL_ERROR_COMMNUTILS_RSVD                         (0U)
#define TIDL_ERROR_COMMNUTILS_FAIL_DMACONFIG_ERROR         (1U)
#define TIDL_ERROR_COMMNUTILS_FAIL_DMADECONFIG_ERROR       (2U)
#define TIDL_ERROR_COMMNUTILS_FAIL_DMAPREPTR_ERROR         (3U)
#define TIDL_ERROR_COMMNUTILS_FAIL_DMAVRPHYADD_ERROR       (4U)

/** TIDL_ERROR_GROUP_XFR_LINK error types */
#define TIDL_ERROR_XFR_LINK_RSVD                           (0U)
#define TIDL_ERROR_XFR_LINK_INVALID_WLCALCPTR_ERROR        (1U)

/** TIDL_ERROR_GROUP_ALG_COMMON error types */
#define TIDL_ERROR_ALG_COMMON_RSVD                         ( 0U)
#define TIDL_ERROR_ALG_COMMON_NULL_ALGHNDLE_ERROR          ( 1U)
#define TIDL_ERROR_ALG_COMMON_ZERO_NETBCK_ERROR            ( 2U)
#define TIDL_ERROR_ALG_COMMON_NULL_DMAUTILSCTNTX_ERROR     ( 3U)
#define TIDL_ERROR_ALG_COMMON_NULL_ZEROVEC1K_ERROR         ( 4U)
#define TIDL_ERROR_ALG_COMMON_NULL_MEMCPYTR_ERROR          ( 5U)
#define TIDL_ERROR_ALG_COMMON_NULL_PRMTHNDLE_ERROR         ( 6U)
#define TIDL_ERROR_ALG_COMMON_MAX_PARAMBUFMEMREC_ERROR     ( 7U)
#define TIDL_ERROR_ALG_COMMON_MAX_SCRDATABUFMEMREC_ERROR   ( 8U)
#define TIDL_ERROR_ALG_COMMON_MULTICORE_INIT_CHECK         ( 9U) /* This is not a error, this e-code is used to detect if init is called from Ivision API*/

/** TIDL_ERROR_GROUP_DATA             error types */

/** TIDL_ERROR_GROUP_CONV             error types */
#define TIDL_ERROR_CONV_RSVD                               (0U)
#define TIDL_ERROR_CONV_INVALID_INPUT_WIDTH                (1U)
#define TIDL_ERROR_CONV_INVALID_INPUT_HEIGHT               (2U)
#define TIDL_ERROR_CONV_INVALID_OUTPUT_WIDTH               (3U)
#define TIDL_ERROR_CONV_INVALID_OUTPUT_HEIGHT              (4U)
#define TIDL_ERROR_CONV_INVALID_NUM_IN_CHANNELS            (5U)
#define TIDL_ERROR_CONV_INVALID_NUM_OUT_CHANNELS           (6U)
#define TIDL_ERROR_CONV_INVALID_KERNEL_WIDTH               (7U)
#define TIDL_ERROR_CONV_INVALID_KERNEL_HEIGHT              (8U)
#define TIDL_ERROR_CONV_INVALID_KERNEL_TYPE                (9U)
#define TIDL_ERROR_CONV_INVALID_STRIDE_WIDTH               (10U)
#define TIDL_ERROR_CONV_INVALID_STRIDE_HEIGHT              (11U)
#define TIDL_ERROR_CONV_NEGATIVE_OUTPUT_SHIFT              (12U)
#define TIDL_ERROR_CONV_UNSUPPORTED_DATA_TYPE              (13U)


/** TIDL_ERROR_GROUP_POOL             error types */
#define TIDL_ERROR_POOL_RSVD                               (0U)
#define TIDL_ERROR_POOL_INVALID_INPUT_WIDTH                (1U)
#define TIDL_ERROR_POOL_INVALID_INPUT_HEIGHT               (2U)
#define TIDL_ERROR_POOL_INVALID_OUTPUT_WIDTH               (3U)
#define TIDL_ERROR_POOL_INVALID_OUTPUT_HEIGHT              (4U)
#define TIDL_ERROR_POOL_INVALID_POOL_TYPE                  (5U)
#define TIDL_ERROR_POOL_INVALID_NUM_CHANNELS               (6U)
#define TIDL_ERROR_POOL_INVALID_KERNEL_WIDTH               (7U)
#define TIDL_ERROR_POOL_INVALID_KERNEL_HEIGHT              (8U)
#define TIDL_ERROR_POOL_INVALID_STRIDE_WIDTH               (9U)
#define TIDL_ERROR_POOL_INVALID_STRIDE_HEIGHT              (10U)
#define TIDL_ERROR_POOL_GLOBALAVG_NOT_IMPLEMENTED          (11U)


/** TIDL_ERROR_GROUP_RELU             error types */
/** TIDL_ERROR_GROUP_PRELU            error types */

/** TIDL_ERROR_GROUP_ELTWISE          error types */
#define TIDL_ERROR_ELTWISE_RSVD                            (0U)
#define TIDL_ERROR_ELTWISE_INVALID_INPUT_WIDTH             (1U)
#define TIDL_ERROR_ELTWISE_INVALID_INPUT_HEIGHT            (2U)
#define TIDL_ERROR_ELTWISE_INVALID_OUTPUT_WIDTH            (3U)
#define TIDL_ERROR_ELTWISE_INVALID_OUTPUT_HEIGHT           (4U)
#define TIDL_ERROR_ELTWISE_INVALID_ELTWISE_TYPE            (5U)
#define TIDL_ERROR_ELTWISE_INVALID_NUM_CHANNELS            (6U)
#define TIDL_ERROR_ELTWISE_NOT_IMPLEMENTED                 (7U)

/** TIDL_ERROR_GROUP_INNERPRODUCT     error types */
#define TIDL_ERROR_INNERPROD_RSVD                         (0U)
#define TIDL_ERROR_INNERPROD_UNSUPPORTED_DATA_TYPE        (1U)
#define TIDL_ERROR_INNERPROD_INVALID_NUM_IN_NODES         (2U)
#define TIDL_ERROR_INNERPROD_INVALID_NUM_OUT_NODES        (3U)
#define TIDL_ERROR_INNERPROD_NEGATIVE_OUTPUT_SHIFT        (4U)
#define TIDL_ERROR_INNERPROD_INSUFFICIENT_MEM_BIAS        (5U)
#define TIDL_ERROR_INNERPROD_INPTR_NULL                   (6U)

/** TIDL_ERROR_GROUP_SOFTMAX          error types */
#define TIDL_ERROR_SOFTMAX_RSVD                           (0U)
#define TIDL_ERROR_SOFTMAX_INVALID_NUM_CHANNELS           (1U)
#define TIDL_ERROR_SOFTMAX_NOT_IMPLEMENTED                (2U)

/** TIDL_ERROR_GROUP_BATCHNORM        error types */
#define TIDL_ERROR_BATCHNORM_RSVD                         (0U)
#define TIDL_ERROR_BATCHNORM_INVALID_INPUT_WIDTH          (1U)
#define TIDL_ERROR_BATCHNORM_INVALID_INPUT_HEIGHT         (2U)
#define TIDL_ERROR_BATCHNORM_INVALID_OUTPUT_WIDTH         (3U)
#define TIDL_ERROR_BATCHNORM_INVALID_OUTPUT_HEIGHT        (4U)
#define TIDL_ERROR_BATCHNORM_INVALID_NUM_CHANNELS         (5U)
#define TIDL_ERROR_BATCHNORM_INVALID_ENABLE_RELU          (6U)
#define TIDL_ERROR_BATCHNORM_NEGATIVE_OUTPUT_SHIFT        (7U)

/** TIDL_ERROR_GROUP_BIAS             error types */
/** TIDL_ERROR_GROUP_SCALE            error types */
/** TIDL_ERROR_GROUP_DECONV2D         error types */

/** TIDL_ERROR_GROUP_CONCAT           error types */
#define TIDL_ERROR_CONCAT_RSVD                            (0U)
#define TIDL_ERROR_CONCAT_NOT_IMPLEMENTED                 (1U)


/** TIDL_ERROR_GROUP_SPLIT            error types */
/** TIDL_ERROR_GROUP_SLICE            error types */

/** TIDL_ERROR_GROUP_CROP             error types */
#define TIDL_ERROR_CROP_RSVD                               (0U)
#define TIDL_ERROR_CROP_INVALID_INPUT_WIDTH                (1U)
#define TIDL_ERROR_CROP_INVALID_INPUT_HEIGHT               (2U)
#define TIDL_ERROR_CROP_INVALID_OUTPUT_WIDTH               (3U)
#define TIDL_ERROR_CROP_INVALID_OUTPUT_HEIGHT              (4U)
#define TIDL_ERROR_CROP_INVALID_NUM_CHANNELS               (5U)
#define TIDL_ERROR_CROP_INVALID_OFFSET_WIDTH               (6U)
#define TIDL_ERROR_CROP_INVALID_OFFSET_HEIGHT              (7U)

/** TIDL_ERROR_GROUP_FLATTEN          error types */
#define TIDL_ERROR_FLATTEN_RSVD                            (0U)
#define TIDL_ERROR_FLATTEN_INVALID_INPUT_WIDTH             (1U)
#define TIDL_ERROR_FLATTEN_INVALID_INPUT_HEIGHT            (2U)
#define TIDL_ERROR_FLATTEN_INVALID_OUTPUT_WIDTH            (3U)
#define TIDL_ERROR_FLATTEN_INVALID_OUTPUT_HEIGHT           (4U)

/** TIDL_ERROR_GROUP_DROPOUT          error types */

/** TIDL_ERROR_GROUP_ARGOP           error types */
#define TIDL_ERROR_ARGOP_RSVD                              (0U)
#define TIDL_ERROR_ARGOP_INVALID_NUM_CHANNELS              (1U)
#define TIDL_ERROR_ARGOP_NOT_IMPLEMENTED                   (2U)

/** TIDL_ERROR_GROUP_DETECTION        error types */

/** TIDL_ERROR_GROUP_SHUFFLE          error types */
#define TIDL_ERROR_SHUFFLE_RSVD                            (0U)
#define TIDL_ERROR_SHUFFLE_INVALID_INPUT_WIDTH             (1U)
#define TIDL_ERROR_SHUFFLE_INVALID_INPUT_HEIGHT            (2U)
#define TIDL_ERROR_SHUFFLE_INVALID_OUTPUT_WIDTH            (3U)
#define TIDL_ERROR_SHUFFLE_INVALID_OUTPUT_HEIGHT           (4U)
#define TIDL_ERROR_SHUFFLE_INVALID_NUM_GROUPS              (5U)

/** TIDL_ERROR_GROUP_RESIZE           error types */
#define TIDL_ERROR_RESIZE_RSVD                             (0U)
#define TIDL_ERROR_RESIZE_UNSUPPORTED_ELEM_TYPE            (1U)
#define TIDL_ERROR_RESIZE_NOT_IMPLEMENTED                  (2U)
#define TIDL_ERROR_RESIZE_ADDR_NULL                        (3U)

/** TIDL_ERROR_GROUP_ROIPOOL          error types */

/** TIDL_ERROR_GROUP_ODPOSTPROC       error types */
#define TIDL_ERROR_ODPOSTPROC_RSVD                            (0U)
#define TIDL_ERROR_ODPOSTPROC_FIND_LOC_SCORE_NOT_IMPLEMENTED  (1U)

/** TIDL_ERROR_GROUP_DEPTHTOSPACE     error types */
/** TIDL_ERROR_GROUP_SIGMOID          error types */
/** TIDL_ERROR_GROUP_PAD              error types */

/** TIDL_ERROR_GROUP_COLORCONV        error types */
#define TIDL_ERROR_COLORCONV_RSVD                          (0U)
#define TIDL_ERROR_COLORCONV_NOT_IMPLEMENTED               (1U)

/** TIDL_ERROR_GROUP_ODOUTPUTREFORMAT error types */
/** TIDL_ERROR_GROUP_DATACONVERT      error types */
/** TIDL_ERROR_GROUP_CUSTOM           error types */

/** TIDL_ERROR_GROUP_BATCHRESHAPE     error types */
#define TIDL_ERROR_BATCHRESHAPE_RSVD                       (0U)
#define TIDL_ERROR_BATCHRESHAPE_NOT_IMPLEMENTED            (1U)
#define TIDL_ERROR_BATCHRESHAPE_UNSUPPORTED_DATA_TYPE      (2U)

/** TIDL_ERROR_GROUP_REDUCE           error types */
#define TIDL_ERROR_REDUCE_RSVD                             (0U)
#define TIDL_ERROR_REDUCE_NOT_IMPLEMENTED                  (1U)
#define TIDL_ERROR_REDUCE_UNSUPPORTED_AXIS                 (2U)

/** TIDL_ERROR_GROUP_SCATTERELEMENTS  error types */
/** TIDL_ERROR_GROUP_SQUEEZE          error types */
/** TIDL_ERROR_GROUP_TANH             error types */
/** TIDL_ERROR_GROUP_HARDSIGMOID      error types */
/** TIDL_ERROR_GROUP_ELU              error types */
/** TIDL_ERROR_GROUP_RESHAPE          error types */
/** TIDL_ERROR_GROUP_CONSTDATA        error types */
/** TIDL_ERROR_GROUP_GATHER           error types */

/** TIDL_ERROR_GROUP_TRANSPOSE        error types */
#define TIDL_ERROR_TRANSPOSE_RSVD                          (0U)
#define TIDL_ERROR_TRANSPOSE_NOT_IMPLEMENTED               (1U)

/** TIDL_ERROR_GROUP_LAYERNORM        error types */
#define TIDL_ERROR_LAYERNORM_RSVD                          (0U)
#define TIDL_ERROR_LAYERNORM_INSUFFICIENT_REF_SCRATCH      (1U)
#define TIDL_ERROR_LAYERNORM_UNSUPPORTED_AXIS              (2U)
#define TIDL_ERROR_LAYERNORM_NOT_IMPLEMENTED               (3U)

/** TIDL_ERROR_GROUP_RMSNORM          error types */
#define TIDL_ERROR_RMSNORM_INSUFFICIENT_REF_SCRATCH        (0U)
#define TIDL_ERROR_RMSNORM_UNSUPPORTED_AXIS                (1U)

/** TIDL_ERROR_GROUP_GRIDSAMPLE       error types */
#define TIDL_ERROR_GRIDSAMPLE_RSVD                         (0U)
#define TIDL_ERROR_GRIDSAMPLE_UNSUPPORTED_ELEM_TYPE        (1U)
#define TIDL_ERROR_GRIDSAMPLE_NOT_IMPLEMENTED              (2U)
#define TIDL_ERROR_GRIDSAMPLE_KERNEL_ERROR                 (3U)

/** TIDL_ERROR_GROUP_GATHERLAYER       error types */
#define TIDL_ERROR_GATHERLAYER_NOT_IMPLEMENTED             (1U)

/** TIDL_ERROR_GROUP_TOPK             error types */
#define TIDL_ERROR_TOPK_RSVD                               (0U)
#define TIDL_ERROR_TOPK_NOT_IMPLEMENTED                    (1U)

/** TIDL_ERROR_GROUP_DEFORMCONV       error types */
#define TIDL_ERROR_DEFORMCONV_RSVD                         (0U)
#define TIDL_ERROR_DEFORMCONV_NOT_IMPLEMENTED              (1U)

/** TIDL_ERROR_GROUP_LSTM error types */
#define TIDL_ERROR_LSTM_RSVD                               (0U)
#define TIDL_ERROR_LSTM_NOT_IMPLEMENTED                    (1U)
// #define TIDL_ERROR_LSTM_SEQ_NOT_SUPPORTED                  (2U)
#define TIDL_ERROR_LSTM_VAR_SEQ_NOT_SUPPORTED              (2U)
#define TIDL_ERROR_LSTM_CIP_NOT_SUPPORTED                  (3U)

/** TIDL_ERROR_GROUP_GRU error types */
#define TIDL_ERROR_GRU_RSVD                                (0U)
#define TIDL_ERROR_GRU_NOT_IMPLEMENTED                     (1U)
#define TIDL_ERROR_GRU_VAR_SEQ_NOT_SUPPORTED               (2U)
#define TIDL_ERROR_GRU_CLIP_NOT_SUPPORTED                  (3U)

/** TIDL_ERROR_GROUP_RNN error types */
#define TIDL_ERROR_RNN_RSVD                                (0U)
#define TIDL_ERROR_RNN_NOT_IMPLEMENTED                     (1U)
#define TIDL_ERROR_RNN_SEQ_NOT_SUPPORTED                   (2U)
#define TIDL_ERROR_RNN_VAR_SEQ_NOT_SUPPORTED               (3U)
#define TIDL_ERROR_RNN_CLIP_NOT_SUPPORTED                  (4U)

void tidl_printf(int8_t traceLevel, const char *format, ...);

#define TIDL_LOG_ERROR(group, type)\
{\
    tidl_printf(0, "Error type %s in group %s occured. File: %s Line: %d\n",\
     #type, #group, __FILE__, __LINE__);\
}

/* @} */

#ifdef __cplusplus
}
#endif

/*@}*/

#endif /*TIDL_ERRORS_H_ */
