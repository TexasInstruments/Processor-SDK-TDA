/*
 *  Copyright (c) Texas Instruments Incorporated 2026
 *  All rights reserved.
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

/**
 *  \file dss_main_rtos.c
 *
 *  \brief Main file for RTOS builds
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include <stdio.h>
#include "ti/osal/osal.h"
#include "ti/osal/TaskP.h"
#include "ti/osal/LoadP.h"

#include <ti/board/board.h>
#include <ti/drv/dss/examples/utils/app_utils.h>
#include "dss_display_test.h"
#include <ti/drv/sciclient/sciclient.h>
#include <ti/csl/soc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Test application stack size */
#define DISP_APP_TSK_STACK_MAIN         (64U * 1024U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void taskFxn(void* a0, void* a1);
extern int32_t DssSyncLoss_DisplayTest(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Test application stack */
static uint8_t gDispAppTskStackMain[DISP_APP_TSK_STACK_MAIN];
/* Task handle */
TaskP_Handle gTask;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    TaskP_Params taskParams;

    OS_init();

    /* Initialize the task params */
    TaskP_Params_init(&taskParams);
    /* Set the task priority higher than the default priority (1) */
    taskParams.priority  = 2;
    taskParams.stack     = gDispAppTskStackMain;
    taskParams.stacksize = sizeof(gDispAppTskStackMain);

    gTask = TaskP_create(&taskFxn, &taskParams);
    if(NULL == gTask)
    {
        OS_stop();
    }

    OS_start();    /* does not return */

    return(0);
}

static void taskFxn(void* a0, void* a1)
{
    int32_t retVal = CSL_PASS;

    Board_initCfg boardCfg;
    App_utilsLcdCfgParams lcdParams;

    boardCfg = BOARD_INIT_PINMUX_CONFIG |
                BOARD_INIT_UNLOCK_MMR |
                BOARD_INIT_UART_STDIO;
    Board_init(boardCfg);

    DssSyncLoss_Print("\n********\nDSS test\n********\n\n");

    lcdParams.outType  = APP_OUTPUT_EDP;
    lcdParams.pixelClk = (uint64_t)148500000ULL;
    App_configureLCD(lcdParams);

    App_configureSoC();

    Sciclient_init(NULL);
    retVal += Sciclient_pmSetModuleState(TISCI_DEV_DSS0,
                                   TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                   TISCI_MSG_FLAG_AOP,
                                   SCICLIENT_SERVICE_WAIT_FOREVER);
    retVal += Sciclient_pmSetModuleState(TISCI_DEV_DSS_EDP0,
                                   TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                   TISCI_MSG_FLAG_AOP,
                                   SCICLIENT_SERVICE_WAIT_FOREVER);                                  

    if (CSL_PASS == retVal)
    {
        TaskP_sleep(2000);
        DssSyncLoss_DisplayTest();
    }

    return;
}

