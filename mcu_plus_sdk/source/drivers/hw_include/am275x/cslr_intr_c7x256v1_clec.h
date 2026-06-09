/********************************************************************
*
* C7X256V1_CLEC INTERRUPT MAP. header file
*
* Copyright (C) 2015-2023 Texas Instruments Incorporated.
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
#ifndef CSLR_C7X256V1_CLEC_INTERRUPT_MAP_H_
#define CSLR_C7X256V1_CLEC_INTERRUPT_MAP_H_

#include <drivers/hw_include/cslr.h>
#include <drivers/hw_include/tistdtypes.h>
#ifdef __cplusplus
extern "C"
{
#endif

/*
* List of intr sources for receiver: C7X256V1_CLEC
*/

#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_RTI5_INTR_WWD_0                                           (4U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_CTRL_MMR0_IPC_SET1_IPC_SET_IPCFG_0                        (5U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_1_MAILBOX_CLUSTER_PEND_3         (6U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_2_MAILBOX_CLUSTER_PEND_3         (7U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_3_MAILBOX_CLUSTER_PEND_3         (8U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_4_MAILBOX_CLUSTER_PEND_3         (9U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_MAILBOX0_MAILBOX_CLUSTER_7_MAILBOX_CLUSTER_PEND_2         (10U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_C7X256V0_CLEC_SOC_EVENTS_OUT_LEVEL_20                     (12U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_C7X256V0_CLEC_SOC_EVENTS_OUT_LEVEL_21                     (13U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_C7X256V0_CLEC_SOC_EVENTS_OUT_LEVEL_22                     (14U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_C7X256V0_CLEC_SOC_EVENTS_OUT_LEVEL_23                     (15U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_100                   (16U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_101                   (17U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_102                   (18U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_103                   (19U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_104                   (20U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_105                   (21U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_106                   (22U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_107                   (23U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_108                   (24U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_109                   (25U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_110                   (26U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_111                   (27U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_112                   (28U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_113                   (29U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_114                   (30U)
#define CSLR_C7X256V1_CLEC_SOC_EVENTS_IN_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_115                   (31U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_0                                     (32U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_1                                     (33U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_2                                     (34U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_3                                     (35U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_4                                     (36U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_5                                     (37U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_6                                     (38U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_7                                     (39U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_8                                     (40U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_9                                     (41U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_10                                    (42U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_11                                    (43U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_12                                    (44U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_13                                    (45U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_14                                    (46U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MAIN_GPIO_INTROUTER0_OUTP_15                                    (47U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_CPSW0_CPTS_COMP_0                                               (48U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC0_INFIFO_LEVEL_0                                           (50U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC0_INGROUP_LEVEL_0                                          (51U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC0_OUTFIFO_LEVEL_0                                          (52U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC0_OUTGROUP_LEVEL_0                                         (53U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC0_ERR_LEVEL_0                                              (54U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC1_INFIFO_LEVEL_0                                           (55U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC1_INGROUP_LEVEL_0                                          (56U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC1_OUTFIFO_LEVEL_0                                          (57U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC1_OUTGROUP_LEVEL_0                                         (58U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_AASRC1_ERR_LEVEL_0                                              (59U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MLB0_MLBSS_MLB_INT_0                                            (60U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MLB0_MLBSS_MLB_AHB_INT_0                                        (61U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MLB0_MLBSS_MLB_AHB_INT_1                                        (62U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER8_INTR_PEND_0                                              (64U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER9_INTR_PEND_0                                              (65U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER10_INTR_PEND_0                                             (66U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER11_INTR_PEND_0                                             (67U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER12_INTR_PEND_0                                             (68U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER13_INTR_PEND_0                                             (69U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER14_INTR_PEND_0                                             (70U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER15_INTR_PEND_0                                             (71U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_32                          (96U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_33                          (97U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_34                          (98U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_35                          (99U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_36                          (100U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_37                          (101U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_38                          (102U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DMASS0_INTAGGR_0_INTAGGR_VINTR_PEND_39                          (103U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCU_MCU_GPIO_INTROUTER0_OUTP_0                                  (104U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCU_MCU_GPIO_INTROUTER0_OUTP_1                                  (105U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCU_MCU_GPIO_INTROUTER0_OUTP_2                                  (106U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCU_MCU_GPIO_INTROUTER0_OUTP_3                                  (107U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_GLUELOGIC_MAINRESET_REQUEST_GLUE_MAIN_PORZ_SYNC_STRETCH_0       (110U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_GLUELOGIC_MAINRESET_REQUEST_GLUE_MAIN_RESETZ_SYNC_STRETCH_0     (111U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SA3_SS0_INTAGGR_0_INTAGGR_VINTR_4                               (112U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SA3_SS0_INTAGGR_0_INTAGGR_VINTR_5                               (113U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_GLUELOGIC_MAIN_DCC_DONE_GLUE_DCC_DONE_0                         (128U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_GLUELOGIC_SOC_ACCESS_ERR_INTR_GLUE_OUT_0                        (129U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_EFUSE0_EFC_ERROR_0                                              (131U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_WKUP_RTCSS0_RTC_EVENT_PEND_0                                    (132U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_GLUELOGIC_SOC_CBASS_ERR_INTR_GLUE_MAIN_CBASS_AGG_ERR_INTR_0     (133U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_CPSW0_EVNT_PEND_0                                               (134U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_CPSW0_MDIO_PEND_0                                               (135U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_CPSW0_STAT_PEND_0                                               (136U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SMS0_TIFS_CBASS_0_FW_EXCEPTION_INTR_0                           (143U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SMS0_COMMON_0_COMBINED_SEC_IN_0                                 (144U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ECAP0_ECAP_INT_0                                                (145U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ECAP1_ECAP_INT_0                                                (146U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ECAP2_ECAP_INT_0                                                (147U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ECAP3_ECAP_INT_0                                                (148U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ECAP4_ECAP_INT_0                                                (149U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ECAP5_ECAP_INT_0                                                (150U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ADC12FCC0_GEN_LEVEL_0                                           (151U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER0_INTR_PEND_0                                              (152U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER1_INTR_PEND_0                                              (153U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER2_INTR_PEND_0                                              (154U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER3_INTR_PEND_0                                              (155U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER4_INTR_PEND_0                                              (156U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER5_INTR_PEND_0                                              (157U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER6_INTR_PEND_0                                              (158U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_TIMER7_INTR_PEND_0                                              (159U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SA3_SS0_SA_UL_0_SA_UL_PKA_0                                     (160U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SA3_SS0_SA_UL_0_SA_UL_TRNG_0                                    (161U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MMCSD0_EMMCSDSS_INTR_0                                          (165U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCRC64_0_INT_MCRC_0                                             (166U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS0_FSAS_0_ECC_INTR_ERR_PEND_0                                 (169U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS_OF_UL0_FSAS_ECC_INTR_ERR_PEND_0                             (170U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS_OF_UL0_OSPI0_LVL_INTR_0                                     (171U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS_OF_UL0_FSAS_FOTA_STAT_INTR_PEND_0                           (172U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS_OF_UL0_FSAS_FOTA_STAT_ERR_PEND_0                            (173U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS_OF_UL0_OTFA_INTR_ERR_PEND_0                                 (174U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS0_FSAS_0_OTFA_INTR_ERR_PEND_0                                (175U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SMS0_AESEIP38T_0_AES_SINTREQUEST_P_0                            (176U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DDPA0_DDPA_INTR_0                                               (177U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS0_OSPI_0_OSPI_LVL_INTR_0                                     (178U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_FSS0_HYPERBUS1P0_0_HPB_INTR_0                                   (179U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ESM0_ESM_INT_CFG_LVL_0                                          (180U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ESM0_ESM_INT_HI_LVL_0                                           (181U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_ESM0_ESM_INT_LOW_LVL_0                                          (182U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_WKUP_VTM0_THERM_LVL_GT_TH1_INTR_0                               (183U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_WKUP_VTM0_THERM_LVL_GT_TH2_INTR_0                               (184U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_WKUP_VTM0_THERM_LVL_LT_TH0_INTR_0                               (185U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN0_MCANSS_EXT_TS_ROLLOVER_LVL_INT_0                          (186U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN0_MCANSS_MCAN_LVL_INT_0                                     (187U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN0_MCANSS_MCAN_LVL_INT_1                                     (188U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN2_MCANSS_EXT_TS_ROLLOVER_LVL_INT_0                          (189U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN3_MCANSS_EXT_TS_ROLLOVER_LVL_INT_0                          (190U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN4_MCANSS_EXT_TS_ROLLOVER_LVL_INT_0                          (191U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_I2C0_POINTRPEND_0                                               (193U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_I2C1_POINTRPEND_0                                               (194U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_I2C2_POINTRPEND_0                                               (195U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_I2C3_POINTRPEND_0                                               (196U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_WKUP_I2C0_POINTRPEND_0                                          (197U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_SMS0_AESEIP38T_0_AES_SINTREQUEST_S_0                            (198U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN1_MCANSS_EXT_TS_ROLLOVER_LVL_INT_0                          (199U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DEBUGSS0_AQCMPINTR_LEVEL_0                                      (201U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_DEBUGSS0_CTM_LEVEL_0                                            (202U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_PSCSS0_PSC_ALLINT_0                                             (203U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCSPI0_INTR_SPI_0                                               (204U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCSPI1_INTR_SPI_0                                               (205U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCSPI2_INTR_SPI_0                                               (206U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCSPI3_INTR_SPI_0                                               (207U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCSPI4_INTR_SPI_0                                               (208U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_UART0_USART_IRQ_0                                               (210U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_UART1_USART_IRQ_0                                               (211U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_UART2_USART_IRQ_0                                               (212U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_UART3_USART_IRQ_0                                               (213U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_UART4_USART_IRQ_0                                               (214U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_UART5_USART_IRQ_0                                               (215U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_UART6_USART_IRQ_0                                               (216U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_WKUP_UART0_USART_IRQ_0                                          (218U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_0                                                      (220U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_1                                                      (221U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_2                                                      (222U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_3                                                      (223U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_4                                                      (224U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_5                                                      (225U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_6                                                      (226U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_IRQ_7                                                      (227U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_USB0_MISC_LEVEL_0                                               (228U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_EPWM0_EPWM_ETINT_0                                              (229U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_EPWM0_EPWM_TRIPZINT_0                                           (230U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_EPWM1_EPWM_ETINT_0                                              (231U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_EPWM1_EPWM_TRIPZINT_0                                           (233U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_EPWM2_EPWM_ETINT_0                                              (234U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_EPWM2_EPWM_TRIPZINT_0                                           (235U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN1_MCANSS_MCAN_LVL_INT_0                                     (245U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN1_MCANSS_MCAN_LVL_INT_1                                     (246U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN2_MCANSS_MCAN_LVL_INT_0                                     (247U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN2_MCANSS_MCAN_LVL_INT_1                                     (248U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN3_MCANSS_MCAN_LVL_INT_0                                     (249U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN3_MCANSS_MCAN_LVL_INT_1                                     (250U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN4_MCANSS_MCAN_LVL_INT_0                                     (251U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCAN4_MCANSS_MCAN_LVL_INT_1                                     (252U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_GLUELOGIC_EXT_INTN_GLUE_OUT_0                                   (256U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_RL2_OF_CBA4_0_ERR_LVL_0                                         (260U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_RL2_OF_CBA4_1_ERR_LVL_0                                         (261U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_RL2_OF_CBA4_2_ERR_LVL_0                                         (262U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_RL2_OF_CBA4_3_ERR_LVL_0                                         (263U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP0_REC_INTR_PEND_0                                          (267U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP0_XMIT_INTR_PEND_0                                         (268U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP1_REC_INTR_PEND_0                                          (269U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP1_XMIT_INTR_PEND_0                                         (270U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP2_REC_INTR_PEND_0                                          (271U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP2_XMIT_INTR_PEND_0                                         (272U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP3_REC_INTR_PEND_0                                          (273U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP3_XMIT_INTR_PEND_0                                         (274U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP4_REC_INTR_PEND_0                                          (275U - 32U)
#define CSLR_C7X256V1_CLEC_GIC_SPI_MCASP4_XMIT_INTR_PEND_0                                         (276U - 32U)

#ifdef __cplusplus
}
#endif
#endif /* CSLR_C7X256V1_CLEC_INTERRUPT_MAP_H_ */

