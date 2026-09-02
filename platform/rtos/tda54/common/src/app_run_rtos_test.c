/*
 *
 * Copyright (c) 2018-2026 Texas Instruments Incorporated
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

#include CORE_CFG_FILE

#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/console_io/include/app_log.h>
#include <utils/console_io/include/app_cli.h>
#include <utils/timer/include/app_timer.h>

typedef struct {

    uint32_t count;
    uint32_t delay;

} app_rtos_test_prm_t;

static int32_t appRtosTestHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    app_rtos_test_prm_t *test_prm = (app_rtos_test_prm_t*)prm;

    uint32_t count = 0;
    uint64_t elasped_time = 0;

    #if 0
    appLogPrintf("TI-RTOS TEST: Running test for %d iterations and delay of %d ms\n",
        test_prm->count,
        test_prm->delay
        );
    #endif

    elasped_time = appLogGetTimeInUsec();
    while (count < test_prm->count)
    {
        appLogPrintf (" %d: Core is UP!!! \n", count);
        count++;
        appLogWaitMsecs(test_prm->delay);
    }
    elasped_time = appLogGetTimeInUsec() - elasped_time;

    appLogPrintf("TI-RTOS TEST: Test done in %d ms (expected value is %d ms + print time)\n",
        (uint32_t)(elasped_time/1000u),
        (uint32_t)(test_prm->count*test_prm->delay));

    return 0;
}

void appRtosTestRegister()
{
    appRemoteServiceRegister("rtos_test", appRtosTestHandler);
}

void appRtosTestUnRegister()
{
    appRemoteServiceUnRegister("rtos_test");
}


int appRunRtosTest(int argc, char *argv[])
{
    uint32_t cpu_id = APP_IPC_CPU_MAX;
    uint32_t all_cpus = 1;
    app_rtos_test_prm_t prm;

    prm.count = 3;
    prm.delay = 100;

    if(argc==2)
    {
        all_cpus = 0;
        cpu_id = appIpcGetAppCpuId(argv[1]);
    }
    if(all_cpus==0)
    {
        if(cpu_id < APP_IPC_CPU_MAX)
        {
            appRemoteServiceRun(cpu_id, "rtos_test", 0, &prm, sizeof(prm), 0);
        }
        else
        {
            appLogPrintf(" %s: Invalid CPU ID (%d) specified\n", argv[0], cpu_id);
        }
    }
    else
    {
        for(cpu_id=0; cpu_id<APP_IPC_CPU_MAX; cpu_id++)
        {
            if(appIpcIsCpuEnabled(cpu_id))
            {
                appRemoteServiceRun(cpu_id, "rtos_test", 0, &prm, sizeof(prm), 0);
            }
        }
    }

    return 0;
}
