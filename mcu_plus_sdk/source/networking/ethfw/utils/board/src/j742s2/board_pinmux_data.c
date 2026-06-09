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
 *           J742S2 EVM.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x405

#include <ti/csl/soc.h>
#include <ti/board/board.h>
#include <ti/board/src/j742s2_evm/include/board_pinmux.h>

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

static pinmuxPerCfg_t gCpsw9x0PinCfg[] =
{
    /* MyCPSW9X -> MDIO1_MDIO -> AC36 */
    {
        PIN_MCASP2_AXR0, PIN_MODE(4) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyCPSW9X -> MDIO1_MDC -> AE37 */
    {
        PIN_MCASP2_AFSX, PIN_MODE(4) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    {PINMUX_END}
};

static pinmuxModuleCfg_t gCpsw9xPinCfg[] =
{
    {0, 1, gCpsw9x0PinCfg},
    {PINMUX_END}
};

static pinmuxPerCfg_t gUart2PinCfg[] =
{
    /* MyUART2 -> UART2_RXD -> AM35 */
    {
        PIN_SPI0_D0, PIN_MODE(11) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyUART2 -> UART2_TXD -> AM36 */
    {
        PIN_SPI0_D1, PIN_MODE(11) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    {PINMUX_END}
};

static pinmuxModuleCfg_t gUartPinCfg[] =
{
    {2, 1, gUart2PinCfg},
    {PINMUX_END}
};

static pinmuxPerCfg_t gI2c0PinCfg[] =
{
    /* MyI2C0 -> I2C0_SCL -> AN36 */
    {
        PIN_I2C0_SCL, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
    },
    /* MyI2C0 -> I2C0_SDA -> AP37 */
    {
        PIN_I2C0_SDA, PIN_MODE(0) | \
        ((PIN_PULL_DISABLE | PIN_INPUT_ENABLE) & (~PIN_PULL_DIRECTION))
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
#if defined(ETHFW_PPS_DEMO_SUPPORT)
    {
        PIN_MCASP2_ACLKX, PIN_MODE(14) | \
        ((PIN_PULL_DISABLE) & (~PIN_PULL_DIRECTION & ~PIN_INPUT_ENABLE))
    },
#endif
    {PINMUX_END}
};

static pinmuxModuleCfg_t gSystemPinCfg[] =
{
    {2, 1, gSystem0PinCfg},
    {PINMUX_END}
};

pinmuxBoardCfg_t gEthFwPinmuxData[] =
{
    {0, gCpsw9xPinCfg},
    {1, gI2cPinCfg},
    {2, gUartPinCfg},
    {3, gSystemPinCfg},
    {PINMUX_END}
};
