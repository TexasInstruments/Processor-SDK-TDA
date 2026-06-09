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

#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/CacheP.h>
#include <kernel/dpl/EventP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/MailboxP.h>

#include <string.h>
#include "stdlib.h"
#include <utils/ethfw_abstract/ethfw_osal.h>

#include <enet_appmemutils.h>
#include <utils/ethfw_common/include/ethfw_types.h>
#include <utils/ethfw_common/include/ethfw_trace.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x504

#define ETHFW_OSAL_FREERTOS_MAX_TASK                (15U)
#define ETHFW_OSAL_FREERTOS_MAX_SEMAPHORE           (15U)
#define ETHFW_OSAL_FREERTOS_MAX_EVENT               (5U)
#define ETHFW_OSAL_FREERTOS_MAX_CLOCK               (5U)
#define ETHFW_OSAL_FREERTOS_MAX_MAILBOX             (5U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct EthFwOsal_Task_s
{
    bool    used;
    TaskP_Object taskObj;
} EthFwOsal_Task;

typedef struct EthFwOsal_Semaphore_s
{
    bool    used;
    SemaphoreP_Object SemObj;
} EthFwOsal_Semaphore;

typedef struct EthFwOsal_Clock_s
{
    bool    used;
    ClockP_Object ClockObj;
} EthFwOsal_Clock;

typedef struct EthFwOsal_Event_s
{
    bool    used;
    EventP_Object EventObj;
} EthFwOsal_Event;

typedef struct EthFwOsal_Mailbox_s
{
    bool    used;
    MailboxP_Object mailboxObj;
} EthFwOsal_Mailbox;

typedef void (*EthFwOsal_clockCbFxn)(void *args);

typedef struct EthFwOsal_clockCbArgs_s
{
    EthFwOsal_clockCbFxn cbFxn;
    void *args;
}EthFwOsal_clockCbArgs;

typedef struct EthFwOsal_Obj_s
{
    uint32_t allocCnt;
    uint32_t peakCnt;

} EthFwOsal_Obj;

typedef struct EthFwOsal_OsalObj_s
{
    bool isMutexValid;
    EthFwOsal_MutexHandle hMutex;
    EthFwOsal_Obj taskObj;
    EthFwOsal_Obj semaphoreObj;
    EthFwOsal_Obj eventObj;
    EthFwOsal_Obj clockObj;
    EthFwOsal_Obj mailboxObj;

} EthFwOsal_OsalObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ToDo: this is workaround for linking error in MCU. Need to fix it */
extern Udma_DrvObject gUdmaDrvObj[0U];

static EthFwOsal_OsalObj gEthFwOsalObj;

/* global pool of statically allocated task pools */
static EthFwOsal_Task gEthFwOsalTaskPfreertosPool[ETHFW_OSAL_FREERTOS_MAX_TASK];

/* global pool of statically allocated semaphore pools */
static EthFwOsal_Semaphore gEthFwOsalSemaphorefreertosPool[ETHFW_OSAL_FREERTOS_MAX_SEMAPHORE];

/* global pool of statically allocated event pools */
static EthFwOsal_Event gEthFwOsalEventfreertosPool[ETHFW_OSAL_FREERTOS_MAX_EVENT];

/* global pool of statically allocated clock pools */
static EthFwOsal_Clock gEthFwOsalClockfreertosPool[ETHFW_OSAL_FREERTOS_MAX_CLOCK];

/* global pool of statically allocated clock pools */
static EthFwOsal_Mailbox gEthFwOsalMailboxfreertosPool[ETHFW_OSAL_FREERTOS_MAX_MAILBOX];

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthFwOsal_init(void)
{
    static bool osalInitDone = BFALSE;
    int32_t status = SystemP_SUCCESS;

    if (!osalInitDone)
    {
        EnetUtils_Cfg *utilsPrms = NULL;

        Enet_init(utilsPrms);

        memset(&gEthFwOsalObj, 0, sizeof(EthFwOsal_OsalObj));

        gEthFwOsalObj.hMutex = EthFwOsal_createMutex();

        if (gEthFwOsalObj.hMutex != NULL)
        {
            gEthFwOsalObj.isMutexValid = BTRUE;
        }

        memset(&gEthFwOsalTaskPfreertosPool[0], 0, sizeof(gEthFwOsalTaskPfreertosPool));
        memset(&gEthFwOsalSemaphorefreertosPool[0], 0, sizeof(gEthFwOsalSemaphorefreertosPool));
        memset(&gEthFwOsalClockfreertosPool[0], 0, sizeof(gEthFwOsalClockfreertosPool));
        memset(&gEthFwOsalEventfreertosPool[0], 0, sizeof(gEthFwOsalEventfreertosPool));
        memset(&gEthFwOsalMailboxfreertosPool[0], 0, sizeof(gEthFwOsalMailboxfreertosPool));

        osalInitDone = BTRUE;

        status = EnetMem_init();
        EnetAppUtils_assert(ENET_SOK == status);
    }
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
    }
}

EthFwOsal_TaskHandle EthFwOsal_createTask(void (*func)(void*), EthFwOsal_TaskParams *params)
{
    int32_t status = SystemP_SUCCESS;
    TaskP_Params taskParams;
    EthFwOsal_TaskHandle handle;
    EthFwOsal_Task *tskHandle = NULL;
    EthFwOsal_Task *taskPool = NULL;
    const uint32_t maxTaskCount = ETHFW_OSAL_FREERTOS_MAX_TASK;
    uint32_t i = 0U;
    uint32_t index = 0U;

    taskPool = &gEthFwOsalTaskPfreertosPool[0];

    EnetAppUtils_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    for (i = 0U; i < maxTaskCount; i++)
    {
        if (BFALSE == taskPool[i].used)
        {
            taskPool[i].used = BTRUE;
            index = i;
            /* Update statistics */
            gEthFwOsalObj.taskObj.allocCnt++;
            if (gEthFwOsalObj.taskObj.allocCnt > gEthFwOsalObj.taskObj.peakCnt)
            {
                gEthFwOsalObj.taskObj.peakCnt = gEthFwOsalObj.taskObj.allocCnt;
            }
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    if (index < maxTaskCount)
    {
        tskHandle = &taskPool[index];
    }

    EnetAppUtils_assert(tskHandle != NULL);

    TaskP_Params_init(&taskParams);
    taskParams.name = params->name;
    if (params->priority > 0U)
    {
        taskParams.priority = params->priority;
    }
    if (params->stacksize > 0U)
    {
        taskParams.stackSize = params->stacksize;
    }
    if (params->stack != NULL)
    {
        taskParams.stack = params->stack;
    }
    taskParams.args = params->arg0;
    taskParams.taskMain = func;

    status = TaskP_construct(&tskHandle->taskObj, &taskParams);

    if (status != SystemP_SUCCESS)
    {
        EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

        tskHandle->used = BFALSE;

        if (0U < gEthFwOsalObj.taskObj.allocCnt)
        {
            gEthFwOsalObj.taskObj.allocCnt--;
        }

        EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

        handle = NULL;
        ETHFWTRACE_ERR(status, "Failed to create Task");
    }
    else
    {
        handle = (EthFwOsal_TaskHandle)tskHandle;
    }

    return handle;
}

int32_t EthFwOsal_deleteTask(EthFwOsal_TaskHandle *handle)
 {
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Task *taskHandle = (EthFwOsal_Task *)handle;

    TaskP_destruct(&taskHandle->taskObj);

    EnetAppUtils_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    taskHandle->used = BFALSE;

    if (0U < gEthFwOsalObj.taskObj.allocCnt)
    {
        gEthFwOsalObj.taskObj.allocCnt--;
    }
    else
    {
        status = SystemP_FAILURE;
        ETHFWTRACE_ERR(status, "Failed to delete the task");
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    return status;
}

uint32_t EthFwOsal_isTaskTerminated(EthFwOsal_TaskHandle taskHandle)
{
    int32_t status = SystemP_FAILURE;

    ETHFWTRACE_ERR(status, "This API is not supported by MCU SDK");

    return status;
}

EthFwOsal_TaskHandle EthFwOsal_getTaskSelf(void)
{
    EthFwOsal_TaskHandle handle = NULL;
    int32_t status = SystemP_FAILURE;

    ETHFWTRACE_ERR(status, "This API is not supported by MCU SDK");

    return handle;
}

void EthFwOsal_setTaskPrio(EthFwOsal_TaskHandle taskHandle, uint32_t priority)
{
    int32_t status = SystemP_FAILURE;

    ETHFWTRACE_ERR(status, "This API is not supported by MCU SDK");
}

void EthFwOsal_exitTask(void)
{
    TaskP_exit();
}

void EthFwOsal_yieldTask(void)
{
    TaskP_yield();
}

void EthFwOsal_sleepTaskinMsecs(uint32_t timeoutInMsecs)
{
    ClockP_usleep(timeoutInMsecs*1000U);
}

void EthFwOsal_sleepTask(uint32_t timeout)
{
    ClockP_sleep(timeout);
}

uint64_t EthFwOsal_getTimeInUsecs(void)
{
    return ClockP_getTimeUsec();
}

EthFwOsal_SemHandle EthFwOsal_createSemaphore(uint32_t count)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Semaphore *handle = NULL;
    EthFwOsal_Semaphore *semPool = NULL;
    EthFwOsal_SemHandle semHandle = NULL;
    const uint32_t maxSemaphoreCount = ETHFW_OSAL_FREERTOS_MAX_SEMAPHORE;
    uint32_t i = 0U;
    uint32_t index = 0U;

    semPool = &gEthFwOsalSemaphorefreertosPool[0];

    EnetAppUtils_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    for (i = 0; i < maxSemaphoreCount; i++)
    {
        if (BFALSE == semPool[i].used)
        {
            semPool[i].used = BTRUE;
            index = i;
            /* Update statistics */
            gEthFwOsalObj.semaphoreObj.allocCnt++;
            if (gEthFwOsalObj.semaphoreObj.allocCnt > gEthFwOsalObj.semaphoreObj.peakCnt)
            {
                gEthFwOsalObj.semaphoreObj.peakCnt = gEthFwOsalObj.semaphoreObj.allocCnt;
            }
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    if (index < maxSemaphoreCount)
    {
        /* Grab the memory */
        handle = &semPool[index];
    }

    EnetAppUtils_assert(handle != NULL);

    status = SemaphoreP_constructBinary(&handle->SemObj, count);

    if (status != SystemP_SUCCESS)
    {
        EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

        handle->used = BFALSE;

        if (0U < gEthFwOsalObj.semaphoreObj.allocCnt)
        {
            gEthFwOsalObj.semaphoreObj.allocCnt--;
        }

        EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

        semHandle = NULL;
        ETHFWTRACE_ERR(status, "Failed to create the Semaphore");
    }
    else
    {
        semHandle = (EthFwOsal_SemHandle)handle;
    }

    return semHandle;
}

int32_t EthFwOsal_deleteSemaphore(EthFwOsal_SemHandle semHandle)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Semaphore *handle = (EthFwOsal_Semaphore *)semHandle;

    EnetAppUtils_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    SemaphoreP_destruct(&handle->SemObj);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    handle->used = BFALSE;

    if (0U < gEthFwOsalObj.semaphoreObj.allocCnt)
    {
        gEthFwOsalObj.semaphoreObj.allocCnt--;
    }
    else
    {
        status = SystemP_FAILURE;
        ETHFWTRACE_ERR(status, "Failed to delete the Semaphore");
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    return status;
}

int32_t EthFwOsal_pendSemaphore(EthFwOsal_SemHandle semHandle, uint32_t timeout)
{
    int32_t status = SystemP_FAILURE;
    EthFwOsal_Semaphore *handle = (EthFwOsal_Semaphore *)semHandle;

    status = SemaphoreP_pend(&handle->SemObj, timeout);
    ETHFWTRACE_DBG_IF((status != SystemP_SUCCESS), "Semaphore pend failed");

    return status;
}

int32_t EthFwOsal_postSemaphore(EthFwOsal_SemHandle semHandle)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Semaphore *handle = (EthFwOsal_Semaphore *)semHandle;

    SemaphoreP_post(&handle->SemObj);

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
    if(NULL != params)
    {
        params->startMode   = 0U;
        params->period      = 0U;
        params->runMode     = 0U;
        params->arg         = NULL;
    }
}

static void EthFwOsal_clockCb(ClockP_Object *handle, void *args)
{
    EthFwOsal_clockCbArgs *cbArgs = (EthFwOsal_clockCbArgs *)args;
    cbArgs->cbFxn(cbArgs->args);
}

EthFwOsal_ClockHandle EthFwOsal_createClock(void (*func)(void*), EthFwOsal_ClockParams *params)
{
    EthFwOsal_clockCbArgs *cbArgs;
    ClockP_Params clkParams;
    EthFwOsal_ClockHandle retHandle = NULL;
    EthFwOsal_Clock *handle = NULL;
    EthFwOsal_Clock *timerPool = NULL;
    const uint32_t maxClockCount = ETHFW_OSAL_FREERTOS_MAX_CLOCK;
    uint32_t i = 0U;
    uint32_t index = 0U;
    int32_t status = SystemP_FAILURE;

    /* Pick up the internal static memory block */
    timerPool = &gEthFwOsalClockfreertosPool[0];

    EnetAppUtils_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    for (i = 0U; i < maxClockCount; i++)
    {
        if (BFALSE == timerPool[i].used)
        {
            timerPool[i].used = BTRUE;
            index = i;
            /* Update statistics */
            gEthFwOsalObj.clockObj.allocCnt++;
            if (gEthFwOsalObj.clockObj.allocCnt > gEthFwOsalObj.clockObj.peakCnt)
            {
                gEthFwOsalObj.clockObj.peakCnt = gEthFwOsalObj.clockObj.allocCnt;
            }
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    if (index < maxClockCount)
    {
        /* Grab the memory */
        handle = &timerPool[index];
    }

    EnetAppUtils_assert(handle != NULL);

    cbArgs = malloc(sizeof(EthFwOsal_clockCbArgs));

    EnetAppUtils_assert(cbArgs != NULL);

    memset(handle, 0, sizeof(EthFwOsal_clockCbArgs));

    ClockP_Params_init(&clkParams);
    clkParams.start     = params->startMode;
    clkParams.period    = params->period;
    clkParams.timeout   = params->period;
    clkParams.callback  = EthFwOsal_clockCb;

    cbArgs->cbFxn       = func;
    cbArgs->args         = params->arg;
    clkParams.args      = (void*)(cbArgs);
    if(params->runMode != 0U) /* If runmode is not oneshot */
    {
        status = ClockP_construct(&handle->ClockObj, &clkParams);
    }

    if (status != SystemP_SUCCESS)
    {
        EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

        handle->used = BFALSE;
        if (0U < gEthFwOsalObj.clockObj.allocCnt)
        {
            gEthFwOsalObj.clockObj.allocCnt--;
        }

        EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

        retHandle = NULL;
        ETHFWTRACE_ERR(status, "Failed to create clock");
    }
    else
    {
        retHandle = (EthFwOsal_ClockHandle)handle;
    }

    return retHandle;
}

int32_t EthFwOsal_startClock(EthFwOsal_ClockHandle clockHandle)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Clock *handle = (EthFwOsal_Clock *)clockHandle;

    ClockP_start(&handle->ClockObj);
    ETHFWTRACE_ERR_IF((status != SystemP_SUCCESS), status, "Clock start failed");
    return status;
}

int32_t EthFwOsal_stopClock(EthFwOsal_ClockHandle clockHandle)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Clock *handle = (EthFwOsal_Clock *)clockHandle;

    ClockP_stop(&handle->ClockObj);
    ETHFWTRACE_ERR_IF((status != SystemP_SUCCESS), status, "Clock stop failed");
    return status;
}

int32_t EthFwOsal_deleteClock(EthFwOsal_ClockHandle clockHandle)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Clock *handle = (EthFwOsal_Clock *)clockHandle;

    ClockP_destruct(&handle->ClockObj);
    ETHFWTRACE_ERR_IF((status != SystemP_SUCCESS), status, "Clock delete failed");

    return ENET_SOK;
}

void EthFwOsal_initEventParams(EthFwOsal_EventParams *params)
{
    /* Do Nothing */
}

EthFwOsal_EventHandle EthFwOsal_createEvent(EthFwOsal_EventParams *params)
{
    int32_t status = ENET_EFAIL;
    EthFwOsal_EventHandle retHandle = NULL;
    EthFwOsal_Event *handle = NULL;
    EthFwOsal_Event *eventPool = NULL;
    const uint32_t maxEventCount = ETHFW_OSAL_FREERTOS_MAX_EVENT;
    uint32_t i = 0U;
    uint32_t index = 0U;

    /* Pick up the internal static memory block */
    eventPool = &gEthFwOsalEventfreertosPool[0];

    EnetAppUtils_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    for (i = 0U; i < maxEventCount; i++)
    {
        if (BFALSE == eventPool[i].used)
        {
            eventPool[i].used = BTRUE;
            index = i;
            /* Update statistics */
            gEthFwOsalObj.eventObj.allocCnt++;
            if (gEthFwOsalObj.eventObj.allocCnt > gEthFwOsalObj.eventObj.peakCnt)
            {
                gEthFwOsalObj.eventObj.peakCnt = gEthFwOsalObj.eventObj.allocCnt;
            }
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    if (index < maxEventCount)
    {
        /* Grab the memory */
        handle = &eventPool[index];
    }

    status = EventP_construct(&handle->EventObj);

    if(status != SystemP_SUCCESS)
    {
        EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

        handle->used = BFALSE;
        /* Found the osal event object to delete */
        if (0U < gEthFwOsalObj.eventObj.allocCnt)
        {
            gEthFwOsalObj.eventObj.allocCnt--;
        }

        EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

        retHandle = NULL;
        ETHFWTRACE_ERR(status, "Failed to create Event");
    }
    else
    {
        retHandle = (EthFwOsal_EventHandle)handle;
    }
    return retHandle;
}

uint32_t EthFwOsal_waitEvent(EthFwOsal_EventHandle eventHandle, uint32_t eventMask, uint8_t waitMode, uint32_t timeout)
{
    int32_t status = SystemP_SUCCESS;
    EthFwOsal_Event *handle = (EthFwOsal_Event *)eventHandle;
    uint32_t events;

    status = EventP_waitBits(&handle->EventObj, eventMask, 1U, waitMode, timeout, &events);

    if(status != SystemP_SUCCESS)
    {
        events = 0U;
        ETHFWTRACE_ERR(status, "Wait for event failed");
    }
    return events;
}

int32_t EthFwOsal_postEvent(EthFwOsal_EventHandle eventHandle, uint32_t eventBits)
{
    int32_t status;
    EthFwOsal_Event *handle = (EthFwOsal_Event *)eventHandle;

    status = EventP_setBits(&handle->EventObj, eventBits);
    ETHFWTRACE_ERR_IF((status != SystemP_SUCCESS), status, "Failed to post Event");
    return status;
}

void EthFwOsal_wbCache(const void *addr, uint32_t size)
{
    CacheP_wb((void *)addr, size, CacheP_TYPE_ALLD);
}

void EthFwOsal_invCache(const void *addr, uint32_t size)
{
    CacheP_inv((void *)addr, size, CacheP_TYPE_ALLD);
}

void EthFwOsal_wbInvCache(const void *addr, uint32_t size)
{
    CacheP_wbInv((void *)addr, size, CacheP_TYPE_ALLD);
}

void EthFwOsal_initMailboxParams(EthFwOsal_MailboxParams *params)
{
    MailboxP_Params_init(params);
}

EthFwOsal_MailboxHandle EthFwOsal_createMailbox(EthFwOsal_MailboxParams *params)
{
    EthFwOsal_MailboxHandle retHandle = NULL;
    EthFwOsal_Mailbox *handle = NULL;
    EthFwOsal_Mailbox *mailboxPool = NULL;
    const uint32_t maxMailboxCount = ETHFW_OSAL_FREERTOS_MAX_MAILBOX;
    uint32_t i = 0U;
    uint32_t index = 0U;
    int32_t status = SystemP_SUCCESS;

    /* Pick up the internal static memory block */
    mailboxPool = &gEthFwOsalMailboxfreertosPool[0];

    EnetAppUtils_assert(gEthFwOsalObj.isMutexValid == BTRUE);

    EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

    for (i = 0U; i < maxMailboxCount; i++)
    {
        if (BFALSE == mailboxPool[i].used)
        {
            mailboxPool[i].used = BTRUE;
            index = i;
            /* Update statistics */
            gEthFwOsalObj.eventObj.allocCnt++;
            if (gEthFwOsalObj.mailboxObj.allocCnt > gEthFwOsalObj.mailboxObj.peakCnt)
            {
                gEthFwOsalObj.mailboxObj.peakCnt = gEthFwOsalObj.mailboxObj.allocCnt;
            }
            break;
        }
    }

    EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

    if (index < maxMailboxCount)
    {
        /* Grab the memory */
        handle = &mailboxPool[index];
    }

    retHandle = MailboxP_create(&handle->mailboxObj, params);

    if(retHandle == NULL)
    {
        EthFwOsal_lockMutex(gEthFwOsalObj.hMutex);

        handle->used = BFALSE;
        /* Found the osal event object to delete */
        if (0U < gEthFwOsalObj.mailboxObj.allocCnt)
        {
            gEthFwOsalObj.mailboxObj.allocCnt--;
        }

        EthFwOsal_unlockMutex(gEthFwOsalObj.hMutex);

        ETHFWTRACE_ERR(status, "Failed to create Mailbox");
    }

    return retHandle;
}

int32_t EthFwOsal_deleteMailbox(EthFwOsal_MailboxHandle handle)
{
    int32_t status = SystemP_SUCCESS;

    status = MailboxP_delete(handle);
    ETHFWTRACE_ERR_IF((status != SystemP_SUCCESS), status, "Failed to delete Mailbox");

    return status;
}

int32_t EthFwOsal_postMailbox(EthFwOsal_MailboxHandle handle,
                              void *msg,
                              uint32_t timeout)
{
    int32_t status = SystemP_SUCCESS;
    MailboxP_Handle hMbx = (MailboxP_Handle)handle;

    status = MailboxP_post(hMbx, msg, timeout);
    ETHFWTRACE_ERR_IF((status != SystemP_SUCCESS), status, "Failed to post Mailbox");

    return status;
}

int32_t EthFwOsal_pendMailbox(EthFwOsal_MailboxHandle handle,
                              void *msg,
                              uint32_t timeout)
{
    int32_t status = SystemP_SUCCESS;
    MailboxP_Handle hMbx = (MailboxP_Handle)handle;

    status = MailboxP_pend(hMbx, msg, timeout);
    ETHFWTRACE_DBG_IF((status != SystemP_SUCCESS), "Failed to pend Mailbox");

    return status;
}

/* ToDo: this is workaround for linking error in MCU. Need to fix it */
Udma_DrvHandle EnetAppUtils_udmaOpen(Enet_Type enetType,
                                     Udma_InitPrms *pInitPrms)
{
    return &gUdmaDrvObj[0U];
}