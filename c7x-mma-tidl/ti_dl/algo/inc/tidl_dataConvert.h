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
 ----------------------------------------------------------------------------
 @file    tidl_dataFormat.h
 @brief   This file defines process function prototype of Dat Format layer.
 @version 0.1 (May 2021) : Intial Layer
 ----------------------------------------------------------------------------
*/

#ifndef ITIDL_DATACONVERT_H
#define ITIDL_DATACONVERT_H

#include "tidl_alg_int.h"
#include "tidl_commonUtils.h"


template <class TBias, class TQ1, class TQ2 >
int32_t TIDL_dataConvertSplitBias(TBias bias,
                                       TQ1 *q1,
                                       TQ2 *q2);

void TIDL_dataConvertGetScaleAndBias(float32_tidl in_scale,
                                                float32_tidl out_scale,
                                                float32_tidl in_zf,
                                                float32_tidl out_zf,
                                                int32_t      shift,
                                                float32_tidl *scale,
                                                float32_tidl *bias,
                                                int64_t      *scaleFixed,
                                                int64_t      *biasFixed);

void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);



/**
 ----------------------------------------------------------------------------
 @ingroup    TIDL_DataCovert
 @fn         TIDL_dataConvertProcess
 @brief      In the Crop process function, a bias value is added to input data
             for each output channel.
 @param      commonParams : Ptr to common params
 @param      algLayer : Ptr to alg layer parameter used in Crop layer
 @param      tidlLayer: Ptr to tidl layer parameter used in Crop layer
 @param      inPtrs: Ptrs to input buffers to be processed
 @param      outPtrs: Ptrs to output buffers to be processed
 @param      sysMems: Ptr to memory releated buffers used in Crop layer
 @remarks    None
 @return     status  - Add status values
 ----------------------------------------------------------------------------
*/
int32_t TIDL_dataConvertProcessNew(TIDL_NetworkCommonParams *commonParams,
                              sTIDL_AlgLayer_t     * algLayer,
                              sTIDL_Layer_t        * tidlLayer,
                              void                 * inPtrs[],
                              void                 * outPtrs[],
                              int32_t                layerIdx);

int32_t TIDL_dataConvertDeviceGetHandleSize(void *linkInitParams,
                                         const sLink_t              *link,
                                         const sGCHelperHandle * gcHelperHandle);

int32_t TIDL_dataConvertDeviceInit(void                                *linkHandle,
                                     void                                *linkInitParams,
                                     const WorkloadUnitExec_CommonParams *commonParams,
                                     const sLink_t                       *link,
                                     const sGCHelperHandle *gcHelperHandle);

int32_t TIDL_dataConvertDeviceExec(void                            *linkHandle,
                               const WorkloadUnitExec_LinkExecArgs *linkExecArgs,
                               int32_t                          currFlowStage[],
                               int32_t                          currIterCount);



int32_t TIDL_dataConvertGetPerfData(void* linkHandle,
                                 double *perfData);

#endif /* ITIDL_DATACONVERT_H*/

