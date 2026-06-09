/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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
#include <string.h>

#include "app_log_priv.h"
#include "app_global_timer_priv.h"

#include <HwiP.h>
#include <TimerP.h>

#include <utils/rtos/include/app_rtos.h>

#if defined(PDK)
#include <sciclient/sciclient.h>
#elif defined(MCU_PLUS_SDK)
#include <kernel/dpl/ClockP.h>
#include <drivers/device_manager/sciclient.h>
#endif

int32_t appLogWrCreateLock(app_log_wr_obj_t *obj)
{
    int32_t status = 0;
    app_rtos_semaphore_params_t semParams;

    appRtosSemaphoreParamsInit(&semParams);

    semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
    semParams.initValue = 1U;

    obj->lock = appRtosSemaphoreCreate(semParams);
    if(obj->lock==NULL)
    {
        status = -1;
    }
    return status;
}

uintptr_t appLogWrLock(app_log_wr_obj_t *obj)
{
    if(obj->lock)
    {
        appRtosSemaphorePend(obj->lock, APP_RTOS_SEMAPHORE_WAIT_FOREVER);
    }
    return (uintptr_t)0;
}

void appLogWrUnLock(app_log_wr_obj_t *obj, uintptr_t key)
{
    if(obj->lock)
    {
        appRtosSemaphorePost(obj->lock);
    }
}

int32_t   appLogRdCreateTask(app_log_rd_obj_t *obj, app_log_init_prm_t *prm)
{
    app_rtos_task_params_t rtos_task_prms;
    int32_t status = 0;

    appRtosTaskParamsInit(&rtos_task_prms);

    rtos_task_prms.stacksize = obj->task_stack_size;
    rtos_task_prms.stack = obj->task_stack;
    rtos_task_prms.priority = prm->log_rd_task_pri;
    rtos_task_prms.arg0 = (void*)(obj);
    rtos_task_prms.arg1 = NULL;
    rtos_task_prms.name = (const char*)&obj->task_name[0];
    rtos_task_prms.taskfxn   = &appLogRdRun;

    (void)strncpy(obj->task_name, "LOG_RD", APP_LOG_MAX_TASK_NAME);
    obj->task_name[APP_LOG_MAX_TASK_NAME-1] = 0;

    obj->task_handle = (void*)appRtosTaskCreate(&rtos_task_prms);
    if(obj->task_handle==NULL)
    {
        status = -1;
    }
    return status;
}
