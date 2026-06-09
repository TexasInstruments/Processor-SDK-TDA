/*
 *
 * Copyright (c) 2023-2026 Texas Instruments Incorporated
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

#include <app.h>
#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/sciserver/include/app_sciserver.h>
#include <utils/misc/include/app_misc.h>
#include <utils/rtos/include/app_rtos.h>
#include <stdio.h>
#include <string.h>
#include <app_ipc_rsctable.h>
#include "app_cfg_mcu1_0.h"

#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <ipc_notify.h>
#include <ipc_notify/v0/ipc_notify_v0.h>

/**< SCI Server Init Task stack size */
#define APP_SCISERVER_INIT_TSK_STACK        (32U * 1024U)
/* SCI Server Init Task Priority - must be higher than High priority Sciserver task */
#define INIT_SCISERVER_TASK_PRI         (6)

/* Sciserver Init Task stack */
static uint8_t  gSciserverInitTskStack[APP_SCISERVER_INIT_TSK_STACK]
#if defined(SAFERTOS)
__attribute__ ((aligned(APP_SCISERVER_INIT_TSK_STACK)));
#else
__attribute__ ((aligned(8192)));
#endif

#if defined(SAFERTOS)
extern void System_lateInit(void);
extern signed long xTaskStartScheduler( void );
#else
extern void vTaskStartScheduler( void );
#endif

void IpcNotify_getConfig(IpcNotify_InterruptConfig **interruptConfig, uint32_t *interruptConfigNum)
{
    /* extern globals that are specific to this core */
    extern IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_wkup_r5fss0_0[];
    extern uint32_t gIpcNotifyInterruptConfigNum_wkup_r5fss0_0;

    *interruptConfig = &gIpcNotifyInterruptConfig_wkup_r5fss0_0[0];
    *interruptConfigNum = gIpcNotifyInterruptConfigNum_wkup_r5fss0_0;
}

static void appMain(void* arg0, void* arg1)
{
    app_rtos_task_handle_t sciserverInitTask;
    app_rtos_task_params_t sciserverInitTaskParams;

    appUtilsTaskInit();

    Drivers_open();
    Board_driversOpen();

    /* Initialize SCI Client Server */
    appRtosTaskParamsInit(&sciserverInitTaskParams);
    sciserverInitTaskParams.priority     = INIT_SCISERVER_TASK_PRI;
    sciserverInitTaskParams.stack        = gSciserverInitTskStack;
    sciserverInitTaskParams.stacksize    = sizeof (gSciserverInitTskStack);
    sciserverInitTaskParams.taskfxn = &appSciserverInit;

    sciserverInitTask = appRtosTaskCreate(&sciserverInitTaskParams);

    if(NULL == sciserverInitTask)
    {
        extern void vTaskEndScheduler( void );
        vTaskEndScheduler();
    }

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

void StartupEmulatorWaitFxn (void)
{
    volatile uint32_t enableDebug = 0;
    do
    {
    }while (enableDebug);
}

/**< SCI Server Init Task stack size */
#define APP_MCU1_0_TASK_STACK        (64U * 1024U)

static uint8_t gTskStackMain[APP_MCU1_0_TASK_STACK]
__attribute__ ((section(".bss:taskStackSection")))
#if defined(SAFERTOS)
__attribute__ ((aligned(APP_MCU1_0_TASK_STACK)));
#else
__attribute__ ((aligned(8192)));
#endif
    ;

int main(void)
{
    app_rtos_task_params_t tskParams;
    app_rtos_task_handle_t task;

#if defined FREERTOS
    /* Relocate FreeRTOS Reset Vectors from BTCM*/
    void _vectors (void);
    memcpy((void *)0x0, (void *)_vectors, 0x40);
#endif

#if defined SAFERTOS
    /* Relocate SafeRTOS Reset Vectors from BTCM*/
    void _axSafeRTOSresetVectors (void);
    memcpy((void *)0x0, (void *)_axSafeRTOSresetVectors, 0x40);
#endif

    /* This is for debug purpose - see the description of function header */
    StartupEmulatorWaitFxn();

    System_init();
    Board_init();

    #if defined(SAFERTOS)
    System_lateInit();
    #endif

    /* Initialize the task params */
    appRtosTaskParamsInit(&tskParams);
    /* Set the task priority higher than the default priority (1) */
    tskParams.priority     = 2;
    tskParams.stack        = gTskStackMain;
    tskParams.stacksize    = sizeof (gTskStackMain);
    tskParams.taskfxn = &appMain;
    task = appRtosTaskCreate(&tskParams);

    DebugP_assert(task != NULL);

    #if defined(SAFERTOS)
    xTaskStartScheduler();
    #else
    vTaskStartScheduler();
    #endif
    
    /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
    DebugP_assertNoLog(0);

    return 0;
}

uint64_t appTarget2SharedConversion(const uint64_t virtAddr)
{
  uint64_t phyAddr = (uint64_t)virtAddr;

  if ( ((uint64_t)virtAddr >= (uint64_t)DDR_SHARED_MEM_ADDR) &&
       ((uint64_t)virtAddr < ((uint64_t)DDR_SHARED_MEM_ADDR + (uint64_t)DDR_SHARED_MEM_SIZE)) )
  {
        if ((uint64_t)DDR_SHARED_MEM_PHYS_ADDR >= (uint64_t)DDR_SHARED_MEM_ADDR)
        {
            phyAddr = (uint64_t)virtAddr + ((uint64_t)DDR_SHARED_MEM_PHYS_ADDR - (uint64_t)DDR_SHARED_MEM_ADDR);
        }
        else
        {
            phyAddr = (uint64_t)virtAddr - ((uint64_t)DDR_SHARED_MEM_ADDR - (uint64_t)DDR_SHARED_MEM_PHYS_ADDR);
        }
  }

  return phyAddr;
}

uint64_t appUdmaVirtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
    return appTarget2SharedConversion((uint64_t)virtAddr);
}

uint64_t appShared2TargetConversion(const uint64_t shared_ptr)
{
    uint64_t target_ptr;

    /* Note: I think this is correct but needs review */
    if ( ((uint64_t)shared_ptr >= (uint64_t)DDR_SHARED_MEM_PHYS_ADDR) &&
         ((uint64_t)shared_ptr < ((uint64_t)DDR_SHARED_MEM_PHYS_ADDR + (uint64_t)DDR_SHARED_MEM_PHYS_SIZE)) )
    {
        if ((uint64_t)DDR_SHARED_MEM_PHYS_ADDR >= (uint64_t)DDR_SHARED_MEM_ADDR)
        {
            target_ptr = (uint64_t)shared_ptr - ((uint64_t)DDR_SHARED_MEM_PHYS_ADDR - (uint64_t)DDR_SHARED_MEM_ADDR);
        }
        else
        {
            target_ptr = (uint64_t)shared_ptr + ((uint64_t)DDR_SHARED_MEM_ADDR - (uint64_t)DDR_SHARED_MEM_PHYS_ADDR);
        }
    }
    else
    {
        target_ptr = (uint64_t)shared_ptr;
    }

    return target_ptr;
}
