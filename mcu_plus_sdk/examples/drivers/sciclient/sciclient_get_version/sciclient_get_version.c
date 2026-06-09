/*
 *  Copyright (C) 2021-2024 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <inttypes.h>
#include "FreeRTOS.h"
#include "task.h"
#include <kernel/dpl/ClockP.h>
#if defined ENABLE_SCICLIENT_DIRECT
/* DM R5 will use the SCI Client direct */
#include <drivers/device_manager/sciclient.h>
#else
#include <drivers/sciclient.h>
#endif
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#if defined (ENABLE_DM_TRACE)
#include <drivers/device_manager/sciclient_direct/sciclient_trace_internal.h>
#endif

#if defined (ENABLE_DM_TRACE)
/*
 * Low priority for DM trace task - must be least than all the tasks to print trace logs.
 * This is perferred because whenever DM receives a service request
 * that request should be excuted first.
 */
#define SCISERVER_DM_TRACE_TASK_PRI      (1)
#define TRACETASK_SIZE (16384U/sizeof(configSTACK_DEPTH_TYPE))
#endif

#if defined (ENABLE_DM_TRACE)
/* Prints the logs stored in tracelog_dm buffer when DM is waiting for message request */
static void Sciclient_dmTrace(void* a0);
#endif

#if defined (ENABLE_DM_TRACE)
extern char tracelog_dm[];
extern uint32_t gDMTraceBufIndex;
extern uint32_t gDMTraceBufCount;
StackType_t gTrace_TaskStack[TRACETASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gTrace_TaskObj;
TaskHandle_t gTrace_Task;
#endif

void sciclient_get_version_main(void *args)
{
    int32_t         retVal = SystemP_SUCCESS;

    /* Check for the SYSFW version by sending a request */
    struct tisci_msg_version_req request;
    const Sciclient_ReqPrm_t      reqPrm =
    {
        TISCI_MSG_VERSION,
        TISCI_MSG_FLAG_AOP,
        (uint8_t *) &request,
        sizeof(request),
        SystemP_WAIT_FOREVER
    };

    struct tisci_msg_version_resp response;
    Sciclient_RespPrm_t           respPrm =
    {
        0,
        (uint8_t *) &response,
        sizeof (response)
    };

    retVal = Sciclient_service(&reqPrm, &respPrm);
    DebugP_assert(SystemP_SUCCESS == retVal && respPrm.flags == TISCI_MSG_FLAG_ACK);

    DebugP_log("SYSFW Firmware Version %s\r\n",(char *) response.str);
    DebugP_log("Firmware revision 0x%x\r\n", response.version);
    DebugP_log("ABI revision %d.%d\r\n",response.abi_major,response.abi_minor);

    /* Do an ABI check */
    retVal = Sciclient_abiCheck();
    DebugP_assert(SystemP_SUCCESS == retVal);

    {
        /* Fetch the CPU clock */
        uint64_t clkRate = 0;

        retVal = Sciclient_pmGetModuleClkFreq(
                    Sciclient_getSelfDevIdCore(),
                    TISCI_DEV_WKUP_R5FSS0_CORE0_CPU_CLK,
                    &clkRate,
                    SystemP_WAIT_FOREVER);
        DebugP_assert(SystemP_SUCCESS == retVal);

        DebugP_log("[SCICLIENT] CPU clock frequency = %" PRId64 " Hz \r\n", clkRate);
    }
#if defined (AMP_FREERTOS_A53)
    DebugP_log("All tests have passed on a53_core%d !!\r\n", Armv8_getCoreId());
#else
    DebugP_log("All tests have passed!!\r\n");
#endif
#if defined (ENABLE_DM_TRACE)
    gTrace_Task = xTaskCreateStatic( Sciclient_dmTrace,            /* Pointer to the function that implements the task. */
                                   "Sciclient_dmTrace",            /* Text name for the task.  This is to facilitate debugging only. */
                                   TRACETASK_SIZE,                 /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                   NULL,                           /* We are not using the task parameter. */
                                   SCISERVER_DM_TRACE_TASK_PRI,    /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                   gTrace_TaskStack,               /* pointer to stack base */
                                   &gTrace_TaskObj );
#endif

    Board_driversClose();
    Drivers_close();
}

#if defined (ENABLE_DM_TRACE)
static void Sciclient_dmTrace(void* a0)
{
    Drivers_open();
    Board_driversOpen();
    uint32_t index;
    uint32_t count = 0;
    DebugP_log("--- Start of DM trace ----\n");

    /* Loop through tracelog_dm buffer to print the DM trace logs */
    for (index = 0; index <= DM_TRACE_LOG_BUF_SIZE; index++)
    {
        /* Here we check if we have reached the end of the buffer
         * and reset index to start while incrementing count
         * so that we can print all the logs without missing */
        if(DM_TRACE_LOG_BUF_SIZE == index)
        {
            index = 0;
            count++;
        }
        /* Wait till gDMTraceBufIndex or gDMTraceBufCount is updated */
        while(index == gDMTraceBufIndex && count == gDMTraceBufCount)
        {
            ClockP_usleep(1);
        }
        DebugP_log("%c", tracelog_dm[index]);
    }

    return;
}
#endif
