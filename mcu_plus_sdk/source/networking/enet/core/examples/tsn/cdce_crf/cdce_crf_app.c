/*
 *  Copyright (C) 2026 Texas Instruments Incorporated
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

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <drivers/mcasp.h>
#include <drivers/pinmux.h>
#include <drivers/hw_include/cslr_soc.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/CacheP.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/SemaphoreP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "crf_task.h"
#include "clock_rec.h"
#include "tsninit.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* AVTP_CRF_TALKER (master) or AVTP_CRF_LISTENER (slave) is set per app in project_am275x.js. */
#ifdef AVTP_CRF_TALKER
#define CDCE_CRF_ROLE          CLOCK_REC_ROLE_TALKER
#else
#define CDCE_CRF_ROLE          CLOCK_REC_ROLE_LISTENER
#endif

#define CDCE_CRF_TASK_PRIORITY (8U)
#define CDCE_CRF_TS_INTERVAL   (160U)
#define CDCE_CRF_STATS_PERIOD_S (1U)

/* ========================================================================== */
/*                           Global Variables                                 */
/* ========================================================================== */

/* Stack for the CRF task, size and alignment come from the TSN config. */
static uint8_t CdceCrf_crfTaskStack[TSN_TSK_STACK_SIZE] \
    __attribute__((aligned(TSN_TSK_STACK_ALIGN)));

static SemaphoreP_Object CdceCrf_clockStartedSem;

static CrfTask_Params CdceCrf_taskParams = {
    .role                   = CDCE_CRF_ROLE,
    /* Clock source is the on-board CDCE clock generator. */
    .clkSrc                 = CLOCK_REC_SOURCE_CDCE,
    .correctionFlags        = CLOCK_REC_CORRECT_FREQUENCY | CLOCK_REC_CORRECT_PHASE,
    .parentClockFrequency   = 48000,
    .nominalFrequency       = 48000,
    .tsInterval             = CDCE_CRF_TS_INTERVAL,
    /* Audio and timestamp paths both use McASP4, one RX and one TX. */
    .audioMcaspBaseAddress  = CSL_MCASP4_CFG_BASE,
    .audioMcaspDir          = CLOCK_HW_MCASP_DIR_RX,
    .tsMcaspBaseAddress     = CSL_MCASP4_CFG_BASE,
    .tsMcaspDir             = CLOCK_HW_MCASP_DIR_TX,
    /* CPTS hardware push lines: mc for media clock, ts for timestamps. */
    .mcHwPush               = CPSW_CPTS_HWPUSH_1,
    .tsHwPush               = CPSW_CPTS_HWPUSH_2,
    .netdev                 = "tilld0",
    .streamId               = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00},
    .destMcast              = "91:E0:F0:00:FE:00",
    .joinMcast              = "91:E0:F0:00:FE:00",
    .vlanId                 = 110,
    .vlanPcp                = 2,
    .pull                   = AVBTP_CRF_PULL_MULT_1,
    /* Set this variable to true when Avtp Rx path is added */
    .skipRxRegistration     = false,
    .taskPriority           = CDCE_CRF_TASK_PRIORITY,
    .taskStack              = CdceCrf_crfTaskStack,
    .taskStackSize          = sizeof(CdceCrf_crfTaskStack),
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void EnetApp_AudioPlaybackDemoMain(void *args);
void CdceCrf_mcaspRxCb(MCASP_Handle handle, MCASP_Transaction *transaction);
void CdceCrf_tsMcaspTxCb(MCASP_Handle handle, MCASP_Transaction *transaction);

extern int32_t Board_codecConfig(void);
extern int32_t Board_MuxSelMcASP4(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void CdceCrf_setPinmux(void)
{
    Pinmux_PerCfg_t gPinMuxMainDomainCfg[] =
    {
        {
            PIN_GPIO1_72,
            ( PIN_MODE(1) | PIN_INPUT_ENABLE | PIN_PULL_DIRECTION  )
        },
        {
            PIN_EXT_REFCLK1,
            ( PIN_MODE(1) )
        },
        {PINMUX_END, 0U}
    };

    Pinmux_config(gPinMuxMainDomainCfg, PINMUX_DOMAIN_ID_MAIN);
}

static int32_t CdceCrf_startMcasp(void)
{
    int32_t status = SystemP_SUCCESS;

    gMcaspHandle[CONFIG_TS_MCASP] = MCASP_open(CONFIG_TS_MCASP, &gMcaspOpenParams[CONFIG_TS_MCASP]);
    if (NULL == gMcaspHandle[CONFIG_TS_MCASP])
    {
        DebugP_logError("MCASP open failed for CONFIG_TS_MCASP !!!\r\n");
        return SystemP_FAILURE;
    }

    /* Interrupts off so the TS and audio McASPs start back to back, keeping their frame syncs aligned. */
    uintptr_t key = HwiP_disable();
    MCASP_startTransferTx(gMcaspHandle[CONFIG_TS_MCASP]);
    MCASP_startTransferRx(gMcaspHandle[CONFIG_TS_MCASP]);
    HwiP_restore(key);

    return status;
}

static const char *CdceCrf_syncStatusName(ClockRec_SyncStatus status)
{
    switch (status)
    {
        case CLOCK_REC_SYNC_UNSYNCED: return "UNSYNCED";
        case CLOCK_REC_SYNC_ACQUIRING: return "ACQUIRING";
        case CLOCK_REC_SYNC_LOCKED: return "LOCKED";
        case CLOCK_REC_SYNC_STABLE: return "STABLE";
        default: return "UNKNOWN";
    }
}

static void CdceCrf_printStats(void)
{
    ClockRec_MeasuredParams measured;
    ClockRec_DiagCounters diag;
    ClockRec_SyncStatus status = ClockRec_getSyncStatus();

    (void)ClockRec_getMeasuredParams(&measured);
    (void)ClockRec_getDiagCounters(&diag);

    DebugP_log("CRF slave: sync=%s period_ns=%d tsMiss=%u edgeDisc=%u dcoDrop=%u "
               "lockSlip=%u courseCorr=%u fineCorr=%u submitRej=%u\r\n",
               CdceCrf_syncStatusName(status), (int32_t)measured.measuredPeriodNs,
               diag.tsMissCount, diag.edgeDiscontinuityCount, diag.dcoDroppedCount,
               diag.lockSlipCount, diag.courseCorrCount, diag.fineCorrCount,
               diag.submitRejectedCount);
}

int ClockRec_clockStartedHook(void)
{
    SemaphoreP_post(&CdceCrf_clockStartedSem);

    return 0;
}

/* Intentionally empty, this app does no per-timestamp work. */
void ClockRec_tsAvailableHook(void)
{
}

int ClockRec_needsRestartHook(void)
{
    (void)ClockRec_restart();

    return 0;
}

/* Audio RX callback is a placeholder, no samples consumed yet. */
void CdceCrf_mcaspRxCb(MCASP_Handle handle, MCASP_Transaction *transaction)
{
}

void CdceCrf_tsMcaspTxCb(MCASP_Handle handle, MCASP_Transaction *transaction)
{
}

void EnetApp_AudioPlaybackDemoMain(void *args)
{
    int32_t status = SystemP_SUCCESS;

    /* Pinmux must be set before McASP is opened. */
    CdceCrf_setPinmux();

    SemaphoreP_constructBinary(&CdceCrf_clockStartedSem, 0);

    status = Board_MuxSelMcASP4();
    DebugP_assert(status == SystemP_SUCCESS);


    /* gPTP daemon may still be starting, so poll until it answers. */
    while (gptpmasterclock_init(NULL))
    {
        DebugP_log("Waiting for tsn_gptpd to be initialzied...\n");
        ClockP_usleep(100000);
    }

    status = CrfTask_create(&CdceCrf_taskParams);
    DebugP_assert(status == SystemP_SUCCESS);

    /* Released once the clock recovery bootstrap starts, McASP must run before it can finish. */
    SemaphoreP_pend(&CdceCrf_clockStartedSem, SystemP_WAIT_FOREVER);
    DebugP_log("Clock Started...\r\n");

    status = CdceCrf_startMcasp();
    DebugP_assert(status == SystemP_SUCCESS);

    /* Only the slave prints stats, once per period. */
    while (1)
    {
    ClockP_sleep(CDCE_CRF_STATS_PERIOD_S);
#ifndef AVTP_CRF_TALKER
    CdceCrf_printStats();
#endif
    }
}
