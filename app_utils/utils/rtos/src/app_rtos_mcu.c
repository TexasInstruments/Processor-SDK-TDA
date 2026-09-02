/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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
#if defined(MCU_PLUS_SDK)
#include <utils/rtos/include/app_rtos.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_log.h>
#include <kernel/dpl/SemaphoreP.h>
#include <kernel/dpl/TaskP.h>
#include <kernel/dpl/ClockP.h>
#include <app_rtos_mcu_priv.h>
#include <stddef.h>
#elif defined(MCU_SDK)
#include <stddef.h>
#include <utils/rtos/include/app_rtos.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_log.h>
#include <SemaphoreP.h>
#include <ClockP.h>
#include <TaskP.h>
#include <SystemP.h>
#include <app_rtos_mcu_priv.h>
#endif

#ifndef QNX
#if defined(SOC_J722S)
extern uint32_t __BSS_START;
extern uint32_t __BSS_END;

void extended_system_pre_init(void);

void extended_system_pre_init(void)
{
    /* initialize .bss section to zero */
    uint32_t bss_size = (uint32_t)((uintptr_t)&__BSS_END - (uintptr_t)&__BSS_START);
    (void)memset((void*)&__BSS_START, 0x00, bss_size);
}
#endif
#endif

/* Semaphore */
static SemaphoreP_Object gAppRtosSemaphoreObjs[APP_RTOS_MAX_SEMAPHORE_COUNT];
static uint8_t gAppRtosSemaphoreUsed[APP_RTOS_MAX_SEMAPHORE_COUNT] = {0};

static TaskP_Object gAppRtosTaskObjs[APP_RTOS_MAX_TASK_COUNT];
static uint8_t gAppRtosTaskObjUsed[APP_RTOS_MAX_TASK_COUNT] = {0};

void appRtosSemaphoreParamsInit(app_rtos_semaphore_params_t *params)
{
    if(params != NULL)
    {
        params->mode = APP_RTOS_SEMAPHORE_MODE_COUNTING;
        params->maxValue = 0xFFU;
        params->initValue = 0U;
    }
}

app_rtos_semaphore_handle_t appRtosSemaphoreCreate(app_rtos_semaphore_params_t params)
{
    app_rtos_semaphore_handle_t handle = NULL;
    int32_t status;
    uint32_t index = 0U;

    for (index = 0U; index < APP_RTOS_MAX_SEMAPHORE_COUNT; index++)
    {
        if (gAppRtosSemaphoreUsed[index] == 0U)
        {
            switch (params.mode)
            {
            case APP_RTOS_SEMAPHORE_MODE_BINARY:
                status = SemaphoreP_constructBinary(&gAppRtosSemaphoreObjs[index], params.initValue);
                break;

            case APP_RTOS_SEMAPHORE_MODE_COUNTING:
                status = SemaphoreP_constructCounting(&gAppRtosSemaphoreObjs[index], params.initValue, params.maxValue);
                break;

            case APP_RTOS_SEMAPHORE_MODE_MUTEX:
                status = SemaphoreP_constructMutex(&gAppRtosSemaphoreObjs[index]);
                break;
            
            default:
                status = APP_RTOS_STATUS_FAILURE;
                appLogPrintf("Invalid semaphore mode provided\n");
                break;
            }
            if(status != APP_RTOS_STATUS_SUCCESS)
            {
                appLogPrintf("RTOS DPL ERROR: Semaphore creation failed\n");
                appLogPrintf("                Check for memory leak, or may need to increase\n");
                break;
            }
            handle = (app_rtos_semaphore_handle_t) &gAppRtosSemaphoreObjs[index];
            gAppRtosSemaphoreUsed[index] = 1U;
            break;
        }
    }

    if (index == APP_RTOS_MAX_SEMAPHORE_COUNT)
    {
        appLogPrintf("RTOS DPL ERROR: Semaphore creation failed\n");
        appLogPrintf("                Need to increase APP_RTOS_MAX_SEMAPHORE_COUNT\n");
    }
    
    return handle;
}

app_rtos_status_t appRtosSemaphoreDelete(app_rtos_semaphore_handle_t* semhandle)
{
    uint16_t index;
    app_rtos_status_t status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;

    if ((NULL != semhandle) && (*semhandle != NULL))
    {
        SemaphoreP_destruct((SemaphoreP_Object *)*semhandle);

        for (index = 0; index < APP_RTOS_MAX_SEMAPHORE_COUNT; index++)
        {
            if (&gAppRtosSemaphoreObjs[index] == (SemaphoreP_Object *)*semhandle)
            {
                gAppRtosSemaphoreUsed[index] = 0U;
                break;
            }
        }

        if (index != APP_RTOS_MAX_SEMAPHORE_COUNT)
        {
            status = APP_RTOS_STATUS_SUCCESS;
        }
    }

    if (status != (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS)
    {
        appLogPrintf("RTOS DPL ERROR: Semaphore deletion failed\n");
    }

    return status;
}

app_rtos_status_t appRtosSemaphorePend(app_rtos_semaphore_handle_t semhandle,
                                         uint32_t timeout)
{
    app_rtos_status_t status;
    uint32_t bsp_timeout;

    if (NULL != semhandle)
    {
        if (APP_RTOS_SEMAPHORE_WAIT_FOREVER == timeout)
        {
            bsp_timeout = SystemP_WAIT_FOREVER;
        }
        else if (APP_RTOS_SEMAPHORE_NO_WAIT == timeout)
        {
            bsp_timeout = SystemP_NO_WAIT;
        }
        else
        {
            bsp_timeout = timeout;
        }

        status = SemaphoreP_pend((SemaphoreP_Object *)semhandle,
                                bsp_timeout);

        if (APP_RTOS_STATUS_TIMEOUT == status)
        {
            appLogPrintf("RTOS DPL ERROR: Semaphore wait failed. Timeout expired.\n");
            status = APP_RTOS_STATUS_TIMEOUT;
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_MCU_PLUS_UM01
<justification end> */
        else if (APP_RTOS_STATUS_FAILURE == status)
        {
            appLogPrintf("RTOS DPL ERROR: Semaphore wait failed. \n");
            status = APP_RTOS_STATUS_FAILURE;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_MCU_PLUS_UM01
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            status = APP_RTOS_STATUS_SUCCESS;
        }
    }   
    else
    {
        appLogPrintf("RTOS DPL ERROR semhandle is NULL\n");
        status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
    }
    return (status);
}

app_rtos_status_t appRtosSemaphorePost(app_rtos_semaphore_handle_t semhandle)
{  
    app_rtos_status_t status = APP_RTOS_STATUS_SUCCESS;
    if (NULL != semhandle)
    { 
        SemaphoreP_post((SemaphoreP_Object *)semhandle); 
    }
    else
    {
        appLogPrintf("RTOS DPL ERROR semhandle is NULL\n");
        status = APP_RTOS_STATUS_FAILURE;
    }
    return status;
}

app_rtos_status_t appRtosSemaphoreReset(app_rtos_semaphore_handle_t semhandle)
{
    if (NULL != semhandle)
    {
        (void)SemaphoreP_pend((SemaphoreP_Object *)semhandle, SystemP_NO_WAIT);
    }

    return ((app_rtos_status_t)APP_RTOS_STATUS_SUCCESS);
}

/* Task */

void appRtosTaskParamsInit(app_rtos_task_params_t *params)
{
    TaskP_Params rtos_task_prms;

    if(params != NULL)
    {
        /* Filling TaskP_Params structure as defined in TaskP.h */
        TaskP_Params_init(&rtos_task_prms);

        params->name        = rtos_task_prms.name;
        params->stacksize   = rtos_task_prms.stackSize;
        params->stack       = rtos_task_prms.stack;
        params->priority    = rtos_task_prms.priority;
        params->arg0        = rtos_task_prms.args;
        params->taskfxn     = NULL;
        params->userData    = NULL;
    }
} 

app_rtos_task_handle_t appRtosTaskCreate(const app_rtos_task_params_t *params)
{
    uint8_t index = 0;
    int32_t status = APP_RTOS_STATUS_FAILURE;
    TaskP_Params rtos_task_prms;
    TaskP_Object* tskHndl = NULL;

    TaskP_Params_init(&rtos_task_prms);
    
    if (NULL != params)
    {
        rtos_task_prms.stackSize = params->stacksize;
        rtos_task_prms.stack     = params->stack;
        rtos_task_prms.priority  = (uint32_t)params->priority;
        rtos_task_prms.args      = params->arg0;
        rtos_task_prms.name      = params->name;
        rtos_task_prms.taskMain  = (TaskP_FxnMain) params->taskfxn;

        for (index = 0; index < APP_RTOS_MAX_TASK_COUNT; index++)
        {
            if (gAppRtosTaskObjUsed[index] == 0U)
            {
                tskHndl = &gAppRtosTaskObjs[index];
                gAppRtosTaskObjUsed[index] = 1U;
                break;
            }
        }

        if (index != APP_RTOS_MAX_TASK_COUNT)
        {
            status = TaskP_construct(tskHndl, &rtos_task_prms);

            if (status != APP_RTOS_STATUS_SUCCESS)
            {
                appLogPrintf("RTOS DPL ERROR: Task handle could not be created\n");
            }
            else
            {
                /* JIRA: ADASVISION-7148 - Temporarily disabled for TDA5, to be enabled once perf_stats is implemented */
                #if !defined (SOC_TDA54)
                (void)appPerfStatsRegisterTask(tskHndl, params->name);
                #endif
            }
        }
        else
        {
            appLogPrintf("RTOS DPL ERROR: Need to increase APP_RTOS_MAX_TASK_COUNT\n");
        }
    }

    return ((app_rtos_task_handle_t)tskHndl);
}

app_rtos_status_t appRtosTaskDelete(app_rtos_task_handle_t* handle)
{
    uint8_t index;
    app_rtos_status_t status = APP_RTOS_STATUS_SUCCESS;
    TaskP_destruct((TaskP_Object*)*handle);
    for (index = 0; index < APP_RTOS_MAX_TASK_COUNT; index++)
    {
        if (&gAppRtosTaskObjs[index] == (TaskP_Object*)*handle)
        {
            gAppRtosTaskObjUsed[index] = 0U;
            break;
        }
    }
    if (index == APP_RTOS_MAX_TASK_COUNT)
    {
        appLogPrintf("RTOS DPL ERROR: Task deletion failed\n");
        status = APP_RTOS_STATUS_FAILURE;
    }

    return status;
}

void appRtosTaskSleep(uint32_t timeout)
{
    ClockP_usleep(ClockP_ticksToUsec(timeout));
}

void appRtosTaskYield(void)
{
    TaskP_yield();
}

void appRtosTaskSleepInMsecs(uint32_t timeoutInMsecs)
{
    ClockP_usleep((uint64_t)timeoutInMsecs * 1000U);
}
