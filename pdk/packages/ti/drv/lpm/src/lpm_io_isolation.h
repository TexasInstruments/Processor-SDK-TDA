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
 * \file lpm_io_isolation.h
 *
 * \brief Register offsets and field descriptions related to io-isolation configuration
 *
 */

#ifndef LPM_IO_ISOLATION_H_
#define LPM_IO_ISOLATION_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* PMCTRL register offsets part of the DMSC, used for IO_ISOLATION configuration on J7200.
 * This register region starts from CSL_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START
 */

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK0                          (0x00000020U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK1                          (0x00000024U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT0_VAL                     (0x00000040U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT0_SET                     (0x00000044U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_STS_IN0                           (0x0000004CU)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT1_VAL                     (0x00000050U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT1_SET                     (0x00000054U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1                          (0x00000088U)

/* LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1 */

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISOBYPASS_MASK        (0x00000040U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISOBYPASS_SHIFT       (0x00000006U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISOBYPASS_EN          (0x1U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISOBYPASS_DIS         (0x0U)

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_CTRL_MASK          (0x00000100U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_CTRL_SHIFT         (0x00000008U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_CTRL_EN            (0x1U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_CTRL_DIS           (0x0U)

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_STATUS_MASK        (0x00000200U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_WUCLK_STATUS_SHIFT       (0x00000009U)

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_GLOBAL_WUEN_MASK         (0x00010000U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_GLOBAL_WUEN_SHIFT        (0x00000010U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_GLOBAL_WUEN_EN           (0x1U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_GLOBAL_WUEN_DIS          (0x0U)

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_CTRL_MASK         (0x01000000U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_CTRL_SHIFT        (0x00000018U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_CTRL_EN           (0x1U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_CTRL_DIS          (0x0U)

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_STATUS_0_MASK     (0x02000000U)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_PMCTRL_IO_1_IO_ISO_STATUS_0_SHIFT    (0x00000019U)

/* WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL */

#define LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_MAGIC_WORD_LOAD_ENABLE_MASK                        (0x00000001U)
#define LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_MAGIC_WORD_LOAD_ENABLE_SHIFT                       (0x00000000U)
#define LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_MAGIC_WORD_LOAD_ENABLE_EN                          (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_MAGIC_WORD_LOAD_ENABLE_DIS                         (0x0U)

#define LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_IO_MAGIC_WORD_MASK                                 (0xFFFFFFFEU)
#define LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_IO_MAGIC_WORD_SHIFT                                (0x00000001U)
#define LPM_WKUP_CTRL_MMR_CFG0_CANUART_WAKE_CTRL_IO_MAGIC_WORD                                      (0x2AAAAAAAU << 1U)

/* WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL */

#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_WKUP_MASK                                    (0x00000001U)
#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_WKUP_SHIFT                                   (0x00000000U)
#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_WKUP_EN                                      (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_WKUP_DIS                                     (0x0U)

#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_MAIN_MASK                                    (0x00000100U)
#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_MAIN_SHIFT                                   (0x00000008U)
#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_MAIN_EN                                      (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_DEEPSLEEP_CTRL_FORCE_DS_MAIN_DIS                                     (0x0U)

/* WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0 (used for J784S4/J742S2) */

#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISOBYPASS_MASK                                        (0x00000040U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISOBYPASS_SHIFT                                       (0x00000006U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISOBYPASS_EN                                          (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISOBYPASS_DIS                                         (0x0U)

#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_WUCLK_CTRL_MASK                                          (0x00000100U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_WUCLK_CTRL_SHIFT                                         (0x00000008U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_WUCLK_CTRL_EN                                            (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_WUCLK_CTRL_DIS                                           (0x0U)

#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_WUCLK_STATUS_MASK                                        (0x00000200U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_WUCLK_STATUS_SHIFT                                       (0x00000009U)

#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_GLOBAL_WUEN_MASK                                         (0x00010000U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_GLOBAL_WUEN_SHIFT                                        (0x00000010U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_GLOBAL_WUEN_EN                                           (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_GLOBAL_WUEN_DIS                                          (0x0U)

#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_CTRL_MASK                                         (0x01000000U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_CTRL_SHIFT                                        (0x00000018U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_CTRL_EN                                           (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_CTRL_DIS                                          (0x0U)

#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_STATUS_0_MASK                                     (0x02000000U)
#define LPM_WKUP_CTRL_MMR_CFG0_PMCTRL_IO_0_IO_ISO_STATUS_0_SHIFT                                    (0x00000019U)

/* WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL */

#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL_WAKE_EN_MASK                                           (0x00010000U)
#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL_WAKE_EN_SHIFT                                          (0x00000010U)
#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL_WAKE_EN_EN                                             (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_MAIN_PWR_CTRL_WAKE_EN_DIS                                            (0x0U)

/* WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET */

#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_0_MASK                                   (0x00000001U)
#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_0_SHIFT                                  (0x00000000U)
#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_0_EN                                     (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_0_DIS                                    (0x0U)

#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_1_MASK                                   (0x00000002U)
#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_1_SHIFT                                  (0x00000001U)
#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_1_EN                                     (0x1U)
#define LPM_WKUP_CTRL_MMR_CFG0_FW_CTRL_OUT0_SET_IO_PM_CTRL_1_DIS                                    (0x0U)

/* CTRL_MMR_CFG0_PADCONFIGxx (common field layout for all MAIN and WKUP pad config registers) */

#define LPM_CTRL_MMR_CFG0_PADCONFIG_MUXMODE_MASK                                                    (0x0000000FU)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_MUXMODE_SHIFT                                                   (0x00000000U)
#define LPM_PMIC_WAKE0n_MUXMODE                                                                     (0x0000000FU)

#define LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_MASK                                                  (0x00040000U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_SHIFT                                                 (0x00000012U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_EN                                                    (0x1U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_RX_ACTIVE_DIS                                                   (0x0U)

#define LPM_CTRL_MMR_CFG0_PADCONFIG_TX_DIS_MASK                                                     (0x00200000U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_TX_DIS_SHIFT                                                    (0x00000015U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_TX_DIS_EN                                                       (0x1U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_TX_DIS_DIS                                                      (0x0U)

#define LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_MASK                                             (0x00800000U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_SHIFT                                            (0x00000017U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_EN                                               (0x1U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_ISO_BYPASSMODE_DIS                                              (0x0U)

#define LPM_CTRL_MMR_CFG0_PADCONFIG_DSOUT_DIS_MASK                                                  (0x02000000U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_DSOUT_DIS_SHIFT                                                 (0x00000019U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_DSOUT_DIS_EN                                                    (0x1U)
#define LPM_CTRL_MMR_CFG0_PADCONFIG_DSOUT_DIS_DIS                                                   (0x0U)

/* Unlock values for DMSC PMCTRL MMRs  */

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK0_VAL                      (0x8A6B7CDAU)
#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_LOCK0_KICK1_VAL                      (0x823CAEF9U)

/* LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT0_SET */

#define LPM_STD_FW_WKUP_DMSC0_PWRCTRL_0_DMSC_PWR_MMR_PWR_START_FW_CTRL_OUT0_SET_PMCTRL1             (0x00000002U)

#endif /* LPM_IO_ISOLATION_H_ */
