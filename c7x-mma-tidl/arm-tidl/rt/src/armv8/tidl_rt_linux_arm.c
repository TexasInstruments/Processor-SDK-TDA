/*
*
* Copyright (c) {2015 - 2020} Texas Instruments Incorporated
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
#include <stdlib.h>

#include <TI/tivx.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/perf_stats/src/app_perf_stats_priv.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/app_init/include/app_init.h>
#include "itidl_rt.h"
#include "../tidl_rt_ovx_utils.h"

void tidlrt_printf(const char *fmt, ...);

uint32_t tidlrt_ddr_stats= 0;

#define APP_PERF_DDR_BURST_SIZE_BYTES   (64u)  /* in units of bytes */

int32_t TIDLRT_getDdrStats(uint64_t *read_bytes, uint64_t *write_bytes)
{
    int32_t status = -1;

    if(tidlrt_ddr_stats != 0U)
    {
        app_perf_stats_ddr_stats_t ddr_stats;
        #if defined(SOC_AM62A)
            status = appRemoteServiceRun(APP_IPC_CPU_MCU1_0, APP_PERF_STATS_SERVICE_NAME,
                APP_PERF_STATS_CMD_GET_DDR_COUNTERS,
                &ddr_stats, (uint32_t)(sizeof(app_perf_stats_ddr_stats_t)),
                0);
        #elif defined(SOC_J721E) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2) || defined (SOC_J722S) 
            status = appRemoteServiceRun(APP_IPC_CPU_MCU2_0, APP_PERF_STATS_SERVICE_NAME,
                APP_PERF_STATS_CMD_GET_DDR_COUNTERS,
                &ddr_stats, (uint32_t)(sizeof(app_perf_stats_ddr_stats_t)),
                0);
        #elif defined(SOC_TDA54) // TODO: Currently, everything is running on MCU0_M55 , need to change in future  
            status = appRemoteServiceRun(APP_IPC_CPU_MCU0_M55, APP_PERF_STATS_SERVICE_NAME,
                APP_PERF_STATS_CMD_GET_DDR_COUNTERS,
                &ddr_stats, (uint32_t)(sizeof(app_perf_stats_ddr_stats_t)),
                0);
        #endif

        if(status == 0)
        {
            *read_bytes  = (uint64_t)ddr_stats.counter0_total * APP_PERF_DDR_BURST_SIZE_BYTES;
            *write_bytes = (uint64_t)ddr_stats.counter1_total * APP_PERF_DDR_BURST_SIZE_BYTES;
        }
        else
        {
            tidlrt_printf("Could not get DDR stats. appRemoteServiceRun failed with status = %d\n", status);
        }
    }
    else
    {
        *read_bytes  = 0;
        *write_bytes = 0;
        status = 0;
    }

    return status;
}

int32_t tidl_rt_ovx_Init(void)
{
    int32_t status = 0;
    char *debug_str = NULL;
    debug_str = getenv("TIDL_RT_DDR_STATS");
    if(debug_str == NULL)
    {
        tidlrt_ddr_stats = 1;
    }
    else
    {
        char *endptr = NULL;
        unsigned long val = strtoul(debug_str, &endptr, 10);
        if ((debug_str != endptr) && (*endptr == '\0'))
        {
            tidlrt_ddr_stats = (uint32_t)val;
        }
    }

    char *skip_init = NULL;
    skip_init = getenv("SKIP_TIOVX_INIT");
    if (skip_init == NULL)
    {
        tidlrt_printf("Calling appInit() in TIDL-RT!\n");
        status = appInit();
    }
    else
    {
        tidlrt_printf("Skipping appInit() in TIDL-RT!\n");
    }

    return status;
}

int32_t tidl_rt_ovx_DeInit(int memOnly)
{
    int32_t status = 0;

    char *skip_init = NULL;
    skip_init = getenv("SKIP_TIOVX_INIT");
    if (skip_init == NULL)
    {
        tidlrt_printf("Calling appDeInit() in TIDL-RT!\n");
        status = appDeInit();
    }
    else
    {
        tidlrt_printf("Skipping appDeInit() in TIDL-RT!\n");
    }

    return status;
}
