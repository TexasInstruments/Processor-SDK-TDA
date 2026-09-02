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
 *  \file tidl_detectionOutput.h
 *
 *  \brief This file defines the process function prototype of DetectionOutput layer
 */

#ifndef ITIDL_DETECTIONOUTPUT_H
#define ITIDL_DETECTIONOUTPUT_H

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "tidl_alg_int.h"
#include "tidl_device_utils.h"
#include "tidl_alg_utils.h"
#include "tidl_commonUtils.h"
#include "tidl_mathlib_utils.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define SSD_CONF_DATA_Q (((uint32_t)1U << 15U) - 1U)

#define TIDL_SOFTMAX_SCRATCH_BYTES_PER_CLASS (12)

#define TIDL_SIGMOID_SCRATCH_BYTES_PER_CLASS (10)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

void TIDL_updateLocConfHeadPtrs(sTIDL_Layer_t *TIDLLayers, sTIDL_AlgLayer_t *algLayer, void *inPtrs[]);

int32_t TIDL_allocInternalMemBuffers(sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams, float32_tidl *priorData, sTIDL_sysMemHandle_t sysMems[TIDL_SYSMEM_MAX]);

int32_t TIDL_topKSelection(const sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams, int32_t countM);

void TIDL_topKAllClassesSelection(const sTIDL_DetectOutputParams_t *params, const sTIDL_ALgDetectOutputParams_t *algDetLyrParams);

template <typename Tloc>
void TIDL_sparseLocDataFetch(const sTIDL_DetectOutputParams_t *params, const sTIDL_ALgDetectOutputParams_t *algDetLyrParams, sTIDL_AnchorBoxParams_t *anchorBox, int32_t curClass, int32_t countK);

void TIDL_sparseLocDataFetchiX(sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                               sTIDL_AnchorBoxParams_t *anchorBox, int32_t curClass, int32_t countK);

                               void TIDL_boxParamsDecoding(const sTIDL_DetectOutputParams_t *params, const sTIDL_ALgDetectOutputParams_t *algDetLyrParams, float32_tidl *priorData, int32_t countK);

int32_t TIDL_applyNMSFast(const sTIDL_DetectOutputParams_t *params, const sTIDL_ALgDetectOutputParams_t *algDetLyrParams, int32_t inCount);

int32_t TIDL_filterNotInRangeBox(const sTIDL_DetectOutputParams_t *params, const sTIDL_ALgDetectOutputParams_t *algDetLyrParams, int32_t inCount);

template <typename Tloc>
int32_t TIDL_objOuputPreperation(const sTIDL_DetectOutputParams_t *params, const sTIDL_ALgDetectOutputParams_t *algDetLyrParams, float32_tidl *priorData, float32_tidl *objData, int32_t keepKCnt, int32_t numDet, int32_t cls);

void TIDL_objOuputPreperationiX(sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                float32_tidl *priorData, float32_tidl *objData, int32_t keepKCnt, int32_t numDet, int32_t cls);

int32_t TIDL_findValidLocAndScore(void *pKerPrivArgs,
                                  const sTIDL_DetectOutputParams_t *params,
                                  sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                  float32_tidl *priorData,
                                  int32_t flowCtrl);

float32_tidl TIDL_jaccardOverlap(
    const BBox *bbox1,
    const BBox *bbox2);

template <class Tconf>
int32_t TIDL_findValidLocation_cn(const sTIDL_DetectOutputParams_t *params,
                                  sTIDL_ALgDetectOutputParams_t *algDetLyrParams,
                                  float32_tidl *priorData);

template <class Tconf>
int32_t TIDL_sparseDetScoreCalc_cn(const sTIDL_DetectOutputParams_t *params, sTIDL_ALgDetectOutputParams_t *algDetLyrParams);

void TIDL_collectLocConfHeadInfo(sTIDL_Layer_t *TIDLLayers, sTIDL_AlgLayer_t *algLayer, void *inPtrs[], void *priorData);

int32_t TIDL_getOdFindValidLocAndScoreKernelHandleSize(sTIDL_Layer_t *tidlLayer);

int32_t TIDL_odFindValidLocAndScoreKernelInit(const TIDL_CreateParams *params,
                                              sTIDL_AlgLayer_t *algLayer,
                                              sTIDL_Layer_t *tidlLayer,
                                              int32_t procType,
                                              int32_t inHeight,
                                              int32_t outHeight);

void TIDL_UpdateScaleFactors(TIDL_Handle intAlgHandle, int32_t i, int32_t updateStats, int64_t accMin, int64_t accMax);


int32_t TIDL_detectionOutputGetPerfData(void* linkHandle,
                                 double *perfData);
/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

int32_t TIDL_DetectionOutputProcess(TIDL_NetworkCommonParams *commonParams,
                                    sTIDL_AlgLayer_t *algLayer,
                                    sTIDL_Layer_t *tidlLayer,
                                    void *inPtrs[],
                                    void *outPtrs[],
                                    int32_t layerIdx);

int32_t TIDL_DetectionOutputAlloc(const TIDL_LayerSpecificParams *layerSpecificParams,
                                  const TIDL_NetworkCommonParams *commonParams,
                                  int32_t layerIdx,
                                  int32_t memorySize[TIDL_LAYER_MEMORY_MAX]);

int32_t TIDL_DetectionOutputInit(const TIDL_LayerSpecificParams *layerSpecificParams,
                                 const TIDL_NetworkCommonParams *commonParams,
                                 sTIDL_AlgLayer_t *algLayer,
                                 int32_t layerIdx,
                                 uint8_t *memory[TIDL_LAYER_MEMORY_MAX],
                                 int32_t memorySize[TIDL_LAYER_MEMORY_MAX],
                                 void **outPtr);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

/* None */

#endif /* ITIDL_DETECTIONOUTPUT_H */
