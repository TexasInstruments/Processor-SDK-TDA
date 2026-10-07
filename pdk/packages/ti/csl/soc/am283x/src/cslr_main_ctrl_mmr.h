/********************************************************************
 * Copyright (C) 2026 Texas Instruments Incorporated.
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
 *  Name        : cslr_main_ctrl_mmr.h
 *  VPVERSION   : 3.0.405 - 2026.02.24.09.53.55
 *  VPREV       : 2.23.4
*/
#ifndef CSLR_MAIN_CTRL_MMR_H_
#define CSLR_MAIN_CTRL_MMR_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <ti/csl/cslr.h>
#include <stdint.h>

/**************************************************************************
* Module Base Offset Values
**************************************************************************/

#define CSL_MAIN_CTRL_MMR_CFG0_REGS_BASE                                       (0x00100000U)


/**************************************************************************
* Hardware Region  : MMRs in region 0
**************************************************************************/


/**************************************************************************
* Register Overlay Structure
**************************************************************************/

typedef struct {
    volatile uint32_t PID;                       /* PID register */
    volatile uint8_t  Resv_8[4];
    volatile uint32_t MMR_CFG1;                  /* Configuration register 1 */
    volatile uint8_t  Resv_256[244];
    volatile uint32_t IPC_SET0;                  /* IPC Generation Register 0 */
    volatile uint8_t  Resv_320[60];
    volatile uint32_t IPC_SET16;                 /* IPC Generation Register 16 */
    volatile uint32_t IPC_SET17;                 /* IPC Generation Register 17 */
    volatile uint32_t IPC_SET18;                 /* IPC Generation Register 18 */
    volatile uint32_t IPC_SET19;                 /* IPC Generation Register 19 */
    volatile uint8_t  Resv_384[48];
    volatile uint32_t IPC_CLR0;                  /* IPC Acknowledge Register0 */
    volatile uint8_t  Resv_448[60];
    volatile uint32_t IPC_CLR16;                 /* IPC Acknowledge Register 16 */
    volatile uint32_t IPC_CLR17;                 /* IPC Acknowledge Register 17 */
    volatile uint32_t IPC_CLR18;                 /* IPC Acknowledge Register 18 */
    volatile uint32_t IPC_CLR19;                 /* IPC Acknowledge Register 19 */
    volatile uint8_t  Resv_4104[3640];
    volatile uint32_t LOCK0_KICK0;               /*  - KICK0 component */
    volatile uint32_t LOCK0_KICK1;               /*  - KICK1 component */
    volatile uint32_t INTR_RAW_STATUS;           /* Interrupt Raw Status/Set Register */
    volatile uint32_t INTR_ENABLED_STATUS_CLEAR;   /* Interrupt Enabled Status/Clear register */
    volatile uint32_t INTR_ENABLE;               /* Interrupt Enable register */
    volatile uint32_t INTR_ENABLE_CLEAR;         /* Interrupt Enable Clear register */
    volatile uint32_t EOI;                       /* EOI register */
    volatile uint32_t FAULT_ADDRESS;             /* Fault Address register */
    volatile uint32_t FAULT_TYPE_STATUS;         /* Fault Type Status register */
    volatile uint32_t FAULT_ATTR_STATUS;         /* Fault Attribute Status register */
    volatile uint32_t FAULT_CLEAR;               /* Fault Clear register */
    volatile uint8_t  Resv_4352[204];
    volatile uint32_t CLAIMREG_P0_R0_READONLY;   /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R1_READONLY;   /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R2_READONLY;   /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R3_READONLY;   /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R4_READONLY;   /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R5_READONLY;   /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R6_READONLY;   /* Claim bits for Partition 0 */
    volatile uint8_t  Resv_8192[3812];
    volatile uint32_t PID_PROXY;                 /* PID register */
    volatile uint8_t  Resv_8200[4];
    volatile uint32_t MMR_CFG1_PROXY;            /* Configuration register 1 */
    volatile uint8_t  Resv_8448[244];
    volatile uint32_t IPC_SET0_PROXY;            /* IPC Generation Register 0 */
    volatile uint8_t  Resv_8512[60];
    volatile uint32_t IPC_SET16_PROXY;           /* IPC Generation Register 16 */
    volatile uint32_t IPC_SET17_PROXY;           /* IPC Generation Register 17 */
    volatile uint32_t IPC_SET18_PROXY;           /* IPC Generation Register 18 */
    volatile uint32_t IPC_SET19_PROXY;           /* IPC Generation Register 19 */
    volatile uint8_t  Resv_8576[48];
    volatile uint32_t IPC_CLR0_PROXY;            /* IPC Acknowledge Register0 */
    volatile uint8_t  Resv_8640[60];
    volatile uint32_t IPC_CLR16_PROXY;           /* IPC Acknowledge Register 16 */
    volatile uint32_t IPC_CLR17_PROXY;           /* IPC Acknowledge Register 17 */
    volatile uint32_t IPC_CLR18_PROXY;           /* IPC Acknowledge Register 18 */
    volatile uint32_t IPC_CLR19_PROXY;           /* IPC Acknowledge Register 19 */
    volatile uint8_t  Resv_12296[3640];
    volatile uint32_t LOCK0_KICK0_PROXY;         /*  - KICK0 component */
    volatile uint32_t LOCK0_KICK1_PROXY;         /*  - KICK1 component */
    volatile uint32_t INTR_RAW_STATUS_PROXY;     /* Interrupt Raw Status/Set Register */
    volatile uint32_t INTR_ENABLED_STATUS_CLEAR_PROXY;   /* Interrupt Enabled Status/Clear register */
    volatile uint32_t INTR_ENABLE_PROXY;         /* Interrupt Enable register */
    volatile uint32_t INTR_ENABLE_CLEAR_PROXY;   /* Interrupt Enable Clear register */
    volatile uint32_t EOI_PROXY;                 /* EOI register */
    volatile uint32_t FAULT_ADDRESS_PROXY;       /* Fault Address register */
    volatile uint32_t FAULT_TYPE_STATUS_PROXY;   /* Fault Type Status register */
    volatile uint32_t FAULT_ATTR_STATUS_PROXY;   /* Fault Attribute Status register */
    volatile uint32_t FAULT_CLEAR_PROXY;         /* Fault Clear register */
    volatile uint8_t  Resv_12544[204];
    volatile uint32_t CLAIMREG_P0_R0;            /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R1;            /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R2;            /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R3;            /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R4;            /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R5;            /* Claim bits for Partition 0 */
    volatile uint32_t CLAIMREG_P0_R6;            /* Claim bits for Partition 0 */
    volatile uint8_t  Resv_16452[3880];
    volatile uint32_t ENET1_CTRL;                /* Ethernet1 Control Register */
    volatile uint32_t ENET2_CTRL;                /* Ethernet2 Control Register */
    volatile uint8_t  Resv_16688[228];
    volatile uint32_t EPWM_TB_CLKEN;             /* EPWM Time Base Clock Enable Register */
    volatile uint8_t  Resv_16704[12];
    volatile uint32_t EPWM0_CTRL;                /* PWM0 Control Register */
    volatile uint32_t EPWM1_CTRL;                /* PWM1 Control Register */
    volatile uint32_t EPWM2_CTRL;                /* PWM2 Control Register */
    volatile uint8_t  Resv_16896[180];
    volatile uint32_t TIMER0_CTRL;               /* TIMER0 Control Register */
    volatile uint32_t TIMER1_CTRL;               /* TIMER1 Control Register */
    volatile uint32_t TIMER2_CTRL;               /* TIMER2 Control Register */
    volatile uint32_t TIMER3_CTRL;               /* TIMER3 Control Register */
    volatile uint32_t TIMER4_CTRL;               /* TIMER4 Control Register */
    volatile uint32_t TIMER5_CTRL;               /* TIMER5 Control Register */
    volatile uint32_t TIMER6_CTRL;               /* TIMER6 Control Register */
    volatile uint32_t TIMER7_CTRL;               /* TIMER7 Control Register */
    volatile uint32_t TIMER8_CTRL;               /* TIMER8 Control Register */
    volatile uint32_t TIMER9_CTRL;               /* TIMER9 Control Register */
    volatile uint8_t  Resv_17024[88];
    volatile uint32_t TIMERIO0_CTRL;             /* TIMERIO0 Control Register */
    volatile uint32_t TIMERIO1_CTRL;             /* TIMERIO1 Control Register */
    volatile uint32_t TIMERIO2_CTRL;             /* TIMERIO2 Control Register */
    volatile uint32_t TIMERIO3_CTRL;             /* TIMERIO3 Control Register */
    volatile uint32_t TIMERIO4_CTRL;             /* TIMERIO4 Control Register */
    volatile uint32_t TIMERIO5_CTRL;             /* TIMERIO5 Control Register */
    volatile uint32_t TIMERIO6_CTRL;             /* TIMERIO6 Control Register */
    volatile uint32_t TIMERIO7_CTRL;             /* TIMERIO7 Control Register */
    volatile uint8_t  Resv_17152[96];
    volatile uint32_t C7XV_CTRL0;                /* C7XV Control Register 0 */
    volatile uint8_t  Resv_17168[12];
    volatile uint32_t DPHY_TX0_CTRL;             /* DPHY0 Lane0 Control Register */
    volatile uint8_t  Resv_17616[444];
    volatile uint32_t ADC0_CTRL;                 /* ADC0 Control Register */
    volatile uint8_t  Resv_17648[28];
    volatile uint32_t ADC0_CAL;                  /* ADC0 Calibration Register */
    volatile uint8_t  Resv_18176[524];
    volatile uint32_t FSS0_CTRL0;                /* Flash Subsystem 0 Control Register */
    volatile uint32_t FSS0_CTRL1;                /* Flash Subsystem 0 Control Register */
    volatile uint8_t  Resv_18256[72];
    volatile uint32_t DCC_STAT;                  /* DCC Status Register */
    volatile uint8_t  Resv_18272[12];
    volatile uint32_t R5SS0_CORE0_RL2_CTRL;      /* R5 Cluster 0 Core0 RL2 Cache Control Register */
    volatile uint32_t R5SS0_CORE1_RL2_CTRL;      /* R5 Cluster 0 Core0 RL2 Cache Control Register */
    volatile uint8_t  Resv_18288[8];
    volatile uint32_t R5SS1_CORE0_RL2_CTRL;      /* R5 Cluster 1 Core0 RL2 Cache Control Register */
    volatile uint32_t R5SS1_CORE1_RL2_CTRL;      /* R5 Cluster 1 Core0 RL2 Cache Control Register */
    volatile uint8_t  Resv_18948[652];
    volatile uint32_t TPCC_PARITY_STATUS;        /* TPCC Parity Status Register */
    volatile uint8_t  Resv_18960[8];
    volatile uint32_t TPTC_XID_REORDER_CFG0;     /* TPTC XID Reordering Register */
    volatile uint8_t  Resv_18976[12];
    volatile uint32_t TPTC_DBS_CFG0;             /* TPTC DBS Config Register */
    volatile uint8_t  Resv_18992[12];
    volatile uint32_t TPCC_MEMINIT_START;        /* TPTC MEMINIT Register */
    volatile uint8_t  Resv_19008[12];
    volatile uint32_t TPCC_PARITY_CTRL0;         /* TPCC Parity Control Register */
    volatile uint8_t  Resv_19024[12];
    volatile uint32_t TPTC_BOUNDARY_CFG0;        /* TPTC Boundary Size Config Register */
    volatile uint8_t  Resv_19056[28];
    volatile uint32_t EDMA_CLK_GATE;             /* EDMA Clock Gate */
    volatile uint8_t  Resv_20488[1428];
    volatile uint32_t LOCK1_KICK0;               /*  - KICK0 component */
    volatile uint32_t LOCK1_KICK1;               /*  - KICK1 component */
    volatile uint8_t  Resv_20736[240];
    volatile uint32_t CLAIMREG_P1_R0_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R1_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R2_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R3_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R4_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R5_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R6_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R7_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R8_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R9_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R10_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R11_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R12_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R13_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R14_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R15_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R16_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R17_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R18_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R19_READONLY;   /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R20_READONLY;   /* Claim bits for Partition 1 */
    volatile uint8_t  Resv_24644[3824];
    volatile uint32_t ENET1_CTRL_PROXY;          /* Ethernet1 Control Register */
    volatile uint32_t ENET2_CTRL_PROXY;          /* Ethernet2 Control Register */
    volatile uint8_t  Resv_24880[228];
    volatile uint32_t EPWM_TB_CLKEN_PROXY;       /* EPWM Time Base Clock Enable Register */
    volatile uint8_t  Resv_24896[12];
    volatile uint32_t EPWM0_CTRL_PROXY;          /* PWM0 Control Register */
    volatile uint32_t EPWM1_CTRL_PROXY;          /* PWM1 Control Register */
    volatile uint32_t EPWM2_CTRL_PROXY;          /* PWM2 Control Register */
    volatile uint8_t  Resv_25088[180];
    volatile uint32_t TIMER0_CTRL_PROXY;         /* TIMER0 Control Register */
    volatile uint32_t TIMER1_CTRL_PROXY;         /* TIMER1 Control Register */
    volatile uint32_t TIMER2_CTRL_PROXY;         /* TIMER2 Control Register */
    volatile uint32_t TIMER3_CTRL_PROXY;         /* TIMER3 Control Register */
    volatile uint32_t TIMER4_CTRL_PROXY;         /* TIMER4 Control Register */
    volatile uint32_t TIMER5_CTRL_PROXY;         /* TIMER5 Control Register */
    volatile uint32_t TIMER6_CTRL_PROXY;         /* TIMER6 Control Register */
    volatile uint32_t TIMER7_CTRL_PROXY;         /* TIMER7 Control Register */
    volatile uint32_t TIMER8_CTRL_PROXY;         /* TIMER8 Control Register */
    volatile uint32_t TIMER9_CTRL_PROXY;         /* TIMER9 Control Register */
    volatile uint8_t  Resv_25216[88];
    volatile uint32_t TIMERIO0_CTRL_PROXY;       /* TIMERIO0 Control Register */
    volatile uint32_t TIMERIO1_CTRL_PROXY;       /* TIMERIO1 Control Register */
    volatile uint32_t TIMERIO2_CTRL_PROXY;       /* TIMERIO2 Control Register */
    volatile uint32_t TIMERIO3_CTRL_PROXY;       /* TIMERIO3 Control Register */
    volatile uint32_t TIMERIO4_CTRL_PROXY;       /* TIMERIO4 Control Register */
    volatile uint32_t TIMERIO5_CTRL_PROXY;       /* TIMERIO5 Control Register */
    volatile uint32_t TIMERIO6_CTRL_PROXY;       /* TIMERIO6 Control Register */
    volatile uint32_t TIMERIO7_CTRL_PROXY;       /* TIMERIO7 Control Register */
    volatile uint8_t  Resv_25344[96];
    volatile uint32_t C7XV_CTRL0_PROXY;          /* C7XV Control Register 0 */
    volatile uint8_t  Resv_25360[12];
    volatile uint32_t DPHY_TX0_CTRL_PROXY;       /* DPHY0 Lane0 Control Register */
    volatile uint8_t  Resv_25808[444];
    volatile uint32_t ADC0_CTRL_PROXY;           /* ADC0 Control Register */
    volatile uint8_t  Resv_25840[28];
    volatile uint32_t ADC0_CAL_PROXY;            /* ADC0 Calibration Register */
    volatile uint8_t  Resv_26368[524];
    volatile uint32_t FSS0_CTRL0_PROXY;          /* Flash Subsystem 0 Control Register */
    volatile uint32_t FSS0_CTRL1_PROXY;          /* Flash Subsystem 0 Control Register */
    volatile uint8_t  Resv_26448[72];
    volatile uint32_t DCC_STAT_PROXY;            /* DCC Status Register */
    volatile uint8_t  Resv_26464[12];
    volatile uint32_t R5SS0_CORE0_RL2_CTRL_PROXY;   /* R5 Cluster 0 Core0 RL2 Cache Control Register */
    volatile uint32_t R5SS0_CORE1_RL2_CTRL_PROXY;   /* R5 Cluster 0 Core0 RL2 Cache Control Register */
    volatile uint8_t  Resv_26480[8];
    volatile uint32_t R5SS1_CORE0_RL2_CTRL_PROXY;   /* R5 Cluster 1 Core0 RL2 Cache Control Register */
    volatile uint32_t R5SS1_CORE1_RL2_CTRL_PROXY;   /* R5 Cluster 1 Core0 RL2 Cache Control Register */
    volatile uint8_t  Resv_27140[652];
    volatile uint32_t TPCC_PARITY_STATUS_PROXY;   /* TPCC Parity Status Register */
    volatile uint8_t  Resv_27152[8];
    volatile uint32_t TPTC_XID_REORDER_CFG0_PROXY;   /* TPTC XID Reordering Register */
    volatile uint8_t  Resv_27168[12];
    volatile uint32_t TPTC_DBS_CFG0_PROXY;       /* TPTC DBS Config Register */
    volatile uint8_t  Resv_27184[12];
    volatile uint32_t TPCC_MEMINIT_START_PROXY;   /* TPTC MEMINIT Register */
    volatile uint8_t  Resv_27200[12];
    volatile uint32_t TPCC_PARITY_CTRL0_PROXY;   /* TPCC Parity Control Register */
    volatile uint8_t  Resv_27216[12];
    volatile uint32_t TPTC_BOUNDARY_CFG0_PROXY;   /* TPTC Boundary Size Config Register */
    volatile uint8_t  Resv_27248[28];
    volatile uint32_t EDMA_CLK_GATE_PROXY;       /* EDMA Clock Gate */
    volatile uint8_t  Resv_28680[1428];
    volatile uint32_t LOCK1_KICK0_PROXY;         /*  - KICK0 component */
    volatile uint32_t LOCK1_KICK1_PROXY;         /*  - KICK1 component */
    volatile uint8_t  Resv_28928[240];
    volatile uint32_t CLAIMREG_P1_R0;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R1;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R2;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R3;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R4;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R5;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R6;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R7;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R8;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R9;            /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R10;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R11;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R12;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R13;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R14;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R15;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R16;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R17;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R18;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R19;           /* Claim bits for Partition 1 */
    volatile uint32_t CLAIMREG_P1_R20;           /* Claim bits for Partition 1 */
    volatile uint8_t  Resv_32768[3756];
    volatile uint32_t OBSCLK0_CTRL;              /* Observe Clock 0 Output Control Register */
    volatile uint8_t  Resv_32784[12];
    volatile uint32_t CLKOUT_CTRL;               /* CLKOUT Control Register */
    volatile uint8_t  Resv_32864[76];
    volatile uint32_t MAIN_PLL0_CLKSEL;          /* MAIN PLL0 Source Clock Select Register */
    volatile uint32_t MAIN_PLL1_CLKSEL;          /* MAIN PLL1 Source Clock Select Register */
    volatile uint32_t MAIN_PLL2_CLKSEL;          /* MAIN PLL2 Source Clock Select Register */
    volatile uint8_t  Resv_32892[16];
    volatile uint32_t MAIN_PLL7_CLKSEL;          /* MAIN PLL7 Source Clock Select Register */
    volatile uint8_t  Resv_32920[24];
    volatile uint32_t MAIN_PLL14_CLKSEL;         /* MAIN PLL14 Source Clock Select Register */
    volatile uint8_t  Resv_33088[164];
    volatile uint32_t CPSW_CLKSEL;               /* CPSW Clock Select Register */
    volatile uint8_t  Resv_33200[108];
    volatile uint32_t TIMER0_CLKSEL;             /* Timer0 Clock Select Register */
    volatile uint32_t TIMER1_CLKSEL;             /* Timer1 Clock Select Register */
    volatile uint32_t TIMER2_CLKSEL;             /* Timer2 Clock Select Register */
    volatile uint32_t TIMER3_CLKSEL;             /* Timer3 Clock Select Register */
    volatile uint32_t TIMER4_CLKSEL;             /* Timer4 Clock Select Register */
    volatile uint32_t TIMER5_CLKSEL;             /* Timer5 Clock Select Register */
    volatile uint32_t TIMER6_CLKSEL;             /* Timer6 Clock Select Register */
    volatile uint32_t TIMER7_CLKSEL;             /* Timer7 Clock Select Register */
    volatile uint32_t TIMER8_CLKSEL;             /* Timer8 Clock Select Register */
    volatile uint32_t TIMER9_CLKSEL;             /* Timer9 Clock Select Register */
    volatile uint8_t  Resv_33280[40];
    volatile uint32_t SPI0_CLKSEL;               /* SPI0 Clock Select Register */
    volatile uint32_t SPI1_CLKSEL;               /* SPI1 Clock Select Register */
    volatile uint32_t SPI2_CLKSEL;               /* SPI2 Clock Select Register */
    volatile uint8_t  Resv_33344[52];
    volatile uint32_t USART0_CLK_CTRL;           /* USART0 Functional Clock Control */
    volatile uint32_t USART1_CLK_CTRL;           /* USART1 Functional Clock Control */
    volatile uint32_t USART2_CLK_CTRL;           /* USART2 Functional Clock Control */
    volatile uint32_t USART3_CLK_CTRL;           /* USART3 Functional Clock Control */
    volatile uint8_t  Resv_33408[48];
    volatile uint32_t USART0_CLKSEL;             /* USART0 Functional Clock Control */
    volatile uint32_t USART1_CLKSEL;             /* USART1 Functional Clock Control */
    volatile uint32_t USART2_CLKSEL;             /* USART2 Functional Clock Control */
    volatile uint32_t USART3_CLKSEL;             /* USART3 Functional Clock Control */
    volatile uint8_t  Resv_33552[128];
    volatile uint32_t DPHY0_CLKSEL;              /* DSI0 PHY Clock Select Register */
    volatile uint8_t  Resv_33664[108];
    volatile uint32_t WWD0_CLKSEL;               /* WWD0 Clock Select Register */
    volatile uint32_t WWD1_CLKSEL;               /* WWD1 Clock Select Register */
    volatile uint32_t WWD2_CLKSEL;               /* WWD2 Clock Select Register */
    volatile uint32_t WWD3_CLKSEL;               /* WWD3 Clock Select Register */
    volatile uint32_t WWD4_CLKSEL;               /* WWD4 Clock Select Register */
    volatile uint8_t  Resv_33920[236];
    volatile uint32_t MCAN0_CLKSEL;              /* MCAN0 Clock Select Register */
    volatile uint32_t MCAN1_CLKSEL;              /* MCAN1 Clock Select Register */
    volatile uint8_t  Resv_34048[120];
    volatile uint32_t OSPI0_CLKSEL;              /* OSPI Clock Select Register */
    volatile uint8_t  Resv_34064[12];
    volatile uint32_t ADC0_CLKSEL;               /* ADC0 Clock Select Register */
    volatile uint8_t  Resv_36872[2804];
    volatile uint32_t LOCK2_KICK0;               /*  - KICK0 component */
    volatile uint32_t LOCK2_KICK1;               /*  - KICK1 component */
    volatile uint8_t  Resv_37120[240];
    volatile uint32_t CLAIMREG_P2_R0_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R1_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R2_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R3_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R4_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R5_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R6_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R7_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R8_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R9_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R10_READONLY;   /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R11_READONLY;   /* Claim bits for Partition 2 */
    volatile uint8_t  Resv_40960[3792];
    volatile uint32_t OBSCLK0_CTRL_PROXY;        /* Observe Clock 0 Output Control Register */
    volatile uint8_t  Resv_40976[12];
    volatile uint32_t CLKOUT_CTRL_PROXY;         /* CLKOUT Control Register */
    volatile uint8_t  Resv_41056[76];
    volatile uint32_t MAIN_PLL0_CLKSEL_PROXY;    /* MAIN PLL0 Source Clock Select Register */
    volatile uint32_t MAIN_PLL1_CLKSEL_PROXY;    /* MAIN PLL1 Source Clock Select Register */
    volatile uint32_t MAIN_PLL2_CLKSEL_PROXY;    /* MAIN PLL2 Source Clock Select Register */
    volatile uint8_t  Resv_41084[16];
    volatile uint32_t MAIN_PLL7_CLKSEL_PROXY;    /* MAIN PLL7 Source Clock Select Register */
    volatile uint8_t  Resv_41112[24];
    volatile uint32_t MAIN_PLL14_CLKSEL_PROXY;   /* MAIN PLL14 Source Clock Select Register */
    volatile uint8_t  Resv_41280[164];
    volatile uint32_t CPSW_CLKSEL_PROXY;         /* CPSW Clock Select Register */
    volatile uint8_t  Resv_41392[108];
    volatile uint32_t TIMER0_CLKSEL_PROXY;       /* Timer0 Clock Select Register */
    volatile uint32_t TIMER1_CLKSEL_PROXY;       /* Timer1 Clock Select Register */
    volatile uint32_t TIMER2_CLKSEL_PROXY;       /* Timer2 Clock Select Register */
    volatile uint32_t TIMER3_CLKSEL_PROXY;       /* Timer3 Clock Select Register */
    volatile uint32_t TIMER4_CLKSEL_PROXY;       /* Timer4 Clock Select Register */
    volatile uint32_t TIMER5_CLKSEL_PROXY;       /* Timer5 Clock Select Register */
    volatile uint32_t TIMER6_CLKSEL_PROXY;       /* Timer6 Clock Select Register */
    volatile uint32_t TIMER7_CLKSEL_PROXY;       /* Timer7 Clock Select Register */
    volatile uint32_t TIMER8_CLKSEL_PROXY;       /* Timer8 Clock Select Register */
    volatile uint32_t TIMER9_CLKSEL_PROXY;       /* Timer9 Clock Select Register */
    volatile uint8_t  Resv_41472[40];
    volatile uint32_t SPI0_CLKSEL_PROXY;         /* SPI0 Clock Select Register */
    volatile uint32_t SPI1_CLKSEL_PROXY;         /* SPI1 Clock Select Register */
    volatile uint32_t SPI2_CLKSEL_PROXY;         /* SPI2 Clock Select Register */
    volatile uint8_t  Resv_41536[52];
    volatile uint32_t USART0_CLK_CTRL_PROXY;     /* USART0 Functional Clock Control */
    volatile uint32_t USART1_CLK_CTRL_PROXY;     /* USART1 Functional Clock Control */
    volatile uint32_t USART2_CLK_CTRL_PROXY;     /* USART2 Functional Clock Control */
    volatile uint32_t USART3_CLK_CTRL_PROXY;     /* USART3 Functional Clock Control */
    volatile uint8_t  Resv_41600[48];
    volatile uint32_t USART0_CLKSEL_PROXY;       /* USART0 Functional Clock Control */
    volatile uint32_t USART1_CLKSEL_PROXY;       /* USART1 Functional Clock Control */
    volatile uint32_t USART2_CLKSEL_PROXY;       /* USART2 Functional Clock Control */
    volatile uint32_t USART3_CLKSEL_PROXY;       /* USART3 Functional Clock Control */
    volatile uint8_t  Resv_41744[128];
    volatile uint32_t DPHY0_CLKSEL_PROXY;        /* DSI0 PHY Clock Select Register */
    volatile uint8_t  Resv_41856[108];
    volatile uint32_t WWD0_CLKSEL_PROXY;         /* WWD0 Clock Select Register */
    volatile uint32_t WWD1_CLKSEL_PROXY;         /* WWD1 Clock Select Register */
    volatile uint32_t WWD2_CLKSEL_PROXY;         /* WWD2 Clock Select Register */
    volatile uint32_t WWD3_CLKSEL_PROXY;         /* WWD3 Clock Select Register */
    volatile uint32_t WWD4_CLKSEL_PROXY;         /* WWD4 Clock Select Register */
    volatile uint8_t  Resv_42112[236];
    volatile uint32_t MCAN0_CLKSEL_PROXY;        /* MCAN0 Clock Select Register */
    volatile uint32_t MCAN1_CLKSEL_PROXY;        /* MCAN1 Clock Select Register */
    volatile uint8_t  Resv_42240[120];
    volatile uint32_t OSPI0_CLKSEL_PROXY;        /* OSPI Clock Select Register */
    volatile uint8_t  Resv_42256[12];
    volatile uint32_t ADC0_CLKSEL_PROXY;         /* ADC0 Clock Select Register */
    volatile uint8_t  Resv_45064[2804];
    volatile uint32_t LOCK2_KICK0_PROXY;         /*  - KICK0 component */
    volatile uint32_t LOCK2_KICK1_PROXY;         /*  - KICK1 component */
    volatile uint8_t  Resv_45312[240];
    volatile uint32_t CLAIMREG_P2_R0;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R1;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R2;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R3;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R4;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R5;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R6;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R7;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R8;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R9;            /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R10;           /* Claim bits for Partition 2 */
    volatile uint32_t CLAIMREG_P2_R11;           /* Claim bits for Partition 2 */
    volatile uint8_t  Resv_49152[3792];
    volatile uint32_t R5SS0_LBIST_CTRL;          /* R5 Cluster0 Logic BIST Control Register */
    volatile uint32_t R5SS0_LBIST_PATCOUNT;      /* R5 Cluster0 Logic BIST Pattern Count Register */
    volatile uint32_t R5SS0_LBIST_SEED0;         /* R5 Cluster0Logic BIST Seed0 Register */
    volatile uint32_t R5SS0_LBIST_SEED1;         /* R5 Cluster0Logic BIST Seed1 Register */
    volatile uint32_t R5SS0_LBIST_SPARE0;        /* R5 Cluster0 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_49176[4];
    volatile uint32_t R5SS0_LBIST_STAT;          /* R5 Cluster0Logic BIST Status Register */
    volatile uint32_t R5SS0_LBIST_MISR;          /* R5 Cluster0 Logic BIST MISR Register */
    volatile uint32_t R5SS1_LBIST_CTRL;          /* R5 Cluster1 Logic BIST Control Register */
    volatile uint32_t R5SS1_LBIST_PATCOUNT;      /* R5 Cluster1 Logic BIST Pattern Count Register */
    volatile uint32_t R5SS1_LBIST_SEED0;         /* R5 Cluster1Logic BIST Seed0 Register */
    volatile uint32_t R5SS1_LBIST_SEED1;         /* R5 Cluster1Logic BIST Seed1 Register */
    volatile uint32_t R5SS1_LBIST_SPARE0;        /* R5 Cluster1 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_49208[4];
    volatile uint32_t R5SS1_LBIST_STAT;          /* R5 Cluster1Logic BIST Status Register */
    volatile uint32_t R5SS1_LBIST_MISR;          /* R5 Cluster1 Logic BIST MISR Register */
    volatile uint8_t  Resv_49280[64];
    volatile uint32_t C70_LBIST_CTRL;            /* C70 Logic BIST Control Register */
    volatile uint32_t C70_LBIST_PATCOUNT;        /* C70 Logic BIST Pattern Count Register */
    volatile uint32_t C70_LBIST_SEED0;           /* C70 Logic BIST Seed0 Register */
    volatile uint32_t C70_LBIST_SEED1;           /* C70 Logic BIST Seed1 Register */
    volatile uint32_t C70_LBIST_SPARE0;          /* C70 Logic BIST Spare0 Register */
    volatile uint32_t C70_LBIST_SPARE1;          /* C70 Logic BIST Spare1 Register */
    volatile uint32_t C70_LBIST_STAT;            /* C70 Logic BIST Status Register */
    volatile uint8_t  Resv_49536[228];
    volatile uint32_t MMA0_LBIST_CTRL;           /* MMA0 Logic BIST Control Register */
    volatile uint32_t MMA0_LBIST_PATCOUNT;       /* MMA0 Logic BIST Pattern Count Register */
    volatile uint32_t MMA0_LBIST_SEED0;          /* MMA0 Logic BIST Seed0 Register */
    volatile uint32_t MMA0_LBIST_SEED1;          /* MMA0 Logic BIST Seed1 Register */
    volatile uint32_t MMA0_LBIST_SPARE0;         /* MMA0 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_49560[4];
    volatile uint32_t MMA0_LBIST_STAT;           /* MMA0 Logic BIST Status Register */
    volatile uint32_t MMA0_LBIST_MISR;           /* MMA0 Logic BIST MISR Register */
    volatile uint8_t  Resv_49712[144];
    volatile uint32_t HWA0_LBIST_CTRL;           /* HWA0 Logic BIST Control Register */
    volatile uint32_t HWA0_LBIST_PATCOUNT;       /* HWA0 Logic BIST Pattern Count Register */
    volatile uint32_t HWA0_LBIST_SEED0;          /* HWA0Logic BIST Seed0 Register */
    volatile uint32_t HWA0_LBIST_SEED1;          /* HWA0Logic BIST Seed1 Register */
    volatile uint32_t HWA0_LBIST_SPARE0;         /* HWA0 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_49736[4];
    volatile uint32_t HWA0_LBIST_STAT;           /* HWA0Logic BIST Status Register */
    volatile uint32_t HWA0_LBIST_MISR;           /* HWA0 Logic BIST MISR Register */
    volatile uint8_t  Resv_49792[48];
    volatile uint32_t R5SS0_LBIST_SIG;           /* MCU Cluster0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_49824[28];
    volatile uint32_t R5SS1_LBIST_SIG;           /* MCU Cluster0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_50000[172];
    volatile uint32_t HWA0_LBIST_SIG;            /* MMA0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_50096[92];
    volatile uint32_t C70_LBIST_SIG;             /* C70 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_50160[60];
    volatile uint32_t MMA0_LBIST_SIG;            /* MMA0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_53256[3092];
    volatile uint32_t LOCK3_KICK0;               /*  - KICK0 component */
    volatile uint32_t LOCK3_KICK1;               /*  - KICK1 component */
    volatile uint8_t  Resv_53504[240];
    volatile uint32_t CLAIMREG_P3_R0_READONLY;   /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R1_READONLY;   /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R2_READONLY;   /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R3_READONLY;   /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R4_READONLY;   /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R5_READONLY;   /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R6_READONLY;   /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R7_READONLY;   /* Claim bits for Partition 3 */
    volatile uint8_t  Resv_57344[3808];
    volatile uint32_t R5SS0_LBIST_CTRL_PROXY;    /* R5 Cluster0 Logic BIST Control Register */
    volatile uint32_t R5SS0_LBIST_PATCOUNT_PROXY;   /* R5 Cluster0 Logic BIST Pattern Count Register */
    volatile uint32_t R5SS0_LBIST_SEED0_PROXY;   /* R5 Cluster0Logic BIST Seed0 Register */
    volatile uint32_t R5SS0_LBIST_SEED1_PROXY;   /* R5 Cluster0Logic BIST Seed1 Register */
    volatile uint32_t R5SS0_LBIST_SPARE0_PROXY;   /* R5 Cluster0 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_57368[4];
    volatile uint32_t R5SS0_LBIST_STAT_PROXY;    /* R5 Cluster0Logic BIST Status Register */
    volatile uint32_t R5SS0_LBIST_MISR_PROXY;    /* R5 Cluster0 Logic BIST MISR Register */
    volatile uint32_t R5SS1_LBIST_CTRL_PROXY;    /* R5 Cluster1 Logic BIST Control Register */
    volatile uint32_t R5SS1_LBIST_PATCOUNT_PROXY;   /* R5 Cluster1 Logic BIST Pattern Count Register */
    volatile uint32_t R5SS1_LBIST_SEED0_PROXY;   /* R5 Cluster1Logic BIST Seed0 Register */
    volatile uint32_t R5SS1_LBIST_SEED1_PROXY;   /* R5 Cluster1Logic BIST Seed1 Register */
    volatile uint32_t R5SS1_LBIST_SPARE0_PROXY;   /* R5 Cluster1 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_57400[4];
    volatile uint32_t R5SS1_LBIST_STAT_PROXY;    /* R5 Cluster1Logic BIST Status Register */
    volatile uint32_t R5SS1_LBIST_MISR_PROXY;    /* R5 Cluster1 Logic BIST MISR Register */
    volatile uint8_t  Resv_57472[64];
    volatile uint32_t C70_LBIST_CTRL_PROXY;      /* C70 Logic BIST Control Register */
    volatile uint32_t C70_LBIST_PATCOUNT_PROXY;   /* C70 Logic BIST Pattern Count Register */
    volatile uint32_t C70_LBIST_SEED0_PROXY;     /* C70 Logic BIST Seed0 Register */
    volatile uint32_t C70_LBIST_SEED1_PROXY;     /* C70 Logic BIST Seed1 Register */
    volatile uint32_t C70_LBIST_SPARE0_PROXY;    /* C70 Logic BIST Spare0 Register */
    volatile uint32_t C70_LBIST_SPARE1_PROXY;    /* C70 Logic BIST Spare1 Register */
    volatile uint32_t C70_LBIST_STAT_PROXY;      /* C70 Logic BIST Status Register */
    volatile uint8_t  Resv_57728[228];
    volatile uint32_t MMA0_LBIST_CTRL_PROXY;     /* MMA0 Logic BIST Control Register */
    volatile uint32_t MMA0_LBIST_PATCOUNT_PROXY;   /* MMA0 Logic BIST Pattern Count Register */
    volatile uint32_t MMA0_LBIST_SEED0_PROXY;    /* MMA0 Logic BIST Seed0 Register */
    volatile uint32_t MMA0_LBIST_SEED1_PROXY;    /* MMA0 Logic BIST Seed1 Register */
    volatile uint32_t MMA0_LBIST_SPARE0_PROXY;   /* MMA0 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_57752[4];
    volatile uint32_t MMA0_LBIST_STAT_PROXY;     /* MMA0 Logic BIST Status Register */
    volatile uint32_t MMA0_LBIST_MISR_PROXY;     /* MMA0 Logic BIST MISR Register */
    volatile uint8_t  Resv_57904[144];
    volatile uint32_t HWA0_LBIST_CTRL_PROXY;     /* HWA0 Logic BIST Control Register */
    volatile uint32_t HWA0_LBIST_PATCOUNT_PROXY;   /* HWA0 Logic BIST Pattern Count Register */
    volatile uint32_t HWA0_LBIST_SEED0_PROXY;    /* HWA0Logic BIST Seed0 Register */
    volatile uint32_t HWA0_LBIST_SEED1_PROXY;    /* HWA0Logic BIST Seed1 Register */
    volatile uint32_t HWA0_LBIST_SPARE0_PROXY;   /* HWA0 Logic BIST Spare0 Register */
    volatile uint8_t  Resv_57928[4];
    volatile uint32_t HWA0_LBIST_STAT_PROXY;     /* HWA0Logic BIST Status Register */
    volatile uint32_t HWA0_LBIST_MISR_PROXY;     /* HWA0 Logic BIST MISR Register */
    volatile uint8_t  Resv_57984[48];
    volatile uint32_t R5SS0_LBIST_SIG_PROXY;     /* MCU Cluster0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_58016[28];
    volatile uint32_t R5SS1_LBIST_SIG_PROXY;     /* MCU Cluster0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_58192[172];
    volatile uint32_t HWA0_LBIST_SIG_PROXY;      /* MMA0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_58288[92];
    volatile uint32_t C70_LBIST_SIG_PROXY;       /* C70 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_58352[60];
    volatile uint32_t MMA0_LBIST_SIG_PROXY;      /* MMA0 Logic BIST MISR Signature Register */
    volatile uint8_t  Resv_61448[3092];
    volatile uint32_t LOCK3_KICK0_PROXY;         /*  - KICK0 component */
    volatile uint32_t LOCK3_KICK1_PROXY;         /*  - KICK1 component */
    volatile uint8_t  Resv_61696[240];
    volatile uint32_t CLAIMREG_P3_R0;            /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R1;            /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R2;            /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R3;            /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R4;            /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R5;            /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R6;            /* Claim bits for Partition 3 */
    volatile uint32_t CLAIMREG_P3_R7;            /* Claim bits for Partition 3 */
    volatile uint8_t  Resv_69640[7912];
    volatile uint32_t LOCK4_KICK0;               /*  - KICK0 component */
    volatile uint32_t LOCK4_KICK1;               /*  - KICK1 component */
    volatile uint8_t  Resv_69888[240];
    volatile uint32_t CLAIMREG_P4_R0_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R1_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R2_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R3_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R4_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R5_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R6_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R7_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R8_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R9_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R10_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R11_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R12_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R13_READONLY;   /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R14_READONLY;   /* Claim bits for Partition 4 */
    volatile uint8_t  Resv_77832[7884];
    volatile uint32_t LOCK4_KICK0_PROXY;         /*  - KICK0 component */
    volatile uint32_t LOCK4_KICK1_PROXY;         /*  - KICK1 component */
    volatile uint8_t  Resv_78080[240];
    volatile uint32_t CLAIMREG_P4_R0;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R1;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R2;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R3;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R4;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R5;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R6;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R7;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R8;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R9;            /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R10;           /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R11;           /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R12;           /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R13;           /* Claim bits for Partition 4 */
    volatile uint32_t CLAIMREG_P4_R14;           /* Claim bits for Partition 4 */
    volatile uint8_t  Resv_102408[24268];
    volatile uint32_t LOCK6_KICK0;               /*  - KICK0 component */
    volatile uint32_t LOCK6_KICK1;               /*  - KICK1 component */
    volatile uint8_t  Resv_102656[240];
    volatile uint32_t CLAIMREG_P6_R0_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R1_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R2_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R3_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R4_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R5_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R6_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R7_READONLY;   /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R8_READONLY;   /* Claim bits for Partition 6 */
    volatile uint8_t  Resv_110600[7908];
    volatile uint32_t LOCK6_KICK0_PROXY;         /*  - KICK0 component */
    volatile uint32_t LOCK6_KICK1_PROXY;         /*  - KICK1 component */
    volatile uint8_t  Resv_110848[240];
    volatile uint32_t CLAIMREG_P6_R0;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R1;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R2;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R3;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R4;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R5;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R6;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R7;            /* Claim bits for Partition 6 */
    volatile uint32_t CLAIMREG_P6_R8;            /* Claim bits for Partition 6 */
} CSL_main_ctrl_mmr_cfg0Regs;


/**************************************************************************
* Register Macros
**************************************************************************/

#define CSL_MAIN_CTRL_MMR_CFG0_PID                                             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1                                        (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0                                        (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16                                       (0x00000140U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17                                       (0x00000144U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18                                       (0x00000148U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19                                       (0x0000014CU)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0                                        (0x00000180U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16                                       (0x000001C0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17                                       (0x000001C4U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18                                       (0x000001C8U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19                                       (0x000001CCU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0                                     (0x00001008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1                                     (0x0000100CU)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS                                 (0x00001010U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR                       (0x00001014U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE                                     (0x00001018U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR                               (0x0000101CU)
#define CSL_MAIN_CTRL_MMR_CFG0_EOI                                             (0x00001020U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS                                   (0x00001024U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS                               (0x00001028U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS                               (0x0000102CU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR                                     (0x00001030U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0_READONLY                         (0x00001100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1_READONLY                         (0x00001104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2_READONLY                         (0x00001108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3_READONLY                         (0x0000110CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4_READONLY                         (0x00001110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5_READONLY                         (0x00001114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6_READONLY                         (0x00001118U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY                                       (0x00002000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY                                  (0x00002008U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_PROXY                                  (0x00002100U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_PROXY                                 (0x00002140U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_PROXY                                 (0x00002144U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_PROXY                                 (0x00002148U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_PROXY                                 (0x0000214CU)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_PROXY                                  (0x00002180U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_PROXY                                 (0x000021C0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_PROXY                                 (0x000021C4U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_PROXY                                 (0x000021C8U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_PROXY                                 (0x000021CCU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0_PROXY                               (0x00003008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1_PROXY                               (0x0000300CU)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY                           (0x00003010U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY                 (0x00003014U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY                               (0x00003018U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY                         (0x0000301CU)
#define CSL_MAIN_CTRL_MMR_CFG0_EOI_PROXY                                       (0x00003020U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS_PROXY                             (0x00003024U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_PROXY                         (0x00003028U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY                         (0x0000302CU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR_PROXY                               (0x00003030U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0                                  (0x00003100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1                                  (0x00003104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2                                  (0x00003108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3                                  (0x0000310CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4                                  (0x00003110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5                                  (0x00003114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6                                  (0x00003118U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL                                      (0x00004044U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL                                      (0x00004048U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN                                   (0x00004130U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL                                      (0x00004140U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL                                      (0x00004144U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL                                      (0x00004148U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL                                     (0x00004200U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL                                     (0x00004204U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL                                     (0x00004208U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL                                     (0x0000420CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL                                     (0x00004210U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL                                     (0x00004214U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL                                     (0x00004218U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL                                     (0x0000421CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL                                     (0x00004220U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL                                     (0x00004224U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL                                   (0x00004280U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL                                   (0x00004284U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL                                   (0x00004288U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL                                   (0x0000428CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL                                   (0x00004290U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL                                   (0x00004294U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL                                   (0x00004298U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL                                   (0x0000429CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0                                      (0x00004300U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL                                   (0x00004310U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL                                       (0x000044D0U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL                                        (0x000044F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0                                      (0x00004700U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1                                      (0x00004704U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT                                        (0x00004750U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL                            (0x00004760U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL                            (0x00004764U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL                            (0x00004770U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL                            (0x00004774U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS                              (0x00004A04U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0                           (0x00004A10U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0                                   (0x00004A20U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START                              (0x00004A30U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0                               (0x00004A40U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0                              (0x00004A50U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE                                   (0x00004A70U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0                                     (0x00005008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1                                     (0x0000500CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0_READONLY                         (0x00005100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1_READONLY                         (0x00005104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2_READONLY                         (0x00005108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3_READONLY                         (0x0000510CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4_READONLY                         (0x00005110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5_READONLY                         (0x00005114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6_READONLY                         (0x00005118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7_READONLY                         (0x0000511CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8_READONLY                         (0x00005120U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9_READONLY                         (0x00005124U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10_READONLY                        (0x00005128U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11_READONLY                        (0x0000512CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12_READONLY                        (0x00005130U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13_READONLY                        (0x00005134U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14_READONLY                        (0x00005138U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15_READONLY                        (0x0000513CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16_READONLY                        (0x00005140U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17_READONLY                        (0x00005144U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18_READONLY                        (0x00005148U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19_READONLY                        (0x0000514CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20_READONLY                        (0x00005150U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PROXY                                (0x00006044U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PROXY                                (0x00006048U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY                             (0x00006130U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_PROXY                                (0x00006140U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL_PROXY                                (0x00006144U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL_PROXY                                (0x00006148U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL_PROXY                               (0x00006200U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_PROXY                               (0x00006204U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL_PROXY                               (0x00006208U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_PROXY                               (0x0000620CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL_PROXY                               (0x00006210U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_PROXY                               (0x00006214U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL_PROXY                               (0x00006218U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_PROXY                               (0x0000621CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL_PROXY                               (0x00006220U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_PROXY                               (0x00006224U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL_PROXY                             (0x00006280U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL_PROXY                             (0x00006284U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL_PROXY                             (0x00006288U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL_PROXY                             (0x0000628CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL_PROXY                             (0x00006290U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL_PROXY                             (0x00006294U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL_PROXY                             (0x00006298U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL_PROXY                             (0x0000629CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY                                (0x00006300U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_PROXY                             (0x00006310U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_PROXY                                 (0x000064D0U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY                                  (0x000064F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0_PROXY                                (0x00006700U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1_PROXY                                (0x00006704U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY                                  (0x00006750U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL_PROXY                      (0x00006760U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL_PROXY                      (0x00006764U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL_PROXY                      (0x00006770U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL_PROXY                      (0x00006774U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS_PROXY                        (0x00006A04U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0_PROXY                     (0x00006A10U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_PROXY                             (0x00006A20U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY                        (0x00006A30U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_PROXY                         (0x00006A40U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_PROXY                        (0x00006A50U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY                             (0x00006A70U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0_PROXY                               (0x00007008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1_PROXY                               (0x0000700CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0                                  (0x00007100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1                                  (0x00007104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2                                  (0x00007108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3                                  (0x0000710CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4                                  (0x00007110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5                                  (0x00007114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6                                  (0x00007118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7                                  (0x0000711CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8                                  (0x00007120U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9                                  (0x00007124U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10                                 (0x00007128U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11                                 (0x0000712CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12                                 (0x00007130U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13                                 (0x00007134U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14                                 (0x00007138U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15                                 (0x0000713CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16                                 (0x00007140U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17                                 (0x00007144U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18                                 (0x00007148U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19                                 (0x0000714CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20                                 (0x00007150U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL                                    (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL                                     (0x00008010U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL                                (0x00008060U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL                                (0x00008064U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL                                (0x00008068U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL                                (0x0000807CU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL                               (0x00008098U)
#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL                                     (0x00008140U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL                                   (0x000081B0U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL                                   (0x000081B4U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL                                   (0x000081B8U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL                                   (0x000081BCU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL                                   (0x000081C0U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL                                   (0x000081C4U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL                                   (0x000081C8U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL                                   (0x000081CCU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL                                   (0x000081D0U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL                                   (0x000081D4U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL                                     (0x00008200U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL                                     (0x00008204U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL                                     (0x00008208U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL                                 (0x00008240U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL                                 (0x00008244U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL                                 (0x00008248U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL                                 (0x0000824CU)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL                                   (0x00008280U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL                                   (0x00008284U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL                                   (0x00008288U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL                                   (0x0000828CU)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL                                    (0x00008310U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL                                     (0x00008380U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL                                     (0x00008384U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL                                     (0x00008388U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL                                     (0x0000838CU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL                                     (0x00008390U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL                                    (0x00008480U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL                                    (0x00008484U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL                                    (0x00008500U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL                                     (0x00008510U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0                                     (0x00009008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1                                     (0x0000900CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0_READONLY                         (0x00009100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1_READONLY                         (0x00009104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2_READONLY                         (0x00009108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3_READONLY                         (0x0000910CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4_READONLY                         (0x00009110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5_READONLY                         (0x00009114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6_READONLY                         (0x00009118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7_READONLY                         (0x0000911CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8_READONLY                         (0x00009120U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9_READONLY                         (0x00009124U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10_READONLY                        (0x00009128U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11_READONLY                        (0x0000912CU)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY                              (0x0000A000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL_PROXY                               (0x0000A010U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_PROXY                          (0x0000A060U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_PROXY                          (0x0000A064U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_PROXY                          (0x0000A068U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_PROXY                          (0x0000A07CU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_PROXY                         (0x0000A098U)
#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL_PROXY                               (0x0000A140U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL_PROXY                             (0x0000A1B0U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL_PROXY                             (0x0000A1B4U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL_PROXY                             (0x0000A1B8U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL_PROXY                             (0x0000A1BCU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL_PROXY                             (0x0000A1C0U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL_PROXY                             (0x0000A1C4U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL_PROXY                             (0x0000A1C8U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL_PROXY                             (0x0000A1CCU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL_PROXY                             (0x0000A1D0U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL_PROXY                             (0x0000A1D4U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL_PROXY                               (0x0000A200U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL_PROXY                               (0x0000A204U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL_PROXY                               (0x0000A208U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_PROXY                           (0x0000A240U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_PROXY                           (0x0000A244U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_PROXY                           (0x0000A248U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_PROXY                           (0x0000A24CU)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL_PROXY                             (0x0000A280U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL_PROXY                             (0x0000A284U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL_PROXY                             (0x0000A288U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL_PROXY                             (0x0000A28CU)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL_PROXY                              (0x0000A310U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_PROXY                               (0x0000A380U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_PROXY                               (0x0000A384U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_PROXY                               (0x0000A388U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_PROXY                               (0x0000A38CU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_PROXY                               (0x0000A390U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL_PROXY                              (0x0000A480U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL_PROXY                              (0x0000A484U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_PROXY                              (0x0000A500U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL_PROXY                               (0x0000A510U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0_PROXY                               (0x0000B008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1_PROXY                               (0x0000B00CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0                                  (0x0000B100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1                                  (0x0000B104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2                                  (0x0000B108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3                                  (0x0000B10CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4                                  (0x0000B110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5                                  (0x0000B114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6                                  (0x0000B118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7                                  (0x0000B11CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8                                  (0x0000B120U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9                                  (0x0000B124U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10                                 (0x0000B128U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11                                 (0x0000B12CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL                                (0x0000C000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT                            (0x0000C004U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0                               (0x0000C008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1                               (0x0000C00CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0                              (0x0000C010U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT                                (0x0000C018U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR                                (0x0000C01CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL                                (0x0000C020U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT                            (0x0000C024U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0                               (0x0000C028U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1                               (0x0000C02CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0                              (0x0000C030U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT                                (0x0000C038U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR                                (0x0000C03CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL                                  (0x0000C080U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT                              (0x0000C084U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0                                 (0x0000C088U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1                                 (0x0000C08CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0                                (0x0000C090U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1                                (0x0000C094U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT                                  (0x0000C098U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL                                 (0x0000C180U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT                             (0x0000C184U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0                                (0x0000C188U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1                                (0x0000C18CU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0                               (0x0000C190U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT                                 (0x0000C198U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR                                 (0x0000C19CU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL                                 (0x0000C230U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT                             (0x0000C234U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0                                (0x0000C238U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1                                (0x0000C23CU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0                               (0x0000C240U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT                                 (0x0000C248U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR                                 (0x0000C24CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG                                 (0x0000C280U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG                                 (0x0000C2A0U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG                                  (0x0000C350U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG                                   (0x0000C3B0U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG                                  (0x0000C3F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0                                     (0x0000D008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1                                     (0x0000D00CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0_READONLY                         (0x0000D100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1_READONLY                         (0x0000D104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2_READONLY                         (0x0000D108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3_READONLY                         (0x0000D10CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4_READONLY                         (0x0000D110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5_READONLY                         (0x0000D114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6_READONLY                         (0x0000D118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7_READONLY                         (0x0000D11CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY                          (0x0000E000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY                      (0x0000E004U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0_PROXY                         (0x0000E008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1_PROXY                         (0x0000E00CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY                        (0x0000E010U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY                          (0x0000E018U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR_PROXY                          (0x0000E01CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY                          (0x0000E020U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY                      (0x0000E024U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0_PROXY                         (0x0000E028U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1_PROXY                         (0x0000E02CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY                        (0x0000E030U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY                          (0x0000E038U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR_PROXY                          (0x0000E03CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY                            (0x0000E080U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY                        (0x0000E084U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0_PROXY                           (0x0000E088U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1_PROXY                           (0x0000E08CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY                          (0x0000E090U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1_PROXY                          (0x0000E094U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY                            (0x0000E098U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY                           (0x0000E180U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY                       (0x0000E184U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0_PROXY                          (0x0000E188U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1_PROXY                          (0x0000E18CU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY                         (0x0000E190U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY                           (0x0000E198U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR_PROXY                           (0x0000E19CU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY                           (0x0000E230U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY                       (0x0000E234U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0_PROXY                          (0x0000E238U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1_PROXY                          (0x0000E23CU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY                         (0x0000E240U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY                           (0x0000E248U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR_PROXY                           (0x0000E24CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG_PROXY                           (0x0000E280U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG_PROXY                           (0x0000E2A0U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG_PROXY                            (0x0000E350U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG_PROXY                             (0x0000E3B0U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG_PROXY                            (0x0000E3F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0_PROXY                               (0x0000F008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1_PROXY                               (0x0000F00CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0                                  (0x0000F100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1                                  (0x0000F104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2                                  (0x0000F108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3                                  (0x0000F10CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4                                  (0x0000F110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5                                  (0x0000F114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6                                  (0x0000F118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7                                  (0x0000F11CU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0                                     (0x00011008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1                                     (0x0001100CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0_READONLY                         (0x00011100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1_READONLY                         (0x00011104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2_READONLY                         (0x00011108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3_READONLY                         (0x0001110CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4_READONLY                         (0x00011110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5_READONLY                         (0x00011114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6_READONLY                         (0x00011118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7_READONLY                         (0x0001111CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8_READONLY                         (0x00011120U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9_READONLY                         (0x00011124U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10_READONLY                        (0x00011128U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11_READONLY                        (0x0001112CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12_READONLY                        (0x00011130U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13_READONLY                        (0x00011134U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14_READONLY                        (0x00011138U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0_PROXY                               (0x00013008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1_PROXY                               (0x0001300CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0                                  (0x00013100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1                                  (0x00013104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2                                  (0x00013108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3                                  (0x0001310CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4                                  (0x00013110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5                                  (0x00013114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6                                  (0x00013118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7                                  (0x0001311CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8                                  (0x00013120U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9                                  (0x00013124U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10                                 (0x00013128U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11                                 (0x0001312CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12                                 (0x00013130U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13                                 (0x00013134U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14                                 (0x00013138U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0                                     (0x00019008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1                                     (0x0001900CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0_READONLY                         (0x00019100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1_READONLY                         (0x00019104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2_READONLY                         (0x00019108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3_READONLY                         (0x0001910CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4_READONLY                         (0x00019110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5_READONLY                         (0x00019114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6_READONLY                         (0x00019118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7_READONLY                         (0x0001911CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8_READONLY                         (0x00019120U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0_PROXY                               (0x0001B008U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1_PROXY                               (0x0001B00CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0                                  (0x0001B100U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1                                  (0x0001B104U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2                                  (0x0001B108U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3                                  (0x0001B10CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4                                  (0x0001B110U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5                                  (0x0001B114U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6                                  (0x0001B118U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7                                  (0x0001B11CU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8                                  (0x0001B120U)

/**************************************************************************
* Field Definition Macros
**************************************************************************/


/* PID */

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MINOR_MASK                              (0x0000003FU)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MINOR_SHIFT                             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MINOR_MAX                               (0x0000003FU)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_CUSTOM_MASK                             (0x000000C0U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_CUSTOM_SHIFT                            (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_CUSTOM_MAX                              (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MAJOR_MASK                              (0x00000700U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MAJOR_SHIFT                             (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MAJOR_MAX                               (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MISC_MASK                               (0x0000F800U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MISC_SHIFT                              (0x0000000BU)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MISC_MAX                                (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MSB16_MASK                              (0xFFFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MSB16_SHIFT                             (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PID_MSB16_MAX                               (0x0000FFFFU)


/* MMR_CFG1 */

#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PARTITIONS_MASK                        (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PARTITIONS_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PARTITIONS_MAX                         (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_EN_MASK                          (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_EN_SHIFT                         (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_EN_MAX                           (0x00000001U)


/* IPC_SET0 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_IPC_SET_MASK                           (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_IPC_SET_SHIFT                          (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_IPC_SET_MAX                            (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_IPC_SRC_SET_MASK                       (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_IPC_SRC_SET_SHIFT                      (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_IPC_SRC_SET_MAX                        (0x0FFFFFFFU)


/* IPC_SET16 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_IPC_SET_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_IPC_SET_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_IPC_SET_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_IPC_SRC_SET_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_IPC_SRC_SET_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_IPC_SRC_SET_MAX                       (0x0FFFFFFFU)


/* IPC_SET17 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_IPC_SET_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_IPC_SET_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_IPC_SET_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_IPC_SRC_SET_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_IPC_SRC_SET_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_IPC_SRC_SET_MAX                       (0x0FFFFFFFU)


/* IPC_SET18 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_IPC_SET_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_IPC_SET_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_IPC_SET_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_IPC_SRC_SET_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_IPC_SRC_SET_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_IPC_SRC_SET_MAX                       (0x0FFFFFFFU)


/* IPC_SET19 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_IPC_SET_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_IPC_SET_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_IPC_SET_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_IPC_SRC_SET_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_IPC_SRC_SET_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_IPC_SRC_SET_MAX                       (0x0FFFFFFFU)


/* IPC_CLR0 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_IPC_CLR_MASK                           (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_IPC_CLR_SHIFT                          (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_IPC_CLR_MAX                            (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_IPC_SRC_CLR_MASK                       (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_IPC_SRC_CLR_SHIFT                      (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_IPC_SRC_CLR_MAX                        (0x0FFFFFFFU)


/* IPC_CLR16 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_IPC_CLR_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_IPC_CLR_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_IPC_CLR_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_IPC_SRC_CLR_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_IPC_SRC_CLR_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_IPC_SRC_CLR_MAX                       (0x0FFFFFFFU)


/* IPC_CLR17 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_IPC_CLR_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_IPC_CLR_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_IPC_CLR_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_IPC_SRC_CLR_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_IPC_SRC_CLR_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_IPC_SRC_CLR_MAX                       (0x0FFFFFFFU)


/* IPC_CLR18 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_IPC_CLR_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_IPC_CLR_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_IPC_CLR_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_IPC_SRC_CLR_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_IPC_SRC_CLR_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_IPC_SRC_CLR_MAX                       (0x0FFFFFFFU)


/* IPC_CLR19 */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_IPC_CLR_MASK                          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_IPC_CLR_SHIFT                         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_IPC_CLR_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_IPC_SRC_CLR_MASK                      (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_IPC_SRC_CLR_SHIFT                     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_IPC_SRC_CLR_MAX                       (0x0FFFFFFFU)


/* LOCK0_KICK0 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0_LOCK0_KICK0_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0_LOCK0_KICK0_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0_LOCK0_KICK0_MAX                     (0xFFFFFFFFU)


/* LOCK0_KICK1 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1_LOCK0_KICK1_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1_LOCK0_KICK1_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1_LOCK0_KICK1_MAX                     (0xFFFFFFFFU)


/* INTR_RAW_STATUS */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROT_ERR_MASK                   (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROT_ERR_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROT_ERR_MAX                    (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_ADDR_ERR_MASK                   (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_ADDR_ERR_SHIFT                  (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_ADDR_ERR_MAX                    (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_KICK_ERR_MASK                   (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_KICK_ERR_SHIFT                  (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_KICK_ERR_MAX                    (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_ERR_MASK                  (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_ERR_SHIFT                 (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_ERR_MAX                   (0x00000001U)


/* INTR_ENABLED_STATUS_CLEAR */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_PROT_ERR_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_PROT_ERR_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_PROT_ERR_MAX  (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_ADDR_ERR_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_ADDR_ERR_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_ADDR_ERR_MAX  (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_KICK_ERR_MASK (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_KICK_ERR_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_KICK_ERR_MAX  (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_PROXY_ERR_MASK (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_PROXY_ERR_SHIFT (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_ENABLED_PROXY_ERR_MAX (0x00000001U)


/* INTR_ENABLE */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROT_ERR_EN_MASK                    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROT_ERR_EN_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROT_ERR_EN_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_ADDR_ERR_EN_MASK                    (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_ADDR_ERR_EN_SHIFT                   (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_ADDR_ERR_EN_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_KICK_ERR_EN_MASK                    (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_KICK_ERR_EN_SHIFT                   (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_KICK_ERR_EN_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_ERR_EN_MASK                   (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_ERR_EN_SHIFT                  (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_ERR_EN_MAX                    (0x00000001U)


/* INTR_ENABLE_CLEAR */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROT_ERR_EN_CLR_MASK          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROT_ERR_EN_CLR_SHIFT         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROT_ERR_EN_CLR_MAX           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_ADDR_ERR_EN_CLR_MASK          (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_ADDR_ERR_EN_CLR_SHIFT         (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_ADDR_ERR_EN_CLR_MAX           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_KICK_ERR_EN_CLR_MASK          (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_KICK_ERR_EN_CLR_SHIFT         (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_KICK_ERR_EN_CLR_MAX           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_ERR_EN_CLR_MASK         (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_ERR_EN_CLR_SHIFT        (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_ERR_EN_CLR_MAX          (0x00000001U)


/* EOI */

#define CSL_MAIN_CTRL_MMR_CFG0_EOI_EOI_VECTOR_MASK                             (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_EOI_EOI_VECTOR_SHIFT                            (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_EOI_EOI_VECTOR_MAX                              (0x000000FFU)


/* FAULT_ADDRESS */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS_FAULT_ADDR_MASK                   (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS_FAULT_ADDR_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS_FAULT_ADDR_MAX                    (0xFFFFFFFFU)


/* FAULT_TYPE_STATUS */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_FAULT_TYPE_MASK               (0x0000003FU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_FAULT_TYPE_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_FAULT_TYPE_MAX                (0x0000003FU)

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_FAULT_NS_MASK                 (0x00000040U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_FAULT_NS_SHIFT                (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_FAULT_NS_MAX                  (0x00000001U)


/* FAULT_ATTR_STATUS */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_PRIVID_MASK             (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_PRIVID_SHIFT            (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_PRIVID_MAX              (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_ROUTEID_MASK            (0x000FFF00U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_ROUTEID_SHIFT           (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_ROUTEID_MAX             (0x00000FFFU)

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_XID_MASK                (0xFFF00000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_XID_SHIFT               (0x00000014U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_FAULT_XID_MAX                 (0x00000FFFU)


/* FAULT_CLEAR */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR_FAULT_CLR_MASK                      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR_FAULT_CLR_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR_FAULT_CLR_MAX                       (0x00000001U)


/* CLAIMREG_P0_R0_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0_READONLY_CLAIMREG_P0_R0_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0_READONLY_CLAIMREG_P0_R0_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0_READONLY_CLAIMREG_P0_R0_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P0_R1_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1_READONLY_CLAIMREG_P0_R1_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1_READONLY_CLAIMREG_P0_R1_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1_READONLY_CLAIMREG_P0_R1_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P0_R2_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2_READONLY_CLAIMREG_P0_R2_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2_READONLY_CLAIMREG_P0_R2_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2_READONLY_CLAIMREG_P0_R2_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P0_R3_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3_READONLY_CLAIMREG_P0_R3_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3_READONLY_CLAIMREG_P0_R3_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3_READONLY_CLAIMREG_P0_R3_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P0_R4_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4_READONLY_CLAIMREG_P0_R4_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4_READONLY_CLAIMREG_P0_R4_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4_READONLY_CLAIMREG_P0_R4_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P0_R5_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5_READONLY_CLAIMREG_P0_R5_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5_READONLY_CLAIMREG_P0_R5_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5_READONLY_CLAIMREG_P0_R5_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P0_R6_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6_READONLY_CLAIMREG_P0_R6_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6_READONLY_CLAIMREG_P0_R6_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6_READONLY_CLAIMREG_P0_R6_READONLY_MAX (0xFFFFFFFFU)


/* PID_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MINOR_PROXY_MASK                  (0x0000003FU)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MINOR_PROXY_SHIFT                 (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MINOR_PROXY_MAX                   (0x0000003FU)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_CUSTOM_PROXY_MASK                 (0x000000C0U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_CUSTOM_PROXY_SHIFT                (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_CUSTOM_PROXY_MAX                  (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MAJOR_PROXY_MASK                  (0x00000700U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MAJOR_PROXY_SHIFT                 (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MAJOR_PROXY_MAX                   (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MISC_PROXY_MASK                   (0x0000F800U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MISC_PROXY_SHIFT                  (0x0000000BU)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MISC_PROXY_MAX                    (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MSB16_PROXY_MASK                  (0xFFFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MSB16_PROXY_SHIFT                 (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_PID_PROXY_PID_MSB16_PROXY_MAX                   (0x0000FFFFU)


/* MMR_CFG1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_MMR_CFG1_PARTITIONS_PROXY_MASK   (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_MMR_CFG1_PARTITIONS_PROXY_SHIFT  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_MMR_CFG1_PARTITIONS_PROXY_MAX    (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_MMR_CFG1_PROXY_EN_PROXY_MASK     (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_MMR_CFG1_PROXY_EN_PROXY_SHIFT    (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMR_CFG1_PROXY_MMR_CFG1_PROXY_EN_PROXY_MAX      (0x00000001U)


/* IPC_SET0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_PROXY_IPC_SET0_IPC_SET_PROXY_MASK      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_PROXY_IPC_SET0_IPC_SET_PROXY_SHIFT     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_PROXY_IPC_SET0_IPC_SET_PROXY_MAX       (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_PROXY_IPC_SET0_IPC_SRC_SET_PROXY_MASK  (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_PROXY_IPC_SET0_IPC_SRC_SET_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET0_PROXY_IPC_SET0_IPC_SRC_SET_PROXY_MAX   (0x0FFFFFFFU)


/* IPC_SET16_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_PROXY_IPC_SET16_IPC_SET_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_PROXY_IPC_SET16_IPC_SET_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_PROXY_IPC_SET16_IPC_SET_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_PROXY_IPC_SET16_IPC_SRC_SET_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_PROXY_IPC_SET16_IPC_SRC_SET_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET16_PROXY_IPC_SET16_IPC_SRC_SET_PROXY_MAX (0x0FFFFFFFU)


/* IPC_SET17_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_PROXY_IPC_SET17_IPC_SET_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_PROXY_IPC_SET17_IPC_SET_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_PROXY_IPC_SET17_IPC_SET_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_PROXY_IPC_SET17_IPC_SRC_SET_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_PROXY_IPC_SET17_IPC_SRC_SET_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET17_PROXY_IPC_SET17_IPC_SRC_SET_PROXY_MAX (0x0FFFFFFFU)


/* IPC_SET18_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_PROXY_IPC_SET18_IPC_SET_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_PROXY_IPC_SET18_IPC_SET_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_PROXY_IPC_SET18_IPC_SET_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_PROXY_IPC_SET18_IPC_SRC_SET_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_PROXY_IPC_SET18_IPC_SRC_SET_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET18_PROXY_IPC_SET18_IPC_SRC_SET_PROXY_MAX (0x0FFFFFFFU)


/* IPC_SET19_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_PROXY_IPC_SET19_IPC_SET_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_PROXY_IPC_SET19_IPC_SET_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_PROXY_IPC_SET19_IPC_SET_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_PROXY_IPC_SET19_IPC_SRC_SET_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_PROXY_IPC_SET19_IPC_SRC_SET_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_SET19_PROXY_IPC_SET19_IPC_SRC_SET_PROXY_MAX (0x0FFFFFFFU)


/* IPC_CLR0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_PROXY_IPC_CLR0_IPC_CLR_PROXY_MASK      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_PROXY_IPC_CLR0_IPC_CLR_PROXY_SHIFT     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_PROXY_IPC_CLR0_IPC_CLR_PROXY_MAX       (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_PROXY_IPC_CLR0_IPC_SRC_CLR_PROXY_MASK  (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_PROXY_IPC_CLR0_IPC_SRC_CLR_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR0_PROXY_IPC_CLR0_IPC_SRC_CLR_PROXY_MAX   (0x0FFFFFFFU)


/* IPC_CLR16_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_PROXY_IPC_CLR16_IPC_CLR_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_PROXY_IPC_CLR16_IPC_CLR_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_PROXY_IPC_CLR16_IPC_CLR_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_PROXY_IPC_CLR16_IPC_SRC_CLR_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_PROXY_IPC_CLR16_IPC_SRC_CLR_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR16_PROXY_IPC_CLR16_IPC_SRC_CLR_PROXY_MAX (0x0FFFFFFFU)


/* IPC_CLR17_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_PROXY_IPC_CLR17_IPC_CLR_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_PROXY_IPC_CLR17_IPC_CLR_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_PROXY_IPC_CLR17_IPC_CLR_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_PROXY_IPC_CLR17_IPC_SRC_CLR_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_PROXY_IPC_CLR17_IPC_SRC_CLR_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR17_PROXY_IPC_CLR17_IPC_SRC_CLR_PROXY_MAX (0x0FFFFFFFU)


/* IPC_CLR18_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_PROXY_IPC_CLR18_IPC_CLR_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_PROXY_IPC_CLR18_IPC_CLR_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_PROXY_IPC_CLR18_IPC_CLR_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_PROXY_IPC_CLR18_IPC_SRC_CLR_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_PROXY_IPC_CLR18_IPC_SRC_CLR_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR18_PROXY_IPC_CLR18_IPC_SRC_CLR_PROXY_MAX (0x0FFFFFFFU)


/* IPC_CLR19_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_PROXY_IPC_CLR19_IPC_CLR_PROXY_MASK    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_PROXY_IPC_CLR19_IPC_CLR_PROXY_SHIFT   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_PROXY_IPC_CLR19_IPC_CLR_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_PROXY_IPC_CLR19_IPC_SRC_CLR_PROXY_MASK (0xFFFFFFF0U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_PROXY_IPC_CLR19_IPC_SRC_CLR_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_IPC_CLR19_PROXY_IPC_CLR19_IPC_SRC_CLR_PROXY_MAX (0x0FFFFFFFU)


/* LOCK0_KICK0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0_PROXY_LOCK0_KICK0_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0_PROXY_LOCK0_KICK0_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK0_PROXY_LOCK0_KICK0_PROXY_MAX         (0xFFFFFFFFU)


/* LOCK0_KICK1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1_PROXY_LOCK0_KICK1_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1_PROXY_LOCK0_KICK1_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK0_KICK1_PROXY_LOCK0_KICK1_PROXY_MAX         (0xFFFFFFFFU)


/* INTR_RAW_STATUS_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_PROT_ERR_PROXY_MASK       (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_PROT_ERR_PROXY_SHIFT      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_PROT_ERR_PROXY_MAX        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_ADDR_ERR_PROXY_MASK       (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_ADDR_ERR_PROXY_SHIFT      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_ADDR_ERR_PROXY_MAX        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_KICK_ERR_PROXY_MASK       (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_KICK_ERR_PROXY_SHIFT      (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_KICK_ERR_PROXY_MAX        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_PROXY_ERR_PROXY_MASK      (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_PROXY_ERR_PROXY_SHIFT     (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_RAW_STATUS_PROXY_PROXY_ERR_PROXY_MAX       (0x00000001U)


/* INTR_ENABLED_STATUS_CLEAR_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_PROT_ERR_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_PROT_ERR_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_PROT_ERR_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_ADDR_ERR_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_ADDR_ERR_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_ADDR_ERR_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_KICK_ERR_PROXY_MASK (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_KICK_ERR_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_KICK_ERR_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_PROXY_ERR_PROXY_MASK (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_PROXY_ERR_PROXY_SHIFT (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLED_STATUS_CLEAR_PROXY_ENABLED_PROXY_ERR_PROXY_MAX (0x00000001U)


/* INTR_ENABLE_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_PROT_ERR_EN_PROXY_MASK        (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_PROT_ERR_EN_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_PROT_ERR_EN_PROXY_MAX         (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_ADDR_ERR_EN_PROXY_MASK        (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_ADDR_ERR_EN_PROXY_SHIFT       (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_ADDR_ERR_EN_PROXY_MAX         (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_KICK_ERR_EN_PROXY_MASK        (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_KICK_ERR_EN_PROXY_SHIFT       (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_KICK_ERR_EN_PROXY_MAX         (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_PROXY_ERR_EN_PROXY_MASK       (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_PROXY_ERR_EN_PROXY_SHIFT      (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_PROXY_PROXY_ERR_EN_PROXY_MAX        (0x00000001U)


/* INTR_ENABLE_CLEAR_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_PROT_ERR_EN_CLR_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_PROT_ERR_EN_CLR_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_PROT_ERR_EN_CLR_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_ADDR_ERR_EN_CLR_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_ADDR_ERR_EN_CLR_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_ADDR_ERR_EN_CLR_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_KICK_ERR_EN_CLR_PROXY_MASK (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_KICK_ERR_EN_CLR_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_KICK_ERR_EN_CLR_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_PROXY_ERR_EN_CLR_PROXY_MASK (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_PROXY_ERR_EN_CLR_PROXY_SHIFT (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_INTR_ENABLE_CLEAR_PROXY_PROXY_ERR_EN_CLR_PROXY_MAX (0x00000001U)


/* EOI_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_EOI_PROXY_EOI_VECTOR_PROXY_MASK                 (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_EOI_PROXY_EOI_VECTOR_PROXY_SHIFT                (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_EOI_PROXY_EOI_VECTOR_PROXY_MAX                  (0x000000FFU)


/* FAULT_ADDRESS_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS_PROXY_FAULT_ADDR_PROXY_MASK       (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS_PROXY_FAULT_ADDR_PROXY_SHIFT      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ADDRESS_PROXY_FAULT_ADDR_PROXY_MAX        (0xFFFFFFFFU)


/* FAULT_TYPE_STATUS_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_PROXY_FAULT_TYPE_PROXY_MASK   (0x0000003FU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_PROXY_FAULT_TYPE_PROXY_SHIFT  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_PROXY_FAULT_TYPE_PROXY_MAX    (0x0000003FU)

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_PROXY_FAULT_NS_PROXY_MASK     (0x00000040U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_PROXY_FAULT_NS_PROXY_SHIFT    (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_TYPE_STATUS_PROXY_FAULT_NS_PROXY_MAX      (0x00000001U)


/* FAULT_ATTR_STATUS_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_PRIVID_PROXY_MASK (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_PRIVID_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_PRIVID_PROXY_MAX  (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_ROUTEID_PROXY_MASK (0x000FFF00U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_ROUTEID_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_ROUTEID_PROXY_MAX (0x00000FFFU)

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_XID_PROXY_MASK    (0xFFF00000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_XID_PROXY_SHIFT   (0x00000014U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_ATTR_STATUS_PROXY_FAULT_XID_PROXY_MAX     (0x00000FFFU)


/* FAULT_CLEAR_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR_PROXY_FAULT_CLR_PROXY_MASK          (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR_PROXY_FAULT_CLR_PROXY_SHIFT         (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FAULT_CLEAR_PROXY_FAULT_CLR_PROXY_MAX           (0x00000001U)


/* CLAIMREG_P0_R0 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0_CLAIMREG_P0_R0_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0_CLAIMREG_P0_R0_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R0_CLAIMREG_P0_R0_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P0_R1 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1_CLAIMREG_P0_R1_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1_CLAIMREG_P0_R1_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R1_CLAIMREG_P0_R1_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P0_R2 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2_CLAIMREG_P0_R2_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2_CLAIMREG_P0_R2_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R2_CLAIMREG_P0_R2_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P0_R3 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3_CLAIMREG_P0_R3_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3_CLAIMREG_P0_R3_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R3_CLAIMREG_P0_R3_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P0_R4 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4_CLAIMREG_P0_R4_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4_CLAIMREG_P0_R4_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R4_CLAIMREG_P0_R4_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P0_R5 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5_CLAIMREG_P0_R5_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5_CLAIMREG_P0_R5_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R5_CLAIMREG_P0_R5_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P0_R6 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6_CLAIMREG_P0_R6_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6_CLAIMREG_P0_R6_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P0_R6_CLAIMREG_P0_R6_MAX               (0xFFFFFFFFU)


/* ENET1_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PORT_MODE_SEL_MASK                   (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PORT_MODE_SEL_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PORT_MODE_SEL_MAX                    (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_RGMII_ID_MODE_MASK                   (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_RGMII_ID_MODE_SHIFT                  (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_RGMII_ID_MODE_MAX                    (0x00000001U)


/* ENET2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PORT_MODE_SEL_MASK                   (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PORT_MODE_SEL_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PORT_MODE_SEL_MAX                    (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_RGMII_ID_MODE_MASK                   (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_RGMII_ID_MODE_SHIFT                  (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_RGMII_ID_MODE_MAX                    (0x00000001U)


/* EPWM_TB_CLKEN */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM0_TB_CLKEN_MASK               (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM0_TB_CLKEN_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM0_TB_CLKEN_MAX                (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM1_TB_CLKEN_MASK               (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM1_TB_CLKEN_SHIFT              (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM1_TB_CLKEN_MAX                (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM2_TB_CLKEN_MASK               (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM2_TB_CLKEN_SHIFT              (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_EPWM2_TB_CLKEN_MAX                (0x00000001U)


/* EPWM0_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_EALLOW_MASK                          (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_EALLOW_SHIFT                         (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_EALLOW_MAX                           (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_SYNCIN_SEL_MASK                      (0x00000700U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_SYNCIN_SEL_SHIFT                     (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_SYNCIN_SEL_MAX                       (0x00000007U)


/* EPWM1_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL_EALLOW_MASK                          (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL_EALLOW_SHIFT                         (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL_EALLOW_MAX                           (0x00000001U)


/* EPWM2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL_EALLOW_MASK                          (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL_EALLOW_SHIFT                         (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL_EALLOW_MAX                           (0x00000001U)


/* TIMER0_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL_CAP_SEL_MAX                         (0x00000007U)


/* TIMER1_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_CAP_SEL_MAX                         (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_CASCADE_EN_MASK                     (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_CASCADE_EN_SHIFT                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_CASCADE_EN_MAX                      (0x00000001U)


/* TIMER2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL_CAP_SEL_MAX                         (0x00000007U)


/* TIMER3_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_CAP_SEL_MAX                         (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_CASCADE_EN_MASK                     (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_CASCADE_EN_SHIFT                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_CASCADE_EN_MAX                      (0x00000001U)


/* TIMER4_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL_CAP_SEL_MAX                         (0x00000007U)


/* TIMER5_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_CAP_SEL_MAX                         (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_CASCADE_EN_MASK                     (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_CASCADE_EN_SHIFT                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_CASCADE_EN_MAX                      (0x00000001U)


/* TIMER6_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL_CAP_SEL_MAX                         (0x00000007U)


/* TIMER7_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_CAP_SEL_MAX                         (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_CASCADE_EN_MASK                     (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_CASCADE_EN_SHIFT                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_CASCADE_EN_MAX                      (0x00000001U)


/* TIMER8_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL_CAP_SEL_MAX                         (0x00000007U)


/* TIMER9_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_CAP_SEL_MASK                        (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_CAP_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_CAP_SEL_MAX                         (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_CASCADE_EN_MASK                     (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_CASCADE_EN_SHIFT                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_CASCADE_EN_MAX                      (0x00000001U)


/* TIMERIO0_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* TIMERIO1_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* TIMERIO2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* TIMERIO3_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* TIMERIO4_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* TIMERIO5_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* TIMERIO6_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* TIMERIO7_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL_OUT_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL_OUT_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL_OUT_SEL_MAX                       (0x0000000FU)


/* C7XV_CTRL0 */

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD0_MASK                            (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD0_SHIFT                           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD0_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD1_MASK                            (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD1_SHIFT                           (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD1_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD2_MASK                            (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD2_SHIFT                           (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD2_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD3_MASK                            (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD3_SHIFT                           (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD3_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD4_MASK                            (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD4_SHIFT                           (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD4_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD5_MASK                            (0x00000020U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD5_SHIFT                           (0x00000005U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD5_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD6_MASK                            (0x00000040U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD6_SHIFT                           (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD6_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD7_MASK                            (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD7_SHIFT                           (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD7_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD8_MASK                            (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD8_SHIFT                           (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD8_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD9_MASK                            (0x00000200U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD9_SHIFT                           (0x00000009U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD9_MAX                             (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD10_MASK                           (0x00000400U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD10_SHIFT                          (0x0000000AU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD10_MAX                            (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD11_MASK                           (0x00000800U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD11_SHIFT                          (0x0000000BU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD11_MAX                            (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD12_MASK                           (0x00001000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD12_SHIFT                          (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD12_MAX                            (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD13_MASK                           (0x00002000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD13_SHIFT                          (0x0000000DU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD13_MAX                            (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD14_MASK                           (0x00004000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD14_SHIFT                          (0x0000000EU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD14_MAX                            (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD15_MASK                           (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD15_SHIFT                          (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_ORD15_MAX                            (0x00000001U)


/* DPHY_TX0_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_LANE_FUNC_SEL_MASK                (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_LANE_FUNC_SEL_SHIFT               (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_LANE_FUNC_SEL_MAX                 (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_RET_EN_MASK                       (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_RET_EN_SHIFT                      (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_RET_EN_MAX                        (0x00000001U)


/* ADC0_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_TRIG_SEL_MASK                         (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_TRIG_SEL_SHIFT                        (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_TRIG_SEL_MAX                          (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_GPI_MODE_EN_MASK                      (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_GPI_MODE_EN_SHIFT                     (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_GPI_MODE_EN_MAX                       (0x00000001U)


/* ADC0_CAL */

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_CAL_CAP_SEL_MASK                       (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_CAL_CAP_SEL_SHIFT                      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_CAL_CAP_SEL_MAX                        (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_DIFF_CAL_EN_MASK                       (0x00004000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_DIFF_CAL_EN_SHIFT                      (0x0000000EU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_DIFF_CAL_EN_MAX                        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_CAL_EN_MASK                            (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_CAL_EN_SHIFT                           (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_CAL_EN_MAX                             (0x00000001U)


/* FSS0_CTRL0 */

#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0_S0_BOOT_SEG_MASK                     (0x000FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0_S0_BOOT_SEG_SHIFT                    (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0_S0_BOOT_SEG_MAX                      (0x000FFFFFU)


/* FSS0_CTRL1 */

#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1_S0_BOOT_MASK_MASK                    (0x000FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1_S0_BOOT_MASK_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1_S0_BOOT_MASK_MAX                     (0x000FFFFFU)


/* DCC_STAT */

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC0_INTR_DONE_MASK                    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC0_INTR_DONE_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC0_INTR_DONE_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC1_INTR_DONE_MASK                    (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC1_INTR_DONE_SHIFT                   (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC1_INTR_DONE_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC2_INTR_DONE_MASK                    (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC2_INTR_DONE_SHIFT                   (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC2_INTR_DONE_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC3_INTR_DONE_MASK                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC3_INTR_DONE_SHIFT                   (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC3_INTR_DONE_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC4_INTR_DONE_MASK                    (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC4_INTR_DONE_SHIFT                   (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC4_INTR_DONE_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC5_INTR_DONE_MASK                    (0x00000020U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC5_INTR_DONE_SHIFT                   (0x00000005U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC5_INTR_DONE_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC6_INTR_DONE_MASK                    (0x00000040U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC6_INTR_DONE_SHIFT                   (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_DCC6_INTR_DONE_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_MCU_DCC0_INTR_DONE_MASK                (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_MCU_DCC0_INTR_DONE_SHIFT               (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_MCU_DCC0_INTR_DONE_MAX                 (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_MCU_DCC1_INTR_DONE_MASK                (0x00020000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_MCU_DCC1_INTR_DONE_SHIFT               (0x00000011U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_MCU_DCC1_INTR_DONE_MAX                 (0x00000001U)


/* R5SS0_CORE0_RL2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL_RTXIP_SEL_MASK             (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL_RTXIP_SEL_SHIFT            (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL_RTXIP_SEL_MAX              (0x00000003U)


/* R5SS0_CORE1_RL2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL_RTXIP_SEL_MASK             (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL_RTXIP_SEL_SHIFT            (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL_RTXIP_SEL_MAX              (0x00000003U)


/* R5SS1_CORE0_RL2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL_RTXIP_SEL_MASK             (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL_RTXIP_SEL_SHIFT            (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL_RTXIP_SEL_MAX              (0x00000003U)


/* R5SS1_CORE1_RL2_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL_RTXIP_SEL_MASK             (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL_RTXIP_SEL_SHIFT            (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL_RTXIP_SEL_MAX              (0x00000003U)


/* TPCC_PARITY_STATUS */

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS_TPCC_A_PARITY_ADDR_MASK      (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS_TPCC_A_PARITY_ADDR_SHIFT     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS_TPCC_A_PARITY_ADDR_MAX       (0x000000FFU)


/* TPTC_XID_REORDER_CFG0 */

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0_DSS_TPTC_XID_REORDERING_DISABLE_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0_DSS_TPTC_XID_REORDERING_DISABLE_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0_DSS_TPTC_XID_REORDERING_DISABLE_MAX (0x00000003U)


/* TPTC_DBS_CFG0 */

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_TPTC_A0_DBS_TIEOFF_MASK           (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_TPTC_A0_DBS_TIEOFF_SHIFT          (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_TPTC_A0_DBS_TIEOFF_MAX            (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_TPTC_A1_DBS_TIEOFF_MASK           (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_TPTC_A1_DBS_TIEOFF_SHIFT          (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_TPTC_A1_DBS_TIEOFF_MAX            (0x00000003U)


/* TPCC_MEMINIT_START */

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_MASK         (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_SHIFT        (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_MAX          (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_DONE_MASK    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_DONE_SHIFT   (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_DONE_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_STATUS_MASK  (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_STATUS_SHIFT (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_STATUS_MAX   (0x00000001U)


/* TPCC_PARITY_CTRL0 */

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_TPCC_A_PARITY_EN_MASK         (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_TPCC_A_PARITY_EN_SHIFT        (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_TPCC_A_PARITY_EN_MAX          (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_TPCC_A_PARITY_TESTEN_MASK     (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_TPCC_A_PARITY_TESTEN_SHIFT    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_TPCC_A_PARITY_TESTEN_MAX      (0x00000001U)


/* TPTC_BOUNDARY_CFG0 */

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_DSS_TPTC0_BOUNDARY_SIZE_MASK (0x0000003FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_DSS_TPTC0_BOUNDARY_SIZE_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_DSS_TPTC0_BOUNDARY_SIZE_MAX  (0x0000003FU)

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_DSS_TPTC1_BOUNDARY_SIZE_MASK (0x00003F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_DSS_TPTC1_BOUNDARY_SIZE_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_DSS_TPTC1_BOUNDARY_SIZE_MAX  (0x0000003FU)


/* EDMA_CLK_GATE */

#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPTCA0_EDMA_CLK_GATE_MASK         (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPTCA0_EDMA_CLK_GATE_SHIFT        (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPTCA0_EDMA_CLK_GATE_MAX          (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPTCA1_EDMA_CLK_GATE_MASK         (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPTCA1_EDMA_CLK_GATE_SHIFT        (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPTCA1_EDMA_CLK_GATE_MAX          (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPCCA_EDMA_CLK_GATE_MASK          (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPCCA_EDMA_CLK_GATE_SHIFT         (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_TPCCA_EDMA_CLK_GATE_MAX           (0x00000001U)


/* LOCK1_KICK0 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0_LOCK1_KICK0_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0_LOCK1_KICK0_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0_LOCK1_KICK0_MAX                     (0xFFFFFFFFU)


/* LOCK1_KICK1 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1_LOCK1_KICK1_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1_LOCK1_KICK1_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1_LOCK1_KICK1_MAX                     (0xFFFFFFFFU)


/* CLAIMREG_P1_R0_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0_READONLY_CLAIMREG_P1_R0_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0_READONLY_CLAIMREG_P1_R0_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0_READONLY_CLAIMREG_P1_R0_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R1_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1_READONLY_CLAIMREG_P1_R1_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1_READONLY_CLAIMREG_P1_R1_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1_READONLY_CLAIMREG_P1_R1_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R2_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2_READONLY_CLAIMREG_P1_R2_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2_READONLY_CLAIMREG_P1_R2_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2_READONLY_CLAIMREG_P1_R2_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R3_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3_READONLY_CLAIMREG_P1_R3_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3_READONLY_CLAIMREG_P1_R3_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3_READONLY_CLAIMREG_P1_R3_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R4_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4_READONLY_CLAIMREG_P1_R4_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4_READONLY_CLAIMREG_P1_R4_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4_READONLY_CLAIMREG_P1_R4_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R5_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5_READONLY_CLAIMREG_P1_R5_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5_READONLY_CLAIMREG_P1_R5_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5_READONLY_CLAIMREG_P1_R5_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R6_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6_READONLY_CLAIMREG_P1_R6_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6_READONLY_CLAIMREG_P1_R6_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6_READONLY_CLAIMREG_P1_R6_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R7_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7_READONLY_CLAIMREG_P1_R7_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7_READONLY_CLAIMREG_P1_R7_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7_READONLY_CLAIMREG_P1_R7_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R8_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8_READONLY_CLAIMREG_P1_R8_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8_READONLY_CLAIMREG_P1_R8_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8_READONLY_CLAIMREG_P1_R8_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R9_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9_READONLY_CLAIMREG_P1_R9_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9_READONLY_CLAIMREG_P1_R9_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9_READONLY_CLAIMREG_P1_R9_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R10_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10_READONLY_CLAIMREG_P1_R10_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10_READONLY_CLAIMREG_P1_R10_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10_READONLY_CLAIMREG_P1_R10_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R11_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11_READONLY_CLAIMREG_P1_R11_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11_READONLY_CLAIMREG_P1_R11_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11_READONLY_CLAIMREG_P1_R11_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R12_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12_READONLY_CLAIMREG_P1_R12_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12_READONLY_CLAIMREG_P1_R12_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12_READONLY_CLAIMREG_P1_R12_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R13_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13_READONLY_CLAIMREG_P1_R13_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13_READONLY_CLAIMREG_P1_R13_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13_READONLY_CLAIMREG_P1_R13_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R14_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14_READONLY_CLAIMREG_P1_R14_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14_READONLY_CLAIMREG_P1_R14_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14_READONLY_CLAIMREG_P1_R14_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R15_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15_READONLY_CLAIMREG_P1_R15_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15_READONLY_CLAIMREG_P1_R15_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15_READONLY_CLAIMREG_P1_R15_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R16_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16_READONLY_CLAIMREG_P1_R16_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16_READONLY_CLAIMREG_P1_R16_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16_READONLY_CLAIMREG_P1_R16_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R17_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17_READONLY_CLAIMREG_P1_R17_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17_READONLY_CLAIMREG_P1_R17_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17_READONLY_CLAIMREG_P1_R17_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R18_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18_READONLY_CLAIMREG_P1_R18_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18_READONLY_CLAIMREG_P1_R18_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18_READONLY_CLAIMREG_P1_R18_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R19_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19_READONLY_CLAIMREG_P1_R19_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19_READONLY_CLAIMREG_P1_R19_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19_READONLY_CLAIMREG_P1_R19_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P1_R20_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20_READONLY_CLAIMREG_P1_R20_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20_READONLY_CLAIMREG_P1_R20_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20_READONLY_CLAIMREG_P1_R20_READONLY_MAX (0xFFFFFFFFU)


/* ENET1_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PROXY_ENET1_CTRL_PORT_MODE_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PROXY_ENET1_CTRL_PORT_MODE_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PROXY_ENET1_CTRL_PORT_MODE_SEL_PROXY_MAX (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PROXY_ENET1_CTRL_RGMII_ID_MODE_PROXY_MASK (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PROXY_ENET1_CTRL_RGMII_ID_MODE_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET1_CTRL_PROXY_ENET1_CTRL_RGMII_ID_MODE_PROXY_MAX (0x00000001U)


/* ENET2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PROXY_ENET2_CTRL_PORT_MODE_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PROXY_ENET2_CTRL_PORT_MODE_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PROXY_ENET2_CTRL_PORT_MODE_SEL_PROXY_MAX (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PROXY_ENET2_CTRL_RGMII_ID_MODE_PROXY_MASK (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PROXY_ENET2_CTRL_RGMII_ID_MODE_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_ENET2_CTRL_PROXY_ENET2_CTRL_RGMII_ID_MODE_PROXY_MAX (0x00000001U)


/* EPWM_TB_CLKEN_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM0_TB_CLKEN_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM0_TB_CLKEN_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM0_TB_CLKEN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM1_TB_CLKEN_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM1_TB_CLKEN_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM1_TB_CLKEN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM2_TB_CLKEN_PROXY_MASK (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM2_TB_CLKEN_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM_TB_CLKEN_PROXY_EPWM_TB_CLKEN_EPWM2_TB_CLKEN_PROXY_MAX (0x00000001U)


/* EPWM0_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_PROXY_EPWM0_CTRL_EALLOW_PROXY_MASK   (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_PROXY_EPWM0_CTRL_EALLOW_PROXY_SHIFT  (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_PROXY_EPWM0_CTRL_EALLOW_PROXY_MAX    (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_PROXY_EPWM0_CTRL_SYNCIN_SEL_PROXY_MASK (0x00000700U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_PROXY_EPWM0_CTRL_SYNCIN_SEL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM0_CTRL_PROXY_EPWM0_CTRL_SYNCIN_SEL_PROXY_MAX (0x00000007U)


/* EPWM1_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL_PROXY_EPWM1_CTRL_EALLOW_PROXY_MASK   (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL_PROXY_EPWM1_CTRL_EALLOW_PROXY_SHIFT  (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM1_CTRL_PROXY_EPWM1_CTRL_EALLOW_PROXY_MAX    (0x00000001U)


/* EPWM2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL_PROXY_EPWM2_CTRL_EALLOW_PROXY_MASK   (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL_PROXY_EPWM2_CTRL_EALLOW_PROXY_SHIFT  (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EPWM2_CTRL_PROXY_EPWM2_CTRL_EALLOW_PROXY_MAX    (0x00000001U)


/* TIMER0_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL_PROXY_TIMER0_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL_PROXY_TIMER0_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CTRL_PROXY_TIMER0_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)


/* TIMER1_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_PROXY_TIMER1_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_PROXY_TIMER1_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_PROXY_TIMER1_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_PROXY_TIMER1_CTRL_CASCADE_EN_PROXY_MASK (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_PROXY_TIMER1_CTRL_CASCADE_EN_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CTRL_PROXY_TIMER1_CTRL_CASCADE_EN_PROXY_MAX (0x00000001U)


/* TIMER2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL_PROXY_TIMER2_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL_PROXY_TIMER2_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CTRL_PROXY_TIMER2_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)


/* TIMER3_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_PROXY_TIMER3_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_PROXY_TIMER3_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_PROXY_TIMER3_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_PROXY_TIMER3_CTRL_CASCADE_EN_PROXY_MASK (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_PROXY_TIMER3_CTRL_CASCADE_EN_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CTRL_PROXY_TIMER3_CTRL_CASCADE_EN_PROXY_MAX (0x00000001U)


/* TIMER4_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL_PROXY_TIMER4_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL_PROXY_TIMER4_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CTRL_PROXY_TIMER4_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)


/* TIMER5_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_PROXY_TIMER5_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_PROXY_TIMER5_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_PROXY_TIMER5_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_PROXY_TIMER5_CTRL_CASCADE_EN_PROXY_MASK (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_PROXY_TIMER5_CTRL_CASCADE_EN_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CTRL_PROXY_TIMER5_CTRL_CASCADE_EN_PROXY_MAX (0x00000001U)


/* TIMER6_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL_PROXY_TIMER6_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL_PROXY_TIMER6_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CTRL_PROXY_TIMER6_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)


/* TIMER7_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_PROXY_TIMER7_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_PROXY_TIMER7_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_PROXY_TIMER7_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_PROXY_TIMER7_CTRL_CASCADE_EN_PROXY_MASK (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_PROXY_TIMER7_CTRL_CASCADE_EN_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CTRL_PROXY_TIMER7_CTRL_CASCADE_EN_PROXY_MAX (0x00000001U)


/* TIMER8_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL_PROXY_TIMER8_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL_PROXY_TIMER8_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CTRL_PROXY_TIMER8_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)


/* TIMER9_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_PROXY_TIMER9_CTRL_CAP_SEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_PROXY_TIMER9_CTRL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_PROXY_TIMER9_CTRL_CAP_SEL_PROXY_MAX (0x00000007U)

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_PROXY_TIMER9_CTRL_CASCADE_EN_PROXY_MASK (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_PROXY_TIMER9_CTRL_CASCADE_EN_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CTRL_PROXY_TIMER9_CTRL_CASCADE_EN_PROXY_MAX (0x00000001U)


/* TIMERIO0_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL_PROXY_TIMERIO0_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL_PROXY_TIMERIO0_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO0_CTRL_PROXY_TIMERIO0_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* TIMERIO1_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL_PROXY_TIMERIO1_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL_PROXY_TIMERIO1_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO1_CTRL_PROXY_TIMERIO1_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* TIMERIO2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL_PROXY_TIMERIO2_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL_PROXY_TIMERIO2_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO2_CTRL_PROXY_TIMERIO2_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* TIMERIO3_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL_PROXY_TIMERIO3_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL_PROXY_TIMERIO3_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO3_CTRL_PROXY_TIMERIO3_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* TIMERIO4_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL_PROXY_TIMERIO4_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL_PROXY_TIMERIO4_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO4_CTRL_PROXY_TIMERIO4_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* TIMERIO5_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL_PROXY_TIMERIO5_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL_PROXY_TIMERIO5_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO5_CTRL_PROXY_TIMERIO5_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* TIMERIO6_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL_PROXY_TIMERIO6_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL_PROXY_TIMERIO6_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO6_CTRL_PROXY_TIMERIO6_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* TIMERIO7_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL_PROXY_TIMERIO7_CTRL_OUT_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL_PROXY_TIMERIO7_CTRL_OUT_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMERIO7_CTRL_PROXY_TIMERIO7_CTRL_OUT_SEL_PROXY_MAX (0x0000000FU)


/* C7XV_CTRL0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD0_PROXY_MASK     (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD0_PROXY_SHIFT    (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD0_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD1_PROXY_MASK     (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD1_PROXY_SHIFT    (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD1_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD2_PROXY_MASK     (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD2_PROXY_SHIFT    (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD2_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD3_PROXY_MASK     (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD3_PROXY_SHIFT    (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD3_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD4_PROXY_MASK     (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD4_PROXY_SHIFT    (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD4_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD5_PROXY_MASK     (0x00000020U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD5_PROXY_SHIFT    (0x00000005U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD5_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD6_PROXY_MASK     (0x00000040U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD6_PROXY_SHIFT    (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD6_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD7_PROXY_MASK     (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD7_PROXY_SHIFT    (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD7_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD8_PROXY_MASK     (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD8_PROXY_SHIFT    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD8_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD9_PROXY_MASK     (0x00000200U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD9_PROXY_SHIFT    (0x00000009U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD9_PROXY_MAX      (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD10_PROXY_MASK    (0x00000400U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD10_PROXY_SHIFT   (0x0000000AU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD10_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD11_PROXY_MASK    (0x00000800U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD11_PROXY_SHIFT   (0x0000000BU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD11_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD12_PROXY_MASK    (0x00001000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD12_PROXY_SHIFT   (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD12_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD13_PROXY_MASK    (0x00002000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD13_PROXY_SHIFT   (0x0000000DU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD13_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD14_PROXY_MASK    (0x00004000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD14_PROXY_SHIFT   (0x0000000EU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD14_PROXY_MAX     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD15_PROXY_MASK    (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD15_PROXY_SHIFT   (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C7XV_CTRL0_PROXY_C7XV_CTRL0_ORD15_PROXY_MAX     (0x00000001U)


/* DPHY_TX0_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_PROXY_DPHY_TX0_CTRL_LANE_FUNC_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_PROXY_DPHY_TX0_CTRL_LANE_FUNC_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_PROXY_DPHY_TX0_CTRL_LANE_FUNC_SEL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_PROXY_DPHY_TX0_CTRL_RET_EN_PROXY_MASK (0x00000100U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_PROXY_DPHY_TX0_CTRL_RET_EN_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY_TX0_CTRL_PROXY_DPHY_TX0_CTRL_RET_EN_PROXY_MAX (0x00000001U)


/* ADC0_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_PROXY_ADC0_CTRL_TRIG_SEL_PROXY_MASK   (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_PROXY_ADC0_CTRL_TRIG_SEL_PROXY_SHIFT  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_PROXY_ADC0_CTRL_TRIG_SEL_PROXY_MAX    (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_PROXY_ADC0_CTRL_GPI_MODE_EN_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_PROXY_ADC0_CTRL_GPI_MODE_EN_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CTRL_PROXY_ADC0_CTRL_GPI_MODE_EN_PROXY_MAX (0x00000001U)


/* ADC0_CAL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_CAL_CAP_SEL_PROXY_MASK  (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_CAL_CAP_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_CAL_CAP_SEL_PROXY_MAX   (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_DIFF_CAL_EN_PROXY_MASK  (0x00004000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_DIFF_CAL_EN_PROXY_SHIFT (0x0000000EU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_DIFF_CAL_EN_PROXY_MAX   (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_CAL_EN_PROXY_MASK       (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_CAL_EN_PROXY_SHIFT      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CAL_PROXY_ADC0_CAL_CAL_EN_PROXY_MAX        (0x00000001U)


/* FSS0_CTRL0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0_PROXY_FSS0_CTRL0_S0_BOOT_SEG_PROXY_MASK (0x000FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0_PROXY_FSS0_CTRL0_S0_BOOT_SEG_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL0_PROXY_FSS0_CTRL0_S0_BOOT_SEG_PROXY_MAX (0x000FFFFFU)


/* FSS0_CTRL1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1_PROXY_FSS0_CTRL1_S0_BOOT_MASK_PROXY_MASK (0x000FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1_PROXY_FSS0_CTRL1_S0_BOOT_MASK_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_FSS0_CTRL1_PROXY_FSS0_CTRL1_S0_BOOT_MASK_PROXY_MAX (0x000FFFFFU)


/* DCC_STAT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC0_INTR_DONE_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC0_INTR_DONE_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC0_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC1_INTR_DONE_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC1_INTR_DONE_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC1_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC2_INTR_DONE_PROXY_MASK (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC2_INTR_DONE_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC2_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC3_INTR_DONE_PROXY_MASK (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC3_INTR_DONE_PROXY_SHIFT (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC3_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC4_INTR_DONE_PROXY_MASK (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC4_INTR_DONE_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC4_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC5_INTR_DONE_PROXY_MASK (0x00000020U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC5_INTR_DONE_PROXY_SHIFT (0x00000005U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC5_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC6_INTR_DONE_PROXY_MASK (0x00000040U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC6_INTR_DONE_PROXY_SHIFT (0x00000006U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_DCC6_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_MCU_DCC0_INTR_DONE_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_MCU_DCC0_INTR_DONE_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_MCU_DCC0_INTR_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_MCU_DCC1_INTR_DONE_PROXY_MASK (0x00020000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_MCU_DCC1_INTR_DONE_PROXY_SHIFT (0x00000011U)
#define CSL_MAIN_CTRL_MMR_CFG0_DCC_STAT_PROXY_DCC_STAT_MCU_DCC1_INTR_DONE_PROXY_MAX (0x00000001U)


/* R5SS0_CORE0_RL2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL_PROXY_R5SS0_CORE0_RL2_CTRL_RTXIP_SEL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL_PROXY_R5SS0_CORE0_RL2_CTRL_RTXIP_SEL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE0_RL2_CTRL_PROXY_R5SS0_CORE0_RL2_CTRL_RTXIP_SEL_PROXY_MAX (0x00000003U)


/* R5SS0_CORE1_RL2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL_PROXY_R5SS0_CORE1_RL2_CTRL_RTXIP_SEL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL_PROXY_R5SS0_CORE1_RL2_CTRL_RTXIP_SEL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_CORE1_RL2_CTRL_PROXY_R5SS0_CORE1_RL2_CTRL_RTXIP_SEL_PROXY_MAX (0x00000003U)


/* R5SS1_CORE0_RL2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL_PROXY_R5SS1_CORE0_RL2_CTRL_RTXIP_SEL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL_PROXY_R5SS1_CORE0_RL2_CTRL_RTXIP_SEL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE0_RL2_CTRL_PROXY_R5SS1_CORE0_RL2_CTRL_RTXIP_SEL_PROXY_MAX (0x00000003U)


/* R5SS1_CORE1_RL2_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL_PROXY_R5SS1_CORE1_RL2_CTRL_RTXIP_SEL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL_PROXY_R5SS1_CORE1_RL2_CTRL_RTXIP_SEL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_CORE1_RL2_CTRL_PROXY_R5SS1_CORE1_RL2_CTRL_RTXIP_SEL_PROXY_MAX (0x00000003U)


/* TPCC_PARITY_STATUS_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS_PROXY_TPCC_PARITY_STATUS_TPCC_A_PARITY_ADDR_PROXY_MASK (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS_PROXY_TPCC_PARITY_STATUS_TPCC_A_PARITY_ADDR_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_STATUS_PROXY_TPCC_PARITY_STATUS_TPCC_A_PARITY_ADDR_PROXY_MAX (0x000000FFU)


/* TPTC_XID_REORDER_CFG0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0_PROXY_TPTC_XID_REORDER_CFG0_DSS_TPTC_XID_REORDERING_DISABLE_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0_PROXY_TPTC_XID_REORDER_CFG0_DSS_TPTC_XID_REORDERING_DISABLE_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_XID_REORDER_CFG0_PROXY_TPTC_XID_REORDER_CFG0_DSS_TPTC_XID_REORDERING_DISABLE_PROXY_MAX (0x00000003U)


/* TPTC_DBS_CFG0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_PROXY_TPTC_DBS_CFG0_TPTC_A0_DBS_TIEOFF_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_PROXY_TPTC_DBS_CFG0_TPTC_A0_DBS_TIEOFF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_PROXY_TPTC_DBS_CFG0_TPTC_A0_DBS_TIEOFF_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_PROXY_TPTC_DBS_CFG0_TPTC_A1_DBS_TIEOFF_PROXY_MASK (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_PROXY_TPTC_DBS_CFG0_TPTC_A1_DBS_TIEOFF_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_DBS_CFG0_PROXY_TPTC_DBS_CFG0_TPTC_A1_DBS_TIEOFF_PROXY_MAX (0x00000003U)


/* TPCC_MEMINIT_START_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_DONE_PROXY_MASK (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_DONE_PROXY_SHIFT (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_DONE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_STATUS_PROXY_MASK (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_STATUS_PROXY_SHIFT (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_MEMINIT_START_PROXY_TPCC_MEMINIT_START_TPCC_A_MEM_INIT_STATUS_PROXY_MAX (0x00000001U)


/* TPCC_PARITY_CTRL0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_PROXY_TPCC_PARITY_CTRL0_TPCC_A_PARITY_EN_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_PROXY_TPCC_PARITY_CTRL0_TPCC_A_PARITY_EN_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_PROXY_TPCC_PARITY_CTRL0_TPCC_A_PARITY_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_PROXY_TPCC_PARITY_CTRL0_TPCC_A_PARITY_TESTEN_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_PROXY_TPCC_PARITY_CTRL0_TPCC_A_PARITY_TESTEN_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPCC_PARITY_CTRL0_PROXY_TPCC_PARITY_CTRL0_TPCC_A_PARITY_TESTEN_PROXY_MAX (0x00000001U)


/* TPTC_BOUNDARY_CFG0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_PROXY_TPTC_BOUNDARY_CFG0_DSS_TPTC0_BOUNDARY_SIZE_PROXY_MASK (0x0000003FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_PROXY_TPTC_BOUNDARY_CFG0_DSS_TPTC0_BOUNDARY_SIZE_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_PROXY_TPTC_BOUNDARY_CFG0_DSS_TPTC0_BOUNDARY_SIZE_PROXY_MAX (0x0000003FU)

#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_PROXY_TPTC_BOUNDARY_CFG0_DSS_TPTC1_BOUNDARY_SIZE_PROXY_MASK (0x00003F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_PROXY_TPTC_BOUNDARY_CFG0_DSS_TPTC1_BOUNDARY_SIZE_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_TPTC_BOUNDARY_CFG0_PROXY_TPTC_BOUNDARY_CFG0_DSS_TPTC1_BOUNDARY_SIZE_PROXY_MAX (0x0000003FU)


/* EDMA_CLK_GATE_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPTCA0_EDMA_CLK_GATE_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPTCA0_EDMA_CLK_GATE_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPTCA0_EDMA_CLK_GATE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPTCA1_EDMA_CLK_GATE_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPTCA1_EDMA_CLK_GATE_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPTCA1_EDMA_CLK_GATE_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPCCA_EDMA_CLK_GATE_PROXY_MASK (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPCCA_EDMA_CLK_GATE_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_EDMA_CLK_GATE_PROXY_EDMA_CLK_GATE_TPCCA_EDMA_CLK_GATE_PROXY_MAX (0x00000001U)


/* LOCK1_KICK0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0_PROXY_LOCK1_KICK0_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0_PROXY_LOCK1_KICK0_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK0_PROXY_LOCK1_KICK0_PROXY_MAX         (0xFFFFFFFFU)


/* LOCK1_KICK1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1_PROXY_LOCK1_KICK1_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1_PROXY_LOCK1_KICK1_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK1_KICK1_PROXY_LOCK1_KICK1_PROXY_MAX         (0xFFFFFFFFU)


/* CLAIMREG_P1_R0 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0_CLAIMREG_P1_R0_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0_CLAIMREG_P1_R0_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R0_CLAIMREG_P1_R0_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R1 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1_CLAIMREG_P1_R1_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1_CLAIMREG_P1_R1_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R1_CLAIMREG_P1_R1_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R2 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2_CLAIMREG_P1_R2_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2_CLAIMREG_P1_R2_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R2_CLAIMREG_P1_R2_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R3 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3_CLAIMREG_P1_R3_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3_CLAIMREG_P1_R3_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R3_CLAIMREG_P1_R3_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R4 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4_CLAIMREG_P1_R4_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4_CLAIMREG_P1_R4_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R4_CLAIMREG_P1_R4_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R5 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5_CLAIMREG_P1_R5_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5_CLAIMREG_P1_R5_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R5_CLAIMREG_P1_R5_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R6 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6_CLAIMREG_P1_R6_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6_CLAIMREG_P1_R6_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R6_CLAIMREG_P1_R6_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R7 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7_CLAIMREG_P1_R7_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7_CLAIMREG_P1_R7_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R7_CLAIMREG_P1_R7_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R8 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8_CLAIMREG_P1_R8_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8_CLAIMREG_P1_R8_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R8_CLAIMREG_P1_R8_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R9 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9_CLAIMREG_P1_R9_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9_CLAIMREG_P1_R9_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R9_CLAIMREG_P1_R9_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P1_R10 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10_CLAIMREG_P1_R10_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10_CLAIMREG_P1_R10_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R10_CLAIMREG_P1_R10_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R11 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11_CLAIMREG_P1_R11_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11_CLAIMREG_P1_R11_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R11_CLAIMREG_P1_R11_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R12 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12_CLAIMREG_P1_R12_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12_CLAIMREG_P1_R12_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R12_CLAIMREG_P1_R12_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R13 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13_CLAIMREG_P1_R13_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13_CLAIMREG_P1_R13_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R13_CLAIMREG_P1_R13_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R14 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14_CLAIMREG_P1_R14_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14_CLAIMREG_P1_R14_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R14_CLAIMREG_P1_R14_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R15 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15_CLAIMREG_P1_R15_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15_CLAIMREG_P1_R15_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R15_CLAIMREG_P1_R15_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R16 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16_CLAIMREG_P1_R16_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16_CLAIMREG_P1_R16_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R16_CLAIMREG_P1_R16_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R17 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17_CLAIMREG_P1_R17_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17_CLAIMREG_P1_R17_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R17_CLAIMREG_P1_R17_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R18 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18_CLAIMREG_P1_R18_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18_CLAIMREG_P1_R18_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R18_CLAIMREG_P1_R18_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R19 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19_CLAIMREG_P1_R19_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19_CLAIMREG_P1_R19_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R19_CLAIMREG_P1_R19_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P1_R20 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20_CLAIMREG_P1_R20_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20_CLAIMREG_P1_R20_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P1_R20_CLAIMREG_P1_R20_MAX             (0xFFFFFFFFU)


/* OBSCLK0_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_SEL_MASK                       (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_SEL_SHIFT                      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_SEL_MAX                        (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_DIV_MASK                       (0x0000FF00U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_DIV_SHIFT                      (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_DIV_MAX                        (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_DIV_LD_MASK                    (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_DIV_LD_SHIFT                   (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_CLK_DIV_LD_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_OUT_MUX_SEL_MASK                   (0x01000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_OUT_MUX_SEL_SHIFT                  (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_OUT_MUX_SEL_MAX                    (0x00000001U)


/* CLKOUT_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL_CLK_SEL_MASK                        (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL_CLK_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL_CLK_SEL_MAX                         (0x00000001U)


/* MAIN_PLL0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_BYP_WARM_RST_MASK              (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_BYP_WARM_RST_SHIFT             (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_BYP_WARM_RST_MAX               (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_BYPASS_SW_OVRD_MASK            (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_BYPASS_SW_OVRD_SHIFT           (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_BYPASS_SW_OVRD_MAX             (0x00000001U)


/* MAIN_PLL1_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_BYP_WARM_RST_MASK              (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_BYP_WARM_RST_SHIFT             (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_BYP_WARM_RST_MAX               (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_BYPASS_SW_OVRD_MASK            (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_BYPASS_SW_OVRD_SHIFT           (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_BYPASS_SW_OVRD_MAX             (0x00000001U)


/* MAIN_PLL2_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_BYP_WARM_RST_MASK              (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_BYP_WARM_RST_SHIFT             (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_BYP_WARM_RST_MAX               (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_BYPASS_SW_OVRD_MASK            (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_BYPASS_SW_OVRD_SHIFT           (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_BYPASS_SW_OVRD_MAX             (0x00000001U)


/* MAIN_PLL7_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_BYP_WARM_RST_MASK              (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_BYP_WARM_RST_SHIFT             (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_BYP_WARM_RST_MAX               (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_BYPASS_SW_OVRD_MASK            (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_BYPASS_SW_OVRD_SHIFT           (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_BYPASS_SW_OVRD_MAX             (0x00000001U)


/* MAIN_PLL14_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_BYP_WARM_RST_MASK             (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_BYP_WARM_RST_SHIFT            (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_BYP_WARM_RST_MAX              (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_BYPASS_SW_OVRD_MASK           (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_BYPASS_SW_OVRD_SHIFT          (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_BYPASS_SW_OVRD_MAX            (0x00000001U)


/* CPSW_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL_CPTS_CLKSEL_MASK                    (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL_CPTS_CLKSEL_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL_CPTS_CLKSEL_MAX                     (0x00000007U)


/* TIMER0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER1_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER2_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER3_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER4_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER5_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER6_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER7_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER8_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* TIMER9_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL_CLK_SEL_MASK                      (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL_CLK_SEL_MAX                       (0x0000000FU)


/* SPI0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL_MSTR_LB_CLKSEL_MASK                 (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL_MSTR_LB_CLKSEL_SHIFT                (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL_MSTR_LB_CLKSEL_MAX                  (0x00000001U)


/* SPI1_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL_MSTR_LB_CLKSEL_MASK                 (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL_MSTR_LB_CLKSEL_SHIFT                (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL_MSTR_LB_CLKSEL_MAX                  (0x00000001U)


/* SPI2_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL_MSTR_LB_CLKSEL_MASK                 (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL_MSTR_LB_CLKSEL_SHIFT                (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL_MSTR_LB_CLKSEL_MAX                  (0x00000001U)


/* USART0_CLK_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_CLK_DIV_MASK                    (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_CLK_DIV_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_CLK_DIV_MAX                     (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_CLK_DIV_LD_MASK                 (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_CLK_DIV_LD_SHIFT                (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_CLK_DIV_LD_MAX                  (0x00000001U)


/* USART1_CLK_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_CLK_DIV_MASK                    (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_CLK_DIV_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_CLK_DIV_MAX                     (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_CLK_DIV_LD_MASK                 (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_CLK_DIV_LD_SHIFT                (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_CLK_DIV_LD_MAX                  (0x00000001U)


/* USART2_CLK_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_CLK_DIV_MASK                    (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_CLK_DIV_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_CLK_DIV_MAX                     (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_CLK_DIV_LD_MASK                 (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_CLK_DIV_LD_SHIFT                (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_CLK_DIV_LD_MAX                  (0x00000001U)


/* USART3_CLK_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_CLK_DIV_MASK                    (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_CLK_DIV_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_CLK_DIV_MAX                     (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_CLK_DIV_LD_MASK                 (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_CLK_DIV_LD_SHIFT                (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_CLK_DIV_LD_MAX                  (0x00000001U)


/* USART0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL_CLK_SEL_MASK                      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL_CLK_SEL_MAX                       (0x00000001U)


/* USART1_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL_CLK_SEL_MASK                      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL_CLK_SEL_MAX                       (0x00000001U)


/* USART2_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL_CLK_SEL_MASK                      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL_CLK_SEL_MAX                       (0x00000001U)


/* USART3_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL_CLK_SEL_MASK                      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL_CLK_SEL_SHIFT                     (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL_CLK_SEL_MAX                       (0x00000001U)


/* DPHY0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL_REF_CLK_SEL_MASK                   (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL_REF_CLK_SEL_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL_REF_CLK_SEL_MAX                    (0x00000001U)


/* WWD0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_CLK_SEL_MASK                        (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_CLK_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_CLK_SEL_MAX                         (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_WRTLOCK_MASK                        (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_WRTLOCK_SHIFT                       (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_WRTLOCK_MAX                         (0x00000001U)


/* WWD1_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_CLK_SEL_MASK                        (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_CLK_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_CLK_SEL_MAX                         (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_WRTLOCK_MASK                        (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_WRTLOCK_SHIFT                       (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_WRTLOCK_MAX                         (0x00000001U)


/* WWD2_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_CLK_SEL_MASK                        (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_CLK_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_CLK_SEL_MAX                         (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_WRTLOCK_MASK                        (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_WRTLOCK_SHIFT                       (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_WRTLOCK_MAX                         (0x00000001U)


/* WWD3_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_CLK_SEL_MASK                        (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_CLK_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_CLK_SEL_MAX                         (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_WRTLOCK_MASK                        (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_WRTLOCK_SHIFT                       (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_WRTLOCK_MAX                         (0x00000001U)


/* WWD4_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_CLK_SEL_MASK                        (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_CLK_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_CLK_SEL_MAX                         (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_WRTLOCK_MASK                        (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_WRTLOCK_SHIFT                       (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_WRTLOCK_MAX                         (0x00000001U)


/* MCAN0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL_CLK_SEL_MASK                       (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL_CLK_SEL_SHIFT                      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL_CLK_SEL_MAX                        (0x00000003U)


/* MCAN1_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL_CLK_SEL_MASK                       (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL_CLK_SEL_SHIFT                      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL_CLK_SEL_MAX                        (0x00000003U)


/* OSPI0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_CLK_SEL_MASK                       (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_CLK_SEL_SHIFT                      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_CLK_SEL_MAX                        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_LOOPCLK_SEL_MASK                   (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_LOOPCLK_SEL_SHIFT                  (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_LOOPCLK_SEL_MAX                    (0x00000001U)


/* ADC0_CLKSEL */

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL_CLK_SEL_MASK                        (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL_CLK_SEL_SHIFT                       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL_CLK_SEL_MAX                         (0x00000003U)


/* LOCK2_KICK0 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0_LOCK2_KICK0_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0_LOCK2_KICK0_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0_LOCK2_KICK0_MAX                     (0xFFFFFFFFU)


/* LOCK2_KICK1 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1_LOCK2_KICK1_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1_LOCK2_KICK1_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1_LOCK2_KICK1_MAX                     (0xFFFFFFFFU)


/* CLAIMREG_P2_R0_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0_READONLY_CLAIMREG_P2_R0_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0_READONLY_CLAIMREG_P2_R0_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0_READONLY_CLAIMREG_P2_R0_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R1_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1_READONLY_CLAIMREG_P2_R1_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1_READONLY_CLAIMREG_P2_R1_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1_READONLY_CLAIMREG_P2_R1_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R2_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2_READONLY_CLAIMREG_P2_R2_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2_READONLY_CLAIMREG_P2_R2_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2_READONLY_CLAIMREG_P2_R2_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R3_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3_READONLY_CLAIMREG_P2_R3_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3_READONLY_CLAIMREG_P2_R3_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3_READONLY_CLAIMREG_P2_R3_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R4_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4_READONLY_CLAIMREG_P2_R4_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4_READONLY_CLAIMREG_P2_R4_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4_READONLY_CLAIMREG_P2_R4_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R5_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5_READONLY_CLAIMREG_P2_R5_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5_READONLY_CLAIMREG_P2_R5_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5_READONLY_CLAIMREG_P2_R5_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R6_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6_READONLY_CLAIMREG_P2_R6_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6_READONLY_CLAIMREG_P2_R6_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6_READONLY_CLAIMREG_P2_R6_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R7_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7_READONLY_CLAIMREG_P2_R7_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7_READONLY_CLAIMREG_P2_R7_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7_READONLY_CLAIMREG_P2_R7_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R8_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8_READONLY_CLAIMREG_P2_R8_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8_READONLY_CLAIMREG_P2_R8_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8_READONLY_CLAIMREG_P2_R8_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R9_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9_READONLY_CLAIMREG_P2_R9_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9_READONLY_CLAIMREG_P2_R9_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9_READONLY_CLAIMREG_P2_R9_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R10_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10_READONLY_CLAIMREG_P2_R10_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10_READONLY_CLAIMREG_P2_R10_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10_READONLY_CLAIMREG_P2_R10_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P2_R11_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11_READONLY_CLAIMREG_P2_R11_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11_READONLY_CLAIMREG_P2_R11_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11_READONLY_CLAIMREG_P2_R11_READONLY_MAX (0xFFFFFFFFU)


/* OBSCLK0_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_SEL_PROXY_MASK (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_SEL_PROXY_MAX (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_DIV_PROXY_MASK (0x0000FF00U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_DIV_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_DIV_PROXY_MAX (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_DIV_LD_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_DIV_LD_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_CLK_DIV_LD_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_OUT_MUX_SEL_PROXY_MASK (0x01000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_OUT_MUX_SEL_PROXY_SHIFT (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_OBSCLK0_CTRL_PROXY_OBSCLK0_CTRL_OUT_MUX_SEL_PROXY_MAX (0x00000001U)


/* CLKOUT_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL_PROXY_CLKOUT_CTRL_CLK_SEL_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL_PROXY_CLKOUT_CTRL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLKOUT_CTRL_PROXY_CLKOUT_CTRL_CLK_SEL_PROXY_MAX (0x00000001U)


/* MAIN_PLL0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_PROXY_MAIN_PLL0_CLKSEL_BYP_WARM_RST_PROXY_MASK (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_PROXY_MAIN_PLL0_CLKSEL_BYP_WARM_RST_PROXY_SHIFT (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_PROXY_MAIN_PLL0_CLKSEL_BYP_WARM_RST_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_PROXY_MAIN_PLL0_CLKSEL_BYPASS_SW_OVRD_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_PROXY_MAIN_PLL0_CLKSEL_BYPASS_SW_OVRD_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL0_CLKSEL_PROXY_MAIN_PLL0_CLKSEL_BYPASS_SW_OVRD_PROXY_MAX (0x00000001U)


/* MAIN_PLL1_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_PROXY_MAIN_PLL1_CLKSEL_BYP_WARM_RST_PROXY_MASK (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_PROXY_MAIN_PLL1_CLKSEL_BYP_WARM_RST_PROXY_SHIFT (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_PROXY_MAIN_PLL1_CLKSEL_BYP_WARM_RST_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_PROXY_MAIN_PLL1_CLKSEL_BYPASS_SW_OVRD_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_PROXY_MAIN_PLL1_CLKSEL_BYPASS_SW_OVRD_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL1_CLKSEL_PROXY_MAIN_PLL1_CLKSEL_BYPASS_SW_OVRD_PROXY_MAX (0x00000001U)


/* MAIN_PLL2_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_PROXY_MAIN_PLL2_CLKSEL_BYP_WARM_RST_PROXY_MASK (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_PROXY_MAIN_PLL2_CLKSEL_BYP_WARM_RST_PROXY_SHIFT (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_PROXY_MAIN_PLL2_CLKSEL_BYP_WARM_RST_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_PROXY_MAIN_PLL2_CLKSEL_BYPASS_SW_OVRD_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_PROXY_MAIN_PLL2_CLKSEL_BYPASS_SW_OVRD_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL2_CLKSEL_PROXY_MAIN_PLL2_CLKSEL_BYPASS_SW_OVRD_PROXY_MAX (0x00000001U)


/* MAIN_PLL7_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_PROXY_MAIN_PLL7_CLKSEL_BYP_WARM_RST_PROXY_MASK (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_PROXY_MAIN_PLL7_CLKSEL_BYP_WARM_RST_PROXY_SHIFT (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_PROXY_MAIN_PLL7_CLKSEL_BYP_WARM_RST_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_PROXY_MAIN_PLL7_CLKSEL_BYPASS_SW_OVRD_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_PROXY_MAIN_PLL7_CLKSEL_BYPASS_SW_OVRD_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL7_CLKSEL_PROXY_MAIN_PLL7_CLKSEL_BYPASS_SW_OVRD_PROXY_MAX (0x00000001U)


/* MAIN_PLL14_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_PROXY_MAIN_PLL14_CLKSEL_BYP_WARM_RST_PROXY_MASK (0x00800000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_PROXY_MAIN_PLL14_CLKSEL_BYP_WARM_RST_PROXY_SHIFT (0x00000017U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_PROXY_MAIN_PLL14_CLKSEL_BYP_WARM_RST_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_PROXY_MAIN_PLL14_CLKSEL_BYPASS_SW_OVRD_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_PROXY_MAIN_PLL14_CLKSEL_BYPASS_SW_OVRD_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MAIN_PLL14_CLKSEL_PROXY_MAIN_PLL14_CLKSEL_BYPASS_SW_OVRD_PROXY_MAX (0x00000001U)


/* CPSW_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL_PROXY_CPSW_CLKSEL_CPTS_CLKSEL_PROXY_MASK (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL_PROXY_CPSW_CLKSEL_CPTS_CLKSEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CPSW_CLKSEL_PROXY_CPSW_CLKSEL_CPTS_CLKSEL_PROXY_MAX (0x00000007U)


/* TIMER0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL_PROXY_TIMER0_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL_PROXY_TIMER0_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER0_CLKSEL_PROXY_TIMER0_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER1_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL_PROXY_TIMER1_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL_PROXY_TIMER1_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER1_CLKSEL_PROXY_TIMER1_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER2_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL_PROXY_TIMER2_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL_PROXY_TIMER2_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER2_CLKSEL_PROXY_TIMER2_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER3_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL_PROXY_TIMER3_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL_PROXY_TIMER3_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER3_CLKSEL_PROXY_TIMER3_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER4_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL_PROXY_TIMER4_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL_PROXY_TIMER4_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER4_CLKSEL_PROXY_TIMER4_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER5_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL_PROXY_TIMER5_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL_PROXY_TIMER5_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER5_CLKSEL_PROXY_TIMER5_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER6_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL_PROXY_TIMER6_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL_PROXY_TIMER6_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER6_CLKSEL_PROXY_TIMER6_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER7_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL_PROXY_TIMER7_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL_PROXY_TIMER7_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER7_CLKSEL_PROXY_TIMER7_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER8_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL_PROXY_TIMER8_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL_PROXY_TIMER8_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER8_CLKSEL_PROXY_TIMER8_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* TIMER9_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL_PROXY_TIMER9_CLKSEL_CLK_SEL_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL_PROXY_TIMER9_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_TIMER9_CLKSEL_PROXY_TIMER9_CLKSEL_CLK_SEL_PROXY_MAX (0x0000000FU)


/* SPI0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL_PROXY_SPI0_CLKSEL_MSTR_LB_CLKSEL_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL_PROXY_SPI0_CLKSEL_MSTR_LB_CLKSEL_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI0_CLKSEL_PROXY_SPI0_CLKSEL_MSTR_LB_CLKSEL_PROXY_MAX (0x00000001U)


/* SPI1_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL_PROXY_SPI1_CLKSEL_MSTR_LB_CLKSEL_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL_PROXY_SPI1_CLKSEL_MSTR_LB_CLKSEL_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI1_CLKSEL_PROXY_SPI1_CLKSEL_MSTR_LB_CLKSEL_PROXY_MAX (0x00000001U)


/* SPI2_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL_PROXY_SPI2_CLKSEL_MSTR_LB_CLKSEL_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL_PROXY_SPI2_CLKSEL_MSTR_LB_CLKSEL_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_SPI2_CLKSEL_PROXY_SPI2_CLKSEL_MSTR_LB_CLKSEL_PROXY_MAX (0x00000001U)


/* USART0_CLK_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_PROXY_USART0_CLK_CTRL_CLK_DIV_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_PROXY_USART0_CLK_CTRL_CLK_DIV_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_PROXY_USART0_CLK_CTRL_CLK_DIV_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_PROXY_USART0_CLK_CTRL_CLK_DIV_LD_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_PROXY_USART0_CLK_CTRL_CLK_DIV_LD_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLK_CTRL_PROXY_USART0_CLK_CTRL_CLK_DIV_LD_PROXY_MAX (0x00000001U)


/* USART1_CLK_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_PROXY_USART1_CLK_CTRL_CLK_DIV_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_PROXY_USART1_CLK_CTRL_CLK_DIV_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_PROXY_USART1_CLK_CTRL_CLK_DIV_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_PROXY_USART1_CLK_CTRL_CLK_DIV_LD_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_PROXY_USART1_CLK_CTRL_CLK_DIV_LD_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLK_CTRL_PROXY_USART1_CLK_CTRL_CLK_DIV_LD_PROXY_MAX (0x00000001U)


/* USART2_CLK_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_PROXY_USART2_CLK_CTRL_CLK_DIV_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_PROXY_USART2_CLK_CTRL_CLK_DIV_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_PROXY_USART2_CLK_CTRL_CLK_DIV_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_PROXY_USART2_CLK_CTRL_CLK_DIV_LD_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_PROXY_USART2_CLK_CTRL_CLK_DIV_LD_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLK_CTRL_PROXY_USART2_CLK_CTRL_CLK_DIV_LD_PROXY_MAX (0x00000001U)


/* USART3_CLK_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_PROXY_USART3_CLK_CTRL_CLK_DIV_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_PROXY_USART3_CLK_CTRL_CLK_DIV_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_PROXY_USART3_CLK_CTRL_CLK_DIV_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_PROXY_USART3_CLK_CTRL_CLK_DIV_LD_PROXY_MASK (0x00010000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_PROXY_USART3_CLK_CTRL_CLK_DIV_LD_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLK_CTRL_PROXY_USART3_CLK_CTRL_CLK_DIV_LD_PROXY_MAX (0x00000001U)


/* USART0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL_PROXY_USART0_CLKSEL_CLK_SEL_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL_PROXY_USART0_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART0_CLKSEL_PROXY_USART0_CLKSEL_CLK_SEL_PROXY_MAX (0x00000001U)


/* USART1_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL_PROXY_USART1_CLKSEL_CLK_SEL_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL_PROXY_USART1_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART1_CLKSEL_PROXY_USART1_CLKSEL_CLK_SEL_PROXY_MAX (0x00000001U)


/* USART2_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL_PROXY_USART2_CLKSEL_CLK_SEL_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL_PROXY_USART2_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART2_CLKSEL_PROXY_USART2_CLKSEL_CLK_SEL_PROXY_MAX (0x00000001U)


/* USART3_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL_PROXY_USART3_CLKSEL_CLK_SEL_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL_PROXY_USART3_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_USART3_CLKSEL_PROXY_USART3_CLKSEL_CLK_SEL_PROXY_MAX (0x00000001U)


/* DPHY0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL_PROXY_DPHY0_CLKSEL_REF_CLK_SEL_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL_PROXY_DPHY0_CLKSEL_REF_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_DPHY0_CLKSEL_PROXY_DPHY0_CLKSEL_REF_CLK_SEL_PROXY_MAX (0x00000001U)


/* WWD0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_PROXY_WWD0_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_PROXY_WWD0_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_PROXY_WWD0_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_PROXY_WWD0_CLKSEL_WRTLOCK_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_PROXY_WWD0_CLKSEL_WRTLOCK_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD0_CLKSEL_PROXY_WWD0_CLKSEL_WRTLOCK_PROXY_MAX (0x00000001U)


/* WWD1_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_PROXY_WWD1_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_PROXY_WWD1_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_PROXY_WWD1_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_PROXY_WWD1_CLKSEL_WRTLOCK_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_PROXY_WWD1_CLKSEL_WRTLOCK_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD1_CLKSEL_PROXY_WWD1_CLKSEL_WRTLOCK_PROXY_MAX (0x00000001U)


/* WWD2_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_PROXY_WWD2_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_PROXY_WWD2_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_PROXY_WWD2_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_PROXY_WWD2_CLKSEL_WRTLOCK_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_PROXY_WWD2_CLKSEL_WRTLOCK_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD2_CLKSEL_PROXY_WWD2_CLKSEL_WRTLOCK_PROXY_MAX (0x00000001U)


/* WWD3_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_PROXY_WWD3_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_PROXY_WWD3_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_PROXY_WWD3_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_PROXY_WWD3_CLKSEL_WRTLOCK_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_PROXY_WWD3_CLKSEL_WRTLOCK_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD3_CLKSEL_PROXY_WWD3_CLKSEL_WRTLOCK_PROXY_MAX (0x00000001U)


/* WWD4_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_PROXY_WWD4_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_PROXY_WWD4_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_PROXY_WWD4_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_PROXY_WWD4_CLKSEL_WRTLOCK_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_PROXY_WWD4_CLKSEL_WRTLOCK_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_WWD4_CLKSEL_PROXY_WWD4_CLKSEL_WRTLOCK_PROXY_MAX (0x00000001U)


/* MCAN0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL_PROXY_MCAN0_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL_PROXY_MCAN0_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN0_CLKSEL_PROXY_MCAN0_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)


/* MCAN1_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL_PROXY_MCAN1_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL_PROXY_MCAN1_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MCAN1_CLKSEL_PROXY_MCAN1_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)


/* OSPI0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_PROXY_OSPI0_CLKSEL_CLK_SEL_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_PROXY_OSPI0_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_PROXY_OSPI0_CLKSEL_CLK_SEL_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_PROXY_OSPI0_CLKSEL_LOOPCLK_SEL_PROXY_MASK (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_PROXY_OSPI0_CLKSEL_LOOPCLK_SEL_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_OSPI0_CLKSEL_PROXY_OSPI0_CLKSEL_LOOPCLK_SEL_PROXY_MAX (0x00000001U)


/* ADC0_CLKSEL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL_PROXY_ADC0_CLKSEL_CLK_SEL_PROXY_MASK (0x00000003U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL_PROXY_ADC0_CLKSEL_CLK_SEL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_ADC0_CLKSEL_PROXY_ADC0_CLKSEL_CLK_SEL_PROXY_MAX (0x00000003U)


/* LOCK2_KICK0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0_PROXY_LOCK2_KICK0_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0_PROXY_LOCK2_KICK0_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK0_PROXY_LOCK2_KICK0_PROXY_MAX         (0xFFFFFFFFU)


/* LOCK2_KICK1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1_PROXY_LOCK2_KICK1_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1_PROXY_LOCK2_KICK1_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK2_KICK1_PROXY_LOCK2_KICK1_PROXY_MAX         (0xFFFFFFFFU)


/* CLAIMREG_P2_R0 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0_CLAIMREG_P2_R0_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0_CLAIMREG_P2_R0_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R0_CLAIMREG_P2_R0_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R1 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1_CLAIMREG_P2_R1_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1_CLAIMREG_P2_R1_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R1_CLAIMREG_P2_R1_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R2 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2_CLAIMREG_P2_R2_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2_CLAIMREG_P2_R2_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R2_CLAIMREG_P2_R2_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R3 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3_CLAIMREG_P2_R3_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3_CLAIMREG_P2_R3_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R3_CLAIMREG_P2_R3_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R4 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4_CLAIMREG_P2_R4_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4_CLAIMREG_P2_R4_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R4_CLAIMREG_P2_R4_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R5 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5_CLAIMREG_P2_R5_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5_CLAIMREG_P2_R5_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R5_CLAIMREG_P2_R5_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R6 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6_CLAIMREG_P2_R6_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6_CLAIMREG_P2_R6_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R6_CLAIMREG_P2_R6_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R7 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7_CLAIMREG_P2_R7_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7_CLAIMREG_P2_R7_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R7_CLAIMREG_P2_R7_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R8 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8_CLAIMREG_P2_R8_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8_CLAIMREG_P2_R8_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R8_CLAIMREG_P2_R8_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R9 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9_CLAIMREG_P2_R9_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9_CLAIMREG_P2_R9_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R9_CLAIMREG_P2_R9_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P2_R10 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10_CLAIMREG_P2_R10_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10_CLAIMREG_P2_R10_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R10_CLAIMREG_P2_R10_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P2_R11 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11_CLAIMREG_P2_R11_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11_CLAIMREG_P2_R11_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P2_R11_CLAIMREG_P2_R11_MAX             (0xFFFFFFFFU)


/* R5SS0_LBIST_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_DIVIDE_RATIO_MASK              (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_DIVIDE_RATIO_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_DIVIDE_RATIO_MAX               (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_LOAD_DIV_MASK                  (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_LOAD_DIV_SHIFT                 (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_LOAD_DIV_MAX                   (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_DC_DEF_MASK                    (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_DC_DEF_SHIFT                   (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_DC_DEF_MAX                     (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_RUNBIST_MODE_MASK              (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_RUNBIST_MODE_SHIFT             (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_RUNBIST_MODE_MAX               (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_BIST_RUN_MASK                  (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_BIST_RUN_SHIFT                 (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_BIST_RUN_MAX                   (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_BIST_RESET_MASK                (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_BIST_RESET_SHIFT               (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_BIST_RESET_MAX                 (0x00000001U)


/* R5SS0_LBIST_PATCOUNT */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_SCAN_PC_DEF_MASK           (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_SCAN_PC_DEF_SHIFT          (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_SCAN_PC_DEF_MAX            (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_RESET_PC_DEF_MASK          (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_RESET_PC_DEF_SHIFT         (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_RESET_PC_DEF_MAX           (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_SET_PC_DEF_MASK            (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_SET_PC_DEF_SHIFT           (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_SET_PC_DEF_MAX             (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_STATIC_PC_DEF_MASK         (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_STATIC_PC_DEF_SHIFT        (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_STATIC_PC_DEF_MAX          (0x00003FFFU)


/* R5SS0_LBIST_SEED0 */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0_PRPG_DEF_MASK                 (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0_PRPG_DEF_SHIFT                (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0_PRPG_DEF_MAX                  (0xFFFFFFFFU)


/* R5SS0_LBIST_SEED1 */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1_PRPG_DEF_MASK                 (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1_PRPG_DEF_SHIFT                (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1_PRPG_DEF_MAX                  (0x001FFFFFU)


/* R5SS0_LBIST_SPARE0 */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_LBIST_SELFTEST_EN_MASK       (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_LBIST_SELFTEST_EN_SHIFT      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_LBIST_SELFTEST_EN_MAX        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PBIST_SELFTEST_EN_MASK       (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PBIST_SELFTEST_EN_SHIFT      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PBIST_SELFTEST_EN_MAX        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_SPARE0_MASK                  (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_SPARE0_SHIFT                 (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_SPARE0_MAX                   (0x3FFFFFFFU)


/* R5SS0_LBIST_STAT */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_MISR_MUX_CTL_MASK              (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_MISR_MUX_CTL_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_MISR_MUX_CTL_MAX               (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_OUT_MUX_CTL_MASK               (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_OUT_MUX_CTL_SHIFT              (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_OUT_MUX_CTL_MAX                (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_BIST_RUNNING_MASK              (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_BIST_RUNNING_SHIFT             (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_BIST_RUNNING_MAX               (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_BIST_DONE_MASK                 (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_BIST_DONE_SHIFT                (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_BIST_DONE_MAX                  (0x00000001U)


/* R5SS0_LBIST_MISR */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR_MISR_RESULT_MASK               (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR_MISR_RESULT_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR_MISR_RESULT_MAX                (0xFFFFFFFFU)


/* R5SS1_LBIST_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_DIVIDE_RATIO_MASK              (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_DIVIDE_RATIO_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_DIVIDE_RATIO_MAX               (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_LOAD_DIV_MASK                  (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_LOAD_DIV_SHIFT                 (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_LOAD_DIV_MAX                   (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_DC_DEF_MASK                    (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_DC_DEF_SHIFT                   (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_DC_DEF_MAX                     (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_RUNBIST_MODE_MASK              (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_RUNBIST_MODE_SHIFT             (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_RUNBIST_MODE_MAX               (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_BIST_RUN_MASK                  (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_BIST_RUN_SHIFT                 (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_BIST_RUN_MAX                   (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_BIST_RESET_MASK                (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_BIST_RESET_SHIFT               (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_BIST_RESET_MAX                 (0x00000001U)


/* R5SS1_LBIST_PATCOUNT */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_SCAN_PC_DEF_MASK           (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_SCAN_PC_DEF_SHIFT          (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_SCAN_PC_DEF_MAX            (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_RESET_PC_DEF_MASK          (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_RESET_PC_DEF_SHIFT         (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_RESET_PC_DEF_MAX           (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_SET_PC_DEF_MASK            (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_SET_PC_DEF_SHIFT           (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_SET_PC_DEF_MAX             (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_STATIC_PC_DEF_MASK         (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_STATIC_PC_DEF_SHIFT        (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_STATIC_PC_DEF_MAX          (0x00003FFFU)


/* R5SS1_LBIST_SEED0 */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0_PRPG_DEF_MASK                 (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0_PRPG_DEF_SHIFT                (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0_PRPG_DEF_MAX                  (0xFFFFFFFFU)


/* R5SS1_LBIST_SEED1 */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1_PRPG_DEF_MASK                 (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1_PRPG_DEF_SHIFT                (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1_PRPG_DEF_MAX                  (0x001FFFFFU)


/* R5SS1_LBIST_SPARE0 */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_LBIST_SELFTEST_EN_MASK       (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_LBIST_SELFTEST_EN_SHIFT      (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_LBIST_SELFTEST_EN_MAX        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PBIST_SELFTEST_EN_MASK       (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PBIST_SELFTEST_EN_SHIFT      (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PBIST_SELFTEST_EN_MAX        (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_SPARE0_MASK                  (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_SPARE0_SHIFT                 (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_SPARE0_MAX                   (0x3FFFFFFFU)


/* R5SS1_LBIST_STAT */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_MISR_MUX_CTL_MASK              (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_MISR_MUX_CTL_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_MISR_MUX_CTL_MAX               (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_OUT_MUX_CTL_MASK               (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_OUT_MUX_CTL_SHIFT              (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_OUT_MUX_CTL_MAX                (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_BIST_RUNNING_MASK              (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_BIST_RUNNING_SHIFT             (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_BIST_RUNNING_MAX               (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_BIST_DONE_MASK                 (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_BIST_DONE_SHIFT                (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_BIST_DONE_MAX                  (0x00000001U)


/* R5SS1_LBIST_MISR */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR_MISR_RESULT_MASK               (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR_MISR_RESULT_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR_MISR_RESULT_MAX                (0xFFFFFFFFU)


/* C70_LBIST_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_DIVIDE_RATIO_MASK                (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_DIVIDE_RATIO_SHIFT               (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_DIVIDE_RATIO_MAX                 (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_LOAD_DIV_MASK                    (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_LOAD_DIV_SHIFT                   (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_LOAD_DIV_MAX                     (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_DC_DEF_MASK                      (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_DC_DEF_SHIFT                     (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_DC_DEF_MAX                       (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_RUNBIST_MODE_MASK                (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_RUNBIST_MODE_SHIFT               (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_RUNBIST_MODE_MAX                 (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_BIST_RUN_MASK                    (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_BIST_RUN_SHIFT                   (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_BIST_RUN_MAX                     (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_BIST_RESET_MASK                  (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_BIST_RESET_SHIFT                 (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_BIST_RESET_MAX                   (0x00000001U)


/* C70_LBIST_PATCOUNT */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_SCAN_PC_DEF_MASK             (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_SCAN_PC_DEF_SHIFT            (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_SCAN_PC_DEF_MAX              (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_RESET_PC_DEF_MASK            (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_RESET_PC_DEF_SHIFT           (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_RESET_PC_DEF_MAX             (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_SET_PC_DEF_MASK              (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_SET_PC_DEF_SHIFT             (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_SET_PC_DEF_MAX               (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_STATIC_PC_DEF_MASK           (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_STATIC_PC_DEF_SHIFT          (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_STATIC_PC_DEF_MAX            (0x00003FFFU)


/* C70_LBIST_SEED0 */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0_PRPG_DEF_MASK                   (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0_PRPG_DEF_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0_PRPG_DEF_MAX                    (0xFFFFFFFFU)


/* C70_LBIST_SEED1 */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1_PRPG_DEF_MASK                   (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1_PRPG_DEF_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1_PRPG_DEF_MAX                    (0x001FFFFFU)


/* C70_LBIST_SPARE0 */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_LBIST_SELFTEST_EN_MASK         (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_LBIST_SELFTEST_EN_SHIFT        (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_LBIST_SELFTEST_EN_MAX          (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PBIST_SELFTEST_EN_MASK         (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PBIST_SELFTEST_EN_SHIFT        (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PBIST_SELFTEST_EN_MAX          (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_SPARE0_MASK                    (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_SPARE0_SHIFT                   (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_SPARE0_MAX                     (0x3FFFFFFFU)


/* C70_LBIST_SPARE1 */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1_SPARE1_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1_SPARE1_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1_SPARE1_MAX                     (0xFFFFFFFFU)


/* C70_LBIST_STAT */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_MISR_MUX_CTL_MASK                (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_MISR_MUX_CTL_SHIFT               (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_MISR_MUX_CTL_MAX                 (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_OUT_MUX_CTL_MASK                 (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_OUT_MUX_CTL_SHIFT                (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_OUT_MUX_CTL_MAX                  (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_BIST_RUNNING_MASK                (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_BIST_RUNNING_SHIFT               (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_BIST_RUNNING_MAX                 (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_BIST_DONE_MASK                   (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_BIST_DONE_SHIFT                  (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_BIST_DONE_MAX                    (0x00000001U)


/* MMA0_LBIST_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_DIVIDE_RATIO_MASK               (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_DIVIDE_RATIO_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_DIVIDE_RATIO_MAX                (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_LOAD_DIV_MASK                   (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_LOAD_DIV_SHIFT                  (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_LOAD_DIV_MAX                    (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_DC_DEF_MASK                     (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_DC_DEF_SHIFT                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_DC_DEF_MAX                      (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_RUNBIST_MODE_MASK               (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_RUNBIST_MODE_SHIFT              (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_RUNBIST_MODE_MAX                (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_BIST_RUN_MASK                   (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_BIST_RUN_SHIFT                  (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_BIST_RUN_MAX                    (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_BIST_RESET_MASK                 (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_BIST_RESET_SHIFT                (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_BIST_RESET_MAX                  (0x00000001U)


/* MMA0_LBIST_PATCOUNT */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_SCAN_PC_DEF_MASK            (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_SCAN_PC_DEF_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_SCAN_PC_DEF_MAX             (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_RESET_PC_DEF_MASK           (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_RESET_PC_DEF_SHIFT          (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_RESET_PC_DEF_MAX            (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_SET_PC_DEF_MASK             (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_SET_PC_DEF_SHIFT            (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_SET_PC_DEF_MAX              (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_STATIC_PC_DEF_MASK          (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_STATIC_PC_DEF_SHIFT         (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_STATIC_PC_DEF_MAX           (0x00003FFFU)


/* MMA0_LBIST_SEED0 */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0_PRPG_DEF_MASK                  (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0_PRPG_DEF_SHIFT                 (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0_PRPG_DEF_MAX                   (0xFFFFFFFFU)


/* MMA0_LBIST_SEED1 */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1_PRPG_DEF_MASK                  (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1_PRPG_DEF_SHIFT                 (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1_PRPG_DEF_MAX                   (0x001FFFFFU)


/* MMA0_LBIST_SPARE0 */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_LBIST_SELFTEST_EN_MASK        (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_LBIST_SELFTEST_EN_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_LBIST_SELFTEST_EN_MAX         (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PBIST_SELFTEST_EN_MASK        (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PBIST_SELFTEST_EN_SHIFT       (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PBIST_SELFTEST_EN_MAX         (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_SPARE0_MASK                   (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_SPARE0_SHIFT                  (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_SPARE0_MAX                    (0x3FFFFFFFU)


/* MMA0_LBIST_STAT */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_MISR_MUX_CTL_MASK               (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_MISR_MUX_CTL_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_MISR_MUX_CTL_MAX                (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_OUT_MUX_CTL_MASK                (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_OUT_MUX_CTL_SHIFT               (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_OUT_MUX_CTL_MAX                 (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_BIST_RUNNING_MASK               (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_BIST_RUNNING_SHIFT              (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_BIST_RUNNING_MAX                (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_BIST_DONE_MASK                  (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_BIST_DONE_SHIFT                 (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_BIST_DONE_MAX                   (0x00000001U)


/* MMA0_LBIST_MISR */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR_MISR_RESULT_MASK                (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR_MISR_RESULT_SHIFT               (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR_MISR_RESULT_MAX                 (0xFFFFFFFFU)


/* HWA0_LBIST_CTRL */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_DIVIDE_RATIO_MASK               (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_DIVIDE_RATIO_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_DIVIDE_RATIO_MAX                (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_LOAD_DIV_MASK                   (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_LOAD_DIV_SHIFT                  (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_LOAD_DIV_MAX                    (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_DC_DEF_MASK                     (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_DC_DEF_SHIFT                    (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_DC_DEF_MAX                      (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_RUNBIST_MODE_MASK               (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_RUNBIST_MODE_SHIFT              (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_RUNBIST_MODE_MAX                (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_BIST_RUN_MASK                   (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_BIST_RUN_SHIFT                  (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_BIST_RUN_MAX                    (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_BIST_RESET_MASK                 (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_BIST_RESET_SHIFT                (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_BIST_RESET_MAX                  (0x00000001U)


/* HWA0_LBIST_PATCOUNT */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_SCAN_PC_DEF_MASK            (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_SCAN_PC_DEF_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_SCAN_PC_DEF_MAX             (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_RESET_PC_DEF_MASK           (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_RESET_PC_DEF_SHIFT          (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_RESET_PC_DEF_MAX            (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_SET_PC_DEF_MASK             (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_SET_PC_DEF_SHIFT            (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_SET_PC_DEF_MAX              (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_STATIC_PC_DEF_MASK          (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_STATIC_PC_DEF_SHIFT         (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_STATIC_PC_DEF_MAX           (0x00003FFFU)


/* HWA0_LBIST_SEED0 */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0_PRPG_DEF_MASK                  (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0_PRPG_DEF_SHIFT                 (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0_PRPG_DEF_MAX                   (0xFFFFFFFFU)


/* HWA0_LBIST_SEED1 */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1_PRPG_DEF_MASK                  (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1_PRPG_DEF_SHIFT                 (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1_PRPG_DEF_MAX                   (0x001FFFFFU)


/* HWA0_LBIST_SPARE0 */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_LBIST_SELFTEST_EN_MASK        (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_LBIST_SELFTEST_EN_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_LBIST_SELFTEST_EN_MAX         (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PBIST_SELFTEST_EN_MASK        (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PBIST_SELFTEST_EN_SHIFT       (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PBIST_SELFTEST_EN_MAX         (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_SPARE0_MASK                   (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_SPARE0_SHIFT                  (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_SPARE0_MAX                    (0x3FFFFFFFU)


/* HWA0_LBIST_STAT */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_MISR_MUX_CTL_MASK               (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_MISR_MUX_CTL_SHIFT              (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_MISR_MUX_CTL_MAX                (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_OUT_MUX_CTL_MASK                (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_OUT_MUX_CTL_SHIFT               (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_OUT_MUX_CTL_MAX                 (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_BIST_RUNNING_MASK               (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_BIST_RUNNING_SHIFT              (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_BIST_RUNNING_MAX                (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_BIST_DONE_MASK                  (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_BIST_DONE_SHIFT                 (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_BIST_DONE_MAX                   (0x00000001U)


/* HWA0_LBIST_MISR */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR_MISR_RESULT_MASK                (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR_MISR_RESULT_SHIFT               (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR_MISR_RESULT_MAX                 (0xFFFFFFFFU)


/* R5SS0_LBIST_SIG */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG_MISR_SIG_MASK                   (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG_MISR_SIG_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG_MISR_SIG_MAX                    (0xFFFFFFFFU)


/* R5SS1_LBIST_SIG */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG_MISR_SIG_MASK                   (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG_MISR_SIG_SHIFT                  (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG_MISR_SIG_MAX                    (0xFFFFFFFFU)


/* HWA0_LBIST_SIG */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG_MISR_SIG_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG_MISR_SIG_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG_MISR_SIG_MAX                     (0xFFFFFFFFU)


/* C70_LBIST_SIG */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG_MISR_SIG_MASK                     (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG_MISR_SIG_SHIFT                    (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG_MISR_SIG_MAX                      (0xFFFFFFFFU)


/* MMA0_LBIST_SIG */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG_MISR_SIG_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG_MISR_SIG_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG_MISR_SIG_MAX                     (0xFFFFFFFFU)


/* LOCK3_KICK0 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0_LOCK3_KICK0_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0_LOCK3_KICK0_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0_LOCK3_KICK0_MAX                     (0xFFFFFFFFU)


/* LOCK3_KICK1 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1_LOCK3_KICK1_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1_LOCK3_KICK1_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1_LOCK3_KICK1_MAX                     (0xFFFFFFFFU)


/* CLAIMREG_P3_R0_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0_READONLY_CLAIMREG_P3_R0_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0_READONLY_CLAIMREG_P3_R0_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0_READONLY_CLAIMREG_P3_R0_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P3_R1_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1_READONLY_CLAIMREG_P3_R1_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1_READONLY_CLAIMREG_P3_R1_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1_READONLY_CLAIMREG_P3_R1_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P3_R2_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2_READONLY_CLAIMREG_P3_R2_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2_READONLY_CLAIMREG_P3_R2_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2_READONLY_CLAIMREG_P3_R2_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P3_R3_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3_READONLY_CLAIMREG_P3_R3_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3_READONLY_CLAIMREG_P3_R3_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3_READONLY_CLAIMREG_P3_R3_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P3_R4_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4_READONLY_CLAIMREG_P3_R4_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4_READONLY_CLAIMREG_P3_R4_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4_READONLY_CLAIMREG_P3_R4_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P3_R5_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5_READONLY_CLAIMREG_P3_R5_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5_READONLY_CLAIMREG_P3_R5_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5_READONLY_CLAIMREG_P3_R5_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P3_R6_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6_READONLY_CLAIMREG_P3_R6_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6_READONLY_CLAIMREG_P3_R6_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6_READONLY_CLAIMREG_P3_R6_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P3_R7_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7_READONLY_CLAIMREG_P3_R7_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7_READONLY_CLAIMREG_P3_R7_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7_READONLY_CLAIMREG_P3_R7_READONLY_MAX (0xFFFFFFFFU)


/* R5SS0_LBIST_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_DIVIDE_RATIO_PROXY_MASK (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_DIVIDE_RATIO_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_DIVIDE_RATIO_PROXY_MAX (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_LOAD_DIV_PROXY_MASK (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_LOAD_DIV_PROXY_SHIFT (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_LOAD_DIV_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_DC_DEF_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_DC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_DC_DEF_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_RUNBIST_MODE_PROXY_MASK (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_RUNBIST_MODE_PROXY_SHIFT (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_RUNBIST_MODE_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_BIST_RUN_PROXY_MASK (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_BIST_RUN_PROXY_SHIFT (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_BIST_RUN_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_BIST_RESET_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_BIST_RESET_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_CTRL_PROXY_R5SS0_LBIST_CTRL_BIST_RESET_PROXY_MAX (0x00000001U)


/* R5SS0_LBIST_PATCOUNT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MASK (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MASK (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MASK (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_PATCOUNT_PROXY_R5SS0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MAX (0x00003FFFU)


/* R5SS0_LBIST_SEED0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0_PROXY_R5SS0_LBIST_SEED0_PRPG_DEF_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0_PROXY_R5SS0_LBIST_SEED0_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED0_PROXY_R5SS0_LBIST_SEED0_PRPG_DEF_PROXY_MAX (0xFFFFFFFFU)


/* R5SS0_LBIST_SEED1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1_PROXY_R5SS0_LBIST_SEED1_PRPG_DEF_PROXY_MASK (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1_PROXY_R5SS0_LBIST_SEED1_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SEED1_PROXY_R5SS0_LBIST_SEED1_PRPG_DEF_PROXY_MAX (0x001FFFFFU)


/* R5SS0_LBIST_SPARE0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_SPARE0_PROXY_MASK (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_SPARE0_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SPARE0_PROXY_R5SS0_LBIST_SPARE0_SPARE0_PROXY_MAX (0x3FFFFFFFU)


/* R5SS0_LBIST_STAT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_MISR_MUX_CTL_PROXY_MASK (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_MISR_MUX_CTL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_MISR_MUX_CTL_PROXY_MAX (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_OUT_MUX_CTL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_OUT_MUX_CTL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_OUT_MUX_CTL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_BIST_RUNNING_PROXY_MASK (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_BIST_RUNNING_PROXY_SHIFT (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_BIST_RUNNING_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_BIST_DONE_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_BIST_DONE_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_STAT_PROXY_R5SS0_LBIST_STAT_BIST_DONE_PROXY_MAX (0x00000001U)


/* R5SS0_LBIST_MISR_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR_PROXY_R5SS0_LBIST_MISR_MISR_RESULT_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR_PROXY_R5SS0_LBIST_MISR_MISR_RESULT_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_MISR_PROXY_R5SS0_LBIST_MISR_MISR_RESULT_PROXY_MAX (0xFFFFFFFFU)


/* R5SS1_LBIST_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_DIVIDE_RATIO_PROXY_MASK (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_DIVIDE_RATIO_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_DIVIDE_RATIO_PROXY_MAX (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_LOAD_DIV_PROXY_MASK (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_LOAD_DIV_PROXY_SHIFT (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_LOAD_DIV_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_DC_DEF_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_DC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_DC_DEF_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_RUNBIST_MODE_PROXY_MASK (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_RUNBIST_MODE_PROXY_SHIFT (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_RUNBIST_MODE_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_BIST_RUN_PROXY_MASK (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_BIST_RUN_PROXY_SHIFT (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_BIST_RUN_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_BIST_RESET_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_BIST_RESET_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_CTRL_PROXY_R5SS1_LBIST_CTRL_BIST_RESET_PROXY_MAX (0x00000001U)


/* R5SS1_LBIST_PATCOUNT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MASK (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MASK (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_SET_PC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MASK (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_PATCOUNT_PROXY_R5SS1_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MAX (0x00003FFFU)


/* R5SS1_LBIST_SEED0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0_PROXY_R5SS1_LBIST_SEED0_PRPG_DEF_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0_PROXY_R5SS1_LBIST_SEED0_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED0_PROXY_R5SS1_LBIST_SEED0_PRPG_DEF_PROXY_MAX (0xFFFFFFFFU)


/* R5SS1_LBIST_SEED1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1_PROXY_R5SS1_LBIST_SEED1_PRPG_DEF_PROXY_MASK (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1_PROXY_R5SS1_LBIST_SEED1_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SEED1_PROXY_R5SS1_LBIST_SEED1_PRPG_DEF_PROXY_MAX (0x001FFFFFU)


/* R5SS1_LBIST_SPARE0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_SPARE0_PROXY_MASK (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_SPARE0_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SPARE0_PROXY_R5SS1_LBIST_SPARE0_SPARE0_PROXY_MAX (0x3FFFFFFFU)


/* R5SS1_LBIST_STAT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_MISR_MUX_CTL_PROXY_MASK (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_MISR_MUX_CTL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_MISR_MUX_CTL_PROXY_MAX (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_OUT_MUX_CTL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_OUT_MUX_CTL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_OUT_MUX_CTL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_BIST_RUNNING_PROXY_MASK (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_BIST_RUNNING_PROXY_SHIFT (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_BIST_RUNNING_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_BIST_DONE_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_BIST_DONE_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_STAT_PROXY_R5SS1_LBIST_STAT_BIST_DONE_PROXY_MAX (0x00000001U)


/* R5SS1_LBIST_MISR_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR_PROXY_R5SS1_LBIST_MISR_MISR_RESULT_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR_PROXY_R5SS1_LBIST_MISR_MISR_RESULT_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_MISR_PROXY_R5SS1_LBIST_MISR_MISR_RESULT_PROXY_MAX (0xFFFFFFFFU)


/* C70_LBIST_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_DIVIDE_RATIO_PROXY_MASK (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_DIVIDE_RATIO_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_DIVIDE_RATIO_PROXY_MAX (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_LOAD_DIV_PROXY_MASK (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_LOAD_DIV_PROXY_SHIFT (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_LOAD_DIV_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_DC_DEF_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_DC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_DC_DEF_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_RUNBIST_MODE_PROXY_MASK (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_RUNBIST_MODE_PROXY_SHIFT (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_RUNBIST_MODE_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_BIST_RUN_PROXY_MASK (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_BIST_RUN_PROXY_SHIFT (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_BIST_RUN_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_BIST_RESET_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_BIST_RESET_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_CTRL_PROXY_C70_LBIST_CTRL_BIST_RESET_PROXY_MAX (0x00000001U)


/* C70_LBIST_PATCOUNT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MASK (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MASK (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_SET_PC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MASK (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_PATCOUNT_PROXY_C70_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MAX (0x00003FFFU)


/* C70_LBIST_SEED0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0_PROXY_C70_LBIST_SEED0_PRPG_DEF_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0_PROXY_C70_LBIST_SEED0_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED0_PROXY_C70_LBIST_SEED0_PRPG_DEF_PROXY_MAX (0xFFFFFFFFU)


/* C70_LBIST_SEED1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1_PROXY_C70_LBIST_SEED1_PRPG_DEF_PROXY_MASK (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1_PROXY_C70_LBIST_SEED1_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SEED1_PROXY_C70_LBIST_SEED1_PRPG_DEF_PROXY_MAX (0x001FFFFFU)


/* C70_LBIST_SPARE0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_SPARE0_PROXY_MASK (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_SPARE0_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE0_PROXY_C70_LBIST_SPARE0_SPARE0_PROXY_MAX (0x3FFFFFFFU)


/* C70_LBIST_SPARE1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1_PROXY_C70_LBIST_SPARE1_SPARE1_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1_PROXY_C70_LBIST_SPARE1_SPARE1_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SPARE1_PROXY_C70_LBIST_SPARE1_SPARE1_PROXY_MAX (0xFFFFFFFFU)


/* C70_LBIST_STAT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_MISR_MUX_CTL_PROXY_MASK (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_MISR_MUX_CTL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_MISR_MUX_CTL_PROXY_MAX (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_OUT_MUX_CTL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_OUT_MUX_CTL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_OUT_MUX_CTL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_BIST_RUNNING_PROXY_MASK (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_BIST_RUNNING_PROXY_SHIFT (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_BIST_RUNNING_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_BIST_DONE_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_BIST_DONE_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_STAT_PROXY_C70_LBIST_STAT_BIST_DONE_PROXY_MAX (0x00000001U)


/* MMA0_LBIST_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_DIVIDE_RATIO_PROXY_MASK (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_DIVIDE_RATIO_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_DIVIDE_RATIO_PROXY_MAX (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_LOAD_DIV_PROXY_MASK (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_LOAD_DIV_PROXY_SHIFT (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_LOAD_DIV_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_DC_DEF_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_DC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_DC_DEF_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_RUNBIST_MODE_PROXY_MASK (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_RUNBIST_MODE_PROXY_SHIFT (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_RUNBIST_MODE_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_BIST_RUN_PROXY_MASK (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_BIST_RUN_PROXY_SHIFT (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_BIST_RUN_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_BIST_RESET_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_BIST_RESET_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_CTRL_PROXY_MMA0_LBIST_CTRL_BIST_RESET_PROXY_MAX (0x00000001U)


/* MMA0_LBIST_PATCOUNT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MASK (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MASK (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MASK (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_PATCOUNT_PROXY_MMA0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MAX (0x00003FFFU)


/* MMA0_LBIST_SEED0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0_PROXY_MMA0_LBIST_SEED0_PRPG_DEF_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0_PROXY_MMA0_LBIST_SEED0_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED0_PROXY_MMA0_LBIST_SEED0_PRPG_DEF_PROXY_MAX (0xFFFFFFFFU)


/* MMA0_LBIST_SEED1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1_PROXY_MMA0_LBIST_SEED1_PRPG_DEF_PROXY_MASK (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1_PROXY_MMA0_LBIST_SEED1_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SEED1_PROXY_MMA0_LBIST_SEED1_PRPG_DEF_PROXY_MAX (0x001FFFFFU)


/* MMA0_LBIST_SPARE0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_SPARE0_PROXY_MASK (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_SPARE0_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SPARE0_PROXY_MMA0_LBIST_SPARE0_SPARE0_PROXY_MAX (0x3FFFFFFFU)


/* MMA0_LBIST_STAT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_MISR_MUX_CTL_PROXY_MASK (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_MISR_MUX_CTL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_MISR_MUX_CTL_PROXY_MAX (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_OUT_MUX_CTL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_OUT_MUX_CTL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_OUT_MUX_CTL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_BIST_RUNNING_PROXY_MASK (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_BIST_RUNNING_PROXY_SHIFT (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_BIST_RUNNING_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_BIST_DONE_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_BIST_DONE_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_STAT_PROXY_MMA0_LBIST_STAT_BIST_DONE_PROXY_MAX (0x00000001U)


/* MMA0_LBIST_MISR_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR_PROXY_MMA0_LBIST_MISR_MISR_RESULT_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR_PROXY_MMA0_LBIST_MISR_MISR_RESULT_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_MISR_PROXY_MMA0_LBIST_MISR_MISR_RESULT_PROXY_MAX (0xFFFFFFFFU)


/* HWA0_LBIST_CTRL_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_DIVIDE_RATIO_PROXY_MASK (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_DIVIDE_RATIO_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_DIVIDE_RATIO_PROXY_MAX (0x0000001FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_LOAD_DIV_PROXY_MASK (0x00000080U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_LOAD_DIV_PROXY_SHIFT (0x00000007U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_LOAD_DIV_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_DC_DEF_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_DC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_DC_DEF_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_RUNBIST_MODE_PROXY_MASK (0x0000F000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_RUNBIST_MODE_PROXY_SHIFT (0x0000000CU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_RUNBIST_MODE_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_BIST_RUN_PROXY_MASK (0x0F000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_BIST_RUN_PROXY_SHIFT (0x00000018U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_BIST_RUN_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_BIST_RESET_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_BIST_RESET_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_CTRL_PROXY_HWA0_LBIST_CTRL_BIST_RESET_PROXY_MAX (0x00000001U)


/* HWA0_LBIST_PATCOUNT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MASK (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_SCAN_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MASK (0x000000F0U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_SHIFT (0x00000004U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_RESET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MASK (0x00000F00U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_SET_PC_DEF_PROXY_MAX (0x0000000FU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MASK (0x3FFF0000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_SHIFT (0x00000010U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_PATCOUNT_PROXY_HWA0_LBIST_PATCOUNT_STATIC_PC_DEF_PROXY_MAX (0x00003FFFU)


/* HWA0_LBIST_SEED0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0_PROXY_HWA0_LBIST_SEED0_PRPG_DEF_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0_PROXY_HWA0_LBIST_SEED0_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED0_PROXY_HWA0_LBIST_SEED0_PRPG_DEF_PROXY_MAX (0xFFFFFFFFU)


/* HWA0_LBIST_SEED1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1_PROXY_HWA0_LBIST_SEED1_PRPG_DEF_PROXY_MASK (0x001FFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1_PROXY_HWA0_LBIST_SEED1_PRPG_DEF_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SEED1_PROXY_HWA0_LBIST_SEED1_PRPG_DEF_PROXY_MAX (0x001FFFFFU)


/* HWA0_LBIST_SPARE0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MASK (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_LBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MASK (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_SHIFT (0x00000001U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_PBIST_SELFTEST_EN_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_SPARE0_PROXY_MASK (0xFFFFFFFCU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_SPARE0_PROXY_SHIFT (0x00000002U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SPARE0_PROXY_HWA0_LBIST_SPARE0_SPARE0_PROXY_MAX (0x3FFFFFFFU)


/* HWA0_LBIST_STAT_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_MISR_MUX_CTL_PROXY_MASK (0x000000FFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_MISR_MUX_CTL_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_MISR_MUX_CTL_PROXY_MAX (0x000000FFU)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_OUT_MUX_CTL_PROXY_MASK (0x00000300U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_OUT_MUX_CTL_PROXY_SHIFT (0x00000008U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_OUT_MUX_CTL_PROXY_MAX (0x00000003U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_BIST_RUNNING_PROXY_MASK (0x00008000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_BIST_RUNNING_PROXY_SHIFT (0x0000000FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_BIST_RUNNING_PROXY_MAX (0x00000001U)

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_BIST_DONE_PROXY_MASK (0x80000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_BIST_DONE_PROXY_SHIFT (0x0000001FU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_STAT_PROXY_HWA0_LBIST_STAT_BIST_DONE_PROXY_MAX (0x00000001U)


/* HWA0_LBIST_MISR_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR_PROXY_HWA0_LBIST_MISR_MISR_RESULT_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR_PROXY_HWA0_LBIST_MISR_MISR_RESULT_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_MISR_PROXY_HWA0_LBIST_MISR_MISR_RESULT_PROXY_MAX (0xFFFFFFFFU)


/* R5SS0_LBIST_SIG_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG_PROXY_R5SS0_LBIST_SIG_MISR_SIG_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG_PROXY_R5SS0_LBIST_SIG_MISR_SIG_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS0_LBIST_SIG_PROXY_R5SS0_LBIST_SIG_MISR_SIG_PROXY_MAX (0xFFFFFFFFU)


/* R5SS1_LBIST_SIG_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG_PROXY_R5SS1_LBIST_SIG_MISR_SIG_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG_PROXY_R5SS1_LBIST_SIG_MISR_SIG_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_R5SS1_LBIST_SIG_PROXY_R5SS1_LBIST_SIG_MISR_SIG_PROXY_MAX (0xFFFFFFFFU)


/* HWA0_LBIST_SIG_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG_PROXY_HWA0_LBIST_SIG_MISR_SIG_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG_PROXY_HWA0_LBIST_SIG_MISR_SIG_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_HWA0_LBIST_SIG_PROXY_HWA0_LBIST_SIG_MISR_SIG_PROXY_MAX (0xFFFFFFFFU)


/* C70_LBIST_SIG_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG_PROXY_C70_LBIST_SIG_MISR_SIG_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG_PROXY_C70_LBIST_SIG_MISR_SIG_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_C70_LBIST_SIG_PROXY_C70_LBIST_SIG_MISR_SIG_PROXY_MAX (0xFFFFFFFFU)


/* MMA0_LBIST_SIG_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG_PROXY_MMA0_LBIST_SIG_MISR_SIG_PROXY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG_PROXY_MMA0_LBIST_SIG_MISR_SIG_PROXY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_MMA0_LBIST_SIG_PROXY_MMA0_LBIST_SIG_MISR_SIG_PROXY_MAX (0xFFFFFFFFU)


/* LOCK3_KICK0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0_PROXY_LOCK3_KICK0_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0_PROXY_LOCK3_KICK0_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK0_PROXY_LOCK3_KICK0_PROXY_MAX         (0xFFFFFFFFU)


/* LOCK3_KICK1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1_PROXY_LOCK3_KICK1_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1_PROXY_LOCK3_KICK1_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK3_KICK1_PROXY_LOCK3_KICK1_PROXY_MAX         (0xFFFFFFFFU)


/* CLAIMREG_P3_R0 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0_CLAIMREG_P3_R0_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0_CLAIMREG_P3_R0_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R0_CLAIMREG_P3_R0_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P3_R1 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1_CLAIMREG_P3_R1_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1_CLAIMREG_P3_R1_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R1_CLAIMREG_P3_R1_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P3_R2 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2_CLAIMREG_P3_R2_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2_CLAIMREG_P3_R2_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R2_CLAIMREG_P3_R2_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P3_R3 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3_CLAIMREG_P3_R3_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3_CLAIMREG_P3_R3_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R3_CLAIMREG_P3_R3_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P3_R4 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4_CLAIMREG_P3_R4_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4_CLAIMREG_P3_R4_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R4_CLAIMREG_P3_R4_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P3_R5 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5_CLAIMREG_P3_R5_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5_CLAIMREG_P3_R5_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R5_CLAIMREG_P3_R5_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P3_R6 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6_CLAIMREG_P3_R6_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6_CLAIMREG_P3_R6_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R6_CLAIMREG_P3_R6_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P3_R7 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7_CLAIMREG_P3_R7_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7_CLAIMREG_P3_R7_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P3_R7_CLAIMREG_P3_R7_MAX               (0xFFFFFFFFU)


/* LOCK4_KICK0 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0_LOCK4_KICK0_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0_LOCK4_KICK0_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0_LOCK4_KICK0_MAX                     (0xFFFFFFFFU)


/* LOCK4_KICK1 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1_LOCK4_KICK1_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1_LOCK4_KICK1_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1_LOCK4_KICK1_MAX                     (0xFFFFFFFFU)


/* CLAIMREG_P4_R0_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0_READONLY_CLAIMREG_P4_R0_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0_READONLY_CLAIMREG_P4_R0_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0_READONLY_CLAIMREG_P4_R0_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R1_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1_READONLY_CLAIMREG_P4_R1_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1_READONLY_CLAIMREG_P4_R1_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1_READONLY_CLAIMREG_P4_R1_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R2_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2_READONLY_CLAIMREG_P4_R2_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2_READONLY_CLAIMREG_P4_R2_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2_READONLY_CLAIMREG_P4_R2_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R3_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3_READONLY_CLAIMREG_P4_R3_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3_READONLY_CLAIMREG_P4_R3_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3_READONLY_CLAIMREG_P4_R3_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R4_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4_READONLY_CLAIMREG_P4_R4_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4_READONLY_CLAIMREG_P4_R4_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4_READONLY_CLAIMREG_P4_R4_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R5_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5_READONLY_CLAIMREG_P4_R5_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5_READONLY_CLAIMREG_P4_R5_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5_READONLY_CLAIMREG_P4_R5_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R6_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6_READONLY_CLAIMREG_P4_R6_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6_READONLY_CLAIMREG_P4_R6_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6_READONLY_CLAIMREG_P4_R6_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R7_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7_READONLY_CLAIMREG_P4_R7_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7_READONLY_CLAIMREG_P4_R7_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7_READONLY_CLAIMREG_P4_R7_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R8_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8_READONLY_CLAIMREG_P4_R8_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8_READONLY_CLAIMREG_P4_R8_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8_READONLY_CLAIMREG_P4_R8_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R9_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9_READONLY_CLAIMREG_P4_R9_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9_READONLY_CLAIMREG_P4_R9_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9_READONLY_CLAIMREG_P4_R9_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R10_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10_READONLY_CLAIMREG_P4_R10_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10_READONLY_CLAIMREG_P4_R10_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10_READONLY_CLAIMREG_P4_R10_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R11_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11_READONLY_CLAIMREG_P4_R11_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11_READONLY_CLAIMREG_P4_R11_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11_READONLY_CLAIMREG_P4_R11_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R12_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12_READONLY_CLAIMREG_P4_R12_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12_READONLY_CLAIMREG_P4_R12_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12_READONLY_CLAIMREG_P4_R12_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R13_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13_READONLY_CLAIMREG_P4_R13_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13_READONLY_CLAIMREG_P4_R13_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13_READONLY_CLAIMREG_P4_R13_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P4_R14_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14_READONLY_CLAIMREG_P4_R14_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14_READONLY_CLAIMREG_P4_R14_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14_READONLY_CLAIMREG_P4_R14_READONLY_MAX (0xFFFFFFFFU)


/* LOCK4_KICK0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0_PROXY_LOCK4_KICK0_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0_PROXY_LOCK4_KICK0_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK0_PROXY_LOCK4_KICK0_PROXY_MAX         (0xFFFFFFFFU)


/* LOCK4_KICK1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1_PROXY_LOCK4_KICK1_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1_PROXY_LOCK4_KICK1_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK4_KICK1_PROXY_LOCK4_KICK1_PROXY_MAX         (0xFFFFFFFFU)


/* CLAIMREG_P4_R0 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0_CLAIMREG_P4_R0_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0_CLAIMREG_P4_R0_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R0_CLAIMREG_P4_R0_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R1 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1_CLAIMREG_P4_R1_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1_CLAIMREG_P4_R1_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R1_CLAIMREG_P4_R1_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R2 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2_CLAIMREG_P4_R2_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2_CLAIMREG_P4_R2_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R2_CLAIMREG_P4_R2_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R3 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3_CLAIMREG_P4_R3_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3_CLAIMREG_P4_R3_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R3_CLAIMREG_P4_R3_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R4 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4_CLAIMREG_P4_R4_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4_CLAIMREG_P4_R4_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R4_CLAIMREG_P4_R4_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R5 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5_CLAIMREG_P4_R5_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5_CLAIMREG_P4_R5_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R5_CLAIMREG_P4_R5_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R6 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6_CLAIMREG_P4_R6_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6_CLAIMREG_P4_R6_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R6_CLAIMREG_P4_R6_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R7 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7_CLAIMREG_P4_R7_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7_CLAIMREG_P4_R7_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R7_CLAIMREG_P4_R7_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R8 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8_CLAIMREG_P4_R8_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8_CLAIMREG_P4_R8_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R8_CLAIMREG_P4_R8_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R9 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9_CLAIMREG_P4_R9_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9_CLAIMREG_P4_R9_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R9_CLAIMREG_P4_R9_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P4_R10 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10_CLAIMREG_P4_R10_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10_CLAIMREG_P4_R10_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R10_CLAIMREG_P4_R10_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P4_R11 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11_CLAIMREG_P4_R11_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11_CLAIMREG_P4_R11_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R11_CLAIMREG_P4_R11_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P4_R12 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12_CLAIMREG_P4_R12_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12_CLAIMREG_P4_R12_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R12_CLAIMREG_P4_R12_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P4_R13 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13_CLAIMREG_P4_R13_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13_CLAIMREG_P4_R13_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R13_CLAIMREG_P4_R13_MAX             (0xFFFFFFFFU)


/* CLAIMREG_P4_R14 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14_CLAIMREG_P4_R14_MASK            (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14_CLAIMREG_P4_R14_SHIFT           (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P4_R14_CLAIMREG_P4_R14_MAX             (0xFFFFFFFFU)


/* LOCK6_KICK0 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0_LOCK6_KICK0_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0_LOCK6_KICK0_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0_LOCK6_KICK0_MAX                     (0xFFFFFFFFU)


/* LOCK6_KICK1 */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1_LOCK6_KICK1_MASK                    (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1_LOCK6_KICK1_SHIFT                   (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1_LOCK6_KICK1_MAX                     (0xFFFFFFFFU)


/* CLAIMREG_P6_R0_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0_READONLY_CLAIMREG_P6_R0_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0_READONLY_CLAIMREG_P6_R0_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0_READONLY_CLAIMREG_P6_R0_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R1_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1_READONLY_CLAIMREG_P6_R1_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1_READONLY_CLAIMREG_P6_R1_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1_READONLY_CLAIMREG_P6_R1_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R2_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2_READONLY_CLAIMREG_P6_R2_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2_READONLY_CLAIMREG_P6_R2_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2_READONLY_CLAIMREG_P6_R2_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R3_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3_READONLY_CLAIMREG_P6_R3_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3_READONLY_CLAIMREG_P6_R3_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3_READONLY_CLAIMREG_P6_R3_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R4_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4_READONLY_CLAIMREG_P6_R4_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4_READONLY_CLAIMREG_P6_R4_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4_READONLY_CLAIMREG_P6_R4_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R5_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5_READONLY_CLAIMREG_P6_R5_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5_READONLY_CLAIMREG_P6_R5_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5_READONLY_CLAIMREG_P6_R5_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R6_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6_READONLY_CLAIMREG_P6_R6_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6_READONLY_CLAIMREG_P6_R6_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6_READONLY_CLAIMREG_P6_R6_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R7_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7_READONLY_CLAIMREG_P6_R7_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7_READONLY_CLAIMREG_P6_R7_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7_READONLY_CLAIMREG_P6_R7_READONLY_MAX (0xFFFFFFFFU)


/* CLAIMREG_P6_R8_READONLY */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8_READONLY_CLAIMREG_P6_R8_READONLY_MASK (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8_READONLY_CLAIMREG_P6_R8_READONLY_SHIFT (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8_READONLY_CLAIMREG_P6_R8_READONLY_MAX (0xFFFFFFFFU)


/* LOCK6_KICK0_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0_PROXY_LOCK6_KICK0_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0_PROXY_LOCK6_KICK0_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK0_PROXY_LOCK6_KICK0_PROXY_MAX         (0xFFFFFFFFU)


/* LOCK6_KICK1_PROXY */

#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1_PROXY_LOCK6_KICK1_PROXY_MASK        (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1_PROXY_LOCK6_KICK1_PROXY_SHIFT       (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_LOCK6_KICK1_PROXY_LOCK6_KICK1_PROXY_MAX         (0xFFFFFFFFU)


/* CLAIMREG_P6_R0 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0_CLAIMREG_P6_R0_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0_CLAIMREG_P6_R0_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R0_CLAIMREG_P6_R0_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R1 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1_CLAIMREG_P6_R1_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1_CLAIMREG_P6_R1_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R1_CLAIMREG_P6_R1_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R2 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2_CLAIMREG_P6_R2_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2_CLAIMREG_P6_R2_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R2_CLAIMREG_P6_R2_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R3 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3_CLAIMREG_P6_R3_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3_CLAIMREG_P6_R3_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R3_CLAIMREG_P6_R3_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R4 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4_CLAIMREG_P6_R4_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4_CLAIMREG_P6_R4_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R4_CLAIMREG_P6_R4_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R5 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5_CLAIMREG_P6_R5_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5_CLAIMREG_P6_R5_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R5_CLAIMREG_P6_R5_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R6 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6_CLAIMREG_P6_R6_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6_CLAIMREG_P6_R6_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R6_CLAIMREG_P6_R6_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R7 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7_CLAIMREG_P6_R7_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7_CLAIMREG_P6_R7_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R7_CLAIMREG_P6_R7_MAX               (0xFFFFFFFFU)


/* CLAIMREG_P6_R8 */

#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8_CLAIMREG_P6_R8_MASK              (0xFFFFFFFFU)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8_CLAIMREG_P6_R8_SHIFT             (0x00000000U)
#define CSL_MAIN_CTRL_MMR_CFG0_CLAIMREG_P6_R8_CLAIMREG_P6_R8_MAX               (0xFFFFFFFFU)


#ifdef __cplusplus
}
#endif
#endif
