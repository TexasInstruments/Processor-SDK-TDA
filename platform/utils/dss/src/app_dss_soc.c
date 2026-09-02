/*
 *  Copyright (c) Texas Instruments Incorporated 2018-2026
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
 *  \file app_utils.c
 *
 *  \brief DSS example utility APIs for J7
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "app_dss_defaults_priv.h"
#include <utils/sciclient/include/app_sciclient_wrapper_api.h>

#include <i2c/i2c.h>
#include <i2c/soc/i2c_soc.h>

#include <ti/board/src/devices/board_devices.h>
#include <ti/board/board.h>
#include <ti/board/board_cfg.h>

#if defined (SOC_J721E)
#include <ti/board/src/j721e_evm/include/board_control.h>
#include <ti/board/src/j721e_evm/include/board_cfg.h>
#include <ti/board/src/j721e_evm/include/board_pinmux.h>
#include <ti/board/src/j721e_evm/include/board_i2c_io_exp.h>
#elif defined (SOC_J721S2)
#include <ti/board/src/j721s2_evm/include/board_control.h>
#include <ti/board/src/j721s2_evm/include/board_cfg.h>
#include <ti/board/src/j721s2_evm/include/board_pinmux.h>
#include <ti/board/src/j721s2_evm/include/board_i2c_io_exp.h>
#elif defined (SOC_J784S4)
#include <ti/board/src/j784s4_evm/include/board_control.h>
#include <ti/board/src/j784s4_evm/include/board_cfg.h>
#include <ti/board/src/j784s4_evm/include/board_pinmux.h>
#include <ti/board/src/j784s4_evm/include/board_i2c_io_exp.h>
#elif defined (SOC_J742S2)
#include <ti/board/src/j742s2_evm/include/board_control.h>
#include <ti/board/src/j742s2_evm/include/board_cfg.h>
#include <ti/board/src/j742s2_evm/include/board_pinmux.h>
#include <ti/board/src/j742s2_evm/include/board_i2c_io_exp.h>
#endif

/* ========================================================================== */
/*                             Global Variables                               */
/* ========================================================================== */

#define VX_DSI_RESO_1080P  (FVID2_STD_1080P_60)
#define VX_DSI_RESO_720P   (FVID2_STD_720P_60)
#define VX_APP_I2C_TIMEOUT (100U)

/* ========================================================================== */
/*                             Global Variables                               */
/* ========================================================================== */

static I2C_Handle gI2cHandle = NULL;

uint8_t Ub941Ub925Config[][4] = {
{0x16, 0x01, 0x0A, 0x5},
{0x16, 0x03, 0x9A, 0x5},
{0x16, 0x17, 0x9E, 0x5},
{0x16, 0x07, 0x58, 0x5},
{0x16, 0x08, 0x22, 0x5},
{0x16, 0x70, 0x80, 0x5},
{0x16, 0x77, 0x24, 0x5},


{0x16, 0x01, 0x08, 0x5},
{0x16, 0x1E, 0x01, 0x5},
{0x16, 0x03, 0x9A, 0x5},
{0x16, 0x03, 0x9A, 0x5},
{0x16, 0x03, 0x9A, 0x5},
{0x16, 0x40, 0x04, 0x5},
{0x16, 0x40, 0x05, 0x5},
{0x16, 0x41, 0x21, 0x5},
{0x16, 0x42, 0x60, 0x5},
{0x16, 0x40, 0x09, 0x5},
{0x16, 0x40, 0x09, 0x5},
{0x16, 0x41, 0x21, 0x5},
{0x16, 0x42, 0x60, 0x5},
{0x16, 0x5b, 0x85, 0x5},
{0x16, 0x4f, 0x8c, 0x5},
{0x16, 0x4f, 0x84, 0x5},
{0x16, 0x40, 0x05, 0x5},
{0x16, 0x40, 0x04, 0x5},
{0x16, 0x41, 0x05, 0x5},
{0x16, 0x42, 0x16, 0x5},
{0x16, 0x40, 0x08, 0x5},
{0x16, 0x40, 0x08, 0x5},
{0x16, 0x41, 0x05, 0x5},
{0x16, 0x42, 0x0c, 0x5},
{0x16, 0x01, 0x00, 0x5},
{0x16, 0x66, 0x03, 0x5},
{0x16, 0x67, 0x03, 0x5},
{0x16, 0x65, 0x01, 0x5},
{0x16, 0x64, 0x00, 0x5},
{0x16, 0x64, 0x04, 0x5},


{0x27, 0x00, 0xFE, 0x5},

{0x11, 0x1D, 0x28, 0x5},
{0x11, 0x1D, 0x29, 0x5},

{0x11, 0x01, 0x06, 0x5},
{0x11, 0x01, 0x04, 0x5},
{0x11, 0x03, 0xf0, 0x5},
{0x11, 0x03, 0xf0, 0x5},
{0x11, 0x03, 0xf8, 0x5},
{0x11, 0x29, 0x00, 0x5},
{0x11, 0x29, 0x00, 0x5},
{0x11, 0x65, 0x00, 0x5},
{0x11, 0x65, 0x00, 0x5},

{0x12, 0x0C, 0x20, 0x5},
{0x12, 0x00, 0x01, 0x5},
{0x12, 0x04, 0xE6, 0x5},

};

/* SN65DSI bridge configurations. Default is for 720p. 
 * {addr, value, delay}
 */
uint8_t gI2cDsiBridgeCfg[][3] = {
{0xFF, 0x7, 200U},
{0x16, 0x1, 200U},
{0xFF, 0x0, 200U},
{0x0A, 0x2, 200U},
{0x10, 0x36, 200U},
{0x12, 0x4C, 200U},
{0x13, 0x4C, 200U},
{0x94, 0x80, 200U},
{0x0D, 0x1, 200U},
{0x5A, 0x4, 200U},
{0x5F, 0x28, 200U},
{0x93, 0x10, 200U},
{0x96, 0x0A, 200U},
{0x20, 0x00, 200U},
{0x21, 0x05, 200U},
{0x22, 0x0, 200U},
{0x23, 0x0, 200U},
{0x24, 0xD0, 200U},
{0x25, 0x02, 200U},
{0x2C, 0x20, 200U},
{0x2D, 0x00, 200U},
{0x30, 0x05, 200U},
{0x31, 0x00, 200U},
{0x34, 0x50, 200U},
{0x36, 0x0C, 200U},
{0x38, 0x30, 200U},
{0x3A, 0x04, 200U},
{0x5B, 0x0, 200U},
{0x3C, 0x00, 200U},
{0x5A, 0x0C, 200U},
};

/* ========================================================================== */
/*                             Local Function Declaration                     */
/* ========================================================================== */

static int32_t appDssDsiInitI2c();
#if defined (SOC_J721E)
static int32_t appDssDsiSetBoardMux();
#elif defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
static uint32_t DispApp_ErrorRegRead();
#endif

/* ========================================================================== */
/*                             Function Defination                            */
/* ========================================================================== */

int32_t appDssDsiConfigureSN65(uint32_t resolution);
void appDssConfigurePm(app_dss_default_prm_t *prm)
{
    appLogPrintf("DSS: SoC init ... !!!\n");

    /* power on DSS */
    #if defined (SOC_J721E)
        SET_DEVICE_STATE_ON(TISCI_DEV_DSS0);
    #elif defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
        SET_DEVICE_STATE(TISCI_DEV_DSS0, TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF);
    #endif

    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
    {
        /* power on modules required for eDP */
        SET_DEVICE_STATE_ON(TISCI_DEV_SERDES_10G0);
        SET_DEVICE_STATE_ON(TISCI_DEV_DSS_EDP0);
    }
    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        /* power on modules required for DSI */
        SET_DEVICE_STATE_ON(TISCI_DEV_DSS_DSI0);
        SET_DEVICE_STATE_ON(TISCI_DEV_DPHY_TX0);
    }

    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
    {
        /*
     * initialize PLL for VP0
     *
     * Summary:
     * VP0 pclk can either come from PLL16 HSDIV0, or VP1's pclk.
     * Since we are not driving cloned displays, it is advised to
     * have independent clocking for VP0 and VP1
     * However, by default clocking scheme VP2 pclk is also
     * sourced from PLL16 HSDIV0.
     *
     * Therefore, before VP0 pclk's parent is set to PLL16 HSDIV0,
     * it is required to reparent VP2's pclk (to PLL18 / PLL19,
     * preferred PLL18) and then set VP0's parent followed by setting
     * rate to 148.5 MHz
     *
     * XXX Due to SYSFW-2793 setting VP2's pclk parent to PLL18 reparents
     *     VP3's pclk (which is used by linux) to PLL16 HSDIV1.
     *     So reparent VP3's pclk to PLL23 again, else linux will
     *     report synclost
     * XXX Is it still required to have VP3 enabled in Linux for GPU
     *     driver issues?
     */
        #if defined (SOC_J721E)
        SET_CLOCK_PARENT(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_18_HSDIVOUT0_CLK);
        SET_CLOCK_PARENT(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_3_IN_2X_CLK, TISCI_DEV_DSS0_DSS_INST0_DPI_3_IN_2X_CLK_PARENT_DPI1_EXT_CLKSEL_OUT0);
        SET_CLOCK_PARENT(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_0_IN_2X_CLK, TISCI_DEV_DSS0_DSS_INST0_DPI_0_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_16_HSDIVOUT0_CLK);
        SET_CLOCK_FREQ (TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_0_IN_2X_CLK, prm->timings.pixelClock);
        SET_CLOCK_STATE(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_0_IN_2X_CLK, 0, TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ);
        #elif defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
        SET_DEVICE_STATE_ON(TISCI_DEV_SERDES_10G0);
        SET_DEVICE_STATE_ON(TISCI_DEV_DSS_EDP0);
        SET_DEVICE_STATE_OFF(TISCI_DEV_DSS0);
        SET_CLOCK_FREQ_ALLOW_CHANGE(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_0_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_16_HSDIVOUT0_CLK, prm->timings.pixelClock);
        SET_CLOCK_STATE(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_0_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_16_HSDIVOUT0_CLK, 0x2, TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ);
        SET_DEVICE_STATE_ON(TISCI_DEV_DSS0);
        #endif
    }
    else if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI)
    {
        /*
         * initialize PLL for VP1
         *
         * Summary:
         * VP1 pclk must come from PLL19 (for our EVM) because
         * we are routing VP1's output to DPI0, which is retimed
         * using PLL19 HSDIV0 output.
         * Therefore, set parent to PLL19 HSDIV0 and then set rate
         * to 148.5 MHz
         */
        SET_DEVICE_STATE_OFF(TISCI_DEV_DSS0);
        SET_CLOCK_PARENT(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_1_IN_2X_CLK, TISCI_DEV_DSS0_DSS_INST0_DPI_1_IN_2X_CLK_PARENT_DPI0_EXT_CLKSEL_OUT0);
        SET_CLOCK_FREQ (TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_1_IN_2X_CLK, prm->timings.pixelClock);
        SET_CLOCK_STATE(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_1_IN_2X_CLK, 0, TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ);
        SET_DEVICE_STATE_ON(TISCI_DEV_DSS0);
    }
    else if (prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        #if defined (SOC_J721E)
        SET_CLOCK_PARENT(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_18_HSDIVOUT0_CLK);
        SET_CLOCK_FREQ (TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK, prm->timings.pixelClock);
        SET_CLOCK_STATE(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK, 0, TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ);
        #elif defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
        SET_DEVICE_STATE_OFF(TISCI_DEV_DSS0);
        SET_CLOCK_PARENT(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_17_HSDIVOUT0_CLK);
        SET_CLOCK_FREQ_ALLOW_CHANGE(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_17_HSDIVOUT0_CLK, prm->timings.pixelClock);
        SET_CLOCK_STATE(TISCI_DEV_DSS0, TISCI_DEV_DSS0_DSS_INST0_DPI_2_IN_2X_CLK_PARENT_HSDIV1_16FFT_MAIN_17_HSDIVOUT0_CLK, 0x2, TISCI_MSG_VALUE_CLOCK_SW_STATE_REQ);
        SET_DEVICE_STATE_ON(TISCI_DEV_DSS0);
        #endif
    }

    appLogPrintf("DSS: SoC init ... Done !!!\n");
}

void appDssConfigureBoard(app_dss_default_prm_t *prm)
{
    appLogPrintf("DSS: Board init ... !!!\n");
    #if defined(SOC_J721E)
    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DPI_HDMI)
    {
        /* Not need, since default takes care, also this conflicts with Linux */
        /* Board_control(BOARD_CTRL_CMD_SET_HDMI_MUX, (void*) 0U); */

        /* Set board mux for DPI/HDMI, ok to keep this even when using eDP */
        Board_control(BOARD_CTRL_CMD_SET_HDMI_PD_HIGH, (void*) 0U);
    }
    #endif

    /* ADASVISION-4188 - There seem to be an I2C conflict when ETHFW is enabled, so this is disabled when ETHFW is enabled */
    /* If customers are not usign ETHFW then below can be enabled to support eDP to HDMI mode */
#if !defined(ENABLE_ETHFW)
    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_EDP)
    {
        appDssConfigureDP();
    }
#endif

#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
    if(prm->display_type == APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        appDssDsiConfigureSN65(VX_DSI_RESO_1080P);
    }
#endif

    appLogPrintf("DSS: Board init ... Done !!!\n");
}

void appDssConfigureDP(void)
{
    Board_STATUS b_status;
    Board_IoExpCfg_t ioExpCfg;

    appLogPrintf("DSS: Turning on DP_PWR pin for eDP adapters ... !!!\n");

    #if defined (SOC_J721E)
    ioExpCfg.i2cInst     = 1U;
    #elif defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
    ioExpCfg.i2cInst     = 4U;
    #endif
    ioExpCfg.socDomain   = BOARD_SOC_DOMAIN_MAIN;
    ioExpCfg.slaveAddr   = 0x20;
    ioExpCfg.enableIntr  = false;
    ioExpCfg.ioExpType   = ONE_PORT_IOEXP;
    ioExpCfg.portNum     = PORTNUM_0;
    ioExpCfg.pinNum      = PIN_NUM_0;
    ioExpCfg.signalLevel = GPIO_SIGNAL_LEVEL_HIGH;

    b_status = Board_control(BOARD_CTRL_CMD_SET_IO_EXP_PIN_OUT, (void *)(&ioExpCfg));

    #if defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
    appLogWaitMsecs(500u);
    #endif

    if (b_status == BOARD_SOK)
    {
        appLogPrintf("DSS: Turning on DP_PWR pin for eDP adapters ... Done!!!\n");
    }
    else
    {
        appLogPrintf("DSS: ERROR: Turning on DP_PWR pin for eDP adapters failed !!!\n");
    }
}

static int32_t appDssDsiInitI2c()
{
    int32_t status = FVID2_SOK;
    uint8_t domain, i2cInst, slaveAddr;
    I2C_Params i2cParams;
    Board_IoExpCfg_t ioExpCfg;
    Board_STATUS b_status;

    /* Initializes the I2C Parameters */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz; /* 400KHz */

#if defined (SOC_J721E)
    Board_fpdUb941GetI2CAddr(&domain, &i2cInst, &slaveAddr);
#else
    i2cInst = 4;
    #if defined (SOC_J784S4) || defined (SOC_J742S2)
        /* For J784S4, linux does not have ownership of GPIOExp that enables DSI to eDP bridge. 
         * Remote core must take full ownership of the I2C, enable the bridge, and configure it as well.
         * For J721S2, Linux uBoot hogs up the GPIO expander pin that enables the DSI to eDP bridge.
         */
        Board_control(BOARD_CTRL_CMD_ENABLE_DSI2DP_BRIDGE, NULL);
        Osal_delay(1000);
    #endif
#endif
    /* Configures the I2C instance with the passed parameters*/
    gI2cHandle = I2C_open(i2cInst, &i2cParams);
    if(gI2cHandle == NULL)
    {
        appLogPrintf("DSS: I2C Open failed!\n");
        status = FVID2_EFAIL;
    }

    return (status);
}

#if defined (SOC_J721E)
void appDssConfigureUB941AndUB925(app_dss_default_prm_t *prm)
{
    int32_t status;
    uint32_t cnt, clientAddr;

    if(prm->display_type==APP_DSS_DEFAULT_DISPLAY_TYPE_DSI)
    {
        #if defined (SOC_J721E)
        appLogPrintf("DSS: Configuring SERDES ... !!!\n");
        status = appDssDsiSetBoardMux();

        if (FVID2_SOK == status)
        {
            status = appDssDsiInitI2c();
        }

        if (FVID2_SOK == status)
        {
            for (cnt = 0; cnt < sizeof(Ub941Ub925Config)/4; cnt ++)
            {
                clientAddr = Ub941Ub925Config[cnt][0];
                status = Board_i2c8BitRegWr(
                    gI2cHandle, clientAddr, Ub941Ub925Config[cnt][1],
                    &Ub941Ub925Config[cnt][2], 1U, BOARD_I2C_TRANSACTION_TIMEOUT);
                appLogWaitMsecs((uint32_t)Ub941Ub925Config[cnt][3]);

                if (0 != status)
                {
                    appLogPrintf("DSS: Write Failed for ClientAddr 0x%x RegAddr 0x%x Value 0x%x !\n",
                        clientAddr, Ub941Ub925Config[cnt][1], Ub941Ub925Config[cnt][2]);
                    break;
                }
            }
        }

        I2C_close(gI2cHandle);
        appLogPrintf("DSS: SERDES Configuration... Done !!!\n");
        #endif
    }
}

static int32_t appDssDsiSetBoardMux()
{
    Board_I2cInitCfg_t i2cCfg;

    /*setting power mux for dsi lcd*/
    i2cCfg.i2cInst   = BOARD_I2C_IOEXP_DEVICE4_INSTANCE;
    i2cCfg.socDomain = BOARD_SOC_DOMAIN_MAIN;
    i2cCfg.enableIntr = false;
    Board_setI2cInitConfig(&i2cCfg);

    /* Enable DSI in IO Expander */
    Board_i2cIoExpInit();

    /* Enable Reset for external Power */
    Board_i2cIoExpSetPinDirection(BOARD_I2C_IOEXP_DEVICE4_ADDR,
                                  ONE_PORT_IOEXP,
                                  PORTNUM_0,
                                  PIN_NUM_7,
                                  PIN_DIRECTION_OUTPUT);
    Board_i2cIoExpPinLevelSet(BOARD_I2C_IOEXP_DEVICE4_ADDR,
                              ONE_PORT_IOEXP,
                              PORTNUM_0,
                              PIN_NUM_7,
                              GPIO_SIGNAL_LEVEL_HIGH);

    /* Enable Power Switch control */
    Board_i2cIoExpSetPinDirection(BOARD_I2C_IOEXP_DEVICE4_ADDR,
                                  ONE_PORT_IOEXP,
                                  PORTNUM_0,
                                  PIN_NUM_2,
                                  PIN_DIRECTION_OUTPUT);

    Board_i2cIoExpPinLevelSet(BOARD_I2C_IOEXP_DEVICE4_ADDR,
                              ONE_PORT_IOEXP,
                              PORTNUM_0,
                              PIN_NUM_2,
                              GPIO_SIGNAL_LEVEL_HIGH);
    Board_i2cIoExpDeInit();

    return (FVID2_SOK);
}
#endif

#if defined (SOC_J721S2) || defined (SOC_J784S4) || defined (SOC_J742S2)
uint32_t appDssSN65DsiErrorRegRead(void)
{
    uint32_t i, retVal = 0U;
    uint8_t readVal;

    for(i = 0xF0U; i < 0xF8U; i++)
    {
        Board_i2c8BitRegRd(gI2cHandle,
                           0x2c,
                           i & 0xFF,
                           &readVal,
                           1,
                           VX_APP_I2C_TIMEOUT);
        if (readVal != 0)
        {
            retVal = 1U;
            appLogPrintf("Status at 0x%x in the SN65DSI bridge is 0x%x...\n", i, readVal);
        }
    }

    return retVal;
}

void appDssPrepareSN65I2cConfig(uint32_t resolution)
{
    int32_t i = 0;
    for(i=0; i<sizeof(gI2cDsiBridgeCfg)/sizeof(gI2cDsiBridgeCfg[0]); i++)
    {
        if (gI2cDsiBridgeCfg[i][0] == 0x10)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x26;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x12)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x4f;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x93)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x20;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x20)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x80;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x21)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x07;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x24)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x38;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x25)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x4;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x2C)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x3E;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x30)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x37;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x34)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x46;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x36)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x3c;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x38)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x3c;
            }
        }
        else if (gI2cDsiBridgeCfg[i][0] == 0x3a)
        {
            if (resolution == VX_DSI_RESO_1080P)
            {
                gI2cDsiBridgeCfg[i][1] = 0x37;
            }
        }
    }
}

int32_t appDssDsiConfigureSN65(uint32_t resolution)
{
    int32_t status = FVID2_SOK;

    appLogPrintf("DSS: Configuring Bridge !!!\n");
    if (FVID2_SOK == status)
    {
        status = appDssDsiInitI2c();
    }

    uint8_t readVal, wrVal=0xFF, i;
    for(i = 0xF0; i < 0xF9; i++)
    {
        Board_i2c8BitRegWr(gI2cHandle,
                            0x2c,
                            i & 0xFF,
                            &wrVal,
                            1,
                            VX_APP_I2C_TIMEOUT);
    }
    appDssPrepareSN65I2cConfig(resolution);
    for(i=0; i<sizeof(gI2cDsiBridgeCfg)/sizeof(gI2cDsiBridgeCfg[0]); i++)
    {
        /* Read the value at bridge address. */
        Board_i2c8BitRegWr(gI2cHandle,
                            0x2c,
                            gI2cDsiBridgeCfg[i][0] & 0xFF,
                            &(gI2cDsiBridgeCfg[i][1]),
                            1,
                            VX_APP_I2C_TIMEOUT);
        Osal_delay(gI2cDsiBridgeCfg[i][2]);
        /* Wait untill the DP_PLL has been locked. */
        if(gI2cDsiBridgeCfg[i][0] == 0x0d)
        {
            do {
                Board_i2c8BitRegRd(gI2cHandle,
                            0x2c,
                            0x0a,
                            &readVal,
                            1,
                            VX_APP_I2C_TIMEOUT);
            } while( (readVal & 0x80) == 0x00);
        }

        /* Main link should not be off. So keep waiting as long as the main links are off. */
        if(gI2cDsiBridgeCfg[i][0] == 0x96)
        {
            do {
                Board_i2c8BitRegRd(gI2cHandle,
                            0x2c,
                            0x96,
                            &readVal,
                            1,
                            VX_APP_I2C_TIMEOUT);
            } while( (readVal) == 0x00);
        }
    }
    appDssSN65DsiErrorRegRead();
    I2C_close(gI2cHandle);
    appLogPrintf("DSS: Configuring Bridge Done !!!\n");

    return (status);
}
#endif
