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
 *  \file     lpm_pmic_i2c_drv.c
 *
 *  \brief    PMIC access using the PDK I2C driver.
 *            Used for mcu_only and io_retention examples.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/osal/TaskP.h>
#include <ti/drv/lpm/lpm.h>
#include <ti/drv/lpm/include/lpm_pmic.h>
#include <ti/drv/lpm/src/lpm_uart_drv.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Write this value to the debug loop variable in CCS to break execution and
 * to pause execution at the corresponding debug loop
 */
#define LPM_PMIC_I2C_CCS_HALT                         (0xFEEDFACEU)

/* I2C transfer byte-count constants used in Lpm_pmicSetupI2cTransfer() calls */

/* Register address + data byte */
#define LPM_PMIC_I2C_REG_WRITE_LEN                     (2U)

/* Register address only (for a read) */
#define LPM_PMIC_I2C_REG_ADDR_LEN                      (1U)

/* Read back one data byte */
#define LPM_PMIC_I2C_READ_ONE_BYTE                     (1U)

/* Write-only transfer, no read phase */
#define LPM_PMIC_I2C_NO_READ                           (0U)

/* I2C buffer size and index constants */

/* Max bytes per transfer: register address + data */
#define LPM_PMIC_I2C_BUF_SIZE                          (2U)

/* Byte 0 of TX buffer: register address */
#define LPM_PMIC_I2C_TX_REG_IDX                        (0U)

/* Byte 1 of TX buffer: data value */
#define LPM_PMIC_I2C_TX_DATA_IDX                       (1U)

/* Byte 0 of RX buffer: received data */
#define LPM_PMIC_I2C_RX_DATA_IDX                       (0U)

#define LPM_PMIC_STATE_TRANSITION_TIMEOUT              (100U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#ifdef LPM_DEBUG
volatile uint32_t loopPMICStateChangeActiveToIORetention = 0;
volatile uint32_t loopPMICStateChangeActiveToMCUOnly = 0;
#endif

I2C_Handle gLpmPmicI2cHandle  = NULL;

/* ========================================================================== */
/*                  Internal/Private Function Declarations                    */
/* ========================================================================== */

/**
 * \brief Write one byte to a PMIC register.
 *
 * \param slaveAddr  I2C slave address of the target PMIC.
 * \param reg        Register address to write.
 * \param val        Value to write.
 */
static void Lpm_pmicWriteReg(uint32_t slaveAddr, uint8_t reg, uint8_t val);

/**
 * \brief Read one byte from a PMIC register.
 *
 * \param slaveAddr  I2C slave address of the target PMIC.
 * \param reg        Register address to read.
 *
 * \return The byte value read from the register.
 */
static uint8_t Lpm_pmicReadReg(uint32_t slaveAddr, uint8_t reg);

/**
 * \brief Perform an I2C transfer to/from a slave device.
 *
 * Issues a write followed optionally by a read on the given I2C handle.
 * Used internally by the PMIC driver to send register write commands and
 * read back register values.
 *
 * \param handle        Open I2C driver handle.
 * \param slaveAddr     7-bit I2C slave address of the target device.
 * \param writeData     Pointer to data bytes to write.
 * \param numWriteBytes Number of bytes to write.
 * \param readData      Pointer to buffer for read bytes (may be NULL).
 * \param numReadBytes  Number of bytes to read (0 if write-only).
 */
static void Lpm_pmicSetupI2cTransfer(I2C_Handle handle, uint32_t slaveAddr,
                              uint8_t *writeData, uint32_t numWriteBytes,
                              uint8_t *readData,  uint32_t numReadBytes);

/**
 *  \brief Initializes PMIC using the I2C driver.
 */
static int32_t Lpm_pmicI2cDrvInit(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_pmicStateChangeActiveToIORetention(void)
{
    uint8_t val;

#ifdef LPM_DEBUG
    if(loopPMICStateChangeActiveToIORetention == LPM_PMIC_I2C_CCS_HALT)
    {
        Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG
                          "Connect CCS and change the loopPMICStateChangeActiveToIORetention to 0x0!!!!\n");
        Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG
                          "This will disconnect the JTAG interface too and you can only see the MCU running
                           from UART prints!!!!\n");
    }

    while(loopPMICStateChangeActiveToIORetention == LPM_PMIC_I2C_CCS_HALT)
    {
        /* Wait for user to change the state of halt variable before
         * executing the Active to IO retention sequence.
         */
    }
#endif

    /* Set NSLEEP1=1 and NSLEEP2=1 to put the PMIC into Active state, this ensures that subsequent register writes
     * are not blocked by an in-progress state transition.
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS, LPM_PMIC_ACTIVE_STATE);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write FSM_NSLEEP_TRIGGERS = 0x%x\n", LPM_PMIC_ACTIVE_STATE);

    /* Clear the NPWRON_START interrupt bit. If there are pending interrupts, the transition to retention state
     * will not happen.
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_STARTUP, LPM_PMIC_INT_STARTUP_NPWRON_START_MASK);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write INT_STARTUP = 0x%x\n", LPM_PMIC_INT_STARTUP_NPWRON_START_MASK);

    /* Configure GPIO4_CONF: Set pmic gpio 4 as input and function select LP_WKUP1 */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_GPIO4_CONF, LPM_PMIC_GPIOx_CONF_GPIO_SEL_LP_WKUP1);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write GPIO4_CONF = 0x%x\n", LPM_PMIC_GPIOx_CONF_GPIO_SEL_LP_WKUP1);

    /* Clear GPIO4 interrupt bit. If there are pending interrupts, the transition to retention state
     * will not happen.
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_GPIO1_8, LPM_PMIC_GPIO_OUT_1_GPIO4_MASK);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write INT_GPIO1_8 = 0x%x\n", LPM_PMIC_GPIO_OUT_1_GPIO4_MASK);

    /* As GPIO4 detects the wakeup signal, unmask GPIO4 falling edge and mask all others. */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_MASK_GPIO1_8_FALL,
                     (uint8_t)~LPM_PMIC_GPIO_OUT_1_GPIO4_MASK);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write MASK_GPIO1_8_FALL = 0x%x\n",
                      (uint8_t)~LPM_PMIC_GPIO_OUT_1_GPIO4_MASK);

    /* Read back GPIO_IN_1 from PMIC to check interrupt state before proceeding */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_GPIO_IN_1);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "PMICA_REG_0x3F = 0x%x\n", val);

    /* Clear nINT by acknowledging the GPIO4 interrupt to allow PMIC to enter IO retention */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_GPIO1_8, LPM_PMIC_GPIO_OUT_1_GPIO4_MASK);

    /* Change FSM_I2C_TRIGGERS: set GPIO_RET_I2C_6 to enter IO retention */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_I2C_TRIGGERS,
                     LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write FSM_I2C_TRIGGERS = 0x%x\n",
                      LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK);

    /* Change FSM_I2C_TRIGGERS for PMIC-B as done for PMIC-A */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_B_ADDR, LPM_PMIC_FSM_I2C_TRIGGERS,
                     LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write FSM_I2C_TRIGGERS = 0x%x\n",
                      LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK);

    /* Change FSM_NSLEEP_TRIGGERS: assert NSLEEP1=low, NSLEEP2=low to enter IO retention */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS, LPM_PMIC_RETENTION_STATE);
    Lpm_uartDrvPrintf(LPM_UART_IO_RETENTION_MSG "Write FSM_NSLEEP_TRIGGERS = 0x%x\n", LPM_PMIC_RETENTION_STATE);
}


void Lpm_pmicStateChangeActiveToMCUOnly(void)
{
    uint8_t val;

    /* FSM_NSLEEP_TRIGGERS must be written before clearing any pending interrupts.
     * Clearing an interrupt immediately presents a trigger to the PMIC FSM.
     * This would cause an unintended state transition.
     * Writing the NSLEEP bits first lets both bits be configured atomically before
     * the PMIC acts on the change.
     */

#ifdef LPM_DEBUG
    if(loopPMICStateChangeActiveToMCUOnly == LPM_PMIC_I2C_CCS_HALT)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                          "Connect CCS and change the loopPMICStateChangeActiveToMCUOnly to 0x0\n");
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG
                          "This will disconnect the JTAG interface too and you can only see the MCU running from UART prints\n");
    }

    while(loopPMICStateChangeActiveToMCUOnly == LPM_PMIC_I2C_CCS_HALT)
    {
        /* Wait for user to change the state of halt variable before
         * executing the Active to MCU only sequence.
         */
    }
#endif

    /* Snapshot the top-level interrupt summary before starting; any asserted bit
     * here must be cleared later or the FSM will not complete the state change
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_TOP);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_TOP = 0x%x\n", val);

    /* Read startup interrupt status to identify any pending power-on events
     * that will need to be acknowledged before the FSM can transition
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_STARTUP);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_STARTUP = 0x%x\n", val);

    /* Read GPIO interrupt status to check whether a GPIO wake event is pending
     * that must be cleared to allow the PMIC FSM to proceed
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_GPIO);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_GPIO = 0x%x\n", val);

    /* Read per-GPIO interrupt bits for GPIO1-8 to identify which GPIO triggered
     * the interrupt so it can be cleared individually below
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_GPIO1_8);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_GPIO1_8 = 0x%x\n", val);

    /* Read current FSM trigger state before modifying it. to confirm we are
     * starting from the expected MCU Only state (NSLEEP1=0, NSLEEP2=1)
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Read FSM_NSLEEP_TRIGGERS = 0x%x\n", val);

    /* Assert NSLEEP1=1 (bit 1) while keeping NSLEEP2=0 to move the PMIC FSM
     * from MCU Only toward Active state; interrupts are cleared afterwards
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS, LPM_PMIC_MCU_ONLY_STATE);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Write FSM_NSLEEP_TRIGGERS = 0x%x\n", LPM_PMIC_MCU_ONLY_STATE);

    /* Read back the trigger register to confirm the NSLEEP1 bit has been latched
     * by the PMIC before we proceed to clear the pending interrupts
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Read FSM_NSLEEP_TRIGGERS = 0x%x\n", val);

    /* Clear the startup interrupt by writing back the read value; this
     * acknowledges the power-on event and unblocks the FSM transition
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_STARTUP, LPM_PMIC_INT_STARTUP_NPWRON_START_MASK);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Write INT_STARTUP = 0x%x\n", LPM_PMIC_INT_STARTUP_NPWRON_START_MASK);

    /* Re-read INT_TOP to check whether all interrupts have been cleared
     * after acknowledging INT_STARTUP
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_TOP);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_TOP = 0x%x\n", val);

    /* Clear GPIO4, GPIO7, and GPIO8 interrupt bits to acknowledge the
     * GPIO wake events that were latched during the MCU Only state
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_GPIO1_8, LPM_PMIC_INT_GPIO1_8_WAKE_INT_MASK);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Write INT_GPIO1_8 = 0x%x\n", LPM_PMIC_INT_GPIO1_8_WAKE_INT_MASK);

    /* Read INT_GPIO summary to verify that the GPIO interrupt sub-group
     * is now clear after acknowledging the GPIO1-8 bits above
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_GPIO);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_GPIO = 0x%x\n", val);

    /* Read INT_TOP again after GPIO clear to confirm the GPIO summary bit
     * has been de-asserted before writing the final interrupt clear
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_TOP);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_TOP = 0x%x\n", val);

    /* Clear the INT_GPIO summary bit (bit 1) to fully acknowledge the GPIO
     * interrupt group and allow the PMIC FSM to complete the Active transition
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_GPIO, LPM_PMIC_INT_GPIO_GPIO2_INT_MASK);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Write INT_GPIO = 0x%x\n", LPM_PMIC_INT_GPIO_GPIO2_INT_MASK);

    /* Final read of INT_TOP to confirm all interrupts have been cleared;
     * A zero value here indicates the PMIC FSM can complete the Active state entry
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_TOP);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Final Read INT_TOP = 0x%x\n", val);

    /* Final read of FSM_NSLEEP_TRIGGERS to verify the PMIC has accepted the
     * active state command and the transition is complete
     */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Final Read FSM_NSLEEP_TRIGGERS = 0x%x\n", val);
}

uint32_t Lpm_pmicInit(void)
{
    uint32_t pmicStatus = Lpm_pmicI2cDrvInit();
    if(CSL_PASS != pmicStatus)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Lpm_pmicInit failed!\n");
    }

    return pmicStatus;
}


void Lpm_pmicStateChangeMCUOnlyToActive(void)
{
    uint8_t val;

    /* Read interrupt summary register to log pending interrupt state before starting the transition */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_INT_TOP);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "INT_TOP = 0x%x\n", val);

    /* Configure NSLEEP1 and NSLEEP2 mask. When set the PMIC ignores NSLEEP
     * gpio pin transitions, allowing FSM_NSLEEP_TRIGGERS to be updated safely.
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_CONFIG_1, LPM_PMIC_CONFIG_1_NSLEEP_MASK);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Write CONFIG_1 = 0x%x\n", LPM_PMIC_CONFIG_1_NSLEEP_MASK);

    /* Set NSLEEP1=1 and NSLEEP2=1 to put the PMIC into Active state, this ensures that subsequent register writes
     * are not blocked by an in-progress state transition.
     */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS, LPM_PMIC_ACTIVE_STATE);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Write FSM_NSLEEP_TRIGGERS = 0x%x\n", LPM_PMIC_ACTIVE_STATE);

    /* Clear the NSLEEP mask so the PMIC can now react to the NSLEEP pin transitions and complete the FSM state change */
    Lpm_pmicWriteReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_CONFIG_1, LPM_PMIC_RETENTION_STATE);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Write CONFIG_1 = 0x%x\n", LPM_PMIC_RETENTION_STATE);

    /* Buffer time for the PMIC FSM to complete the Active state transition before we read back to verify */
    TaskP_sleep(LPM_PMIC_STATE_TRANSITION_TIMEOUT);

    /* Read back to confirm the PMIC has latched the ACTIVE_STATE trigger */
    val = Lpm_pmicReadReg(BOARD_I2C_LEO_PMIC_A_ADDR, LPM_PMIC_FSM_NSLEEP_TRIGGERS);
    Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "Read FSM_NSLEEP_TRIGGERS = 0x%x\n", val);
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static int32_t Lpm_pmicI2cDrvInit(void)
{
    int32_t retVal = CSL_PASS;
    I2C_Params i2cParams;

    /* Initialize i2c core instances */
    I2C_init();
    uint8_t i2c_instance = 0U;
    uint32_t baseAddr = CSL_WKUP_I2C0_CFG_BASE;

    I2C_HwAttrs i2cCfg;
    I2C_socGetInitCfg(i2c_instance, &i2cCfg);
    i2cCfg.baseAddr   = baseAddr;
    i2cCfg.enableIntr = false;
    I2C_socSetInitCfg(i2c_instance, &i2cCfg);

    /* Configured i2cParams.bitRate with standard I2C_100kHz */
    I2C_Params_init(&i2cParams);
    gLpmPmicI2cHandle = I2C_open(i2c_instance, &i2cParams);
    if(NULL == gLpmPmicI2cHandle)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "ERROR: I2C_open failed!\n");
        retVal = CSL_EFAIL;
    }

    return retVal;
}

static void Lpm_pmicWriteReg(uint32_t slaveAddr, uint8_t reg, uint8_t val)
{
    uint8_t txBuf[LPM_PMIC_I2C_BUF_SIZE];

    txBuf[LPM_PMIC_I2C_TX_REG_IDX]  = reg;
    txBuf[LPM_PMIC_I2C_TX_DATA_IDX] = val;
    Lpm_pmicSetupI2cTransfer(gLpmPmicI2cHandle, slaveAddr,
                             txBuf, LPM_PMIC_I2C_REG_WRITE_LEN, NULL, LPM_PMIC_I2C_NO_READ);
}

static uint8_t Lpm_pmicReadReg(uint32_t slaveAddr, uint8_t reg)
{
    uint8_t txBuf[LPM_PMIC_I2C_BUF_SIZE];
    uint8_t rxBuf[LPM_PMIC_I2C_BUF_SIZE] = {0U, 0U};

    txBuf[LPM_PMIC_I2C_TX_REG_IDX] = reg;
    Lpm_pmicSetupI2cTransfer(gLpmPmicI2cHandle, slaveAddr,
                             txBuf, LPM_PMIC_I2C_REG_ADDR_LEN, rxBuf, LPM_PMIC_I2C_READ_ONE_BYTE);

    return rxBuf[LPM_PMIC_I2C_RX_DATA_IDX];
}

static void Lpm_pmicSetupI2cTransfer(I2C_Handle handle, uint32_t slaveAddr,
                          uint8_t *writeData, uint32_t numWriteBytes,
                          uint8_t *readData,  uint32_t numReadBytes)
{
    bool status;
    I2C_Transaction i2cTransaction;

    I2C_transactionInit(&i2cTransaction);

    i2cTransaction.slaveAddress = slaveAddr;
    i2cTransaction.writeBuf = writeData;
    i2cTransaction.writeCount = numWriteBytes;
    i2cTransaction.readBuf = readData;
    i2cTransaction.readCount = numReadBytes;

    status = I2C_transfer(handle, &i2cTransaction);
    if(FALSE == status)
    {
        Lpm_uartDrvPrintf(LPM_UART_MCU_ONLY_MSG "\n Data Transfer failed. \n");
    }
}
