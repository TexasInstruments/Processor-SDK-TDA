/*
*
* Copyright (c) {2015 - 2026} Texas Instruments Incorporated
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
 *  \file tidl_lstm.h
 *
 *  \brief This file defines the process function prototype of LSTM layer
 */

#ifndef TIDL_LSTM_H_
#define TIDL_LSTM_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_device_utils.h"
#include "gc_helper.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Move similar to tidl-kernels/source/tidl_eltWise_ixx_oxx/tidl_eltWise_ixX_oxX_priv.h??? */
/* 0 - Actual Inp, followed by W, R, B, InitH, InitC, peepholeP */
#define TIDL_LSTM_MAX_NUM_INPUTS (7U)
/* 0 - Actual Out, 1 - out H, 2 - out C */
#define TIDL_LSTM_MAX_NUM_OUTPUTS (3U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/*
 * Contents of destination buffer in LSTM.
 * LSTM_OUT_Y - actual lstm output, LSTM_OUT_H - final hidden state, , LSTM_OUT_H - final cell state
 */
enum {
    LSTM_OUT_Y = 0,
    LSTM_OUT_H,
    LSTM_OUT_C,
    LSTM_MAX_OUTPUTS
};

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t TIDL_lstmProcess(TIDL_NetworkCommonParams *commonParams,
                         sTIDL_AlgLayer_t     * algLayer,
                         sTIDL_Layer_t        * tidlLayer,
                         void                 * inPtrs[],
                         void                 * outPtrs[],
                         int32_t                layerIdx);

int32_t TIDL_lstmAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                       const TIDL_NetworkCommonParams *commonParams,
                       int32_t layerIdx,
                       int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);

int32_t TIDL_lstmInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                      const TIDL_NetworkCommonParams *commonParams,
                      sTIDL_AlgLayer_t *algLayer,
                      int32_t layerIdx,
                      uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                      int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                      void **outPtr);

int32_t TIDL_lstmDeviceGetHandleSize(void                  *linkInitParams,
                                     const sLink_t         *link,
                                     const sGCHelperHandle *gcHelperHandle);

int32_t TIDL_lstmDeviceInit(void                                *linkHandle,
                            void                                *linkInitParams,
                            const WorkloadUnitExec_CommonParams *commonParams,
                            const sLink_t                       *link,
                            const sGCHelperHandle               *gcHelperHandle);

int32_t TIDL_lstmDeviceExec(void                                *linkHandle,
                             const WorkloadUnitExec_LinkExecArgs *linkExecArgs,
                             int32_t                              currFlowStage[],
                             int32_t                              currIterCount);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#endif /* TIDL_LSTM_H_*/
