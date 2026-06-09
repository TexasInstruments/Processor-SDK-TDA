/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
 *  \file tidl_batchNorm.h
 *
 *  \brief This file defines the process function prototype of batch norm layer
 */

#ifndef TIDL_BATCHNORM_H_
#define TIDL_BATCHNORM_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t TIDL_batchNormDspProcess(TIDL_Handle        intAlgHandle,
                              sTIDL_AlgLayer_t     *algLayer,
                              sTIDL_Layer_t        *tidlLayer,
                              void                 *inPtrs[],
                              void                 *outPtrs[],
                              sTIDL_sysMemHandle_t *sysMems);

int32_t TIDL_batchNormAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx,
                                   int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);

int32_t TIDL_batchNormInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr);

int32_t TIDL_batchNormProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx);

int32_t TIDL_batchNormDeviceGetHandleSize(void *linkInitParams,
                                         const sLink_t              *link,
                                         const sGCHelperHandle * gcHelperHandle);

int32_t TIDL_batchNormDeviceInit(void                                *linkHandle,
                                     void                                *linkInitParams,
                                     const WorkloadUnitExec_CommonParams *commonParams,
                                     const sLink_t                       *link,
                                     const sGCHelperHandle * gcHelperHandle);

int32_t TIDL_batchNormDeviceExec(void                            *linkHandle,
                               const WorkloadUnitExec_LinkExecArgs *linkExecArgs,
                               int32_t                          currFlowStage[],
                               int32_t                          currIterCount);
                              
template <class Tin, class Tout>
int32_t TIDL_gelu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_tanh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_elu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_hardSigmoid_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_asin_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_asinh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_hardswish_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_swish_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_mish_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_log_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_abs_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_floor_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_exp_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_sin_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_erf_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_sqrt_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_pow_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_acos_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_atan_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_cos_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_cosh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_neg_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_tan_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_sinh_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_logit_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_reciprocal_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_silu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_softplus_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_softsign_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_celu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_selu_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_ceil_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_round_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

template <class Tin, class Tout>
int32_t TIDL_sign_nonLut(const void* dataIn, void* dataOut, int32_t Zx, float32_tidl Sx, int32_t Zy, float32_tidl Sy, float32_tidl param1, float32_tidl param2);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#endif /* TIDL_BATCHNORM_H_*/
