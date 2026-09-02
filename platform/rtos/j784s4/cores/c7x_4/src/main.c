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
#include <ti/osal/osal.h>
#include <ti/osal/HwiP.h>
#include <ti/osal/CacheP.h>
#include <ti/csl/soc.h>
#include <ti/csl/arch/csl_arch.h>
#include <ti/csl/arch/c7x/Cache.h>
#include <ti/csl/arch/c7x/Hwi.h>
#include <ti/csl/arch/c7x/Mmu.h>

#if (defined (SAFERTOS))
#include "SafeRTOS_API.h"
#include "SafeRTOSConfig.h"
#endif

#define C7X_CORE_ID 3

/* For J7ES/J721E/TDA4VM the upper 2GB DDR starts from 0x0008_8000_0000 */
/* This address is mapped to a virtual address of 0x0001_0000_0000 */

static void appMain(void* arg0, void* arg1)
{
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

void StartupEmulatorWaitFxn1 (void)
{
    volatile uint32_t enableDebug = 0;
    do
    {
    }while (enableDebug);
}

/* IMPORTANT NOTE: For C7x,
 * - stack size and stack ptr MUST be 8KB aligned
 * - AND min stack size MUST be 16KB
 * - AND stack assigned for task context is "size - 8KB"
 *       - 8KB chunk for the stack area is used for interrupt handling in this task context
 */
static uint8_t gTskStackMain[64*1024]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(8192)))
    ;

/* DRU configuration */
uint32_t gDruQoS_Enable    = 1;
uint32_t gQoS_DRU_Priority = 3;
uint32_t gQoS_DRU_OrderID  = 0;

void setup_dru_qos(void)
{
   uint64_t DRU_BASE = CSL_COMPUTE_CLUSTER0_MMR_DRU7_MMR_CFG_DRU_BASE;
   volatile uint64_t* queue0CFG     = (uint64_t*)(DRU_BASE + 0x8000);

   if(gQoS_DRU_Priority > 7 || (gDruQoS_Enable == 0))
   {
     gQoS_DRU_Priority = 0;
   }
   if(gQoS_DRU_OrderID > 15 || (gDruQoS_Enable == 0))
   {
     gQoS_DRU_OrderID = 0;
   }

   uint64_t queue0CFG_VAL = 0x0;
   queue0CFG_VAL |= ((uint64_t)gQoS_DRU_OrderID)<<4;
   queue0CFG_VAL |= ((uint64_t)gQoS_DRU_Priority);

   *queue0CFG = queue0CFG_VAL;
}

#define ENABLE_C7X_4_CLEC_INIT 1

#if ENABLE_C7X_4_CLEC_INIT
/* Important Note: The CLEC configuration in this file corresponds to the default UDMA
 * partitioning of events as found in pdk/packages/ti/drv/udma/soc/j784s4/udma_rmcfg.c
 * for the UDMA_RM_C7X_MSMC_DRU6.  Starting at event 12, the next 20 events are configured
 * for C7X-4, so enabling these CLEC events for C7X-4 here.  If the default UDMA event
 * partitioning is changed, this mapping of events will need a corresponding change */
static void appC7xClecInitDru(void)
{
    CSL_ClecEventConfig   cfgClec;
    CSL_CLEC_EVTRegs   *clecBaseAddr = (CSL_CLEC_EVTRegs*) CSL_COMPUTE_CLUSTER0_CLEC_REGS_BASE;

    uint32_t i;
    uint32_t dru_input_start = 664 + 96*C7X_CORE_ID + 12;
    uint32_t dru_input_num   = 20;
    /* program CLEC events from DRU used for polling by TIDL
     * to map to required events in C7x
     */
    for(i=dru_input_start; i<(dru_input_start+dru_input_num); i++)
    {
        /* Configure CLEC */
        cfgClec.secureClaimEnable = UFALSE;
        cfgClec.evtSendEnable     = UTRUE;

        /* cfgClec.rtMap value is different for each C7x */
        cfgClec.rtMap             = CSL_clecGetC7xRtmapCpuId();

        cfgClec.extEvtNum         = 0;
        cfgClec.c7xEvtNum         = (i-dru_input_start)+32;
        cfgClec.acDru             = 0;
        CSL_clecConfigEvent(clecBaseAddr, i, &cfgClec);
    }
}
#endif

int main(void)
{
    app_rtos_task_params_t tskParams;
    app_rtos_task_handle_t task;
    /* This is for debug purpose - see the description of function header */
    StartupEmulatorWaitFxn1();
    OS_init();

    #if ENABLE_C7X_4_CLEC_INIT
    appC7xClecInitDru();
    #endif

    setup_dru_qos();

    appPerfStatsInit();

    appRtosTaskParamsInit(&tskParams);
    tskParams.priority = 8u;
    tskParams.stack = gTskStackMain;
    tskParams.stacksize = sizeof (gTskStackMain);
    tskParams.taskfxn = &appMain;
    task = appRtosTaskCreate(&tskParams);
    if(NULL == task)
    {
        OS_stop();
    }
    OS_start();

    return 0;
}

uint32_t g_app_rtos_c7x_mmu_map_error = 0;

void appMmuMap(Bool is_secure)
{
    Bool            retVal;
    Mmu_MapAttrs    attrs;

    uint32_t ns = 1;

    if(is_secure)
        ns = 0;
    else
        ns = 1;

    Mmu_initMapAttrs(&attrs);

    attrs.attrIndx = Mmu_AttrIndx_MAIR0;
    attrs.ns = ns;

    retVal = Mmu_map(0x00000000U, 0x00000000U, 0x20000000U, &attrs, is_secure);
    if(retVal==UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(0x20000000U, 0x20000000U, 0x20000000U, &attrs, is_secure);
    if(retVal==UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(0x40000000U, 0x40000000U, 0x20000000U, &attrs, is_secure);
    if(retVal==UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(0x60000000U, 0x60000000U, 0x10000000U, &attrs, is_secure);
    if(retVal==UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(0x78000000U, 0x78000000U, 0x08000000U, &attrs, is_secure); /* CLEC */
    if(retVal==UFALSE)
    {
        goto mmu_exit;
    }
    
    /* DDR Local heap non cacheable */
    retVal = Mmu_map(DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_ADDR, DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_1_LOCAL_HEAP_NON_CACHEABLE_ADDR, DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_2_LOCAL_HEAP_NON_CACHEABLE_ADDR, DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_3_LOCAL_HEAP_NON_CACHEABLE_ADDR, DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    /* DDR scratch non cacheable */
    retVal = Mmu_map(DDR_C7X_4_SCRATCH_NON_CACHEABLE_ADDR, DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_4_SCRATCH_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_1_SCRATCH_NON_CACHEABLE_ADDR, DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_1_SCRATCH_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_2_SCRATCH_NON_CACHEABLE_ADDR, DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_2_SCRATCH_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_3_SCRATCH_NON_CACHEABLE_ADDR, DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_ADDR, DDR_C7X_3_SCRATCH_NON_CACHEABLE_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    /*-------------------------------------------------------*/
    /* Cachable region memory attributes                     */
    /*-------------------------------------------------------*/

    attrs.attrIndx = Mmu_AttrIndx_MAIR7;

    retVal = Mmu_map(0x80000000U, 0x80000000U, 0x20000000U, &attrs, is_secure); /* OCMC - 1MB */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(0xA0000000U, 0xA0000000U, 0x20000000U, &attrs, is_secure); /* OCMC - 1MB */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(0x70000000U, 0x70000000U, 0x00800000U, &attrs, is_secure); /* MSMC - 8MB */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(0x41C00000U, 0x41C00000U, 0x00100000U, &attrs, is_secure); /* OCMC - 1MB */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    /*The region mapped by the MMU is intentionally set to 2MB for L2 SRAM since
      page sizes are a function of the region size and having smaller page sizes
      negatively affects the performance of L2 SRAM as the table walks with the
      translation table in DDR are expensive, especially in context of high-
      throughput, low-latency memory like L2 SRAM*/
    retVal = Mmu_map(L2RAM_C7x_4_ADDR, L2RAM_C7x_4_ADDR, 0x00200000, &attrs, is_secure); /* L2 sram 448KB        */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7x_4_DTS_ADDR, DDR_C7x_4_DTS_ADDR, DDR_C7x_4_DTS_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    /* DDR Local heap */
    retVal = Mmu_map(DDR_C7X_4_LOCAL_HEAP_ADDR, DDR_C7X_4_LOCAL_HEAP_PHYS_ADDR, DDR_C7X_4_LOCAL_HEAP_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_1_LOCAL_HEAP_ADDR, DDR_C7X_1_LOCAL_HEAP_PHYS_ADDR, DDR_C7X_1_LOCAL_HEAP_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_2_LOCAL_HEAP_ADDR, DDR_C7X_2_LOCAL_HEAP_PHYS_ADDR, DDR_C7X_2_LOCAL_HEAP_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_3_LOCAL_HEAP_ADDR, DDR_C7X_3_LOCAL_HEAP_PHYS_ADDR, DDR_C7X_3_LOCAL_HEAP_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }
    
    /* DDR scratch */
    retVal = Mmu_map(DDR_C7X_4_SCRATCH_ADDR, DDR_C7X_4_SCRATCH_PHYS_ADDR, DDR_C7X_4_SCRATCH_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_1_SCRATCH_ADDR, DDR_C7X_1_SCRATCH_PHYS_ADDR, DDR_C7X_1_SCRATCH_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_2_SCRATCH_ADDR, DDR_C7X_2_SCRATCH_PHYS_ADDR, DDR_C7X_2_SCRATCH_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7X_4_3_SCRATCH_ADDR, DDR_C7X_3_SCRATCH_PHYS_ADDR, DDR_C7X_3_SCRATCH_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }
    
    /* DDR shared mem */
    retVal = Mmu_map(DDR_SHARED_MEM_ADDR, DDR_SHARED_MEM_PHYS_ADDR, DDR_SHARED_MEM_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(MSMC_C7x_4_ADDR, MSMC_C7x_4_ADDR, MSMC_C7x_4_SIZE, &attrs, is_secure); /* Local MSMC   */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    attrs.attrIndx = Mmu_AttrIndx_MAIR4;

    retVal = Mmu_map(APP_LOG_MEM_ADDR, APP_LOG_MEM_ADDR, APP_LOG_MEM_SIZE, &attrs, is_secure);
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(APP_FILEIO_MEM_ADDR, APP_FILEIO_MEM_ADDR, APP_FILEIO_MEM_SIZE, &attrs, is_secure);
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(TIOVX_OBJ_DESC_MEM_ADDR, TIOVX_OBJ_DESC_MEM_ADDR, TIOVX_OBJ_DESC_MEM_SIZE, &attrs, is_secure);
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(IPC_VRING_MEM_ADDR, IPC_VRING_MEM_ADDR, IPC_VRING_MEM_SIZE, &attrs, is_secure);
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(DDR_C7x_4_IPC_ADDR, DDR_C7x_4_IPC_ADDR, DDR_C7x_4_IPC_SIZE, &attrs, is_secure); /* ddr            */
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

    retVal = Mmu_map(TIOVX_LOG_RT_MEM_ADDR, TIOVX_LOG_RT_MEM_ADDR, TIOVX_LOG_RT_MEM_SIZE, &attrs, is_secure);
    if(retVal == UFALSE)
    {
        goto mmu_exit;
    }

mmu_exit:
    if(retVal == UFALSE)
    {
        g_app_rtos_c7x_mmu_map_error++;
    }



    return;
}

void appCacheInit()
{
    Cache_Size  cacheSize;

    /* init cache size here, since this needs to be done in secure mode */
    cacheSize.l1pSize = Cache_L1Size_32K;
    cacheSize.l1dSize = Cache_L1Size_32K;
    cacheSize.l2Size  = Cache_L2Size_64K;

    Cache_setSize(&cacheSize);
}

void InitMmu(void)
{
    /* This is for debug purpose - see the description of function header */
    StartupEmulatorWaitFxn();

    g_app_rtos_c7x_mmu_map_error = 0;

    appC7xClecInitForNonSecAccess();

    appMmuMap(UFALSE);
    appMmuMap(UTRUE);

    appCacheInit();
}

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
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_1_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_2_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_3_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_1_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_2_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_3_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_PHYS_SIZE, &phyAddr);
        
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_1_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_2_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_3_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &phyAddr);
        
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_1_SCRATCH_ADDR, 
            (uint64_t) DDR_C7X_1_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_2_SCRATCH_ADDR, 
            (uint64_t) DDR_C7X_2_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_3_SCRATCH_ADDR, 
            (uint64_t) DDR_C7X_3_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_PHYS_SIZE, &phyAddr);
        convertVirt2Phys(virtAddr, (uint64_t)DDR_C7X_4_SCRATCH_ADDR, 
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
    uint64_t target_ptr = shared_ptr;

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
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_1_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_2_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_3_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_1_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_1_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_1_LOCAL_HEAP_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_2_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_2_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_2_LOCAL_HEAP_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_3_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_3_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_3_LOCAL_HEAP_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_LOCAL_HEAP_ADDR, 
            (uint64_t) DDR_C7X_4_LOCAL_HEAP_PHYS_ADDR, (uint64_t)DDR_C7X_4_LOCAL_HEAP_PHYS_SIZE, &target_ptr);
        
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_1_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_2_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_3_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_SCRATCH_NON_CACHEABLE_ADDR, 
            (uint64_t) DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_ADDR, (uint64_t)DDR_C7X_4_SCRATCH_NON_CACHEABLE_PHYS_SIZE, &target_ptr);
        
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_1_SCRATCH_ADDR, 
            (uint64_t) DDR_C7X_1_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_1_SCRATCH_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_2_SCRATCH_ADDR, 
            (uint64_t) DDR_C7X_2_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_2_SCRATCH_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_3_SCRATCH_ADDR, 
            (uint64_t) DDR_C7X_3_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_3_SCRATCH_PHYS_SIZE, &target_ptr);
        convertPhys2Virt(shared_ptr, (uint64_t)DDR_C7X_4_SCRATCH_ADDR, 
            (uint64_t) DDR_C7X_4_SCRATCH_PHYS_ADDR, (uint64_t)DDR_C7X_4_SCRATCH_PHYS_SIZE, &target_ptr);
    }
    return target_ptr;
}

