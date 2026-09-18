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
 * \file lpm_ddr.h
 *
 * \brief DDR self-refresh functions and macros used in LPM stub.
 *
 */

#ifndef LPM_DDR_H_
#define LPM_DDR_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* MAIN_PLL_MMR_CFG_PLLn_CTRL */

#define LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_MASK                                       (0x80000000U)
#define LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_SHIFT                                      (0x0000001FU)
#define LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_EN                                         (0x1U)
#define LPM_DDR_MAIN_PLL_MMR_CFG_PLLn_CTRL_PLL_BYPASS_EN_DIS                                        (0x0U)

/* EMIF_CTLCFG_DENALI_PI_12 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_12_PI_FREQ_MAP_FSP1_MASK                                      (0x00000002U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_12_PI_FREQ_MAP_FSP1_SHIFT                                     (0x00000001U)

#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_12_PI_FREQ_MAP_FSP2_MASK                                      (0x00000004U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_12_PI_FREQ_MAP_FSP2_SHIFT                                     (0x00000002U)

/* EMIF_CTLCFG_DENALI_PI_64 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_64_PI_MC_DFS_PI_SET_VREF_ENABLE_MASK                          (0x00000001U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_64_PI_MC_DFS_PI_SET_VREF_ENABLE_SHIFT                         (0x00000000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_64_PI_MC_DFS_PI_SET_VREF_ENABLE_EN                            (0x1U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_64_PI_MC_DFS_PI_SET_VREF_ENABLE_DIS                           (0x0U)

/* EMIF_CTLCFG_DENALI_PI_212 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_212_PI_WDQLVL_EN_F0_MASK                                      (0x00000300U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_212_PI_WDQLVL_EN_F0_SHIFT                                     (0x00000008U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_212_PI_DISABLE_WDQLVL_INITIALIZATION_SUPPORT                  (0x00000002U)

/* EMIF_CTLCFG_DENALI_PI_259 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_WR_MASK                                          (0x40000000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_WR_SHIFT                                         (0x0000001EU)

#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_OP_MASK                                          (0x80000000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_PI_259_MR13_FSP_OP_SHIFT                                         (0x0000001FU)

/* EMIF_CTLCFG_DENALI_CTL_132 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_132_LP_CMD_MASK                                              (0x0000007FU)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_132_LP_CMD_SHIFT                                             (0x00000018U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_132_LP_CMD_SRPD_LONG_WITH_MEM_AND_CTL_CLK_GATING_MASK        (0x00000051U)

/* EMIF_CTLCFG_DENALI_CTL_141 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CS0_MASK                                        (0x0000007FU)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CS0_SHIFT                                       (0x00000000U)

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CS1_MASK                                        (0x0000007FU)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CSx_SHIFT                                       (0x00000008U)

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_AUTO_ENTRY_EXIT_EN_MASK                               (0x0000000FU)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_AUTO_ENTRY_EN_SHIFT                                   (0x00000010U)

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_AUTO_EXIT_EN_SHIFT                                    (0x00000018U)

/* Helper constants for building and verifying LP state values */
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_VALID_MASK                                      (0x00000004U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_VALID_CS0_SHIFT                                 (0x00000004U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_VALID_CS1_SHIFT                                 (0x00000008U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_141_LP_STATE_CSx_SRPD_LONG_WITH_MEM_AND_CTL_CLK_GATING_MASK  (0x0000000FU)

/* EMIF_CTLCFG_DENALI_CTL_190 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F0_MASK                                (0x00000100U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F0_SHIFT                               (0x00000008U)

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F1_MASK                                (0x00010000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F1_SHIFT                               (0x00000010U)

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F2_MASK                                (0x01000000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_190_MR_FSP_DATA_VALID_F2_SHIFT                               (0x00000018U)

/* EMIF_CTLCFG_DENALI_CTL_191 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_191_FSP_PHY_UPDATE_MRW_MASK                                  (0x01000000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_191_FSP_PHY_UPDATE_MRW_SHIFT                                 (0x00000018U)

/* EMIF_CTLCFG_DENALI_CTL_192 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_192_FSP_OP_CURRENT_MASK                                      (0x00010000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_192_FSP_OP_CURRENT_SHIFT                                     (0x00000010U)

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_192_FSP_WR_CURRENT_MASK                                      (0x01000000U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_192_FSP_WR_CURRENT_SHIFT                                     (0x00000018U)

/* EMIF_CTLCFG_DENALI_CTL_293 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_293_INT_STATUS_LP_CMD_COMPLETE_MASK                          (0x00000400U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_293_INT_STATUS_LP_CMD_COMPLETE_SHIFT                         (0x0000000AU)

/* EMIF_CTLCFG_DENALI_CTL_295 */

#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_295_INT_ACK_LP_CMD_COMPLETE_MASK                             (0x00000400U)
#define LPM_DDR_EMIF_CTLCFG_DENALI_CTL_295_INT_ACK_LP_CMD_COMPLETE_SHIFT                            (0x0000000AU)

/* MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn */

#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn_REQ_TYPE_MASK                                 (0x00000003U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn_REQ_TYPE_SHIFT                                (0x00000000U)

#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn_REQ_MASK                                      (0x00000100U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_REQn_REQ_SHIFT                                     (0x00000008U)

/* MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn */

#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_MASK                                   (0x00000001U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_SHIFT                                  (0x00000000U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_EN                                     (0x1U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_ACKn_ACK_DIS                                    (0x0U)

/* MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQn */

#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQn_REQ_MASK                                   (0x00000080U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQn_REQ_SHIFT                                  (0x00000007U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQn_REQ_EN                                     (0x1U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_DDR4_FSP_CLKCHNG_REQn_REQ_DIS                                    (0x0U)

/* MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACKn */

#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACKn_ACK_MASK                                      (0x00000080U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACKn_ACK_SHIFT                                     (0x00000007U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACKn_ACK_EN                                        (0x1U)
#define LPM_DDR_MAIN_CTRL_MMR_CFG0_CHNG_DDR4_FSP_ACKn_ACK_DIS                                       (0x0U)

/* HSDIV field encodes (divisor - 1); the PLL12 VCO runs at 2133 MHz */

#define LPM_DDR_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_DIVBY2                                     (0x1U)
#define LPM_DDR_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_DIVBY4                                     (0x3U)
#define LPM_DDR_MAIN_PLL_MMR_CFG_PLL12_HSDIV_CTRL0_HSDIV_DIVBY8                                     (0x7U)

/* DDR4 Frequency Set Point (FSP) index passed to Lpm_initiateFspFreqChange()
 * to select which trained frequency profile the controller switches to
 * during self refresh sequence.
 */
#define LPM_DDR_FSP0                                                                                (0x00000000U)

/* DDR data-rate targets used to populate the pllFreq[] table in each
 * Lpm_DdrInstanceConfig. Lpm_setPllForDDR() maps these to the appropriate
 * PLL12 HSDIV divider value at runtime.
 */
#define LPM_DDR_FSP_19_2_MHZ                                                                        (19200000U)
#define LPM_DDR_FSP_800_MHZ                                                                         (800000000U)
#define LPM_DDR_FSP_1066_5_MHZ                                                                      (1066500000U)

/* PLL output frequency thresholds used by Lpm_setPllForDDR() to select the
 * correct HSDIV divider. Each band maps to a fixed divider ratio against the
 * 2133 MHz PLL12 VCO output.
 */
#define LPM_DDR_PLL_FREQ_27_5_MHZ                                                                   (27500000U)
#define LPM_DDR_PLL_FREQ_133_MHZ                                                                    (133000000U)
#define LPM_DDR_PLL_FREQ_266_5_MHZ                                                                  (266500000U)
#define LPM_DDR_PLL_FREQ_533_MHZ                                                                    (533000000U)

/* PLL12 VCO output frequency — used as the dividend when computing HSDIV ratios */
#define LPM_DDR_PLL_FREQ_2133_MHZ                                                                   (2133000000U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Set all DDR instances into self-refresh mode.
 */
void Lpm_ddrEnterSelfRefresh(void);

#endif /* LPM_DDR_H_ */
