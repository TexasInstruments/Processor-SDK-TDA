/*
 *  Copyright (c) Texas Instruments Incorporated 2022-2024
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

/*!
 * \file     board_pinmux_data.c
 *
 * \brief    This file contains the pinmux settings needed by EthFw on
 *           J721E EVM.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x402

#include <ti/csl/soc.h>
#include <ti/board/board.h>
#include <ti/board/src/j721e_evm/include/board_pinmux.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/** Peripheral Pin Configurations */


static pinmuxPerCfg_t gDebugss0PinCfg[] =
{
    /* MyDEBUG1 -> TDI -> V1 */
    {
        PIN_TDI, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyDEBUG1 -> TDO -> V3 */
    {
        PIN_TDO, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
    /* MyDEBUG1 -> TMS -> V2 */
    {
        PIN_TMS, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    {PINMUX_END}
};

static pinmuxModuleCfg_t gDebugssPinCfg[] =
{
    {0, 1, gDebugss0PinCfg},
    {PINMUX_END}
};


static pinmuxPerCfg_t gGpio0PinCfg[] =
{
    /* MySYSTEM1 -> GPIO0_0 -> AC18 */
    {
        PIN_EXTINTN, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO0 -> GPIO0_97 -> Y28 */
    {
        PIN_RGMII6_TX_CTL, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO0 -> GPIO0_98 -> V23 */
    {
        PIN_RGMII6_RX_CTL, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO0 -> GPIO0_117 -> W4 */
    {
        PIN_SPI1_CS1, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO0 -> GPIO0_127 -> AC4 */
    {
        PIN_UART1_CTSN, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    {PINMUX_END}
};

static pinmuxPerCfg_t gGpio1PinCfg[] =
{
    /* MyGPIO1 -> GPIO1_0 -> AD5 */
    {
        PIN_UART1_RTSN, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_3 -> W3 */
    {
        PIN_MCAN1_RX, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_5 -> W2 */
    {
        PIN_I3C0_SCL, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_6 -> W1 */
    {
        PIN_I3C0_SDA, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_11 -> U2 */
    {
        PIN_ECAP0_IN_APWM_OUT, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_12 -> U3 */
    {
        PIN_EXT_REFCLK1, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_22 -> R28 */
    {
        PIN_MMC1_SDWP, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_23 -> T28 */
    {
        PIN_MMC2_DAT3, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_24 -> T29 */
    {
        PIN_MMC2_DAT2, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_25 -> T27 */
    {
        PIN_MMC2_DAT1, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO1_26 -> T24 */
    {
        PIN_MMC2_DAT0, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    {PINMUX_END}
};

/* Used for  reset signal for PRG0/PRG1 Ethernet PHYs and RMII phy */
static pinmuxPerCfg_t gGpio2PinCfg_phyReset[] =
{
    /* MyGPIO1 -> GPIO0_61 -> AB29 */
    {
        PIN_PRG0_PRU0_GPO18, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyGPIO1 -> GPIO0_62 -> AB28 */
    {
        PIN_PRG0_PRU0_GPO19, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },

    /* MyGPIO1 -> GPIO0_96 -> T23 */
    {
        PIN_RGMII5_RD0, PIN_MODE(7) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    {PINMUX_END}
};

static pinmuxModuleCfg_t gGpioPinCfg[] =
{
    {0, 1, gGpio0PinCfg},
    {1, 1, gGpio1PinCfg},
    {2, 1, gGpio2PinCfg_phyReset},
    {PINMUX_END}
};

static pinmuxPerCfg_t gI2c0PinCfg[] =
{
    /* MyI2C0 -> I2C0_SCL -> AC5 */
    {
        PIN_I2C0_SCL, PIN_MODE(0) | \
        ((PIN_PULL_DIRECTION | PIN_INPUT_ENABLE) & (~PIN_PULL_DISABLE))
    },
    /* MyI2C0 -> I2C0_SDA -> AA5 */
    {
        PIN_I2C0_SDA, PIN_MODE(0) | \
        ((PIN_PULL_DIRECTION | PIN_INPUT_ENABLE) & (~PIN_PULL_DISABLE))
    },
    {PINMUX_END}
};

static pinmuxModuleCfg_t gI2cPinCfg[] =
{
    {0, 1, gI2c0PinCfg},
    {PINMUX_END}
};

static pinmuxPerCfg_t gSystem0PinCfg[] =
{
    /* MySYSTEM1 -> AUDIO_EXT_REFCLK2 -> W26 */
    {
        PIN_RGMII6_RXC, PIN_MODE(3) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MySYSTEM1 -> OBSCLK0 -> V5 */
    {
        PIN_TIMER_IO1, PIN_MODE(2) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
    /* MySYSTEM1 -> PORz_OUT -> U1 */
    {
        PIN_PORZ_OUT, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
    /* MySYSTEM1 -> RESETSTATz -> T6 */
    {
        PIN_RESETSTATZ, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
    /* MySYSTEM1 -> SOC_SAFETY_ERRORn -> U4 */
    {
        PIN_SOC_SAFETY_ERRORN, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MySYSTEM1 -> SYSCLKOUT0 -> V6 */
    {
        PIN_TIMER_IO0, PIN_MODE(2) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
#if defined(ETHFW_PPS_DEMO_SUPPORT)
    {
        PIN_RGMII5_TD3, PIN_MODE(3) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
#endif
    {PINMUX_END}
};

static pinmuxModuleCfg_t gSystemPinCfg[] =
{
    {0, 1, gSystem0PinCfg},
    {PINMUX_END}
};

static pinmuxPerCfg_t gUart2PinCfg[] =
{
    /* MyUART2 -> UART2_RXD -> Y1 */
    {
        PIN_SPI1_CLK, PIN_MODE(3) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyUART2 -> UART2_TXD -> Y5 */
    {
        PIN_SPI1_D0, PIN_MODE(3) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
    {PINMUX_END}
};

static pinmuxModuleCfg_t gUartPinCfg[] =
{
    {2, 1, gUart2PinCfg},
    {PINMUX_END}
};

static pinmuxPerCfg_t gWkup_i2c0PinCfg[] =
{
    /* MyWKUP_I2C0 -> WKUP_I2C0_SCL -> J25 */
    {
        PIN_WKUP_I2C0_SCL, PIN_MODE(0) | \
        ((PIN_PULL_DIRECTION | PIN_INPUT_ENABLE) & (~PIN_PULL_DISABLE))
    },
    /* MyWKUP_I2C0 -> WKUP_I2C0_SDA -> H24 */
    {
        PIN_WKUP_I2C0_SDA, PIN_MODE(0) | \
        ((PIN_PULL_DIRECTION | PIN_INPUT_ENABLE) & (~PIN_PULL_DISABLE))
    },
    {PINMUX_END}
};

static pinmuxModuleCfg_t gWkup_i2cPinCfg[] =
{
    {0, 1, gWkup_i2c0PinCfg},
    {PINMUX_END}
};

pinmuxBoardCfg_t gEthFwPinmuxData[] =
{
    {0, gWkup_i2cPinCfg},
    {1, gDebugssPinCfg},
    {2, gGpioPinCfg},
    {3, gI2cPinCfg},
    {4, gSystemPinCfg},
    {5, gUartPinCfg},
    {PINMUX_END}
};


