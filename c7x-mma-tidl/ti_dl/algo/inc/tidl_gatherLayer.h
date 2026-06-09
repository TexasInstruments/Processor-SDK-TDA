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
 *  \file tidl_gatherLayer.h
 *
 *  \brief This file defines the process function prototype of Gather Layer
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#ifndef TIDL_GATHERLAYER_H_
#define TIDL_GATHERLAYER_H_

#include "tidl_alg_int.h"
#include "tidl_device_utils.h"
#include "tidl_alg_utils.h"
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


int32_t TIDL_getGatherLayerKernelHandleSize(sTIDL_Layer_t *tidlLayer);
int32_t TIDL_getGatherLayerFeaturePlaneSize(const TIDL_CreateParams *createParams,
sTIDL_Layer_t *tidlLayer, int32_t inHeight);

int32_t TIDL_gatherLayerKernelInit(const TIDL_CreateParams *params,
                               sTIDL_AlgLayer_t     *algLayer,
                               sTIDL_Layer_t        *tidlLayer,
                               int32_t              procType,
                               int32_t              inHeight,
                               int32_t              outHeight
                               );

int32_t TIDL_gatherProcess(TIDL_Handle          intAlgHandle,
                              sTIDL_AlgLayer_t     *algLayer,
                              sTIDL_Layer_t        *tidlLayer,
                              void                 *inPtrs[],
                              void                 *outPtrs[],
                              sTIDL_sysMemHandle_t *sysMems);

int32_t TIDL_gatherDspProcess(TIDL_Handle        intAlgHandle,
                              sTIDL_AlgLayer_t     *algLayer,
                              sTIDL_Layer_t        *tidlLayer,
                              void                 *inPtrs[],
                              void                 *outPtrs[],
                              sTIDL_sysMemHandle_t *sysMems);


int32_t TIDL_gatherLayerAllocNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                   const TIDL_NetworkCommonParams *commonParams,
                                   int32_t layerIdx,
                                   int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);

int32_t TIDL_gatherLayerInitNew(const TIDL_LayerSpecificParams *layerSpecificParams,
                                const TIDL_NetworkCommonParams  *commonParams,
                                sTIDL_AlgLayer_t               *algLayer,
                                int32_t layerIdx,
                                uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                void    **outPtr);

int32_t TIDL_gatherLayerProcessNew(TIDL_NetworkCommonParams *commonParams,
                                          sTIDL_AlgLayer_t     * algLayer,
                                          sTIDL_Layer_t        * tidlLayer,
                                          void                 * inPtrs[],
                                          void                 * outPtrs[],
                                          int32_t                layerIdx);


int32_t TIDL_gatherLayerKernelInitNew(const TIDL_CreateParams *params,
                                            sTIDL_AlgLayer_t *algLayer,
                                            sTIDL_Layer_t *tidlLayer,
                                            void * handle);

int32_t TIDL_gatherLayerDspProcessNew( const TIDL_NetworkCommonParams *commonParams,
                                            const sTIDL_AlgLayer_t *algLayer,
                                            const sTIDL_Layer_t *tidlLayer,
                                            void *inPtrs[],
                                            void *outPtrs[],
                                            sTIDL_sysMemHandle_t *sysMems);

void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);

int32_t TIDL_gatherLayerDspAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                             const TIDL_NetworkCommonParams *commonParams,
                             int32_t layerIdx,
                             int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);
/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */
#endif /* TIDL_GATHERLAYER_H_*/

