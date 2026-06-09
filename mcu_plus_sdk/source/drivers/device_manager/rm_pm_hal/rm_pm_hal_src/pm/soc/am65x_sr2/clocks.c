/*
 * Data version: 000000_000000
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
#include <soc/am65x_sr2/clk_ids.h>
#include <soc/am65x_sr2/devices.h>
#include <soc/am65x_sr2/clocks.h>
#include <clk_fixed.h>
#include <clk_adpllm.h>
#include <clk_gate.h>
#include <clk_div.h>
#include <clk_pllctrl.h>
#include <soc/am65x_sr2/clk_soc_hfosc0.h>


enum {
	AM6_FREQ_VALUE_ADPLLLJM_HSDIV_WRAP_MAIN_0,
	AM6_FREQ_VALUE_ADPLLLJM_HSDIV_WRAP_MAIN_2,
	AM6_FREQ_VALUE_ADPLLLJM_WRAP_MAIN_1,
	AM6_FREQ_VALUE_ADPLLLJM_WRAP_MAIN_3,
	AM6_FREQ_VALUE_ADPLLLJM_WRAP_MAIN_4,
	AM6_FREQ_VALUE_ADPLLM_HSDIV_WRAP_MCU_0,
	AM6_FREQ_VALUE_ADPLLM_HSDIV_WRAP_MCU_1,
	AM6_FREQ_VALUE_ADPLLM_WRAP_MAIN_6,
	AM6_FREQ_VALUE_ADPLLM_WRAP_MAIN_7,
	AM6_FREQ_VALUE_BOARD_0_BUS_CCDC0_PCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_CPTS_RFT_CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_DSS0EXTPCLKIN_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_EXT_REFCLK1_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_GPMCCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0ACLKR_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0ACLKX_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0AHCLKR_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0AHCLKX_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1ACLKR_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1ACLKX_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1AHCLKR_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1AHCLKX_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2ACLKR_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2ACLKX_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2AHCLKR_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2AHCLKX_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_CPTS_RFT_CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_OSPI0DQS_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_OSPI1DQS_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_RGMII1_RCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_RGMII1_TCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_RMII1_REFCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_SPI0CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_MCU_SPI1CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII1_RCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII1_TCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII2_RCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII2_TCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII1_RCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII1_TCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII2_RCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII2_TCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII1_RCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII1_TCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII2_RCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII2_TCLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_SPI0CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_SPI1CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_SPI2CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_SPI3CLK_OUT,
	AM6_FREQ_VALUE_BOARD_0_BUS_WKUP_TCK_OUT,
	AM6_FREQ_VALUE_BOARD_0_HFOSC1_CLK_OUT,
	AM6_FREQ_VALUE_FSS_MCU_0_OSPI_0_BUS_OSPI_OCLK_CLK,
	AM6_FREQ_VALUE_FSS_MCU_0_OSPI_1_BUS_OSPI_OCLK_CLK,
	AM6_FREQ_VALUE_GPMC_MAIN_0_BUS_PO_GPMC_DEV_CLK,
	AM6_FREQ_VALUE_ICSS_G_MAIN_2_BUS_WIZ0_TX_MST_CLK,
	AM6_FREQ_VALUE_ICSS_G_MAIN_2_BUS_WIZ1_TX_MST_CLK,
	AM6_FREQ_VALUE_K3_DSS_UL_MAIN_0_BUS_DPI_1_OUT_CLK,
	AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF2_0,
	AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF3_0,
	AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF4_0,
	AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF5_0,
	AM6_FREQ_VALUE_PCIE_G3X2_MAIN_0_BUS_PCIE_TXR0_CLK,
	AM6_FREQ_VALUE_PCIE_G3X2_MAIN_0_BUS_PCIE_TXR1_CLK,
	AM6_FREQ_VALUE_PCIE_G3X2_MAIN_1_BUS_PCIE_TXR0_CLK,
	AM6_FREQ_VALUE_SPI_MAIN_0_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_SPI_MAIN_1_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_SPI_MAIN_2_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_SPI_MAIN_3_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_SPI_MAIN_4_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_SPI_MCU_0_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_SPI_MCU_1_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_SPI_MCU_2_BUS_IO_CLKSPIO_CLK,
	AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_0_BUS_LN0_RXCLK,
	AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_0_BUS_LN0_TXCLK,
	AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_1_BUS_LN0_RXCLK,
	AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_1_BUS_LN0_TXCLK,
	AM6_FREQ_VALUE_COUNT,
};

enum {
	AM6_FREQ_RANGE_ANY,
	AM6_FREQ_RANGE_GLUELOGIC_LFOSC_CLK_BUS_OUT,
	AM6_FREQ_RANGE_VCO_ADPLLLJM_HSDIV_WRAP_MAIN_0,
	AM6_FREQ_RANGE_VCO_IN_ADPLLLJM_HSDIV_WRAP_MAIN_0,
	AM6_FREQ_RANGE_VCO_ADPLLM_HSDIV_WRAP_MCU_0,
	AM6_FREQ_RANGE_VCO_IN_ADPLLM_HSDIV_WRAP_MCU_0,
	AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_PRG_MCU_EFUSE_CLK,
	AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
	AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
	AM6_FREQ_RANGE_ID_MAX,
};

enum {
	PLL_ENTRY_MCU0_19_2MHZ_400MHZ,
	PLL_ENTRY_MCU0_20MHZ_400MHZ,
	PLL_ENTRY_MCU0_24MHZ_400MHZ,
	PLL_ENTRY_MCU0_25MHZ_400MHZ,
	PLL_ENTRY_MCU0_26MHZ_400MHZ,
	PLL_ENTRY_MCU0_27MHZ_400MHZ,
	PLL_ENTRY_CPSW_19_2MHZ_250MHZ,
	PLL_ENTRY_CPSW_20MHZ_250MHZ,
	PLL_ENTRY_CPSW_24MHZ_250MHZ,
	PLL_ENTRY_CPSW_25MHZ_250MHZ,
	PLL_ENTRY_CPSW_26MHZ_250MHZ,
	PLL_ENTRY_CPSW_27MHZ_250MHZ,
	PLL_ENTRY_MAIN_19_2MHZ_100MHZ,
	PLL_ENTRY_MAIN_20MHZ_100MHZ,
	PLL_ENTRY_MAIN_24MHZ_100MHZ,
	PLL_ENTRY_MAIN_25MHZ_100MHZ,
	PLL_ENTRY_MAIN_26MHZ_100MHZ,
	PLL_ENTRY_MAIN_27MHZ_100MHZ,
	PLL_ENTRY_MAIN_19_2MHZ_125MHZ,
	PLL_ENTRY_MAIN_20MHZ_125MHZ,
	PLL_ENTRY_MAIN_24MHZ_125MHZ,
	PLL_ENTRY_MAIN_25MHZ_125MHZ,
	PLL_ENTRY_MAIN_26MHZ_125MHZ,
	PLL_ENTRY_MAIN_27MHZ_125MHZ,
	PLL_ENTRY_MAIN_19_2MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_20MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_24MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_25MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_26MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_27MHZ_156_25MHZ,
	PLL_ENTRY_PER0_19_2MHZ_960MHZ,
	PLL_ENTRY_PER0_20MHZ_960MHZ,
	PLL_ENTRY_PER0_24MHZ_960MHZ,
	PLL_ENTRY_PER0_25MHZ_960MHZ,
	PLL_ENTRY_PER0_26MHZ_960MHZ,
	PLL_ENTRY_PER0_27MHZ_960MHZ,
	PLL_ENTRY_PER1_19_2MHZ_300MHZ,
	PLL_ENTRY_PER1_20MHZ_300MHZ,
	PLL_ENTRY_PER1_24MHZ_300MHZ,
	PLL_ENTRY_PER1_25MHZ_300MHZ,
	PLL_ENTRY_PER1_26MHZ_300MHZ,
	PLL_ENTRY_PER1_27MHZ_300MHZ,
	PLL_ENTRY_DDR_19_2MHZ_333MHZ,
	PLL_ENTRY_DDR_20MHZ_333MHZ,
	PLL_ENTRY_DDR_24MHZ_333MHZ,
	PLL_ENTRY_DDR_25MHZ_333MHZ,
	PLL_ENTRY_DDR_26MHZ_333MHZ,
	PLL_ENTRY_DDR_27MHZ_333MHZ,
	PLL_ENTRY_DDR_19_2MHZ_400MHZ,
	PLL_ENTRY_DDR_20MHZ_400MHZ,
	PLL_ENTRY_DDR_24MHZ_400MHZ,
	PLL_ENTRY_DDR_25MHZ_400MHZ,
	PLL_ENTRY_DDR_26MHZ_400MHZ,
	PLL_ENTRY_DDR_27MHZ_400MHZ,
	PLL_ENTRY_DSS_19_2MHZ_1155MHZ,
	PLL_ENTRY_DSS_20MHZ_1155MHZ,
	PLL_ENTRY_DSS_24MHZ_1155MHZ,
	PLL_ENTRY_DSS_25MHZ_1155MHZ,
	PLL_ENTRY_DSS_26MHZ_1155MHZ,
	PLL_ENTRY_DSS_27MHZ_1155MHZ,
	PLL_ENTRY_ARM_19_2MHZ_800MHZ,
	PLL_ENTRY_ARM_20MHZ_800MHZ,
	PLL_ENTRY_ARM_24MHZ_800MHZ,
	PLL_ENTRY_ARM_25MHZ_800MHZ,
	PLL_ENTRY_ARM_26MHZ_800MHZ,
	PLL_ENTRY_ARM_27MHZ_800MHZ,
	PLL_ENTRY_ARM_19_2MHZ_1000MHZ,
	PLL_ENTRY_ARM_20MHZ_1000MHZ,
	PLL_ENTRY_ARM_24MHZ_1000MHZ,
	PLL_ENTRY_ARM_25MHZ_1000MHZ,
	PLL_ENTRY_ARM_26MHZ_1000MHZ,
	PLL_ENTRY_ARM_27MHZ_1000MHZ,
	PLL_ENTRY_ARM_19_2MHZ_1100MHZ,
	PLL_ENTRY_ARM_20MHZ_1100MHZ,
	PLL_ENTRY_ARM_24MHZ_1100MHZ,
	PLL_ENTRY_ARM_25MHZ_1100MHZ,
	PLL_ENTRY_ARM_26MHZ_1100MHZ,
	PLL_ENTRY_ARM_27MHZ_1100MHZ,
	PLL_ENTRY_ARM_19_2MHZ_1200MHZ,
	PLL_ENTRY_ARM_20MHZ_1200MHZ,
	PLL_ENTRY_ARM_24MHZ_1200MHZ,
	PLL_ENTRY_ARM_25MHZ_1200MHZ,
	PLL_ENTRY_ARM_26MHZ_1200MHZ,
	PLL_ENTRY_ARM_27MHZ_1200MHZ,
};

enum {
	NO_DEFAULT_FREQ,
	FREQ_ADPLLLJM_HSDIV_WRAP_MAIN_0_DEFAULT,
	FREQ_ADPLLLJM_HSDIV_WRAP_MAIN_2_DEFAULT,
	FREQ_ADPLLLJM_WRAP_MAIN_1_DEFAULT,
	FREQ_ADPLLLJM_WRAP_MAIN_3_DEFAULT,
	FREQ_ADPLLLJM_WRAP_MAIN_4_DEFAULT,
	FREQ_ADPLLM_HSDIV_WRAP_MCU_1_DEFAULT,
	FREQ_ADPLLM_WRAP_MAIN_6_DEFAULT,
};

u32 soc_clock_values[AM6_FREQ_VALUE_COUNT];

const struct clk_range soc_clock_ranges[AM6_FREQ_RANGE_ID_MAX] = {
	CLK_RANGE(AM6_FREQ_RANGE_ANY,						 0,	    ULONG_MAX),
	CLK_RANGE(AM6_FREQ_RANGE_GLUELOGIC_LFOSC_CLK_BUS_OUT,			 32768,	    32768),
	CLK_RANGE(AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_PRG_MCU_EFUSE_CLK,	 1,	    1),
	CLK_RANGE(AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK, 12500000,  12500000),
	CLK_RANGE(AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,	 32000,	    32000),
	CLK_RANGE(AM6_FREQ_RANGE_VCO_ADPLLLJM_HSDIV_WRAP_MAIN_0,		 750000000, 2500000000),
	CLK_RANGE(AM6_FREQ_RANGE_VCO_ADPLLM_HSDIV_WRAP_MCU_0,			 40000000,  2800000000),
	CLK_RANGE(AM6_FREQ_RANGE_VCO_IN_ADPLLLJM_HSDIV_WRAP_MAIN_0,		 32000,	    52000000),
	CLK_RANGE(AM6_FREQ_RANGE_VCO_IN_ADPLLM_HSDIV_WRAP_MCU_0,		 620000,    2500000),
};

const struct clk_default soc_clock_freq_defaults[8] = {
	CLK_DEFAULT(FREQ_ADPLLLJM_HSDIV_WRAP_MAIN_0_DEFAULT, 100000000UL,  100000000UL,	 100000000UL),
	CLK_DEFAULT(FREQ_ADPLLLJM_HSDIV_WRAP_MAIN_2_DEFAULT, 300000000UL,  300000000UL,	 300000000UL),
	CLK_DEFAULT(FREQ_ADPLLLJM_WRAP_MAIN_1_DEFAULT,	     960000000UL,  960000000UL,	 960000000UL),
	CLK_DEFAULT(FREQ_ADPLLLJM_WRAP_MAIN_3_DEFAULT,	     333333333UL,  333333333UL,	 333333333UL),
	CLK_DEFAULT(FREQ_ADPLLLJM_WRAP_MAIN_4_DEFAULT,	     1155000000UL, 1155000000UL, 1155000000UL),
	CLK_DEFAULT(FREQ_ADPLLM_HSDIV_WRAP_MCU_1_DEFAULT,    250000000UL,  250000000UL,	 250000000UL),
	CLK_DEFAULT(FREQ_ADPLLM_WRAP_MAIN_6_DEFAULT,	     800000000UL,  800000000UL,	 800000000UL),
};

const struct pll_table_entry soc_pll_table[84] = {
	[PLL_ENTRY_MCU0_19_2MHZ_400MHZ] =    {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 2U,
		.pllm		= 250U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_MCU0_20MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 1U,
		.pllm		= 120U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_MCU0_24MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_MCU0_25MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 1U,
		.pllm		= 96U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_MCU0_26MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 13U,
		.pllm		= 1200U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_MCU0_27MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 9U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_CPSW_19_2MHZ_250MHZ] =    {
		.freq_min_hz	= 250000000U,
		.freq_max_hz	= 250000000U,
		.plld		= 12U,
		.pllm		= 1250U,
		.pllfm		= 0U,
		.clkod		= 8U,
	},
	[PLL_ENTRY_CPSW_20MHZ_250MHZ] =	     {
		.freq_min_hz	= 250000000U,
		.freq_max_hz	= 250000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 8U,
	},
	[PLL_ENTRY_CPSW_24MHZ_250MHZ] =	     {
		.freq_min_hz	= 250000000U,
		.freq_max_hz	= 250000000U,
		.plld		= 3U,
		.pllm		= 250U,
		.pllfm		= 0U,
		.clkod		= 8U,
	},
	[PLL_ENTRY_CPSW_25MHZ_250MHZ] =	     {
		.freq_min_hz	= 250000000U,
		.freq_max_hz	= 250000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 8U,
	},
	[PLL_ENTRY_CPSW_26MHZ_250MHZ] =	     {
		.freq_min_hz	= 250000000U,
		.freq_max_hz	= 250000000U,
		.plld		= 13U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 8U,
	},
	[PLL_ENTRY_CPSW_27MHZ_250MHZ] =	     {
		.freq_min_hz	= 250000000U,
		.freq_max_hz	= 250000000U,
		.plld		= 27U,
		.pllm		= 2000U,
		.pllfm		= 0U,
		.clkod		= 8U,
	},
	[PLL_ENTRY_MAIN_19_2MHZ_100MHZ] =    {
		.freq_min_hz	= 100000000U,
		.freq_max_hz	= 100000000U,
		.plld		= 12U,
		.pllm		= 1250U,
		.pllfm		= 0U,
		.clkod		= 20U,
	},
	[PLL_ENTRY_MAIN_20MHZ_100MHZ] =	     {
		.freq_min_hz	= 100000000U,
		.freq_max_hz	= 100000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 20U,
	},
	[PLL_ENTRY_MAIN_24MHZ_100MHZ] =	     {
		.freq_min_hz	= 100000000U,
		.freq_max_hz	= 100000000U,
		.plld		= 12U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 20U,
	},
	[PLL_ENTRY_MAIN_25MHZ_100MHZ] =	     {
		.freq_min_hz	= 100000000U,
		.freq_max_hz	= 100000000U,
		.plld		= 10U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 20U,
	},
	[PLL_ENTRY_MAIN_26MHZ_100MHZ] =	     {
		.freq_min_hz	= 100000000U,
		.freq_max_hz	= 100000000U,
		.plld		= 13U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 20U,
	},
	[PLL_ENTRY_MAIN_27MHZ_100MHZ] =	     {
		.freq_min_hz	= 100000000U,
		.freq_max_hz	= 100000000U,
		.plld		= 27U,
		.pllm		= 2000U,
		.pllfm		= 0U,
		.clkod		= 20U,
	},
	[PLL_ENTRY_MAIN_19_2MHZ_125MHZ] =    {
		.freq_min_hz	= 125000000U,
		.freq_max_hz	= 125000000U,
		.plld		= 12U,
		.pllm		= 1250U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_20MHZ_125MHZ] =	     {
		.freq_min_hz	= 125000000U,
		.freq_max_hz	= 125000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_24MHZ_125MHZ] =	     {
		.freq_min_hz	= 125000000U,
		.freq_max_hz	= 125000000U,
		.plld		= 12U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_25MHZ_125MHZ] =	     {
		.freq_min_hz	= 125000000U,
		.freq_max_hz	= 125000000U,
		.plld		= 10U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_26MHZ_125MHZ] =	     {
		.freq_min_hz	= 125000000U,
		.freq_max_hz	= 125000000U,
		.plld		= 13U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_27MHZ_125MHZ] =	     {
		.freq_min_hz	= 125000000U,
		.freq_max_hz	= 125000000U,
		.plld		= 27U,
		.pllm		= 2000U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_19_2MHZ_156_25MHZ] = {
		.freq_min_hz	= 156250000U,
		.freq_max_hz	= 156250000U,
		.plld		= 24U,
		.pllm		= 3125U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_20MHZ_156_25MHZ] =   {
		.freq_min_hz	= 156250000U,
		.freq_max_hz	= 156250000U,
		.plld		= 8U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_24MHZ_156_25MHZ] =   {
		.freq_min_hz	= 156250000U,
		.freq_max_hz	= 156250000U,
		.plld		= 12U,
		.pllm		= 1250U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_25MHZ_156_25MHZ] =   {
		.freq_min_hz	= 156250000U,
		.freq_max_hz	= 156250000U,
		.plld		= 10U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_26MHZ_156_25MHZ] =   {
		.freq_min_hz	= 156250000U,
		.freq_max_hz	= 156250000U,
		.plld		= 13U,
		.pllm		= 1250U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_MAIN_27MHZ_156_25MHZ] =   {
		.freq_min_hz	= 156250000U,
		.freq_max_hz	= 156250000U,
		.plld		= 27U,
		.pllm		= 2500U,
		.pllfm		= 0U,
		.clkod		= 16U,
	},
	[PLL_ENTRY_PER0_19_2MHZ_960MHZ] =    {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 8U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_20MHZ_960MHZ] =	     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 8U,
		.pllm		= 768U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_24MHZ_960MHZ] =	     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 10U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_25MHZ_960MHZ] =	     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 10U,
		.pllm		= 768U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_26MHZ_960MHZ] =	     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 13U,
		.pllm		= 960U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER0_27MHZ_960MHZ] =	     {
		.freq_min_hz	= 960000000U,
		.freq_max_hz	= 960000000U,
		.plld		= 18U,
		.pllm		= 1280U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_PER1_19_2MHZ_300MHZ] =    {
		.freq_min_hz	= 300000000U,
		.freq_max_hz	= 300000000U,
		.plld		= 8U,
		.pllm		= 750U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_PER1_20MHZ_300MHZ] =	     {
		.freq_min_hz	= 300000000U,
		.freq_max_hz	= 300000000U,
		.plld		= 8U,
		.pllm		= 720U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_PER1_24MHZ_300MHZ] =	     {
		.freq_min_hz	= 300000000U,
		.freq_max_hz	= 300000000U,
		.plld		= 10U,
		.pllm		= 750U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_PER1_25MHZ_300MHZ] =	     {
		.freq_min_hz	= 300000000U,
		.freq_max_hz	= 300000000U,
		.plld		= 10U,
		.pllm		= 720U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_PER1_26MHZ_300MHZ] =	     {
		.freq_min_hz	= 300000000U,
		.freq_max_hz	= 300000000U,
		.plld		= 13U,
		.pllm		= 900U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_PER1_27MHZ_300MHZ] =	     {
		.freq_min_hz	= 300000000U,
		.freq_max_hz	= 300000000U,
		.plld		= 12U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 6U,
	},
	[PLL_ENTRY_DDR_19_2MHZ_333MHZ] =     {
		.freq_min_hz	= 333333333U,
		.freq_max_hz	= 333333333U,
		.plld		= 12U,
		.pllm		= 625U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_20MHZ_333MHZ] =	     {
		.freq_min_hz	= 333333333U,
		.freq_max_hz	= 333333333U,
		.plld		= 8U,
		.pllm		= 400U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_24MHZ_333MHZ] =	     {
		.freq_min_hz	= 333333333U,
		.freq_max_hz	= 333333333U,
		.plld		= 12U,
		.pllm		= 500U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_25MHZ_333MHZ] =	     {
		.freq_min_hz	= 333333333U,
		.freq_max_hz	= 333333333U,
		.plld		= 10U,
		.pllm		= 400U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_26MHZ_333MHZ] =	     {
		.freq_min_hz	= 333333333U,
		.freq_max_hz	= 333333333U,
		.plld		= 13U,
		.pllm		= 500U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_27MHZ_333MHZ] =	     {
		.freq_min_hz	= 333333333U,
		.freq_max_hz	= 333333333U,
		.plld		= 27U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_19_2MHZ_400MHZ] =     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 8U,
		.pllm		= 500U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_20MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 8U,
		.pllm		= 480U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_24MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 12U,
		.pllm		= 600U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_25MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 10U,
		.pllm		= 480U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_26MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 13U,
		.pllm		= 600U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DDR_27MHZ_400MHZ] =	     {
		.freq_min_hz	= 400000000U,
		.freq_max_hz	= 400000000U,
		.plld		= 18U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 3U,
	},
	[PLL_ENTRY_DSS_19_2MHZ_1155MHZ] =    {
		.freq_min_hz	= 1155000000U,
		.freq_max_hz	= 1155000000U,
		.plld		= 32U,
		.pllm		= 1925U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DSS_20MHZ_1155MHZ] =	     {
		.freq_min_hz	= 1155000000U,
		.freq_max_hz	= 1155000000U,
		.plld		= 8U,
		.pllm		= 462U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DSS_24MHZ_1155MHZ] =	     {
		.freq_min_hz	= 1155000000U,
		.freq_max_hz	= 1155000000U,
		.plld		= 24U,
		.pllm		= 1155U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DSS_25MHZ_1155MHZ] =	     {
		.freq_min_hz	= 1155000000U,
		.freq_max_hz	= 1155000000U,
		.plld		= 10U,
		.pllm		= 462U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DSS_26MHZ_1155MHZ] =	     {
		.freq_min_hz	= 1155000000U,
		.freq_max_hz	= 1155000000U,
		.plld		= 26U,
		.pllm		= 1155U,
		.pllfm		= 0U,
		.clkod		= 1U,
	},
	[PLL_ENTRY_DSS_27MHZ_1155MHZ] =	     {
		.freq_min_hz	= 1155000000U,
		.freq_max_hz	= 1155000000U,
		.plld		= 18U,
		.pllm		= 1540U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_19_2MHZ_800MHZ] =     {
		.freq_min_hz	= 800000000U,
		.freq_max_hz	= 800000000U,
		.plld		= 3U,
		.pllm		= 250U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_20MHZ_800MHZ] =	     {
		.freq_min_hz	= 800000000U,
		.freq_max_hz	= 800000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_24MHZ_800MHZ] =	     {
		.freq_min_hz	= 800000000U,
		.freq_max_hz	= 800000000U,
		.plld		= 3U,
		.pllm		= 200U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_25MHZ_800MHZ] =	     {
		.freq_min_hz	= 800000000U,
		.freq_max_hz	= 800000000U,
		.plld		= 1U,
		.pllm		= 64U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_26MHZ_800MHZ] =	     {
		.freq_min_hz	= 800000000U,
		.freq_max_hz	= 800000000U,
		.plld		= 13U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_27MHZ_800MHZ] =	     {
		.freq_min_hz	= 800000000U,
		.freq_max_hz	= 800000000U,
		.plld		= 27U,
		.pllm		= 1600U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_19_2MHZ_1000MHZ] =    {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 12U,
		.pllm		= 1250U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_20MHZ_1000MHZ] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_24MHZ_1000MHZ] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 3U,
		.pllm		= 250U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_25MHZ_1000MHZ] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 1U,
		.pllm		= 80U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_26MHZ_1000MHZ] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 13U,
		.pllm		= 1000U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_27MHZ_1000MHZ] =	     {
		.freq_min_hz	= 1000000000U,
		.freq_max_hz	= 1000000000U,
		.plld		= 27U,
		.pllm		= 2000U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_19_2MHZ_1100MHZ] =    {
		.freq_min_hz	= 1100000000U,
		.freq_max_hz	= 1100000000U,
		.plld		= 24U,
		.pllm		= 2750U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_20MHZ_1100MHZ] =	     {
		.freq_min_hz	= 1100000000U,
		.freq_max_hz	= 1100000000U,
		.plld		= 1U,
		.pllm		= 110U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_24MHZ_1100MHZ] =	     {
		.freq_min_hz	= 1100000000U,
		.freq_max_hz	= 1100000000U,
		.plld		= 6U,
		.pllm		= 550U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_25MHZ_1100MHZ] =	     {
		.freq_min_hz	= 1100000000U,
		.freq_max_hz	= 1100000000U,
		.plld		= 1U,
		.pllm		= 88U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_26MHZ_1100MHZ] =	     {
		.freq_min_hz	= 1100000000U,
		.freq_max_hz	= 1100000000U,
		.plld		= 13U,
		.pllm		= 1100U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_27MHZ_1100MHZ] =	     {
		.freq_min_hz	= 1100000000U,
		.freq_max_hz	= 1100000000U,
		.plld		= 27U,
		.pllm		= 2200U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_19_2MHZ_1200MHZ] =    {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 2U,
		.pllm		= 250U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_20MHZ_1200MHZ] =	     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 1U,
		.pllm		= 120U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_24MHZ_1200MHZ] =	     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 1U,
		.pllm		= 100U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_25MHZ_1200MHZ] =	     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 1U,
		.pllm		= 96U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_26MHZ_1200MHZ] =	     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 13U,
		.pllm		= 1200U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
	[PLL_ENTRY_ARM_27MHZ_1200MHZ] =	     {
		.freq_min_hz	= 1200000000U,
		.freq_max_hz	= 1200000000U,
		.plld		= 9U,
		.pllm		= 800U,
		.pllfm		= 0U,
		.clkod		= 2U,
	},
};

static const u8 adpllljm_hsdiv_wrap_main_0_entries[19] = {
	PLL_ENTRY_MAIN_19_2MHZ_100MHZ,
	PLL_ENTRY_MAIN_20MHZ_100MHZ,
	PLL_ENTRY_MAIN_24MHZ_100MHZ,
	PLL_ENTRY_MAIN_25MHZ_100MHZ,
	PLL_ENTRY_MAIN_26MHZ_100MHZ,
	PLL_ENTRY_MAIN_27MHZ_100MHZ,
	PLL_ENTRY_MAIN_19_2MHZ_125MHZ,
	PLL_ENTRY_MAIN_20MHZ_125MHZ,
	PLL_ENTRY_MAIN_24MHZ_125MHZ,
	PLL_ENTRY_MAIN_25MHZ_125MHZ,
	PLL_ENTRY_MAIN_26MHZ_125MHZ,
	PLL_ENTRY_MAIN_27MHZ_125MHZ,
	PLL_ENTRY_MAIN_19_2MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_20MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_24MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_25MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_26MHZ_156_25MHZ,
	PLL_ENTRY_MAIN_27MHZ_156_25MHZ,
	PLL_TABLE_LAST,
};
static const u8 adpllljm_hsdiv_wrap_main_2_entries[7] = {
	PLL_ENTRY_PER1_19_2MHZ_300MHZ,
	PLL_ENTRY_PER1_20MHZ_300MHZ,
	PLL_ENTRY_PER1_24MHZ_300MHZ,
	PLL_ENTRY_PER1_25MHZ_300MHZ,
	PLL_ENTRY_PER1_26MHZ_300MHZ,
	PLL_ENTRY_PER1_27MHZ_300MHZ,
	PLL_TABLE_LAST,
};
static const u8 adpllljm_wrap_main_1_entries[7] = {
	PLL_ENTRY_PER0_19_2MHZ_960MHZ,
	PLL_ENTRY_PER0_20MHZ_960MHZ,
	PLL_ENTRY_PER0_24MHZ_960MHZ,
	PLL_ENTRY_PER0_25MHZ_960MHZ,
	PLL_ENTRY_PER0_26MHZ_960MHZ,
	PLL_ENTRY_PER0_27MHZ_960MHZ,
	PLL_TABLE_LAST,
};
static const u8 adpllljm_wrap_main_3_entries[13] = {
	PLL_ENTRY_DDR_19_2MHZ_333MHZ,
	PLL_ENTRY_DDR_20MHZ_333MHZ,
	PLL_ENTRY_DDR_24MHZ_333MHZ,
	PLL_ENTRY_DDR_25MHZ_333MHZ,
	PLL_ENTRY_DDR_26MHZ_333MHZ,
	PLL_ENTRY_DDR_27MHZ_333MHZ,
	PLL_ENTRY_DDR_19_2MHZ_400MHZ,
	PLL_ENTRY_DDR_20MHZ_400MHZ,
	PLL_ENTRY_DDR_24MHZ_400MHZ,
	PLL_ENTRY_DDR_25MHZ_400MHZ,
	PLL_ENTRY_DDR_26MHZ_400MHZ,
	PLL_ENTRY_DDR_27MHZ_400MHZ,
	PLL_TABLE_LAST,
};
static const u8 adpllljm_wrap_main_4_entries[7] = {
	PLL_ENTRY_DSS_19_2MHZ_1155MHZ,
	PLL_ENTRY_DSS_20MHZ_1155MHZ,
	PLL_ENTRY_DSS_24MHZ_1155MHZ,
	PLL_ENTRY_DSS_25MHZ_1155MHZ,
	PLL_ENTRY_DSS_26MHZ_1155MHZ,
	PLL_ENTRY_DSS_27MHZ_1155MHZ,
	PLL_TABLE_LAST,
};
static const u8 adpllm_hsdiv_wrap_mcu_0_entries[7] = {
	PLL_ENTRY_MCU0_19_2MHZ_400MHZ,
	PLL_ENTRY_MCU0_20MHZ_400MHZ,
	PLL_ENTRY_MCU0_24MHZ_400MHZ,
	PLL_ENTRY_MCU0_25MHZ_400MHZ,
	PLL_ENTRY_MCU0_26MHZ_400MHZ,
	PLL_ENTRY_MCU0_27MHZ_400MHZ,
	PLL_TABLE_LAST,
};
static const u8 adpllm_hsdiv_wrap_mcu_1_entries[7] = {
	PLL_ENTRY_CPSW_19_2MHZ_250MHZ,
	PLL_ENTRY_CPSW_20MHZ_250MHZ,
	PLL_ENTRY_CPSW_24MHZ_250MHZ,
	PLL_ENTRY_CPSW_25MHZ_250MHZ,
	PLL_ENTRY_CPSW_26MHZ_250MHZ,
	PLL_ENTRY_CPSW_27MHZ_250MHZ,
	PLL_TABLE_LAST,
};
static const u8 adpllm_wrap_main_6_entries[25] = {
	PLL_ENTRY_ARM_19_2MHZ_800MHZ,
	PLL_ENTRY_ARM_20MHZ_800MHZ,
	PLL_ENTRY_ARM_24MHZ_800MHZ,
	PLL_ENTRY_ARM_25MHZ_800MHZ,
	PLL_ENTRY_ARM_26MHZ_800MHZ,
	PLL_ENTRY_ARM_27MHZ_800MHZ,
	PLL_ENTRY_ARM_19_2MHZ_1000MHZ,
	PLL_ENTRY_ARM_20MHZ_1000MHZ,
	PLL_ENTRY_ARM_24MHZ_1000MHZ,
	PLL_ENTRY_ARM_25MHZ_1000MHZ,
	PLL_ENTRY_ARM_26MHZ_1000MHZ,
	PLL_ENTRY_ARM_27MHZ_1000MHZ,
	PLL_ENTRY_ARM_19_2MHZ_1100MHZ,
	PLL_ENTRY_ARM_20MHZ_1100MHZ,
	PLL_ENTRY_ARM_24MHZ_1100MHZ,
	PLL_ENTRY_ARM_25MHZ_1100MHZ,
	PLL_ENTRY_ARM_26MHZ_1100MHZ,
	PLL_ENTRY_ARM_27MHZ_1100MHZ,
	PLL_ENTRY_ARM_19_2MHZ_1200MHZ,
	PLL_ENTRY_ARM_20MHZ_1200MHZ,
	PLL_ENTRY_ARM_24MHZ_1200MHZ,
	PLL_ENTRY_ARM_25MHZ_1200MHZ,
	PLL_ENTRY_ARM_26MHZ_1200MHZ,
	PLL_ENTRY_ARM_27MHZ_1200MHZ,
	PLL_TABLE_LAST,
};

static const struct clk_parent clk_GPMC_fclk_mux_parents[4] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK,
		3,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK,
		2,
	},
	{
		CLK_AM6_K3_PLL_CTRL_WRAP_MAIN_0_BUS_CHIP_DIV1_CLK_CLK,
		4,
	},
};
static const struct clk_data_mux_reg clk_data_GPMC_fclk_mux_bus_out0 = {
	.data_mux		= {
		.parents	= clk_GPMC_fclk_mux_parents,
		.n		= ARRAY_SIZE(clk_GPMC_fclk_mux_parents),
	},
	.reg			= 0x00100000 + 32976,
	.bit			= 0,
};
static const struct clk_parent clk_GTC_clk_parents[8] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_CPTS_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_CPTS_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM6_WIZ8B2M4VSB_MAIN_0_BUS_LN0_TXCLK,
		1,
	},
	{
		CLK_AM6_WIZ8B2M4VSB_MAIN_1_BUS_LN0_TXCLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_GTC_clk_bus_out0 = {
	.data_mux		= {
		.parents	= clk_GTC_clk_parents,
		.n		= ARRAY_SIZE(clk_GTC_clk_parents),
	},
	.reg			= 0x00100000 + 32816,
	.bit			= 0,
};
static const struct clk_parent clk_HFOSC_sel_bus_out0_parents[2] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_HFOSC_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_HFOSC_sel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_HFOSC_sel_bus_out0_parents),
	},
	.reg			= 0x43000000 + 32896,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_HFOSC_sel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_HFOSC_sel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_HFOSC_sel_bus_out0_parents),
	},
	.reg			= 0x43000000 + 32900,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_HFOSC_sel_bus_out2 = {
	.data_mux		= {
		.parents	= clk_HFOSC_sel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_HFOSC_sel_bus_out0_parents),
	},
	.reg			= 0x43000000 + 32904,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_HFOSC_sel_bus_out3 = {
	.data_mux		= {
		.parents	= clk_HFOSC_sel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_HFOSC_sel_bus_out0_parents),
	},
	.reg			= 0x43000000 + 32908,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_HFOSC_sel_bus_out4 = {
	.data_mux		= {
		.parents	= clk_HFOSC_sel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_HFOSC_sel_bus_out0_parents),
	},
	.reg			= 0x43000000 + 32912,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_HFOSC_sel_bus_out6 = {
	.data_mux		= {
		.parents	= clk_HFOSC_sel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_HFOSC_sel_bus_out0_parents),
	},
	.reg			= 0x43000000 + 32920,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_HFOSC_sel_bus_out7 = {
	.data_mux		= {
		.parents	= clk_HFOSC_sel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_HFOSC_sel_bus_out0_parents),
	},
	.reg			= 0x43000000 + 32924,
	.bit			= 0,
};
static const struct clk_parent clk_ICSSG_core_clk_bus_out0_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT1_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_ICSSG_core_clk_bus_out0 = {
	.data_mux		= {
		.parents	= clk_ICSSG_core_clk_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_ICSSG_core_clk_bus_out0_parents),
	},
	.reg			= 0x00100000 + 32832,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ICSSG_core_clk_bus_out1 = {
	.data_mux		= {
		.parents	= clk_ICSSG_core_clk_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_ICSSG_core_clk_bus_out0_parents),
	},
	.reg			= 0x00100000 + 32836,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ICSSG_core_clk_bus_out2 = {
	.data_mux		= {
		.parents	= clk_ICSSG_core_clk_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_ICSSG_core_clk_bus_out0_parents),
	},
	.reg			= 0x00100000 + 32840,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_ICSS_iep_clk_bus_out0 = {
	.data_mux		= {
		.parents	= clk_GTC_clk_parents,
		.n		= ARRAY_SIZE(clk_GTC_clk_parents),
	},
	.reg			= 0x00100000 + 32832,
	.bit			= 16,
};
static const struct clk_data_mux_reg clk_data_ICSS_iep_clk_bus_out1 = {
	.data_mux		= {
		.parents	= clk_GTC_clk_parents,
		.n		= ARRAY_SIZE(clk_GTC_clk_parents),
	},
	.reg			= 0x00100000 + 32836,
	.bit			= 16,
};
static const struct clk_data_mux_reg clk_data_ICSS_iep_clk_bus_out2 = {
	.data_mux		= {
		.parents	= clk_GTC_clk_parents,
		.n		= ARRAY_SIZE(clk_GTC_clk_parents),
	},
	.reg			= 0x00100000 + 32840,
	.bit			= 16,
};
static const struct clk_parent clk_MCU_ADC_clksel_bus_out0_parents[4] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT1_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_ADC_clksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_ADC_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_ADC_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32832,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MCU_ADC_clksel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_MCU_ADC_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_ADC_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32836,
	.bit			= 0,
};
static const struct clk_parent clk_MCU_R5_CPU_clock_mux_bus_out0_parents[2] = {
	{
		CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV1_CLK_CLK,
		1,
	},
	{
		CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV1_CLK_CLK,
		2,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_R5_CPU_clock_mux_bus_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_R5_CPU_clock_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_R5_CPU_clock_mux_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32896,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MCU_R5_CPU_clock_mux_bus_out1 = {
	.data_mux		= {
		.parents	= clk_MCU_R5_CPU_clock_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_R5_CPU_clock_mux_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32900,
	.bit			= 0,
};
static const struct clk_parent clk_MCU_R5_Phase_mux_bus_out0_parents[2] = {
	{
		CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV1_CLK_CLK,
		2,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_MCU_R5_Phase_mux_bus_out0 = {
	.data_mux		= {
		.parents	= clk_MCU_R5_Phase_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_R5_Phase_mux_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32896,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_MCU_R5_Phase_mux_bus_out1 = {
	.data_mux		= {
		.parents	= clk_MCU_R5_Phase_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_MCU_R5_Phase_mux_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32896,
	.bit			= 0,
};
static const struct clk_parent clk_Timer_mux_bus_out0_parents[16] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM6_GLUELOGIC_LFOSC_CLK_BUS_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_CPTS_RFT_CLK_OUT,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
		5,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
		1,
	},
	{
		CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF2_0,
		1,
	},
	{
		CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF3_0,
		1,
	},
	{
		CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF4_0,
		1,
	},
	{
		CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF5_0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out0 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33024,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out1 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33028,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out10 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33064,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out11 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33068,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out2 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33032,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out3 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33036,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out4 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33040,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out5 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33044,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out6 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33048,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out7 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33052,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out8 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33056,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_Timer_mux_bus_out9 = {
	.data_mux		= {
		.parents	= clk_Timer_mux_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_Timer_mux_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33060,
	.bit			= 0,
};
static const struct clk_parent clk_USB0_PIPE3_CLK_SEL_parents[2] = {
	{
		CLK_AM6_WIZ8B2M4VSB_MAIN_0_BUS_LN0_TXCLK,
		1,
	},
	{
		CLK_AM6_USB0_PIPE3_CLK_SEL_DIV_BUS_WKUP_RCOSC_12P5M_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_USB0_PIPE3_CLK_SEL_bus_out0 = {
	.data_mux		= {
		.parents	= clk_USB0_PIPE3_CLK_SEL_parents,
		.n		= ARRAY_SIZE(clk_USB0_PIPE3_CLK_SEL_parents),
	},
	.reg			= 0x00100000 + 17412,
	.bit			= 0,
};
static const struct clk_data_div clk_data_USB0_PIPE3_CLK_SEL_div_bus_in1 = {
	.n	= 256,
};
static const struct clk_parent clk_XREF_sel_parents[2] = {
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT2,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_EXT_REFCLK1_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_XREF_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_XREF_sel_parents,
		.n		= ARRAY_SIZE(clk_XREF_sel_parents),
	},
	.reg			= 0x43000000 + 32904,
	.bit			= 4,
};
static const struct clk_data_pll_adpllm clk_data_adpllljm_hsdiv_wrap_main_0 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= adpllljm_hsdiv_wrap_main_0_entries,
		.default_freq_idx	= FREQ_ADPLLLJM_HSDIV_WRAP_MAIN_0_DEFAULT,
	},
	.idx				= 0,
	.hsdiv				= (sbool) 1,
	.ljm				= (sbool) 1,
	.base				= 0x00680000,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 127,
		},
		.reg		= 0x00680000U + (0x1000UL * 0U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x00680000U + (0x1000UL * 0U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_0_bus_clkoutldo_clk = {
	.reg	= 0x00680000U + (0x1000UL * 0U) + 0x30UL,
	.bit	= 5,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_0_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv0_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_0_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_0_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv0_gate = {
	.reg	= 0x00680000U + (0x1000UL * 0U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv1 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 4,
		},
		.reg			= 0x00680000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 0,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 1,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_0_hsdiv1_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT1_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv1_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_0_hsdiv1_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_0_hsdiv1_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv1_gate = {
	.reg	= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 0,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv2 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 40,
		},
		.reg			= 0x00680000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 8,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 2,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_0_hsdiv2_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT2_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv2_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_0_hsdiv2_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_0_hsdiv2_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv2_gate = {
	.reg	= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 1,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv3 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 8,
		},
		.reg			= 0x00680000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 16,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 3,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_0_hsdiv3_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT3_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv3_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_0_hsdiv3_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_0_hsdiv3_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv3_gate = {
	.reg	= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 2,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv4 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 20,
		},
		.reg			= 0x00680000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 24,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 4,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_0_hsdiv4_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT4_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv4_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_0_hsdiv4_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_0_hsdiv4_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv4_gate = {
	.reg	= 0x00680000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllljm_hsdiv_wrap_main_2 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= adpllljm_hsdiv_wrap_main_2_entries,
		.default_freq_idx	= FREQ_ADPLLLJM_HSDIV_WRAP_MAIN_2_DEFAULT,
	},
	.idx				= 2,
	.hsdiv				= (sbool) 1,
	.ljm				= (sbool) 1,
	.base				= 0x00680000,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 127,
		},
		.reg		= 0x00680000U + (0x1000UL * 2U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x00680000U + (0x1000UL * 2U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_2_bus_clkoutldo_clk = {
	.reg	= 0x00680000U + (0x1000UL * 2U) + 0x30UL,
	.bit	= 5,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_2_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_XREF_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv0_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_2_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_2_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv0_gate = {
	.reg	= 0x00680000U + (0x1000UL * 2U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv1 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 8,
		},
		.reg			= 0x00680000U + (0x1000UL * 2U) + 0x120UL,
		.bit			= 0,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 1,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_2_hsdiv1_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT1_CLK_DIV,
		1,
	},
	{
		CLK_AM6_XREF_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv1_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_2_hsdiv1_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_2_hsdiv1_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv1_gate = {
	.reg	= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.bit	= 0,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv2 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 18,
		},
		.reg			= 0x00680000U + (0x1000UL * 2U) + 0x120UL,
		.bit			= 8,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 2,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_2_hsdiv2_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK_DIV,
		1,
	},
	{
		CLK_AM6_XREF_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv2_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_2_hsdiv2_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_2_hsdiv2_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv2_gate = {
	.reg	= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.bit	= 1,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv3 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 9,
		},
		.reg			= 0x00680000U + (0x1000UL * 2U) + 0x120UL,
		.bit			= 16,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 3,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_2_hsdiv3_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT3_CLK_DIV,
		1,
	},
	{
		CLK_AM6_XREF_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv3_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_2_hsdiv3_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_2_hsdiv3_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv3_gate = {
	.reg	= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.bit	= 2,
};
static const struct clk_data_hsdiv clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv4 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 4,
		},
		.reg			= 0x00680000U + (0x1000UL * 2U) + 0x120UL,
		.bit			= 24,
	},
	.go_reg				= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 4,
};
static const struct clk_parent clk_adpllljm_hsdiv_wrap_main_2_hsdiv4_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT4_CLK_DIV,
		1,
	},
	{
		CLK_AM6_XREF_SEL_BUS_OUT0,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv4_mux = {
	.parents	= clk_adpllljm_hsdiv_wrap_main_2_hsdiv4_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_hsdiv_wrap_main_2_hsdiv4_parents),
};
static const struct clk_data_reg clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv4_gate = {
	.reg	= 0x00680000U + (0x1000UL * 2U) + 0x124UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllljm_wrap_main_1 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= adpllljm_wrap_main_1_entries,
		.default_freq_idx	= FREQ_ADPLLLJM_WRAP_MAIN_1_DEFAULT,
	},
	.idx				= 1,
	.hsdiv				= (sbool) 0,
	.ljm				= (sbool) 1,
	.base				= 0x00680000,
};
static const struct clk_data_hsdiv clk_data_adpllljm_wrap_main_1_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 127,
		},
		.reg		= 0x00680000U + (0x1000UL * 1U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x00680000U + (0x1000UL * 1U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_data_reg clk_data_adpllljm_wrap_main_1_bus_clkoutldo_clk = {
	.reg	= 0x00680000U + (0x1000UL * 1U) + 0x30UL,
	.bit	= 5,
};
static const struct clk_parent clk_adpllljm_wrap_main_1_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT1,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_wrap_main_1_hsdiv0_mux = {
	.parents	= clk_adpllljm_wrap_main_1_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_wrap_main_1_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllljm_wrap_main_1_hsdiv0_gate = {
	.reg	= 0x00680000U + (0x1000UL * 1U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllljm_wrap_main_3 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= adpllljm_wrap_main_3_entries,
		.default_freq_idx	= FREQ_ADPLLLJM_WRAP_MAIN_3_DEFAULT,
	},
	.idx				= 3,
	.hsdiv				= (sbool) 0,
	.ljm				= (sbool) 1,
	.base				= 0x00680000,
};
static const struct clk_data_hsdiv clk_data_adpllljm_wrap_main_3_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 127,
		},
		.reg		= 0x00680000U + (0x1000UL * 3U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x00680000U + (0x1000UL * 3U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_data_reg clk_data_adpllljm_wrap_main_3_bus_clkoutldo_clk = {
	.reg	= 0x00680000U + (0x1000UL * 3U) + 0x30UL,
	.bit	= 5,
};
static const struct clk_parent clk_adpllljm_wrap_main_3_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT3,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_wrap_main_3_hsdiv0_mux = {
	.parents	= clk_adpllljm_wrap_main_3_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_wrap_main_3_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllljm_wrap_main_3_hsdiv0_gate = {
	.reg	= 0x00680000U + (0x1000UL * 3U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllljm_wrap_main_4 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLLJM_HSDIV_WRAP_MAIN_0,
		.fractional_support	= STRUE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= adpllljm_wrap_main_4_entries,
		.default_freq_idx	= FREQ_ADPLLLJM_WRAP_MAIN_4_DEFAULT,
	},
	.idx				= 4,
	.hsdiv				= (sbool) 0,
	.ljm				= (sbool) 1,
	.base				= 0x00680000,
};
static const struct clk_data_hsdiv clk_data_adpllljm_wrap_main_4_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 127,
		},
		.reg		= 0x00680000U + (0x1000UL * 4U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x00680000U + (0x1000UL * 4U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_data_reg clk_data_adpllljm_wrap_main_4_bus_clkoutldo_clk = {
	.reg	= 0x00680000U + (0x1000UL * 4U) + 0x30UL,
	.bit	= 5,
};
static const struct clk_parent clk_adpllljm_wrap_main_4_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT4,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllljm_wrap_main_4_hsdiv0_mux = {
	.parents	= clk_adpllljm_wrap_main_4_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllljm_wrap_main_4_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllljm_wrap_main_4_hsdiv0_gate = {
	.reg	= 0x00680000U + (0x1000UL * 4U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllm_hsdiv_wrap_mcu_0 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLM_HSDIV_WRAP_MCU_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLM_HSDIV_WRAP_MCU_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= adpllm_hsdiv_wrap_mcu_0_entries,
	},
	.idx				= 0,
	.hsdiv				= (sbool) 1,
	.ljm				= (sbool) 0,
	.base				= 0x40D00000,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_0_bus_dcc_clk_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCOCLKLDO_CLK,
		2,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCOCLKLDO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_adpllm_hsdiv_wrap_mcu_0_bus_dcc_clk = {
	.data_mux		= {
		.parents	= clk_adpllm_hsdiv_wrap_mcu_0_bus_dcc_clk_parents,
		.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_0_bus_dcc_clk_parents),
	},
	.reg			= 0x40D00000U + (0x1000UL * 0U) + 0x30UL,
	.bit			= 27,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 31,
		},
		.reg		= 0x40D00000U + (0x1000UL * 0U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x40D00000U + (0x1000UL * 0U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_0_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv0_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_0_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_0_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv0_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 0U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv1 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 40,
		},
		.reg			= 0x40D00000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 0,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 1,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_0_hsdiv1_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT1_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv1_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_0_hsdiv1_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_0_hsdiv1_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv1_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 0,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv2 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 30,
		},
		.reg			= 0x40D00000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 8,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 2,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_0_hsdiv2_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv2_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_0_hsdiv2_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_0_hsdiv2_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv2_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 1,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv3 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 25,
		},
		.reg			= 0x40D00000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 16,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 3,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_0_hsdiv3_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv3_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_0_hsdiv3_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_0_hsdiv3_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv3_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 2,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv4 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 15,
		},
		.reg			= 0x40D00000U + (0x1000UL * 0U) + 0x120UL,
		.bit			= 24,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 4,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_0_hsdiv4_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT4_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv4_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_0_hsdiv4_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_0_hsdiv4_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv4_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 0U) + 0x124UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllm_hsdiv_wrap_mcu_1 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLM_HSDIV_WRAP_MCU_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLM_HSDIV_WRAP_MCU_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_00,
		.pll_entries		= adpllm_hsdiv_wrap_mcu_1_entries,
		.default_freq_idx	= FREQ_ADPLLM_HSDIV_WRAP_MCU_1_DEFAULT,
	},
	.idx				= 1,
	.hsdiv				= (sbool) 1,
	.ljm				= (sbool) 0,
	.base				= 0x40D00000,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_1_bus_dcc_clk_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCOCLKLDO_CLK,
		2,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCOCLKLDO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_adpllm_hsdiv_wrap_mcu_1_bus_dcc_clk = {
	.data_mux		= {
		.parents	= clk_adpllm_hsdiv_wrap_mcu_1_bus_dcc_clk_parents,
		.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_1_bus_dcc_clk_parents),
	},
	.reg			= 0x40D00000U + (0x1000UL * 1U) + 0x30UL,
	.bit			= 27,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 31,
		},
		.reg		= 0x40D00000U + (0x1000UL * 1U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x40D00000U + (0x1000UL * 1U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_1_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv0_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_1_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_1_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv0_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 1U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv1 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 10,
		},
		.reg			= 0x40D00000U + (0x1000UL * 1U) + 0x120UL,
		.bit			= 0,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 1,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_1_hsdiv1_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT1_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv1_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_1_hsdiv1_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_1_hsdiv1_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv1_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.bit	= 0,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv2 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 10,
		},
		.reg			= 0x40D00000U + (0x1000UL * 1U) + 0x120UL,
		.bit			= 8,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 2,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_1_hsdiv2_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv2_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_1_hsdiv2_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_1_hsdiv2_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv2_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.bit	= 1,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv3 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 15,
		},
		.reg			= 0x40D00000U + (0x1000UL * 1U) + 0x120UL,
		.bit			= 16,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 3,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_1_hsdiv3_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv3_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_1_hsdiv3_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_1_hsdiv3_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv3_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.bit	= 2,
};
static const struct clk_data_hsdiv clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv4 = {
	.data_div_reg			= {
		.data_div		= {
			.n		= 63,
			.default_div	= 6,
		},
		.reg			= 0x40D00000U + (0x1000UL * 1U) + 0x120UL,
		.bit			= 24,
	},
	.go_reg				= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.go_bit				= 31,
	.idx				= 4,
};
static const struct clk_parent clk_adpllm_hsdiv_wrap_mcu_1_hsdiv4_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT4_CLK_DIV,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv4_mux = {
	.parents	= clk_adpllm_hsdiv_wrap_mcu_1_hsdiv4_parents,
	.n		= ARRAY_SIZE(clk_adpllm_hsdiv_wrap_mcu_1_hsdiv4_parents),
};
static const struct clk_data_reg clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv4_gate = {
	.reg	= 0x40D00000U + (0x1000UL * 1U) + 0x124UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllm_wrap_main_6 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLM_HSDIV_WRAP_MCU_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLM_HSDIV_WRAP_MCU_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= adpllm_wrap_main_6_entries,
		.default_freq_idx	= FREQ_ADPLLM_WRAP_MAIN_6_DEFAULT,
	},
	.idx				= 6,
	.hsdiv				= (sbool) 0,
	.ljm				= (sbool) 0,
	.base				= 0x00680000,
};
static const struct clk_parent clk_adpllm_wrap_main_6_bus_dcc_clk_parents[2] = {
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_DCOCLKLDO_CLK,
		2,
	},
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_DCOCLKLDO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_adpllm_wrap_main_6_bus_dcc_clk = {
	.data_mux		= {
		.parents	= clk_adpllm_wrap_main_6_bus_dcc_clk_parents,
		.n		= ARRAY_SIZE(clk_adpllm_wrap_main_6_bus_dcc_clk_parents),
	},
	.reg			= 0x00680000U + (0x1000UL * 6U) + 0x30UL,
	.bit			= 27,
};
static const struct clk_data_hsdiv clk_data_adpllm_wrap_main_6_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 31,
		},
		.reg		= 0x00680000U + (0x1000UL * 6U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x00680000U + (0x1000UL * 6U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_parent clk_adpllm_wrap_main_6_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT6,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_wrap_main_6_hsdiv0_mux = {
	.parents	= clk_adpllm_wrap_main_6_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllm_wrap_main_6_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllm_wrap_main_6_hsdiv0_gate = {
	.reg	= 0x00680000U + (0x1000UL * 6U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_pll_adpllm clk_data_adpllm_wrap_main_7 = {
	.data_pll			= {
		.vco_range_idx		= AM6_FREQ_RANGE_VCO_ADPLLM_HSDIV_WRAP_MCU_0,
		.vco_in_range_idx	= AM6_FREQ_RANGE_VCO_IN_ADPLLM_HSDIV_WRAP_MCU_0,
		.fractional_support	= SFALSE,
		.devgrp			= DEVGRP_01,
		.pll_entries		= adpllm_wrap_main_6_entries,
		.default_freq_idx	= FREQ_ADPLLM_WRAP_MAIN_6_DEFAULT,
	},
	.idx				= 7,
	.hsdiv				= (sbool) 0,
	.ljm				= (sbool) 0,
	.base				= 0x00680000,
};
static const struct clk_parent clk_adpllm_wrap_main_7_bus_dcc_clk_parents[2] = {
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_DCOCLKLDO_CLK,
		2,
	},
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_DCOCLKLDO_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_adpllm_wrap_main_7_bus_dcc_clk = {
	.data_mux		= {
		.parents	= clk_adpllm_wrap_main_7_bus_dcc_clk_parents,
		.n		= ARRAY_SIZE(clk_adpllm_wrap_main_7_bus_dcc_clk_parents),
	},
	.reg			= 0x00680000U + (0x1000UL * 7U) + 0x30UL,
	.bit			= 27,
};
static const struct clk_data_hsdiv clk_data_adpllm_wrap_main_7_hsdiv0 = {
	.data_div_reg		= {
		.data_div	= {
			.n	= 31,
		},
		.reg		= 0x00680000U + (0x1000UL * 7U) + 0x28UL,
		.bit		= 8,
		.start_at_1	= 1U,
	},
	.go_reg			= 0x00680000U + (0x1000UL * 7U) + 0x2cUL,
	.go_bit			= 8,
	.idx			= 0,
};
static const struct clk_parent clk_adpllm_wrap_main_7_hsdiv0_parents[2] = {
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK_DIV,
		1,
	},
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT7,
		1,
	},
};
static const struct clk_data_mux clk_data_adpllm_wrap_main_7_hsdiv0_mux = {
	.parents	= clk_adpllm_wrap_main_7_hsdiv0_parents,
	.n		= ARRAY_SIZE(clk_adpllm_wrap_main_7_hsdiv0_parents),
};
static const struct clk_data_reg clk_data_adpllm_wrap_main_7_hsdiv0_gate = {
	.reg	= 0x00680000U + (0x1000UL * 7U) + 0x30UL,
	.bit	= 3,
};
static const struct clk_data_from_dev clk_data_board_0_hfosc1_clk_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_HFOSC1_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_CCDC0_PCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_CCDC0_PCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_CPTS_RFT_CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_CPTS_RFT_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_DSS0EXTPCLKIN_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_DSS0EXTPCLKIN_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_EXT_REFCLK1_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_EXT_REFCLK1_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_GPMCCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_GPMCCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP0ACLKR_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP0ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP0ACLKX_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP0ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP0AHCLKR_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP0AHCLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP0AHCLKX_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP0AHCLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP1ACLKR_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP1ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP1ACLKX_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP1ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP1AHCLKR_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP1AHCLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP1AHCLKX_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP1AHCLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP2ACLKR_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP2ACLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP2ACLKX_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP2ACLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP2AHCLKR_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP2AHCLKR_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCASP2AHCLKX_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCASP2AHCLKX_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_CPTS_RFT_CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_CPTS_RFT_CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_EXT_REFCLK0_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_EXT_REFCLK0_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_OSPI0DQS_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_OSPI0DQS_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_OSPI1DQS_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_OSPI1DQS_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_RGMII1_RCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_RGMII1_RCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_RGMII1_TCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_RGMII1_TCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_RMII1_REFCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_RMII1_REFCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_SPI0CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_SPI0CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_MCU_SPI1CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_MCU_SPI1CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG0_RGMII1_RCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG0_RGMII1_RCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG0_RGMII1_TCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG0_RGMII1_TCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG0_RGMII2_RCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG0_RGMII2_RCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG0_RGMII2_TCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG0_RGMII2_TCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG1_RGMII1_RCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG1_RGMII1_RCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG1_RGMII1_TCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG1_RGMII1_TCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG1_RGMII2_RCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG1_RGMII2_RCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG1_RGMII2_TCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG1_RGMII2_TCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG2_RGMII1_RCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG2_RGMII1_RCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG2_RGMII1_TCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG2_RGMII1_TCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG2_RGMII2_RCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG2_RGMII2_RCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_PRG2_RGMII2_TCLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_PRG2_RGMII2_TCLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_SPI0CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_SPI0CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_SPI1CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_SPI1CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_SPI2CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_SPI2CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_SPI3CLK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_SPI3CLK_OUT,
};
static const struct clk_data_from_dev clk_data_board_0_bus_WKUP_TCK_out = {
	.dev		= AM6_DEV_BOARD0,
	.clk_idx	= AM6_DEV_BOARD0_BUS_WKUP_TCK_OUT,
};
static const struct clk_data_div_reg clk_data_debugss_div_bus_out0 = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00100000 + 32824,
	.bit		= 8,
};
static const struct clk_data_div_reg clk_data_dss_bus_out0 = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00100000 + 32944,
	.bit		= 8,
};
static const struct clk_data_div_reg_go clk_data_dss_bus_out1 = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00100000 + 32960,
	.bit		= 8,
	.go		= 16,
};
static const struct clk_parent clk_dss_dpi1_parents[4] = {
	{
		CLK_AM6_DSS_BUS_OUT0,
		7,
	},
	{
		CLK_AM6_BOARD_0_BUS_DSS0EXTPCLKIN_OUT,
		1,
	},
	{
		CLK_AM6_DSS_BUS_OUT1,
		1,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_dss_dpi1_bus_out0 = {
	.data_mux		= {
		.parents	= clk_dss_dpi1_parents,
		.n		= ARRAY_SIZE(clk_dss_dpi1_parents),
	},
	.reg			= 0x00100000 + 32960,
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_fss_mcu_0_ospi_0_bus_ospi_oclk_clk = {
	.dev		= AM6_DEV_MCU_FSS0_OSPI_0,
	.clk_idx	= AM6_DEV_MCU_FSS0_OSPI_0_BUS_OSPI_OCLK_CLK,
};
static const struct clk_data_from_dev clk_data_fss_mcu_0_ospi_1_bus_ospi_oclk_clk = {
	.dev		= AM6_DEV_MCU_FSS0_OSPI_1,
	.clk_idx	= AM6_DEV_MCU_FSS0_OSPI_1_BUS_OSPI_OCLK_CLK,
};
static const struct clk_data_from_dev clk_data_gpmc_main_0_bus_po_gpmc_dev_clk = {
	.dev		= AM6_DEV_GPMC0,
	.clk_idx	= AM6_DEV_GPMC0_BUS_PO_GPMC_DEV_CLK,
};
static const struct clk_data_from_dev clk_data_icss_g_main_2_bus_wiz0_tx_mst_clk = {
	.dev		= AM6_DEV_PRU_ICSSG2,
	.clk_idx	= AM6_DEV_PRU_ICSSG2_BUS_WIZ0_TX_MST_CLK,
};
static const struct clk_data_from_dev clk_data_icss_g_main_2_bus_wiz1_tx_mst_clk = {
	.dev		= AM6_DEV_PRU_ICSSG2,
	.clk_idx	= AM6_DEV_PRU_ICSSG2_BUS_WIZ1_TX_MST_CLK,
};
static const struct clk_data_from_dev clk_data_k3_dss_ul_main_0_bus_dpi_1_out_clk = {
	.dev		= AM6_DEV_DSS0,
	.clk_idx	= AM6_DEV_DSS0_BUS_DPI_1_OUT_CLK,
};
static const struct clk_parent clk_k3_pll_ctrl_wrap_main_0_parents[2] = {
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT0,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT1_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_k3_pll_ctrl_wrap_main_0_bus_sysclkout_clk = {
	.data_mux		= {
		.parents	= clk_k3_pll_ctrl_wrap_main_0_parents,
		.n		= ARRAY_SIZE(clk_k3_pll_ctrl_wrap_main_0_parents),
	},
	.reg			= 0x00410000,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_main_0_bus_chip_div1_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00410000 + 0x118,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_main_0_bus_chip_div24_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00410000 + 0x11c,
	.bit		= 0,
};
static const struct clk_parent clk_k3_pll_ctrl_wrap_wkup_0_parents[2] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_k3_pll_ctrl_wrap_wkup_0_bus_sysclkout_clk = {
	.data_mux		= {
		.parents	= clk_k3_pll_ctrl_wrap_wkup_0_parents,
		.n		= ARRAY_SIZE(clk_k3_pll_ctrl_wrap_wkup_0_parents),
	},
	.reg			= 0x42010000,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_wkup_0_bus_chip_div1_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x42010000 + 0x118,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_k3_pll_ctrl_wrap_wkup_0_bus_chip_div24_clk_clk = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x42010000 + 0x11c,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_main_usart_div_bus_out0 = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00100000 + 33216,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_main_usart_div_bus_out1 = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00100000 + 33220,
	.bit		= 0,
};
static const struct clk_data_div_reg clk_data_main_usart_div_bus_out2 = {
	.data_div	= {
		.n	= 32,
	},
	.reg		= 0x00100000 + 33224,
	.bit		= 0,
};
static const struct clk_parent clk_main_wwd_clksel_bus_out0_parents[8] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_GLUELOGIC_LFOSC_CLK_BUS_OUT,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_main_wwd_clksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_main_wwd_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_main_wwd_clksel_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33152,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_main_wwd_clksel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_main_wwd_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_main_wwd_clksel_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33156,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_main_wwd_clksel_bus_out2 = {
	.data_mux		= {
		.parents	= clk_main_wwd_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_main_wwd_clksel_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33160,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_main_wwd_clksel_bus_out3 = {
	.data_mux		= {
		.parents	= clk_main_wwd_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_main_wwd_clksel_bus_out0_parents),
	},
	.reg			= 0x00100000 + 33164,
	.bit			= 0,
};
static const struct clk_parent clk_mcasp0_auxclk_sel_parents[8] = {
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_MCASP_ARM1_PLL_DIV_BUS_OUT0,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcasp0_auxclk_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcasp0_auxclk_sel_parents,
		.n		= ARRAY_SIZE(clk_mcasp0_auxclk_sel_parents),
	},
	.reg			= 0x00100000 + 32864,
	.bit			= 0,
};
static const struct clk_parent clk_mcasp1_auxclk_sel_parents[8] = {
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_MCASP_ARM1_PLL_DIV_BUS_OUT1,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcasp1_auxclk_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcasp1_auxclk_sel_parents,
		.n		= ARRAY_SIZE(clk_mcasp1_auxclk_sel_parents),
	},
	.reg			= 0x00100000 + 32868,
	.bit			= 0,
};
static const struct clk_parent clk_mcasp2_auxclk_sel_parents[8] = {
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_MCASP_ARM1_PLL_DIV_BUS_OUT2,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_EXT_REFCLK1_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcasp2_auxclk_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcasp2_auxclk_sel_parents,
		.n		= ARRAY_SIZE(clk_mcasp2_auxclk_sel_parents),
	},
	.reg			= 0x00100000 + 32872,
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_mcasp_arm1_pll_div_bus_out0 = {
	.data_div	= {
		.n	= 16,
	},
	.reg		= 0x00100000 + 32864,
	.bit		= 8,
};
static const struct clk_data_div_reg clk_data_mcasp_arm1_pll_div_bus_out1 = {
	.data_div	= {
		.n	= 16,
	},
	.reg		= 0x00100000 + 32868,
	.bit		= 8,
};
static const struct clk_data_div_reg clk_data_mcasp_arm1_pll_div_bus_out2 = {
	.data_div	= {
		.n	= 16,
	},
	.reg		= 0x00100000 + 32872,
	.bit		= 8,
};
static const struct clk_parent clk_mcu_OSPI_refclksel_bus_out0_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT4_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT4_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_OSPI_refclksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_OSPI_refclksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_OSPI_refclksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32816,
	.bit			= 4,
};
static const struct clk_data_mux_reg clk_data_mcu_OSPI_refclksel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_mcu_OSPI_refclksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_OSPI_refclksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32820,
	.bit			= 4,
};
static const struct clk_parent clk_mcu_WWD_clksel_bus_out0_parents[8] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_GLUELOGIC_LFOSC_CLK_BUS_OUT,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
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
static const struct clk_data_mux_reg clk_data_mcu_WWD_clksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_WWD_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_WWD_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 33152,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_mcu_WWD_clksel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_mcu_WWD_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_WWD_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 33156,
	.bit			= 0,
};
static const struct clk_parent clk_mcu_clkout_mux_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK,
		5,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK,
		10,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_clkout_mux_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_clkout_mux_parents,
		.n		= ARRAY_SIZE(clk_mcu_clkout_mux_parents),
	},
	.reg			= 0x40F00000 + 32784,
	.bit			= 0,
};
static const struct clk_parent clk_mcu_mcan_clksel_bus_out0_parents[4] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK,
		2,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK,
		2,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_mcan_clksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_mcan_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_mcan_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32800,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_mcu_mcan_clksel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_mcu_mcan_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_mcan_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 32804,
	.bit			= 0,
};
static const struct clk_parent clk_mcu_obsclk_parents[16] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT1_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT4_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT1_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT4_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_GLUELOGIC_LFOSC_CLK_BUS_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_obsclk_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_obsclk_parents,
		.n		= ARRAY_SIZE(clk_mcu_obsclk_parents),
	},
	.reg			= 0x43000000 + 32768,
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_mcu_obsclk_div_bus_out0 = {
	.data_div	= {
		.n	= 16,
	},
	.reg		= 0x43000000 + 32768,
	.bit		= 8,
};
static const struct clk_parent clk_mcu_ospi0_iclk_sel_parents[2] = {
	{
		CLK_AM6_BOARD_0_BUS_MCU_OSPI0DQS_OUT,
		1,
	},
	{
		CLK_AM6_FSS_MCU_0_OSPI_0_BUS_OSPI_OCLK_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_ospi0_iclk_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_ospi0_iclk_sel_parents,
		.n		= ARRAY_SIZE(clk_mcu_ospi0_iclk_sel_parents),
	},
	.reg			= 0x40F00000 + 32816,
	.bit			= 0,
};
static const struct clk_parent clk_mcu_ospi1_iclk_sel_parents[2] = {
	{
		CLK_AM6_BOARD_0_BUS_MCU_OSPI1DQS_OUT,
		1,
	},
	{
		CLK_AM6_FSS_MCU_0_OSPI_1_BUS_OSPI_OCLK_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_ospi1_iclk_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_ospi1_iclk_sel_parents,
		.n		= ARRAY_SIZE(clk_mcu_ospi1_iclk_sel_parents),
	},
	.reg			= 0x40F00000 + 32820,
	.bit			= 0,
};
static const struct clk_parent clk_mcu_rmii_refclk_mux_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK,
		5,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_RMII1_REFCLK_OUT,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_rmii_refclk_mux_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_rmii_refclk_mux_parents,
		.n		= ARRAY_SIZE(clk_mcu_rmii_refclk_mux_parents),
	},
	.reg			= 0x40F00000 + 32848,
	.bit			= 0,
};
static const struct clk_parent clk_mcu_timer_clksel_bus_out0_parents[8] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV1_CLK_CLK,
		2,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
		1,
	},
	{
		CLK_AM6_GLUELOGIC_LFOSC_CLK_BUS_OUT,
		1,
	},
	{
		0,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_timer_clksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_timer_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_timer_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 33024,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_mcu_timer_clksel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_mcu_timer_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_timer_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 33028,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_mcu_timer_clksel_bus_out2 = {
	.data_mux		= {
		.parents	= clk_mcu_timer_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_timer_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 33032,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_mcu_timer_clksel_bus_out3 = {
	.data_mux		= {
		.parents	= clk_mcu_timer_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_mcu_timer_clksel_bus_out0_parents),
	},
	.reg			= 0x40F00000 + 33036,
	.bit			= 0,
};
static const struct clk_parent clk_mcu_usart_fclksel_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
		5,
	},
};
static const struct clk_data_mux_reg clk_data_mcu_usart_fclksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_usart_fclksel_parents,
		.n		= ARRAY_SIZE(clk_mcu_usart_fclksel_parents),
	},
	.reg			= 0x40F00000 + 33216,
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_navss256l_main_0_bus_cpts0_genf2_0 = {
	.dev		= AM6_DEV_NAVSS0,
	.clk_idx	= AM6_DEV_NAVSS0_BUS_CPTS0_GENF2_0,
};
static const struct clk_data_from_dev clk_data_navss256l_main_0_bus_cpts0_genf3_0 = {
	.dev		= AM6_DEV_NAVSS0,
	.clk_idx	= AM6_DEV_NAVSS0_BUS_CPTS0_GENF3_0,
};
static const struct clk_data_from_dev clk_data_navss256l_main_0_bus_cpts0_genf4_0 = {
	.dev		= AM6_DEV_NAVSS0,
	.clk_idx	= AM6_DEV_NAVSS0_BUS_CPTS0_GENF4_0,
};
static const struct clk_data_from_dev clk_data_navss256l_main_0_bus_cpts0_genf5_0 = {
	.dev		= AM6_DEV_NAVSS0,
	.clk_idx	= AM6_DEV_NAVSS0_BUS_CPTS0_GENF5_0,
};
static const struct clk_parent clk_obsclk_parents[16] = {
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_GLUELOGIC_LFOSC_CLK_BUS_OUT,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT1_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
	{
		0,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_obsclk_bus_out0 = {
	.data_mux		= {
		.parents	= clk_obsclk_parents,
		.n		= ARRAY_SIZE(clk_obsclk_parents),
	},
	.reg			= 0x00100000 + 32768,
	.bit			= 0,
};
static const struct clk_data_div_reg clk_data_obsclk_div_bus_out0 = {
	.data_div	= {
		.n	= 256,
	},
	.reg		= 0x00100000 + 32768,
	.bit		= 8,
};
static const struct clk_data_from_dev clk_data_pcie_g3x2_main_0_bus_pcie_txr0_clk = {
	.dev		= AM6_DEV_PCIE0,
	.clk_idx	= AM6_DEV_PCIE0_BUS_PCIE_TXR0_CLK,
};
static const struct clk_data_from_dev clk_data_pcie_g3x2_main_0_bus_pcie_txr1_clk = {
	.dev		= AM6_DEV_PCIE0,
	.clk_idx	= AM6_DEV_PCIE0_BUS_PCIE_TXR1_CLK,
};
static const struct clk_data_from_dev clk_data_pcie_g3x2_main_1_bus_pcie_txr0_clk = {
	.dev		= AM6_DEV_PCIE1,
	.clk_idx	= AM6_DEV_PCIE1_BUS_PCIE_TXR0_CLK,
};
static const struct clk_parent clk_serdes_clksel_bus_out0_parents[4] = {
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
	{
		CLK_AM6_BOARD_0_HFOSC1_CLK_OUT,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT4_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_serdes_clksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_serdes_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_serdes_clksel_bus_out0_parents),
	},
	.reg			= 0x00100000 + 32800,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_serdes_clksel_bus_out1 = {
	.data_mux		= {
		.parents	= clk_serdes_clksel_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_serdes_clksel_bus_out0_parents),
	},
	.reg			= 0x00100000 + 32804,
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_spi_main_0_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCSPI0,
	.clk_idx	= AM6_DEV_MCSPI0_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_1_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCSPI1,
	.clk_idx	= AM6_DEV_MCSPI1_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_2_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCSPI2,
	.clk_idx	= AM6_DEV_MCSPI2_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_3_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCSPI3,
	.clk_idx	= AM6_DEV_MCSPI3_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_main_4_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCSPI4,
	.clk_idx	= AM6_DEV_MCSPI4_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_mcu_0_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCU_MCSPI0,
	.clk_idx	= AM6_DEV_MCU_MCSPI0_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_mcu_1_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCU_MCSPI1,
	.clk_idx	= AM6_DEV_MCU_MCSPI1_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_data_from_dev clk_data_spi_mcu_2_bus_io_clkspio_clk = {
	.dev		= AM6_DEV_MCU_MCSPI2,
	.clk_idx	= AM6_DEV_MCU_MCSPI2_BUS_IO_CLKSPIO_CLK,
};
static const struct clk_parent clk_usb_ref_bus_out0_parents[2] = {
	{
		CLK_AM6_HFOSC_SEL_BUS_OUT0,
		1,
	},
	{
		CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
		48,
	},
};
static const struct clk_data_mux_reg clk_data_usb_ref_bus_out0 = {
	.data_mux		= {
		.parents	= clk_usb_ref_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_usb_ref_bus_out0_parents),
	},
	.reg			= 0x00100000 + 32992,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_usb_ref_bus_out1 = {
	.data_mux		= {
		.parents	= clk_usb_ref_bus_out0_parents,
		.n		= ARRAY_SIZE(clk_usb_ref_bus_out0_parents),
	},
	.reg			= 0x00100000 + 32996,
	.bit			= 0,
};
static const struct clk_data_from_dev clk_data_wiz8b2m4vsb_main_0_bus_ln0_rxclk = {
	.dev		= AM6_DEV_SERDES0,
	.clk_idx	= AM6_DEV_SERDES0_BUS_LN0_RXCLK,
};
static const struct clk_data_from_dev clk_data_wiz8b2m4vsb_main_0_bus_ln0_txclk = {
	.dev		= AM6_DEV_SERDES0,
	.clk_idx	= AM6_DEV_SERDES0_BUS_LN0_TXCLK,
};
static const struct clk_data_from_dev clk_data_wiz8b2m4vsb_main_1_bus_ln0_rxclk = {
	.dev		= AM6_DEV_SERDES1,
	.clk_idx	= AM6_DEV_SERDES1_BUS_LN0_RXCLK,
};
static const struct clk_data_from_dev clk_data_wiz8b2m4vsb_main_1_bus_ln0_txclk = {
	.dev		= AM6_DEV_SERDES1,
	.clk_idx	= AM6_DEV_SERDES1_BUS_LN0_TXCLK,
};
static const struct clk_parent clk_wkup_I2C_piclk_sel_parents[2] = {
	{
		CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_wkup_I2C_piclk_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_wkup_I2C_piclk_sel_parents,
		.n		= ARRAY_SIZE(clk_wkup_I2C_piclk_sel_parents),
	},
	.reg			= 0x43000000 + 32864,
	.bit			= 0,
};
static const struct clk_parent clk_wkup_Usart_mcupllbypass_clksel_parents[2] = {
	{
		CLK_AM6_WKUPUSART_CLK_SEL_BUS_OUT0,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_wkup_Usart_mcupllbypass_clksel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_wkup_Usart_mcupllbypass_clksel_parents,
		.n		= ARRAY_SIZE(clk_wkup_Usart_mcupllbypass_clksel_parents),
	},
	.reg			= 0x43000000 + 32864,
	.bit			= 0,
};
static const struct clk_parent clk_wkup_gpio_clkmux_parents[4] = {
	{
		CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV1_CLK_CLK,
		4,
	},
	{
		CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV1_CLK_CLK,
		4,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
		1,
	},
	{
		CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
		1,
	},
};
static const struct clk_data_mux_reg clk_data_wkup_gpio_clkmux_bus_out0 = {
	.data_mux		= {
		.parents	= clk_wkup_gpio_clkmux_parents,
		.n		= ARRAY_SIZE(clk_wkup_gpio_clkmux_parents),
	},
	.reg			= 0x43000000 + 32880,
	.bit			= 0,
};
static const struct clk_data_mux_reg clk_data_wkupusart_clk_sel_bus_out0 = {
	.data_mux		= {
		.parents	= clk_mcu_usart_fclksel_parents,
		.n		= ARRAY_SIZE(clk_mcu_usart_fclksel_parents),
	},
	.reg			= 0x43000000 + 32868,
	.bit			= 0,
};

const struct clk_data soc_clock_data[254] = {
	[CLK_AM6_GLUELOGIC_LFOSC_CLK_BUS_OUT] =				 {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM6_FREQ_RANGE_GLUELOGIC_LFOSC_CLK_BUS_OUT,
	},
	[CLK_AM6_BOARD_0_HFOSC1_CLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_hfosc1_clk_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_HFOSC1_CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_CPTS_RFT_CLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_CPTS_RFT_CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_CPTS_RFT_CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_EXT_REFCLK0_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_EXT_REFCLK0_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_OSPI0DQS_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_OSPI0DQS_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_OSPI0DQS_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_RGMII1_RCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_RGMII1_RCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_RGMII1_RCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_RGMII1_TCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_RGMII1_TCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_RGMII1_TCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_RMII1_REFCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_RMII1_REFCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_RMII1_REFCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_SPI0CLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_SPI0CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_SPI0CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_SPI1CLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_SPI1CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_SPI1CLK_OUT,
	},
	[CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK] =		 {
		.drv	= &clk_drv_soc_hfosc0,
		.flags	= 0,
	},
	[CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK] =	 {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
	},
	[CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK] =	 {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_32K_CLK,
	},
	[CLK_AM6_SPI_MCU_0_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_mcu_0_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MCU_0_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_SPI_MCU_1_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_mcu_1_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MCU_1_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_SPI_MCU_2_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_mcu_2_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MCU_2_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_HFOSC_SEL_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_HFOSC_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_HFOSC_SEL_BUS_OUT1] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_HFOSC_sel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCOCLKLDO_CLK] =		 {
		.parent		=					 {
			CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllm_hsdiv_wrap_mcu_0.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLM_HSDIV_WRAP_MCU_0,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCC_CLK] =			 {
		.drv	= &clk_drv_mux_reg_ro.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_bus_dcc_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK_DIV] =		 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCC_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK_AON] =		 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK] =		 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT1_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv1.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT1_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv1_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT1_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT1_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv1_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv2.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv2_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT2_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv2_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv3.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv3_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT3_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv3_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT4_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv4.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv4.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT4_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv4_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT4_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_0_BUS_HSDIV_CLKOUT4_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_0_hsdiv4_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCOCLKLDO_CLK] =		 {
		.parent		=					 {
			CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_OSC0_CLK,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllm_hsdiv_wrap_mcu_1.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLM_HSDIV_WRAP_MCU_1,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCC_CLK] =			 {
		.drv	= &clk_drv_mux_reg_ro.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_bus_dcc_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK_DIV] =		 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCC_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK_AON] =		 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK] =		 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT1_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv1.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT1_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv1_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT1_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT1_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv1_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv2.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv2_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT2_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv2_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv3.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv3_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT3_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv3_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT4_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv4.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv4.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT4_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv4_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT4_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLM_HSDIV_WRAP_MCU_1_BUS_HSDIV_CLKOUT4_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_hsdiv_wrap_mcu_1_hsdiv4_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_SYSCLKOUT_CLK] =		 {
		.drv	= &clk_drv_pllctrl_mux_reg_ro.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_wkup_0_bus_sysclkout_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV1_CLK_CLK] =	 {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_wkup_0_bus_chip_div1_clk_clk.data_div.data,
		.parent =						 {
			CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM6_MCU_WWD_CLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_WWD_clksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_WWD_CLKSEL_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_WWD_clksel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_MCAN_CLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_mcan_clksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_MCAN_CLKSEL_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_mcan_clksel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_RMII_REFCLK_MUX_BUS_OUT0] =			 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_rmii_refclk_mux_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_TIMER_CLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_timer_clksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_TIMER_CLKSEL_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_timer_clksel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_TIMER_CLKSEL_BUS_OUT2] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_timer_clksel_bus_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_TIMER_CLKSEL_BUS_OUT3] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_timer_clksel_bus_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_USART_FCLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_usart_fclksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_WKUP_I2C_PICLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_wkup_I2C_piclk_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_WKUP_GPIO_CLKMUX_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_wkup_gpio_clkmux_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_WKUPUSART_CLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_wkupusart_clk_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_ADC_CLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_ADC_clksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_ADC_CLKSEL_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_ADC_clksel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_R5_CPU_CLOCK_MUX_BUS_OUT0] =			 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_R5_CPU_clock_mux_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_R5_CPU_CLOCK_MUX_BUS_OUT1] =			 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_R5_CPU_clock_mux_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_R5_PHASE_MUX_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_R5_Phase_mux_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_R5_PHASE_MUX_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_MCU_R5_Phase_mux_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_WKUP_USART_MCUPLLBYPASS_CLKSEL_BUS_OUT0] =		 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_wkup_Usart_mcupllbypass_clksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_HFOSC_SEL_BUS_OUT2] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_HFOSC_sel_bus_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_HFOSC_SEL_BUS_OUT3] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_HFOSC_sel_bus_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_HFOSC_SEL_BUS_OUT4] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_HFOSC_sel_bus_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_HFOSC_SEL_BUS_OUT6] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_HFOSC_sel_bus_out6.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_HFOSC_SEL_BUS_OUT7] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_HFOSC_sel_bus_out7.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_USB0_PIPE3_CLK_SEL_DIV_BUS_WKUP_RCOSC_12P5M_CLK] =	 {
		.parent =						 {
			CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_WKUP_RCOSC_12P5M_CLK,
			1,
		},
		.drv	= &clk_drv_div_fixed.drv,
		.flags	= 0,
		.data	= &clk_data_USB0_PIPE3_CLK_SEL_div_bus_in1.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_DCOCLKLDO_CLK] =	 {
		.parent		=					 {
			CLK_AM6_HFOSC_SEL_BUS_OUT0,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllljm_hsdiv_wrap_main_0.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLLJM_HSDIV_WRAP_MAIN_0,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT1_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv1.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT1_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv1_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT1_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT1_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv1_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT2_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv2.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT2_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv2_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT2_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT2_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv2_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT3_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv3.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT3_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv3_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT3_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT3_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv3_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT4_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv4.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv4.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT4_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv4_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT4_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_HSDIV_CLKOUT4_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_hsdiv4_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_DCOCLKLDO_CLK] =		 {
		.parent		=					 {
			CLK_AM6_HFOSC_SEL_BUS_OUT1,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllljm_wrap_main_1.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLLJM_WRAP_MAIN_1,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK_DIV] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_wrap_main_1_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUTLDO_CLK] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK_DIV,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_wrap_main_1_bus_clkoutldo_clk.data,
		.flags	= 0,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK_AON] =		 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_wrap_main_1_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK] =			 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_wrap_main_1_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_DCOCLKLDO_CLK] =		 {
		.parent		=					 {
			CLK_AM6_HFOSC_SEL_BUS_OUT3,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllljm_wrap_main_3.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLLJM_WRAP_MAIN_3,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUT_CLK_DIV] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_wrap_main_3_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUT_CLK_AON] =		 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_wrap_main_3_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUT_CLK] =			 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_wrap_main_3_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_DCOCLKLDO_CLK] =		 {
		.parent		=					 {
			CLK_AM6_HFOSC_SEL_BUS_OUT4,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllljm_wrap_main_4.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLLJM_WRAP_MAIN_4,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK_DIV] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_wrap_main_4_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK_AON] =		 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_wrap_main_4_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK] =			 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_wrap_main_4_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_DCOCLKLDO_CLK] =		 {
		.parent		=					 {
			CLK_AM6_HFOSC_SEL_BUS_OUT6,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllm_wrap_main_6.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLM_WRAP_MAIN_6,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_DCC_CLK] =			 {
		.drv	= &clk_drv_mux_reg_ro.drv,
		.data	= &clk_data_adpllm_wrap_main_6_bus_dcc_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_CLKOUT_CLK_DIV] =		 {
		.parent =						 {
			CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_DCC_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_wrap_main_6_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_CLKOUT_CLK_AON] =		 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_wrap_main_6_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_CLKOUT_CLK] =			 {
		.parent =						 {
			CLK_AM6_ADPLLM_WRAP_MAIN_6_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_wrap_main_6_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_DCOCLKLDO_CLK] =		 {
		.parent		=					 {
			CLK_AM6_HFOSC_SEL_BUS_OUT7,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllm_wrap_main_7.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLM_WRAP_MAIN_7,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_DCC_CLK] =			 {
		.drv	= &clk_drv_mux_reg_ro.drv,
		.data	= &clk_data_adpllm_wrap_main_7_bus_dcc_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK_DIV] =		 {
		.parent =						 {
			CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_DCC_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllm_wrap_main_7_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK_AON] =		 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllm_wrap_main_7_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK] =			 {
		.parent =						 {
			CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllm_wrap_main_7_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_BOARD_0_BUS_CCDC0_PCLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_CCDC0_PCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_CCDC0_PCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_CPTS_RFT_CLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_CPTS_RFT_CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_CPTS_RFT_CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_DSS0EXTPCLKIN_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_DSS0EXTPCLKIN_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_DSS0EXTPCLKIN_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_EXT_REFCLK1_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_EXT_REFCLK1_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_EXT_REFCLK1_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_GPMCCLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_GPMCCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_GPMCCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP0ACLKR_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP0ACLKR_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0ACLKR_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP0ACLKX_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP0ACLKX_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0ACLKX_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP0AHCLKR_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP0AHCLKR_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0AHCLKR_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP0AHCLKX_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP0AHCLKX_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP0AHCLKX_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP1ACLKR_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP1ACLKR_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1ACLKR_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP1ACLKX_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP1ACLKX_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1ACLKX_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP1AHCLKR_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP1AHCLKR_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1AHCLKR_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP1AHCLKX_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP1AHCLKX_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP1AHCLKX_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP2ACLKR_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP2ACLKR_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2ACLKR_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP2ACLKX_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP2ACLKX_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2ACLKX_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP2AHCLKR_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP2AHCLKR_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2AHCLKR_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCASP2AHCLKX_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCASP2AHCLKX_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCASP2AHCLKX_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_MCU_OSPI1DQS_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_MCU_OSPI1DQS_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_MCU_OSPI1DQS_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG0_RGMII1_RCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG0_RGMII1_RCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII1_RCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG0_RGMII1_TCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG0_RGMII1_TCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII1_TCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG0_RGMII2_RCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG0_RGMII2_RCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII2_RCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG0_RGMII2_TCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG0_RGMII2_TCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG0_RGMII2_TCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG1_RGMII1_RCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG1_RGMII1_RCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII1_RCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG1_RGMII1_TCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG1_RGMII1_TCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII1_TCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG1_RGMII2_RCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG1_RGMII2_RCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII2_RCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG1_RGMII2_TCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG1_RGMII2_TCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG1_RGMII2_TCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG2_RGMII1_RCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG2_RGMII1_RCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII1_RCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG2_RGMII1_TCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG2_RGMII1_TCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII1_TCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG2_RGMII2_RCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG2_RGMII2_RCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII2_RCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_PRG2_RGMII2_TCLK_OUT] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_PRG2_RGMII2_TCLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_PRG2_RGMII2_TCLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_SPI0CLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_SPI0CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_SPI0CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_SPI1CLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_SPI1CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_SPI1CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_SPI2CLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_SPI2CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_SPI2CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_SPI3CLK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_SPI3CLK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_SPI3CLK_OUT,
	},
	[CLK_AM6_BOARD_0_BUS_WKUP_TCK_OUT] =				 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_board_0_bus_WKUP_TCK_out.data,
		.freq_idx	= AM6_FREQ_VALUE_BOARD_0_BUS_WKUP_TCK_OUT,
	},
	[CLK_AM6_DEBUGSS_DIV_BUS_OUT0] =				 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_debugss_div_bus_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_DSS_BUS_OUT0] =					 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
		.data	= &clk_data_dss_bus_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_DSS_BUS_OUT1] =					 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg_go.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ | CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
		.data	= &clk_data_dss_bus_out1.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_DSS_DPI1_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= CLK_DATA_FLAG_ALLOW_FREQ_CHANGE,
		.data	= &clk_data_dss_dpi1_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_FSS_MCU_0_OSPI_0_BUS_OSPI_OCLK_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fss_mcu_0_ospi_0_bus_ospi_oclk_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_FSS_MCU_0_OSPI_0_BUS_OSPI_OCLK_CLK,
	},
	[CLK_AM6_FSS_MCU_0_OSPI_1_BUS_OSPI_OCLK_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_fss_mcu_0_ospi_1_bus_ospi_oclk_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_FSS_MCU_0_OSPI_1_BUS_OSPI_OCLK_CLK,
	},
	[CLK_AM6_GPMC_MAIN_0_BUS_PO_GPMC_DEV_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_gpmc_main_0_bus_po_gpmc_dev_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_GPMC_MAIN_0_BUS_PO_GPMC_DEV_CLK,
	},
	[CLK_AM6_ICSS_G_MAIN_2_BUS_WIZ0_TX_MST_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_main_2_bus_wiz0_tx_mst_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_ICSS_G_MAIN_2_BUS_WIZ0_TX_MST_CLK,
	},
	[CLK_AM6_ICSS_G_MAIN_2_BUS_WIZ1_TX_MST_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_icss_g_main_2_bus_wiz1_tx_mst_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_ICSS_G_MAIN_2_BUS_WIZ1_TX_MST_CLK,
	},
	[CLK_AM6_K3_DSS_UL_MAIN_0_BUS_DPI_1_OUT_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_k3_dss_ul_main_0_bus_dpi_1_out_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_K3_DSS_UL_MAIN_0_BUS_DPI_1_OUT_CLK,
	},
	[CLK_AM6_K3_PLL_CTRL_WRAP_MAIN_0_BUS_SYSCLKOUT_CLK] =		 {
		.drv	= &clk_drv_pllctrl_mux_reg_ro.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_main_0_bus_sysclkout_clk.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_K3_PLL_CTRL_WRAP_MAIN_0_BUS_CHIP_DIV1_CLK_CLK] =	 {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_main_0_bus_chip_div1_clk_clk.data_div.data,
		.parent =						 {
			CLK_AM6_K3_PLL_CTRL_WRAP_MAIN_0_BUS_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM6_MAIN_USART_DIV_BUS_OUT0] =				 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_main_usart_div_bus_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_MAIN_USART_DIV_BUS_OUT1] =				 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_main_usart_div_bus_out1.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_MAIN_USART_DIV_BUS_OUT2] =				 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_1_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_main_usart_div_bus_out2.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_MAIN_WWD_CLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_main_wwd_clksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MAIN_WWD_CLKSEL_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_main_wwd_clksel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MAIN_WWD_CLKSEL_BUS_OUT2] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_main_wwd_clksel_bus_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MAIN_WWD_CLKSEL_BUS_OUT3] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_main_wwd_clksel_bus_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCASP_ARM1_PLL_DIV_BUS_OUT0] =				 {
		.parent =						 {
			CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcasp_arm1_pll_div_bus_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_MCASP_ARM1_PLL_DIV_BUS_OUT1] =				 {
		.parent =						 {
			CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcasp_arm1_pll_div_bus_out1.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_MCASP_ARM1_PLL_DIV_BUS_OUT2] =				 {
		.parent =						 {
			CLK_AM6_ADPLLM_WRAP_MAIN_7_BUS_CLKOUT_CLK,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcasp_arm1_pll_div_bus_out2.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_MCU_OSPI_REFCLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_OSPI_refclksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_OSPI_REFCLKSEL_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_OSPI_refclksel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_CLKOUT_MUX_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_clkout_mux_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_OBSCLK_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_obsclk_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_OBSCLK_DIV_BUS_OUT0] =				 {
		.parent =						 {
			CLK_AM6_MCU_OBSCLK_BUS_OUT0,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_obsclk_div_bus_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_MCU_OSPI0_ICLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_ospi0_iclk_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCU_OSPI1_ICLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcu_ospi1_iclk_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF2_0] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_navss256l_main_0_bus_cpts0_genf2_0.data,
		.freq_idx	= AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF2_0,
	},
	[CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF3_0] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_navss256l_main_0_bus_cpts0_genf3_0.data,
		.freq_idx	= AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF3_0,
	},
	[CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF4_0] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_navss256l_main_0_bus_cpts0_genf4_0.data,
		.freq_idx	= AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF4_0,
	},
	[CLK_AM6_NAVSS256L_MAIN_0_BUS_CPTS0_GENF5_0] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_navss256l_main_0_bus_cpts0_genf5_0.data,
		.freq_idx	= AM6_FREQ_VALUE_NAVSS256L_MAIN_0_BUS_CPTS0_GENF5_0,
	},
	[CLK_AM6_PCIE_G3X2_MAIN_0_BUS_PCIE_TXR0_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_pcie_g3x2_main_0_bus_pcie_txr0_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_PCIE_G3X2_MAIN_0_BUS_PCIE_TXR0_CLK,
	},
	[CLK_AM6_PCIE_G3X2_MAIN_0_BUS_PCIE_TXR1_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_pcie_g3x2_main_0_bus_pcie_txr1_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_PCIE_G3X2_MAIN_0_BUS_PCIE_TXR1_CLK,
	},
	[CLK_AM6_PCIE_G3X2_MAIN_1_BUS_PCIE_TXR0_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_pcie_g3x2_main_1_bus_pcie_txr0_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_PCIE_G3X2_MAIN_1_BUS_PCIE_TXR0_CLK,
	},
	[CLK_AM6_SERDES_CLKSEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_serdes_clksel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_SERDES_CLKSEL_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_serdes_clksel_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_SPI_MAIN_0_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_0_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MAIN_0_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_SPI_MAIN_1_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_1_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MAIN_1_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_SPI_MAIN_2_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_2_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MAIN_2_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_SPI_MAIN_3_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_3_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MAIN_3_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_SPI_MAIN_4_BUS_IO_CLKSPIO_CLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_spi_main_4_bus_io_clkspio_clk.data,
		.freq_idx	= AM6_FREQ_VALUE_SPI_MAIN_4_BUS_IO_CLKSPIO_CLK,
	},
	[CLK_AM6_USB_REF_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_usb_ref_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_USB_REF_BUS_OUT1] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_usb_ref_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_WIZ8B2M4VSB_MAIN_0_BUS_LN0_RXCLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz8b2m4vsb_main_0_bus_ln0_rxclk.data,
		.freq_idx	= AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_0_BUS_LN0_RXCLK,
	},
	[CLK_AM6_WIZ8B2M4VSB_MAIN_0_BUS_LN0_TXCLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz8b2m4vsb_main_0_bus_ln0_txclk.data,
		.freq_idx	= AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_0_BUS_LN0_TXCLK,
	},
	[CLK_AM6_WIZ8B2M4VSB_MAIN_1_BUS_LN0_RXCLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz8b2m4vsb_main_1_bus_ln0_rxclk.data,
		.freq_idx	= AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_1_BUS_LN0_RXCLK,
	},
	[CLK_AM6_WIZ8B2M4VSB_MAIN_1_BUS_LN0_TXCLK] =			 {
		.drv		= &clk_drv_from_device,
		.flags		= 0,
		.data		= &clk_data_wiz8b2m4vsb_main_1_bus_ln0_txclk.data,
		.freq_idx	= AM6_FREQ_VALUE_WIZ8B2M4VSB_MAIN_1_BUS_LN0_TXCLK,
	},
	[CLK_AM6_GTC_CLK_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_GTC_clk_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ICSS_IEP_CLK_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ICSS_iep_clk_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ICSS_IEP_CLK_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ICSS_iep_clk_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ICSS_IEP_CLK_BUS_OUT2] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ICSS_iep_clk_bus_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_USB0_PIPE3_CLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_USB0_PIPE3_CLK_SEL_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_XREF_SEL_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_XREF_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_DCOCLKLDO_CLK] =	 {
		.parent		=					 {
			CLK_AM6_XREF_SEL_BUS_OUT0,
			1,
		},
		.drv		= &clk_drv_adpllm,
		.data		= &clk_data_adpllljm_hsdiv_wrap_main_2.data_pll.data,
		.freq_idx	= AM6_FREQ_VALUE_ADPLLLJM_HSDIV_WRAP_MAIN_2,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK_DIV] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv0.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK_AON] =	 {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv0_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv0_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT1_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv1.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT1_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv1_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT1_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT1_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv1_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv2.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv2_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT2_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv2_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT3_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv3.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT3_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv3_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT3_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT3_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv3_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT4_CLK_DIV] = {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_DCOCLKLDO_CLK,
			1,
		},
		.drv	= &clk_drv_div_hsdiv4.drv,
		.flags	= 0,
		.type	= CLK_TYPE_DIV,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv4.data_div_reg.data_div.data,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT4_CLK_AON] = {
		.drv	= &clk_drv_adpllm_bypass_mux.drv,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv4_mux.data,
		.flags	= 0,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT4_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_HSDIV_CLKOUT4_CLK_AON,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_hsdiv4_gate.data,
		.flags	= CLK_DATA_FLAG_MODIFY_PARENT_FREQ,
	},
	[CLK_AM6_MCASP0_AUXCLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcasp0_auxclk_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCASP1_AUXCLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcasp1_auxclk_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_MCASP2_AUXCLK_SEL_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_mcasp2_auxclk_sel_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_OBSCLK_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_obsclk_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_OBSCLK_DIV_BUS_OUT0] =					 {
		.parent =						 {
			CLK_AM6_OBSCLK_BUS_OUT0,
			1,
		},
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_obsclk_div_bus_out0.data_div.data,
		.type	= CLK_TYPE_DIV,
	},
	[CLK_AM6_GPMC_FCLK_MUX_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_GPMC_fclk_mux_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ICSSG_CORE_CLK_BUS_OUT0] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ICSSG_core_clk_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ICSSG_CORE_CLK_BUS_OUT1] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ICSSG_core_clk_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ICSSG_CORE_CLK_BUS_OUT2] =				 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_ICSSG_core_clk_bus_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT0] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out0.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT1] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out1.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT10] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out10.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT11] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out11.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT2] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out2.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT3] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out3.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT4] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out4.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT5] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out5.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT6] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out6.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT7] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out7.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT8] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out8.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_TIMER_MUX_BUS_OUT9] =					 {
		.drv	= &clk_drv_mux_reg.drv,
		.flags	= 0,
		.data	= &clk_data_Timer_mux_bus_out9.data_mux.data,
		.type	= CLK_TYPE_MUX,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUTLDO_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_0_BUS_CLKOUT_CLK_DIV,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_0_bus_clkoutldo_clk.data,
		.flags	= 0,
	},
	[CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUTLDO_CLK] =	 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_HSDIV_WRAP_MAIN_2_BUS_CLKOUT_CLK_DIV,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_hsdiv_wrap_main_2_bus_clkoutldo_clk.data,
		.flags	= 0,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUTLDO_CLK] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_3_BUS_CLKOUT_CLK_DIV,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_wrap_main_3_bus_clkoutldo_clk.data,
		.flags	= 0,
	},
	[CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUTLDO_CLK] =		 {
		.parent =						 {
			CLK_AM6_ADPLLLJM_WRAP_MAIN_4_BUS_CLKOUT_CLK_DIV,
			1,
		},
		.drv	= &clk_gate_drv_reg,
		.data	= &clk_data_adpllljm_wrap_main_4_bus_clkoutldo_clk.data,
		.flags	= 0,
	},
	[CLK_AM6_K3_PLL_CTRL_WRAP_MAIN_0_BUS_CHIP_DIV24_CLK_CLK] =	 {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_main_0_bus_chip_div24_clk_clk.data_div.data,
		.parent =						 {
			CLK_AM6_K3_PLL_CTRL_WRAP_MAIN_0_BUS_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_CHIP_DIV24_CLK_CLK] =	 {
		.drv	= &clk_drv_div_reg.drv,
		.flags	= 0,
		.data	= &clk_data_k3_pll_ctrl_wrap_wkup_0_bus_chip_div24_clk_clk.data_div.data,
		.parent =						 {
			CLK_AM6_K3_PLL_CTRL_WRAP_WKUP_0_BUS_SYSCLKOUT_CLK,
			1,
		},
	},
	[CLK_AM6_MX_WAKEUP_GS80_WKUP_0_BUS_PRG_MCU_EFUSE_CLK] =		 {
		.drv		= &clk_drv_fixed,
		.flags		= 0,
		.range_idx	= AM6_FREQ_RANGE_MX_WAKEUP_GS80_WKUP_0_BUS_PRG_MCU_EFUSE_CLK,
	},
};

struct clk soc_clocks[254];
const size_t soc_clock_count = ARRAY_SIZE(soc_clock_data);
