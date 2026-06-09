/*
 *
 * Copyright (c) 2019-2026 Texas Instruments Incorporated
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
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#if !defined(SOC_TDA54)
#include <sciclient/sciclient.h>
#endif

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
#include <time.h>   /* for nanosleep */
int nanosleep(const struct timespec *req, struct timespec *rem);
#else
#include <unistd.h> /* for usleep */
extern int usleep (useconds_t __useconds);
#endif
#include "app_timer_priv.h"
#include <inttypes.h>

#undef APP_TIMER_DEBUG

static void *GTC_BASE_ADDR = NULL;
static uint64_t mhzFreq = (uint64_t)0U;

#define GET_GTC_LO_VALUE (*(volatile uint32_t*)(GTC_BASE_ADDR + 0x8U))
#define GET_GTC_HI_VALUE (*(volatile uint32_t*)(GTC_BASE_ADDR + 0xCU))

/* Temporarily disabled, to be enabled once sci-client is enabled in mcu_sdk*/
#if defined(SOC_TDA54)
#define GTC_CLOCK_FREQ (200 * 1000 * 1000)
#endif

#define PRI_MAX  sched_get_priority_max(SCHED_FIFO)
#define PRI_MIN  sched_get_priority_min(SCHED_FIFO)

#define ONE_BILLION ((uint64_t) 1000000000U)

int32_t appLogGlobalTimeInit(void)
{
    int32_t status = (int32_t)0;
    uint64_t clkFreq;

    if(NULL == GTC_BASE_ADDR)
    {
        GTC_BASE_ADDR = appMemMap((void*)GTC_TIMER_MAPPED_BASE, (uint32_t)GTC_TIMER_MAPPED_SIZE);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM01
<justification end> */
        if(GTC_BASE_ADDR==NULL)
        {
            appLogPrintf("APP_LOG: ERROR: Unable to mmap gtc (%p of %d bytes) !!!\n",
                        (void*)GTC_TIMER_MAPPED_BASE, GTC_TIMER_MAPPED_SIZE );
            status = (int32_t)-1;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM01
<justification end> */
    if (0 == status)
    {
        #if defined(SOC_J722S) || defined(SOC_AM62A)
        status = Sciclient_pmGetModuleClkFreq(TISCI_DEV_WKUP_GTC0,
                                           TISCI_DEV_WKUP_GTC0_GTC_CLK,
                                           &clkFreq,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);

        /* Temporarily disabled, to be enabled once sci-client is enabled in mcu_sdk*/
        #elif defined(SOC_TDA54)
        clkFreq = GTC_CLOCK_FREQ;
        #else
        status = Sciclient_pmGetModuleClkFreq(TISCI_DEV_GTC0,
                                           TISCI_DEV_GTC0_GTC_CLK,
                                           &clkFreq,
                                           SCICLIENT_SERVICE_WAIT_FOREVER);
        #endif
    }
/* LDRA_JUSTIFY_END */

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM03
<justification end> */
        if (0 == status)
        {
            mhzFreq = clkFreq / APP_TIMER_HZ_TO_MHZ;
        }
/* LDRA_JUSTIFY_END */
    }
    else
    {
        appLogPrintf("APP_TIMER has already been initialized.\n");
    }

    return status;
}

int32_t appLogGlobalTimeDeInit(void)
{
    int32_t status = (int32_t)0;

/* LDRA_JUSTIFY_START
<metric start> branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM04
<justification end> */
    if (NULL != GTC_BASE_ADDR)
/* LDRA_JUSTIFY_END */
    {
        status = appMemUnMap((void*)GTC_BASE_ADDR, GTC_TIMER_MAPPED_SIZE);

        GTC_BASE_ADDR = NULL;
    }

    return status;
}

void appLogPrintGtcFreq(void)
{
    appLogPrintf("GTC Frequency = %" PRIu64 " MHz\n", mhzFreq);
}

#ifdef APP_TIMER_USE_GLOBAL_TIME

static uint64_t appLogGetGtc(void);

static uint64_t appLogGetGtc(void)
{
    uint32_t vct_lo, vct_hi, tmp_hi;

    do
    {
        vct_hi = GET_GTC_HI_VALUE;
        vct_lo = GET_GTC_LO_VALUE;
        tmp_hi = GET_GTC_HI_VALUE;
    }
    while (vct_hi != tmp_hi); /* TIOVX-2018- LDRA Uncovered Branch Id: TIOVX_CODE_COVERAGE_TIMER_QNX_UM05 */

    return ((uint64_t) vct_hi << 32) | vct_lo;
}

uint64_t appLogGetGlobalTimeInUsec(void)
{
    uint64_t cur_ts = 0; /* Returning ts in usecs */

    if ((NULL != GTC_BASE_ADDR) && /* TIOVX-2018- LDRA Uncovered Branch Id: TIOVX_CODE_COVERAGE_TIMER_QNX_UM04 */
        (0U != mhzFreq) ) /* TIOVX-2018- LDRA Uncovered Branch Id: TIOVX_CODE_COVERAGE_TIMER_QNX_UM06 */
    {
        cur_ts = appLogGetGtc() / mhzFreq;
    }

    return cur_ts;
}
#else
uint64_t appLogGetLocalTimeInUsec(void)
{
    uint64_t timeInUsecs = 0U;
    struct timespec currentTime;
    static uint64_t g_start_time = 0U;

    if( clock_gettime( CLOCK_MONOTONIC, &currentTime) == -1 ) {
        perror( "clock gettime" );
        return EXIT_FAILURE;
    }

    timeInUsecs = ((((uint64_t) currentTime.tv_sec * ONE_BILLION) + (uint64_t) currentTime.tv_nsec) / 1000);
    if(g_start_time==0)
    {
        g_start_time = timeInUsecs;
    }
    return (timeInUsecs-g_start_time);
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

void appLogWaitMsecs(uint32_t time_in_msecs)
{
#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
    struct timespec delay_time, remain_time;
    int ret;

    delay_time.tv_sec  = time_in_msecs/1000;
    delay_time.tv_nsec = (time_in_msecs%1000)*1000000;

    do
    {
        ret = nanosleep(&delay_time, &remain_time);
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM02
<justification end> */
        if(ret < 0 && remain_time.tv_sec > 0 && remain_time.tv_nsec > 0)
        {
            /* restart for remaining time */
            delay_time = remain_time;
        }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM02
<justification end> */
        else
/* LDRA_JUSTIFY_END */
        {
            break;
        }
/* LDRA_JUSTIFY_START
<metric start> statement <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM02
<justification end> */
    }
/* LDRA_JUSTIFY_END */
/* LDRA_JUSTIFY_START
<metric start> statement branch <metric end>
<justification start> TIOVX_CODE_COVERAGE_TIMER_QNX_UM02
<justification end> */
    while(1);
/* LDRA_JUSTIFY_END */
#else
    (void)usleep(time_in_msecs * 1000U);
#endif
}
