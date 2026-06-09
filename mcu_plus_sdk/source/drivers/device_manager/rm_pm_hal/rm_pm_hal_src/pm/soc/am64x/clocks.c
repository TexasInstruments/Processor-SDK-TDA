/*
 * Data version: 230209_212853
 *
 * Copyright (C) 2017-2026, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <clk.h>
#include <clk_mux.h>
#include <clk_dev.h>
#include <clk_div.h>
#include <clk_pll.h>
#include <types/array_size.h>
#include <types/devgrps.h>
#include <soc/am64x/clk_ids.h>
#include <soc/am64x/devices.h>
#include <soc/am64x/clocks.h>
#include <soc/am64x/clk_soc_hfosc0.h>
#include <clk_fixed.h>
#include <clk_div.h>
#include <clk_pllctrl.h>
#include <clk_pll_16fft.h>


enum {
	AM64X_FREQ_VALUE_BOARD_0_CPTS0_RFT_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_EXT_REFCLK1_OUT,
	AM64X_FREQ_VALUE_BOARD_0_FSI_RX0_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_FSI_RX1_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_FSI_RX2_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_FSI_RX3_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_FSI_RX4_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_FSI_RX5_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_GPMC0_CLKLB_OUT,
	AM64X_FREQ_VALUE_BOARD_0_I2C0_SCL_OUT,
	AM64X_FREQ_VALUE_BOARD_0_I2C1_SCL_OUT,
	AM64X_FREQ_VALUE_BOARD_0_I2C2_SCL_OUT,
	AM64X_FREQ_VALUE_BOARD_0_I2C3_SCL_OUT,
	AM64X_FREQ_VALUE_BOARD_0_LED_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_MCU_EXT_REFCLK0_OUT,
	AM64X_FREQ_VALUE_BOARD_0_MCU_I2C0_SCL_OUT,
	AM64X_FREQ_VALUE_BOARD_0_MCU_I2C1_SCL_OUT,
	AM64X_FREQ_VALUE_BOARD_0_MCU_SPI0_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_MCU_SPI1_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_MMC1_CLKLB_OUT,
	AM64X_FREQ_VALUE_BOARD_0_OSPI0_DQS_OUT,
	AM64X_FREQ_VALUE_BOARD_0_OSPI0_LBCLKO_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII1_RXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII1_TXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII2_RXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII2_TXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII1_RXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII1_TXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII2_RXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII2_TXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_RGMII1_RXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_RGMII1_TXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_RGMII2_RXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_RGMII2_TXC_OUT,
	AM64X_FREQ_VALUE_BOARD_0_RMII_REF_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_SPI0_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_SPI1_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_SPI2_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_SPI3_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_SPI4_CLK_OUT,
	AM64X_FREQ_VALUE_BOARD_0_TCK_OUT,
	AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_CPTS_GENF0,
	AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_CPTS_GENF1,
	AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_RGMII1_TXC_O,
	AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_RGMII2_TXC_O,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_10_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_11_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_4_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_5_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_6_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_7_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_8_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_9_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_0_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_1_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_2_TIMER_PWM,
	AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_3_TIMER_PWM,
	AM64X_FREQ_VALUE_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	AM64X_FREQ_VALUE_FSI_TX_MAIN_0_FSI_TX_CK,
	AM64X_FREQ_VALUE_FSI_TX_MAIN_1_FSI_TX_CK,
	AM64X_FREQ_VALUE_FSS_UL_MAIN_0_OSPI_0_OSPI_OCLK_CLK,
	AM64X_FREQ_VALUE_GPMC_MAIN_0_PO_GPMC_DEV_CLK,
	AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_0_PR1_MDIO_MDCLK_O,
	AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_0_PR1_RGMII0_TXC_O,
	AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_0_PR1_RGMII1_TXC_O,
	AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_1_PR1_MDIO_MDCLK_O,
	AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_1_PR1_RGMII0_TXC_O,
	AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_1_PR1_RGMII1_TXC_O,
	AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF1,
	AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF2,
	AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF3,
	AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF4,
	AM64X_FREQ_VALUE_MSHSI2C_MAIN_0_PORSCL,
	AM64X_FREQ_VALUE_MSHSI2C_MAIN_1_PORSCL,
	AM64X_FREQ_VALUE_MSHSI2C_MAIN_2_PORSCL,
	AM64X_FREQ_VALUE_MSHSI2C_MAIN_3_PORSCL,
	AM64X_FREQ_VALUE_MSHSI2C_MCU_0_PORSCL,
	AM64X_FREQ_VALUE_MSHSI2C_MCU_1_PORSCL,
	AM64X_FREQ_VALUE_PCIE_G2X1_64_MAIN_0_PCIE_LANE0_TXCLK,
	AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_0,
	AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_1,
	AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_12,
	AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_14,
	AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_2,
	AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_8,
	AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MCU_0,
	AM64X_FREQ_VALUE_SPI_MAIN_0_IO_CLKSPIO_CLK,
	AM64X_FREQ_VALUE_SPI_MAIN_1_IO_CLKSPIO_CLK,
	AM64X_FREQ_VALUE_SPI_MAIN_2_IO_CLKSPIO_CLK,
	AM64X_FREQ_VALUE_SPI_MAIN_3_IO_CLKSPIO_CLK,
	AM64X_FREQ_VALUE_SPI_MAIN_4_IO_CLKSPIO_CLK,
	AM64X_FREQ_VALUE_SPI_MCU_0_IO_CLKSPIO_CLK,
	AM64X_FREQ_VALUE_SPI_MCU_1_IO_CLKSPIO_CLK,
	AM64X_FREQ_VALUE_USB3P0SS64_16FFC_MAIN_0_PIPE_TXCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_REFCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_RXCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_RXFCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXFCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXMCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_REFCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_RXCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_RXFCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_TXFCLK,
	AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_TXMCLK,
	AM64X_FREQ_VALUE_COUNT,
};

enum {
	AM64X_FREQ_RANGE_ANY,
	AM64X_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,
	AM64X_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
	AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
	AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
	AM64X_FREQ_RANGE_ID_MAX,
};

enum {
	PLL_ENTRY_MCU_19_2MHZ_2400MHZ,
	PLL_ENTRY_MCU_20MHZ_2400MHZ,
	PLL_ENTRY_MCU_24MHZ_2400MHZ,
	PLL_ENTRY_MCU_25MHZ_2400MHZ,
	PLL_ENTRY_MCU_26MHZ_2400MHZ_F24BIT,
	PLL_ENTRY_MCU_27MHZ_2400MHZ_F24BIT,
	PLL_ENTRY_MAIN_19_2MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_20MHZ_1000MHZ,
	PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_25MHZ_1000MHZ,
	PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_27MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_PER0_19_2MHZ_960MHZ,
	PLL_ENTRY_PER0_20MHZ_960MHZ,
	PLL_ENTRY_PER0_24MHZ_960MHZ,
	PLL_ENTRY_PER0_25MHZ_960MHZ_F24BIT,
	PLL_ENTRY_PER0_26MHZ_960MHZ_F24BIT,
	PLL_ENTRY_PER0_27MHZ_960MHZ_F24BIT,
	PLL_ENTRY_PER1_19_2MHZ_1800MHZ_F24BIT,
	PLL_ENTRY_PER1_20MHZ_1800MHZ,
	PLL_ENTRY_PER1_24MHZ_1800MHZ,
	PLL_ENTRY_PER1_25MHZ_1800MHZ,
	PLL_ENTRY_PER1_26MHZ_1800MHZ_F24BIT,
	PLL_ENTRY_PER1_27MHZ_1800MHZ_F24BIT,
	PLL_ENTRY_ARM0_19_2MHZ_2000MHZ_F24BIT,
	PLL_ENTRY_ARM0_20MHZ_2000MHZ,
	PLL_ENTRY_ARM0_24MHZ_2000MHZ_F24BIT,
	PLL_ENTRY_ARM0_25MHZ_2000MHZ,
	PLL_ENTRY_ARM0_26MHZ_2000MHZ_F24BIT,
	PLL_ENTRY_ARM0_27MHZ_2000MHZ_F24BIT,
	PLL_ENTRY_DDR_20MHZ_1600MHZ,
	PLL_ENTRY_DDR_19_2MHZ_1600MHZ_F24BIT,
	PLL_ENTRY_DDR_24MHZ_1600MHZ_F24BIT,
	PLL_ENTRY_DDR_25MHZ_1600MHZ,
	PLL_ENTRY_DDR_26MHZ_1600MHZ_F24BIT,
	PLL_ENTRY_DDR_27MHZ_1600MHZ_F24BIT,
};

enum {
	NO_DEFAULT_FREQ,
	FREQ_PLLFRACF_SSMOD_16FFT_MAIN_0_DEFAULT,
	FREQ_PLLFRACF_SSMOD_16FFT_MAIN_1_DEFAULT,
	FREQ_PLLFRACF_SSMOD_16FFT_MAIN_12_DEFAULT,
	FREQ_PLLFRACF_SSMOD_16FFT_MAIN_14_DEFAULT,
	FREQ_PLLFRACF_SSMOD_16FFT_MAIN_2_DEFAULT,
	FREQ_PLLFRACF_SSMOD_16FFT_MAIN_8_DEFAULT,
	FREQ_PLLFRACF_SSMOD_16FFT_MCU_0_DEFAULT,
};

u32 soc_clock_values[AM64X_FREQ_VALUE_COUNT];

const struct clk_range soc_clock_ranges[AM64X_FREQ_RANGE_ID_MAX] = {
	CLK_RANGE(AM64X_FREQ_RANGE_ANY,				       0,	  ULONG_MAX),
	CLK_RANGE(AM64X_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,	       12500000,  12500000),
	CLK_RANGE(AM64X_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,    97656,	  97656),
	CLK_RANGE(AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0, 5000000,	  1200000000),
	CLK_RANGE(AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,    800000000, 3200000000),
};

const struct clk_default soc_clock_freq_defaults[8] = {
	CLK_DEFAULT(FREQ_PLLFRACF_SSMOD_16FFT_MAIN_0_DEFAULT,  1000000000UL, 1000000000UL, 1000000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF_SSMOD_16FFT_MAIN_1_DEFAULT,  960000000UL,  960000000UL,  960000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF_SSMOD_16FFT_MAIN_12_DEFAULT, 1600000000UL, 1600000000UL, 1600000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF_SSMOD_16FFT_MAIN_14_DEFAULT, 2400000000UL, 2400000000UL, 2400000001UL),
	CLK_DEFAULT(FREQ_PLLFRACF_SSMOD_16FFT_MAIN_2_DEFAULT,  1800000000UL, 1800000000UL, 1800000001UL),
	CLK_DEFAULT(FREQ_PLLFRACF_SSMOD_16FFT_MAIN_8_DEFAULT,  2000000000UL, 2000000000UL, 2000000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF_SSMOD_16FFT_MCU_0_DEFAULT,   2400000000UL, 2400000000UL, 2400000000UL),
};

const struct pll_table_entry soc_pll_table[36] = {
	[PLL_ENTRY_MCU_19_2MHZ_2400MHZ] =	  {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000000U,
		.plld		= 1U,
		.pllm		= 125U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_MCU_20MHZ_2400MHZ] =		  {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000000U,
		.plld		= 1U,
		.pllm		= 120U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_MCU_24MHZ_2400MHZ] =		  {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_MCU_25MHZ_2400MHZ] =		  {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000000U,
		.plld		= 1U,
		.pllm		= 96U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_MCU_26MHZ_2400MHZ_F24BIT] =	  {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000001U,
		.plld		= 1U,
		.pllm		= 92U,
		.pllfm		= 5162221U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_MCU_27MHZ_2400MHZ_F24BIT] =	  {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000000U,
		.plld		= 1U,
		.pllm		= 88U,
		.pllfm		= 14913081U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_MAIN_19_2MHZ_1000MHZ_F24BIT] = {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 104U,
		.pllfm		= 2796203U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_20MHZ_1000MHZ] =	  {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT] =	  {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 83U,
		.pllfm		= 5592406U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_25MHZ_1000MHZ] =	  {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT] =	  {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 76U,
		.pllfm		= 15486661U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_27MHZ_1000MHZ_F24BIT] =	  {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 74U,
		.pllfm		= 1242757U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_19_2MHZ_960MHZ] =	  {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_20MHZ_960MHZ] =		  {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 96U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_24MHZ_960MHZ] =		  {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_25MHZ_960MHZ_F24BIT] =	  {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 76U,
		.pllfm		= 13421773U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_26MHZ_960MHZ_F24BIT] =	  {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 73U,
		.pllfm		= 14196106U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_27MHZ_960MHZ_F24BIT] =	  {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 71U,
		.pllfm		= 1864136U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER1_19_2MHZ_1800MHZ_F24BIT] = {
		.freq_min_hz	= 1800000000U,
		.freq_max_hz	= 1800000000U,
		.plld		= 1U,
		.pllm		= 93U,
		.pllfm		= 12582912U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_PER1_20MHZ_1800MHZ] =	  {
		.freq_min_hz	= 1800000000U,
		.freq_max_hz	= 1800000000U,
		.plld		= 1U,
		.pllm		= 90U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_PER1_24MHZ_1800MHZ] =	  {
		.freq_min_hz	= 1800000000U,
		.freq_max_hz	= 1800000000U,
		.plld		= 1U,
		.pllm		= 75U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_PER1_25MHZ_1800MHZ] =	  {
		.freq_min_hz	= 1800000000U,
		.freq_max_hz	= 1800000000U,
		.plld		= 1U,
		.pllm		= 72U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_PER1_26MHZ_1800MHZ_F24BIT] =	  {
		.freq_min_hz	= 1800000000U,
		.freq_max_hz	= 1800000001U,
		.plld		= 1U,
		.pllm		= 69U,
		.pllfm		= 3871666U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_PER1_27MHZ_1800MHZ_F24BIT] =	  {
		.freq_min_hz	= 1800000000U,
		.freq_max_hz	= 1800000000U,
		.plld		= 1U,
		.pllm		= 66U,
		.pllfm		= 11184811U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_ARM0_19_2MHZ_2000MHZ_F24BIT] = {
		.freq_min_hz	= 2000000000U,
		.freq_max_hz	= 2000000000U,
		.plld		= 1U,
		.pllm		= 104U,
		.pllfm		= 2796203U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_ARM0_20MHZ_2000MHZ] =	  {
		.freq_min_hz	= 2000000000U,
		.freq_max_hz	= 2000000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_ARM0_24MHZ_2000MHZ_F24BIT] =	  {
		.freq_min_hz	= 2000000000U,
		.freq_max_hz	= 2000000000U,
		.plld		= 1U,
		.pllm		= 83U,
		.pllfm		= 5592406U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_ARM0_25MHZ_2000MHZ] =	  {
		.freq_min_hz	= 2000000000U,
		.freq_max_hz	= 2000000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_ARM0_26MHZ_2000MHZ_F24BIT] =	  {
		.freq_min_hz	= 2000000000U,
		.freq_max_hz	= 2000000000U,
		.plld		= 1U,
		.pllm		= 76U,
		.pllfm		= 15486661U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_ARM0_27MHZ_2000MHZ_F24BIT] =	  {
		.freq_min_hz	= 2000000000U,
		.freq_max_hz	= 2000000000U,
		.plld		= 1U,
		.pllm		= 74U,
		.pllfm		= 1242757U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DDR_20MHZ_1600MHZ] =		  {
		.freq_min_hz	= 1600000000U,
		.freq_max_hz	= 1600000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DDR_19_2MHZ_1600MHZ_F24BIT] =  {
		.freq_min_hz	= 1600000000U,
		.freq_max_hz	= 1600000000U,
		.plld		= 1U,
		.pllm		= 83U,
		.pllfm		= 5592406U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DDR_24MHZ_1600MHZ_F24BIT] =	  {
		.freq_min_hz	= 1600000000U,
		.freq_max_hz	= 1600000000U,
		.plld		= 1U,
		.pllm		= 66U,
		.pllfm		= 11184811U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DDR_25MHZ_1600MHZ] =		  {
		.freq_min_hz	= 1600000000U,
		.freq_max_hz	= 1600000000U,
		.plld		= 1U,
		.pllm		= 64U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DDR_26MHZ_1600MHZ_F24BIT] =	  {
		.freq_min_hz	= 1600000000U,
		.freq_max_hz	= 1600000000U,
		.plld		= 1U,
		.pllm		= 61U,
		.pllfm		= 9033886U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DDR_27MHZ_1600MHZ_F24BIT] =	  {
		.freq_min_hz	= 1600000000U,
		.freq_max_hz	= 1600000000U,
		.plld		= 1U,
		.pllm		= 59U,
		.pllfm		= 4349649U,
		.clkod		= 1U,
	},
};

static const u8 pllfracf_ssmod_16fft_main_0_entries[7] = {
	PLL_ENTRY_MAIN_19_2MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_20MHZ_1000MHZ,
	PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_25MHZ_1000MHZ,
	PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_27MHZ_1000MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf_ssmod_16fft_main_1_entries[7] = {
	PLL_ENTRY_PER0_19_2MHZ_960MHZ,
	PLL_ENTRY_PER0_20MHZ_960MHZ,
	PLL_ENTRY_PER0_24MHZ_960MHZ,
	PLL_ENTRY_PER0_25MHZ_960MHZ_F24BIT,
	PLL_ENTRY_PER0_26MHZ_960MHZ_F24BIT,
	PLL_ENTRY_PER0_27MHZ_960MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf_ssmod_16fft_main_12_entries[7] = {
	PLL_ENTRY_DDR_19_2MHZ_1600MHZ_F24BIT,
	PLL_ENTRY_DDR_20MHZ_1600MHZ,
	PLL_ENTRY_DDR_24MHZ_1600MHZ_F24BIT,
	PLL_ENTRY_DDR_25MHZ_1600MHZ,
	PLL_ENTRY_DDR_26MHZ_1600MHZ_F24BIT,
	PLL_ENTRY_DDR_27MHZ_1600MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf_ssmod_16fft_main_14_entries[7] = {
	PLL_ENTRY_MCU_19_2MHZ_2400MHZ,
	PLL_ENTRY_MCU_20MHZ_2400MHZ,
	PLL_ENTRY_MCU_24MHZ_2400MHZ,
	PLL_ENTRY_MCU_25MHZ_2400MHZ,
	PLL_ENTRY_MCU_26MHZ_2400MHZ_F24BIT,
	PLL_ENTRY_MCU_27MHZ_2400MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf_ssmod_16fft_main_2_entries[7] = {
	PLL_ENTRY_PER1_19_2MHZ_1800MHZ_F24BIT,
	PLL_ENTRY_PER1_20MHZ_1800MHZ,
	PLL_ENTRY_PER1_24MHZ_1800MHZ,
	PLL_ENTRY_PER1_25MHZ_1800MHZ,
	PLL_ENTRY_PER1_26MHZ_1800MHZ_F24BIT,
	PLL_ENTRY_PER1_27MHZ_1800MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf_ssmod_16fft_main_8_entries[7] = {
	PLL_ENTRY_ARM0_19_2MHZ_2000MHZ_F24BIT,
	PLL_ENTRY_ARM0_20MHZ_2000MHZ,
	PLL_ENTRY_ARM0_24MHZ_2000MHZ_F24BIT,
	PLL_ENTRY_ARM0_25MHZ_2000MHZ,
	PLL_ENTRY_ARM0_26MHZ_2000MHZ_F24BIT,
	PLL_ENTRY_ARM0_27MHZ_2000MHZ_F24BIT,
	PLL_TABLE_LAST,
};

static const struct clk_parent clk_CLKOUT_CTRL_parents[2] = {
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK,
		5,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK,
		10,
	},
};
static const struct clk_data_mux_reg clk_data_CLKOUT_CTRL_out0 = {
	.data_mux		= {
		.parents	= clk_CLKOUT_CTRL_parents,
		.n		= ARRAY_SIZE(clk_CLKOUT_CTRL_parents),
	},
	.reg			= 0x43000000 + 32784,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_ADC_clk_sel_parents[4] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_ADC_clk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_ADC_clk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_ADC_clk_sel_parents),
	},
	.reg			= 0x43000000 + 34064,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_CPTS_CLK_SEL_parents[8] = {
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXMCLK,
		1,
	},
	{
		CLK_AM64X_K3_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_CPTS_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_CPTS_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_CPTS_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33104,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_CP_GEMAC_CPTS_CLK_SEL_parents[16] = {
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXMCLK,
		1,
	},
	{
		CLK_AM64X_K3_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK,
		1,
	},
	{
		0,
		0,
	},
	{
		0,
		0,
	},
	{
		0,
		0,
	},
	{
		0,
		0,
	},
	{
		0,
		0,
	},
	{
		0,
		0,
	},
	{
		0,
		0,
	},
	{
		0,
		0,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_CP_GEMAC_CPTS_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_CP_GEMAC_CPTS_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_CP_GEMAC_CPTS_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33088,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_EMMCSD0_REFCLK_SEL_parents[2] = {
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT2_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD0_REFCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_EMMCSD0_REFCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_EMMCSD0_REFCLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33120,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_EMMCSD1_IO_CLKLB_SEL_parents[2] = {
	{
		CLK_AM64X_BOARD_0_MMC1_CLKLB_OUT,
		1,
	},
	{
		CLK_AM64X_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD1_IO_CLKLB_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_EMMCSD1_IO_CLKLB_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_EMMCSD1_IO_CLKLB_SEL_parents),
	},
	.reg			= 0x43000000 + 33128,
	.bit			= 16,
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD1_REFCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_EMMCSD0_REFCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_EMMCSD0_REFCLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33128,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_GPMC_FCLK_SEL_parents[2] = {
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT3_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT7_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_GPMC_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_GPMC_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_GPMC_FCLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33152,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_GTCCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_CPTS_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_CPTS_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 32816,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_ICSSGn_CORE_CLK_SEL_out0_parents[2] = {
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT9_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_ICSSGn_CORE_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_ICSSGn_CORE_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_ICSSGn_CORE_CLK_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 32832,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_ICSSGn_CORE_CLK_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_ICSSGn_CORE_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_ICSSGn_CORE_CLK_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 32836,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_ICSSGn_IEPCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_CPTS_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_CPTS_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 32832,
	.bit			= 16,
};
static const struct clk_data_mux_reg clk_data_MAIN_ICSSGn_IEPCLK_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_CPTS_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_CPTS_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 32836,
	.bit			= 16,
};
static const struct clk_parent clk_MAIN_MCANn_CLK_SEL_out0_parents[4] = {
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK,
		1,
	},
	{
		CLK_AM64X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33920,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33924,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_OBSCLK0_MUX_SEL_parents[16] = {
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV0_16FFT_MAIN_8_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV0_16FFT_MAIN_12_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV0_16FFT_MCU_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM64X_CPSW_3GUSS_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM64X_CPSW_3GUSS_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF2,
		1,
	},
	{
		CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF3,
		1,
	},
	{
		CLK_AM64X_HSDIV1_16FFT_MAIN_14_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OBSCLK0_MUX_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OBSCLK0_MUX_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OBSCLK0_MUX_SEL_parents),
	},
	.reg			= 0x43000000 + 32768,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_OSPI_LOOPBACK_CLK_SEL_parents[2] = {
	{
		CLK_AM64X_BOARD_0_OSPI0_DQS_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_OSPI0_LBCLKO_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI_LOOPBACK_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OSPI_LOOPBACK_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OSPI_LOOPBACK_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 34048,
	.bit			= 4,
};
static const struct clk_parent clk_MAIN_OSPI_REF_CLK_SEL_parents[2] = {
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT1_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT5_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI_REF_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OSPI_REF_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OSPI_REF_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 34048,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_PCIE0_CPTS_CLK_SEL_parents[8] = {
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXMCLK,
		1,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_PCIE0_CPTS_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_PCIE0_CPTS_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_PCIE0_CPTS_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33056,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_SERDES0_CORE_REFCLK_SEL_parents[4] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT4_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_SERDES0_CORE_REFCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_SERDES0_CORE_REFCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_SERDES0_CORE_REFCLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33792,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_TIMERCLKn_SEL_out0_parents[16] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV0_16FFT_MCU_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT3_CLK,
		1,
	},
	{
		CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM64X_CPSW_3GUSS_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM64X_CPSW_3GUSS_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF2,
		1,
	},
	{
		CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF3,
		1,
	},
	{
		CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF4,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33200,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33204,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out10 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33240,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out11 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33244,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33208,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33212,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33216,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out5 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33220,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out6 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33224,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out7 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33228,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out8 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33232,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out9 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33236,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USB0_REFCLK_SEL_parents[2] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT4_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USB0_REFCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USB0_REFCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USB0_REFCLK_SEL_parents),
	},
	.reg			= 0x43000000 + 33168,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_WWDTCLKn_SEL_out0_parents[4] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV0_16FFT_MCU_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33664,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33668,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33696,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33700,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33704,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out5 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33708,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_spi0_mstr_lp_clksel_parents[2] = {
	{
		CLK_AM64X_BOARD_0_SPI0_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_SPI_MAIN_0_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_spi0_mstr_lp_clksel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_spi0_mstr_lp_clksel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_spi0_mstr_lp_clksel_parents),
	},
	.reg			= 0x43000000 + 33280,
	.bit			= 16,
};
static const struct clk_parent clk_MAIN_spi1_mstr_lp_clksel_parents[2] = {
	{
		CLK_AM64X_BOARD_0_SPI1_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_SPI_MAIN_1_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_spi1_mstr_lp_clksel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_spi1_mstr_lp_clksel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_spi1_mstr_lp_clksel_parents),
	},
	.reg			= 0x43000000 + 33284,
	.bit			= 16,
};
static const struct clk_parent clk_MAIN_spi2_mstr_lp_clksel_parents[2] = {
	{
		CLK_AM64X_BOARD_0_SPI2_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_SPI_MAIN_2_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_spi2_mstr_lp_clksel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_spi2_mstr_lp_clksel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_spi2_mstr_lp_clksel_parents),
	},
	.reg			= 0x43000000 + 33288,
	.bit			= 16,
};
static const struct clk_parent clk_MAIN_spi3_mstr_lp_clksel_parents[2] = {
	{
		CLK_AM64X_BOARD_0_SPI3_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_SPI_MAIN_3_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_spi3_mstr_lp_clksel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_spi3_mstr_lp_clksel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_spi3_mstr_lp_clksel_parents),
	},
	.reg			= 0x43000000 + 33292,
	.bit			= 16,
};
static const struct clk_parent clk_MAIN_spi4_mstr_lp_clksel_parents[2] = {
	{
		CLK_AM64X_BOARD_0_SPI4_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_SPI_MAIN_4_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_spi4_mstr_lp_clksel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_spi4_mstr_lp_clksel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_spi4_mstr_lp_clksel_parents),
	},
	.reg			= 0x43000000 + 33296,
	.bit			= 16,
};
static const struct clk_parent clk_MAIN_usart0_fclk_sel_parents[2] = {
	{
		CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT0,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_usart0_fclk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_usart0_fclk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_usart0_fclk_sel_parents),
	},
	.reg			= 0x43000000 + 33408,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_usart1_fclk_sel_parents[2] = {
	{
		CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT1,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_usart1_fclk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_usart1_fclk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_usart1_fclk_sel_parents),
	},
	.reg			= 0x43000000 + 33412,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_usart2_fclk_sel_parents[2] = {
	{
		CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT2,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_usart2_fclk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_usart2_fclk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_usart2_fclk_sel_parents),
	},
	.reg			= 0x43000000 + 33416,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_usart3_fclk_sel_parents[2] = {
	{
		CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT3,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_usart3_fclk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_usart3_fclk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_usart3_fclk_sel_parents),
	},
	.reg			= 0x43000000 + 33420,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_usart4_fclk_sel_parents[2] = {
	{
		CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT4,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_usart4_fclk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_usart4_fclk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_usart4_fclk_sel_parents),
	},
	.reg			= 0x43000000 + 33424,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_usart5_fclk_sel_parents[2] = {
	{
		CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT5,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_usart5_fclk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_usart5_fclk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_usart5_fclk_sel_parents),
	},
	.reg			= 0x43000000 + 33428,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_usart6_fclk_sel_parents[2] = {
	{
		CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT6,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_usart6_fclk_sel_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_usart6_fclk_sel_parents,
		.n		= ARRAY_SIZE(clk_MAIN_usart6_fclk_sel_parents),
	},
	.reg			= 0x43000000 + 33432,
	.bit			= 0,
};
static const struct clk_parent clk_MCU_M4FSS_CLKSEL_parents[2] = {
	{
		CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK,
		1,
	},
	{
		CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK,
		2,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_M4FSS_CLKSEL_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_M4FSS_CLKSEL_parents,
		.n		= ARRAY_SIZE(clk_MCU_M4FSS_CLKSEL_parents),
	},
	.reg			= 0x04500000 + 32832,
	.bit			= 0,
};
static const struct clk_parent clk_MCU_OBSCLK_MUX_SEL_parents[8] = {
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT4_CLK,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV0_16FFT_MCU_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_OBSCLK_MUX_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_OBSCLK_MUX_SEL_parents,
		.n		= ARRAY_SIZE(clk_MCU_OBSCLK_MUX_SEL_parents),
	},
	.reg			= 0x04500000 + 32768,
	.bit			= 0,
};
static const struct clk_parent clk_MCU_OBSCLK_OUTMUX_SEL_parents[2] = {
	{
		CLK_AM64X_MCU_OBSCLK_DIV_OUT0,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_OBSCLK_OUTMUX_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_OBSCLK_OUTMUX_SEL_parents,
		.n		= ARRAY_SIZE(clk_MCU_OBSCLK_OUTMUX_SEL_parents),
	},
	.reg			= 0x04500000 + 32768,
	.bit			= 24,
};
static const struct clk_parent clk_MCU_TIMERCLKn_SEL_out0_parents[8] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK,
		2,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT3_CLK,
		1,
	},
	{
		CLK_AM64X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM64X_HSDIV0_16FFT_MCU_32KHZ_GEN_0_HSDIVOUT0_CLK,
		8,
	},
	{
		CLK_AM64X_CPSW_3GUSS_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM64X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_TIMERCLKn_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x04500000 + 32864,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MCU_TIMERCLKn_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MCU_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x04500000 + 32868,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MCU_TIMERCLKn_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_MCU_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x04500000 + 32872,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MCU_TIMERCLKn_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_MCU_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x04500000 + 32876,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MCU_WWDTCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x04500000 + 32944,
	.bit			= 0,
};
static const struct clk_parent clk_MCU_spi0_mstr_lp_clksel_parents[2] = {
	{
		CLK_AM64X_BOARD_0_MCU_SPI0_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_SPI_MCU_0_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_spi0_mstr_lp_clksel_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_spi0_mstr_lp_clksel_parents,
		.n		= ARRAY_SIZE(clk_MCU_spi0_mstr_lp_clksel_parents),
	},
	.reg			= 0x04500000 + 32928,
	.bit			= 16,
};
static const struct clk_parent clk_MCU_spi1_mstr_lp_clksel_parents[2] = {
	{
		CLK_AM64X_BOARD_0_MCU_SPI1_CLK_OUT,
		1,
	},
	{
		CLK_AM64X_SPI_MCU_1_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_spi1_mstr_lp_clksel_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_spi1_mstr_lp_clksel_parents,
		.n		= ARRAY_SIZE(clk_MCU_spi1_mstr_lp_clksel_parents),
	},
	.reg			= 0x04500000 + 32932,
	.bit			= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_Programmable_Clock_Divider_out0 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x43000000 + 33344,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_Programmable_Clock_Divider_out1 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x43000000 + 33348,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_Programmable_Clock_Divider_out2 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x43000000 + 33352,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_Programmable_Clock_Divider_out3 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x43000000 + 33356,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_Programmable_Clock_Divider_out4 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x43000000 + 33360,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_Programmable_Clock_Divider_out5 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x43000000 + 33364,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_Programmable_Clock_Divider_out6 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x43000000 + 33368,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_from_dev clk_data_board_0_CPTS0_RFT_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_CPTS0_RFT_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_CP_GEMAC_CPTS0_RFT_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_EXT_REFCLK1_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_EXT_REFCLK1_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_FSI_RX0_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_FSI_RX0_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_FSI_RX1_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_FSI_RX1_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_FSI_RX2_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_FSI_RX2_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_FSI_RX3_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_FSI_RX3_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_FSI_RX4_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_FSI_RX4_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_FSI_RX5_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_FSI_RX5_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_GPMC0_CLKLB_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_GPMC0_CLKLB_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C0_SCL_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_I2C0_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C1_SCL_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_I2C1_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C2_SCL_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_I2C2_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C3_SCL_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_I2C3_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_LED_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_LED_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCU_EXT_REFCLK0_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_MCU_EXT_REFCLK0_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCU_I2C0_SCL_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_MCU_I2C0_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCU_I2C1_SCL_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_MCU_I2C1_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCU_SPI0_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_MCU_SPI0_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCU_SPI1_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_MCU_SPI1_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC1_CLKLB_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_MMC1_CLKLB_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI0_DQS_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_OSPI0_DQS_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI0_LBCLKO_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_OSPI0_LBCLKO_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG0_RGMII1_RXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG0_RGMII1_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG0_RGMII1_TXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG0_RGMII1_TXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG0_RGMII2_RXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG0_RGMII2_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG0_RGMII2_TXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG0_RGMII2_TXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG1_RGMII1_RXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG1_RGMII1_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG1_RGMII1_TXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG1_RGMII1_TXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG1_RGMII2_RXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG1_RGMII2_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_PRG1_RGMII2_TXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_PRG1_RGMII2_TXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RGMII1_RXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_RGMII1_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RGMII1_TXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_RGMII1_TXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RGMII2_RXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_RGMII2_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RGMII2_TXC_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_RGMII2_TXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RMII_REF_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_RMII_REF_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI0_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_SPI0_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI1_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_SPI1_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI2_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_SPI2_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI3_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_SPI3_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI4_CLK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_SPI4_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_TCK_out = {
	.dev		= AM64X_DEV_BOARD0,
	.clk_idx	= AM64X_DEV_BOARD0_TCK_OUT,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_main_0_cpts_genf0 = {
	.dev		= AM64X_DEV_CPSW0,
	.clk_idx	= AM64X_DEV_CPSW0_CPTS_GENF0,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_main_0_cpts_genf1 = {
	.dev		= AM64X_DEV_CPSW0,
	.clk_idx	= AM64X_DEV_CPSW0_CPTS_GENF1,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_main_0_rgmii1_txc_o = {
	.dev		= AM64X_DEV_CPSW0,
	.clk_idx	= AM64X_DEV_CPSW0_RGMII1_TXC_O,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_main_0_rgmii2_txc_o = {
	.dev		= AM64X_DEV_CPSW0,
	.clk_idx	= AM64X_DEV_CPSW0_RGMII2_TXC_O,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_0_timer_pwm = {
	.dev		= AM64X_DEV_TIMER0,
	.clk_idx	= AM64X_DEV_TIMER0_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_1_timer_pwm = {
	.dev		= AM64X_DEV_TIMER1,
	.clk_idx	= AM64X_DEV_TIMER1_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_10_timer_pwm = {
	.dev		= AM64X_DEV_TIMER10,
	.clk_idx	= AM64X_DEV_TIMER10_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_11_timer_pwm = {
	.dev		= AM64X_DEV_TIMER11,
	.clk_idx	= AM64X_DEV_TIMER11_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_2_timer_pwm = {
	.dev		= AM64X_DEV_TIMER2,
	.clk_idx	= AM64X_DEV_TIMER2_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_3_timer_pwm = {
	.dev		= AM64X_DEV_TIMER3,
	.clk_idx	= AM64X_DEV_TIMER3_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_4_timer_pwm = {
	.dev		= AM64X_DEV_TIMER4,
	.clk_idx	= AM64X_DEV_TIMER4_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_5_timer_pwm = {
	.dev		= AM64X_DEV_TIMER5,
	.clk_idx	= AM64X_DEV_TIMER5_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_6_timer_pwm = {
	.dev		= AM64X_DEV_TIMER6,
	.clk_idx	= AM64X_DEV_TIMER6_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_7_timer_pwm = {
	.dev		= AM64X_DEV_TIMER7,
	.clk_idx	= AM64X_DEV_TIMER7_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_8_timer_pwm = {
	.dev		= AM64X_DEV_TIMER8,
	.clk_idx	= AM64X_DEV_TIMER8_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_9_timer_pwm = {
	.dev		= AM64X_DEV_TIMER9,
	.clk_idx	= AM64X_DEV_TIMER9_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_mcu_0_timer_pwm = {
	.dev		= AM64X_DEV_MCU_TIMER0,
	.clk_idx	= AM64X_DEV_MCU_TIMER0_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_mcu_1_timer_pwm = {
	.dev		= AM64X_DEV_MCU_TIMER1,
	.clk_idx	= AM64X_DEV_MCU_TIMER1_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_mcu_2_timer_pwm = {
	.dev		= AM64X_DEV_MCU_TIMER2,
	.clk_idx	= AM64X_DEV_MCU_TIMER2_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_mcu_3_timer_pwm = {
	.dev		= AM64X_DEV_MCU_TIMER3,
	.clk_idx	= AM64X_DEV_MCU_TIMER3_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_emmcsd4ss_main_0_emmcsdss_io_clk_o = {
	.dev		= AM64X_DEV_MMCSD1,
	.clk_idx	= AM64X_DEV_MMCSD1_EMMCSDSS_IO_CLK_O,
};
static const struct clk_data_from_dev clk_data_fsi_tx_main_0_fsi_tx_ck = {
	.dev		= AM64X_DEV_FSITX0,
	.clk_idx	= AM64X_DEV_FSITX0_FSI_TX_CK,
};
static const struct clk_data_from_dev clk_data_fsi_tx_main_1_fsi_tx_ck = {
	.dev		= AM64X_DEV_FSITX1,
	.clk_idx	= AM64X_DEV_FSITX1_FSI_TX_CK,
};
static const struct clk_data_from_dev clk_data_fss_ul_main_0_ospi_0_ospi_oclk_clk = {
	.dev		= AM64X_DEV_FSS0_OSPI_0,
	.clk_idx	= AM64X_DEV_FSS0_OSPI_0_OSPI_OCLK_CLK,
};
static const struct clk_data_from_dev clk_data_gpmc_main_0_po_gpmc_dev_clk = {
	.dev		= AM64X_DEV_GPMC0,
	.clk_idx	= AM64X_DEV_GPMC0_PO_GPMC_DEV_CLK,
};
static const struct clk_data_div_reg clk_data_hsdiv0_16fft_main_12_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 4,
	},
	.reg			= 0x00680000UL + (0x1000UL * 12UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv0_16fft_main_8_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 2,
	},
	.reg			= 0x00680000UL + (0x1000UL * 8UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv0_16fft_mcu_32khz_gen_0_hsdiv0 = {
	.data_div	= {
		.n	= 128,
	},
	.reg		= 0x04500000UL + 0x8030UL + (0x4UL * 0UL),
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv1_16fft_main_14_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 3,
	},
	.reg			= 0x00680000UL + (0x1000UL * 14UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv1_16fft_main_14_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 3,
	},
	.reg			= 0x00680000UL + (0x1000UL * 14UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 4,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 10,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 25,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv3 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 15,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 3UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv4 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 8,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 4UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_1_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 10,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_1_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 12,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_1_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_1_hsdiv3 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 10,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 3UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_1_hsdiv4 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 80,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 4UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_2_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 8,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_2_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 9,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_2_hsdiv3 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 6,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 3UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_2_hsdiv4 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 18,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 4UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_mcu_0_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 6,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_mcu_0_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 25,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_mcu_0_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 25,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_mcu_0_hsdiv3 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 12,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 3UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_mcu_0_hsdiv4 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 12,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 4UL),
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_icss_g_16ff_main_0_pr1_mdio_mdclk_o = {
	.dev		= AM64X_DEV_PRU_ICSSG0,
	.clk_idx	= AM64X_DEV_PRU_ICSSG0_PR1_MDIO_MDCLK_O,
};
static const struct clk_data_from_dev clk_data_icss_g_16ff_main_0_pr1_rgmii0_txc_o = {
	.dev		= AM64X_DEV_PRU_ICSSG0,
	.clk_idx	= AM64X_DEV_PRU_ICSSG0_PR1_RGMII0_TXC_O,
};
static const struct clk_data_from_dev clk_data_icss_g_16ff_main_0_pr1_rgmii1_txc_o = {
	.dev		= AM64X_DEV_PRU_ICSSG0,
	.clk_idx	= AM64X_DEV_PRU_ICSSG0_PR1_RGMII1_TXC_O,
};
static const struct clk_data_from_dev clk_data_icss_g_16ff_main_1_pr1_mdio_mdclk_o = {
	.dev		= AM64X_DEV_PRU_ICSSG1,
	.clk_idx	= AM64X_DEV_PRU_ICSSG1_PR1_MDIO_MDCLK_O,
};
static const struct clk_data_from_dev clk_data_icss_g_16ff_main_1_pr1_rgmii0_txc_o = {
	.dev		= AM64X_DEV_PRU_ICSSG1,
	.clk_idx	= AM64X_DEV_PRU_ICSSG1_PR1_RGMII0_TXC_O,
};
static const struct clk_data_from_dev clk_data_icss_g_16ff_main_1_pr1_rgmii1_txc_o = {
	.dev		= AM64X_DEV_PRU_ICSSG1,
	.clk_idx	= AM64X_DEV_PRU_ICSSG1_PR1_RGMII1_TXC_O,
};
static const struct clk_data_from_dev clk_data_k3_cpts_main_0_cpts_genf1 = {
	.dev		= AM64X_DEV_CPTS0,
	.clk_idx	= AM64X_DEV_CPTS0_CPTS_GENF1,
};
static const struct clk_data_from_dev clk_data_k3_cpts_main_0_cpts_genf2 = {
	.dev		= AM64X_DEV_CPTS0,
	.clk_idx	= AM64X_DEV_CPTS0_CPTS_GENF2,
};
static const struct clk_data_from_dev clk_data_k3_cpts_main_0_cpts_genf3 = {
	.dev		= AM64X_DEV_CPTS0,
	.clk_idx	= AM64X_DEV_CPTS0_CPTS_GENF3,
};
static const struct clk_data_from_dev clk_data_k3_cpts_main_0_cpts_genf4 = {
	.dev		= AM64X_DEV_CPTS0,
	.clk_idx	= AM64X_DEV_CPTS0_CPTS_GENF4,
};
static const struct clk_parent clk_k3_pll_ctrl_wrap_main_0_parents[2] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_k3_pll_ctrl_wrap_main_0_sysclkout_clk = {
	.data_mux		= {
		.parents	= clk_k3_pll_ctrl_wrap_main_0_parents,
		.n		= ARRAY_SIZE(clk_k3_pll_ctrl_wrap_main_0_parents),
	},
	.reg			= 0x00410000,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00410000 + 0x118,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_main_0_chip_div24_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00410000 + 0x11c,
	.bit		= 0,
};
static const struct clk_parent clk_k3_pll_ctrl_wrap_mcu_0_parents[2] = {
	{
		CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_k3_pll_ctrl_wrap_mcu_0_sysclkout_clk = {
	.data_mux		= {
		.parents	= clk_k3_pll_ctrl_wrap_mcu_0_parents,
		.n		= ARRAY_SIZE(clk_k3_pll_ctrl_wrap_mcu_0_parents),
	},
	.reg			= 0x04020000,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x04020000 + 0x118,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_mcu_0_chip_div24_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x04020000 + 0x11c,
	.bit		= 0,
};
static const struct clk_data_div_reg_go clk_data_mcu_obsclk_div_out0 = {
	.data_div	= {
		.n	= 16,
	},
	.reg		= 0x04500000 + 32768,
	.bit		= 8,
	.go		= 16,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_0_porscl = {
	.dev		= AM64X_DEV_I2C0,
	.clk_idx	= AM64X_DEV_I2C0_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_1_porscl = {
	.dev		= AM64X_DEV_I2C1,
	.clk_idx	= AM64X_DEV_I2C1_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_2_porscl = {
	.dev		= AM64X_DEV_I2C2,
	.clk_idx	= AM64X_DEV_I2C2_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_3_porscl = {
	.dev		= AM64X_DEV_I2C3,
	.clk_idx	= AM64X_DEV_I2C3_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_mcu_0_porscl = {
	.dev		= AM64X_DEV_MCU_I2C0,
	.clk_idx	= AM64X_DEV_MCU_I2C0_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_mcu_1_porscl = {
	.dev		= AM64X_DEV_MCU_I2C1,
	.clk_idx	= AM64X_DEV_MCU_I2C1_PORSCL,
};
static const struct clk_data_div_reg_go clk_data_osbclk0_div_out0 = {
	.data_div	= {
		.n	= 256,
	},
	.reg		= 0x43000000 + 32768,
	.bit		= 8,
	.go		= 16,
};
static const struct clk_data_from_dev clk_data_pcie_g2x1_64_main_0_pcie_lane0_txclk = {
	.dev		= AM64X_DEV_PCIE0,
	.clk_idx	= AM64X_DEV_PCIE0_PCIE_LANE0_TXCLK,
};
static const struct clk_data_pll_16fft clk_data_pllfracf_ssmod_16fft_main_0 = {
	.data_pll			= {
		.vco_range_idx		= AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf_ssmod_16fft_main_0_entries,
		.default_freq_idx	= FREQ_PLLFRACF_SSMOD_16FFT_MAIN_0_DEFAULT,
	},
	.idx				= 0,
	.base				= 0x00680000,
};
static const struct clk_data_div clk_data_pllfracf_ssmod_16fft_main_0_postdiv = {
	.n	= 49,
};
static const struct clk_data_pll_16fft clk_data_pllfracf_ssmod_16fft_main_1 = {
	.data_pll			= {
		.vco_range_idx		= AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf_ssmod_16fft_main_1_entries,
		.default_freq_idx	= FREQ_PLLFRACF_SSMOD_16FFT_MAIN_1_DEFAULT,
	},
	.idx				= 1,
	.base				= 0x00680000,
};
static const struct clk_data_div clk_data_pllfracf_ssmod_16fft_main_1_postdiv = {
	.n	= 49,
};
static const struct clk_data_pll_16fft clk_data_pllfracf_ssmod_16fft_main_12 = {
	.data_pll			= {
		.vco_range_idx		= AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf_ssmod_16fft_main_12_entries,
		.default_freq_idx	= FREQ_PLLFRACF_SSMOD_16FFT_MAIN_12_DEFAULT,
	},
	.idx				= 12,
	.base				= 0x00680000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf_ssmod_16fft_main_14 = {
	.data_pll			= {
		.vco_range_idx		= AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf_ssmod_16fft_main_14_entries,
		.default_freq_idx	= FREQ_PLLFRACF_SSMOD_16FFT_MAIN_14_DEFAULT,
	},
	.idx				= 14,
	.base				= 0x00680000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf_ssmod_16fft_main_2 = {
	.data_pll			= {
		.vco_range_idx		= AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf_ssmod_16fft_main_2_entries,
		.default_freq_idx	= FREQ_PLLFRACF_SSMOD_16FFT_MAIN_2_DEFAULT,
	},
	.idx				= 2,
	.base				= 0x00680000,
};
static const struct clk_data_div clk_data_pllfracf_ssmod_16fft_main_2_postdiv = {
	.n	= 49,
};
static const struct clk_data_pll_16fft clk_data_pllfracf_ssmod_16fft_main_8 = {
	.data_pll			= {
		.vco_range_idx		= AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf_ssmod_16fft_main_8_entries,
		.default_freq_idx	= FREQ_PLLFRACF_SSMOD_16FFT_MAIN_8_DEFAULT,
	},
	.idx				= 8,
	.base				= 0x00680000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf_ssmod_16fft_mcu_0 = {
	.data_pll			= {
		.vco_range_idx		= AM64X_FREQ_RANGE_VCO_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM64X_FREQ_RANGE_VCO_IN_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= pllfracf_ssmod_16fft_main_14_entries,
		.default_freq_idx	= FREQ_PLLFRACF_SSMOD_16FFT_MCU_0_DEFAULT,
	},
	.idx				= 0,
	.base				= 0x04040000,
};
static const struct clk_data_div_reg clk_data_postdiv1_16fft_main_1_hsdiv5 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 6,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 5UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv1_16fft_main_1_hsdiv6 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 16,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 6UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv5 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 5UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv6 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 6UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv7 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 7UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv8 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 8,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 8UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv9 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 3,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 9UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv5 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 8,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 5UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv6 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 8,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 6UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv7 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 18,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 7UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv8 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 30,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 8UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv9 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 9UL),
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_spi_main_0_io_clkspio_clk = {
	.dev		= AM64X_DEV_MCSPI0,
	.clk_idx	= AM64X_DEV_MCSPI0_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_1_io_clkspio_clk = {
	.dev		= AM64X_DEV_MCSPI1,
	.clk_idx	= AM64X_DEV_MCSPI1_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_2_io_clkspio_clk = {
	.dev		= AM64X_DEV_MCSPI2,
	.clk_idx	= AM64X_DEV_MCSPI2_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_3_io_clkspio_clk = {
	.dev		= AM64X_DEV_MCSPI3,
	.clk_idx	= AM64X_DEV_MCSPI3_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_4_io_clkspio_clk = {
	.dev		= AM64X_DEV_MCSPI4,
	.clk_idx	= AM64X_DEV_MCSPI4_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_mcu_0_io_clkspio_clk = {
	.dev		= AM64X_DEV_MCU_MCSPI0,
	.clk_idx	= AM64X_DEV_MCU_MCSPI0_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_mcu_1_io_clkspio_clk = {
	.dev		= AM64X_DEV_MCU_MCSPI1,
	.clk_idx	= AM64X_DEV_MCU_MCSPI1_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_usb3p0ss64_16ffc_main_0_pipe_txclk = {
	.dev		= AM64X_DEV_USB0,
	.clk_idx	= AM64X_DEV_USB0_PIPE_TXCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip1_ln0_refclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP1_LN0_REFCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip1_ln0_rxclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP1_LN0_RXCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip1_ln0_rxfclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP1_LN0_RXFCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip1_ln0_txfclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP1_LN0_TXFCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip1_ln0_txmclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP1_LN0_TXMCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip2_ln0_refclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP2_LN0_REFCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip2_ln0_rxclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP2_LN0_RXCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip2_ln0_rxfclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP2_LN0_RXFCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip2_ln0_txfclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP2_LN0_TXFCLK,
};
static const struct clk_data_from_dev clk_data_wiz16b2m4ct_main_0_ip2_ln0_txmclk = {
	.dev		= AM64X_DEV_SERDES_10G0,
	.clk_idx	= AM64X_DEV_SERDES_10G0_IP2_LN0_TXMCLK,
};

const struct clk_data soc_clock_data[228] = {
	[CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT] =			  {
		.drv	= &clk_drv_soc_hfosc0,
		.flags	= 0,
	},
	[CLK_AM64X_GLUELOGIC_RCOSC_CLKOUT] =			  {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM64X_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,
	},
	[CLK_AM64X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K] =		  {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM64X_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
	},
	[CLK_AM64X_BOARD_0_CPTS0_RFT_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_CPTS0_RFT_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_CPTS0_RFT_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_CP_GEMAC_CPTS0_RFT_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_EXT_REFCLK1_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_EXT_REFCLK1_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_EXT_REFCLK1_OUT,
	},
	[CLK_AM64X_BOARD_0_FSI_RX0_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_FSI_RX0_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_FSI_RX0_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_FSI_RX1_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_FSI_RX1_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_FSI_RX1_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_FSI_RX2_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_FSI_RX2_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_FSI_RX2_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_FSI_RX3_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_FSI_RX3_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_FSI_RX3_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_FSI_RX4_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_FSI_RX4_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_FSI_RX4_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_FSI_RX5_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_FSI_RX5_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_FSI_RX5_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_GPMC0_CLKLB_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_GPMC0_CLKLB_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_GPMC0_CLKLB_OUT,
	},
	[CLK_AM64X_BOARD_0_I2C0_SCL_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C0_SCL_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_I2C0_SCL_OUT,
	},
	[CLK_AM64X_BOARD_0_I2C1_SCL_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C1_SCL_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_I2C1_SCL_OUT,
	},
	[CLK_AM64X_BOARD_0_I2C2_SCL_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C2_SCL_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_I2C2_SCL_OUT,
	},
	[CLK_AM64X_BOARD_0_I2C3_SCL_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C3_SCL_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_I2C3_SCL_OUT,
	},
	[CLK_AM64X_BOARD_0_LED_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_LED_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_LED_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_MCU_EXT_REFCLK0_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCU_EXT_REFCLK0_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_MCU_EXT_REFCLK0_OUT,
	},
	[CLK_AM64X_BOARD_0_MMC1_CLKLB_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MMC1_CLKLB_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_MMC1_CLKLB_OUT,
	},
	[CLK_AM64X_BOARD_0_OSPI0_DQS_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_OSPI0_DQS_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_OSPI0_DQS_OUT,
	},
	[CLK_AM64X_BOARD_0_OSPI0_LBCLKO_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_OSPI0_LBCLKO_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_OSPI0_LBCLKO_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG0_RGMII1_RXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG0_RGMII1_RXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII1_RXC_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG0_RGMII1_TXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG0_RGMII1_TXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII1_TXC_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG0_RGMII2_RXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG0_RGMII2_RXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII2_RXC_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG0_RGMII2_TXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG0_RGMII2_TXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG0_RGMII2_TXC_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG1_RGMII1_RXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG1_RGMII1_RXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII1_RXC_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG1_RGMII1_TXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG1_RGMII1_TXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII1_TXC_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG1_RGMII2_RXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG1_RGMII2_RXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII2_RXC_OUT,
	},
	[CLK_AM64X_BOARD_0_PRG1_RGMII2_TXC_OUT] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_PRG1_RGMII2_TXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_PRG1_RGMII2_TXC_OUT,
	},
	[CLK_AM64X_BOARD_0_RGMII1_RXC_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RGMII1_RXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_RGMII1_RXC_OUT,
	},
	[CLK_AM64X_BOARD_0_RGMII1_TXC_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RGMII1_TXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_RGMII1_TXC_OUT,
	},
	[CLK_AM64X_BOARD_0_RGMII2_RXC_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RGMII2_RXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_RGMII2_RXC_OUT,
	},
	[CLK_AM64X_BOARD_0_RGMII2_TXC_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RGMII2_TXC_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_RGMII2_TXC_OUT,
	},
	[CLK_AM64X_BOARD_0_RMII_REF_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RMII_REF_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_RMII_REF_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_SPI0_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI0_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_SPI0_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_SPI1_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI1_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_SPI1_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_SPI2_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI2_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_SPI2_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_SPI3_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI3_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_SPI3_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_SPI4_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI4_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_SPI4_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_TCK_OUT] =				  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_TCK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_TCK_OUT,
	},
	[CLK_AM64X_CPSW_3GUSS_MAIN_0_CPTS_GENF0] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_cpsw_3guss_main_0_cpts_genf0.data,
		.freq_idx	= AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_CPTS_GENF0,
	},
	[CLK_AM64X_CPSW_3GUSS_MAIN_0_CPTS_GENF1] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_cpsw_3guss_main_0_cpts_genf1.data,
		.freq_idx	= AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_CPTS_GENF1,
	},
	[CLK_AM64X_CPSW_3GUSS_MAIN_0_RGMII1_TXC_O] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_cpsw_3guss_main_0_rgmii1_txc_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_RGMII1_TXC_O,
	},
	[CLK_AM64X_CPSW_3GUSS_MAIN_0_RGMII2_TXC_O] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_cpsw_3guss_main_0_rgmii2_txc_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_CPSW_3GUSS_MAIN_0_RGMII2_TXC_O,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_0_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_1_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_10_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_10_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_10_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_11_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_11_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_11_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_2_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_3_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_4_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_4_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_4_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_5_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_5_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_5_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_6_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_6_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_6_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_7_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_7_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_7_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_8_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_8_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_8_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MAIN_9_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_9_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_9_TIMER_PWM,
	},
	[CLK_AM64X_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_emmcsd4ss_main_0_emmcsdss_io_clk_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	},
	[CLK_AM64X_FSI_TX_MAIN_0_FSI_TX_CK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fsi_tx_main_0_fsi_tx_ck.data,
		.freq_idx	= AM64X_FREQ_VALUE_FSI_TX_MAIN_0_FSI_TX_CK,
	},
	[CLK_AM64X_FSI_TX_MAIN_1_FSI_TX_CK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fsi_tx_main_1_fsi_tx_ck.data,
		.freq_idx	= AM64X_FREQ_VALUE_FSI_TX_MAIN_1_FSI_TX_CK,
	},
	[CLK_AM64X_FSS_UL_MAIN_0_OSPI_0_OSPI_OCLK_CLK] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fss_ul_main_0_ospi_0_ospi_oclk_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_FSS_UL_MAIN_0_OSPI_0_OSPI_OCLK_CLK,
	},
	[CLK_AM64X_GPMC_MAIN_0_PO_GPMC_DEV_CLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_gpmc_main_0_po_gpmc_dev_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_GPMC_MAIN_0_PO_GPMC_DEV_CLK,
	},
	[CLK_AM64X_HSDIV0_16FFT_MCU_32KHZ_GEN_0_HSDIVOUT0_CLK] =  {
		.parent =					  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv0_16fft_mcu_32khz_gen_0_hsdiv0.data_div.data,
	},
	[CLK_AM64X_ICSS_G_16FF_MAIN_0_PR1_MDIO_MDCLK_O] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_16ff_main_0_pr1_mdio_mdclk_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_0_PR1_MDIO_MDCLK_O,
	},
	[CLK_AM64X_ICSS_G_16FF_MAIN_0_PR1_RGMII0_TXC_O] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_16ff_main_0_pr1_rgmii0_txc_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_0_PR1_RGMII0_TXC_O,
	},
	[CLK_AM64X_ICSS_G_16FF_MAIN_0_PR1_RGMII1_TXC_O] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_16ff_main_0_pr1_rgmii1_txc_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_0_PR1_RGMII1_TXC_O,
	},
	[CLK_AM64X_ICSS_G_16FF_MAIN_1_PR1_MDIO_MDCLK_O] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_16ff_main_1_pr1_mdio_mdclk_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_1_PR1_MDIO_MDCLK_O,
	},
	[CLK_AM64X_ICSS_G_16FF_MAIN_1_PR1_RGMII0_TXC_O] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_16ff_main_1_pr1_rgmii0_txc_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_1_PR1_RGMII0_TXC_O,
	},
	[CLK_AM64X_ICSS_G_16FF_MAIN_1_PR1_RGMII1_TXC_O] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_16ff_main_1_pr1_rgmii1_txc_o.data,
		.freq_idx	= AM64X_FREQ_VALUE_ICSS_G_16FF_MAIN_1_PR1_RGMII1_TXC_O,
	},
	[CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF1] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_k3_cpts_main_0_cpts_genf1.data,
		.freq_idx	= AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF1,
	},
	[CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF2] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_k3_cpts_main_0_cpts_genf2.data,
		.freq_idx	= AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF2,
	},
	[CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF3] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_k3_cpts_main_0_cpts_genf3.data,
		.freq_idx	= AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF3,
	},
	[CLK_AM64X_K3_CPTS_MAIN_0_CPTS_GENF4] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_k3_cpts_main_0_cpts_genf4.data,
		.freq_idx	= AM64X_FREQ_VALUE_K3_CPTS_MAIN_0_CPTS_GENF4,
	},
	[CLK_AM64X_MSHSI2C_MAIN_0_PORSCL] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_0_porscl.data,
		.freq_idx	= AM64X_FREQ_VALUE_MSHSI2C_MAIN_0_PORSCL,
	},
	[CLK_AM64X_MSHSI2C_MAIN_1_PORSCL] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_1_porscl.data,
		.freq_idx	= AM64X_FREQ_VALUE_MSHSI2C_MAIN_1_PORSCL,
	},
	[CLK_AM64X_MSHSI2C_MAIN_2_PORSCL] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_2_porscl.data,
		.freq_idx	= AM64X_FREQ_VALUE_MSHSI2C_MAIN_2_PORSCL,
	},
	[CLK_AM64X_MSHSI2C_MAIN_3_PORSCL] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_3_porscl.data,
		.freq_idx	= AM64X_FREQ_VALUE_MSHSI2C_MAIN_3_PORSCL,
	},
	[CLK_AM64X_PCIE_G2X1_64_MAIN_0_PCIE_LANE0_TXCLK] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_pcie_g2x1_64_main_0_pcie_lane0_txclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_PCIE_G2X1_64_MAIN_0_PCIE_LANE0_TXCLK,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK] =	  {
		.parent		=				  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_0,
		.data		= &clk_data_pllfracf_ssmod_16fft_main_0.data_pll.data,
		.flags		= CLK_DATA_FLAG_NO_HW_REINIT,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK] = {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv.drv,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_pllfracf_ssmod_16fft_main_0_postdiv.data,
		.flags	= 0,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK] =	  {
		.parent		=				  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_1,
		.data		= &clk_data_pllfracf_ssmod_16fft_main_1.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTPOSTDIV_CLK] = {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv.drv,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_pllfracf_ssmod_16fft_main_1_postdiv.data,
		.flags	= 0,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_12_FOUTVCOP_CLK] =	  {
		.parent		=				  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_12,
		.data		= &clk_data_pllfracf_ssmod_16fft_main_12.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_14_FOUTVCOP_CLK] =	  {
		.parent		=				  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_14,
		.data		= &clk_data_pllfracf_ssmod_16fft_main_14.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK] =	  {
		.parent		=				  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_2,
		.data		= &clk_data_pllfracf_ssmod_16fft_main_2.data_pll.data,
		.flags		= CLK_DATA_FLAG_NO_HW_REINIT,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK] = {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv.drv,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_pllfracf_ssmod_16fft_main_2_postdiv.data,
		.flags	= 0,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_8_FOUTVCOP_CLK] =	  {
		.parent		=				  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MAIN_8,
		.data		= &clk_data_pllfracf_ssmod_16fft_main_8.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM64X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT5_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv1_16fft_main_1_hsdiv5.data_div.data,
	},
	[CLK_AM64X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT6_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv1_16fft_main_1_hsdiv6.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv5.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv6.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv7.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv8.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_0_HSDIVOUT9_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_NO_HW_REINIT,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv9.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT5_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv5.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT6_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv6.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT7_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv7.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT8_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv8.data_div.data,
	},
	[CLK_AM64X_POSTDIV4_16FF_MAIN_2_HSDIVOUT9_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv9.data_div.data,
	},
	[CLK_AM64X_SPI_MAIN_0_IO_CLKSPIO_CLK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_0_io_clkspio_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_SPI_MAIN_0_IO_CLKSPIO_CLK,
	},
	[CLK_AM64X_SPI_MAIN_1_IO_CLKSPIO_CLK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_1_io_clkspio_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_SPI_MAIN_1_IO_CLKSPIO_CLK,
	},
	[CLK_AM64X_SPI_MAIN_2_IO_CLKSPIO_CLK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_2_io_clkspio_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_SPI_MAIN_2_IO_CLKSPIO_CLK,
	},
	[CLK_AM64X_SPI_MAIN_3_IO_CLKSPIO_CLK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_3_io_clkspio_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_SPI_MAIN_3_IO_CLKSPIO_CLK,
	},
	[CLK_AM64X_SPI_MAIN_4_IO_CLKSPIO_CLK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_4_io_clkspio_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_SPI_MAIN_4_IO_CLKSPIO_CLK,
	},
	[CLK_AM64X_USB3P0SS64_16FFC_MAIN_0_PIPE_TXCLK] =	  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_usb3p0ss64_16ffc_main_0_pipe_txclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_USB3P0SS64_16FFC_MAIN_0_PIPE_TXCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_REFCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip1_ln0_refclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_REFCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_RXCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip1_ln0_rxclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_RXCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_RXFCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip1_ln0_rxfclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_RXFCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXFCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip1_ln0_txfclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXFCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXMCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip1_ln0_txmclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP1_LN0_TXMCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP2_LN0_REFCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip2_ln0_refclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_REFCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP2_LN0_RXCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip2_ln0_rxclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_RXCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP2_LN0_RXFCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip2_ln0_rxfclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_RXFCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP2_LN0_TXFCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip2_ln0_txfclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_TXFCLK,
	},
	[CLK_AM64X_WIZ16B2M4CT_MAIN_0_IP2_LN0_TXMCLK] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz16b2m4ct_main_0_ip2_ln0_txmclk.data,
		.freq_idx	= AM64X_FREQ_VALUE_WIZ16B2M4CT_MAIN_0_IP2_LN0_TXMCLK,
	},
	[CLK_AM64X_MAIN_ADC_CLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_ADC_clk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_EMMCSD1_IO_CLKLB_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_EMMCSD1_IO_CLKLB_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_OSPI_LOOPBACK_CLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OSPI_LOOPBACK_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_PCIE0_CPTS_CLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_PCIE0_CPTS_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_WWDTCLKN_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_WWDTCLKN_SEL_OUT1] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_WWDTCLKN_SEL_OUT2] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_WWDTCLKN_SEL_OUT3] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_WWDTCLKN_SEL_OUT4] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_WWDTCLKN_SEL_OUT5] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out5.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_SPI0_MSTR_LP_CLKSEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_spi0_mstr_lp_clksel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_SPI1_MSTR_LP_CLKSEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_spi1_mstr_lp_clksel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_SPI2_MSTR_LP_CLKSEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_spi2_mstr_lp_clksel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_SPI3_MSTR_LP_CLKSEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_spi3_mstr_lp_clksel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_SPI4_MSTR_LP_CLKSEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_spi4_mstr_lp_clksel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_HSDIV0_16FFT_MAIN_12_HSDIVOUT0_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_12_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv0_16fft_main_12_hsdiv0.data_div.data,
	},
	[CLK_AM64X_HSDIV0_16FFT_MAIN_8_HSDIVOUT0_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_8_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv0_16fft_main_8_hsdiv0.data_div.data,
	},
	[CLK_AM64X_HSDIV1_16FFT_MAIN_14_HSDIVOUT0_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_14_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv1_16fft_main_14_hsdiv0.data_div.data,
	},
	[CLK_AM64X_HSDIV1_16FFT_MAIN_14_HSDIVOUT1_CLK] =	  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_14_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv1_16fft_main_14_hsdiv1.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv0.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT1_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv1.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv2.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT3_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv3.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv4.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv0.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv1.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT2_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv2.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT3_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv3.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT4_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv4.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT0_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_NO_HW_REINIT,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv0.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT2_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv2.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT3_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv3.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MAIN_2_HSDIVOUT4_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv4.data_div.data,
	},
	[CLK_AM64X_K3_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK] =	  {
		.drv	= &clk_drv_pllctrl_mux_reg_ro.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_main_0_sysclkout_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_K3_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK] =	  {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_main_0_chip_div1_clk_clk.data_div.data,
		.parent =					  {
			CLK_AM64X_K3_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM64X_MAIN_CPTS_CLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_CPTS_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_CP_GEMAC_CPTS_CLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_CP_GEMAC_CPTS_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_EMMCSD0_REFCLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_EMMCSD0_REFCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_EMMCSD1_REFCLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_EMMCSD1_REFCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_GPMC_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_GPMC_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_GTCCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_GTCCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_ICSSGN_CORE_CLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_ICSSGn_CORE_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_ICSSGN_CORE_CLK_SEL_OUT1] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_ICSSGn_CORE_CLK_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_ICSSGN_IEPCLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_ICSSGn_IEPCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_ICSSGN_IEPCLK_SEL_OUT1] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_ICSSGn_IEPCLK_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_MCANN_CLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_MCANn_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_MCANN_CLK_SEL_OUT1] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_MCANn_CLK_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_OSPI_REF_CLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OSPI_REF_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_SERDES0_CORE_REFCLK_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_SERDES0_CORE_REFCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT1] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT10] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out10.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT11] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out11.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT2] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT3] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT4] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT5] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out5.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT6] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out6.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT7] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out7.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT8] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out8.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_TIMERCLKN_SEL_OUT9] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out9.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_USB0_REFCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USB0_REFCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT0] =	  {
		.parent =					  {
			CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_Programmable_Clock_Divider_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT1] =	  {
		.parent =					  {
			CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_Programmable_Clock_Divider_out1.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT2] =	  {
		.parent =					  {
			CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_Programmable_Clock_Divider_out2.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT3] =	  {
		.parent =					  {
			CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_Programmable_Clock_Divider_out3.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT4] =	  {
		.parent =					  {
			CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_Programmable_Clock_Divider_out4.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT5] =	  {
		.parent =					  {
			CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_Programmable_Clock_Divider_out5.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_USART_PROGRAMMABLE_CLOCK_DIVIDER_OUT6] =	  {
		.parent =					  {
			CLK_AM64X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_Programmable_Clock_Divider_out6.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_MAIN_USART0_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_usart0_fclk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_USART1_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_usart1_fclk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_USART2_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_usart2_fclk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_USART3_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_usart3_fclk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_USART4_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_usart4_fclk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_USART5_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_usart5_fclk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_USART6_FCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_usart6_fclk_sel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_CLKOUT_CTRL_OUT0] =				  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_CLKOUT_CTRL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MAIN_OBSCLK0_MUX_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OBSCLK0_MUX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_WWDTCLK_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_WWDTCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_BOARD_0_MCU_I2C0_SCL_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCU_I2C0_SCL_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_MCU_I2C0_SCL_OUT,
	},
	[CLK_AM64X_BOARD_0_MCU_I2C1_SCL_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCU_I2C1_SCL_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_MCU_I2C1_SCL_OUT,
	},
	[CLK_AM64X_BOARD_0_MCU_SPI0_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCU_SPI0_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_MCU_SPI0_CLK_OUT,
	},
	[CLK_AM64X_BOARD_0_MCU_SPI1_CLK_OUT] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCU_SPI1_CLK_out.data,
		.freq_idx	= AM64X_FREQ_VALUE_BOARD_0_MCU_SPI1_CLK_OUT,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MCU_0_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_mcu_0_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_0_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MCU_1_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_mcu_1_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_1_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MCU_2_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_mcu_2_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_2_TIMER_PWM,
	},
	[CLK_AM64X_DMTIMER_DMC1MS_MCU_3_TIMER_PWM] =		  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_mcu_3_timer_pwm.data,
		.freq_idx	= AM64X_FREQ_VALUE_DMTIMER_DMC1MS_MCU_3_TIMER_PWM,
	},
	[CLK_AM64X_MSHSI2C_MCU_0_PORSCL] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_mcu_0_porscl.data,
		.freq_idx	= AM64X_FREQ_VALUE_MSHSI2C_MCU_0_PORSCL,
	},
	[CLK_AM64X_MSHSI2C_MCU_1_PORSCL] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_mcu_1_porscl.data,
		.freq_idx	= AM64X_FREQ_VALUE_MSHSI2C_MCU_1_PORSCL,
	},
	[CLK_AM64X_OSBCLK0_DIV_OUT0] =				  {
		.parent =					  {
			CLK_AM64X_MAIN_OBSCLK0_MUX_SEL_OUT0,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_osbclk0_div_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_PLLFRACF_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK] =	  {
		.parent		=				  {
			CLK_AM64X_GLUELOGIC_HFOSC0_CLKOUT,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM64X_FREQ_VALUE_PLLFRACF_SSMOD_16FFT_MCU_0,
		.data		= &clk_data_pllfracf_ssmod_16fft_mcu_0.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM64X_SPI_MCU_0_IO_CLKSPIO_CLK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_mcu_0_io_clkspio_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_SPI_MCU_0_IO_CLKSPIO_CLK,
	},
	[CLK_AM64X_SPI_MCU_1_IO_CLKSPIO_CLK] =			  {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_mcu_1_io_clkspio_clk.data,
		.freq_idx	= AM64X_FREQ_VALUE_SPI_MCU_1_IO_CLKSPIO_CLK,
	},
	[CLK_AM64X_MCU_SPI0_MSTR_LP_CLKSEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_spi0_mstr_lp_clksel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_SPI1_MSTR_LP_CLKSEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_spi1_mstr_lp_clksel_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv0.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT1_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv1.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT2_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv2.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT3_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv3.data_div.data,
	},
	[CLK_AM64X_HSDIV4_16FFT_MCU_0_HSDIVOUT4_CLK] =		  {
		.parent =					  {
			CLK_AM64X_PLLFRACF_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv4.data_div.data,
	},
	[CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_SYSCLKOUT_CLK] =	  {
		.drv	= &clk_drv_pllctrl_mux_reg_ro.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_mcu_0_sysclkout_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK] =	  {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk.data_div.data,
		.parent =					  {
			CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM64X_MCU_M4FSS_CLKSEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_M4FSS_CLKSEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_OBSCLK_MUX_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_OBSCLK_MUX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_TIMERCLKN_SEL_OUT0] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_TIMERCLKn_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_TIMERCLKN_SEL_OUT1] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_TIMERCLKn_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_TIMERCLKN_SEL_OUT2] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_TIMERCLKn_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_TIMERCLKN_SEL_OUT3] =			  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_TIMERCLKn_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_MCU_OBSCLK_DIV_OUT0] =			  {
		.parent =					  {
			CLK_AM64X_MCU_OBSCLK_MUX_SEL_OUT0,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_obsclk_div_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM64X_MCU_OBSCLK_OUTMUX_SEL_OUT0] =		  {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_OBSCLK_OUTMUX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM64X_K3_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV24_CLK_CLK] =  {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_main_0_chip_div24_clk_clk.data_div.data,
		.parent =					  {
			CLK_AM64X_K3_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_CHIP_DIV24_CLK_CLK] =	  {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_mcu_0_chip_div24_clk_clk.data_div.data,
		.parent =					  {
			CLK_AM64X_K3_PLL_CTRL_WRAP_MCU_0_SYSCLKOUT_CLK,
			1,
		},
	},
};

struct clk soc_clocks[228];
const size_t soc_clock_count = ARRAY_SIZE(soc_clock_data);
