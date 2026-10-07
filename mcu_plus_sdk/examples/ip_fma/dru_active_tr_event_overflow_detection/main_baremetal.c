/*
 * Copyright (c) 2026 Texas Instruments Incorporated
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
 *  \file main_baremetal.c
 *
 *  \brief DRU overflow detection test for active UDMA TR events.
 *
 *  This application validates overflow detection by monitoring DRU CAUSE
 *  registers before and after issuing software triggers on an active
 *  UDMA channel. Channel-specific overflow is identified by mapping the
 *  UDMA channel number to the corresponding CAUSE register and bit field.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdbool.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <drivers/udma.h>
#include <drivers/soc.h>
#include <drivers/sciclient.h>
#include <drivers/hw_include/cslr64.h>
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< UDMA instance carrying the UTC/DRU instance information */
#define APP_DRU_UDMA_INST_ID            (UDMA_INST_ID_BCDMA_0)
/**< UTC instance under test - VPAC0 DRU on J722S R5F builds */
#define APP_DRU_UTC_ID                  (UDMA_UTC_ID_MSMC_DRU0)
/**< Device that must be clocked before the DRU MMRs can be accessed */
#define APP_DRU_TISCI_DEV_ID            (TISCI_DEV_VPAC0)
/**< Number of DRU CAUSE registers - CSL_dru_causeRegs_CAUSE.CAUSE[] */
#define APP_DRU_NUM_CAUSE_REG           (4U)
/**< Number of UDMA channels per CAUSE register */
#define APP_DRU_NUM_CH_IN_CAUSE_REG     (16U)
/**< Width in bits of the per channel slot inside a CAUSE register */
#define APP_DRU_CAUSE_CH_SLOT_WIDTH     (4U)
/**< Expected active-trigger overflow bit within a channel slot - T_ERR */
#define APP_DRU_CAUSE_CH_OVERFLOW_MASK  ((uint64_t) 0x2U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Enable the DRU host module clock so that its MMRs are accessible.
 *
 * \retval  UDMA_SOK    DRU register space is accessible.
 * \retval  UDMA_EFAIL  Module clock enable failed.
 */
static int32_t AppDru_enableModuleClock(void);

/**
 * \brief   Read all DRU CAUSE registers into the supplied array.
 *
 * \param   pDruRegs    DRU register overlay base.
 * \param   pCause      Array of #APP_DRU_NUM_CAUSE_REG entries to be filled.
 */
static void AppDru_readCauseRegs(const CSL_DRU_t *pDruRegs, uint64_t *pCause);

/**
 * \brief   Check that a channel has raised its overflow bits.
 *
 *          Each channel occupies a 4-bit slot within a CAUSE register. This
 *          test checks the T_ERR bit at bit 1 of that slot.
 *
 * \param   causeReg    CAUSE register holding the slot of the channel.
 * \param   chNum       Channel number to check.
 *
 * \retval  true        Overflow bit set for the channel.
 * \retval  false       No overflow detected.
 */
static bool AppDru_isChannelOverflowSet(uint64_t causeReg, uint32_t chNum);

/**
 * \brief   Log one CAUSE register value.
 *
 * \param   causeIdx    CAUSE register index.
 * \param   cause       CAUSE register value.
 */
static void AppDru_logCauseReg(uint32_t causeIdx, uint64_t cause);

/**
 * \brief   Validate DRU active TR event overflow detection using software
 *          trigger.
 *
 *          Opens a DRU channel in direct TR mode, verifies that all CAUSE
 *          registers are initially zero, issues two consecutive software
 *          triggers on the active channel and verifies that the T_ERR overflow
 *          bit for that channel is reported by the corresponding CAUSE register.
 *
 * \param   drvHandle   UDMA driver handle.
 * \param   chHandle    UDMA channel handle to be tested.
 * \param   pDruRegs    DRU register overlay base.
 *
 * \retval  UDMA_SOK    Expected overflow detected.
 * \retval  UDMA_EFAIL  Overflow not detected or validation failed.
 */
static int32_t AppDru_swTrigTest(Udma_DrvHandle drvHandle,
                                 Udma_ChHandle chHandle,
                                 const CSL_DRU_t *pDruRegs);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

Udma_DrvObject gUdmaDrvObj;
Udma_ChObject gUdmaChObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    int32_t status = SystemP_SUCCESS;
    int32_t retVal = UDMA_SOK;
    Udma_DrvHandle drvHandle = &gUdmaDrvObj;
    Udma_ChHandle chHandle = &gUdmaChObj;
    const CSL_DRU_t *pDruRegs = (const CSL_DRU_t *) UDMA_UTC_BASE_DRU0;
    Udma_InitPrms initPrms;
    bool initDone = false;

    /* --- */

    System_init();
    Board_init();

    Drivers_open();
    status = Board_driversOpen();
    DebugP_assert(status == SystemP_SUCCESS);

    DebugP_log("DRU Active TR Event Overflow Detection\r\n");

    /* --- */
    /* Enable the clock for the module containing the local DRU.
    In this application VPAC0 module is used, and can be accessed from
    the main core.
    */

    retVal = AppDru_enableModuleClock();

    /* --- */
    /* Initialize UDMA driver instance */

    if(UDMA_SOK == retVal)
    {
        /* 
        The DRU in the VPAC0 module is instanced as an UTC instance of 
        BCDMA_0 which is in the DMASS0 shared-DMA-engines module.
        The call to UdmaInitPrms_init will only initialize initPrms 
        structure instance with default field values.
        */
        retVal = UdmaInitPrms_init(APP_DRU_UDMA_INST_ID, &initPrms);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA init params init failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        /*
        Since the call to UdmaInitPrms_init initialized fields 
        with defualt values, we need to explicitly define UTC as
        enabled for this UDMA instance - this will allow us to
        use the VPAC DRU DMA engine.
        */
        initPrms.enableUtc = (uint8_t) TRUE;

        /*
        The call Udma_init will allow us to, from this point onwards,
        use UDMA driver handle for API calls in the context of BCDMA_0,
        which will allow us to configure DRU/UTC channels and use them.
        The exact DRU - the one inside VPAC0 hardware module - will
        be targeted later when using UDMA API calls.
        */
        retVal = Udma_init(drvHandle, &initPrms);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA init failed!!\r\n");
        }
        else
        {
            initDone = true;
        }
    }

    /* --- */
    /* Perform the actual test */
    
    if(UDMA_SOK == retVal)
    {
        retVal = AppDru_swTrigTest(drvHandle, chHandle, pDruRegs);
    }

    /* --- */

    if(initDone)
    {
        status = Udma_deinit(drvHandle);
        if(UDMA_SOK != status)
        {
            DebugP_log("[Error] UDMA deinit failed!!\r\n");

            if(UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
    }

    if(UDMA_SOK == retVal)
    {
        DebugP_log("All tests have passed!!\r\n");
    }
    else
    {
        DebugP_log("Some tests have failed!!\r\n");
    }

    Board_driversClose();
    Drivers_close();

    Board_deinit();
    System_deinit();

    return (retVal);
}

static int32_t AppDru_enableModuleClock(void)
{
    int32_t retVal = UDMA_SOK;
    int32_t status;

    status = SOC_moduleClockEnable(APP_DRU_TISCI_DEV_ID, 1U);
    if(SystemP_SUCCESS != status)
    {
        DebugP_log("[Error] DRU host module clock enable failed!!\r\n");
        retVal = UDMA_EFAIL;
    }

    return (retVal);
}

static void AppDru_readCauseRegs(const CSL_DRU_t *pDruRegs, uint64_t *pCause)
{
    uint32_t i;

    for(i = 0U; i < APP_DRU_NUM_CAUSE_REG; i++)
    {
        pCause[i] = CSL_REG64_RD(&pDruRegs->CAUSE.CAUSE.CAUSE[i]);
    }

    return;
}

static bool AppDru_isChannelOverflowSet(uint64_t causeReg, uint32_t chNum)
{
    uint32_t channelInCause = chNum % APP_DRU_NUM_CH_IN_CAUSE_REG;
    uint32_t bitBase = channelInCause * APP_DRU_CAUSE_CH_SLOT_WIDTH;
    uint64_t mask = APP_DRU_CAUSE_CH_OVERFLOW_MASK << bitBase;

    return ((causeReg & mask) != 0ULL);
}

static void AppDru_logCauseReg(uint32_t causeIdx, uint64_t cause)
{
    DebugP_log("  CAUSE[%u] = 0x%08x%08x\r\n",
               causeIdx,
               (uint32_t) (cause >> 32U),
               (uint32_t) cause);

    return;
}

static int32_t AppDru_swTrigTest(Udma_DrvHandle drvHandle,
                                 Udma_ChHandle chHandle,
                                 const CSL_DRU_t *pDruRegs)
{
    int32_t retVal = UDMA_SOK;
    uint32_t chType;
    Udma_ChPrms chPrms;
    Udma_ChUtcPrms utcPrms;
    uint32_t trigger = CSL_UDMAP_TR_FLAGS_TRIGGER_GLOBAL1;
    uint64_t causeRegs[APP_DRU_NUM_CAUSE_REG];
    bool allZero = true;
    bool chOpenDone = false;
    uint32_t chNum = 0U;
    uint32_t causeIdx;
    uint64_t cause;
    uint32_t i;
    int32_t cleanupStatus;

    /* 
    Initialize a channel in the VPAC0's DRU DMA engine.
    The channel is what DMA engine operates with - it can
    be opened, used, and closed. The UDMA driver needs
    the information about the channel in order to operate 
    on it - for example what type of channel it is and 
    which exact DRU will be using this channel. 
    */
    chType = UDMA_CH_TYPE_UTC;
    UdmaChPrms_init(&chPrms, chType);
    
    /*
    UTC ID must be configured before opening a channel. This
    will direct UDMA driver to operate a channel on the right
    DRU/UTC instance.
    */
    chPrms.utcId = APP_DRU_UTC_ID;
    retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
    if(UDMA_SOK != retVal)
    {
        DebugP_log("[Error] UDMA channel open failed!!\r\n");
    }
    else
    {
        chOpenDone = true;
    }

    if(UDMA_SOK == retVal)
    {
        chNum = Udma_chGetNum(chHandle);
        DebugP_log("Udma channel number %u\r\n", chNum);

        /*
        Initialize a structure instance that will be used to
        configure the exact UTC/DRU instance in VPAC0 module.
        */
        UdmaChUtcPrms_init(&utcPrms);

        /*
        Over-ride default filled values in order to instruct
        UDMA driver to configure DRU/UTC instance as directly
        controlled.
        */
        utcPrms.druOwner = CSL_DRU_OWNER_DIRECT_TR;
        utcPrms.druQueueId = CSL_DRU_QUEUE_ID_3;
        retVal = Udma_chConfigUtc(chHandle, &utcPrms);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA UTC channel config failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        retVal = Udma_chEnable(chHandle);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] UDMA channel enable failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        DebugP_log("Test core execution begin\r\n");

        /* Check if all CAUSE registers are zero initially */

        AppDru_readCauseRegs(pDruRegs, &causeRegs[0U]);

        for(i = 0U; i < APP_DRU_NUM_CAUSE_REG; i++)
        {
            if(causeRegs[i] != 0ULL)
            {
                allZero = false;
            }
        }

        if(allZero)
        {
            DebugP_log("Before swTrig writing check, all CAUSE registers are ZERO - Check passed\r\n");
        }
        else
        {
            DebugP_log("Before swTrig writing check FAILED - non-zero CAUSE detected\r\n");
            for(i = 0U; i < APP_DRU_NUM_CAUSE_REG; i++)
            {
                if(causeRegs[i] != 0ULL)
                {
                    AppDru_logCauseReg(i, causeRegs[i]);
                }
            }
            retVal = UDMA_EFAIL;
        }
    }

    /* 
    Perform the test action - send two triggers, but do not consume the first trigger
    by any Transfer Request. The DRU should detect the second trigger as an overflow 
    since there are two active triggers at the same time and raise an error bit.
    */

    if(UDMA_SOK == retVal)
    {
        retVal = Udma_chSetSwTrigger(chHandle, trigger);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] First Udma_chSetSwTrigger call failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        retVal = Udma_chSetSwTrigger(chHandle, trigger);
        if(UDMA_SOK != retVal)
        {
            DebugP_log("[Error] Second Udma_chSetSwTrigger call failed!!\r\n");
        }
    }

    if(UDMA_SOK == retVal)
    {
        AppDru_readCauseRegs(pDruRegs, &causeRegs[0U]);

        causeIdx = chNum / APP_DRU_NUM_CH_IN_CAUSE_REG;
        DebugP_assert(causeIdx < APP_DRU_NUM_CAUSE_REG);
        cause = causeRegs[causeIdx];

        if(AppDru_isChannelOverflowSet(cause, chNum))
        {
            DebugP_log("UDMA overflow detected on channel %u\r\n", chNum);
            AppDru_logCauseReg(causeIdx, cause);
        }
        else
        {
            DebugP_log("[Error] Expected UDMA overflow was not detected\r\n");
            AppDru_logCauseReg(causeIdx, cause);
            retVal = UDMA_EFAIL;
        }
    }

    if(chOpenDone)
    {
        cleanupStatus = Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
        if(UDMA_SOK != cleanupStatus)
        {
            DebugP_log("[Error] UDMA channel disable failed!!\r\n");
            if(UDMA_SOK == retVal)
            {
                retVal = cleanupStatus;
            }
        }

        cleanupStatus = Udma_chClose(chHandle);
        if(UDMA_SOK != cleanupStatus)
        {
            DebugP_log("[Error] UDMA channel close failed!!\r\n");
            if(UDMA_SOK == retVal)
            {
                retVal = cleanupStatus;
            }
        }
    }

    return (retVal);
}
