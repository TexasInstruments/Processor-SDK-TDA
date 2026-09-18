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
 * \file sciclient_s2r.c
 *
 * \brief This file contains the code which load DDR retention and PMIC S2R code
 * into SRAM
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <string.h>
#include <ti/csl/arch/csl_arch.h>
#include <osal.h>
#if defined(SOC_J7200)
#include <ti/csl/soc/j7200/src/cslr_soc.h>
#include <ti/board/src/j7200_evm/include/board_internal.h>
#endif
#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_soc.h>
#include <ti/board/src/j721s2_evm/include/board_internal.h>
#endif
#if defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_soc.h>
#include <ti/board/src/j784s4_evm/include/board_internal.h>
#endif
#if defined(SOC_J742S2)
#include <ti/csl/soc/j784s4/src/cslr_soc.h>
#include <ti/board/src/j742s2_evm/include/board_internal.h>
#endif
#include <ti/board/board.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/drv/lpm/include/lpm_stub.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define SCICLIENT_MSMC_CFGS0_CACHE_CTRL      (0x00001000U)

/* Address to point the sp register in R5F-BTCM */
#define SCICLIENT_STUB_SRAM_STACK_POINTER    (0x41014000U)

#define SCICLIENT_SAVE_STACK_PTR(sp)         asm volatile("mov sp, %0" : : "r" ((sp)))

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

uint32_t gLpmWakeReason = SCICLIENT_LPM_WAKE_SOURCE_INVALID;

/* ========================================================================== */
/*                         Function Declarations                              */
/* ========================================================================== */

/* Clear the data cache of R5 core, as we will be executing from BTCM */
static void Sciclient_s2rCleanAllDCache(void);

/* Clear the L3 cache in MSMC */
static bool Sciclient_s2rCleanL3Cache(void);

Sciclient_LpmData gSciclientLpmData = {
    .lpm_mode = 0xFFU,
    .suspend_initiator = 0xFFU,
    .lpm_rsvd1 = 0xFFFFU,
    .lpm_rsvd2 = 0xFFFFFFFFU,
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int32_t Sciclient_s2rEnableWkupI2c(void)
{
    /* The LPM entry sequence communicates with the PMIC over WKUP_I2C0 to
     * trigger IO retention and DDR retention.
     * Hence, make sure DM explicitly turns on the I2C instance
     * so that it can be used in the lpm_stub.
     */
    return Sciclient_pmSetModuleState(TISCI_DEV_WKUP_I2C0,
                                      TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
                                      TISCI_MSG_FLAG_AOP,
                                      SCICLIENT_SERVICE_WAIT_FOREVER);
}

int32_t Sciclient_getWakeReason(uint32_t *msg_recv)
{
    int32_t ret = CSL_PASS;
    uint32_t mode;
    struct tisci_msg_lpm_wake_reason_resp *resp = (struct tisci_msg_lpm_wake_reason_resp *) msg_recv;

    if (NULL == resp)
    {
        ret = CSL_EFAIL;
    }

    if (SCICLIENT_LPM_WAKE_SOURCE_INVALID == gLpmWakeReason)
    {
        ret = CSL_EFAIL;
    }

    if (ret == CSL_PASS)
    {
        resp->wake_source = gLpmWakeReason;

        if (SCICLIENT_LPM_WAKE_SOURCE_PMIC_GPIO == gLpmWakeReason)
        {
            mode = TISCI_MSG_VALUE_SLEEP_MODE_SOC_OFF;
        }
        else if ((TISCI_MSG_VALUE_LPM_WAKE_SOURCE_MCU_IO == gLpmWakeReason) ||
                 (TISCI_MSG_VALUE_LPM_WAKE_SOURCE_MAIN_IO == gLpmWakeReason))
        {
            mode = TISCI_MSG_VALUE_SLEEP_MODE_IO_ONLY_PLUS_DDR;
        }
        else
        {
            ret = CSL_EFAIL;
        }
    }

    if (ret == CSL_PASS)
    {
        resp->mode = mode;
        resp->wake_pin = TISCI_MSG_VALUE_LPM_WAKE_PIN_INVALID;
    }

    return ret;
}

int32_t Sciclient_getSuspendMaster(uint32_t *msg_recv)
{
    int32_t ret = CSL_PASS;
    struct tisci_msg_get_suspend_initiator_resp *resp =
        (struct tisci_msg_get_suspend_initiator_resp *) msg_recv;

    if (NULL == resp)
    {
        ret = CSL_EFAIL;
    }
    else
    {
        resp->current_initiator = gSciclientLpmData.suspend_initiator;
    }

    return ret;
}

static void Sciclient_s2rCleanAllDCache(void)
{
    uint32_t set, way;
    uint32_t numSets = CSL_armR5CacheGetNumSets();
    uint32_t numWays = CSL_armR5CacheGetNumWays();

    for (way = 0; way < numWays ; way++)
    {
        for (set = 0; set < numSets; set++)
        {
            CSL_armR5CacheCleanDcacheSetWay(set, way);
        }
    }
}

static bool Sciclient_s2rCleanL3Cache(void)
{
    uint64_t t;
    bool ret = true;

    /*
     * Do not flush the L3 cache if is not enabled, otherwise writing in
     * SCICLIENT_MSMC_CFGS0_CACHE_CTRL register hangs the SoC.
     */
    if (CSL_REG32_RD_OFF(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE, SCICLIENT_MSMC_CFGS0_CACHE_CTRL) == 0U)
        return ret;

    CSL_REG32_WR_OFF(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE, SCICLIENT_MSMC_CFGS0_CACHE_CTRL, 0U);

    t = TimerP_getTimeInUsecs();
    while (CSL_REG32_RD_OFF(CSL_COMPUTE_CLUSTER0_MSMC_CFGS0_BASE, SCICLIENT_MSMC_CFGS0_CACHE_CTRL) != 0U)
    {
	    /* timeout set to 100ms */
	    if ((TimerP_getTimeInUsecs() - t) > 100000U)
        {
            ret = false;
            break;
        }
    }

    return ret;
}

void Sciclient_goRetention(uint32_t mode)
{
    if (!Sciclient_s2rCleanL3Cache())
    {
        Sciclient_debugPrintf("Failed to clean L3 cache\n");
        Sciclient_pmDomainReset(DOMGRP_00, SCICLIENT_SERVICE_WAIT_FOREVER);
    }

    Sciclient_s2rCleanAllDCache();
    Sciclient_debugPrintf("Suspending\n");

    SCICLIENT_SAVE_STACK_PTR(SCICLIENT_STUB_SRAM_STACK_POINTER);
    Lpm_stubEnterRetention(mode);

    /* We never reach this point as we enter into low power mode
     * and we reload the DM during resume.
     */
}
