/**
 *   Copyright (c) Texas Instruments Incorporated 2018
 *   All rights reserved.
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
 *  \file csl_hts.h
 *
 *  \brief HAL interface file for HTS module of VPAC
 *
 */

#ifndef CSL_HTS_H_
#define CSL_HTS_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
/* This is needed for memset/memcpy */
#include <string.h>
#include <ti/csl/csl_fvid2_dataTypes.h>
#include <ti/csl/cslr_hts.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/** \brief Maximum number of consumer supported in HTS */
#define CSL_HTS_MAX_CONSUMER                         (6U)
/** \brief Maximum number of producer supported in HTS */
#define CSL_HTS_MAX_PRODUCER                         (11U)

/** For Invalid parameter in HTS */
#define CSL_HTS_INVALID_PARAM                        (0U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/**
 *  \anchor CSL_HtsHwaSch
 *  \name   HTS hardware schedular ID's
 *  \brief  MACRO for HTS Hardware Scheduler
 *          The number of hardware scheduler supporte in HTS are fixed and
 *          each scheduler is fixed for one of the HWA core. This enum is used
 *          for setting/configuring up appropriate core.
 *
 *  @{
 */
/** \brief Hardware Scheduler for VISS0 */
#define CSL_HTS_HWA_SCH_VISS0               (0U)
/** \brief Hardware Scheduler for VISS1 */
#define CSL_HTS_HWA_SCH_VISS1               (1U)
/** \brief Hardware Scheduler for LDC0 */
#define CSL_HTS_HWA_SCH_LDC0                (2U)
/** \brief Hardware Scheduler for LDC1 */
#define CSL_HTS_HWA_SCH_LDC1                (3U)
/** \brief Hardware Scheduler for MSC0 */
#define CSL_HTS_HWA_SCH_MSC0                (4U)
/** \brief Hardware Scheduler for MSC1 */
#define CSL_HTS_HWA_SCH_MSC1                (5U)
/** \brief Hardware Scheduler for NF */
#define CSL_HTS_HWA_SCH_NF                  (6U)
/** \brief Hardware Scheduler for DOF, uses same SCH as VISS0 */
#define CSL_HTS_HWA_SCH_DOF                 (7U)
/** \brief Hardware Scheduler for SDE, uses same SCH as VISS1 */
#define CSL_HTS_HWA_SCH_SDE                 (8U)
/** \brief Hardware Scheduler for FOCO1 */
#define CSL_HTS_HWA_SCH_FOCO1               (9U)
/** \brief Hardware Scheduler for FOCO2 */
#define CSL_HTS_HWA_SCH_FOCO2               (10U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP0                 (11U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP1                 (12U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP2                 (13U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP3                 (14U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP4                 (15U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP5                 (16U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP6                 (17U)
/** \brief Common Hardware Scheduler for VPAC */
#define CSL_HTS_HWA_SCH_SP7                 (18U)
/** \brief Maximum number of Hardware scehduler supported */
#define CSL_HTS_HWA_MAX_SCH                 (19U)
/** @} */

/**
 *  \anchor CSL_HtsPipeline
 *  \name   HTS hardware schedular ID's
 *  \brief  MACRO for HTS pipeline id
 *
 *  @{
 */
 /**< \brief Pipeline 0 */
#define CSL_HTS_PIPELINE_0           (0U)
/**< \brief Pipeline 1 */
#define CSL_HTS_PIPELINE_1           (1U)
/**< \brief Pipeline 2 */
#define CSL_HTS_PIPELINE_2           (2U)
/**< \brief Pipeline 3 */
#define CSL_HTS_PIPELINE_3           (3U)
/**< \brief Pipeline 4 */
#define CSL_HTS_PIPELINE_4           (4U)
/**< \brief Pipeline 5 */
#define CSL_HTS_PIPELINE_5           (5U)
/**< \brief Max pipeline */
#define VPSHAL_HTS_MAX_PIPELINE      (6U)
/** @} */

/**
 *  \anchor CSL_HtsProdIdx
 *  \name   HTS Producer Id's
 *  \brief  MACRO for HTS HTS Producer Id
 *          The producer are nodes producing data to be consumed by the
 *          consumer nodes. This producers and the number of producers are
 *          fixed for each core and Dma schedulers. This enum uis used
 *          for identifying producer for given HWA scheduler. It is used also
 *          in DMA scheduler.
 *
 *  @{
 */
#define CSL_HTS_PROD_IDX_UDMA        (0U)
#define CSL_HTS_PROD_IDX_VISS0_Y12   (1U)
#define CSL_HTS_PROD_IDX_VISS0_UV12  (2U)
#define CSL_HTS_PROD_IDX_VISS0_Y8    (3U)
#define CSL_HTS_PROD_IDX_VISS0_UV8   (4U)
#define CSL_HTS_PROD_IDX_VISS0_S8    (5U)
#define CSL_HTS_PROD_IDX_VISS0_H3A   (6U)
#define CSL_HTS_PROD_IDX_VISS0_SP    (7U)
#define CSL_HTS_PROD_IDX_VISS1_Y12   (17U)
#define CSL_HTS_PROD_IDX_VISS1_YU12  (18U)
#define CSL_HTS_PROD_IDX_VISS1_Y8    (19U)
#define CSL_HTS_PROD_IDX_VISS1_UV8   (20U)
#define CSL_HTS_PROD_IDX_VISS1_S8    (21U)
#define CSL_HTS_PROD_IDX_VISS1_H3A   (22U)
#define CSL_HTS_PROD_IDX_VISS1_SP    (23U)
#define CSL_HTS_PROD_IDX_LDC0_Y12    (33U)
#define CSL_HTS_PROD_IDX_LDC0_UV12   (34U)
#define CSL_HTS_PROD_IDX_LDC0_Y8     (35U)
#define CSL_HTS_PROD_IDX_LDC0_UV8    (36U)
#define CSL_HTS_PROD_IDX_LDC0_MSC0   (37U)
#define CSL_HTS_PROD_IDX_LDC0_MSC1   (38U)
#define CSL_HTS_PROD_IDX_LDC0_NF     (39U)
#define CSL_HTS_PROD_IDX_LDC0_SP     (40U)
#define CSL_HTS_PROD_IDX_MSC_0       (65U)
#define CSL_HTS_PROD_IDX_MSC_1       (66U)
#define CSL_HTS_PROD_IDX_MSC_2       (67U)
#define CSL_HTS_PROD_IDX_MSC_3       (68U)
#define CSL_HTS_PROD_IDX_MSC_4       (69U)
#define CSL_HTS_PROD_IDX_MSC_5       (70U)
#define CSL_HTS_PROD_IDX_MSC_6       (71U)
#define CSL_HTS_PROD_IDX_MSC_7       (72U)
#define CSL_HTS_PROD_IDX_MSC_8       (73U)
#define CSL_HTS_PROD_IDX_MSC_9       (74U)
#define CSL_HTS_PROD_IDX_MSC0_SP     (75U)
#define CSL_HTS_PROD_IDX_MSC1_SP     (91U)
#define CSL_HTS_PROD_IDX_NF          (97U)
#define CSL_HTS_PROD_IDX_NF_SP       (98U)

#define CSL_HTS_PROD_IDX_SCH0_0      (193U)
#define CSL_HTS_PROD_IDX_SCH0_1      (194U)
#define CSL_HTS_PROD_IDX_SCH0_2      (195U)
#define CSL_HTS_PROD_IDX_SCH0_SP     (196U)
#define CSL_HTS_PROD_IDX_SCH1_0      (209U)
#define CSL_HTS_PROD_IDX_SCH1_1      (210U)
#define CSL_HTS_PROD_IDX_SCH1_2      (211U)
#define CSL_HTS_PROD_IDX_SCH1_SP     (212U)
#define CSL_HTS_PROD_IDX_SCH2_0      (225U)
#define CSL_HTS_PROD_IDX_SCH2_1      (226U)
#define CSL_HTS_PROD_IDX_SCH2_2      (227U)
#define CSL_HTS_PROD_IDX_SCH2_SP     (228U)
#define CSL_HTS_PROD_IDX_SCH3_0      (241U)
#define CSL_HTS_PROD_IDX_SCH3_1      (242U)
#define CSL_HTS_PROD_IDX_SCH3_2      (243U)
#define CSL_HTS_PROD_IDX_SCH3_SP     (244U)
#define CSL_HTS_PROD_IDX_SCH4_0      (257U)
#define CSL_HTS_PROD_IDX_SCH4_1      (258U)
#define CSL_HTS_PROD_IDX_SCH4_2      (259U)
#define CSL_HTS_PROD_IDX_SCH4_SP     (260U)
#define CSL_HTS_PROD_IDX_SCH5_0      (273U)
#define CSL_HTS_PROD_IDX_SCH5_1      (274U)
#define CSL_HTS_PROD_IDX_SCH5_2      (275U)
#define CSL_HTS_PROD_IDX_SCH5_SP     (276U)
#define CSL_HTS_PROD_IDX_SCH6_0      (289U)
#define CSL_HTS_PROD_IDX_SCH6_1      (290U)
#define CSL_HTS_PROD_IDX_SCH6_2      (291U)
#define CSL_HTS_PROD_IDX_SCH6_SP     (292U)
#define CSL_HTS_PROD_IDX_SCH7_0      (305U)
#define CSL_HTS_PROD_IDX_SCH7_1      (306U)
#define CSL_HTS_PROD_IDX_SCH7_2      (307U)
#define CSL_HTS_PROD_IDX_SCH7_SP     (308U)

#define CSL_HTS_PROD_IDX_DOF_FLOW_V  (0U)
#define CSL_HTS_PROD_IDX_SDE         (6U)
#define CSL_HTS_PROD_IDX_FOCO0_0     (113U)
#define CSL_HTS_PROD_IDX_FOCO0_1     (114U)
#define CSL_HTS_PROD_IDX_FOCO1_0     (129U)
#define CSL_HTS_PROD_IDX_FOCO1_1     (130U)
/** @} */

/**
 *  \anchor CSL_HtsConsIdx
 *  \name   HTS Consumer Id's
 *  \brief  MACRO for HTS Consumer
 *          The consumer are nodes consumer data to be provided by the
 *          producer nodes. This consumer and the number of consumer are
 *          fixed for each core and Dma schedulers. This enum is used
 *          for identifying consumer for given HWA scheduler. It is used also
 *          in DMA scheduler.
 *
 *  @{
 */
#define CSL_HTS_CONS_IDX_UDMA        (0U)
#define CSL_HTS_CONS_IDX_ISS0_EXP0   (1U)
#define CSL_HTS_CONS_IDX_ISS0_EXP1   (2U)
#define CSL_HTS_CONS_IDX_ISS0_EXP2   (3U)
#define CSL_HTS_CONS_IDX_ISS0_SP     (6U)

#define CSL_HTS_CONS_IDX_ISS1_EXP0   (9U)
#define CSL_HTS_CONS_IDX_ISS1_EXP1   (10U)
#define CSL_HTS_CONS_IDX_ISS1_EXP2   (11U)
#define CSL_HTS_CONS_IDX_ISS1_SP     (12U)

#define CSL_HTS_CONS_IDX_LDC0_Y      (17U)
#define CSL_HTS_CONS_IDX_LDC0_UV     (18U)
#define CSL_HTS_CONS_IDX_LDC0_SP     (19U)
#define CSL_HTS_CONS_IDX_LDC1_Y      (25U)
#define CSL_HTS_CONS_IDX_LDC1_UV     (26U)
#define CSL_HTS_CONS_IDX_LDC1_SP     (27U)

#define CSL_HTS_CONS_IDX_MSC0_CH0    (33U)
#define CSL_HTS_CONS_IDX_MSC0_SP     (34U)
#define CSL_HTS_CONS_IDX_MSC1_CH0    (41U)
#define CSL_HTS_CONS_IDX_MSC1_SP     (42U)

#define CSL_HTS_CONS_IDX_NF          (49U)
#define CSL_HTS_CONS_IDX_NF_SP       (50U)

#define CSL_HTS_CONS_IDX_SCH0        (97u)
#define CSL_HTS_CONS_IDX_SCH0_SP     (98u)
#define CSL_HTS_CONS_IDX_SCH1        (105u)
#define CSL_HTS_CONS_IDX_SCH1_SP     (106)
#define CSL_HTS_CONS_IDX_SCH2        (113u)
#define CSL_HTS_CONS_IDX_SCH2_SP     (114u)
#define CSL_HTS_CONS_IDX_SCH3        (121u)
#define CSL_HTS_CONS_IDX_SCH3_SP     (122u)
#define CSL_HTS_CONS_IDX_SCH4        (129u)
#define CSL_HTS_CONS_IDX_SCH4_SP     (130u)
#define CSL_HTS_CONS_IDX_SCH5        (137u)
#define CSL_HTS_CONS_IDX_SCH5_SP     (138u)
#define CSL_HTS_CONS_IDX_SCH6        (145u)
#define CSL_HTS_CONS_IDX_SCH6_SP     (146u)
#define CSL_HTS_CONS_IDX_SCH7        (153u)
#define CSL_HTS_CONS_IDX_SCH7_SP     (154u)

#define CSL_HTS_CONS_IDX_DOF_CURR    (1U)
#define CSL_HTS_CONS_IDX_DOF_REF     (2U)
#define CSL_HTS_CONS_IDX_DOF_TEMP    (3U)
#define CSL_HTS_CONS_IDX_DOF_PYRAMID (4U)
#define CSL_HTS_CONS_IDX_DOF_BINMAP  (5U)
#define CSL_HTS_CONS_IDX_SDE_BASE    (9U)
#define CSL_HTS_CONS_IDX_SDE_REF     (10U)

/** @} */

/**
 *  \anchor CSL_HtsDmaSchNodes
 *  \name   HTS DMA Scheduler Id's
 *  \brief  MACRO for DMA schedulers
 *
 *  @{
 */
#define CSL_HTS_DMA_SCH_NODE_PROD_ISS0_EXP0              (0U)
#define CSL_HTS_DMA_SCH_NODE_PROD_ISS0_EXP1              (1U)
#define CSL_HTS_DMA_SCH_NODE_PROD_ISS0_EXP2              (2U)
#define CSL_HTS_DMA_SCH_NODE_PROD_ISS1_EXP0              (6U)
#define CSL_HTS_DMA_SCH_NODE_PROD_ISS1_EXP1              (7U)
#define CSL_HTS_DMA_SCH_NODE_PROD_ISS1_EXP2              (8U)
#define CSL_HTS_DMA_SCH_NODE_PROD_MSC0_IN                (9U)
#define CSL_HTS_DMA_SCH_NODE_PROD_MSC1_IN                (10U)
#define CSL_HTS_DMA_SCH_NODE_PROD_NF_IN                  (11U)

#define CSL_HTS_DMA_SCH_NODE_CONS_ISS0_Y12               (16U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS0_UV12              (17U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS0_Y8                (18U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS0_UV8               (19U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS0_S8                (20U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS0_AE                (21U)

#define CSL_HTS_DMA_SCH_NODE_CONS_ISS1_Y12               (23U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS1_UV12              (24U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS1_Y8                (25U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS1_UV8               (26U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS1_S8                (27U)
#define CSL_HTS_DMA_SCH_NODE_CONS_ISS1_AE                (28U)

#define CSL_HTS_DMA_SCH_NODE_CONS_LDC_Y                  (30U)
#define CSL_HTS_DMA_SCH_NODE_CONS_LDC_UV                 (31U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O0_MSC1_O9        (32U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O1_MSC1_O8        (33U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O2_MSC1_O7        (34U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O3_MSC1_O6        (35U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O4_MSC1_O5        (36U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O5_MSC1_O4        (37U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O6_MSC1_O3        (38U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O7_MSC1_O2        (39U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O8_MSC1_O1        (40U)
#define CSL_HTS_DMA_SCH_NODE_CONS_MSC0_O9_MSC1_O0        (41U)
#define CSL_HTS_DMA_SCH_NODE_CONS_NF_OUT                 (42U)

#define CSL_HTS_DMA_SCH_NODE_PROD_DOF_CURR               (0U)
#define CSL_HTS_DMA_SCH_NODE_PROD_DOF_REF                (1U)
#define CSL_HTS_DMA_SCH_NODE_PROD_DOF_TEMP               (2U)
#define CSL_HTS_DMA_SCH_NODE_PROD_DOF_PYRAMID            (3U)
#define CSL_HTS_DMA_SCH_NODE_PROD_DOF_BINMAP             (4U)
#define CSL_HTS_DMA_SCH_NODE_PROD_SDE_BASE               (6U)
#define CSL_HTS_DMA_SCH_NODE_PROD_SDE_REF                (7U)

#define CSL_HTS_DMA_SCH_NODE_CONS_DOF_FLOW_V             (16U)
#define CSL_HTS_DMA_SCH_NODE_CONS_SDE_OUT                (23U)

#define HWA_PROD_CONTROL_OFFSET                 (0x0U)
#define HWA_PROD_BUF_CONTROL_OFFSET             (0x4U)
#define HWA_PROD_COUNT_OFFSET                   (0x8U)
#define HWA_PROD_PA_CONTROL_OFFSET              (0xCU)
#define HWA_PROD_PA_PROD_COUNT_OFFSET           (0x10U)

#define HWA_CONS_CONTROL_OFFSET                 (0U)

#define HWA_DMA_PROD_SCH_CONTROL_OFFSET         (0x0U)
#define HWA_DMA_PROD_HOP_OFFSET                 (0x4U)
#define HWA_DMA_PROD_CONTROL_OFFSET             (0x10U)
#define HWA_DMA_PROD_BUF_CONTROL_OFFSET         (0x14U)
#define HWA_DMA_PROD_COUNT_OFFSET               (0x18U)
#define HWA_DMA_PROD_PA_CONTROL_OFFSET          (0x1CU)
#define HWA_DMA_PROD_PA_PROD_COUNT_OFFSET       (0x20U)

#define HWA_DMA_CONS_SCH_CONTROL_OFFSET         (0x0U)
#define HWA_DMA_CONS_CONTROL_OFFSET             (0x10U)

#define HWA_SCH_CONTROL_OFFSET                  (0x0U)
#define HWA_HOP_OFFSET                          (0x4U)
#define HWA_WDTIMER_OFFSET                      (0x8U)
#define HWA_BW_LIMITER_OFFSET                   (0xCU)
#define HWA_HTS_NUM_SCH                         (0xBU)

#define CSL_HTS_NUM_NF_CONSUMERS                 (1U)
#define CSL_HTS_NUM_NF_PRODUCERS                 (1U)
#define CSL_HTS_NUM_NF_PA_PRODUCERS              (1U)

#define CSL_HTS_NUM_NF_PIPELINE                  (1U)
#define CSL_HTS_NUM_NF_SCHEDULERS                (1U)

#define CSL_HTS_NUM_NF_PRODUCERS_DMA             (1U)
#define CSL_HTS_NUM_NF_CONSUMERS_DMA             (1U)

#define CSL_HTS_NUM_MSC0_CONSUMERS               (2U)

#define CSL_HTS_NUM_MSC1_CONSUMERS               (2U)

#define CSL_HTS_NUM_MSC0_PRODUCERS               (11U)
#define CSL_HTS_NUM_MSC0_PA_PRODUCERS            (2U)

#define CSL_HTS_NUM_MSC1_PRODUCERS               (11U)
#define CSL_HTS_NUM_MSC1_PA_PRODUCERS            (2U)

#define CSL_HTS_NUM_MSC0_PIPELINE                (1U)
#define CSL_HTS_NUM_MSC0_SCHEDULERS              (1U)

#define CSL_HTS_NUM_MSC1_PIPELINE                (1U)
#define CSL_HTS_NUM_MSC1_SCHEDULERS              (1U)

#define CSL_HTS_NUM_MSC0_PRODUCERS_DMA           (2U)
#define CSL_HTS_NUM_MSC0_CONSUMERS_DMA           (10U)

#define CSL_HTS_NUM_MSC1_PRODUCERS_DMA           (2U)
#define CSL_HTS_NUM_MSC1_CONSUMERS_DMA           (10U)

/* Summation macros for MSC0 and MSC1 */
#define CSL_HTS_NUM_MSC_TOTAL_CONSUMERS          (CSL_HTS_NUM_MSC0_CONSUMERS + CSL_HTS_NUM_MSC1_CONSUMERS)
#define CSL_HTS_NUM_MSC_TOTAL_PRODUCERS          (CSL_HTS_NUM_MSC0_PRODUCERS + CSL_HTS_NUM_MSC1_PRODUCERS)
#define CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA      (CSL_HTS_NUM_MSC0_PRODUCERS_DMA + CSL_HTS_NUM_MSC1_PRODUCERS_DMA)
#define CSL_HTS_NUM_MSC_TOTAL_CONSUMERS_DMA      (CSL_HTS_NUM_MSC0_CONSUMERS_DMA + CSL_HTS_NUM_MSC1_CONSUMERS_DMA)
#define CSL_HTS_NUM_MSC_SCHEDULERS               (CSL_HTS_NUM_MSC0_SCHEDULERS + CSL_HTS_NUM_MSC1_SCHEDULERS)
#define CSL_HTS_NUM_MSC_PIPELINES                (CSL_HTS_NUM_MSC0_PIPELINE + CSL_HTS_NUM_MSC1_PIPELINE)

#define CSL_HTS_NUM_LDC_CONSUMERS                (1U)
#define CSL_HTS_NUM_LDC_PRODUCERS                (4U)
#define CSL_HTS_NUM_LDC_PRODUCERS_DMA            (1U)
#define CSL_HTS_NUM_LDC_CONSUMERS_DMA            (4U)
#define CSL_HTS_NUM_LDC_PIPELINES                (1U)
#define CSL_HTS_NUM_LDC_SCHEDULERS               (1U)

#define CSL_HTS_NUM_VISS_CONSUMERS               (5U)
#define CSL_HTS_NUM_VISS_PRODUCERS               (7U)
#define CSL_HTS_NUM_VISS_PRODUCERS_DMA           (5U)
#define CSL_HTS_NUM_VISS_CONSUMERS_DMA           (7U)
#define CSL_HTS_NUM_VISS_PIPELINES               (1U)
#define CSL_HTS_NUM_VISS_SCHEDULERS              (1U)

/** @} */

/**
 *  \struct CSL_HtsPaConfig
 *  \brief  Structure defining configuration for pattern adapter
 *          Pattern adapter is used for converting LDC's 2D output to 1D output.
 *          This PA is available only in the LDC producer nodes.
 */
typedef struct
{
    uint32_t            enable;
    /**< Flag to enable/disable Pattern Adapter */
    uint32_t            countDec;
    /**< Decrement count value of prod_count after pattern adapter prod
         count reaches threshold */
    uint32_t            psMaxCnt;
    /**< destination count for pattern adaptation (N), M --> N */
    uint32_t            csMaxCnt;
    /**< source count for pattern adaptation (M), M --> N */
    uint32_t            enableBufCtrl;
    /**< UTRUE = Apply threshold, count_preload and count_postload on
                 pattern adapter count
         UFALSE -> Apply threshold, count_preload and count_postload on
                  prod count (same behavior as if no pattern adapter) */
    uint32_t            enableDecCtrl;
    /**< UTRUE = post pattern adaptation, decrement ps count by count_dec
         UFALSE = post pattern adaptation, decrement ps count by
                 pa_cs_maxcount */
} CSL_HtsPaConfig;

/**
 *  \struct CSL_HtsProdConfig
 *  \brief  Structure defining producer configuration
 */
typedef struct
{
    uint32_t            enable;
    /**< Flag to enable/disable Producer */
    uint32_t            consId;
    /**< ID of the consumer */
    uint32_t            countDec;
    /**< Count decrement value for prod count */
    uint32_t            threshold;
    /**< Count threshold to generate pend for consumer */
    uint32_t            depth;
    /**< Maximum number of producer buffer count */
    uint32_t            cntPreLoad;
    /**< count preload after scheduler start/init */
    uint32_t            cntPostLoad;
    /**< count postload after producer max thread count reached */
    uint32_t            isMaskSelect;
    /**< Flasg to set if mask select is used */
    uint32_t            maskSelect;
    /**< Mask select to select the output for extra producer socket */
    CSL_HtsPaConfig     paCfg;
    /**< Pattern Adapter configuration */
} CSL_HtsProdConfig;

/**
 *  \struct CSL_HtsConsConfig
 *  \brief  Structure defining consumer configuration
 */
typedef struct
{
    uint32_t            enable;
    /**< Flag to enable/disable Consumer */
    uint32_t            prodId;
    /**< ID of the producer */
} CSL_HtsConsConfig;

/**
 *  \struct CSL_HtsDmaProdConfig
 *  \brief  Structure defining configuration for DMA scheduler used as producer
 *
 */
typedef struct
{
    uint32_t            enable;
    /**< Flag to enable/Disable Producer DMA Scheduler */
    uint32_t            bypass;
    /**< UTRUE: Do not perform Enable/Disable for the DMA producer
         UFALSE: Perform Enable/Disable for the DMA producer
         Typically set to UFALSE for all HWA,
         For MSC, it is set to UFALSE only for the allocated MSC outputs,
         For all other outputs, it is set to UTRUE */
    uint32_t            dmaChNum;
    /**< Channel number of the DMA */
    uint32_t            pipeline;
    /**< Pipeline number */
    uint32_t            enableStream;
    /**< Streaming input enable Flag */

    uint32_t            enableHop;
    /**< UTRUE: head of the pipe scheduler */
    uint32_t            numHop;
    /**< Number of Thread count for Head of pipe */

    uint32_t            consId;
    /**< Id of the consumer used in DMA */
    uint32_t            countDec;
    /**< Count decrement value for prod count */
    uint32_t            threshold;
    /**< Count threshold to generate pend for consumer */
    uint32_t            depth;
    /**< Maximum number of producer buffer count */
    uint32_t            cntPreLoad;
    /**< count preload after scheduler start/init */
    uint32_t            cntPostLoad;
    /**< count postload after producer max thread count reached */
    CSL_HtsPaConfig     paCfg;
    /**< Pattern Adapter configuration */
} CSL_HtsDmaProdConfig;

/**
 *  \struct CSL_HtsDmaConsConfig
 *  \brief  Structure defining configuration for DMA scheduler used as consume
 */
typedef struct
{
    uint32_t            enable;
    /**< Flag to enable/Disable Consumer DMA Scheduler */
    uint32_t            bypass;
    /**< UTRUE: Do not perform Enable/Disable for the DMA producer
         UFALSE: Perform Enable/Disable for the DMA producer
         Typically set to UFALSE for all HWA,
         For MSC, it is set to UFALSE only for the allocated MSC outputs,
         For all other outputs, it is set to UTRUE */
    uint32_t            dmaChNum;
    /**< Channel number of the DMA */
    uint32_t            pipeline;
    /**< Pipeline number */
    uint32_t            enableStream;
    /**< Streaming input enable Flag */

    uint32_t            prodId;
    /**< ID of the producer for DMA */
} CSL_HtsDmaConsConfig;

/**
 *  \struct CSL_HtsSchConfig
 *  \brief  Structure defining configuration for HWA scheduler
 */
typedef struct
{
    uint32_t                    schId;
    /**< Id of the Scheduler */
    uint32_t                    pipeline;
    /**< Pipeline number */
    uint32_t                    enableStream;
    /**< Streaming input enable Flag */
    uint32_t                    isDmaChannel;
    uint32_t                    dmaChannelNum;

    uint32_t                    enableHop;
    /**< UTRUE: head of the pipe scheduler */
    uint32_t                    numHop;
    /**< Number of Thread count for Head of pipe */

    uint32_t                    enableWdTimer;
    /**< Enable watch dog timer */

    uint32_t                    isWdTimer128KMode;
    /** Sets to enable 128K mode for the Wd Timer,
     *  0: uses 64K cycle mode for WD timer
     *  Used only when enableWdTimer is enabled */

    uint32_t                    enableBwLimit;
    /**< Enable Bandwidth Limiter */
    uint32_t                    cycleCnt;
    /**< Average Cycle count between successive Thread Start */
    uint32_t                    tokenCnt;
    /**< Max Token count to create average BW */

    CSL_HtsConsConfig           consCfg[CSL_HTS_MAX_CONSUMER];
    /**< Consumer configuration */

    CSL_HtsProdConfig           prodCfg[CSL_HTS_MAX_PRODUCER];
    /**< Producer configuration */

    CSL_HtsDmaConsConfig        dmaConsCfg[CSL_HTS_MAX_PRODUCER];
    /**< DMA scheduler configuration for producer node */

    CSL_HtsDmaProdConfig        dmaProdCfg[CSL_HTS_MAX_CONSUMER];
    /**< DMA scheduler configuration for consumer node */
} CSL_HtsSchConfig;

typedef struct
{
    uint32_t                schId;
    /**< Id of the HWA Scheuler */
    uint32_t                schOffset;
    /**< Offset of the scheduler */
    uint32_t                maxConsumer;
    /**< Maximum number of consumer supported for given HWA */
    uint32_t                consOffset[CSL_HTS_MAX_CONSUMER];
    /**< The register offset of the consumer from the HTS base address */
    uint32_t                maxProducer;
    /**< Maximum number of producer supported for given HWA */
    uint32_t                prodOffset[CSL_HTS_MAX_PRODUCER];
    /**< The register offset of the producer from the HTS base address */
    uint32_t                isPaSupported[CSL_HTS_MAX_PRODUCER];
    /**< Flag to indicate whether Pattern adapter is supported for the
         given producer or not */
    uint32_t                dmaProdOffset[CSL_HTS_MAX_CONSUMER];
    /**< The register offset of the DMA Schedular, used as producer,
         from the HTS base address */
    uint32_t                isDmaProdPaSupported[CSL_HTS_MAX_CONSUMER];
    /**< Flag to indicate whether Pattern adapter is supported for the
         given DMA Producer or not */
    uint32_t                dmaConsOffset[CSL_HTS_MAX_PRODUCER];
    /**< The register offset of the DMA Schedular, used as consumer,
         from the HTS base address */
} CSL_HtsHwaParams;

/**
 * \brief Holds HTS status registers for NF.
 */
typedef struct {
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t HWA_HOP[CSL_HTS_NUM_NF_SCHEDULERS];
    #endif
    uint32_t PROD_COUNT[CSL_HTS_NUM_NF_PRODUCERS];
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t PROD_DMA_HOP[CSL_HTS_NUM_NF_PRODUCERS_DMA];
    #endif
    uint32_t PROD_DMA_COUNT[CSL_HTS_NUM_NF_PRODUCERS_DMA];
} CSL_NfHtsStatusRead;

typedef struct
{
    uint32_t PIPELINE_CONTROL[CSL_HTS_NUM_NF_PIPELINE];
    uint32_t SCHEDULER_CONTROL[CSL_HTS_NUM_NF_SCHEDULERS];
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t HOP[CSL_HTS_NUM_NF_SCHEDULERS];
    #endif
    uint32_t WDTIMER[CSL_HTS_NUM_NF_SCHEDULERS];
    uint32_t BW_LIMITER[CSL_HTS_NUM_NF_SCHEDULERS];
    uint32_t CONS_CONTROL[CSL_HTS_NUM_NF_CONSUMERS];

    uint32_t PROD_CONTROL[CSL_HTS_NUM_NF_PRODUCERS];
    uint32_t PROD_BUF_CONTROL[CSL_HTS_NUM_NF_PRODUCERS];
    uint32_t PROD_COUNT[CSL_HTS_NUM_NF_PRODUCERS];
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t PA_CONTROL[CSL_HTS_NUM_NF_PA_PRODUCERS];
    uint32_t PA_PRODCOUNT[CSL_HTS_NUM_NF_PA_PRODUCERS];
    #endif

    uint32_t PROD_DMA_SCHEDULER_CONTROL[CSL_HTS_NUM_NF_PRODUCERS_DMA];
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t PROD_DMA_HOP[CSL_HTS_NUM_NF_PRODUCERS_DMA];
    #endif
    uint32_t PROD_CONTROL_DMA[CSL_HTS_NUM_NF_PRODUCERS_DMA];
    uint32_t PROD_BUF_CONTROL_DMA[CSL_HTS_NUM_NF_PRODUCERS_DMA];
    uint32_t PROD_COUNT_DMA[CSL_HTS_NUM_NF_PRODUCERS_DMA];

    uint32_t CONS_DMA_SCHEDULER_CONTROL[CSL_HTS_NUM_NF_CONSUMERS_DMA];
    uint32_t CONS_CONTROL_DMA[CSL_HTS_NUM_NF_CONSUMERS_DMA];
} CSL_NfHtsConfigReg;

/**
 * \brief Holds HTS status registers for MSC.
 */
typedef struct {
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t HWA_HOP[CSL_HTS_NUM_MSC_SCHEDULERS];
    #endif
    uint32_t PROD_COUNT[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS];
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t PROD_DMA_HOP[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    #endif
    uint32_t PROD_DMA_COUNT[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
} CSL_MscHtsStatusRead;

/**
 * \brief HTS configuration registers for MSC.
 * Following DOF pattern: comprehensive HTS register coverage including
 * pipeline control, schedulers, producers, and consumers.
 */
typedef struct
{
    uint32_t PIPELINE_CONTROL[CSL_HTS_NUM_MSC_PIPELINES];
    /**< Pipeline control registers for MSC schedulers */
    uint32_t SCHEDULER_CONTROL[CSL_HTS_NUM_MSC_SCHEDULERS];
    /**< Scheduler control registers */
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t HOP[CSL_HTS_NUM_MSC_SCHEDULERS];
    #endif
    /**< HOP registers for schedulers */
    uint32_t WDTIMER[CSL_HTS_NUM_MSC_SCHEDULERS];
    /**< Watchdog timer registers */
    uint32_t BW_LIMITER[CSL_HTS_NUM_MSC_SCHEDULERS];
    /**< Bandwidth limiter registers */
    uint32_t CONS_CONTROL[CSL_HTS_NUM_MSC_TOTAL_CONSUMERS];
    /**< Consumer control registers */


    uint32_t PROD_CONTROL[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS];
    /**< Producer control registers */
    uint32_t PROD_BUF_CONTROL[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS];
    /**< Producer buffer control registers */
    uint32_t PROD_COUNT[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS];
    /**< Producer count registers */
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t PA_CONTROL[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS];
    /**< Ping-pong address control registers */
    uint32_t PA_PRODCOUNT[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS];
    /**< Ping-pong address producer count registers */
    #endif

    uint32_t PROD_DMA_SCHEDULER_CONTROL[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    /**< DMA producer scheduler control registers */
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t PROD_DMA_HOP[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    #endif
    /**< DMA producer HOP registers */
    uint32_t PROD_CONTROL_DMA[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    /**< DMA producer control registers */
    uint32_t PROD_BUF_CONTROL_DMA[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    /**< DMA producer buffer control registers */
    uint32_t PROD_COUNT_DMA[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    /**< DMA producer count registers */
    #if defined(VHWA_VPAC_IP_REV_VPAC3) || defined(VHWA_VPAC_IP_REV_VPAC3L)
    uint32_t PA_CONTROL_DMA[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    /**< DMA producer ping-pong address control registers */
    uint32_t PA_PRODCOUNT_DMA[CSL_HTS_NUM_MSC_TOTAL_PRODUCERS_DMA];
    /**< DMA producer ping-pong address producer count registers */
    #endif

    uint32_t CONS_DMA_SCHEDULER_CONTROL[CSL_HTS_NUM_MSC_TOTAL_CONSUMERS_DMA];
    /**< DMA consumer scheduler control registers */
    uint32_t CONS_CONTROL_DMA[CSL_HTS_NUM_MSC_TOTAL_CONSUMERS_DMA];
    /**< DMA consumer control registers */
} CSL_MscHtsConfigReg;

/**
 * \brief Holds VPAC HTS status registers for LDC.
 */
typedef struct {
    uint32_t HWA_HOP[CSL_HTS_NUM_LDC_PIPELINES];
    /**< HWA scheduler HOP register - should be 0 when HOP disabled */
    uint32_t PROD_COUNT[CSL_HTS_NUM_LDC_PRODUCERS];
    /**< Producer count registers for all LDC producers */
    uint32_t PA_PRODCOUNT[CSL_HTS_NUM_LDC_PRODUCERS];
    /**< Pattern Adapter producer count registers */
    uint32_t PROD_DMA_HOP[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer HOP registers - should be 0 when HOP disabled */
    uint32_t PROD_DMA_COUNT[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer count registers */
} CSL_LdcHtsStatusRead;

/**
 * \brief Holds VPAC HTS configuration registers for LDC (VPAC7).
 *        Used for configuration register readback safety mechanism.
 */
typedef struct
{
    uint32_t PIPELINE_CONTROL[CSL_HTS_NUM_LDC_PIPELINES];
    /**< Pipeline control register */
    uint32_t SCHEDULER_CONTROL[CSL_HTS_NUM_LDC_PIPELINES];
    /**< HWA scheduler control register */
    uint32_t HOP[CSL_HTS_NUM_LDC_PIPELINES];
    /**< HWA scheduler HOP register */
    uint32_t WDTIMER[CSL_HTS_NUM_LDC_PIPELINES];
    /**< HWA scheduler watchdog timer register */
    uint32_t BW_LIMITER[CSL_HTS_NUM_LDC_PIPELINES];
    /**< HWA scheduler bandwidth limiter register */
    uint32_t CONS_CONTROL[CSL_HTS_NUM_LDC_CONSUMERS];
    /**< HWA consumer control registers */

    uint32_t PROD_CONTROL[CSL_HTS_NUM_LDC_PRODUCERS];
    /**< HWA producer control registers */
    uint32_t PROD_BUF_CONTROL[CSL_HTS_NUM_LDC_PRODUCERS];
    /**< HWA producer buffer control registers */
    uint32_t PROD_COUNT[CSL_HTS_NUM_LDC_PRODUCERS];
    /**< HWA producer count registers */
    uint32_t PA_CONTROL[CSL_HTS_NUM_LDC_PRODUCERS];
    /**< Pattern Adapter control registers */
    uint32_t PA_PRODCOUNT[CSL_HTS_NUM_LDC_PRODUCERS];
    /**< Pattern Adapter producer count registers */

    uint32_t PROD_DMA_SCHEDULER_CONTROL[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer scheduler control registers */
    uint32_t PROD_DMA_HOP[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer HOP registers */
    uint32_t PROD_CONTROL_DMA[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer control registers */
    uint32_t PROD_BUF_CONTROL_DMA[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer buffer control registers */
    uint32_t PROD_COUNT_DMA[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer count registers */
    uint32_t PA_CONTROL_DMA[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer ping-pong address control registers */
    uint32_t PA_PRODCOUNT_DMA[CSL_HTS_NUM_LDC_PRODUCERS_DMA];
    /**< DMA producer ping-pong address producer count registers */

    uint32_t CONS_DMA_SCHEDULER_CONTROL[CSL_HTS_NUM_LDC_CONSUMERS_DMA];
    /**< DMA consumer scheduler control registers */
    uint32_t CONS_CONTROL_DMA[CSL_HTS_NUM_LDC_CONSUMERS_DMA];
    /**< DMA consumer control registers */
} CSL_LdcHtsConfigReg;

typedef struct {
    uint32_t HWA_HOP[CSL_HTS_NUM_VISS_PIPELINES];
    /**< HWA scheduler HOP register - should be 0 when HOP disabled */
    uint32_t PROD_COUNT[CSL_HTS_NUM_VISS_PRODUCERS];
    /**< Pattern Adapter producer count registers */
    uint32_t PROD_DMA_HOP[CSL_HTS_NUM_VISS_PRODUCERS_DMA];
    /**< DMA producer HOP registers - should be 0 when HOP disabled */
    uint32_t PROD_DMA_COUNT[CSL_HTS_NUM_VISS_PRODUCERS_DMA];
    /**< DMA producer count registers */
} CSL_VissHtsStatusRead;

/* ========================================================================== */
/*                           Function Declarations                            */
/* ========================================================================== */

/**
 *  \brief This API is used to validate configuration.
 *
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
int32_t CSL_htsCheckThreadConfig(const CSL_HtsSchConfig *cfg);

/**
 *  \brief This API is used to set the complete configuration HWA scheduler.
 *         It set the configuration for producer/consumer/dma scheduler nodes.
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
int32_t CSL_htsSetThreadConfig(const CSL_htsRegs *htsRegs,
                               const CSL_HtsSchConfig *cfg);

/**
 *  \brief This API is used to set config specific to frame size.
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
int32_t CSL_htsSetThreadUpdateConfig(const CSL_htsRegs *htsRegs,
    const CSL_HtsSchConfig *cfg);

/**
 *  \brief This API is used to enable/start all the producer/consumer/scheduler
 *         nodes for given thread.
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig Used
 *                          to identify complete thread
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
int32_t CSL_htsThreadStart(const CSL_htsRegs *htsRegs,
                           const CSL_HtsSchConfig *cfg);

/**
 *  \brief This API is used to disable/stop the enabled producer/consumer/scheduler
 *         nodes for given thread.
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig Used
 *                          to identify complete thread
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
void CSL_htsThreadStop(const CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg);

/**
 *  \brief This API is used to disable/stop all the producer/consumer/scheduler
 *         nodes for given thread.
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig Used
 *                          to identify complete thread
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
int32_t CSL_htsThreadStopAll(const CSL_htsRegs *htsRegs,
                             const CSL_HtsSchConfig *cfg);

/**
 *  \brief This API is used to start the pipeline. This should be called
 *         at the last after configuring all HWA and starting all threads
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig Used
 *                          to identify complete thread
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
int32_t CSL_htsPipelineStart(CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg);

/**
 *  \brief This API is used to stop the pipeline
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig Used
 *                          to identify complete thread
 *                          This pointer must not be null
 *
 *  \return                 returns FVID2_SOK on success else returns
 *                          error value
 */
int32_t CSL_htsPipelineStop(CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg);

/**
 *  \brief This function should be used to initialize HTS scheduler config
 *         structure
 *
 *  \param cfg      Pointer to #CSL_HtsSchConfig
 *  \return         None
 */
static inline void CSL_htsSchConfigInit(CSL_HtsSchConfig *cfg);

/**
 *  \brief This function to get the status of the pipeline
 *
 *  \param htsRegs          Pointer to structure containing HTS Base Address
 *  \param cfg              Pointer to structure of type #CSL_HtsSchConfig Used
 *                          used to get the pipeline id.
 *
 *  \return                 1 if pipeline is running
 *                          0 if pipeline is not running
 */
uint32_t CSL_htsPipelineStatus(const CSL_htsRegs *htsRegs,
                               const CSL_HtsSchConfig *cfg);

const CSL_HtsHwaParams *GetVpacHtsParams(uint32_t schId);

void CSL_htsDisableWatchdogTimer(const CSL_htsRegs *htsRegs, const CSL_HtsSchConfig *cfg);

/* ========================================================================== */
/*                           Function Definition                              */
/* ========================================================================== */

static inline void CSL_htsSchConfigInit(CSL_HtsSchConfig *cfg)
{
    uint32_t cnt = 0;

    if (NULL != cfg)
    {
        (void)memset(cfg, 0, sizeof(CSL_HtsSchConfig));

        for (cnt = 0; cnt < CSL_HTS_MAX_PRODUCER; cnt++)
        {
            cfg->dmaConsCfg[cnt].bypass = UFALSE;
        }
        for (cnt = 0; cnt < CSL_HTS_MAX_CONSUMER; cnt++)
        {
            cfg->dmaProdCfg[cnt].bypass = UFALSE;
        }
    }

}

#ifdef __cplusplus
}
#endif

#endif  /* CSL_HTS_H_ */
