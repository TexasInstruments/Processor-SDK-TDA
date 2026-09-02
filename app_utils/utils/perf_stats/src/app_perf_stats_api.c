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
#include <inttypes.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/mem/include/app_mem.h>
#include <utils/ipc/include/app_ipc.h>
#include "app_perf_stats_priv.h"

#define APP_PERF_EXPORT_WRITELN(fp, message, ...) do { \
    snprintf(line, APP_PERF_POINT_MAX_FILENAME, message"\n", ##__VA_ARGS__); \
    fwrite(line, 1, strlen(line), fp); \
    } while (0)

int32_t appPerfStatsCpuLoadReset(uint32_t app_cpu_id)
{
    int32_t status;

    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_RESET_LOAD_CALC,
        NULL, 0,
        APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK);

    return status;
}

int32_t appPerfStatsCpuLoadGet(uint32_t app_cpu_id, app_perf_stats_cpu_load_t *cpu_load)
{
    int32_t status = 0;

    #ifndef VDK
    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_CPU_LOAD,
        cpu_load, sizeof(app_perf_stats_cpu_load_t),
        0);
    #endif

    return status;
}

int32_t appPerfStatsCpuTaskStatsGet(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats)
{
    int32_t status = 0;

    #ifndef VDK
    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_CPU_TASK_STATS,
        cpu_stats, sizeof(app_perf_stats_task_stats_t),
        0);
    #endif

    return status;
}

int32_t appPerfStatsCpuTaskStacksStatsGet(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats)
{
    int32_t status;

    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_CPU_TASK_STACKS_STATS,
        cpu_stats, sizeof(app_perf_stats_task_stats_t),
        0);

    return status;
}

int32_t appPerfStatsCpuMemStatsGet(uint32_t app_cpu_id, app_perf_stats_mem_stats_t *cpu_stats)
{
    int32_t status = 0;

    #ifndef VDK
    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_CPU_MEM_STATS,
        cpu_stats, sizeof(app_perf_stats_mem_stats_t),
        0);
    #endif

    return status;
}

int32_t appPerfStatsCpuOsStatsGet(uint32_t app_cpu_id, app_perf_stats_os_stats_t *os_stats)
{
    int32_t status;

    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_OS_STATS,
        os_stats, sizeof(app_perf_stats_os_stats_t),
        0);

    return status;
}

int32_t appPerfStatsCpuTiovxStatsGet(uint32_t app_cpu_id, app_perf_stats_tiovx_stats_t *tiovx_stats)
{
    int32_t status;

    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_TIOVX_STATS,
        tiovx_stats, (uint32_t)sizeof(app_perf_stats_tiovx_stats_t),
        0);

    return status;
}

int32_t appPerfStatsCpuLoadPrintAll()
{
    int32_t status=0;

    #ifndef VDK
    uint32_t cpu_id;
    app_perf_stats_cpu_load_t cpu_load;

    printf("\n");
    printf("Summary of CPU load,\n");
    printf("====================\n");
    printf("\n");
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            status = appPerfStatsCpuLoadGet(cpu_id, &cpu_load);
            if(status==0)
            {
                appPerfStatsCpuLoadPrint(cpu_id, &cpu_load);
            }
        }
    }
    printf("\n");
    #endif

    return status;
}

#ifndef VDK
static void appPerfStatsCpuLoadExport(FILE *fp)
{
    char line[APP_PERF_MAX_LINE_SIZE];
    int32_t status=0;
    app_perf_stats_cpu_load_t cpu_load;
    uint32_t cpu_id;

    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            status = appPerfStatsCpuLoadGet(cpu_id, &cpu_load);
            if(status==0)
            {
                APP_PERF_EXPORT_WRITELN(fp, "%6s    | %3d.%2d ",
                            appIpcGetCpuName(cpu_id),
                            cpu_load.cpu_load/100u,
                            cpu_load.cpu_load%100u
                        );
            }
        }
    }
}
#endif


#ifndef VDK
static void appPerfStatsHwaStatsExport(FILE *fp)
{
    app_perf_stats_hwa_stats_t hwa_load;
    app_perf_hwa_id_t hwa_id;
    app_perf_stats_hwa_load_t *hwaLoad;
    uint64_t load;
    char line[APP_PERF_MAX_LINE_SIZE];
    int32_t status=0;

    #if defined(SOC_FAMILY_AM)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU1_0, &hwa_load);
    if(status==0)
    {
        APP_PERF_EXPORT_WRITELN(fp, "HWA      | LOAD");
        APP_PERF_EXPORT_WRITELN(fp, "----------|--------------");
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            hwaLoad = &hwa_load.hwa_stats[hwa_id];

            if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                load = (hwaLoad->active_time*10000)/hwaLoad->total_time;
                APP_PERF_EXPORT_WRITELN(fp, "%6s    | %3"PRIu64".%2"PRIu64" %% ( %"PRIu64" MP/s )",
                        appPerfStatsGetHwaName(hwa_id),
                        load/100,
                        load%100,
                        (hwaLoad->pixels_processed/hwaLoad->total_time)
                    );
            }
        }
    }
    #elif defined(SOC_FAMILY_J7)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU2_0, &hwa_load);
    if(status==0)
    {
        APP_PERF_EXPORT_WRITELN(fp, "HWA      | LOAD");
        APP_PERF_EXPORT_WRITELN(fp, "----------|--------------");
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            hwaLoad = &hwa_load.hwa_stats[hwa_id];

            if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                load = (hwaLoad->active_time*10000)/hwaLoad->total_time;
                APP_PERF_EXPORT_WRITELN(fp, "%6s    | %3"PRIu64".%2"PRIu64" %% ( %"PRIu64" MP/s )",
                        appPerfStatsGetHwaName(hwa_id),
                        load/100,
                        load%100,
                        (hwaLoad->pixels_processed/hwaLoad->total_time)
                    );
            }
        }
    }
    #if !defined(SOC_J722S)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU2_1, &hwa_load);
    if(status==0)
    {
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            hwaLoad = &hwa_load.hwa_stats[hwa_id];

            if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                load = (hwaLoad->active_time*10000)/hwaLoad->total_time;
                APP_PERF_EXPORT_WRITELN(fp, "%6s    | %3"PRIu64".%2"PRIu64" %% ( %"PRIu64" MP/s )",
                        appPerfStatsGetHwaName(hwa_id),
                        load/100,
                        load%100,
                        (hwaLoad->pixels_processed/hwaLoad->total_time)
                    );
            }
        }
    }
    #endif
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU4_0, &hwa_load);
    if(status==0)
    {
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            hwaLoad = &hwa_load.hwa_stats[hwa_id];

            if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                load = (hwaLoad->active_time*10000)/hwaLoad->total_time;
                APP_PERF_EXPORT_WRITELN(fp, "%6s    | %3"PRIu64".%2"PRIu64" %% ( %"PRIu64" MP/s )",
                        appPerfStatsGetHwaName(hwa_id),
                        load/100,
                        load%100,
                        (hwaLoad->pixels_processed/hwaLoad->total_time)
                    );
            }
        }
    }
    #endif
    #endif
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MPU1_0, &hwa_load);
    if(status==0)
    {
        for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
        {
            hwaLoad = &hwa_load.hwa_stats[hwa_id];

            if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
            {
                load = (hwaLoad->active_time*10000)/hwaLoad->total_time;
                APP_PERF_EXPORT_WRITELN(fp, "%6s    | %3"PRIu64".%2"PRIu64" %% ( %"PRIu64" MP/s )",
                        appPerfStatsGetHwaName(hwa_id),
                        load/100,
                        load%100,
                        (hwaLoad->pixels_processed/hwaLoad->total_time)
                    );
            }
        }
    }
}
#endif

#ifndef VDK
static void appPerfStatsDdrStatsExport(FILE *fp)
{
    char line[APP_PERF_MAX_LINE_SIZE];
    app_perf_stats_ddr_stats_t ddr_stats;
    int32_t status=0;

    status = appPerfStatsDdrStatsGet(&ddr_stats);
    if(status==0)
    {
        APP_PERF_EXPORT_WRITELN(fp, "DDR BW   | AVG          | PEAK");
        APP_PERF_EXPORT_WRITELN(fp, "----------|--------------|-------");
        APP_PERF_EXPORT_WRITELN(fp, "READ BW | %6d MB/s  | %6d MB/s",
            ddr_stats.read_bw_avg,
            ddr_stats.read_bw_peak);
        APP_PERF_EXPORT_WRITELN(fp, "WRITE BW | %6d MB/s  | %6d MB/s",
            ddr_stats.write_bw_avg,
            ddr_stats.write_bw_peak);
        APP_PERF_EXPORT_WRITELN(fp, "TOTAL BW | %6d MB/s  | %6d MB/s",
            ddr_stats.read_bw_avg+ddr_stats.write_bw_avg,
            ddr_stats.write_bw_peak+ddr_stats.read_bw_peak);
    }
}
#endif

#ifndef VDK
static void appPerfStatsCpuStatsExportMemTable(FILE *fp, uint32_t app_cpu_id, app_perf_stats_mem_stats_t *cpu_stats)
{
    uint32_t i;
    char line[APP_PERF_MAX_LINE_SIZE];

    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }
    APP_PERF_EXPORT_WRITELN(fp, "\n###CPU Heap Table");
    APP_PERF_EXPORT_WRITELN(fp, "\nHEAP   | Size  | Free | Unused");
    APP_PERF_EXPORT_WRITELN(fp, "--------|-------|------|---------");
    for(i=0; i<APP_MEM_HEAP_MAX; i++)
    {
        if(strcmp(cpu_stats->mem_stats[i].heap_name, "INVALID") != 0 )
        {
            uint32_t free_size_in_percentage = 0;

            if(cpu_stats->mem_stats[i].heap_size > 0
                && cpu_stats->mem_stats[i].heap_size >= cpu_stats->mem_stats[i].free_size
                )
            {
                free_size_in_percentage = (uint32_t)(((float)cpu_stats->mem_stats[i].free_size/(float)cpu_stats->mem_stats[i].heap_size)*100);
            }

            APP_PERF_EXPORT_WRITELN(fp, "%16s | %10d B | %10d B | %3d %%",
                cpu_stats->mem_stats[i].heap_name,
                cpu_stats->mem_stats[i].heap_size,
                cpu_stats->mem_stats[i].free_size,
                free_size_in_percentage
                );
        }
    }
    return;
}
#endif

#ifndef VDK
static void appPerfStatsCpuStatsExportTaskTable(FILE *fp, uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats)
{
    uint32_t i;
    char line[APP_PERF_MAX_LINE_SIZE];

    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }
    APP_PERF_EXPORT_WRITELN(fp, "\n##CPU: %s", appIpcGetCpuName(app_cpu_id));
    APP_PERF_EXPORT_WRITELN(fp, "\n###Task Table");
    APP_PERF_EXPORT_WRITELN(fp, "\nTASK          | TASK LOAD");
    APP_PERF_EXPORT_WRITELN(fp, "--------------|-------");
    for(i=0; i<cpu_stats->num_tasks; i++)
    {
        APP_PERF_EXPORT_WRITELN(fp, "%16s   | %3d.%2d %%",
            cpu_stats->task_stats[i].task_name,
            cpu_stats->task_stats[i].task_load/100,
            cpu_stats->task_stats[i].task_load%100
            );
    }
    return;
}
#endif

#ifndef VDK
static void appPerfStatsCpuStatsExport(FILE *fp)
{
    uint32_t cpu_id;
    int32_t status=0;
    app_perf_stats_task_stats_t cpu_task_stats;
    app_perf_stats_mem_stats_t cpu_mem_stats;

    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            #if defined(LINUX) || defined(QNX)
            /* NOT supported for Linux/QNX A72 as of now */
            if(cpu_id!=appIpcGetSelfCpuId())
            #endif
            {
                cpu_task_stats.starting_task = 0;
                status = appPerfStatsCpuTaskStatsGet(cpu_id, &cpu_task_stats);
                if(status==0)
                {
                    appPerfStatsCpuStatsExportTaskTable(fp, cpu_id, &cpu_task_stats);
                    if(cpu_task_stats.num_tasks == APP_PERF_STATS_TASK_MAX)
                    {
                        cpu_task_stats.starting_task = APP_PERF_STATS_TASK_MAX;
                        status = appPerfStatsCpuTaskStatsGet(cpu_id, &cpu_task_stats);
                        if(status==0)
                        {
                            appPerfStatsCpuStatsExportTaskTable(fp, cpu_id, &cpu_task_stats);
                        }
                    }
                    status = appPerfStatsCpuMemStatsGet(cpu_id, &cpu_mem_stats);
                    if(status==0)
                    {
                        appPerfStatsCpuStatsExportMemTable(fp, cpu_id, &cpu_mem_stats);
                    }
                }
            }
        }
    }
}
#endif

int32_t appPerfStatsCpuStatsPrintAll()
{
    int32_t status=0;

    #ifndef VDK
    uint32_t cpu_id;
    app_perf_stats_task_stats_t cpu_task_stats;
    app_perf_stats_mem_stats_t cpu_mem_stats;

    printf("\n");
    printf("Detailed CPU performance/memory statistics,\n");
    printf("===========================================\n");
    printf("\n");
    #ifdef LINUX
    appMemPrintMemAllocInfo();
    printf("\n");
    #endif
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            #if defined(LINUX) || defined(QNX)
            /* NOT supported for Linux/QNX A72 as of now */
            if(cpu_id!=appIpcGetSelfCpuId())
            #endif
            {
                cpu_task_stats.starting_task = 0;
                status = appPerfStatsCpuTaskStatsGet(cpu_id, &cpu_task_stats);
                if(status==0)
                {
                    appPerfStatsCpuTaskStatsPrint(cpu_id, &cpu_task_stats);
                    if(cpu_task_stats.num_tasks == APP_PERF_STATS_TASK_MAX)
                    {
                        cpu_task_stats.starting_task = APP_PERF_STATS_TASK_MAX;
                        status = appPerfStatsCpuTaskStatsGet(cpu_id, &cpu_task_stats);
                        if(status==0)
                        {
                            appPerfStatsCpuTaskStatsPrint(cpu_id, &cpu_task_stats);
                        }
                    }
                    printf("\n");
                    status = appPerfStatsCpuMemStatsGet(cpu_id, &cpu_mem_stats);
                    if(status==0)
                    {
                        appPerfStatsCpuMemStatsPrint(cpu_id, &cpu_mem_stats);
                    }
                }
            }
        }
    }
    printf("\n");
    #endif

    return status;
}

int32_t appPerfStatsCpuLoadResetAll()
{
    int32_t status=0;

    #ifndef VDK
    uint32_t cpu_id;

    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            status = appPerfStatsCpuLoadReset(cpu_id);
        }
    }
    #endif

    return status;
}

int32_t appPerfStatsPrintAll()
{
    appPerfStatsCpuLoadPrintAll();
    appPerfStatsHwaLoadPrintAll();
    appPerfStatsDdrStatsPrintAll();
    appPerfStatsCpuStatsPrintAll();
    appPerfStatsResetAll();
    return 0;
}

int32_t appPerfStatsCpuTaskStatsPrint(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats)
{
    int32_t status = 0;

    #ifndef VDK
    uint32_t i;

    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }
    for(i=0; i<cpu_stats->num_tasks; i++)
    {
        printf("CPU: %6s: TASK: %16s: %3d.%2d %%\n",
            appIpcGetCpuName(app_cpu_id),
            cpu_stats->task_stats[i].task_name,
            cpu_stats->task_stats[i].task_load/100,
            cpu_stats->task_stats[i].task_load%100
            );
    }
    #endif

    return status;
}

int32_t appPerfStatsCpuTaskStacksStatsPrint(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats)
{
    int32_t status = 0;
    uint32_t i;

    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }
    if(cpu_stats->starting_task == 0)
    {
        printf("\n");
    }
    for(i=0; i<cpu_stats->num_tasks; i++)
    {
        printf("CPU: %6s: TASK: %16s: %6d\n",
            appIpcGetCpuName(app_cpu_id),
            cpu_stats->task_stats[i].task_name,
            cpu_stats->task_stats[i].task_load
            );
    }
    return status;
}

int32_t appPerfStatsCpuMemStatsPrint(uint32_t app_cpu_id, app_perf_stats_mem_stats_t *cpu_stats)
{
    int32_t status = 0;

    #ifndef VDK
    uint32_t i;

    if(cpu_stats->num_tasks>APP_PERF_STATS_TASK_MAX)
    {
        cpu_stats->num_tasks = APP_PERF_STATS_TASK_MAX;
    }
    for(i=0; i<APP_MEM_HEAP_MAX; i++)
    {
        if(strcmp(cpu_stats->mem_stats[i].heap_name, "INVALID") != 0 )
        {
            uint32_t free_size_in_percentage = 0;

            if(cpu_stats->mem_stats[i].heap_size > 0
                && cpu_stats->mem_stats[i].heap_size >= cpu_stats->mem_stats[i].free_size
                )
            {
                free_size_in_percentage = (uint32_t)(((float)cpu_stats->mem_stats[i].free_size/(float)cpu_stats->mem_stats[i].heap_size)*100);
            }

            printf("CPU: %6s: HEAP: %16s: size = %10d B, free = %10d B (%3d %% unused)\n",
                appIpcGetCpuName(app_cpu_id),
                cpu_stats->mem_stats[i].heap_name,
                cpu_stats->mem_stats[i].heap_size,
                cpu_stats->mem_stats[i].free_size,
                free_size_in_percentage
                );
        }
    }
    printf("\n");
    #endif

    return status;
}

int32_t appPerfStatsCpuOsStatsPrint(uint32_t app_cpu_id, app_perf_stats_os_stats_t *os_stats, uint32_t showPeak)
{
    int32_t status = 0;

    printf("CPU: %6s\n", appIpcGetCpuName(app_cpu_id));

    if (showPeak != 0)
    {
        printf("  semaphore:  cnt= %6d: peak: %6d\n", os_stats->semaphore_count, os_stats->semaphore_peak);
        printf("  mutex:      cnt= %6d: peak: %6d\n", os_stats->mutex_count, os_stats->mutex_peak);
        printf("  queue:      cnt= %6d: peak: %6d\n", os_stats->queue_count, os_stats->queue_peak);
        printf("  event:      cnt= %6d: peak: %6d\n", os_stats->event_count, os_stats->event_peak);
        printf("  heap:       cnt= %6d: peak: %6d\n", os_stats->heap_count, os_stats->heap_peak);
        printf("  mailbox:    cnt= %6d: peak: %6d\n", os_stats->mailbox_count, os_stats->mailbox_peak);
        printf("  task:       cnt= %6d: peak: %6d\n", os_stats->task_count, os_stats->task_peak);
        printf("  clock:      cnt= %6d: peak: %6d\n", os_stats->clock_count, os_stats->clock_peak);
        printf("  hwi:        cnt= %6d: peak: %6d\n", os_stats->hwi_count, os_stats->hwi_peak);
        printf("  timer:      cnt= %6d: peak: %6d\n", os_stats->timer_count, os_stats->timer_peak);
    }
    else
    {
        printf("  semaphore:  cnt= %6d\n", os_stats->semaphore_count);
        printf("  mutex:      cnt= %6d\n", os_stats->mutex_count);
        printf("  queue:      cnt= %6d\n", os_stats->queue_count);
        printf("  event:      cnt= %6d\n", os_stats->event_count);
        printf("  heap:       cnt= %6d\n", os_stats->heap_count);
        printf("  mailbox:    cnt= %6d\n", os_stats->mailbox_count);
        printf("  task:       cnt= %6d\n", os_stats->task_count);
        printf("  clock:      cnt= %6d\n", os_stats->clock_count);
        printf("  hwi:        cnt= %6d\n", os_stats->hwi_count);
        printf("  timer:      cnt= %6d\n", os_stats->timer_count);
    }

    printf("\n");
    return status;
}

int32_t appPerfStatsCpuTiovxStatsPrint(uint32_t app_cpu_id, app_perf_stats_tiovx_stats_t *tiovx_stats)
{
    int32_t status = 0;

    printf("CPU: %6s\n", appIpcGetCpuName(app_cpu_id));

    printf("\nTIOVX Target Information\n\n");
    printf("  targets_in_cpu:              cnt= %6d\n", tiovx_stats->targets_in_cpu);
    printf("  max_targets_per_kernel:      cnt= %6d\n", tiovx_stats->max_targets_per_kernel);
    printf("  target_kernel_instance_max:  cnt= %6d\n", tiovx_stats->target_kernel_instance_max);
    printf("  target_kernel_max:           cnt= %6d\n", tiovx_stats->target_kernel_max);

    printf("\n");
    return status;
}


int32_t appPerfStatsCpuLoadPrint(uint32_t app_cpu_id, app_perf_stats_cpu_load_t *cpu_load)
{
    int32_t status = 0;

    #ifndef VDK
    printf("CPU: %6s: TOTAL LOAD = %3d.%2d %% ( HWI = %3d.%2d %%, SWI = %3d.%2d %% )\n",
                appIpcGetCpuName(app_cpu_id),
                cpu_load->cpu_load/100u,
                cpu_load->cpu_load%100u,
                cpu_load->hwi_load/100u,
                cpu_load->hwi_load%100u,
                cpu_load->swi_load/100u,
                cpu_load->swi_load%100u
            );
    #endif

    return status;
}

#if defined(LINUX) || defined(QNX)
int32_t appPerfStatsRegisterTask(void *task_handle, const char *name)
{
    #ifndef VDK
    /* NOT supported for LINUX */
    return -1;
    #else
    return 0;
    #endif
}
#endif

void appPerfPointSetName(app_perf_point_t *prm, const char *name)
{
    #ifndef VDK
    strncpy(prm->name, name, APP_PERF_POINT_NAME_MAX -1U);
    prm->name[APP_PERF_POINT_NAME_MAX-1] = 0;

    appPerfPointReset(prm);
    #endif
}

void appPerfPointReset(app_perf_point_t *prm)
{
    #ifndef VDK
    prm->sum = 0;
    prm->avg = 0;
    prm->min = 0xFFFFFFFFu;
    prm->max = 0;
    prm->num = 0;
    #endif
}

/**
 * \brief Start a performance point
 *
 * \param prm [out] performance point
 */
void appPerfPointBegin(app_perf_point_t *prm)
{
    #ifndef VDK
    prm->tmp = appLogGetTimeInUsec();
    #endif
}


void appPerfPointEnd(app_perf_point_t *prm)
{
    #ifndef VDK
    uint64_t tmp, num;

    tmp  = appLogGetTimeInUsec() - prm->tmp;
    if(tmp > prm->max)
    {
        prm->max = tmp;
    }
    if(tmp < prm->min)
    {
        prm->min = tmp;
    }
    prm->sum += tmp;

    num = prm->num;
    num++;

    prm->avg  = prm->sum/num;

    prm->num = num;
    #endif
}

void appPerfPointGet(app_perf_point_t *prm, app_perf_point_t *perf)
{
    #ifndef VDK
    *perf = *prm;
    perf->name[APP_PERF_POINT_NAME_MAX-1] = 0;
    #endif
}

void appPerfPointPrint(app_perf_point_t *prm)
{
    #ifndef VDK
    app_perf_point_t perf;

    appPerfPointGet(prm, &perf);

    printf(" PERF: %16s: avg = %6"PRIu64" usecs, min/max = %6"PRIu64" / %6"PRIu64" usecs, #executions = %10"PRIu64"\n",
        perf.name,
        (perf.avg),
        (perf.min),
        (perf.max),
        (perf.num)
        );
    #endif
}

void appPerfPointPrintFPS(app_perf_point_t *prm)
{
    #ifndef VDK
    uint32_t fps;
    app_perf_point_t perf;

    appPerfPointGet(prm, &perf);

    if(perf.avg>0)
    {
        fps = (uint32_t)((1000*1000*100ull)/perf.avg);
    }
    else
    {
        fps = 0;
    }

    printf(" PERF: %16s: %4d.%2d FPS\n",
        perf.name,
        fps/100,
        fps%100
        );
    #endif
}

void appPerfPointExport(FILE *fp, app_perf_point_t *prm[], uint32_t num_points)
{
    app_perf_point_t perf;
    char line[APP_PERF_MAX_LINE_SIZE];
    uint32_t fps, i;

    for (i = 0; i < num_points; i++)
    {
        if (NULL != prm[i])
        {
            appPerfPointGet(prm[i], &perf);
            APP_PERF_EXPORT_WRITELN(fp, "\n##%s Performance\n", prm[i]->name);
            APP_PERF_EXPORT_WRITELN(fp, "PERF      | avg (usecs)  | min/max (usecs)  | number of executions");
            APP_PERF_EXPORT_WRITELN(fp, "----------|----------|----------|----------");
            APP_PERF_EXPORT_WRITELN(fp, "%16s | %6"PRIu64" | %6"PRIu64" / %6"PRIu64" | %10"PRIu64"",
                perf.name,
                (perf.avg),
                (perf.min),
                (perf.max),
                (perf.num)
                );

            if(perf.avg>0)
            {
                fps = (uint32_t)((1000*1000*100ull)/perf.avg);
            }
            else
            {
                fps = 0;
            }

            APP_PERF_EXPORT_WRITELN(fp, "\n##%s FPS\n", prm[i]->name);
            APP_PERF_EXPORT_WRITELN(fp, "PERF      | Frames per sec (FPS)");
            APP_PERF_EXPORT_WRITELN(fp, "----------|----------");
            APP_PERF_EXPORT_WRITELN(fp, "%16s | %4d.%2d\n",
                perf.name,
                fps/100,
                fps%100
                );
        }
    }

}

FILE *appPerfStatsExportOpenFile(const char *output_file_path, const char *output_file_prefix)
{
    FILE *fp = NULL;

    #ifndef VDK
    if ( (output_file_path!=NULL)
        && (output_file_prefix!=NULL) )
    {
        char filename[APP_PERF_POINT_MAX_FILENAME];
        char line[APP_PERF_MAX_LINE_SIZE];

        snprintf(filename, APP_PERF_POINT_MAX_FILENAME,
            "%s/%s.md",
            output_file_path,
            output_file_prefix);

        fp = fopen(filename, "wb");
        if (NULL != fp)
        {
            APP_PERF_EXPORT_WRITELN(fp, "# Datasheet {#group_apps_%s_datasheet}\n", output_file_prefix);
        }
    }
    else
    {
        printf("appPerfStatsExportOpenFile error: Invalid arguments!\n");
    }
    #endif

    return fp;
}

int32_t appPerfStatsExportCloseFile(FILE *fp)
{
    int32_t retVal = 0;

    #ifndef VDK
    if(fp!=NULL)
    {
        fclose(fp);
    }
    else
    {
        retVal = -1;
        printf("appPerfStatsExportCloseFile error: File handle was NULL!\n");
    }
    #endif

    return retVal;
}

int32_t appPerfStatsExportAll(FILE *fp, app_perf_point_t *perf_points[], uint32_t num_points)
{
    int32_t retVal = 0;

    #ifndef VDK
    if(fp!=NULL)
    {
        char line[APP_PERF_MAX_LINE_SIZE];
        APP_PERF_EXPORT_WRITELN(fp, "# Summary of CPU load\n");
        APP_PERF_EXPORT_WRITELN(fp, "CPU      | TOTAL LOAD");
        APP_PERF_EXPORT_WRITELN(fp, "----------|--------------");
        appPerfStatsCpuLoadExport(fp);
        APP_PERF_EXPORT_WRITELN(fp, "\n# HWA performance statistics\n");
        appPerfStatsHwaStatsExport(fp);
        APP_PERF_EXPORT_WRITELN(fp, "\n# DDR performance statistics\n");
        appPerfStatsDdrStatsExport(fp);
        APP_PERF_EXPORT_WRITELN(fp, "\n# Detailed CPU performance/memory statistics\n");
        appPerfStatsCpuStatsExport(fp);
        APP_PERF_EXPORT_WRITELN(fp, "\n# Performance point statistics\n");
        appPerfPointExport(fp, perf_points, num_points);
    }
    else
    {
        retVal = -1;
        printf("appPerfStatsExportAll error: File handle was NULL!\n");
    }
    #endif

    return retVal;
}

char *appPerfStatsGetHwaName(app_perf_hwa_id_t hwa_id)
{
    static char *hwa_name[] = {
        " VISS",
        " LDC ",
        " BLNF",
        " MSC0",
        " MSC1",
        " DOF ",
        " SDE ",
        " GPU ",
#if defined(SOC_J784S4) || defined(SOC_J742S2) || defined (SOC_TDA54)
        " VISS_1",
        " LDC_1",
        " BLNF_1",
        " MSC0_1",
        " MSC1_1",
#endif
#if defined(SOC_TDA54)
        " VISS_2",
        " LDC_2",
        " BLNF_2",
        " MSC0_2",
        " MSC1_2",
#endif
        "INVAL"};
    char *name;

    if(hwa_id < APP_PERF_HWA_MAX)
    {
        name = hwa_name[hwa_id];
    }
    else
    {
        name = hwa_name[APP_PERF_HWA_MAX];
    }
    return name;
}

int32_t appPerfStatsHwaStatsGet(uint32_t app_cpu_id, app_perf_stats_hwa_stats_t *hwa_stats)
{
    int32_t status;

    status = appRemoteServiceRun(app_cpu_id, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_HWA_LOAD,
        hwa_stats, sizeof(app_perf_stats_hwa_stats_t),
        0);

    return status;
}

int32_t appPerfStatsHwaLoadPrint(app_perf_stats_hwa_stats_t *hwa_load)
{
    int32_t status = 0;
    app_perf_hwa_id_t hwa_id;
    app_perf_stats_hwa_load_t *hwaLoad;
    uint64_t load;

    for(hwa_id=(app_perf_hwa_id_t)0; hwa_id<APP_PERF_HWA_MAX; hwa_id++)
    {
        hwaLoad = &hwa_load->hwa_stats[hwa_id];

        if(hwaLoad->active_time > 0 && hwaLoad->pixels_processed > 0 && hwaLoad->total_time > 0)
        {
            load = (hwaLoad->active_time*10000)/hwaLoad->total_time;
            printf("HWA: %6s: LOAD = %3"PRIu64".%2"PRIu64" %% ( %"PRIu64" MP/s )\n",
                    appPerfStatsGetHwaName(hwa_id),
                    load/100,
                    load%100,
                    (hwaLoad->pixels_processed/hwaLoad->total_time)
                );
        }
    }

    return status;
}

int32_t appPerfStatsHwaLoadPrintAll()
{
    int32_t status = 0;

    #ifndef VDK
    app_perf_stats_hwa_stats_t hwa_load;

    printf("\n");
    printf("HWA performance statistics,\n");
    printf("===========================\n");
    printf("\n");

    #if defined(SOC_FAMILY_AM)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU1_0, &hwa_load);
    if(status==0)
    {
        appPerfStatsHwaLoadPrint(&hwa_load);
    }
    #elif defined(SOC_FAMILY_J7)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU2_0, &hwa_load);
    if(status==0)
    {
        appPerfStatsHwaLoadPrint(&hwa_load);
    }
    #if !defined(SOC_J722S)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU2_1, &hwa_load);
    if(status==0)
    {
        appPerfStatsHwaLoadPrint(&hwa_load);
    }
    #endif
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MCU4_0, &hwa_load);
    if(status==0)
    {
        appPerfStatsHwaLoadPrint(&hwa_load);
    }
    #endif
    #endif
    status = appPerfStatsHwaStatsGet(APP_IPC_CPU_MPU1_0, &hwa_load);
    if(status==0)
    {
        appPerfStatsHwaLoadPrint(&hwa_load);
    }
    printf("\n");
    #endif

    return status;
}

int32_t appPerfStatsHwaLoadResetAll()
{
    int32_t status = 0;

    #ifndef VDK
    #if defined(SOC_FAMILY_AM)
    status = appRemoteServiceRun(APP_IPC_CPU_MCU1_0, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC,
        NULL, 0,
        0);
    #elif defined(SOC_FAMILY_J7)
    status = appRemoteServiceRun(APP_IPC_CPU_MCU2_0, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC,
        NULL, 0,
        0);
    #if !defined(SOC_J722S)
    status = appRemoteServiceRun(APP_IPC_CPU_MCU2_1, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC,
        NULL, 0,
        0);
    #endif
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    status = appRemoteServiceRun(APP_IPC_CPU_MCU4_0, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC,
        NULL, 0,
        0);
    #endif
    #endif
    status = appRemoteServiceRun(APP_IPC_CPU_MPU1_0, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_RESET_HWA_LOAD_CALC,
        NULL, 0,
        0);
    #endif

    return status;
}

int32_t appPerfStatsDdrStatsGet(app_perf_stats_ddr_stats_t *ddr_stats)
{
    int32_t status;

    status = appRemoteServiceRun(APP_PERF_STATS_GET_DDR_STATS_CORE, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_GET_DDR_STATS,
        ddr_stats, sizeof(app_perf_stats_ddr_stats_t),
        0);

    return status;
}

int32_t appPerfStatsDdrStatsPrint(app_perf_stats_ddr_stats_t *ddr_load)
{
    int32_t status = 0;

    printf("DDR: READ  BW: AVG = %6d MB/s, PEAK = %6d MB/s\n",
        ddr_load->read_bw_avg,
        ddr_load->read_bw_peak);
    printf("DDR: WRITE BW: AVG = %6d MB/s, PEAK = %6d MB/s\n",
        ddr_load->write_bw_avg,
        ddr_load->write_bw_peak);
    printf("DDR: TOTAL BW: AVG = %6d MB/s, PEAK = %6d MB/s\n",
        ddr_load->read_bw_avg+ddr_load->write_bw_avg,
        ddr_load->write_bw_peak+ddr_load->read_bw_peak);

    return status;
}

int32_t appPerfStatsDdrStatsPrintAll()
{
    int32_t status = 0;

    #ifndef VDK
    app_perf_stats_ddr_stats_t ddr_stats;

    printf("\n");
    printf("DDR performance statistics,\n");
    printf("===========================\n");
    printf("\n");
    status = appPerfStatsDdrStatsGet(&ddr_stats);
    if(status==0)
    {
        appPerfStatsDdrStatsPrint(&ddr_stats);
    }
    printf("\n");
    #endif

    return status;
}

int32_t appPerfStatsDdrStatsResetAll()
{
    int32_t status = 0;

    #ifndef VDK
    status = appRemoteServiceRun(APP_PERF_STATS_GET_DDR_STATS_CORE, APP_PERF_STATS_SERVICE_NAME,
        APP_PERF_STATS_CMD_RESET_DDR_STATS,
        NULL, 0,
        0);
    #endif

    return status;
}

void appPerfStatsResetAll()
{
    appPerfStatsCpuLoadResetAll();
    appPerfStatsHwaLoadResetAll();
    appPerfStatsDdrStatsResetAll();
}
