/*
 *  Copyright (C) 2025 Texas Instruments Incorporated
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
 *
 */

/**
 *  \file  sciclient_merged_main.c
 *
 *  \brief Test application implementation for sciclient merged library.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <string.h>
#include <ti/csl/soc.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/csl/cslr.h>
#include <ti/osal/osal.h>
#include <ti/osal/TaskP.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/sciclient/examples/common/sci_app_common.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Test application stack size */
#define SCICLIENT_APP_TASK_STACK_MAIN (32U * 1024U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if defined(SAFERTOS)
/* Test application stack */
static uint8_t gSciclientAppTaskStackMain[SCICLIENT_APP_TASK_STACK_MAIN] __attribute__((aligned(SCICLIENT_APP_TASK_STACK_MAIN))) = {0};
#else
/* Test application stack */
static uint8_t gSciclientAppTaskStackMain[SCICLIENT_APP_TASK_STACK_MAIN] __attribute__((aligned(32)));
#endif

/* ========================================================================== */
/*                         Structures Declarations                            */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Function Declarations                              */
/* ========================================================================== */

static void SciApp_mainTask(void* arg0, void* arg1);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    TaskP_Handle taskHandle;
    TaskP_Params taskParams;
    int32_t retVal = CSL_PASS;

    SciApp_consoleInit();
    OS_init();

    TaskP_Params_init(&taskParams);
    taskParams.priority  = 4;
    taskParams.stack     = gSciclientAppTaskStackMain;
    taskParams.stacksize = sizeof(gSciclientAppTaskStackMain);

    taskHandle = TaskP_create(&SciApp_mainTask, &taskParams);
    if (taskHandle == NULL)
    {
        SciApp_printf("Task_create() SciApp_mainTask failed!\n");
        OS_stop();
    }

    /* Start BIOS */
    OS_start();

    return retVal;
}

static void SciApp_mainTask(void* arg0, void* arg1)
{

    SciApp_printf("Start of Sciclient Merged Test App\n\n");

    int32_t status                  = CSL_PASS;
    Sciclient_ConfigPrms_t   config =
    {
        SCICLIENT_SERVICE_OPERATION_MODE_POLLED,
        NULL
    };
    struct tisci_msg_version_req request;
    const Sciclient_ReqPrm_t reqPrm =
    {
        TISCI_MSG_VERSION,
        TISCI_MSG_FLAG_AOP,
        (uint8_t *) &request,
        sizeof(request),
        SCICLIENT_SERVICE_WAIT_FOREVER
    };
    struct tisci_msg_version_resp response;
    Sciclient_RespPrm_t     respPrm =
    {
        0,
        (uint8_t *) &response,
        sizeof(response)
    };

    status = Sciclient_init(&config);
    if (status == CSL_PASS)
    {
        status = Sciclient_service(&reqPrm, &respPrm);
        if ((status == CSL_PASS) && (TISCI_MSG_FLAG_ACK == respPrm.flags))
        {
            SciApp_printf(" TIFS Firmware Version 1 %s\n",
                            (char *) response.str);
            SciApp_printf(" Firmware revision 0x%x\n", response.version);
            SciApp_printf(" ABI revision %d.%d\n", response.abi_major,
                            response.abi_minor);
        }
        else
        {
            SciApp_printf(" TIFS Firmware Get Version failed \n");
        }
    }
    if (status == CSL_PASS)
    {
        uint32_t deviceState = 0;
        uint32_t resetState = 0;
        uint32_t contextLossState = 0;
        status = Sciclient_pmGetModuleState(TISCI_DEV_RTI0,
                                            &deviceState,
                                            &resetState,
                                            &contextLossState,
                                            SCICLIENT_SERVICE_WAIT_FOREVER);
        if (status == CSL_PASS)
        {
            SciApp_printf("%u Device Id's: Device state %u, reset state %u, context loss state %u\n",
                          (uint32_t)(TISCI_DEV_RTI0),
                          deviceState,
                          resetState,
                          contextLossState);
        }
        else
        {
            SciApp_printf(" Device: Get Module state failed \n");
        }
    }
    if (status == CSL_PASS)
    {
        status = Sciclient_deinit();
    }
    if (status == CSL_PASS)
    {
        SciApp_printf("\nAll tests have PASSED.\n");
    }
    else
    {
        SciApp_printf("\nSome of the Tests have FAILED.\n");
    }
}