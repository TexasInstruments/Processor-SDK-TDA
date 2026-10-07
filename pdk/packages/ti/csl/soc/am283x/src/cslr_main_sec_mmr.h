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
 *  Name        : cslr_main_sec_mmr.h
 *  VPVERSION   : 3.0.405 - 2026.02.24.09.53.55
 *  VPREV       : 2.24.0
*/
#ifndef CSLR_MAIN_SEC_MMR_H_
#define CSLR_MAIN_SEC_MMR_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <ti/csl/cslr.h>
#include <stdint.h>

/**************************************************************************
* Module Base Offset Values
**************************************************************************/

#define CSL_MAIN_SEC_MMR_CFG0_REGS_BASE                                        (0x45A00000U)
#define CSL_MAIN_SEC_MMR_CFG2_REGS_BASE                                        (0x45900000U)


/**************************************************************************
* Hardware Region  : MMRs in region 0
**************************************************************************/


/**************************************************************************
* Register Overlay Structure
**************************************************************************/

typedef struct {
    volatile uint32_t PID;                       /* PID register */
    volatile uint8_t  Resv_32[28];
    volatile uint32_t CLSTR0_DEF;                /* Cluster0 Definition Register */
    volatile uint8_t  Resv_64[28];
    volatile uint32_t CLSTR0_CFG;                /* Cluster0 Configuration Register */
    volatile uint8_t  Resv_128[60];
    volatile uint32_t CLSTR0_PMCTRL;             /* Cluster0 Power Management Control Register */
    volatile uint8_t  Resv_144[12];
    volatile uint32_t CLSTR0_PMSTAT;             /* Cluster0 Power Management Status Register */
    volatile uint8_t  Resv_256[108];
    volatile uint32_t CLSTR0_CORE0_CFG;          /* Cluster0 Core0 Configuration Register */
    volatile uint8_t  Resv_272[12];
    volatile uint32_t CLSTR0_CORE0_BOOTVECT_LO;   /* Cluster0 Core0 Boot Vector Lo Register */
    volatile uint32_t CLSTR0_CORE0_BOOTVECT_HI;   /* Cluster0 Core0 Boot Vector Hi Register */
    volatile uint8_t  Resv_288[8];
    volatile uint32_t CLSTR0_CORE0_PMCTRL;       /* Cluster0 Core0 Power Management Control */
    volatile uint8_t  Resv_304[12];
    volatile uint32_t CLSTR0_CORE0_PMSTAT;       /* Cluster0 Core0 Power Management Status Register */
    volatile uint8_t  Resv_384[76];
    volatile uint32_t CLSTR0_CORE1_CFG;          /* Cluster0 Core1 Configuration Register 0 */
    volatile uint8_t  Resv_400[12];
    volatile uint32_t CLSTR0_CORE1_BOOTVECT_LO;   /* Cluster0 Core1 Boot Vector Lo Register */
    volatile uint32_t CLSTR0_CORE1_BOOTVECT_HI;   /* Cluster0 Core1 Boot Vector Hi Register */
    volatile uint8_t  Resv_416[8];
    volatile uint32_t CLSTR0_CORE1_PMCTRL;       /* Cluster0 Core1 Power Management Control */
    volatile uint8_t  Resv_432[12];
    volatile uint32_t CLSTR0_CORE1_PMSTAT;       /* Cluster0 Core1 Power Management Status Register */
    volatile uint8_t  Resv_4128[3692];
    volatile uint32_t CLSTR1_DEF;                /* Cluster1 Definition Register */
    volatile uint8_t  Resv_4160[28];
    volatile uint32_t CLSTR1_CFG;                /* Cluster1 Configuration Register */
    volatile uint8_t  Resv_4224[60];
    volatile uint32_t CLSTR1_PMCTRL;             /* Cluster1 Power Management Control Register */
    volatile uint8_t  Resv_4240[12];
    volatile uint32_t CLSTR1_PMSTAT;             /* Cluster1 Power Management Status Register */
    volatile uint8_t  Resv_4352[108];
    volatile uint32_t CLSTR1_CORE0_CFG;          /* Cluster1 Core0 Configuration Register */
    volatile uint8_t  Resv_4368[12];
    volatile uint32_t CLSTR1_CORE0_BOOTVECT_LO;   /* Cluster1 Core0 Boot Vector Lo Register */
    volatile uint32_t CLSTR1_CORE0_BOOTVECT_HI;   /* Cluster1 Core0 Boot Vector Hi Register */
    volatile uint8_t  Resv_4384[8];
    volatile uint32_t CLSTR1_CORE0_PMCTRL;       /* Cluster1 Core0 Power Management Control */
    volatile uint8_t  Resv_4400[12];
    volatile uint32_t CLSTR1_CORE0_PMSTAT;       /* Cluster1 Core0 Power Management Status Register */
    volatile uint8_t  Resv_4480[76];
    volatile uint32_t CLSTR1_CORE1_CFG;          /* Cluster1 Core1 Configuration Register 0 */
    volatile uint8_t  Resv_4496[12];
    volatile uint32_t CLSTR1_CORE1_BOOTVECT_LO;   /* Cluster1 Core1 Boot Vector Lo Register */
    volatile uint32_t CLSTR1_CORE1_BOOTVECT_HI;   /* Cluster1 Core1 Boot Vector Hi Register */
    volatile uint8_t  Resv_4512[8];
    volatile uint32_t CLSTR1_CORE1_PMCTRL;       /* Cluster1 Core1 Power Management Control */
    volatile uint8_t  Resv_4528[12];
    volatile uint32_t CLSTR1_CORE1_PMSTAT;       /* Cluster1 Core1 Power Management Status Register */
    volatile uint8_t  Resv_53280[48748];
    volatile uint32_t CLSTR13_DEF;               /* Cluster13 Definition Register */
    volatile uint8_t  Resv_53312[28];
    volatile uint32_t CLSTR13_CFG;               /* Cluster13 Configuration Register */
    volatile uint8_t  Resv_53376[60];
    volatile uint32_t CLSTR13_PMCTRL;            /* Cluster13 Power Management Control Register */
    volatile uint8_t  Resv_53392[12];
    volatile uint32_t CLSTR13_PMSTAT;            /* Cluster13 Power Management Status Register */
    volatile uint8_t  Resv_53504[108];
    volatile uint32_t CLSTR13_CORE0_CFG;         /* Cluster13 Core0 Configuration Register */
    volatile uint8_t  Resv_53520[12];
    volatile uint32_t CLSTR13_CORE0_BOOTVECT_LO;   /* Cluster13 Core0 Boot Vector Lo Register */
    volatile uint32_t CLSTR13_CORE0_BOOTVECT_HI;   /* Cluster13 Core0 Boot Vector Hi Register */
    volatile uint8_t  Resv_53536[8];
    volatile uint32_t CLSTR13_CORE0_PMCTRL;      /* Cluster13 Core0 Power Management Control */
    volatile uint8_t  Resv_53552[12];
    volatile uint32_t CLSTR13_CORE0_PMSTAT;      /* Cluster13 Core0 Power Management Status Register */
} CSL_main_sec_mmr_cfg0Regs;


/**************************************************************************
* Register Macros
**************************************************************************/

#define CSL_MAIN_SEC_MMR_CFG0_PID                                              (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF                                       (0x00000020U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG                                       (0x00000040U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_PMCTRL                                    (0x00000080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_PMSTAT                                    (0x00000090U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG                                 (0x00000100U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO                         (0x00000110U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI                         (0x00000114U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL                              (0x00000120U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT                              (0x00000130U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG                                 (0x00000180U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_LO                         (0x00000190U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_HI                         (0x00000194U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMCTRL                              (0x000001A0U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT                              (0x000001B0U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF                                       (0x00001020U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG                                       (0x00001040U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_PMCTRL                                    (0x00001080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_PMSTAT                                    (0x00001090U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG                                 (0x00001100U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_LO                         (0x00001110U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_HI                         (0x00001114U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMCTRL                              (0x00001120U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT                              (0x00001130U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG                                 (0x00001180U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_LO                         (0x00001190U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_HI                         (0x00001194U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMCTRL                              (0x000011A0U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT                              (0x000011B0U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF                                      (0x0000D020U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG                                      (0x0000D040U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMCTRL                                   (0x0000D080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT                                   (0x0000D090U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_CFG                                (0x0000D100U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_LO                        (0x0000D110U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_HI                        (0x0000D114U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_PMCTRL                             (0x0000D120U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_PMSTAT                             (0x0000D130U)

/**************************************************************************
* Field Definition Macros
**************************************************************************/


/* PID */

#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MINOR_MASK                               (0x0000003FU)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MINOR_SHIFT                              (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MINOR_MAX                                (0x0000003FU)

#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_CUSTOM_MASK                              (0x000000C0U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_CUSTOM_SHIFT                             (0x00000006U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_CUSTOM_MAX                               (0x00000003U)

#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MAJOR_MASK                               (0x00000700U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MAJOR_SHIFT                              (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MAJOR_MAX                                (0x00000007U)

#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MISC_MASK                                (0x0000F800U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MISC_SHIFT                               (0x0000000BU)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MISC_MAX                                 (0x0000001FU)

#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MSB16_MASK                               (0xFFFF0000U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MSB16_SHIFT                              (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_PID_PID_MSB16_MAX                                (0x0000FFFFU)


/* CLSTR0_DEF */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_ARM_CORE_TYPE_MASK                    (0x000000FFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_ARM_CORE_TYPE_SHIFT                   (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_ARM_CORE_TYPE_MAX                     (0x000000FFU)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_DSP_CORE_TYPE_MASK                    (0x0000FF00U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_DSP_CORE_TYPE_SHIFT                   (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_DSP_CORE_TYPE_MAX                     (0x000000FFU)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_CORE_NUM_MASK                         (0x00070000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_CORE_NUM_SHIFT                        (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_DEF_CORE_NUM_MAX                          (0x00000007U)


/* CLSTR0_CFG */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_MASK                         (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_SHIFT                        (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_MAX                          (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_TEINIT_MASK                           (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_TEINIT_SHIFT                          (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_TEINIT_MAX                            (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_DBG_NO_CLKSTOP_MASK                   (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_DBG_NO_CLKSTOP_SHIFT                  (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_DBG_NO_CLKSTOP_MAX                    (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_EN_MASK                      (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_EN_SHIFT                     (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_LOCKSTEP_EN_MAX                       (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_MEM_INIT_DIS_MASK                     (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_MEM_INIT_DIS_SHIFT                    (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_MEM_INIT_DIS_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_MASK                      (0x00000020U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_SHIFT                     (0x00000005U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_MAX                       (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_ONLY_MASK                 (0x00000040U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_ONLY_SHIFT                (0x00000006U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CFG_SINGLE_CORE_ONLY_MAX                  (0x00000001U)


/* CLSTR0_PMCTRL */


/* CLSTR0_PMSTAT */


/* CLSTR0_CORE0_CFG */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_ATCM_EN_MASK                    (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_ATCM_EN_SHIFT                   (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_ATCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_BTCM_EN_MASK                    (0x00000080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_BTCM_EN_SHIFT                   (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_BTCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_TCM_RSTBASE_MASK                (0x00000800U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_TCM_RSTBASE_SHIFT               (0x0000000BU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_TCM_RSTBASE_MAX                 (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_NMFI_EN_MASK                    (0x00008000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_NMFI_EN_SHIFT                   (0x0000000FU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_CFG_NMFI_EN_MAX                     (0x00000001U)


/* CLSTR0_CORE0_BOOTVECT_LO */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO_VECT_ADDR_MASK          (0xFFFFFF80U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO_VECT_ADDR_SHIFT         (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_LO_VECT_ADDR_MAX           (0x01FFFFFFU)


/* CLSTR0_CORE0_BOOTVECT_HI */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI_VECT_ADDR_MASK          (0x0000FFFFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI_VECT_ADDR_SHIFT         (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_BOOTVECT_HI_VECT_ADDR_MAX           (0x0000FFFFU)


/* CLSTR0_CORE0_PMCTRL */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL_CORE_HALT_MASK               (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL_CORE_HALT_SHIFT              (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMCTRL_CORE_HALT_MAX                (0x00000001U)


/* CLSTR0_CORE0_PMSTAT */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFI_MASK                     (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFI_SHIFT                    (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFI_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFE_MASK                     (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFE_SHIFT                    (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_WFE_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_CLK_GATE_MASK                (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_CLK_GATE_SHIFT               (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE0_PMSTAT_CLK_GATE_MAX                 (0x00000001U)


/* CLSTR0_CORE1_CFG */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_ATCM_EN_MASK                    (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_ATCM_EN_SHIFT                   (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_ATCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_BTCM_EN_MASK                    (0x00000080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_BTCM_EN_SHIFT                   (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_BTCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_TCM_RSTBASE_MASK                (0x00000800U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_TCM_RSTBASE_SHIFT               (0x0000000BU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_TCM_RSTBASE_MAX                 (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_NMFI_EN_MASK                    (0x00008000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_NMFI_EN_SHIFT                   (0x0000000FU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_CFG_NMFI_EN_MAX                     (0x00000001U)


/* CLSTR0_CORE1_BOOTVECT_LO */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_LO_VECT_ADDR_MASK          (0xFFFFFF80U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_LO_VECT_ADDR_SHIFT         (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_LO_VECT_ADDR_MAX           (0x01FFFFFFU)


/* CLSTR0_CORE1_BOOTVECT_HI */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_HI_VECT_ADDR_MASK          (0x0000FFFFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_HI_VECT_ADDR_SHIFT         (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_BOOTVECT_HI_VECT_ADDR_MAX           (0x0000FFFFU)


/* CLSTR0_CORE1_PMCTRL */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMCTRL_CORE_HALT_MASK               (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMCTRL_CORE_HALT_SHIFT              (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMCTRL_CORE_HALT_MAX                (0x00000001U)


/* CLSTR0_CORE1_PMSTAT */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_WFI_MASK                     (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_WFI_SHIFT                    (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_WFI_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_WFE_MASK                     (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_WFE_SHIFT                    (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_WFE_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_CLK_GATE_MASK                (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_CLK_GATE_SHIFT               (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR0_CORE1_PMSTAT_CLK_GATE_MAX                 (0x00000001U)


/* CLSTR1_DEF */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_ARM_CORE_TYPE_MASK                    (0x000000FFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_ARM_CORE_TYPE_SHIFT                   (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_ARM_CORE_TYPE_MAX                     (0x000000FFU)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_DSP_CORE_TYPE_MASK                    (0x0000FF00U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_DSP_CORE_TYPE_SHIFT                   (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_DSP_CORE_TYPE_MAX                     (0x000000FFU)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_CORE_NUM_MASK                         (0x00070000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_CORE_NUM_SHIFT                        (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_DEF_CORE_NUM_MAX                          (0x00000007U)


/* CLSTR1_CFG */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_LOCKSTEP_MASK                         (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_LOCKSTEP_SHIFT                        (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_LOCKSTEP_MAX                          (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_TEINIT_MASK                           (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_TEINIT_SHIFT                          (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_TEINIT_MAX                            (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_DBG_NO_CLKSTOP_MASK                   (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_DBG_NO_CLKSTOP_SHIFT                  (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_DBG_NO_CLKSTOP_MAX                    (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_LOCKSTEP_EN_MASK                      (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_LOCKSTEP_EN_SHIFT                     (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_LOCKSTEP_EN_MAX                       (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_MEM_INIT_DIS_MASK                     (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_MEM_INIT_DIS_SHIFT                    (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_MEM_INIT_DIS_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_SINGLE_CORE_MASK                      (0x00000020U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_SINGLE_CORE_SHIFT                     (0x00000005U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_SINGLE_CORE_MAX                       (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_SINGLE_CORE_ONLY_MASK                 (0x00000040U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_SINGLE_CORE_ONLY_SHIFT                (0x00000006U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CFG_SINGLE_CORE_ONLY_MAX                  (0x00000001U)


/* CLSTR1_PMCTRL */


/* CLSTR1_PMSTAT */


/* CLSTR1_CORE0_CFG */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_ATCM_EN_MASK                    (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_ATCM_EN_SHIFT                   (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_ATCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_BTCM_EN_MASK                    (0x00000080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_BTCM_EN_SHIFT                   (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_BTCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_TCM_RSTBASE_MASK                (0x00000800U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_TCM_RSTBASE_SHIFT               (0x0000000BU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_TCM_RSTBASE_MAX                 (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_NMFI_EN_MASK                    (0x00008000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_NMFI_EN_SHIFT                   (0x0000000FU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_CFG_NMFI_EN_MAX                     (0x00000001U)


/* CLSTR1_CORE0_BOOTVECT_LO */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_LO_VECT_ADDR_MASK          (0xFFFFFF80U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_LO_VECT_ADDR_SHIFT         (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_LO_VECT_ADDR_MAX           (0x01FFFFFFU)


/* CLSTR1_CORE0_BOOTVECT_HI */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_HI_VECT_ADDR_MASK          (0x0000FFFFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_HI_VECT_ADDR_SHIFT         (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_BOOTVECT_HI_VECT_ADDR_MAX           (0x0000FFFFU)


/* CLSTR1_CORE0_PMCTRL */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMCTRL_CORE_HALT_MASK               (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMCTRL_CORE_HALT_SHIFT              (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMCTRL_CORE_HALT_MAX                (0x00000001U)


/* CLSTR1_CORE0_PMSTAT */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_WFI_MASK                     (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_WFI_SHIFT                    (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_WFI_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_WFE_MASK                     (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_WFE_SHIFT                    (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_WFE_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_CLK_GATE_MASK                (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_CLK_GATE_SHIFT               (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE0_PMSTAT_CLK_GATE_MAX                 (0x00000001U)


/* CLSTR1_CORE1_CFG */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_ATCM_EN_MASK                    (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_ATCM_EN_SHIFT                   (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_ATCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_BTCM_EN_MASK                    (0x00000080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_BTCM_EN_SHIFT                   (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_BTCM_EN_MAX                     (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_TCM_RSTBASE_MASK                (0x00000800U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_TCM_RSTBASE_SHIFT               (0x0000000BU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_TCM_RSTBASE_MAX                 (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_NMFI_EN_MASK                    (0x00008000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_NMFI_EN_SHIFT                   (0x0000000FU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_CFG_NMFI_EN_MAX                     (0x00000001U)


/* CLSTR1_CORE1_BOOTVECT_LO */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_LO_VECT_ADDR_MASK          (0xFFFFFF80U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_LO_VECT_ADDR_SHIFT         (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_LO_VECT_ADDR_MAX           (0x01FFFFFFU)


/* CLSTR1_CORE1_BOOTVECT_HI */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_HI_VECT_ADDR_MASK          (0x0000FFFFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_HI_VECT_ADDR_SHIFT         (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_BOOTVECT_HI_VECT_ADDR_MAX           (0x0000FFFFU)


/* CLSTR1_CORE1_PMCTRL */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMCTRL_CORE_HALT_MASK               (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMCTRL_CORE_HALT_SHIFT              (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMCTRL_CORE_HALT_MAX                (0x00000001U)


/* CLSTR1_CORE1_PMSTAT */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_WFI_MASK                     (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_WFI_SHIFT                    (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_WFI_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_WFE_MASK                     (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_WFE_SHIFT                    (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_WFE_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_CLK_GATE_MASK                (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_CLK_GATE_SHIFT               (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR1_CORE1_PMSTAT_CLK_GATE_MAX                 (0x00000001U)


/* CLSTR13_DEF */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_ARM_CORE_TYPE_MASK                   (0x000000FFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_ARM_CORE_TYPE_SHIFT                  (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_ARM_CORE_TYPE_MAX                    (0x000000FFU)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_DSP_CORE_TYPE_MASK                   (0x0000FF00U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_DSP_CORE_TYPE_SHIFT                  (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_DSP_CORE_TYPE_MAX                    (0x000000FFU)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_CORE_NUM_MASK                        (0x00070000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_CORE_NUM_SHIFT                       (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_DEF_CORE_NUM_MAX                         (0x00000007U)


/* CLSTR13_CFG */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_CPU_MASK                             (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_CPU_SHIFT                            (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_CPU_MAX                              (0x00000003U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_MMA_MASK                             (0x0000000CU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_MMA_SHIFT                            (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_MMA_MAX                              (0x00000003U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_RISCV_MASK                           (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_RISCV_SHIFT                          (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_RISCV_MAX                            (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_VWIDTH_MASK                          (0x00000060U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_VWIDTH_SHIFT                         (0x00000005U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_VWIDTH_MAX                           (0x00000003U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L1PSIZE_MASK                         (0x00000180U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L1PSIZE_SHIFT                        (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L1PSIZE_MAX                          (0x00000003U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L1DSIZE_MASK                         (0x00000600U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L1DSIZE_SHIFT                        (0x00000009U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L1DSIZE_MAX                          (0x00000003U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L2SIZE_MASK                          (0x0000F800U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L2SIZE_SHIFT                         (0x0000000BU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_L2SIZE_MAX                           (0x0000001FU)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_EL2SIZE_MASK                         (0x00070000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_EL2SIZE_SHIFT                        (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_EL2SIZE_MAX                          (0x00000007U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_MMA_PRESENT_MASK                     (0x00080000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_MMA_PRESENT_SHIFT                    (0x00000013U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_MMA_PRESENT_MAX                      (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_CPU_SYS_RISCV_MODE_MASK              (0x40000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_CPU_SYS_RISCV_MODE_SHIFT             (0x0000001EU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_CPU_SYS_RISCV_MODE_MAX               (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_ENDIAN_MASK                          (0x80000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_ENDIAN_SHIFT                         (0x0000001FU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CFG_ENDIAN_MAX                           (0x00000001U)


/* CLSTR13_PMCTRL */


/* CLSTR13_PMSTAT */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT0_MASK               (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT0_SHIFT              (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT0_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT1_MASK               (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT1_SHIFT              (0x00000001U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT1_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT2_MASK               (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT2_SHIFT              (0x00000002U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT2_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT3_MASK               (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT3_SHIFT              (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT3_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT4_MASK               (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT4_SHIFT              (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT4_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT5_MASK               (0x00000020U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT5_SHIFT              (0x00000005U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT5_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT6_MASK               (0x00000040U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT6_SHIFT              (0x00000006U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT6_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT7_MASK               (0x00000080U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT7_SHIFT              (0x00000007U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT7_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT8_MASK               (0x00000100U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT8_SHIFT              (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT8_MAX                (0x00000001U)

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT16_MASK              (0x00010000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT16_SHIFT             (0x00000010U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_PMSTAT_DSP_IDLE_STAT16_MAX               (0x00000001U)


/* CLSTR13_CORE0_CFG */


/* CLSTR13_CORE0_BOOTVECT_LO */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_LO_VECT_ADDR_MASK         (0xFFFFFFFFU)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_LO_VECT_ADDR_SHIFT        (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_LO_VECT_ADDR_MAX          (0xFFFFFFFFU)


/* CLSTR13_CORE0_BOOTVECT_HI */

#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_HI_VECT_ADDR_MASK         (0x00000003U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_HI_VECT_ADDR_SHIFT        (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG0_CLSTR13_CORE0_BOOTVECT_HI_VECT_ADDR_MAX          (0x00000003U)


/* CLSTR13_CORE0_PMCTRL */


/* CLSTR13_CORE0_PMSTAT */


/**************************************************************************
* Hardware Region  : MMRs in region 2
**************************************************************************/


/**************************************************************************
* Register Overlay Structure
**************************************************************************/

typedef struct {
    volatile uint32_t CLSTR0_CORE0_DBG_CFG;      /* Cluster0 Core0 Debug Configuration Register */
    volatile uint8_t  Resv_64[60];
    volatile uint32_t CLSTR0_CORE1_DBG_CFG;      /* Cluster0 Core1 Debug Configuration Register */
    volatile uint8_t  Resv_4096[4028];
    volatile uint32_t CLSTR1_CORE0_DBG_CFG;      /* Cluster1 Core0 Debug Configuration Register */
    volatile uint8_t  Resv_4160[60];
    volatile uint32_t CLSTR1_CORE1_DBG_CFG;      /* Cluster1 Core1 Debug Configuration Register */
    volatile uint8_t  Resv_53248[49084];
    volatile uint32_t CLSTR13_CORE0_DBG_CFG;     /* Cluster13 Core0 Debug Configuration Register */
} CSL_main_sec_mmr_CFG2Regs;


/**************************************************************************
* Register Macros
**************************************************************************/

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG                             (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG                             (0x00000040U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE0_DBG_CFG                             (0x00001000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE1_DBG_CFG                             (0x00001040U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG                            (0x0000D000U)

/**************************************************************************
* Field Definition Macros
**************************************************************************/


/* CLSTR0_CORE0_DBG_CFG */

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_NIDEN_MASK                  (0x00000F00U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_NIDEN_SHIFT                 (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_NIDEN_MAX                   (0x0000000FU)

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_DBGEN_MASK                  (0x0000F000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_DBGEN_SHIFT                 (0x0000000CU)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE0_DBG_CFG_DBGEN_MAX                   (0x0000000FU)


/* CLSTR0_CORE1_DBG_CFG */

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG_NIDEN_MASK                  (0x00000F00U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG_NIDEN_SHIFT                 (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG_NIDEN_MAX                   (0x0000000FU)

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG_DBGEN_MASK                  (0x0000F000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG_DBGEN_SHIFT                 (0x0000000CU)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR0_CORE1_DBG_CFG_DBGEN_MAX                   (0x0000000FU)


/* CLSTR1_CORE0_DBG_CFG */

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE0_DBG_CFG_NIDEN_MASK                  (0x00000F00U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE0_DBG_CFG_NIDEN_SHIFT                 (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE0_DBG_CFG_NIDEN_MAX                   (0x0000000FU)

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE0_DBG_CFG_DBGEN_MASK                  (0x0000F000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE0_DBG_CFG_DBGEN_SHIFT                 (0x0000000CU)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE0_DBG_CFG_DBGEN_MAX                   (0x0000000FU)


/* CLSTR1_CORE1_DBG_CFG */

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE1_DBG_CFG_NIDEN_MASK                  (0x00000F00U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE1_DBG_CFG_NIDEN_SHIFT                 (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE1_DBG_CFG_NIDEN_MAX                   (0x0000000FU)

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE1_DBG_CFG_DBGEN_MASK                  (0x0000F000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE1_DBG_CFG_DBGEN_SHIFT                 (0x0000000CU)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR1_CORE1_DBG_CFG_DBGEN_MAX                   (0x0000000FU)


/* CLSTR13_CORE0_DBG_CFG */

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_DBGEN_MASK                 (0x0000000FU)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_DBGEN_SHIFT                (0x00000000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_DBGEN_MAX                  (0x0000000FU)

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_NIDEN_MASK                 (0x000000F0U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_NIDEN_SHIFT                (0x00000004U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_NIDEN_MAX                  (0x0000000FU)

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_SPIDEN_MASK                (0x00000F00U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_SPIDEN_SHIFT               (0x00000008U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_SPIDEN_MAX                 (0x0000000FU)

#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_SPNIDEN_MASK               (0x0000F000U)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_SPNIDEN_SHIFT              (0x0000000CU)
#define CSL_MAIN_SEC_MMR_CFG2_CLSTR13_CORE0_DBG_CFG_SPNIDEN_MAX                (0x0000000FU)


#ifdef __cplusplus
}
#endif
#endif
