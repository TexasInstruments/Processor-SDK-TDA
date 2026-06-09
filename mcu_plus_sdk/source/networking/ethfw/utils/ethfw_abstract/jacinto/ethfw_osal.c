/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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
 * \file ethfw_osal.c
 *
 * \brief Ethernet Firmware osal implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ti/osal/osal.h>
#include <utils/ethfw_common/include/ethfw_types.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_common/include/ethfw_utils.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x502

#define ETHFW_OSAL_FREERTOS_MAX_TASK                (15U)


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct EthFwOsal_TaskObj_s
{
    TaskP_Handle handle;

    void (*func)(void*);
} EthFwOsal_TaskObj;

typedef struct EthFwOsal_OsalObj_s
{
    bool isMutexValid;
    EthFwOsal_MutexHandle hMutex;
} EthFwOsal_OsalObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* global pool of statically allocated task objects */
static EthFwOsal_TaskObj gEthFwOsalTaskPfreertosPool[ETHFW_OSAL_FREERTOS_MAX_TASK];

static EthFwOsal_OsalObj gEthFwOsalObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthFwOsal_init(void)
{
    static bool osalInitDone = BFALSE;

    if (!osalInitDone)
    {
        memset(&gEthFwOsalObj, 0, sizeof(EthFwOsal_OsalObj));

        gEthFwOsalObj.hMutex = EthFwOsal_createMutex();

        if (gEthFwOsalObj.hMutex != NULL)
        {
            gEthFwOsalObj.isMutexValid = BTRUE;
        }

        memset(&gEthFwOsalTaskPfreertosPool[0], 0, sizeof(gEthFwOsalTaskPfreertosPool));

        osalInitDone = BTRUE;
    }
}

static void EthFwOsal_tempTask(void* a0, void* a1)
{
    EthFwOsal_TaskObj *taskObj = (EthFwOsal_TaskObj *)a0;
    taskObj->func(a1);
}

void EthFwOsal_initTaskParams(EthFwOsal_TaskParams *params)
{
    if(NULL_PTR != params)
    {
        params->name = NULL;
        params->stacksize = 0U;
        params->stack = NULL;
        params->priority = 0U;
        params->arg0 = NULL;
        params->userData = NULL;
    }
}

EthFwOsal_TaskHandle EthFwOsal_createTask(void (*func)(void*), EthFwOsal_TaskParams *params)
{
    int32_t status = ETHFW_SOK;
    TaskP_Params taskParams;
    EthFwOsal_TaskObj *taskObj = NULL;
    uint32_t i;

    EthFw_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    /* Find a free slot in the pool */
    for (i = 0U; i < ETHFW_OSAL_FREERTOS_MAX_TASK; i++)
    {
        if (gEthFwOsalTaskPfreertosPool[i].handle == NULL)
        {
            taskObj = &gEthFwOsalTaskPfreertosPool[i];
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    EthFw_assert(taskObj != NULL);
    
    TaskP_Params_init(&taskParams);
    if (params->name != NULL)
    {
        /* using string copy to avoid memory corruption once memory has been
         * freed from the stack */
        strcpy((char *)taskParams.name, (char *)params->name);
    }
    if (params->priority >= 0)
    {
        taskParams.priority = params->priority;
    }
    if (params->stack != NULL)
    {
        taskParams.stack = params->stack;
    }
    if (params->stacksize > 0)
    {
        taskParams.stacksize = params->stacksize;
    }
#if defined(SAFERTOS)
    taskParams.userData  = params->userData;
#endif
    taskParams.arg0 = taskObj;
    taskParams.arg1 = params->arg0;
    taskObj->func = func;

    taskObj->handle = TaskP_create(EthFwOsal_tempTask, &taskParams);

    if (taskObj->handle == NULL)
    {
        status = ETHFW_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to create Task");
    }

    return (EthFwOsal_TaskHandle)taskObj->handle;
}

int32_t EthFwOsal_deleteTask(EthFwOsal_TaskHandle *handle)
{
    int32_t status = ETHFW_EFAIL;
    TaskP_Handle *hTask = (TaskP_Handle *)handle;
    uint32_t i;

    EthFw_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    /* Find and clear the pool entry before deleting */
    for (i = 0U; i < ETHFW_OSAL_FREERTOS_MAX_TASK; i++)
    {
        if (gEthFwOsalTaskPfreertosPool[i].handle == *hTask)
        {
            gEthFwOsalTaskPfreertosPool[i].handle = NULL;
            gEthFwOsalTaskPfreertosPool[i].func = NULL;
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    status = TaskP_delete(hTask);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to delete the task");

    return status;
}

uint32_t EthFwOsal_isTaskTerminated(EthFwOsal_TaskHandle handle)
{
    uint32_t isTaskTerminated = UFALSE;
    TaskP_Handle hTask = (TaskP_Handle)handle;

    isTaskTerminated = TaskP_isTerminated(hTask);

    return isTaskTerminated;
}

EthFwOsal_TaskHandle EthFwOsal_getTaskSelf(void)
{
    TaskP_Handle hTask = NULL;

    hTask = TaskP_self();

    return (EthFwOsal_TaskHandle)hTask;
}

void EthFwOsal_setTaskPrio(EthFwOsal_TaskHandle handle, uint32_t priority)
{
    TaskP_Handle hTask = (TaskP_Handle)handle;

    TaskP_setPrio(hTask, priority);
}

void EthFwOsal_sleepTaskinMsecs(uint32_t timeoutInMsecs)
{
    TaskP_sleepInMsecs(timeoutInMsecs);
}

void EthFwOsal_sleepTask(uint32_t timeout)
{
    TaskP_sleep(timeout);
}

void EthFwOsal_yieldTask(void)
{
    TaskP_yield();
}

uint64_t EthFwOsal_getTimeInUsecs(void)
{
    return TimerP_getTimeInUsecs();
}

void EthFwOsal_exitTask(void)
{
    /* Do Nothing */
}

void EthFwOsal_wbCache(const void *addr, uint32_t size)
{
    CacheP_wb(addr, size);
}

void EthFwOsal_invCache(const void *addr, uint32_t size)
{
    CacheP_Inv((void *)addr, size);
}

void EthFwOsal_wbInvCache(const void *addr, uint32_t size)
{
    CacheP_wbInv((void *)addr, size);
}

EthFwOsal_SemHandle EthFwOsal_createSemaphore(uint32_t count)
{
    int32_t status = ETHFW_SOK;
    SemaphoreP_Params semParams;
    EthFwOsal_SemHandle handle = NULL;

    SemaphoreP_Params_init(&semParams);
    semParams.mode = SemaphoreP_Mode_BINARY;
    handle = (EthFwOsal_SemHandle)SemaphoreP_create(count, &semParams);

    if (NULL == handle)
    {
        status = ETHFW_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to create the Semaphore");
    }

    return handle;
}

int32_t EthFwOsal_deleteSemaphore(EthFwOsal_SemHandle handle)
{
    int32_t status = ETHFW_SOK;
    SemaphoreP_Handle hSem = (SemaphoreP_Handle)handle;

    status = SemaphoreP_delete(hSem);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to delete the Semaphore");

    return status;
}

int32_t EthFwOsal_pendSemaphore(EthFwOsal_SemHandle handle, uint32_t timeout)
{
    int32_t status = ETHFW_SOK;
    SemaphoreP_Handle hSem = (SemaphoreP_Handle)handle;

    status = SemaphoreP_pend(hSem, timeout);
    ETHFWTRACE_DBG_IF((status != ETHFW_SOK), "Semaphore pend failed");

    return status;
}

int32_t EthFwOsal_postSemaphore(EthFwOsal_SemHandle handle)
{
    int32_t status = ETHFW_SOK;
    SemaphoreP_Handle hSem = (SemaphoreP_Handle)handle;

    status = SemaphoreP_post(hSem);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Semaphore post failed");

    return status;
}

EthFwOsal_MutexHandle EthFwOsal_createMutex(void)
{
    return EnetOsal_createMutex();
}

void EthFwOsal_deleteMutex(EthFwOsal_MutexHandle hMutex)
{
    EnetOsal_deleteMutex(hMutex);
}

void EthFwOsal_lockMutex(EthFwOsal_MutexHandle hMutex)
{
    EnetOsal_lockMutex(hMutex);
}

void EthFwOsal_unlockMutex(EthFwOsal_MutexHandle hMutex)
{
    EnetOsal_unlockMutex(hMutex);
}

void EthFwOsal_initClockParams(EthFwOsal_ClockParams *params)
{
    if(NULL_PTR != params)
    {
        params->startMode   = 0U;
        params->period      = 0U;
        params->runMode     = 0U;
        params->arg         = NULL;
    }
}

EthFwOsal_ClockHandle EthFwOsal_createClock(void (*func)(void*), EthFwOsal_ClockParams *params)
{
    EthFwOsal_ClockHandle handle;
    ClockP_Params clkParams;
    int32_t status = ETHFW_SOK;

    ClockP_Params_init(&clkParams);
    clkParams.startMode = params->startMode;
    clkParams.period    = params->period;
    clkParams.runMode   = params->runMode;
    clkParams.arg       = params->arg;

    handle = (EthFwOsal_ClockHandle)ClockP_create(func, &clkParams);

    if (NULL == handle)
    {
        status = ETHFW_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to create clock");
    }

    return handle;
}

int32_t EthFwOsal_startClock(EthFwOsal_ClockHandle handle)
{
    int32_t status = ETHFW_SOK;
    ClockP_Handle hClock = (ClockP_Handle)handle;

    status = ClockP_start(hClock);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Clock start failed");

    return status;
}

int32_t EthFwOsal_stopClock(EthFwOsal_ClockHandle handle)
{
    int32_t status = ETHFW_SOK;
    ClockP_Handle hClock = (ClockP_Handle)handle;

    status = ClockP_stop(hClock);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Clock stop failed");

    return status;
}

int32_t EthFwOsal_deleteClock(EthFwOsal_ClockHandle handle)
{
    int32_t status = ETHFW_SOK;
    ClockP_Handle hClock = (ClockP_Handle)handle;

    status = ClockP_delete(hClock);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Clock delete failed");

    return status;
}

void EthFwOsal_initEventParams(EthFwOsal_EventParams *params)
{
    EventP_Params_init((EventP_Params *)params);
}

EthFwOsal_EventHandle EthFwOsal_createEvent(EthFwOsal_EventParams *params)
{
    EthFwOsal_EventHandle handle;
    int32_t status = ETHFW_SOK;

    handle = (EthFwOsal_EventHandle)EventP_create((EventP_Params *)params);

    if (NULL == handle)
    {
        status = ETHFW_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to create Event");
    }

    return handle;
}

uint32_t EthFwOsal_waitEvent(EthFwOsal_EventHandle handle,
                            uint32_t eventMask,
                            uint8_t waitMode,
                            uint32_t timeout)
{
    int32_t events = 0U;
    EventP_Handle hEvt = (EventP_Handle)handle;

    events = EventP_wait(hEvt, eventMask, waitMode, timeout);

    return events;
}

int32_t EthFwOsal_postEvent(EthFwOsal_EventHandle handle, uint32_t eventBits)
{
    int32_t status = ETHFW_SOK;
    EventP_Handle hEvt = (EventP_Handle)handle;

    status = EventP_post(hEvt, eventBits);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to post Event");

    return status;
}

void EthFwOsal_initMailboxParams(EthFwOsal_MailboxParams *params)
{
    MailboxP_Params_init(params);
}

EthFwOsal_MailboxHandle EthFwOsal_createMailbox(EthFwOsal_MailboxParams *params)
{
    EthFwOsal_MailboxHandle retHandle = NULL;
    int32_t status = ETHFW_SOK;

    retHandle = (EthFwOsal_MailboxHandle)MailboxP_create(params);

    if (NULL == retHandle)
    {
        status = ETHFW_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to create Mailbox");
    }

    return retHandle;
}

int32_t EthFwOsal_deleteMailbox(EthFwOsal_MailboxHandle handle)
{
    int32_t status = MailboxP_OK;
    MailboxP_Handle hMbx = (MailboxP_Handle)handle;

    status = MailboxP_delete(hMbx);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to delete Mailbox");

    return status;
}

int32_t EthFwOsal_postMailbox(EthFwOsal_MailboxHandle handle,
                              void *msg,
                              uint32_t timeout)
{
    int32_t status = MailboxP_OK;
    MailboxP_Handle hMbx = (MailboxP_Handle)handle;

    status = MailboxP_post(hMbx, msg, timeout);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to post Mailbox");

    return status;
}

int32_t EthFwOsal_pendMailbox(EthFwOsal_MailboxHandle handle,
                              void *msg,
                              uint32_t timeout)
{
    int32_t status = MailboxP_OK;
    MailboxP_Handle hMbx = (MailboxP_Handle)handle;

    status = MailboxP_pend(hMbx, msg, timeout);
    ETHFWTRACE_DBG_IF((status != ETHFW_SOK), "Failed to pend Mailbox");

    return status;
}
