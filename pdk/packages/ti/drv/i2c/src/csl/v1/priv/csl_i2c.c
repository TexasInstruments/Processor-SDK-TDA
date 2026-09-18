/*
 *  Copyright (C) 2024 Texas Instruments Incorporated -www.ti.com/
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
 *
 */

/*
 *  \file   csl_i2c.c
 *
 *  \brief  This file contains the device abstraction layer API implementation
 *          corresponding to I2C module.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "stdint.h"
#include "ti/drv/i2c/src/csl/v1/cslr_i2c.h"
#include "ti/drv/i2c/src/csl/v1/csl_i2c.h"

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

#define I2C_MAX_CLK_PRESCALAR    (255U)

#define I2C_INTERNAL_CLK_STEP    (1000U)

#define I2C_FS_MODE_FREQ         (400000U)  /* F/S mode maximum frequency (per I2C spec) */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void I2CMasterInitExpClk(uintptr_t baseAddr,
                         uint32_t  sysClk,
                         uint32_t  internalClk,
                         uint32_t  outputClk)
{
    uint32_t prescalar;
    uint32_t divisor;
    uint32_t div_h, div_l;
    uint32_t actIntClk = 0U;

    /* Iterate through the valid pre-scalar values until one is found that is
     * the closest match to the desired internal clock speed
     */
    for (prescalar = 0U; prescalar < I2C_MAX_CLK_PRESCALAR; prescalar++)
    {
        /* Calculate the actual speed that would be used for the current
         * pre-scalar value, and if it is within 1 KHz of the desired value then
         * we have a match
         */
        actIntClk = sysClk / (prescalar + 1U);

        if (actIntClk <= (internalClk + I2C_INTERNAL_CLK_STEP))
        {
            break;
        }
    }

    if (outputClk > I2C_FS_MODE_FREQ)
    {
        /* Prescalar bypassed for frequencies > 400 kHz */
        prescalar = 0U;
        actIntClk = sysClk;
    }
    CSL_REG32_WR(baseAddr + CSL_I2C_PSC, prescalar);

    /* Calculate the divisor to be used based on actual internal clock speed
     * based on pre-scalar value used
     */
    divisor = actIntClk / outputClk;

    if ((outputClk * divisor) != actIntClk)
    {
        /* Round up the divisor so that output clock never exceeds the
         * requested value if it is not exact
         */
        divisor += 1U;
    }
    /* Split into SCK HIGH and LOW time to take odd numbered divisors
     * into account and avoid reducing the output clock frequency
     */
    div_h = divisor / 2U;
    div_l = divisor - div_h;

    if (outputClk > I2C_FS_MODE_FREQ)
    {
        /* HS mode (> 400 kHz): Two-phase transmission
         * Phase 1 (F/S, 400 kHz): Address/control in bits [7:0]
         * Phase 2 (HS, target freq): Data in bits [15:8]
         * Applies to both 1 MHz and 3.4 MHz (per TRM specification)
         */
        uint32_t divisor_fs;
        uint32_t div_h_fs, div_l_fs;
        uint32_t scll_val, sclh_val;

        /* Calculate F/S phase divisor for 400 kHz */
        divisor_fs = (actIntClk / I2C_FS_MODE_FREQ);
        div_h_fs = divisor_fs / 2U;
        div_l_fs = divisor_fs - div_h_fs;

        /* Combine: HS phase in [15:8], F/S phase in [7:0] */
        scll_val = ((div_l - 7U) << 8U) | (div_l_fs - 7U);
        sclh_val = ((div_h - 5U) << 8U) | (div_h_fs - 5U);

        CSL_REG32_WR(baseAddr + CSL_I2C_SCLL, scll_val);
        CSL_REG32_WR(baseAddr + CSL_I2C_SCLH, sclh_val);
    }
    else
    {
        /* Standard mode or Fast mode (≤ 400 kHz)
         * Write to bits [7:0] only (F/S phase registers)
         */
        CSL_REG32_WR(baseAddr + CSL_I2C_SCLL, div_l - 7U);
        CSL_REG32_WR(baseAddr + CSL_I2C_SCLH, div_h - 5U);
    }
}

void I2CMasterEnable(uintptr_t baseAddr)
{
    /* Bring the I2C module out of reset */
    CSL_REG32_FINS(baseAddr + CSL_I2C_CON, I2C_CON_I2C_EN,
                   CSL_I2C_CON_I2C_EN_ENABLE);
}

void I2CMasterEnableFreeRun(uintptr_t baseAddr)
{
    /* Set the I2C module in free running mode */
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSTEST, I2C_SYSTEST_FREE,
                   CSL_I2C_SYSTEST_FREE_FREE);
}

void I2CMasterSetSysTest(uintptr_t baseAddr, uint32_t sysTest)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_SYSTEST, sysTest);
}

uint32_t I2CMasterGetSysTest(uintptr_t baseAddr)
{
    uint32_t sysTest;

    sysTest = CSL_REG32_RD(baseAddr + CSL_I2C_SYSTEST);

    return(sysTest);
}

void I2CMasterDisable(uintptr_t baseAddr)
{
    /* Put I2C module in reset */
    CSL_REG32_FINS(baseAddr + CSL_I2C_CON, I2C_CON_I2C_EN,
                   CSL_I2C_CON_I2C_EN_DISABLE);
}

bool I2CMasterBusBusy(uintptr_t baseAddr)
{
    bool status;

    if (CSL_I2C_IRQSTATUS_RAW_BB_SET == CSL_REG32_FEXT(baseAddr + CSL_I2C_IRQSTATUS_RAW,
                       I2C_IRQSTATUS_RAW_BB))
    {
        status = BTRUE;
    }
    else
    {
        status = BFALSE;
    }
    return(status);
}

uint32_t I2CMasterBusy(uintptr_t baseAddr)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_CON) & CSL_I2C_CON_MST_MASK);
}

uint32_t I2CMasterErr(uintptr_t baseAddr)
{
    uint32_t err;
    uint32_t maskOr;
    
    maskOr = (CSL_I2C_IRQSTATUS_RAW_AL_MASK | CSL_I2C_IRQSTATUS_RAW_AERR_MASK |
              CSL_I2C_IRQSTATUS_RAW_NACK_MASK | CSL_I2C_IRQSTATUS_RAW_ROVR_MASK);
    
    err = CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS_RAW) & maskOr;

    return(err);
}

void I2CMasterControl(uintptr_t baseAddr, uint32_t cmd)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_CON, cmd | CSL_I2C_CON_I2C_EN_MASK);
}

void I2CMasterStart(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_CON, I2C_CON_STT, CSL_I2C_CON_STT_STT);
}

void I2CMasterStop(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_CON, I2C_CON_STP, CSL_I2C_CON_STP_STP);
}

void I2CMasterIntEnableEx(uintptr_t baseAddr, uint32_t intFlag)
{
    uint32_t i2cRegValue = CSL_REG32_RD(baseAddr + CSL_I2C_IRQENABLE_SET);

    i2cRegValue |= intFlag;
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQENABLE_SET, i2cRegValue);
}

void I2CMasterIntDisableEx(uintptr_t baseAddr, uint32_t intFlag)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQENABLE_CLR, intFlag);
}

uint32_t I2CMasterIntStatus(uintptr_t baseAddr)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS));
}

uint32_t I2CMasterIntStatusEx(uintptr_t baseAddr, uint32_t intFlag)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS) & intFlag);
}

uint32_t I2CMasterIntRawStatus(uintptr_t baseAddr)
{
    return(CSL_REG32_RD((baseAddr + CSL_I2C_IRQSTATUS_RAW)));
}

uint32_t I2CMasterIntRawStatusEx(uintptr_t baseAddr, uint32_t intFlag)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS_RAW) & intFlag);
}

void I2CMasterIntClearEx(uintptr_t baseAddr, uint32_t intFlag)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQSTATUS, intFlag);
}

void I2CMasterIntRawStatusClearEx(uintptr_t baseAddr, uint32_t intFlag)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQSTATUS_RAW, intFlag);
}

uint32_t I2CGetEnabledIntStatus(uintptr_t baseAddr, uint32_t intFlag)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQENABLE_SET) & intFlag);
}

void I2CMasterSlaveAddrSet(uintptr_t baseAddr, uint32_t slaveAdd)
{
    /*Set the address of the slave with which the master will communicate.*/
    CSL_REG32_WR(baseAddr + CSL_I2C_SA, slaveAdd);
}

void I2CSetDataCount(uintptr_t baseAddr, uint32_t count)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_CNT, count);
}

uint32_t I2CDataCountGet(uintptr_t baseAddr)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_CNT));
}

void I2CFIFOThresholdConfig(uintptr_t baseAddr,
                            uint32_t  thresholdVal,
                            uint32_t  flag)
{
    if (CSL_I2C_TX_MODE == flag)
    {
        CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_TXTRSH, thresholdVal);
    }
    else
    {
        CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_RXTRSH, thresholdVal);
    }
}

void I2CFIFOClear(uintptr_t baseAddr, uint32_t flag)
{
    if (CSL_I2C_TX_MODE == flag)
    {
        CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_TXFIFO_CLR,
                       CSL_I2C_BUF_TXFIFO_CLR_RSTMODE);
    }
    else
    {
        CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_RXFIFO_CLR,
                       CSL_I2C_BUF_RXFIFO_CLR_RSTMODE);
    }
}

#if (1U == CSL_I2C_ENABLE_SLAVE_MODE)
void I2CSlaveIntEnableEx(uintptr_t baseAddr, uint32_t intFlag)
{
    uint32_t i2cRegValue = CSL_REG32_RD(baseAddr + CSL_I2C_IRQENABLE_SET);

    i2cRegValue |= intFlag;
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQENABLE_SET, i2cRegValue);
}

void I2CSlaveIntDisableEx(uintptr_t baseAddr, uint32_t intFlag)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQENABLE_CLR, intFlag);
}

uint32_t I2CSlaveIntStatus(uintptr_t baseAddr)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS));
}

uint32_t I2CSlaveIntStatusEx(uintptr_t baseAddr, uint32_t intFlag)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS) & intFlag);
}

uint32_t I2CSlaveIntRawStatus(uintptr_t baseAddr)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS_RAW));
}

uint32_t I2CSlaveIntRawStatusEx(uintptr_t baseAddr, uint32_t intFlag)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_IRQSTATUS_RAW) & intFlag);
}

void I2CSlaveIntClearEx(uintptr_t baseAddr, uint32_t intFlag)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQSTATUS, intFlag);
}

void I2CSlaveIntRawStatusClearEx(uintptr_t baseAddr, uint32_t intFlag)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_IRQSTATUS_RAW, intFlag);
}

uint32_t I2CActiveOwnAddressGet(uintptr_t baseAddr)
{
    uint32_t ownAddr;

    if (CSL_I2C_ACTOA_OA0_ACT_ACTIVE == CSL_REG32_FEXT(baseAddr + CSL_I2C_ACTOA,
                       I2C_ACTOA_OA0_ACT))
    {
        ownAddr = CSL_REG32_RD(baseAddr + CSL_I2C_OA);
    }
    else if (CSL_I2C_ACTOA_OA1_ACT_ACTIVE == CSL_REG32_FEXT(baseAddr + CSL_I2C_ACTOA,
                            I2C_ACTOA_OA1_ACT))
    {
        ownAddr = CSL_REG32_RD(baseAddr + CSL_I2C_OA1);
    }
    else if (CSL_I2C_ACTOA_OA2_ACT_ACTIVE == CSL_REG32_FEXT(baseAddr + CSL_I2C_ACTOA,
                            I2C_ACTOA_OA2_ACT))
    {
        ownAddr = CSL_REG32_RD(baseAddr + CSL_I2C_OA2);
    }
    else if (CSL_I2C_ACTOA_OA3_ACT_ACTIVE == CSL_REG32_FEXT(baseAddr + CSL_I2C_ACTOA,
                            I2C_ACTOA_OA3_ACT))
    {
        ownAddr = CSL_REG32_RD(baseAddr + CSL_I2C_OA3);
    }
    else
    {
        /*
         * Do nothing
         */
        ownAddr = 0U;
    }
    return(ownAddr);
}

void I2COwnAddressSet(uintptr_t baseAddr, uint32_t ownAdd,
                      uint32_t flag)
{
    switch (flag)
    {
        case CSL_I2C_OWN_ADDR_0:
            CSL_REG32_WR(baseAddr + CSL_I2C_OA, ownAdd);
            break;

        case CSL_I2C_OWN_ADDR_1:
            CSL_REG32_WR(baseAddr + CSL_I2C_OA1, ownAdd);
            break;

        case CSL_I2C_OWN_ADDR_2:
            CSL_REG32_WR(baseAddr + CSL_I2C_OA2, ownAdd);
            break;

        case CSL_I2C_OWN_ADDR_3:
            CSL_REG32_WR(baseAddr + CSL_I2C_OA3, ownAdd);
            break;

        default:
            /* Invalid input */
            break;
    }
}

uint32_t I2CMasterSlaveAddrGet(uintptr_t baseAddr)
{
    /* Returns the address of the slave with which the master will
     * communicate.*/
    return(CSL_REG32_RD(baseAddr + CSL_I2C_SA) & CSL_I2C_SA_SA_MASK);
}

void I2CClockBlockingControl(uintptr_t baseAddr,
                             uint32_t  ownAdd0,
                             uint32_t  ownAdd1,
                             uint32_t  ownAdd2,
                             uint32_t  ownAdd3)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SBLOCK, I2C_SBLOCK_OA0_EN, ownAdd0);

    CSL_REG32_FINS(baseAddr + CSL_I2C_SBLOCK, I2C_SBLOCK_OA1_EN, ownAdd1);

    CSL_REG32_FINS(baseAddr + CSL_I2C_SBLOCK, I2C_SBLOCK_OA2_EN, ownAdd2);

    CSL_REG32_FINS(baseAddr + CSL_I2C_SBLOCK, I2C_SBLOCK_OA3_EN, ownAdd3);
}

void I2CSlaveDataPut(uintptr_t baseAddr, uint32_t data)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_DATA, data);
}

uint8_t I2CSlaveDataGet(uintptr_t baseAddr)
{
    uint8_t rData;

    rData = (uint8_t)CSL_REG32_RD(baseAddr + CSL_I2C_DATA);

    return(rData);
}
#endif

#if (1U == CSL_I2C_ENABLE_DMA_MODE)
void I2CDMATxEventEnable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_XDMA_EN,
                   CSL_I2C_BUF_XDMA_EN_ENABLE);

    CSL_REG32_WR(baseAddr + CSL_I2C_DMATXENABLE_SET,
                 (uint32_t)CSL_I2C_DMATXENABLE_SET_DMATX_ENABLE_SET_MASK);
}

void I2CDMARxEventEnable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_RDMA_EN,
                   CSL_I2C_BUF_RDMA_EN_ENABLE);

    CSL_REG32_WR(baseAddr + CSL_I2C_DMARXENABLE_SET,
                 (uint32_t)CSL_I2C_DMARXENABLE_SET_DMARX_ENABLE_SET_MASK);
}

void I2CDMATxEventDisable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_XDMA_EN,
                   CSL_I2C_BUF_XDMA_EN_DISABLE);

    CSL_REG32_WR(baseAddr + CSL_I2C_DMATXENABLE_CLR,
                 (uint32_t)CSL_I2C_DMATXENABLE_CLR_DMATX_ENABLE_CLEAR_MASK);
}

void I2CDMARxEventDisable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_BUF, I2C_BUF_RDMA_EN,
                   CSL_I2C_BUF_RDMA_EN_DISABLE);

    CSL_REG32_WR(baseAddr + CSL_I2C_DMARXENABLE_CLR,
                 (uint32_t)CSL_I2C_DMARXENABLE_CLR_DMARX_ENABLE_CLEAR_MASK);
}
#endif

uint32_t I2CBufferStatus(uintptr_t baseAddr, uint32_t flag)
{
    uint32_t status;

    switch (flag)
    {
        case CSL_I2C_TX_BUFFER_STATUS:
            status = CSL_REG32_FEXT(baseAddr + CSL_I2C_BUFSTAT, I2C_BUFSTAT_TXSTAT);
            break;

        case CSL_I2C_RX_BUFFER_STATUS:
            status = CSL_REG32_FEXT(baseAddr + CSL_I2C_BUFSTAT, I2C_BUFSTAT_RXSTAT);
            break;

        case CSL_I2C_FIFO_DEPTH:
            status = CSL_REG32_FEXT(baseAddr + CSL_I2C_BUFSTAT, I2C_BUFSTAT_FIFODEPTH);
            break;

        default:
            /* Invalid input */
            status = 0U;
            break;
    }
    return(status);
}

void I2CSoftReset(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSC, I2C_SYSC_SRST,
                   CSL_I2C_SYSC_SRST_RSTMODE);
}

void I2CAutoIdleEnable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSC, I2C_SYSC_AUTOIDLE,
                   CSL_I2C_SYSC_AUTOIDLE_ENABLE);
}

void I2CAutoIdleDisable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSC, I2C_SYSC_AUTOIDLE,
                   CSL_I2C_SYSC_AUTOIDLE_DISABLE);
}

void I2CGlobalWakeUpEnable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSC, I2C_SYSC_ENAWAKEUP,
                   CSL_I2C_SYSC_ENAWAKEUP_ENABLE);
}

void I2CGlobalWakeUpDisable(uintptr_t baseAddr)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSC, I2C_SYSC_ENAWAKEUP,
                   CSL_I2C_SYSC_ENAWAKEUP_DISABLE);
}

void I2CIdleModeSelect(uintptr_t baseAddr, uint32_t flag)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSC, I2C_SYSC_IDLEMODE,
                   flag >> CSL_I2C_SYSC_IDLEMODE_SHIFT);
}

void I2CClockActivitySelect(uintptr_t baseAddr, uint32_t flag)
{
    CSL_REG32_FINS(baseAddr + CSL_I2C_SYSC, I2C_SYSC_CLKACTIVITY,
                   flag >> CSL_I2C_SYSC_CLKACTIVITY_SHIFT);
}

void I2CWakeUpEnable(uintptr_t baseAddr,
                     uint32_t  eventFlag,
                     uint32_t  flag)
{
    uint32_t i2cRegValue;

    if (CSL_I2C_WAKE_UP_IRQ == flag)
    {
        i2cRegValue  = CSL_REG32_RD(baseAddr + CSL_I2C_WE);
        i2cRegValue |= eventFlag;
        CSL_REG32_WR(baseAddr + CSL_I2C_WE, i2cRegValue);
    }
#if (1U == CSL_I2C_ENABLE_DMA_MODE)
    else if (CSL_I2C_WAKE_UP_DMA_RECV == flag)
    {
        i2cRegValue  = CSL_REG32_RD(baseAddr + CSL_I2C_DMARXWAKE_EN);
        i2cRegValue |= eventFlag;
        CSL_REG32_WR(baseAddr + CSL_I2C_DMARXWAKE_EN, i2cRegValue);
    }
    else if (CSL_I2C_WAKE_UP_DMA_TRANSMIT == flag)
    {
        i2cRegValue  = CSL_REG32_RD(baseAddr + CSL_I2C_DMATXWAKE_EN);
        i2cRegValue |= eventFlag;
        CSL_REG32_WR(baseAddr + CSL_I2C_DMATXWAKE_EN, i2cRegValue);
    }
#endif
    else
    {
        /*
         * Do nothing
         */
    }
}

void I2CWakeUpDisable(uintptr_t baseAddr,
                      uint32_t  eventFlag,
                      uint32_t  flag)
{
    uint32_t i2cRegValue;

    if (CSL_I2C_WAKE_UP_IRQ == flag)
    {
        i2cRegValue  = CSL_REG32_RD(baseAddr + CSL_I2C_WE);
        i2cRegValue &= ~eventFlag;
        CSL_REG32_WR(baseAddr + CSL_I2C_WE, i2cRegValue);
    }
#if (1U == CSL_I2C_ENABLE_DMA_MODE)
    else if (CSL_I2C_WAKE_UP_DMA_RECV == flag)
    {
        i2cRegValue  = CSL_REG32_RD(baseAddr + CSL_I2C_DMARXWAKE_EN);
        i2cRegValue &= ~eventFlag;
        CSL_REG32_WR(baseAddr + CSL_I2C_DMARXWAKE_EN, i2cRegValue);
    }
    else if (CSL_I2C_WAKE_UP_DMA_TRANSMIT == flag)
    {
        i2cRegValue  = CSL_REG32_RD(baseAddr + CSL_I2C_DMATXWAKE_EN);
        i2cRegValue &= ~eventFlag;
        CSL_REG32_WR(baseAddr + CSL_I2C_DMATXWAKE_EN, i2cRegValue);
    }
#endif
    else
    {
        /*
         * Do nothing
         */
    }
}

uint32_t I2CSystemStatusGet(uintptr_t baseAddr)
{
    return(CSL_REG32_RD(baseAddr + CSL_I2C_SYSS) & CSL_I2C_SYSS_RDONE_MASK);
}

void I2CMasterDataPut(uintptr_t baseAddr, uint8_t data)
{
    /*write data to be transmitted to Data transmit register */
    CSL_REG32_WR(baseAddr + CSL_I2C_DATA, (uint32_t)data);
}

uint8_t I2CMasterDataGet(uintptr_t baseAddr)
{
    uint8_t rData;

    rData = (uint8_t)CSL_REG32_RD(baseAddr + CSL_I2C_DATA);
    return(rData);
}

void I2CSyscInit(uintptr_t baseAddr, uint32_t syscFlag)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_SYSC, syscFlag);
}

void I2CConfig(uintptr_t baseAddr, uint32_t conParams)
{
    CSL_REG32_WR(baseAddr + CSL_I2C_CON, conParams);
}

void I2CFlushFifo(uintptr_t baseAddr)
{
    uint32_t i2cRegValue = CSL_REG32_RD(baseAddr + CSL_I2C_BUF);

    i2cRegValue |= CSL_I2C_BUF_RXFIFO_CLR_MASK | CSL_I2C_BUF_TXFIFO_CLR_MASK;
    CSL_REG32_WR(baseAddr + CSL_I2C_BUF, i2cRegValue);
}

