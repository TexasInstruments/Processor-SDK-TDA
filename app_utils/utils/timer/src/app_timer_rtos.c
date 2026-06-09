/*
 *
 * Copyright (c) 2017-2026 Texas Instruments Incorporated
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

#include "app_timer_priv.h"
#include <HwiP.h>
#include <TimerP.h>
#include <utils/rtos/include/app_rtos.h>
#include <string.h>
#include "app_global_timer_priv.h"

#if defined(PDK)
#include <sciclient/sciclient.h>
#elif defined(MCU_PLUS_SDK)
#include <kernel/dpl/ClockP.h>
#include <drivers/device_manager/sciclient.h>
#define SCICLIENT_SERVICE_WAIT_FOREVER SystemP_WAIT_FOREVER
#elif defined(MCU_SDK)
#include <ClockP.h>
#define SCICLIENT_SERVICE_WAIT_FOREVER SystemP_WAIT_FOREVER
#endif


static uintptr_t GTC_BASE_ADDR = 0;
static uint64_t mhzFreq = 0;

#define GET_GTC_LO_VALUE (*(volatile uint32_t*)(GTC_BASE_ADDR + 0x8U))
#define GET_GTC_HI_VALUE (*(volatile uint32_t*)(GTC_BASE_ADDR + 0xCU))

/* JIRA: ADASVISION-7144 - Temporarily disabled, to be enabled once sci-client is enabled in mcu_sdk*/
#if defined(SOC_TDA54)
#define GTC_CLOCK_FREQ (200 * 1000 * 1000)
#endif

#ifdef APP_TIMER_USE_GLOBAL_TIME

static uint64_t appLogGetGtc(void);

static uint64_t appLogGetGtc(void)
{
    uint32_t vct_lo, vct_hi = 0, tmp_hi = 0;

    do
    {
        vct_hi = GET_GTC_HI_VALUE;
        vct_lo = GET_GTC_LO_VALUE;
        tmp_hi = GET_GTC_HI_VALUE;
    }
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> APP_UTILS_BRANCH_COVERAGE_TIMER_RTOS_UBR001
<justification end>*/
    while (vct_hi != tmp_hi);
/* LDRA_JUSTIFY_END */

    return ((uint64_t) vct_hi << 32) | vct_lo;
}

uint64_t appLogGetGlobalTimeInUsec(void)
{
    uint64_t cur_ts = 0; /* Returning ts in usecs */
    if ((0u != GTC_BASE_ADDR) &&
        (0u != mhzFreq))
    {
        cur_ts = appLogGetGtc() / mhzFreq;
    }

    return cur_ts;
}
#else
uint64_t appLogGetLocalTimeInUsec(void)
{
#if defined(PDK)
    return TimerP_getTimeInUsecs(); /* in units of usecs */
#elif defined(MCU_PLUS_SDK)
    return ClockP_getTimeUsec(); /* in units of usecs */
#endif
}
#endif

uint64_t appLogGetTimeInUsec(void)
{
    #ifdef APP_TIMER_USE_GLOBAL_TIME
    return appLogGetGlobalTimeInUsec();
    #else
    return appLogGetLocalTimeInUsec();
    #endif
}

/* LDRA_JUSTIFY
<metric start> statement branch <metric end>
<function start> int32_t appLogGlobalTimeInit.* <function end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_RTOS_UM01
<justification end> */
int32_t appLogGlobalTimeInit(void)
{
    int32_t status = 0;
    uint64_t clkFreq = 0;

    #ifdef C66
    GTC_BASE_ADDR = (uintptr_t)GTC_TIMER_MAPPED_BASE_C66;
    #else
    GTC_BASE_ADDR = (uintptr_t)GTC_TIMER_MAPPED_BASE;
    #endif

    #ifdef C66
    CSL_RatTranslationCfgInfo translationCfg;
    CSL_ratRegs *pGTCRatRegs = (CSL_ratRegs *)(CSL_C66_COREPAC_C66_RATCFG_BASE);

    translationCfg.sizeInBytes = GTC_TIMER_MAPPED_SIZE;
    translationCfg.baseAddress = GTC_BASE_ADDR;
    translationCfg.translatedAddress = CSL_GTC0_GTC_CFG1_BASE;

    if((bool)false == CSL_ratConfigRegionTranslation(pGTCRatRegs, 1, &translationCfg))
    {
        appLogPrintf("appLogGlobalTimeInit(): Error in CSL_ratConfigRegionTranslation()\n");
        status = -1;
    }
    #endif

    /* needs to be enabled only once, do it from R5F (TDA4) or M55 (TDA5) */
    #if defined(R5F) || defined(M55)
    /* Configure GTC Timer - running at 200MHz as per config and default mux mode */
    /* 200 MHz depends on 'MCU_PLL1' and is selected through 'GTCCLK_SEL' mux */
    /* Enable GTC */
    HW_WR_REG32((volatile uint32_t*)GTC_BASE_ADDR + 0x0U, 0x1);
    #endif

    if (0 == status)
    {
        #if defined(SOC_AM62A) || defined(SOC_J722S)
        status = Sciclient_pmGetModuleClkFreq(TISCI_DEV_WKUP_GTC0,
                                           TISCI_DEV_WKUP_GTC0_GTC_CLK,
                                           &clkFreq,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);
        /* JIRA: ADASVISION-7144 - Temporarily disabled, to be enabled once sci-client is enabled in mcu_sdk*/
        #elif defined(SOC_TDA54)
        clkFreq = GTC_CLOCK_FREQ;
        #else
        status = Sciclient_pmGetModuleClkFreq(TISCI_DEV_GTC0,
                                           TISCI_DEV_GTC0_GTC_CLK,
                                           &clkFreq,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);
        #endif

        if (0 == status)
        {
            mhzFreq = clkFreq / APP_TIMER_HZ_TO_MHZ;
        }
    }

    return status;
}

/* LDRA_JUSTIFY
<metric start> statement <metric end>
<function start> int32_t appLogGlobalTimeDeInit.* <function end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_RTOS_UM02
<justification end> */
int32_t appLogGlobalTimeDeInit(void)
{
    return 0;
}

void appLogWaitMsecs(uint32_t time_in_msecs)
{
#if defined(PC)
    ClockP_usleep((uint64_t)time_in_msecs * 1000U);
#else
    appRtosTaskSleepInMsecs(time_in_msecs);
#endif
}
