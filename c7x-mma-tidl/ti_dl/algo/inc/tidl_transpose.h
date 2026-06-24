/*
*
* Copyright (c) {2022 - 2023} Texas Instruments Incorporated
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
 ----------------------------------------------------------------------------
 @file    tidl_transpose.h
 @brief   This file defines process function prototype of Transpose layer.
 @version 0.1 (Jun 2023) : Intial Layer
 ----------------------------------------------------------------------------
*/

#ifndef ITIDL_TRANSPOSE_H
#define ITIDL_TRANSPOSE_H

#include "tidl_alg_int.h"
#include "tidl_device_utils.h"
#include "tidl_alg_utils.h"
#include "tidl_commonUtils.h"

/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_Transpose
 @fn         TIDL_transposeProcess
 @brief      In the Transpose process function, layout of the data is changed
             based on the given permutation of axes
 @param      intAlgHandle : Ptr to alg internal handle
 @param      algLayer : Ptr to alg layer parameter used in Transpose layer
 @param      tidlLayer: Ptr to tidl layer parameter used in Transpose layer
 @param      inPtrs: Ptrs to input buffers to be processed
 @param      outPtrs: Ptrs to output buffers to be processed
 @param      sysMems: Ptr to memory releated buffers used in Transpose layer
 @remarks    None
 @return     status  - Add status values 
 ----------------------------------------------------------------------------
*/     
#if TIDL_NET_VERSION_FW_PASSIVE_CODE
int32_t TIDL_getTransposeKernelHandleSize(sTIDL_Layer_t *tidlLayer);
#endif
int32_t TIDL_transposeProcess(
                    TIDL_Handle          intAlgHandle,
                    sTIDL_AlgLayer_t     * algLayer, 
                    const sTIDL_Layer_t  * tidlLayer, 
                    void                 * inPtrs[], 
                    void                 * outPtrs[],
                    sTIDL_sysMemHandle_t * sysMems);        

int32_t TIDL_transposeDspProcess(
           TIDL_Handle          intAlgHandle,
           sTIDL_AlgLayer_t     * algLayer,
           void                 * inPtrs[],
           void                 * outPtrs[],
           sTIDL_DataParams_t* inDataParams,
           const sTIDL_Layer_t  * tidlLayer,
           sTIDL_sysMemHandle_t * sysMems);

int32_t TIDL_transposeKernelInit(const TIDL_CreateParams *params,
                                       sTIDL_AlgLayer_t *algLayer,
                                       sTIDL_Layer_t *tidlLayer,
                                       int32_t procType,
                                       int32_t inHeight,
                                       int32_t outHeight);

int32_t TIDL_transposeAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx,
                                   int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);

int32_t TIDL_transposeInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr);

int32_t TIDL_transposeProcessNew(TIDL_NetworkCommonParams *commonParams,
                                          sTIDL_AlgLayer_t     * algLayer,
                                          sTIDL_Layer_t        * tidlLayer,
                                          void                 * inPtrs[],
                                          void                 * outPtrs[],
                                          int32_t                layerIdx);

#if TIDL_NET_VERSION_FW_PASSIVE_CODE
int32_t TIDL_transposeKernelInitNew(const TIDL_CreateParams *params,
                                            sTIDL_AlgLayer_t *algLayer,
                                            sTIDL_Layer_t *tidlLayer,
                                            void * handle);

int32_t TIDL_transposeDspProcessNew(const TIDL_NetworkCommonParams *commonParams,
                                            const sTIDL_AlgLayer_t *algLayer,
                                            const sTIDL_Layer_t *tidlLayer,
                                            void *inPtrs[],
                                            void *outPtrs[],
                                            sTIDL_sysMemHandle_t *sysMems);
#endif
void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);

int32_t TIDL_transposeDeviceGetHandleSize(void *linkInitParams,
                                         const sLink_t              *link,
                                         const sGCHelperHandle * gcHelperHandle);
                                         
int32_t TIDL_transposeDeviceInit(void   *linkHandle,
    void                                *linkInitParams,
    const WorkloadUnitExec_CommonParams *commonParams,
    const sLink_t                       *link,
    const sGCHelperHandle               *gcHelperHandle);

int32_t TIDL_transposeDeviceExec(void                            *linkHandle,
                               const WorkloadUnitExec_LinkExecArgs *linkExecArgs,
                               int32_t                          currFlowStage[],
                               int32_t                          currIterCount);

int32_t TIDL_transposeLayerFrameInit(void *linkHandle,
                                     WorkloadUnitExec_LinkFrameInitParams_t *linkExecInitParams,
                                     const sLink_t *link);

int32_t TIDL_transposeGetPerfData(void* linkHandle, 
                                 double* perfModelEstimates);
                                    
#endif /* ITIDL_TRANSPOSE_H*/

