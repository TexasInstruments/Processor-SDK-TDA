/********************************************************************
*
* SOC PSC PD and LPSC ID definitions. header file
*
* Copyright (C) 2015-2025 Texas Instruments Incorporated.
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
#ifndef CSL_SOC_PSC_H_
#define CSL_SOC_PSC_H_

#include <ti/csl/cslr.h>
#include <ti/csl/tistdtypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*
* Auto-generated PSC definitions File
*/

/*
* PD Indices:
*/

/*
*  MAIN PD
*/

#define CSL_MAIN_GP_CORE                                                                           (0U)
#define CSL_MAIN_MAIN_SRAM0                                                                        (1U)
#define CSL_MAIN_MAIN_SRAM1                                                                        (2U)
#define CSL_MAIN_MAIN_SRAM2                                                                        (3U)
#define CSL_MAIN_R5SS_0                                                                            (4U)
#define CSL_MAIN_R5SS_1                                                                            (5U)
#define CSL_MAIN_C7X0                                                                              (6U)
#define CSL_MAIN_HWA                                                                               (7U)
#define CSL_MAIN_PD_SRAM3                                                                          (8U)
#define CSL_MAIN_PD_SRAM4                                                                          (9U)
#define CSL_MAIN_PD_RSVD0                                                                          (10U)
#define CSL_MAIN_PD_RSVD1                                                                          (11U)
/*
*  WKUP PD
*/

#define CSL_WKUP_GP_CORE_CTL_MCU                                                                   (0U)
#define CSL_WKUP_PD_MCUSS                                                                          (1U)
/*
* LPSC Modules:
*/

/*
*  MAIN LPSC
*/

#define CSL_MAIN_LPSC_MAIN_ALWAYSON                                                                (0U)
#define CSL_MAIN_LPSC_MAIN_DM                                                                      (1U)
#define CSL_MAIN_LPSC_MAIN_DM_PBIST0                                                               (2U)
#define CSL_MAIN_LPSC_MAIN_MAIN2DM_ISO                                                             (3U)
#define CSL_MAIN_LPSC_MAIN_DM2MAIN_ISO                                                             (4U)
#define CSL_MAIN_LPSC_MAIN_DM2MAIN_INFRA_ISO                                                       (5U)
#define CSL_MAIN_LPSC_MAIN_DM2CENTRAL_ISO                                                          (6U)
#define CSL_MAIN_LPSC_MAIN_CENTRAL2DM_ISO                                                          (7U)
#define CSL_MAIN_LPSC_MAIN_DM_PBIST1                                                               (8U)
#define CSL_MAIN_LPSC_MAIN_TEST                                                                    (9U)
#define CSL_MAIN_LPSC_MAIN_DEBUGSS                                                                 (10U)
#define CSL_MAIN_LPSC_MAIN_PBIST0                                                                  (11U)
#define CSL_MAIN_LPSC_MAIN_USB0_RSVD                                                               (12U)
#define CSL_MAIN_LPSC_MAIN_USB0_ISO_RSVD                                                           (13U)
#define CSL_MAIN_LPSC_SMS_COM                                                                      (14U)
#define CSL_MAIN_LPSC_MAIN_TIFS                                                                    (15U)
#define CSL_MAIN_LPSC_MAIN_HSM                                                                     (16U)
#define CSL_MAIN_LPSC_MAIN_HSM_ISO                                                                 (17U)
#define CSL_MAIN_LPSC_MAIN_IP0                                                                     (18U)
#define CSL_MAIN_LPSC_MAIN_IP1                                                                     (19U)
#define CSL_MAIN_LPSC_MAIN_EMMC8B_RSVD                                                             (20U)
#define CSL_MAIN_LPSC_MAIN_CPSW                                                                    (21U)
#define CSL_MAIN_LPSC_MAIN_RSVD0                                                                   (22U)
#define CSL_MAIN_LPSC_MAIN_MCAN0                                                                   (23U)
#define CSL_MAIN_LPSC_MAIN_MCAN1                                                                   (24U)
#define CSL_MAIN_LPSC_MAIN_RSVD1                                                                   (25U)
#define CSL_MAIN_LPSC_MAIN_RSVD2                                                                   (26U)
#define CSL_MAIN_LPSC_MAIN_RSVD3                                                                   (27U)
#define CSL_MAIN_LPSC_MAIN_OSPI0                                                                   (28U)
#define CSL_MAIN_LPSC_MAIN_RSVD4                                                                   (29U)
#define CSL_MAIN_LPSC_MAIN_RSVD5                                                                   (30U)
#define CSL_MAIN_LPSC_MAIN_SA3UL                                                                   (31U)
#define CSL_MAIN_LPSC_MAIN_CSI_RX0                                                                 (32U)
#define CSL_MAIN_LPSC_MAIN_DHPHY_RX0                                                               (33U)
#define CSL_MAIN_LPSC_MAIN_RSVD6                                                                   (34U)
#define CSL_MAIN_LPSC_MAIN_RSVD7                                                                   (35U)
#define CSL_MAIN_LPSC_MAIN_CSI_TX0                                                                 (36U)
#define CSL_MAIN_LPSC_MAIN_DPHY_TX0                                                                (37U)
#define CSL_MAIN_LPSC_MAIN_RSVD8                                                                   (38U)
#define CSL_MAIN_LPSC_MAIN_RSVD9                                                                   (39U)
#define CSL_MAIN_LPSC_MAIN_RSVD10                                                                  (40U)
#define CSL_MAIN_LPSC_MAIN_ADC                                                                     (41U)
#define CSL_MAIN_LPSC_MAIN_RSVD11                                                                  (42U)
#define CSL_MAIN_LPSC_MAIN_SRAM_0_1                                                                (43U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST0                                                             (44U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST1                                                             (45U)
#define CSL_MAIN_LPSC_MAIN_SRAM_2_3                                                                (46U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST2                                                             (47U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST3                                                             (48U)
#define CSL_MAIN_LPSC_MAIN_SRAM_4_5                                                                (49U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST4                                                             (50U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST5                                                             (51U)
#define CSL_MAIN_LPSC_MAIN_R5SS0_CORE0                                                             (52U)
#define CSL_MAIN_LPSC_MAIN_R5SS0_CORE1                                                             (53U)
#define CSL_MAIN_LPSC_MAIN_R5SS0_PBIST                                                             (54U)
#define CSL_MAIN_LPSC_MAIN_R5SS1_CORE0                                                             (55U)
#define CSL_MAIN_LPSC_MAIN_R5SS1_CORE1                                                             (56U)
#define CSL_MAIN_LPSC_MAIN_R5SS1_PBIST                                                             (57U)
#define CSL_MAIN_LPSC_MAIN_C7X0_COMMON                                                             (58U)
#define CSL_MAIN_LPSC_MAIN_C7X0_CORE                                                               (59U)
#define CSL_MAIN_LPSC_MAIN_C7X0_PBIST                                                              (60U)
#define CSL_MAIN_LPSC_MAIN_HWA                                                                     (61U)
#define CSL_MAIN_LPSC_MAIN_HWA_PBIST                                                               (62U)
#define CSL_MAIN_LPSC_MAIN_HWA_EDMA                                                                (63U)
#define CSL_MAIN_LPSC_MAIN_SRAM_6_7                                                                (64U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST6                                                             (65U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST7                                                             (66U)
#define CSL_MAIN_LPSC_MAIN_SRAM3_RSVD                                                              (67U)
#define CSL_MAIN_LPSC_MAIN_SRAM_8_9                                                                (68U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST8                                                             (69U)
#define CSL_MAIN_LPSC_MAIN_SRAM_PBIST9                                                             (70U)
#define CSL_MAIN_LPSC_MAIN_SRAM4_RSVD                                                              (71U)
#define CSL_MAIN_LPSC_MAIN_PDRSVD0                                                                 (72U)
#define CSL_MAIN_LPSC_MAIN_PDRSVD1                                                                 (73U)
/*
*  WKUP LPSC
*/

#define CSL_WKUP_LPSC_MCU_ALWAYSON                                                                 (0U)
#define CSL_WKUP_LPSC_GP_CORE_RSVD0                                                                (1U)
#define CSL_WKUP_LPSC_GP_CORE_RSVD1                                                                (2U)
#define CSL_WKUP_LPSC_DM2SAFE_ISO                                                                  (3U)
#define CSL_WKUP_LPSC_GP_CORE_RSVD2                                                                (4U)
#define CSL_WKUP_LPSC_MCU_TEST                                                                     (5U)
#define CSL_WKUP_LPSC_MCU_RSVD0                                                                    (6U)
#define CSL_WKUP_LPSC_MCU_RSVD1                                                                    (7U)
#define CSL_WKUP_LPSC_MCU_RSVD2                                                                    (8U)
#define CSL_WKUP_LPSC_MCU_RSVD3                                                                    (9U)
#define CSL_WKUP_LPSC_MCU_RSVD4                                                                    (10U)

#ifdef __cplusplus
}
#endif
#endif /* CSL_SOC_PSC_H_ */

