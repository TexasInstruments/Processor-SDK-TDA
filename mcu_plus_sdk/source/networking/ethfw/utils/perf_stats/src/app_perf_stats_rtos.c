/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#include <ti/osal/LoadP.h>
#include <ti/osal/osal.h>

#include <ti/drv/enet/examples/utils/include/enet_apputils.h>

#include <utils/remote_service/include/app_remote_service.h>
#include <utils/perf_stats/src/app_perf_stats_priv.h>
#include <utils/console_io/include/app_log.h>

typedef struct {

    uint64_t total_time;
    uint64_t thread_time;

} app_perf_stats_load_t;

typedef struct {

    SemaphoreP_Handle lock;
    app_perf_stats_load_t idleLoad;
    uint32_t num_tasks;
    app_perf_stats_load_t taskLoad[APP_PERF_STATS_TASK_MAX];
    char task_name[APP_PERF_STATS_TASK_MAX][APP_PERF_STATS_TASK_NAME_MAX];
    void *task_handle[APP_PERF_STATS_TASK_MAX];

} app_perf_stats_obj_t;

static app_perf_stats_obj_t g_app_perf_stats_obj;

uint32_t g_perf_stats_load_update_enable = 0;

void appPerfStatsLock(app_perf_stats_obj_t *obj)
{
    if(obj->lock!=NULL)
    {
        SemaphoreP_pend(obj->lock, SemaphoreP_WAIT_FOREVER);
    }
}

void appPerfStatsUnLock(app_perf_stats_obj_t *obj)
{
    if(obj->lock!=NULL)
    {
        SemaphoreP_post(obj->lock);
    }
}

void appPerfStatsResetLoadCalc(app_perf_stats_load_t *load_stats)
{
    load_stats->total_time = 0;
    load_stats->thread_time = 0;
}

uint32_t appPerfStatsLoadCalc(app_perf_stats_load_t *load_stats)
{
    uint32_t load;

    load = (uint32_t)((load_stats->thread_time*10000ul)/load_stats->total_time);

    return load;
}

void appPerfStatsResetLoadCalcAll(app_perf_stats_obj_t *obj)
{
    uint32_t i;

    appPerfStatsLock(obj);

#if defined(FREERTOS)
    /* LoadP_reset() currently supported only for FreeRTOS */
    LoadP_reset();
#endif

    appPerfStatsResetLoadCalc(&obj->idleLoad);
    for(i=0; i<APP_PERF_STATS_TASK_MAX; i++)
    {
        appPerfStatsResetLoadCalc(&obj->taskLoad[i]);
    }

    appPerfStatsUnLock(obj);
}

void appPerfStatsGetTaskLoadAll(app_perf_stats_obj_t *obj, app_perf_stats_task_stats_t *cpu_stats)
{
    uint32_t i;

    appPerfStatsLock(obj);

    cpu_stats->num_tasks = obj->num_tasks;
    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }

    for(i=0; i<cpu_stats->num_tasks; i++)
    {
        strncpy(cpu_stats->task_stats[i].task_name, obj->task_name[i], APP_PERF_STATS_TASK_NAME_MAX);
        cpu_stats->task_stats[i].task_name[APP_PERF_STATS_TASK_NAME_MAX-1]=0;
        cpu_stats->task_stats[i].task_load = appPerfStatsLoadCalc(&obj->taskLoad[i]);
    }

    appPerfStatsUnLock(obj);
}

int32_t appPerfStatsHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    int32_t status = 0;
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;

    switch(cmd)
    {
        case APP_PERF_STATS_CMD_RESET_LOAD_CALC:
            appPerfStatsResetLoadCalcAll(obj);
            break;
        case APP_PERF_STATS_CMD_GET_CPU_LOAD:
            if(prm_size == sizeof(app_perf_stats_cpu_load_t))
            {
                app_perf_stats_cpu_load_t *cpu_load = (app_perf_stats_cpu_load_t*)prm;

                appPerfStatsLock(obj);

                /* LoadP_getCPULoad() currently supported only for FreeRTOS */
#if defined(FREERTOS)
                cpu_load->cpu_load = LoadP_getCPULoad();
#endif
                appPerfStatsUnLock(obj);
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                             cmd, prm_size, sizeof(app_perf_stats_cpu_load_t));
            }
            break;
        case APP_PERF_STATS_CMD_GET_CPU_TASK_STATS:
            if(prm_size == sizeof(app_perf_stats_task_stats_t))
            {
                app_perf_stats_task_stats_t *cpu_stats = (app_perf_stats_task_stats_t*)prm;

                appPerfStatsGetTaskLoadAll(obj, cpu_stats);
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                             cmd, prm_size, sizeof(app_perf_stats_task_stats_t));
            }
            break;
        default:
            status = -1;
            appLogPrintf("PERF STATS: ERROR: Invalid command (cmd = %08x, prm_size = %d B\n", cmd, prm_size);
            break;
    }

    return status;
}

int32_t appPerfStatsInit()
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    int32_t status = 0;
    SemaphoreP_Params semParams;

    memset(obj, 0, sizeof(app_perf_stats_obj_t));

    SemaphoreP_Params_init(&semParams);
    semParams.mode = SemaphoreP_Mode_BINARY;
    obj->lock = SemaphoreP_create(1U, &semParams);
    if(obj->lock==NULL)
    {
        appLogPrintf("PERF STATS: Unable to create lock semaphore\n");
        status = -1;
    }

    if(status==0)
    {
        appPerfStatsResetLoadCalcAll(obj);
        /* now enable load calculation in appPerfStatsRtosLoadUpdate */
        g_perf_stats_load_update_enable = 1;
    }

    return status;
}

int32_t appPerfStatsRemoteServiceInit()
{
    int32_t status;

    status = appRemoteServiceRegister(APP_PERF_STATS_SERVICE_NAME, appPerfStatsHandler);
    if(status!=0)
    {
        appLogPrintf("PERF STATS: ERROR: Unable to register service \n");
    }
    return status;
}

int32_t appPerfStatsDeInit()
{
    appRemoteServiceUnRegister(APP_PERF_STATS_SERVICE_NAME);
    /* SemaphoreP_delete(obj->lock);  DO NOT delete since idle task will keep running even after deinit */
    return 0;
}

void appPerfStatsTaskLoadUpdate(TaskP_Handle task, app_perf_stats_load_t *load)
{
    /* LoadP_getTaskLoad() currently supported only for FreeRTOS */
    LoadP_Stats rtos_load_stat;

    LoadP_getTaskLoad(task, &rtos_load_stat);
    load->total_time += rtos_load_stat.totalTime;
    load->thread_time += rtos_load_stat.threadTime;
}

void appPerfStatsTaskLoadUpdateAll(app_perf_stats_obj_t *obj)
{
    uint32_t i;

    for(i=0; i< obj->num_tasks; i++)
    {
        appPerfStatsTaskLoadUpdate((TaskP_Handle)obj->task_handle[i], &obj->taskLoad[i]);
    }
}

void appPerfStatsRtosLoadUpdate(void)
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;

    if(g_perf_stats_load_update_enable)
    {
        appPerfStatsLock(obj);

        appPerfStatsTaskLoadUpdateAll(obj);

        appPerfStatsUnLock(obj);
    }
}

int32_t appPerfStatsRegisterTask(void *task_handle, char *name)
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    int32_t status = -1;
    uint32_t idx;

    appPerfStatsLock(obj);
    if(obj->num_tasks < APP_PERF_STATS_TASK_MAX
        && task_handle != NULL
        && name != NULL
        )
    {
        idx = obj->num_tasks;

        obj->task_handle[idx] = task_handle;
        strncpy(obj->task_name[idx], name, APP_PERF_STATS_TASK_NAME_MAX);
        obj->task_name[idx][APP_PERF_STATS_TASK_NAME_MAX-1]=0;

        obj->num_tasks = idx + 1;
        status = 0;
    }
    appPerfStatsUnLock(obj);
    return status;
}
