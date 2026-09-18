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
 * \file lpm_i2c_baremetal.c
 *
 * \brief Bare-metal I2C APIs used by the LPM stub to configure the PMIC.
 *        Other LPM applications use the I2C driver present in PDK.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/csl/csl_types.h>
#include <ti/csl/soc.h>
#include <ti/drv/i2c/i2c.h>
#include <ti/drv/lpm/include/lpm_pmic.h>
#include <ti/drv/lpm/src/lpm_i2c_baremetal.h>
#include <ti/drv/lpm/src/lpm_uart_baremetal.h>
#include <ti/drv/lpm/src/lpm_io_isolation.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Expected value for Lpm_i2cPollBit when waiting for a bit to be de-asserted */
#define LPM_I2C_BIT_CLEARED     (0U)

/* Timeout value that disables the limit, poll until the operation completes */
#define LPM_I2C_WAIT_FOREVER    (0xFFFFFFFFU)

/* I2C transfer byte counts written to the CNT register */

/* Address-only phase (register address byte) or single-byte receive */
#define LPM_I2C_CNT_ONE_BYTE    (1U)

/* Full register write: register address + data byte */
#define LPM_I2C_CNT_TWO_BYTES   (2U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/**
 * \brief Poll a register field until it reaches the expected value or timeout expires.
 *
 * \param  reg      Register offset
 * \param  mask     Bit mask of the field to check.
 * \param  expected Value the masked field must reach to exit.
 * \param  timeout  Maximum number of polling iterations before giving up.
 *
 * \return CSL_PASS when the expected value is seen, CSL_EFAIL on timeout.
 */
static int32_t Lpm_i2cPollBit(uint32_t reg, uint32_t mask, uint32_t expected, uint32_t timeout);

/**
 * \brief  Read one byte from the I2C slave register at address \a addr,
 *         with a polling timeout.
 *
 * \param  addr     Register address to read.
 * \param  rxd      Pointer to store the received byte.
 * \param  timeout  Maximum number of polling iterations before giving up.
 *
 * \return CSL_PASS on success, CSL_EFAIL on timeout.
 */
static int32_t Lpm_i2cReadTimeout(char addr, uint8_t *rxd, uint32_t timeout);

/**
 * \brief Phase 1 of a register read: wait for bus free, configure TX mode,
 *        write the register address byte, and wait for ARDY.
 */
static int32_t Lpm_i2cSendRegisterAddr(char addr, uint32_t timeout);

/**
 * \brief Phase 2 of a register read: switch to RX mode, receive one byte,
 *        wait for ARDY, and clear all pending interrupts.
 */
static int32_t Lpm_i2cReceiveData(uint8_t *rxd, uint32_t timeout);

/**
 * \brief Enable RX_ACTIVE on WKUP I2C padconfigs so pads remain active during IO isolation.
 */
static void Lpm_i2cConfigureIOPads(void);

/** \brief Reset the I2C module to a known state and wait until it is ready to accept configuration.
 */
static void Lpm_i2cSoftReset(void);

/**
 * \brief Configure the I2C clock for 400 kHz Fast Mode, clear the previous
 *        controller state, and re-enable the I2C module.
 */
static void Lpm_i2cSetClockFreq(void);

/**
 * \brief Set the PMIC slave address, flush TX and RX FIFOs, and unlock PMIC register space.
 *
 * \param pmic  I2C slave address of the PMIC to target.
 */
static void Lpm_i2cSetupTarget(char pmic);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

uint8_t Lpm_i2cRead(char addr)
{
    uint8_t rxData = 0U;
    int32_t status;

    status = Lpm_i2cReadTimeout(addr, &rxData, LPM_I2C_WAIT_FOREVER);
    if (status != CSL_PASS)
    {
        Lpm_uartDebugFullPrintf("Lpm_i2cRead: read failed for register 0x%x\n", addr);
    }

    return rxData;
}

void Lpm_i2cConfigWkup(char pmic)
{
    /*
     * Initialise WKUP_I2C0 for PMIC communication during the S2R sequence:
     *
     *  1. Enable RX_ACTIVE on the SCL and SDA pad receivers so the pads
     *     remain functional after IO isolation is asserted.
     *
     *  2. Soft-reset the I2C module (SRST) to reach a known state and enable
     *     smart-idle mode.
     *
     *  3. Configure the SCL clock: set PSC (Prescaler) to derive 12 MHz
     *     from the 96 MHz input, then set SCLL and SCLH appropriately for 400 kHz
     *     Fast mode timing. Clear old state of the controller and re-enable
     *     the module, and wait for RDONE to confirm the new settings are active.
     *
     *  4. Set the PMIC slave address as the transfer target, flush the
     *     TX and RX FIFOs, and unlock the PMIC register space.
     */
    Lpm_i2cConfigureIOPads();
    Lpm_i2cSoftReset();
    Lpm_i2cSetClockFreq();
    Lpm_i2cSetupTarget(pmic);
}

void Lpm_i2cWrite(char addr, char data)
{
    /*
     * Perform a 2-byte I2C master write to the PMIC:
     *
     *  1. Wait for the bus to be free (BB bit de-asserted).
     *
     *  2. Set transfer count to 2 (addr and data), configure transmit mode, and
     *     assert START condition to begin the transaction.
     *
     *  3. For each byte (register address 'addr'; then the data byte 'data'):
     *     a. Poll XRDY (Transmit Data Ready) until the TX FIFO can accept the next byte.
     *     b. Load the byte into the TX FIFO.
     *
     *  4. Clear XRDY (Transmit Data Ready) to signal the last byte is loaded.
     *
     *  5. Poll ARDY (Access Ready) until the STOP condition has been issued and both bytes
     *     have been acknowledged by the slave.
     *
     *  6. Clear all pending interrupt status bits to leave the controller state clean.
     */

    uint32_t n;
    uint32_t regVal;

    /* Ensure the Bus Busy (BB) bit is de-asserted before starting a new write transfer */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
    while((regVal & CSL_I2C_IRQSTATUS_RAW_BB_MASK) != 0U)
    {
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
    }

    /* Set transfer count to 2 bytes (addr and data) and configure transmit mode
     * (TRX=1, Transmit mode) with START + STOP
     */
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CNT, LPM_I2C_CNT_TWO_BYTES);
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal &= ~CSL_I2C_CON_STP_MASK;
    regVal |= (CSL_I2C_CON_TRX_MASK | CSL_I2C_CON_MST_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Assert START condition to begin the two-byte write transfer */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal |= ((CSL_I2C_CON_STT_MASK | CSL_I2C_CON_STP_MASK) << CSL_I2C_CON_STT_SHIFT);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Send register address (addr) then data value (data) across two TX iterations */
    for(n = 0; n < LPM_I2C_CNT_TWO_BYTES; n++)
    {
        /* Poll XRDY (Transmit Data Ready) before loading the next byte into the TX FIFO */
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
        while((regVal & CSL_I2C_IRQSTATUS_RAW_XRDY_MASK) == 0U)
        {
            regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
        }

        if(0U == n)
        {
            /* Load the target register address into the TX FIFO */
            CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_DATA, addr);
        }
        else
        {
            /* Load the data value into the TX FIFO and clear XRDY (Transmit Data Ready) to transmit it */
            CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_DATA, data);
            CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS, CSL_I2C_IRQSTATUS_RAW_XRDY_MASK);
        }
    }

    /* Poll ARDY (Access Ready) flag to make sure both bytes are transmitted and the STOP
     * condition has been issued.
    */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
    while((regVal & CSL_I2C_IRQSTATUS_RAW_ARDY_MASK) == 0U)
    {
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
    }

    /* Clear all pending interrupt status bits to leave the controller state clean */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS, regVal);
}


/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static int32_t Lpm_i2cPollBit(uint32_t reg, uint32_t mask, uint32_t expected, uint32_t timeout)
{
    /* Busy-wait is used because this driver runs baremetal in the
     * lpm stub with no OS scheduler or interrupt handler available
     */
    uint32_t regVal;
    uint32_t loop = timeout;

    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, reg);
    while ((loop > 0U) && ((regVal & mask) != expected))
    {
        loop--;
        regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, reg);
    }

    return (0U == loop) ? CSL_EFAIL : CSL_PASS;
}

static int32_t Lpm_i2cSendRegisterAddr(char addr, uint32_t timeout)
{
    uint32_t regVal;
    int32_t retVal;

    /* Ensure the Bus Busy (BB) bit is de-asserted before starting a new transfer */
    retVal = Lpm_i2cPollBit(CSL_I2C_IRQSTATUS_RAW, CSL_I2C_IRQSTATUS_RAW_BB_MASK, LPM_I2C_BIT_CLEARED, timeout);
    if (retVal != CSL_PASS)
    {
        Lpm_uartDebugFullPrintf("Lpm_i2cReadTimeout: timeout loop exceed %d\n", timeout);
        return retVal;
    }

    /* Set transfer count to 1 byte and set transmit mode (TRX=1) and master mode (MST=1) */
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CNT, LPM_I2C_CNT_ONE_BYTE);
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal &= ~CSL_I2C_CON_STP_MASK;
    regVal |= (CSL_I2C_CON_TRX_MASK | CSL_I2C_CON_MST_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Assert START to begin the address-phase write */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal |= CSL_I2C_CON_STT_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Poll XRDY (Transmit Data Ready) until the TX FIFO is ready to accept the register address byte */
    retVal = Lpm_i2cPollBit(CSL_I2C_IRQSTATUS_RAW, CSL_I2C_IRQSTATUS_RAW_XRDY_MASK,
                            CSL_I2C_IRQSTATUS_RAW_XRDY_MASK, timeout);
    if (retVal != CSL_PASS)
    {
        Lpm_uartDebugFullPrintf("Lpm_i2cReadTimeout: timeout for XRDY: loop exceed %d\n", timeout);
        return retVal;
    }

    /* Load the register address into the TX FIFO and clear XRDY (Transmit Data Ready) to transmit it */
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_DATA, addr);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS, CSL_I2C_IRQSTATUS_RAW_XRDY_MASK);

    /* Poll ARDY (Access Ready) until the address-write phase is complete and the controller is ready for the read phase */
    retVal = Lpm_i2cPollBit(CSL_I2C_IRQSTATUS_RAW, CSL_I2C_IRQSTATUS_RAW_ARDY_MASK,
                            CSL_I2C_IRQSTATUS_RAW_ARDY_MASK, timeout);
    if (retVal != CSL_PASS)
    {
        Lpm_uartDebugFullPrintf("Lpm_i2cReadTimeout: timeout for ARDY: loop exceed %d\n", timeout);
    }

    return retVal;
}

static int32_t Lpm_i2cReceiveData(uint8_t *rxd, uint32_t timeout)
{
    uint32_t regVal;
    int32_t retVal;

    /* Clear all pending interrupt status bits before switching to receive mode */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS, regVal);

    /* Switch to receive mode (TRX=0 enables receive mode) and keep master mode (MST=1 is master mode)
     * for a 1-byte read
     */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal &= ~CSL_I2C_CON_TRX_MASK;
    regVal |= CSL_I2C_CON_MST_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CNT, LPM_I2C_CNT_ONE_BYTE);

    /* Issue a repeated START + STOP to clock in the data byte from the slave */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal |= (CSL_I2C_CON_STT_MASK | CSL_I2C_CON_STP_MASK) << CSL_I2C_CON_STT_SHIFT;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Poll RRDY (Receive Data Ready) until the RX FIFO holds the response byte from the slave */
    retVal = Lpm_i2cPollBit(CSL_I2C_IRQSTATUS_RAW, CSL_I2C_IRQSTATUS_RAW_RRDY_MASK,
                            CSL_I2C_IRQSTATUS_RAW_RRDY_MASK, timeout);
    if (retVal != CSL_PASS)
    {
        Lpm_uartDebugFullPrintf("Lpm_i2cReadTimeout: timeout for RRDY: loop exceed %d\n", timeout);
        return retVal;
    }

    *rxd = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_DATA);

    /* Poll ARDY (Access Ready) until the data read phase is complete and the STOP condition has been issued */
    retVal = Lpm_i2cPollBit(CSL_I2C_IRQSTATUS_RAW, CSL_I2C_IRQSTATUS_RAW_ARDY_MASK,
                            CSL_I2C_IRQSTATUS_RAW_ARDY_MASK, timeout);
    if (retVal != CSL_PASS)
    {
        Lpm_uartDebugFullPrintf("Lpm_i2cReadTimeout: timeout for ARDY: loop exceed %d\n", timeout);
        return retVal;
    }

    /* Clear all pending interrupt status bits to leave the controller clean */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS_RAW);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_IRQSTATUS, regVal);

    return retVal;
}

static int32_t Lpm_i2cReadTimeout(char addr, uint8_t *rxd, uint32_t timeout)
{
    int32_t retVal = Lpm_i2cSendRegisterAddr(addr, timeout);
    if (retVal == CSL_PASS)
    {
        retVal = Lpm_i2cReceiveData(rxd, timeout);
    }
    return retVal;
}

static void Lpm_i2cConfigureIOPads(void)
{
    uint32_t regVal;

    /* Enable RX_ACTIVE on WKUP_I2C SCL (PADCONFIG62) and SDA (PADCONFIG63) to keep receivers active during IO isolation */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_PADCONFIG62);
    regVal |= LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_PADCONFIG62, regVal);

    regVal = CSL_REG32_RD_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_PADCONFIG63);
    regVal |= LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_CTRL_MMR0_CFG0_BASE, CSL_MAIN_CTRL_MMR_CFG0_PADCONFIG63, regVal);
}

static void Lpm_i2cSoftReset(void)
{
    uint32_t regVal;

    /* Assert SRST (Soft Reset) to bring the I2C module to a known state */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SYSC);
    regVal |= CSL_I2C_SYSC_SRST_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SYSC, regVal);

    /* Enable smart-idle mode: the i2c module refuses PRCM idle requests during
     * an active transfer, preventing clock gating mid-transaction and
     * ensuring PMIC commands complete without corruption.
     */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SYSC);
    regVal |= (1U << CSL_I2C_SYSC_IDLEMODE_SHIFT);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SYSC, regVal);

    /* Assert I2C_EN (I2C Enable) to start the reset sequence */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal |= CSL_I2C_CON_I2C_EN_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Poll SYSS.RDONE (Reset Done) flag until the soft-reset completes */
    (void)Lpm_i2cPollBit(CSL_I2C_SYSS, CSL_I2C_SYSS_RDONE_MASK,
                         CSL_I2C_SYSS_RDONE_MASK, LPM_I2C_DETECT_TIMEOUT);
}

static void Lpm_i2cSetClockFreq(void)
{
    uint32_t regVal;

    /*
     * Configure SCL clock for I2C Fast Mode (400 kHz).
     *
     * Fast Mode minimum timing requirements are (I2C spec):
     *   t_LOW  >= 1.3 µs   (for SCL low period)
     *   t_HIGH >= 0.6 µs   (for SCL high period)
     *
     * The TI I2C controller derives SCL from the module's input clock via a
     * two-stage process:
     *   1. PSC divides the 96 MHz input to produce the internal 12 MHz clock.
     *   2. SCLL and SCLH count in multiples of that 12 MHz tick (83.3 ns each).
     *      The hardware adds fixed overheads: +7 cycles for low and +5 cycles for high period.
     *
     * Result: SCLL set to 9 derives 1.333 µs as low period,
     *         SCLH set to 9 derives 1.167 µs as high period.
     *
     * Hence, SCLL + SCLH = 1.333 + 1.167 = 2.5 µs = 400 kHz.
     */

    /* Set I2C_PSC (Prescaler) to derive 12 Mhz internal clock */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_PSC);
    regVal &= ~CSL_I2C_PSC_PSC_MASK;
    regVal |= LPM_I2C_PRESCALER_DIVIDER_12MHZ;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_PSC, regVal);

    /* Set SCLL (SCL Low period): (9+7 cycles)/12MHz = 1.333 µs.
     * This meets Fast Mode requirement that, t_LOW >= 1.3 µs
     */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SCLL);
    regVal &= ~CSL_I2C_SCLL_SCLL_MASK;
    regVal |= LPM_I2C_SCLL_16;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SCLL, regVal);

    /* Set SCLH (SCL High period): (9+5 cycles)/12MHz = 1.167 µs,
     * this meets Fast Mode requirement that, t_HIGH >= 0.6 µs;
     */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SCLH);
    regVal &= ~CSL_I2C_SCLH_SCLH_MASK;
    regVal |= LPM_I2C_SCLH_14;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SCLH, regVal);

    /* Clear write-control bits, data count, and DMA fields before re-enabling */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal &= ~(LPM_I2C_CON_WRITE_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Clear DCOUNT (data count) to prevent a stale byte count from a previous transaction
     * from carrying over
     */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CNT);
    regVal &= ~(CSL_I2C_CNT_DCOUNT_MAX);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CNT, regVal);

    /* Disable RX (RDMA_EN) and TX (XDMA_EN) DMA use polling IO mode */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_BUF);
    regVal &= ~(CSL_I2C_BUF_RDMA_EN_MASK | CSL_I2C_BUF_XDMA_EN_MASK);
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_BUF, regVal);

    /* Set own address OA (Own Address) to initialise the hardware address comparator,
     * required even in master-only mode
     */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_OA);
    regVal &= ~CSL_I2C_OA_OA_MASK;
    regVal |= LPM_I2C_OA_REG;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_OA, regVal);

    /* Re-assert I2C_EN (I2C Enable) after clock configuration */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON);
    regVal |= CSL_I2C_CON_I2C_EN_MASK;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_CON, regVal);

    /* Poll SYSS.RDONE (Reset Done) flag until the soft-reset completes and new
     * clock settings are active.
     */
    (void)Lpm_i2cPollBit(CSL_I2C_SYSS, CSL_I2C_SYSS_RDONE_MASK,
                         CSL_I2C_SYSS_RDONE_MASK, LPM_I2C_DETECT_TIMEOUT);
}

static void Lpm_i2cSetupTarget(char pmic)
{
    uint32_t regVal;

    /* Set the PMIC's I2C slave address as the target for subsequent transfers */
    regVal = CSL_REG32_RD_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SA);
    regVal &= ~CSL_I2C_SA_SA_MASK;
    regVal |= pmic;
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_SA, regVal);

    /* Reset TX and RX FIFOs and set both transfer thresholds to 1 byte */
    CSL_REG32_WR_OFF(CSL_WKUP_I2C0_CFG_BASE, CSL_I2C_BUF,
                    (CSL_I2C_BUF_TXFIFO_CLR_RSTMODE << CSL_I2C_BUF_TXFIFO_CLR_SHIFT) |
                    (CSL_I2C_BUF_RXFIFO_CLR_RSTMODE << CSL_I2C_BUF_RXFIFO_CLR_SHIFT));

    /* Unlock PMIC configuration registers to enable writes to protected register space */
    Lpm_i2cWrite(LPM_PMIC_REGISTER_LOCK, LPM_PMIC_REGISTER_LOCK_UNLOCK);
}
