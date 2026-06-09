/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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
#include <utils/pc_osal/include/dpl_osal.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include <TaskP.h>

typedef struct {
    pthread_t hndl; /**< Thread handle */
} app_pc_task_handle_t;

typedef struct {
    void (*taskfxn)(void *arg0, void *arg1);
    void *arg0;
    void *arg1;
} app_pc_task_fxn_args_t;

void appRtosTaskParamsInit(app_rtos_task_params_t *params)
{
    if(params != NULL)
    {
        params->name        = "Task (PC)";
        params->stacksize   = (uint32_t)0U;
        params->stack       = NULL;
        params->priority    = (uint32_t)0U;
        params->taskfxn     = NULL;
        params->arg0        = NULL;
    }
}

static void *appPcTaskFxnWrapper(void *args);
static void *appPcTaskFxnWrapper(void *args)
{
    app_pc_task_fxn_args_t *wrapper_args = (app_pc_task_fxn_args_t *)args;

    wrapper_args->taskfxn(wrapper_args->arg0, wrapper_args->arg1);

    free(wrapper_args);

    return NULL;
}

app_rtos_task_handle_t appRtosTaskCreate(const app_rtos_task_params_t *params)
{
    int32_t status = 0;
    app_pc_task_handle_t *task_handle = NULL;

    if ((NULL != params) && (NULL != params->taskfxn))
    {
        pthread_attr_t thread_attr;

        status = pthread_attr_init(&thread_attr);

        if (status == 0)
        {
            if(params->stacksize > 0U)
            {
                status = pthread_attr_setstacksize(&thread_attr, params->stacksize);
            }

            if (status == 0)
            {
                task_handle = malloc(sizeof(app_pc_task_handle_t));
                if (task_handle == NULL)
                {
                    printf("ERROR - appRtosTaskCreate: Failed to allocate memory for task handle\n");
                    status = -1;
                }

                if (status == 0)
                {
                    app_pc_task_fxn_args_t *wrapper_args = malloc(sizeof(app_pc_task_fxn_args_t));
                    if (wrapper_args == NULL)
                    {
                        printf("ERROR - appRtosTaskCreate: Failed to allocate memory for wrapper args\n");
                        status = -1;
                    }

                    if (status == 0)
                    {
                        wrapper_args->taskfxn = params->taskfxn;
                        wrapper_args->arg0 = params->arg0;
                        wrapper_args->arg1 = params->arg1;

                        status = pthread_create(&task_handle->hndl, &thread_attr, &appPcTaskFxnWrapper, wrapper_args);
                        if (status != 0)
                        {
                            printf("ERROR - appRtosTaskCreate: pthread_create failed\n");
                            free(wrapper_args);
                        }
                    }
                    free(task_handle);
                }
            }

            (void)pthread_attr_destroy(&thread_attr);
        }
    }
    else
    {
        printf("ERROR - appRtosTaskCreate: params or params->taskMain is NULL\n");
        status = -1;
    }

    return ((app_rtos_task_handle_t)task_handle);
}

app_rtos_status_t appRtosTaskDelete(app_rtos_task_handle_t *handle)
{
    app_rtos_status_t status = APP_RTOS_STATUS_SUCCESS;

    if (handle != NULL)
    {
        uint32_t ret_val;
        app_pc_task_handle_t *task_handle = (app_pc_task_handle_t *)handle;

        status = pthread_cancel(task_handle->hndl);
        status = pthread_join(task_handle->hndl, (void *)&ret_val);

        if (ret_val != 0)
        {
            printf("ERROR - appRtosTaskDelete: pthread_join failed with status %d and return %u\n", status, (uint32_t)ret_val);
        }
    }
    else
    {
        printf("ERROR - appRtosTaskDelete: Invalid thread handle\n");
    }

    return status;
}

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
    int32_t status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;
    SemaphoreP_Object *handle = malloc(sizeof(SemaphoreP_Object));

    if (handle == NULL)
    {
        status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
    }

    if (status == (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS)
    {
        status = SemaphoreP_constructBinary(handle, params.initValue);
    }

    if (status != (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS)
    {
        printf("ERROR - appRtosSemaphoreCreate: SemaphoreP_constructBinary failed or memory allocation failed\n");
        free(handle);
        handle = NULL;
    }

    return (app_rtos_semaphore_handle_t)handle;
}

app_rtos_status_t appRtosSemaphoreDelete(app_rtos_semaphore_handle_t* semhandle)
{
    app_rtos_status_t status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;

    SemaphoreP_destruct((SemaphoreP_Object *)*semhandle);

    return status;
}

app_rtos_status_t appRtosSemaphorePend(app_rtos_semaphore_handle_t semhandle, uint32_t timeout)
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

        if ((app_rtos_status_t)APP_RTOS_STATUS_TIMEOUT == status)
        {
            printf("ERROR - appRtosSemaphorePend: Semaphore wait failed. Timeout expired.\n");
            status = (app_rtos_status_t)APP_RTOS_STATUS_TIMEOUT;
        }
        else if ((app_rtos_status_t)APP_RTOS_STATUS_FAILURE == status)
        {
            printf("ERROR - appRtosSemaphorePend: Semaphore wait failed. \n");
            status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
        }
        else
        {
            status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;
        }
    }
    else
    {
        printf("ERROR - appRtosSemaphorePend: semhandle is NULL\n");
        status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
    }
    return (status);
}

app_rtos_status_t appRtosSemaphorePost(app_rtos_semaphore_handle_t semhandle)
{
    app_rtos_status_t status = (app_rtos_status_t)APP_RTOS_STATUS_SUCCESS;

    if (NULL != semhandle)
    {
        SemaphoreP_post((SemaphoreP_Object *)semhandle);
    }
    else
    {
        appLogPrintf("ERROR - appRtosSemaphorePost: semhandle is NULL\n");
        status = (app_rtos_status_t)APP_RTOS_STATUS_FAILURE;
    }
    return status;
}
