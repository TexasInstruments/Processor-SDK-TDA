/*
 *  Copyright (c) Texas Instruments Incorporated 2018-2025
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
 *  \file main_tirtos.c
 *
 *  \brief Main file for TI-RTOS build
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ti/drv/ipc/examples/common/src/ipc_setup.h>

#include <ti/osal/osal.h>
#include <ti/osal/TaskP.h>

#include "ipc_utils.h"
#include <ti/csl/csl_types.h>
#if defined (BUILD_C7X)
#include <ti/csl/soc.h>
#include <ti/csl/arch/csl_arch.h>

#include <ti/osal/soc/osal_soc.h>

#if (defined (FREERTOS))
#include "Hwi.h"
#include "Mmu.h"
#elif (defined (SAFERTOS))
#include "Mmu.h"
#endif
#endif

#include <ti/drv/sciclient/sciclient.h>
#include <ti/board/board.h>

#if defined(SOC_J721E)
#include <ti/board/src/j721e_evm/include/board_power.h>
#elif defined(SOC_J7200)
#include <ti/board/src/j7200_evm/include/board_power.h>
#elif defined(SOC_J721S2)
#include <ti/board/src/j721s2_evm/include/board_power.h>
#elif defined(SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_power.h>
#elif defined(SOC_J742S2)
#include <ti/board/src/j742s2_evm/include/board_power.h>
#endif

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
#include <ti/drv/sciclient/src/sciclient/sciclient_priv.h>
#include <ti/drv/sciclient/sciserver_tirtos.h>
#endif

/* This needs to be enabled only for negative test cases */
#ifdef IPC_NEGATIVE_TEST
#include <ti/drv/ipc/examples/rtos/ipc_negative_test/ipc_neg_setup.h>
#endif

#ifdef IPC_INVALID_ENDPT_TEST
#include <ti/drv/ipc/examples/rtos/ipc_invalid_endpt_test/ipc_invalid_endpt_test.h>
#endif

/* This needs to be enabled only for extended test cases */
#ifdef IPC_EXTENDED_TEST
#include <ti/drv/ipc/examples/rtos/ipc_extended_test/ipc_extended_setup.h>
#endif

#if defined (ENABLE_DM_TRACE)
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/sciclient/src/sciclient/sciclient_trace_internal.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define APP_TSK_STACK_MAIN              (32U * 1024U)
/**< Test application stack size */

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
/**< SCI Server Init Task stack size */
#define APP_SCISERVER_INIT_TSK_STACK        (32U * 1024U)
/* SCI Server Init Task Priority - must be higher than High priority Sciserver task */
#define IPC_INIT_SCISERVER_TASK_PRI         (6)
#endif

/* High Priority for SCI Server - must be higher than Low priority task */
#define IPC_SETUP_SCISERVER_TASK_PRI_HIGH   (5)
/*
 * Low Priority for SCI Server - must be higher than IPC echo test tasks
 * to prevent delay in handling Sciserver requests when test is performing
 * multicore ping/pong.
 */
#define IPC_SETUP_SCISERVER_TASK_PRI_LOW    (4)
/*
 * Low priority for DM trace task - must be least than all the tasks to print trace logs.
 * This is perferred because whenever DM receives a service request
 * that request should be excuted first.
 */
#define IPC_SETUP_DM_TRACE_TASK_PRI         (3)

#if defined (ENABLE_DM_TRACE)
/* DM_TRACE_LOG_BUF_SIZE is of size 8MB
 * which is the size of tracelog_dm to store trace logs
 */
#define DM_TRACE_LOG_BUF_SIZE              (8U*1024U*1024U)
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void taskFxn(void* a0, void* a1);

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
int32_t Ipc_pmicShutdown(void);
void Ipc_setupSciServer(void *arg0, void *arg1);
/**< Initialize SCI Server, to process RM/PM Requests by other cores */
#endif

#if defined (ENABLE_DM_TRACE)
/* This function prints the logs stored in tracelog_dm buffer when DM is waiting for message request */
static void Ipc_dmTrace(void* a0, void* a1);
#endif

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if defined (ENABLE_DM_TRACE)
char tracelog_dm[DM_TRACE_LOG_BUF_SIZE];
extern uint32_t gDMTraceBufIndex;
extern uint32_t gDMTraceBufCount;
TaskP_Handle gTrace_task;
TaskP_Params gTrace_taskParams;
static uint8_t  gTrace_TskStackMain[APP_TSK_STACK_MAIN];
#endif

#if (defined (BUILD_MCU1_0) && (defined (SOC_J7200) || defined (SOC_J784S4) || defined (SOC_J742S2) || defined (SOC_J721S2)))
extern uint32_t gLpmWakeReason;
#endif

/* Test application stack */
/* For SafeRTOS on R5F with FFI Support, task stack should be aligned to the stack size */
#if defined(SAFERTOS) && defined (BUILD_MCU)
static uint8_t  gAppTskStackMain[APP_TSK_STACK_MAIN]
__attribute__ ((aligned(APP_TSK_STACK_MAIN)));
#else
static uint8_t  gAppTskStackMain[APP_TSK_STACK_MAIN]
__attribute__ ((aligned(8192)));
#endif

/* Variable to check if ipc_boardInit has completed or not*/
uint8_t  gBoardinit=0;

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
/* Sciserver Init TAsk stack */
#if defined(SAFERTOS)
static uint8_t  gSciserverInitTskStack[APP_SCISERVER_INIT_TSK_STACK]
__attribute__ ((aligned(APP_SCISERVER_INIT_TSK_STACK)));
#else
static uint8_t  gSciserverInitTskStack[APP_SCISERVER_INIT_TSK_STACK]
__attribute__ ((aligned(8192)));
#endif
extern Sciclient_ServiceHandle_t gSciclientHandle;
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */


void ipc_initSciclient()
{
    int32_t ret = CSL_PASS;
    Sciclient_ConfigPrms_t        config;

    /* Now reinitialize it as default parameter */
    ret = Sciclient_configPrmsInit(&config);
    if (ret != CSL_PASS)
    {
        App_printf("Sciclient_configPrmsInit Failed\n");
    }

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
    if (ret == CSL_PASS)
    {
        ret = Sciclient_boardCfgParseHeader(
            (uint8_t *)SCISERVER_COMMON_X509_HEADER_ADDR,
            &config.inPmPrms, &config.inRmPrms);
        if (ret != CSL_PASS)
        {
            App_printf("Sciclient_boardCfgParseHeader Failed\n");
        }
    }
#endif

    if (ret == CSL_PASS)
    {
        ret = Sciclient_init(&config);
        if (ret != CSL_PASS)
        {
            App_printf("Sciclient_init Failed\n");
        }
#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
        if (gSciclientHandle.rmBoardConfigComplete == SCICLIENT_FT_PASS)
        {
            App_printf("Sciclient_boardCfgRm init Passed\n");
        }
        else
        {
            App_printf("Sciclient_boardCfgRm init FAILED!\n");
        }
#endif
    }
}

void ipc_boardInit()
{
    Board_initCfg           boardCfg;

    boardCfg = BOARD_INIT_UART_STDIO;
#if defined(A72_LINUX_OS)
    /* Configure UART TX pinmux only. Linux doesn't support full pinmux config */
    Board_uartTxPinmuxConfig();
#else
    boardCfg |= BOARD_INIT_PINMUX_CONFIG;
#endif

    Board_init(boardCfg);

    /* Mark Board_init() has been called */
    gBoardinit = 1;
}

int main(void)
{
    TaskP_Handle task;
    TaskP_Params taskParams;



#if defined ECHO_TEST_BTCM && defined FREERTOS && defined BUILD_MCU
    /* Relocate FreeRTOS Reset Vectors from BTCM*/
    void _freertosresetvectors (void);
    memcpy((void *)0x0, (void *)_freertosresetvectors, 0x40);
#endif

#if defined ECHO_TEST_BTCM && defined SAFERTOS && defined BUILD_MCU
    /* Relocate FreeRTOS Reset Vectors from BTCM*/
    void _axSafeRTOSresetVectors (void);
    memcpy((void *)0x0, (void *)_axSafeRTOSresetVectors, 0x40);
#endif

    /*  This should be called before any other OS calls (like Task creation, OS_start, etc..) */
    OS_init();

    #if defined (ENABLE_DM_TRACE)
    /* Giving access  of user defined tracelog_dm buffer to debug application */
    Sciclient_setTracelogBuffer(tracelog_dm, DM_TRACE_LOG_BUF_SIZE);
    #endif

    /* Initialize the task params */
    TaskP_Params_init(&taskParams);
    /* Set the task priority higher than the default priority (1) */
    taskParams.priority = 2;
    taskParams.stack        = gAppTskStackMain;
    taskParams.stacksize    = sizeof (gAppTskStackMain);

    task = TaskP_create(&taskFxn, &taskParams);
    if(NULL == task)
    {
        OS_stop();
    }
    OS_start();    /* does not return */

    return(0);
}

static void taskFxn(void* a0, void* a1)
{
#if (defined (BUILD_MCU1_0) && (defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
    if (gLpmWakeReason == 0xFFFFFFFFU)
    {
        /* The first two bytes store the wakeup reason */
        gLpmWakeReason = (CSL_REG32_RD_OFF(SCICLIENT_MCU_SCRATCHPAD, SCICLIENT_WAKE_REASON_OFFSET)) & 0xFFFFU;
    }
#endif

    /* Initialize SCI Client - It must be called before board init */
    ipc_initSciclient();
    /* IPC Board Init should be done only for MCU1_0 for Linux,
     * unconditionally for RTOS
     */
#if defined(A72_LINUX_OS) && defined(BUILD_MCU1_0)
    ipc_boardInit();
#elif !defined(A72_LINUX_OS)
    ipc_boardInit();
#endif

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
    TaskP_Handle sciserverInitTask;
    TaskP_Params sciserverInitTaskParams;

    /* Initialize SCI Client Server */
    TaskP_Params_init(&sciserverInitTaskParams);
    sciserverInitTaskParams.priority     = IPC_INIT_SCISERVER_TASK_PRI;
    sciserverInitTaskParams.stack        = gSciserverInitTskStack;
    sciserverInitTaskParams.stacksize    = sizeof (gSciserverInitTskStack);

    sciserverInitTask = TaskP_create(&Ipc_setupSciServer, &sciserverInitTaskParams);
    if(NULL == sciserverInitTask)
    {
        OS_stop();
    }
#endif

#if defined (_TMS320C6X)
#if defined (FREERTOS) || defined (SAFERTOS)
    ipc_cacheMarInit();
#endif
#endif

#if defined IPC_NEGATIVE_TEST
    Ipc_echo_neg_test();
#elif defined IPC_EXTENDED_TEST
    IpcApp_extTest();
#elif defined IPC_INVALID_ENDPT_TEST
    ipc_invalid_endpt_test();
#else
    Ipc_echo_test();

#if(defined(BUILD_C7X_1) && (IPC_SANITY_C7X))
#if defined LDRA_DYN_COVERAGE_EXIT
    UART_printf("\n LDRA ENTRY... \n");
    upload_execution_history();
    UART_printf("\n LDRA EXIT... \n");
#endif
#endif

#if defined (ENABLE_DM_TRACE)
    /* Initialize the task params */
    TaskP_Params_init(&gTrace_taskParams);
    gTrace_taskParams.priority     = IPC_SETUP_DM_TRACE_TASK_PRI;
    gTrace_taskParams.stack        = gTrace_TskStackMain;
    gTrace_taskParams.stacksize    = sizeof (gTrace_TskStackMain);
    gTrace_task = TaskP_create(&Ipc_dmTrace, &gTrace_taskParams);
#endif
#endif

}

#if defined(BUILD_C7X)
void InitMmu(void)
{
    IpcInitMmu(FALSE);
    IpcInitMmu(TRUE);
    OsalCfgClecAccessCtrl(false);

}
#endif

#if defined(BUILD_MPU)
extern void Osal_initMmuDefault(void);
void InitMmu(void)
{
    Osal_initMmuDefault();
}
#endif

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
void Ipc_setupSciServer(void *arg0, void *arg1)
{

    Sciserver_TirtosCfgPrms_t appPrms;
    int32_t ret = CSL_PASS;
    char *version_str = NULL;
    char *rmpmhal_version_str = NULL;

    ret = Sciserver_tirtosInitPrms_Init(&appPrms);

    appPrms.taskPriority[SCISERVER_TASK_USER_LO] =
                                            IPC_SETUP_SCISERVER_TASK_PRI_LOW;
    appPrms.taskPriority[SCISERVER_TASK_USER_HI] =
                                            IPC_SETUP_SCISERVER_TASK_PRI_HIGH;

    if (ret == CSL_PASS)
    {
        ret = Sciserver_tirtosInit(&appPrms);
    }

    /* Set PMIC Shutdown Function Pointer */
    if (ret == CSL_PASS)
    {
        ret = Sciclient_setPmicShutdownCb(Ipc_pmicShutdown);
    }

    version_str = Sciserver_getVersionStr();
    rmpmhal_version_str = Sciserver_getRmPmHalVersionStr();
    App_printf("DM Built On: %s %s\n", __DATE__, __TIME__);
    App_printf("Sciserver Version: %s\n", version_str);
    App_printf("RM_PM_HAL Version: %s\n", rmpmhal_version_str);

    if (ret == CSL_PASS)
    {
        App_printf("Starting Sciserver..... PASSED\n");
    }
    else
    {
        App_printf("Starting Sciserver..... FAILED\n");
    }

    return;
}
#endif

#if (defined (BUILD_MCU1_0) && (defined (SOC_J721E) || defined (SOC_J7200) || defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)))
int32_t Ipc_pmicShutdown(void)
{
    int32_t ret = CSL_PASS;
    Board_STATUS status = -1;

    status = Board_pmPowerOff(0x48);
    if (status != 0) {
        ret = CSL_EFAIL;
    }

#if defined(SOC_J721E)
    if (ret == CSL_PASS) {
        status = Board_pmPowerOff(0x4C);
        if (status != 0) {
            ret = CSL_EFAIL;
        }
    }
#endif

    return ret;
}
#endif

#if defined (ENABLE_DM_TRACE)
void Ipc_dmTrace(void* a0, void* a1)
{
    uint32_t index;
    uint32_t count = 0;
    UART_printf("--- Start of DM trace ----\n");

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
            Osal_delay(1);
        }
        UART_printf("%c", tracelog_dm[index]);
    }

     return;
}
#endif
