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
 @file    tidl_preEmption.c
 @brief   This file defines APIS for TIDL Preemption functions.
 @version 0.1 (Sep 2021) : Initial Version
 ----------------------------------------------------------------------------
*/

#include "tidl_alg_int.h"
#include "tidl_types.h"
#include <stdlib.h>
#include "stdint.h"
#include "perfsim.h"
#include "tidl_preEmption.h"
#include "tidl_priv_algo.h"
#include "tidl_commonUtils.h"
#include "tidl_alg_utils.h"

#ifdef DEBUG_PREEMPTION_HOST_EMULATION
IALG_Handle gHandleForPremptTest;
#endif

#define PREEMPT_PRINT_LVL (1)
void * TIDL_lastActiveHandle[TIDL_MAX_CORENUM]  = {NULL};
TIDL_Lock_t gTIDLpFxnLockTest[TIDL_MAX_CORENUM] = {NULL};
TIDL_Unlock_t gTIDLpFxnUnlockTest[TIDL_MAX_CORENUM] = {NULL};

void TIDL_setLastActiveHandle(int32_t coreId, void * handle)
{
  TIDL_lastActiveHandle[coreId] = handle;
}

void* TIDL_getLastActiveHandle(int32_t coreId)
{
  return TIDL_lastActiveHandle[coreId];
}
#if ENABLE_PREEMPTION
#ifdef TIDL_COVERAGE_DEAD_CODE
TIDL_Lock_t TIDL_getLockFxnPtr(int32_t coreId)
{
  return gTIDLpFxnLockTest[coreId];
}

TIDL_Unlock_t TIDL_getUnLockFxnPtr(int32_t coreId)
{
  return gTIDLpFxnUnlockTest[coreId];
}
#endif
/**<
 *******************************************************************************
  @struct TIDL_preEmptionObj
  @brief  This structure holds the key parameters for PreEmption Handle

  @param ctxSize           : Context memory size required to backup for each layer
  @param dataId            : Data idx the current preEmption handle corresponds to (preEmption handle is indexed using execution order)
  @param procTime          : Time consumed for processing each layer in millisecond
 *******************************************************************************
*/
typedef struct
{
  int32_t  ctxSize;
  int16_t  dataId;
  float32_tidl  procTime;
}TIDL_preEmptionObj;

/*---------------------------------------------------------------------*/
/*For each priority level there are N models allowed                   */
/* Need to store the handle during init and remove handle during free  */
/* for each level, there should be only context memory once            */
/*---------------------------------------------------------------------*/
typedef struct{
  void *handle;
  int32_t alive ;
  IALG_MemRec memRec;
} TIDL_priorityObject_t ;

//zeroth index will always have the alive object, if it is removed then
//
typedef struct{
  int32_t priorityId ;
  int32_t numObjects ;
  TIDL_priorityObject_t objInfo[TIDL_MAX_OBJECTS_PER_LEVEL];
} TIDL_priorityWiseTable_t ;

int32_t gTIDL_firstObject = 1 ;
int32_t gTIDL_activePriorityLevels = 0 ;
TIDL_priorityWiseTable_t gTIDL_priorityTable[TIDL_MAX_PRIORITY_LEVEL];

typedef enum{
  TIDL_ALIVE_EXCLUDING_FIRST = 0,
  TIDL_FIRST_NON_ALIVE = 1
} eTIDL_prioritySearchType;

static TIDL_priorityObject_t*
  TIDL_getPriorityObject(TIDL_priorityWiseTable_t *pPriorityInfo, int32_t searchType);
static int32_t TIDL_getPriorityIndex(int32_t priorityId);

static void TIDL_resetPriorityDataBase(void)
{
  if(gTIDL_firstObject != 0)
  { //During first time, reset everything
    (void)memset(gTIDL_priorityTable, 0, sizeof(TIDL_priorityWiseTable_t)*(uint32_t)TIDL_MAX_PRIORITY_LEVEL);
    //Latch to right priority
    for(uint32_t idx = 0 ; idx < TIDL_MAX_PRIORITY_LEVEL ; idx++ )
    {
      gTIDL_priorityTable[idx].priorityId = -1;
    }
    gTIDL_firstObject = 0 ;
  }
  return;
}

int32_t TIDL_getContextMemSize(int32_t priorityId, const sPerfSim_t * perfSimInfo)
{
  int32_t contextMemSize = TIDL_PREEMPT_CONTEXT_MEMSIZE_DUMMY ;
  TIDL_resetPriorityDataBase() ;
  if(TIDL_getPriorityIndex(priorityId) == -1)
  {
    //Not found any existing entry,so request for allocation
    if(perfSimInfo != NULL)
    {
      contextMemSize = perfSimInfo->simConfig.sizeL3MemKB * 1024 ;
    }
    else
    {
      contextMemSize = TIDL_DEFAULT_L3_MEM_SIZE ;
    }
  }
  tidl_printf(PREEMPT_PRINT_LVL,"PREEMPTION: Requesting memory of size %d for targetPriority = %d\n",
    contextMemSize,priorityId);
  return contextMemSize ;
}

void* TIDL_getContextMemRec(void* handle)
{
  void *addr = NULL ;
  uint32_t idx, objIdx ;
  for(idx = 0 ; idx < TIDL_MAX_PRIORITY_LEVEL ; idx++ )
  {
    for(objIdx = 0 ; objIdx < (uint32_t)TIDL_MAX_OBJECTS_PER_LEVEL ;  objIdx++)
    {
      if(gTIDL_priorityTable[idx].objInfo[objIdx].handle == handle)
      {
        //0th entry holds the main context memory
        addr = &(gTIDL_priorityTable[idx].objInfo[0].memRec) ;
        break ;
      }
    }
  }
  tidl_printf(PREEMPT_PRINT_LVL,"PREEMPTION: Requesting context memory addr for handle %p, return Addr = %p\n",
    handle,addr);
  return addr ;
}


int32_t TIDL_addPriorityObject(
  void *handle, const IALG_MemRec *pMemRec, int32_t priorityId)
{

  int32_t priorityIndex = -1, objIdx = 0 ;
  int32_t status = IALG_EOK ;
  TIDL_priorityWiseTable_t *pPriorityInfo = NULL ;
  TIDL_priorityObject_t *pPriorityObj = NULL;

  TIDL_resetPriorityDataBase() ;
  priorityIndex = TIDL_getPriorityIndex(priorityId);

  //If not exhausted then use one entry otherwise return error
  if((priorityIndex == -1))
  {
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if (gTIDL_activePriorityLevels < TIDL_MAX_PRIORITY_LEVEL)
    /* LDRA_JUSTIFY_END */
    {
      priorityIndex = gTIDL_activePriorityLevels ;
      gTIDL_activePriorityLevels++ ;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    else
    {
      /* tidl_printf(0, "Error: Number of priority level exceeds than %d\n", TIDL_MAX_PRIORITY_LEVEL );*/
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_COMMON_EXCEED_PRIORITY_LEVEL);
      status = IALG_EFAIL;
    }
    /* LDRA_JUSTIFY_END */
  }
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if(status == IALG_EOK)
  /* LDRA_JUSTIFY_END */
  {
    pPriorityInfo = &gTIDL_priorityTable[priorityIndex];
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    for(objIdx = 0 ; objIdx < TIDL_MAX_OBJECTS_PER_LEVEL ; objIdx++)
    /* LDRA_JUSTIFY_END */
    {
      if(pPriorityInfo->objInfo[objIdx].alive == 0)
      {
        break ;
      }
      else
      {
        /*Do Nothing*/
      }
    }
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if(objIdx < TIDL_MAX_OBJECTS_PER_LEVEL)
    /* LDRA_JUSTIFY_END */
    {
      pPriorityObj = &pPriorityInfo->objInfo[objIdx];
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    else
    {
      /* tidl_printf(0, "Error: Number of objects exceeds the limit of %d for priority ID %d\n", TIDL_MAX_OBJECTS_PER_LEVEL, priorityId); */
      TIDL_LOG_ERROR(TIDL_ERROR_GROUP_COMMON, TIDL_ERROR_COMMON_EXCEED_OBJECTS_PER_LEVEL);
      status = IALG_EFAIL ;
    }
    /* LDRA_JUSTIFY_END */
  }
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if(status == IALG_EOK)
  /* LDRA_JUSTIFY_END */
  {
    pPriorityInfo->priorityId = priorityId ;
    pPriorityInfo->numObjects++;
    pPriorityObj->alive = 1;
    pPriorityObj->memRec = *pMemRec;
    pPriorityObj->handle = handle ;
  }

  tidl_printf(PREEMPT_PRINT_LVL,"PREEMPTION: Adding a new priority object for targetPriority = %d, handle = %p\n"
    ,priorityId, handle);
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if ((pPriorityInfo != NULL) && (pPriorityObj != NULL))
  /* LDRA_JUSTIFY_END */
  {
    tidl_printf(PREEMPT_PRINT_LVL,"PREEMPTION: Now total number of priority objects = %d at priorityId = %d,\
    with new memRec of base = %p and size = %d\n" ,pPriorityInfo->numObjects, pPriorityInfo->priorityId,
    pPriorityObj->memRec.base,pPriorityObj->memRec.size);
  }
  return status ;
}

int32_t TIDL_removePriorityObject( void *handle, IALG_MemRec *pMemRec)
{
  int32_t status = IALG_EOK ;

  int32_t objIdx = 0;
  uint32_t idx = 0;
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if(gTIDL_firstObject != 0)
  {
      tidl_printf(0, "Error: Trying to remove the pririoty object without initializing\n" );
      status = IALG_EFAIL ;
  }
  else
  /* LDRA_JUSTIFY_END */
  {
    //Do Nothing
  }
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if(status != IALG_EFAIL)
  /* LDRA_JUSTIFY_END */
  {
    for(idx = 0 ; idx < TIDL_MAX_PRIORITY_LEVEL ; idx++ )
    {
      for(objIdx = 0 ; objIdx < TIDL_MAX_OBJECTS_PER_LEVEL ;  objIdx++)
      {
        if(gTIDL_priorityTable[idx].objInfo[objIdx].handle == handle)
        {
          break ;
        }
        else
        {
          /*Do Nothing*/
        }
      }
      if(objIdx < TIDL_MAX_OBJECTS_PER_LEVEL)
      {
        break;
      }
      else
      {
        /*Do Nothing*/
      }
    }
  }
  if(idx == TIDL_MAX_PRIORITY_LEVEL)
  {
    tidl_printf(0, "Error: handle (%p) doesn't exist in priority table\n", handle);
    status = IALG_EFAIL ;
  }
  else
  {
    TIDL_priorityWiseTable_t *pPriorityInfo = &gTIDL_priorityTable[idx];
    TIDL_priorityObject_t *pPriorityObj = &pPriorityInfo->objInfo[objIdx];
    //Free and return the mem record
    pPriorityInfo->numObjects--;
    //Check if there are still left over objects and 0th is being freedup
    //if so then swap 0th indexed obj with  other alive object because
    // need to keep the object with memory at 0th index
    if((objIdx == 0) && (pPriorityInfo->numObjects != 0))
    {
      void *tmpHandle ;
      TIDL_priorityObject_t *firstAliveObj =
        TIDL_getPriorityObject(pPriorityInfo,TIDL_ALIVE_EXCLUDING_FIRST);
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      if(firstAliveObj != NULL)
      /* LDRA_JUSTIFY_END */
      {
      tmpHandle = firstAliveObj->handle ;
      firstAliveObj->handle = pPriorityObj->handle;
      pPriorityObj->handle = tmpHandle ;
      pPriorityObj = firstAliveObj;
      }
    }
    pPriorityObj->alive = 0 ;
    pPriorityObj->handle = NULL;
    *pMemRec = pPriorityObj->memRec ;
    tidl_printf(PREEMPT_PRINT_LVL,"PREEMPTION: Removing priroty object with handle = %p and targetPriority = %d,\
      Number of obejcts left are = %d, removed object with base  = %p and size =%d\n",
      handle, pPriorityInfo->priorityId, pPriorityInfo->numObjects,
      pMemRec->base,pMemRec->size);
    if(pPriorityInfo->numObjects == 0)
    {
      pPriorityInfo->priorityId = -1 ;
      gTIDL_activePriorityLevels--;
    }
  }

  return status;
}

static TIDL_priorityObject_t* TIDL_getPriorityObject(TIDL_priorityWiseTable_t *pPriorityInfo, int32_t searchType)
{
  TIDL_priorityObject_t* pPriorityObj = NULL;
  int32_t objIdx = 0;
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if(searchType == (int32_t)TIDL_ALIVE_EXCLUDING_FIRST )
  /* LDRA_JUSTIFY_END */
  {
    objIdx = 1 ;
  }
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  for(; objIdx < TIDL_MAX_OBJECTS_PER_LEVEL; objIdx++)
  /* LDRA_JUSTIFY_END */
  {
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if(((uint32_t)pPriorityInfo->objInfo[objIdx].alive != 0U) && (searchType == (int32_t)TIDL_ALIVE_EXCLUDING_FIRST) )
    {
      /* LDRA_JUSTIFY_END */
      break ;
    }
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    else
    {
      /*Do Nothing*/
    }
     /* LDRA_JUSTIFY_END */
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if((pPriorityInfo->objInfo[objIdx].alive ==0) && (searchType == (int32_t)TIDL_FIRST_NON_ALIVE) )
    {
      break ;
    }
    /* LDRA_JUSTIFY_END */
    else
    {
      /*Do Nothing*/
    }

  }
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if(objIdx < TIDL_MAX_OBJECTS_PER_LEVEL)
  /* LDRA_JUSTIFY_END */
  {
      pPriorityObj = &pPriorityInfo->objInfo[objIdx];
  }
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  else
  {
    /*Do Nothing*/
  }
  /* LDRA_JUSTIFY_END */
  return pPriorityObj;
}

static int32_t TIDL_getPriorityIndex(int32_t priorityId)
{
  int32_t priorityIndex = -1;
  //Latch to right priority
  for(uint32_t idx = 0 ; idx < TIDL_MAX_PRIORITY_LEVEL ; idx++ )
  {
    if(gTIDL_priorityTable[idx].priorityId == priorityId )
    {
      priorityIndex = idx ;
      break ;
    }
  }
  return priorityIndex ;
}

uint32_t TIDL_getHandleSizeForPreemption(int32_t numLayers)
{
  return (sizeof(TIDL_preEmptionObj)*(uint32_t)numLayers);
}

int32_t TIDL_checkIfPreEmptionEnabled(const TIDL_CreateParams * createParams)
{
  int32_t isPreEmptionEnable = 0;
#ifdef HOST_EMULATION
  if(((uint32_t)createParams->flowCtrl & TIDL_FLOW_CTRL_REF_ONLY) == 0U)
#endif
  {
    if(((createParams->pFxnLock != NULL) && (createParams->pFxnUnLock != NULL)))
    {
      isPreEmptionEnable = 1;
    }
  }

  #ifdef DEBUG_PREEMPTION_HOST_EMULATION
  isPreEmptionEnable = 1;
  #endif
  
  #ifdef __C7604__
  isPreEmptionEnable = 0; //Giving Loki error when enabling preeemption
  #endif

  return isPreEmptionEnable;
}

static void TIDL_sortLayersByCtxSize(TIDL_preEmptionObj* pSortedPreEmptObj, const TIDL_preEmptionObj* handle, int32_t numLayers)
{
  /* Assign from Scratch here */
  TIDL_preEmptionObj *tmpPreEmptObj;
  tmpPreEmptObj = (TIDL_preEmptionObj *)((uint8_t*)pSortedPreEmptObj + TIDL_getHandleSizeForPreemption(numLayers));

  /* This for loop will sort layers in asending order by context size */
  int32_t i, j, minIndx = 0, minCtxSize ;
  for (j = 0; j < numLayers; j++)
  {
    minCtxSize = INT32_MAX;
    for (i = 0; i < numLayers; i++)
    {
      if (pSortedPreEmptObj[i].ctxSize < minCtxSize)
      {
        minCtxSize = pSortedPreEmptObj[i].ctxSize;
        minIndx = i;
      }
    }
    tmpPreEmptObj[j].ctxSize = minCtxSize;
    tmpPreEmptObj[j].procTime = pSortedPreEmptObj[minIndx].procTime; /* This is the handle with cumulative time */
    tmpPreEmptObj[j].dataId = handle[minIndx].dataId;
    pSortedPreEmptObj[minIndx].ctxSize = INT32_MAX;
  }
  (void)memcpy((void*)pSortedPreEmptObj, (void*)tmpPreEmptObj, TIDL_getHandleSizeForPreemption(numLayers));
}



void updateAlgLayerPremptInfo(sTIDL_AlgLayer_t *pAlgLayerList, int32_t layerDataId, int32_t numWL)
{
  for(int32_t i = 0 ; i < numWL ; i++)
  {
    if((pAlgLayerList + i)->layerIdx == layerDataId)
    {
      (pAlgLayerList + i)->isOptimalForPreemption = 1 ;
    }
  }
  return ;
}


void TIDL_calculatePreemptionList(void * preEmptHandle, int32_t numLayers, int32_t numWL, float32_tidl threshold, uint8_t* l2Scratch, float32_tidl ddrThroughput,
  int32_t freqMHz, sTIDL_AlgLayer_t *pAlgLayerList)
{
  TIDL_preEmptionObj *handle = (TIDL_preEmptionObj *)preEmptHandle;
  int32_t i, j, indx = -1;
  float32_tidl  curProcTime, minProcTime, activateProcTime;
  /* Below get from scratch memory */
  TIDL_preEmptionObj *sortedPreEmptObj = (TIDL_preEmptionObj *)(l2Scratch);

  /* Initialize sortedPreEmptObj to original handle */
  (void)memcpy((void*)sortedPreEmptObj, (void*)handle, TIDL_getHandleSizeForPreemption(numLayers));

  /* Accumulate layer by layer procTime to check for threshold */
  for(i = 1; i < numLayers; i++)
  {
    sortedPreEmptObj[i].procTime = sortedPreEmptObj[i-1].procTime + handle[i].procTime;
  }

  /* Add cycles for deactivate and restore context memory cycles for procTime */
  for(i = 1; i < numLayers; i++)
  {
    sortedPreEmptObj[i].procTime = sortedPreEmptObj[i].procTime + (((float32_tidl)handle[i].ctxSize/ddrThroughput)/((float32_tidl)freqMHz * 1000.0F));
  }
  activateProcTime = ((float32_tidl)handle[0].ctxSize/ddrThroughput)/((float32_tidl)freqMHz * 1000.0F);
  /* if threshold is more than total procTime then no need to find optimal layers for pre-emption */
  if (threshold < sortedPreEmptObj[numLayers - 1].procTime)
  {
    TIDL_sortLayersByCtxSize(sortedPreEmptObj, handle, numLayers);
    do
    {
      indx = -1;
      minProcTime = FLT_MAX;
      for(j = 0; j < numLayers; j++)
      {
        curProcTime = sortedPreEmptObj[j].procTime;
        if((int64_t)curProcTime > 0)
        {
          /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          <justification end> */
          if (curProcTime <= (threshold - activateProcTime))
          {
            indx = j;
            minProcTime = curProcTime;
            break;
          }
          /* LDRA_JUSTIFY_END */
          else
          {
            if(curProcTime < minProcTime)
            {
              minProcTime = curProcTime;
              indx = j;
            }
          }
        }
      }
      if (indx != -1)
      {
        updateAlgLayerPremptInfo(pAlgLayerList, sortedPreEmptObj[indx].dataId, numWL);
        /* Recalculate procTime after finding one preemption layer */
        for(j = 0; j < numLayers; j++)
        {
          sortedPreEmptObj[j].procTime -= minProcTime;
        }
        /* Subtarct this activateProcTime from breathing time (threshold) to
           account these activate cycles in the pre-emption list calculation */
        activateProcTime = ((float32_tidl)sortedPreEmptObj[indx].ctxSize/ddrThroughput)/((float32_tidl)freqMHz * 1000.0F);
      }
    }while(indx != -1);
  }
}

void TIDL_initializeHandleForPreemption(void * preEmptHandle,
                                        const sGCHelperHandle  *gcHelperHandle, const sPerfSim_t * perfSimOutput,
     int32_t numLayers, int32_t numWL, float32_tidl threshold, uint8_t* l2Scratch, float32_tidl ddrThroughput, int32_t freqMHz,
     void *pAlgLayerList)
{
  int32_t i, index, nextLyr;
  int32_t outputDataSize;
  TIDL_preEmptionObj *handle = (TIDL_preEmptionObj *)preEmptHandle;
  sBufParams_t *pBufList = gcHelperHandle->pBufDB->pBufList, *bufInfo ;
  int32_t bufDBIndex ;
  tidl_printf(PREEMPT_PRINT_LVL, "Print preEmption Hnadle during init stage :\n" );
  tidl_printf(PREEMPT_PRINT_LVL, "%6s, %12s, %12s\n" ,"ProcTime", "ctxSize", "dataId");

  for (i = 0; i < (numLayers-1); i++)
  {
    handle[i].procTime = perfSimOutput->procTime[i];
    handle[i].dataId = perfSimOutput->layerExecutionOrder[i];

    outputDataSize = TIDL_privGetContextSize() + TIDL_getHandleSizeForPreemption(numLayers);
    nextLyr = i+1;
    index = 0;
    int32_t flag = 1;
    do
    {
      bufDBIndex = perfSimOutput->bufDBIndex[nextLyr][index] ;
      if(bufDBIndex == NOT_VALID)
      {
        flag = 0; // This set is avoid justification of flag == 1 case
      }
      if(flag != 0) {
      bufInfo = &pBufList[bufDBIndex];
      if ( bufInfo->space != (int32_t)DDR ) // Do not backup DDR buffer
      {
        outputDataSize += bufInfo->bufSize;
      }
      index++;
      }
    } while (flag == 1);
    handle[i].ctxSize = outputDataSize;
    tidl_printf(PREEMPT_PRINT_LVL, "%2.3f, %12d, %12d\n" ,handle[i].procTime, handle[i].ctxSize, handle[i].dataId);
  }
  /* For last layer there is no next layer to get required context size, so handling outside the loop */
  handle[i].procTime = perfSimOutput->procTime[i];
  handle[i].ctxSize = 0;
  handle[i].dataId = perfSimOutput->layerExecutionOrder[i];

  tidl_printf(PREEMPT_PRINT_LVL, "%2.3f, %12d, %12d\n" ,handle[i].procTime, handle[i].ctxSize, handle[i].dataId);

  TIDL_calculatePreemptionList((void*)preEmptHandle, numLayers, numWL, threshold, l2Scratch, ddrThroughput, freqMHz, (sTIDL_AlgLayer_t*)pAlgLayerList);
}

void TIDL_updateLayerCyclesForPreemption(void * preEmptHandle, int32_t perfHW, uint64_t cycles, int32_t layerId, int32_t procType, int32_t freqMHz)
{
  TIDL_preEmptionObj *handle = (TIDL_preEmptionObj *)preEmptHandle;
  float32_tidl procTime;
  if(perfHW == 1)
  {
    /* convert cycles to time in ms and store */
    procTime = (float32_tidl)cycles/((float32_tidl)freqMHz * 1000.0F);
    if (procType == 0)
    {
      handle[layerId].procTime = procTime;
    }
    else /* To handle ST case */
    {
      if(procTime > handle[layerId].procTime)
      {
        handle[layerId].procTime = procTime;
      }
    }
  }
}

void TIDL_reCalculateListForPreemption(void * preEmptHandle, int32_t *perfHW, int32_t numLayers, int32_t numWL, float32_tidl threshold,
  uint8_t* l2Scratch, float32_tidl ddrThroughput, int32_t freqMHz,
  void *pAlgLayerList)
{
  TIDL_preEmptionObj *handle = (TIDL_preEmptionObj *)preEmptHandle;
  if(*perfHW == 1)
  {
    *perfHW = 0;
    TIDL_calculatePreemptionList((void*)handle, numLayers, numWL, threshold, l2Scratch, ddrThroughput, freqMHz, (sTIDL_AlgLayer_t*)pAlgLayerList);
  }
}

int32_t TIDL_lockForPreemption(void* algHandle, const TIDL_CreateParams * createParams, int32_t *plockState)
{
  int32_t oldIntState = 0;
#ifdef TIDL_COVERAGE_DEAD_CODE
  int32_t relativeCoreId = GET_RELATIVE_COREIDX(createParams->coreId, createParams->coreStartIdx);
#endif
  #ifdef DEBUG_PREEMPTION_HOST_EMULATION
  (void)TIDL_activate(gHandleForPremptTest);
  #endif
  /* LDRA_JUSTIFY_START
  <metric start> branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  <justification end> */
  if (*plockState != LOCKED)
   /* LDRA_JUSTIFY_END */
  {
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if (createParams->pFxnLock != NULL)
    /* LDRA_JUSTIFY_END */
    {
      tidl_printf(PREEMPT_PRINT_LVL, "PREEMPTION: Requesting LOCK for priroty object with handle = %p and targetPriority %d\n", algHandle, createParams->targetPriority);
      oldIntState = createParams->pFxnLock();
      tidl_printf(PREEMPT_PRINT_LVL, "PREEMPTION: Request of LOCK for priroty object with handle = %p and targetPriority %d is serviced with state %d\n", algHandle, createParams->targetPriority, oldIntState);
      *plockState = LOCKED;
    }
#ifdef TIDL_COVERAGE_DEAD_CODE
    else if (TIDL_getLockFxnPtr(relativeCoreId) != NULL)
    {
      tidl_printf(PREEMPT_PRINT_LVL, "PREEMPTION: (Default) Requesting LOCK for priroty object with handle = %p and targetPriority %d\n", algHandle, createParams->targetPriority);
      oldIntState = TIDL_getLockFxnPtr(relativeCoreId)();
      tidl_printf(PREEMPT_PRINT_LVL, "PREEMPTION: (Default) Request of LOCK for priroty object with handle = %p and targetPriority %d is serviced with state %d\n", algHandle, createParams->targetPriority, oldIntState);
      *plockState = LOCKED;
    }
#endif
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    else
    {
      //Do Nothing
    }
    /* LDRA_JUSTIFY_END */
  }
  TIDL_switchHandles(algHandle);
  return oldIntState;
}
void TIDL_unlockForPreemption(int32_t oldIntState, const TIDL_CreateParams * createParams, int32_t *plockState)
{
#ifdef TIDL_COVERAGE_DEAD_CODE
  int32_t relativeCoreId = GET_RELATIVE_COREIDX(createParams->coreId, createParams->coreStartIdx);
#endif
  if (*plockState != UNLOCKED)
  {
    /* LDRA_JUSTIFY_START
    <metric start> branch <metric end>
    <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
    <justification end> */
    if (createParams->pFxnUnLock != NULL)
    /* LDRA_JUSTIFY_END */
    {
      *plockState = UNLOCKED;
      tidl_printf(PREEMPT_PRINT_LVL, "PREEMPTION: Requesting UNLOCK for priroty object and targetPriority %d\n", createParams->targetPriority);
      createParams->pFxnUnLock(oldIntState);
    }
#ifdef TIDL_COVERAGE_DEAD_CODE
    else if (TIDL_getUnLockFxnPtr(relativeCoreId) != NULL)
    {
      *plockState = UNLOCKED;
      tidl_printf(PREEMPT_PRINT_LVL, "PREEMPTION: (Default) Requesting UNLOCK for priroty object and targetPriority %d\n", createParams->targetPriority);
      TIDL_getUnLockFxnPtr(relativeCoreId)(oldIntState);
    }
    else
    {
      //Do Nothing
    }
#endif    
  }
  #ifdef DEBUG_PREEMPTION_HOST_EMULATION
  TIDL_deactivate(gHandleForPremptTest);

  sPerfSim_t * perfInfoOut;
  TIDL_Handle algHandle = (TIDL_Handle)(void*)(gHandleForPremptTest);
  int32_t msmcSramSize, l2SramSize;
  perfInfoOut =  (sPerfSim_t *) TIDL_getNetObjBufPtr( createParams->net, algHandle->memRec, TIDL_NET_COMPILER_BUF);
  msmcSramSize = (perfInfoOut->simConfig.sizeL3MemKB - L3MEM_SCRATCH_BUF_SIZEKB) * 1024;
  l2SramSize   = (perfInfoOut->simConfig.sizeL2MemKB - L2MEM_SCRATCH_BUF_SIZEKB)* 1024;


  (void)memset( algHandle->sysScratchPtr.l2BasePtr[0], 0xCD , l2SramSize);
  {
    uint8_t* tempSrc = ( uint8_t *)algHandle->sysScratchPtr.l3BasePtr[0];
    uint32_t tempSize = msmcSramSize ;
    (void)memset( tempSrc, 0xCD ,tempSize );
    tidl_printf( PREEMPT_PRINT_LVL , "Corrupting MSMC at address %p of size %d bytes\n", tempSrc, tempSize );
  }
  #endif

  tidl_printf(PREEMPT_PRINT_LVL, "PREEMPTION: Requesting UNLOCK for priroty object and targetPriority %d is serviced\n", createParams->targetPriority);

}

void TIDL_giveChanceForPreemption(void* algHandle, int32_t *plockState, int32_t * oldIntState)
{
  TIDL_Handle algHandlePtr = (TIDL_Handle)algHandle;
  if (algHandlePtr->isPreEmptionEnable != 0)
  {
    TIDL_unlockForPreemption(*oldIntState, algHandlePtr->createParams, plockState);
    *oldIntState = TIDL_lockForPreemption(algHandlePtr, algHandlePtr->createParams, plockState);
  }
}


int32_t TIDL_updateContextMemPtrsForPreemption(TIDL_preEmptContextObj  *contextInfoPtr, const sGCHelperHandle  *gcHelperHandle, void* privContext,
  void* preEmptHandle, const sPerfSim_t * perfSimOutput, uint8_t *l2BasePtr, uint8_t* l3BasePtr, int32_t lyrNum, int32_t numLayers)
{
  int32_t status          = IALG_EOK;
  uint8_t *contextMemPtr  = contextInfoPtr->contextMemPtr;
  contextInfoPtr->numContextBufs = 0;
  /* Backup of preEmptHandle to restore after pre-emption */
  contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].src = (uint8_t *) preEmptHandle;
  contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].dst = (uint8_t *) contextMemPtr;
  contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].size = TIDL_getHandleSizeForPreemption(numLayers);
  contextMemPtr += ALIGN_SIZE(TIDL_getHandleSizeForPreemption(numLayers), 128U);
  contextInfoPtr->numContextBufs++;

  if(contextInfoPtr->earlyPreEmpt != 0)
  {
    /* Backup of privContext to restore layer index after pre-emption */
    contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].src = (uint8_t *) privContext;
    contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].dst = (uint8_t *) contextMemPtr;
    contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].size = TIDL_privGetContextSize();
    contextMemPtr += ALIGN_SIZE(TIDL_privGetContextSize(), 128U);
    contextInfoPtr->numContextBufs++;


    TIDL_preEmptionObj *handle = (TIDL_preEmptionObj *)preEmptHandle;
    /* LDRA_JUSTIFY_START
    <metric start> statement branch <metric end>
    <justification start> DEBUG_TRACE: This code is for debugging purpose.
    The condition check is deterministic and does not impact the safety or reliability of the system.
    Therefore, it is excluded from safety coverage requirements.
    <justification end> */
    if(lyrNum == 0){
      tidl_printf(PREEMPT_PRINT_LVL,"%8s,%8s,%8s,%8s", "Data_idx", "procTime", "ctxSize-total", "preEmptLayerId");
      tidl_printf(PREEMPT_PRINT_LVL,",(%8s|%8s|%8s)\n", "int_ctxt_ptr", "ext_ctxt_ptr", "ctxtSize-part");
    }
    /* LDRA_JUSTIFY_END */
    tidl_printf(PREEMPT_PRINT_LVL,"%8d,%2.5f,%8d", handle->dataId, handle->procTime, handle->ctxSize);

    /* Backup the context memory of the holding, input layers and ST context mem for pre-emption */
    {
      uint8_t * outPtr;
      int32_t index = 0;

      sBufParams_t *pBufList = gcHelperHandle->pBufDB->pBufList, *bufInfo ;
      int32_t bufDBIndex ;
      int32_t flag = 1;
      /* LDRA_JUSTIFY_START
      <metric start> branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      if(lyrNum != NOT_VALID)
      /* LDRA_JUSTIFY_END */
      {
        do
        {
          bufDBIndex = perfSimOutput->bufDBIndex[lyrNum][index] ;
          if(bufDBIndex == NOT_VALID)
          {
            flag = 0;
          }
          if(flag !=0) {
          bufInfo = &pBufList[bufDBIndex];
          /* LDRA_JUSTIFY_START
          <metric start> statement branch <metric end>
          <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
          <justification end> */
          if (( bufInfo->space == (int32_t)L2 ) || ( bufInfo->space == (int32_t)MSMC )) // Do not backup DDR buffer
          /* LDRA_JUSTIFY_END */
          {
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
            <justification end> */
            if ( bufInfo->space == (int32_t)L2 )
            {
              outPtr = &(l2BasePtr[bufInfo->baseMem]);
            }
            else if ( bufInfo->space == (int32_t)MSMC )
            /* LDRA_JUSTIFY_END */
            {
              outPtr = &(l3BasePtr[bufInfo->baseMem]);
            }
            /* LDRA_JUSTIFY_START
            <metric start> statement branch <metric end>
            <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data. This else block is to maintained for MISRAC coding guidelines.
            The condition check is deterministic and does not impact the safety or reliability of the system.\
            Therefore, it is excluded from safety coverage requirements.
            <justification end> */
            else
            {
              /*do nothing*/
            }
            /* LDRA_JUSTIFY_END */
            contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].src = (uint8_t *) outPtr;
            contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].dst = (uint8_t *) contextMemPtr;
            contextInfoPtr->contextBufInfo[contextInfoPtr->numContextBufs].size = bufInfo->bufSize;
            contextMemPtr += ALIGN_SIZE((uint32_t)bufInfo->bufSize, 128U);
            contextInfoPtr->numContextBufs++;

            tidl_printf(PREEMPT_PRINT_LVL,",(%p|%p|%8d)", outPtr, contextMemPtr, bufInfo->bufSize);
          }
          index++;
          }
        } while (flag == 1);
      }
     tidl_printf(PREEMPT_PRINT_LVL,"\n");
    }
  }
  /* LDRA_JUSTIFY_START
  <metric start> statement branch <metric end>
  <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
  Here bufInfo->space cannot be equal to ( int32_t ) L2.
  The condition check is deterministic and does not impact the safety or reliability of the system. \
  Therefore, it is excluded from safety coverage requirements.
  <justification end> */
  if (((uintptr_t)contextMemPtr - (uintptr_t)contextInfoPtr->contextMemPtr ) > contextInfoPtr->contextMemSize)
  {
    tidl_printf(0, "Insufficient memory to backup context buffers for pre-emption lyrNum %d \n", lyrNum);
    status = TIDL_ERR_FAILURE;
  }
  /* LDRA_JUSTIFY_END */
  return status;
}

void  TIDL_restoreContextMemToIntMemForPreemption(const TIDL_preEmptContextObj  *contextInfoPtr,
      void *dmaUtilsContext, uint8_t * trMem, const TIDL_CreateParams * createParams)
{

  {
    int32_t contextMemSize, tempBufHeight;
    for(int32_t i = 0; i < contextInfoPtr->numContextBufs; i++)
    {
      contextMemSize = contextInfoPtr->contextBufInfo[i].size;
      tempBufHeight = (int32_t)contextMemSize/UINT16_MAX;
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      if(contextMemSize > UINT16_MAX)
      {
          (void)TIDL_memcpy2D(contextInfoPtr->contextBufInfo[i].src,
                        contextInfoPtr->contextBufInfo[i].dst,
                        UINT16_MAX,
                        (uint16_t)tempBufHeight,
                        UINT16_MAX,
                        UINT16_MAX,
                        dmaUtilsContext,
                        trMem);
          contextMemSize = contextMemSize - ((contextMemSize/UINT16_MAX)*UINT16_MAX);
      }
      if((uint32_t)contextMemSize > 0U)
      /* LDRA_JUSTIFY_END */
      {
          (void)TIDL_memcpy2D((uint8_t *)contextInfoPtr->contextBufInfo[i].src + (tempBufHeight*UINT16_MAX),
                        (uint8_t *)contextInfoPtr->contextBufInfo[i].dst + (tempBufHeight*UINT16_MAX),
                        (uint16_t)contextMemSize,
                        1,
                        1,
                        1,
                        dmaUtilsContext,
                        trMem);
      }
    }

    (void)TIDL_initDebugTraceParams(createParams->traceLogLevel,
              createParams->traceWriteLevel,
              createParams->TIDLVprintf,
              createParams->TIDLWriteBinToFile,
              createParams->TIDLReadBinFromFile,
              createParams->traceBaseName);
  }
}

void  TIDL_storeContextMemToExtMemForPreemption(TIDL_preEmptContextObj  *contextInfoPtr,
      void *dmaUtilsContext, uint8_t * trMem)
{
  {
    int32_t contextMemSize, tempBufHeight;
    for(int32_t i = 0; i < contextInfoPtr->numContextBufs; i++)
    {
      contextMemSize = contextInfoPtr->contextBufInfo[i].size;
      tempBufHeight = (int32_t)contextMemSize/UINT16_MAX;
      /* LDRA_JUSTIFY_START
      <metric start> statement branch <metric end>
      <justification start> SAFETY_CHECK: Safe programming hard to hit this condition with real world data.
      <justification end> */
      if(contextMemSize > UINT16_MAX)
      {
          (void)TIDL_memcpy2D(contextInfoPtr->contextBufInfo[i].dst,
                        contextInfoPtr->contextBufInfo[i].src,
                        UINT16_MAX,
                        (uint16_t)tempBufHeight,
                        UINT16_MAX,
                        UINT16_MAX,
                        dmaUtilsContext,
                        trMem);

          #ifdef DEBUG_PREEMPTION_HOST_EMULATION
          (void)memset(contextInfoPtr->contextBufInfo[i].src, '0xCD', UINT16_MAX );
          #endif
          contextMemSize = contextMemSize - ((contextMemSize/UINT16_MAX)*UINT16_MAX);
      }
      if((uint32_t)contextMemSize > 0U)
      /* LDRA_JUSTIFY_END */
      {
          (void)TIDL_memcpy2D((uint8_t *)contextInfoPtr->contextBufInfo[i].dst + (tempBufHeight*UINT16_MAX),
                        (uint8_t *)contextInfoPtr->contextBufInfo[i].src + (tempBufHeight*UINT16_MAX),
                        (uint16_t)contextMemSize,
                        1,
                        1,
                        1,
                        dmaUtilsContext,
                        trMem);

          #ifdef DEBUG_PREEMPTION_HOST_EMULATION
          (void)memset( (void *)((uint8_t *)contextInfoPtr->contextBufInfo[i].src) + tempBufHeight*UINT16_MAX, '0xCD', contextMemSize );
          tidl_printf( PREEMPT_PRINT_LVL, "Corrupting at address %p after backign up to address %p of size %d bytes\n",
                      (uint8_t *)contextInfoPtr->contextBufInfo[i].src + tempBufHeight*UINT16_MAX,
                      (uint8_t *)contextInfoPtr->contextBufInfo[i].dst + tempBufHeight*UINT16_MAX,
                      (uint16_t)contextMemSize );
          #endif
      }
    }
  }
}
#endif
