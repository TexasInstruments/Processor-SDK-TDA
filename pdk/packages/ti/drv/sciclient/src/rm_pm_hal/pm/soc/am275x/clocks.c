/*
 * Data version: 251105_135430
 *
 * Copyright (C) 2017-2025, Texas Instruments Incorporated
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
#include <soc/am275x/clk_ids.h>
#include <soc/am275x/devices.h>
#include <soc/am275x/clocks.h>
#include <soc/am275x/clk_soc_hfosc0.h>
#include <soc/am275x/clk_soc_lfosc0.h>
#include <clk_fixed.h>
#include <clk_pll_16fft.h>
#include <clk_div.h>
#include <clk_pllctrl.h>


enum {
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_PULSAR_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_SOC_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_SOC_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVP_CLK1_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_PULSAR_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_SOC_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_SOC_GCLK,
	AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVP_CLK1_GCLK,
	AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT,
	AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_1,
	AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_2,
	AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_3,
	AM275X_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
	AM275X_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
	AM275X_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
	AM275X_FREQ_VALUE_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_EXT_REFCLK1_OUT,
	AM275X_FREQ_VALUE_BOARD_0_HFOSC1_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_I2C0_SCL_OUT,
	AM275X_FREQ_VALUE_BOARD_0_I2C1_SCL_OUT,
	AM275X_FREQ_VALUE_BOARD_0_I2C2_SCL_OUT,
	AM275X_FREQ_VALUE_BOARD_0_I2C3_SCL_OUT,
	AM275X_FREQ_VALUE_BOARD_0_I2C4_SCL_OUT,
	AM275X_FREQ_VALUE_BOARD_0_I2C5_SCL_OUT,
	AM275X_FREQ_VALUE_BOARD_0_I2C6_SCL_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP0_ACLKR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP0_ACLKX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP0_AFSR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP0_AFSX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP1_ACLKR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP1_ACLKX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP1_AFSR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP1_AFSX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP2_ACLKR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP2_ACLKX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP2_AFSR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP2_AFSX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP3_ACLKR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP3_ACLKX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP3_AFSR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP3_AFSX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP4_ACLKR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP4_ACLKX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP4_AFSR_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCASP4_AFSX_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MCU_EXT_REFCLK0_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MLB0_MLBCLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MMC0_CLKLB_OUT,
	AM275X_FREQ_VALUE_BOARD_0_MMC0_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_OSPI0_DQS_OUT,
	AM275X_FREQ_VALUE_BOARD_0_OSPI0_LBCLKO_OUT,
	AM275X_FREQ_VALUE_BOARD_0_OSPI1_DQS_OUT,
	AM275X_FREQ_VALUE_BOARD_0_OSPI1_LBCLKO_OUT,
	AM275X_FREQ_VALUE_BOARD_0_RGMII1_RXC_OUT,
	AM275X_FREQ_VALUE_BOARD_0_RGMII2_RXC_OUT,
	AM275X_FREQ_VALUE_BOARD_0_RMII1_REF_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_RMII2_REF_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_SPI0_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_SPI1_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_SPI2_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_SPI3_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_SPI4_CLK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_TCK_OUT,
	AM275X_FREQ_VALUE_BOARD_0_WKUP_I2C0_SCL_OUT,
	AM275X_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
	AM275X_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
	AM275X_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O,
	AM275X_FREQ_VALUE_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_10_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_12_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_14_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_4_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_5_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_6_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_7_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_8_TIMER_PWM,
	AM275X_FREQ_VALUE_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM,
	AM275X_FREQ_VALUE_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	AM275X_FREQ_VALUE_FSS_MAIN_0_HYPERBUS1P0_0_HPB_OUT_CLK_N,
	AM275X_FREQ_VALUE_FSS_MAIN_0_HYPERBUS1P0_0_HPB_OUT_CLK_P,
	AM275X_FREQ_VALUE_FSS_MAIN_0_OSPI_0_OSPI_OCLK_CLK,
	AM275X_FREQ_VALUE_FSS_OF_UL_MAIN_0_OSPI0_OCLK_CLK,
	AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_ACLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_ACLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AFSR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AFSX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AHCLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AHCLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_ACLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_ACLKX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AFSR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AFSX_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AHCLKR_POUT,
	AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AHCLKX_POUT,
	AM275X_FREQ_VALUE_MSHSI2C_MAIN_0_PORSCL,
	AM275X_FREQ_VALUE_MSHSI2C_MAIN_1_PORSCL,
	AM275X_FREQ_VALUE_MSHSI2C_MAIN_2_PORSCL,
	AM275X_FREQ_VALUE_MSHSI2C_MAIN_3_PORSCL,
	AM275X_FREQ_VALUE_MSHSI2C_MAIN_4_PORSCL,
	AM275X_FREQ_VALUE_MSHSI2C_MAIN_5_PORSCL,
	AM275X_FREQ_VALUE_MSHSI2C_MAIN_6_PORSCL,
	AM275X_FREQ_VALUE_MSHSI2C_WKUP_0_PORSCL,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_0,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_1,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_14,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_15,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_2,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_4,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_7,
	AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MCU_0,
	AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_0_CORE0_TAGMEM_CLK,
	AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_0_CORE1_TAGMEM_CLK,
	AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_1_CORE0_TAGMEM_CLK,
	AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_1_CORE1_TAGMEM_CLK,
	AM275X_FREQ_VALUE_SPI_MAIN_0_IO_CLKSPIO_CLK,
	AM275X_FREQ_VALUE_SPI_MAIN_1_IO_CLKSPIO_CLK,
	AM275X_FREQ_VALUE_SPI_MAIN_2_IO_CLKSPIO_CLK,
	AM275X_FREQ_VALUE_SPI_MAIN_3_IO_CLKSPIO_CLK,
	AM275X_FREQ_VALUE_SPI_MAIN_4_IO_CLKSPIO_CLK,
	AM275X_FREQ_VALUE_COUNT,
};

enum {
	AM275X_FREQ_RANGE_ANY,
	AM275X_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,
	AM275X_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
	AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
	AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
	AM275X_FREQ_RANGE_ID_MAX,
};

enum {
	PLL_ENTRY_MCU_24MHZ_1200MHZ,
	PLL_ENTRY_MCU_25MHZ_1200MHZ,
	PLL_ENTRY_MCU_26MHZ_1200MHZ_F24BIT,
	PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_25MHZ_1000MHZ,
	PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_PER0_24MHZ_960MHZ,
	PLL_ENTRY_PER0_25MHZ_960MHZ_F24BIT,
	PLL_ENTRY_PER0_26MHZ_960MHZ_F24BIT,
	PLL_ENTRY_AUDIO_24MHZ_2359_296MHZ_F24BIT,
	PLL_ENTRY_AUDIO_25MHZ_2359_296MHZ_F24BIT,
	PLL_ENTRY_AUDIO_26MHZ_2359_296MHZ_F24BIT,
	PLL_ENTRY_C7X_24MHZ_2550MHZ_F24BIT,
	PLL_ENTRY_C7X_25MHZ_2550MHZ,
	PLL_ENTRY_C7X_26MHZ_2550MHZ_F24BIT,
	PLL_ENTRY_R5SS_24MHZ_2400MHZ,
	PLL_ENTRY_R5SS_25MHZ_2400MHZ,
	PLL_ENTRY_R5SS_26MHZ_2400MHZ_F24BIT,
};

enum {
	NO_DEFAULT_FREQ,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_0_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_1_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_14_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_4_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_7_DEFAULT,
	FREQ_PLLFRACF2_SSMOD_16FFT_MCU_0_DEFAULT,
};

u32 soc_clock_values[AM275X_FREQ_VALUE_COUNT];

const struct clk_range soc_clock_ranges[AM275X_FREQ_RANGE_ID_MAX] = {
	CLK_RANGE(AM275X_FREQ_RANGE_ANY,				 0,	    ULONG_MAX),
	CLK_RANGE(AM275X_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,		 12500000,  12500000),
	CLK_RANGE(AM275X_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,	 97656,	    97656),
	CLK_RANGE(AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0, 5000000,   1200000000),
	CLK_RANGE(AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,	 800000000, 3200000000),
};

const struct clk_default soc_clock_freq_defaults[7] = {
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_0_DEFAULT,	1000000000UL, 1000000000UL, 1000000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_1_DEFAULT,	960000000UL,  960000000UL,  960000000UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_14_DEFAULT, 2400000000UL, 2400000000UL, 2400000001UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_4_DEFAULT,	2359296000UL, 2359296000UL, 2359296001UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_7_DEFAULT,	2550000000UL, 2550000000UL, 2550000001UL),
	CLK_DEFAULT(FREQ_PLLFRACF2_SSMOD_16FFT_MCU_0_DEFAULT,	1200000000UL, 1200000000UL, 1200000000UL),
};

const struct pll_table_entry soc_pll_table[18] = {
	[PLL_ENTRY_MCU_24MHZ_1200MHZ] =		     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MCU_25MHZ_1200MHZ] =		     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 1U,
		.pllm		= 96U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MCU_26MHZ_1200MHZ_F24BIT] =	     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000001U,
		.plld		= 1U,
		.pllm		= 92U,
		.pllfm		= 5162221U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 83U,
		.pllfm		= 5592406U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_25MHZ_1000MHZ] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 76U,
		.pllfm		= 15486661U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_24MHZ_960MHZ] =		     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_25MHZ_960MHZ_F24BIT] =	     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 76U,
		.pllfm		= 13421773U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_26MHZ_960MHZ_F24BIT] =	     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 1U,
		.pllm		= 73U,
		.pllfm		= 14196106U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_AUDIO_24MHZ_2359_296MHZ_F24BIT] = {
		.freq_min_hz	= 2359296000U,
		.freq_max_hz	= 2359296000U,
		.plld		= 1U,
		.pllm		= 98U,
		.pllfm		= 5100274U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_AUDIO_25MHZ_2359_296MHZ_F24BIT] = {
		.freq_min_hz	= 2359296000U,
		.freq_max_hz	= 2359296000U,
		.plld		= 1U,
		.pllm		= 94U,
		.pllfm		= 6238440U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_AUDIO_26MHZ_2359_296MHZ_F24BIT] = {
		.freq_min_hz	= 2359296000U,
		.freq_max_hz	= 2359296000U,
		.plld		= 1U,
		.pllm		= 90U,
		.pllfm		= 12451276U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_C7X_24MHZ_2550MHZ_F24BIT] =	     {
		.freq_min_hz	= 2550000000U,
		.freq_max_hz	= 2550000000U,
		.plld		= 1U,
		.pllm		= 106U,
		.pllfm		= 4194304U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_C7X_25MHZ_2550MHZ] =		     {
		.freq_min_hz	= 2550000000U,
		.freq_max_hz	= 2550000000U,
		.plld		= 1U,
		.pllm		= 102U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_C7X_26MHZ_2550MHZ_F24BIT] =	     {
		.freq_min_hz	= 2550000000U,
		.freq_max_hz	= 2550000001U,
		.plld		= 1U,
		.pllm		= 98U,
		.pllfm		= 1290556U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_R5SS_24MHZ_2400MHZ] =	     {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_R5SS_25MHZ_2400MHZ] =	     {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000000U,
		.plld		= 1U,
		.pllm		= 96U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_R5SS_26MHZ_2400MHZ_F24BIT] =	     {
		.freq_min_hz	= 2400000000U,
		.freq_max_hz	= 2400000001U,
		.plld		= 1U,
		.pllm		= 92U,
		.pllfm		= 5162221U,
		.clkod		= 1U,
	},
};

static const u8 pllfracf2_ssmod_16fft_main_0_entries[4] = {
	PLL_ENTRY_MAIN_24MHZ_1000MHZ_F24BIT,
	PLL_ENTRY_MAIN_25MHZ_1000MHZ,
	PLL_ENTRY_MAIN_26MHZ_1000MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf2_ssmod_16fft_main_1_entries[4] = {
	PLL_ENTRY_PER0_24MHZ_960MHZ,
	PLL_ENTRY_PER0_25MHZ_960MHZ_F24BIT,
	PLL_ENTRY_PER0_26MHZ_960MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf2_ssmod_16fft_main_14_entries[4] = {
	PLL_ENTRY_R5SS_24MHZ_2400MHZ,
	PLL_ENTRY_R5SS_25MHZ_2400MHZ,
	PLL_ENTRY_R5SS_26MHZ_2400MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf2_ssmod_16fft_main_4_entries[4] = {
	PLL_ENTRY_AUDIO_24MHZ_2359_296MHZ_F24BIT,
	PLL_ENTRY_AUDIO_25MHZ_2359_296MHZ_F24BIT,
	PLL_ENTRY_AUDIO_26MHZ_2359_296MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf2_ssmod_16fft_main_7_entries[4] = {
	PLL_ENTRY_C7X_24MHZ_2550MHZ_F24BIT,
	PLL_ENTRY_C7X_25MHZ_2550MHZ,
	PLL_ENTRY_C7X_26MHZ_2550MHZ_F24BIT,
	PLL_TABLE_LAST,
};
static const u8 pllfracf2_ssmod_16fft_mcu_0_entries[4] = {
	PLL_ENTRY_MCU_24MHZ_1200MHZ,
	PLL_ENTRY_MCU_25MHZ_1200MHZ,
	PLL_ENTRY_MCU_26MHZ_1200MHZ_F24BIT,
	PLL_TABLE_LAST,
};

static const struct clk_parent clk_ADC_CLK_SEL_parents[4] = {
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT4_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_ADC_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ADC_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_ADC_CLK_SEL_parents),
	},
	.reg			= 0x00100000 + 34064,
	.bit			= 0,
};
static const struct clk_parent clk_ASRC0_RXSYNC_SEL_out0_parents[32] = {
	{
		CLK_AM275X_BOARD_0_MCASP0_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSR_OUT,
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
		CLK_AM275X_BOARD_0_MCASP0_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSX_OUT,
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
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_ADC_CLK_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MLB0_MLBCLK_OUT,
		1,
	},
	{
		CLK_AM275X_ASRC_SYNC_DIV_OUT0,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
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
};
static const struct clk_data_mux_reg clk_data_ASRC0_RXSYNC_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33984,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC0_RXSYNC_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33988,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC0_RXSYNC_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33992,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC0_RXSYNC_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33996,
	.bit			= 0,
};
static const struct clk_parent clk_ASRC0_TXSYNC_SEL_out0_parents[32] = {
	{
		CLK_AM275X_BOARD_0_MCASP0_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSX_OUT,
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
		CLK_AM275X_BOARD_0_MCASP0_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSX_OUT,
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
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_ADC_CLK_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MLB0_MLBCLK_OUT,
		1,
	},
	{
		CLK_AM275X_ASRC_SYNC_DIV_OUT0,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
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
};
static const struct clk_data_mux_reg clk_data_ASRC0_TXSYNC_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34016,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC0_TXSYNC_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34020,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC0_TXSYNC_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34024,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC0_TXSYNC_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34028,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_RXSYNC_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34000,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_RXSYNC_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34004,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_RXSYNC_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34008,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_RXSYNC_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_ASRC0_RXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_RXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34012,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_TXSYNC_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34032,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_TXSYNC_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34036,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_TXSYNC_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34040,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ASRC1_TXSYNC_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_ASRC0_TXSYNC_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ASRC0_TXSYNC_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34044,
	.bit			= 0,
};
static const struct clk_data_div_reg_go clk_data_ASRC_SYNC_DIV_out0 = {
	.data_div	= {
		.n	= 16,
	},
	.reg		= 0x00100000 + 33520,
	.bit		= 0,
	.go		= 8,
};
static const struct clk_parent clk_ATL_AWS_SEL_out0_parents[16] = {
	{
		CLK_AM275X_BOARD_0_MCASP0_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP0_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
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
};
static const struct clk_data_mux_reg clk_data_ATL_AWS_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ATL_AWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_AWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33488,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ATL_AWS_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_ATL_AWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_AWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33492,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ATL_AWS_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_ATL_AWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_AWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33496,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ATL_AWS_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_ATL_AWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_AWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33500,
	.bit			= 0,
};
static const struct clk_parent clk_ATL_BWS_SEL_out0_parents[16] = {
	{
		CLK_AM275X_BOARD_0_MCASP0_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSR_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP0_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP1_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP2_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP3_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCASP4_AFSX_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
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
};
static const struct clk_data_mux_reg clk_data_ATL_BWS_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ATL_BWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_BWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33472,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ATL_BWS_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_ATL_BWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_BWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33476,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ATL_BWS_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_ATL_BWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_BWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33480,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ATL_BWS_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_ATL_BWS_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_ATL_BWS_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33484,
	.bit			= 0,
};
static const struct clk_parent clk_ATL_CLK_SEL_parents[8] = {
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT1_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		0,
		0,
	},
};
static const struct clk_data_mux_reg clk_data_ATL_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_ATL_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_ATL_CLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33456,
	.bit			= 0,
};
static const struct clk_parent clk_AUDIO_REFCLKn_out0_parents[16] = {
	{
		CLK_AM275X_MCASP_MAIN_0_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_1_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_2_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_3_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_4_MCASP_AHCLKR_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_0_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_1_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_2_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_3_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM275X_MCASP_MAIN_4_MCASP_AHCLKX_POUT,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_1,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_2,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_3,
		1,
	},
	{
		CLK_AM275X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_AUDIO_REFCLKn_out0 = {
	.data_mux		= {
		.parents	= clk_AUDIO_REFCLKn_out0_parents,
		.n		= ARRAY_SIZE(clk_AUDIO_REFCLKn_out0_parents),
	},
	.reg			= 0x00100000 + 33504,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_AUDIO_REFCLKn_out1 = {
	.data_mux		= {
		.parents	= clk_AUDIO_REFCLKn_out0_parents,
		.n		= ARRAY_SIZE(clk_AUDIO_REFCLKn_out0_parents),
	},
	.reg			= 0x00100000 + 33508,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_AUDIO_REFCLKn_out2 = {
	.data_mux		= {
		.parents	= clk_AUDIO_REFCLKn_out0_parents,
		.n		= ARRAY_SIZE(clk_AUDIO_REFCLKn_out0_parents),
	},
	.reg			= 0x00100000 + 33512,
	.bit			= 0,
};
static const struct clk_parent clk_CLKOUT0_CTRL_parents[2] = {
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT1_CLK,
		5,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT1_CLK,
		10,
	},
};
static const struct clk_data_mux_reg clk_data_CLKOUT0_CTRL_out0 = {
	.data_mux		= {
		.parents	= clk_CLKOUT0_CTRL_parents,
		.n		= ARRAY_SIZE(clk_CLKOUT0_CTRL_parents),
	},
	.reg			= 0x00100000 + 32784,
	.bit			= 0,
};
static const struct clk_parent clk_CLK_32K_RC_SEL_parents[4] = {
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		8,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM275X_GLUELOGIC_LFOSC0_CLKOUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_CLK_32K_RC_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_CLK_32K_RC_SEL_parents,
		.n		= ARRAY_SIZE(clk_CLK_32K_RC_SEL_parents),
	},
	.reg			= 0x04500000 + 32856,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_CPSW_CPTS_CLK_SEL_parents[8] = {
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT1_CLK,
		1,
	},
	{
		CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_CPSW_CPTS_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_CPSW_CPTS_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_CPSW_CPTS_CLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33088,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_EMMCSD0_IO_CLKLB_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_MMC0_CLKLB_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MMC0_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD0_IO_CLKLB_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_EMMCSD0_IO_CLKLB_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_EMMCSD0_IO_CLKLB_SEL_parents),
	},
	.reg			= 0x00100000 + 33120,
	.bit			= 16,
};
static const struct clk_parent clk_MAIN_EMMCSD0_REFCLK_SEL_parents[2] = {
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT2_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_EMMCSD0_REFCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_EMMCSD0_REFCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_EMMCSD0_REFCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33120,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_GTCCLK_SEL_parents[8] = {
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK,
		2,
	},
	{
		CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_GTCCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_GTCCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_GTCCLK_SEL_parents),
	},
	.reg			= 0x43000000 + 32816,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_MCANn_CLK_SEL_out0_parents[4] = {
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33920,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33924,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33928,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33932,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_MCANn_CLK_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_MAIN_MCANn_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_MCANn_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33936,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_OBSCLK0_MUX_SEL_parents[32] = {
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV1_16FFT_MAIN_14_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		8,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_15_HSDIVOUT0_CLK,
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
		CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK,
		1,
	},
	{
		CLK_AM275X_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK,
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
static const struct clk_data_mux_reg clk_data_MAIN_OBSCLK0_MUX_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OBSCLK0_MUX_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OBSCLK0_MUX_SEL_parents),
	},
	.reg			= 0x00100000 + 32768,
	.bit			= 0,
};
static const struct clk_data_div_reg_go clk_data_MAIN_OBSCLK_DIV_out0 = {
	.data_div	= {
		.n	= 256,
	},
	.reg		= 0x00100000 + 32768,
	.bit		= 8,
	.go		= 16,
};
static const struct clk_parent clk_MAIN_OBSCLK_OUTMUX_SEL_out0_parents[2] = {
	{
		CLK_AM275X_MAIN_OBSCLK_DIV_OUT0,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OBSCLK_OUTMUX_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OBSCLK_OUTMUX_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OBSCLK_OUTMUX_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 32768,
	.bit			= 24,
};
static const struct clk_data_mux_reg clk_data_MAIN_OBSCLK_OUTMUX_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_OBSCLK_OUTMUX_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OBSCLK_OUTMUX_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 32768,
	.bit			= 24,
};
static const struct clk_parent clk_MAIN_OSPI0_LB_CLK_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_OSPI0_DQS_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_OSPI0_LBCLKO_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI0_LB_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OSPI0_LB_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OSPI0_LB_CLK_SEL_parents),
	},
	.reg			= 0x00100000 + 34048,
	.bit			= 4,
};
static const struct clk_parent clk_MAIN_OSPI1_LB_CLK_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_OSPI1_DQS_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_OSPI1_LBCLKO_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI1_LB_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OSPI1_LB_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OSPI1_LB_CLK_SEL_parents),
	},
	.reg			= 0x00100000 + 34052,
	.bit			= 4,
};
static const struct clk_parent clk_MAIN_OSPI_REF_CLK_SEL_out0_parents[2] = {
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT1_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT5_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI_REF_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_OSPI_REF_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OSPI_REF_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34048,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_OSPI_REF_CLK_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_OSPI_REF_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_OSPI_REF_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34052,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_PLL4_HFOSC_SEL_parents[2] = {
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_PLL4_HFOSC_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_PLL4_HFOSC_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_PLL4_HFOSC_SEL_parents),
	},
	.reg			= 0x00100000 + 32880,
	.bit			= 4,
};
static const struct clk_parent clk_MAIN_TIMER11_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT11,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_10_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER11_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER11_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER11_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16940,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMER13_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT13,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_12_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER13_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER13_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER13_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16948,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMER15_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT15,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_14_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER15_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER15_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER15_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16956,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMER1_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT1,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER1_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER1_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER1_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16900,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMER3_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT3,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER3_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER3_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER3_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16908,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMER5_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT5,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_4_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER5_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER5_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER5_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16916,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMER7_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT7,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_6_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER7_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER7_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER7_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16924,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMER9_CASCADE_parents[2] = {
	{
		CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT9,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_MAIN_8_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMER9_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMER9_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMER9_CASCADE_parents),
	},
	.reg			= 0x00100000 + 16932,
	.bit			= 8,
};
static const struct clk_parent clk_MAIN_TIMERCLKn_SEL_out0_parents[16] = {
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT3_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT6_CLK,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
		1,
	},
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT2_CLK,
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
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33200,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33204,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out10 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33240,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out11 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33244,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out12 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33248,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out13 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33252,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out14 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33256,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out15 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33260,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33208,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33212,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33216,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out5 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33220,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out6 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33224,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out7 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33228,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out8 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33232,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_TIMERCLKn_SEL_out9 = {
	.data_mux		= {
		.parents	= clk_MAIN_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33236,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USART0_FCLK_SEL_parents[2] = {
	{
		CLK_AM275X_USART_CLK_DIV_OUT0,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USART0_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USART0_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USART0_FCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33408,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USART1_FCLK_SEL_parents[2] = {
	{
		CLK_AM275X_USART_CLK_DIV_OUT1,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USART1_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USART1_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USART1_FCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33412,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USART2_FCLK_SEL_parents[2] = {
	{
		CLK_AM275X_USART_CLK_DIV_OUT2,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USART2_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USART2_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USART2_FCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33416,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USART3_FCLK_SEL_parents[2] = {
	{
		CLK_AM275X_USART_CLK_DIV_OUT3,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USART3_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USART3_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USART3_FCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33420,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USART4_FCLK_SEL_parents[2] = {
	{
		CLK_AM275X_USART_CLK_DIV_OUT4,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USART4_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USART4_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USART4_FCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33424,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USART5_FCLK_SEL_parents[2] = {
	{
		CLK_AM275X_USART_CLK_DIV_OUT5,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USART5_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USART5_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USART5_FCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33428,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USART6_FCLK_SEL_parents[2] = {
	{
		CLK_AM275X_USART_CLK_DIV_OUT6,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_USART6_FCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_USART6_FCLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_MAIN_USART6_FCLK_SEL_parents),
	},
	.reg			= 0x00100000 + 33432,
	.bit			= 0,
};
static const struct clk_parent clk_MAIN_USB0_REFCLK_SEL_parents[2] = {
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
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
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33664,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33668,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33672,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33676,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33680,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MAIN_WWDTCLKn_SEL_out5 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33684,
	.bit			= 0,
};
static const struct clk_parent clk_MCU_GPIO0_CLKSEL_parents[4] = {
	{
		CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK,
		4,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_GPIO0_CLKSEL_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_GPIO0_CLKSEL_parents,
		.n		= ARRAY_SIZE(clk_MCU_GPIO0_CLKSEL_parents),
	},
	.reg			= 0x04500000 + 32880,
	.bit			= 0,
};
static const struct clk_data_div_reg_go clk_data_MCU_OBSCLK_DIV_out0 = {
	.data_div	= {
		.n	= 16,
	},
	.reg		= 0x04500000 + 32768,
	.bit		= 8,
	.go		= 16,
};
static const struct clk_parent clk_MCU_OBSCLK_MUX_SEL_parents[16] = {
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT4_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		8,
	},
	{
		CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK,
		1,
	},
	{
		CLK_AM275X_CLK_32K_RC_SEL_OUT0,
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
		CLK_AM275X_MCU_OBSCLK_DIV_OUT0,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
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
static const struct clk_parent clk_McASP0_AUXCLK_GF_SEL_parents[2] = {
	{
		CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_MCASPN_AUXCLK_SEL_OUT0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_McASP0_AUXCLK_GF_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASP0_AUXCLK_GF_SEL_parents,
		.n		= ARRAY_SIZE(clk_McASP0_AUXCLK_GF_SEL_parents),
	},
	.reg			= 0x00100000 + 33760,
	.bit			= 16,
};
static const struct clk_parent clk_McASP1_AUXCLK_GF_SEL_parents[2] = {
	{
		CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT1,
		1,
	},
	{
		CLK_AM275X_MCASPN_AUXCLK_SEL_OUT1,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_McASP1_AUXCLK_GF_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASP1_AUXCLK_GF_SEL_parents,
		.n		= ARRAY_SIZE(clk_McASP1_AUXCLK_GF_SEL_parents),
	},
	.reg			= 0x00100000 + 33764,
	.bit			= 16,
};
static const struct clk_parent clk_McASP2_AUXCLK_GF_SEL_parents[2] = {
	{
		CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT2,
		1,
	},
	{
		CLK_AM275X_MCASPN_AUXCLK_SEL_OUT2,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_McASP2_AUXCLK_GF_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASP2_AUXCLK_GF_SEL_parents,
		.n		= ARRAY_SIZE(clk_McASP2_AUXCLK_GF_SEL_parents),
	},
	.reg			= 0x00100000 + 33768,
	.bit			= 16,
};
static const struct clk_parent clk_McASP3_AUXCLK_GF_SEL_parents[2] = {
	{
		CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT3,
		1,
	},
	{
		CLK_AM275X_MCASPN_AUXCLK_SEL_OUT3,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_McASP3_AUXCLK_GF_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASP3_AUXCLK_GF_SEL_parents,
		.n		= ARRAY_SIZE(clk_McASP3_AUXCLK_GF_SEL_parents),
	},
	.reg			= 0x00100000 + 33772,
	.bit			= 16,
};
static const struct clk_parent clk_McASP4_AUXCLK_GF_SEL_parents[2] = {
	{
		CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT4,
		1,
	},
	{
		CLK_AM275X_MCASPN_AUXCLK_SEL_OUT4,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_McASP4_AUXCLK_GF_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASP4_AUXCLK_GF_SEL_parents,
		.n		= ARRAY_SIZE(clk_McASP4_AUXCLK_GF_SEL_parents),
	},
	.reg			= 0x00100000 + 33776,
	.bit			= 16,
};
static const struct clk_parent clk_McASPn_AHCLKR_SEL_out0_parents[16] = {
	{
		CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_BOARD_0_MLB0_MLBCLK_OUT,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_1,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_2,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_3,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
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
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKR_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33616,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKR_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33620,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKR_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33624,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKR_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33628,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKR_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33632,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKX_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33616,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKX_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33620,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKX_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33624,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKX_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33628,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AHCLKX_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_McASPn_AHCLKR_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AHCLKR_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33632,
	.bit			= 8,
};
static const struct clk_parent clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents[4] = {
	{
		CLK_AM275X_MAIN_PLL4_HFOSC_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_MAIN_PLL4_HFOSC_SEL_OUT0,
		2,
	},
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT0_CLK,
		1,
	},
	{
		0,
		0,
	},
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_LOCAL_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33584,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_LOCAL_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33588,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_LOCAL_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33592,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_LOCAL_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33596,
	.bit			= 8,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_LOCAL_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_LOCAL_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33600,
	.bit			= 8,
};
static const struct clk_parent clk_McASPn_AUXCLK_SEL_out0_parents[8] = {
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT8_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT6_CLK,
		1,
	},
	{
		0,
		0,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT,
		1,
	},
	{
		CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_1,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33584,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33588,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_SEL_out2 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33592,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_SEL_out3 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33596,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_McASPn_AUXCLK_SEL_out4 = {
	.data_mux		= {
		.parents	= clk_McASPn_AUXCLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_McASPn_AUXCLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 33600,
	.bit			= 0,
};
static const struct clk_parent clk_R5SS_CLK_SEL_out0_parents[2] = {
	{
		CLK_AM275X_HSDIV1_16FFT_MAIN_14_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV1_16FFT_MAIN_14_HSDIVOUT0_CLK,
		2,
	},
};
static const struct clk_data_mux_reg clk_data_R5SS_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_R5SS_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_R5SS_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34176,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_R5SS_CLK_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_R5SS_CLK_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_R5SS_CLK_SEL_out0_parents),
	},
	.reg			= 0x00100000 + 34180,
	.bit			= 0,
};
static const struct clk_parent clk_RTC_CLK_SEL_parents[2] = {
	{
		CLK_AM275X_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
};
static const struct clk_data_mux_reg clk_data_RTC_CLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_RTC_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_RTC_CLK_SEL_parents),
	},
	.reg			= 0x43000000 + 34304,
	.bit			= 0,
};
static const struct clk_parent clk_SPI0_CLK_LPBK_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_SPI0_CLK_OUT,
		1,
	},
	{
		CLK_AM275X_SPI_MAIN_0_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_SPI0_CLK_LPBK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_SPI0_CLK_LPBK_SEL_parents,
		.n		= ARRAY_SIZE(clk_SPI0_CLK_LPBK_SEL_parents),
	},
	.reg			= 0x00100000 + 33280,
	.bit			= 16,
};
static const struct clk_parent clk_SPI1_CLK_LPBK_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_SPI1_CLK_OUT,
		1,
	},
	{
		CLK_AM275X_SPI_MAIN_1_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_SPI1_CLK_LPBK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_SPI1_CLK_LPBK_SEL_parents,
		.n		= ARRAY_SIZE(clk_SPI1_CLK_LPBK_SEL_parents),
	},
	.reg			= 0x00100000 + 33284,
	.bit			= 16,
};
static const struct clk_parent clk_SPI2_CLK_LPBK_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_SPI2_CLK_OUT,
		1,
	},
	{
		CLK_AM275X_SPI_MAIN_2_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_SPI2_CLK_LPBK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_SPI2_CLK_LPBK_SEL_parents,
		.n		= ARRAY_SIZE(clk_SPI2_CLK_LPBK_SEL_parents),
	},
	.reg			= 0x00100000 + 33288,
	.bit			= 16,
};
static const struct clk_parent clk_SPI3_CLK_LPBK_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_SPI3_CLK_OUT,
		1,
	},
	{
		CLK_AM275X_SPI_MAIN_3_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_SPI3_CLK_LPBK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_SPI3_CLK_LPBK_SEL_parents,
		.n		= ARRAY_SIZE(clk_SPI3_CLK_LPBK_SEL_parents),
	},
	.reg			= 0x00100000 + 33292,
	.bit			= 16,
};
static const struct clk_parent clk_SPI4_CLK_LPBK_SEL_parents[2] = {
	{
		CLK_AM275X_BOARD_0_SPI4_CLK_OUT,
		1,
	},
	{
		CLK_AM275X_SPI_MAIN_4_IO_CLKSPIO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_SPI4_CLK_LPBK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_SPI4_CLK_LPBK_SEL_parents,
		.n		= ARRAY_SIZE(clk_SPI4_CLK_LPBK_SEL_parents),
	},
	.reg			= 0x00100000 + 33296,
	.bit			= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_CLK_DIV_out0 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x00100000 + 33344,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_CLK_DIV_out1 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x00100000 + 33348,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_CLK_DIV_out2 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x00100000 + 33352,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_CLK_DIV_out3 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x00100000 + 33356,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_CLK_DIV_out4 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x00100000 + 33360,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_CLK_DIV_out5 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x00100000 + 33364,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_data_div_reg_go clk_data_USART_CLK_DIV_out6 = {
	.data_div	= {
		.n	= 4,
	},
	.reg		= 0x00100000 + 33368,
	.bit		= 0,
	.go		= 16,
};
static const struct clk_parent clk_WKUP_CLKOUT_SEL_parents[8] = {
	{
		0,
		0,
	},
	{
		CLK_AM275X_GLUELOGIC_LFOSC0_CLKOUT,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT2_CLK,
		1,
	},
	{
		CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT9_CLK,
		1,
	},
	{
		CLK_AM275X_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_CLKOUT_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_WKUP_CLKOUT_SEL_parents,
		.n		= ARRAY_SIZE(clk_WKUP_CLKOUT_SEL_parents),
	},
	.reg			= 0x43000000 + 32800,
	.bit			= 0,
};
static const struct clk_parent clk_WKUP_CLKOUT_SEL_IO_parents[2] = {
	{
		CLK_AM275X_WKUP_CLKOUT_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_CLKOUT_SEL_IO_out0 = {
	.data_mux		= {
		.parents	= clk_WKUP_CLKOUT_SEL_IO_parents,
		.n		= ARRAY_SIZE(clk_WKUP_CLKOUT_SEL_IO_parents),
	},
	.reg			= 0x43000000 + 32800,
	.bit			= 24,
};
static const struct clk_parent clk_WKUP_CLKSEL_parents[2] = {
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_15_HSDIVOUT0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_CLKSEL_out0 = {
	.data_mux		= {
		.parents	= clk_WKUP_CLKSEL_parents,
		.n		= ARRAY_SIZE(clk_WKUP_CLKSEL_parents),
	},
	.reg			= 0x43000000 + 32784,
	.bit			= 0,
};
static const struct clk_parent clk_WKUP_CLKSEL_R5FSS_parents[2] = {
	{
		CLK_AM275X_HSDIV3_16FFT_MAIN_15_HSDIVOUT2_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_CLKSEL_R5FSS_out0 = {
	.data_mux		= {
		.parents	= clk_WKUP_CLKSEL_R5FSS_parents,
		.n		= ARRAY_SIZE(clk_WKUP_CLKSEL_R5FSS_parents),
	},
	.reg			= 0x43000000 + 32784,
	.bit			= 0,
};
static const struct clk_parent clk_WKUP_TIMER1_CASCADE_parents[2] = {
	{
		CLK_AM275X_WKUP_TIMERCLKN_SEL_OUT1,
		1,
	},
	{
		CLK_AM275X_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_TIMER1_CASCADE_out0 = {
	.data_mux		= {
		.parents	= clk_WKUP_TIMER1_CASCADE_parents,
		.n		= ARRAY_SIZE(clk_WKUP_TIMER1_CASCADE_parents),
	},
	.reg			= 0x43000000 + 16900,
	.bit			= 8,
};
static const struct clk_parent clk_WKUP_TIMERCLKn_SEL_out0_parents[8] = {
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_WKUP_CLKSEL_OUT0,
		2,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT,
		1,
	},
	{
		CLK_AM275X_POSTDIV1_16FFT_MCU_0_HSDIVOUT5_CLK,
		1,
	},
	{
		CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM275X_CLK_32K_RC_SEL_OUT0,
		1,
	},
	{
		CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		1,
	},
	{
		CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
		3,
	},
};
static const struct clk_data_mux_reg clk_data_WKUP_TIMERCLKn_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_WKUP_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_WKUP_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33200,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_WKUP_TIMERCLKn_SEL_out1 = {
	.data_mux		= {
		.parents	= clk_WKUP_TIMERCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_WKUP_TIMERCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33204,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_WKUP_WWDTCLK_SEL_out0 = {
	.data_mux		= {
		.parents	= clk_MAIN_WWDTCLKn_SEL_out0_parents,
		.n		= ARRAY_SIZE(clk_MAIN_WWDTCLKn_SEL_out0_parents),
	},
	.reg			= 0x43000000 + 33664,
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_c7xv_divh_clk4_obsclk_out_clk = {
	.dev		= AM275X_DEV_C7X256V0_CLK,
	.clk_idx	= AM275X_DEV_C7X256V0_CLK_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk2_pulsar_gclk = {
	.dev		= AM275X_DEV_C7X256V0_CLK,
	.clk_idx	= AM275X_DEV_C7X256V0_CLK_DIVH_CLK2_PULSAR_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk2_soc_gclk = {
	.dev		= AM275X_DEV_C7X256V0_CLK,
	.clk_idx	= AM275X_DEV_C7X256V0_CLK_DIVH_CLK2_SOC_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk4_gclk = {
	.dev		= AM275X_DEV_C7X256V0_CLK,
	.clk_idx	= AM275X_DEV_C7X256V0_CLK_DIVH_CLK4_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk4_soc_gclk = {
	.dev		= AM275X_DEV_C7X256V0_CLK,
	.clk_idx	= AM275X_DEV_C7X256V0_CLK_DIVH_CLK4_SOC_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divp_clk1_gclk = {
	.dev		= AM275X_DEV_C7X256V0_CLK,
	.clk_idx	= AM275X_DEV_C7X256V0_CLK_DIVP_CLK1_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_c7xv_divh_clk4_obsclk_out_clk = {
	.dev		= AM275X_DEV_C7X256V1_CLK,
	.clk_idx	= AM275X_DEV_C7X256V1_CLK_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk2_pulsar_gclk = {
	.dev		= AM275X_DEV_C7X256V1_CLK,
	.clk_idx	= AM275X_DEV_C7X256V1_CLK_DIVH_CLK2_PULSAR_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk2_soc_gclk = {
	.dev		= AM275X_DEV_C7X256V1_CLK,
	.clk_idx	= AM275X_DEV_C7X256V1_CLK_DIVH_CLK2_SOC_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk4_gclk = {
	.dev		= AM275X_DEV_C7X256V1_CLK,
	.clk_idx	= AM275X_DEV_C7X256V1_CLK_DIVH_CLK4_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk4_soc_gclk = {
	.dev		= AM275X_DEV_C7X256V1_CLK,
	.clk_idx	= AM275X_DEV_C7X256V1_CLK_DIVH_CLK4_SOC_GCLK,
};
static const struct clk_data_from_dev clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divp_clk1_gclk = {
	.dev		= AM275X_DEV_C7X256V1_CLK,
	.clk_idx	= AM275X_DEV_C7X256V1_CLK_DIVP_CLK1_GCLK,
};
static const struct clk_data_from_dev clk_data_atl_main_0_atl_io_port_atclk_out = {
	.dev		= AM275X_DEV_ATL0,
	.clk_idx	= AM275X_DEV_ATL0_ATL_IO_PORT_ATCLK_OUT,
};
static const struct clk_data_from_dev clk_data_atl_main_0_atl_io_port_atclk_out_1 = {
	.dev		= AM275X_DEV_ATL0,
	.clk_idx	= AM275X_DEV_ATL0_ATL_IO_PORT_ATCLK_OUT_1,
};
static const struct clk_data_from_dev clk_data_atl_main_0_atl_io_port_atclk_out_2 = {
	.dev		= AM275X_DEV_ATL0,
	.clk_idx	= AM275X_DEV_ATL0_ATL_IO_PORT_ATCLK_OUT_2,
};
static const struct clk_data_from_dev clk_data_atl_main_0_atl_io_port_atclk_out_3 = {
	.dev		= AM275X_DEV_ATL0,
	.clk_idx	= AM275X_DEV_ATL0_ATL_IO_PORT_ATCLK_OUT_3,
};
static const struct clk_data_from_dev clk_data_board_0_hfosc1_clk_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_HFOSC1_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_AUDIO_EXT_REFCLK0_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_AUDIO_EXT_REFCLK0_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_AUDIO_EXT_REFCLK1_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_AUDIO_EXT_REFCLK1_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_AUDIO_EXT_REFCLK2_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_AUDIO_EXT_REFCLK2_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_CP_GEMAC_CPTS0_RFT_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_EXT_REFCLK1_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_EXT_REFCLK1_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C0_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_I2C0_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C1_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_I2C1_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C2_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_I2C2_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C3_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_I2C3_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C4_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_I2C4_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C5_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_I2C5_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_I2C6_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_I2C6_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_ACLKR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP0_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_ACLKX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP0_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_AFSR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP0_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP0_AFSX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP0_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_ACLKR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP1_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_ACLKX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP1_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_AFSR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP1_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP1_AFSX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP1_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_ACLKR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP2_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_ACLKX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP2_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_AFSR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP2_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP2_AFSX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP2_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP3_ACLKR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP3_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP3_ACLKX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP3_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP3_AFSR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP3_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP3_AFSX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP3_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP4_ACLKR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP4_ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP4_ACLKX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP4_ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP4_AFSR_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP4_AFSR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCASP4_AFSX_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCASP4_AFSX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MCU_EXT_REFCLK0_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MCU_EXT_REFCLK0_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MLB0_MLBCLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MLB0_MLBCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC0_CLKLB_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MMC0_CLKLB_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_MMC0_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_MMC0_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI0_DQS_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_OSPI0_DQS_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI0_LBCLKO_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_OSPI0_LBCLKO_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI1_DQS_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_OSPI1_DQS_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_OSPI1_LBCLKO_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_OSPI1_LBCLKO_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RGMII1_RXC_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_RGMII1_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RGMII2_RXC_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_RGMII2_RXC_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RMII1_REF_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_RMII1_REF_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_RMII2_REF_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_RMII2_REF_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI0_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_SPI0_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI1_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_SPI1_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI2_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_SPI2_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI3_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_SPI3_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_SPI4_CLK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_SPI4_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_TCK_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_TCK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_WKUP_I2C0_SCL_out = {
	.dev		= AM275X_DEV_BOARD0,
	.clk_idx	= AM275X_DEV_BOARD0_WKUP_I2C0_SCL_OUT,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_am62l_main_0_cpts_genf0 = {
	.dev		= AM275X_DEV_CPSW0,
	.clk_idx	= AM275X_DEV_CPSW0_CPTS_GENF0,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_am62l_main_0_cpts_genf1 = {
	.dev		= AM275X_DEV_CPSW0,
	.clk_idx	= AM275X_DEV_CPSW0_CPTS_GENF1,
};
static const struct clk_data_from_dev clk_data_cpsw_3guss_am62l_main_0_mdio_mdclk_o = {
	.dev		= AM275X_DEV_CPSW0,
	.clk_idx	= AM275X_DEV_CPSW0_MDIO_MDCLK_O,
};
static const struct clk_data_from_dev clk_data_debugss_k3_wrap_cv0_main_0_cstpiu_traceclk = {
	.dev		= AM275X_DEV_DEBUGSS_WRAP0,
	.clk_idx	= AM275X_DEV_DEBUGSS_WRAP0_CSTPIU_TRACECLK,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_0_timer_pwm = {
	.dev		= AM275X_DEV_TIMER0,
	.clk_idx	= AM275X_DEV_TIMER0_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_1_timer_pwm = {
	.dev		= AM275X_DEV_TIMER1,
	.clk_idx	= AM275X_DEV_TIMER1_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_10_timer_pwm = {
	.dev		= AM275X_DEV_TIMER10,
	.clk_idx	= AM275X_DEV_TIMER10_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_12_timer_pwm = {
	.dev		= AM275X_DEV_TIMER12,
	.clk_idx	= AM275X_DEV_TIMER12_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_14_timer_pwm = {
	.dev		= AM275X_DEV_TIMER14,
	.clk_idx	= AM275X_DEV_TIMER14_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_2_timer_pwm = {
	.dev		= AM275X_DEV_TIMER2,
	.clk_idx	= AM275X_DEV_TIMER2_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_3_timer_pwm = {
	.dev		= AM275X_DEV_TIMER3,
	.clk_idx	= AM275X_DEV_TIMER3_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_4_timer_pwm = {
	.dev		= AM275X_DEV_TIMER4,
	.clk_idx	= AM275X_DEV_TIMER4_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_5_timer_pwm = {
	.dev		= AM275X_DEV_TIMER5,
	.clk_idx	= AM275X_DEV_TIMER5_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_6_timer_pwm = {
	.dev		= AM275X_DEV_TIMER6,
	.clk_idx	= AM275X_DEV_TIMER6_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_7_timer_pwm = {
	.dev		= AM275X_DEV_TIMER7,
	.clk_idx	= AM275X_DEV_TIMER7_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_main_8_timer_pwm = {
	.dev		= AM275X_DEV_TIMER8,
	.clk_idx	= AM275X_DEV_TIMER8_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_dmtimer_dmc1ms_wkup_0_timer_pwm = {
	.dev		= AM275X_DEV_WKUP_TIMER0,
	.clk_idx	= AM275X_DEV_WKUP_TIMER0_TIMER_PWM,
};
static const struct clk_data_from_dev clk_data_emmcsd8ss_main_0_emmcsdss_io_clk_o = {
	.dev		= AM275X_DEV_MMCSD0,
	.clk_idx	= AM275X_DEV_MMCSD0_EMMCSDSS_IO_CLK_O,
};
static const struct clk_data_from_dev clk_data_fss_main_0_hyperbus1p0_0_hpb_out_clk_n = {
	.dev		= AM275X_DEV_FSS1_HYPERBUS1P0_0,
	.clk_idx	= AM275X_DEV_FSS1_HYPERBUS1P0_0_HPB_OUT_CLK_N,
};
static const struct clk_data_from_dev clk_data_fss_main_0_hyperbus1p0_0_hpb_out_clk_p = {
	.dev		= AM275X_DEV_FSS1_HYPERBUS1P0_0,
	.clk_idx	= AM275X_DEV_FSS1_HYPERBUS1P0_0_HPB_OUT_CLK_P,
};
static const struct clk_data_from_dev clk_data_fss_main_0_ospi_0_ospi_oclk_clk = {
	.dev		= AM275X_DEV_FSS1_OSPI_0,
	.clk_idx	= AM275X_DEV_FSS1_OSPI_0_OSPI_OCLK_CLK,
};
static const struct clk_data_from_dev clk_data_fss_of_ul_main_0_ospi0_oclk_clk = {
	.dev		= AM275X_DEV_FSS0,
	.clk_idx	= AM275X_DEV_FSS0_OSPI0_OCLK_CLK,
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
	.data_div	= {
		.n	= 128,
	},
	.reg		= 0x00680000UL + (0x1000UL * 14UL) + 0x80UL + (0x4UL * 1UL),
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv1_16fft_main_7_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 3,
	},
	.reg			= 0x00680000UL + (0x1000UL * 7UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv1_16fft_main_7_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 3,
	},
	.reg			= 0x00680000UL + (0x1000UL * 7UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_15_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 6,
	},
	.reg			= 0x00680000UL + (0x1000UL * 15UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_15_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 15UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_15_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 3,
	},
	.reg			= 0x00680000UL + (0x1000UL * 15UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_15_hsdiv3 = {
	.data_div	= {
		.n	= 128,
	},
	.reg		= 0x00680000UL + (0x1000UL * 15UL) + 0x80UL + (0x4UL * 3UL),
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_4_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 24,
	},
	.reg			= 0x00680000UL + (0x1000UL * 4UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_4_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 3,
	},
	.reg			= 0x00680000UL + (0x1000UL * 4UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_4_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 12,
	},
	.reg			= 0x00680000UL + (0x1000UL * 4UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv3_16fft_main_4_hsdiv3 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 96,
	},
	.reg			= 0x00680000UL + (0x1000UL * 4UL) + 0x80UL + (0x4UL * 3UL),
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
		.default_div	= 15,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 10,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv3 = {
	.data_div	= {
		.n	= 128,
	},
	.reg		= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 3UL),
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_0_hsdiv4 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 100,
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
		.default_div	= 10,
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
		.default_div	= 32,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 4UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_2_hsdiv0 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 10,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 0UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_2_hsdiv1 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 8,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 1UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_main_2_hsdiv2 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 10,
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
		.default_div	= 6,
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
		.default_div	= 50,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 2UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_mcu_0_hsdiv3 = {
	.data_div	= {
		.n	= 128,
	},
	.reg		= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 3UL),
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_hsdiv4_16fft_mcu_0_hsdiv4 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 120,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 4UL),
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_aclkr_pout = {
	.dev		= AM275X_DEV_MCASP0,
	.clk_idx	= AM275X_DEV_MCASP0_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_aclkx_pout = {
	.dev		= AM275X_DEV_MCASP0,
	.clk_idx	= AM275X_DEV_MCASP0_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_afsr_pout = {
	.dev		= AM275X_DEV_MCASP0,
	.clk_idx	= AM275X_DEV_MCASP0_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_afsx_pout = {
	.dev		= AM275X_DEV_MCASP0,
	.clk_idx	= AM275X_DEV_MCASP0_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_ahclkr_pout = {
	.dev		= AM275X_DEV_MCASP0,
	.clk_idx	= AM275X_DEV_MCASP0_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_0_mcasp_ahclkx_pout = {
	.dev		= AM275X_DEV_MCASP0,
	.clk_idx	= AM275X_DEV_MCASP0_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_aclkr_pout = {
	.dev		= AM275X_DEV_MCASP1,
	.clk_idx	= AM275X_DEV_MCASP1_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_aclkx_pout = {
	.dev		= AM275X_DEV_MCASP1,
	.clk_idx	= AM275X_DEV_MCASP1_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_afsr_pout = {
	.dev		= AM275X_DEV_MCASP1,
	.clk_idx	= AM275X_DEV_MCASP1_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_afsx_pout = {
	.dev		= AM275X_DEV_MCASP1,
	.clk_idx	= AM275X_DEV_MCASP1_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_ahclkr_pout = {
	.dev		= AM275X_DEV_MCASP1,
	.clk_idx	= AM275X_DEV_MCASP1_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_1_mcasp_ahclkx_pout = {
	.dev		= AM275X_DEV_MCASP1,
	.clk_idx	= AM275X_DEV_MCASP1_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_aclkr_pout = {
	.dev		= AM275X_DEV_MCASP2,
	.clk_idx	= AM275X_DEV_MCASP2_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_aclkx_pout = {
	.dev		= AM275X_DEV_MCASP2,
	.clk_idx	= AM275X_DEV_MCASP2_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_afsr_pout = {
	.dev		= AM275X_DEV_MCASP2,
	.clk_idx	= AM275X_DEV_MCASP2_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_afsx_pout = {
	.dev		= AM275X_DEV_MCASP2,
	.clk_idx	= AM275X_DEV_MCASP2_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_ahclkr_pout = {
	.dev		= AM275X_DEV_MCASP2,
	.clk_idx	= AM275X_DEV_MCASP2_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_2_mcasp_ahclkx_pout = {
	.dev		= AM275X_DEV_MCASP2,
	.clk_idx	= AM275X_DEV_MCASP2_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_3_mcasp_aclkr_pout = {
	.dev		= AM275X_DEV_MCASP3,
	.clk_idx	= AM275X_DEV_MCASP3_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_3_mcasp_aclkx_pout = {
	.dev		= AM275X_DEV_MCASP3,
	.clk_idx	= AM275X_DEV_MCASP3_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_3_mcasp_afsr_pout = {
	.dev		= AM275X_DEV_MCASP3,
	.clk_idx	= AM275X_DEV_MCASP3_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_3_mcasp_afsx_pout = {
	.dev		= AM275X_DEV_MCASP3,
	.clk_idx	= AM275X_DEV_MCASP3_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_3_mcasp_ahclkr_pout = {
	.dev		= AM275X_DEV_MCASP3,
	.clk_idx	= AM275X_DEV_MCASP3_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_3_mcasp_ahclkx_pout = {
	.dev		= AM275X_DEV_MCASP3,
	.clk_idx	= AM275X_DEV_MCASP3_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_4_mcasp_aclkr_pout = {
	.dev		= AM275X_DEV_MCASP4,
	.clk_idx	= AM275X_DEV_MCASP4_MCASP_ACLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_4_mcasp_aclkx_pout = {
	.dev		= AM275X_DEV_MCASP4,
	.clk_idx	= AM275X_DEV_MCASP4_MCASP_ACLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_4_mcasp_afsr_pout = {
	.dev		= AM275X_DEV_MCASP4,
	.clk_idx	= AM275X_DEV_MCASP4_MCASP_AFSR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_4_mcasp_afsx_pout = {
	.dev		= AM275X_DEV_MCASP4,
	.clk_idx	= AM275X_DEV_MCASP4_MCASP_AFSX_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_4_mcasp_ahclkr_pout = {
	.dev		= AM275X_DEV_MCASP4,
	.clk_idx	= AM275X_DEV_MCASP4_MCASP_AHCLKR_POUT,
};
static const struct clk_data_from_dev clk_data_mcasp_main_4_mcasp_ahclkx_pout = {
	.dev		= AM275X_DEV_MCASP4,
	.clk_idx	= AM275X_DEV_MCASP4_MCASP_AHCLKX_POUT,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_0_porscl = {
	.dev		= AM275X_DEV_I2C0,
	.clk_idx	= AM275X_DEV_I2C0_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_1_porscl = {
	.dev		= AM275X_DEV_I2C1,
	.clk_idx	= AM275X_DEV_I2C1_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_2_porscl = {
	.dev		= AM275X_DEV_I2C2,
	.clk_idx	= AM275X_DEV_I2C2_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_3_porscl = {
	.dev		= AM275X_DEV_I2C3,
	.clk_idx	= AM275X_DEV_I2C3_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_4_porscl = {
	.dev		= AM275X_DEV_I2C4,
	.clk_idx	= AM275X_DEV_I2C4_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_5_porscl = {
	.dev		= AM275X_DEV_I2C5,
	.clk_idx	= AM275X_DEV_I2C5_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_main_6_porscl = {
	.dev		= AM275X_DEV_I2C6,
	.clk_idx	= AM275X_DEV_I2C6_PORSCL,
};
static const struct clk_data_from_dev clk_data_mshsi2c_wkup_0_porscl = {
	.dev		= AM275X_DEV_WKUP_I2C0,
	.clk_idx	= AM275X_DEV_WKUP_I2C0_PORSCL,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_0 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_main_0_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_0_DEFAULT,
	},
	.idx				= 0,
	.base				= 0x00680000,
};
static const struct clk_data_div clk_data_pllfracf2_ssmod_16fft_main_0_postdiv = {
	.n	= 49,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_1 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_main_1_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_1_DEFAULT,
	},
	.idx				= 1,
	.base				= 0x00680000,
};
static const struct clk_data_div clk_data_pllfracf2_ssmod_16fft_main_1_postdiv = {
	.n	= 49,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_14 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_main_14_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_14_DEFAULT,
	},
	.idx				= 14,
	.base				= 0x00680000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_15 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_main_14_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_14_DEFAULT,
	},
	.idx				= 15,
	.base				= 0x00680000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_2 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_main_0_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_0_DEFAULT,
	},
	.idx				= 2,
	.base				= 0x00680000,
};
static const struct clk_data_div clk_data_pllfracf2_ssmod_16fft_main_2_postdiv = {
	.n	= 49,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_4 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_main_4_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_4_DEFAULT,
	},
	.idx				= 4,
	.base				= 0x00680000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_main_7 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_main_7_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MAIN_7_DEFAULT,
	},
	.idx				= 7,
	.base				= 0x00680000,
};
static const struct clk_data_pll_16fft clk_data_pllfracf2_ssmod_16fft_mcu_0 = {
	.data_pll			= {
		.vco_range_idx		= AM275X_FREQ_RANGE_VCO_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.vco_in_range_idx	= AM275X_FREQ_RANGE_VCO_IN_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= pllfracf2_ssmod_16fft_mcu_0_entries,
		.default_freq_idx	= FREQ_PLLFRACF2_SSMOD_16FFT_MCU_0_DEFAULT,
	},
	.idx				= 0,
	.base				= 0x04040000,
};
static const struct clk_data_div clk_data_pllfracf2_ssmod_16fft_mcu_0_postdiv = {
	.n	= 49,
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
		.default_div	= 10,
	},
	.reg			= 0x00680000UL + (0x1000UL * 1UL) + 0x80UL + (0x4UL * 6UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv1_16fft_mcu_0_hsdiv5 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 6,
	},
	.reg			= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 5UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv1_16fft_mcu_0_hsdiv6 = {
	.data_div	= {
		.n	= 128,
	},
	.reg		= 0x04040000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 6UL),
	.bit		= 0,
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
		.default_div	= 20,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 8UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_0_hsdiv9 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 0UL) + 0x80UL + (0x4UL * 9UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv5 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 2,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 5UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv6 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 5,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 6UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv7 = {
	.data_div	= {
		.n	= 128,
	},
	.reg		= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 7UL),
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv8 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 10,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 8UL),
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_postdiv4_16ff_main_2_hsdiv9 = {
	.data_div		= {
		.n		= 128,
		.default_div	= 40,
	},
	.reg			= 0x00680000UL + (0x1000UL * 2UL) + 0x80UL + (0x4UL * 9UL),
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_rl2_of_cba4_main_r5_0_core0_tagmem_clk = {
	.dev		= AM275X_DEV_RL2_OF_CBA4_0,
	.clk_idx	= AM275X_DEV_RL2_OF_CBA4_0_TAGMEM_CLK,
};
static const struct clk_data_from_dev clk_data_rl2_of_cba4_main_r5_0_core1_tagmem_clk = {
	.dev		= AM275X_DEV_RL2_OF_CBA4_1,
	.clk_idx	= AM275X_DEV_RL2_OF_CBA4_1_TAGMEM_CLK,
};
static const struct clk_data_from_dev clk_data_rl2_of_cba4_main_r5_1_core0_tagmem_clk = {
	.dev		= AM275X_DEV_RL2_OF_CBA4_2,
	.clk_idx	= AM275X_DEV_RL2_OF_CBA4_2_TAGMEM_CLK,
};
static const struct clk_data_from_dev clk_data_rl2_of_cba4_main_r5_1_core1_tagmem_clk = {
	.dev		= AM275X_DEV_RL2_OF_CBA4_3,
	.clk_idx	= AM275X_DEV_RL2_OF_CBA4_3_TAGMEM_CLK,
};
static const struct clk_parent clk_sam62_pll_ctrl_wrap_main_0_parents[2] = {
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_sam62_pll_ctrl_wrap_main_0_sysclkout_clk = {
	.data_mux		= {
		.parents	= clk_sam62_pll_ctrl_wrap_main_0_parents,
		.n		= ARRAY_SIZE(clk_sam62_pll_ctrl_wrap_main_0_parents),
	},
	.reg			= 0x00410000,
};
static const struct clk_data_div_reg clk_data_sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00410000 + 0x118,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_sam62_pll_ctrl_wrap_main_0_chip_div24_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00410000 + 0x11c,
	.bit		= 0,
};
static const struct clk_parent clk_sam62_pll_ctrl_wrap_mcu_0_parents[2] = {
	{
		CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
		1,
	},
	{
		CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk = {
	.data_mux		= {
		.parents	= clk_sam62_pll_ctrl_wrap_mcu_0_parents,
		.n		= ARRAY_SIZE(clk_sam62_pll_ctrl_wrap_mcu_0_parents),
	},
	.reg			= 0x04020000,
};
static const struct clk_data_div_reg clk_data_sam62_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x04020000 + 0x118,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_sam62_pll_ctrl_wrap_mcu_0_chip_div24_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x04020000 + 0x11c,
	.bit		= 0,
};
static const struct clk_data_from_dev clk_data_spi_main_0_io_clkspio_clk = {
	.dev		= AM275X_DEV_MCSPI0,
	.clk_idx	= AM275X_DEV_MCSPI0_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_1_io_clkspio_clk = {
	.dev		= AM275X_DEV_MCSPI1,
	.clk_idx	= AM275X_DEV_MCSPI1_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_2_io_clkspio_clk = {
	.dev		= AM275X_DEV_MCSPI2,
	.clk_idx	= AM275X_DEV_MCSPI2_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_3_io_clkspio_clk = {
	.dev		= AM275X_DEV_MCSPI3,
	.clk_idx	= AM275X_DEV_MCSPI3_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_4_io_clkspio_clk = {
	.dev		= AM275X_DEV_MCSPI4,
	.clk_idx	= AM275X_DEV_MCSPI4_IO_CLKSPIO_CLK,
};

const struct clk_data soc_clock_data[346] = {
	[CLK_AM275X_GLUELOGIC_HFOSC0_CLK] =						      {
		.drv	= &clk_drv_soc_hfosc0,
		.flags	= 0,
	},
	[CLK_AM275X_GLUELOGIC_LFOSC0_CLKOUT] =						      {
		.drv	= &clk_drv_soc_lfosc0,
		.flags	= 0,
	},
	[CLK_AM275X_GLUELOGIC_RCOSC_CLKOUT] =						      {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM275X_FREQ_RANGE_GLUELOGIC_RCOSC_CLKOUT,
	},
	[CLK_AM275X_GLUELOGIC_RCOSC_CLK_1P0V_97P65K] =					      {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM275X_FREQ_RANGE_GLUELOGIC_RCOSC_CLK_1P0V_97P65K,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK] = {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_c7xv_divh_clk4_obsclk_out_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_PULSAR_GCLK] =	      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk2_pulsar_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_PULSAR_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_SOC_GCLK] =	      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk2_soc_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_SOC_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_GCLK] =		      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk4_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_SOC_GCLK] =	      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divh_clk4_soc_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_SOC_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVP_CLK1_GCLK] =		      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_0_main_0_clock_control_0_divp_clk1_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_0_MAIN_0_CLOCK_CONTROL_0_DIVP_CLK1_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK] = {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_c7xv_divh_clk4_obsclk_out_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_C7XV_DIVH_CLK4_OBSCLK_OUT_CLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_PULSAR_GCLK] =	      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk2_pulsar_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_PULSAR_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_SOC_GCLK] =	      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk2_soc_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK2_SOC_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_GCLK] =		      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk4_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_SOC_GCLK] =	      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divh_clk4_soc_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVH_CLK4_SOC_GCLK,
	},
	[CLK_AM275X_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVP_CLK1_GCLK] =		      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_am275_c7xv_wrap_1_main_0_clock_control_0_divp_clk1_gclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_AM275_C7XV_WRAP_1_MAIN_0_CLOCK_CONTROL_0_DIVP_CLK1_GCLK,
	},
	[CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_atl_main_0_atl_io_port_atclk_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT,
	},
	[CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_1] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_atl_main_0_atl_io_port_atclk_out_1.data,
		.freq_idx	= AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_1,
	},
	[CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_2] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_atl_main_0_atl_io_port_atclk_out_2.data,
		.freq_idx	= AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_2,
	},
	[CLK_AM275X_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_3] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_atl_main_0_atl_io_port_atclk_out_3.data,
		.freq_idx	= AM275X_FREQ_VALUE_ATL_MAIN_0_ATL_IO_PORT_ATCLK_OUT_3,
	},
	[CLK_AM275X_BOARD_0_HFOSC1_CLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_hfosc1_clk_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_HFOSC1_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK0_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_AUDIO_EXT_REFCLK0_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
	},
	[CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK1_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_AUDIO_EXT_REFCLK1_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
	},
	[CLK_AM275X_BOARD_0_AUDIO_EXT_REFCLK2_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_AUDIO_EXT_REFCLK2_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_AUDIO_EXT_REFCLK2_OUT,
	},
	[CLK_AM275X_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_CP_GEMAC_CPTS0_RFT_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_EXT_REFCLK1_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_EXT_REFCLK1_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_EXT_REFCLK1_OUT,
	},
	[CLK_AM275X_BOARD_0_I2C0_SCL_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C0_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_I2C0_SCL_OUT,
	},
	[CLK_AM275X_BOARD_0_I2C1_SCL_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C1_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_I2C1_SCL_OUT,
	},
	[CLK_AM275X_BOARD_0_I2C2_SCL_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C2_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_I2C2_SCL_OUT,
	},
	[CLK_AM275X_BOARD_0_I2C3_SCL_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C3_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_I2C3_SCL_OUT,
	},
	[CLK_AM275X_BOARD_0_I2C4_SCL_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C4_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_I2C4_SCL_OUT,
	},
	[CLK_AM275X_BOARD_0_I2C5_SCL_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C5_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_I2C5_SCL_OUT,
	},
	[CLK_AM275X_BOARD_0_I2C6_SCL_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_I2C6_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_I2C6_SCL_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP0_ACLKR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP0_ACLKR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP0_ACLKR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP0_ACLKX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP0_ACLKX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP0_ACLKX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP0_AFSR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP0_AFSR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP0_AFSR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP0_AFSX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP0_AFSX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP0_AFSX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP1_ACLKR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP1_ACLKR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP1_ACLKR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP1_ACLKX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP1_ACLKX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP1_ACLKX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP1_AFSR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP1_AFSR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP1_AFSR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP1_AFSX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP1_AFSX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP1_AFSX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP2_ACLKR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP2_ACLKR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP2_ACLKR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP2_ACLKX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP2_ACLKX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP2_ACLKX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP2_AFSR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP2_AFSR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP2_AFSR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP2_AFSX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP2_AFSX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP2_AFSX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP3_ACLKR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP3_ACLKR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP3_ACLKR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP3_ACLKX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP3_ACLKX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP3_ACLKX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP3_AFSR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP3_AFSR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP3_AFSR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP3_AFSX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP3_AFSX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP3_AFSX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP4_ACLKR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP4_ACLKR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP4_ACLKR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP4_ACLKX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP4_ACLKX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP4_ACLKX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP4_AFSR_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP4_AFSR_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP4_AFSR_OUT,
	},
	[CLK_AM275X_BOARD_0_MCASP4_AFSX_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCASP4_AFSX_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCASP4_AFSX_OUT,
	},
	[CLK_AM275X_BOARD_0_MCU_EXT_REFCLK0_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MCU_EXT_REFCLK0_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MCU_EXT_REFCLK0_OUT,
	},
	[CLK_AM275X_BOARD_0_MLB0_MLBCLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MLB0_MLBCLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MLB0_MLBCLK_OUT,
	},
	[CLK_AM275X_BOARD_0_MMC0_CLKLB_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MMC0_CLKLB_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MMC0_CLKLB_OUT,
	},
	[CLK_AM275X_BOARD_0_MMC0_CLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_MMC0_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_MMC0_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_OSPI0_DQS_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_OSPI0_DQS_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_OSPI0_DQS_OUT,
	},
	[CLK_AM275X_BOARD_0_OSPI0_LBCLKO_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_OSPI0_LBCLKO_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_OSPI0_LBCLKO_OUT,
	},
	[CLK_AM275X_BOARD_0_OSPI1_DQS_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_OSPI1_DQS_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_OSPI1_DQS_OUT,
	},
	[CLK_AM275X_BOARD_0_OSPI1_LBCLKO_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_OSPI1_LBCLKO_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_OSPI1_LBCLKO_OUT,
	},
	[CLK_AM275X_BOARD_0_RGMII1_RXC_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RGMII1_RXC_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_RGMII1_RXC_OUT,
	},
	[CLK_AM275X_BOARD_0_RGMII2_RXC_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RGMII2_RXC_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_RGMII2_RXC_OUT,
	},
	[CLK_AM275X_BOARD_0_RMII1_REF_CLK_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RMII1_REF_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_RMII1_REF_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_RMII2_REF_CLK_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_RMII2_REF_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_RMII2_REF_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_SPI0_CLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI0_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_SPI0_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_SPI1_CLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI1_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_SPI1_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_SPI2_CLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI2_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_SPI2_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_SPI3_CLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI3_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_SPI3_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_SPI4_CLK_OUT] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_SPI4_CLK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_SPI4_CLK_OUT,
	},
	[CLK_AM275X_BOARD_0_TCK_OUT] =							      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_TCK_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_TCK_OUT,
	},
	[CLK_AM275X_BOARD_0_WKUP_I2C0_SCL_OUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_WKUP_I2C0_SCL_out.data,
		.freq_idx	= AM275X_FREQ_VALUE_BOARD_0_WKUP_I2C0_SCL_OUT,
	},
	[CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_cpsw_3guss_am62l_main_0_cpts_genf0.data,
		.freq_idx	= AM275X_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
	},
	[CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_cpsw_3guss_am62l_main_0_cpts_genf1.data,
		.freq_idx	= AM275X_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
	},
	[CLK_AM275X_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_cpsw_3guss_am62l_main_0_mdio_mdclk_o.data,
		.freq_idx	= AM275X_FREQ_VALUE_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O,
	},
	[CLK_AM275X_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK] =			      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_debugss_k3_wrap_cv0_main_0_cstpiu_traceclk.data,
		.freq_idx	= AM275X_FREQ_VALUE_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_0_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_1_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_10_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_10_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_10_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_12_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_12_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_12_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_14_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_14_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_14_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_2_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_3_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_4_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_4_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_4_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_5_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_5_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_5_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_6_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_6_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_6_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_7_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_7_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_7_TIMER_PWM,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_MAIN_8_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_main_8_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_MAIN_8_TIMER_PWM,
	},
	[CLK_AM275X_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_emmcsd8ss_main_0_emmcsdss_io_clk_o.data,
		.freq_idx	= AM275X_FREQ_VALUE_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O,
	},
	[CLK_AM275X_FSS_MAIN_0_HYPERBUS1P0_0_HPB_OUT_CLK_N] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fss_main_0_hyperbus1p0_0_hpb_out_clk_n.data,
		.freq_idx	= AM275X_FREQ_VALUE_FSS_MAIN_0_HYPERBUS1P0_0_HPB_OUT_CLK_N,
	},
	[CLK_AM275X_FSS_MAIN_0_HYPERBUS1P0_0_HPB_OUT_CLK_P] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fss_main_0_hyperbus1p0_0_hpb_out_clk_p.data,
		.freq_idx	= AM275X_FREQ_VALUE_FSS_MAIN_0_HYPERBUS1P0_0_HPB_OUT_CLK_P,
	},
	[CLK_AM275X_FSS_MAIN_0_OSPI_0_OSPI_OCLK_CLK] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fss_main_0_ospi_0_ospi_oclk_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_FSS_MAIN_0_OSPI_0_OSPI_OCLK_CLK,
	},
	[CLK_AM275X_FSS_OF_UL_MAIN_0_OSPI0_OCLK_CLK] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fss_of_ul_main_0_ospi0_oclk_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_FSS_OF_UL_MAIN_0_OSPI0_OCLK_CLK,
	},
	[CLK_AM275X_MCASP_MAIN_0_MCASP_ACLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_0_mcasp_aclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_0_MCASP_ACLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_0_mcasp_aclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_ACLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_0_MCASP_AFSR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_0_mcasp_afsr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_0_MCASP_AFSX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_0_mcasp_afsx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AFSX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_0_MCASP_AHCLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_0_mcasp_ahclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_0_MCASP_AHCLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_0_mcasp_ahclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_0_MCASP_AHCLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_1_MCASP_ACLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_1_mcasp_aclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_1_MCASP_ACLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_1_mcasp_aclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_ACLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_1_MCASP_AFSR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_1_mcasp_afsr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_1_MCASP_AFSX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_1_mcasp_afsx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AFSX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_1_MCASP_AHCLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_1_mcasp_ahclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_1_MCASP_AHCLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_1_mcasp_ahclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_1_MCASP_AHCLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_2_MCASP_ACLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_2_mcasp_aclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_2_MCASP_ACLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_2_mcasp_aclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_ACLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_2_MCASP_AFSR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_2_mcasp_afsr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_2_MCASP_AFSX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_2_mcasp_afsx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AFSX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_2_MCASP_AHCLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_2_mcasp_ahclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_2_MCASP_AHCLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_2_mcasp_ahclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_2_MCASP_AHCLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_3_MCASP_ACLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_3_mcasp_aclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_ACLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_3_MCASP_ACLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_3_mcasp_aclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_ACLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_3_MCASP_AFSR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_3_mcasp_afsr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AFSR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_3_MCASP_AFSX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_3_mcasp_afsx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AFSX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_3_MCASP_AHCLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_3_mcasp_ahclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AHCLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_3_MCASP_AHCLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_3_mcasp_ahclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_3_MCASP_AHCLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_4_MCASP_ACLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_4_mcasp_aclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_ACLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_4_MCASP_ACLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_4_mcasp_aclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_ACLKX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_4_MCASP_AFSR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_4_mcasp_afsr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AFSR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_4_MCASP_AFSX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_4_mcasp_afsx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AFSX_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_4_MCASP_AHCLKR_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_4_mcasp_ahclkr_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AHCLKR_POUT,
	},
	[CLK_AM275X_MCASP_MAIN_4_MCASP_AHCLKX_POUT] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mcasp_main_4_mcasp_ahclkx_pout.data,
		.freq_idx	= AM275X_FREQ_VALUE_MCASP_MAIN_4_MCASP_AHCLKX_POUT,
	},
	[CLK_AM275X_MSHSI2C_MAIN_0_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_0_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_MAIN_0_PORSCL,
	},
	[CLK_AM275X_MSHSI2C_MAIN_1_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_1_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_MAIN_1_PORSCL,
	},
	[CLK_AM275X_MSHSI2C_MAIN_2_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_2_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_MAIN_2_PORSCL,
	},
	[CLK_AM275X_MSHSI2C_MAIN_3_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_3_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_MAIN_3_PORSCL,
	},
	[CLK_AM275X_MSHSI2C_MAIN_4_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_4_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_MAIN_4_PORSCL,
	},
	[CLK_AM275X_MSHSI2C_MAIN_5_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_5_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_MAIN_5_PORSCL,
	},
	[CLK_AM275X_MSHSI2C_MAIN_6_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_main_6_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_MAIN_6_PORSCL,
	},
	[CLK_AM275X_MSHSI2C_WKUP_0_PORSCL] =						      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_mshsi2c_wkup_0_porscl.data,
		.freq_idx	= AM275X_FREQ_VALUE_MSHSI2C_WKUP_0_PORSCL,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK] =			      {
		.parent		=							      {
			CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_0,
		.data		= &clk_data_pllfracf2_ssmod_16fft_main_0.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK] =			      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv.drv,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_pllfracf2_ssmod_16fft_main_0_postdiv.data,
		.flags	= 0,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK] =			      {
		.parent		=							      {
			CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_1,
		.data		= &clk_data_pllfracf2_ssmod_16fft_main_1.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTPOSTDIV_CLK] =			      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv.drv,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_pllfracf2_ssmod_16fft_main_1_postdiv.data,
		.flags	= 0,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_14_FOUTVCOP_CLK] =			      {
		.parent		=							      {
			CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_14,
		.data		= &clk_data_pllfracf2_ssmod_16fft_main_14.data_pll.data,
		.flags		= CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_15_FOUTVCOP_CLK] =			      {
		.parent		=							      {
			CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_15,
		.data		= &clk_data_pllfracf2_ssmod_16fft_main_15.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK] =			      {
		.parent		=							      {
			CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_2,
		.data		= &clk_data_pllfracf2_ssmod_16fft_main_2.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK] =			      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv.drv,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_pllfracf2_ssmod_16fft_main_2_postdiv.data,
		.flags	= 0,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_7_FOUTVCOP_CLK] =			      {
		.parent		=							      {
			CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_7,
		.data		= &clk_data_pllfracf2_ssmod_16fft_main_7.data_pll.data,
		.flags		= CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK] =				      {
		.parent		=							      {
			CLK_AM275X_GLUELOGIC_HFOSC0_CLK,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MCU_0,
		.data		= &clk_data_pllfracf2_ssmod_16fft_mcu_0.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM275X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT5_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv1_16fft_main_1_hsdiv5.data_div.data,
	},
	[CLK_AM275X_POSTDIV1_16FFT_MAIN_1_HSDIVOUT6_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv1_16fft_main_1_hsdiv6.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv5.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv6.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv7.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv8.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_0_HSDIVOUT9_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_0_hsdiv9.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT5_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv5.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT6_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv6.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT7_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv7.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT8_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv8.data_div.data,
	},
	[CLK_AM275X_POSTDIV4_16FF_MAIN_2_HSDIVOUT9_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv4_16ff_main_2_hsdiv9.data_div.data,
	},
	[CLK_AM275X_RL2_OF_CBA4_MAIN_R5_0_CORE0_TAGMEM_CLK] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_rl2_of_cba4_main_r5_0_core0_tagmem_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_0_CORE0_TAGMEM_CLK,
	},
	[CLK_AM275X_RL2_OF_CBA4_MAIN_R5_0_CORE1_TAGMEM_CLK] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_rl2_of_cba4_main_r5_0_core1_tagmem_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_0_CORE1_TAGMEM_CLK,
	},
	[CLK_AM275X_RL2_OF_CBA4_MAIN_R5_1_CORE0_TAGMEM_CLK] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_rl2_of_cba4_main_r5_1_core0_tagmem_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_1_CORE0_TAGMEM_CLK,
	},
	[CLK_AM275X_RL2_OF_CBA4_MAIN_R5_1_CORE1_TAGMEM_CLK] =				      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_rl2_of_cba4_main_r5_1_core1_tagmem_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_RL2_OF_CBA4_MAIN_R5_1_CORE1_TAGMEM_CLK,
	},
	[CLK_AM275X_SPI_MAIN_0_IO_CLKSPIO_CLK] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_0_io_clkspio_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_SPI_MAIN_0_IO_CLKSPIO_CLK,
	},
	[CLK_AM275X_SPI_MAIN_1_IO_CLKSPIO_CLK] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_1_io_clkspio_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_SPI_MAIN_1_IO_CLKSPIO_CLK,
	},
	[CLK_AM275X_SPI_MAIN_2_IO_CLKSPIO_CLK] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_2_io_clkspio_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_SPI_MAIN_2_IO_CLKSPIO_CLK,
	},
	[CLK_AM275X_SPI_MAIN_3_IO_CLKSPIO_CLK] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_3_io_clkspio_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_SPI_MAIN_3_IO_CLKSPIO_CLK,
	},
	[CLK_AM275X_SPI_MAIN_4_IO_CLKSPIO_CLK] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_4_io_clkspio_clk.data,
		.freq_idx	= AM275X_FREQ_VALUE_SPI_MAIN_4_IO_CLKSPIO_CLK,
	},
	[CLK_AM275X_ATL_AWS_SEL_OUT0] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_AWS_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ATL_AWS_SEL_OUT1] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_AWS_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ATL_AWS_SEL_OUT2] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_AWS_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ATL_AWS_SEL_OUT3] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_AWS_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ATL_BWS_SEL_OUT0] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_BWS_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ATL_BWS_SEL_OUT1] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_BWS_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ATL_BWS_SEL_OUT2] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_BWS_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ATL_BWS_SEL_OUT3] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_BWS_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_CLK_32K_RC_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_CLK_32K_RC_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_EMMCSD0_IO_CLKLB_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_EMMCSD0_IO_CLKLB_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_OSPI0_LB_CLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OSPI0_LB_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_OSPI1_LB_CLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OSPI1_LB_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_PLL4_HFOSC_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_PLL4_HFOSC_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USB0_REFCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USB0_REFCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_WWDTCLKN_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_WWDTCLKN_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_WWDTCLKN_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_WWDTCLKN_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_WWDTCLKN_SEL_OUT4] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_WWDTCLKN_SEL_OUT5] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_WWDTCLKn_SEL_out5.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKR_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKR_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKR_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKR_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKR_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKR_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKR_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKR_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKR_SEL_OUT4] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKR_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKX_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKX_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKX_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKX_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKX_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKX_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKX_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AHCLKX_SEL_OUT4] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AHCLKX_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_SEL_OUT4] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SPI0_CLK_LPBK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_SPI0_CLK_LPBK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SPI1_CLK_LPBK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_SPI1_CLK_LPBK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SPI2_CLK_LPBK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_SPI2_CLK_LPBK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SPI3_CLK_LPBK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_SPI3_CLK_LPBK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SPI4_CLK_LPBK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_SPI4_CLK_LPBK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_HSDIV1_16FFT_MAIN_14_HSDIVOUT0_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_14_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv1_16fft_main_14_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV1_16FFT_MAIN_14_HSDIVOUT1_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_14_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv1_16fft_main_14_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV1_16FFT_MAIN_7_HSDIVOUT0_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_7_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv1_16fft_main_7_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV1_16FFT_MAIN_7_HSDIVOUT1_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_7_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv1_16fft_main_7_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_15_HSDIVOUT0_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_15_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_15_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_15_HSDIVOUT1_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_15_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_15_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_15_HSDIVOUT2_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_15_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_15_hsdiv2.data_div.data,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_15_HSDIVOUT3_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_15_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_15_hsdiv3.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT1_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv2.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT3_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv3.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_0_hsdiv4.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT1_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT2_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv2.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT3_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv3.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT4_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_1_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_1_hsdiv4.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT0_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT1_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT2_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv2.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT3_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv3.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MAIN_2_HSDIVOUT4_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_2_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_main_2_hsdiv4.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT0_CLK] =					      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT4_CLK] =					      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv4.data_div.data,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_4_FOUTVCOP_CLK] =			      {
		.parent		=							      {
			CLK_AM275X_MAIN_PLL4_HFOSC_SEL_OUT0,
			1,
		},
		.drv		= &clk_drv_pll_16fft,
		.freq_idx	= AM275X_FREQ_VALUE_PLLFRACF2_SSMOD_16FFT_MAIN_4,
		.data		= &clk_data_pllfracf2_ssmod_16fft_main_4.data_pll.data,
		.flags		= 0,
	},
	[CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK] =				      {
		.drv	= &clk_drv_pllctrl_mux_reg_ro.drv,
		.flags	= 0,
		.data	= &clk_data_sam62_pll_ctrl_wrap_main_0_sysclkout_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV1_CLK_CLK] =			      {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_sam62_pll_ctrl_wrap_main_0_chip_div1_clk_clk.data_div.data,
		.parent =								      {
			CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_SYSCLKOUT_CLK] =				      {
		.drv	= &clk_drv_pllctrl_mux_reg_ro.drv,
		.flags	= 0,
		.data	= &clk_data_sam62_pll_ctrl_wrap_mcu_0_sysclkout_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_CHIP_DIV1_CLK_CLK] =			      {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_sam62_pll_ctrl_wrap_mcu_0_chip_div1_clk_clk.data_div.data,
		.parent =								      {
			CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM275X_ADC_CLK_SEL_OUT0] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ADC_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_CLKOUT0_CTRL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_CLKOUT0_CTRL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_EMMCSD0_REFCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_EMMCSD0_REFCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_MCANN_CLK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_MCANn_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_MCANN_CLK_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_MCANn_CLK_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_MCANN_CLK_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_MCANn_CLK_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_MCANN_CLK_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_MCANn_CLK_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_MCANN_CLK_SEL_OUT4] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_MCANn_CLK_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_OSPI_REF_CLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OSPI_REF_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_OSPI_REF_CLK_SEL_OUT1] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OSPI_REF_CLK_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCU_OBSCLK_MUX_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_OBSCLK_MUX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_R5SS_CLK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_R5SS_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_R5SS_CLK_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_R5SS_CLK_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_USART_CLK_DIV_OUT0] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_CLK_DIV_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_USART_CLK_DIV_OUT1] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_CLK_DIV_out1.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_USART_CLK_DIV_OUT2] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_CLK_DIV_out2.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_USART_CLK_DIV_OUT3] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_CLK_DIV_out3.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_USART_CLK_DIV_OUT4] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_CLK_DIV_out4.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_USART_CLK_DIV_OUT5] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_CLK_DIV_out5.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_USART_CLK_DIV_OUT6] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV4_16FFT_MAIN_1_HSDIVOUT0_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_USART_CLK_DIV_out6.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_WKUP_CLKOUT_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_CLKOUT_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_WKUP_CLKOUT_SEL_IO_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_CLKOUT_SEL_IO_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_WKUP_CLKSEL_OUT0] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_CLKSEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT0_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_4_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_4_hsdiv0.data_div.data,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT1_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_4_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_4_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT2_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_4_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_4_hsdiv2.data_div.data,
	},
	[CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT3_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MAIN_4_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv3_16fft_main_4_hsdiv3.data_div.data,
	},
	[CLK_AM275X_ASRC_SYNC_DIV_OUT0] =						      {
		.parent =								      {
			CLK_AM275X_HSDIV3_16FFT_MAIN_4_HSDIVOUT3_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC_SYNC_DIV_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_ATL_CLK_SEL_OUT0] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ATL_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_AUDIO_REFCLKN_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_AUDIO_REFCLKn_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_AUDIO_REFCLKN_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_AUDIO_REFCLKn_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_AUDIO_REFCLKN_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_AUDIO_REFCLKn_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_CPSW_CPTS_CLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_CPSW_CPTS_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_OBSCLK0_MUX_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OBSCLK0_MUX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_OBSCLK_DIV_OUT0] =						      {
		.parent =								      {
			CLK_AM275X_MAIN_OBSCLK0_MUX_SEL_OUT0,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OBSCLK_DIV_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_MAIN_OBSCLK_OUTMUX_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OBSCLK_OUTMUX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_OBSCLK_OUTMUX_SEL_OUT1] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_OBSCLK_OUTMUX_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT10] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out10.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT11] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out11.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT12] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out12.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT13] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out13.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT14] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out14.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT15] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out15.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT4] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT5] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out5.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT6] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out6.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT7] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out7.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT8] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out8.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMERCLKN_SEL_OUT9] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMERCLKn_SEL_out9.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USART0_FCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USART0_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USART1_FCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USART1_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USART2_FCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USART2_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USART3_FCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USART3_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USART4_FCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USART4_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USART5_FCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USART5_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_USART6_FCLK_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_USART6_FCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCU_OBSCLK_DIV_OUT0] =						      {
		.parent =								      {
			CLK_AM275X_MCU_OBSCLK_MUX_SEL_OUT0,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_OBSCLK_DIV_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM275X_MCU_OBSCLK_OUTMUX_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_OBSCLK_OUTMUX_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_LOCAL_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT1] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_LOCAL_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT2] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_LOCAL_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT3] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_LOCAL_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASPN_AUXCLK_LOCAL_SEL_OUT4] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASPn_AUXCLK_LOCAL_SEL_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_RXSYNC_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_RXSYNC_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_RXSYNC_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_RXSYNC_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_RXSYNC_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_RXSYNC_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_RXSYNC_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_RXSYNC_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_TXSYNC_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_TXSYNC_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_TXSYNC_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_TXSYNC_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_TXSYNC_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_TXSYNC_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC0_TXSYNC_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC0_TXSYNC_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_RXSYNC_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_RXSYNC_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_RXSYNC_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_RXSYNC_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_RXSYNC_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_RXSYNC_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_RXSYNC_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_RXSYNC_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_TXSYNC_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_TXSYNC_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_TXSYNC_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_TXSYNC_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_TXSYNC_SEL_OUT2] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_TXSYNC_SEL_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_ASRC1_TXSYNC_SEL_OUT3] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ASRC1_TXSYNC_SEL_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER11_CASCADE_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER11_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER13_CASCADE_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER13_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER15_CASCADE_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER15_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER1_CASCADE_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER1_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER3_CASCADE_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER3_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER5_CASCADE_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER5_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER7_CASCADE_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER7_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_TIMER9_CASCADE_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_TIMER9_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASP0_AUXCLK_GF_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASP0_AUXCLK_GF_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASP1_AUXCLK_GF_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASP1_AUXCLK_GF_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASP2_AUXCLK_GF_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASP2_AUXCLK_GF_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASP3_AUXCLK_GF_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASP3_AUXCLK_GF_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCASP4_AUXCLK_GF_SEL_OUT0] =					      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_McASP4_AUXCLK_GF_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MAIN_GTCCLK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MAIN_GTCCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_MCU_GPIO0_CLKSEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_GPIO0_CLKSEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_RTC_CLK_SEL_OUT0] =							      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_RTC_CLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_WKUP_CLKSEL_R5FSS_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_CLKSEL_R5FSS_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_WKUP_WWDTCLK_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_WWDTCLK_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM] =					      {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_dmtimer_dmc1ms_wkup_0_timer_pwm.data,
		.freq_idx	= AM275X_FREQ_VALUE_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM,
	},
	[CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT1_CLK] =					      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv1.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT2_CLK] =					      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv2.data_div.data,
	},
	[CLK_AM275X_HSDIV4_16FFT_MCU_0_HSDIVOUT3_CLK] =					      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_hsdiv4_16fft_mcu_0_hsdiv3.data_div.data,
	},
	[CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTPOSTDIV_CLK] =			      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTVCOP_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv.drv,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_pllfracf2_ssmod_16fft_mcu_0_postdiv.data,
		.flags	= 0,
	},
	[CLK_AM275X_POSTDIV1_16FFT_MCU_0_HSDIVOUT5_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv1_16fft_mcu_0_hsdiv5.data_div.data,
	},
	[CLK_AM275X_POSTDIV1_16FFT_MCU_0_HSDIVOUT6_CLK] =				      {
		.parent =								      {
			CLK_AM275X_PLLFRACF2_SSMOD_16FFT_MCU_0_FOUTPOSTDIV_CLK,
			1,
		},
		.drv	= &clk_drv_div_pll_16fft_postdiv_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_postdiv1_16fft_mcu_0_hsdiv6.data_div.data,
	},
	[CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_CHIP_DIV24_CLK_CLK] =			      {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_sam62_pll_ctrl_wrap_mcu_0_chip_div24_clk_clk.data_div.data,
		.parent =								      {
			CLK_AM275X_SAM62_PLL_CTRL_WRAP_MCU_0_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM275X_WKUP_TIMERCLKN_SEL_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_TIMERCLKn_SEL_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_WKUP_TIMERCLKN_SEL_OUT1] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_TIMERCLKn_SEL_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_WKUP_TIMER1_CASCADE_OUT0] =						      {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_WKUP_TIMER1_CASCADE_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_CHIP_DIV24_CLK_CLK] =			      {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_sam62_pll_ctrl_wrap_main_0_chip_div24_clk_clk.data_div.data,
		.parent =								      {
			CLK_AM275X_SAM62_PLL_CTRL_WRAP_MAIN_0_SYSCLKOUT_CLK,
			1,
		},
	},
};

struct clk soc_clocks[346];
const size_t soc_clock_count = ARRAY_SIZE(soc_clock_data);
