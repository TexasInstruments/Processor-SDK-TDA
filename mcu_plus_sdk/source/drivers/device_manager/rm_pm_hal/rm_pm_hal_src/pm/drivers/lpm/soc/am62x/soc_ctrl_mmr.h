/*
 * DM Firmware
 *
 * am62x soc_ctrl_mmr.h
 *
 * Copyright (C) 2021-2025, Texas Instruments Incorporated
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

#ifndef LPM_SOC_CTRL_MMR_H_
#define LPM_SOC_CTRL_MMR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define WKUP_CTRL_BASE                      (0x43000000UL)

/* WFI status of cores */
#define WKUP_CTRL_WFI_STATUS                (0x18400UL)
#define SMS_CPU0_WFI_MASK                    BIT(2)

/* Device Manager Deep sleep reset mask */
#define WKUP_CTRL_DS_DM_RESET               (0x00018440U)
#define DS_DM_RESET_UNMASK                   0xFU

/* PMCTRL IO0(WKUP) and IO1(MAIN) IOs */
#define WKUP_CTRL_PMCTRL_IO_0               (0x18084U)
#define WKUP_CTRL_PMCTRL_IO_1               (0x18088U)
#define WKUP_CTRL_PMCTRL_IO_0_ISOCLK_OVRD   BIT(0)
#define WKUP_CTRL_PMCTRL_IO_0_ISOOVR_EXTEND BIT(4)
#define WKUP_CTRL_PMCTRL_IO_0_ISO_BYPASS    BIT(6)
#define WKUP_CTRL_PMCTRL_IO_0_WUCLK_CTRL    BIT(8)
#define WKUP_CTRL_PMCTRL_IO_0_IO_ISO_STATUS BIT(25)
#define WKUP_CTRL_PMCTRL_IO_0_WUCLK_STATUS_ENABLED  1U
#define WKUP_CTRL_PMCTRL_IO_0_WUCLK_STATUS_DISABLED 0U
#define WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN   BIT(16)
#define WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL   BIT(24)
#define WKUP_CTRL_PMCTRL_IO_0_WRITE_MASK (WKUP_CTRL_PMCTRL_IO_0_ISOCLK_OVRD	\
					  | WKUP_CTRL_PMCTRL_IO_0_ISOOVR_EXTEND	 \
					  | WKUP_CTRL_PMCTRL_IO_0_ISO_BYPASS	 \
					  | WKUP_CTRL_PMCTRL_IO_0_WUCLK_CTRL	 \
					  | WKUP_CTRL_PMCTRL_IO_0_GLOBAL_WUEN	 \
					  | WKUP_CTRL_PMCTRL_IO_0_IO_ISO_CTRL)

/* IOs DEEPSLEEP CTRL */
#define WKUP_CTRL_DEEPSLEEP_CTRL                (0x18160U)
#define WKUP_CTRL_DEEPSLEEP_CTRL_ENABLE_IO      (0x101U)
#define WKUP_CTRL_DEEPSLEEP_CTRL_DISABLE_IO      0

/* PMCTRL IOs GLOBAL CTRL */
#define WKUP_CTRL_PMCTRL_IO_GLB                 (0x1809CU)
#define WKUP_CTRL_PMCTRL_IO_GLB_ENABLE_IO        1
#define WKUP_CTRL_PMCTRL_IO_GLB_DISABLE_IO       0

/* CANUART WAKE CTRL */
#define WKUP_CANUART_CTRL                       (0x00018300U)
#define WKUP_CANUART_WAKE_STAT0                 (0x00018308U)
#define WKUP_CANUART_WAKE_STAT1                 (0x0001830CU)
#define WKUP_CANUART_MAGIC_WRD                  (0x2AAAAAAAU << 1U)
#define WKUP_CANUART_MAGIC_WRD_LD_EN            (0x1U)
#define WKUP_CANUART_IO_MODE_ACTIVE             (0x1U)

/* WKUP_PMCTRL_SYS */
#define WKUP0_PMCTRL_SYS                        (0x00018080U)
#define WKUP0_PMCTRL_SYS_LPM_EN_DS              (0x5U)
#define WKUP0_PMCTRL_SYS_LPM_EN_PMIC            (0x0U)
#define WKUP0_LPM_PMIC_OUT_EN                   (0x10U)
#define WKUP0_LPM_PMIC_OUT_DIS                  (0x0U)

/* WKUP0_EN */
#define WKUP0_EN                                (0x00018180U)
#define WKUP0_EN_CANUART_IO_DAISY_CHAIN          BIT(18)

/* BACKUP0 REG */
#define WKUP_BACKUP0_REG                        (0x0001C100U)

#define WKUP_PIN_SRC_REG                        WKUP_BACKUP0_REG
#define WKUP_PIN_SRC_CLR                        (0U)

/* SOC Device IDs */
#define DEV_GTC                 AM62X_DEV_WKUP_GTC0
#define POWER_MASTER            AM62X_DEV_A53SS0_CORE_0
#define POWER_MASTER_CLUSTER    AM62X_DEV_A53SS0
#define USB0_DEV_ID             AM62X_DEV_USB0
#define USB1_DEV_ID             AM62X_DEV_USB1
#define USB0_ISO_DEV_ID         AM62X_DEV_MAIN_USB0_ISO_VD
#define USB1_ISO_DEV_ID         AM62X_DEV_MAIN_USB1_ISO_VD

/* GTC Config base */
#define GTC_CFG0_BASE           (0xA80000UL)
#define GTC_CFG1_BASE           (0xA90000UL)

/* MAIN PADCFG address */
#define MAIN_PADCFG_CTRL_BASE   (0xf0000UL)
#define PADCFG_OFFSET           (0x4000U)
#define MAIN_PADCFG_REG_END     (0x42A8U)

/* MCU PADCFG address */
#define MCU_PADCFG_CTRL_BASE    (0x4080000UL)
#define MCU_PADCFG_REG_END      (0x4084U)

/* CAN PADCFG range */
#define PADCFG_CAN_START_OFFSET (0x4014U)
#define PADCFG_CAN_END_OFFSET   (0x4040U)

#define RTC_BASE                (0x2b1f0000UL)

#define MAIN_CTRL_BASE          (0x100000UL)
#define MCU_CTRL_BASE           (0x04500000UL)
#define MAX_MMR_DATA            (20U)

/* DEVGRP and LPSC ID */
#define MAIN_DEVGRP             DEVGRP_00
#define ALWAYS_ON_LPSC_ID       0U

/* Deepest low power mode supported according to fw caps */
#define DEEPEST_LOW_POWER_MODE  TISCI_MSG_VALUE_SLEEP_MODE_DEEP_SLEEP

/* SOC MCU Core Host ID */
#define MCU_HOST_ID             HOST_ID_M4_0
#define VIM_BASE_ADDR           (0x2fff0000U)

#define WKUP_TIMER1_BASE_ADDR   (0x2b110000U)

/* Devgroup having RM resources */
#define RM_RSRC_DEVGRP          MAIN_DEVGRP

/* CAN IO Pin Numbers that can wakeup the SoC from partial I/O mode */
#define CAN_IO_WKUP_PIN_NUM_START   (5U)
#define CAN_IO_WKUP_PIN_NUM_END     (16U)

#ifdef __cplusplus
}


#endif
#endif /* LPM_SOC_CTRL_MMR_H_ */
