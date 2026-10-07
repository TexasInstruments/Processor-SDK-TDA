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
 *  Name        : cslr_wkup_sec_mmr.h
 *  VPVERSION   : 3.0.405 - 2026.02.24.09.53.55
 *  VPREV       : 2.23.4
*/
#ifndef CSLR_WKUP_SEC_MMR_H_
#define CSLR_WKUP_SEC_MMR_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <ti/csl/cslr.h>
#include <stdint.h>

/**************************************************************************
* Module Base Offset Values
**************************************************************************/

#define CSL_WKUP_SEC_MMR_CFG0_REGS_BASE                                        (0x45A20000U)
#define CSL_WKUP_SEC_MMR_CFG2_REGS_BASE                                        (0x45920000U)


/**************************************************************************
* Hardware Region  : MMRs in region 0
**************************************************************************/


/**************************************************************************
* Register Overlay Structure
**************************************************************************/

typedef struct {
    volatile uint32_t PID;                       /* PID register */
    volatile uint8_t  Resv_114720[114716];
    volatile uint32_t CLSTR28_DEF;               /* Cluster28 Definition Register */
    volatile uint8_t  Resv_114752[28];
    volatile uint32_t CLSTR28_CFG;               /* Cluster28 Configuration Register */
    volatile uint8_t  Resv_114816[60];
    volatile uint32_t CLSTR28_PMCTRL;            /* Cluster28 Power Management Control Register */
    volatile uint8_t  Resv_114832[12];
    volatile uint32_t CLSTR28_PMSTAT;            /* Cluster28 Power Management Status Register */
    volatile uint8_t  Resv_114944[108];
    volatile uint32_t CLSTR28_CORE0_CFG;         /* Cluster28 Core0 Configuration Register */
    volatile uint8_t  Resv_114960[12];
    volatile uint32_t CLSTR28_CORE0_BOOTVECT_LO;   /* Cluster28 Core0 Boot Vector Lo Register */
    volatile uint32_t CLSTR28_CORE0_BOOTVECT_HI;   /* Cluster28 Core0 Boot Vector Hi Register */
    volatile uint8_t  Resv_114976[8];
    volatile uint32_t CLSTR28_CORE0_PMCTRL;      /* Cluster28 Core0 Power Management Control */
    volatile uint8_t  Resv_114992[12];
    volatile uint32_t CLSTR28_CORE0_PMSTAT;      /* Cluster28 Core0 Power Management Status Register */
} CSL_wkup_sec_mmr_cfg0Regs;


/**************************************************************************
* Register Macros
**************************************************************************/

#define CSL_WKUP_SEC_MMR_CFG0_PID                                              (0x00000000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF                                      (0x0001C020U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG                                      (0x0001C040U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_PMCTRL                                   (0x0001C080U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_PMSTAT                                   (0x0001C090U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG                                (0x0001C100U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_LO                        (0x0001C110U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_HI                        (0x0001C114U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMCTRL                             (0x0001C120U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT                             (0x0001C130U)

/**************************************************************************
* Field Definition Macros
**************************************************************************/


/* PID */

#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MINOR_MASK                               (0x0000003FU)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MINOR_SHIFT                              (0x00000000U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MINOR_MAX                                (0x0000003FU)

#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_CUSTOM_MASK                              (0x000000C0U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_CUSTOM_SHIFT                             (0x00000006U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_CUSTOM_MAX                               (0x00000003U)

#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MAJOR_MASK                               (0x00000700U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MAJOR_SHIFT                              (0x00000008U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MAJOR_MAX                                (0x00000007U)

#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MISC_MASK                                (0x0000F800U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MISC_SHIFT                               (0x0000000BU)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MISC_MAX                                 (0x0000001FU)

#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MSB16_MASK                               (0xFFFF0000U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MSB16_SHIFT                              (0x00000010U)
#define CSL_WKUP_SEC_MMR_CFG0_PID_PID_MSB16_MAX                                (0x0000FFFFU)


/* CLSTR28_DEF */

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_ARM_CORE_TYPE_MASK                   (0x000000FFU)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_ARM_CORE_TYPE_SHIFT                  (0x00000000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_ARM_CORE_TYPE_MAX                    (0x000000FFU)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_DSP_CORE_TYPE_MASK                   (0x0000FF00U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_DSP_CORE_TYPE_SHIFT                  (0x00000008U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_DSP_CORE_TYPE_MAX                    (0x000000FFU)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_CORE_NUM_MASK                        (0x00030000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_CORE_NUM_SHIFT                       (0x00000010U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_DEF_CORE_NUM_MAX                         (0x00000003U)


/* CLSTR28_CFG */

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_LOCKSTEP_MASK                        (0x00000001U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_LOCKSTEP_SHIFT                       (0x00000000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_LOCKSTEP_MAX                         (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_TEINIT_MASK                          (0x00000002U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_TEINIT_SHIFT                         (0x00000001U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_TEINIT_MAX                           (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_DBG_NO_CLKSTOP_MASK                  (0x00000004U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_DBG_NO_CLKSTOP_SHIFT                 (0x00000002U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_DBG_NO_CLKSTOP_MAX                   (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_LOCKSTEP_EN_MASK                     (0x00000008U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_LOCKSTEP_EN_SHIFT                    (0x00000003U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_LOCKSTEP_EN_MAX                      (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_MEM_INIT_DIS_MASK                    (0x00000010U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_MEM_INIT_DIS_SHIFT                   (0x00000004U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_MEM_INIT_DIS_MAX                     (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_SINGLE_CORE_MASK                     (0x00000020U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_SINGLE_CORE_SHIFT                    (0x00000005U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_SINGLE_CORE_MAX                      (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_SINGLE_CORE_ONLY_MASK                (0x00000040U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_SINGLE_CORE_ONLY_SHIFT               (0x00000006U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CFG_SINGLE_CORE_ONLY_MAX                 (0x00000001U)


/* CLSTR28_PMCTRL */


/* CLSTR28_PMSTAT */


/* CLSTR28_CORE0_CFG */

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_ATCM_EN_MASK                   (0x00000008U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_ATCM_EN_SHIFT                  (0x00000003U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_ATCM_EN_MAX                    (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_BTCM_EN_MASK                   (0x00000080U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_BTCM_EN_SHIFT                  (0x00000007U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_BTCM_EN_MAX                    (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_TCM_RSTBASE_MASK               (0x00000800U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_TCM_RSTBASE_SHIFT              (0x0000000BU)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_TCM_RSTBASE_MAX                (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_NMFI_EN_MASK                   (0x00008000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_NMFI_EN_SHIFT                  (0x0000000FU)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_CFG_NMFI_EN_MAX                    (0x00000001U)


/* CLSTR28_CORE0_BOOTVECT_LO */

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_LO_VECT_ADDR_MASK         (0xFFFFFF80U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_LO_VECT_ADDR_SHIFT        (0x00000007U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_LO_VECT_ADDR_MAX          (0x01FFFFFFU)


/* CLSTR28_CORE0_BOOTVECT_HI */

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_HI_VECT_ADDR_MASK         (0x0000FFFFU)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_HI_VECT_ADDR_SHIFT        (0x00000000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_BOOTVECT_HI_VECT_ADDR_MAX          (0x0000FFFFU)


/* CLSTR28_CORE0_PMCTRL */

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMCTRL_CORE_HALT_MASK              (0x00000001U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMCTRL_CORE_HALT_SHIFT             (0x00000000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMCTRL_CORE_HALT_MAX               (0x00000001U)


/* CLSTR28_CORE0_PMSTAT */

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_WFI_MASK                    (0x00000001U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_WFI_SHIFT                   (0x00000000U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_WFI_MAX                     (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_WFE_MASK                    (0x00000002U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_WFE_SHIFT                   (0x00000001U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_WFE_MAX                     (0x00000001U)

#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_CLK_GATE_MASK               (0x00000008U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_CLK_GATE_SHIFT              (0x00000003U)
#define CSL_WKUP_SEC_MMR_CFG0_CLSTR28_CORE0_PMSTAT_CLK_GATE_MAX                (0x00000001U)


/**************************************************************************
* Hardware Region  : MMRs in region 2
**************************************************************************/


/**************************************************************************
* Register Overlay Structure
**************************************************************************/

typedef struct {
    volatile uint8_t  Resv_114688[114688];
    volatile uint32_t CLSTR28_CORE0_DBG_CFG;     /* CLSTR28 Core0 Debug Configuration Register */
    volatile uint8_t  Resv_114752[60];
    volatile uint32_t CLSTR28_CORE1_DBG_CFG;     /* CLSTR28 Core1 Debug Configuration Register */
} CSL_wkup_sec_mmr_CFG2Regs;


/**************************************************************************
* Register Macros
**************************************************************************/

#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE0_DBG_CFG                            (0x0001C000U)
#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE1_DBG_CFG                            (0x0001C040U)

/**************************************************************************
* Field Definition Macros
**************************************************************************/


/* CLSTR28_CORE0_DBG_CFG */

#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE0_DBG_CFG_NIDEN_MASK                 (0x00000F00U)
#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE0_DBG_CFG_NIDEN_SHIFT                (0x00000008U)
#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE0_DBG_CFG_NIDEN_MAX                  (0x0000000FU)

#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE0_DBG_CFG_DBGEN_MASK                 (0x0000F000U)
#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE0_DBG_CFG_DBGEN_SHIFT                (0x0000000CU)
#define CSL_WKUP_SEC_MMR_CFG2_CLSTR28_CORE0_DBG_CFG_DBGEN_MAX                  (0x0000000FU)


/* CLSTR28_CORE1_DBG_CFG */


#ifdef __cplusplus
}
#endif
#endif
