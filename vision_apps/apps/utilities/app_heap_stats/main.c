/*
 *
 * Copyright (c) 2020 Texas Instruments Incorporated
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
#include <assert.h>
#include <stdint.h>
#include <VX/vx.h>
#include <app_init.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>

int32_t appPerfStatsMemStatsPrintAll()
{
    uint32_t cpu_id;
    int32_t status=0;
    app_perf_stats_mem_stats_t cpu_mem_stats;

    printf("\n");
    printf("Detailed CPU memory statistics,\n");
    printf("===============================\n");
    printf("\n");
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            #if defined(LINUX) || defined(QNX)
            /* NOT supported for Linux/QNX A72 as of now */
            if(cpu_id!=appIpcGetSelfCpuId())
            #endif
            {
                status = appPerfStatsCpuMemStatsGet(cpu_id, &cpu_mem_stats);
                if(status==0)
                {
                    appPerfStatsCpuMemStatsPrint(cpu_id, &cpu_mem_stats);
                }
            }
        }
    }
    printf("\n");
    return status;
}

int32_t appPerfStatsOsStatsPrintAll()
{
    uint32_t cpu_id;
    int32_t status=0;
    app_perf_stats_os_stats_t cpu_os_stats;

    printf("\n");
    printf("Detailed CPU OS memory statistics,\n");
    printf("==================================\n");
    printf("\n");
    for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
    {
        if(appIpcIsCpuEnabled(cpu_id))
        {
            #if defined(LINUX) || defined(QNX)
            /* NOT supported for Linux/QNX A72 as of now */
            if(cpu_id!=appIpcGetSelfCpuId())
            #endif
            {
                status = appPerfStatsCpuOsStatsGet(cpu_id, &cpu_os_stats);
                if(status==0)
                {
                    appPerfStatsCpuOsStatsPrint(cpu_id, &cpu_os_stats, 0);
                }
            }
        }
    }
    printf("\n");
    return status;
}

int32_t appPerfStatsStackStatsPrintAll()
{
    uint32_t cpu_id;
    int32_t status=0;
    app_perf_stats_task_stats_t cpu_task_stats;

    printf("\n");
    printf("Detailed CPU Task Stack statistics,\n");
    printf(" (Unused stack memory, in bytes)\n");
    printf("====================================\n");
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
                status = appPerfStatsCpuTaskStacksStatsGet(cpu_id, &cpu_task_stats);
                if(status==0)
                {
                    appPerfStatsCpuTaskStacksStatsPrint(cpu_id, &cpu_task_stats);
                    if(cpu_task_stats.num_tasks == APP_PERF_STATS_TASK_MAX)
                    {
                        cpu_task_stats.starting_task = APP_PERF_STATS_TASK_MAX;
                        status = appPerfStatsCpuTaskStacksStatsGet(cpu_id, &cpu_task_stats);
                        if(status==0)
                        {
                            appPerfStatsCpuTaskStacksStatsPrint(cpu_id, &cpu_task_stats);
                        }
                    }
                }
            }
        }
    }
    printf("\n");
    return status;
}

int main(int argc, char *argv[])
{
    vx_status status = 0;
    int i;

    status = appCommonInit();

    if(status == VX_SUCCESS)
    {
        status = appPerfStatsMemStatsPrintAll();

        if(status == VX_SUCCESS)
        {
            status = appPerfStatsOsStatsPrintAll();
        }

        if(status == VX_SUCCESS)
        {
            if(argc>1)
            {
                for(i=0; i<argc; i++)
                {
                    if(strcmp(argv[i], "-v")==0)
                    {
                        status = appPerfStatsStackStatsPrintAll();
                    }
                }
            }
        }

        if(status == VX_SUCCESS)
        {
            status = appCommonDeInit();
        }
    }
    printf("APP HEAP STATS: Done !!!\n");

    return status;
}
