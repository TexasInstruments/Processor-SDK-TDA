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
 *  \file main_rtos.c
 *
 *  \brief Main file for TI-RTOS build
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <ti/csl/arch/csl_arch.h>
#include <ti/csl/soc.h>
#include <ti/csl/cslr.h>
#include <ti/board/board.h>

#include <ti/osal/osal.h>
#include <ti/osal/TaskP.h>

#include <ti/drv/sciclient/sciserver_tirtos.h>

#include <ti/drv/lpm/lpm.h>
#include <ti/drv/lpm/src/lpm_mcu_only_recovery.h>
#include <ti/drv/lpm/src/lpm_uart_drv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Test application stack size */
#define APP_TASK_STACK                  (10U * 1024U)
/**< Stack required for the stack */
#define MAIN_APP_TASK_PRIORITY          (2)
/**< Task Priority Levels */

#define APP_SCISERVER_INIT_TSK_STACK    (32U * 1024U)

#define LPM_INIT_SCISERVER_TASK_PRI     (8)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void MainApp_TaskFxn(void* a0, void* a1);
static void SetupSciServer(void* a0, void* a1);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

TaskP_Handle mainAppTask;
TaskP_Params mainAppTaskParams;
static uint8_t MainApp_TaskStack[APP_TASK_STACK] __attribute__((aligned(32)));
/**< Stack for the Main task */

static uint8_t  gSciserverInitTskStack[APP_SCISERVER_INIT_TSK_STACK]
__attribute__ ((aligned(8192)));

/* ========================================================================== */
/*                            External Variables                              */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
int main(void)
{
    OS_init();

    /* Initialize the task params */
    TaskP_Params_init(&mainAppTaskParams);
    mainAppTaskParams.priority       = MAIN_APP_TASK_PRIORITY;
    mainAppTaskParams.stack          = MainApp_TaskStack;
    mainAppTaskParams.stacksize      = sizeof (MainApp_TaskStack);

    mainAppTask = TaskP_create(&MainApp_TaskFxn, &mainAppTaskParams);
    if (NULL == mainAppTask)
    {
        OS_stop();
    }

    OS_start();    /* does not return */

    return(0);
}


/* MCU Only task */
uint32_t Lpm_pmicApp(void)
{
    uint32_t status = 0;
    uint8_t input=0;
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Inside MCU ONLY task!\n");

    /* Before entering MCU_ONLY mode we need to disable all VTM temp sensors in
       the MAIN domain - VTM_TMPSENS1-4 */
    /* Disabling the VTM MAXT_OUTRG_ALERT_THR */
    Lpm_mcuOnlyDisableMaxOutrgAlert();

    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                    "STATE INFO :: CURRENTLY IN ACTIVE MODE!\n");
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "LED LD5 should be ON\n");
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                    "Please measure TP133 and TP134!\n");
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                    "Expected values in ACTIVE mode:\nTP133: HIGH\nTP134: HIGH\n");

    while(1)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                        "Kindly unload Remoteproc modules from Linux. Type these commands in Linux CMD:\n   modprobe -r ti_k3_r5_remoteproc\n   modprobe -r ti_k3_dsp_remoteproc\n");
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "After modules have been removed, press 1 and enter\n");
        UART_scanFmt("%d", &input);
        if (1U == input)
        {
            /* Change state from ACTIVE to MCU ONLY */
            if (0 == status)
            {
                Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                                "############################ ACTIVE -> MCU ONLY MODE ############################\n");
                /* The status is dummy currently */
                status = Lpm_mcuOnlyActiveToMcuSwitch();
                Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                                "######################### ACTIVE -> MCU ONLY MODE DONE ##########################\n");
            }
            else
            {
                Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG">> ERROR :: Status not correct!!!\n");
            }

            Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                            "STATE INFO :: NOW IN MCU ONLY MODE!\n");
            Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "LED LD5 should be OFF\n");
            Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                            "Sleeping for 10s, please measure TP133 and TP134!\n");
            Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                            "Expected values in MCU ONLY mode:\nTP133: HIGH\nTP134: LOW\n");
            TaskP_sleep(10000);

            /* Change state from MCU ONLY to ACTIVE */
            if (0 == status)
            {
                Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                                "############################ MCU ONLY -> ACTIVE MODE ############################\n");
                Lpm_mcuOnlyToActiveSwitch();
                Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                                "######################### MCU ONLY -> ACTIVE MODE DONE ##########################\n");
            }
            else
            {
                Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG">> ERROR :: Status not correct!!!\n");
            }

            Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                            "STATE INFO :: CURRENTLY IN ACTIVE MODE!\n");
            Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "LED LD5 should be ON\n");
            Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                            "Expected values in ACTIVE mode:\nTP133: HIGH\nTP134: HIGH\n");
            break;

        }
        else
        {
            Lpm_uartDrvPrintf("Invalid input: ");
        }
    }
    return status;
}

static void MainApp_TaskFxn(void* a0, void* a1)
{
#if defined(UART_PRINT_DEBUG)
    uint64_t timeIPCStart, timeIPCFinish;
    uint64_t timeBootAppStart, timeBootAppFinish;
    uint64_t timeMcuOnlyAppStart, timeMcuOnlyAppFinish;
#endif
    uint32_t i, numBoots=5;
    int32_t ret = CSL_PASS;

    Board_init(BOARD_INIT_UART_STDIO);

    Sciclient_init(NULL_PTR);

    TaskP_Handle sciserverInitTask;
    TaskP_Params sciserverInitTaskParams;

    /* Initialize SCI Client Server */
    TaskP_Params_init(&sciserverInitTaskParams);
    sciserverInitTaskParams.priority     = LPM_INIT_SCISERVER_TASK_PRI;
    sciserverInitTaskParams.stack        = gSciserverInitTskStack;
    sciserverInitTaskParams.stacksize    = sizeof (gSciserverInitTskStack);

    sciserverInitTask = TaskP_create(&SetupSciServer, &sciserverInitTaskParams);
    if(NULL == sciserverInitTask)
    {
        OS_stop();
    }

    if(ret != CSL_PASS)
    {
        OS_stop();
    }

    Lpm_uartDrvPrintf("\nMCU R5F App started at %d usecs\r\n", (uint32_t)TimerP_getTimeInUsecs());

    Lpm_bootAppInit();
    Lpm_pmicInit();
    Lpm_uartDrvPrintf("\nPMIC initialization done.\r\n");

    for(i=0; i<numBoots; i++)
    {
#if defined(UART_PRINT_DEBUG)
        Lpm_uartDrvPrintf("\nLoop %d starts!\n",i);
        timeIPCStart = TimerP_getTimeInUsecs();
#endif

        Lpm_ipcEchoApp();

#if defined(UART_PRINT_DEBUG)
        timeIPCFinish = TimerP_getTimeInUsecs();
        Lpm_uartDrvPrintf("\nIPC Task started at %d usecs and finished at %d usecs\r\n",
                        (uint32_t)timeIPCStart,
                        (uint32_t)timeIPCFinish);
        Lpm_uartDrvPrintf("************* Sleeping for 1 sec after IPC Task ***********\n");
        TaskP_sleep(1000);
        timeBootAppStart = TimerP_getTimeInUsecs();
#endif
        
        Lpm_bootApp();
        
#if defined(UART_PRINT_DEBUG)
        timeBootAppFinish = TimerP_getTimeInUsecs();
        Lpm_uartDrvPrintf("\nBootApp Task started at %d usecs and finished at %d usecs\r\n",
                        (uint32_t)timeBootAppStart,
                        (uint32_t)timeBootAppFinish);
        Lpm_uartDrvPrintf("************* Sleeping for 1 sec after MCU Boot Task***********\n");
        TaskP_sleep(1000);
        Lpm_uartDrvPrintf("De-initializing the IPC driver and deleting the tasks\n");
#endif

        Lpm_ipcExitResponseTask();
        TaskP_sleep(1000);
        
#if defined(UART_PRINT_DEBUG)
        Lpm_uartDrvPrintf("************* Slept for 1 sec after IPC de-initializing Task. IPC should have deinitialized now. ***********\n");
        timeMcuOnlyAppStart = TimerP_getTimeInUsecs();
#endif

        Lpm_pmicApp();

#if defined(UART_PRINT_DEBUG)
        timeMcuOnlyAppFinish = TimerP_getTimeInUsecs();
        Lpm_uartDrvPrintf("\nMCU Only Task started at %d usecs and finished at %d usecs\r\n",
                        (uint32_t)timeMcuOnlyAppStart,
                        (uint32_t)timeMcuOnlyAppFinish);
        Lpm_uartDrvPrintf("\nLoop %d ends!\n",i);
#endif
    }

    Lpm_bootAppDeInit();

    return;
}

void SetupSciServer(void* a0, void* a1)
{

    Sciserver_TirtosCfgPrms_t appPrms;
    Sciclient_ConfigPrms_t clientPrms;
    int32_t ret = CSL_PASS;

    appPrms.taskPriority[SCISERVER_TASK_USER_LO] = 6;
    appPrms.taskPriority[SCISERVER_TASK_USER_HI] = 7;

    /* Sciclient needs to be initialized before Sciserver. Sciserver depends on
     * Sciclient API to execute message forwarding */
    ret = Sciclient_configPrmsInit(&clientPrms);

    if (ret == CSL_PASS)
    {
        ret = Sciclient_init(&clientPrms);
    }

    if (ret == CSL_PASS)
    {
        ret = Sciserver_tirtosInit(&appPrms);
    }

    if (ret == CSL_PASS)
    {
        Lpm_uartDrvPrintf("Starting Sciserver..... PASSED\n");
    }
    else
    {
        Lpm_uartDrvPrintf("Starting Sciserver..... FAILED\n");
    }

    return ;
}
