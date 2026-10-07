/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \file main_rtos.c
 *
 *  \brief Main file for FreeRTOS build
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdlib.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_config.h"
#include "ti_board_open_close.h"
#include "FreeRTOS.h"
#include "task.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define MAIN_TASK_PRI                   (configMAX_PRIORITIES - 1)

/* Test application stack size */
#define APP_TSK_STACK_MAIN              (16U * 1024U)

#define MAIN_TASK_SIZE                  (APP_TSK_STACK_MAIN / sizeof(configSTACK_DEPTH_TYPE))

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void taskFxn(void *args);
extern int32_t DruApp_ReadbackTest(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Test application stack */
static StackType_t gAppTskStackMain[MAIN_TASK_SIZE] __attribute__((aligned(32)));

static StaticTask_t gAppTskObjMain;
static TaskHandle_t gAppTskMain;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    /* init SOC specific modules */
    System_init();
    Board_init();

    gAppTskMain = xTaskCreateStatic(&taskFxn,
                                    "dru_static_regs_readback",
                                    MAIN_TASK_SIZE,
                                    NULL,
                                    MAIN_TASK_PRI,
                                    gAppTskStackMain,
                                    &gAppTskObjMain);
    configASSERT(gAppTskMain != NULL);

    /* Start the scheduler to start the tasks executing. */
    vTaskStartScheduler();

    /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
    DebugP_assertNoLog(0);

    return (0);
}

static void taskFxn(void *args)
{
    /* Open drivers to open the UART driver for console */
    Drivers_open();
    /* Open flash and board specific drivers */
    Board_driversOpen();

    DruApp_ReadbackTest();

    /* Close Board related and dependent drivers */
    Board_driversClose();
    Drivers_close();

    vTaskDelete(NULL);
}
