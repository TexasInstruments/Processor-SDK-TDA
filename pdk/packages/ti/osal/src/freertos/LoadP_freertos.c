/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ======== LoadP_freertos.c ========
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/osal/TaskP.h>
#include <ti/osal/LoadP.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ti/csl/csl_types.h>
#include <ti/osal/osal.h>
#include <ti/osal/DebugP.h>
#include <ti/osal/soc/osal_soc.h>

#include <FreeRTOS.h>
#include <task.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct LoadP_taskLoadObj_s
{
    bool                used;
    TaskP_Handle        pTsk;
    uint64_t            threadTime;
    uint32_t            lastUpdate_threadTime;
} LoadP_taskLoadObj;

typedef struct LoadP_freertos_s
{   
    LoadP_taskLoadObj taskLoadObj[OSAL_FREERTOS_CONFIGNUM_TASK];
    uint64_t          idlTskTime;
    uint64_t          totalTime;
    uint32_t          lastUpdate_idlTskTime;
    uint32_t          lastUpdate_totalTime;
} LoadP_freertos;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static uint32_t LoadP_calcPercentLoad(uint64_t threadTime, uint64_t totalTime);
void LoadP_addTask(TaskP_Handle handle, uint32_t tskId);
void LoadP_removeTask(uint32_t tskId);
extern TaskHandle_t TaskP_getFreertosHandle(TaskP_Handle handle);
extern uint32_t TaskP_getTaskId(TaskP_Handle handle);
void vApplicationLoadHook(void);
/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static LoadP_freertos   gLoadP_freertos;
static bool             gLoadP_initDone = BFALSE;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void LoadP_reset(void)
{
    uint32_t            i;
    LoadP_taskLoadObj   *pHndl;

    vTaskSuspendAll();

    gLoadP_freertos.idlTskTime = 0U;
    gLoadP_freertos.totalTime = 0U;

    for (i = 0U; i < OSAL_FREERTOS_CONFIGNUM_TASK; i++)
    {
        pHndl = &gLoadP_freertos.taskLoadObj[i];

        if (BTRUE == pHndl->used )
        {
            pHndl->threadTime = 0U;
        }
    }
    (void)xTaskResumeAll();

    return;
}

LoadP_Status LoadP_getTaskLoad(TaskP_Handle taskHandle, LoadP_Stats *stats)
{
    LoadP_taskLoadObj   *pHndl;
    LoadP_Status        ret_val = LoadP_FAILURE;
    TaskStatus_t        tskStat;
    uint32_t            tskId;

    if((NULL_PTR != stats) && (NULL_PTR != taskHandle))
    {
        vTaskSuspendAll();

        LoadP_update();

        tskId = TaskP_getTaskId(taskHandle);

        pHndl = &gLoadP_freertos.taskLoadObj[tskId];

        if ((BTRUE == pHndl->used) && (pHndl->pTsk == taskHandle))
        {
            vTaskGetInfo( TaskP_getFreertosHandle(pHndl->pTsk), &tskStat, pdFALSE, eRunning);
            stats->threadTime  = pHndl->threadTime;
            stats->totalTime   = gLoadP_freertos.totalTime;
            stats->percentLoad = LoadP_calcPercentLoad(pHndl->threadTime, gLoadP_freertos.totalTime);
            stats->name        = tskStat.pcTaskName;
            ret_val = LoadP_OK;
        }

        (void)xTaskResumeAll();
    }

    return ret_val;
}

uint32_t LoadP_getCPULoad(void)
{
    uint32_t cpuLoad;

    LoadP_update();

    vTaskSuspendAll();

    cpuLoad = 100U - LoadP_calcPercentLoad(gLoadP_freertos.idlTskTime, gLoadP_freertos.totalTime);

    (void)xTaskResumeAll();

    return cpuLoad;
}


void LoadP_update(void)
{
    uint32_t            i;
    uint32_t            curTime;
    uint32_t            delta;
    LoadP_taskLoadObj   *pHndl;
    TaskStatus_t        tskStat;

    vTaskSuspendAll();
    
    if(!gLoadP_initDone)
    {
        (void)memset( (void *)&gLoadP_freertos,0,sizeof(gLoadP_freertos));
        gLoadP_initDone = BTRUE;
    }

    /* Idle Task Update */
    curTime = (uint32_t)ulTaskGetIdleRunTimeCounter();
    delta = LoadP_calcCounterDiff(curTime, gLoadP_freertos.lastUpdate_idlTskTime);
    gLoadP_freertos.lastUpdate_idlTskTime = curTime;

    gLoadP_freertos.idlTskTime += delta;
    
    /* Total Time Update */
    curTime = (uint32_t)uiPortGetRunTimeCounterValue();
    delta = LoadP_calcCounterDiff(curTime, gLoadP_freertos.lastUpdate_totalTime);
    gLoadP_freertos.lastUpdate_totalTime = curTime;

    gLoadP_freertos.totalTime += delta;

    /* All tasks Update */
    for (i = 0U; i < OSAL_FREERTOS_CONFIGNUM_TASK; i++)
    {
        pHndl = &gLoadP_freertos.taskLoadObj[i];

        if ((BTRUE == pHndl->used) && (NULL_PTR != pHndl->pTsk))
        {
            vTaskGetInfo( TaskP_getFreertosHandle(pHndl->pTsk),
                          &tskStat,
                          pdFALSE,
                          eRunning  );

            delta = LoadP_calcCounterDiff(tskStat.ulRunTimeCounter, pHndl->lastUpdate_threadTime);
            pHndl->lastUpdate_threadTime = tskStat.ulRunTimeCounter;

            pHndl->threadTime += delta;
            
        }
    }

    (void)xTaskResumeAll();

    return;
}

/* ========================================================================================================================== */

uint32_t LoadP_calcCounterDiff(uint32_t cur, uint32_t last)
{
    uint32_t delta;

    if(cur >= last)
    {
        delta = cur - last;
    }
    else
    {
        delta = ( 0xFFFFFFFFU - last ) + cur;
    }
    return delta;
}

static uint32_t LoadP_calcPercentLoad(uint64_t threadTime, uint64_t totalTime)
{
    uint32_t percentLoad;
    
    /* If thread time of a particular task is shown greater than the totalTime 
     * due to a Kernel Sync of threadTime, set totalTime equal to threadTime */
    if(threadTime > totalTime)
    {
        totalTime = threadTime;
    }
    percentLoad = (uint32_t)(threadTime  / (totalTime / 100U));

    return percentLoad;
}

void LoadP_addTask(TaskP_Handle handle, uint32_t tskId)
{
    LoadP_taskLoadObj   *pHndl = &gLoadP_freertos.taskLoadObj[tskId];
    
    if(!gLoadP_initDone)
    {
        (void)memset( (void *)&gLoadP_freertos,0,sizeof(gLoadP_freertos));
        gLoadP_initDone = BTRUE;
    }

    pHndl->used                  = BTRUE;
    pHndl->pTsk                  = handle;
    pHndl->threadTime            = 0U;
    pHndl->lastUpdate_threadTime = 0U;

}

void LoadP_removeTask(uint32_t tskId)
{
    LoadP_taskLoadObj   *pHndl = &gLoadP_freertos.taskLoadObj[tskId];

    pHndl->used = BFALSE; 
}

void vApplicationLoadHook(void)
{
    static uint32_t t0          = 0U; /* Last check time */
    static uint32_t timeElapsed = 0U; /* Elapsed time until last update */
    
    uint32_t t1     = uiPortGetRunTimeCounterValue(); /* Current time */
    uint32_t delta  = t1 - t0;

    t0 = t1; /* Update last check time */

    /* Accumulate elapsed time until last update */
    timeElapsed += delta;

    /* Check for window */
    if((uint32_t)configLOAD_WINDOW_IN_MS < timeElapsed)
    {
        /* Update load and reset elapsed time */
        LoadP_update();
        timeElapsed = 0U;
    }
}

