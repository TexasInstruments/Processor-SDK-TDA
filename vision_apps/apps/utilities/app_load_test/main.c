/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
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

#include <stdint.h>
#include <stdlib.h>
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

#define APP_TEST_DURATION   (30*1000)
#define MCU1_FAMILY 3
#define MCU2_FAMILY 4
#define MCU3_FAMILY 6
#define MCU4_FAMILY 7
#define C6X_FAMILY  5
#define C7X_FAMILY  2

vx_status appCpuLoadPrint(uint32_t cpu_id)
{
    vx_status status = VX_SUCCESS;
    app_perf_stats_cpu_load_t cpu_load;

    if(appIpcIsCpuEnabled(cpu_id))
    {
        if(status == VX_SUCCESS)
        {
            status = appPerfStatsCpuLoadGet(cpu_id, &cpu_load);
        }
        if(status == VX_SUCCESS)
        {
            status = appPerfStatsCpuLoadPrint(cpu_id, &cpu_load);
        }
    }
    return status;
}

int main(int argc, char *argv[])
{
    vx_status status = 0;
    int load, time, core, core_id[4], core_cnt = 2, i;
    uint64_t startTime, elaspedTime;

    if (argc != 4)
    {
        printf("Wrong number of arguments! Need 4 arguments\n");
        return -1;
    }

    status = appCommonInit();

    core = atoi(argv[1]);
    load = atoi(argv[2]);
    time = atoi(argv[3]);

    #if defined(SOC_AM62A)
    if (core == MCU1_FAMILY)
    {
        core_id[0] = APP_IPC_CPU_MCU1_0;
        core_cnt = 1;
    }
    #elif defined(SOC_J722S)
    if (core == MCU2_FAMILY)
    {
        core_id[0] = APP_IPC_CPU_MCU2_0;
        core_cnt   = 1;
    }
    #else
    if (core == MCU2_FAMILY)
    {
        core_id[0] = APP_IPC_CPU_MCU2_0;
        core_id[1] = APP_IPC_CPU_MCU2_1;
    }
    else if (core == MCU3_FAMILY)
    {
        core_id[0] = APP_IPC_CPU_MCU3_0;
        core_id[1] = APP_IPC_CPU_MCU3_1;
    }
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    else if (core == MCU4_FAMILY)
    {
        core_id[0] = APP_IPC_CPU_MCU4_0;
        core_id[1] = APP_IPC_CPU_MCU4_1;
    }
    #endif
    #if defined(SOC_J721E)
    else if (core == C6X_FAMILY)
    {
        core_id[0] = APP_IPC_CPU_C6x_1;
        core_id[1] = APP_IPC_CPU_C6x_2;
    }
    #endif
    #endif
    else if (core == C7X_FAMILY)
    {
        #if defined(SOC_J721E) || defined(SOC_AM62A)
        core_id[0] = APP_IPC_CPU_C7x_1;
        core_cnt = 1;
        #endif

        #if defined(SOC_J721S2) || defined(SOC_J722S)
        core_id[0] = APP_IPC_CPU_C7x_1;
        core_id[1] = APP_IPC_CPU_C7x_2;
        core_cnt = 2;
        #endif

        #if defined(SOC_J784S4)
        core_id[0] = APP_IPC_CPU_C7x_1;
        core_id[1] = APP_IPC_CPU_C7x_2;
        core_id[2] = APP_IPC_CPU_C7x_3;
        core_id[3] = APP_IPC_CPU_C7x_4;
        core_cnt = 4;
        #endif

        #if defined(SOC_J742S2)
        core_id[0] = APP_IPC_CPU_C7x_1;
        core_id[1] = APP_IPC_CPU_C7x_2;
        core_id[2] = APP_IPC_CPU_C7x_3;
        core_cnt = 3;
        #endif
    }
    else
    {
        printf("Invalid core_id!!\n");
        return -1;
    }

    printf("core is %d The load is %d percent time is %d seconds\n", core, load, time);

    if(status == VX_SUCCESS)
    {
        vx_status appRemoteServiceTestRunLoadTestStart(uint32_t cpu_id, uint32_t load);
        vx_status appRemoteServiceTestRunLoadTestStop(uint32_t cpu_id);


        for (i = 0; i < core_cnt; i++)
            status = appRemoteServiceTestRunLoadTestStart(core_id[i], load);

        appPerfStatsResetAll();
        startTime = appLogGetTimeInUsec();
        while(status == VX_SUCCESS)
        {
            appLogWaitMsecs(2000);

            for (i = 0; i < core_cnt; i++)
                status = appCpuLoadPrint(core_id[i]);

            elaspedTime = appLogGetTimeInUsec() - startTime;

            if( (elaspedTime/1000) > time * 1000)
            {
                break;
            }
        }
        if(status == VX_SUCCESS)
        {
            for (i = 0; i < core_cnt; i++)
                status = appRemoteServiceTestRunLoadTestStop(core_id[i]);
        }
        if(status == VX_SUCCESS)
        {
            status = appCommonDeInit();
        }
    }
    printf("APP IPC TIOVX: Done !!!\n");

    return status;
}
