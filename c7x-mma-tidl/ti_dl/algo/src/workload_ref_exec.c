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
@file    workload_ref_exec.c
@brief   This file contains workload reference flow implementation
@version 0.1 (Oct 2022) : Initial version [ADK]
----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_types.h"
#include "tidl_layer.h"
#include "tidl_function_mapping.h"
#include "workload_ref_exec.h"
#include "tidl_priv_algo.h"
#include "gc.h"
#include "gc_helper.h"
#include "tidl_commonUtils_stats.h"

#include "tidl_forceNegativeTest.h"
/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define WORKLOAD_REF_CONTEXT_STR (0)
#define WORKLOAD_REF_CONTEXT_RSTR (1)
#define WORKLOAD_REF_BUF_NOT_FOUND (-1)
#define TIDL_ALG_INPUT (0)
#define TIDL_ALG_OUTPUT (1)

/* ========================================================================== */
/*                         Static Function Declarations                       */
/* ========================================================================== */

static int32_t WorkloadRefExec_getIndexFromDataId(int32_t dataId, int32_t dataIdArr[], int32_t numDataId, void *ptrArr[])
{
  int32_t arrIdx = WORKLOAD_REF_BUF_NOT_FOUND;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  for (int32_t i = 0; i < numDataId; i++)
  {
    if (dataId == dataIdArr[i])
    {
      arrIdx = i;
      /* Takes care of the case where the same input can appear multiple times in the dataIdArr array. In case
        a valid ptr is already stored in the ptrArr at the arrIdx position, continue searching but maintain the
        latest occurence of the data member */
      if (ptrArr[arrIdx] == NULL)
      {
        break;
      }
    }
  }
  /* LDRA_JUSTIFY_END */
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  if (!(arrIdx != WORKLOAD_REF_BUF_NOT_FOUND))
  {
    arrIdx = WORKLOAD_REF_BUF_NOT_FOUND;
  }
  /* LDRA_JUSTIFY_END */
  return arrIdx;
}

static int32_t WorkloadRefExec_getValidOffset(const sTIDL_DataParams_t *dataParams,
                                              const sTIDL_Layer_t *TIDLLayer,
                                              const sTIDL_AlgLayer_t *algLayer)
{
  int32_t offset = 0;
  int32_t padR_h = 0;
  int32_t padR_w = 0;
  int32_t isNetInOut = -1;

  for (int32_t j = 0; j < TIDLLayer->numInBufs; j++)
  {
    if (TIDLLayer->inData[j] == dataParams->dataId)
    {
      isNetInOut = TIDL_ALG_INPUT;
      break;
    }
  }
  if ((isNetInOut == -1) && (TIDLLayer->outData.dataId == dataParams->dataId))
  {
    isNetInOut = TIDL_ALG_OUTPUT;
  }
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  if (algLayer->gcHelperHandle == NULL)
  {
    if (isNetInOut == TIDL_ALG_INPUT)
    {
#ifdef HOST_EMULATION
      if (TIDLLayer->layerType == TIDL_ConvolutionLayer)
      {
        padR_h = TIDLLayer->layerParams.convParams.padH;
        padR_w = TIDLLayer->layerParams.convParams.padW;
      }
      else if (TIDLLayer->layerType == TIDL_Deconv2DLayer)
      {
        padR_h = TIDLLayer->layerParams.convParams.padH;
        padR_w = TIDLLayer->layerParams.convParams.padW;
      }
      else if (TIDLLayer->layerType == TIDL_PoolingLayer)
      {
        padR_h = TIDLLayer->layerParams.poolParams.padH;
        padR_w = TIDLLayer->layerParams.poolParams.padW;
      }
      else if ((TIDLLayer->layerType == TIDL_ResizeLayer))
      {
        padR_h = 1;
        padR_w = 1; // Need to relook this kernel. Looks like the pointers are weird
      }
      else
      {
        /*do nothing*/
      }
#endif
      offset = ((dataParams->padH - padR_h) * dataParams->pitch[TIDL_LINE_PITCH]) + (dataParams->padW - padR_w);
#ifdef HOST_EMULATION
      if ((TIDLLayer->layerType == TIDL_ConvolutionLayer) && (TIDLLayer->strideOffsetMethod != TIDL_StrideOffsetTopLeft))
      {
        int32_t inWidth = dataParams->dimValues[TIDL_DIM_WIDTH];
        int32_t inHeight = dataParams->dimValues[TIDL_DIM_HEIGHT];
        /* Line 94 : /tensorflow/core/kernels/quantized_conv_ops.cc*/
        int32_t coeffsWidth = TIDLLayer->layerParams.convParams.kernelW;
        int32_t coeffsHeight = TIDLLayer->layerParams.convParams.kernelH;
        int32_t strideWidth = TIDLLayer->layerParams.convParams.strideW;
        int32_t strideHeight = TIDLLayer->layerParams.convParams.strideH;
        int32_t dilationWidth = TIDLLayer->layerParams.convParams.dilationW;
        int32_t dilationHeight = TIDLLayer->layerParams.convParams.dilationH;
        int32_t effectiveKw = ((coeffsWidth - 1) * dilationWidth) + 1;
        int32_t effectiveKh = ((coeffsHeight - 1) * dilationHeight) + 1;
        int32_t filter_left_offset =
            (((TIDLLayer->outData.dimValues[TIDL_DIM_WIDTH] - 1) * strideWidth) + (effectiveKw - inWidth)) / 2;
        int32_t filter_top_offset =
            (((TIDLLayer->outData.dimValues[TIDL_DIM_HEIGHT] - 1) * strideHeight) + (effectiveKh - inHeight)) / 2;

        offset = offset + (((effectiveKh / 2) * dataParams->pitch[TIDL_LINE_PITCH]) + (effectiveKw / 2));
        offset = offset - ((filter_top_offset * dataParams->pitch[TIDL_LINE_PITCH]) + filter_left_offset);
      }
#endif
    }
    else if (isNetInOut == TIDL_ALG_OUTPUT)
    {
      offset = ((dataParams->padH) * dataParams->pitch[TIDL_LINE_PITCH]) + (dataParams->padW);
    }
    else
    {
      /*do nothing*/
    }
  }
  /* LDRA_JUSTIFY_END */

  return offset;
}

static inline void WorkloadRefExec_getPtrsFromWorkload(const TIDL_NetworkCommonParams *commonParams,
                                                       const sWorkloadUnit_t *workloadUnit,
                                                       const sTIDL_AlgLayer_t *algLayer,
                                                       sTIDL_Layer_t *tidlLayer,
                                                       void *inPtrs[],
                                                       void *outPtrs[])
{
  int32_t linkIdx = 0;
  int32_t jointIdx = 0;

  const sGCHelperHandle *gcHelperHandle = algLayer->gcHelperHandle;
  void *refInPtrs[TIDL_NUM_IN_BUFS] = {0};
  void *refOutPtrs[TIDL_NUM_OUT_BUFS] = {0};

  sLink_t *linkPtrList[MAX_LINKS_PER_WL];
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  if(workloadUnit != NULL)
  {
    /* LDRA_JUSTIFY_END */
    /* Searches the workload for input/output buffer and stores the pointer at the
      corresponding position in the inPtrs/outPtrs array. The position of pointer
      in the array is decided based on tidlLayer->inData or outData list whichever
      ve the case. */
    for (linkIdx = 0; linkIdx < workloadUnit->numLinks; linkIdx++)
    {
      getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
      sLink_t *link = linkPtrList[0];

      for (jointIdx = 0; jointIdx < (link->numSrc + 1); jointIdx++)
      {
        const sJoint_t *joint;
        const sBufParams_t *bufParams;

        if (jointIdx == link->numSrc)
        {
          joint = &link->sink;
        }
        else
        {
          joint = &link->src[jointIdx];
        }

        bufParams = getBufParamsFromBufIndex(gcHelperHandle, joint->bufDBindex);

        if ((bufParams->type == (int32_t)BUF_FM_FULL) &&
            (!((link->subType == (int32_t)LINK_X_FM_CTXT_ST) || (link->subType == (int32_t)LINK_X_FM_CTXT_RST) || (link->subType == (int32_t)LINK_X_FM_CTXT_ACROSS_CORE))))
        {
          int32_t arrIdx;
          sMetaDataID_t pMetaDataID = {0, 0, 0, 0, 0, 0};
          void **ptrToArrMem = &refOutPtrs[TIDL_NUM_OUT_BUFS - 1]; // initializing with first index pointer to avoid MISRA error
          void *gcPtr;

          int32_t elemSize = getByteCnt(bufParams->dataType);
          getMetaDataID(bufParams->dataId, &pMetaDataID);

          gcPtr = TIDL_getPointerFromGC(joint, gcHelperHandle, commonParams->tidlCommonParams->sysScratchPtr);

          if (jointIdx < link->numSrc)
          {
            arrIdx = WorkloadRefExec_getIndexFromDataId(pMetaDataID.layerId, tidlLayer->inData, tidlLayer->numInBufs, refInPtrs);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if (arrIdx >= 0)
            /* LDRA_JUSTIFY_END */
            {
              ptrToArrMem = &refInPtrs[arrIdx];
              if ((gcPtr == NULL) || (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY))
              {
                if((tidlLayer->layerType == TIDL_GatherLayer) && (tidlLayer->layerParams.gatherParams.axis == TIDL_DIM_WIDTH) && (arrIdx == 1)){
                  /*in width wise gather, we are doing process in reverse order. So from NC offset will come in indices, that is not required in ref with nc.*/
                  gcPtr = (uint8_t *)inPtrs[arrIdx];
                }
                else{
                  gcPtr = (uint8_t *)inPtrs[arrIdx] + (joint->offset * elemSize);
                }
              }
            }
          }
          else
          {
            arrIdx = WorkloadRefExec_getIndexFromDataId(pMetaDataID.layerId, &tidlLayer->outData.dataId, tidlLayer->numOutBufs, refOutPtrs);
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data. This code is maintained to avoid array out of bound error.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if (arrIdx >= 0) // checking arrIdx before accessing, MISRA caught this issue
            /* LDRA_JUSTIFY_END */
            {
              ptrToArrMem = &refOutPtrs[arrIdx];
              if((gcPtr == NULL) || (((uint32_t)commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY))
              {
                if((tidlLayer->layerType == TIDL_GatherLayer) && (tidlLayer->layerParams.gatherParams.axis == TIDL_DIM_WIDTH)){
                  /*in width wise gather, we are doing process in reverse order. So from NC offset will come in output, that is not required in ref with nc.*/
                  gcPtr = (uint8_t *)outPtrs[arrIdx];
                }
                else{
                  gcPtr = (uint8_t *)outPtrs[arrIdx] + (joint->offset * elemSize);
                }
              }
            }
          }

          gcPtr = (uint8_t *)gcPtr + (joint->superOffset * (algLayer->wlRepeatIter * elemSize));
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          if (((uintptr_t)gcPtr < (uintptr_t)(*ptrToArrMem)) || ((*ptrToArrMem) == NULL))
          {
            /* LDRA_JUSTIFY_END */
            *ptrToArrMem = gcPtr;
          }
        }
      }
    }
  }

  (void)memcpy(inPtrs, refInPtrs, sizeof(refInPtrs[0]) * TIDL_NUM_IN_BUFS);
  (void)memcpy(outPtrs, refOutPtrs, sizeof(refOutPtrs[0]) * TIDL_NUM_OUT_BUFS);
}

static inline void WorkloadRefExec_getPtrsFromTIDL(const TIDL_NetworkCommonParams *commonParams,
                                                   sWorkloadUnit_t *workloadUnit,
                                                   const sTIDL_AlgLayer_t *algLayer,
                                                   sTIDL_Layer_t *tidlLayer,
                                                   void *inPtrs[],
                                                   void *outPtrs[])
{
  int32_t bufIdx = 0;
  int32_t elemSize = 0;
  sTIDL_DataParams_t *dataParams;

  for (bufIdx = 0; bufIdx < tidlLayer->numInBufs; bufIdx++)
  {
    dataParams = TIDL_getDataParams(commonParams->createParams->net, tidlLayer->inData[bufIdx]);
    elemSize = TIDL_getDatElementSize(dataParams->elementType);
    inPtrs[bufIdx] = (void *)((uint8_t *)inPtrs[bufIdx] + (WorkloadRefExec_getValidOffset(dataParams, tidlLayer, algLayer) * elemSize));
  }

  dataParams = &tidlLayer->outData;
  elemSize = TIDL_getDatElementSize(dataParams->elementType);
  outPtrs[0] = (void *)((uint8_t *)outPtrs[0] + (WorkloadRefExec_getValidOffset(dataParams, tidlLayer, algLayer) * elemSize));
}

void WorkloadRefExec_getPtrs(TIDL_NetworkCommonParams *commonParams,
                             sWorkloadUnit_t *workloadUnit,
                             sTIDL_AlgLayer_t *algLayer,
                             sTIDL_Layer_t *tidlLayer,
                             void *refInPtrs[],
                             void *refOutPtrs[])
{
  int32_t isGcSupportAvailable = 0;
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
<justification end> */
#endif
  if (workloadUnit != NULL)
  {
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_END */
#endif
    if ((tidlLayer->layerType != TIDL_DetectionOutputLayer) &&
        (tidlLayer->layerType != TIDL_CustomLayer) &&
        (tidlLayer->layerType != TIDL_ScatterElementsLayer))
    {
      isGcSupportAvailable = 1;
    }
    #ifdef HOST_EMULATION
    if (((commonParams->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY) && 
        ((tidlLayer->layerType == TIDL_LSTMLayer) ||
         (tidlLayer->layerType == TIDL_GRULayer)  ||
         (tidlLayer->layerType == TIDL_RNNLayer)))
    {
      isGcSupportAvailable = 0;
    }
    #endif
  }

/* Detection output layer and custom layer internally handles offsets and only takes input pointers from GC */
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
<justification end> */
#endif
  if (tidlLayer->layerType != TIDL_CustomLayer)
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_END */
#endif
  {
    if (isGcSupportAvailable == 1)
    {
      WorkloadRefExec_getPtrsFromWorkload(commonParams, workloadUnit, algLayer, tidlLayer, refInPtrs, refOutPtrs);
    }
    else
    {
      WorkloadRefExec_getPtrsFromTIDL(commonParams, workloadUnit, algLayer, tidlLayer, refInPtrs, refOutPtrs);
    }
  }
}

void WorkloadRefExec_getRefParams(WorkloadRefExec_RefExecParams *wlRefParams,
                                  const TIDL_CreateParams *createParams,
                                  const sWorkloadUnit_t *workloadUnit,
                                  const sTIDL_Layer_t *tidlLayer,
                                  const sTIDL_AlgLayer_t *algLayer)
{
  const sGCHelperHandle *gcHelperHandle = algLayer->gcHelperHandle;
  // int32_t wlRepeatIter = algLayer->wlRepeatIter;
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];

  (void)memset(wlRefParams, 0, sizeof(*wlRefParams));

  for (int32_t inBuf = 0; inBuf < tidlLayer->numInBufs; inBuf++)
  {
    sTIDL_DataParams_t *dataParams = &createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[inBuf]].outData;
    wlRefParams->origInHeight[inBuf] = dataParams->dimValues[TIDL_DIM_HEIGHT];
    wlRefParams->origInChPitch[inBuf] = dataParams->pitch[TIDL_CHANNEL_PITCH];
  }
  wlRefParams->origOutHeight = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  wlRefParams->outTileHeight = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  wlRefParams->origOutChPitch = tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH];
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start>
Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
<justification end> */
#endif
  if ((workloadUnit != NULL) && (gcHelperHandle != NULL))
  {
#ifndef HOST_EMULATION
/* LDRA_JUSTIFY_END */
#endif
    sWLMetaDataID_t wlMetaData = {0, 0, 0, 0, 0, 0, 0};
    getWLMetaDataID(workloadUnit->metadata, &wlMetaData);
    #if TIDL_DEVICE_MULTICORE
    int32_t multiCoreCtxCopyLinkIdx = 0;
    #endif

    for (int32_t linkIdx = 0; linkIdx < workloadUnit->numLinks; linkIdx++)
    {
      getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
      sLink_t *link = linkPtrList[0];
      if (link->subType == (int32_t)LINK_X_FM_CTXT_ST)
      {
        wlRefParams->ctxtStrLink = link;
      }
      else if (link->subType == (int32_t)LINK_X_FM_CTXT_RST)
      {
        // Restore doesn't happen for the first normal tile when there is not first tile
        if (!((wlMetaData.passiveLinkIndex != NOT_VALID) && (wlMetaData.tileType == NORMAL_TILE) && (algLayer->wlRepeatIter == 0)))
        {
          wlRefParams->ctxtRstrLink = link;
        }
      }
#if TIDL_DEVICE_MULTICORE
      else if (link->subType == (int32_t)LINK_X_FM_CTXT_ACROSS_CORE)
      {
        wlRefParams->multiCoreCtxCopyLink[multiCoreCtxCopyLinkIdx] = link;
        multiCoreCtxCopyLinkIdx++;
      }
#endif
      else
      {
        /*do nothing*/
      }
    }

    wlRefParams->outTileHeight = wlMetaData.outTileHeight;
    wlRefParams->procType = algLayer->procType;
  }
}

void WorkloadRefExec_strRstrContext(TIDL_Handle algHandle,
                                    const WorkloadRefExec_RefExecParams *wlRefParams,
                                    int32_t wlIter,
                                    const sWorkloadUnit_t *workloadUnit,
                                    const int32_t *multiCoreCtxtLinkIndex,
                                    int32_t trigContext)
{
  if ((wlRefParams->ctxtRstrLink != NULL) && (trigContext == TIDL_CONTEXT_RSTR))
  {
    uint8_t *contextPtr = (uint8_t *)TIDL_getPointerFromGC(&wlRefParams->ctxtRstrLink->src[0], algHandle->gcHelperHandle, &algHandle->sysScratchPtr);
    uint8_t *rstrPtr = (uint8_t *)TIDL_getPointerFromGC(&wlRefParams->ctxtRstrLink->sink, algHandle->gcHelperHandle, &algHandle->sysScratchPtr);

    int32_t elemSize = getByteCnt(getBufParamsFromBufIndex(algHandle->gcHelperHandle, wlRefParams->ctxtRstrLink->sink.bufDBindex)->dataType);

    (void)TIDL_memcpy2D(rstrPtr,
                        contextPtr,
                        wlRefParams->ctxtRstrLink->src[0].flowCnt[FLOW_PIPELINE][0] * elemSize,
                        wlRefParams->ctxtRstrLink->src[0].flowCnt[FLOW_PIPELINE][1],
                        wlRefParams->ctxtRstrLink->sink.flowDim[FLOW_PIPELINE][0] * elemSize,
                        wlRefParams->ctxtRstrLink->src[0].flowDim[FLOW_PIPELINE][0] * elemSize,
                        algHandle->dmaUtilsContext,
                        algHandle->memcpyTr);
  }

  if ((wlRefParams->ctxtStrLink != NULL) && (trigContext == TIDL_CONTEXT_STR))
  {
    uint8_t *contextPtr = (uint8_t *)TIDL_getPointerFromGC(&wlRefParams->ctxtStrLink->sink, algHandle->gcHelperHandle, &algHandle->sysScratchPtr);
    uint8_t *strPtr = (uint8_t *)TIDL_getPointerFromGC(&wlRefParams->ctxtStrLink->src[0], algHandle->gcHelperHandle, &algHandle->sysScratchPtr);

    int32_t elemSize = getByteCnt(getBufParamsFromBufIndex(algHandle->gcHelperHandle, wlRefParams->ctxtStrLink->sink.bufDBindex)->dataType);

    (void)TIDL_memcpy2D(contextPtr,
                        strPtr,
                        wlRefParams->ctxtStrLink->src[0].flowCnt[FLOW_PIPELINE][0] * elemSize,
                        wlRefParams->ctxtStrLink->src[0].flowCnt[FLOW_PIPELINE][1],
                        wlRefParams->ctxtStrLink->sink.flowDim[FLOW_PIPELINE][0] * elemSize,
                        wlRefParams->ctxtStrLink->src[0].flowDim[FLOW_PIPELINE][0] * elemSize,
                        algHandle->dmaUtilsContext,
                        algHandle->memcpyTr);
  }
#if TIDL_DEVICE_MULTICORE
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];

  for (int32_t i = 0; i < MAX_MULTI_CORE_CTX_COPY_LINK; i++)
  {
    if ((multiCoreCtxtLinkIndex[i] != NOT_VALID) && (trigContext == TIDL_CONTEXT_COPY_MULTI_CORE))
    {
      getLinkPtrs(workloadUnit, NOT_VALID, multiCoreCtxtLinkIndex[i], linkPtrList);
      sLink_t *link = linkPtrList[0];

      uint8_t *contextPtr = (uint8_t *)TIDL_getPointerFromGC(&link->sink, algHandle->gcHelperHandle, &algHandle->sysScratchPtr);
      uint8_t *strPtr = (uint8_t *)TIDL_getPointerFromGC(&link->src[0], algHandle->gcHelperHandle, &algHandle->sysScratchPtr);

      int32_t elemSize = getByteCnt(getBufParamsFromBufIndex(algHandle->gcHelperHandle, link->sink.bufDBindex)->dataType);

      (void)TIDL_memcpy2D(contextPtr,
                          strPtr,
                          link->src[0].flowCnt[FLOW_PIPELINE][0] * elemSize,
                          link->src[0].flowCnt[FLOW_PIPELINE][1],
                          link->sink.flowDim[FLOW_PIPELINE][0] * elemSize,
                          link->src[0].flowDim[FLOW_PIPELINE][0] * elemSize,
                          algHandle->dmaUtilsContext,
                          algHandle->memcpyTr);
    }
  }
#endif
}

int32_t WorkloadRefExec_enableRefExecFlow(uint32_t flowCtrl,
                                          const sTIDL_Layer_t *tidlLayer,
                                          const sTIDL_AlgLayer_t *algLayer)
{
  int32_t isRefExecFlowEnabled = 0;
#ifdef HOST_EMULATION
  if ((flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
  {
    /* Ref flow should execute workload exec regardless */
    isRefExecFlowEnabled = 1;
  }
  else
#endif
      if (algLayer->workloadUnit == NULL)
  {
    /* Unsupported layers */
    isRefExecFlowEnabled = 1;
  }
  else if (tidlLayer->layerType == TIDL_ResizeLayer)
  {
    /* Resize unoptimized flow executes in ref mode */
    if (algLayer->layerParams.resizeParams.isOptResizeAvail == 0)
    {
      isRefExecFlowEnabled = 1;
    }
  }
  else if (tidlLayer->layerType == TIDL_SoftMaxLayer)
  {
#ifdef HOST_EMULATION
    if (((flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)) 
    {
      isRefExecFlowEnabled = 1;
    }
#endif
  }
  else if (tidlLayer->layerType == TIDL_ShuffleChannelLayer)
  {
    isRefExecFlowEnabled = 1;
  }
  else if (tidlLayer->layerType == TIDL_PadLayer)
  {
    isRefExecFlowEnabled = 1;
  }
  else
  {
    /*do nothing*/
  }

  return isRefExecFlowEnabled;
}

#ifdef HOST_EMULATION
static void WorkloadRefExec_splitGroupedConv(int32_t tileProcIndex,
                                             sTIDL_Layer_t *tidlLayer,
                                             sTIDL_AlgLayer_t *algLayer)
{
  // Probably should move this init to the layer level file
  if ((tidlLayer->layerType == TIDL_ConvolutionLayer) || (tidlLayer->layerType == TIDL_Deconv2DLayer))
  {
    algLayer->layerParams.convParams.coeffRefExtraOffset = 0U;
    algLayer->layerParams.convParams.biasRefExtraOffset = 0U;
  }

  if (algLayer->gcHelperHandle != NULL)
  {
    if (tidlLayer->layerType == TIDL_ConvolutionLayer)
    {
      sTIDL_ConvParams_t *convParams = &tidlLayer->layerParams.convParams;
      sWorkloadUnitAuxilary_t *auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t *)algLayer->gcHelperHandle->pAuxWLDB->pWLAuxList;
      const sWorkloadUnitAuxilary_t *workloadOutParams = &auxilaryWorkloadDB[tileProcIndex];

      /*Assuming
        1) Tiling across channel dimension not happening
      */
      if (workloadOutParams->outSubTensorDims.channel < workloadOutParams->outFullTensorDims.channel)
      {
        int32_t groupIdx = workloadOutParams->outSubTensorPosition.startChannel / workloadOutParams->outSubTensorDims.channel;
        int32_t weightSize = TIDL_ALIGN_CEIL(tidlLayer->weightsElementSizeInBits, 8) / 8;

        algLayer->layerParams.convParams.coeffRefExtraOffset = groupIdx *
                                                               ((convParams->numInChannels / convParams->numGroups) *
                                                                (convParams->kernelH * convParams->kernelW) *
                                                                (convParams->numOutChannels / convParams->numGroups)) *
                                                               weightSize;

        algLayer->layerParams.convParams.biasRefExtraOffset = groupIdx * (convParams->numOutChannels / convParams->numGroups);
      }

      if ((convParams->numGroups > 1) && (convParams->numGroups < convParams->numInChannels))
      {
        if (workloadOutParams->outSubTensorDims.channel < workloadOutParams->outFullTensorDims.channel)
        {
          convParams->numInChannels = convParams->numInChannels / convParams->numGroups;
          convParams->numOutChannels = convParams->numOutChannels / convParams->numGroups;
          convParams->numGroups = 1;
        }
      }
    }
  }
}
#endif

/* ========================================================================== */
/*                         Public  Function Declarations                      */
/* ========================================================================== */

int32_t WorkloadRefExec_GetOutBufIdx(const sWorkloadUnit_t *workloadUnit, const sGCHelperHandle *gcHelperHandle)
{
  int32_t bufDbIdx = IALG_EOK;
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];
  if ((gcHelperHandle != NULL) && (workloadUnit != NULL))
  {
    for (int32_t linkIdx = workloadUnit->numLinks - 1; linkIdx >= 0; linkIdx--)
    {
      getLinkPtrs(workloadUnit, NOT_VALID, linkIdx, linkPtrList);
      const sLink_t *link = linkPtrList[0];
      const sBufParams_t *buf = getBufParamsFromBufIndexStrict(gcHelperHandle, link->sink.bufDBindex);

      if (buf != NULL)
      {
        if (buf->type == (int32_t)BUF_FM_FULL)
        {
          bufDbIdx = link->sink.bufDBindex;
          break;
        }
      }
      else
      {
        tidl_printf(0, "Outbuffer not found! \n");
        bufDbIdx = NOT_VALID; // assert(0);
      }
    }
    /* bufDbIdx = NOT_VALID is verified in getBufParamsFromBufIndex() function and
       should not be used further in the code */
  }
  else
  {
    bufDbIdx = NOT_VALID;
  }
  return bufDbIdx;
}

/**
 *  \brief This function runs workload through the reference flow.
 *
 *  \param  algHandle      [IN] Handle for the dataFlow ( this is created via WorkloadUnitExec_Init API)
 *  \param  commonParams   [IN] Common params structure required for process
 *  \param  workloadUnit   [IN] Workload structure containing gc info
 *  \param  algLayer       [IN] Pointer to algLayer structure
 *  \param  tidlLayer      [IN] Pinter to tidlLayer structure coming from net structure
 *  \param  inPtrs[]       [IN] List of pointers to inputs (Expected to be A pointer)
 *  \param  outPtrs[]      [OUT] Pointer to the output tensor (Expected to be A pointer)
 *  \param  layerId        [IN] Layer Index
 *
 *  \return Error Status
 */
int32_t WorkloadRefExec_Process(TIDL_Handle algHandle,
                                TIDL_NetworkCommonParams *commonParams,
                                sWorkloadUnit_t *workloadUnit,
                                sTIDL_AlgLayer_t *algLayer,
                                sTIDL_Layer_t *tidlLayer,
                                void *inPtrs[],
                                void *outPtrs[],
                                int32_t layerId,
                                int32_t coreId)
{
  int32_t status = IALG_EOK;
  void *refInPtrs[TIDL_NUM_IN_BUFS] = {0};
  void *refOutPtrs[TIDL_NUM_OUT_BUFS] = {0};
  WorkloadRefExec_RefExecParams wlRefParams;
  int32_t isRefExecFlowEnabled = WorkloadRefExec_enableRefExecFlow(algHandle->createParams->flowCtrl, tidlLayer, algLayer);

  (void)memcpy(refInPtrs, inPtrs, sizeof(refInPtrs[0]) * TIDL_NUM_IN_BUFS);
  (void)memcpy(refOutPtrs, outPtrs, sizeof(refOutPtrs[0]) * TIDL_NUM_OUT_BUFS);

  if (isRefExecFlowEnabled == 1)
  {
    WorkloadRefExec_getRefParams(&wlRefParams, algHandle->createParams, workloadUnit, tidlLayer, algLayer);
  }

  if ((isRefExecFlowEnabled == 1))
  {
    /* Gets B/C pointer */
    WorkloadRefExec_getPtrs(commonParams, workloadUnit, algLayer, tidlLayer, refInPtrs, refOutPtrs);
#ifdef HOST_EMULATION
    /* Temporary fix to emulate unoptimized grouped convolution in reference flow */
    WorkloadRefExec_splitGroupedConv(algHandle->currAlgLayer, tidlLayer, algLayer);
#endif
  }

  status = TIDL_layerProcessNew(commonParams, algLayer, tidlLayer, refInPtrs, refOutPtrs, layerId);

#ifdef HOST_EMULATION
  if ((status == IALG_EOK) && (algHandle->gcHelperHandle == NULL)) // Only for stats collection run
  {
    if ((((uint32_t)algHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT) ||
        (algHandle->createParams->quantRangeUpdateFactor != 0.0f))
    {
      status = TIDL_updateAllLayersTensorStats(algHandle, tidlLayer, outPtrs, layerId);
    }
  }
#endif
  if (isRefExecFlowEnabled == 1)
  {
    if (status == IALG_EOK)
    {
      WorkloadRefExec_strRstrContext(algHandle, &wlRefParams, algLayer->wlRepeatIter, workloadUnit, algLayer->multiCoreCtxtLinkIndex, TIDL_CONTEXT_RSTR);
    }

    if (status == IALG_EOK)
    {
#ifdef HOST_EMULATION
      if (commonParams->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_WL_EXECPROC_FORCE_PADCOLS_INVALID)
      {
        algLayer->wlPadParams.tileType = 1;
        algLayer->wlPadParams.isFirstTile = 1;
      }
#endif
      // Need to fix the outPtr here
      status = WorkloadUnitExec_padBufferExec(outPtrs[0], algLayer->wlRepeatIter, tidlLayer, commonParams->tidlCommonParams,
                                              &algLayer->wlPadParams, workloadUnit, algLayer->gcHelperHandle,
                                              tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH] * TIDL_getDatElementSize(tidlLayer->outData.elementType),
                                              tidlLayer->outData.pitch[TIDL_ROI_PITCH] * TIDL_getDatElementSize(tidlLayer->outData.elementType));
    }

    if (status == IALG_EOK)
    {
      WorkloadRefExec_strRstrContext(algHandle, &wlRefParams, algLayer->wlRepeatIter, workloadUnit, algLayer->multiCoreCtxtLinkIndex, TIDL_CONTEXT_STR);
    }
  }
#if TIDL_DEVICE_MULTICORE
  int32_t numCores = 0;
  if ((status == IALG_EOK) && (workloadUnit != NULL) && (commonParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency))
  {
    uint64_t tStart, tEnd;
    numCores = commonParams->net->numCores;
    if (algLayer->multiCoreCtxtLinkIndex[0] != NOT_VALID)
    {
      /* Sync to check whether all cores' compute is done and cores are ready for DMA transfer across each other -- this sync is required only if cross core DMA is needed
      else this can result in hang for layers executing on single core */

      TIDL_syncWl(algHandle->controlArgs.ctrlSetArgs, {}, TIDL_SYNC_POINT_DMA_READY, workloadUnit->wlExecId, coreId, numCores);
    }
    TIDL_getTscTime(&tStart); /* Start counting multi core context copy cycles after sync across all cores complete */
    WorkloadRefExec_strRstrContext(algHandle, &wlRefParams, algLayer->wlRepeatIter, workloadUnit, algLayer->multiCoreCtxtLinkIndex, TIDL_CONTEXT_COPY_MULTI_CORE);
    TIDL_getTscTime(&tEnd);
    TIDL_updateprofileData(algHandle->alglayerParams[algHandle->currAlgLayer].metaData.profilePoint,
                           TIDL_PROFILE_CONTEXT_CROSSCORE_COPY,
                           tStart,
                           tEnd);
  }
#endif

/*over writing Layer meta data with performance data from workload*/
//TODO: remove when silicon is available
#if defined (SOC_TDA54) && defined (HOST_EMULATION)
  if(algHandle->gcHelperHandle != NULL)/*should not be populated during stats collection run*/
  {
    TIDL_updateprofileData(algHandle->alglayerParams[algHandle->currAlgLayer].metaData.profilePoint,
                          TIDL_PROFILE_LAYER,
                          0,
                          workloadUnit->layerCycles);

    TIDL_updateprofileData(algHandle->alglayerParams[algHandle->currAlgLayer].metaData.profilePoint,
                          TIDL_PROFILE_KERNEL_ONLY,
                          0,
                          workloadUnit->kernelCycles);

    TIDL_updateprofileData(algHandle->alglayerParams[algHandle->currAlgLayer].metaData.profilePoint,
                          TIDL_PROFILE_CORE_LOOP,
                          0,
                          workloadUnit->coreLoopCycles);

    TIDL_updateprofileData(algHandle->alglayerParams[algHandle->currAlgLayer].metaData.profilePoint,
                          TIDL_PROFILE_DMA_PIPEUP,
                          0,
                          workloadUnit->dmaPipeUpCycles);
  }

#endif
  return status;
}

// Need to understand the way trace writing is done. For ST and grouped conv would be different
// with regards to file read/write
/* Expects baseMem pointer */
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t WorkloadRefExec_writeTraceDataBuf.* <function end>
<justification start>
Rationale - DEBUG_TRACE: This function is solely for debugging purposes and is not part of the production code.
Effect on this UNIT - If the control reaches here, code is expected to function correctly for debug/trace purpose. 
However, due to the stated rationale, this is not covered in production code.
<justification end> */
int32_t WorkloadRefExec_writeTraceDataBuf(void *outPtr,
                                          TIDL_Handle algHandle,
                                          sTIDL_Layer_t *tidlLayer,
                                          sTIDL_AlgLayer_t *algLayer,
                                          const sTensorPosition_t *tensorPosition,
                                          const sTensorDims_t *subTensorDims,
                                          const sTensorDims_t *fullTensorDims)
{
  void *temp_outPtr = NULL;
  int32_t status = IALG_EOK;
  int32_t height = tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT];
  int32_t numChannels = tidlLayer->outData.dimValues[TIDL_DIM_NUMCH];
  int32_t numBatches = tidlLayer->outData.dimValues[TIDL_DIM_BATCH];
  int32_t startHeight = 0;
  int32_t startChannel = 0;
  int32_t startBatch = 0;
  int32_t totalBatches = numBatches;
  if (fullTensorDims != NULL)
  {
    totalBatches = fullTensorDims->batch;
  }

  int32_t outOffset = algLayer->wlPadParams.outOffset;       /* Offset from baseMemPtr to C ptr for 0th iteration of workload */
  int32_t superOffset = algLayer->wlPadParams.superOffset;   /* outOffset change per workload */
  int32_t outPadOffset = algLayer->wlPadParams.outPadOffset; /* Offset between A and C pointer */

  if ((subTensorDims != NULL) && (tidlLayer->layerType != TIDL_TransposeLayer))
  {
    height = subTensorDims->height;
    numChannels = subTensorDims->channel;
    numBatches = subTensorDims->batch;
  }

  if (tensorPosition != NULL)
  {
    /* @TODO: Need to handle cases where tiling is done along other dimensions */
    startHeight = (tensorPosition->startHeight + (algLayer->wlRepeatIter * height));
    startChannel = tensorPosition->startChannel;
    startBatch = tensorPosition->startBatch;
  }
  else
  {
    /* will write traces for whole data */
  }

  /* Trace buffer expects 'A' pointer */
  temp_outPtr = (uint8_t *)outPtr +
                (((outOffset + (superOffset * algLayer->wlRepeatIter)) - outPadOffset) * algLayer->wlPadParams.elemSize);

  status = tidl_writeTraceDataBuf((int8_t *)temp_outPtr,
                                  algHandle->createParams->net,
                                  algLayer,
                                  tidlLayer->outData.dataId,
                                  (int8_t *)algHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base,
                                  algHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size,
                                  startHeight,
                                  startChannel,
                                  startBatch,
                                  height,
                                  numChannels,
                                  numBatches,
                                  totalBatches,
                                  tidlLayer->outData.pitch[TIDL_CHANNEL_PITCH],
                                  algHandle->createParams->coreId,
                                  algHandle->createParams->traceBaseName,
                                  0);

  return status;
}
