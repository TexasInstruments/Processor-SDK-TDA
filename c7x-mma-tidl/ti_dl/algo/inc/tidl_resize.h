/*
*
* Copyright (c) {2015 - 2024} Texas Instruments Incorporated
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
 @file    tidl_resize.h
 @brief   This file defines process function for for ShuffleChannel layer.
 // @version 0.1 (Oct 2016) : Initial version [TVS] ???
 // @version 0.2 (Jan 2017) : Cleaned up [TVS] ???
 ----------------------------------------------------------------------------
*/


#ifndef ITIDL_RESIZE_H
#define ITIDL_RESIZE_H

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"

/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_RESIZE
 @fn         TIDL_resizeProcess
 @brief      To be written
 @param      algLayer : Ptr Common Layer parameters and other layer parameters.
 @param      tidlLayer: Ptr to ShuffleChannel layer parameters which includes input, 
                        output buffer pointers also.
 @param      inPtrs: Ptr array to input buffers  in external memory.
 @param      outPtrs: Ptr to output buffer in external memory.
 @param      sysMems: Structure Ptr to internal memory buffers.
 @remarks    None
 @return     status
 ----------------------------------------------------------------------------
*/
typedef struct 
{
    uint32_t copyLineInfo;
    int32_t inputHeight;
}sTIDL_ResizeInfo_t;

    #define TIDL_COPY_NO_LINE          0x00000000U
    #define TIDL_COPY_TOP_LINE         0x00000001U
    #define TIDL_COPY_BOTTOM_LINE      0x000000002U

int32_t isOptimizedResizeAvailable(const sTIDL_ResizeLayerParams_t * params);
int32_t TIDL_getResizeFeaturePlaneSize(const TIDL_CreateParams *createParams,
const sTIDL_Layer_t *tidlLayer, int32_t inHeight);

int32_t TIDL_resizeDspProcess(
                        TIDL_Handle          intAlgHandle,
                        sTIDL_AlgLayer_t     * algLayer, 
                        sTIDL_Layer_t        * tidlLayer, 
                        void                 * inPtrs[], 
                        void                 * outPtrs[],
                        sTIDL_sysMemHandle_t * sysMems);

int32_t TIDL_resizeAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx,
                                   int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);

int32_t TIDL_resizeInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr);

int32_t TIDL_resizeProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx);

int32_t TIDL_resizeDeviceGetHandleSize(void *linkInitParams,
                                         const sLink_t              *link,
                                         const sGCHelperHandle * gcHelperHandle);

int32_t TIDL_resizeDeviceInit(void                                *linkHandle,
                                     void                                *linkInitParams,
                                     const WorkloadUnitExec_CommonParams *commonParams,
                                     const sLink_t                       *link,
                                     const sGCHelperHandle *gcHelperHandle);

int32_t TIDL_resizeDeviceExec(void                            *linkHandle,
                               const WorkloadUnitExec_LinkExecArgs *linkExecArgs,
                               int32_t                          currFlowStage[],
                               int32_t                          currIterCount);

void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);

void getResizeInfo(sTIDL_ResizeInfo_t* resizeInfo, const sTIDL_AlgLayer_t* algLayer, const sTIDL_Layer_t * tidlLayer, const TIDL_CreateParams *createParams, int32_t resizeInHeight);


int32_t TIDL_resizeGetPerfData(void* linkHandle,
                                 double *perfData);

#endif /* ITIDL_RESIZE_H */

