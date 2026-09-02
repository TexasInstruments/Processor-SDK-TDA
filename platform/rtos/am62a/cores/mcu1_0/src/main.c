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

#include <platform.h>
#include <app_init.h>

#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/misc/include/app_misc.h>
#include <utils/rtos/include/app_rtos.h>
#include <utils/perf_stats/include/app_perf_stats.h>

#if !defined(MCU_PLUS_SDK)
#include <osal.h>
#else
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <ipc_notify.h>
#include <ipc_notify/v0/ipc_notify_v0.h>
#if defined (THREADX)
#include <tx_api.h>
#endif
#endif

/**< SCI Server Init Task stack size */
#define APP_SCISERVER_INIT_TSK_STACK        (32U * 1024U)
/* SCI Server Init Task Priority - must be higher than High priority Sciserver task */
#if defined (THREADX)
#define INIT_SCISERVER_TASK_PRI         (25)
#else
#define INIT_SCISERVER_TASK_PRI         (6)
#endif

/* Sciserver Init Task stack */
static uint8_t  gSciserverInitTskStack[APP_SCISERVER_INIT_TSK_STACK]
__attribute__ ((aligned(8192)));
DM_LPMData_t gDMLPMData __attribute__((section(".lpm_data"), aligned(4)));

#if defined (MCU_PLUS_SDK)

#if !defined (THREADX)
extern void vTaskStartScheduler( void );
#else
#define TASK_PRI_MAIN_THREAD  (2)
TX_THREAD main_thread;
extern void tx_kernel_enter( void );
#endif

void IpcNotify_getConfig(IpcNotify_InterruptConfig **interruptConfig, uint32_t *interruptConfigNum)
{
    /* extern globals that are specific to this core */
    extern IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_r5fss0_0[];
    extern uint32_t gIpcNotifyInterruptConfigNum_r5fss0_0;

    *interruptConfig = &gIpcNotifyInterruptConfig_r5fss0_0[0];
    *interruptConfigNum = gIpcNotifyInterruptConfigNum_r5fss0_0;
}

#endif

void StartupEmulatorWaitFxn (void)
{
    volatile uint32_t enableDebug = 0;
    do
    {
    }while (enableDebug);
}

#if defined (THREADX)
static void appMain(ULONG arg)
#else
static void appMain(void* arg0, void* arg1)
#endif
{
    app_rtos_task_handle_t sciserverInitTask;
    app_rtos_task_params_t sciserverInitTaskParams;

    appUtilsTaskInit();

#if !defined(MCU_PLUS_SDK)
    appSciserverSciclientInit();
#else
    Drivers_open();
    Board_driversOpen();
    Sciclient_initDeviceManagerLPMData(&gDMLPMData);
#endif

    int32_t retVal = 0;
    retVal |= Sciclient_pmSetModuleState(TISCI_DEV_DMASS1,
                                TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                TISCI_MSG_FLAG_AOP,
                                SCICLIENT_SERVICE_WAIT_FOREVER);

    retVal |= Sciclient_pmSetModuleState(TISCI_DEV_DMASS1_BCDMA_0,
                                TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                TISCI_MSG_FLAG_AOP,
                                SCICLIENT_SERVICE_WAIT_FOREVER);

    retVal |= Sciclient_pmSetModuleState(TISCI_DEV_DMASS1_INTAGGR_0,
                                TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                TISCI_MSG_FLAG_AOP,
                                SCICLIENT_SERVICE_WAIT_FOREVER);

    /* Initialize SCI Client Server */
    appRtosTaskParamsInit(&sciserverInitTaskParams);
    sciserverInitTaskParams.priority     = INIT_SCISERVER_TASK_PRI;
    sciserverInitTaskParams.stack        = gSciserverInitTskStack;
    sciserverInitTaskParams.stacksize    = sizeof (gSciserverInitTskStack);
    sciserverInitTaskParams.taskfxn = &appSciserverInit;

    sciserverInitTask = appRtosTaskCreate(&sciserverInitTaskParams);

#if !defined(MCU_PLUS_SDK)
    if(NULL == sciserverInitTask)
    {
        OS_stop();
    }
#else
    if(NULL == sciserverInitTask)
    {
        extern void TaskP_endScheduler( void );
        TaskP_endScheduler();
    }
#endif

    StartupEmulatorWaitFxn();
    appInit();
    appRun();
    #if 1
    while(1)
    {
        appLogWaitMsecs(100u);
    }
    #else
    appDeInit();
    #endif
}


static uint8_t gTskStackMain[64*1024]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;

int appResume()
{
    appResumeInit();
    return 0;
}

int appSuspend()
{
    appResumeDeInit();
    return 0;
}

int main(void)
{
    app_rtos_task_params_t tskParams;
    app_rtos_task_handle_t task;

#if !defined(MCU_PLUS_SDK)
#if defined FREERTOS
    /* Relocate FreeRTOS Reset Vectors from BTCM*/
    void _freertosresetvectors (void);
    memcpy((void *)0x0, (void *)_freertosresetvectors, 0x40);
#endif

    /* This is for debug purpose - see the description of function header */
    OS_init();
#else
#if defined FREERTOS
    /* Relocate FreeRTOS Reset Vectors from BTCM*/
    void _vectors (void);
    memcpy((void *)0x0, (void *)_vectors, 0x40);
#endif
    System_init();
    Board_init();
    appPerfStatsInit();
#endif

/* LPM for VPAC will be enabled once support is provided */
#if 0
    Sciclient_initLPMSusResHook(appSuspend, appResume);
#endif

#if !defined (THREADX)
    /* Initialize the task params */
    appRtosTaskParamsInit(&tskParams);
    /* Set the task priority higher than the default priority (1) */
    tskParams.priority     = 2;
    tskParams.stack        = gTskStackMain;
    tskParams.stacksize    = sizeof (gTskStackMain);
    tskParams.taskfxn = &appMain;
    task = appRtosTaskCreate(&tskParams);
#endif

#if !defined(MCU_PLUS_SDK)
    if(NULL == task)
    {
        OS_stop();
    }
    OS_start();
#else
    #if !defined (THREADX)
    DebugP_assert(task != NULL);
    vTaskStartScheduler();
    /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
    #else
    tx_kernel_enter();
    #endif
    DebugP_assertNoLog(0);
#endif

    return 0;
}

#if defined (THREADX)
void tx_application_define(void *first_unused_memory)
{
    UINT status;
    status = tx_thread_create(&main_thread,           /* Pointer to the main thread object. */
                              "main_thread",          /* Name of the task for debugging purposes. */
                              appMain,                /* Entry function for the main thread. */
                              0,                      /* Arguments passed to the entry function. */
                              gTskStackMain,          /* Main thread stack. */
                              sizeof (gTskStackMain), /* Main thread stack size in bytes. */
                              TASK_PRI_MAIN_THREAD,   /* Main task priority. */
                              TASK_PRI_MAIN_THREAD,   /* Highest priority level of disabled preemption. */
                              TX_NO_TIME_SLICE,       /* No time slice. */
                              TX_AUTO_START);         /* Start immediately. */

    DebugP_assertNoLog(status == TX_SUCCESS);
}
#endif

uint32_t appGetDdrSharedHeapSize()
{
    return DDR_SHARED_MEM_SIZE;

}

uint64_t appUdmaVirtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{

  return (uint64_t)virtAddr;
}
