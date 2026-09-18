/*
 *  Copyright (C) 2025 Texas Instruments Incorporated
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

/**
 *  \file  board_ecu.c
 *
 *  \brief Auto generated file from DM Power Configuration Tool, used to configure
 *         PLL and PSC based on use case.
 *
 */
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/sciclient/sciclient.h>
#include "board_ecu.h"
#include "board_pll.h"
#include "board_clock.h"
#include "board_utils.h"

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static uint32_t gBoardEcuClkModuleMcuIDInit_powerConfig0[] ={
    /* Fixed config - None */
    /* GUI configured */

};

static uint32_t gBoardEcuClkModuleMcuIDInit_powerConfig1[] ={
    /* Fixed config - None */
    /* GUI configured */

};

static uint32_t gBoardEcuClkModuleMcuIDInit_powerConfig2[] ={
    /* Fixed config - None */
    /* GUI configured */

};

static uint32_t gBoardEcuClkModuleMainIDInit_powerConfig0[] = {
    /* Fixed config - None */
    /* GUI configured */
    TISCI_DEV_VPAC0,
    TISCI_DEV_VPAC1,
    TISCI_DEV_CODEC0,
    TISCI_DEV_EMIF_DATA_0_VD,
    TISCI_DEV_DDR0,
    TISCI_DEV_EMIF_DATA_1_VD,
    TISCI_DEV_DDR1,
    TISCI_DEV_EMIF_DATA_2_VD,
    TISCI_DEV_DDR2,
    TISCI_DEV_EMIF_DATA_3_VD,
    TISCI_DEV_DDR3,
};

static uint32_t gBoardEcuClkModuleMainIDInit_powerConfig1[] = {
    /* Fixed config - None */
    /* GUI configured */
    TISCI_DEV_VPAC0,
    TISCI_DEV_VPAC1,
    TISCI_DEV_DMPAC0,
    TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPU_SS_0,
    TISCI_DEV_EMIF_DATA_0_VD,
    TISCI_DEV_DDR0,
    TISCI_DEV_EMIF_DATA_1_VD,
    TISCI_DEV_DDR1,
    TISCI_DEV_EMIF_DATA_2_VD,
    TISCI_DEV_DDR2,
    TISCI_DEV_EMIF_DATA_3_VD,
    TISCI_DEV_DDR3,
};

static uint32_t gBoardEcuClkModuleMainIDInit_powerConfig2[] = {
    /* Fixed config - None */
    /* GUI configured */
    TISCI_DEV_VPAC0,
    TISCI_DEV_VPAC1,
    TISCI_DEV_DMPAC0,
    TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPU_SS_0,
    TISCI_DEV_EMIF_DATA_0_VD,
    TISCI_DEV_DDR0,
    TISCI_DEV_EMIF_DATA_1_VD,
    TISCI_DEV_DDR1,
    TISCI_DEV_EMIF_DATA_2_VD,
    TISCI_DEV_DDR2,
    TISCI_DEV_EMIF_DATA_3_VD,
    TISCI_DEV_DDR3,
};

static uint32_t gBoardEcuClkModuleMcuIDDisable_powerConfig0[] ={
    /* GUI configured disable PSCs */

    TISCI_DEV_WKUP_GPIO0,
    TISCI_DEV_WKUP_GPIO1,
    TISCI_DEV_WKUP_I2C0,
    TISCI_DEV_MCU_CPSW0,
    TISCI_DEV_MCU_I2C0,
    TISCI_DEV_MCU_I2C1,
    TISCI_DEV_MCU_I3C0,
    TISCI_DEV_MCU_I3C1,
    TISCI_DEV_MCU_MCSPI0,
    TISCI_DEV_MCU_MCSPI1,
    TISCI_DEV_MCU_MCSPI2,
    TISCI_DEV_MCU_MCAN0,
    TISCI_DEV_MCU_MCAN1,
    TISCI_DEV_MCU_ADC12FC_16FFC0,
    TISCI_DEV_MCU_ADC12FC_16FFC1,
    TISCI_DEV_MCU_FSS0_HYPERBUS1P0_0,
    TISCI_DEV_MCU_FSS0_OSPI_0,
    TISCI_DEV_MCU_FSS0_OSPI_1,
};

static uint32_t gBoardEcuClkModuleMcuIDDisable_powerConfig1[] ={
    /* GUI configured disable PSCs */

    TISCI_DEV_WKUP_GPIO0,
    TISCI_DEV_WKUP_GPIO1,
    TISCI_DEV_MCU_CPSW0,
    TISCI_DEV_MCU_I3C0,
    TISCI_DEV_MCU_I3C1,
    TISCI_DEV_MCU_MCSPI0,
    TISCI_DEV_MCU_MCSPI1,
    TISCI_DEV_MCU_MCSPI2,
    TISCI_DEV_MCU_MCAN0,
    TISCI_DEV_MCU_MCAN1,
    TISCI_DEV_MCU_ADC12FC_16FFC0,
    TISCI_DEV_MCU_ADC12FC_16FFC1,
    TISCI_DEV_MCU_FSS0_HYPERBUS1P0_0,
    TISCI_DEV_MCU_FSS0_OSPI_0,
    TISCI_DEV_MCU_FSS0_OSPI_1,
};

static uint32_t gBoardEcuClkModuleMcuIDDisable_powerConfig2[] ={
    /* GUI configured disable PSCs */

    TISCI_DEV_WKUP_GPIO0,
    TISCI_DEV_WKUP_GPIO1,
    TISCI_DEV_MCU_CPSW0,
    TISCI_DEV_MCU_I3C0,
    TISCI_DEV_MCU_I3C1,
    TISCI_DEV_MCU_MCSPI0,
    TISCI_DEV_MCU_MCSPI1,
    TISCI_DEV_MCU_MCSPI2,
    TISCI_DEV_MCU_MCAN0,
    TISCI_DEV_MCU_MCAN1,
    TISCI_DEV_MCU_ADC12FC_16FFC0,
    TISCI_DEV_MCU_ADC12FC_16FFC1,
    TISCI_DEV_MCU_FSS0_HYPERBUS1P0_0,
    TISCI_DEV_MCU_FSS0_OSPI_0,
    TISCI_DEV_MCU_FSS0_OSPI_1,
};

static uint32_t gBoardEcuClkModuleMainIDDisable_powerConfig0[] = {
    /* GUI configured disable PSCs */
    TISCI_DEV_GPIO0,
    TISCI_DEV_GPIO2,
    TISCI_DEV_GPIO4,
    TISCI_DEV_GPIO6,
    TISCI_DEV_I2C0,
    TISCI_DEV_I2C1,
    TISCI_DEV_I2C2,
    TISCI_DEV_I2C3,
    TISCI_DEV_I2C4,
    TISCI_DEV_I2C5,
    TISCI_DEV_I2C6,
    TISCI_DEV_UART0,
    TISCI_DEV_UART1,
    TISCI_DEV_UART2,
    TISCI_DEV_UART3,
    TISCI_DEV_UART4,
    TISCI_DEV_UART5,
    TISCI_DEV_UART6,
    TISCI_DEV_UART7,
    TISCI_DEV_CPSW1,
    TISCI_DEV_MCSPI0,
    TISCI_DEV_MCSPI1,
    TISCI_DEV_MCSPI2,
    TISCI_DEV_MCSPI3,
    TISCI_DEV_MCSPI4,
    TISCI_DEV_MCSPI5,
    TISCI_DEV_MCSPI6,
    TISCI_DEV_MCSPI7,
    TISCI_DEV_MCASP0,
    TISCI_DEV_MCASP1,
    TISCI_DEV_MCASP2,
    TISCI_DEV_MCASP3,
    TISCI_DEV_MCASP4,
    TISCI_DEV_ECAP0,
    TISCI_DEV_ECAP1,
    TISCI_DEV_ECAP2,
    TISCI_DEV_EPWM0,
    TISCI_DEV_EPWM1,
    TISCI_DEV_EPWM2,
    TISCI_DEV_EPWM3,
    TISCI_DEV_EPWM4,
    TISCI_DEV_EPWM5,
    TISCI_DEV_EQEP0,
    TISCI_DEV_EQEP1,
    TISCI_DEV_EQEP2,
    TISCI_DEV_MCAN0,
    TISCI_DEV_MCAN1,
    TISCI_DEV_MCAN10,
    TISCI_DEV_MCAN11,
    TISCI_DEV_MCAN12,
    TISCI_DEV_MCAN13,
    TISCI_DEV_MCAN14,
    TISCI_DEV_MCAN15,
    TISCI_DEV_MCAN16,
    TISCI_DEV_MCAN17,
    TISCI_DEV_MCAN2,
    TISCI_DEV_MCAN3,
    TISCI_DEV_MCAN4,
    TISCI_DEV_MCAN5,
    TISCI_DEV_MCAN6,
    TISCI_DEV_MCAN7,
    TISCI_DEV_MCAN8,
    TISCI_DEV_MCAN9,
    TISCI_DEV_MMCSD0,
    TISCI_DEV_DSS0,
    TISCI_DEV_USB0,
};

static uint32_t gBoardEcuClkModuleMainIDDisable_powerConfig1[] = {
    /* GUI configured disable PSCs */
    TISCI_DEV_UART0,
    TISCI_DEV_UART1,
    TISCI_DEV_UART2,
    TISCI_DEV_UART3,
    TISCI_DEV_UART4,
    TISCI_DEV_UART5,
    TISCI_DEV_UART6,
    TISCI_DEV_UART7,
    TISCI_DEV_UART9,
    TISCI_DEV_CPSW1,
    TISCI_DEV_MCASP0,
    TISCI_DEV_MCASP1,
    TISCI_DEV_MCASP2,
    TISCI_DEV_MCASP3,
    TISCI_DEV_MCASP4,
    TISCI_DEV_ECAP0,
    TISCI_DEV_ECAP1,
    TISCI_DEV_ECAP2,
    TISCI_DEV_EPWM0,
    TISCI_DEV_EPWM1,
    TISCI_DEV_EPWM2,
    TISCI_DEV_EPWM3,
    TISCI_DEV_EPWM4,
    TISCI_DEV_EPWM5,
    TISCI_DEV_EQEP0,
    TISCI_DEV_EQEP1,
    TISCI_DEV_EQEP2,
    TISCI_DEV_MCAN0,
    TISCI_DEV_MCAN1,
    TISCI_DEV_MCAN10,
    TISCI_DEV_MCAN11,
    TISCI_DEV_MCAN12,
    TISCI_DEV_MCAN13,
    TISCI_DEV_MCAN14,
    TISCI_DEV_MCAN15,
    TISCI_DEV_MCAN16,
    TISCI_DEV_MCAN17,
    TISCI_DEV_MCAN2,
    TISCI_DEV_MCAN3,
    TISCI_DEV_MCAN4,
    TISCI_DEV_MCAN5,
    TISCI_DEV_MCAN6,
    TISCI_DEV_MCAN7,
    TISCI_DEV_MCAN8,
    TISCI_DEV_MCAN9,
    TISCI_DEV_MMCSD0,
    TISCI_DEV_DSS0,
    TISCI_DEV_USB0,
};

static uint32_t gBoardEcuClkModuleMainIDDisable_powerConfig2[] = {
    /* GUI configured disable PSCs */
    TISCI_DEV_GPIO0,
    TISCI_DEV_GPIO2,
    TISCI_DEV_GPIO4,
    TISCI_DEV_GPIO6,
    TISCI_DEV_UART0,
    TISCI_DEV_UART1,
    TISCI_DEV_UART2,
    TISCI_DEV_UART3,
    TISCI_DEV_UART4,
    TISCI_DEV_UART5,
    TISCI_DEV_UART6,
    TISCI_DEV_UART7,
    TISCI_DEV_CPSW1,
    TISCI_DEV_MCSPI0,
    TISCI_DEV_MCSPI1,
    TISCI_DEV_MCSPI2,
    TISCI_DEV_MCSPI3,
    TISCI_DEV_MCSPI4,
    TISCI_DEV_MCSPI5,
    TISCI_DEV_MCSPI6,
    TISCI_DEV_MCSPI7,
    TISCI_DEV_MCASP0,
    TISCI_DEV_MCASP1,
    TISCI_DEV_MCASP2,
    TISCI_DEV_MCASP3,
    TISCI_DEV_MCASP4,
    TISCI_DEV_ECAP0,
    TISCI_DEV_ECAP1,
    TISCI_DEV_ECAP2,
    TISCI_DEV_EPWM0,
    TISCI_DEV_EPWM1,
    TISCI_DEV_EPWM2,
    TISCI_DEV_EPWM3,
    TISCI_DEV_EPWM4,
    TISCI_DEV_EPWM5,
    TISCI_DEV_EQEP0,
    TISCI_DEV_EQEP1,
    TISCI_DEV_EQEP2,
    TISCI_DEV_MCAN0,
    TISCI_DEV_MCAN1,
    TISCI_DEV_MCAN10,
    TISCI_DEV_MCAN11,
    TISCI_DEV_MCAN12,
    TISCI_DEV_MCAN13,
    TISCI_DEV_MCAN14,
    TISCI_DEV_MCAN15,
    TISCI_DEV_MCAN16,
    TISCI_DEV_MCAN17,
    TISCI_DEV_MCAN2,
    TISCI_DEV_MCAN3,
    TISCI_DEV_MCAN4,
    TISCI_DEV_MCAN5,
    TISCI_DEV_MCAN6,
    TISCI_DEV_MCAN7,
    TISCI_DEV_MCAN8,
    TISCI_DEV_MCAN9,
    TISCI_DEV_MMCSD1,
    TISCI_DEV_DSS0,
    TISCI_DEV_USB0,
};

static Board_PllClkCfg_t gBoardEcuPllClkCfgMcu_powerConfig0[] =
{   /* Fixed config */
    {
      TISCI_DEV_MCU_CPSW0,
      TISCI_DEV_MCU_CPSW0_CPTS_RFT_CLK_PARENT_HSDIV4_16FFT_MCU_2_HSDIVOUT1_CLK,
      200000000
    },
   /* GUI configured */
};

static Board_PllClkCfg_t gBoardEcuPllClkCfgMcu_powerConfig1[] =
{   /* Fixed config */
    {
      TISCI_DEV_MCU_CPSW0,
      TISCI_DEV_MCU_CPSW0_CPTS_RFT_CLK_PARENT_HSDIV4_16FFT_MCU_2_HSDIVOUT1_CLK,
      200000000
    },
   /* GUI configured */
};

static Board_PllClkCfg_t gBoardEcuPllClkCfgMcu_powerConfig2[] =
{   /* Fixed config */
    {
      TISCI_DEV_MCU_CPSW0,
      TISCI_DEV_MCU_CPSW0_CPTS_RFT_CLK_PARENT_HSDIV4_16FFT_MCU_2_HSDIVOUT1_CLK,
      200000000
    },
   /* GUI configured */
};

static Board_PllClkCfg_t gBoardEcuPllClkCfgMain_powerConfig0[] =
{   /* Fixed config */
    {
      TISCI_DEV_GTC0,
      TISCI_DEV_GTC0_GTC_CLK_PARENT_POSTDIV3_16FFT_MAIN_0_HSDIVOUT6_CLK,
      200000000
    },
    /* GUI configured */
    {
      TISCI_DEV_A72SS0_CORE0,
      TISCI_DEV_A72SS0_ARM0_CLK_CLK,
      1000000000
    },
    {
      TISCI_DEV_A72SS1_CORE0,
      TISCI_DEV_A72SS1_ARM1_CLK_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS0_CORE0,
      TISCI_DEV_R5FSS0_CORE0_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS0_CORE1,
      TISCI_DEV_R5FSS0_CORE1_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS1_CORE0,
      TISCI_DEV_R5FSS1_CORE0_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_R5FSS1_CORE1,
      TISCI_DEV_R5FSS1_CORE1_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_R5FSS2_CORE0,
      TISCI_DEV_R5FSS2_CORE0_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_R5FSS2_CORE1,
      TISCI_DEV_R5FSS2_CORE1_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS0,
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS0_CORE0_C7X_CLK,
      500000000
    },
    {
      TISCI_DEV_VPAC0,
      TISCI_DEV_VPAC0_MAIN_CLK,
      720000000
    },
    {
      TISCI_DEV_VPAC1,
      TISCI_DEV_VPAC1_MAIN_CLK,
      720000000
    },
    {
      TISCI_DEV_CODEC0,
      TISCI_DEV_CODEC0_VPU_ACLK_CLK,
      600000000
    },
};

static Board_PllClkCfg_t gBoardEcuPllClkCfgMain_powerConfig1[] =
{   /* Fixed config */
    {
      TISCI_DEV_GTC0,
      TISCI_DEV_GTC0_GTC_CLK_PARENT_POSTDIV3_16FFT_MAIN_0_HSDIVOUT6_CLK,
      200000000
    },
    /* GUI configured */
    {
      TISCI_DEV_A72SS0_CORE0,
      TISCI_DEV_A72SS0_ARM0_CLK_CLK,
      500000000
    },
    {
      TISCI_DEV_A72SS1_CORE0,
      TISCI_DEV_A72SS1_ARM1_CLK_CLK,
      500000000
    },
    {
      TISCI_DEV_R5FSS0_CORE0,
      TISCI_DEV_R5FSS0_CORE0_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS0_CORE1,
      TISCI_DEV_R5FSS0_CORE1_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS1_CORE0,
      TISCI_DEV_R5FSS1_CORE0_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS1_CORE1,
      TISCI_DEV_R5FSS1_CORE1_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS2_CORE0,
      TISCI_DEV_R5FSS2_CORE0_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS2_CORE1,
      TISCI_DEV_R5FSS2_CORE1_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS0,
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS0_CORE0_C7X_CLK,
      500000000
    },
    {
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS1,
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS1_CORE0_C7X_CLK,
      500000000
    },
    {
      TISCI_DEV_VPAC0,
      TISCI_DEV_VPAC0_MAIN_CLK,
      720000000
    },
    {
      TISCI_DEV_VPAC1,
      TISCI_DEV_VPAC1_MAIN_CLK,
      720000000
    },
    {
      TISCI_DEV_DMPAC0,
      TISCI_DEV_DMPAC0_CLK,
      480000000
    },
    {
      TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPU_SS_0,
      TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPU_SS_0_GPU_PLL_CLK,
      800000000
    },
};

static Board_PllClkCfg_t gBoardEcuPllClkCfgMain_powerConfig2[] =
{   /* Fixed config */
    {
      TISCI_DEV_GTC0,
      TISCI_DEV_GTC0_GTC_CLK_PARENT_POSTDIV3_16FFT_MAIN_0_HSDIVOUT6_CLK,
      200000000
    },
    /* GUI configured */
    {
      TISCI_DEV_A72SS0_CORE0,
      TISCI_DEV_A72SS0_ARM0_CLK_CLK,
      500000000
    },
    {
      TISCI_DEV_A72SS1_CORE0,
      TISCI_DEV_A72SS1_ARM1_CLK_CLK,
      500000000
    },
    {
      TISCI_DEV_R5FSS0_CORE0,
      TISCI_DEV_R5FSS0_CORE0_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS0_CORE1,
      TISCI_DEV_R5FSS0_CORE1_CPU_CLK,
      1000000000
    },
    {
      TISCI_DEV_R5FSS1_CORE0,
      TISCI_DEV_R5FSS1_CORE0_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_R5FSS1_CORE1,
      TISCI_DEV_R5FSS1_CORE1_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_R5FSS2_CORE0,
      TISCI_DEV_R5FSS2_CORE0_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_R5FSS2_CORE1,
      TISCI_DEV_R5FSS2_CORE1_CPU_CLK,
      100000000
    },
    {
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS0,
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS0_CORE0_C7X_CLK,
      1000000000
    },
    {
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS1,
      TISCI_DEV_COMPUTE_CLUSTER0_C71SS1_CORE0_C7X_CLK,
      1000000000
    },
    {
      TISCI_DEV_VPAC0,
      TISCI_DEV_VPAC0_MAIN_CLK,
      600000000
    },
    {
      TISCI_DEV_VPAC1,
      TISCI_DEV_VPAC1_MAIN_CLK,
      600000000
    },
    {
      TISCI_DEV_DMPAC0,
      TISCI_DEV_DMPAC0_CLK,
      240000000
    },
    {
      TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPU_SS_0,
      TISCI_DEV_J7AEP_GPU_BXS464_WRAP0_GPU_SS_0_GPU_PLL_CLK,
      800000000
    },
};

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                        Function Declarations                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                       Internal Function Declarations                       */
/* ========================================================================== */

static Board_STATUS Board_ecuInitPll(uint32_t cfg);
static Board_STATUS Board_ecuInitClock(uint32_t cfg);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

Board_STATUS Board_ecuInit(uint32_t cfg)
{
    Board_STATUS status = BOARD_SOK;

    status = Board_ecuInitPll(cfg);
    if(status == BOARD_SOK)
    {
        status = Board_ecuInitClock(cfg);
    }
    if(status == BOARD_SOK)
    {
        status = Board_ecuDisableClock(cfg);
    }

    return status;
}

Board_STATUS Board_ecuDisableClock(uint32_t cfg)
{
    uint32_t *pMcuDisableArray = NULL;
    uint32_t *pMainDisableArray = NULL;
    uint32_t mcuDisableArraySize = 0;
    uint32_t mainDisableArraySize = 0;
    Board_STATUS status = BOARD_SOK;
    uint32_t index = 0;

    /* Select array to use based on config */
    switch (cfg)
    {
        case BOARD_ECU_NONE:
        {
            /* For ECU_NONE, no clocks to disable */
        }
        break;

        case BOARD_ECU_FC:
        {
            pMcuDisableArray = gBoardEcuClkModuleMcuIDDisable_powerConfig0;
            mcuDisableArraySize = sizeof(gBoardEcuClkModuleMcuIDDisable_powerConfig0) / sizeof(uint32_t);

            pMainDisableArray = gBoardEcuClkModuleMainIDDisable_powerConfig0;
            mainDisableArraySize = sizeof(gBoardEcuClkModuleMainIDDisable_powerConfig0) / sizeof(uint32_t);
        }
        break;

        case BOARD_ECU_SRV:
        {
            pMcuDisableArray = gBoardEcuClkModuleMcuIDDisable_powerConfig1;
            mcuDisableArraySize = sizeof(gBoardEcuClkModuleMcuIDDisable_powerConfig1) / sizeof(uint32_t);

            pMainDisableArray = gBoardEcuClkModuleMainIDDisable_powerConfig1;
            mainDisableArraySize = sizeof(gBoardEcuClkModuleMainIDDisable_powerConfig1) / sizeof(uint32_t);
        }
        break;

        case BOARD_ECU_AVP4:
        {
            pMcuDisableArray = gBoardEcuClkModuleMcuIDDisable_powerConfig2;
            mcuDisableArraySize = sizeof(gBoardEcuClkModuleMcuIDDisable_powerConfig2) / sizeof(uint32_t);

            pMainDisableArray = gBoardEcuClkModuleMainIDDisable_powerConfig2;
            mainDisableArraySize = sizeof(gBoardEcuClkModuleMainIDDisable_powerConfig2) / sizeof(uint32_t);
        }
        break;

        default:
        {
            status = BOARD_INVALID_PARAM;
        }
        break;
    }

    /* Disable MCU clocks */
    if (pMcuDisableArray != NULL)
     {
        for (index = 0; index < mcuDisableArraySize && status == BOARD_SOK; index++)
        {
            status = Board_moduleClockDisable(pMcuDisableArray[index]);
        }
    }

    /* Disable Main clocks */
    if (pMainDisableArray != NULL)
    {
        for (index = 0; index < mainDisableArraySize && status == BOARD_SOK; index++)
        {
            status = Board_moduleClockDisable(pMainDisableArray[index]);
        }
    }

    return status;
}

/* ========================================================================== */
/*                       Internal Function Definitions                        */
/* ========================================================================== */

static Board_STATUS Board_ecuInitPll(uint32_t cfg)
{
    Board_PllClkCfg_t *pMcuArray = NULL;
    Board_PllClkCfg_t *pMainArray = NULL;
    uint32_t mcuArraySize = 0;
    uint32_t mainArraySize = 0;
    Board_STATUS status = BOARD_SOK;
    uint32_t index = 0;

   /* Select array to use based on config */
    switch(cfg)
    {
        case BOARD_ECU_NONE:
        {
          /* For ECU_NONE use default PLL init */
            status= Board_PLLInitMcu();

            if(status == BOARD_SOK)
            {
                status = Board_PLLInitMain();
            }
        }
        break;

        case BOARD_ECU_FC:
        {
            pMcuArray = gBoardEcuPllClkCfgMcu_powerConfig0;
            mcuArraySize = sizeof(gBoardEcuPllClkCfgMcu_powerConfig0) / sizeof(Board_PllClkCfg_t);

            pMainArray = gBoardEcuPllClkCfgMain_powerConfig0;
            mainArraySize = sizeof(gBoardEcuPllClkCfgMain_powerConfig0) / sizeof(Board_PllClkCfg_t);
        }
        break;

        case BOARD_ECU_SRV:
        {
            pMcuArray = gBoardEcuPllClkCfgMcu_powerConfig1;
            mcuArraySize = sizeof(gBoardEcuPllClkCfgMcu_powerConfig1) / sizeof(Board_PllClkCfg_t);

            pMainArray = gBoardEcuPllClkCfgMain_powerConfig1;
            mainArraySize = sizeof(gBoardEcuPllClkCfgMain_powerConfig1) / sizeof(Board_PllClkCfg_t);
        }
        break;

        case BOARD_ECU_AVP4:
        {
            pMcuArray = gBoardEcuPllClkCfgMcu_powerConfig2;
            mcuArraySize = sizeof(gBoardEcuPllClkCfgMcu_powerConfig2) / sizeof(Board_PllClkCfg_t);

            pMainArray = gBoardEcuPllClkCfgMain_powerConfig2;
            mainArraySize = sizeof(gBoardEcuPllClkCfgMain_powerConfig2) / sizeof(Board_PllClkCfg_t);
        }
        break;

        default:
        {
            status = BOARD_INVALID_PARAM;
        }
        break;
    }

    /* Set MCU PLLs */
    if (pMcuArray != NULL)
    {
        for (index = 0; index < mcuArraySize && status == BOARD_SOK; index++)
        {
            status = Board_PLLInit(pMcuArray[index].tisciDevID,
                                   pMcuArray[index].tisciClkID,
                                   pMcuArray[index].clkRate);
        }
    }

    /* Set Main PLLs */
    if (pMainArray != NULL)
    {
        for (index = 0; index < mainArraySize && status == BOARD_SOK; index++)
        {
            status = Board_PLLInit(pMainArray[index].tisciDevID,
                                   pMainArray[index].tisciClkID,
                                   pMainArray[index].clkRate);
        }
    }

    return status;
}

static Board_STATUS Board_ecuInitClock(uint32_t cfg)
{
    uint32_t *pMcuArray = NULL;
    uint32_t *pMainArray = NULL;
    uint32_t mcuArraySize = 0;
    uint32_t mainArraySize = 0;
    Board_STATUS status = BOARD_SOK;
    uint32_t index = 0;

    /* Select array to use based on config */
    switch (cfg)
    {
        case BOARD_ECU_NONE:
        {
          /* For ECU_NONE, set main and mcu clock groups to all groups */
            Board_initParams_t initParams;
            Board_getInitParams(&initParams);
            initParams.mainClkGrp = BOARD_MAIN_CLOCK_GROUP_ALL;
            initParams.mcuClkGrp  = BOARD_MCU_CLOCK_GROUP_ALL;
            Board_setInitParams(&initParams);

            Board_init(BOARD_INIT_MODULE_CLOCK);
        }
        break;

        case BOARD_ECU_FC:
        {
            pMcuArray = gBoardEcuClkModuleMcuIDInit_powerConfig0;
            mcuArraySize = sizeof(gBoardEcuClkModuleMcuIDInit_powerConfig0) / sizeof(uint32_t);

            pMainArray = gBoardEcuClkModuleMainIDInit_powerConfig0;
            mainArraySize = sizeof(gBoardEcuClkModuleMainIDInit_powerConfig0) / sizeof(uint32_t);
        }
        break;

        case BOARD_ECU_SRV:
        {
            pMcuArray = gBoardEcuClkModuleMcuIDInit_powerConfig1;
            mcuArraySize = sizeof(gBoardEcuClkModuleMcuIDInit_powerConfig1) / sizeof(uint32_t);

            pMainArray = gBoardEcuClkModuleMainIDInit_powerConfig1;
            mainArraySize = sizeof(gBoardEcuClkModuleMainIDInit_powerConfig1) / sizeof(uint32_t);
        }
        break;

        case BOARD_ECU_AVP4:
        {
            pMcuArray = gBoardEcuClkModuleMcuIDInit_powerConfig2;
            mcuArraySize = sizeof(gBoardEcuClkModuleMcuIDInit_powerConfig2) / sizeof(uint32_t);

            pMainArray = gBoardEcuClkModuleMainIDInit_powerConfig2;
            mainArraySize = sizeof(gBoardEcuClkModuleMainIDInit_powerConfig2) / sizeof(uint32_t);
        }
        break;

        default:
        {
            status = BOARD_INVALID_PARAM;
        }
        break;
    }

    /* Set MCU clocks */
    if (pMcuArray != NULL)
    {
        for (index = 0; index < mcuArraySize && status == BOARD_SOK; index++)
        {
            status = Board_moduleClockEnable(pMcuArray[index]);
        }
    }

    /* Set Main clocks */
    if (pMainArray != NULL)
    {
        for (index = 0; index < mainArraySize && status == BOARD_SOK; index++)
        {
            status = Board_moduleClockEnable(pMainArray[index]);
        }
    }

    return status;
}

/**
 * @brief  Get the CPU clock rate for a given device ID and ECU configuration.
 *
 * This function searches the appropriate clock configuration array based on the ECU config
 * (FC, SRV, AVP4) and returns the clock rate for the specified TISCI device ID.
 *
 * @param tisci_dev_id  TISCI device ID for which the CPU clock rate is requested.
 * @param cfg           ECU configuration (BOARD_ECU_FC, BOARD_ECU_SRV, BOARD_ECU_AVP4).
 *
 * @return              CPU clock rate in Hz if found, BOARD_INVALID_CORE_CLK_FREQ otherwise.
 */
int64_t Board_ecuGetCpuClock(uint32_t tisci_dev_id, uint32_t cfg)
{
    uint32_t index = 0;    
    Board_STATUS status = BOARD_SOK;
    Board_PllClkCfg_t *pMainArray = NULL;
    uint32_t mainArraySize = 0;
    int64_t retVal = BOARD_INVALID_CORE_CLK_FREQ;

    /* Select the appropriate clock configuration array based on ECU config */
    switch (cfg)
    {
        
        case BOARD_ECU_FC:
        {
            pMainArray = gBoardEcuPllClkCfgMain_powerConfig0;
            mainArraySize = sizeof(gBoardEcuPllClkCfgMain_powerConfig0) / sizeof(Board_PllClkCfg_t);
        }
        break;

        case BOARD_ECU_SRV:
        {
            pMainArray = gBoardEcuPllClkCfgMain_powerConfig1;
            mainArraySize = sizeof(gBoardEcuPllClkCfgMain_powerConfig1) / sizeof(Board_PllClkCfg_t);
        }
        break;

        case BOARD_ECU_AVP4:
        {
            pMainArray = gBoardEcuPllClkCfgMain_powerConfig2;
            mainArraySize = sizeof(gBoardEcuPllClkCfgMain_powerConfig2) / sizeof(Board_PllClkCfg_t);
        }
        break;

        default:
        {
            status = BOARD_INVALID_PARAM;
        }
        break;
    }

    /* Search for the device ID in the selected array and return its clock rate */
    if (pMainArray != NULL)
    {
        for (index = 0; index < mainArraySize && status == BOARD_SOK; index++)
        {
            if (tisci_dev_id == pMainArray[index].tisciDevID)
            {
                retVal = pMainArray[index].clkRate;
                break;
            }
        }
    }

    return retVal;
}
