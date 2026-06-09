/*
 *
 * Copyright (c) 2026 Texas Instruments Incorporated
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

#include <app.h>
#include <device.h>
#include <HwiP.h>

#include <utils/console_io/include/app_log.h>
#include <utils/timer/include/app_timer.h>
#include <utils/misc/include/app_misc.h>
#include <utils/rtos/include/app_rtos.h>
// #include <utils/perf_stats/include/app_perf_stats.h>

#include <app_mem_map.h>
#include <app_ipc_rsctable.h>
#include <app_cfg_c7x_2.h>

extern void vTaskStartScheduler( void );
extern void vTaskDelete( void * );

void Dpl_Init(void)
{
    HwiP_init();
    HwiP_enable();
}

static void appMain(void* arg0, void* arg1)
{
    appUtilsTaskInit();

    appInit();
    /* HACK: to be enabled in EVM, since the task spawned slows down the simulation*/
    // appRun();

    #if 1
    vTaskDelete(NULL);
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

/* HACK to be enabled once C7x is enabled on VDK
 static void appC7xClecInitDru(void)
 {
     CSL_ClecEventConfig   cfgClec;
     CSL_CLEC_EVTRegs   *clecBaseAddr;
     int32_t status = SystemP_SUCCESS;
     uint32_t clusterId;

     clusterId=CSL_clecGetC7xClusterId();

     if (clusterId == CSL_C75_CPU_CLUSTER_NUM_C75_1)
     {
         clecBaseAddr = (CSL_CLEC_EVTRegs*)CSL_C7X1024V0_CLEC_BASE;
     }
     else if (clusterId == CSL_C75_CPU_CLUSTER_NUM_C75_2)
     {
         clecBaseAddr = (CSL_CLEC_EVTRegs*)CSL_C7X1024V0_CLEC_BASE;
     }
     else
     {
         status = SystemP_FAILURE;
     }

     if (SystemP_SUCCESS == status)
     {
         uint32_t i;
         uint32_t dru_input_start = 128;
         uint32_t dru_input_num   = 16;
         for(i=dru_input_start; i<(dru_input_start+dru_input_num); i++)
         {
             cfgClec.secureClaimEnable = FALSE;
             cfgClec.evtSendEnable     = TRUE;

             cfgClec.rtMap             = CSL_CLEC_RTMAP_CPU_4;
             cfgClec.extEvtNum         = 0;
             cfgClec.c7xEvtNum         = (i-dru_input_start)+32;
             CSL_clecConfigEvent(clecBaseAddr, i, &cfgClec);
         }
     }
 }
*/

/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 16K
 * - AND stack assigned for task context is "size - 8KB"
 * - 8KB chunk for the stack area is used for interrupt handling in this task context
 */
static uint8_t gTskStackMain[64*1024]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;

int main(void)
{
    app_rtos_task_params_t tskParams;
    app_rtos_task_handle_t task;
    StartupEmulatorWaitFxn();

    Device_Init();
    Dpl_Init();
    /* HACK to be enabled once C7x is enabled on VDK
    appC7xClecInitDru();
    */

    /* HACK to be enabled once perf stats is enabled in app_utils
    appPerfStatsInit();
    */

    appRtosTaskParamsInit(&tskParams);
    tskParams.priority = 8u;
    tskParams.stack = gTskStackMain;
    tskParams.stacksize = sizeof (gTskStackMain);
    tskParams.taskfxn = &appMain;
    task = appRtosTaskCreate(&tskParams);

    DebugP_assert(task != NULL);
    vTaskStartScheduler();
    /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
    DebugP_assertNoLog(0);

    return 0;
}

/** Description : This function converts a virtual memory region address to physical memory region address given the base addresses of both memory regions
 *  which are mapped using MMU in appMmuMap
 *  Arguments:
 *      - virtAddr : Virtual pointer
 *      - virtBase   : Base address of virtual memory space
 *      - physBase   : Base address of physical memory space
 *      - size       : Size of memory space
 *      - phyAddr :  Physical pointer to be returned after conversion if virtAddr belongs to memory space with base address virtBase
 *                     else, phyAddr is returned as it is without any modification
 */
static void convertVirt2Phys(uint64_t virtAddr, uint64_t virtBase, uint64_t physBase, uint64_t size, uint64_t * phyAddr)
{
    if ( ((uint64_t)virtAddr >= virtBase) &&
         ((uint64_t)virtAddr < (virtBase + size)) )
    {
        if (virtBase >= physBase)
        {
            *phyAddr = (uint64_t)virtAddr - (virtBase - physBase);
        }
        else
        {
            *phyAddr = (uint64_t)virtAddr + (physBase - virtBase);
        }
    }
}

uint64_t appTarget2SharedConversion(const uint64_t virtAddr)
{
    uint64_t phyAddr = (uint64_t)virtAddr; /* Default : Return virtAddr without any modification */

  /* Note: I think this is correct but needs review */
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
    else
    {
        /* Below code converts c7x_1 virtual addresses of all cores to C7x DDR physical addresses
           If virtAddr does not belong to any of the below defined virtual memory regions, it will be returned without any modification.
           A virtAddr would belong to any one of the below memory spaces, so convertVirt2Phys call for that particular space would be executed
           All other function calls would just pass through the phyAddr */
        convertVirt2Phys(virtAddr, (uint64_t) DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_3_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_4_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);

        convertVirt2Phys(virtAddr, (uint64_t) DDR_C7X_1_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_2_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_3_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_4_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_PHYS_SIZE, &phyAddr);

        convertVirt2Phys(virtAddr, (uint64_t) DDR_C7X_1_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_2_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_3_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_4_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);

        convertVirt2Phys(virtAddr, (uint64_t) DDR_C7X_1_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_1_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_2_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_2_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_3_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_3_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_1_4_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_4_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_4_SCRATCH_PHYS_SIZE, &phyAddr);
    }
    return phyAddr;
}

uint64_t appUdmaVirtToPhyAddrConversion(const void *virtAddr,
                                      uint32_t chNum,
                                      void *appData)
{
    return appTarget2SharedConversion((uint64_t)virtAddr);
}

/** Description : This function converts a physical memory region address to virtual memory region address given the base addresses of both memory regions
 *  which are mapped using MMU in appMmuMap
 *  Arguments:
 *      - shared_ptr : Physical pointer
 *      - virtBase   : Base address of virtual memory space
 *      - physBase   : Base address of physical memory space
 *      - size       : Size of memory space
 *      - target_ptr : Virtual pointer to be returned after conversion if shared_ptr belongs to memory space with base address physBase
 *                     else, target_ptr is returned as it is without any modification
 */
static void convertPhys2Virt(uint64_t shared_ptr, uint64_t virtBase, uint64_t physBase, uint64_t size, uint64_t * target_ptr)
{
    if ( ((uint64_t)shared_ptr >= physBase) &&
         ((uint64_t)shared_ptr < (physBase + size)) )
    {
        if (physBase >= virtBase)
        {
            *target_ptr = shared_ptr - (physBase - virtBase);
        }
        else
        {
            *target_ptr = (uint64_t)shared_ptr + (virtBase - physBase);
        }
    }
}

uint64_t appShared2TargetConversion(const uint64_t shared_ptr)
{
    uint64_t target_ptr = shared_ptr; /* Default : Return shared_ptr without any modification */

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
        /* Below code converts C7x DDR physical addresses of all cores to c7x_1 virtual addresses
           If shared_ptr does not belong to any of the below defined physical memory regions, it will be returned without any modification.
           A shared_ptr would belong to any one of the below memory spaces, so convertPhys2Virt call for that particular space would be executed
           All other function calls would just pass through the target_ptr */
        convertPhys2Virt(shared_ptr, (uint64_t) DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_2_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_3_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_4_LOCAL_HEAP_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);

        convertPhys2Virt(shared_ptr, (uint64_t) DDR_C7X_1_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_2_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_3_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_4_LOCAL_HEAP_ADDR,
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_PHYS_SIZE, &target_ptr);

        convertPhys2Virt(shared_ptr, (uint64_t) DDR_C7X_1_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_2_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_3_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_4_SCRATCH_NON_CACHEABLE_ADDR,
            (uint64_t) DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);

        convertPhys2Virt(shared_ptr, (uint64_t) DDR_C7X_1_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_1_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_2_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_2_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_3_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_3_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_1_4_SCRATCH_ADDR,
            (uint64_t) DDR_C7X_4_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_4_SCRATCH_PHYS_SIZE, &target_ptr);
    }
    return target_ptr;
}
