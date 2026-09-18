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
 * \file lpm_pmic_i2c_baremetal.c
 *
 * \brief PMIC driver which uses a baremetal i2c driver underneath.
 *        It is used inside lpm stub to perform low power mode entry.
 */

#include <stdint.h>
#include <ti/csl/soc.h>
#if defined(SOC_J7200)
#include <ti/board/src/j7200_evm/include/board_cfg.h>
#elif defined(SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_cfg.h>
#elif defined(SOC_J742S2)
#include <ti/board/src/j742s2_evm/include/board_cfg.h>
#elif defined(SOC_J721S2)
#include <ti/board/src/j721s2_evm/include/board_cfg.h>
#endif
#include <ti/drv/lpm/include/lpm_pmic.h>
#include <ti/drv/lpm/src/lpm_uart_baremetal.h>
#include <ti/drv/lpm/src/lpm_i2c_baremetal.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*
 * The -DLPM_DEBUG should be passed at compile time in the lpm_stub makefile
 * as part of CFLAGS_LOCAL_COMMON to enable debug logging functionality.
 */
#ifdef LPM_DEBUG
#define Lpm_pmicDebugReadPmicA  Lpm_pmicReadPmicA
#define Lpm_pmicDebugReadPmicB  Lpm_pmicReadPmicB
#else
#define Lpm_pmicDebugReadPmicA(...)  do {} while(0)
#define Lpm_pmicDebugReadPmicB(...)  do {} while(0)
#endif

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
 * \brief Put the PMIC FSM into Active state and clear all pending interrupts.
 */
static void Lpm_pmicActivateAndClearInterrupts(void);

/**
 * \brief Configure FSM trigger registers for the target low-power mode.
 *
 * \param lpm_mode  LPM_PMIC_SOC_OFF or LPM_PMIC_IO_ONLY_PLUS_DDR.
 */
static void Lpm_pmicConfigureFsmTriggers(uint32_t lpm_mode);

/**
 * \brief Configure GPIO4 as the LP_WKUP1 wakeup input and unmask the correct
 *        Edge interrupt for the target mode.
 *
 * \param lpm_mode  LPM_PMIC_SOC_OFF or LPM_PMIC_IO_ONLY_PLUS_DDR.
 */
static void Lpm_pmicConfigureWakeupGpio(uint32_t lpm_mode);

/**
 * \brief Assert the DDR retention signal(s) on the appropriate PMIC for this SoC.
 */
static void Lpm_pmicAssertDdrRetention(void);

/**
 * \brief Write \a val to register \a reg on PMIC A via the baremetal I2C driver.
 */
static void Lpm_pmicWritePmicA(uint8_t reg, uint8_t val);

/**
 * \brief Write \a val to register \a reg on PMIC B via the baremetal I2C driver.
 */
static void Lpm_pmicWritePmicB(uint8_t reg, uint8_t val);

/**
 * \brief Read one byte from register \a reg on PMIC A.
 *
 * \return The byte value read from the register.
 */
static uint8_t Lpm_pmicReadPmicA(uint8_t reg);

/**
 * \brief Read one byte from register \a reg on PMIC B.
 *
 * \return The byte value read from the register.
 */
static uint8_t Lpm_pmicReadPmicB(uint8_t reg);

/**
 * \brief Clear all pending PMIC interrupt flags by reading each active
 *        sub-register and writing the value back to acknowledge it.
 *        Must be called before transitioning to SOC_OFF or IO Retention mode.
 */
static void Lpm_pmicClearInterrupts(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void Lpm_pmicSetupForLpm(uint32_t lpm_mode)
{
    /*
     * Prepare the PMIC to enter the requested low-power mode:
     *
     *  1. Put the PMIC FSM into Active state so subsequent register writes
     *     are not blocked by an in-progress state transition.
     *
     *  2. Clear all pending PMIC interrupts; any unacknowledged interrupt
     *     will prevent the FSM from completing its next state change.
     *
     *  3. Configure FSM trigger registers for the target mode:
     *       - LPM_PMIC_SOC_OFF:            Arm  the FSM_I2C_TRIGGERS for full power-off.
     *       - LPM_PMIC_IO_ONLY_PLUS_DDR:   configure GPIO_RET and DDR_RET signals
     *                                      for IO retention with DDR kept powered.
     *
     *  4. Configure the wakeup GPIO (GPIO4) on PMIC A as the LP_WKUP1 input and
     *     unmask the appropriate edge interrupt for the chosen mode.
     *
     *  5. Assert the DDR retention signal(s) on the correct PMIC (topology varies
     *     by SoC: PMIC-B for J7200/J721S2, PMIC-A for J784S4/J742S2).
     *
     *  6. Write the suspend magic word to the scratch pad register
     *     (SOC_OFF mode only) so bootloader can detect resume vs cold-boot.
     *
     *  7. Drive FSM_NSLEEP_TRIGGERS low to command the PMIC into the target
     *     retention or power-off state.
     */

    Lpm_pmicActivateAndClearInterrupts();
    Lpm_pmicConfigureFsmTriggers(lpm_mode);
    Lpm_pmicConfigureWakeupGpio(lpm_mode);
    Lpm_pmicAssertDdrRetention();

    if (lpm_mode == LPM_PMIC_SOC_OFF)
    {
        /* Write magic number to scratch register to indicate the suspend */
        Lpm_pmicWritePmicA(LPM_PMIC_SCRATCH_PAD_REG_3, LPM_PMIC_SCRATCH_PAD_REG_3_MAGIC_SUSPEND);
        Lpm_pmicDebugReadPmicA(LPM_PMIC_SCRATCH_PAD_REG_3);
    }

    /* Drive FSM_NSLEEP_TRIGGERS low to command the PMIC into the target retention or power-off state */
    Lpm_pmicWritePmicA(LPM_PMIC_FSM_NSLEEP_TRIGGERS, LPM_PMIC_RETENTION_STATE);
}

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static void Lpm_pmicActivateAndClearInterrupts(void)
{
    /* Change FSM_NSLEEP_TRIGGERS: NSLEEP1=high, NSLEEP2=high to put PMIC in Active mode */
    Lpm_pmicWritePmicA(LPM_PMIC_FSM_NSLEEP_TRIGGERS, LPM_PMIC_ACTIVE_STATE);
    Lpm_uartDebugFullPrintf("Lpm_pmicSetupA: Write FSM_NSLEEP_TRIGGERS\n");
    Lpm_pmicDebugReadPmicA(LPM_PMIC_FSM_NSLEEP_TRIGGERS);
    #if defined(SOC_J721S2)
    /* J721S2 has a second PMIC; ensure it is also in Active state before the sequence */
    Lpm_pmicWritePmicB(LPM_PMIC_FSM_NSLEEP_TRIGGERS, LPM_PMIC_ACTIVE_STATE);
    Lpm_uartDebugFullPrintf("Lpm_pmicSetupB: Write FSM_NSLEEP_TRIGGERS\n");
    Lpm_pmicDebugReadPmicB(LPM_PMIC_FSM_NSLEEP_TRIGGERS);
    #endif

    /* Clear all the interrupts of the PMIC else state transition will be blocked */
    Lpm_i2cConfigWkup(BOARD_I2C_LEO_PMIC_A_ADDR);
    Lpm_pmicClearInterrupts();
    #if defined(SOC_J7200) || defined(SOC_J721S2)
    /* J7200 and J721S2 have a second PMIC; clear its interrupts as well */
    Lpm_i2cConfigWkup(BOARD_I2C_LEO_PMIC_B_ADDR);
    Lpm_pmicClearInterrupts();
    #endif
}

static void Lpm_pmicConfigureFsmTriggers(uint32_t lpm_mode)
{
    if (lpm_mode == LPM_PMIC_SOC_OFF)
    {
        /* Change SCICLIENT_LPM_FSM_I2C_TRIGGERS to arm the PMIC for entry into DDR retention state */
        Lpm_pmicWritePmicA(LPM_PMIC_FSM_I2C_TRIGGERS, LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET);
        Lpm_uartDebugFullPrintf("Lpm_pmicSetupA: Write FSM_TRIGGERS\n");
        Lpm_pmicDebugReadPmicA(LPM_PMIC_FSM_I2C_TRIGGERS);

        /* J7200 and J721S2 have a second pmic, so configure it as well */
        #if defined(SOC_J7200) || defined(SOC_J721S2)
        Lpm_pmicWritePmicB(LPM_PMIC_FSM_I2C_TRIGGERS, LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET);
        Lpm_uartDebugFullPrintf("Lpm_pmicSetupB: Write FSM_TRIGGERS\n");
        Lpm_pmicDebugReadPmicB(LPM_PMIC_FSM_I2C_TRIGGERS);
        #endif
    }
    else if (lpm_mode == LPM_PMIC_IO_ONLY_PLUS_DDR)
    {
        #if defined(SOC_J7200)
        /* J7200 has two PMICs and GPIO_RET_I2C_6 sets the IO retention state */
        Lpm_pmicWritePmicA(LPM_PMIC_FSM_I2C_TRIGGERS,
                           (LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET_I2C_7_MASK |
                            LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK));

        Lpm_pmicWritePmicB(LPM_PMIC_FSM_I2C_TRIGGERS,
                           (LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET_I2C_7_MASK |
                            LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK));
        #elif defined(SOC_J784S4) || defined(SOC_J742S2)
        /* J784s4 has one PMIC and GPIO_RET_I2C_5 sets the IO retention state */
        Lpm_pmicWritePmicA(LPM_PMIC_FSM_I2C_TRIGGERS,
                           (LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET_I2C_7_MASK |
                            LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_5_MASK));
        #elif defined(SOC_J721S2)
        /* J721S2 has two PMICs; GPIO_RET_I2C_6 sets the io retention state on both */
        Lpm_pmicWritePmicA(LPM_PMIC_FSM_I2C_TRIGGERS,
                           (LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET_I2C_7_MASK |
                            LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK));
        Lpm_pmicWritePmicB(LPM_PMIC_FSM_I2C_TRIGGERS,
                           (LPM_PMIC_FSM_I2C_TRIGGERS_DDR_RET_I2C_7_MASK |
                            LPM_PMIC_FSM_I2C_TRIGGERS_GPIO_RET_I2C_6_MASK));
        #else
        #error "Unsupported SoC for LPM"
        #endif
    }
}

static void Lpm_pmicConfigureWakeupGpio(uint32_t lpm_mode)
{
    /* Configure GPIO4_CONF as input, open-drain, pull-down enabled and LP_WKUP1 function
     * the LP_WKUP1 function is a trigger that commands the PMIC to transition to Active state.
     */
    Lpm_pmicWritePmicA(LPM_PMIC_GPIO4_CONF,
                       LPM_PMIC_GPIOx_CONF_GPIO_OD_MASK       |
                       LPM_PMIC_GPIOx_CONF_GPIO_PU_PD_EN_MASK |
                       LPM_PMIC_GPIOx_CONF_GPIO_SEL_LP_WKUP1);

    Lpm_uartDebugFullPrintf("Lpm_pmicSetupA: Write GPIO4_CONF\n");
    Lpm_pmicDebugReadPmicA(LPM_PMIC_GPIO4_CONF);

    if (lpm_mode == LPM_PMIC_SOC_OFF)
    {
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO9_11, LPM_PMIC_MASK_ALL_GPIO9_11);
        #if defined(SOC_J721S2)
        /* On J721S2 evm, the wakeup signal is a falling edge on GPIO4 */
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO1_8_RISE, LPM_PMIC_MASK_ALL_GPIO1_8);
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO1_8_FALL, LPM_PMIC_MASK_GPIO4_ONLY);
        #elif defined(SOC_J784S4) || defined(SOC_J742S2) || defined(SOC_J7200)
        /* Unmask only GPIO4 rising interrupts because in SOC_OFF mode
         * the wakeup signal is a rising edge for J784S4, J742S2 and J7200 EVMs.
         */
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO1_8_RISE, LPM_PMIC_MASK_GPIO4_ONLY);
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO1_8_FALL, LPM_PMIC_MASK_ALL_GPIO1_8);
        #else
        #error "Unsupported SoC for LPM"
        #endif

    }
    else if (lpm_mode == LPM_PMIC_IO_ONLY_PLUS_DDR)
    {
        /* Unmask only GPIO4 falling interrupts because in IO_ONLY_PLUS_DDR mode
         * the wakeup signal is a falling edge.
         */
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO1_8_RISE, LPM_PMIC_MASK_ALL_GPIO1_8);
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO1_8_FALL, LPM_PMIC_MASK_GPIO4_ONLY);
        Lpm_pmicWritePmicA(LPM_PMIC_MASK_GPIO9_11, LPM_PMIC_MASK_ALL_GPIO9_11);
    }

    #if defined(SOC_J721S2)
        /* Disable all GPIO interrupts on PMIC B, as only PMIC A receives the wake signal */
        Lpm_pmicWritePmicB(LPM_PMIC_MASK_GPIO1_8_RISE, LPM_PMIC_MASK_ALL_GPIO1_8);
        Lpm_pmicWritePmicB(LPM_PMIC_MASK_GPIO1_8_FALL, LPM_PMIC_MASK_ALL_GPIO1_8);
        Lpm_pmicWritePmicB(LPM_PMIC_MASK_GPIO9_11, LPM_PMIC_MASK_ALL_GPIO9_11);
    #endif
}

static void Lpm_pmicAssertDdrRetention(void)
{
    uint8_t buf;

    #if defined(SOC_J7200)
    /* J7200 uses a two-PMIC topology (Leo PMIC-A + PMIC-B).
     * The DDR retention signal is driven by PMIC-B GPIO2 (DDR_RET_VAL) and GPIO3 (DDR_RET_CLK).
     * The sequence is: assert DDR_RET_VAL, then toggle DDR_RET_CLK high-low-high
     * to latch the retention request into the DDR controller.
     */
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO2_CONF, LPM_PMIC_GPIOx_CONF_GPIO_DIR_MASK);
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO3_CONF, LPM_PMIC_GPIOx_CONF_GPIO_DIR_MASK);

    buf = Lpm_pmicReadPmicB(LPM_PMIC_GPIO_OUT_1) | LPM_PMIC_GPIO_OUT_1_DDR_RET_VAL_MASK;
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO_OUT_1, buf);

    /* Toggle DDR_RET_CLK to latch retention request in DDR controller */
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO_OUT_1, buf | LPM_PMIC_GPIO_OUT_1_DDR_RET_CLK_MASK);
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO_OUT_1, buf & ~LPM_PMIC_GPIO_OUT_1_DDR_RET_CLK_MASK);
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO_OUT_1, buf | LPM_PMIC_GPIO_OUT_1_DDR_RET_CLK_MASK);
    #elif defined(SOC_J721S2)
    /* J721S2: DDR retention is signalled by asserting GPIO3 and GPIO4 high on PMIC B */
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO3_CONF, LPM_PMIC_GPIOx_CONF_GPIO_DIR_MASK);
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO4_CONF, LPM_PMIC_GPIOx_CONF_GPIO_DIR_MASK);
    buf = Lpm_pmicReadPmicB(LPM_PMIC_GPIO_OUT_1)           |
                            LPM_PMIC_GPIO_OUT_1_GPIO3_MASK |
                            LPM_PMIC_GPIO_OUT_1_GPIO4_MASK;
    Lpm_pmicWritePmicB(LPM_PMIC_GPIO_OUT_1, buf);
    #elif defined(SOC_J784S4) || defined(SOC_J742S2)
    /* J784S4/J742S2 use a single Leo PMIC-A.
     * DDR retention is enabled by asserting EN_DDR_RET_1V1 (GPIO6) high,
     * which puts the 1.1 V DDR supply into retention mode. No clock-toggle sequence is needed.
     */
    Lpm_pmicWritePmicA(LPM_PMIC_GPIO6_CONF, LPM_PMIC_GPIOx_CONF_GPIO_DIR_MASK);

    buf = Lpm_pmicReadPmicA(LPM_PMIC_GPIO_OUT_1) | LPM_PMIC_GPIO_OUT_1_EN_DDR_RET_1V1_MASK;
    Lpm_pmicWritePmicA(LPM_PMIC_GPIO_OUT_1, buf);
    #else
    #error "Unsupported SoC for LPM"
    #endif
}

static void Lpm_pmicWritePmicA(uint8_t reg, uint8_t val)
{
    Lpm_i2cConfigWkup(BOARD_I2C_LEO_PMIC_A_ADDR);
    Lpm_i2cWrite(reg, val);
    Lpm_uartDebugFullPrintf("Lpm_pmicWritePmicA: reg=0x%x 0x%x\n", reg, val);
}

static void Lpm_pmicWritePmicB(uint8_t reg, uint8_t val)
{
    Lpm_i2cConfigWkup(BOARD_I2C_LEO_PMIC_B_ADDR);
    Lpm_i2cWrite(reg, val);
    Lpm_uartDebugFullPrintf("Lpm_pmicWritePmicB: reg=0x%x 0x%x\n", reg, val);
}

static uint8_t Lpm_pmicReadPmicA(uint8_t reg)
{
    uint8_t rxd;

    Lpm_i2cConfigWkup(BOARD_I2C_LEO_PMIC_A_ADDR);
    rxd = Lpm_i2cRead(reg);
    Lpm_uartDebugFullPrintf("Lpm_pmicReadPmicA: reg=0x%x val=0x%x\n", reg, rxd);

    return rxd;
}

static uint8_t Lpm_pmicReadPmicB(uint8_t reg)
{
    uint8_t rxd;

    Lpm_i2cConfigWkup(BOARD_I2C_LEO_PMIC_B_ADDR);
    rxd = Lpm_i2cRead(reg);
    Lpm_uartDebugFullPrintf("Lpm_pmicReadPmicB: reg=0x%x val=0x%x\n", reg, rxd);

    return rxd;
}

static void Lpm_pmicClearInterrupts(void)
{
    /* All pending PMIC interrupts must be cleared before transitioning to
     * low power mode. If any interrupt remains asserted when the NSLEEP
     * triggers are written, the PMIC FSM will not enter the retention state
     * and the transition will silently fail.
     */

    uint8_t intr_top, val, val1;

    intr_top = Lpm_i2cRead(LPM_PMIC_INT_TOP);

    if(0U == intr_top)
    {
        /* No PMIC intterupts are pending, hence we dont need to clear anything */
    }
    else
    {
        /* Clear buck converter interrupts (BUCK1/2, BUCK3/4, BUCK5) */
        if((intr_top & LPM_PMIC_INT_TOP_BUCK_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_BUCK);
            Lpm_uartDebugFullPrintf("INT_BUCK = 0x%02X\n", val);
            /* Clear BUCK1/2 interrupt */
            if ((val & LPM_PMIC_INT_BUCK_BUCK1_2_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_BUCK1_2);
                Lpm_uartDebugFullPrintf("INT_BUCK1_2 = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_BUCK1_2, val1);
            }

            /* Clear BUCK3/4 interrupt */
            if((val & LPM_PMIC_INT_BUCK_BUCK3_4_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_BUCK3_4);
                Lpm_uartDebugFullPrintf("INT_BUCK3_4 = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_BUCK3_4, val1);
            }

            /* Clear BUCK5 interrupt */
            if((val & LPM_PMIC_INT_BUCK_BUCK5_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_BUCK5);
                Lpm_uartDebugFullPrintf("INT_BUCK5 = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_BUCK5, val1);
            }
        }

        /* Clear LDO and voltage monitor interrupts (LDO1/2, LDO3/4, VMON) */
        if((intr_top & LPM_PMIC_INT_TOP_LDO_VMON_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_LDO_VMON);
            Lpm_uartDebugFullPrintf("INT_LDO_VMON = 0x%02X\n", val);
            /* Clear LDO1/2 interrupt */
            if((val & LPM_PMIC_INT_LDO_VMON_LDO1_2_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_LDO1_2);
                Lpm_uartDebugFullPrintf("INT_LDO1_2 = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_LDO1_2, val1);
            }

            /* Clear LDO3/4 interrupt */
            if((val & LPM_PMIC_INT_LDO_VMON_LDO3_4_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_LDO3_4);
                Lpm_uartDebugFullPrintf("INT_LDO3_4 = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_LDO3_4, val1);
            }

            /* Clear voltage monitor interrupt */
            if((val & LPM_PMIC_INT_LDO_VMON_VMON_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_VMON);
                Lpm_uartDebugFullPrintf("INT_VMON = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_VMON, val1);
            }
        }

        /* Clear GPIO interrupts (GPIO1-8 group, GPIO9-11) */
        if((intr_top & LPM_PMIC_INT_TOP_GPIO_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_GPIO);
            Lpm_uartDebugFullPrintf("INT_GPIO = 0x%02X\n", val);

            /* Clear GPIO1-8 interrupt */
            if((val & LPM_PMIC_INT_GPIO_GPIO1_8_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_GPIO1_8);
                Lpm_uartDebugFullPrintf("INT_GPIO1_8 = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_GPIO1_8, val1);
            }

            /* Clear GPIO9, GPIO10, GPIO11 */
            Lpm_i2cWrite(LPM_PMIC_INT_GPIO, val);
        }

        /* Clear startup interrupt (e.g. NPWRON_START) */
        if((intr_top & LPM_PMIC_INT_TOP_STARTUP_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_STARTUP);
            Lpm_uartDebugFullPrintf("INT_STARTUP = 0x%02X\n", val);
            Lpm_i2cWrite(LPM_PMIC_INT_STARTUP, val);
        }

        /* Clear miscellaneous interrupt */
        if((intr_top & LPM_PMIC_INT_TOP_MISC_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_MISC);
            Lpm_uartDebugFullPrintf("INT_MISC = 0x%02X\n", val);
            Lpm_i2cWrite(LPM_PMIC_INT_MISC, val);
        }

        /* Clear moderate error interrupt */
        if((intr_top & LPM_PMIC_INT_TOP_MODERATE_ERR_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_MODERATE_ERR);
            Lpm_uartDebugFullPrintf("INT_MODERATE_ERR = 0x%02X\n", val);
            Lpm_i2cWrite(LPM_PMIC_INT_MODERATE_ERR, val);
        }

        /* Clear severe error interrupt */
        if((intr_top & LPM_PMIC_INT_TOP_SEVERE_ERR_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_SEVERE_ERR);
            Lpm_uartDebugFullPrintf("INT_SEVERE_ERR = 0x%02X\n", val);
            Lpm_i2cWrite(LPM_PMIC_INT_SEVERE_ERR, val);
        }

        /* Clear FSM error interrupt and its sub-errors (comm, readback, ESM) */
        if((intr_top & LPM_PMIC_INT_TOP_FSM_ERR_MASK) != 0U)
        {
            val = Lpm_i2cRead(LPM_PMIC_INT_FSM_ERR);
            Lpm_uartDebugFullPrintf("INT_FSM_ERR = 0x%02X\n", val);
            /* Clear I2C/SPI communication error */
            if((val & LPM_PMIC_INT_FSM_ERR_COMM_ERR_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_COMM_ERR);
                Lpm_uartDebugFullPrintf("INT_COMM_ERR = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_COMM_ERR, val1);
            }

            /* Clear register readback error */
            if((val & LPM_PMIC_INT_FSM_ERR_READBACK_ERR_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_READBACK_ERR);
                Lpm_uartDebugFullPrintf("INT_READBACK_ERR = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_READBACK_ERR, val1);
            }

            /* Clear error signal monitor (ESM) error */
            if((val & LPM_PMIC_INT_FSM_ERR_ESM_MASK) != 0U)
            {
                val1 = Lpm_i2cRead(LPM_PMIC_INT_ESM_ERR);
                Lpm_uartDebugFullPrintf("INT_ESM = 0x%02X\n", val1);
                Lpm_i2cWrite(LPM_PMIC_INT_ESM_ERR, val1);
            }

            /* Clear INT_FSM_ERR */
            Lpm_i2cWrite(LPM_PMIC_INT_FSM_ERR, val);
        }

        intr_top = Lpm_i2cRead(LPM_PMIC_INT_TOP);
    }

    Lpm_uartDebugFullPrintf("INT_TOP = 0x%02X\n", intr_top);
}
