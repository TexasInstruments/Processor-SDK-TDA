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

#include <utils/ipc/include/app_ipc.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <app_init.h>
#include "app_common.h"

#define NUM_MSGS        (5u)
#define NUM_ITERATIONS  (1u)

int main(int argc, char *argv[])
{
    vx_status status = 0;
    uint32_t i, msg, k;


    status = appInit();

    if(status == VX_SUCCESS)
    {
        #if 1
        for(k=0; k<NUM_ITERATIONS; k++)
        {
            for(i=0; i<APP_IPC_CPU_MAX; i++)
            {
                if(appIpcIsCpuEnabled(i) && i != appIpcGetSelfCpuId())
                {
                    for(msg=0; msg<NUM_MSGS; msg++)
                    {
                        status = appIpcSendNotify(i, (0xDEAD0000 | (msg & 0xFFFF)));
                        APP_PRINTF("APP IPC: Sent msg %d to CPU [%s]\n", msg+1, appIpcGetCpuName(i));
                        if(status != VX_SUCCESS)
                        {
                            printf("APP IPC: ERROR: Send msg %d to CPU [%s] failed !!!\n", msg+1, appIpcGetCpuName(i));
                            break;
                        }
                        /* appLogWaitMsecs(10); */
                    }
                }
            }
        }
        printf("APP IPC: Waiting for all messages to get echoed from remote core...\n");
        appLogWaitMsecs(10000);
        printf("APP IPC: Waiting for all messages to get echoed ... Done.\n");
        #endif

        #if 1
        {
            int32_t appRemoteServiceTestRun(uint32_t cpu_id);

            printf("APP IPC: Running remote service test ... \n");
            for(i=0; i<APP_IPC_CPU_MAX; i++)
            {
                if(appIpcIsCpuEnabled(i) && i != appIpcGetSelfCpuId())
                {
                    appRemoteServiceTestRun(i);
                }
            }
            printf("APP IPC: Running remote service test ... Done.\n");
        }
        #endif

        appDeInit();
    }
    printf("APP IPC: Done !!!\n");

    return status;
}
