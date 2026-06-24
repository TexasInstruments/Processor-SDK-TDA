/*
*******************************************************************************
*
* Module Name: Graph compiler - Primarilly designed for execution plan for DNN
* based graphs, but should scale for any graph based execution planner
*
* "Performance simulator - Predicts time for overall graph execution for TI's
* C7x + MMA processor architecture. It models the performance of different
* processing elements of graph and provides a summarized output of performance
*
* Copyright (C) 2015-21 Texas Instruments Incorporated - http://www.ti.com/
* ALL RIGHTS RESERVED
*******************************************************************************
*/

/**
******************************************************************************
*  @file     gc_helper.c
*
*  @brief    Implementation of helper function module for GC,
*            can be used by client of GC module
*
*  @author   PKS
*
*  @version  0.1 - Sep 2021: Initial Version [PKS]
*
*****************************************************************************
*/

#include "gc_helper.h"
#include <string.h>
#include <math.h>

#if defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J722S)
#define TIDL_DEVICE_MULTICORE (1)
#else
#define TIDL_DEVICE_MULTICORE (0)
#endif
void getWLMetaDataID(int64_t WLId, sWLMetaDataID_t *pWLMetaDataID)
{
  sIntWLMetaDataID_t *psIntWLMetaDataID;
  psIntWLMetaDataID = (sIntWLMetaDataID_t *)&WLId;
  pWLMetaDataID->numSplit = psIntWLMetaDataID->numSplit;
  pWLMetaDataID->tileType = psIntWLMetaDataID->tileType;
  pWLMetaDataID->inTileHeight = psIntWLMetaDataID->inTileHeight;
  pWLMetaDataID->outTileHeight = psIntWLMetaDataID->outTileHeight;
  pWLMetaDataID->passiveLinkIndex =
      psIntWLMetaDataID->passiveLinkIndex;
  pWLMetaDataID->isFirstTile =
      psIntWLMetaDataID->isFirstTile;
  pWLMetaDataID->isLastTile =
      psIntWLMetaDataID->isLastTile;
      pWLMetaDataID->dataFlowType =
      psIntWLMetaDataID->dataFlowType;
  pWLMetaDataID->packetizeMode =
      psIntWLMetaDataID->packetizeMode;

  return;
}

void getWLMetaDataIDExt(int64_t WLId, sWLMetaDataIDExt_t *pWLMetaDataIDExt)
{
  sIntWLMetaDataIDExt_t *psIntWLMetaDataIDExt;
  psIntWLMetaDataIDExt = (sIntWLMetaDataIDExt_t *)&WLId;
  pWLMetaDataIDExt->numSplitB = psIntWLMetaDataIDExt->numSplitB;
  pWLMetaDataIDExt->inNumBatch = psIntWLMetaDataIDExt->inNumBatch;
  pWLMetaDataIDExt->outNumBatch = psIntWLMetaDataIDExt->outNumBatch;
  pWLMetaDataIDExt->numInCh = psIntWLMetaDataIDExt->numInCh;
  pWLMetaDataIDExt->numOutCh = psIntWLMetaDataIDExt->numOutCh;
  pWLMetaDataIDExt->numGrp = psIntWLMetaDataIDExt->numGrp;

  return;
}

void getMetaDataID(int64_t id, sMetaDataID_t *pMetaDataID)
{
  sIntMetaDataID_t *psIntMetaDataID;
  psIntMetaDataID = (sIntMetaDataID_t *)&id;
  pMetaDataID->networkId = psIntMetaDataID->networkId;
  pMetaDataID->layerId = psIntMetaDataID->layerId;
  pMetaDataID->subLayerId = psIntMetaDataID->subLayerId;
  pMetaDataID->type = psIntMetaDataID->type;
  pMetaDataID->dependentLayerId =
      psIntMetaDataID->dependentLayerId;
  pMetaDataID->multiBufIndex =
      psIntMetaDataID->multiBufIndex;
  // some issue with how the int to MetaData casting happens , causing all zero binary bits to give -1 in decimal
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  pMetaDataID->tileType = (psIntMetaDataID->tileType <= (int64_t)NO_TILE) ? (int64_t)NO_TILE : psIntMetaDataID->tileType;
  /* LDRA_JUSTIFY_END */
  return;
}

int32_t initGCHelperHandle(sGCHelperHandle *gcHelperHandle,
                           const sTIDL_Network_t *pNet,
                           sGCDataBaseInfo_t *pGCDataBaseInfo,
                           sGCCommonDataBaseInfo_t *pGCCommonDataBaseInfo)
{
  int32_t status = 0;
  if ((gcHelperHandle != NULL) && (pNet != NULL) && (pGCDataBaseInfo != NULL) && (pGCCommonDataBaseInfo != NULL))
  {
    gcHelperHandle->pNet = pNet;
    gcHelperHandle->pBufDB = &pGCCommonDataBaseInfo->bufDB;
    gcHelperHandle->pWLDB = &pGCDataBaseInfo->wlDB;
    gcHelperHandle->pAuxWLDB = &pGCDataBaseInfo->wlAuxDB;
  }
  else
  {
    status = -1;
  }

  return status;
}
#define CHANGE_RECURSIVE (1)

sBufParams_t *getBufParamsFromBufIndex(const sGCHelperHandle *gcHelperHandle, int32_t bufIndex)
{
#if 0
  sBufParams_t *pBufParam = NULL;
  //if(gcHelperHandle != NULL)
  {
    sBufParams_t *pBufList = gcHelperHandle->pBufDB->pBufList;
    if (bufIndex != NOT_VALID)
    {
      if (pBufList != NULL)
      {
        pBufParam = &pBufList[bufIndex];
      }
    }
  }
#else
  sBufParams_t *pBufParam;
  sBufParams_t *pBufList = gcHelperHandle->pBufDB->pBufList;
  pBufParam = &pBufList[bufIndex];
#endif
  return pBufParam;
}
sBufParams_t *getBufParamsFromBufIndexStrict(const sGCHelperHandle *gcHelperHandle, int32_t bufIndex)
{
  sBufParams_t *pBufParam = NULL;
  // if(gcHelperHandle != NULL)
  {
    sBufParams_t *pBufList = gcHelperHandle->pBufDB->pBufList;
    if (bufIndex != NOT_VALID)
    {
      if (pBufList != NULL)
      {
        pBufParam = &pBufList[bufIndex];
      }
    }
  }
  return pBufParam;
}
sWorkloadUnit_t *getWLUnitPtr(sGCHelperHandle *gcHelperHandle, int32_t unitIdx)
{
  sWorkloadUnit_t *pWLUnit = NULL;
#if 0
  if(unitIdx>=0) pWLUnit = gWLUnitDBHandle + unitIdx ;
#else
  if (gcHelperHandle != NULL)
  {
    if ((unitIdx >= 0) && (gcHelperHandle->pWLDB->wlOffset[unitIdx] != NOT_VALID)) /* for unitIdx < 0 or unitIndx > number of workloadsreturn NULL*/
    {
      int32_t offset = gcHelperHandle->pWLDB->wlOffset[unitIdx];
      pWLUnit = (sWorkloadUnit_t *)((int8_t *)gcHelperHandle->pWLDB->pWLList + offset);
    }
  }
#endif
  return pWLUnit;
}

int32_t getLinkIdx(const sWorkloadUnit_t *pWL, int32_t linkSubType, int32_t *linkIdx)
{
  const sLink_t *pLink = NULL;
  // sLink_t *linkPtrList[MAX_LINKS_PER_WL];
  int32_t linkCount = 0;
  int32_t offset = 0;
  for (int32_t linkNum = 0; (pWL != NULL) && (linkNum < pWL->numLinks); linkNum++)
  {
    // getLinkPtrs(pWL, NOT_VALID,linkNum, linkPtrList);
    if (linkNum == 0)
    {
      offset = sizeof(sWorkloadUnit_t) - (sizeof(sLink_t) * NUM_LINKS_PER_WL_STATIC);
    }
    else
    {
      offset = pWL->linkOffset[linkNum];
    }
    pLink = (sLink_t *)((int8_t *)pWL + offset);
    if (pLink->subType == linkSubType)
    {
      linkIdx[linkCount] = linkNum;
      linkCount = linkCount + 1;
    }
  }
  linkIdx[linkCount] = NOT_VALID;
  return linkCount;
}

void getLinkPtrs(const sWorkloadUnit_t *pWLUnit, int32_t linkSubType, int32_t linkIdx, sLink_t *linkPtrList[])
{
  sLink_t *pLink = NULL;
  int32_t offset;
  int32_t linkIdxList[MAX_LINKS_PER_WL];
  int32_t linkCount = 0;
  int32_t linkIdexLocal;
  if (linkSubType != NOT_VALID)
  { // Query is based on linkSubType so need to find linkIdx
    linkCount = getLinkIdx(pWLUnit, linkSubType, &linkIdxList[0]);
  }
  if (linkSubType == NOT_VALID && linkIdx >= 0)
  {
    linkIdxList[0] = linkIdx;
    linkCount = 1;
  }
  for (int32_t i = 0; i < linkCount; i++)
  {
    linkIdexLocal = linkIdxList[i];
    if (linkIdexLocal == 0)
    {
      offset = sizeof(sWorkloadUnit_t) - (sizeof(sLink_t) * NUM_LINKS_PER_WL_STATIC);
    }
    else
    {
      offset = pWLUnit->linkOffset[linkIdexLocal];
    }
    pLink = (sLink_t *)((int8_t *)pWLUnit + offset);
    linkPtrList[i] = pLink;
  }
  linkPtrList[linkCount] = NULL;
}

void getLayerIdInit(void *getIdContext,
                    int32_t coreId,
                    int32_t layerGroupId, // Needed for backwards compatibility
                    sTIDL_Network_t *net,
                    sGCHelperHandle *gcHelperHandle,
                    sWorkloadSuperGroup_t *wlSuperGroup)
{
  sGetLayerIdContext_t *context = (sGetLayerIdContext_t *)getIdContext;

  context->groupIdx = 0;
  context->subGroupIdx = 0;
  context->countIdx = 0;
  context->workLoadIdx = 0;
  context->algLayerIdx = 0;

  context->coreId = coreId;
  context->layerGroupId = layerGroupId;
  context->net = net;
  context->gcHelperHandle = gcHelperHandle;
  context->wlSuperGroup = wlSuperGroup;

  if (wlSuperGroup != NULL)
  {
    context->workLoadIdx = wlSuperGroup->workloadGrpList[0].subGroup[0].startIndexWLUnitDB;
  }

  return;
}

int32_t getLayerIdToExecute(void *getIdContext,
                            int32_t isInit,
                            int32_t *wlRepeatIter,
                            int32_t *currAlgLayerIdx,
                            sWorkloadUnit_t **workLoadUnit)
{
  sGetLayerIdContext_t *context = (sGetLayerIdContext_t *)getIdContext;
  int32_t currAlgLayer = VALID;
  int32_t workLoadId;
  int32_t layerId = 0;
  int32_t wlRepeat = 0;
  sWorkloadUnit_t *wlPtr = NULL;

  //: TODO: Need to handle going to old flow if supertiling is enabled

  if (context->wlSuperGroup != NULL)
  {

    const sWorkloadGroup_t *wlGroup = &context->wlSuperGroup->workloadGrpList[context->groupIdx];
    const sWorkloadSubGroup_t *wlSubGroup = &wlGroup->subGroup[context->subGroupIdx];

    workLoadId = context->workLoadIdx;
    context->algLayerIdx = workLoadId;
    currAlgLayer = context->algLayerIdx;
    wlRepeat = context->countIdx;

    if (workLoadId != NOT_VALID)
    {
      context->workLoadIdx++;

      if ((context->workLoadIdx >= wlSubGroup->startIndexWLUnitDB) &&
          (context->workLoadIdx <= wlSubGroup->endIndexWLUnitDB))
      {
        /* Next iteration will be part of same subGroup hence do nothing */
      }
      else
      {
        context->countIdx++;
        if ((context->countIdx < wlSubGroup->count) && (isInit == 0))
        {
          /* Count indicates that we have to repeat the same sub group again.
          Subtraction by 1 so that next time when we increment we get the right index */
          context->workLoadIdx = wlSubGroup->startIndexWLUnitDB;
        }
        else
        {
          context->countIdx = 0;
          context->subGroupIdx++;
          if (context->subGroupIdx < wlGroup->numSubGroups)
          {
            const sWorkloadSubGroup_t *wlSubGroupNext = &wlGroup->subGroup[context->subGroupIdx];
            context->workLoadIdx = wlSubGroupNext->startIndexWLUnitDB;
          }
          else
          {
            context->groupIdx++;
            context->subGroupIdx = 0;
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> FUTURE_USE: This condition is present to support future testing scenarios and it is retained for robustness and exception handling.
            <justification end> */
            if (context->groupIdx < context->wlSuperGroup->numGroups)
            {

              const sWorkloadSubGroup_t *wlSubGroupNext = &wlGroup->subGroup[0];
              context->workLoadIdx = wlSubGroupNext->startIndexWLUnitDB;
            }
            else
            /* LDRA_JUSTIFY_END */
            {
              context->workLoadIdx = NOT_VALID;
              context->groupIdx = 0;
            }
          }
        }
      }
    }

    if (workLoadId != NOT_VALID)
    {
      wlPtr = getWLUnitPtr(context->gcHelperHandle, workLoadId);
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start> PRIOR_CHECK : Under current execution paths, the condition cannot be reached because of logically and structurally preempted by earlier check.
      This condition is guarded by a prior check in the control flow tagged as below mentioned tag in the code.
      TIDL_LDRA_TAG : TIDL_LDRA_TAG_GC_HELPER_PRIOR_CHECK_001
      <justification end> */
      if (wlPtr != NULL)
      /* LDRA_JUSTIFY_END */
      {
        layerId = wlPtr->layerId;
      }
    }
    else
    {
      wlPtr = NULL;
      layerId = NOT_VALID;
    }
  }
  else
  {
    /* Reference flow with GC information*/
    sTIDL_Layer_t *tidlLayer;

    wlPtr = NULL;
    workLoadId = NOT_VALID;
    currAlgLayer = context->algLayerIdx;

    if (context->workLoadIdx < context->net->numLayers)
    {
      layerId = context->workLoadIdx;
      context->workLoadIdx++;

      tidlLayer = &context->net->TIDLLayers[layerId];
      if (tidlLayer->layersGroupId == context->layerGroupId)
      {
        context->algLayerIdx++;
      }
    }
    else
    {
      layerId = NOT_VALID;
      context->algLayerIdx = NOT_VALID;
    }
  }

  if (currAlgLayerIdx != NULL)
  {
    *currAlgLayerIdx = currAlgLayer;
  }
  if (wlRepeatIter != NULL)
  {
    *wlRepeatIter = wlRepeat;
  }
  if (workLoadUnit != NULL)
  {
    *workLoadUnit = wlPtr;
  }
  else
  {
    layerId = NOT_VALID;
  }

  return layerId;
}

int32_t getNumSplits(const void *workloadHandle)
{
  sWLMetaDataID_t pWLMetaDataID;
  getWLMetaDataID(((const sWorkloadUnit_t *)workloadHandle)->metadata, &pWLMetaDataID);
  return pWLMetaDataID.numSplit;
}

int8_t *get_int8_t_pointer(int8_t arr[], int32_t offset)
{
  return &arr[offset];
}

void TIDL_factorizeBy64K(uint32_t icnt0, uint32_t icnt1, uint32_t *factorLarge, uint32_t *factorSmall)
{
  uint32_t maxFactor = (uint32_t)sqrt(icnt1);
  while (maxFactor > 1U)
  {
    if (((icnt1 % maxFactor) == 0U) && ((icnt0 * maxFactor) < ICNT_MAX_16_BIT))
    {
      break;
    }
    maxFactor--;
  }
  *factorLarge = icnt0 * maxFactor;
  *factorSmall = icnt1 / maxFactor;
}

int8_t *getGCDataBase(sGraphCompilerOutArgs_t *gcInfo, int32_t dataBaseType)
{
  int8_t *pointerToDataBase = NULL;
  pointerToDataBase = &gcInfo->dataBase[gcInfo->dataBaseRegionInfo[dataBaseType].offset];
  return pointerToDataBase;
}

#if (TIDL_DEVICE_MULTICORE == 0)
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t getDimSplitForCore.* <function end>
<justification start> NOT_IN_SCOPE : This code is maintained to be common between X86 and EVM.
And this code is applicable for only multi core devices. We cannot use the macro of multi core
devices to mask this code because these pre-processor directives are not available on X86 side of code for
good reasons. So preventing this code to be checked by LDRA for multi core devices on EVM
<justification end> */
#endif

int32_t getDimSplitForCore(int32_t coreIdx, int32_t numCore, int32_t totalDimValue)
{
  int32_t splitDimValue;

  if (totalDimValue < numCore)
  {
    splitDimValue = totalDimValue;
  }
  else
  {
    splitDimValue = totalDimValue / numCore;
    if (numCore > 1 && (coreIdx + 1) <= (totalDimValue % numCore)) /*when Dim cannot be split across cores Evenly*/
    {
      splitDimValue++;
    }
  }

  return splitDimValue;
}
#if (TIDL_DEVICE_MULTICORE == 0)
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t getNumInChforCore.* <function end>
<justification start> NOT_IN_SCOPE : This code is maintained to be common between X86 and EVM.
And this code is applicable for only multi core devices. We cannot use the macro of multi core
devices to mask this code because these pre-processor directives are not available on X86 side of code for
good reasons. So preventing this code to be checked by LDRA for multi core devices on EVM
<justification end> */
#endif
int32_t getNumInChforCore(int32_t coreIdx, int32_t numCore, int32_t totalNi, int32_t totalGrpNum, int32_t layerType)
{
  int32_t splitNi;
  int32_t NiPerG = totalNi / totalGrpNum;

  if (NiPerG != 1 && layerType == TIDL_ConvolutionLayer)
  {
    splitNi = NiPerG * getDimSplitForCore(coreIdx, numCore, totalGrpNum);
  }
  else
  {
    splitNi = getDimSplitForCore(coreIdx, numCore, totalNi);
  }

  return splitNi;
}
#if (TIDL_DEVICE_MULTICORE == 0)
/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t getNumOutChforCore.* <function end>
<justification start> NOT_IN_SCOPE : This code is maintained to be common between X86 and EVM.
And this code is applicable for only multi core devices. We cannot use the macro of multi core
devices to mask this code because these pre-processor directives are not available on X86 side of code for
good reasons. So preventing this code to be checked by LDRA for multi core devices on EVM
<justification end> */
#endif
int32_t getNumOutChforCore(int32_t coreIdx, int32_t numCore, int32_t totalNo, int32_t totalGrpNum, int32_t layerType)
{
  int32_t splitNo;
  int32_t NoPerG = totalNo / totalGrpNum;

  if (totalGrpNum == 1 || layerType != TIDL_ConvolutionLayer)
  {
    splitNo = getDimSplitForCore(coreIdx, numCore, totalNo);
  }
  else
  {
    splitNo = getDimSplitForCore(coreIdx, numCore, totalGrpNum) * NoPerG;
  }

  return splitNo;
}
