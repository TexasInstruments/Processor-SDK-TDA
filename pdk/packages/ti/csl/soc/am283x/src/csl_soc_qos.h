/********************************************************************
*
* SOC QOS PROPERTIES. header file
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
#ifndef CSLR_SOC_QOS_H_
#define CSLR_SOC_QOS_H_

#include <ti/csl/cslr.h>
#include <ti/csl/tistdtypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*
* Auto-generated CSL definitions for SoC QOS Instances:
*/


/* Properties of QOS in: WKUP_R5FSS0 */
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_MMR_BASE                                                     (0x45d14400ul)
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_CHANNEL_COUNT                                                (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_QOS_CAPABLE                                                  (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_VIRTID_CAPABLE                                               (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_ORDERID_CAPABLE                                              (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_EPRIORITY_CAPABLE                                            (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_ATYPE_CAPABLE                                                (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_RMST_ASEL_CAPABLE                                                 (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_MMR_BASE                                                     (0x45d14800ul)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_CHANNEL_COUNT                                                (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_QOS_CAPABLE                                                  (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_VIRTID_CAPABLE                                               (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_ORDERID_CAPABLE                                              (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_EPRIORITY_CAPABLE                                            (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_ATYPE_CAPABLE                                                (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_WMST_ASEL_CAPABLE                                                 (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_MMR_BASE                                                     (0x45d15000ul)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_CHANNEL_COUNT                                                (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_QOS_CAPABLE                                                  (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_VIRTID_CAPABLE                                               (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_ORDERID_CAPABLE                                              (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_EPRIORITY_CAPABLE                                            (1U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_ATYPE_CAPABLE                                                (0U)
#define CSL_QOS_WKUP_R5FSS0_CPU0_PMST_ASEL_CAPABLE                                                 (1U)

/* Properties of QOS in: R5FSS0 */
#define CSL_QOS_R5FSS0_CPU0_PMST_MMR_BASE                                                          (0x45d18000ul)
#define CSL_QOS_R5FSS0_CPU0_PMST_CHANNEL_COUNT                                                     (1U)
#define CSL_QOS_R5FSS0_CPU0_PMST_QOS_CAPABLE                                                       (0U)
#define CSL_QOS_R5FSS0_CPU0_PMST_VIRTID_CAPABLE                                                    (0U)
#define CSL_QOS_R5FSS0_CPU0_PMST_ORDERID_CAPABLE                                                   (1U)
#define CSL_QOS_R5FSS0_CPU0_PMST_EPRIORITY_CAPABLE                                                 (1U)
#define CSL_QOS_R5FSS0_CPU0_PMST_ATYPE_CAPABLE                                                     (0U)
#define CSL_QOS_R5FSS0_CPU0_PMST_ASEL_CAPABLE                                                      (1U)
#define CSL_QOS_R5FSS0_CPU1_PMST_MMR_BASE                                                          (0x45d18400ul)
#define CSL_QOS_R5FSS0_CPU1_PMST_CHANNEL_COUNT                                                     (1U)
#define CSL_QOS_R5FSS0_CPU1_PMST_QOS_CAPABLE                                                       (0U)
#define CSL_QOS_R5FSS0_CPU1_PMST_VIRTID_CAPABLE                                                    (0U)
#define CSL_QOS_R5FSS0_CPU1_PMST_ORDERID_CAPABLE                                                   (1U)
#define CSL_QOS_R5FSS0_CPU1_PMST_EPRIORITY_CAPABLE                                                 (1U)
#define CSL_QOS_R5FSS0_CPU1_PMST_ATYPE_CAPABLE                                                     (0U)
#define CSL_QOS_R5FSS0_CPU1_PMST_ASEL_CAPABLE                                                      (1U)

/* Properties of QOS in: R5FSS1 */
#define CSL_QOS_R5FSS1_CPU0_PMST_MMR_BASE                                                          (0x45d18800ul)
#define CSL_QOS_R5FSS1_CPU0_PMST_CHANNEL_COUNT                                                     (1U)
#define CSL_QOS_R5FSS1_CPU0_PMST_QOS_CAPABLE                                                       (0U)
#define CSL_QOS_R5FSS1_CPU0_PMST_VIRTID_CAPABLE                                                    (0U)
#define CSL_QOS_R5FSS1_CPU0_PMST_ORDERID_CAPABLE                                                   (1U)
#define CSL_QOS_R5FSS1_CPU0_PMST_EPRIORITY_CAPABLE                                                 (1U)
#define CSL_QOS_R5FSS1_CPU0_PMST_ATYPE_CAPABLE                                                     (0U)
#define CSL_QOS_R5FSS1_CPU0_PMST_ASEL_CAPABLE                                                      (1U)
#define CSL_QOS_R5FSS1_CPU1_PMST_MMR_BASE                                                          (0x45d18c00ul)
#define CSL_QOS_R5FSS1_CPU1_PMST_CHANNEL_COUNT                                                     (1U)
#define CSL_QOS_R5FSS1_CPU1_PMST_QOS_CAPABLE                                                       (0U)
#define CSL_QOS_R5FSS1_CPU1_PMST_VIRTID_CAPABLE                                                    (0U)
#define CSL_QOS_R5FSS1_CPU1_PMST_ORDERID_CAPABLE                                                   (1U)
#define CSL_QOS_R5FSS1_CPU1_PMST_EPRIORITY_CAPABLE                                                 (1U)
#define CSL_QOS_R5FSS1_CPU1_PMST_ATYPE_CAPABLE                                                     (0U)
#define CSL_QOS_R5FSS1_CPU1_PMST_ASEL_CAPABLE                                                      (1U)

/* Properties of QOS in: DEBUGSS_WRAP0 */
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_MMR_BASE                                                      (0x45d20800ul)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_CHANNEL_COUNT                                                 (1U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_QOS_CAPABLE                                                   (0U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_VIRTID_CAPABLE                                                (0U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_ORDERID_CAPABLE                                               (1U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_EPRIORITY_CAPABLE                                             (1U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_ATYPE_CAPABLE                                                 (0U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMR_ASEL_CAPABLE                                                  (1U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_MMR_BASE                                                      (0x45d20c00ul)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_CHANNEL_COUNT                                                 (1U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_QOS_CAPABLE                                                   (0U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_VIRTID_CAPABLE                                                (0U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_ORDERID_CAPABLE                                               (1U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_EPRIORITY_CAPABLE                                             (1U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_ATYPE_CAPABLE                                                 (0U)
#define CSL_QOS_DEBUGSS_WRAP0_VBUSMW_ASEL_CAPABLE                                                  (1U)

/* Properties of QOS in: LED0 */
#define CSL_QOS_LED0_VBUSP_MMR_BASE                                                                (0x45d21800ul)
#define CSL_QOS_LED0_VBUSP_CHANNEL_COUNT                                                           (1U)
#define CSL_QOS_LED0_VBUSP_QOS_CAPABLE                                                             (0U)
#define CSL_QOS_LED0_VBUSP_VIRTID_CAPABLE                                                          (0U)
#define CSL_QOS_LED0_VBUSP_ORDERID_CAPABLE                                                         (1U)
#define CSL_QOS_LED0_VBUSP_EPRIORITY_CAPABLE                                                       (1U)
#define CSL_QOS_LED0_VBUSP_ATYPE_CAPABLE                                                           (0U)
#define CSL_QOS_LED0_VBUSP_ASEL_CAPABLE                                                            (1U)

/* Properties of QOS in: SA3_SS0 */
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_MMR_BASE                                                   (0x45d22000ul)
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_CHANNEL_COUNT                                              (1U)
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_QOS_CAPABLE                                                (0U)
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_VIRTID_CAPABLE                                             (0U)
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_ORDERID_CAPABLE                                            (1U)
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_EPRIORITY_CAPABLE                                          (1U)
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_ATYPE_CAPABLE                                              (0U)
#define CSL_QOS_SA3_SS0_CTXCACH_EXT_DMA_ASEL_CAPABLE                                               (1U)

/* Properties of QOS in: C7X256V0 */
#define CSL_QOS_C7X256V0_C7XV_SOC_MMR_BASE                                                         (0x45d30000ul)
#define CSL_QOS_C7X256V0_C7XV_SOC_CHANNEL_COUNT                                                    (2U)
#define CSL_QOS_C7X256V0_C7XV_SOC_QOS_CAPABLE                                                      (0U)
#define CSL_QOS_C7X256V0_C7XV_SOC_VIRTID_CAPABLE                                                   (0U)
#define CSL_QOS_C7X256V0_C7XV_SOC_ORDERID_CAPABLE                                                  (1U)
#define CSL_QOS_C7X256V0_C7XV_SOC_EPRIORITY_CAPABLE                                                (0U)
#define CSL_QOS_C7X256V0_C7XV_SOC_ATYPE_CAPABLE                                                    (0U)
#define CSL_QOS_C7X256V0_C7XV_SOC_ASEL_CAPABLE                                                     (0U)

/* Properties of QOS in: RL2_OF_CBA4_0 */
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_MMR_BASE                                                        (0x45d30800ul)
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFR_ASEL_CAPABLE                                                    (1U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_MMR_BASE                                                        (0x45d30c00ul)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_0_IIFW_ASEL_CAPABLE                                                    (1U)

/* Properties of QOS in: RL2_OF_CBA4_1 */
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_MMR_BASE                                                        (0x45d31000ul)
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFR_ASEL_CAPABLE                                                    (1U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_MMR_BASE                                                        (0x45d31400ul)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_1_IIFW_ASEL_CAPABLE                                                    (1U)

/* Properties of QOS in: RL2_OF_CBA4_2 */
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_MMR_BASE                                                        (0x45d31800ul)
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFR_ASEL_CAPABLE                                                    (1U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_MMR_BASE                                                        (0x45d31c00ul)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_2_IIFW_ASEL_CAPABLE                                                    (1U)

/* Properties of QOS in: RL2_OF_CBA4_3 */
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_MMR_BASE                                                        (0x45d32000ul)
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFR_ASEL_CAPABLE                                                    (1U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_MMR_BASE                                                        (0x45d32400ul)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_CHANNEL_COUNT                                                   (1U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_QOS_CAPABLE                                                     (0U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_VIRTID_CAPABLE                                                  (0U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_ORDERID_CAPABLE                                                 (1U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_EPRIORITY_CAPABLE                                               (1U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_ATYPE_CAPABLE                                                   (0U)
#define CSL_QOS_RL2_OF_CBA4_3_IIFW_ASEL_CAPABLE                                                    (1U)

/* Properties of QOS in: EDMA1 */
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_MMR_BASE                                                     (0x45d33000ul)
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_CHANNEL_COUNT                                                (1U)
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_QOS_CAPABLE                                                  (0U)
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_VIRTID_CAPABLE                                               (0U)
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_ORDERID_CAPABLE                                              (1U)
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_EPRIORITY_CAPABLE                                            (0U)
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_ATYPE_CAPABLE                                                (0U)
#define CSL_QOS_EDMA1_TPTC_A0_R_VBUSM_ASEL_CAPABLE                                                 (1U)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_MMR_BASE                                                     (0x45d33400ul)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_CHANNEL_COUNT                                                (1U)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_QOS_CAPABLE                                                  (0U)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_VIRTID_CAPABLE                                               (0U)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_ORDERID_CAPABLE                                              (1U)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_EPRIORITY_CAPABLE                                            (0U)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_ATYPE_CAPABLE                                                (0U)
#define CSL_QOS_EDMA1_TPTC_A0_W_VBUSM_ASEL_CAPABLE                                                 (1U)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_MMR_BASE                                                     (0x45d33800ul)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_CHANNEL_COUNT                                                (1U)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_QOS_CAPABLE                                                  (0U)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_VIRTID_CAPABLE                                               (0U)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_ORDERID_CAPABLE                                              (1U)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_EPRIORITY_CAPABLE                                            (0U)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_ATYPE_CAPABLE                                                (0U)
#define CSL_QOS_EDMA1_TPTC_A1_R_VBUSM_ASEL_CAPABLE                                                 (1U)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_MMR_BASE                                                     (0x45d33c00ul)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_CHANNEL_COUNT                                                (1U)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_QOS_CAPABLE                                                  (0U)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_VIRTID_CAPABLE                                               (0U)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_ORDERID_CAPABLE                                              (1U)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_EPRIORITY_CAPABLE                                            (0U)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_ATYPE_CAPABLE                                                (0U)
#define CSL_QOS_EDMA1_TPTC_A1_W_VBUSM_ASEL_CAPABLE                                                 (1U)

#ifdef __cplusplus
}
#endif
#endif /* CSLR_SOC_QOS_H_ */

