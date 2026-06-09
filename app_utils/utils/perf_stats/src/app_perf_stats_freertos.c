/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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

#include <stdio.h>
#include <string.h>

#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/mem/include/app_mem.h>
#include <utils/rtos/include/app_rtos.h>

#if defined(R5F) || defined(M55)
#include <utils/ipc/include/app_ipc.h>
#endif
#include <HwiP.h>
#include "app_perf_stats_priv.h"
#include <inttypes.h>

#if defined(FREERTOS)
#if defined(PDK)
#include <LoadP.h>
#elif defined(MCU_PLUS_SDK)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <TaskP.h>
#endif
#endif

#if defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
#define APP_PERF_DDR_MHZ                (2133u)  /* DDR clock speed in MHZ */
#elif defined(SOC_AM62A)
#define APP_PERF_DDR_MHZ                (1866u)  /* DDR clock speed in MHZ */
#elif defined(SOC_TDA54)
#define APP_PERF_DDR_MHZ                (4266u)  /* DDR clock speed in MHZ */
#endif

#define APP_PERF_DDR_BUS_WIDTH          (  32u)  /* in units of bits */
#define APP_PERF_DDR_BURST_SIZE_BYTES   (  64u)  /* in units of bytes */

#define APP_PERF_DDR_STATS_CTR0         (0x00) /* A value of 0x00 configures counter 0 to return number of write transactions  */
#define APP_PERF_DDR_STATS_CTR1         (0x01) /* A value of 0x01 configures counter 1 to return number of read transactions   */
/* Use counter 2 and 3 to provide stats other than read/write transactions */
#define APP_PERF_DDR_STATS_CTR2         (0x03) /* A value of 0x03 configures counter 3 to return number of command activations */
#define APP_PERF_DDR_STATS_CTR3         (0x1C) /* A value of 0x1C configures counter 4 to return number of queue full states   */

#if defined(SOC_J721E) || defined(SOC_AM62A) ||  defined(SOC_J722S)
#define APP_PERF_NUM_DDR_INSTANCES      (1u)
#elif defined (SOC_J721S2) || defined (SOC_J742S2)
#define APP_PERF_NUM_DDR_INSTANCES      (2u)
#elif defined (SOC_J784S4) || defined (SOC_TDA54)
#define APP_PERF_NUM_DDR_INSTANCES      (4u)
#endif

/** \brief Max number of tasks whose information can be locally maintained (transmitted may be smaller) */
#define APP_PERF_STATS_LOCAL_TASK_MAX (40u)

/* Define this to print counter2 and counter3 values */
//#define APP_PERF_SHOW_DDR_STATS

/* Specify the duration for with counter2 and counter3 values are to be accumulated before printing */
#define APP_PERF_SNAPSHOT_WINDOW_WIDTH (500000 * 4) /* Configured for 2 seconds */

typedef struct {

    uint64_t total_time;
    uint64_t thread_time;

} app_perf_stats_load_t;

typedef struct {

    app_perf_stats_ddr_stats_t ddr_stats;
    uint64_t total_time;
    uint64_t last_timestamp;
    uint64_t total_read;
    uint64_t total_write;
    int32_t snapshot_count;

} app_perf_stats_ddr_load_t;

typedef struct {

    app_rtos_semaphore_handle_t lock;
    app_perf_stats_load_t idleLoad;
    uint32_t num_tasks;
    app_perf_stats_load_t taskLoad[APP_PERF_STATS_LOCAL_TASK_MAX];
    char task_name[APP_PERF_STATS_LOCAL_TASK_MAX][APP_PERF_STATS_TASK_NAME_MAX];
    void *task_handle[APP_PERF_STATS_LOCAL_TASK_MAX];
    app_perf_stats_hwa_stats_t hwaLoad;
    app_perf_stats_ddr_load_t ddrLoad;

} app_perf_stats_obj_t;

static app_perf_stats_obj_t g_app_perf_stats_obj;
static app_perf_registration_t *g_app_perf_reg_fxns = NULL;

uint32_t g_perf_stats_load_update_enable = 0;

void appPerfStatsResetHwaLoadCalcAll();
void appPerfStatsResetDdrLoadCalcAll();
void appPerfStatsDddrStatsUpdate();
void appPerfStatsDdrStatsReadCounters(uint32_t *val0, uint32_t *val1, uint32_t *val2, uint32_t *val3, bool raw);

void appPerfStatsLock(app_perf_stats_obj_t *obj)
{
    if(obj->lock!=NULL)
    {
        appRtosSemaphorePend(obj->lock, APP_RTOS_SEMAPHORE_WAIT_FOREVER);
    }
}

void appPerfStatsUnLock(app_perf_stats_obj_t *obj)
{
    if(obj->lock!=NULL)
    {
        appRtosSemaphorePost(obj->lock);
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

    appPerfStatsResetLoadCalc(&obj->idleLoad);
    for(i=0; i<APP_PERF_STATS_LOCAL_TASK_MAX; i++)
    {
        appPerfStatsResetLoadCalc(&obj->taskLoad[i]);
    }

    appPerfStatsUnLock(obj);
}

void appPerfStatsTaskLoadUpdate(app_rtos_task_handle_t task, app_perf_stats_load_t *load)
{

#if defined(PDK)
    #if defined(FREERTOS)
    LoadP_Stats rtos_load_stat;

    LoadP_getTaskLoad(task, &rtos_load_stat);

    load->total_time = rtos_load_stat.totalTime;
    load->thread_time = rtos_load_stat.threadTime;
    #endif
#elif defined(MCU_PLUS_SDK)
    #if defined(FREERTOS)
    TaskP_Load rtos_load_stat;

    TaskP_loadGet(task, &rtos_load_stat);

    load->total_time = rtos_load_stat.totalTime;
    load->thread_time = rtos_load_stat.runTime;
    #endif
#endif

}

void appPerfStatsTaskLoadUpdateAll(app_perf_stats_obj_t *obj)
{
    uint32_t i;

    for(i=0; i< obj->num_tasks; i++)
    {
        appPerfStatsTaskLoadUpdate((app_rtos_task_handle_t)obj->task_handle[i], &obj->taskLoad[i]);
    }
}

void appPerfStatsGetTaskLoadAll(app_perf_stats_obj_t *obj, app_perf_stats_task_stats_t *cpu_stats)
{
    uint32_t i, j;

    appPerfStatsLock(obj);

    appPerfStatsTaskLoadUpdateAll(obj);

    cpu_stats->num_tasks = obj->num_tasks - cpu_stats->starting_task;
    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }

    j = cpu_stats->starting_task;
    for(i=0; i<cpu_stats->num_tasks; i++)
    {
        strncpy(cpu_stats->task_stats[i].task_name, obj->task_name[j], APP_PERF_STATS_TASK_NAME_MAX -1U);
        cpu_stats->task_stats[i].task_name[APP_PERF_STATS_TASK_NAME_MAX-1]=0;
        cpu_stats->task_stats[i].task_load = appPerfStatsLoadCalc(&obj->taskLoad[j]);
        j++;
    }

    appPerfStatsUnLock(obj);
}

void appPerfStatsGetTaskStackWatermarksAll(app_perf_stats_obj_t *obj, app_perf_stats_task_stats_t *cpu_stats)
{
    uint32_t i, j;

    appPerfStatsLock(obj);

    cpu_stats->num_tasks = obj->num_tasks - cpu_stats->starting_task;
    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }

    j = cpu_stats->starting_task;
    for(i=0; i<cpu_stats->num_tasks; i++)
    {
        strncpy(cpu_stats->task_stats[i].task_name, obj->task_name[j], APP_PERF_STATS_TASK_NAME_MAX -1U);
        cpu_stats->task_stats[i].task_name[APP_PERF_STATS_TASK_NAME_MAX-1]=0;
#if defined(PDK)
        cpu_stats->task_stats[i].task_load = TaskP_getTaskStackHighWatermark((TaskP_Handle)obj->task_handle[j]);
#else
        /* MCU+ does not yet support this */
        cpu_stats->task_stats[i].task_load = 0;
#endif
        j++;
    }

    appPerfStatsUnLock(obj);
}

void appPerfStatsGetMemStatsAll(app_perf_stats_obj_t *obj, app_perf_stats_mem_stats_t *cpu_stats)
{
    uint32_t i;

    appPerfStatsLock(obj);

    for(i=0; i<APP_MEM_HEAP_MAX; i++)
    {
        appMemStats(i, &cpu_stats->mem_stats[i]);
    }

    appPerfStatsUnLock(obj);

}

#if defined(PDK)
extern uint32_t  gOsalSemAllocCnt, gOsalSemPeak;
extern uint32_t  gOsalMutexAllocCnt, gOsalMutexPeak;
extern uint32_t  gOsalQueueAllocCnt, gOsalQueuePeak;
extern uint32_t  gOsalEventAllocCnt, gOsalEventPeak;
extern uint32_t  gOsalHeapAllocCnt, gOsalHeapPeak;
extern uint32_t  gOsalMailboxAllocCnt, gOsalMailboxPeak;
extern uint32_t  gOsalTaskAllocCnt, gOsalTaskPeak;
extern uint32_t  gOsalClockAllocCnt, gOsalClockPeak;
extern uint32_t  gOsalHwiAllocCnt, gOsalHwiPeak;
extern uint32_t  gOsalTimerAllocCnt, gOsalTimerPeak;

#elif defined(MCU_PLUS_SDK) || defined(MCU_SDK)
uint32_t  gOsalSemAllocCnt, gOsalSemPeak;
uint32_t  gOsalMutexAllocCnt, gOsalMutexPeak;
uint32_t  gOsalQueueAllocCnt, gOsalQueuePeak;
uint32_t  gOsalEventAllocCnt, gOsalEventPeak;
uint32_t  gOsalHeapAllocCnt, gOsalHeapPeak;
uint32_t  gOsalMailboxAllocCnt, gOsalMailboxPeak;
uint32_t  gOsalTaskAllocCnt, gOsalTaskPeak;
uint32_t  gOsalClockAllocCnt, gOsalClockPeak;
#if defined(CPU_c7504) || defined(CPU_c7524)
extern uint32_t  gOsalHwiAllocCnt, gOsalHwiPeak;
#else
uint32_t  gOsalHwiAllocCnt, gOsalHwiPeak;
#endif
uint32_t  gOsalTimerAllocCnt, gOsalTimerPeak;
#endif

void appPerfStatsGetOsStatsAll(app_perf_stats_obj_t *obj, app_perf_stats_os_stats_t *os_stats)
{
    if (NULL != os_stats)
    {
        memset(os_stats, 0, sizeof(app_perf_stats_os_stats_t));

        os_stats->semaphore_count  = gOsalSemAllocCnt;
        os_stats->mutex_count      = gOsalMutexAllocCnt;
        os_stats->queue_count      = gOsalQueueAllocCnt;
        os_stats->event_count      = gOsalEventAllocCnt;
        os_stats->heap_count       = gOsalHeapAllocCnt;
        os_stats->mailbox_count    = gOsalMailboxAllocCnt;
        os_stats->task_count       = gOsalTaskAllocCnt;
        os_stats->clock_count      = gOsalClockAllocCnt;
        os_stats->hwi_count        = gOsalHwiAllocCnt;
        os_stats->timer_count      = gOsalTimerAllocCnt;

        os_stats->semaphore_peak   = gOsalSemPeak;
        os_stats->mutex_peak       = gOsalMutexPeak;
        os_stats->queue_peak       = gOsalQueueAllocCnt;
        os_stats->event_peak       = gOsalEventPeak;
        os_stats->heap_peak        = gOsalHeapPeak;
        os_stats->mailbox_peak     = gOsalMailboxPeak;
        os_stats->task_peak        = gOsalTaskPeak;
        os_stats->clock_peak       = gOsalClockPeak;
        os_stats->hwi_peak         = gOsalHwiPeak;
        os_stats->timer_peak       = gOsalTimerPeak;
    }
}

int32_t appPerfStatsHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    int32_t status = 0;
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;

    switch(cmd)
    {
        case APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC:
            appPerfStatsResetHwaLoadCalcAll();
            break;
        case APP_PERF_STATS_CMD_RESET_LOAD_CALC:
            #if defined(FREERTOS)

            #if defined(PDK)
            LoadP_reset();
            #elif defined(MCU_PLUS_SDK)
            TaskP_loadResetAll();
            #endif

            #endif
            appPerfStatsResetLoadCalcAll(obj);
            break;
        case APP_PERF_STATS_CMD_RESET_DDR_STATS:
            appPerfStatsResetDdrLoadCalcAll();
            break;
        case APP_PERF_STATS_CMD_GET_DDR_COUNTERS:
            if(prm_size == sizeof(app_perf_stats_ddr_stats_t))
            {
                app_perf_stats_ddr_stats_t *ddrLoad = (app_perf_stats_ddr_stats_t*)prm;

                uint32_t cookie;

                /* interrupts disabled since update happens in ISR */
                cookie = HwiP_disable();

                appPerfStatsDdrStatsReadCounters(&ddrLoad->counter0_total,
                        &ddrLoad->counter1_total,
                        &ddrLoad->counter2_total,
                        &ddrLoad->counter3_total,
                        true);

                HwiP_restore(cookie);

            }
            break;
        case APP_PERF_STATS_CMD_GET_DDR_STATS:
            if(prm_size == sizeof(app_perf_stats_ddr_stats_t))
            {
                app_perf_stats_ddr_stats_t *ddr_load = (app_perf_stats_ddr_stats_t*)prm;

                uint32_t cookie;

                appPerfStatsLock(obj);
                /* interrupts disabled since update happens in ISR */
                cookie = HwiP_disable();

                *ddr_load = obj->ddrLoad.ddr_stats;

                HwiP_restore(cookie);

                appPerfStatsUnLock(obj);
            }
            break;
        case APP_PERF_STATS_CMD_GET_HWA_LOAD:
            if(prm_size == sizeof(app_perf_stats_hwa_stats_t))
            {
                app_perf_stats_hwa_stats_t *hwa_load = (app_perf_stats_hwa_stats_t*)prm;

                appPerfStatsLock(obj);

                *hwa_load = obj->hwaLoad;

                appPerfStatsUnLock(obj);

            }
            break;
        case APP_PERF_STATS_CMD_GET_CPU_LOAD:
            if(prm_size == sizeof(app_perf_stats_cpu_load_t))
            {
                app_perf_stats_cpu_load_t *cpu_load = (app_perf_stats_cpu_load_t*)prm;

                appPerfStatsLock(obj);

                #if defined(FREERTOS)
                /* Multiplying by 100 to show decimal points when printing */

                #if defined(PDK)
                cpu_load->cpu_load = 100 * LoadP_getCPULoad();
                #elif defined(MCU_PLUS_SDK)
                cpu_load->cpu_load = TaskP_loadGetTotalCpuLoad();
                #endif

                #endif

                cpu_load->hwi_load = 0U;
                cpu_load->swi_load = 0U;

                appPerfStatsUnLock(obj);
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                    cmd,
                    prm_size,
                    sizeof(app_perf_stats_cpu_load_t)
                    );
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
                    cmd,
                    prm_size,
                    sizeof(app_perf_stats_task_stats_t)
                    );
            }
            break;
        case APP_PERF_STATS_CMD_GET_CPU_TASK_STACKS_STATS:
            if(prm_size == sizeof(app_perf_stats_task_stats_t))
            {
                app_perf_stats_task_stats_t *cpu_stats = (app_perf_stats_task_stats_t*)prm;

                appPerfStatsGetTaskStackWatermarksAll(obj, cpu_stats);
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                    cmd,
                    prm_size,
                    sizeof(app_perf_stats_task_stats_t)
                    );
            }
            break;
        case APP_PERF_STATS_CMD_GET_CPU_MEM_STATS:
            if(prm_size == sizeof(app_perf_stats_mem_stats_t))
            {
                app_perf_stats_mem_stats_t *cpu_stats = (app_perf_stats_mem_stats_t*)prm;

                appPerfStatsGetMemStatsAll(obj, cpu_stats);
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                    cmd,
                    prm_size,
                    sizeof(app_perf_stats_mem_stats_t)
                    );
            }
            break;
        case APP_PERF_STATS_CMD_GET_OS_STATS:
            if(prm_size == sizeof(app_perf_stats_os_stats_t))
            {
                app_perf_stats_os_stats_t *os_stats = (app_perf_stats_os_stats_t*)prm;

                appPerfStatsGetOsStatsAll(obj, os_stats);
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                    cmd,
                    prm_size,
                    sizeof(app_perf_stats_os_stats_t)
                    );
            }
            break;
        case APP_PERF_STATS_CMD_GET_TIOVX_STATS:
            if(prm_size == sizeof(app_perf_stats_tiovx_stats_t))
            {
                app_perf_stats_tiovx_stats_t *tiovx_stats = (app_perf_stats_tiovx_stats_t*)prm;

                if (g_app_perf_reg_fxns != NULL)
                {
                    g_app_perf_reg_fxns->get_tiovx_stats(tiovx_stats);
                }
                else
                {
                    appLogPrintf("PERF STATS: WARNING: Callback registration unsuccessful (cmd = %08x)\n", cmd);
                }
            }
            else
            {
                status = -1;
                appLogPrintf("PERF STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                    cmd,
                    prm_size,
                    sizeof(app_perf_stats_tiovx_stats_t)
                    );
            }
            break;
        default:
            status = -1;
            appLogPrintf("PERF STATS: ERROR: Invalid command (cmd = %08x, prm_size = %d B\n",
                cmd,
                prm_size
                );
            break;
    }

    return status;
}

int32_t appPerfStatsInit()
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    int32_t status = 0;
    app_rtos_semaphore_params_t semParams;

    if(0 == g_perf_stats_load_update_enable)
    {
        memset(obj, 0, sizeof(app_perf_stats_obj_t));

        appRtosSemaphoreParamsInit(&semParams);

        semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
        semParams.initValue = 1U;

        obj->lock = appRtosSemaphoreCreate(semParams);
        if(obj->lock==NULL)
        {
            appLogPrintf("PERF STATS: Unable to create lock semaphore\n");
            status = -1;
        }

        if(status==0)
        {
            #if defined(FREERTOS)

            #if defined(PDK)
            LoadP_reset();
            #elif defined(MCU_PLUS_SDK)
            TaskP_loadResetAll();
            #endif

            #endif
            appPerfStatsResetLoadCalcAll(obj);
            appPerfStatsResetHwaLoadCalcAll();
            appPerfStatsResetDdrLoadCalcAll();
        }
        if(status==0)
        {
            /* now enable load calculation in appPerfStatsRtosLoadUpdate */
            g_perf_stats_load_update_enable = 1U;
        }
    }

    return status;
}

int32_t appPerfStatsInitRegister(app_perf_registration_t * reg_fxns)
{
    if (reg_fxns != NULL)
    {
        g_app_perf_reg_fxns = reg_fxns;
    }

    return appPerfStatsInit();
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
    g_perf_stats_load_update_enable = 0;
    appRemoteServiceUnRegister(APP_PERF_STATS_SERVICE_NAME);
    /* appRtosSemaphoreDelete(obj->lock);  DO NOT delete since idle task will keep running even after deinit */
    return 0;
}

void appPerfStatsRtosLoadUpdate(void)
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;

    if(1U == g_perf_stats_load_update_enable)
    {
        appPerfStatsLock(obj);

        appPerfStatsTaskLoadUpdateAll(obj);

        #ifdef R5F
        /* not taken in inside lock since interrupts are disabled for locking
         * interrupts are disable since counters are read in ISR context
         */
        appPerfStatsDddrStatsUpdate();
        #endif
        appPerfStatsUnLock(obj);
    }
}

int32_t appPerfStatsRegisterTask(void *task_handle, const char *name)
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    int32_t status = -1;
    uint32_t idx;

    if(1U == g_perf_stats_load_update_enable)
    {
        appPerfStatsLock(obj);
        if(obj->num_tasks < APP_PERF_STATS_LOCAL_TASK_MAX
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
    }
    return status;
}

void appPerfStatsHwaResetLoadCalc(app_perf_hwa_id_t id)
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    app_perf_stats_hwa_load_t *hwaLoad;

    if(id < APP_PERF_HWA_MAX)
    {
        hwaLoad = &obj->hwaLoad.hwa_stats[id];

        appPerfStatsLock(obj);

        hwaLoad->total_time = 0;
        hwaLoad->active_time = 0;
        hwaLoad->pixels_processed = 0;
        hwaLoad->last_timestamp = appLogGetTimeInUsec();

        appPerfStatsUnLock(obj);
    }
}

void appPerfStatsResetHwaLoadCalcAll()
{
    app_perf_hwa_id_t hwa_id;

    for(hwa_id = (app_perf_hwa_id_t)0; hwa_id <APP_PERF_HWA_MAX;hwa_id++)
    {
        appPerfStatsHwaResetLoadCalc(hwa_id);
    }
}

void appPerfStatsHwaUpdateLoad(app_perf_hwa_id_t id, uint32_t active_time_in_usecs, uint32_t pixels_processed)
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    app_perf_stats_hwa_load_t *hwaLoad;
    uint64_t cur_time;

    if(id < APP_PERF_HWA_MAX)
    {
        hwaLoad = &obj->hwaLoad.hwa_stats[id];

        appPerfStatsLock(obj);

        cur_time = appLogGetTimeInUsec();

        if(cur_time > hwaLoad->last_timestamp)
        {
            hwaLoad->total_time += (cur_time - hwaLoad->last_timestamp);
            hwaLoad->active_time += active_time_in_usecs;
            hwaLoad->pixels_processed += pixels_processed;
        }
        hwaLoad->last_timestamp = cur_time;

        appPerfStatsUnLock(obj);
    }
}

/* read EMIF counter and calculate read and write bytes since last read */
void appPerfStatsDdrStatsReadCounters(uint32_t *val0, uint32_t *val1, uint32_t *val2, uint32_t *val3, bool raw)
{
    static uint32_t is_first_time = 1;
    static volatile uint32_t *cnt_sel[APP_PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt0[APP_PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt1[APP_PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt2[APP_PERF_NUM_DDR_INSTANCES];
    static volatile uint32_t *cnt3[APP_PERF_NUM_DDR_INSTANCES];

    static volatile uint32_t last_cnt0 = 0, last_cnt1 = 0, last_cnt2 = 0, last_cnt3 = 0;
    volatile uint32_t cur_cnt0 = 0, cur_cnt1 = 0, cur_cnt2 = 0, cur_cnt3 = 0;
    uint32_t diff_cnt0, diff_cnt1, diff_cnt2, diff_cnt3, ddr_inst;


    #if defined(SOC_AM62A) || defined(SOC_J722S)
    cnt_sel[0] = (volatile uint32_t *)0x0f300100;
    cnt0[0]    = (volatile uint32_t *)0x0f300104;
    cnt1[0]    = (volatile uint32_t *)0x0f300108;
    cnt2[0]    = (volatile uint32_t *)0x0f30010C;
    cnt3[0]    = (volatile uint32_t *)0x0f300110;
    #else
    cnt_sel[0] = (volatile uint32_t *)0x02980100;
    cnt0[0]    = (volatile uint32_t *)0x02980104;
    cnt1[0]    = (volatile uint32_t *)0x02980108;
    cnt2[0]    = (volatile uint32_t *)0x0298010C;
    cnt3[0]    = (volatile uint32_t *)0x02980110;
    #endif

    #if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    cnt_sel[1] = (volatile uint32_t *)0x029A0100;
    cnt0[1]    = (volatile uint32_t *)0x029A0104;
    cnt1[1]    = (volatile uint32_t *)0x029A0108;
    cnt2[1]    = (volatile uint32_t *)0x029A010C;
    cnt3[1]    = (volatile uint32_t *)0x029A0110;
    #endif

    #if defined(SOC_J784S4)
    cnt_sel[2] = (volatile uint32_t *)0x029C0100;
    cnt0[2]    = (volatile uint32_t *)0x029C0104;
    cnt1[2]    = (volatile uint32_t *)0x029C0108;
    cnt2[2]    = (volatile uint32_t *)0x029C010C;
    cnt3[2]    = (volatile uint32_t *)0x029C0110;

    cnt_sel[3] = (volatile uint32_t *)0x029E0100;
    cnt0[3]    = (volatile uint32_t *)0x029E0104;
    cnt1[3]    = (volatile uint32_t *)0x029E0108;
    cnt2[3]    = (volatile uint32_t *)0x029E010C;
    cnt3[3]    = (volatile uint32_t *)0x029E0110;
    #endif

    if(is_first_time)
    {
        is_first_time = 0;

        for (ddr_inst = 0; ddr_inst < APP_PERF_NUM_DDR_INSTANCES; ddr_inst++)
        {
            /* cnt0 is counting reads, cnt1 is counting writes, cnt2, cnt3 not used */
            *cnt_sel[ddr_inst] = (APP_PERF_DDR_STATS_CTR0 <<  0u) |
                       (APP_PERF_DDR_STATS_CTR1 <<  8u) |
                       (APP_PERF_DDR_STATS_CTR2 << 16u) |
                       (APP_PERF_DDR_STATS_CTR3 << 24u);

            last_cnt0 += *cnt0[ddr_inst];
            last_cnt1 += *cnt1[ddr_inst];
            last_cnt2 += *cnt2[ddr_inst];
            last_cnt3 += *cnt3[ddr_inst];
        }
    }

    for (ddr_inst = 0; ddr_inst < APP_PERF_NUM_DDR_INSTANCES; ddr_inst++)
    {
        cur_cnt0 += *cnt0[ddr_inst];
        cur_cnt1 += *cnt1[ddr_inst];
        cur_cnt2 += *cnt2[ddr_inst];
        cur_cnt3 += *cnt3[ddr_inst];
    }

    if(raw)
    {
        *val0 = (uint32_t)cur_cnt0;
        *val1 = (uint32_t)cur_cnt1;
        *val2 = (uint32_t)cur_cnt2;
        *val3 = (uint32_t)cur_cnt3;
        return;
    }

    if(cur_cnt0 < last_cnt0)
    {
        /* wrap around case */
        diff_cnt0 = (0xFFFFFFFFu - last_cnt0) + cur_cnt0;
    }
    else
    {
        diff_cnt0 = cur_cnt0 - last_cnt0;
    }

    if(cur_cnt1 < last_cnt1)
    {
        /* wrap around case */
        diff_cnt1 = (0xFFFFFFFFu - last_cnt1) + cur_cnt1;
    }
    else
    {
        diff_cnt1 = cur_cnt1 - last_cnt1;
    }

    if(cur_cnt2 < last_cnt2)
    {
        /* wrap around case */
        diff_cnt2 = (0xFFFFFFFFu - last_cnt2) + cur_cnt2;
    }
    else
    {
        diff_cnt2 = cur_cnt2 - last_cnt2;
    }

    if(cur_cnt3 < last_cnt3)
    {
        /* wrap around case */
        diff_cnt3 = (0xFFFFFFFFu - last_cnt3) + cur_cnt3;
    }
    else
    {
        diff_cnt3 = cur_cnt3 - last_cnt3;
    }


    last_cnt0 = cur_cnt0;
    last_cnt1 = cur_cnt1;
    last_cnt2 = cur_cnt2;
    last_cnt3 = cur_cnt3;

    *val0 = (uint32_t)diff_cnt0;
    *val1 = (uint32_t)diff_cnt1;
    *val2 = (uint32_t)diff_cnt2;
    *val3 = (uint32_t)diff_cnt3;
}

void appPerfStatsDddrStatsUpdate()
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    app_perf_stats_ddr_load_t *ddrLoad;
    uint32_t cookie;
    uint32_t val0 = 0, val1 = 0, val2 = 0, val3 = 0;
    uint64_t cur_time;
    uint32_t elapsed_time;

    ddrLoad = &obj->ddrLoad;

    cookie = HwiP_disable();

    cur_time = appLogGetTimeInUsec();
    if(cur_time > ddrLoad->last_timestamp)
    {
        elapsed_time = cur_time - ddrLoad->last_timestamp;
        if(elapsed_time==0)
            elapsed_time = 1; /* to avoid divide by 0 */
        ddrLoad->total_time += elapsed_time;
        appPerfStatsDdrStatsReadCounters(&val0, &val1, &val2, &val3, false);

        uint64_t write_bytes = val0 * APP_PERF_DDR_BURST_SIZE_BYTES;
        uint64_t read_bytes  = val1 * APP_PERF_DDR_BURST_SIZE_BYTES;

        ddrLoad->total_read += read_bytes;
        ddrLoad->total_write += write_bytes;

        ddrLoad->ddr_stats.read_bw_avg = (ddrLoad->total_read/ddrLoad->total_time); /* in MB/s */
        ddrLoad->ddr_stats.write_bw_avg = (ddrLoad->total_write/ddrLoad->total_time); /* in MB/s */

        uint32_t read_bw_peak = read_bytes/elapsed_time; /* in MB/s */
        uint32_t write_bw_peak = write_bytes/elapsed_time; /* in MB/s */
        if(read_bw_peak > ddrLoad->ddr_stats.read_bw_peak)
            ddrLoad->ddr_stats.read_bw_peak = read_bw_peak;
        if(write_bw_peak > ddrLoad->ddr_stats.write_bw_peak)
            ddrLoad->ddr_stats.write_bw_peak = write_bw_peak;

#ifdef APP_PERF_SHOW_DDR_STATS
        ddrLoad->ddr_stats.counter0_total += val2;
        ddrLoad->ddr_stats.counter1_total += val3;

        ddrLoad->snapshot_count -= elapsed_time;

        if(ddrLoad->snapshot_count <= 0)
        {
            appLogPrintf("ACTIVE_CMD = %d, QUEUE_FULL = %d  \n", ddrLoad->ddr_stats.counter0_total,
                                                                 ddrLoad->ddr_stats.counter1_total);

            ddrLoad->ddr_stats.counter0_total = 0;
            ddrLoad->ddr_stats.counter1_total = 0;

            ddrLoad->snapshot_count = APP_PERF_SNAPSHOT_WINDOW_WIDTH;
        }

#endif
    }

    ddrLoad->last_timestamp = cur_time;

    HwiP_restore(cookie);
}

void appPerfStatsResetDdrLoadCalcAll()
{
    app_perf_stats_obj_t *obj = &g_app_perf_stats_obj;
    app_perf_stats_ddr_load_t *ddrLoad;
    uint32_t cookie;

    appPerfStatsLock(obj);

    ddrLoad = &obj->ddrLoad;

    cookie = HwiP_disable();

    ddrLoad->ddr_stats.read_bw_avg = 0;
    ddrLoad->ddr_stats.write_bw_avg = 0;
    ddrLoad->ddr_stats.read_bw_peak = 0;
    ddrLoad->ddr_stats.write_bw_peak = 0;
    ddrLoad->ddr_stats.total_available_bw = APP_PERF_DDR_MHZ*APP_PERF_DDR_BUS_WIDTH*APP_PERF_NUM_DDR_INSTANCES*2/8;
    ddrLoad->total_time = 0;
    ddrLoad->total_read = 0;
    ddrLoad->total_write = 0;
    ddrLoad->last_timestamp = appLogGetTimeInUsec();
    ddrLoad->snapshot_count = APP_PERF_SNAPSHOT_WINDOW_WIDTH;

    ddrLoad->ddr_stats.counter0_total = 0;
    ddrLoad->ddr_stats.counter1_total = 0;
    ddrLoad->ddr_stats.counter2_total = 0;
    ddrLoad->ddr_stats.counter3_total = 0;

    HwiP_restore(cookie);

    appPerfStatsUnLock(obj);
}

#if defined(R5F) || defined(M55)
/* This function is called when configUSE_IDLE_HOOK is 1 in FreeRTOSConfig.h */
void vApplicationIdleHook( void )
{
    #if (configLOAD_UPDATE_IN_IDLE==1)
        void vApplicationLoadHook();

        vApplicationLoadHook();
    #endif

    if(1U == g_perf_stats_load_update_enable)
    {
        uint32_t cpu_id = appIpcGetSelfCpuId();

        if (cpu_id == APP_PERF_STATS_GET_DDR_STATS_CORE)
        {
            /* not taken in inside lock since interrupts are disabled for locking
             * interrupts are disable since counters are read in ISR context
             */
            appPerfStatsDddrStatsUpdate();
        }
    }
    asm ( " WFI " );
}
#endif
