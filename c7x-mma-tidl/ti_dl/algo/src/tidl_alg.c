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
@file    tidl_alg.c
@brief   This file contains TILD alg inteface implementation
@version 0.1 (Oct 2016) : Initial version [ADK]
@version 0.5 (Jan 2017) : Cleaned up [ADK]
----------------------------------------------------------------------------
*/
#include "tidl_alg_int.h"
#include "tidl_types.h"
#include "tidl_layer.h"
#include "tidl_innerProduct.h"
#include "itidl_ti.h"
#include "perfsim.h"
#include "tidl_priv_algo.h"
#include "tidl_preEmption.h"
#include "tidl_deviceInfo.h"
#include "tidl_device_functions.h"
#include "tidl_device_utils.h"
#include "tidl_alg_utils.h"
#include "tidl_commonUtils.h"
#include "tidl_function_mapping.h"
#include "workload_ref_exec.h"
#include "gc_helper.h"
#include "tidl_forceNegativeTest.h"
#include "tidl_device_mem_properties.h"
#include "tidl_coverage.h"

#define TEMP_FORCE_OUTPUT_TO_DDR (0)
#define VAILD_DDR_BUFFER         (1U)

#define DEFAULT_CORE_LOOP_CYCLES (6000ULL)
#if defined(SOC_J784S4) 
    #define EFFECTIVE_NUM_EMIF_PORTS_J784S4 (3U)
#endif

#if defined(SOC_AM62A)
// Reducing scratch ask to less for smaller devices
#define TIDL_DDR_EXCESS_SCRATCH (2U * 1024U * 1024U)
#else
#define TIDL_DDR_EXCESS_SCRATCH (8U * 1024U * 1024U)
#endif

#define LAYER_TO_RUN NOT_VALID
#define LAYER_TO_RUN_INPUT_0 ""
#define LAYER_TO_RUN_INPUT_1 ""
#define LAYER_TO_RUN_INPUT_2 ""
#define LAYER_TO_RUN_SIZE_0 0
#define LAYER_TO_RUN_SIZE_1 0
#define LAYER_TO_RUN_SIZE_2 0

#ifdef BUILD_WITH_CUDA
#include "tidl_cuda.h"
#include "tidl_cuda_mem_manager.h"
#endif
#include <math.h>
#include <float.h>
#ifdef HOST_EMULATION
#include "time.h"
#endif

#define TIDL_L3_SCRATCH_SIZE (128U * 1024U)
#define TIDL_L3_MSMC_DDR_STAGING_MEM (1025U * 1024U)

#define USE_16BIT_BIAS_FOR_8BIT_MODE (1)

#if (ENABLE_TRACE_PROFILE) && defined(CORE_DSP)
int64_t t0, t1, t2, t3, t4, t5, t6, t7, t8, t9,
    accCycle1, accCycle2, accCycle3, accCycle4, accCycle5, accCycle6;
#endif

#define TIDL_DDR_MEMREC_CACHEABLE ((IALG_MemSpace)IALG_EXTERNAL)
#if TIDL_DEVICE_MULTICORE
#define TIDL_DDR_MEMREC_NON_CACHEABLE ((IALG_MemSpace)IALG_EXTERNAL_NON_CACHEABLE)
#else
#define TIDL_DDR_MEMREC_NON_CACHEABLE ((IALG_MemSpace)IALG_EXTERNAL)
#endif
/**
 * IVISION interface of the algorithm
 */
const int32_t ImplementationID = 0;

const IVISION_Fxns TIDL_VISION_FXNS =
    {
        {(void *)(&ImplementationID),
         &TIDL_activate,
         &TIDL_alloc,
         (int32_t (*)(IALG_Handle handle, IALG_Cmd cmd, IALG_Status *status))NULL,
         &TIDL_deactivate,
         &TIDL_free,
         &TIDL_init,
         (void (*)(IALG_Handle handle, const IALG_MemRec *memRec, IALG_Handle handle1, const IALG_Params *params))NULL,
         &TIDL_numAlloc},
        &TIDL_process,
        &TIDL_control};

void TIDL_play(sTIDL_Network_t *pNet, int32_t mode);

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_numAlloc
@brief      Function to return the number of memory table needed by algorithm
@remarks    None
@return     Number of memory table
----------------------------------------------------------------------------
*/
int32_t TIDL_numAlloc(void)
{
  return (int32_t)MAX_NUM_MEMRECS;
}

static uint32_t TIDL_getMaxParamSizeInBytes(const sTIDL_Layer_t *tidlLayer)
{
  uint32_t dataSize = 0;

  if ((tidlLayer->layerType == TIDL_ConvolutionLayer) ||
      (tidlLayer->layerType == TIDL_Deconv2DLayer) ||
      (tidlLayer->layerType == TIDL_BatchNormLayer) ||
      (tidlLayer->layerType == TIDL_BatchReshapeLayer) ||
      (tidlLayer->layerType == TIDL_InnerProductLayer))
  {
    if ((tidlLayer->layerType == TIDL_ConvolutionLayer) ||
        (tidlLayer->layerType == TIDL_Deconv2DLayer))
    {
      dataSize = (tidlLayer->layerParams.convParams.numOutChannels *
                  tidlLayer->layerParams.convParams.numInChannels *
                  tidlLayer->layerParams.convParams.kernelW *
                  tidlLayer->layerParams.convParams.kernelH) /
                 tidlLayer->layerParams.convParams.numGroups;
    }
  }

  return (dataSize * sizeof(float32_tidl));
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_getMaxOutputBufSizeFromNC
@brief      Function to return the maximum bufSize from all the buffers
@remarks    None
@return     Maximum buffer size from buffer database
----------------------------------------------------------------------------
*/

static int32_t TIDL_getMaxOutputBufSizeFromNC(const sGCHelperHandle *gcHelperHandle)
{
  int32_t maxSize = 0;
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
  TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_001
  Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
  This does not affect runtime behavior or safety.
  <justification end> */
  if (gcHelperHandle != NULL)
  {
    /* LDRA_JUSTIFY_END */
    int32_t bufCount = gcHelperHandle->pBufDB->count;
    int32_t i1 = 0;

    for (i1 = 0; i1 < bufCount; i1++)
    {
      const sBufParams_t *bufParams = getBufParamsFromBufIndex(gcHelperHandle, i1);
      if (bufParams->type == (int32_t)BUF_FM_FULL)
      {
        maxSize = (maxSize < bufParams->bufSize) ? bufParams->bufSize : maxSize;
      }
    }
  }

  return maxSize;
}

static int32_t TIDL_getTotalInputBufferSizeFromWl(const sGCHelperHandle *gcHelperHandle, const sWorkloadUnit_t *pWL)
{
  const sLink_t *pLink = NULL;
  sLink_t *linkPtrList[MAX_LINKS_PER_WL];
  int32_t totalInputBufferSizeFromLink = 0;

  for (int32_t linkNum = 0; (pWL != NULL) && (linkNum < pWL->numLinks); linkNum++)
  {
    getLinkPtrs(pWL, NOT_VALID, linkNum, linkPtrList);
    pLink = linkPtrList[0];
    if (linkNum == 0)
    {
      // This is to make sure that at least once we can call getLinkPtrs with valid subType
      //  to validate the coverage of function: For safety use case
      getLinkPtrs(pWL, pLink->subType, linkNum, linkPtrList);
    }
    pLink = linkPtrList[0];
    for (int32_t srcNum = 0; (pLink != NULL) && (srcNum < pLink->numSrc); srcNum++)
    {
      const sBufParams_t *bufParams = getBufParamsFromBufIndex(gcHelperHandle, pLink->src[srcNum].bufDBindex);
      if (bufParams->type == (int32_t)BUF_FM_FULL)
      {
        totalInputBufferSizeFromLink += bufParams->bufSize;
      }
    }
  }
  return totalInputBufferSizeFromLink;
}

static void TIDL_printMemorySizeStats(const IALG_MemRec memRec[],
                                      int32_t numMemRec, int32_t alloc)
{
  int32_t i, j, totalSize[(uint32_t)IALG_EXTERNAL_NON_CACHEABLE + 1U][IALG_PERSIST + 1U] = {0};
  const char *spaceMapping[(uint32_t)IALG_EXTERNAL_NON_CACHEABLE+1U] = {0};
  const char *attrMapping[(uint32_t)IALG_WRITEONCE + 1U] = {0};
  void *ptr;
  spaceMapping[IALG_DARAM0] = "L1D";
  spaceMapping[IALG_DARAM1] = "L2";
  spaceMapping[IALG_SARAM0] = "L3/MSMC";
  spaceMapping[IALG_EXTERNAL] = "DDR Cacheable";
  spaceMapping[IALG_EXTERNAL_NON_CACHEABLE] = "DDR Non-cacheable";

  attrMapping[IALG_SCRATCH] = "Scratch";
  attrMapping[IALG_PERSIST] = "Persistent";

  tidl_printf(1, "\n--------------------------------------------\n");

  tidl_printf(1, "TIDL Memory size requiement (record wise):\n");
  tidl_printf(1, "%-12s, %-20s, %-12s, %-12s, %-12s, %-12s\n", "MemRecNum", "Space", "Attribute", "Alignment", "Size(KBytes)", "BasePtr");

  for (i = 0; i < numMemRec; i++)
  {
    if (alloc != 0)
    {
      ptr = NULL;
    }
    else
    {
      ptr = memRec[i].base;
    }
    tidl_printf(1, "%-12d, %-20s, %-12s, %4d, %-8.2f, 0x%08x\n",i,
      spaceMapping[memRec[i].space], attrMapping[memRec[i].attrs], memRec[i].alignment, (float32_tidl)memRec[i].size/1024.0, ptr);
      totalSize[memRec[i].space][memRec[i].attrs] += (int32_t)memRec[i].size ;
  }
  tidl_printf(1, "--------------------------------------------\n");
  tidl_printf(1, "Total memory size requirement (space wise):\n");
  tidl_printf(1, "%-10s, %-10s\n", "Mem Space", "Size(KBytes)");
  for (i = 0; i <= (int32_t)TIDL_DDR_MEMREC_NON_CACHEABLE; i++)
  {
    for (j = 0; j <= IALG_PERSIST; j++)
    {
      if (totalSize[i][j] != 0)
      {
        tidl_printf(1,"%s(%s), %-8.2f\n", spaceMapping[i], attrMapping[j],  (float32_tidl)totalSize[i][j]/1024.0);
      }
    }
  }
  tidl_printf(1, "--------------------------------------------\n");
  tidl_printf(1, "NOTE: Memory requirement in host emulation can be different from the same on EVM\n");
  tidl_printf(1, "      To get the actual TIDL memory requirement make sure to run on EVM with \n");
  tidl_printf(1, "      debugTraceLevel = 2\n\n");
  tidl_printf(1, "--------------------------------------------\n");
  return;
}

/*this function sets the offset in weight buffers for Convolution Channel split*/
static void TIDL_setMultiCoreCoeffOffset(sTIDL_Layer_t *tidlLayer, sTIDL_AlgLayer_t *algLayer, int32_t startCoreIdx, int32_t curCoreIdx, int32_t numCores)
{

  algLayer->layerParams.convParams.coeffMultiCoreExtraOffset = 0U;
  algLayer->layerParams.convParams.biasMultiCoreExtraOffset = 0U;

#if TIDL_DEVICE_MULTICORE
  
  int32_t multiCoreMode  = (algLayer->workloadUnit != NULL)
                            ? algLayer->workloadUnit->multiCoreMode
                            : TIDL_NOT_MULTI_CORE;
          
  if ((tidlLayer->layerType == TIDL_ConvolutionLayer) && ((multiCoreMode & TIDL_MULTI_CORE_CHANNEL) == TIDL_MULTI_CORE_CHANNEL) && (curCoreIdx > startCoreIdx))
  {
    int32_t numInChannelsSplit = 0, numOutChannelsSplit = 0, numGroupSplit = 0;
    int32_t weightSize = TIDL_ALIGN_CEIL(tidlLayer->weightsElementSizeInBits, 8U) / 8U;

    sTIDL_ConvParams_t *convParams = &(tidlLayer->layerParams.convParams);
    for (int32_t i = startCoreIdx; i < curCoreIdx; i++)
    {
      if ((convParams->numGroups == 1) && ((convParams->numInChannels / convParams->numGroups) != 1)) /*Row-Flow non-grouped uses all input channels so we do not accumulate*/
      {
        numInChannelsSplit = getNumInChforCore(GET_RELATIVE_COREIDX(i, startCoreIdx), numCores, convParams->numInChannels, convParams->numGroups, tidlLayer->layerType);
        numGroupSplit = getDimSplitForCore(GET_RELATIVE_COREIDX(i, startCoreIdx), numCores, convParams->numGroups);
      }
      else
      {
        numInChannelsSplit += getNumInChforCore(GET_RELATIVE_COREIDX(i, startCoreIdx), numCores, convParams->numInChannels, convParams->numGroups, tidlLayer->layerType);
        numGroupSplit += getDimSplitForCore(GET_RELATIVE_COREIDX(i, startCoreIdx), numCores, convParams->numGroups);
      }

      numOutChannelsSplit += getNumOutChforCore(GET_RELATIVE_COREIDX(i, startCoreIdx), numCores, convParams->numOutChannels, convParams->numGroups, tidlLayer->layerType);
    }

    algLayer->layerParams.convParams.coeffMultiCoreExtraOffset = numInChannelsSplit * (numOutChannelsSplit / numGroupSplit) * convParams->kernelH * convParams->kernelW * weightSize;
    algLayer->layerParams.convParams.biasMultiCoreExtraOffset = numGroupSplit * (numOutChannelsSplit / numGroupSplit);
  }
#endif
}

int32_t getLayerExecutionNumber(int32_t dataId, int32_t layerExecutionOrder[], int32_t numLayers)
{
  int32_t execLayerNum = NOT_VALID;

  for (int32_t i = 0; i < numLayers; i++)
  {
    if (dataId == layerExecutionOrder[i])
    {
      execLayerNum = i;
      break;
    }
  }
  return execLayerNum;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_alloc
@brief      Function to supply memory requirement to user
            refer algAlloc function for details in XADIS (ialg.h file)
@param      params       : Input Parameters
@param      parentFxns   : Parent function pointer
@param      memRec       : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

static sTIDL_LayerBuf_t TIDLLayersBuf = {0};
int32_t TIDL_alloc(const IALG_Params *params,
                   IALG_Fxns **parentFxns,
                   IALG_MemRec memRec[])
{
  int32_t status = IALG_EOK;
  int32_t l1MemSize, l2MemSize, l3MemSize;
  int32_t i;
  const TIDL_CreateParams *createParams =
      (const TIDL_CreateParams *)(const void *)params;
  uint32_t coreId = createParams->coreId;
  int32_t relativeCoreId = GET_RELATIVE_COREIDX(coreId, createParams->coreStartIdx);
  sPerfSim_t *perfInfoOut = NULL;
  sGraphCompilerOutArgs_t *gcOutArgs = NULL;
  sGraphCompilerOutArgsCore_t *gcOut = NULL;
  sWorkloadUnit_t *workloadUnit = NULL;
  sGCHelperHandle *gcHelperHandle = NULL;
  sGCHelperHandle gcHelperHandleObj;
  TIDL_LayerSpecificParams layerSpecificParams;
  TIDL_NetworkCommonParams commonParams;
  uint32_t memorySize[TIDL_LAYER_MEMORY_MAX] = {0};
  sWorkloadSuperGroup_t *wlSuperGrp = NULL;
  int32_t flag = 1;

  /* TIDL_coverage_start is a ldra coverage tag to mark the start of coverage for TIDL */
  TIDL_coverage_start();

  /* TIDL_odOutputReformat_dummyFunc is a dummy function to avoid unused function warning */
  TIDL_odOutputReformat_dummyFunc();

  status = TIDL_initDebugTraceParams(createParams->traceLogLevel, createParams->traceWriteLevel, createParams->TIDLVprintf, createParams->TIDLWriteBinToFile,
                                     createParams->TIDLReadBinFromFile, createParams->traceBaseName);
  tidl_printf(2, "TIDL_initDebugTraceParams Done \n");
  /* TIDL_LDRA_TAG_ALG_PRIOR_CHECK_002 */
  if (status != IALG_EOK)
  {
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_DEBUG_TRACE_INVALID_PARAM);
    status = IALG_EFAIL;
  }

  if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALLOC_FORCE_NETVERSION)
  {
    createParams->net->netVersion = 0x20220809;
  }

  if (status == IALG_EOK)
  {
    if ((createParams->net->netVersion != TIDL_NET_VERSION_FW_ACTIVE))
    {
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_COMMON_INVALID_NET_VERSION);
      status = IALG_EFAIL;
    }
  }

  if (status == IALG_EOK)
  {
#ifdef HOST_EMULATION
    if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == 0U)
    {
      if (createParams->net->isQuantStatsAvailable == 0)
      {
        TIDL_LOG_ERROR(TIDL_ERROR_GROUP_QUANTIZATION, TIDL_ERROR_QUANTIZATION_STATS_NOT_AVAILABALE);
        status = IALG_EFAIL;
      }
    }

    if (status == IALG_EOK)
#endif
    {
      if (createParams->isInbufsPaded == 0)
      {
        /* tidl_printf(1,"Padding of input buffer in Library is not supported now\n"); */
        TIDL_LOG_ERROR(TIDL_ERROR_GROUP_PADDING, TIDL_ERROR_PADDING_INPUT_BUF_NOT_SUPPORTED);
        status = IALG_EFAIL;
      }
    }

    if (status == IALG_EOK)
    {
#ifdef HOST_EMULATION
      if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_COMP) == TIDL_FLOW_CTRL_REF_COMP)
      {
        tidl_printf(0, "WARNING : TIDL_FLOW_CTRL_REF_COMP Support is deprecated in Inference Software now \n");
      }
#endif
      if (createParams->net->dataFlowInfo != 0)
      {
        perfInfoOut = (sPerfSim_t *)get_int8_t_pointer((int8_t *)(createParams->net), createParams->net->dataFlowInfo);
      }

      TIDL_play(createParams->net, 1);
      /*TIDL_LDRA_TAG_ALG_PRIOR_CHECK_006*/
      if (createParams->net->isGCInfoAvailable != 0)
      {
        gcOutArgs = (sGraphCompilerOutArgs_t *)get_int8_t_pointer((int8_t *)(createParams->net), createParams->net->originalGcBufInfoOffset);
        gcOut = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs, (int32_t)DATABASE_GCOUT_1 + relativeCoreId);
        sGCDataBase_t *gcDatbase = &gcOut->dataBase;
        sGCCommonDataBase_t *gcCommonDataBase = (sGCCommonDataBase_t *)getGCDataBase(gcOutArgs, DATABASE_COMMON);

        sBufParams_t *bufNodesDB = (sBufParams_t *)&gcCommonDataBase->dbPayLoad[gcCommonDataBase->dbParams[DB_COMMON_BUF].startOffset];
        sWorkloadUnit_t *workLoadUnitDB = (sWorkloadUnit_t *)&gcDatbase->dbPayLoad[gcDatbase->dbParams[DB_WL].startOffset];
        // sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) gcDatbase->dbPayLoad[gcDatbase->dbParams[DB_BUF].size +
        //                                                                       gcDatbase->dbParams[DB_WL].size];
        sWorkloadUnitAuxilary_t *auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t *)&gcDatbase->dbPayLoad[gcDatbase->dbParams[DB_AUXWL].startOffset];

        gcHelperHandle = &gcHelperHandleObj;
        gcOutArgs->commonDatabaseInfo.bufDB.pBufList = bufNodesDB;
        gcOut->dataBaseInfo.wlDB.pWLList = workLoadUnitDB;
        gcOut->dataBaseInfo.wlAuxDB.pWLAuxList = auxilaryWorkloadDB;
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALLOC_FORCE_NET_NULL)
        {
          gcHelperHandle = NULL;
        }
        status = initGCHelperHandle(gcHelperHandle, createParams->net, &gcOut->dataBaseInfo, &gcOutArgs->commonDatabaseInfo);
        if (status == IALG_EOK)
        {
          wlSuperGrp = &gcOut->superWorkload;
          if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_WLSUPERGRP_NULL)
          {
            wlSuperGrp = NULL;
          }
          if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_WLUNIT_NULL)
          {
            gcHelperHandle = NULL;
          }
        }
      }
      if (status == IALG_EOK)
      {
#ifdef HOST_EMULATION
        if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
        {
          perfInfoOut = NULL;
          gcHelperHandle = NULL;
        }
#endif
        if (NULL == perfInfoOut)
        {
          l1MemSize = TIDL_DEFAULT_L1_MEM_SIZE;
          l2MemSize = TIDL_DEFAULT_L2_MEM_SIZE;
          l3MemSize = TIDL_DEFAULT_L3_MEM_SIZE;
        }
        else
        {
          l1MemSize = TIDL_DEFAULT_L1_MEM_SIZE;
          l2MemSize = (perfInfoOut->simConfig.sizeL2MemKB * 1024);
          l3MemSize = perfInfoOut->simConfig.sizeL3MemKB * 1024;
        }

        int32_t maxInFeatMapSize = 0;

        memRec[ALG_HANDLE_MEMREC].size = (uint32_t)sizeof(TIDL_Obj);
        memRec[ALG_HANDLE_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
        memRec[ALG_HANDLE_MEMREC].attrs = IALG_PERSIST;
        memRec[ALG_HANDLE_MEMREC].alignment = 128;

        memRec[ALG_SCRATCH_L1_MEM_MEMREC].size = (uint32_t)l1MemSize;
        memRec[ALG_SCRATCH_L1_MEM_MEMREC].attrs = IALG_SCRATCH;
        memRec[ALG_SCRATCH_L1_MEM_MEMREC].alignment = 128;
        #if defined(SOC_TDA54) && !defined(HOST_EMULATION) // No L1 cachable mem in VDK
        //TODO: remove when silicon is available
        tidl_printf(0, "TIDL_alloc : l1MemSize = %dKB, l2MemSize = %dKB, l3MemSize = %dKB\n", l1MemSize/1024, l2MemSize/1024, l3MemSize/1024);
        tidl_printf(0, "TIDL_alloc : l1MemSize = %dKB != 0 but no L1 memory support for TDA54 VDK, allocating in L2 memory\n", l1MemSize/1024);
        memRec[ALG_SCRATCH_L1_MEM_MEMREC].space = (IALG_MemSpace)IALG_DARAM1;
        #else
        memRec[ALG_SCRATCH_L1_MEM_MEMREC].space = (IALG_MemSpace)IALG_DARAM0;
        #endif

        memRec[ALG_SCRATCH_L2_MEM_MEMREC].size = (uint32_t)l2MemSize;
        memRec[ALG_SCRATCH_L2_MEM_MEMREC].attrs = IALG_SCRATCH;
        memRec[ALG_SCRATCH_L2_MEM_MEMREC].alignment = 128;
        memRec[ALG_SCRATCH_L2_MEM_MEMREC].space = IALG_DARAM1;

        memRec[ALG_SCRATCH_L3_MEM_MEMREC].size = (uint32_t)l3MemSize;
        memRec[ALG_SCRATCH_L3_MEM_MEMREC].space = (IALG_MemSpace)IALG_SARAM0;
        memRec[ALG_SCRATCH_L3_MEM_MEMREC].attrs = IALG_SCRATCH;
        memRec[ALG_SCRATCH_L3_MEM_MEMREC].alignment = 128;

        memRec[ALG_CREATE_PARAM_MEMREC].size = (uint32_t)sizeof(TIDL_CreateParams);
        memRec[ALG_CREATE_PARAM_MEMREC].space = TIDL_DDR_MEMREC_CACHEABLE;
        memRec[ALG_CREATE_PARAM_MEMREC].attrs = IALG_PERSIST;
        memRec[ALG_CREATE_PARAM_MEMREC].alignment = 128;

        memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].size = 256 * 1024;                 // Minimum 256 KB to allow some last unaligned transfers
        memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].space = TIDL_DDR_MEMREC_CACHEABLE; /* Supposed to be read only during process call, so mark cacheable */
        memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].attrs = IALG_PERSIST;
        memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].alignment = 128;

        memRec[ALG_LAYERS_MEMREC].size = 0;
        memRec[ALG_LAYERS_MEMREC].space = TIDL_DDR_MEMREC_CACHEABLE;
        memRec[ALG_LAYERS_MEMREC].attrs = IALG_PERSIST;
        memRec[ALG_LAYERS_MEMREC].alignment = 128;

        memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size = 256;
        memRec[ALG_REF_SCRATCH_BUFF_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
        memRec[ALG_REF_SCRATCH_BUFF_MEMREC].attrs = IALG_SCRATCH;
        memRec[ALG_REF_SCRATCH_BUFF_MEMREC].alignment = 128;

        memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].size = 256;
        memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
        memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].attrs = IALG_SCRATCH;
        memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].alignment = 128;

        memRec[ALG_SCRATCH_DDR_MEM_MEMREC].size = 256U + TIDL_DDR_EXCESS_SCRATCH; // Alloc at allocResize
        memRec[ALG_SCRATCH_DDR_MEM_MEMREC].space = TIDL_DDR_MEMREC_CACHEABLE;
        memRec[ALG_SCRATCH_DDR_MEM_MEMREC].attrs = IALG_SCRATCH;
        memRec[ALG_SCRATCH_DDR_MEM_MEMREC].alignment = 128;

        memRec[ALG_PERSIST_DDR_INIT_DATA].size = 128;
        memRec[ALG_PERSIST_DDR_INIT_DATA].space = TIDL_DDR_MEMREC_CACHEABLE;
        memRec[ALG_PERSIST_DDR_INIT_DATA].attrs = IALG_PERSIST;
        memRec[ALG_PERSIST_DDR_INIT_DATA].alignment = 128;

        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_ALLOC_FORCE_ALGLAYERSSIZE)
        {
          memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].size = INT32_MAX;
        }
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_ALLOC_FORCE_SCRATCHBUFFSIZE)
        {
          memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].size = INT32_MAX;
        }
        /* each Workload maintains a state value - Core 0 has max. number of workloads, allocate same memory for all cores */
#if TIDL_DEVICE_MULTICORE
        if (createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
        {
          sGraphCompilerOutArgsCore_t *gcOutCore = NULL;
          if (gcOutArgs != NULL)
          {
            gcOutCore = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs, DATABASE_GCOUT_1);
          }
          if (gcOutCore != NULL)
          {
            memRec[ALG_PERSIST_SYNC_MEMREC].size = (gcOutCore->dataBaseInfo.wlDB.count * sizeof(int8_t)) + (int8_t)TIDL_GLOBAL_SYNC_OFFSET;
          }
          memRec[ALG_PERSIST_SYNC_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
          memRec[ALG_PERSIST_SYNC_MEMREC].attrs = IALG_PERSIST;
          memRec[ALG_PERSIST_SYNC_MEMREC].alignment = 128;
        }
        else
        {
#endif
          /* Synchronization memory is required only for low latency mode. size = 0 cannot be requested, so setting size = 1*/
          memRec[ALG_PERSIST_SYNC_MEMREC].size = 1;
          memRec[ALG_PERSIST_SYNC_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
          memRec[ALG_PERSIST_SYNC_MEMREC].attrs = IALG_PERSIST;
          memRec[ALG_PERSIST_SYNC_MEMREC].alignment = 128;
#if TIDL_DEVICE_MULTICORE
        }
#endif

        /* ALG_SCRATCH_DATA_BUFF_MEMREC Will be filled
        as max of all the temp buffer requirement by all the layers */

        /* Allocate Memory to store newdataID's in TIDL_Handle to use in Process call */
        memRec[ALG_LAYERS_MEMREC].size +=
            (uint32_t)(sizeof(sTIDL_LayerBuf_t) + 128U);

        /* Initialize newDataId of inData with dataId, so that this can be used later
            to find the free output buffer for Re-using
        */
        if (createParams->optimiseExtMem != TIDL_OptimiseExtMemL0)
        {
          for (i = 0; i < createParams->net->numLayers; i++)
          {
#ifdef HOST_EMULATION
            int32_t j;
            const sTIDL_Layer_t *TIDLLayerCur = &createParams->net->TIDLLayers[i];
            for (j = 0; j < TIDLLayerCur->numInBufs; j++)
            {
              TIDLLayersBuf.newInDataId[i][j] = (int16_t)TIDLLayerCur->inData[j];
            }
            for (j = 0; j < TIDLLayerCur->numOutBufs; j++)
            {
              TIDLLayersBuf.newOutDataId[i][j] = 0;
            }
#endif
            TIDLLayersBuf.outDataSize[i] = 0;
          }
        }

        commonParams.createParams = createParams;
        commonParams.gcHelperHandle = gcHelperHandle;
        commonParams.net = createParams->net;
        commonParams.TIDLLayersBufPtr = &TIDLLayersBuf;
        commonParams.tidlCommonParams = NULL; /* Not required during alloc */

        sGetLayerIdContext_t getLayerIdContext;
        int32_t layerId;
#ifdef HOST_EMULATION
        /*
         * Tracks whether any layer in this network is forced to run on the
         * reference flow (e.g: InnerProduct)
         */
        uint8_t hasForceRefLayer = 0U;
#endif

        getLayerIdInit(&getLayerIdContext,
                       relativeCoreId,
                       createParams->currLayersGroupId,
                       createParams->net,
                       gcHelperHandle,
                       wlSuperGrp);
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_ALLOC_FORCE_LOOP_FLAG_CHECK)
        {
          flag = 0;
          status = IALG_EFAIL;
        }
        while (flag == 1)
        {
          layerId = getLayerIdToExecute(&getLayerIdContext,
                                        1,
                                        NULL,
                                        NULL,
                                        &workloadUnit);
          if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_GC_HELPER_FORCE_WL_UNIT_NULL)
          {
            /* Force workloadUnit to be NULL to check null pointer dereference */
            layerId = getLayerIdToExecute(&getLayerIdContext,
                                          1,
                                          NULL,
                                          NULL,
                                          NULL);
            if (layerId == (int32_t)NOT_VALID)
            {
              status = IALG_EFAIL;
            }
          }
          if (layerId == (int32_t)NOT_VALID)
          {
            break;
          }

          //: TODO: Eventually this should be removed once process also  uses the same loop structure
          if (createParams->net->TIDLLayers[layerId].layerType == TIDL_DataLayer)
          {
            continue;
          }

          if (gcOutArgs != NULL)
          {
            int32_t isWorkLoadSupported;
            isWorkLoadSupported = TIDL_isWorkLoadUnitFlowSupported(layerId,
                                                                   createParams->net,
                                                                   createParams->net->deviceName,
                                                                   workloadUnit);
            /* Force workloadUnit to be NULL to going to old flow */
            if (isWorkLoadSupported == 0)
            {
              workloadUnit = NULL;
            }
          }
          layerSpecificParams.workloadUnit = workloadUnit;
          layerSpecificParams.isMixedPrecEnabled = 1U;

          (void)memset(memorySize, 0, sizeof(memorySize));
          
          if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_ALLOC_FORCE_LAYERTYPE_INVALID)
          {
            int32_t *playerType = &(commonParams.createParams->net->TIDLLayers[layerId].layerType);
            *playerType = TIDL_UnsupportedLayer;
          }
          
          status = TIDL_layerAllocNew(&layerSpecificParams,
                                      &commonParams,
                                      layerId,
                                      (int32_t *)memorySize);

          if (status != IALG_EOK)
          {
            break;
          }
          /*TIDL_LDRA_TAG_ALG_PRIOR_CHECK_007*/
          if ((workloadUnit != NULL) && (commonParams.gcHelperHandle != NULL))
          {
            int32_t maxBufSizeFromNC = TIDL_getTotalInputBufferSizeFromWl(commonParams.gcHelperHandle, workloadUnit);
            maxInFeatMapSize = (maxBufSizeFromNC > maxInFeatMapSize) ? maxBufSizeFromNC : maxInFeatMapSize;
          }

          memRec[ALG_LAYERS_MEMREC].size += (uint32_t)(sizeof(sTIDL_AlgLayer_t) + 128U);

          /* Memory used for output buffers, needed only for reference flow without NC */
          if (((uint64_t)UINT32_MAX - (memorySize[TIDL_LAYER_MEMORY_OUTPUT] + 128U)) < memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].size)
          {
            status = TIDL_ERR_FAILURE;
            tidl_printf(0, "TIDL_alloc : Error | Intermediate output memory required to execute this models exceeds available scratch memory\n");
            break;
          }
          memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].size += (memorySize[TIDL_LAYER_MEMORY_OUTPUT] + 128U);

          /* Scratch memory used by layer, needed only for reference flow without NC */
          if (((uint64_t)UINT32_MAX - (memorySize[TIDL_LAYER_MEMORY_PERSISTENT] + 128U)) < memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].size)
          {
            status = TIDL_ERR_FAILURE;
            tidl_printf(0, "TIDL_alloc : Error | Parameter memory required to execute this models exceeds available layer memory\n");
            break;
          }
          memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].size += (memorySize[TIDL_LAYER_MEMORY_PERSISTENT] + 128U);

          if (TIDL_ALIGN_CEIL((int32_t)memorySize[TIDL_LAYER_MEMORY_SCRATCH], 128) > (int32_t)memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size)
          {
            /* Scratch buffer is common across all the layers hence only request the maximum scratch size */
            memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size = TIDL_ALIGN_CEIL((int32_t)memorySize[TIDL_LAYER_MEMORY_SCRATCH], 128);
          }

#ifdef HOST_EMULATION
          /*
           * Even in non-ref mode, InnerProduct may be forced to run on the 
           * reference flow. Scratch buffer must be preserved for such layers.
           */
          if ((createParams->net->TIDLLayers[layerId].layerType == TIDL_InnerProductLayer) &&
              (TIDL_forceInnerProductRef(createParams, &createParams->net->TIDLLayers[layerId]) == 1))
          {
            hasForceRefLayer = 1U;
          }
#endif
        }

        if (status == IALG_EOK)
        {

          memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = 128;
          memRec[ALG_REF_OUTPUT_BUFF_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
          memRec[ALG_REF_OUTPUT_BUFF_MEMREC].attrs = IALG_SCRATCH;
          memRec[ALG_REF_OUTPUT_BUFF_MEMREC].alignment = 128;

          int32_t maxOutFeatMapSize = 0;
          int32_t outFeatMapSize;
          int32_t outFeatMapSize1;
          uint32_t maxParamSize = 0;

          for (i = 0; i < createParams->net->numLayers; i++)
          {
            uint32_t paramSize;
            sTIDL_DataParams_t *dataParams = TIDL_getDataParams(createParams->net,
                                                                createParams->net->TIDLLayers[i].outData.dataId);
            outFeatMapSize = dataParams->dimValues[TIDL_DIM_BATCH] *
                             dataParams->dimValues[TIDL_DIM_DIM1] *
                             dataParams->dimValues[TIDL_DIM_DIM2] *
                             dataParams->dimValues[TIDL_DIM_NUMCH] *
                             dataParams->pitch[TIDL_CHANNEL_PITCH] *
                             (TIDL_getDatElementSize(dataParams->elementType));

            outFeatMapSize1 = dataParams->dimValues[TIDL_DIM_BATCH] *
                              dataParams->dimValues[TIDL_DIM_DIM1] *
                              dataParams->dimValues[TIDL_DIM_DIM2] *
                              dataParams->dimValues[TIDL_DIM_NUMCH] *
                              dataParams->dimValues[TIDL_DIM_HEIGHT] *
                              dataParams->dimValues[TIDL_DIM_WIDTH] *
                              (TIDL_getDatElementSize(dataParams->elementType));

            outFeatMapSize = (outFeatMapSize > outFeatMapSize1) ? outFeatMapSize : outFeatMapSize1;

            if (outFeatMapSize > maxOutFeatMapSize)
            {
              maxOutFeatMapSize = outFeatMapSize;
            }

            paramSize = TIDL_getMaxParamSizeInBytes(&createParams->net->TIDLLayers[i]);

            maxParamSize = (paramSize > maxParamSize) ? paramSize : maxParamSize;
          }
          
          #ifdef HOST_EMULATION
          int32_t inFeatMapSizeSum = 0;
          for (i = 0; i < createParams->net->numLayers; i++)
          {
            inFeatMapSizeSum = 0;
            for (int32_t inDataIdx = 0; inDataIdx < createParams->net->TIDLLayers[i].numInBufs ; inDataIdx++)
            {
              sTIDL_DataParams_t *dataParams = TIDL_getDataParams(createParams->net, createParams->net->TIDLLayers[i].inData[inDataIdx]);
              int32_t inFeatMapSize = dataParams->dimValues[TIDL_DIM_BATCH] *
                             dataParams->dimValues[TIDL_DIM_DIM1] *
                             dataParams->dimValues[TIDL_DIM_DIM2] *
                             dataParams->dimValues[TIDL_DIM_NUMCH] *
                             dataParams->pitch[TIDL_CHANNEL_PITCH] *
                             (TIDL_getDatElementSize(dataParams->elementType));

              int32_t inFeatMapSize1 = dataParams->dimValues[TIDL_DIM_BATCH] *
                              dataParams->dimValues[TIDL_DIM_DIM1] *
                              dataParams->dimValues[TIDL_DIM_DIM2] *
                              dataParams->dimValues[TIDL_DIM_NUMCH] *
                              dataParams->dimValues[TIDL_DIM_HEIGHT] *
                              dataParams->dimValues[TIDL_DIM_WIDTH] *
                              (TIDL_getDatElementSize(dataParams->elementType));

              inFeatMapSize = (inFeatMapSize > inFeatMapSize1) ? inFeatMapSize : inFeatMapSize1;
              inFeatMapSizeSum += inFeatMapSize;
            }
            /*
            * MaxOutFeatMapSize is being used to derive the buffer requirement for temporary buffer, 
            * where the input are copied in REF flow. So need to consider sum of all the inputs.
            * Existing code uses the variable maxOutFeatMapSize, so assigning to it. 
            * Though the variable is not indicating its purpose.
            */
            if (inFeatMapSizeSum > maxOutFeatMapSize)
            {
              maxOutFeatMapSize = inFeatMapSizeSum;
            }
          }
          #endif

          /* In 1D data, the scratchBufSize can become less then required for per channel mean when the data is along the channel dimension,
            finding the minimum memory required(taking all channel dimensions): */
          int32_t totalChannels = 0;
          for (i = 0; i < createParams->net->numLayers; i++)
          {
            sTIDL_DataParams_t *dataParams = TIDL_getDataParams(createParams->net, createParams->net->TIDLLayers[i].outData.dataId);
            totalChannels += dataParams->dimValues[TIDL_DIM_NUMCH];
          }
          int32_t totalChannelSize = totalChannels * (int32_t)sizeof(float32_tidl);
          maxOutFeatMapSize = (totalChannelSize > maxOutFeatMapSize) ? totalChannelSize : maxOutFeatMapSize;

          if (commonParams.gcHelperHandle != NULL)
          {
            /* TIDL_LDRA_TAG_ALG_PRIOR_CHECK_001 */
            /**
             * Network compiler sometimes allocates higher buffer size than the actual output size specific to kernel
             * requirements, allocating higher scratch buffer here based on the maximum buffer size given by NC
             */
            int32_t maxBufSizeFromNC = TIDL_getMaxOutputBufSizeFromNC(commonParams.gcHelperHandle);

            maxOutFeatMapSize = (maxBufSizeFromNC > maxOutFeatMapSize) ? maxBufSizeFromNC : maxOutFeatMapSize;
          }
          // if(status == IALG_EOK) /* commented this check to avoid MISRA-C violations, status is not changed from above check(901-line) to this*/
          {
            maxOutFeatMapSize = (maxInFeatMapSize > maxOutFeatMapSize) ? maxInFeatMapSize : maxOutFeatMapSize;
#ifdef HOST_EMULATION
            if (!(((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY))
#endif
            {

            #ifdef HOST_EMULATION
              // Skip the ref scratch buffer reset when any layer is forced to ref
              if (hasForceRefLayer == 0U)
              {
                memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size = 128U;
              }
            #else
                memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size = 128U;
            #endif

              if (createParams->traceWriteLevel == 3)
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (uint32_t)((sizeof(float32_tidl) + 1U) * (uint32_t)maxOutFeatMapSize) + TRACE_STRINGS_MEM_SIZE;
              }
              else if (createParams->traceWriteLevel == 2)
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (2U * (uint32_t)maxOutFeatMapSize) + TRACE_STRINGS_MEM_SIZE;
              }
              else if (createParams->traceWriteLevel == 1)
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (uint32_t)maxOutFeatMapSize + TRACE_STRINGS_MEM_SIZE;
              }
              else
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = 128U + TRACE_STRINGS_MEM_SIZE;
              }
            }
#ifdef HOST_EMULATION
            else
            {
              /*maxOutFeatMapSize is in bytes*/
              memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (2U * (uint32_t)maxOutFeatMapSize) + TRACE_STRINGS_MEM_SIZE; /* Twice for input and output */
              /*Allocating extra space for traces, ideally there should not be a reference/target specific condition for this:*/
              if (createParams->traceWriteLevel == 3)
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (uint32_t)((sizeof(float32_tidl) + 1U) * (uint32_t)maxOutFeatMapSize) + TRACE_STRINGS_MEM_SIZE;
              }
              else if (createParams->traceWriteLevel == 2)
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (2U * (uint32_t)maxOutFeatMapSize) + TRACE_STRINGS_MEM_SIZE;
              }
              else if (createParams->traceWriteLevel == 1)
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (uint32_t)maxOutFeatMapSize + TRACE_STRINGS_MEM_SIZE;
              }
              else
              {
                /*do nothing*/
              }
            }

            if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
            {
              /*Disabling histogram based clipping for Asymmetric, per-channel mode*/
              if ((((uint32_t)createParams->net->calibrationOption & (uint32_t)TIDL_CalibOptionActivationRange) ==
                   (uint32_t)TIDL_CalibOptionActivationRange) &&
                  (createParams->net->quantizationStyle != TIDL_QuantStyleAsymNP2))
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += 128U;
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += ((uint32_t)createParams->net->numLayers *
                                                            TIDL_NUM_ACTIVATION_HISTOGRAM_BINS *
                                                            sizeof(int32_t));
                /* To store the min and max value of each layer */
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += 128U;
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += ((uint32_t)createParams->net->numLayers *
                                                            2U * sizeof(float32_tidl));
              }
            }

            if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
            {
              if ((((uint32_t)createParams->net->calibrationOption & (uint32_t)TIDL_CalibOptionActivationRange) ==
                   (uint32_t)TIDL_CalibOptionActivationRange) &&
                  (createParams->net->quantizationStyle != TIDL_QuantStyleAsymNP2))
              {
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += 128U;
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += ((uint32_t)createParams->net->numLayers *
                                                            TIDL_NUM_ACTIVATION_HISTOGRAM_BINS *
                                                            sizeof(int32_t));
                /* To store the min and max value of each layer */
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += 128U;
                memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size += ((uint32_t)createParams->net->numLayers *
                                                            2U * sizeof(float32_tidl));
              }
            }
#endif

            if (createParams->traceWriteLevel == 4)
            {
              maxParamSize = maxParamSize + TRACE_STRINGS_MEM_SIZE;
              memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size = (maxParamSize >
                                                         memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size)
                                                            ? maxParamSize
                                                            : memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size;
            }

            memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
            memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].attrs = IALG_SCRATCH;
            memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].alignment = 128;
            memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].size = 128;
#if ENABLE_PREEMPTION
/* memTab to allocate memory to backup context memory for pre-emption */
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            The condition check is deterministic and does not impact the safety or reliability of the system.
            Therefore, it is excluded from safety coverage requirements.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if (TIDL_checkIfPreEmptionEnabled(createParams) != 0)
            /* LDRA_JUSTIFY_END */
            {
              /* Making this memTab as PERSIST to avoid context memory DDR to DDR copy */
              memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].attrs = IALG_PERSIST;
              memRec[ALG_CONTEXT_MEM_MEMREC].size =
                  TIDL_getContextMemSize(createParams->targetPriority, perfInfoOut);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            The condition check is deterministic and does not impact the safety or reliability of the system.
            Therefore, it is excluded from safety coverage requirements.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            else
            {
#endif
              memRec[ALG_CONTEXT_MEM_MEMREC].size = TIDL_PREEMPT_CONTEXT_MEMSIZE_DUMMY;
#if ENABLE_PREEMPTION
            }
#endif
            /* LDRA_JUSTIFY_END */
            memRec[ALG_CONTEXT_MEM_MEMREC].space = TIDL_DDR_MEMREC_NON_CACHEABLE;
            memRec[ALG_CONTEXT_MEM_MEMREC].attrs = IALG_PERSIST;
            memRec[ALG_CONTEXT_MEM_MEMREC].alignment = 128;

            if ((perfInfoOut != NULL) && (gcOutArgs != NULL))
            {
              /* Use init data backup from Net if available */
              if (createParams->net->netInitBackupDataOffset[relativeCoreId] == 0)
              {
                memRec[ALG_PERSIST_DDR_INIT_DATA].size += ((uint32_t)gcOut->memoryRequirementPerCore[DDR_PERSIST] + 128U);
              }
              memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].size = (gcOut->memoryRequirementPerCore[DDR] + 128);
            }

            /* memTab to allocate memory for net structure along with compiled information*/
            memRec[ALG_PERSIST_DDR_NET_MEMREC].size = (int32_t)TIDL_getSizeNet(createParams) + TIDL_ALIGNMENT_SIZE;
            memRec[ALG_PERSIST_DDR_NET_MEMREC].space = TIDL_DDR_MEMREC_CACHEABLE;
            memRec[ALG_PERSIST_DDR_NET_MEMREC].attrs = IALG_PERSIST;
            memRec[ALG_PERSIST_DDR_NET_MEMREC].alignment = 128;

#ifdef HOST_EMULATION
            if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
            {
              for (i = 0; i < (int32_t)MAX_NUM_MEMRECS; i++)
              {
                memRec[i].space = TIDL_DDR_MEMREC_CACHEABLE;
              }
            }
#endif
          }
        }

        if (status == IALG_EOK)
        {
          if (tidl_getTraceLogLevel() >= 2)
          {
            TIDL_printMemorySizeStats(memRec, MAX_NUM_MEMRECS, 1);
          }
          TIDL_play(createParams->net, 0);
        }
      }
    }
  }
  return (status);
}
/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_init
@brief      Function to accept memory pointers from user and initialize algo
            Refer algInit function for details in XADIS (ialg.h file)
@param      handle   : Algorithm Instance handle
@param      memRec   : memory table populated by the Algorithm
@param      parent   : Parent function pointer
@param      params   : Input Parameters
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_init(IALG_Handle handle,
                  const IALG_MemRec memRec[],
                  IALG_Handle parent,
                  const IALG_Params *params)
{
  int32_t status = IALG_EOK;
  TIDL_Handle algHandle = (TIDL_Handle)handle;
  algHandle->numMemRecs = (uint32_t)MAX_NUM_MEMRECS;
  const TIDL_CreateParams *createParams;
  uint32_t coreId;
  int32_t relativeCoreId;

#ifdef DEBUG_PREEMPTION_HOST_EMULATION
  gHandleForPremptTest = handle;
#endif

  if ((memRec != NULL)) /* TIDL init call from ivision API */
  {
    tidl_printf(1, "TIDL init call from ivision API \n");
    if (tidl_getTraceLogLevel() >= 2)
    {
      TIDL_printMemorySizeStats(memRec, MAX_NUM_MEMRECS, 0);
    }
    createParams = (const TIDL_CreateParams *)(const void *)params;

    (void)memcpy(algHandle->memRec, memRec, sizeof(IALG_MemRec) * algHandle->numMemRecs);
    algHandle->ivision = &TIDL_VISION_FXNS;
    algHandle->createParams = (TIDL_CreateParams *)createParams;
#if TIDL_DEVICE_MULTICORE
    if (((createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)) && ((createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) != TIDL_FLOW_CTRL_REF_STAT))
    {
      /* TODO_NG : Remove TIDL_inferenceModeHighThroughput after low latency mode is stable*/
      tidl_printf(1, "Returning from TIDL ivision API call \n");
      /* In case of low latency mode, control API is called from nested graph which does the actual init of pointers
         Current init is only used to copy over memRecs and ivision function ptrs */
      /* Import tool still used the TIDL test executable and not TIDL-RT executable for init flow for import tool is same as other inference modes */
      status = TIDL_ERROR_ALG_COMMON_MULTICORE_INIT_CHECK;
    }
#endif
  }
  else /* TIDL control call for init */
  {
    tidl_printf(1, "TIDL control init call \n");
    memRec = (IALG_MemRec *)(algHandle->memRec);
    createParams = (TIDL_CreateParams *)algHandle->createParams;
  }
#if TIDL_DEVICE_MULTICORE
  if(status == IALG_EOK)
#endif
  {
   TIDL_play(createParams->net, 1);

  coreId = createParams->coreId;
  relativeCoreId = GET_RELATIVE_COREIDX(coreId, createParams->coreStartIdx);

  int32_t currAlgLayer = 0;
  uint32_t paramMemTabOffset = 0;
  uint32_t dataMemTabOffset = 0;
  sPerfSim_t *perfInfoOut;

  int32_t i, j;
  int32_t intAlghandleMemSize;
  int32_t dmaContextSize;
  sGraphCompilerOutArgs_t *gcOutArgs = NULL;
  sGraphCompilerOutArgsCore_t *gcOut = NULL;
  sWorkloadUnit_t *workloadUnit = NULL;
  sGCHelperHandle *gcHelperHandle = NULL;
  int32_t msmcSramSize;
  int32_t l2SramSize;
  int32_t l3ReservedSize;

  algHandle->perfSimOutput = NULL;
  algHandle->isPreEmptionEnable = 0;
  TIDL_LayerSpecificParams layerSpecificParams;
  TIDL_NetworkCommonParams commonParams;
  WorkloadUnitExec_CommonParams tidlCommonParams;

  uint8_t *memory[TIDL_LAYER_MEMORY_MAX] = {0};
  int32_t memorySize[TIDL_LAYER_MEMORY_MAX] = {0};
  sWorkloadSuperGroup_t *wlSuperGrp = NULL;

  status = TIDL_initDebugTraceParams(createParams->traceLogLevel, createParams->traceWriteLevel, createParams->TIDLVprintf, createParams->TIDLWriteBinToFile,
                                     createParams->TIDLReadBinFromFile, createParams->traceBaseName);
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
  TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_002
  Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
  This does not affect runtime behavior or safety.
  <justification end> */
  if (status != IALG_EOK)
  {
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_DEBUG_TRACE, TIDL_ERROR_DEBUG_TRACE_INVALID_PARAM);
    status = IALG_EFAIL;
  }
  /* LDRA_JUSTIFY_END */
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
  TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_002
  Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
  This does not affect runtime behavior or safety.
  <justification end> */
  if (status == IALG_EOK)
  /* LDRA_JUSTIFY_END */
  {
    algHandle->TIDLLayersBuf =
        (sTIDL_LayerBuf_t *)(void *)memRec[ALG_LAYERS_MEMREC].base;

    algHandle->alglayerParams =
        (sTIDL_AlgLayer_t *)(void *)(get_int8_t_pointer((int8_t *)(void *)memRec[ALG_LAYERS_MEMREC].base,
                                                        ALIGN_SIZE((uint32_t)sizeof(sTIDL_LayerBuf_t), 128U)));

    algHandle->udmaDrvObj = createParams->udmaDrvObj;

#ifdef HOST_EMULATION
    if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U)
#endif
    {
      algHandle->createParams->quantRangeUpdateFactor = 0.0f;
      if (algHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_TIDLRT_UTILS_FORCE_QNTRNG_NULL)
      {
        algHandle->createParams->quantRangeUpdateFactor = -1.0f;
      }
      if (algHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_TIDLRT_ALG_GCINFO_NULL)
      {
        algHandle->createParams->net->isGCInfoAvailable = 0;
      }
      if (algHandle->udmaDrvObj == NULL)
      {
        status = IALG_EFAIL;
      }
    }
  }

  if (status == IALG_EOK)
  {
    for (i = 0; i < (int32_t)TIDL_MAX_DATA_BUFS; i++)
    {
      algHandle->dataBuf[i] = NULL;
    }
  }

  if (status == IALG_EOK)
  {
    /* analyze the network */
    perfInfoOut = NULL;
    msmcSramSize = 128;
    l3ReservedSize = 128;
    l2SramSize = 128;
    algHandle->gcHelperHandle = NULL;
    algHandle->wlSuperGrp = NULL;
    algHandle->groupId = 0;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_006
    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
    This does not affect runtime behavior or safety.
    <justification end> */
    if (algHandle->createParams->net->isGCInfoAvailable != 0)
    {
      /* LDRA_JUSTIFY_END */
      int32_t isReference = TIDL_referencFlow(algHandle->createParams);
      sGCCommonDataBase_t *gcCommonDataBase = NULL;
      sGCDataBase_t *gcDatabase;
      perfInfoOut = (sPerfSim_t *)TIDL_getNetObjBufPtr(algHandle->createParams->net, memRec, (uint32_t)TIDL_NET_COMPILER_BUF);
      msmcSramSize = (perfInfoOut->simConfig.sizeL3MemKB - (int32_t)L3MEM_SCRATCH_BUF_SIZEKB) * 1024;
      l2SramSize = (perfInfoOut->simConfig.sizeL2MemKB - (int32_t)L2MEM_SCRATCH_BUF_SIZEKB) * 1024;
      // TODO: [8.5 rebase]. Make this similar to perfsimInfo
      /* Use memrec pointer for GC flow instead of directly using it from the net as these pointers are stored
      during init itself */

      gcOutArgs = (sGraphCompilerOutArgs_t *)TIDL_CopyAndReturnGCPrivateMemPtr(algHandle, memRec, isReference);

      gcOut = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs, (int32_t)DATABASE_GCOUT_1 + relativeCoreId);

      gcDatabase = &gcOut->dataBase;
      gcHelperHandle = &algHandle->gcHelperHandleObj;
      gcCommonDataBase = (sGCCommonDataBase_t *)getGCDataBase(gcOutArgs, DATABASE_COMMON);

      wlSuperGrp = &gcOut->superWorkload;
      algHandle->gcHelperHandle = gcHelperHandle;
      algHandle->wlSuperGrp = wlSuperGrp;
      sBufParams_t *bufNodesDB = (sBufParams_t *)&gcCommonDataBase->dbPayLoad[gcCommonDataBase->dbParams[DB_COMMON_BUF].startOffset];
      sWorkloadUnit_t *workLoadUnitDB = (sWorkloadUnit_t *)&gcDatabase->dbPayLoad[gcDatabase->dbParams[DB_WL].startOffset];
      // sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) gcDatabase->dbPayLoad[gcDatabase->dbParams[DB_BUF].size +
      //                                                                     gcDatabase->dbParams[DB_WL].size];

      sWorkloadUnitAuxilary_t *auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t *)&gcDatabase->dbPayLoad[gcDatabase->dbParams[DB_AUXWL].startOffset];
      gcOutArgs->commonDatabaseInfo.bufDB.pBufList = bufNodesDB;
      gcOut->dataBaseInfo.wlDB.pWLList = workLoadUnitDB;
      gcOut->dataBaseInfo.wlAuxDB.pWLAuxList = auxilaryWorkloadDB;

      status = initGCHelperHandle(gcHelperHandle, createParams->net, &gcOut->dataBaseInfo, &gcOutArgs->commonDatabaseInfo);
    }
#ifdef HOST_EMULATION
    if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      perfInfoOut = NULL;
    }
#endif

    algHandle->perfSimOutput = perfInfoOut;
  }

  if (status == IALG_EOK)
  {
    status = TIDL_AsymQuantizationCheck(algHandle);
  }

  if (status == IALG_EOK)
  {
    status = TIDL_OTFPadErrorCheck(algHandle);
  }

  /* Initialize newDataId of inData with dataId, so that this can be used later
      to find the free output buffer for Re-using
  */
#ifdef HOST_EMULATION
  if (status == IALG_EOK)
  {
    if (createParams->optimiseExtMem != TIDL_OptimiseExtMemL0)
    {
      for (i = 0; i < createParams->net->numLayers; i++)
      {
        const sTIDL_Layer_t *TIDLLayerCur = &createParams->net->TIDLLayers[i];
        for (j = 0; j < TIDLLayerCur->numInBufs; j++)
        {
          TIDLLayersBuf.newInDataId[i][j] = (int16_t)TIDLLayerCur->inData[j];
        }
      }
    }
#ifdef HOST_EMULATION
    if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
    {
      algHandle->createParams->net->dataFlowInfo = 0;
    }
    if ((((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U) && (algHandle->createParams->net->dataFlowInfo == 0))
    {
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_COMMON_DATAFLOW_INFO_NULL);
      status = IALG_EFAIL;
    }
#endif
  }
#endif

  if (status == IALG_EOK)
  {
    /* Setup System memories */
    algHandle->sysMems[TIDL_SYSMEM_L1_SCRATCH].base =
        (void *)memRec[ALG_SCRATCH_L1_MEM_MEMREC].base;
    algHandle->sysMems[TIDL_SYSMEM_L1_SCRATCH].size =
        (int32_t)memRec[ALG_SCRATCH_L1_MEM_MEMREC].size;

    algHandle->sysMems[TIDL_SYSMEM_L2_SCRATCH].base =
        (void *)memRec[ALG_SCRATCH_L2_MEM_MEMREC].base;
    algHandle->sysMems[TIDL_SYSMEM_L2_SCRATCH].size =
        (int32_t)memRec[ALG_SCRATCH_L2_MEM_MEMREC].size;

    algHandle->sysMems[TIDL_SYSMEM_L3_SCRATCH].base =
        (void *)memRec[ALG_SCRATCH_L3_MEM_MEMREC].base;
    algHandle->sysMems[TIDL_SYSMEM_L3_SCRATCH].size =
        (int32_t)memRec[ALG_SCRATCH_L3_MEM_MEMREC].size;

    algHandle->sysMems[TIDL_SYSMEM_DDR_SCRATCH].base =
        (void *)memRec[ALG_SCRATCH_DDR_MEM_MEMREC].base;
    algHandle->sysMems[TIDL_SYSMEM_DDR_SCRATCH].size =
        (int32_t)memRec[ALG_SCRATCH_DDR_MEM_MEMREC].size;

    /* Reset all memories for the first time */
    TIDL_resetSysmem(algHandle->sysMems);
    intAlghandleMemSize = (int32_t)(ALIGN_SIZE((uint32_t)sizeof(TIDL_Obj), 128U));

    /*we are allocating a buffer of size 128 in beginning of msmc.
     Requirement : So in resize layer, it try to access some memory before it starting pointer.
     If starting pointer is 0, then it can't access. so to handle this, allocating a small buffer in beginning of msmc.
    */

    algHandle->sysScratchPtr.l3ReservedMem[relativeCoreId] = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                                                      (uint32_t)l3ReservedSize,
                                                                                                      0U,
                                                                                                      TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);

    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_INIT_FORCE_L3BASEPTR_NULL)
    {
      msmcSramSize = INT32_MAX;
    }
    uint32_t alignment = 128U;
    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_ALIGNMENT_ZERO)
    {
      alignment = 0U;
    }
    algHandle->sysScratchPtr.l3BasePtr[relativeCoreId] = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                                                  (uint32_t)msmcSramSize,
                                                                                                  alignment,
                                                                                                  TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);

    if (algHandle->sysScratchPtr.l3BasePtr[relativeCoreId] == NULL)
    {
      status = IALG_EFAIL; //: TODO: Add new error types for this
      tidl_printf(0, "Not able to allocate MSMC memory\n");
    }
    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_INTALGHNDLE_NULL)
    {
      intAlghandleMemSize = INT32_MAX; // algHandle->intAlgHandle = NULL;
    }

    algHandle->intAlgHandle = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                       (uint32_t)intAlghandleMemSize,
                                                                       128U,
                                                                       TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);

    if (algHandle->intAlgHandle == NULL)
    {
      /* TIDL_LDRA_TAG_ALG_PRIOR_CHECK_004 */
      status = IALG_EFAIL; //: TODO: Add new error types for this
      tidl_printf(0, "Not able to allocate intAlgHandle memory\n");
    }
  }

  if (status == IALG_EOK)
  {
    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_L2BASEPTR_NULL)
    {
      l2SramSize = INT32_MAX;
    }
    algHandle->sysScratchPtr.l2BasePtr[relativeCoreId] = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                                                  (uint32_t)l2SramSize,
                                                                                                  128U,
                                                                                                  TIDL_SYSMEM_L2_SCRATCH, (uint32_t)IALG_PERSIST);

    if (algHandle->sysScratchPtr.l2BasePtr[relativeCoreId] == NULL)
    {
      status = IALG_EFAIL; //: TODO: Add new error types for this
      tidl_printf(0, "Not able to allocate L2 memory\n");
    }
  }

  if (status == IALG_EOK)
  {
    uint32_t privContextSize = TIDL_privGetContextSize();
    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_PRVCNTX_NULL)
    {
      privContextSize = INT32_MAX; // not possible force above function to wrong value
    }
    algHandle->privContext = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                      (uint32_t)privContextSize,
                                                                      128U,
                                                                      TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);

    if (algHandle->privContext == NULL)
    {
      status = IALG_EFAIL;
      tidl_printf(0, "Not able to allocate priv algo context memory\n");
    }
  }

#ifdef HOST_EMULATION
  if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U)
#endif
  {
#if defined TIDL_COVERAGE_DEAD_CODE
    if (status == IALG_EOK)
    {
      if (gTempUseMsmcStageForWt == 1)
      {
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_WTMEMBASEPTR_NULL)
        {
          algHandle->layerShare.wtMemBasePtr = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                                        INT32_MAX,
                                                                                        128U,
                                                                                        TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);
        }
        else
        {
          algHandle->layerShare.wtMemBasePtr = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                                        TIDL_WT_STAGING_MEM,
                                                                                        128U,
                                                                                        TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);
        }

        algHandle->layerShare.currPtrOffset = (int32_t)0;
        algHandle->layerShare.lastPtrOffset = (int32_t)0;
        algHandle->layerShare.isWtDmaPending = (int32_t)0;
        algHandle->layerShare.totStageMemAvail = (int32_t)TIDL_WT_STAGING_MEM;

        algHandle->weightStageSync.wtMemBasePtr = algHandle->layerShare.wtMemBasePtr;
        algHandle->weightStageSync.currPtrOffset = (uint32_t)algHandle->layerShare.currPtrOffset;
        algHandle->weightStageSync.lastPtrOffset = (uint32_t)algHandle->layerShare.lastPtrOffset;
        algHandle->weightStageSync.isWtDmaPending = (uint32_t)algHandle->layerShare.isWtDmaPending;
        algHandle->weightStageSync.totStageMemAvail = (uint32_t)algHandle->layerShare.totStageMemAvail;

        if (algHandle->layerShare.wtMemBasePtr == NULL)
        {
          status = IALG_EFAIL; //: TODO: Add new error types for this
          tidl_printf(0, "Not able to allocate wt stage memory\n");
        }
      }
    }
#endif
    if (status == IALG_EOK)
    {
      dmaContextSize = DmaUtilsAutoInc3d_getContextSize((uint32_t)TIDL_DMA_CHANNEL_MAX);
      if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_DMAUTILSCNTX_NULL)
      {
        dmaContextSize = INT32_MAX - 2048; // not possible force above function to wrong value
      }

      algHandle->dmaUtilsContext = (void *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                         (uint32_t)dmaContextSize,
                                                                         128U,
                                                                         TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);

      if (algHandle->dmaUtilsContext == NULL)
      {
        /* TIDL_LDRA_TAG_ALG_PRIOR_CHECK_005 */
        status = TIDL_ERROR_ALG_COMMON_NULL_DMAUTILSCTNTX_ERROR;
        tidl_printf(0, "Not able to allocate DMAUTILS context memory\n");
      }
    }

    if (status == IALG_EOK)
    {
      uint32_t zeroVector1kSize = TIDL_1K_ZERO_VECTOR;
      uint32_t memcpyTrSize = TIDL_MEMCPY_TR_MEM;
      if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_ZEROVEC1K_NULL)
      {
        zeroVector1kSize = INT32_MAX;
      }
      algHandle->zeroVector1k = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                         (zeroVector1kSize),
                                                                         128U,
                                                                         TIDL_SYSMEM_L2_SCRATCH,
                                                                         (uint32_t)IALG_PERSIST);

      if (algHandle->zeroVector1k == NULL)
      {
        tidl_printf(0, "Not able to allocate zero vector in L2  memory\n");
        status = TIDL_ERROR_ALG_COMMON_NULL_ZEROVEC1K_ERROR;
      }
      if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_MCPYTR_NULL)
      {
        memcpyTrSize = INT32_MAX;
      }
      algHandle->memcpyTr = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                     (memcpyTrSize),
                                                                     128U,
                                                                     TIDL_SYSMEM_L2_SCRATCH,
                                                                     (uint32_t)IALG_PERSIST);

      if (algHandle->memcpyTr == NULL)
      {
        tidl_printf(0, "Not able to allocate memcpy xfer in L2  memory\n");
        status = TIDL_ERROR_ALG_COMMON_NULL_MEMCPYTR_ERROR;
      }
    }
#if ENABLE_PREEMPTION
    if (status == IALG_EOK)
    {
      algHandle->isPreEmptionEnable = TIDL_checkIfPreEmptionEnabled(createParams);

      if (algHandle->isPreEmptionEnable != 0)
      {
        uint32_t preEmptHandleSize = TIDL_getHandleSizeForPreemption(algHandle->createParams->net->numLayers);
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_PREMPHNDLE_NULL)
        {
          preEmptHandleSize = INT32_MAX; // fails to allocate for both L3 and DDR
        }
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_PREMPHNDLEDDR_NULL)
        {
          preEmptHandleSize = 3 * 1024; // 3MB mem request to avoid allocation in L3 and should able to allocate in DDR
        }
        /* Allocate memory for preemption handle */
        algHandle->preEmptHandle = (uint8_t *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                            preEmptHandleSize,
                                                                            128U,
                                                                            TIDL_SYSMEM_L3_SCRATCH, (uint32_t)IALG_PERSIST);

        if (algHandle->preEmptHandle == NULL)
        {
          {
            algHandle->preEmptHandle = (void *)TIDL_getMemoryChunkFromSysmem(algHandle->sysMems,
                                                                             preEmptHandleSize,
                                                                             128U,
                                                                             TIDL_SYSMEM_DDR_SCRATCH, (uint32_t)IALG_PERSIST);
          }

          tidl_printf(2, "WARNING: Preemption handle allocated in DDR!\n");
        }

        if (algHandle->preEmptHandle == NULL)
        {
          status = TIDL_ERROR_ALG_COMMON_NULL_PRMTHNDLE_ERROR;
          tidl_printf(0, "Not able to allocate preEmption Handle\n");
        }
      }
    }
#endif
  }

  if (status == IALG_EOK)
  {
    //: TODO: Remove this once inner product buffers are correclty given by perfsim
    algHandle->sysMems[TIDL_SYSMEM_L2_SCRATCH].base =
        (void *)memRec[ALG_SCRATCH_L2_MEM_MEMREC].base;
    algHandle->sysMems[TIDL_SYSMEM_L2_SCRATCH].size =
        (int32_t)memRec[ALG_SCRATCH_L2_MEM_MEMREC].size - (int32_t)TIDL_1K_ZERO_VECTOR - (int32_t)TIDL_MEMCPY_TR_MEM;

    algHandle->sysScratchPtr.l1BasePtr[relativeCoreId] = (uint8_t *)memRec[ALG_SCRATCH_L1_MEM_MEMREC].base;
    algHandle->sysScratchPtr.ddrBasePtr[relativeCoreId] = (uint8_t *)memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].base;

    //: TODO: Find the right place
#if TIDL_DEVICE_MULTICORE
    if (algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
    {
      uint32_t coreIdx;
      for (coreIdx = 0; coreIdx < MAX_CORES; coreIdx++)
      {
        algHandle->sysScratchPtr.l1BasePtr[coreIdx] = (uint8_t *)algHandle->controlArgs.ctrlSetArgs[coreIdx].ncScratchPtr[TIDL_Virtual][TIDL_L1];
        algHandle->sysScratchPtr.l2BasePtr[coreIdx] = (uint8_t *)algHandle->controlArgs.ctrlSetArgs[coreIdx].ncScratchPtr[TIDL_Virtual][TIDL_L2];
        algHandle->sysScratchPtr.l3BasePtr[coreIdx] = (uint8_t *)algHandle->controlArgs.ctrlSetArgs[coreIdx].ncScratchPtr[TIDL_Virtual][TIDL_MSMC];
        algHandle->sysScratchPtr.ddrBasePtr[coreIdx] = (uint8_t *)algHandle->controlArgs.ctrlSetArgs[coreIdx].ncScratchPtr[TIDL_Virtual][TIDL_DDR];
        algHandle->sysScratchPtr.ddrPersistentBasePtr[coreIdx] = (uint8_t *)algHandle->controlArgs.ctrlSetArgs[coreIdx].ncScratchPtr[TIDL_Virtual][TIDL_DDR_PERSIST];
      }
    }
#endif
  }

  if (status == IALG_EOK)
  {
    /* #######################################################################################################################################*/
    /* ##############################  No new Persistent memory records should be allocated in INIT after this point   #######################*/
    /* INIT for each layer resets sysMem resulting in offset set to 0 without any base ptr movement and any persistent memories allocated here on
        will be allocated at offset 0 and thereby overlap with layer level allocated scratch memories and can be potentially overwritten */
    /* ########################################################################################################################################*/
    algHandle->dmaChannelAllocContext.currChannelIdx = 0;
    algHandle->dmaChannelAllocContext.totalNumChannels = TIDL_DMA_CHANNEL_MEMCPY;

    tidlCommonParams.dmaChannelAllocContext = &algHandle->dmaChannelAllocContext;
    tidlCommonParams.dmaUtilsContext = algHandle->dmaUtilsContext;
    tidlCommonParams.sysMems = &algHandle->sysMems[0];
    tidlCommonParams.sysScratchPtr = &algHandle->sysScratchPtr;
    tidlCommonParams.zeroVector1k = algHandle->zeroVector1k;
    tidlCommonParams.coreId = relativeCoreId;
    tidlCommonParams.forceNegativeTest = createParams->forceNegativeTest;

    commonParams.createParams = createParams;
    commonParams.gcHelperHandle = gcHelperHandle;
    commonParams.net = createParams->net;
    commonParams.TIDLLayersBufPtr = &TIDLLayersBuf; //: TODO: Check if this needs to be a global variable
    commonParams.tidlCommonParams = &tidlCommonParams;

    if (tidlCommonParams.forceNegativeTest == TIDL_SAFETY_FLAG_WL_EXECINIT_FORCE_SYSMEMSIZES_ZERO)
    {
      tidlCommonParams.sysMems[TIDL_SYSMEM_L1_SCRATCH].size = 0;
      tidlCommonParams.sysMems[TIDL_SYSMEM_L2_SCRATCH].size = 0;
      tidlCommonParams.sysMems[TIDL_SYSMEM_L3_SCRATCH].size = 0;
      tidlCommonParams.sysMems[TIDL_SYSMEM_DDR_SCRATCH].size = 0;
    }

    int32_t layerId;
    int32_t wlRepeatIter;
    sGetLayerIdContext_t getLayerIdContext;

    if (perfInfoOut != NULL)
    {
      if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_PROCESS_FORCE_GETEXECNUMLYR_NULL)
      {
        perfInfoOut->layerExecutionOrder[4] = 0;
      }
      /* Use init data backup from Net if available */
      if (createParams->net->netInitBackupDataOffset[relativeCoreId] != 0)
      {
        algHandle->sysScratchPtr.ddrPersistentBasePtr[relativeCoreId] = (uint8_t *)get_int8_t_pointer((int8_t *)(createParams->net), createParams->net->netInitBackupDataOffset[relativeCoreId]);
      }
      else
      {
        algHandle->sysScratchPtr.ddrPersistentBasePtr[relativeCoreId] = (uint8_t *)memRec[ALG_PERSIST_DDR_INIT_DATA].base;
      }
    }

    getLayerIdInit(&getLayerIdContext,
                   relativeCoreId,
                   createParams->currLayersGroupId,
                   createParams->net,
                   gcHelperHandle,
                   wlSuperGrp);
#ifdef HOST_EMULATION
    int32_t prevDataIdExecuted = NOT_VALID;
#endif
    int32_t flag = 1;

    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_INIT_FORCE_WHILEFLAG_ZERO)
    {
      flag = 0;
      status = IALG_EFAIL;
    }
    while (flag == 1)
    {
      layerId = getLayerIdToExecute(&getLayerIdContext,
                                    1,
                                    &wlRepeatIter,
                                    &currAlgLayer,
                                    &workloadUnit);

      if (layerId == NOT_VALID)
      {
        break;
      }

      #ifdef HOST_EMULATION
      #if LAYER_TO_RUN != NOT_VALID
      if (layerId != LAYER_TO_RUN)
        continue;
      #endif
      if ( (createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
      {
        /* If previous and current layer id are the same then this indicates that layer is split across multiple workloads.
           Ref process full data in one call. So we have to skip ref execution after first call. 
        */

        if (((createParams->net->TIDLLayers[layerId].layerType == TIDL_TransposeLayer) ||
             (createParams->net->TIDLLayers[layerId].layerType == TIDL_GatherNDLayer)  ||
             (createParams->net->TIDLLayers[layerId].layerType == TIDL_LSTMLayer)  ||
             (createParams->net->TIDLLayers[layerId].layerType == TIDL_GRULayer)  ||
             (createParams->net->TIDLLayers[layerId].layerType == TIDL_RNNLayer)
            ) && ( prevDataIdExecuted == layerId )
           )
        {
          continue;
        }
      }
#endif
#ifdef HOST_EMULATION
      //: TODO: Eventually this should be removed once process also  uses the same loop structure
      if (createParams->net->TIDLLayers[layerId].layerType == TIDL_DataLayer)
      {
        continue;
      }
#endif
      tidl_printf(1, "Alg Init for Layer # - %4d\n", layerId);
      memory[TIDL_LAYER_MEMORY_OUTPUT] = (uint8_t *)memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].base + dataMemTabOffset;
      memory[TIDL_LAYER_MEMORY_SCRATCH] = (uint8_t *)memRec[ALG_REF_SCRATCH_BUFF_MEMREC].base;
      memory[TIDL_LAYER_MEMORY_PERSISTENT] = (uint8_t *)memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].base + paramMemTabOffset;

      sTIDL_AlgLayer_t *algLayer = get_AlgLayer_t_pointer(algHandle->alglayerParams, currAlgLayer);

      if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_INIT_FORCE_WLUNIT_NULL)
      {
        currAlgLayer = 32;
      }

      TIDL_getPadParams(&algLayer->wlPadParams, &createParams->net->TIDLLayers[layerId], createParams->flowCtrl, workloadUnit, gcHelperHandle);

      int32_t isWorkLoadSupported;

      isWorkLoadSupported = TIDL_isWorkLoadUnitFlowSupported(layerId,
                                                             createParams->net,
                                                             createParams->net->deviceName,
                                                             workloadUnit);

      /* Force workload unit to be NULL for going to old flow */
      if (isWorkLoadSupported == 0)
      {
        workloadUnit = NULL;
      }

      /*setting defualt values for multiCoreCtxtLinkIndex */
      for (i = 0; i < MAX_MULTI_CORE_CTX_COPY_LINK; i++)
      {
        algLayer->multiCoreCtxtLinkIndex[i] = NOT_VALID;
      }

#if TIDL_DEVICE_MULTICORE
      if ((isWorkLoadSupported != 0) && (createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency))
      {
        TIDL_setMultiCoreCtxtLinkIndex(algHandle, algLayer, currAlgLayer);
      }
#endif

#ifdef HOST_EMULATION
      if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == 0U)
      {
        if (algHandle->createParams->quantRangeUpdateFactor == 0.0f)
        {
          TIDL_UpdateScaleFactors(algHandle, layerId, 0, 0, 0);
        }
      }
#endif

      int32_t bufDBIndexTemp = 0;
      for (j = 0; j < createParams->net->TIDLLayers[layerId].numInBufs; j++)
      {
        int32_t inDataId = createParams->net->TIDLLayers[layerId].inData[j];
        algLayer->inLayerIdx[j] = (int16_t)TIDL_getLayerNum(createParams->net, inDataId);
        algLayer->isInData[j] = (int16_t)TIDL_isInDataBuff(createParams->net, inDataId, createParams->currLayersGroupId);
        algLayer->isInOutData[j] = (int16_t)TIDL_isOutDataBuff(createParams->net, inDataId, createParams->currLayersGroupId);

        algLayer->inWorkloadIdx[j] = (int16_t)-1;
        /* if (createParams->net->TIDLLayers[algLayer->inLayerIdx[j]].layerType != TIDL_DataLayer && gcHelperHandle != NULL) */
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
        This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
        TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_007
        Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
        This does not affect runtime behavior or safety.
        <justification end> */
        if (gcHelperHandle != NULL)
        {
          /* LDRA_JUSTIFY_END */
          /* TODO: Might have to make this multi-core safe */
          /* Workload db is expected to be ordered by group execution order. Hence, the producer workload is expected to be
            nearby to the consumer workload in the array. Parsing db backwards from the current workload may result in quickly
            finding the producer workload. */

          sWorkloadUnit_t *pWLUnit = getWLUnitPtr(gcHelperHandle, currAlgLayer);
#ifdef HOST_EMULATION
          if ((createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
          {
            if ((createParams->net->TIDLLayers[layerId].layerType == TIDL_LSTMLayer ||
                 createParams->net->TIDLLayers[layerId].layerType == TIDL_GRULayer  ||
                 createParams->net->TIDLLayers[layerId].layerType == TIDL_RNNLayer) &&
                 j < 4)
            {
              pWLUnit = getWLUnitPtr(gcHelperHandle, currAlgLayer + 1);
            }
          }
#endif

          if (pWLUnit != NULL)
          {
            /* TIDL_LDRA_TAG_GC_HELPER_PRIOR_CHECK_001 */
            int32_t bufDBindex = NOT_VALID;
            for (int32_t linkIdx = 0; linkIdx < pWLUnit->numLinks; linkIdx++)
            {
              sLink_t *linkPtrList[MAX_LINKS_PER_WL];
              getLinkPtrs(pWLUnit, NOT_VALID, linkIdx, linkPtrList);
              const sLink_t *pLink = linkPtrList[0];

              if ((pLink->subType == (int32_t)LINK_X_FM_IN_FULL_TO_PART) || (pLink->subType == (int32_t)LINK_P))
              {
                for (int32_t srcIdx = 0; srcIdx < pLink->numSrc; srcIdx++)
                {
                  bufDBindex = pLink->src[srcIdx].bufDBindex;
                  const sBufParams_t *bufParams = getBufParamsFromBufIndex(gcHelperHandle, bufDBindex);
                  sMetaDataID_t metaDataId = {0, 0, 0, 0, 0, 0};
                  getMetaDataID(bufParams->dataId, &metaDataId);

                  if ((bufParams->type == (int32_t)BUF_FM_FULL) && (metaDataId.layerId == inDataId))
                  {
                    algLayer->inWorkloadIdx[j] = (int16_t)bufDBindex;
                    bufDBIndexTemp = bufDBindex;
                  }
                }
              }
            }
          }
          // for (int32_t k = currAlgLayer-1; k >= 0; k--)
          // {
          //   sWorkloadUnit_t *pWLUnit =  getWLUnitPtr(gcHelperHandle ,k);
          //   if (pWLUnit->layerId == algLayer->inLayerIdx[j])
          //   {
          //     algLayer->inWorkloadIdx[j] = k;
          //     break;
          //   }
          // }
        }
      }
      for (j = 0; j < createParams->net->TIDLLayers[layerId].numInBufs; j++)
      {
        if (algLayer->inWorkloadIdx[j] == (int16_t)-1)
        {
          algLayer->inWorkloadIdx[j] = (int16_t)bufDBIndexTemp;
        }
      }
      // TODO: Is this really needed now that we know we have only one output?
      for (j = 0; j < createParams->net->TIDLLayers[layerId].numOutBufs; j++)
      {
        int32_t outDataId = createParams->net->TIDLLayers[layerId].outData.dataId;
        algLayer->isOutData[j] = (int16_t)TIDL_isOutDataBuff(createParams->net, outDataId, createParams->currLayersGroupId);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
        This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
        TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_007
        Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
        This does not affect runtime behavior or safety.
        <justification end> */
        if (gcHelperHandle != NULL)
        {
          /* LDRA_JUSTIFY_END */
          if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_WORKLOAD_REF_EXEC_GCHELPERHANDLE_NULL)
          {
            gcHelperHandle = NULL;
          }
          if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_WORKLOAD_REF_EXEC_PBUFLIST_NULL)
          {
            gcHelperHandle->pBufDB->pBufList = NULL;
          }
          sWorkloadUnit_t *pWLUnit = getWLUnitPtr(gcHelperHandle, currAlgLayer);
#ifdef HOST_EMULATION
          if ((createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
          {
            if (createParams->net->TIDLLayers[layerId].layerType == TIDL_LSTMLayer ||
                createParams->net->TIDLLayers[layerId].layerType == TIDL_GRULayer  ||
                createParams->net->TIDLLayers[layerId].layerType == TIDL_RNNLayer)
            {
              pWLUnit = getWLUnitPtr(gcHelperHandle, currAlgLayer + 1);
            }
          }
#endif
          algLayer->outBufIdxWl = WorkloadRefExec_GetOutBufIdx(pWLUnit, gcHelperHandle);
          if (algLayer->outBufIdxWl == IALG_EFAIL)
          {
            status = IALG_EFAIL;                                   // Update status to EFAIL, as wl out buffer IDX is invalid
            j = createParams->net->TIDLLayers[layerId].numOutBufs; // Break out of the loop
          }
        }
      }
      if (status == IALG_EOK)
      {
        algLayer->workloadUnit = workloadUnit;
        algLayer->gcHelperHandle = gcHelperHandle;
        algLayer->layerIdx = layerId;
        algLayer->memcpyTr = algHandle->memcpyTr;
        algLayer->isOptimalForPreemption = 0;
        if (perfInfoOut != NULL)
        {
          algLayer->execLayerNum = getLayerExecutionNumber(algLayer->layerIdx,
                                                           perfInfoOut->layerExecutionOrder, createParams->net->numLayers);
        }
        else
        {
          algLayer->execLayerNum = algLayer->layerIdx;
        }

        void **outPtr = &algHandle->dataBuf[createParams->net->TIDLLayers[layerId].outData.dataId];

        layerSpecificParams.workloadUnit = workloadUnit;
        layerSpecificParams.isMixedPrecEnabled = 1U;

        /* Reset DMA resources and scratch memory resources for every iteration */
        (void)TIDL_ResetDmaChannel(&algHandle->dmaChannelAllocContext);
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_TIDLRT_UTILS_FORCE_DMACURIDX_INVALID)
        {
          /* this case to exceed non-interlave case*/
          algHandle->dmaChannelAllocContext.currChannelIdx = 32;
        }
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_PROCESS_FORCE_DMACURIDX_INVALID)
        {
          /* this to exceed totalNumChannels(13) for non-interleave and then interleave case*/
          algHandle->dmaChannelAllocContext.currChannelIdx = 15;
        }

        TIDL_resetSysmem(algHandle->sysMems);
        (void)memset(memorySize, 0, sizeof(memorySize));

        if (createParams->net->TIDLLayers[layerId].layerType == TIDL_PoolingLayer)
        {
          //: TODO: Remove this from here. Should be happening at init time. Need to remove the tidlLayer changing heights from
          // init routine and use things directly from workload in NatC before attempting this change.
          algLayer->layerParams.poolParams.orgInTensorHeight = createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[0]].outData.dimValues[TIDL_DIM_HEIGHT];
        }

        TIDL_setMultiCoreCoeffOffset(&(createParams->net->TIDLLayers[layerId]), algLayer, createParams->coreStartIdx, createParams->coreId, createParams->net->numCores);
        WorkloadRefExec_RefExecParams wlRefParams;

        int32_t multiCoreMode = (algLayer->workloadUnit != NULL)
                                  ? algLayer->workloadUnit->multiCoreMode
                                  : TIDL_NOT_MULTI_CORE;

#ifdef HOST_EMULATION
        /* TODO: Remove once workload migration is completed */
        if (((gcHelperHandle != NULL) && ((((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY))) ||
            (createParams->net->TIDLLayers[layerId].layerType == TIDL_ConvolutionLayer && (multiCoreMode & TIDL_MULTI_CORE_CHANNEL) == TIDL_MULTI_CORE_CHANNEL))
        {
          sWorkloadUnit_t *pWLUnit = getWLUnitPtr(gcHelperHandle, currAlgLayer);
          TIDL_privSetTensorDimensions(algHandle->createParams, algHandle->alglayerParams, algLayer, pWLUnit,
                                      wlRefParams.origInHeight, wlRefParams.origInCh, &wlRefParams.origGrp, wlRefParams.origInChPitch, wlRefParams.origInRoiPitch, wlRefParams.origInBatch,
                                      &wlRefParams.origOutHeight, &wlRefParams.origOutCh, &wlRefParams.origOutChPitch, &wlRefParams.origOutRoiPitch, &wlRefParams.origOutBatch);
        }
#endif
#ifdef TIDL_DEVICE_MULTICORE
        if ((gcHelperHandle != NULL) &&
            (createParams->net->TIDLLayers[layerId].layerType == TIDL_TransposeLayer && (multiCoreMode & TIDL_MULTI_CORE_BATCH) == TIDL_MULTI_CORE_BATCH))
        {/* Batch dimension in Transpose Layer for multicore batch split will be set here, because the original batch dimension in layer property is coming as total batch of this layer.*/
          sWorkloadUnit_t *pWLUnit = getWLUnitPtr(gcHelperHandle, currAlgLayer);
          TIDL_privSetTensorDimensions(algHandle->createParams, algHandle->alglayerParams, algLayer, pWLUnit,
                                      wlRefParams.origInHeight, wlRefParams.origInCh, &wlRefParams.origGrp, wlRefParams.origInChPitch, wlRefParams.origInRoiPitch, wlRefParams.origInBatch,
                                      &wlRefParams.origOutHeight, &wlRefParams.origOutCh, &wlRefParams.origOutChPitch, &wlRefParams.origOutRoiPitch, &wlRefParams.origOutBatch);
        }
#endif
        if ((createParams->forceNegativeTest == TIDL_SAFETY_FLAG_DEVICE_UTILS_FORCE_WL_NULL) || (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_CONV2D_DEVICE_FORCE_LAYERSPECIFICPARAMS_WL_NULL))
        {
          layerSpecificParams.workloadUnit = NULL;
          commonParams.gcHelperHandle = NULL;
        }

        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_INIT_FORCE_LAYERTYPE_INVALID)
        {
          int32_t *playerType = &(commonParams.createParams->net->TIDLLayers[layerId].layerType);
          *playerType = TIDL_UnsupportedLayer;
        }
        
        status = TIDL_layerInitNew(&layerSpecificParams,
                                   &commonParams,
                                   algLayer,
                                   layerId,
                                   memory,
                                   memorySize,
                                   outPtr);

#ifdef HOST_EMULATION
        /*TODO: Remove after workload migration */
        if (((gcHelperHandle != NULL) && ((((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY))) ||
            (createParams->net->TIDLLayers[layerId].layerType == TIDL_ConvolutionLayer && (multiCoreMode & TIDL_MULTI_CORE_CHANNEL) == TIDL_MULTI_CORE_CHANNEL))
        {
          TIDL_privRestoreTensorDimensions(algHandle->createParams, algLayer, wlRefParams.origInHeight, wlRefParams.origInCh, &wlRefParams.origGrp, wlRefParams.origInChPitch, wlRefParams.origInRoiPitch, wlRefParams.origInBatch,
                                          &wlRefParams.origOutHeight, &wlRefParams.origOutCh, &wlRefParams.origOutChPitch, &wlRefParams.origOutRoiPitch, &wlRefParams.origOutBatch);
        }
#endif

        if (status != IALG_EOK)
        {
          break;
        }

        dataMemTabOffset += memorySize[TIDL_LAYER_MEMORY_OUTPUT];
        paramMemTabOffset += memorySize[TIDL_LAYER_MEMORY_PERSISTENT];

        dataMemTabOffset = ALIGN_SIZE((uint32_t)dataMemTabOffset, 128U);
        paramMemTabOffset = ALIGN_SIZE((uint32_t)paramMemTabOffset, 128U);

        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_INIT_FORCE_LYERSPRMMEMTBOFS_MAX)
        {
          paramMemTabOffset = INT32_MAX;
        }
        if (paramMemTabOffset > memRec[ALG_LAYERS_PARAMS_BUFF_MEMREC].size)
        {
          /*tidl_printf(0,"Memory used for  ALG_LAYERS_PARAMS_BUFF_MEMREC is greater than requested \n");*/
          TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_COMMON_EXCEED_PARAMS_MEMTAB_REQUEST);
          status = IALG_EFAIL;
        }
        if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_INIT_FORCE_PRMMEMTBOFS_MAX)
        {
          dataMemTabOffset = UINT32_MAX;
        }
        if (dataMemTabOffset > memRec[ALG_SCRATCH_DATA_BUFF_MEMREC].size)
        {
          /* tidl_printf(0,"Memory used for  ALG_SCRATCH_DATA_BUFF_MEMREC is greater than requested \n"); */
          TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_COMMON_EXCEED_DATA_MEMTAB_REQUEST);
          status = IALG_EFAIL;
        }
#if defined (SOC_TDA54)
        algLayer->metaData.totalOps = 0;
        algLayer->metaData.actualOps = 0;
        algLayer->metaData.ddrBandwidthRead = 0;
        algLayer->metaData.ddrBandwidthWrite = 0;
#endif

        for (int32_t k = 0; k < TIDL_PROFILE_MAX; k++)
        {
          algLayer->metaData.profilePoint[k] = 0;
        }
        if(commonParams.createParams->net->TIDLLayers[layerId].layerType == TIDL_ReshapeLayer)
        {
          algLayer->metaData.profilePoint[2] = DEFAULT_CORE_LOOP_CYCLES; //CoreLoop cycles are set to default value here as Reshape is not WL;
        }
#ifdef HOST_EMULATION
        prevDataIdExecuted = layerId;
#endif
      }
    }
  }

  if (status == IALG_EOK)
  {
#ifdef HOST_EMULATION
    if (createParams->optimiseExtMem != TIDL_OptimiseExtMemL0)
    {
      for (i = 0; i < createParams->net->numLayers; i++)
      {
        const sTIDL_Layer_t *TIDLLayerCur = &createParams->net->TIDLLayers[i];
        for (j = 0; j < TIDLLayerCur->numInBufs; j++)
        {
          algHandle->TIDLLayersBuf->newInDataId[i][j] = TIDLLayersBuf.newInDataId[i][j];
        }
        algHandle->TIDLLayersBuf->newOutDataId[i][0] = TIDLLayersBuf.newOutDataId[i][0];
      }
    }
#endif
    algHandle->procCallCounter = 0;

#ifdef HOST_EMULATION
    {
      uint32_t ptrOffset = 0;
      algHandle->activationHistPtr = NULL;
      algHandle->activationRangePtr = NULL;

      if (((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
      {
        if ((((uint32_t)createParams->net->calibrationOption & (uint32_t)TIDL_CalibOptionActivationRange) ==
             (uint32_t)TIDL_CalibOptionActivationRange) &&
            (createParams->net->quantizationStyle != TIDL_QuantStyleAsymNP2))
        {
          uint32_t histogramSize = ((uint32_t)createParams->net->numLayers *
                                    TIDL_NUM_ACTIVATION_HISTOGRAM_BINS *
                                    sizeof(int32_t));

          algHandle->activationHistPtr = (int32_t *)get_uint8_t_pointer((uint8_t *)memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base, ptrOffset);
          (void)memset(algHandle->activationHistPtr, 0, histogramSize);
          ptrOffset += ALIGN_SIZE(histogramSize, 128U);
          if (createParams->net->calibrationParams.activationRangeMethod ==
              TIDL_ActivationRangeMethodGlobalHistogram)
          {
            int32_t layerIdx;
            uint32_t activationRangePtrSize = ((uint32_t)createParams->net->numLayers *
                                               2U * sizeof(float32_tidl));
            algHandle->activationRangePtr = (float32_tidl *)get_uint8_t_pointer((uint8_t *)memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base, ptrOffset);
            ptrOffset += ALIGN_SIZE(activationRangePtrSize, 128U);
            for (layerIdx = 0; layerIdx < createParams->net->numLayers; layerIdx++)
            {
              /* Copy the activation range during init these will come from the previous
              run of TIDL. Multiply it by a factor to account for any variation across
              iteration */
              algHandle->activationRangePtr[layerIdx * 2] =
                  createParams->net->TIDLLayers[layerIdx].outData.minTensorValue;
              algHandle->activationRangePtr[(layerIdx * 2) + 1] =
                  createParams->net->TIDLLayers[layerIdx].outData.maxTensorValue;
            }
          }
        }
      }

      algHandle->refScratchBuf = (uint8_t *)get_uint8_t_pointer((uint8_t *)memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base,
                                                                ptrOffset);
      algHandle->refScratchBufSize = memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size - ptrOffset;
    }
#endif
  }

  if (status == IALG_EOK)
  {
    if (algHandle->perfSimOutput != NULL)
    {
      sPerfSimConfig_t *pSimConfig = &algHandle->perfSimOutput->simConfig;
#ifdef SOC_J784S4
      pSimConfig->numEMIFPorts = EFFECTIVE_NUM_EMIF_PORTS_J784S4;
#endif
      algHandle->ddrBytesPerCPUCycle = (float32_tidl)pSimConfig->numEMIFPorts * (float32_tidl)pSimConfig->freqDDR *
                                       ((float32_tidl)pSimConfig->busWidthDDR / 8.0f) * (pSimConfig->ddrEfficiency / (float32_tidl)pSimConfig->freqMHz);
  #ifdef PERF_MODELLING
      sDeviceMemProperties_t deviceMemProperties[MEMSPACE_MAX];

      deviceMemProperties[MEMSPACE_L2] =
      {
        memRec[ALG_SCRATCH_L2_MEM_MEMREC].base,
        memRec[ALG_SCRATCH_L2_MEM_MEMREC].size,
        (double)((pSimConfig->busWidth_L3_L2/8.0f) * (pSimConfig->L2Efficiency))
      };
      deviceMemProperties[MEMSPACE_MSMC] =
      {
        memRec[ALG_SCRATCH_L3_MEM_MEMREC].base,
        memRec[ALG_SCRATCH_L3_MEM_MEMREC].size,
        (double)((pSimConfig->matPanelSize) * (pSimConfig->msmcEfficiency))
      };
      deviceMemProperties[MEMSPACE_DDR] =
      {
        memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].base,
        memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].size,
        (double)(algHandle->ddrBytesPerCPUCycle)
      };      
      deviceMemProperties[MEMSPACE_L1] =
      {
        memRec[ALG_SCRATCH_L1_MEM_MEMREC].base,
        memRec[ALG_SCRATCH_L1_MEM_MEMREC].size,
        (double)((pSimConfig->busWidth_L3_L2/8.0f) * (pSimConfig->L2Efficiency))
      };
      deviceMemProperties[MEMSPACE_L2].trCyclesPerByte   = 1.0 / (pSimConfig->matPanelSize * pSimConfig->L2Efficiency);
      deviceMemProperties[MEMSPACE_MSMC].trCyclesPerByte = 1.0 / (pSimConfig->matPanelSize * pSimConfig->msmcEfficiency);
      deviceMemProperties[MEMSPACE_DDR].trCyclesPerByte  = 1.0 / (algHandle->ddrBytesPerCPUCycle);/* efficiency is already included */
      
      TIDL_setMemSpaceConfig(deviceMemProperties);

      if(createParams->flowCtrl == TIDL_FLOW_CTRL_PERF_MODEL)
      {
        DmaUtilsAutoInc3d_setPerfExecMode();
      }
#endif
    }
#if ENABLE_PREEMPTION
    if (algHandle->isPreEmptionEnable != 0)
    {
      algHandle->preEmptContextInfo.contextMemPtr = (uint8_t *)memRec[ALG_CONTEXT_MEM_MEMREC].base;
      algHandle->preEmptContextInfo.contextMemSize = memRec[ALG_CONTEXT_MEM_MEMREC].size;
      algHandle->preEmptContextInfo.earlyPreEmpt = 0;
      status = TIDL_addPriorityObject(algHandle, &memRec[ALG_CONTEXT_MEM_MEMREC],
                                      algHandle->createParams->targetPriority);

      IALG_MemRec *pMemRec = (IALG_MemRec *)TIDL_getContextMemRec(algHandle);
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start>
      Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
      <justification end> */
      if (pMemRec != NULL)
      {
        /* LDRA_JUSTIFY_END */
        algHandle->preEmptContextInfo.contextMemPtr = (uint8_t *)pMemRec->base;
        algHandle->preEmptContextInfo.contextMemSize = (int32_t)pMemRec->size;
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
        Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
        <justification end> */
        if (gcOut != NULL)
        {
          /* LDRA_JUSTIFY_END */
          /* LDRA_JUSTIFY_START
          <metric start> branch <metric end>
          <justification start>
          Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
          <justification end> */
          /* Initialize preemption handle */
          if (status == IALG_EOK)
          {
            /* LDRA_JUSTIFY_END */
            int32_t numWL = gcOut->dataBaseInfo.wlDB.count;
            algHandle->preemptPerfHW = 1;
            /* Avoided initializing preEmptHandle in L3, instead initializing it in the contextMemPtr (DDR)
              and copy it to preEmptHandle in L3 in the activate function
            */
            TIDL_initializeHandleForPreemption(algHandle->preEmptContextInfo.contextMemPtr, algHandle->gcHelperHandle,
                                               algHandle->perfSimOutput,
                                               algHandle->createParams->net->numLayers, numWL, algHandle->createParams->maxPreEmptDelay,
                                               algHandle->sysScratchPtr.l2BasePtr[relativeCoreId], algHandle->ddrBytesPerCPUCycle,
                                               algHandle->perfSimOutput->simConfig.freqMHz, algHandle->alglayerParams);
            status = TIDL_updateContextMemPtrsForPreemption(&algHandle->preEmptContextInfo, algHandle->gcHelperHandle,
                                                            algHandle->privContext,
                                                            algHandle->preEmptHandle, algHandle->perfSimOutput, algHandle->sysScratchPtr.l2BasePtr[relativeCoreId],
                                                            algHandle->sysScratchPtr.l3BasePtr[relativeCoreId], NOT_VALID, algHandle->createParams->net->numLayers);
          }
          tidl_printf(1, "TIDL_initializeHandleForPreemption is completed \n");
        }
      } // pMemRec
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>
      Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
      <justification end> */
      else
      {
        status = IALG_EFAIL;
      }
      /* LDRA_JUSTIFY_END */
    }
#endif
  }

  if (status == IALG_EOK)
  {
#if TIDL_DEVICE_MULTICORE
    if ((algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency))
    {
      /* Set the Global sync buf values to default state */
      volatile int8_t *syncPtr = (int8_t *)algHandle->controlArgs.ctrlSetArgs[relativeCoreId].syncBufferPtr[TIDL_Virtual];
      (void)memset((void *)syncPtr, TIDL_SYNC_WL_STATE_DEFAULT, (uint64_t)TIDL_GLOBAL_SYNC_OFFSET * sizeof(int8_t));
    }
#endif
    sTIDL_Network_t *origNet = createParams->net;

    if (algHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_MOVENET_ALGLAYERPARAMS_NULL)
    {
      algHandle->createParams->net->netInitBackupDataOffset[0] = 1;
      algHandle->alglayerParams = NULL; /* Forcefully set alglayerParams to NULL for negative test */
    }
    status = TIDL_MoveNetToPrivateMemory(algHandle, memRec, relativeCoreId);
    if (status == IALG_EOK)
    {

      /*-----------------------------------------------------------------------*/
      /* State to indicate that internal context of algorithm is not active    */
      /*-----------------------------------------------------------------------*/
      algHandle->algState = (uint8_t)ALG_NOT_ACTIVE;
      TIDL_play(origNet, 0);
    }
  }

  if (status == IALG_EOK)
  {
    if (createParams->dumpNetInitBackupData != 0)
    {
      char fileName[TIDL_STRING_SIZE + 28];
      sprintf(fileName, "%s_persistent_data_%d", createParams->traceBaseName, relativeCoreId);
      (void)createParams->TIDLWriteBinToFile(fileName, memRec[ALG_PERSIST_DDR_INIT_DATA].base, memRec[ALG_PERSIST_DDR_INIT_DATA].size, 1);
    }

    // if(createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_2DMEMCPY_FORCE_INTALGHNDLE_NULL)
    //{
    //   algHandle->intAlgHandle = NULL;
    // }
    // if(createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_ACT_FORCE_ALGSTATE_INVALID)
    //{
    //   algHandle->algState = INT8_MAX;
    // }
    // if(createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_ACT_FORCE_2DMEMCPYSTATE_INVALID)
    //{
    //   algHandle->memRec[ALG_HANDLE_MEMREC].base = NULL;
    // }
    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_PROCESS_FORCE_TRACEWRITE_INVALID)
    {
      algHandle->createParams->TIDLVprintf = NULL;
    }
    // if(createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_GRPID_INVALID)
    //{
    //   algHandle->groupId = 2;
    // }
    // if(createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_LNKTYPE_INVALID)
    //{
    //   /* corrupting flowcnt here instead of bin corruption as bin corruption resulting in init failure */
    //   sWorkloadUnit_t *workloadUnitLocal = getWLUnitPtr(gcHelperHandle, 20);
    //   sLink_t *linkPtrListLocal[MAX_LINKS_PER_WL];
    //   getLinkPtrs(workloadUnitLocal, NOT_VALID, 0, linkPtrListLocal);
    //   sLink_t *linkLocal = linkPtrListLocal[0];
    //   linkLocal->type = 0;
    // }
    if (createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_FLOWCNT_INVALID)
    {
      /* corrupting flowcnt here instead of bin corruption as bin corruption resulting in init failure */
      sWorkloadUnit_t *workloadUnitLocal = getWLUnitPtr(gcHelperHandle, 20);
      sLink_t *linkPtrListLocal[MAX_LINKS_PER_WL];
      getLinkPtrs(workloadUnitLocal, NOT_VALID, 0, linkPtrListLocal);
      sLink_t *linkLocal = linkPtrListLocal[0];
      linkLocal->src[0].flowCnt[FLOW_PIPELINE][0] = 0;
    }
  }

  #ifdef BUILD_WITH_CUDA_MEM_MANAGER
  /* Initialize CUDA Memory Manager - parallel GPU memory for all memrecs */
  if(status == IALG_EOK)
  {
    tidl_printf(1, "Initializing CUDA Memory Manager...\n");
    
    /* Allocate memory for the manager structure */
    algHandle->cudaMemManager = malloc(sizeof(TIDL_CudaMemManager));
    if(algHandle->cudaMemManager == NULL)
    {
      tidl_printf(0, "Failed to allocate CUDA Memory Manager structure\n");
      status = IALG_EFAIL;
    }
    else
    {
      TIDL_cudaSetThreadManager((TIDL_CudaMemManager*)algHandle->cudaMemManager);
    }
    
    if(status == IALG_EOK)
    {
      /* Initialize the memory manager */
      status = TIDL_cudaMemManagerInit((TIDL_CudaMemManager*)algHandle->cudaMemManager, 
                                       algHandle->memRec, 
                                       (int32_t)algHandle->numMemRecs,
                                       algHandle->createParams->net->numLayers,
                                       algHandle->createParams->net->TIDLLayers);
      
      if(status == IALG_EOK)
      {
        /* Allocate GPU memory for all memrecs */
        status = TIDL_cudaMemManagerAllocate((TIDL_CudaMemManager*)algHandle->cudaMemManager);
        
        if(status == IALG_EOK)
        {
          /* Copy persistent data to GPU */
          status = TIDL_cudaMemManagerCopyPersistentH2D((TIDL_CudaMemManager*)algHandle->cudaMemManager);
          
          if(status == IALG_EOK)
          {
            tidl_printf(1, "CUDA Memory Manager initialized successfully\n");
            
            /* Mark which layers have GPU support - this can be populated based on layer type */
            /* For now, we'll mark this during layer execution based on CUDA kernel availability */
          }
          else
          {
            tidl_printf(0, "Failed to copy persistent data to GPU\n");
          }
        }
        else
        {
          tidl_printf(0, "Failed to allocate GPU memory\n");
        }
      }
      else
      {
        tidl_printf(0, "Failed to initialize CUDA Memory Manager\n");
      }
      
      /* If any step failed, cleanup */
      if(status != IALG_EOK && algHandle->cudaMemManager != NULL)
      {
        free(algHandle->cudaMemManager);
        algHandle->cudaMemManager = NULL;
      }
    }
  }
  #endif

  TIDL_L1DandL2CacheWbInv();
}
#if TIDL_DEVICE_MULTICORE
else
{
  status = IALG_EOK; // flow came here due to TIDL_ERROR_ALG_COMMON_MULTICORE_INIT_CHECK, its not a error, so reset status to EOK
}
#endif
  return (status);
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_control
@brief      Function to accept any run time updates with recreation of object
            Refer algControl function for details in ivision.h file
@param      handle   : Algorithm Instance handle
@param      cmd      : Command to be processed
@param      inParams : Input Parameters
@param      outParams: Output Parameters
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t TIDL_control.* <function end>
<justification start>
Rationale - NOT_IN_SCOPE: This code is not in scope for single core SOC
Effect on this UNIT - Code prevents the software executing code which is not in scope and maintain system predictability and integrity.
<justification end> */
int32_t TIDL_control(IVISION_Handle Handle,
                     IALG_Cmd cmd,
                     const IALG_Params *inParams,
                     IALG_Params *outParams)
{
  int32_t status = IALG_EOK;

  tidl_printf(1, "TIDL control call ...\n");

  TIDL_Handle algHandle1 = NULL;
  if (Handle != NULL)
  {
    algHandle1 = (TIDL_Handle)(void *)Handle;
  }

  TIDL_Handle algHandle = algHandle1;
  if (algHandle != NULL)
  {
    switch (cmd)
    {
    case TIVX_TIDL_CMD_GET_C7X_PTRS:
    {
      if (outParams == NULL)
      {
        status = IALG_EFAIL;
      }
      else
      {
        TIDL_controlGetArgs *outArgs = (TIDL_controlGetArgs *)outParams;

        outArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_L1] = (void *)algHandle->memRec[ALG_SCRATCH_L1_MEM_MEMREC].base;
        outArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_L2] = (void *)algHandle->memRec[ALG_SCRATCH_L2_MEM_MEMREC].base;
        outArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_MSMC] = (void *)algHandle->memRec[ALG_SCRATCH_L3_MEM_MEMREC].base;
        outArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_DDR] = (void *)algHandle->memRec[ALG_SCRATCH_DATA_BUFF_EXT_MEMREC].base;
        outArgs->ctrlGetArgs.ncScratchPtr[TIDL_Virtual][TIDL_DDR_PERSIST] = (void *)algHandle->memRec[ALG_PERSIST_DDR_INIT_DATA].base;
        outArgs->ctrlGetArgs.syncBufferPtr[TIDL_Virtual] = (void *)algHandle->memRec[ALG_PERSIST_SYNC_MEMREC].base;
      }
      break;
    }
    case TIVX_TIDL_CMD_SET_C7X_PTRS:
    {
      if (inParams == NULL)
      {
        status = IALG_EFAIL;
      }
      else
      {
        TIDL_controlSetArgs *inArgs = (TIDL_controlSetArgs *)inParams;
        (void)memcpy((void *)(&algHandle->controlArgs), (void *)inArgs, sizeof(TIDL_controlSetArgs));
      }

      break;
    }
    case TIVX_TIDL_CMD_INIT:
    {
      status = TIDL_init((IALG_Handle)Handle, NULL, NULL, NULL);
      break;
    }
    case TIVX_TIDL_CODE_COVERAGE_UPLOAD:
    {
#ifdef CODE_COVERAGE_ENABLED_FOR_TIDL
      upload_execution_history();
#endif
      break;
    }
    default:
    {
      tidl_printf(0, "Invalid Node Cmd Id\n");
      status = IALG_EFAIL;
      break;
    }
    }
  }
  else
  {
    tidl_printf(0, "Alg handle is null \n");
    status = TIDL_ERROR_ALG_COMMON_NULL_ALGHNDLE_ERROR;
  }
  return (status);
}
/* Disabling INTERNAL_PROFILING by default*/
/* #define INTERNAL_PROFILING */
#ifdef INTERNAL_PROFILING
typedef struct
{
  char fmt[20];
  char fmtVal[20];
  char string[64];
  void *pValue;
} sTIDLProfilePrintInfo_t;

static TIDL_LayerMetaData gLayerMetaData = {0};

static sTIDLProfilePrintInfo_t gProfilePrintInfo[] =
    {
        {"%6s,", "%6d,", "Layer", NULL},
        {"%6s,", "%6d,", "LayerType", &gLayerMetaData.layerType},
        {"%15s,", "%15llu,", "Layer Cycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_LAYER]},
        {"%15s,", "%15llu,", "kernelOnlyCycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_KERNEL_ONLY]},
        {"%15s,", "%15llu,", "coreLoopCycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_CORE_LOOP]},
        {"%15s,", "%15llu,", "LayerSetupCycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_LAYER_SETUP]},
        {"%15s,", "%15llu,", "dmaPipeupCycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_DMA_PIPEUP]},
        {"%15s,", "%15llu,", "LayerHandleCopy", &gLayerMetaData.profilePoint[TIDL_PROFILE_LAYER_SETUP_HANDLE_COPY]},
        {"%15s,", "%15llu,", "RestoreCycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_RESTORE]},
        {"%15s,", "%15llu,", "Multic7xContextCopyCycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_CONTEXT_CROSSCORE_COPY]},
        {"%15s,", "%15llu,", "DDRBWReadInBytes", &gLayerMetaData.profilePoint[TIDL_PROFILE_DDR_BW_READ]},
        {"%15s,", "%15llu,", "DDRBWWriteInBytes", &gLayerMetaData.profilePoint[TIDL_PROFILE_DDR_BW_WRITE]},
        {"%15s,", "%15llu,", "ShapeInferCycles", &gLayerMetaData.profilePoint[TIDL_PROFILE_SHAPE_INFER]}};

void TIDL_printProfileData(
    int32_t numLayers,
    uint64_t tStartNetwork, uint64_t tEndNetwork, TIDL_outArgs *tidlOutArgs)
{
  int32_t numProfilePoints, i, j;
  uint64_t sumOfLayerCycles = 0;

  for (i = 0; i < numLayers; i++)
  {
    if (i == 0)
    {
      uint64_t netWorkCycles;
      netWorkCycles = (uint64_t)(tEndNetwork - tStartNetwork);
      numProfilePoints = (int32_t)(sizeof(gProfilePrintInfo) / sizeof(sTIDLProfilePrintInfo_t));
      tidl_printf(0, "\n");
      tidl_printf(0, "Network Cycles %llu \n", netWorkCycles);
      for (j = 0; j < numProfilePoints; j++)
      {
        tidl_printf(0, gProfilePrintInfo[j].fmt, gProfilePrintInfo[j].string);
      }
      tidl_printf(0, "\n");
    }

    gLayerMetaData = tidlOutArgs->metaDataLayer[i];
    gProfilePrintInfo[0].pValue = &tidlOutArgs->metaDataLayer[i].layerExecId;

    if (tidlOutArgs->metaDataLayer[i].layerExecId != -1) /* Print for layers other than data layers */
    {
      for (j = 0; j < numProfilePoints; j++)
      {
        tidl_printf(0, gProfilePrintInfo[j].fmtVal, *((uint32_t *)gProfilePrintInfo[j].pValue));
      }
      tidl_printf(0, "\n");
    }
    sumOfLayerCycles += gLayerMetaData.profilePoint[TIDL_PROFILE_LAYER];
  }

  tidl_printf(0, " Sum of Layer Cycles %llu \n", sumOfLayerCycles);
}

#endif

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_process
@brief      This is the main processing function and should be called
            for each frame.
            Refer process function for details in ivision.h file

@param      handle : Algorithm Instance handle
@param      inBufs : Input Buffer Pointers descriptor
@param      outBufs: Output Buffer Pointers descriptor
@param      inArgs : Input Arguments
@param      outArgs: Output Arguments
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/

int32_t gCurrLayerNumber;
int32_t gLayerNumber = -1;
/* CHECK_MISRA("-16.7")  -> Disable rule 16.7
Pointer parameter inBufs and outBufs are not used to modify the addressed
object but is not declared as a pointer to const
Deviation is allowed because this function is of type IVISION_Fxns :
algProcess in order to be ASSIGNED IVISION_Fxns */
int32_t TIDL_process(IVISION_Handle Handle,
                     IVISION_InBufs *inBufs,
                     IVISION_OutBufs *outBufs,
                     IVISION_InArgs *inArgs,
                     IVISION_OutArgs *outArgs)
{
  /*RESET_MISRA("16.7")  -> Reset rule 16.7  */
  TIDL_Handle algHandle = (TIDL_Handle)(void *)(Handle);
  tidl_printf(1, "TIDL_process is started with handle : %p \n", algHandle);

#ifdef BUILD_WITH_CUDA_MEM_MANAGER
  /* Set thread-local CUDA memory manager for this instance */
  if(algHandle->cudaMemManager != NULL)
  {
    TIDL_cudaSetThreadManager((TIDL_CudaMemManager*)algHandle->cudaMemManager);
  }
#endif

  int32_t oldIntState = 0;
  int32_t lockState = UNLOCKED;

/*****  Critical section start - Avoid writing any other code before this call ******/
#if ENABLE_PREEMPTION
  /* This call enables interrupt lock at beginning of critical section */
  TIDL_giveChanceForPreemption(algHandle, &lockState, &oldIntState);
#endif
  /**************************************************************************************/

  uint32_t coreId = algHandle->createParams->coreId;
  int32_t relativeCoreId = GET_RELATIVE_COREIDX(coreId, algHandle->createParams->coreStartIdx);
  TIDL_Handle intAlgHandle = NULL;
  int32_t twStatus = IALG_EOK;
  int32_t status = IALG_EOK;
  TIDL_InArgs *tidlInArgs = (TIDL_InArgs *)(void *)inArgs;
  TIDL_outArgs *tidlOutArgs = (TIDL_outArgs *)(void *)outArgs;

  int32_t currAlgLayer = 0;
  int32_t i;
  TIDL_NetworkCommonParams commonParams;
  WorkloadUnitExec_CommonParams tidlCommonParams;
  // WorkloadUnitExec_RefParams  wlRefParams;
  uint64_t tStart, tEnd;
  uint64_t tStartNetwork, tEndNetwork;
  uint64_t tStartShapeInfer, tEndShapeInfer;
  uint64_t read_bytes_end, write_bytes_end;
  int32_t isLayerLevelPreemptionAllowed = 0;
#if TIDL_DEVICE_MULTICORE
  int32_t numCores = 0;
#endif

  TIDL_getTscTime(&tStartNetwork);

  status = TIDL_initDebugTraceParams(algHandle->createParams->traceLogLevel,
                                     algHandle->createParams->traceWriteLevel,
                                     algHandle->createParams->TIDLVprintf,
                                     algHandle->createParams->TIDLWriteBinToFile,
                                     algHandle->createParams->TIDLReadBinFromFile,
                                     algHandle->createParams->traceBaseName);

  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start>
  Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
  TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_002
  Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
  This does not affect runtime behavior or safety.
  <justification end> */
  if (status != IALG_EOK)
  {
    TIDL_LOG_ERROR(TIDL_ERROR_GROUP_DEBUG_TRACE, TIDL_ERROR_DEBUG_TRACE_INVALID_PARAM);
    status = IALG_EFAIL;
  }
  /* LDRA_JUSTIFY_END */

  /*-----------------------------------------------------------------------*/
  /* Activate the algorithm to make sure that now onwards internal memory  */
  /* handle can be utilized                                                */
  /*-----------------------------------------------------------------------*/
  if (status == IALG_EOK)
  {
    status = TIDL_switchHandles((IALG_Handle)(void *)Handle);

    if (status == IALG_EOK)
    {
      intAlgHandle = (TIDL_Handle)(void *)algHandle->intAlgHandle;

      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>
      Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_004
      Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
      This does not affect runtime behavior or safety.
      <justification end> */
      if (intAlgHandle != NULL)
      /* LDRA_JUSTIFY_END */
      {
        status = IALG_EOK;
      }
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start>
      Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_004
      Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
      This does not affect runtime behavior or safety.
      <justification end> */
      else
      {
        /* TIDL_LDRA_TAG_ALG_PRIOR_CHECK_003 */
        status = IALG_EFAIL;
      }
      /* LDRA_JUSTIFY_END */
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start>
      Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_004
      Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
      This does not affect runtime behavior or safety.
      <justification end> */
      if (status == IALG_EOK)
      {
        /* LDRA_JUSTIFY_END */
        intAlgHandle->isPadDmaPending = 0;

/*Scale factors should never be updated for TIDL_QuantStyleAsymNP2_TFL*/
#ifdef HOST_EMULATION
        if (((uint32_t)intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == 0U)
#endif
        {
          if ((intAlgHandle->createParams->net->quantizationStyle == TIDL_QuantStyleNP2Fixed) ||
              (intAlgHandle->createParams->net->quantizationStyle == TIDL_QuantStyleP2Dynamic))
          {
            for (i = 0; i < intAlgHandle->createParams->net->numLayers; i++)
            {
#ifdef TIDL_COVERAGE_DEAD_CODE
              if ((intAlgHandle->createParams->quantRangeUpdateFactor != 0.0f) &&
                  (intAlgHandle->createParams->net->TIDLLayers[i].layersGroupId ==
                   intAlgHandle->createParams->currLayersGroupId))
              {
                TIDL_UpdateScaleFactors(intAlgHandle, i, 0, 0, 0);
              }
#endif
            }
          }
        }

        for (i = 0; i < (int32_t)inBufs->numBufs; i++)
        {
          if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_NUMLAYERS_MAX)
          {
            /* Forcefully making numLayers as MAX(1536) to coverage negative case */
            intAlgHandle->createParams->net->numLayers = TIDL_NUM_MAX_LAYERS;
          }
          if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_NUMLAYERS_MIN)
          {
            /* Forcefully making numLayers as -1 to coverage negative case */
            intAlgHandle->createParams->net->numLayers = -1;
          }

          int32_t id = TIDL_getLayerIDForOutDataID(intAlgHandle->createParams->net, inBufs->bufDesc[i]->bufferId);
          if (id == -1)
          {
            status = IALG_EFAIL;
            break;
          }

// Remove this deadcode one dynamic input is enabled
#if defined TIDL_DYNAMIC_SHAPE

           /*
           * If input data layer is dynamic in shape, parse the shape from
           * tidlInArgs
           */
          sTIDL_Layer_t *layer = &intAlgHandle->createParams->net->TIDLLayers[id];

          if (layer->layerType == TIDL_DataLayer && 0U != layer->outData.dynDimMask)
          {
            layer->outData.dimValues[TIDL_DIM_BATCH] = tidlInArgs->inDimValues[i][TIDL_DIM_BATCH];
            layer->outData.dimValues[TIDL_DIM_DIM1] = tidlInArgs->inDimValues[i][TIDL_DIM_DIM1];
            layer->outData.dimValues[TIDL_DIM_DIM2] = tidlInArgs->inDimValues[i][TIDL_DIM_DIM2];
            layer->outData.dimValues[TIDL_DIM_NUMCH] = tidlInArgs->inDimValues[i][TIDL_DIM_NUMCH];
            layer->outData.dimValues[TIDL_DIM_HEIGHT] = tidlInArgs->inDimValues[i][TIDL_DIM_HEIGHT];
            layer->outData.dimValues[TIDL_DIM_WIDTH] = tidlInArgs->inDimValues[i][TIDL_DIM_WIDTH];

            /*
             * TODO: The above logic might not be correct for pitch calculation
             * since NC might explicitly set different pitch especially for 
             * channel. This needs to be handled.
             */
            layer->outData.pitch[TIDL_LINE_PITCH] = layer->outData.dimValues[TIDL_DIM_WIDTH] + layer->outData.padW;
            int32_t isPadW = (layer->outData.padW != 0) ? 1 : 0;
            layer->outData.pitch[TIDL_CHANNEL_PITCH] = (layer->outData.dimValues[TIDL_DIM_HEIGHT] + (2 * layer->outData.padH) + isPadW) * layer->outData.pitch[TIDL_LINE_PITCH];
            layer->outData.pitch[TIDL_DIM2_PITCH] = (layer->outData.dimValues[TIDL_DIM_NUMCH] * layer->outData.pitch[TIDL_CHANNEL_PITCH]);
            layer->outData.pitch[TIDL_DIM1_PITCH]= (layer->outData.dimValues[TIDL_DIM_DIM2] * layer->outData.pitch[TIDL_DIM2_PITCH]);
            layer->outData.pitch[TIDL_ROI_PITCH]= (layer->outData.dimValues[TIDL_DIM_DIM1] * layer->outData.pitch[TIDL_DIM1_PITCH]);
            
            /*
             * TODO: Handle case where data layer is connected to dataconvert
             * layer with NHWC format.
             */
          }
#endif

#if defined TIDL_COVERAGE_DEAD_CODE
          if (intAlgHandle->createParams->net->TIDLLayers[id].layerType != TIDL_DataLayer)
          {
            sTIDL_DataParams_t *inDataParams = TIDL_getDataParams(intAlgHandle->createParams->net, inBufs->bufDesc[i]->bufferId);
            inDataParams->tensorScale = tidlInArgs->scale[i];
          }
#endif
        }
      }
      if (status == IALG_EOK)
      {
#if ENABLE_PREEMPTION
        if (intAlgHandle->isPreEmptionEnable != 0)
        {
          status = TIDL_updateContextMemPtrsForPreemption(&intAlgHandle->preEmptContextInfo, intAlgHandle->gcHelperHandle,
                                                          intAlgHandle->privContext,
                                                          intAlgHandle->preEmptHandle, intAlgHandle->perfSimOutput, intAlgHandle->sysScratchPtr.l2BasePtr[relativeCoreId],
                                                          intAlgHandle->sysScratchPtr.l3BasePtr[relativeCoreId], 0, intAlgHandle->createParams->net->numLayers);
        }
#endif
        sGetLayerIdContext_t getLayerIdContext = {0};
        sWorkloadUnit_t *workloadUnit = NULL;
        const sTensorPosition_t *tensorPosition = NULL;
        const sTensorDims_t *subTensorDims = NULL;
        const sTensorDims_t *fullTensorDims = NULL;
        int32_t layerId;
        int32_t wlRepeatIter;

        getLayerIdInit(&getLayerIdContext,
                       relativeCoreId,
                       intAlgHandle->createParams->currLayersGroupId,
                       intAlgHandle->createParams->net,
                       intAlgHandle->gcHelperHandle,
                       intAlgHandle->wlSuperGrp);

        tidlCommonParams.dmaChannelAllocContext = &intAlgHandle->dmaChannelAllocContext;
        tidlCommonParams.dmaUtilsContext = intAlgHandle->dmaUtilsContext;
        tidlCommonParams.sysMems = &intAlgHandle->sysMems[0];
        tidlCommonParams.sysScratchPtr = &intAlgHandle->sysScratchPtr;
        tidlCommonParams.zeroVector1k = intAlgHandle->zeroVector1k;
        tidlCommonParams.forceNegativeTest = intAlgHandle->createParams->forceNegativeTest;

        commonParams.createParams = intAlgHandle->createParams;
        commonParams.gcHelperHandle = (const sGCHelperHandle *)intAlgHandle->gcHelperHandle;
        commonParams.net = intAlgHandle->createParams->net;
        commonParams.TIDLLayersBufPtr = intAlgHandle->TIDLLayersBuf; //: TODO: Check if this needs to be a global variable
        commonParams.tidlCommonParams = &tidlCommonParams;

        int32_t layer, numProfile;
        if (tidlInArgs->enableLayerPerfTraces == 1)
        {
          for (layer = 0; layer < intAlgHandle->createParams->net->numLayers; layer++)
          {
            for (numProfile = 0; numProfile < TIDL_PROFILE_MAX; numProfile++)
            {
              tidlOutArgs->metaDataLayer[layer].profilePoint[numProfile] = 0;
            }
            tidlOutArgs->metaDataLayer[layer].layerExecId = NOT_VALID;
  #if defined(SOC_TDA54)
          tidlOutArgs->metaDataLayer[layer].ddrBandwidthRead = 0;
          tidlOutArgs->metaDataLayer[layer].ddrBandwidthWrite = 0;
#endif
        }
        }
#if TIDL_DEVICE_MULTICORE
        TIDL_controlSetArgs *args = (TIDL_controlSetArgs *)(&intAlgHandle->controlArgs);

        /* Reset the sync buffer to -1 at the start of process call */
        if ((algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency))
        {
          sGraphCompilerOutArgsCore_t *gcOutCore = NULL;
          sGraphCompilerOutArgs_t *gcOutArgs = (sGraphCompilerOutArgs_t *)get_int8_t_pointer((int8_t *)(algHandle->createParams->net),
                                                                                             algHandle->createParams->net->netBufInfo[TIDL_NET_GC_BUF].offset);
          gcOutCore = (sGraphCompilerOutArgsCore_t *)getGCDataBase(gcOutArgs, DATABASE_GCOUT_1); /* Used to get WL count - always read from core 0 since it has maximum number of layers */
          volatile int8_t *syncPtr = (int8_t *)args->ctrlSetArgs[relativeCoreId].syncBufferPtr[TIDL_Virtual] + (int8_t)TIDL_GLOBAL_SYNC_OFFSET;
          /* Set all workloads' states to default - core 0 has max. number of workloads, so take number of workloads from core 0 db info */
          (void)memset((void *)syncPtr, TIDL_SYNC_WL_STATE_DEFAULT, gcOutCore->dataBaseInfo.wlDB.count * (int64_t)sizeof(int8_t));
          TIDL_syncWl(args->ctrlSetArgs, {}, TIDL_SYNC_POINT_GLOBAL_SYNC, 0,
                      relativeCoreId, algHandle->createParams->net->numCores);
          if (algHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_PROCESS_FORCE_SYNC_INVALID)
          {
            /* Forcefully making syncPoint NOT_VALID to cover negative case */
            TIDL_syncWl(args->ctrlSetArgs, {}, NOT_VALID, 0,
                        relativeCoreId, algHandle->createParams->net->numCores);
          }
        }
#endif
#ifdef HOST_EMULATION
        int32_t prevDataIdExecuted = NOT_VALID;
#endif
#if ENABLE_PREEMPTION
        /* Give chance for pre-emption before the while loop for layer level processing starts */
        TIDL_giveChanceForPreemption(algHandle, &lockState, &oldIntState);
#endif

        int32_t flag = 1;
        if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_PROCESS_FORCE_WHILEFLAG_ZERO)
        {
          /* TODO needs to relook */
          flag = 0;
          status = IALG_EFAIL;
        }
        while (flag == 1)
        {
          sTIDL_AlgLayer_t *algLayer;
          sTIDL_Layer_t *tidlLayer;
          void *inPtrs[TIDL_NUM_IN_BUFS] = {0};
          void *outPtrs[TIDL_NUM_OUT_BUFS] = {0};

          uint32_t val0 = 0, val1 = 0, val2 = 0, val3 = 0;
          tidlPerfStatsDdrStatsReadCounters(&val0, &val1, &val2, &val3, false);

          /* Should probably keep this in context mem to prevent overwrite during preemption */
          layerId = getLayerIdToExecute(&getLayerIdContext,
                                        0,
                                        &wlRepeatIter,
                                        &currAlgLayer,
                                        &workloadUnit);

        tidl_printf(2, "Coreid %d Layerid to execute = %d \n", relativeCoreId, layerId);
        if ( layerId == NOT_VALID)
        {
          break;
        }
        #ifdef HOST_EMULATION
        #if LAYER_TO_RUN != NOT_VALID
        if (layerId != LAYER_TO_RUN)
          continue;
        #endif

        if ( (intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
        {
          /* If previous and current layer id is same then skip execution of the workload in reference flow. This is because
            in reference flow we will be calling the complete layer during first workload processing itself*/
          if (((intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_TransposeLayer) ||
               (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_GatherNDLayer)  ||
               (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_LSTMLayer)  ||
               (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_GRULayer)  ||
               (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_RNNLayer)
              ) && ( prevDataIdExecuted == layerId )
            )
          {
            #if TIDL_DEVICE_MULTICORE
            if(algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
            {
              numCores = algHandle->createParams->net->numCores;
              /* Update current workload completion in the state array */
              TIDL_syncWl(args->ctrlSetArgs, {},TIDL_SYNC_POINT_COMPLETE, workloadUnit->wlExecId, relativeCoreId, numCores);
            }
            #endif
            continue;
          }
          if ( ((intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_ConvolutionLayer) || (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_Deconv2DLayer))  && (prevDataIdExecuted == layerId) && (intAlgHandle->createParams->net->TIDLLayers[layerId].layerParams.convParams.numGroups>1) )
          {
            #if TIDL_DEVICE_MULTICORE
            if(algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
            {
              numCores = algHandle->createParams->net->numCores;
              /* Update current workload completion in the state array */
              TIDL_syncWl(args->ctrlSetArgs, {},TIDL_SYNC_POINT_COMPLETE, workloadUnit->wlExecId, relativeCoreId, numCores);
            }
            #endif
            continue;
          }
        }
        #endif

#if TIDL_DEVICE_MULTICORE
          if (algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
          {
            numCores = algHandle->createParams->net->numCores;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if (workloadUnit != NULL)
            {
              /* LDRA_JUSTIFY_END */
              TIDL_syncWl(args->ctrlSetArgs, workloadUnit->dependId, TIDL_SYNC_POINT_START, workloadUnit->wlExecId, relativeCoreId, numCores);
            }
          }
#endif

          tidl_printf(1, "Core %d Alg Process for Layer # - %4d, layer type %d\n", relativeCoreId, layerId, intAlgHandle->createParams->net->TIDLLayers[layerId].layerType);
#ifdef HOST_EMULATION
          //: TODO: Eventually this should be removed once process also  uses the same loop structure
          if (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_DataLayer)
          {
            continue;
          }
#endif
          TIDL_getTscTime(&tStart); /* Start layer cycles count after Sync is done and layer execution starts*/

          /*  Critical section starts here */
          tidlLayer = &intAlgHandle->createParams->net->TIDLLayers[layerId];
          uint32_t targetDevice = intAlgHandle->createParams->net->deviceName;
          TIDL_forceTargetDeviceNonOTF(&targetDevice, tidlLayer->layerType, -1);

          /* Layer specific internal parameters */
          algLayer = get_AlgLayer_t_pointer(algHandle->alglayerParams, currAlgLayer);
          intAlgHandle->alglayerParams[currAlgLayer].metaData.layerExecId = algLayer->layerIdx;
          intAlgHandle->currAlgLayer = currAlgLayer;
          
          int32_t isRestoreHeightsRequired = 0;

          if(tidlLayer->layerType != TIDL_ConstDataLayer)
          {
            /* TIDL_LDRA_TAG_CONSTDATA_PRIOR_CHECK_001 */
            /* Reset DMA resources and scratch memory resources for every iteration */
            (void)TIDL_ResetDmaChannel(&intAlgHandle->dmaChannelAllocContext);

            TIDL_resetSysmem(intAlgHandle->sysMems);

            int32_t isRefExecFlowEnabled = WorkloadRefExec_enableRefExecFlow(intAlgHandle->createParams->flowCtrl, tidlLayer, algLayer);
            WorkloadRefExec_RefExecParams wlRefParams;

            /* Handle dynamic shapes */
            TIDL_getTscTime(&tStartShapeInfer);

#if defined TIDL_DYNAMIC_SHAPE
            if (0U != tidlLayer->outData.dynDimMask)
            {
              status = TIDL_resolveLayerOutputShape(inBufs,
                                                    outBufs,
                                                    intAlgHandle,
                                                    algLayer,
                                                    tidlLayer);
              tidl_printf(1, "Core %d Shape Inference for Layer # - %4d, outShape = [%d,%d,%d,%d,%d,%d] \n",
                    relativeCoreId,
                    layerId,
                    tidlLayer->outData.dimValues[TIDL_DIM_BATCH],
                    tidlLayer->outData.dimValues[TIDL_DIM_DIM1],
                    tidlLayer->outData.dimValues[TIDL_DIM_DIM2],
                    tidlLayer->outData.dimValues[TIDL_DIM_NUMCH],
                    tidlLayer->outData.dimValues[TIDL_DIM_HEIGHT],
                    tidlLayer->outData.dimValues[TIDL_DIM_WIDTH]);
            }
#endif

            TIDL_getTscTime(&tEndShapeInfer);

            TIDL_updateprofileData(intAlgHandle->alglayerParams[currAlgLayer].metaData.profilePoint, TIDL_PROFILE_SHAPE_INFER, tStartShapeInfer, tEndShapeInfer);

#if defined TIDL_DYNAMIC_SHAPE
            if (status == IALG_EOK)
#endif
            {
              /* Back up for original numBatches of inLayer of currLayer - will be used to write traces for Data layer */
              int32_t orgNumBatches[TIDL_NUM_IN_BUFS] = {0};
              for (int32_t inBufIdx = 0; inBufIdx < tidlLayer->numInBufs; inBufIdx++)
              {
                sTIDL_Layer_t *inTidlLayer = &intAlgHandle->createParams->net->TIDLLayers[(int32_t)algLayer->inLayerIdx[inBufIdx]];
                orgNumBatches[inBufIdx] = inTidlLayer->outData.dimValues[TIDL_DIM_BATCH];
              }
              /*Inputs of GatherND can have different number of dimensions and batches. We need to avoid call this function for gatherND*/
              if (isRefExecFlowEnabled == 1 && tidlLayer->layerType != TIDL_GatherNDLayer && tidlLayer->layerType != TIDL_GatherLayer)
              {
                TIDL_privSetTensorDimensions(intAlgHandle->createParams, intAlgHandle->alglayerParams, algLayer, workloadUnit,
                                            wlRefParams.origInHeight, wlRefParams.origInCh, &wlRefParams.origGrp, wlRefParams.origInChPitch, wlRefParams.origInRoiPitch, wlRefParams.origInBatch,
                                            &wlRefParams.origOutHeight, &wlRefParams.origOutCh, &wlRefParams.origOutChPitch, &wlRefParams.origOutRoiPitch, &wlRefParams.origOutBatch);
                isRestoreHeightsRequired = 1;
              }

              status = TIDL_getLayerInPtrs(inBufs, outBufs, intAlgHandle, algLayer, tidlLayer, inPtrs, layerId,
                                          orgNumBatches);

              #ifdef HOST_EMULATION
              #if LAYER_TO_RUN != NOT_VALID
              for (int i=0; i < tidlLayer->numInBufs; i++)
              {
                FILE *fp = NULL;
                switch (i)
                {
                  case 0:
                    fp = fopen(LAYER_TO_RUN_INPUT_0, "r");
                    break;
                  case 1:
                    fp = fopen(LAYER_TO_RUN_INPUT_1, "r");
                    break;
                  case 2:
                    fp = fopen(LAYER_TO_RUN_INPUT_2, "r");
                    break;
                  default:
                    break;
                }
                if (fp != NULL)
                {
                  switch (i)
                  {
                    case 0:
                      fread(inPtrs[i], LAYER_TO_RUN_SIZE_0, 1, fp);
                      break;
                    case 1:
                      fread(inPtrs[i], LAYER_TO_RUN_SIZE_1, 1, fp);
                      break;
                    case 2:
                      fread(inPtrs[i], LAYER_TO_RUN_SIZE_2, 1, fp);
                      break;
                    default:
                      break;
                  }
                  fclose(fp);
                }
              }
              #endif
              #endif
          }
          if(status == IALG_EOK)
          {
            if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_OUTPTRS_NULL) /* forcing invalid dataId to check the negative case of TIDL_E_INVALID_IO_LINE_PITCH */
            {
              tidlLayer->outData.dataId = -1;
            }
            if (intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_UTILS_FORCE_OUTPRTS_INVALID_DATATYPE) /* forcing dataFlowInfo = 0 to enter the negative case flow */
            {
              intAlgHandle->createParams->net->dataFlowInfo = 0;
            }
            status = TIDL_getLayerOutPtrs(outBufs, intAlgHandle, algLayer, tidlLayer, outPtrs, layerId);
          }

            if (status == IALG_EOK)
            {
              tidl_printf(1, "Processing Layer # - %4d\n", layerId);

              algLayer->wlRepeatIter = wlRepeatIter;

              // TODO: Change this for ST
              algLayer->procType = TIDL_getAlgProcType(workloadUnit);
              algLayer->memcpyTr = intAlgHandle->memcpyTr;

              #ifdef BUILD_WITH_CUDA
              TIDL_cudaSetThreadLayerIdx(layerId);
              #endif

              status = WorkloadRefExec_Process(intAlgHandle, &commonParams, workloadUnit, algLayer, tidlLayer, inPtrs, outPtrs, layerId, relativeCoreId);

            if(status == IALG_EOK)
            {
              if (intAlgHandle->createParams->traceWriteLevel > 0)
              {
                /* LDRA_JUSTIFY_START
                <metric start> branch <metric end>
                <justification start>
                Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
                This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
                TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_001
                Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
                This does not affect runtime behavior or safety.
                <justification end> */                
                if (algLayer->gcHelperHandle != NULL)
                /* LDRA_JUSTIFY_END */                
                {
                  /* Need to find a better way to load this */
                  // sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) algLayer->gcHelperHandle->dbPayLoad[algLayer->gcHelperHandle->dbParams[DB_BUF].size +
                  //                                                             algLayer->gcHelperHandle->dbParams[DB_WL].size];
                  sWorkloadUnitAuxilary_t* auxilaryWorkloadDB = (sWorkloadUnitAuxilary_t*) algLayer->gcHelperHandle->pAuxWLDB->pWLAuxList;
                  subTensorDims     = &auxilaryWorkloadDB[currAlgLayer].outSubTensorDims;
                  #ifdef HOST_EMULATION
                  if ( (intAlgHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
                  {
                    // Setting Full tesnsor dims for layers in which all WLs are processed in one shot for ref flow
                    if ((intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_TransposeLayer) || (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_GatherNDLayer))
                    {
                      subTensorDims     = &auxilaryWorkloadDB[currAlgLayer].outFullTensorDims;
                    }
                  }
                  #endif
                  fullTensorDims    = &auxilaryWorkloadDB[currAlgLayer].outFullTensorDims;
                  tensorPosition = &auxilaryWorkloadDB[currAlgLayer].outSubTensorPosition;
                }
                #if TIDL_DEVICE_MULTICORE
                if((isRefExecFlowEnabled == 0) && (intAlgHandle->createParams->net->inferenceMode  == (int32_t)TIDL_inferenceModeLowLatency))
                {
                  TIDL_privSetTensorDimensions(intAlgHandle->createParams, intAlgHandle->alglayerParams ,algLayer, workloadUnit,
                                              wlRefParams.origInHeight, wlRefParams.origInCh, &wlRefParams.origGrp, wlRefParams.origInChPitch, wlRefParams.origInRoiPitch, wlRefParams.origInBatch,
                                              &wlRefParams.origOutHeight, &wlRefParams.origOutCh, &wlRefParams.origOutChPitch, &wlRefParams.origOutRoiPitch, &wlRefParams.origOutBatch);
                  isRestoreHeightsRequired = 1;
                }
                #endif
                if((isRefExecFlowEnabled == 1) && (isRestoreHeightsRequired == 1))
                {
                  /* moved inferenceMode check under device specific macro to pass SAFETY case*/
                  #if TIDL_DEVICE_MULTICORE
                  /* Temporary fix for supertiling ref flow traces. Needs update for supertiling with low latency */
                  /* LDRA_JUSTIFY_START
                  <metric start> branch <metric end>
                  <justification start>
                  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
                  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
                  <justification end> */
                  if((intAlgHandle->createParams->net->inferenceMode  != (int32_t)TIDL_inferenceModeLowLatency))
                  /* LDRA_JUSTIFY_END */
                  #endif
                  {
                    TIDL_privRestoreTensorDimensions(intAlgHandle->createParams, algLayer, wlRefParams.origInHeight, wlRefParams.origInCh, &wlRefParams.origGrp, wlRefParams.origInChPitch, wlRefParams.origInRoiPitch, wlRefParams.origInBatch,
                                                        &wlRefParams.origOutHeight, &wlRefParams.origOutCh, &wlRefParams.origOutChPitch, &wlRefParams.origOutRoiPitch, &wlRefParams.origOutBatch);
                  }
                }
                if(intAlgHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_WORKLOAD_REF_EXEC_TENSORPOSITION_NULL)
                {
                  tidl_setWriteLevel(5);      /* Force write level to 5 for negative test */
                }
                twStatus = WorkloadRefExec_writeTraceDataBuf(outPtrs[0], intAlgHandle, tidlLayer, algLayer, tensorPosition, subTensorDims, fullTensorDims);
              }

                if (twStatus != IALG_EOK)
                {
                  tidl_printf(0, "TIDL_process : Trace write failed\n");
                  status = IALG_EFAIL; // Updating status as EFAIL for trace write failure
                }
                if (status == IALG_EOK)
                {
                  status = tidl_writeTraceParamBuf(intAlgHandle->createParams->net,
                                                   algLayer,
                                                   tidlLayer,
                                                   tidlLayer->outData.dataId,
                                                   (int8_t *)intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].base,
                                                   intAlgHandle->memRec[ALG_REF_OUTPUT_BUFF_MEMREC].size);
                }
              }
            }
            if (status == IALG_EOK)
            {
              if (isRestoreHeightsRequired == 1)
              {
                TIDL_privRestoreTensorDimensions(intAlgHandle->createParams, algLayer, wlRefParams.origInHeight, wlRefParams.origInCh, &wlRefParams.origGrp, wlRefParams.origInChPitch, wlRefParams.origInRoiPitch, wlRefParams.origInBatch,
                                                 &wlRefParams.origOutHeight, &wlRefParams.origOutCh, &wlRefParams.origOutChPitch, &wlRefParams.origOutRoiPitch, &wlRefParams.origOutBatch);
              }

              //: TODO: Need to handle copying persistent weights when group changes
              intAlgHandle->groupId = getLayerIdContext.groupIdx;

              TIDL_getTscTime(&tEnd);

              TIDL_updateprofileData(intAlgHandle->alglayerParams[currAlgLayer].metaData.profilePoint, TIDL_PROFILE_LAYER, tStart, tEnd);

              tidl_printf(1, "Core %d End of Layer # - %4d with outPtrs[0] = %p\n", coreId, layerId, (int8_t *)outPtrs[0]);
#if ENABLE_PREEMPTION
              if (intAlgHandle->isPreEmptionEnable != 0)
              {
                TIDL_updateLayerCyclesForPreemption((void *)intAlgHandle->preEmptHandle, intAlgHandle->preemptPerfHW, (tEnd - tStart),
                                                    algLayer->execLayerNum, algLayer->procType,
                                                    intAlgHandle->perfSimOutput->simConfig.freqMHz);
              }
#endif
            }
          }

          if (status != IALG_EOK)
          {
            break;
          }

#if TIDL_DEVICE_MULTICORE
          if (algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency)
          {
            numCores = algHandle->createParams->net->numCores;
            /* LDRA_JUSTIFY_START
            <metric start> branch <metric end>
            <justification start>
            Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
            <justification end> */
            if (workloadUnit != NULL)
            {
              /* LDRA_JUSTIFY_END */
              /* Update current workload completion in the state array */
              TIDL_syncWl(args->ctrlSetArgs, {}, TIDL_SYNC_POINT_COMPLETE, workloadUnit->wlExecId, relativeCoreId, numCores);
            }
          }

#endif
          tidlPerfStatsDdrStatsReadCounters(&val0, &val1, &val2, &val3, false);
          write_bytes_end = val0;
          read_bytes_end  = val1;
          TIDL_updateDDRBandwidthData((uint64_t *)(intAlgHandle->alglayerParams[currAlgLayer].metaData.profilePoint) + TIDL_PROFILE_DDR_BW_READ,
                                    read_bytes_end, write_bytes_end);
                                  
          /* Update tidlOutArgs with algLayer metaData*/
          if (tidlInArgs->enableLayerPerfTraces == 1 && algLayer->gcHelperHandle != NULL) /*for stats collection run we do not need to populate
                                                                                         performance data if we are running without network compiler*/
          {
            /* Put this inside while loop since some layers may execute on single core in low latency mode
              Corresponding algLayer will be present only if layer executes on current core */
            tidlOutArgs->metaDataLayer[algLayer->execLayerNum].actualOps = algLayer->metaData.actualOps;
            tidlOutArgs->metaDataLayer[algLayer->execLayerNum].layerExecId = layerId;
            tidlOutArgs->metaDataLayer[algLayer->execLayerNum].layerType = intAlgHandle->createParams->net->TIDLLayers[layerId].layerType;
            tidlOutArgs->metaDataLayer[algLayer->execLayerNum].totalOps = algLayer->metaData.totalOps;
#ifdef SOC_TDA54
          tidlOutArgs->metaDataLayer[algLayer->execLayerNum].ddrBandwidthRead  += workloadUnit->ddrBandwidthRead;
          tidlOutArgs->metaDataLayer[algLayer->execLayerNum].ddrBandwidthWrite += workloadUnit->ddrBandwidthWrite;
#endif

            /* Set layerExec Id = -1 for Data Layers */
            if (intAlgHandle->createParams->net->TIDLLayers[layerId].layerType == TIDL_ConstDataLayer) /*nned to set for Const Data Layer as WL is created for Const Data Layer*/
            {
              tidlOutArgs->metaDataLayer[algLayer->execLayerNum].layerExecId = -1;
            }

            for (int32_t k = 0; k < TIDL_PROFILE_MAX; k++)
            {
              tidlOutArgs->metaDataLayer[algLayer->execLayerNum].profilePoint[k] += algLayer->metaData.profilePoint[k]; /* += for layers which have multiple workloads */
              algLayer->metaData.profilePoint[k] = 0;
            }
          }
#ifdef HOST_EMULATION
          prevDataIdExecuted = layerId;
#endif
#if ENABLE_PREEMPTION
          if (intAlgHandle->isPreEmptionEnable != 0)
          {
            if (algLayer->isOptimalForPreemption != 0)
            {
              int32_t currExecutionLyr = algLayer->execLayerNum;

              int32_t nextExecutionLyr = currExecutionLyr + 1;
              intAlgHandle->preEmptContextInfo.earlyPreEmpt = 1;
              status = TIDL_updateContextMemPtrsForPreemption(&intAlgHandle->preEmptContextInfo, intAlgHandle->gcHelperHandle,
                                                              intAlgHandle->privContext,
                                                              intAlgHandle->preEmptHandle, intAlgHandle->perfSimOutput, intAlgHandle->sysScratchPtr.l2BasePtr[relativeCoreId],
                                                              intAlgHandle->sysScratchPtr.l3BasePtr[relativeCoreId], nextExecutionLyr, intAlgHandle->createParams->net->numLayers);
              /* Give chance for pre-emption at end of layer which is optimal for pre-emption */
              isLayerLevelPreemptionAllowed = 1;
              TIDL_giveChanceForPreemption(algHandle, &lockState, &oldIntState);
            }
          }
#endif
          tidlPerfStatsDdrStatsReadCounters(&val0, &val1, &val2, &val3, false);
          write_bytes_end = val0;
          read_bytes_end = val1;
          TIDL_updateDDRBandwidthData((uint64_t *)(intAlgHandle->alglayerParams[currAlgLayer].metaData.profilePoint) + TIDL_PROFILE_DDR_BW_READ,
                                      read_bytes_end, write_bytes_end);
        }
      }
    }
  }
  if (status == IALG_EOK)
  {
    /* Give chance for pre-emption after all layers' processing is complete.
    This is required irrespective of layer level TIDL_giveChanceForPreemption call since it is possible none of the layers is optimal for pre-emption */

#if ENABLE_PREEMPTION
    if ((isLayerLevelPreemptionAllowed == 1) && (intAlgHandle != NULL))
    {
      intAlgHandle->preEmptContextInfo.earlyPreEmpt = 0; /* In case layer level preemption is allowed, context mem ptr sets layer level buffers to be backed up, which is not needed in case pre-emption happens after the layer level while loop */
      status = TIDL_updateContextMemPtrsForPreemption(&intAlgHandle->preEmptContextInfo, intAlgHandle->gcHelperHandle,
                                                      intAlgHandle->privContext,
                                                      intAlgHandle->preEmptHandle, intAlgHandle->perfSimOutput, intAlgHandle->sysScratchPtr.l2BasePtr[relativeCoreId],
                                                      intAlgHandle->sysScratchPtr.l3BasePtr[relativeCoreId], NOT_VALID, intAlgHandle->createParams->net->numLayers);
    }
    TIDL_giveChanceForPreemption(algHandle, &lockState, &oldIntState);
#endif
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_001
    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
    This does not affect runtime behavior or safety.
    <justification end> */
    if ((status == IALG_EOK) && (intAlgHandle != NULL))
    {
    /* LDRA_JUSTIFY_END */
#if ENABLE_PREEMPTION
      /* Disable interrupts here */
      if (intAlgHandle->isPreEmptionEnable != 0)
      {
        /* Recalculate pre-emption layer list using HW performance */
        TIDL_reCalculateListForPreemption((void *)intAlgHandle->preEmptHandle,
                                          &intAlgHandle->preemptPerfHW,
                                          intAlgHandle->createParams->net->numLayers,
                                          intAlgHandle->gcHelperHandle->pWLDB->count,
                                          intAlgHandle->createParams->maxPreEmptDelay,
                                          intAlgHandle->sysScratchPtr.l2BasePtr[relativeCoreId],
                                          intAlgHandle->ddrBytesPerCPUCycle,
                                          intAlgHandle->perfSimOutput->simConfig.freqMHz,
                                          intAlgHandle->alglayerParams);
      }
#endif

      if (intAlgHandle->procCallCounter < 256U)
      {
        intAlgHandle->procCallCounter++;
      }
      for (i = 0; i < (int32_t)outBufs->numBufs; i++)
      {
        sTIDL_DataParams_t *dataParams;
        dataParams = TIDL_getDataParams(intAlgHandle->createParams->net, outBufs->bufDesc[i]->bufferId);
        tidlOutArgs->scale[i] = dataParams->tensorScale;
        tidlOutArgs->outDimValues[i][TIDL_DIM_BATCH] = dataParams->dimValues[TIDL_DIM_BATCH];
        tidlOutArgs->outDimValues[i][TIDL_DIM_DIM1] = dataParams->dimValues[TIDL_DIM_DIM1];
        tidlOutArgs->outDimValues[i][TIDL_DIM_DIM2] = dataParams->dimValues[TIDL_DIM_DIM2];
        tidlOutArgs->outDimValues[i][TIDL_DIM_NUMCH] = dataParams->dimValues[TIDL_DIM_NUMCH];
        tidlOutArgs->outDimValues[i][TIDL_DIM_HEIGHT] = dataParams->dimValues[TIDL_DIM_HEIGHT];
        tidlOutArgs->outDimValues[i][TIDL_DIM_WIDTH] = dataParams->dimValues[TIDL_DIM_WIDTH];
        tidlOutArgs->outPitchValues[i][TIDL_ROI_PITCH] = dataParams->pitch[TIDL_ROI_PITCH];
        tidlOutArgs->outPitchValues[i][TIDL_DIM1_PITCH] = dataParams->pitch[TIDL_DIM1_PITCH];
        tidlOutArgs->outPitchValues[i][TIDL_DIM2_PITCH] = dataParams->pitch[TIDL_DIM2_PITCH];
        tidlOutArgs->outPitchValues[i][TIDL_CHANNEL_PITCH] = dataParams->pitch[TIDL_CHANNEL_PITCH];
        tidlOutArgs->outPitchValues[i][TIDL_LINE_PITCH] = dataParams->pitch[TIDL_LINE_PITCH];
      }
      tidlOutArgs->numOutBufs = (int32_t)outBufs->numBufs;
      tidlOutArgs->numLayers = intAlgHandle->createParams->net->numLayers;
      tidlOutArgs->privArgs = (void *)intAlgHandle->createParams->net;

      TIDL_getTscTime(&tEndNetwork);

      if (tidlInArgs->enableLayerPerfTraces == 1)
      {
#ifdef INTERNAL_PROFILING
        TIDL_printProfileData(intAlgHandle->createParams->net->numLayers, tStartNetwork, tEndNetwork, tidlOutArgs);
#endif
      }

#if TIDL_DEVICE_MULTICORE
      if ((algHandle->createParams->net->inferenceMode == (int32_t)TIDL_inferenceModeLowLatency))
      {
        /* Set global sync state to TIDL_SYNC_WL_STATE_DEFAULT for next frame */
        volatile int8_t *syncPtr = (int8_t *)intAlgHandle->controlArgs.ctrlSetArgs[relativeCoreId].syncBufferPtr[TIDL_Virtual];
        (void)memset((void *)syncPtr, TIDL_SYNC_WL_STATE_DEFAULT, (uint64_t)TIDL_GLOBAL_SYNC_OFFSET * sizeof(int8_t));
      }
#endif

      tidl_printf(1, "TIDL_process is completed with handle : %p \n", algHandle);

#if ENABLE_PREEMPTION
      if (intAlgHandle->isPreEmptionEnable != 0)
      {
        TIDL_unlockForPreemption(oldIntState, intAlgHandle->createParams, &lockState);
      }
#endif
      /**** No code should be written after TIDL_unlockForPreemption call ******/
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start>
    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_003
    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
    This does not affect runtime behavior or safety.
    <justification end> */
    else
    {
      status = IALG_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
  }

  return status;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_activate
@brief      This is the function to do activation of algorithm.
            All scratch memory  are restored in this
            Refer algFree function for details in ialg.h file.
@param      handle : Algorithm Instance handle
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_activate(IALG_Handle handle)
{
  TIDL_Handle algHandle = (TIDL_Handle)(void *)(handle);

  /*----------------------------------------------------------------*/
  /* Do Activation only if it is deactivated or just after creation */
  /*----------------------------------------------------------------*/
  int32_t status = IALG_EOK;
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  The condition check is deterministic and does not impact the safety or reliability of the system.
  Therefore, it is excluded from safety coverage requirements.
  Effect on this UNIT - Safety checks to avoid undefined behavior and improves the stability and error handling.
  <justification end> */
  if (algHandle->algState == (uint8_t)ALG_NOT_ACTIVE)
  {
  /* LDRA_JUSTIFY_END */
  /* TIDL_LDRA_TAG_ALG_PRIOR_CHECK_005 */
  /* Initialize the DMA Utils Handle */
#ifdef HOST_EMULATION
    if (((uint32_t)algHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == TIDL_FLOW_CTRL_REF_ONLY)
    {
      algHandle->dmaUtilsContext = NULL;
    }
    else
#endif
    {
      status = TIDL_initDmaUtils(algHandle);
    }
    if (status != IALG_EOK)
    {
      // algHandle->algState = (uint8_t)ALG_NOT_ACTIVE;
      //  tidl_printf(0, "TIDL_initDmaUtils returned Error Code for handle: %p \n", algHandle);
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_ALG_COMMON, TIDL_ERROR_ALG_COMMON_NULL_DMAUTILSCTNTX_ERROR);
      status = IALG_EFAIL; // uncommenting return, needs to update misra c issue.
    }
    if (status == IALG_EOK)
    {
      TIDL_Handle intAlgHandle = (TIDL_Handle)algHandle->intAlgHandle;
      tidl_printf(1, "TIDL_activate is called with handle : %p - Copying handle of size %d from %p to %p \n",
                  algHandle, algHandle->memRec[ALG_HANDLE_MEMREC].size, algHandle->memRec[ALG_HANDLE_MEMREC].base, intAlgHandle);
      status = TIDL_memcpy2D(intAlgHandle,
                             (const void *)algHandle->memRec[ALG_HANDLE_MEMREC].base,
                             algHandle->memRec[ALG_HANDLE_MEMREC].size,
                             1U,
                             0,
                             0,
                             algHandle->dmaUtilsContext,
                             algHandle->memcpyTr);
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start>
      Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
      Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
      This does not impact functional correctness or safety.
      <justification end> */
      if (status == IALG_EOK)
      {
        /* LDRA_JUSTIFY_END */
        TIDL_mmaHwaStatusWorkAround();

        /* Copy persistent weight buffers for all layers*/
        status = TIDL_copyPersistentWeightsToIntMem(intAlgHandle);
        /* LDRA_JUSTIFY_START
        <metric start> branch <metric end>
        <justification start>
        Rationale - FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
        Effect on this UNIT - This condition results in partial structural coverage(eg. uncovered statement/branch) in the current test context. 
        This does not impact functional correctness or safety.
        <justification end> */
        if (status == IALG_EOK)
        {
          /* LDRA_JUSTIFY_END */
#if ENABLE_PREEMPTION
          /* Copy Pre-emption ContextMemory to Internal memory*/
          if (intAlgHandle->isPreEmptionEnable != 0)
          {
            TIDL_restoreContextMemToIntMemForPreemption(&intAlgHandle->preEmptContextInfo,
                                                        intAlgHandle->dmaUtilsContext, intAlgHandle->memcpyTr, intAlgHandle->createParams);
          }
#endif
          algHandle->algState = (uint8_t)ALG_ACTIVE;
        }
      }
    }
    else
    {
      // algHandle->algState = (uint8_t)ALG_NOT_ACTIVE;
      // status = IALG_EFAIL; commenting for now avoid MISRA-C, we should pass state as return from this function
      tidl_printf(0, "failed to activate the Algo; Alg state is: %d \n", algHandle->algState);
    }
// Map the scratch area on the GPU. All kernels accessing this
// scratch area should be using the "data present()" clause instead
// of copy or create.
// ALG_REF_SCRATCH_BUFF_MEMREC=8
#if BUILD_WITH_OPENACC // Scratch buffer has issues to be resolved
    uint8_t *scratch_buffer_base = (uint8_t *)algHandle->memRec[ALG_REF_SCRATCH_BUFF_MEMREC].base;
    uint32_t scratch_buffer_size = (uint32_t)algHandle->memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size;
    OPENACC(enter data create(scratch_buffer_base[:scratch_buffer_size]))
#endif // Scratch buffer

// Map the coefficients memory (persistant contents) area into the GPU
// ALG_PERSIST_DDR_NET_MEMREC = 14
#if BUILD_WITH_OPENACC
    uint8_t *coeff_buffer_base = (uint8_t *)algHandle->memRec[ALG_PERSIST_DDR_NET_MEMREC].base;
    uint32_t coeff_buffer_size = (uint32_t)algHandle->memRec[ALG_PERSIST_DDR_NET_MEMREC].size;
    // fprintf(stderr, "mapping 0x%lx - 0x%lx host memory area to GPU (ALG_PERSIST_DDR_NET_MEMREC=%d)\n", coeff_buffer_base, coeff_buffer_base+coeff_buffer_size,ALG_PERSIST_DDR_NET_MEMREC);
    OPENACC(enter data copyin(coeff_buffer_base[:coeff_buffer_size]))
#endif
  }
  return;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_deactivate
@brief      This is the function to do de-activation of algorithm.
            All scratch memory are stoed to external memory in this
            Refer algFree function for details in ialg.h file.
@param      handle : Algorithm Instance handle
@remarks    None
@return     None
----------------------------------------------------------------------------
*/
void TIDL_deactivate(IALG_Handle handle)
{
  TIDL_Handle algHandle = (TIDL_Handle)(void *)(handle);
  TIDL_Handle intAlgHandle = (TIDL_Handle)(void *)(algHandle->intAlgHandle);

  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start>
  Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
  This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
  TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_005
  Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
  This does not affect runtime behavior or safety.
  <justification end> */
  if (algHandle->algState == (uint8_t)ALG_ACTIVE)
  {
    /* LDRA_JUSTIFY_END */

    TIDL_L1DandL2CacheWbInv();

// Unmap the accPtr scratch area from the GPU.
#if BUILD_WITH_OPENACC // Scratch buffer has issues to be resolved
    if (algHandle->algState == (uint8_t)ALG_ACTIVE)
    {
      uint8_t *scratch_buffer_base = (uint8_t *)algHandle->memRec[ALG_REF_SCRATCH_BUFF_MEMREC].base;
      uint32_t scratch_buffer_size = (uint32_t)algHandle->memRec[ALG_REF_SCRATCH_BUFF_MEMREC].size;
      OPENACC(exit data delete(scratch_buffer_base[:scratch_buffer_size]))
    }
#endif // Scratch buffer

// Unmap the coefficients area from GPU.
// ALG_PERSIST_DDR_NET_MEMREC = 14
#ifdef BUILD_WITH_OPENACC
    if (algHandle->algState == (uint8_t)ALG_ACTIVE)
    {
      uint8_t *coeff_buffer_base = (uint8_t *)algHandle->memRec[ALG_PERSIST_DDR_NET_MEMREC].base;
      uint32_t coeff_buffer_size = (uint32_t)algHandle->memRec[ALG_PERSIST_DDR_NET_MEMREC].size;
      OPENACC(exit data delete(coeff_buffer_base[:coeff_buffer_size]))
    }
#endif

    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start>
    Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
    This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
    TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_004
    Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
    This does not affect runtime behavior or safety.
    <justification end> */
    if (NULL != intAlgHandle)
    {
      /* LDRA_JUSTIFY_END */
      tidl_printf(1, "TIDL_deactivate is called with handle : %p - Copying handle of size %d from %p to %p \n",
                  algHandle, intAlgHandle->memRec[ALG_HANDLE_MEMREC].size, intAlgHandle, intAlgHandle->memRec[ALG_HANDLE_MEMREC].base);
      (void)TIDL_memcpy2D(intAlgHandle->memRec[ALG_HANDLE_MEMREC].base,
                          intAlgHandle,
                          intAlgHandle->memRec[ALG_HANDLE_MEMREC].size,
                          1U,
                          0,
                          0,
                          intAlgHandle->dmaUtilsContext,
                          intAlgHandle->memcpyTr);
#if ENABLE_PREEMPTION
      /* Backup Pre-emption ContextMemory to external memory*/
      if (intAlgHandle->isPreEmptionEnable != 0)
      {
        TIDL_storeContextMemToExtMemForPreemption(&intAlgHandle->preEmptContextInfo,
                                                  intAlgHandle->dmaUtilsContext, intAlgHandle->memcpyTr);
      }
#endif
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start>
      Rationale - PRIOR_CHECK: Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_ALG_PRIOR_CHECK_005
      Effect on this UNIT - As the condition is effectively bypassed due to earlier checks, it remains unexecuted in current test scenarios. 
      This does not affect runtime behavior or safety.
      <justification end> */
      /* Release the dma resources*/
      if (intAlgHandle->dmaUtilsContext != NULL)
      {
        /* LDRA_JUSTIFY_END */
        (void)DmaUtilsAutoInc3d_deinit(intAlgHandle->dmaUtilsContext);
      }
    }

    algHandle->algState = (uint8_t)ALG_NOT_ACTIVE;
  }

  return;
}

/**
----------------------------------------------------------------------------
@ingroup    TIDL_IVISION
@fn         TIDL_free
@brief      This is the function which user can call to get back
            the memory given to algo.
            Refer algFree function for details in ialg.h file.
@param      handle : Algorithm Instance handle
@param      memTab : memory table populated by the Algorithm
@remarks    None
@return     IALG_EOK   - Successful
            IALG_EFAIL - Unspecified error
----------------------------------------------------------------------------
*/
int32_t TIDL_free(IALG_Handle handle, IALG_MemRec *memRec)
{
  int32_t status = IALG_EOK;
  TIDL_Handle algHandle = (TIDL_Handle)(void *)(handle);
  void *plastActiveHandle;
  /* Removed TIDL_deactivate from test bench and added here to release DMA resources before freeing handle */
  TIDL_deactivate((IALG_Handle)handle);

  if (algHandle->createParams->forceNegativeTest == TIDL_SAFETY_FLAG_ALG_FREE_FORCE_LSTHNDLE_INVALID)
  {
    algHandle->createParams->coreId = 1;
  }
  /* Since we are freeing this handle, we need to set TIDL_lastActiveHandle to NULL if it matches,
   * so that a deactivate is not called by the preemption logic later */
  plastActiveHandle = TIDL_getLastActiveHandle(algHandle->createParams->coreId);

  if (plastActiveHandle == handle)
  {
    TIDL_setLastActiveHandle(algHandle->createParams->coreId, NULL);
  }

/* For stats collection update the final calculated tensor scales which should be used during
inference */
#ifdef HOST_EMULATION
  if (((uint32_t)algHandle->createParams->flowCtrl & TIDL_FLOW_CTRL_REF_STAT) == TIDL_FLOW_CTRL_REF_STAT)
  {
    if (algHandle->createParams->quantRangeUpdateFactor == 0.0f)
    {
      int32_t layerIdx;
      for (layerIdx = 0; layerIdx < algHandle->createParams->net->numLayers; layerIdx++)
      {
        TIDL_UpdateScaleFactors(algHandle, layerIdx, 0, 0, 0);
      }
    }
  }
#endif
#if ENABLE_PREEMPTION
  if (algHandle->isPreEmptionEnable != 0)
  {
    status = TIDL_removePriorityObject(algHandle, &algHandle->memRec[ALG_CONTEXT_MEM_MEMREC]);
  }
#endif
  (void)memcpy(memRec, algHandle->memRec, sizeof(IALG_MemRec) * algHandle->numMemRecs);

#ifdef BUILD_WITH_CUDA_MEM_MANAGER
  /* Free CUDA Memory Manager */
  if(algHandle->cudaMemManager != NULL)
  {
    tidl_printf(1, "Freeing CUDA Memory Manager...\n");
    TIDL_cudaSetThreadManager((TIDL_CudaMemManager*)algHandle->cudaMemManager);
    //TIDL_cudaMemManagerPrintStats((TIDL_CudaMemManager*)algHandle->cudaMemManager);
    TIDL_cudaMemManagerFree((TIDL_CudaMemManager*)algHandle->cudaMemManager);
    free(algHandle->cudaMemManager);
    algHandle->cudaMemManager = NULL;
    TIDL_cudaSetThreadManager(NULL);
  }
#endif
  return status;
}
