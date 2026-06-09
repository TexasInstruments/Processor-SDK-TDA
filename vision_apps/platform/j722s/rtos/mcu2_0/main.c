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
#include <utils/misc/include/app_misc.h>
#include <utils/rtos/include/app_rtos.h>
#include <stdio.h>
#include <string.h>
#include <app_ipc_rsctable.h>
#include "app_cfg_mcu2_0.h"
#include <utils/perf_stats/include/app_perf_stats.h>
#ifdef ENABLE_ETHFW
#include <utils/ethfw/include/app_ethfw.h>
#endif
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <ipc_notify.h>
#include <ipc_notify/v0/ipc_notify_v0.h>

#if defined(SAFERTOS)
extern void System_lateInit(void);
extern signed long xTaskStartScheduler( void );
#else
extern void vTaskStartScheduler( void );
#endif

void IpcNotify_getConfig(IpcNotify_InterruptConfig **interruptConfig, uint32_t *interruptConfigNum)
{
    /* extern globals that are specific to this core */
    extern IpcNotify_InterruptConfig gIpcNotifyInterruptConfig_main_r5fss0_0[];
    extern uint32_t gIpcNotifyInterruptConfigNum_main_r5fss0_0;

    *interruptConfig = &gIpcNotifyInterruptConfig_main_r5fss0_0[0];
    *interruptConfigNum = gIpcNotifyInterruptConfigNum_main_r5fss0_0;
}

static void appMain(void* arg0, void* arg1)
{
    appUtilsTaskInit();

    Drivers_open();
    Board_driversOpen();

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

#define APP_MCU2_0_TASK_STACK        (16U * 1024U)

static uint8_t gTskStackMain[APP_MCU2_0_TASK_STACK]
__attribute__ ((section(".bss:taskStackSection")))
#if defined(SAFERTOS)
__attribute__ ((aligned(APP_MCU2_0_TASK_STACK)));
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

    appPerfStatsInit();

    #ifdef ENABLE_ETHFW
    appEthFwEarlyInit();
    #endif

    appRtosTaskParamsInit(&tskParams);
    tskParams.priority = 8u;
    tskParams.stack = gTskStackMain;
    tskParams.stacksize = sizeof (gTskStackMain);
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

uint32_t appGetDdrSharedHeapSize()
{
    return DDR_SHARED_MEM_SIZE;

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
