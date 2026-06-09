/*
 *
 * Copyright (c) 2024-2025 Texas Instruments Incorporated
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

/*!
 * \file ethfw_ipc.c
 *
 * \brief Ethernet Firmware IPC abstraction layer for Sitara
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <utils/ethfw_common/include/ethfw_types.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>

#include <kernel/dpl/ClockP.h>
#include <drivers/ipc_rpmsg.h>
#include <drivers/ipc_rpmsg/include/ipc_rpmsg_linux_resource_table.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x505

/* Excluding A53SS0_0 Maximum two cores are enabled for IPC RPMessage
 * (R5FSS0_0 & MCU_R5FSS0_0) */
#define ETHFW_IPC_MAX_NUM_CPUS                  (2U)
#define ETHFW_IPC_VRING_MAX_NUM                 (ETHFW_IPC_MAX_NUM_CPUS*(ETHFW_IPC_MAX_NUM_CPUS-1))
#define ETHFW_IPC_VRING_MAX_NUM_BUF             (8U)
#define ETHFW_IPC_VRING_MAX_MSG_SIZE            (128U)

#define ETHFW_IPC_VRING_SIZE                    RPMESSAGE_VRING_SIZE(ETHFW_IPC_VRING_MAX_NUM_BUF, ETHFW_IPC_VRING_MAX_MSG_SIZE)

#define ETHFW_IPC_RPMSG_ENDPOINT_NAME_SIZE      (32U)

#define ETHFW_IPC_ENDPT_LOOKUP_SLEEP_US         (10000)

#define ETHFW_IPC_MAX_ANNOUNCE_ENDPT            (8U)
#define ETHFW_IPC_FREERTOS_MAX_ENDPT            (5U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct EthFwOsal_Rpmsg_s
{
    bool    used;
    RPMessage_Object RpmsgObj;
} EthFwOsal_Rpmsg;

typedef struct EthFwIpc_remoteProc_s
{
    uint32_t remoteProc[CSL_CORE_ID_MAX];
    uint32_t numProc;
} EthFwIpc_remoteProc;

typedef struct EthFwIpc_endPtTable_s
{
    bool isAnnounced;
    char name[ETHFW_IPC_RPMSG_ENDPOINT_NAME_SIZE];
    uint32_t remoteProcId;
    uint32_t remoteEndPt;
} EthFwIpc_endPtTable;

typedef struct EthFwIpc_Obj_s
{
    bool isMutexValid;
    EthFwOsal_MutexHandle hMutex;
    uint32_t allocCnt;
    uint32_t peakCnt;
    uint32_t selfCoreId;
    EthFwIpc_remoteProc remoteCores;
    EthFwIpc_endPtTable endPtTable[ETHFW_IPC_MAX_ANNOUNCE_ENDPT];
} EthFwIpc_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void EthFwIpc_ctrlEndPtCb(void *arg,
                          uint16_t remoteCoreId,
                          uint16_t remoteEndPt,
                          const char *remoteServiceName);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static EthFwIpc_Obj gEthFwIpcObj;

/* global pool of statically allocated rpmsg object pools */
static EthFwOsal_Rpmsg gEthFwOsalRpmsgfreertosPool[ETHFW_IPC_FREERTOS_MAX_ENDPT];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwIpc_init(uint32_t selfId,
                      uint16_t numProc,
                      uint32_t procArray[IPC_MAX_PROCS],
                      void (*func)(const char *str))
{
    int32_t status = SystemP_SUCCESS;
    uint32_t i = 0U;
    static bool ipcInitDone = BFALSE;

    if (!ipcInitDone)
    {
        gEthFwIpcObj.hMutex = EthFwOsal_createMutex();

        if (gEthFwIpcObj.hMutex != NULL)
        {
            gEthFwIpcObj.isMutexValid = BTRUE;
        }

        memset(&gEthFwOsalRpmsgfreertosPool[0], 0 , sizeof(gEthFwOsalRpmsgfreertosPool));

        ipcInitDone = BTRUE;
    }

    for (i = 0U; i < numProc; i++)
    {
        gEthFwIpcObj.remoteCores.remoteProc[i] = procArray[i];
        gEthFwIpcObj.remoteCores.numProc++;
    }

    if (gEthFwIpcObj.remoteCores.numProc == 0U)
    {
        status = SystemP_FAILURE;
        ETHFWTRACE_ERR(status, "EthFwIpc_init() Failed");
    }

    return status;
}

int32_t EthFwIpc_initVirtIO(uint16_t numProc,
                            void *vqObj,
                            void *vringAddr,
                            uint32_t vringBufSize)
{
    int32_t status = SystemP_SUCCESS;

    /* Do Nothing */

    return status;
}

void EthFwIpc_resetObj()
{
    memset(&gEthFwIpcObj, 0, sizeof(EthFwIpc_Obj));
}

void EthFwIpc_enableControlEndPt()
{
    RPMessage_controlEndPtCallback(EthFwIpc_ctrlEndPtCb, NULL);
}

int32_t EthFwIpc_initRpmsg(void *rpmsgBuff, void *taskBuff, uint32_t selfCoreId)
{
    int32_t status = SystemP_SUCCESS;

    gEthFwIpcObj.selfCoreId = selfCoreId;
    RPMessage_controlEndPtCallback(EthFwIpc_ctrlEndPtCb, NULL);

    return status;
}

int32_t EthFwIpc_isRemoteReady(uint32_t coreId, uint32_t timeout)
{
    int32_t status = ENET_EBADARGS;

    status = RPMessage_waitForLinuxReady(timeout);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "RPMessage_waitForLinuxReady Failed");

    return status;
}

int32_t EthFwIpc_lateInit(uint32_t coreId)
{
    int32_t status = SystemP_SUCCESS;

    /* Do Nothing */

    return status;
}

void EthFwIpc_initRpmsgParams(EthFwIpc_RpmsgCreateParams *params)
{
    params->requestedEndpt = 0U;
}

EthFwIpc_RpmsgHandle EthFwIpc_createRpmsg(EthFwIpc_RpmsgCreateParams *params)
{
	EthFwIpc_RpmsgHandle retHandle = NULL;
	EthFwOsal_Rpmsg *handle = NULL;
	EthFwOsal_Rpmsg *rpmsgPool = NULL;
    const uint32_t maxRpmsgCount = ETHFW_IPC_FREERTOS_MAX_ENDPT;
    RPMessage_CreateParams createParams;
    int32_t status = SystemP_SUCCESS;
    uint32_t i = 0U;
    uint32_t index = 0U;

    rpmsgPool = &gEthFwOsalRpmsgfreertosPool[0];

    EnetAppUtils_assert(gEthFwIpcObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwIpcObj.hMutex);

    for (i = 0U; i < maxRpmsgCount; i++)
    {
        if (BFALSE == rpmsgPool[i].used)
        {
            rpmsgPool[i].used = BTRUE;
            index = i;
            /* Update statistics */
            gEthFwIpcObj.allocCnt++;
            if (gEthFwIpcObj.allocCnt > gEthFwIpcObj.peakCnt)
            {
                gEthFwIpcObj.peakCnt = gEthFwIpcObj.allocCnt;
            }
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwIpcObj.hMutex);

    if (index < maxRpmsgCount)
    {
        /* Grab the memory */
        handle = &rpmsgPool[index];
    }

    RPMessage_CreateParams_init(&createParams);
    createParams.localEndPt = params->requestedEndpt;

    status = RPMessage_construct(&handle->RpmsgObj, &createParams);

    if(status != SystemP_SUCCESS)
    {
        EthFwOsal_lockMutex(gEthFwIpcObj.hMutex);

        handle->used = BFALSE;
        /* Found the osal event object to delete */
        if (0U < gEthFwIpcObj.allocCnt)
        {
            gEthFwIpcObj.allocCnt--;
        }

        EthFwOsal_unlockMutex(gEthFwIpcObj.hMutex);

        retHandle = NULL;
        ETHFWTRACE_ERR(status, "Failed to allocate RPMessage endpoint");
    }
    else
    {
        retHandle = (EthFwIpc_RpmsgHandle)handle;
    }

    return retHandle;
}

int32_t EthFwIpc_sendRpmsg(EthFwIpc_RpmsgHandle rpmsgHandle,
                           uint32_t dstProcId,
                           uint32_t dstEndPt,
                           uint32_t srcEndPt,
                           void* data,
                           uint32_t len)
{
    int32_t status = SystemP_SUCCESS;

    status = RPMessage_send(data, len, dstProcId, dstEndPt, srcEndPt, ETHFWIPC_WAIT_FOREVER);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "RPMessage_send Failed");

    return status;
}

int32_t EthFwIpc_recvRpmsg(EthFwIpc_RpmsgHandle rpmsgHandle,
                           void *data,
                           uint16_t *len,
                           uint32_t *remoteEndPt,
                           uint32_t *remoteProcId,
                           uint32_t timeout)
{
    int32_t status = SystemP_SUCCESS;
    uint16_t remoteCoreId = 0U;
    EthFwOsal_Rpmsg *handle = (EthFwOsal_Rpmsg *)rpmsgHandle;
    /* set max available len to recv buffer to avoid truncation */
    *len = 512U;

    status = RPMessage_recv(&handle->RpmsgObj, data, len, &remoteCoreId, remoteEndPt, timeout);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "RPMessage_recv Failed");

    *remoteProcId = remoteCoreId;
    return status;
}

void EthFwIpc_ctrlEndPtCb(void *arg,
                          uint16_t remoteCoreId,
                          uint16_t remoteEndPt,
                          const char *remoteServiceName)
{
    uint32_t endPtTableIdx = 0U;

    for (endPtTableIdx = 0; endPtTableIdx < RPMESSAGE_MAX_LOCAL_ENDPT; endPtTableIdx++)
    {
        if (gEthFwIpcObj.endPtTable[endPtTableIdx].isAnnounced == false)
        {
            gEthFwIpcObj.endPtTable[endPtTableIdx].remoteEndPt = remoteEndPt;
            gEthFwIpcObj.endPtTable[endPtTableIdx].remoteProcId = remoteCoreId;
            strncpy(gEthFwIpcObj.endPtTable[endPtTableIdx].name, remoteServiceName, ETHFW_IPC_RPMSG_ENDPOINT_NAME_SIZE);
            gEthFwIpcObj.endPtTable[endPtTableIdx].isAnnounced = true;
            break;
        }
    }

    if (endPtTableIdx == RPMESSAGE_MAX_LOCAL_ENDPT)
    {
        ETHFWTRACE_ERR(SystemP_FAILURE, "Local endpoint table is full, couldn't save the received end point");
    }
}

int32_t EthFwIpc_getRemoteEndPt(uint32_t currProcId,
                                const char* name,
                                uint32_t *remoteProcId,
                                uint32_t *remoteEndPt,
                                uint32_t timeout)
{
    int32_t status = SystemP_FAILURE;
    bool isFound = false;
    uintptr_t key;
    uint32_t endPtTableIdx = 0U;
    
    while (!isFound)
    {
        key = EnetOsal_disableAllIntr();

        for (endPtTableIdx = 0U; endPtTableIdx < RPMESSAGE_MAX_LOCAL_ENDPT; endPtTableIdx++)
        {
            if ((!strcmp(gEthFwIpcObj.endPtTable[endPtTableIdx].name, name))
                  && (gEthFwIpcObj.endPtTable[endPtTableIdx].isAnnounced))
            {
                if (RPMESSAGE_ANY == currProcId || (currProcId == gEthFwIpcObj.endPtTable[endPtTableIdx].remoteProcId))
                {
                    *remoteProcId = gEthFwIpcObj.endPtTable[endPtTableIdx].remoteProcId;
                    *remoteEndPt = gEthFwIpcObj.endPtTable[endPtTableIdx].remoteEndPt;
                    isFound = true;
                    break;
                }
            }
        }

        EnetOsal_restoreAllIntr(key);

        ClockP_usleep(ETHFW_IPC_ENDPT_LOOKUP_SLEEP_US);
    }

    if (isFound)
    {
        status = SystemP_SUCCESS;
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to get remote endpoint");
    }

    return status;
}

int32_t EthFwIpc_announceAll(uint32_t localEp,
                             const char* name)
{
    int32_t status = SystemP_SUCCESS;
    uint32_t remoteProcIdx = 0U;

    for (remoteProcIdx = 0U; remoteProcIdx < gEthFwIpcObj.remoteCores.numProc; remoteProcIdx++)
    {
        if (gEthFwIpcObj.selfCoreId != gEthFwIpcObj.remoteCores.remoteProc[remoteProcIdx])
        {
            status = RPMessage_announce(gEthFwIpcObj.remoteCores.remoteProc[remoteProcIdx],
                                        localEp,
                                        name);
        }

        if (SystemP_SUCCESS != status)
        {
            ETHFWTRACE_ERR(status, "Failed to announce to all remote cores");
            break;
        }
    }

    return status;
}

int32_t EthFwIpc_announce(uint32_t remoteProcId,
                          uint32_t localEp,
                          const char* name)
{
    int32_t status = SystemP_FAILURE;

    status = RPMessage_announce(remoteProcId,
                                localEp,
                                name);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to announce to %u remote core", remoteProcId);

    return status;
}

void EthFwIpc_unblockRpmsg(EthFwIpc_RpmsgHandle rpmsgHandle)
{
	EthFwOsal_Rpmsg *handle = (EthFwOsal_Rpmsg *)rpmsgHandle;
    RPMessage_unblock(&handle->RpmsgObj);
}

int32_t EthFwIpc_deleteRpmsg(EthFwIpc_RpmsgHandle rpmsgHandle)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Rpmsg *handle = (EthFwOsal_Rpmsg *)rpmsgHandle;

    EnetAppUtils_assert(gEthFwIpcObj.isMutexValid == BTRUE);

    RPMessage_destruct(&handle->RpmsgObj);

    EthFwOsal_lockMutex(gEthFwIpcObj.hMutex);

    handle->used = BFALSE;

    if (0U < gEthFwIpcObj.allocCnt)
    {
        gEthFwIpcObj.allocCnt--;
    }
    else
    {
        status = SystemP_FAILURE;
        ETHFWTRACE_ERR(status, "Failed to delete RPMessage endpoint");
    }

    EthFwOsal_unlockMutex(gEthFwIpcObj.hMutex);

    return status;
}
