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
 * \brief Ethernet Firmware IPC implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <utils/ethfw_common/include/ethfw_types.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x503

#define ETHFW_IPC_RPC_MSG_SIZE                 (496U + 32U)
#define ETHFW_IPC_NUM_RPMSG_BUFS               (256U)
#define ETHFW_IPC_RPMSG_OBJ_SIZE               (256U)
#define ETHFW_IPC_DATA_SIZE                    (ETHFW_IPC_RPC_MSG_SIZE * \
                                                ETHFW_IPC_NUM_RPMSG_BUFS + \
                                                ETHFW_IPC_RPMSG_OBJ_SIZE)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t EthFwIpc_init(uint32_t selfId,
                      uint16_t numProc,
                      uint32_t procArray[IPC_MAX_PROCS],
                      void (*func)(const char *str))
{
     int32_t status = ETHFW_SOK;
     Ipc_InitPrms initPrms;

     Ipc_mpSetConfig(selfId, numProc, &procArray[0]);

     IpcInitPrms_init(0U, &initPrms);
     initPrms.printFxn = func;
     status = Ipc_init(&initPrms);

     return status;
}

int32_t EthFwIpc_initVirtIO(uint16_t numProc,
                            void *vqObj,
                            void *vringAddr,
                            uint32_t vringBufSize)
{
    int32_t status = ETHFW_SOK;
    Ipc_VirtIoParams vqParam;

    /* Initialize Virtio */
    vqParam.vqObjBaseAddr = vqObj;
    vqParam.vqBufSize = numProc * Ipc_getVqObjMemoryRequiredPerCore();
    vqParam.vringBaseAddr = vringAddr;
    vqParam.vringBufSize = vringBufSize;
    vqParam.timeoutCnt = 100;     /* Wait for counts */
    status = Ipc_initVirtIO(&vqParam);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to init VirtIO");

    return status;
}

int32_t EthFwIpc_initRpmsg(void *rpmsgBuff, void *taskBuff, uint32_t selfCoreId)
{
    int32_t status = ETHFW_SOK;
    RPMessage_Params cntrlParam;

    /* Initialize RPMessage */
    /* Initialize the param and set memory for HeapMemory for control task */
    RPMessageParams_init(&cntrlParam);
    cntrlParam.buf = rpmsgBuff;
    cntrlParam.bufSize = ETHFW_IPC_DATA_SIZE;
    cntrlParam.stackBuffer = taskBuff;
    cntrlParam.stackSize = IPC_TASK_STACKSIZE;
    status = RPMessage_init(&cntrlParam);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to init RPMessage");

    return status;
}

int32_t EthFwIpc_isRemoteReady(uint32_t coreId, uint32_t timeout)
{
    int32_t status = ETHFW_SOK;

    while (!Ipc_isRemoteReady(IPC_MPU1_0))
    {
        TaskP_sleep(10U);
    }

    return status;
}

int32_t EthFwIpc_lateInit(uint32_t coreId)
{
    int32_t status = ETHFW_SOK;

    /* Create the VRing now ... */
    if (status == ETHFW_SOK)
    {
        /* Create virtio if one hasn't been created already */
        if(!Ipc_isRemoteVirtioCreated(coreId))
        {
            status = Ipc_lateVirtioCreate(coreId);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Late VirtIO init failed");
        }
    }

    /* Late init */
    if (status == ETHFW_SOK)
    {
        status = RPMessage_lateInit(coreId);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Late RPMessage init failed");
    }

    return status;
}

void EthFwIpc_initRpmsgParams(EthFwIpc_RpmsgCreateParams *params)
{
    int32_t status = ETHFW_SOK;

    params->requestedEndpt = RPMESSAGE_ANY;
    params->numBufs        = ETHFW_IPC_NUM_RPMSG_BUFS;
    params->buf            = NULL;
    params->bufSize        = 0U;
}

EthFwIpc_RpmsgHandle EthFwIpc_createRpmsg(EthFwIpc_RpmsgCreateParams *params)
{
    EthFwIpc_RpmsgHandle handle = NULL;
    RPMessage_Params comChParam;
    uint32_t  localEp = 0U;
    int32_t status = ETHFW_SOK;

    RPMessageParams_init(&comChParam);
    comChParam.numBufs = params->numBufs;
    comChParam.buf     = params->buf;
    comChParam.bufSize = params->bufSize;
    comChParam.requestedEndpt = params->requestedEndpt;

    handle = (EthFwIpc_RpmsgHandle)RPMessage_create(&comChParam, &localEp);

    if (NULL == handle)
    {
        status = ETHFW_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to allocate RPMessage endpoint");
    }
    else if (params->requestedEndpt != RPMESSAGE_ANY)
    {
        if (localEp != params->requestedEndpt)
        {
            handle = NULL;
            status = ETHFW_EALLOC;
            ETHFWTRACE_ERR(status, "Failed to allocate requested endpoint, deleting the endpoint");

            status = EthFwIpc_deleteRpmsg(handle);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to delete the endpoint");
        }
    }

    return handle;
}

int32_t EthFwIpc_sendRpmsg(EthFwIpc_RpmsgHandle rpmsgHandle,
                           uint32_t dstProcId,
                           uint32_t dstEndPt,
                           uint32_t srcEndPt,
                           void* data,
                           uint32_t len)
{
    int32_t status = ETHFW_SOK;
    RPMessage_Handle handle = (RPMessage_Handle)rpmsgHandle;

    status = RPMessage_send(handle, dstProcId, dstEndPt, srcEndPt, data, len);
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
    int32_t status = ETHFW_SOK;
    RPMessage_Handle handle = (RPMessage_Handle)rpmsgHandle;

    status = RPMessage_recv(handle, data, len, remoteEndPt, remoteProcId, timeout);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "RPMessage_recv Failed");

    return status;
}

int32_t EthFwIpc_getRemoteEndPt(uint32_t currProcId,
                                const char* name,
                                uint32_t *remoteProcId,
                                uint32_t *remoteEndPt,
                                uint32_t timeout)
{
    int32_t status = ETHFW_SOK;

    status = RPMessage_getRemoteEndPt(currProcId,
                                      name,
                                      remoteProcId,
                                      remoteEndPt,
                                      timeout);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to get remote endpoint");

    return status;
}

int32_t EthFwIpc_announceAll(uint32_t localEp,
                             const char* name)
{
    int32_t status = ETHFW_SOK;

    status = RPMessage_announce(RPMESSAGE_ALL,
                                localEp,
                                name);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to announce to all remote cores");

    return status;
}

int32_t EthFwIpc_announce(uint32_t remoteProcId,
                          uint32_t localEp,
                          const char* name)
{
    int32_t status = ETHFW_SOK;

    status = RPMessage_announce(remoteProcId,
                                localEp,
                                name);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to announce to %u remote core", remoteProcId);

    return status;
}

void EthFwIpc_unblockRpmsg(EthFwIpc_RpmsgHandle rpmsgHandle)
{
    RPMessage_Handle handle = (RPMessage_Handle)rpmsgHandle;
    RPMessage_unblock(handle);
}

int32_t EthFwIpc_deleteRpmsg(EthFwIpc_RpmsgHandle rpmsgHandle)
{
    int32_t status = ETHFW_SOK;
    RPMessage_Handle handle = (RPMessage_Handle)rpmsgHandle;

    status = RPMessage_delete(&handle);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to delete RPMessage endpoint");

    return status;
}

void EthFwIpc_resetObj()
{
    /* Do nothing */
}

void EthFwIpc_enableControlEndPt()
{
    int32_t status = ETHFW_ENOTSUPPORTED;

    ETHFWTRACE_ERR(status, "This API is not supported for Jacinto");
}
