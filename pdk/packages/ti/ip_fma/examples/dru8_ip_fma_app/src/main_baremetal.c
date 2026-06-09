/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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

#include <stdio.h>
#include <ti/board/board.h>
#include <ti/drv/udma/udma.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/drv/udma/dmautils/udma_standalone/include/udma_ch.h>
#include <udma_test_soc.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< Maximum number of DRU CAUSE registers */
#define MAX_CAUSE_REG_NUMBER      (32U)
/**< Number of UDMA channels per CAUSE register */
#define NUM_CHANNELS_IN_CAUSE_REG (16U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Validate DRU active TR event overflow detection using software trigger.
 *
 *          This function performs an overflow detection check for the specified
 *          UDMA channel by examining the DRU CAUSE registers. It ensures that all
 *          CAUSE registers are cleared prior to issuing a software trigger and
 *          then verifies whether the corresponding overflow bits are set for the
 *          active channel after the trigger is generated.
 *
 *          The UDMA channel number is mapped to the appropriate CAUSE register
 *          and 4-bit slot, where the lower three bits indicate overflow events.
 *          This mapping allows precise identification of channel-specific
 *          overflow conditions.
 *
 *          The test is used to detect invalid or repeated software trigger
 *          operations that may result in active TR event overflow.
 *
 * \param   drvHandle   UDMA driver handle.
 * \param   chHandle    UDMA channel handle to be tested.
 *
 * \retval  UDMA_SOK    No overflow detected.
 * \retval  UDMA_EFAIL  Overflow detected or validation failed.
 */
static int32_t AppDru8_SwTrigTest(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle);

/**
 * \brief   Check that a channel has raised overflow bit
 *
 * \param   causeReg Cause register with corresponding overflow bit
 *
 * \param   chNum Channel number to check
 *
 * \retval  true   Overflow bit set for the channel.
 * \retval  false  No overflow detected.
 */
static bool AppDru8_IsChannelOverflowSet(uint64_t causeReg, uint16_t chNum);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/*
 * UDMA driver objects
 */
struct Udma_DrvObj gUdmaDrvObj;
struct Udma_ChObj gUdmaChObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/*
 * Application main
 */
int main(void)
{
    int32_t retVal = UDMA_SOK;
    int32_t status = UDMA_SOK;
    Udma_DrvHandle drvHandle = &gUdmaDrvObj;
    Udma_ChHandle chHandle = &gUdmaChObj;
    int32_t initDone = 0;

    Board_initCfg boardCfg;

    boardCfg = BOARD_INIT_PINMUX_CONFIG |
               BOARD_INIT_UART_STDIO;
    Board_init(boardCfg);

    Udma_InitPrms initPrms;
    uint32_t instId;

    /* Note: There is no external channel support in MCU NAVSS. So always use
     * main NAVSS even for MCU builds */
    /* UDMA driver init */
    instId = UDMA_INST_ID_MAIN_0;
    UdmaInitPrms_init(instId, &initPrms);
    initPrms.virtToPhyFxn = &Udma_defaultVirtToPhyFxn;
    initPrms.phyToVirtFxn = &Udma_defaultPhyToVirtFxn;

    retVal = Udma_init(drvHandle, &initPrms);
    if (UDMA_SOK != retVal)
    {
        UART_printf("[Error] UDMA init failed!!\n");
    }
    else
    {
        initDone = 1;

        retVal = AppDru8_SwTrigTest(drvHandle, chHandle);
    }

    if (initDone)
    {
        status = Udma_deinit(drvHandle);
        if (UDMA_SOK != status)
        {
            UART_printf("[Error] UDMA deinit failed!!\n");

            if (UDMA_SOK == retVal)
            {
                retVal = status;
            }
        }
    }

    if(UDMA_SOK == retVal)
    {
        UART_printf("All tests have passed!!\r\n");
    }
    else
    {
        UART_printf("Some tests have failed!!\r\n");
    }

    return retVal;
}

static bool AppDru8_IsChannelOverflowSet(uint64_t causeReg, uint16_t chNum)
{
    uint32_t channelInCause = chNum % NUM_CHANNELS_IN_CAUSE_REG;
    uint32_t bitBase = channelInCause * 4;
    uint64_t mask = (uint64_t)0x7 << bitBase;

    return ((causeReg & mask) != 0ULL);
}

static int32_t AppDru8_SwTrigTest(Udma_DrvHandle drvHandle, Udma_ChHandle chHandle)
{
    int32_t retVal = UDMA_SOK;
    uint32_t chType;
    Udma_ChPrms chPrms;
    uint32_t trigger;
    uint64_t causeRegsBefore[MAX_CAUSE_REG_NUMBER];

    Udma_ChUtcPrms utcPrms;
    bool allZero = true;
    uint16_t chNum;
    uint32_t causeIdx;
    uint64_t cause;
    CSL_DRU_CAUSE druCause;
    uint32_t i;

    if (UDMA_SOK == retVal)
    {
        /* Init channel parameters */
        chType = UDMA_CH_TYPE_UTC;
        UdmaChPrms_init(&chPrms, chType);
        chPrms.utcId = UDMA_UTC_ID_MSMC_DRU0;
        chPrms.peerChNum = UDMA_TEST_MAIN_PEER_CH_NUM_TX;
        retVal = Udma_chOpen(drvHandle, chHandle, chType, &chPrms);
        trigger = CSL_UDMAP_TR_FLAGS_TRIGGER_GLOBAL1;
        UdmaChUtcPrms_init(&utcPrms);
        utcPrms.druOwner = CSL_DRU_OWNER_DIRECT_TR;
        utcPrms.druQueueId = CSL_DRU_QUEUE_ID_3;
        if (UDMA_SOK != retVal)
        {
            UART_printf("[Error] UDMA channel open failed!!\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        UART_printf("Udma channel number %d\n", Udma_chGetNum(chHandle));
    }

    if (UDMA_SOK == retVal)
    {
        retVal = Udma_chConfigUtc(chHandle, &utcPrms);
    }

    if (UDMA_SOK == retVal)
    {
        /* Channel enable */
        retVal = Udma_chEnable(chHandle);
        if (UDMA_SOK != retVal)
        {
            UART_printf("[Error] UDMA channel enable failed!!\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        retVal = CSL_druGetCauseRegs((const CSL_DRU_t *)drvHandle->utcInfo[chPrms.utcId].druRegs, &druCause);
        if (CSL_PASS != retVal)
        {
            UART_printf("[Error] Failed to read DRU CAUSE registers!!\r\n");
        }
    }

    if (UDMA_SOK == retVal)
    {
        UART_printf("Test core execution begin\n");
        for (i = 0U; i < MAX_CAUSE_REG_NUMBER; i++)
        {
            causeRegsBefore[i] = druCause.CAUSE[i];

            if (causeRegsBefore[i] != 0ULL)
            {
                allZero = false;
            }
        }

        if (allZero)
        {
            UART_printf("Before swTrig writing check, all CAUSE registers are ZERO - Check passed\r\n");
        }
        else
        {
            UART_printf("Before swTrig writing check FAILED - non-zero CAUSE detected\r\n");
            for (i = 0U; i < MAX_CAUSE_REG_NUMBER; i++)
            {
                if (causeRegsBefore[i] != 0ULL)
                {
                    UART_printf("  CAUSE[%d] = 0x%08X%08X\r\n", i, (uint32_t)(causeRegsBefore[i] >> 32), (uint32_t)(causeRegsBefore[i]));
                }
            }
            retVal = UDMA_EFAIL;
        }

        if (UDMA_SOK == retVal)
        {
            retVal = Udma_chSetSwTrigger(chHandle, trigger);
            if (retVal != UDMA_SOK)
            {
                UART_printf("[Error] First Udma_chSetSwTrigger call failed!!\n");
            }
        }

        if (UDMA_SOK == retVal)
        {
            retVal = Udma_chSetSwTrigger(chHandle, trigger);
            if (retVal != UDMA_SOK)
            {
                UART_printf("[Error] Second Udma_chSetSwTrigger call failed!!\n");
            }
        }

        if (UDMA_SOK == retVal)
        {
            retVal = CSL_druGetCauseRegs((const CSL_DRU_t *)drvHandle->utcInfo[chPrms.utcId].druRegs, &druCause);
            if (CSL_PASS != retVal)
            {
                UART_printf("[Error] Failed to read DRU CAUSE registers!!\r\n");
            }
        }

        if (UDMA_SOK == retVal)
        {
            chNum = Udma_chGetNum(chHandle);
            causeIdx = chNum / NUM_CHANNELS_IN_CAUSE_REG;
            cause = druCause.CAUSE[causeIdx];

            if (AppDru8_IsChannelOverflowSet(cause, chNum))
            {
                UART_printf(
                    "UDMA overflow detected on channel %u (CAUSE[%u]=0x%08X%08X)\r\n",
                    chNum,
                    causeIdx,
                    (uint32_t)(cause >> 32),
                    (uint32_t)cause);
            }
            else
            {
                UART_printf("[Error] Expected UDMA overflow was not detected "
                            "(CAUSE[%u]=0x%08X%08X)\r\n",
                            causeIdx,
                            (uint32_t)(cause >> 32),
                            (uint32_t)cause);

                retVal = UDMA_EFAIL;
            }
        }
    }

    if (chHandle != NULL)
    {
        Udma_chDisable(chHandle, UDMA_DEFAULT_CH_DISABLE_TIMEOUT);
        Udma_chClose(chHandle);
    }

    return (retVal);
}
