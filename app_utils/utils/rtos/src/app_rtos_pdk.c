/*
 *
 * Copyright (c) 2023 Texas Instruments Incorporated
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

#include <utils/rtos/include/app_rtos.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_log.h>
#include <ti/osal/SemaphoreP.h>
#include <ti/osal/TaskP.h>

#ifndef QNX
extern uint32_t __BSS_START;
extern uint32_t __BSS_END;

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> void extended_system_pre_init.* <function end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_PDK_UM02
<justification end> */
void extended_system_pre_init(void)
{
    /* initialize .bss section to zero */
    uint32_t bss_size = ((uintptr_t)&__BSS_END - (uintptr_t)&__BSS_START);
    memset((void*)&__BSS_START, 0x00, bss_size);
}
#endif

/* Semaphore */
void appRtosSemaphoreParamsInit(app_rtos_semaphore_params_t *params)
{
    if(params != NULL)
    {
        params->mode = (uint32_t)APP_RTOS_SEMAPHORE_MODE_COUNTING;
        params->maxValue = (uint32_t)0xFFU;
        params->initValue = (uint32_t)0U;
    }
}

app_rtos_semaphore_handle_t appRtosSemaphoreCreate(app_rtos_semaphore_params_t params)
{
    SemaphoreP_Handle handle;
    SemaphoreP_Params semParams;

    /* Default parameter initialization */
    SemaphoreP_Params_init(&semParams);

    if (params.mode == (uint32_t)APP_RTOS_SEMAPHORE_MODE_COUNTING)
    {
        semParams.mode = (uint32_t)SemaphoreP_Mode_COUNTING;
    }
    else
    {
        semParams.mode = (uint32_t)SemaphoreP_Mode_BINARY;
    }
    semParams.maxCount = params.maxValue;

    handle = SemaphoreP_create(params.initValue, &semParams);

    if (NULL == handle)
    {
        appLogPrintf("RTOS: Semaphore creation failed\n");
    }

    return (app_rtos_semaphore_handle_t)handle;
}

app_rtos_status_t appRtosSemaphoreDelete(app_rtos_semaphore_handle_t *semhandle)
{
    app_rtos_status_t status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
    SemaphoreP_Handle handle;

    if ((NULL != semhandle) && (*semhandle != NULL))
    {
        handle = (SemaphoreP_Handle)*semhandle;
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RTOS_PDK_UBR001
<justification end> */
        if(SemaphoreP_OK == SemaphoreP_delete(handle))
        {
            *semhandle = NULL;
            status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;
        }
/* LDRA_JUSTIFY_END */
    }

    return (status);
}

app_rtos_status_t appRtosSemaphorePend(app_rtos_semaphore_handle_t semhandle,
                                         uint32_t timeout)
{
    app_rtos_status_t status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;
    SemaphoreP_Status retVal;
    uint32_t bsp_timeout;

    if (NULL != semhandle)
    {
        if (APP_RTOS_SEMAPHORE_WAIT_FOREVER == timeout)
        {
            bsp_timeout = SemaphoreP_WAIT_FOREVER;
        }
        else if (APP_RTOS_SEMAPHORE_NO_WAIT == timeout)
        {
            bsp_timeout = SemaphoreP_NO_WAIT;
        }
        else
        {
            bsp_timeout = timeout;
        }

        retVal = SemaphoreP_pend((SemaphoreP_Handle)semhandle, bsp_timeout);

        if (SemaphoreP_TIMEOUT == retVal)
        {
            status = (app_rtos_status_t)APP_RTOS_STATUS_TIMEOUT;
        }
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_PDK_UM01
<justification end> */
        else if (SemaphoreP_OK != retVal)
        {
            appLogPrintf("RTOS: Semaphore creation failed\n");
            status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_RTOS_PDK_UM01
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;
        }
    }
    else
    {
        appLogPrintf("RTOS: semhandle was NULL\n");
        status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
    }

    return (status);
}

app_rtos_status_t appRtosSemaphorePost(app_rtos_semaphore_handle_t semhandle)
{
    if (NULL != semhandle)
    {
        (void)SemaphoreP_post((SemaphoreP_Handle)semhandle);
    }

    return ((app_rtos_status_t)APP_RTOS_STATUS_SUCCESS);
}

#ifndef QNX /* This function is not needed in QNX code, nor is SemaphoreP_reset implemented on QNX */

app_rtos_status_t appRtosSemaphoreReset(app_rtos_semaphore_handle_t semhandle)
{
    return ((app_rtos_status_t)APP_RTOS_STATUS_SUCCESS);
}

#endif /* #ifndef QNX */

/* Task */

void appRtosTaskParamsInit(app_rtos_task_params_t *params)
{
    TaskP_Params rtos_task_prms;

    if(params != NULL)
    {
        /* Filling TaskP_Params structure as defined in TaskP.h */
        TaskP_Params_init(&rtos_task_prms);

        params->name        = rtos_task_prms.name;
        params->stacksize   = rtos_task_prms.stacksize;
        params->stack       = rtos_task_prms.stack;
        params->priority    = (uint32_t)rtos_task_prms.priority;
        params->arg0        = rtos_task_prms.arg0;
        params->arg1        = rtos_task_prms.arg1;
        params->taskfxn     = NULL;
        params->userData    = NULL;
    }
}

app_rtos_task_handle_t appRtosTaskCreate(const app_rtos_task_params_t *params)
{
    TaskP_Handle tskHndl = NULL;
    TaskP_Params rtos_task_prms;

    if (NULL != params)
    {
        /* Filling TaskP_Params structure as defined in TaskP.h */
        TaskP_Params_init(&rtos_task_prms);

        rtos_task_prms.stacksize = params->stacksize;
        rtos_task_prms.stack     = params->stack;
        rtos_task_prms.priority  = (int8_t)params->priority;
        rtos_task_prms.arg0      = params->arg0;
        rtos_task_prms.arg1      = params->arg1;
        rtos_task_prms.name      = (const char*)&params->name[0];
        rtos_task_prms.userData  = params->userData;

        tskHndl = (void*)TaskP_create(
                            (TaskP_Fxn)params->taskfxn,
                            &rtos_task_prms);

        if (NULL == tskHndl)
        {
            appLogPrintf("RTOS: Task handle could not be created\n");
        }
        else
        {
            (void)appPerfStatsRegisterTask(tskHndl, params->name);
        }
    }
    else
    {
        appLogPrintf("RTOS: Task or params are NUL\n");
    }

    return ((app_rtos_task_handle_t)tskHndl);
}

app_rtos_status_t appRtosTaskDelete(app_rtos_task_handle_t *handle)
{
    app_rtos_status_t status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;

    if ((NULL != handle) && (*handle != NULL))
    {
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_RTOS_PDK_UBR002
<justification end> */
        if(TaskP_OK == TaskP_delete((TaskP_Handle*)handle))
        {
            *handle = NULL;
            status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;
        }
/* LDRA_JUSTIFY_END */
    }

    return (status);
}

uint32_t appRtosTaskIsTerminated(app_rtos_task_handle_t handle)
{
    return TaskP_isTerminated((TaskP_Handle)handle);
}

void appRtosTaskYield(void)
{
    TaskP_yield();
}

void appRtosTaskSleep(uint32_t timeout)
{
    TaskP_sleep(timeout);
}

void appRtosTaskSleepInMsecs(uint32_t timeoutInMsecs)
{
    TaskP_sleepInMsecs(timeoutInMsecs);
}
