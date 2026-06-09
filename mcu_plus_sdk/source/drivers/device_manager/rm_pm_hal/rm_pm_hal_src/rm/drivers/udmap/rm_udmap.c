/*
 * UDMAP
 *
 * UDMAP management infrastructure
 *
 * Copyright (C) 2018-2025, Texas Instruments Incorporated
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

#include <types/sbool.h>
#include <types/short_types.h>
#include <types/address_types.h>
#include <types/errno.h>
#include <lib/io.h>
#include <lib/trace.h>
#include <hosts_internal.h>

#include <config.h>

#include <osal/osal_core.h>
#include <tisci/rm/tisci_rm_udmap.h>
#include <rm.h>

#include <rm_core.h>
#include <rm_udmap.h>
#include <rm_ra.h>

#include <udmap_inst.h>
#include <udmap_cfg.h>

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
#include <security/rm_int_firewall.h>
#include <tifs/src/security/secure_rm/sec_rm.h>
#endif

/*
 * UDMAP Common Channel Configuration Register Macros
 */
#define UDMAP_CH_BASE(CH)                          (((u32) CH) * ((u32) 0x100U))
#define UDMAP_CH_RT_BASE(CH)                       ((CH) * 0x1000u)
#define UDMAP_FLOW_BASE(FL)                        ((FL) * 0x40u)

/*
 * UDMAP Global Configuration Register Macros
 */
#define UDMAP_GCFG_REVISION                        (0x00000000U)
#define UDMAP_GCFG_PERF_CTRL                       (0x00000004U)
#define UDMAP_GCFG_EMU_CTRL                        (0x00000008U)
#define UDMAP_GCFG_PSIL_TO                         (0x00000010U)
#define UDMAP_GCFG_UTC_CTRL                        (0x0000001CU)
#define UDMAP_GCFG_CAP0                            (0x00000020U)
#define UDMAP_GCFG_CAP1                            (0x00000024U)
#define UDMAP_GCFG_CAP2                            (0x00000028U)
#define UDMAP_GCFG_CAP3                            (0x0000002CU)
#define UDMAP_GCFG_RFLOWFWOES                      (0x00000080U)
#define UDMAP_GCFG_RFLOWFWSTAT                     (0x00000088U)

/*
 * UDMAP TX Channel Configuration Register Macros
 */
#define UDMAP_TCHAN_TCFG                           (0x00000000u)
#define UDMAP_TCHAN_TCREDIT                        (0x00000004u)
#define UDMAP_TCHAN_TCQ                            (0x00000014u)
#define UDMAP_TCHAN_TOES                           (0x00000020u)
#define UDMAP_TCHAN_TEOES                          (0x00000060u)
#define UDMAP_TCHAN_TPRI_CTRL                      (0x00000064u)
#define UDMAP_TCHAN_THRD_ID                        (0x00000068u)
#define UDMAP_TCHAN_TFIFO_DEPTH                    (0x00000070u)
#define UDMAP_TCHAN_TST_SCHED                      (0x00000080u)

/*
 * UDMAP RX Channel Configuration Register Macros
 */
#define UDMAP_RCHAN_RCFG                           (0x00000000u)
#define UDMAP_RCHAN_RCQ                            (0x00000014u)
#define UDMAP_RCHAN_ROES                           (0x00000020u)
#define UDMAP_RCHAN_REOES                          (0x00000060u)
#define UDMAP_RCHAN_RPRI_CTRL                      (0x00000064u)
#define UDMAP_RCHAN_THRD_ID                        (0x00000068u)
#define UDMAP_RCHAN_RST_SCHED                      (0x00000080u)
#define UDMAP_RCHAN_RFLOW_RNG                      (0x000000F0u)

/*
 * UDMAP RX Flow Configuration Register Macros
 */
#define UDMAP_RFLOW_RFA                            (0x00000000u)
#define UDMAP_RFLOW_RFB                            (0x00000004u)
#define UDMAP_RFLOW_RFC                            (0x00000008u)
#define UDMAP_RFLOW_RFD                            (0x0000000Cu)
#define UDMAP_RFLOW_RFE                            (0x00000010u)
#define UDMAP_RFLOW_RFF                            (0x00000014u)
#define UDMAP_RFLOW_RFG                            (0x00000018u)
#define UDMAP_RFLOW_RFH                            (0x0000001Cu)

/*
 * UDMAP Global Configuration Register Fields
 */
#define UDMAP_GCFG_UTC_CTRL_UTC_CH_START_MASK      (0x0000FFFFU)
#define UDMAP_GCFG_UTC_CTRL_UTC_CH_START_SHIFT     (0x00000000U)

/*
 * UDMAP TX Channel Configuration Register Fields
 */
#define UDMAP_TCHAN_TCFG_PAUSE_ON_ERR_MASK         (0x80000000u)
#define UDMAP_TCHAN_TCFG_PAUSE_ON_ERR_SHIFT        (0x0000001Fu)
#define UDMAP_TCHAN_TCFG_FILT_EINFO_MASK           (0x40000000u)
#define UDMAP_TCHAN_TCFG_FILT_EINFO_SHIFT          (0x0000001Eu)
#define UDMAP_TCHAN_TCFG_FILT_PSWORDS_MASK         (0x20000000u)
#define UDMAP_TCHAN_TCFG_FILT_PSWORDS_SHIFT        (0x0000001Du)
#define UDMAP_TCHAN_TCFG_ATYPE_MASK                (0x03000000u)
#define UDMAP_TCHAN_TCFG_ATYPE_SHIFT               (0x00000018u)
#define UDMAP_TCHAN_TCFG_CHAN_TYPE_MASK            (0x000F0000u)
#define UDMAP_TCHAN_TCFG_CHAN_TYPE_SHIFT           (0x00000010u)
#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
#define UDMAP_TCHAN_TCFG_BURST_SIZE_MASK           (0x00000C00U)
#define UDMAP_TCHAN_TCFG_BURST_SIZE_SHIFT          (0x0000000AU)
#endif
#ifdef CONFIG_UDMAP_TX_CHANNEL_TEARDOWN_TYPE
#define UDMAP_TCHAN_TCFG_TDTYPE_MASK               (0x00000200U)
#define UDMAP_TCHAN_TCFG_TDTYPE_SHIFT              (0x00000009U)
#endif
#define UDMAP_TCHAN_TCFG_SUPR_TDPKT_MASK           (0x00000100u)
#define UDMAP_TCHAN_TCFG_SUPR_TDPKT_SHIFT          (0x00000008u)
#define UDMAP_TCHAN_TCFG_FETCH_SIZE_MASK           (0x0000007Fu)
#define UDMAP_TCHAN_TCFG_FETCH_SIZE_SHIFT          (0x00000000u)

#define UDMAP_TCHAN_TCREDIT_COUNT_MASK             (0x00000007u)
#define UDMAP_TCHAN_TCREDIT_COUNT_SHIFT            (0x00000000u)

#define UDMAP_TCHAN_TCQ_TXCQ_QNUM_MASK             (0x0000FFFFu)
#define UDMAP_TCHAN_TCQ_TXCQ_QNUM_SHIFT            (0x00000000u)

#define UDMAP_TCHAN_TPRI_CTRL_PRIORITY_MASK        (0x70000000u)
#define UDMAP_TCHAN_TPRI_CTRL_PRIORITY_SHIFT       (0x0000001Cu)
#define UDMAP_TCHAN_TPRI_CTRL_QOS_MASK             (0x00070000u)
#define UDMAP_TCHAN_TPRI_CTRL_QOS_SHIFT            (0x00000010u)
#define UDMAP_TCHAN_TPRI_CTRL_ORDERID_MASK         (0x0000000Fu)
#define UDMAP_TCHAN_TPRI_CTRL_ORDERID_SHIFT        (0x00000000u)

#define UDMAP_TCHAN_THRD_ID_THREAD_ID_MASK         (0x0000FFFFu)
#define UDMAP_TCHAN_THRD_ID_THREAD_ID_SHIFT        (0x00000000u)

#define UDMAP_TCHAN_TFIFO_DEPTH_FDEPTH_MASK        (0x0000FFFFu)
#define UDMAP_TCHAN_TFIFO_DEPTH_FDEPTH_SHIFT       (0x00000000u)

#define UDMAP_TCHAN_TST_SCHED_PRIORITY_MASK        (0x00000003u)
#define UDMAP_TCHAN_TST_SCHED_PRIORITY_SHIFT       (0x00000000u)

/*
 * UDMAP RX Channel Configuration Register Fields
 */
#define UDMAP_RCHAN_RCFG_PAUSE_ON_ERR_MASK         (0x80000000u)
#define UDMAP_RCHAN_RCFG_PAUSE_ON_ERR_SHIFT        (0x0000001Fu)
#define UDMAP_RCHAN_RCFG_ATYPE_MASK                (0x03000000u)
#define UDMAP_RCHAN_RCFG_ATYPE_SHIFT               (0x00000018u)
#define UDMAP_RCHAN_RCFG_CHAN_TYPE_MASK            (0x000F0000u)
#define UDMAP_RCHAN_RCFG_CHAN_TYPE_SHIFT           (0x00000010u)
#define UDMAP_RCHAN_RCFG_RX_IGNORE_SHORT_MASK      (0x00008000u)
#define UDMAP_RCHAN_RCFG_RX_IGNORE_SHORT_SHIFT     (0x0000000Fu)
#define UDMAP_RCHAN_RCFG_RX_IGNORE_LONG_MASK       (0x00004000u)
#define UDMAP_RCHAN_RCFG_RX_IGNORE_LONG_SHIFT      (0x0000000Eu)
#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
#define UDMAP_TCHAN_TCFG_BURST_SIZE_RESERVED       (0x0U)
#define UDMAP_RCHAN_RCFG_BURST_SIZE_MASK           (0x00000C00U)
#define UDMAP_RCHAN_RCFG_BURST_SIZE_SHIFT          (0x0000000AU)
#endif
#define UDMAP_RCHAN_RCFG_FETCH_SIZE_MASK           (0x0000007Fu)
#define UDMAP_RCHAN_RCFG_FETCH_SIZE_SHIFT          (0x00000000u)

#define UDMAP_RCHAN_RCQ_RXCQ_QNUM_MASK             (0x0000FFFFu)
#define UDMAP_RCHAN_RCQ_RXCQ_QNUM_SHIFT            (0x00000000u)

#define UDMAP_RCHAN_RPRI_CTRL_PRIORITY_MASK        (0x70000000u)
#define UDMAP_RCHAN_RPRI_CTRL_PRIORITY_SHIFT       (0x0000001Cu)
#define UDMAP_RCHAN_RPRI_CTRL_QOS_MASK             (0x00070000u)
#define UDMAP_RCHAN_RPRI_CTRL_QOS_SHIFT            (0x00000010u)
#define UDMAP_RCHAN_RPRI_CTRL_ORDERID_MASK         (0x0000000Fu)
#define UDMAP_RCHAN_RPRI_CTRL_ORDERID_SHIFT        (0x00000000u)

#define UDMAP_RCHAN_THRD_ID_THREAD_ID_MASK         (0x0000FFFFu)
#define UDMAP_RCHAN_THRD_ID_THREAD_ID_SHIFT        (0x00000000u)

#define UDMAP_RCHAN_RST_SCHED_PRIORITY_MASK        (0x00000003u)
#define UDMAP_RCHAN_RST_SCHED_PRIORITY_SHIFT       (0x00000000u)

#define UDMAP_RCHAN_RFLOW_RNG_FLOWID_CNT_MASK      (0x7FFF0000u)
#define UDMAP_RCHAN_RFLOW_RNG_FLOWID_CNT_SHIFT     (0x00000010u)
#define UDMAP_RCHAN_RFLOW_RNG_FLOWID_START_MASK    (0x00003FFFu)
#define UDMAP_RCHAN_RFLOW_RNG_FLOWID_START_SHIFT   (0x00000000u)

/*
 * UDMAP TX & RX Channel Real-Time Configuration Register Fields
 */
#define UDMAP_CHANRT_RT_CTL_ENABLE_MASK            (0x80000000u)
#define UDMAP_CHANRT_RT_CTL_ENABLE_SHIFT           (0x0000001Fu)
#define UDMAP_CHANRT_RT_CTL_ENABLE_ENABLED         (0x00000001u)
#define UDMAP_CHANRT_RT_CTL_TEARDOWN_MASK          (0x40000000u)
#define UDMAP_CHANRT_RT_CTL_TEARDOWN_SHIFT         (0x0000001Eu)
#define UDMAP_CHANRT_RT_CTL_TEARDOWN_ENABLE        (0x00000001u)
#define UDMAP_CHANRT_RT_CTL_FTEARDOWN_MASK         (0x10000000u)
#define UDMAP_CHANRT_RT_CTL_FTEARDOWN_SHIFT        (0x0000001Cu)
#define UDMAP_CHANRT_RT_CTL_FTEARDOWN_ENABLE       (0x00000001u)

/*
 * UDMAP RX Flow Configuration Register Fields
 */
#define UDMAP_RFLOW_RFA_RX_EINFO_PRESENT_MASK     (0x40000000u)
#define UDMAP_RFLOW_RFA_RX_EINFO_PRESENT_SHIFT    (0x0000001Eu)
#define UDMAP_RFLOW_RFA_RX_PSINFO_PRESENT_MASK    (0x20000000u)
#define UDMAP_RFLOW_RFA_RX_PSINFO_PRESENT_SHIFT   (0x0000001Du)
#define UDMAP_RFLOW_RFA_RX_ERROR_HANDLING_MASK    (0x10000000u)
#define UDMAP_RFLOW_RFA_RX_ERROR_HANDLING_SHIFT   (0x0000001Cu)
#define UDMAP_RFLOW_RFA_RX_DESC_TYPE_MASK         (0x0C000000u)
#define UDMAP_RFLOW_RFA_RX_DESC_TYPE_SHIFT        (0x0000001Au)
#define UDMAP_RFLOW_RFA_RX_PS_LOCATION_MASK       (0x02000000u)
#define UDMAP_RFLOW_RFA_RX_PS_LOCATION_SHIFT      (0x00000019u)
#define UDMAP_RFLOW_RFA_RX_SOP_OFFSET_MASK        (0x01FF0000u)
#define UDMAP_RFLOW_RFA_RX_SOP_OFFSET_SHIFT       (0x00000010u)
#define UDMAP_RFLOW_RFA_RX_DEST_QNUM_MASK         (0x0000FFFFu)
#define UDMAP_RFLOW_RFA_RX_DEST_QNUM_SHIFT        (0x00000000u)

#define UDMAP_RFLOW_RFB_RX_SRC_TAG_HI_MASK        (0xFF000000u)
#define UDMAP_RFLOW_RFB_RX_SRC_TAG_HI_SHIFT       (0x00000018u)
#define UDMAP_RFLOW_RFB_RX_SRC_TAG_LO_MASK        (0x00FF0000u)
#define UDMAP_RFLOW_RFB_RX_SRC_TAG_LO_SHIFT       (0x00000010u)
#define UDMAP_RFLOW_RFB_RX_DEST_TAG_HI_MASK       (0x0000FF00u)
#define UDMAP_RFLOW_RFB_RX_DEST_TAG_HI_SHIFT      (0x00000008u)
#define UDMAP_RFLOW_RFB_RX_DEST_TAG_LO_MASK       (0x000000FFu)
#define UDMAP_RFLOW_RFB_RX_DEST_TAG_LO_SHIFT      (0x00000000u)

#define UDMAP_RFLOW_RFC_RX_SRC_TAG_HI_SEL_MASK    (0x70000000u)
#define UDMAP_RFLOW_RFC_RX_SRC_TAG_HI_SEL_SHIFT   (0x0000001Cu)
#define UDMAP_RFLOW_RFC_RX_SRC_TAG_LO_SEL_MASK    (0x07000000u)
#define UDMAP_RFLOW_RFC_RX_SRC_TAG_LO_SEL_SHIFT   (0x00000018u)
#define UDMAP_RFLOW_RFC_RX_DEST_TAG_HI_SEL_MASK   (0x00700000u)
#define UDMAP_RFLOW_RFC_RX_DEST_TAG_HI_SEL_SHIFT  (0x00000014u)
#define UDMAP_RFLOW_RFC_RX_DEST_TAG_LO_SEL_MASK   (0x00070000u)
#define UDMAP_RFLOW_RFC_RX_DEST_TAG_LO_SEL_SHIFT  (0x00000010u)
#define UDMAP_RFLOW_RFC_RX_SIZE_THRESH_EN_MASK    (0x00000007u)
#define UDMAP_RFLOW_RFC_RX_SIZE_THRESH_EN_SHIFT   (0x00000000u)

#define UDMAP_RFLOW_RFD_RX_FDQ0_SZ0_QNUM_MASK     (0xFFFF0000u)
#define UDMAP_RFLOW_RFD_RX_FDQ0_SZ0_QNUM_SHIFT    (0x00000010u)
#define UDMAP_RFLOW_RFD_RX_FDQ1_QNUM_MASK         (0x0000FFFFu)
#define UDMAP_RFLOW_RFD_RX_FDQ1_QNUM_SHIFT        (0x00000000u)

#define UDMAP_RFLOW_RFE_RX_FDQ2_QNUM_MASK         (0xFFFF0000u)
#define UDMAP_RFLOW_RFE_RX_FDQ2_QNUM_SHIFT        (0x00000010u)
#define UDMAP_RFLOW_RFE_RX_FDQ3_QNUM_MASK         (0x0000FFFFu)
#define UDMAP_RFLOW_RFE_RX_FDQ3_QNUM_SHIFT        (0x00000000u)

#define UDMAP_RFLOW_RFF_RX_SIZE_THRESH0_MASK      (0xFFFF0000u)
#define UDMAP_RFLOW_RFF_RX_SIZE_THRESH0_SHIFT     (0x00000010u)
#define UDMAP_RFLOW_RFF_RX_SIZE_THRESH1_MASK      (0x0000FFFFu)
#define UDMAP_RFLOW_RFF_RX_SIZE_THRESH1_SHIFT     (0x00000000u)

#define UDMAP_RFLOW_RFG_RX_SIZE_THRESH2_MASK      (0xFFFF0000u)
#define UDMAP_RFLOW_RFG_RX_SIZE_THRESH2_SHIFT     (0x00000010u)
#define UDMAP_RFLOW_RFG_RX_FDQ0_SZ1_QNUM_MASK     (0x0000FFFFu)
#define UDMAP_RFLOW_RFG_RX_FDQ0_SZ1_QNUM_SHIFT    (0x00000000u)

#define UDMAP_RFLOW_RFH_RX_FDQ0_SZ2_QNUM_MASK     (0xFFFF0000u)
#define UDMAP_RFLOW_RFH_RX_FDQ0_SZ2_QNUM_SHIFT    (0x00000010u)
#define UDMAP_RFLOW_RFH_RX_FDQ0_SZ3_QNUM_MASK     (0x0000FFFFu)
#define UDMAP_RFLOW_RFH_RX_FDQ0_SZ3_QNUM_SHIFT    (0x00000000u)

#define UDMAP_GENERIC_EVENT_EVT_MASK              (0x0000FFFFu)
#define UDMAP_GENERIC_EVENT_EVT_SHIFT             (0x00000000u)
#define UDMAP_GENERIC_EVENT_EVT_RESETVAL          (0x0000FFFFu)

#define UDMAP_NULL_CH_INDEX                       (0xFFFFu)

/*
 * Device masks for each channel type
 */
#ifdef CONFIG_UDMAP_UDMA
#define UDMAP_GCFG_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_PERF_CTRL_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_EMU_CTRL_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_PSIL_TO_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_RFLOWFWSTAT_VALID	\
		)

#define UDMAP_TX_CHAN_DEVMASK (	\
		TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_QOS_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_EINFO_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_PSWORDS_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_CREDIT_COUNT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FDEPTH_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_TDTYPE_VALID \
		)

#define UDMAP_RX_CHAN_DEVMASK (	\
		TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_QOS_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_SHORT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_LONG_VALID  \
		)

#define UDMAP_UDMA_RX_FLOW_CFG_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_EINFO_PRESENT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_PSINFO_PRESENT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_ERROR_HANDLING_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_DESC_TYPE_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SOP_OFFSET_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_QNUM_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_SEL_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_SEL_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_SEL_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_SEL_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ0_QNUM_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ1_QNUM_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ2_QNUM_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ3_QNUM_VALID |	\
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_PS_LOCATION_VALID	\
		)

#define UDMAP_UDMA_RX_FLOW_SIZE_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH0_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH1_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH2_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ1_QNUM_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ2_QNUM_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ3_QNUM_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH_EN_VALID \
		)
#else
#define UDMAP_GCFG_DEVMASK (0U)
#define UDMAP_TX_CHAN_DEVMASK (0U)
#define UDMAP_RX_CHAN_DEVMASK (0U)
#define UDMAP_UDMA_RX_FLOW_CFG_DEVMASK (0U)
#define UDMAP_UDMA_RX_FLOW_SIZE_DEVMASK (0U)
#endif

#ifdef CONFIG_UDMAP_BCDMA
#define UDMAP_BCDMA_GCFG_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_EMU_CTRL_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_PSIL_TO_VALID \
		)

#define UDMAP_BCDMA_TX_CHAN_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FDEPTH_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_TDTYPE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_EXTENDED_CH_TYPE_VALID	\
		)

#define UDMAP_BCDMA_RX_CHAN_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_LONG_VALID \
		)

#define UDMAP_BCDMA_CHAN_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_EXTENDED_CH_TYPE_VALID	\
		)
#else
#define UDMAP_BCDMA_GCFG_DEVMASK (0U)
#define UDMAP_BCDMA_TX_CHAN_DEVMASK (0U)
#define UDMAP_BCDMA_RX_CHAN_DEVMASK (0U)
#define UDMAP_BCDMA_CHAN_DEVMASK (0U)
#endif

#ifdef CONFIG_UDMAP_PKTDMA
#define UDMAP_PKTDMA_GCFG_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_EMU_CTRL_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_GCFG_PSIL_TO_VALID \
		)

#define UDMAP_PKTDMA_TX_CHAN_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_EINFO_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_PSWORDS_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FDEPTH_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_TX_TDTYPE_VALID \
		)

#define UDMAP_PKTDMA_RX_CHAN_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID \
		)

#define UDMAP_PKTDMA_RX_FLOW_CFG_DEVMASK ( \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_EINFO_PRESENT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_PSINFO_PRESENT_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_ERROR_HANDLING_VALID | \
		TISCI_MSG_VALUE_RM_UDMAP_FLOW_SOP_OFFSET_VALID \
		)

#define UDMAP_PKTDMA_RX_FLOW_SIZE_DEVMASK (0U)
#else
#define UDMAP_PKTDMA_GCFG_DEVMASK (0U)
#define UDMAP_PKTDMA_TX_CHAN_DEVMASK (0U)
#define UDMAP_PKTDMA_RX_CHAN_DEVMASK (0U)
#define UDMAP_PKTDMA_RX_FLOW_CFG_DEVMASK (0U)
#define UDMAP_PKTDMA_RX_FLOW_SIZE_DEVMASK (0U)
#endif

/**
 * \brief Structure for local data for RM RA valid param selection per ring type
 *
 * \param chan_mask Bitwise OR of all TISCI_MSG_VALUE_RM_UDMAP_CH_*_VALID
 *                  supported
 *
 * \param flow_cfg_mask Bitwise OR of all TISCI_MSG_VALUE_RM_UDMAP_FLOW_*_VALID
 *                      supported
 *
 * \param flow_size_mask Bitwise OR of all
 *                       TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_*_VALID supported
 */
struct udmap_valid_masks_local {
	u32	chan_mask;
	u32	flow_cfg_mask;
	u32	flow_size_mask;
};

/**
 * \brief Local list of valid param mask per channel type.
 */
static const struct udmap_valid_masks_local local_rm_udmap_ch_valid_masks[UDMAP_NUM_CHAN_TYPES] = {
	[UDMAP_TX_CHAN] =			 {
		UDMAP_TX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[UDMAP_TX_HCHAN] =			 {
		UDMAP_TX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[UDMAP_TX_UHCHAN] =			 {
		UDMAP_TX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[UDMAP_TX_ECHAN] =			 {
		UDMAP_TX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[UDMAP_RX_CHAN] =			 {
		UDMAP_RX_CHAN_DEVMASK,
		UDMAP_UDMA_RX_FLOW_CFG_DEVMASK,
		UDMAP_UDMA_RX_FLOW_SIZE_DEVMASK,
	},
	[UDMAP_RX_HCHAN] =			 {
		UDMAP_RX_CHAN_DEVMASK,
		UDMAP_UDMA_RX_FLOW_CFG_DEVMASK,
		UDMAP_UDMA_RX_FLOW_SIZE_DEVMASK,
	},
	[UDMAP_RX_UHCHAN] =			 {
		UDMAP_RX_CHAN_DEVMASK,
		UDMAP_UDMA_RX_FLOW_CFG_DEVMASK,
		UDMAP_UDMA_RX_FLOW_SIZE_DEVMASK,
	},
	[DMSS_BCDMA_TX_CHAN] =			 {
		UDMAP_BCDMA_TX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[DMSS_BCDMA_RX_CHAN] =			 {
		UDMAP_BCDMA_RX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[DMSS_BCDMA_BLOCK_COPY_CHAN] =		 {
		UDMAP_BCDMA_CHAN_DEVMASK,
		0U,
		0U,
	},
	[DMSS_PKTDMA_TX_CHAN] =			 {
		UDMAP_PKTDMA_TX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[DMSS_PKTDMA_RX_CHAN] =			 {
		UDMAP_PKTDMA_RX_CHAN_DEVMASK,
		UDMAP_PKTDMA_RX_FLOW_CFG_DEVMASK,
		UDMAP_PKTDMA_RX_FLOW_SIZE_DEVMASK,
	},
	[DMSS_BCDMA_BLOCK_COPY_HCCHAN] =	 {
		UDMAP_BCDMA_CHAN_DEVMASK,
		0U,
		0U,
	},
	[DMSS_BCDMA_TX_HCCHAN] =		 {
		UDMAP_BCDMA_TX_CHAN_DEVMASK,
		0U,
		0U,
	},
	[DMSS_BCDMA_RX_HCCHAN] =		 {
		UDMAP_BCDMA_RX_CHAN_DEVMASK,
		0U,
		0U,
	},
};

/**
 * \brief Local list of valid param mask per DMA GCFG region
 */
static const u32 local_udmap_gcfg_valid_masks[UDMAP_NUM_GCFG_TYPES] = {
	[UDMAP_GCFG] = UDMAP_GCFG_DEVMASK,
	[BCDMA_GCFG] = UDMAP_BCDMA_GCFG_DEVMASK,
	[PKTDMA_GCFG] = UDMAP_PKTDMA_GCFG_DEVMASK,
};

/**
 * \brief Get UDMAP instance mapped to provided device ID
 *
 * \param id UDMAP device ID
 *
 * \param trace_action RA action to trace
 *
 * \return Pointer to UDMAP instance, NULL if no instance mapped to ID
 */
static const struct udmap_instance *udmap_get_inst(u16 id, u8 trace_action)
{
	const struct udmap_instance *inst = NULL;
	u8 i;
	u8 trace_action_val_p = trace_action;

	for (i = 0U; i < udmap_inst_count; i++) {
		if (udmap_inst[i].id == id) {
			inst = &udmap_inst[i];

			if (rm_core_validate_devgrp(inst->id, inst->devgrp) !=
			    SUCCESS) {
				/*
				 * Return NULL instance if instance devgrp
				 * doesn't align with boardcfg devgrp
				 */
				inst = NULL;
			}
			break;
		}
	}

	if (inst == NULL) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_DEVICE_ID,
		     id);

	return inst;
}

/**
 * \brief Validate UDMAP channel index against the board cfg resasg data
 *
 * \param inst UDMAP instance
 *
 * \param host Host ID
 *
 * \param index UDMAP tx or rx ch index
 *
 * \param utype Returns the channel utype if not NULL
 *
 * \param chan_type Channel type to determine dev_mask
 *
 * \param tx_ch Transmit channel if STRUE, Receive channel if SFALSE
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if ring index is valid, else -EINVAL
 */
static s32 udmap_check_index_range(const struct udmap_instance *inst, u8 host,
				   u16 index, u16 *utype, u8 *chan_type,
				   sbool tx_ch, u8 trace_action, u8 extended_ch_type)
{
	s32 r = -EINVAL;
	u8 n_ch_type = ((tx_ch == STRUE) ? inst->n_tx_ch_type :
			inst->n_rx_ch_type);
	const struct udmap_ch_type *ch_types = ((tx_ch == STRUE) ?
						inst->tx_ch_types :
						inst->rx_ch_types);
	u8 i;
	u16 loc_index = index;
	u8 trace_action_val_p = trace_action;

	if ((inst->bc_ch_types != NULL) && (tx_ch == STRUE) &&
	    (extended_ch_type == (u8) 1)) {
		/* Use block copy channel types array and the number of channel types*/
		ch_types = inst->bc_ch_types;
		n_ch_type = inst->n_bc_ch_type;
	}

	for (i = 0U; i < n_ch_type; i++) {
		/* Search for index among all ch type index ranges */
		if ((loc_index >= ch_types[i].start) &&
		    (loc_index <= ch_types[i].end)) {
			r = rm_core_resasg_validate_resource(
				host,
				ch_types[i].utype,
				loc_index);
			break;
		}
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	} else {
		if (utype != NULL) {
			*utype = ch_types[i].utype;
		}
		if (chan_type != NULL) {
			*chan_type = ch_types[i].type;
		}
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_INDEX,
		     index);

	return r;
}

/**
 * \brief Get the resource assignment type corresponding to a particular resource
 *
 * \param inst UDMAP instance
 *
 * \param index UDMAP tx or rx ch index
 *
 * \param utype Returns the channel utype if not NULL
 *
 * \param tx_ch Transmit channel if STRUE, Receive channel if SFALSE
 *
 * \return SUCCESS if resource type is found, else -EINVAL
 */
#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
static s32 udmap_get_type(const struct udmap_instance *inst,
			  u16 index, u16 *utype,
			  sbool tx_ch, u8 extended_ch_type)
{
	s32 r = -EINVAL;
	u8 n_ch_type = ((tx_ch == STRUE) ? inst->n_tx_ch_type :
			inst->n_rx_ch_type);
	const struct udmap_ch_type *ch_types = ((tx_ch == STRUE) ?
						inst->tx_ch_types :
						inst->rx_ch_types);
	u8 i;
	u16 loc_index = index;

	if ((inst->bc_ch_types != NULL) && (tx_ch == STRUE) &&
	    (extended_ch_type == (u8) 1)) {
		/* Use block copy channel types array */
		ch_types = inst->bc_ch_types;
	}

	for (i = 0U; i < n_ch_type; i++) {
		/* Search for index among all ch type index ranges */
		if ((loc_index >= ch_types[i].start) &&
		    (loc_index <= ch_types[i].end)) {
			r = SUCCESS;
			break;
		}
	}

	if (r == SUCCESS) {
		if (utype != NULL) {
			*utype = ch_types[i].utype;
		}
	}

	return r;
}

/**
 * \brief Get the host ID corresponding to a particular DMA channel
 *
 * \param id Device ID
 *
 * \param index UDMAP tx or rx ch index
 *
 * \param tx_ch Transmit channel if STRUE, Receive channel if SFALSE
 *
 * \param n_hosts Pointer to a variable which is populated with the number
 *                of hosts assigned to the resource in the RM board
 *                configuration
 *
 * \param host_array Pointer to a array variable which is populated with the
 *                   hosts assigned to the resource in the RM board
 *                   configuration
 *
 * \param host_array_max Maximum number of elements in the array pointed to by
 *                       host_array
 *
 * \return SUCCESS or -EINVAL depending if valid or not
 */
s32 udmap_get_host(u16 id, u16 index, sbool tx_ch, u8 *n_hosts, u8 *hosts_array, u8 host_array_max)
{
	s32 r = SUCCESS;
	const struct udmap_instance *inst = NULL;
	u16 utype;
	u8 trace_action = (tx_ch == STRUE) ?
			  TRACE_RM_ACTION_UDMAP_TX_CH_CFG :
			  TRACE_RM_ACTION_UDMAP_RX_CH_CFG;

	inst = udmap_get_inst(id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		if (tx_ch) {
			if (index >= inst->n_tx_ch) {
				trace_action |= TRACE_RM_ACTION_FAIL;
				r = -EINVAL;
			}
		} else {
			if (index >= inst->n_rx_ch) {
				trace_action |= TRACE_RM_ACTION_FAIL;
				r = -EINVAL;
			}
		}
	}

	/* Get the resasg type */
	if (r == SUCCESS) {
		r = udmap_get_type(inst, index, &utype, tx_ch, 0);
	}

	/* Convert utype to host ID */
	if (r == SUCCESS) {
		r = rm_core_get_resasg_hosts(utype, index, n_hosts,
					     &hosts_array[0U], host_array_max);
	}

	return r;
}
#endif

/**
 * \brief Validate UDMAP receive flow index against the board cfg resasg data
 *
 * \param inst UDMAP instance
 *
 * \param host Host ID
 *
 * \param index UDMAP rx flow index
 *
 * \param cfg Flow index check is for config if STRUE, flow delegation if SFALSE
 *
 * \param assoc_chan_type Channel type tied to flow to determine dev_mask
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if flow index is valid, else -EINVAL
 */
static s32 udmap_check_flow_index(const struct udmap_instance *inst, u8 host,
				  u16 index, sbool cfg, u8 *assoc_chan_type,
				  u8 trace_action)
{
	s32 r = -EINVAL;
	u8 n_ch_type = inst->n_rx_ch_type;
	const struct udmap_ch_type *ch_types = inst->rx_ch_types;
	u8 i;
	sbool ch_flow = SFALSE;
	/*
	 * Showing any type for UDMA common flow results in UDMA flow params
	 * allowed.
	 */
	u8 chan_type = UDMAP_RX_CHAN;
	u8 trace_action_val_p = trace_action;

	/* Rx ch and flow index must be valid in the UDMAP instance */
	if (index < inst->n_rx_flow) {
		/* Attempt to validate against rx ch */
		for (i = 0U; i < n_ch_type; i++) {
			if ((index >= ch_types[i].flow_start) &&
			    (index <= ch_types[i].flow_end)) {
				if (cfg == STRUE) {
					/*
					 * Only perform validation for
					 * statically assigned flows when the
					 * validation request comes from a flow
					 * configuration message.  Flow
					 * delegation messages only apply to
					 * common flows so the validation must
					 * fail out at this point if the
					 * validation request came from a flow
					 * delegation message.
					 */
					r = rm_core_resasg_validate_resource(
						host,
						ch_types[i].flow_utype,
						index);
				}
				ch_flow = STRUE;
				chan_type = ch_types[i].type;
				break;
			}
		}

		if ((ch_flow == SFALSE) &&
		    (inst->common_flow != NULL)) {
			if (cfg == SFALSE) {
				/*
				 * Always validate against board configuration
				 * when the request comes from flow delegation
				 */
				r = rm_core_resasg_validate_resource(
					host,
					inst->common_flow->utype,
					index);
			} else if (host !=
				   inst->common_flow->delegates[index -
								inst->common_flow->start]) {
				/*
				 * Skip validation when request comes from a
				 * flow configuration message and the flow has
				 * been delegated by its original owner
				 */
				r = rm_core_resasg_validate_resource(
					host,
					inst->common_flow->utype,
					index);
			} else {
				/* Do Nothing */
			}
		}
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	} else {
		if (assoc_chan_type != NULL) {
			*assoc_chan_type = chan_type;
		}
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_INDEX,
		     index);

	return r;
}

/**
 * \brief Validate UDMAP channel pause_on_err
 *
 * \param ch_pause_on_err Tx or rx ch pause on error config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if pause_on_err is valid, else -EINVAL
 */
static s32 udmap_validate_ch_pause_on_err(u8 ch_pause_on_err, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((ch_pause_on_err !=
	     TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERROR_DISABLED) &&
	    (ch_pause_on_err !=
	     TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERROR_ENABLED)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_PAUSE_ON_ERR,
		     ch_pause_on_err);

	return r;
}

/**
 * \brief Validate UDMAP transmit channel tx_filt_einfo
 *
 * \param tx_filt_einfo Tx ch filter extended info config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if tx_filt_einfo is valid, else -EINVAL
 */
static s32 udmap_validate_tx_filt_einfo(u8 tx_filt_einfo, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((tx_filt_einfo !=
	     TISCI_MSG_VALUE_RM_UDMAP_TX_CH_FILT_EINFO_DISABLED) &&
	    (tx_filt_einfo !=
	     TISCI_MSG_VALUE_RM_UDMAP_TX_CH_FILT_EINFO_ENABLED)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_TX_CH_FILT_EINFO,
		     tx_filt_einfo);

	return r;
}

/**
 * \brief Validate UDMAP transmit channel tx_filt_pswords
 *
 * \param tx_filt_pswords Tx ch filter protocol specific words config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if tx_filt_pswords is valid, else -EINVAL
 */
static s32 udmap_validate_tx_filt_pswords(u8 tx_filt_pswords, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((tx_filt_pswords !=
	     TISCI_MSG_VALUE_RM_UDMAP_TX_CH_FILT_PSWORDS_DISABLED) &&
	    (tx_filt_pswords !=
	     TISCI_MSG_VALUE_RM_UDMAP_TX_CH_FILT_PSWORDS_ENABLED)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_TX_CH_FILT_PSWORDS,
		     tx_filt_pswords);

	return r;
}

/**
 * \brief Validate UDMAP channel atype
 *
 * \param atype Tx or rx ch address type config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if atype is valid, else -EINVAL
 */
static s32 udmap_validate_atype(u8 atype, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (atype) {
	case TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_PHYS:
		/* Also catches TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_RESET */
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_INTERMEDIATE:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VIRTUAL:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_NON_COHERENT:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_ATYPE,
		     atype);

	return r;
}

/**
 * \brief Validate UDMAP channel functional type
 *
 * \param chan_type Channel functional type config
 *
 * \param tx_ch Transmit channel if STRUE, Receive channel if SFALSE
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if chan_type is valid, else -EINVAL
 */
static s32 udmap_validate_chan_type(u8 chan_type, sbool tx_ch, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (chan_type) {
	case TISCI_MSG_VALUE_RM_UDMAP_CH_TYPE_PACKET:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_TYPE_PACKET_SINGLE_BUF:
		if (tx_ch == STRUE) {
			/* Only valid for rx channels */
			r = -EINVAL;
		}
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_TYPE_3P_DMA_REF:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_TYPE_3P_DMA_VAL:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_TYPE_3P_BLOCK_REF:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_TYPE_3P_BLOCK_VAL:
		break;
	default:
		if (((tx_ch == STRUE) &&
		     (chan_type != 0U)) ||
		    ((tx_ch == SFALSE) &&
		     (chan_type != 0U))) {
			r = -EINVAL;
		}
		break;
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_TYPE,
		     chan_type);

	return r;
}

/**
 * \brief Validate UDMAP transmit channel tx_supr_tdpkt
 *
 * \param tx_supr_tdpkt Tx ch teardown packet suppression config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if tx_supr_tdpkt is valid, else -EINVAL
 */
static s32 udmap_validate_tx_supr_tdpkt(u8 tx_supr_tdpkt, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((tx_supr_tdpkt !=
	     TISCI_MSG_VALUE_RM_UDMAP_TX_CH_SUPPRESS_TD_DISABLED) &&
	    (tx_supr_tdpkt !=
	     TISCI_MSG_VALUE_RM_UDMAP_TX_CH_SUPPRESS_TD_ENABLED)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_TX_CH_SUPR_TDPKT,
		     tx_supr_tdpkt);

	return r;
}

/**
 * \brief Validate UDMAP channel fetch_size
 *
 * \param fetch_size Tx or rx ch descriptor words fetch config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if fetch_size is valid, else -EINVAL
 */
static s32 udmap_validate_ch_fetch_size(u16 fetch_size, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (fetch_size > TISCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_MAX) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_FETCH_SIZE,
		     fetch_size);

	return r;
}

/**
 * \brief Validate UDMAP transmit channel credit count
 *
 * \param chan_type Channel type
 *
 * \param tx_credit_count Tx ch TR credit count config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if tx_credit_count is valid, else -EINVAL
 */
static s32 udmap_validate_tx_credit_count(u8 chan_type, u8 tx_credit_count,
					  u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((chan_type != UDMAP_TX_ECHAN) && (tx_credit_count != 0U)) {
		r = -EINVAL;
	} else {
		if (tx_credit_count >
		    TISCI_MSG_VALUE_RM_UDMAP_TX_CH_CREDIT_CNT_MAX) {
			r = -EINVAL;
		}
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_TX_CH_CREDIT_COUNT,
		     tx_credit_count);

	return r;
}

/**
 * \brief Validate UDMAP transmit channel fdepth
 *
 * \param chan_type Channel type
 *
 * \param fdepth fdepth configuration
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if fdepth is valid, else -EINVAL
 */
static s32 udmap_validate_fdepth(u8 chan_type, u16 fdepth,
				 u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((chan_type == UDMAP_TX_ECHAN) && (fdepth != 0U)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_TX_CH_FDEPTH,
		     fdepth);

	return r;
}

/**
 * \brief Validate UDMAP channel priority
 *
 * \param priority Tx or rx ch transmit priority config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if priority is valid, else -EINVAL
 */
static s32 udmap_validate_priority(u8 priority, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (priority > TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_MAX) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_PRIORITY,
		     priority);

	return r;
}

/**
 * \brief Validate UDMAP channel qos
 *
 * \param qos Tx or rx ch transmit qos config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if qos is valid, else -EINVAL
 */
static s32 udmap_validate_qos(u8 qos, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (qos > TISCI_MSG_VALUE_RM_UDMAP_CH_QOS_MAX) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_QOS,
		     qos);

	return r;
}

/**
 * \brief Validate UDMAP channel orderid
 *
 * \param orderid Tx or rx ch bus order id config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if orderid is valid, else -EINVAL
 */
static s32 udmap_validate_orderid(u8 orderid, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (orderid > TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_MAX) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_ORDERID,
		     orderid);

	return r;
}

/**
 * \brief Validate UDMAP channel scheduling priority
 *
 * \param sched_priority Tx or rx ch scheduling priority config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if sched_priority is valid, else -EINVAL
 */
static s32 udmap_validate_sched_priority(u8 sched_priority, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (sched_priority) {
	case TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_HIGH:
		/*
		 * Catches TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_RESET
		 * as well
		 */
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_MEDHIGH:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_MEDLOW:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIOR_LOW:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_SCHED_PRIORITY,
		     sched_priority);

	return r;
}

/**
 * \brief Validate UDMAP receive channel flowid range
 *
 * \param inst UDMAP instance
 *
 * \param host SoC host ID
 *
 * \param flowid_start Receive ch starting flow ID
 *
 * \param flowid_cnt Receive ch flow ID count starting from flowid_start
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if flow ID range is valid, else -EINVAL
 */
static s32 udmap_validate_rx_flowid_range(const struct udmap_instance *inst,
					  u8 host, u16 flowid_start,
					  u16 flowid_cnt, u8 trace_action)
{
	s32 r;
	u16 range_max = 0u;
	u16 i;
	u8 trace_action_val_p = trace_action;

	if (flowid_cnt == TISCI_MSG_VALUE_RM_UDMAP_RX_CH_FLOW_CNT_NONE) {
		/* Not an error if cnt is zero, which disables flow range */
		r = SUCCESS;
	} else {
		range_max = flowid_start + flowid_cnt - 1u;
		if (range_max > inst->common_flow->end) {
			range_max = inst->common_flow->end;
		}

		r = SUCCESS;
		for (i = flowid_start; i <= range_max; i++) {
			if (i < inst->common_flow->start) {
				/*
				 * Validate against rx channel for statically
				 * mapped flows
				 */
				r = udmap_check_index_range(inst, host, i,
							    NULL, NULL, SFALSE,
							    trace_action_val_p, 0);
			} else {
				r = rm_core_resasg_validate_resource(
					host,
					inst->common_flow->utype,
					i);
			}

			if (r != SUCCESS) {
				break;
			}
		}
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_RX_CH_FLOWID_START,
		     flowid_start);
	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_RX_CH_FLOWID_COUNT,
		     flowid_cnt);

	return r;
}

/**
 * \brief Validate UDMAP receive channel short packet treatment config
 *
 * \param rx_ignore_short UDMAP rx ch ignore short packet config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_ignore_short is valid, else -EINVAL
 */
static s32 udmap_validate_rx_ignore_short(u8 rx_ignore_short, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((rx_ignore_short !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_CH_PACKET_EXCEPTION) &&
	    (rx_ignore_short !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_CH_PACKET_IGNORED)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_RX_CH_IGNORE_SHORT,
		     rx_ignore_short);

	return r;
}

/**
 * \brief Validate UDMAP receive channel long packet treatment config
 *
 * \param rx_ignore_long UDMAP rx ch ignore long packet config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_ignore_long is valid, else -EINVAL
 */
static s32 udmap_validate_rx_ignore_long(u8 rx_ignore_long, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((rx_ignore_long !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_CH_PACKET_EXCEPTION) &&
	    (rx_ignore_long !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_CH_PACKET_IGNORED)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_RX_CH_IGNORE_LONG,
		     rx_ignore_long);

	return r;
}

/**
 * \brief Validate UDMAP receive flow extended info present config
 *
 * \param rx_einfo_present UDMAP rx flow extended info present config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_einfo_present is valid, else -EINVAL
 */
static s32 udmap_validate_flow_einfo_present(u8 rx_einfo_present,
					     u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((rx_einfo_present !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_EINFO_NOT_PRESENT) &&
	    (rx_einfo_present !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_EINFO_PRESENT)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_EINFO_PRESENT,
		     rx_einfo_present);

	return r;
}

/**
 * \brief Validate UDMAP receive flow PS words present config
 *
 * \param rx_psinfo_present UDMAP rx flow PS words present config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_psinfo_present is valid, else -EINVAL
 */
static s32 udmap_validate_flow_psinfo_present(u8	rx_psinfo_present,
					      u8	trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((rx_psinfo_present !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_PSINFO_NOT_PRESENT) &&
	    (rx_psinfo_present !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_PSINFO_PRESENT)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_PSINFO_PRESENT,
		     rx_psinfo_present);

	return r;
}

/**
 * \brief Validate UDMAP receive flow error handling config
 *
 * \param rx_error_handling UDMAP rx flow error handling config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_error_handling is valid, else -EINVAL
 */
static s32 udmap_validate_flow_error_handling(u8	rx_error_handling,
					      u8	trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((rx_error_handling !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_ERR_DROP) &&
	    (rx_error_handling !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_ERR_RETRY)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_ERROR_HANDLING,
		     rx_error_handling);

	return r;
}

#ifdef CONFIG_UDMAP_UDMA
/**
 * \brief Validate UDMAP receive flow descriptor type
 *
 * \param desc_type UDMAP rx flow descriptor type config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if desc_type is valid, else -EINVAL
 */
static s32 udmap_validate_flow_desc_type(u8 desc_type, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (desc_type) {
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_DESC_HOST:
		/*
		 * Also catches
		 * TISCI_MSG_VALUE_RM_UDMAP-FLOW_DEST_TYPE_RESET
		 */
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_DESC_MONO:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_DESC_TYPE,
		     desc_type);

	return r;
}
#endif

/**
 * \brief Validate UDMAP receive flow SOP offset
 *
 * \param rx_sop_offset UDMAP rx flow SOP offset config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_sop_offset is valid, else -EINVAL
 */
static s32 udmap_validate_flow_sop_offset(u16 rx_sop_offset, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (rx_sop_offset > TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_SOP_MAX) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_SOP_OFFSET,
		     rx_sop_offset);

	return r;
}

#ifdef CONFIG_UDMAP_UDMA
/**
 * \brief Validate UDMAP receive flow PS words location
 *
 * \param rx_ps_location UDMAP rx flow PS words location config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_ps_location is valid, else -EINVAL
 */
static s32 udmap_validate_flow_ps_location(u8 rx_ps_location, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if ((rx_ps_location !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_PS_END_PD) &&
	    (rx_ps_location !=
	     TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_PS_BEGIN_DB)) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_PS_LOCATION,
		     rx_ps_location);

	return r;
}

/**
 * \brief Validate UDMAP receive flow source tag byte selector
 *
 * \param src_tag_sel UDMAP rx flow source tag byte selector config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if src_tag_sel is valid, else -EINVAL
 */
static s32 udmap_validate_flow_src_tag_sel(u8 src_tag_sel, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (src_tag_sel) {
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_SRC_SELECT_NONE:
		/*
		 * Also catches
		 * TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_RESET and
		 * TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_RESET
		 */
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_SRC_SELECT_CFG_TAG:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_SRC_SELECT_FLOW_ID:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_SRC_SELECT_SRC_TAG:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_SRC_TAG_SEL,
		     src_tag_sel);

	return r;
}

/**
 * \brief Validate UDMAP receive flow destination tag byte selector
 *
 * \param dest_tag_sel UDMAP rx flow destination tag byte selector config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if dest_tag_sel is valid, else -EINVAL
 */
static s32 udmap_validate_flow_dest_tag_sel(u8 dest_tag_sel, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (dest_tag_sel) {
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_DEST_SELECT_NONE:
		/*
		 * Also catches
		 * TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_RESET and
		 * TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_RESET
		 */
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_DEST_SELECT_CFG_TAG:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_DEST_SELECT_FLOW_ID:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_DEST_SELECT_DEST_TAG_LO:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_DEST_SELECT_DEST_TAG_HI:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_DEST_TAG_SEL,
		     dest_tag_sel);

	return r;
}

/**
 * \brief Validate UDMAP receive flow size threshold enable
 *
 * \param rx_size_thresh_en UDMAP rx flow size threshold enable config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if rx_size_thresh_en is valid, else -EINVAL
 */
static s32 udmap_validate_flow_size_thresh_en(u8	rx_size_thresh_en,
					      u8	trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	if (rx_size_thresh_en > TISCI_MSG_VALUE_RM_UDMAP_RX_FLOW_SIZE_THRESH_MAX) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_SIZE_THRESH_EN,
		     rx_size_thresh_en);

	return r;
}
#endif

#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
/**
 * \brief Validate UDMAP channel burst size
 *
 * \param chan_type Channel type
 *
 * \param burst_size UDMAP channel burst size config
 *
 * \param trace_action RA action to trace
 *
 * \return SUCCESS if burst_size is valid, else -EINVAL
 */
static s32 udmap_validate_burst_size(u8 chan_type, u8 burst_size,
				     u8 trace_action)
{
	s32 r = -EINVAL;
	u8 trace_action_val_p = trace_action;

	switch (burst_size) {
	case UDMAP_TCHAN_TCFG_BURST_SIZE_RESERVED:
		/*
		 * Setting to RESERVED value will cause the channel to be
		 * set to 64 byte burst size.  This allows backwards
		 * compatibility with devices which do not support burst size
		 */
		r = SUCCESS;
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_64_BYTES:
		r = SUCCESS;
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_128_BYTES:
		if ((chan_type == UDMAP_TX_HCHAN) ||
		    (chan_type == UDMAP_TX_UHCHAN) ||
		    (chan_type == UDMAP_TX_ECHAN) ||
		    (chan_type == UDMAP_RX_HCHAN) ||
		    (chan_type == UDMAP_RX_UHCHAN) ||
		    (chan_type == DMSS_BCDMA_TX_HCCHAN) ||
		    (chan_type == DMSS_BCDMA_RX_HCCHAN) ||
		    (chan_type == DMSS_BCDMA_BLOCK_COPY_CHAN) ||
		    (chan_type == DMSS_BCDMA_BLOCK_COPY_HCCHAN)) {
			/*
			 * Only high capacity and BCDMA block copy channels can
			 * support burst size above 64 bytes
			 */
			r = SUCCESS;
		}
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_256_BYTES:
		if ((chan_type == UDMAP_TX_HCHAN) ||
		    (chan_type == UDMAP_TX_UHCHAN) ||
		    (chan_type == UDMAP_TX_ECHAN) ||
		    (chan_type == UDMAP_RX_HCHAN) ||
		    (chan_type == UDMAP_RX_UHCHAN) ||
		    (chan_type == DMSS_BCDMA_TX_HCCHAN) ||
		    (chan_type == DMSS_BCDMA_RX_HCCHAN) ||
		    (chan_type == DMSS_BCDMA_BLOCK_COPY_HCCHAN)) {
			/*
			 * Only high capacity channels can support burst size
			 * above 128 bytes
			 */
			r = SUCCESS;
		}
		break;
	default:
		break;
	}

	if (r != SUCCESS) {
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_CH_BURST_SIZE,
		     burst_size);

	return r;
}
#endif

#ifdef CONFIG_UDMAP_TX_CHANNEL_TEARDOWN_TYPE
/**
 * \brief Validate UDMAP transmit channel teardown type
 *
 * \param tdtype UDMAP transmit channel teardown type config
 *
 * \param trace_action UDMA action to trace
 *
 * \return SUCCESS if tdtype is valid, else -EINVAL
 */
static s32 udmap_validate_tdtype(u8 tdtype, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (tdtype) {
	case TISCI_MSG_VALUE_RM_UDMAP_TX_CH_TDTYPE_IMMEDIATE:
		break;
	case TISCI_MSG_VALUE_RM_UDMAP_TX_CH_TDTYPE_WAIT:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_TX_CH_TDTYPE,
		     tdtype);

	return r;
}
#endif

static s32 udmap_validate_extended_ch_type(u8 extended_ch_type, u8 trace_action)
{
	s32 r = SUCCESS;
	u8 trace_action_val_p = trace_action;

	switch (extended_ch_type) {
	case 0:
		break;
	case 1:
		break;
	default:
		trace_action_val_p |= TRACE_RM_ACTION_FAIL;
		r = -EINVAL;
		break;
	}

	rm_trace_sub(trace_action_val_p,
		     TRACE_RM_SUB_ACTION_UDMA_TX_CH_EXTENDED_CH_TYPE,
		     extended_ch_type);

	return r;
}

/**
 * \brief Checks if the OES index is valid for the UDMAP instance and, if
 *        valid, returns the channel details required to access the OES
 *        register mapped to the OES index
 *
 * \param inst UDMAP instance
 *
 * \param host Host ID getting/setting OES event
 *
 * \param oes_index Flattened UDMAP channel OES index.
 *
 * \param tx_ch Returns STRUE if oes_index maps to a transmit channel.  Returns
 *              SFALSE if maps to a receive channel
 *
 * \param err_evt Returns STRUE if oes_index maps to a channel's EOES register.
 *                Returns SFALSE if maps to channel's OES register
 *
 * \param ch_index Returns the channel index if OES index is valid
 *
 * \return SUCCESS if OES index is valid, else -EINVAL
 */
static s32 udmap_oes_index_validate(const struct udmap_instance *inst, u8 host,
				    u16 oes_index, sbool *tx_ch, sbool *err_evt,
				    u16 *ch_index)
{
	s32 r = -EINVAL;
	u8 n_tx_ch_type = inst->n_tx_ch_type;
	u8 n_rx_ch_type = inst->n_rx_ch_type;
	const struct udmap_ch_type *tx_ch_types = inst->tx_ch_types;
	const struct udmap_ch_type *rx_ch_types = inst->rx_ch_types;
	u8 i;
	u16 start, end, offset;
	u16 utype;

	*ch_index = UDMAP_NULL_CH_INDEX;

	if (oes_index == inst->invalid_flow_oes_offset) {
		r = rm_core_resasg_validate_resource(
			host,
			inst->invalid_flow_oes_utype,
			0u);
	} else {
		for (i = 0u; i < n_tx_ch_type; i++) {
			if (r != SUCCESS) {
				/* Tx OES */
				offset = tx_ch_types[i].oes_offset;
				start = tx_ch_types[i].start + offset;
				end = tx_ch_types[i].end + offset;
				utype = tx_ch_types[i].utype;
				if ((oes_index >= start) &&
				    (oes_index <= end)) {
					*ch_index = oes_index - offset;
					*tx_ch = STRUE;
					*err_evt = SFALSE;
					r = SUCCESS;
				}
			}

			if (r != SUCCESS) {
				/* Tx EOES */
				offset = tx_ch_types[i].err_oes_offset;
				start = tx_ch_types[i].start + offset;
				end = tx_ch_types[i].end + offset;
				utype = tx_ch_types[i].utype;
				if ((oes_index >= start) &&
				    (oes_index <= end)) {
					*ch_index = oes_index - offset;
					*tx_ch = STRUE;
					*err_evt = STRUE;
					r = SUCCESS;
				}
			}
		}

		for (i = 0u; i < n_rx_ch_type; i++) {
			if (r != SUCCESS) {
				/* Rx OES */
				offset = rx_ch_types[i].oes_offset;
				start = rx_ch_types[i].start + offset;
				end = rx_ch_types[i].end + offset;
				utype = rx_ch_types[i].utype;
				if ((oes_index >= start) &&
				    (oes_index <= end)) {
					*ch_index = oes_index - offset;
					*tx_ch = SFALSE;
					*err_evt = SFALSE;
					r = SUCCESS;
				}
			}

			if (r != SUCCESS) {
				/* Rx EOES */
				offset = rx_ch_types[i].err_oes_offset;
				start = rx_ch_types[i].start + offset;
				end = rx_ch_types[i].end + offset;
				utype = rx_ch_types[i].utype;
				if ((oes_index >= start) &&
				    (oes_index <= end)) {
					*ch_index = oes_index - offset;
					*tx_ch = SFALSE;
					*err_evt = STRUE;
					r = SUCCESS;
				}
			}
		}

		if (r == SUCCESS) {
			/* Validate against boardcfg resasg */
			r = rm_core_resasg_validate_resource(host, utype,
							     *ch_index);
		}
	}

	return r;
}

/**
 * \brief Clear a UDMA channel's OES value programmed by ROM
 *
 * \param inst Pointer to UDMA instance
 *
 * \param ring Ring index
 *
 * \param tx_ch STRUE if transmit channel.  SFALSE if receive channel
 *
 * \param err_evt STRUE if channel's EOES register.  SFALSE if channel's
 *                OES register
 *
 * \param ch_index Channel index
 *
 * \return SUCCESS if no clear performed or if clear succeeds, else -EFAIL
 */
static s32 udmap_clear_rom_oes(const struct udmap_instance *inst, sbool tx_ch,
			       sbool err_evt, u16 ch_index)
{
	s32 r = SUCCESS;
	u8 i;
	struct udmap_used_mapping *used_mapping;
	mapped_addr_t maddr;
	u32 evt_addr;
	u32 evt_reg;

	for (i = 0U; i < inst->n_rom_usage; i++) {
		used_mapping = &inst->rom_usage[i];
		if ((used_mapping->cleared == SFALSE) &&
		    (used_mapping->ch == ch_index) &&
		    (used_mapping->tx_ch == tx_ch) &&
		    (used_mapping->err_evt == err_evt)) {
			evt_reg = rm_fmk(UDMAP_GENERIC_EVENT_EVT_SHIFT,
					 UDMAP_GENERIC_EVENT_EVT_MASK,
					 UDMAP_GENERIC_EVENT_EVT_RESETVAL);

			if (tx_ch == STRUE) {
				maddr = rm_core_map_region(inst->tchan->base);
				if (err_evt == STRUE) {
					evt_addr = UDMAP_TCHAN_TEOES;
				} else {
					evt_addr = UDMAP_TCHAN_TOES;
				}
			} else {
				maddr = rm_core_map_region(inst->rchan->base);
				if (err_evt == STRUE) {
					evt_addr = UDMAP_RCHAN_REOES;
				} else {
					evt_addr = UDMAP_RCHAN_ROES;
				}
			}

			evt_addr += maddr + UDMAP_CH_BASE(ch_index);
			r = writel_verified(evt_reg, evt_addr);
			rm_core_unmap_region();
			if (r == SUCCESS) {
				used_mapping->cleared = STRUE;
			}
			break;
		}
	}

	return r;
}

/**
 * \brief Gets the global event from a UDMAP event register
 *
 * \param inst UDMAP instance
 *
 * \param ch_index UDMAP channel index
 *
 * \param tx_ch True if oes_index maps to a transmit channel.  False if maps to
 *              a receive channel
 *
 * \param err_evt True if oes_index maps to a channel's EOES register.  False
 *                if maps to channel's OES register
 *
 * \return Global event value within the event register
 */
static u16 udmap_get_evt(const struct udmap_instance *inst, u32 ch_index,
			 sbool tx_ch, sbool err_evt)
{
	mapped_addr_t maddr;
	u32 evt_addr;
	u32 evt_reg;

	if (ch_index == UDMAP_NULL_CH_INDEX) {
		/* Invalid flow OES */
		maddr = rm_core_map_region(inst->gcfg->base);
		evt_addr = maddr + UDMAP_GCFG_RFLOWFWOES;
	} else if ((tx_ch == STRUE) && (err_evt == SFALSE)) {
		/* Read from tx channel OES */
		maddr = rm_core_map_region(inst->tchan->base);
		evt_addr = maddr + UDMAP_CH_BASE(ch_index) + UDMAP_TCHAN_TOES;
	} else if ((tx_ch == STRUE) && (err_evt == STRUE)) {
		/* Read from tx channel EOES */
		maddr = rm_core_map_region(inst->tchan->base);
		evt_addr = maddr + UDMAP_CH_BASE(ch_index) + UDMAP_TCHAN_TEOES;
	} else if ((tx_ch == SFALSE) && (err_evt == SFALSE)) {
		/* Read from rx channel OES */
		maddr = rm_core_map_region(inst->rchan->base);
		evt_addr = maddr + UDMAP_CH_BASE(ch_index) + UDMAP_RCHAN_ROES;
	} else {
		/* Read from rx channel EOES */
		maddr = rm_core_map_region(inst->rchan->base);
		evt_addr = maddr + UDMAP_CH_BASE(ch_index) + UDMAP_RCHAN_REOES;
	}

	evt_reg = readl(evt_addr);
	rm_core_unmap_region();

	return (u16) rm_fext(evt_reg, UDMAP_GENERIC_EVENT_EVT_SHIFT,
			     UDMAP_GENERIC_EVENT_EVT_MASK);
}

/**
 * \brief Gets the global event within a UDMAP channel's OES registers
 *
 * \param host Host ID of processing entity attempting to get the event.
 *
 * \param id SoC device ID in which the channel OES Index resides
 *
 * \param oes_index Flattened channel oes index.  Needs to be decoded based on
 *                  UDMAP instance channel OES offsets
 *
 * \param evt Returns the global event value within the index's OES
 *            register
 *
 * \return SUCCESS if global event retrieved successfully, else error
 */
static s32 udmap_get_ch_evt(u8 host, u16 id, u16 oes_index, u16 *evt)
{
	s32 r = SUCCESS;
	const struct udmap_instance *inst = NULL;
	sbool tx_ch, err_evt;
	u16 ch_index;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_OES_GET;

	if (evt == NULL) {
		r = -EINVAL;
	} else {
		inst = udmap_get_inst(id, trace_action);
		if (inst == NULL) {
			r = -EINVAL;
		} else {
			r = udmap_oes_index_validate(inst, host, oes_index,
						     &tx_ch, &err_evt,
						     &ch_index);
			if (r == SUCCESS) {
				rm_trace_sub(trace_action,
					     TRACE_RM_SUB_ACTION_INDEX,
					     ch_index);

				r = udmap_clear_rom_oes(inst, tx_ch, err_evt,
							ch_index);
			}

			if (r != SUCCESS) {
				trace_action |= TRACE_RM_ACTION_FAIL;
				r = -EINVAL;
			}
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_OES_REG_INDEX,
			     oes_index);
	}

	if (r == SUCCESS) {
		*evt = udmap_get_evt(inst, ch_index, tx_ch, err_evt);
		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_EVENT,
			     *evt);
	}

	return r;
}

/**
 * \brief Sets the global event in a UDMAP channel's OES register
 *
 * \param host Host ID of processing entity attempting to set the event.
 *
 * \param id SoC device ID in which the channel OES index resides
 *
 * \param oes_index Flattened channel oes index.  Needs to be decoded based on
 *                  UDMAP instance channel OES offsets
 *
 * \param evt Global event value to program into the index's OES
 *            register
 *
 * \return SUCCESS if global event programmed successfully, else error
 */
static s32 udmap_set_ch_evt(u8 host, u16 id, u16 oes_index, u16 evt)
{
	s32 r = SUCCESS;
	const struct udmap_instance *inst = NULL;
	sbool tx_ch, err_evt;
	u16 ch_index;
	mapped_addr_t maddr;
	u32 evt_addr;
	u32 evt_reg;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_OES_SET;

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_EVENT,
		     evt);

	inst = udmap_get_inst(id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = udmap_oes_index_validate(inst, host, oes_index, &tx_ch,
					     &err_evt, &ch_index);
		if (r == SUCCESS) {
			rm_trace_sub(trace_action,
				     TRACE_RM_SUB_ACTION_INDEX,
				     ch_index);

			r = udmap_clear_rom_oes(inst, tx_ch, err_evt,
						ch_index);
		}

		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
			r = -EINVAL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_OES_REG_INDEX,
			     oes_index);
	}

	if (r == SUCCESS) {
		if (ch_index == UDMAP_NULL_CH_INDEX) {
			/* Write invalid flow OES */
			maddr = rm_core_map_region(inst->gcfg->base);
			evt_addr = maddr + UDMAP_GCFG_RFLOWFWOES;
		} else if ((tx_ch == STRUE) && (err_evt == SFALSE)) {
			/* Write to tx channel OES */
			maddr = rm_core_map_region(inst->tchan->base);
			evt_addr = maddr + UDMAP_CH_BASE(ch_index) +
				   UDMAP_TCHAN_TOES;
		} else if ((tx_ch == STRUE) && (err_evt == STRUE)) {
			/* Write to tx channel EOES */
			maddr = rm_core_map_region(inst->tchan->base);
			evt_addr = maddr + UDMAP_CH_BASE(ch_index) +
				   UDMAP_TCHAN_TEOES;
		} else if ((tx_ch == SFALSE) && (err_evt == SFALSE)) {
			/* Write to rx channel OES */
			maddr = rm_core_map_region(inst->rchan->base);
			evt_addr = maddr + UDMAP_CH_BASE(ch_index) +
				   UDMAP_RCHAN_ROES;
		} else {
			/* Write to rx channel EOES */
			maddr = rm_core_map_region(inst->rchan->base);
			evt_addr = maddr + UDMAP_CH_BASE(ch_index) +
				   UDMAP_RCHAN_REOES;
		}

		evt_reg = rm_fmk(UDMAP_GENERIC_EVENT_EVT_SHIFT,
				 UDMAP_GENERIC_EVENT_EVT_MASK,
				 evt);
		if (writel_verified(evt_reg, evt_addr) != SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
		rm_core_unmap_region();
	}

	return r;
}

/*
 * \brief Configure UDMAP global configuration
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the global configuration configure TISCI message
 */
static s32 udmap_gcfg_cfg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_gcfg_cfg_req	*msg)
{
	mapped_addr_t maddr;
	s32 r = SUCCESS;

	maddr = rm_core_map_region(inst->gcfg->base);

	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_GCFG_PERF_CTRL_VALID) ==
	    STRUE) {
		if (writel_verified(msg->perf_ctrl,
				    maddr + UDMAP_GCFG_PERF_CTRL) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_GCFG_EMU_CTRL_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		if (writel_verified(msg->emu_ctrl,
				    maddr + UDMAP_GCFG_EMU_CTRL) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_GCFG_PSIL_TO_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		if (writel_verified(msg->psil_to,
				    maddr + UDMAP_GCFG_PSIL_TO) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_GCFG_RFLOWFWSTAT_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		if (writel_verified(msg->rflowfwstat,
				    maddr + UDMAP_GCFG_RFLOWFWSTAT) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	rm_core_unmap_region();
	return r;
}

/**
 * \brief Create a local gcfg cfg msg using register values if the register
 *        parameter is specified as not valid
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the received gcfg cfg message
 *
 * \param loc_msg Pointer to the local gcfg cfg message
 */
static void udmap_format_local_gcfg_cfg_msg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_gcfg_cfg_req	*msg,
	struct tisci_msg_rm_udmap_gcfg_cfg_req		*loc_msg)
{
	mapped_addr_t maddr;
	u32 perf_ctrl_reg;
	u32 emu_ctrl_reg;
	u32 psil_to_reg;
	u32 rflowfwstat_reg;
	u32 dev_mask;

	dev_mask = local_udmap_gcfg_valid_masks[inst->gcfg_type];

	maddr = rm_core_map_region(inst->gcfg->base);
	/* EMU_CTRL and PSIL_TO are present for all nav types */
	emu_ctrl_reg = readl(maddr + UDMAP_GCFG_EMU_CTRL);
	psil_to_reg = readl(maddr + UDMAP_GCFG_PSIL_TO);
	/* Only read registers if present on device */
	if (rm_core_param_is_valid(dev_mask,
				   TISCI_MSG_VALUE_RM_UDMAP_GCFG_PERF_CTRL_VALID) ==
	    STRUE) {
		perf_ctrl_reg = readl(maddr + UDMAP_GCFG_PERF_CTRL);
	} else {
		perf_ctrl_reg = 0U;
	}
	if (rm_core_param_is_valid(dev_mask,
				   TISCI_MSG_VALUE_RM_UDMAP_GCFG_RFLOWFWSTAT_VALID) ==
	    STRUE) {
		rflowfwstat_reg = readl(maddr + UDMAP_GCFG_RFLOWFWSTAT);
	} else {
		rflowfwstat_reg = 0U;
	}
	rm_core_unmap_region();

	loc_msg->valid_params = msg->valid_params & dev_mask;
	loc_msg->nav_id = msg->nav_id;

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_GCFG_PERF_CTRL_VALID) ==
	    STRUE) {
		loc_msg->perf_ctrl = msg->perf_ctrl;
	} else {
		loc_msg->perf_ctrl = perf_ctrl_reg;
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_GCFG_EMU_CTRL_VALID) ==
	    STRUE) {
		loc_msg->emu_ctrl = msg->emu_ctrl;
	} else {
		loc_msg->emu_ctrl = emu_ctrl_reg;
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_GCFG_PSIL_TO_VALID) ==
	    STRUE) {
		loc_msg->psil_to = msg->psil_to;
	} else {
		loc_msg->psil_to = psil_to_reg;
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_GCFG_RFLOWFWSTAT_VALID) ==
	    STRUE) {
		loc_msg->rflowfwstat = msg->rflowfwstat;
	} else {
		loc_msg->rflowfwstat = rflowfwstat_reg;
	}
}

/*
 * \brief Configure a UDMAP transmit channel
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the transmit channel configure TISCI message
 */
static s32 udmap_tx_ch_cfg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_tx_ch_cfg_req	*msg)
{
	mapped_addr_t maddr;
	u32 ch_base = 0u;
	u32 tcfg_reg = 0u;
	u32 tcredit_reg = 0u;
	u32 tcq_reg = 0u;
	u32 tpri_ctrl_reg = 0u;
	u32 tfifo_depth_reg = 0u;
	u32 tst_sched_reg = 0u;
	sbool write;
	s32 r = SUCCESS;
	u8 extended_ch_type =
		((msg->valid_params & TISCI_MSG_VALUE_RM_UDMAP_EXTENDED_CH_TYPE_VALID) == 0U) ?
		0U : msg->extended_ch_type;

	if ((inst->bc_ch_types != NULL) &&
	    (extended_ch_type == (u8) 1)) {
		maddr = rm_core_map_region(inst->bchan->base);
		ch_base = maddr + UDMAP_CH_BASE(msg->index);
	} else {
		maddr = rm_core_map_region(inst->tchan->base);
		ch_base = maddr + UDMAP_CH_BASE(msg->index);
	}

	/* TCFG is available on all channel types */
	tcfg_reg = readl(ch_base + UDMAP_TCHAN_TCFG);
	write = SFALSE;

	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_PAUSE_ON_ERR_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_PAUSE_ON_ERR_SHIFT,
				   UDMAP_TCHAN_TCFG_PAUSE_ON_ERR_MASK,
				   msg->tx_pause_on_err);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_EINFO_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_FILT_EINFO_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_FILT_EINFO_SHIFT,
				   UDMAP_TCHAN_TCFG_FILT_EINFO_MASK,
				   msg->tx_filt_einfo);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_PSWORDS_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_FILT_PSWORDS_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_FILT_PSWORDS_SHIFT,
				   UDMAP_TCHAN_TCFG_FILT_PSWORDS_MASK,
				   msg->tx_filt_pswords);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_ATYPE_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_ATYPE_SHIFT,
				   UDMAP_TCHAN_TCFG_ATYPE_MASK,
				   msg->tx_atype);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_CHAN_TYPE_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_CHAN_TYPE_SHIFT,
				   UDMAP_TCHAN_TCFG_CHAN_TYPE_MASK,
				   msg->tx_chan_type);
		write = STRUE;
	}
#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_BURST_SIZE_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_BURST_SIZE_SHIFT,
				   UDMAP_TCHAN_TCFG_BURST_SIZE_MASK,
				   msg->tx_burst_size);
		write = STRUE;
	}
#endif
#ifdef CONFIG_UDMAP_TX_CHANNEL_TEARDOWN_TYPE
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_TDTYPE_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_TDTYPE_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_TDTYPE_SHIFT,
				   UDMAP_TCHAN_TCFG_TDTYPE_MASK,
				   msg->tx_tdtype);
		write = STRUE;
	}
#endif
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_SUPR_TDPKT_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_SUPR_TDPKT_SHIFT,
				   UDMAP_TCHAN_TCFG_SUPR_TDPKT_MASK,
				   msg->tx_supr_tdpkt);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID) ==
	    STRUE) {
		tcfg_reg &= ~UDMAP_TCHAN_TCFG_FETCH_SIZE_MASK;
		tcfg_reg |= rm_fmk(UDMAP_TCHAN_TCFG_FETCH_SIZE_SHIFT,
				   UDMAP_TCHAN_TCFG_FETCH_SIZE_MASK,
				   msg->tx_fetch_size);
		write = STRUE;
	}
	if (write == STRUE) {
		if (writel_verified(tcfg_reg, ch_base + UDMAP_TCHAN_TCFG) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_TX_CREDIT_COUNT_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		tcredit_reg = rm_fmk(UDMAP_TCHAN_TCREDIT_COUNT_SHIFT,
				     UDMAP_TCHAN_TCREDIT_COUNT_MASK,
				     msg->tx_credit_count);
		if (writel_verified(tcredit_reg,
				    ch_base + UDMAP_TCHAN_TCREDIT) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		tcq_reg = rm_fmk(UDMAP_TCHAN_TCQ_TXCQ_QNUM_SHIFT,
				 UDMAP_TCHAN_TCQ_TXCQ_QNUM_MASK,
				 msg->txcq_qnum);
		if (writel_verified(tcq_reg,
				    ch_base + UDMAP_TCHAN_TCQ) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		/* TPRI_CTRL is available on all channel types */
		tpri_ctrl_reg = readl(ch_base + UDMAP_TCHAN_TPRI_CTRL);
		write = SFALSE;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		tpri_ctrl_reg &= ~UDMAP_TCHAN_TPRI_CTRL_PRIORITY_MASK;
		tpri_ctrl_reg |= rm_fmk(UDMAP_TCHAN_TPRI_CTRL_PRIORITY_SHIFT,
					UDMAP_TCHAN_TPRI_CTRL_PRIORITY_MASK,
					msg->tx_priority);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_QOS_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		tpri_ctrl_reg &= ~UDMAP_TCHAN_TPRI_CTRL_QOS_MASK;
		tpri_ctrl_reg |= rm_fmk(UDMAP_TCHAN_TPRI_CTRL_QOS_SHIFT,
					UDMAP_TCHAN_TPRI_CTRL_QOS_MASK,
					msg->tx_qos);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		tpri_ctrl_reg &= ~UDMAP_TCHAN_TPRI_CTRL_ORDERID_MASK;
		tpri_ctrl_reg |= rm_fmk(UDMAP_TCHAN_TPRI_CTRL_ORDERID_SHIFT,
					UDMAP_TCHAN_TPRI_CTRL_ORDERID_MASK,
					msg->tx_orderid);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(tpri_ctrl_reg,
				    ch_base + UDMAP_TCHAN_TPRI_CTRL) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FDEPTH_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		tfifo_depth_reg = rm_fmk(UDMAP_TCHAN_TFIFO_DEPTH_FDEPTH_SHIFT,
					 UDMAP_TCHAN_TFIFO_DEPTH_FDEPTH_MASK,
					 msg->fdepth);
		if (writel_verified(tfifo_depth_reg,
				    ch_base + UDMAP_TCHAN_TFIFO_DEPTH) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		tst_sched_reg = rm_fmk(UDMAP_TCHAN_TST_SCHED_PRIORITY_SHIFT,
				       UDMAP_TCHAN_TST_SCHED_PRIORITY_MASK,
				       msg->tx_sched_priority);
		if (writel_verified(tst_sched_reg,
				    ch_base + UDMAP_TCHAN_TST_SCHED) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	rm_core_unmap_region();
	return r;
}

/**
 * \brief Create a local tx ch cfg msg using register values if the register
 *        parameter is specified as not valid
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the received tx ch cfg message
 *
 * \param loc_msg Pointer to the local tx ch cfg message
 *
 * \param chan_type Channel type to determine dev_mask
 */
static void udmap_format_local_tx_ch_cfg_msg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_tx_ch_cfg_req	*msg,
	struct tisci_msg_rm_udmap_tx_ch_cfg_req		*loc_msg,
	u8						chan_type)
{
	mapped_addr_t maddr;
	u32 ch_base;
	u32 tcfg_reg;
	u32 tcredit_reg;
	u32 tcq_reg;
	u32 tpri_ctrl_reg;
	u32 tfifo_depth_reg;
	u32 tst_sched_reg;
	u32 dev_mask;
	u8 extended_ch_type =
		((msg->valid_params & TISCI_MSG_VALUE_RM_UDMAP_EXTENDED_CH_TYPE_VALID) == 0U) ?
		0U : msg->extended_ch_type;

	dev_mask = local_rm_udmap_ch_valid_masks[chan_type].chan_mask;

	if ((inst->bc_ch_types != NULL) &&
	    (extended_ch_type == (u8) 1)) {
		maddr = rm_core_map_region(inst->bchan->base);
		ch_base = maddr + UDMAP_CH_BASE(msg->index);
	} else {
		maddr = rm_core_map_region(inst->tchan->base);
		ch_base = maddr + UDMAP_CH_BASE(msg->index);
	}

	/* TCFG, TPRI_CTRL, TFIFO_DEPTH, TST_SCHED exist on all channel types */
	tcfg_reg = readl(ch_base + UDMAP_TCHAN_TCFG);
	tpri_ctrl_reg = readl(ch_base + UDMAP_TCHAN_TPRI_CTRL);
	tfifo_depth_reg = readl(ch_base + UDMAP_TCHAN_TFIFO_DEPTH);
	tst_sched_reg = readl(ch_base + UDMAP_TCHAN_TST_SCHED);
	/* TCREDIT and TCQ are only in UDMA channels */
	if (rm_core_param_is_valid(dev_mask,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_CREDIT_COUNT_VALID) ==
	    STRUE) {
		tcredit_reg = readl(ch_base + UDMAP_TCHAN_TCREDIT);
		tcq_reg = readl(ch_base + UDMAP_TCHAN_TCQ);
	} else {
		tcredit_reg = 0U;
		tcq_reg = 0U;
	}
	rm_core_unmap_region();

	loc_msg->valid_params = msg->valid_params & dev_mask;
	loc_msg->nav_id = msg->nav_id;
	loc_msg->index = msg->index;

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID) ==
	    STRUE) {
		loc_msg->tx_pause_on_err = msg->tx_pause_on_err;
	} else {
		loc_msg->tx_pause_on_err = (u8) rm_fext(tcfg_reg,
							UDMAP_TCHAN_TCFG_PAUSE_ON_ERR_SHIFT,
							UDMAP_TCHAN_TCFG_PAUSE_ON_ERR_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_EINFO_VALID) ==
	    STRUE) {
		loc_msg->tx_filt_einfo = msg->tx_filt_einfo;
	} else {
		loc_msg->tx_filt_einfo = (u8) rm_fext(tcfg_reg,
						      UDMAP_TCHAN_TCFG_FILT_EINFO_SHIFT,
						      UDMAP_TCHAN_TCFG_FILT_EINFO_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FILT_PSWORDS_VALID) ==
	    STRUE) {
		loc_msg->tx_filt_pswords = msg->tx_filt_pswords;
	} else {
		loc_msg->tx_filt_pswords = (u8) rm_fext(tcfg_reg,
							UDMAP_TCHAN_TCFG_FILT_PSWORDS_SHIFT,
							UDMAP_TCHAN_TCFG_FILT_PSWORDS_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID) ==
	    STRUE) {
		loc_msg->tx_atype = msg->tx_atype;
	} else {
		loc_msg->tx_atype = (u8) rm_fext(tcfg_reg,
						 UDMAP_TCHAN_TCFG_ATYPE_SHIFT,
						 UDMAP_TCHAN_TCFG_ATYPE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID) ==
	    STRUE) {
		loc_msg->tx_chan_type = msg->tx_chan_type;
	} else {
		loc_msg->tx_chan_type = (u8) rm_fext(tcfg_reg,
						     UDMAP_TCHAN_TCFG_CHAN_TYPE_SHIFT,
						     UDMAP_TCHAN_TCFG_CHAN_TYPE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_SUPR_TDPKT_VALID) ==
	    STRUE) {
		loc_msg->tx_supr_tdpkt = msg->tx_supr_tdpkt;
	} else {
		loc_msg->tx_supr_tdpkt = (u8) rm_fext(tcfg_reg,
						      UDMAP_TCHAN_TCFG_SUPR_TDPKT_SHIFT,
						      UDMAP_TCHAN_TCFG_SUPR_TDPKT_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID) ==
	    STRUE) {
		loc_msg->tx_fetch_size = msg->tx_fetch_size;
	} else {
		loc_msg->tx_fetch_size = (u16) rm_fext(tcfg_reg,
						       UDMAP_TCHAN_TCFG_FETCH_SIZE_SHIFT,
						       UDMAP_TCHAN_TCFG_FETCH_SIZE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_CREDIT_COUNT_VALID) ==
	    STRUE) {
		loc_msg->tx_credit_count = msg->tx_credit_count;
	} else {
		loc_msg->tx_credit_count = (u8) rm_fext(tcredit_reg,
							UDMAP_TCHAN_TCREDIT_COUNT_SHIFT,
							UDMAP_TCHAN_TCREDIT_COUNT_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID) ==
	    STRUE) {
		loc_msg->txcq_qnum = msg->txcq_qnum;
	} else {
		loc_msg->txcq_qnum = (u16) rm_fext(tcq_reg,
						   UDMAP_TCHAN_TCQ_TXCQ_QNUM_SHIFT,
						   UDMAP_TCHAN_TCQ_TXCQ_QNUM_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID) ==
	    STRUE) {
		loc_msg->tx_priority = msg->tx_priority;
	} else {
		loc_msg->tx_priority = (u8) rm_fext(tpri_ctrl_reg,
						    UDMAP_TCHAN_TPRI_CTRL_PRIORITY_SHIFT,
						    UDMAP_TCHAN_TPRI_CTRL_PRIORITY_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_QOS_VALID) ==
	    STRUE) {
		loc_msg->tx_qos = msg->tx_qos;
	} else {
		loc_msg->tx_qos = (u8) rm_fext(tpri_ctrl_reg,
					       UDMAP_TCHAN_TPRI_CTRL_QOS_SHIFT,
					       UDMAP_TCHAN_TPRI_CTRL_QOS_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID) ==
	    STRUE) {
		loc_msg->tx_orderid = msg->tx_orderid;
	} else {
		loc_msg->tx_orderid = (u8) rm_fext(tpri_ctrl_reg,
						   UDMAP_TCHAN_TPRI_CTRL_ORDERID_SHIFT,
						   UDMAP_TCHAN_TPRI_CTRL_ORDERID_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_FDEPTH_VALID) ==
	    STRUE) {
		loc_msg->fdepth = msg->fdepth;
	} else {
		loc_msg->fdepth = (u16) rm_fext(tfifo_depth_reg,
						UDMAP_TCHAN_TFIFO_DEPTH_FDEPTH_SHIFT,
						UDMAP_TCHAN_TFIFO_DEPTH_FDEPTH_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID) ==
	    STRUE) {
		loc_msg->tx_sched_priority = msg->tx_sched_priority;
	} else {
		loc_msg->tx_sched_priority = (u8) rm_fext(tst_sched_reg,
							  UDMAP_TCHAN_TST_SCHED_PRIORITY_SHIFT,
							  UDMAP_TCHAN_TST_SCHED_PRIORITY_MASK);
	}

#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID) ==
	    STRUE) {
		loc_msg->tx_burst_size = msg->tx_burst_size;
	} else {
		loc_msg->tx_burst_size = (u8) rm_fext(tcfg_reg,
						      UDMAP_TCHAN_TCFG_BURST_SIZE_SHIFT,
						      UDMAP_TCHAN_TCFG_BURST_SIZE_MASK);
	}
#else
	loc_msg->tx_burst_size = 0U;
#endif

#ifdef CONFIG_UDMAP_TX_CHANNEL_TEARDOWN_TYPE
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_TX_TDTYPE_VALID) ==
	    STRUE) {
		loc_msg->tx_tdtype = msg->tx_tdtype;
	} else {
		loc_msg->tx_tdtype = (u8) rm_fext(tcfg_reg,
						  UDMAP_TCHAN_TCFG_TDTYPE_SHIFT,
						  UDMAP_TCHAN_TCFG_TDTYPE_MASK);
	}
#else
	loc_msg->tx_tdtype = 0U;
#endif
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_EXTENDED_CH_TYPE_VALID) ==
	    STRUE) {
		loc_msg->extended_ch_type = msg->extended_ch_type;
	} else {
		loc_msg->extended_ch_type = (u8) 0;
	}
}

/**
 * \brief Configure a UDMAP receive channel
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the receive channel configure TISCI message
 *
 * \param chan_type Channel type to determine dev_mask
 */
static s32 udmap_rx_ch_cfg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_rx_ch_cfg_req	*msg,
	u8						chan_type)
{
	mapped_addr_t maddr;
	u32 ch_base = 0u;
	u32 rcfg_reg = 0u;
	u32 rcq_reg = 0u;
	u32 rpri_ctrl_reg = 0u;
	u32 rst_sched_reg = 0u;
	u32 rflow_rng_reg = 0u;
	sbool write;
	u32 dev_mask;
	s32 r = SUCCESS;

	dev_mask = local_rm_udmap_ch_valid_masks[chan_type].chan_mask;

	maddr = rm_core_map_region(inst->rchan->base);
	ch_base = maddr + UDMAP_CH_BASE(msg->index);

	/* RCFG available on all channel types */
	rcfg_reg = readl(ch_base + UDMAP_RCHAN_RCFG);
	write = SFALSE;

	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID) ==
	    STRUE) {
		rcfg_reg &= ~UDMAP_RCHAN_RCFG_PAUSE_ON_ERR_MASK;
		rcfg_reg |= rm_fmk(UDMAP_RCHAN_RCFG_PAUSE_ON_ERR_SHIFT,
				   UDMAP_RCHAN_RCFG_PAUSE_ON_ERR_MASK,
				   msg->rx_pause_on_err);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID) ==
	    STRUE) {
		rcfg_reg &= ~UDMAP_RCHAN_RCFG_ATYPE_MASK;
		rcfg_reg |= rm_fmk(UDMAP_RCHAN_RCFG_ATYPE_SHIFT,
				   UDMAP_RCHAN_RCFG_ATYPE_MASK,
				   msg->rx_atype);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID) ==
	    STRUE) {
		rcfg_reg &= ~UDMAP_RCHAN_RCFG_CHAN_TYPE_MASK;
		rcfg_reg |= rm_fmk(UDMAP_RCHAN_RCFG_CHAN_TYPE_SHIFT,
				   UDMAP_RCHAN_RCFG_CHAN_TYPE_MASK,
				   msg->rx_chan_type);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_SHORT_VALID) ==
	    STRUE) {
		rcfg_reg &= ~UDMAP_RCHAN_RCFG_RX_IGNORE_SHORT_MASK;
		rcfg_reg |= rm_fmk(UDMAP_RCHAN_RCFG_RX_IGNORE_SHORT_SHIFT,
				   UDMAP_RCHAN_RCFG_RX_IGNORE_SHORT_MASK,
				   msg->rx_ignore_short);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_LONG_VALID) ==
	    STRUE) {
		rcfg_reg &= ~UDMAP_RCHAN_RCFG_RX_IGNORE_LONG_MASK;
		rcfg_reg |= rm_fmk(UDMAP_RCHAN_RCFG_RX_IGNORE_LONG_SHIFT,
				   UDMAP_RCHAN_RCFG_RX_IGNORE_LONG_MASK,
				   msg->rx_ignore_long);
		write = STRUE;
	}
#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID) ==
	    STRUE) {
		rcfg_reg &= ~UDMAP_RCHAN_RCFG_BURST_SIZE_MASK;
		rcfg_reg |= rm_fmk(UDMAP_RCHAN_RCFG_BURST_SIZE_SHIFT,
				   UDMAP_RCHAN_RCFG_BURST_SIZE_MASK,
				   msg->rx_burst_size);
		write = STRUE;
	}
#endif
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID) ==
	    STRUE) {
		rcfg_reg &= ~UDMAP_RCHAN_RCFG_FETCH_SIZE_MASK;
		rcfg_reg |= rm_fmk(UDMAP_RCHAN_RCFG_FETCH_SIZE_SHIFT,
				   UDMAP_RCHAN_RCFG_FETCH_SIZE_MASK,
				   msg->rx_fetch_size);
		write = STRUE;
	}
	if (write == STRUE) {
		if (writel_verified(rcfg_reg, ch_base + UDMAP_RCHAN_RCFG) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rcq_reg = rm_fmk(UDMAP_RCHAN_RCQ_RXCQ_QNUM_SHIFT,
				 UDMAP_RCHAN_RCQ_RXCQ_QNUM_MASK,
				 msg->rxcq_qnum);
		if (writel_verified(rcq_reg, ch_base + UDMAP_RCHAN_RCQ) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		/* RPRI_CTRL available on all chan types */
		rpri_ctrl_reg = readl(ch_base + UDMAP_RCHAN_RPRI_CTRL);
		write = SFALSE;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rpri_ctrl_reg &= ~UDMAP_RCHAN_RPRI_CTRL_PRIORITY_MASK;
		rpri_ctrl_reg |= rm_fmk(UDMAP_RCHAN_RPRI_CTRL_PRIORITY_SHIFT,
					UDMAP_RCHAN_RPRI_CTRL_PRIORITY_MASK,
					msg->rx_priority);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_QOS_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rpri_ctrl_reg &= ~UDMAP_RCHAN_RPRI_CTRL_QOS_MASK;
		rpri_ctrl_reg |= rm_fmk(UDMAP_RCHAN_RPRI_CTRL_QOS_SHIFT,
					UDMAP_RCHAN_RPRI_CTRL_QOS_MASK,
					msg->rx_qos);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rpri_ctrl_reg &= ~UDMAP_RCHAN_RPRI_CTRL_ORDERID_MASK;
		rpri_ctrl_reg |= rm_fmk(UDMAP_RCHAN_RPRI_CTRL_ORDERID_SHIFT,
					UDMAP_RCHAN_RPRI_CTRL_ORDERID_MASK,
					msg->rx_orderid);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rpri_ctrl_reg,
				    ch_base + UDMAP_RCHAN_RPRI_CTRL) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rst_sched_reg = rm_fmk(UDMAP_RCHAN_RST_SCHED_PRIORITY_SHIFT,
				       UDMAP_RCHAN_RST_SCHED_PRIORITY_MASK,
				       msg->rx_sched_priority);
		if (writel_verified(rst_sched_reg,
				    ch_base + UDMAP_RCHAN_RST_SCHED) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	/* RFLOW_RNG only available on UDMA channel types */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rflow_rng_reg = readl(ch_base + UDMAP_RCHAN_RFLOW_RNG);
	} else {
		rflow_rng_reg = 0U;
	}
	if (r == SUCCESS) {
		write = SFALSE;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rflow_rng_reg &= ~UDMAP_RCHAN_RFLOW_RNG_FLOWID_CNT_MASK;
		rflow_rng_reg |= rm_fmk(UDMAP_RCHAN_RFLOW_RNG_FLOWID_CNT_SHIFT,
					UDMAP_RCHAN_RFLOW_RNG_FLOWID_CNT_MASK,
					msg->flowid_cnt);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rflow_rng_reg &= ~UDMAP_RCHAN_RFLOW_RNG_FLOWID_START_MASK;
		rflow_rng_reg |= rm_fmk(
			UDMAP_RCHAN_RFLOW_RNG_FLOWID_START_SHIFT,
			UDMAP_RCHAN_RFLOW_RNG_FLOWID_START_MASK,
			msg->flowid_start);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rflow_rng_reg,
				    ch_base + UDMAP_RCHAN_RFLOW_RNG) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	rm_core_unmap_region();
	return r;
}

/**
 * \brief Create a local rx ch cfg msg using register values if the register
 *        parameter is specified as not valid
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the received rx ch cfg message
 *
 * \param loc_msg Pointer to the local rx ch cfg message
 *
 * \param chan_type Channel type to determine dev_mask
 */
static void udmap_format_local_rx_ch_cfg_msg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_rx_ch_cfg_req	*msg,
	struct tisci_msg_rm_udmap_rx_ch_cfg_req		*loc_msg,
	u8						chan_type)
{
	mapped_addr_t maddr;
	u32 ch_base;
	u32 rcfg_reg;
	u32 rcq_reg;
	u32 rpri_ctrl_reg;
	u32 rst_sched_reg;
	u32 rflow_rng_reg;
	u32 dev_mask;

	dev_mask = local_rm_udmap_ch_valid_masks[chan_type].chan_mask;

	maddr = rm_core_map_region(inst->rchan->base);
	ch_base = maddr + UDMAP_CH_BASE(msg->index);
	/* RCFG, RPRI_CTRL, RST_SCHED are available on all channel types */
	rcfg_reg = readl(ch_base + UDMAP_RCHAN_RCFG);
	rpri_ctrl_reg = readl(ch_base + UDMAP_RCHAN_RPRI_CTRL);
	rst_sched_reg = readl(ch_base + UDMAP_RCHAN_RST_SCHED);
	/* RCQ and RFLOW_RNG are only available in UDMA channels */
	if (rm_core_param_is_valid(dev_mask,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID) ==
	    STRUE) {
		rcq_reg = readl(ch_base + UDMAP_RCHAN_RCQ);
		rflow_rng_reg = readl(ch_base + UDMAP_RCHAN_RFLOW_RNG);
	} else {
		rcq_reg = 0U;
		rflow_rng_reg = 0U;
	}
	rm_core_unmap_region();

	loc_msg->valid_params = msg->valid_params & dev_mask;
	loc_msg->nav_id = msg->nav_id;
	loc_msg->index = msg->index;

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_FETCH_SIZE_VALID) ==
	    STRUE) {
		loc_msg->rx_fetch_size = msg->rx_fetch_size;
	} else {
		loc_msg->rx_fetch_size = (u16) rm_fext(rcfg_reg,
						       UDMAP_RCHAN_RCFG_FETCH_SIZE_SHIFT,
						       UDMAP_RCHAN_RCFG_FETCH_SIZE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rxcq_qnum = msg->rxcq_qnum;
	} else {
		loc_msg->rxcq_qnum = (u16) rm_fext(rcq_reg,
						   UDMAP_RCHAN_RCQ_RXCQ_QNUM_SHIFT,
						   UDMAP_RCHAN_RCQ_RXCQ_QNUM_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_PRIORITY_VALID) ==
	    STRUE) {
		loc_msg->rx_priority = msg->rx_priority;
	} else {
		loc_msg->rx_priority = (u8) rm_fext(rpri_ctrl_reg,
						    UDMAP_RCHAN_RPRI_CTRL_PRIORITY_SHIFT,
						    UDMAP_RCHAN_RPRI_CTRL_PRIORITY_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_QOS_VALID) ==
	    STRUE) {
		loc_msg->rx_qos = msg->rx_qos;
	} else {
		loc_msg->rx_qos = (u8) rm_fext(rpri_ctrl_reg,
					       UDMAP_RCHAN_RPRI_CTRL_QOS_SHIFT,
					       UDMAP_RCHAN_RPRI_CTRL_QOS_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_ORDER_ID_VALID) ==
	    STRUE) {
		loc_msg->rx_orderid = msg->rx_orderid;
	} else {
		loc_msg->rx_orderid = (u8) rm_fext(rpri_ctrl_reg,
						   UDMAP_RCHAN_RPRI_CTRL_ORDERID_SHIFT,
						   UDMAP_RCHAN_RPRI_CTRL_ORDERID_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_SCHED_PRIORITY_VALID) ==
	    STRUE) {
		loc_msg->rx_sched_priority = msg->rx_sched_priority;
	} else {
		loc_msg->rx_sched_priority = (u8) rm_fext(rst_sched_reg,
							  UDMAP_RCHAN_RST_SCHED_PRIORITY_SHIFT,
							  UDMAP_RCHAN_RST_SCHED_PRIORITY_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID) ==
	    STRUE) {
		loc_msg->flowid_start = msg->flowid_start;
	} else {
		loc_msg->flowid_start = (u16) rm_fext(rflow_rng_reg,
						      UDMAP_RCHAN_RFLOW_RNG_FLOWID_START_SHIFT,
						      UDMAP_RCHAN_RFLOW_RNG_FLOWID_START_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID) ==
	    STRUE) {
		loc_msg->flowid_cnt = msg->flowid_cnt;
	} else {
		loc_msg->flowid_cnt = (u16) rm_fext(rflow_rng_reg,
						    UDMAP_RCHAN_RFLOW_RNG_FLOWID_CNT_SHIFT,
						    UDMAP_RCHAN_RFLOW_RNG_FLOWID_CNT_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_PAUSE_ON_ERR_VALID) ==
	    STRUE) {
		loc_msg->rx_pause_on_err = msg->rx_pause_on_err;
	} else {
		loc_msg->rx_pause_on_err = (u8) rm_fext(rcfg_reg,
							UDMAP_RCHAN_RCFG_PAUSE_ON_ERR_SHIFT,
							UDMAP_RCHAN_RCFG_PAUSE_ON_ERR_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_ATYPE_VALID) ==
	    STRUE) {
		loc_msg->rx_atype = msg->rx_atype;
	} else {
		loc_msg->rx_atype = (u8) rm_fext(rcfg_reg,
						 UDMAP_RCHAN_RCFG_ATYPE_SHIFT,
						 UDMAP_RCHAN_RCFG_ATYPE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_CHAN_TYPE_VALID) ==
	    STRUE) {
		loc_msg->rx_chan_type = msg->rx_chan_type;
	} else {
		loc_msg->rx_chan_type = (u8) rm_fext(rcfg_reg,
						     UDMAP_RCHAN_RCFG_CHAN_TYPE_SHIFT,
						     UDMAP_RCHAN_RCFG_CHAN_TYPE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_SHORT_VALID) ==
	    STRUE) {
		loc_msg->rx_ignore_short = msg->rx_ignore_short;
	} else {
		loc_msg->rx_ignore_short = (u8) rm_fext(rcfg_reg,
							UDMAP_RCHAN_RCFG_RX_IGNORE_SHORT_SHIFT,
							UDMAP_RCHAN_RCFG_RX_IGNORE_SHORT_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_RX_IGNORE_LONG_VALID) ==
	    STRUE) {
		loc_msg->rx_ignore_long = msg->rx_ignore_long;
	} else {
		loc_msg->rx_ignore_long = (u8) rm_fext(rcfg_reg,
						       UDMAP_RCHAN_RCFG_RX_IGNORE_LONG_SHIFT,
						       UDMAP_RCHAN_RCFG_RX_IGNORE_LONG_MASK);
	}

#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_CH_BURST_SIZE_VALID) ==
	    STRUE) {
		loc_msg->rx_burst_size = msg->rx_burst_size;
	} else {
		loc_msg->rx_burst_size = (u8) rm_fext(rcfg_reg,
						      UDMAP_RCHAN_RCFG_BURST_SIZE_SHIFT,
						      UDMAP_RCHAN_RCFG_BURST_SIZE_MASK);
	}
#else
	loc_msg->rx_burst_size = 0U;
#endif
}

/**
 * \brief Configure a UDMAP receive flow's standard registers
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the flow config TISCI message
 *
 * \param assoc_chan_type Channel type tied to flow to determine dev_mask
 */
static s32 udmap_flow_cfg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_flow_cfg_req	*msg,
	u8 assoc_chan_type
#ifndef							CONFIG_UDMAP_UDMA
	__attribute__((unused))
#endif
	)
{
	mapped_addr_t maddr;
	u32 flow_base;
	u32 rf_reg;
	sbool write;
	s32 r = SUCCESS;

#ifdef CONFIG_UDMAP_UDMA
	u32 dev_mask;

	dev_mask = local_rm_udmap_ch_valid_masks[assoc_chan_type].flow_cfg_mask;
#endif

	maddr = rm_core_map_region(inst->rflow->base);
	flow_base = maddr + (u32) (UDMAP_FLOW_BASE(msg->flow_index));

	write = SFALSE;
	/* Always read RFA as its on all devices */
	rf_reg = readl(flow_base + UDMAP_RFLOW_RFA);

	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_EINFO_PRESENT_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFA_RX_EINFO_PRESENT_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFA_RX_EINFO_PRESENT_SHIFT,
				 UDMAP_RFLOW_RFA_RX_EINFO_PRESENT_MASK,
				 msg->rx_einfo_present);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_PSINFO_PRESENT_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFA_RX_PSINFO_PRESENT_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFA_RX_PSINFO_PRESENT_SHIFT,
				 UDMAP_RFLOW_RFA_RX_PSINFO_PRESENT_MASK,
				 msg->rx_psinfo_present);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_ERROR_HANDLING_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFA_RX_ERROR_HANDLING_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFA_RX_ERROR_HANDLING_SHIFT,
				 UDMAP_RFLOW_RFA_RX_ERROR_HANDLING_MASK,
				 msg->rx_error_handling);
		write = STRUE;
	}
#ifdef CONFIG_UDMAP_UDMA
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DESC_TYPE_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFA_RX_DESC_TYPE_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFA_RX_DESC_TYPE_SHIFT,
				 UDMAP_RFLOW_RFA_RX_DESC_TYPE_MASK,
				 msg->rx_desc_type);
		write = STRUE;
	}
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_PS_LOCATION_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFA_RX_PS_LOCATION_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFA_RX_PS_LOCATION_SHIFT,
				 UDMAP_RFLOW_RFA_RX_PS_LOCATION_MASK,
				 msg->rx_ps_location);
		write = STRUE;
	}
#endif
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SOP_OFFSET_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFA_RX_SOP_OFFSET_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFA_RX_SOP_OFFSET_SHIFT,
				 UDMAP_RFLOW_RFA_RX_SOP_OFFSET_MASK,
				 msg->rx_sop_offset);
		write = STRUE;
	}
#ifdef CONFIG_UDMAP_UDMA
	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_QNUM_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFA_RX_DEST_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFA_RX_DEST_QNUM_SHIFT,
				 UDMAP_RFLOW_RFA_RX_DEST_QNUM_MASK,
				 msg->rx_dest_qnum);
		write = STRUE;
	}
#endif
	if (write == STRUE) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFA) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

#ifdef CONFIG_UDMAP_UDMA
	if (r == SUCCESS) {
		write = SFALSE;
	}
	/* read RFB if its present in device */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFB);
	} else {
		rf_reg = 0U;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFB_RX_SRC_TAG_HI_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFB_RX_SRC_TAG_HI_SHIFT,
				 UDMAP_RFLOW_RFB_RX_SRC_TAG_HI_MASK,
				 msg->rx_src_tag_hi);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFB_RX_SRC_TAG_LO_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFB_RX_SRC_TAG_LO_SHIFT,
				 UDMAP_RFLOW_RFB_RX_SRC_TAG_LO_MASK,
				 msg->rx_src_tag_lo);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFB_RX_DEST_TAG_HI_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFB_RX_DEST_TAG_HI_SHIFT,
				 UDMAP_RFLOW_RFB_RX_DEST_TAG_HI_MASK,
				 msg->rx_dest_tag_hi);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFB_RX_DEST_TAG_LO_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFB_RX_DEST_TAG_LO_SHIFT,
				 UDMAP_RFLOW_RFB_RX_DEST_TAG_LO_MASK,
				 msg->rx_dest_tag_lo);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFB) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		write = SFALSE;
	}
	/* read RFC if its present in device */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_SEL_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFC);
	} else {
		rf_reg = 0U;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_SEL_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFC_RX_SRC_TAG_HI_SEL_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFC_RX_SRC_TAG_HI_SEL_SHIFT,
				 UDMAP_RFLOW_RFC_RX_SRC_TAG_HI_SEL_MASK,
				 msg->rx_src_tag_hi_sel);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_SEL_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFC_RX_SRC_TAG_LO_SEL_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFC_RX_SRC_TAG_LO_SEL_SHIFT,
				 UDMAP_RFLOW_RFC_RX_SRC_TAG_LO_SEL_MASK,
				 msg->rx_src_tag_lo_sel);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_SEL_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFC_RX_DEST_TAG_HI_SEL_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFC_RX_DEST_TAG_HI_SEL_SHIFT,
				 UDMAP_RFLOW_RFC_RX_DEST_TAG_HI_SEL_MASK,
				 msg->rx_dest_tag_hi_sel);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_SEL_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFC_RX_DEST_TAG_LO_SEL_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFC_RX_DEST_TAG_LO_SEL_SHIFT,
				 UDMAP_RFLOW_RFC_RX_DEST_TAG_LO_SEL_MASK,
				 msg->rx_dest_tag_lo_sel);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFC) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		write = SFALSE;
	}
	/* read RFD if its present in device */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ0_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFD);
	} else {
		rf_reg = 0U;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ0_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFD_RX_FDQ0_SZ0_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFD_RX_FDQ0_SZ0_QNUM_SHIFT,
				 UDMAP_RFLOW_RFD_RX_FDQ0_SZ0_QNUM_MASK,
				 msg->rx_fdq0_sz0_qnum);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ1_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFD_RX_FDQ1_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFD_RX_FDQ1_QNUM_SHIFT,
				 UDMAP_RFLOW_RFD_RX_FDQ1_QNUM_MASK,
				 msg->rx_fdq1_qnum);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFD) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		write = SFALSE;
	}
	/* read RFE if its present in device */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ2_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFE);
	} else {
		rf_reg = 0U;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ2_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFE_RX_FDQ2_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFE_RX_FDQ2_QNUM_SHIFT,
				 UDMAP_RFLOW_RFE_RX_FDQ2_QNUM_MASK,
				 msg->rx_fdq2_qnum);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ3_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFE_RX_FDQ3_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFE_RX_FDQ3_QNUM_SHIFT,
				 UDMAP_RFLOW_RFE_RX_FDQ3_QNUM_MASK,
				 msg->rx_fdq3_qnum);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFE) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
#endif

	rm_core_unmap_region();
	return r;
}

/**
 * \brief Create a local flow cfg msg using register values if the register
 *        parameter is specified as not valid
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the received flow cfg message
 *
 * \param loc_msg Pointer to the local flow cfg message
 *
 * \param assoc_chan_type Channel type tied to flow to determine dev_mask
 */
static void udmap_format_local_flow_cfg_msg(
	const struct udmap_instance			*inst,
	const struct tisci_msg_rm_udmap_flow_cfg_req	*msg,
	struct tisci_msg_rm_udmap_flow_cfg_req		*loc_msg,
	u8						assoc_chan_type)
{
	mapped_addr_t maddr;
	u32 flow_base;
	u32 rfa_reg;
	u32 rfb_reg;
	u32 rfc_reg;
	u32 rfd_reg;
	u32 rfe_reg;
	u32 dev_mask;

	dev_mask = local_rm_udmap_ch_valid_masks[assoc_chan_type].flow_cfg_mask;

	maddr = rm_core_map_region(inst->rflow->base);
	flow_base = maddr + (u32) (UDMAP_FLOW_BASE(msg->flow_index));
	rfa_reg = readl(flow_base + UDMAP_RFLOW_RFA);
	if (rm_core_param_is_valid(dev_mask,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_VALID) ==
	    STRUE) {
		/* If SRC_TAG_HI is in device then all RFB..RFE are in device */
		rfb_reg = readl(flow_base + UDMAP_RFLOW_RFB);
		rfc_reg = readl(flow_base + UDMAP_RFLOW_RFC);
		rfd_reg = readl(flow_base + UDMAP_RFLOW_RFD);
		rfe_reg = readl(flow_base + UDMAP_RFLOW_RFE);
	} else {
		/* None of rfb..rfe are in device */
		rfb_reg = 0U;
		rfc_reg = 0U;
		rfd_reg = 0U;
		rfe_reg = 0U;
	}

	rm_core_unmap_region();

	loc_msg->valid_params = msg->valid_params & dev_mask;
	loc_msg->nav_id = msg->nav_id;
	loc_msg->flow_index = msg->flow_index;

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_EINFO_PRESENT_VALID) ==
	    STRUE) {
		loc_msg->rx_einfo_present = msg->rx_einfo_present;
	} else {
		loc_msg->rx_einfo_present = (u8) rm_fext(rfa_reg,
							 UDMAP_RFLOW_RFA_RX_EINFO_PRESENT_SHIFT,
							 UDMAP_RFLOW_RFA_RX_EINFO_PRESENT_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_PSINFO_PRESENT_VALID) ==
	    STRUE) {
		loc_msg->rx_psinfo_present = msg->rx_psinfo_present;
	} else {
		loc_msg->rx_psinfo_present = (u8) rm_fext(rfa_reg,
							  UDMAP_RFLOW_RFA_RX_PSINFO_PRESENT_SHIFT,
							  UDMAP_RFLOW_RFA_RX_PSINFO_PRESENT_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_ERROR_HANDLING_VALID) ==
	    STRUE) {
		loc_msg->rx_error_handling = msg->rx_error_handling;
	} else {
		loc_msg->rx_error_handling = (u8) rm_fext(rfa_reg,
							  UDMAP_RFLOW_RFA_RX_ERROR_HANDLING_SHIFT,
							  UDMAP_RFLOW_RFA_RX_ERROR_HANDLING_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DESC_TYPE_VALID) ==
	    STRUE) {
		loc_msg->rx_desc_type = msg->rx_desc_type;
	} else {
		loc_msg->rx_desc_type = (u8) rm_fext(rfa_reg,
						     UDMAP_RFLOW_RFA_RX_DESC_TYPE_SHIFT,
						     UDMAP_RFLOW_RFA_RX_DESC_TYPE_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SOP_OFFSET_VALID) ==
	    STRUE) {
		loc_msg->rx_sop_offset = msg->rx_sop_offset;
	} else {
		loc_msg->rx_sop_offset = (u16) rm_fext(rfa_reg,
						       UDMAP_RFLOW_RFA_RX_SOP_OFFSET_SHIFT,
						       UDMAP_RFLOW_RFA_RX_SOP_OFFSET_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_dest_qnum = msg->rx_dest_qnum;
	} else {
		loc_msg->rx_dest_qnum = (u16) rm_fext(rfa_reg,
						      UDMAP_RFLOW_RFA_RX_DEST_QNUM_SHIFT,
						      UDMAP_RFLOW_RFA_RX_DEST_QNUM_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_VALID) ==
	    STRUE) {
		loc_msg->rx_src_tag_hi = msg->rx_src_tag_hi;
	} else {
		loc_msg->rx_src_tag_hi = (u8) rm_fext(rfb_reg,
						      UDMAP_RFLOW_RFB_RX_SRC_TAG_HI_SHIFT,
						      UDMAP_RFLOW_RFB_RX_SRC_TAG_HI_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_VALID) ==
	    STRUE) {
		loc_msg->rx_src_tag_lo = msg->rx_src_tag_lo;
	} else {
		loc_msg->rx_src_tag_lo = (u8) rm_fext(rfb_reg,
						      UDMAP_RFLOW_RFB_RX_SRC_TAG_LO_SHIFT,
						      UDMAP_RFLOW_RFB_RX_SRC_TAG_LO_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_VALID) ==
	    STRUE) {
		loc_msg->rx_dest_tag_hi = msg->rx_dest_tag_hi;
	} else {
		loc_msg->rx_dest_tag_hi = (u8) rm_fext(rfb_reg,
						       UDMAP_RFLOW_RFB_RX_DEST_TAG_HI_SHIFT,
						       UDMAP_RFLOW_RFB_RX_DEST_TAG_HI_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_VALID) ==
	    STRUE) {
		loc_msg->rx_dest_tag_lo = msg->rx_dest_tag_lo;
	} else {
		loc_msg->rx_dest_tag_lo = (u8) rm_fext(rfb_reg,
						       UDMAP_RFLOW_RFB_RX_DEST_TAG_LO_SHIFT,
						       UDMAP_RFLOW_RFB_RX_DEST_TAG_LO_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_HI_SEL_VALID) ==
	    STRUE) {
		loc_msg->rx_src_tag_hi_sel = msg->rx_src_tag_hi_sel;
	} else {
		loc_msg->rx_src_tag_hi_sel = (u8) rm_fext(rfc_reg,
							  UDMAP_RFLOW_RFC_RX_SRC_TAG_HI_SEL_SHIFT,
							  UDMAP_RFLOW_RFC_RX_SRC_TAG_HI_SEL_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SRC_TAG_LO_SEL_VALID) ==
	    STRUE) {
		loc_msg->rx_src_tag_lo_sel = msg->rx_src_tag_lo_sel;
	} else {
		loc_msg->rx_src_tag_lo_sel = (u8) rm_fext(rfc_reg,
							  UDMAP_RFLOW_RFC_RX_SRC_TAG_LO_SEL_SHIFT,
							  UDMAP_RFLOW_RFC_RX_SRC_TAG_LO_SEL_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_HI_SEL_VALID) ==
	    STRUE) {
		loc_msg->rx_dest_tag_hi_sel = msg->rx_dest_tag_hi_sel;
	} else {
		loc_msg->rx_dest_tag_hi_sel = (u8) rm_fext(rfc_reg,
							   UDMAP_RFLOW_RFC_RX_DEST_TAG_HI_SEL_SHIFT,
							   UDMAP_RFLOW_RFC_RX_DEST_TAG_HI_SEL_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_TAG_LO_SEL_VALID) ==
	    STRUE) {
		loc_msg->rx_dest_tag_lo_sel = msg->rx_dest_tag_lo_sel;
	} else {
		loc_msg->rx_dest_tag_lo_sel = (u8) rm_fext(rfc_reg,
							   UDMAP_RFLOW_RFC_RX_DEST_TAG_LO_SEL_SHIFT,
							   UDMAP_RFLOW_RFC_RX_DEST_TAG_LO_SEL_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ0_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_fdq0_sz0_qnum = msg->rx_fdq0_sz0_qnum;
	} else {
		loc_msg->rx_fdq0_sz0_qnum = (u16) rm_fext(rfd_reg,
							  UDMAP_RFLOW_RFD_RX_FDQ0_SZ0_QNUM_SHIFT,
							  UDMAP_RFLOW_RFD_RX_FDQ0_SZ0_QNUM_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ1_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_fdq1_qnum = msg->rx_fdq1_qnum;
	} else {
		loc_msg->rx_fdq1_qnum = (u16) rm_fext(rfd_reg,
						      UDMAP_RFLOW_RFD_RX_FDQ1_QNUM_SHIFT,
						      UDMAP_RFLOW_RFD_RX_FDQ1_QNUM_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ2_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_fdq2_qnum = msg->rx_fdq2_qnum;
	} else {
		loc_msg->rx_fdq2_qnum = (u16) rm_fext(rfe_reg,
						      UDMAP_RFLOW_RFE_RX_FDQ2_QNUM_SHIFT,
						      UDMAP_RFLOW_RFE_RX_FDQ2_QNUM_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ3_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_fdq3_qnum = msg->rx_fdq3_qnum;
	} else {
		loc_msg->rx_fdq3_qnum = (u16) rm_fext(rfe_reg,
						      UDMAP_RFLOW_RFE_RX_FDQ3_QNUM_SHIFT,
						      UDMAP_RFLOW_RFE_RX_FDQ3_QNUM_MASK);
	}

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_PS_LOCATION_VALID) ==
	    STRUE) {
		loc_msg->rx_ps_location = msg->rx_ps_location;
	} else {
		loc_msg->rx_ps_location = (u8) rm_fext(rfa_reg,
						       UDMAP_RFLOW_RFA_RX_PS_LOCATION_SHIFT,
						       UDMAP_RFLOW_RFA_RX_PS_LOCATION_MASK);
	}
}

#ifdef CONFIG_UDMAP_UDMA
/**
 * \brief Configure a UDMAP receive flow's size threshold based free queue
 *        routing registers
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the flow size threshold config TISCI message
 *
 * \param assoc_chan_type Channel type tied to flow to determine dev_mask
 */
static s32 udmap_flow_size_thresh_cfg(
	const struct udmap_instance					*inst,
	const struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req	*msg,
	u8								assoc_chan_type)
{
	mapped_addr_t maddr;
	u32 flow_base;
	u32 rf_reg;
	sbool write;
	u32 dev_mask;
	s32 r = SUCCESS;

	dev_mask = local_rm_udmap_ch_valid_masks[assoc_chan_type].flow_size_mask;

	maddr = rm_core_map_region(inst->rflow->base);
	flow_base = maddr + (u32) (UDMAP_FLOW_BASE(msg->flow_index));

	write = SFALSE;
	/* read RFC if its present in device */
	if (rm_core_param_is_valid(dev_mask,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH_EN_VALID) ==
	    STRUE) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFC);
	} else {
		rf_reg = 0U;
	}

	if (rm_core_param_is_valid(msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH_EN_VALID) ==
	    STRUE) {
		rf_reg &= ~UDMAP_RFLOW_RFC_RX_SIZE_THRESH_EN_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFC_RX_SIZE_THRESH_EN_SHIFT,
				 UDMAP_RFLOW_RFC_RX_SIZE_THRESH_EN_MASK,
				 msg->rx_size_thresh_en);
		write = STRUE;
	}
	if (write == STRUE) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFC) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		write = SFALSE;
	}
	/* read RFF if its present in device */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH0_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFF);
	} else {
		rf_reg = 0U;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH0_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFF_RX_SIZE_THRESH0_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFF_RX_SIZE_THRESH0_SHIFT,
				 UDMAP_RFLOW_RFF_RX_SIZE_THRESH0_MASK,
				 msg->rx_size_thresh0);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH1_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFF_RX_SIZE_THRESH1_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFF_RX_SIZE_THRESH1_SHIFT,
				 UDMAP_RFLOW_RFF_RX_SIZE_THRESH1_MASK,
				 msg->rx_size_thresh1);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFF) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		write = SFALSE;
	}
	/* read RFG if its present in device */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH2_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFG);
	} else {
		rf_reg = 0U;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH2_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFG_RX_SIZE_THRESH2_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFG_RX_SIZE_THRESH2_SHIFT,
				 UDMAP_RFLOW_RFG_RX_SIZE_THRESH2_MASK,
				 msg->rx_size_thresh2);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ1_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFG_RX_FDQ0_SZ1_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFG_RX_FDQ0_SZ1_QNUM_SHIFT,
				 UDMAP_RFLOW_RFG_RX_FDQ0_SZ1_QNUM_MASK,
				 msg->rx_fdq0_sz1_qnum);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFG) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}
	if (r == SUCCESS) {
		write = SFALSE;
	}
	/* read RFH if its present in device */
	if ((rm_core_param_is_valid(dev_mask,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ2_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg = readl(flow_base + UDMAP_RFLOW_RFH);
	} else {
		rf_reg = 0U;
	}

	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ2_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFH_RX_FDQ0_SZ2_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFH_RX_FDQ0_SZ2_QNUM_SHIFT,
				 UDMAP_RFLOW_RFH_RX_FDQ0_SZ2_QNUM_MASK,
				 msg->rx_fdq0_sz2_qnum);
		write = STRUE;
	}
	if ((rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ3_QNUM_VALID) ==
	     STRUE) && (r == SUCCESS)) {
		rf_reg &= ~UDMAP_RFLOW_RFH_RX_FDQ0_SZ3_QNUM_MASK;
		rf_reg |= rm_fmk(UDMAP_RFLOW_RFH_RX_FDQ0_SZ3_QNUM_SHIFT,
				 UDMAP_RFLOW_RFH_RX_FDQ0_SZ3_QNUM_MASK,
				 msg->rx_fdq0_sz3_qnum);
		write = STRUE;
	}
	if ((write == STRUE) && (r == SUCCESS)) {
		if (writel_verified(rf_reg, flow_base + UDMAP_RFLOW_RFH) !=
		    SUCCESS) {
			/* Readback of write failed: halt */
			r = -EFAILVERIFY;
		}
	}

	rm_core_unmap_region();
	return r;
}

/**
 * \brief Create a local flow size threshold based queue routing cfg msg using
 *        register values if the register parameter is specified as not valid
 *
 * \param inst UDMAP instance
 *
 * \param msg Pointer to the received flow size thresh cfg message
 *
 * \param loc_msg Pointer to the local flow size thresh cfg message
 *
 * \param assoc_chan_type Channel type tied to flow to determine dev_mask
 */
static void udmap_format_local_flow_size_thresh_cfg_msg(
	const struct udmap_instance					*inst,
	const struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req	*msg,
	struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req		*loc_msg,
	u8								assoc_chan_type)
{
	mapped_addr_t maddr;
	u32 flow_base;
	u32 rfc_reg;
	u32 rff_reg;
	u32 rfg_reg;
	u32 rfh_reg;
	u32 dev_mask;

	dev_mask = local_rm_udmap_ch_valid_masks[assoc_chan_type].flow_size_mask;

	/* If FLOW_SIZE_THRESH0 is on device then assume all rfc..rfh are on device */
	if (rm_core_param_is_valid(dev_mask,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH0_VALID) ==
	    STRUE) {
		maddr = rm_core_map_region(inst->rflow->base);
		flow_base = maddr + (u32) (UDMAP_FLOW_BASE(msg->flow_index));
		rfc_reg = readl(flow_base + UDMAP_RFLOW_RFC);
		rff_reg = readl(flow_base + UDMAP_RFLOW_RFF);
		rfg_reg = readl(flow_base + UDMAP_RFLOW_RFG);
		rfh_reg = readl(flow_base + UDMAP_RFLOW_RFH);
		rm_core_unmap_region();
	} else {
		rfc_reg = 0U;
		rff_reg = 0U;
		rfg_reg = 0U;
		rfh_reg = 0U;
	}

	loc_msg->valid_params = msg->valid_params;
	loc_msg->nav_id = msg->nav_id;
	loc_msg->flow_index = msg->flow_index;

	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH0_VALID) ==
	    STRUE) {
		loc_msg->rx_size_thresh0 = msg->rx_size_thresh0;
	} else {
		loc_msg->rx_size_thresh0 = (u16) rm_fext(rff_reg,
							 UDMAP_RFLOW_RFF_RX_SIZE_THRESH0_SHIFT,
							 UDMAP_RFLOW_RFF_RX_SIZE_THRESH0_MASK);
	}
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH1_VALID) ==
	    STRUE) {
		loc_msg->rx_size_thresh1 = msg->rx_size_thresh1;
	} else {
		loc_msg->rx_size_thresh1 = (u16) rm_fext(rff_reg,
							 UDMAP_RFLOW_RFF_RX_SIZE_THRESH1_SHIFT,
							 UDMAP_RFLOW_RFF_RX_SIZE_THRESH1_MASK);
	}
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH2_VALID) ==
	    STRUE) {
		loc_msg->rx_size_thresh2 = msg->rx_size_thresh2;
	} else {
		loc_msg->rx_size_thresh2 = (u16) rm_fext(rfg_reg,
							 UDMAP_RFLOW_RFG_RX_SIZE_THRESH2_SHIFT,
							 UDMAP_RFLOW_RFG_RX_SIZE_THRESH2_MASK);
	}
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ1_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_fdq0_sz1_qnum = msg->rx_fdq0_sz1_qnum;
	} else {
		loc_msg->rx_fdq0_sz1_qnum = (u16) rm_fext(rfg_reg,
							  UDMAP_RFLOW_RFG_RX_FDQ0_SZ1_QNUM_SHIFT,
							  UDMAP_RFLOW_RFG_RX_FDQ0_SZ1_QNUM_MASK);
	}
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ2_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_fdq0_sz2_qnum = msg->rx_fdq0_sz2_qnum;
	} else {
		loc_msg->rx_fdq0_sz2_qnum = (u16) rm_fext(rfh_reg,
							  UDMAP_RFLOW_RFH_RX_FDQ0_SZ2_QNUM_SHIFT,
							  UDMAP_RFLOW_RFH_RX_FDQ0_SZ2_QNUM_MASK);
	}
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ3_QNUM_VALID) ==
	    STRUE) {
		loc_msg->rx_fdq0_sz3_qnum = msg->rx_fdq0_sz3_qnum;
	} else {
		loc_msg->rx_fdq0_sz3_qnum = (u16) rm_fext(rfh_reg,
							  UDMAP_RFLOW_RFH_RX_FDQ0_SZ3_QNUM_SHIFT,
							  UDMAP_RFLOW_RFH_RX_FDQ0_SZ3_QNUM_MASK);
	}
	if (rm_core_param_is_valid(loc_msg->valid_params,
				   TISCI_MSG_VALUE_RM_UDMAP_FLOW_SIZE_THRESH_EN_VALID) ==
	    STRUE) {
		loc_msg->rx_size_thresh_en = msg->rx_size_thresh_en;
	} else {
		loc_msg->rx_size_thresh_en = (u8) rm_fext(rfc_reg,
							  UDMAP_RFLOW_RFC_RX_SIZE_THRESH_EN_SHIFT,
							  UDMAP_RFLOW_RFC_RX_SIZE_THRESH_EN_MASK);
	}
}
#endif

s32 rm_udmap_gcfg_cfg(u32 *msg_recv)
{
	s32 r = SUCCESS;
	struct tisci_msg_rm_udmap_gcfg_cfg_req *msg =
		(struct tisci_msg_rm_udmap_gcfg_cfg_req *) msg_recv;
	const struct udmap_instance *inst = NULL;
	struct tisci_msg_rm_udmap_gcfg_cfg_req loc_msg;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_GCFG_CFG;

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = udmap_get_inst(msg->nav_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = rm_core_resasg_validate_resource(msg->hdr.host,
						     inst->gcfg_utype, 0U);
	}

	if (r == SUCCESS) {
		/*
		 * Create a local version of cfg message with register
		 * values for parameters that were specified as not valid
		 */
		udmap_format_local_gcfg_cfg_msg(inst, msg, &loc_msg);
	}

	if (r == SUCCESS) {
		r = udmap_gcfg_cfg(inst, &loc_msg);
	}

	return r;
}

s32 rm_udmap_tx_ch_cfg(u32 *msg_recv)
{
	s32 r = SUCCESS;
	struct tisci_msg_rm_udmap_tx_ch_cfg_req *msg =
		(struct tisci_msg_rm_udmap_tx_ch_cfg_req *) msg_recv;
	const struct udmap_instance *inst = NULL;
	struct tisci_msg_rm_udmap_tx_ch_cfg_req loc_msg;
	u16 utype;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_TX_CH_CFG;
	u8 chan_type;
	u8 extended_ch_type =
		((msg->valid_params & TISCI_MSG_VALUE_RM_UDMAP_EXTENDED_CH_TYPE_VALID) == 0U) ?
		0U : msg->extended_ch_type;

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	u8 hosts[FWL_MAX_PRIVID_SLOTS];
	u8 n_hosts = 0U;
#endif

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = udmap_get_inst(msg->nav_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = udmap_check_index_range(inst, msg->hdr.host, msg->index,
					    &utype, &chan_type, STRUE,
					    trace_action, extended_ch_type);
	}

	if (r == SUCCESS) {
		/*
		 * Create a local version of cfg message with register
		 * values for parameters that were specified as not valid
		 */
		udmap_format_local_tx_ch_cfg_msg(inst, msg, &loc_msg, chan_type);
	}

	if (r == SUCCESS) {
		r = udmap_validate_ch_pause_on_err(loc_msg.tx_pause_on_err,
						   trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_tx_filt_einfo(loc_msg.tx_filt_einfo,
						 trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_tx_filt_pswords(loc_msg.tx_filt_pswords,
						   trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_atype(loc_msg.tx_atype, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_chan_type(loc_msg.tx_chan_type, STRUE,
					     trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_tx_supr_tdpkt(loc_msg.tx_supr_tdpkt,
						 trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_ch_fetch_size(loc_msg.tx_fetch_size,
						 trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_tx_credit_count(chan_type,
						   loc_msg.tx_credit_count,
						   trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_fdepth(chan_type, loc_msg.fdepth,
					  trace_action);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.txcq_qnum != 0U) &&
	    (loc_msg.txcq_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, chan_type, msg->hdr.host,
					      loc_msg.txcq_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_CH_CQ_QNUM,
			     loc_msg.txcq_qnum);
	}
	if (r == SUCCESS) {
		r = udmap_validate_priority(loc_msg.tx_priority, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_qos(loc_msg.tx_qos, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_orderid(loc_msg.tx_orderid, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_sched_priority(loc_msg.tx_sched_priority,
						  trace_action);
	}
#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
	if (r == SUCCESS) {
		r = udmap_validate_burst_size(chan_type, loc_msg.tx_burst_size,
					      trace_action);
	}
#endif
#ifdef CONFIG_UDMAP_TX_CHANNEL_TEARDOWN_TYPE
	if (r == SUCCESS) {
		r = udmap_validate_tdtype(loc_msg.tx_tdtype, trace_action);
	}
#endif
	if (r == SUCCESS) {
		r = udmap_validate_extended_ch_type(loc_msg.extended_ch_type, trace_action);
	}

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	if (r == SUCCESS) {
		/* Call Secure RM to configure tx channel firewalls */

		r = rm_core_get_resasg_hosts(utype,
					     loc_msg.index,
					     &n_hosts,
					     &hosts[0U],
					     FWL_MAX_PRIVID_SLOTS);

		if (r == SUCCESS) {
			r = sec_rm_udmap_tx_ch_fwl_cfg(msg_recv, hosts, n_hosts);
		}
	}
#endif

	if (r == SUCCESS) {
		r = udmap_tx_ch_cfg(inst, &loc_msg);
	}

	return r;
}

s32 rm_udmap_rx_ch_cfg(u32 *msg_recv)
{
	s32 r = SUCCESS;
	struct tisci_msg_rm_udmap_rx_ch_cfg_req *msg =
		(struct tisci_msg_rm_udmap_rx_ch_cfg_req *) msg_recv;
	const struct udmap_instance *inst = NULL;
	struct tisci_msg_rm_udmap_rx_ch_cfg_req loc_msg;
	u16 utype;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_RX_CH_CFG;
	u8 chan_type;

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	u8 hosts[FWL_MAX_PRIVID_SLOTS];
	u8 n_hosts = 0U;
#endif

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = udmap_get_inst(msg->nav_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = udmap_check_index_range(inst, msg->hdr.host, msg->index,
					    &utype, &chan_type, SFALSE,
					    trace_action, 0);
	}

	if (r == SUCCESS) {
		/*
		 * Create a local version of cfg message with register
		 * values for parameters that were specified as not valid
		 */
		udmap_format_local_rx_ch_cfg_msg(inst, msg, &loc_msg, chan_type);
	}

	if (r == SUCCESS) {
		r = udmap_validate_ch_fetch_size(loc_msg.rx_fetch_size,
						 trace_action);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rxcq_qnum != 0U) &&
	    (loc_msg.rxcq_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_CH_CQ_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, chan_type, msg->hdr.host,
					      loc_msg.rxcq_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_CH_CQ_QNUM,
			     loc_msg.rxcq_qnum);
	}
	if (r == SUCCESS) {
		r = udmap_validate_priority(loc_msg.rx_priority, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_qos(loc_msg.rx_qos, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_orderid(loc_msg.rx_orderid, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_sched_priority(loc_msg.rx_sched_priority,
						  trace_action);
	}
	if ((r == SUCCESS) &&
	    ((rm_core_param_is_valid(loc_msg.valid_params,
				     TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_START_VALID) ==
	      STRUE) ||
	     (rm_core_param_is_valid(loc_msg.valid_params,
				     TISCI_MSG_VALUE_RM_UDMAP_CH_RX_FLOWID_CNT_VALID) ==
	      STRUE))) {
		r = udmap_validate_rx_flowid_range(inst,
						   msg->hdr.host,
						   loc_msg.flowid_start,
						   loc_msg.flowid_cnt,
						   trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_ch_pause_on_err(loc_msg.rx_pause_on_err,
						   trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_atype(loc_msg.rx_atype, trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_chan_type(loc_msg.rx_chan_type, SFALSE,
					     trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_rx_ignore_short(loc_msg.rx_ignore_short,
						   trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_rx_ignore_long(loc_msg.rx_ignore_long,
						  trace_action);
	}
#ifdef CONFIG_UDMAP_CHANNEL_BURST_SIZE
	if (r == SUCCESS) {
		r = udmap_validate_burst_size(chan_type, loc_msg.rx_burst_size,
					      trace_action);
	}
#endif

#ifdef CONFIG_RM_LOCAL_SUBSYSTEM_REQUESTS
	if (r == SUCCESS) {
		/* Call Secure RM to configure rx channel firewalls */

		r = rm_core_get_resasg_hosts(utype,
					     loc_msg.index,
					     &n_hosts,
					     &hosts[0U],
					     FWL_MAX_PRIVID_SLOTS);

		if (r == SUCCESS) {
			r = sec_rm_udmap_rx_ch_fwl_cfg(msg_recv, hosts, n_hosts);
		}
	}
#endif

	if (r == SUCCESS) {
		r = udmap_rx_ch_cfg(inst, &loc_msg, chan_type);
	}

	return r;
}

s32 rm_udmap_flow_cfg(u32 *msg_recv)
{
	s32 r = SUCCESS;
	struct tisci_msg_rm_udmap_flow_cfg_req *msg =
		(struct tisci_msg_rm_udmap_flow_cfg_req *) msg_recv;
	const struct udmap_instance *inst = NULL;
	struct tisci_msg_rm_udmap_flow_cfg_req loc_msg;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_FLOW_CFG;
	u8 assoc_chan_type;

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = udmap_get_inst(msg->nav_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = udmap_check_flow_index(inst, msg->hdr.host,
					   msg->flow_index, STRUE,
					   &assoc_chan_type,
					   trace_action);
	}

	if (r == SUCCESS) {
		/*
		 * Create a local version of cfg message with register
		 * values for parameters that were specified as not valid
		 */
		udmap_format_local_flow_cfg_msg(inst, msg, &loc_msg, assoc_chan_type);
	}

	if (r == SUCCESS) {
		r = udmap_validate_flow_einfo_present(loc_msg.rx_einfo_present,
						      trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_flow_psinfo_present(loc_msg.rx_psinfo_present,
						       trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_flow_error_handling(loc_msg.rx_error_handling,
						       trace_action);
	}
#ifdef CONFIG_UDMAP_UDMA
	if (r == SUCCESS) {
		r = udmap_validate_flow_desc_type(loc_msg.rx_desc_type,
						  trace_action);
	}
#endif
	if (r == SUCCESS) {
		r = udmap_validate_flow_sop_offset(loc_msg.rx_sop_offset,
						   trace_action);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rx_dest_qnum != 0U) &&
	    (loc_msg.rx_dest_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_DEST_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_dest_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_DEST_QNUM,
			     loc_msg.rx_dest_qnum);
	}
#ifdef CONFIG_UDMAP_UDMA
	if (r == SUCCESS) {
		r = udmap_validate_flow_src_tag_sel(loc_msg.rx_src_tag_hi_sel,
						    trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_flow_src_tag_sel(loc_msg.rx_src_tag_lo_sel,
						    trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_flow_dest_tag_sel(loc_msg.rx_dest_tag_hi_sel,
						     trace_action);
	}
	if (r == SUCCESS) {
		r = udmap_validate_flow_dest_tag_sel(loc_msg.rx_dest_tag_lo_sel,
						     trace_action);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rx_fdq0_sz0_qnum != 0U) &&
	    (loc_msg.rx_fdq0_sz0_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ0_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_fdq0_sz0_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_FDQ0_SZ0_QNUM,
			     loc_msg.rx_fdq0_sz0_qnum);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rx_fdq1_qnum != 0U) &&
	    (loc_msg.rx_fdq1_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ1_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_fdq1_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_FDQ1_QNUM,
			     loc_msg.rx_fdq1_qnum);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rx_fdq2_qnum != 0U) &&
	    (loc_msg.rx_fdq2_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ2_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_fdq2_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_FDQ2_QNUM,
			     loc_msg.rx_fdq2_qnum);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rx_fdq3_qnum != 0U) &&
	    (loc_msg.rx_fdq3_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ3_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_fdq3_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_FDQ3_QNUM,
			     loc_msg.rx_fdq3_qnum);
	}
	if (r == SUCCESS) {
		r = udmap_validate_flow_ps_location(loc_msg.rx_ps_location,
						    trace_action);
	}
#endif

	if (r == SUCCESS) {
		r = udmap_flow_cfg(inst, &loc_msg, assoc_chan_type);
	}

	return r;
}

#ifdef CONFIG_UDMAP_UDMA
s32 rm_udmap_flow_size_thresh_cfg(u32 *msg_recv)
#else
s32 rm_udmap_flow_size_thresh_cfg(u32 *msg_recv __attribute__((unused)))
#endif
{
	s32 r = SUCCESS;

#ifdef CONFIG_UDMAP_UDMA
	struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req *msg =
		(struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req *) msg_recv;
#endif

	/* Silently accept any flow_size_thresh_cfg on devices without UDMAP */
#ifdef CONFIG_UDMAP_UDMA
	const struct udmap_instance *inst = NULL;
	struct tisci_msg_rm_udmap_flow_size_thresh_cfg_req loc_msg;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_FLOW_SZ_CFG;
	u8 assoc_chan_type;

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = udmap_get_inst(msg->nav_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		r = udmap_check_flow_index(inst, msg->hdr.host,
					   msg->flow_index, STRUE,
					   &assoc_chan_type,
					   trace_action);
	}

	if (r == SUCCESS) {
		/*
		 * Create a local version of cfg message with register
		 * values for parameters that were specified as not valid
		 */
		udmap_format_local_flow_size_thresh_cfg_msg(inst, msg,
							    &loc_msg,
							    assoc_chan_type);
	}

	if ((r == SUCCESS) &&
	    (loc_msg.rx_fdq0_sz1_qnum != 0U) &&
	    (loc_msg.rx_fdq0_sz1_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ1_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_fdq0_sz1_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_FDQ0_SZ1_QNUM,
			     loc_msg.rx_fdq0_sz1_qnum);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rx_fdq0_sz2_qnum != 0U) &&
	    (loc_msg.rx_fdq0_sz2_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ2_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_fdq0_sz2_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_FDQ0_SZ2_QNUM,
			     loc_msg.rx_fdq0_sz2_qnum);
	}
	if ((r == SUCCESS) &&
	    (loc_msg.rx_fdq0_sz3_qnum != 0U) &&
	    (loc_msg.rx_fdq0_sz3_qnum != TISCI_MSG_VALUE_RM_UDMAP_QNUM_SUPPRESS) &&
	    (rm_core_param_is_valid(loc_msg.valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_FDQ0_SZ3_QNUM_VALID) ==
	     STRUE)) {
		/*
		 * Only validate ring index if valid through config and not
		 * reset value.  No need to validate register value since it's
		 * either reset value or a previously validated and configured
		 * ring
		 */
		r = rm_ra_validate_ring_index(inst->root_id, assoc_chan_type, msg->hdr.host,
					      loc_msg.rx_fdq0_sz3_qnum);
		if (r != SUCCESS) {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}

		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_RX_FDQ0_SZ3_QNUM,
			     loc_msg.rx_fdq0_sz3_qnum);
	}
	if (r == SUCCESS) {
		r = udmap_validate_flow_size_thresh_en(msg->rx_size_thresh_en,
						       trace_action);
	}

	if (r == SUCCESS) {
		r = udmap_flow_size_thresh_cfg(inst, &loc_msg, assoc_chan_type);
	}
#endif

	return r;
}

s32 rm_udmap_flow_delegate(u32 *msg_recv)
{
	s32 r = SUCCESS;
	struct tisci_msg_rm_udmap_flow_delegate_req *msg =
		(struct tisci_msg_rm_udmap_flow_delegate_req *) msg_recv;
	const struct udmap_instance *inst = NULL;
	u8 delegate_host = HOST_ID_NONE;
	u8 trace_action = TRACE_RM_ACTION_UDMAP_FLOW_DELEGATE;

	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_HI,
		     ((msg->valid_params >> 16U) & TRACE_DEBUG_SUB_ACTION_VAL_MASK));
	rm_trace_sub(trace_action,
		     TRACE_RM_SUB_ACTION_VALID_PARAM_LO,
		     (msg->valid_params & TRACE_DEBUG_SUB_ACTION_VAL_MASK));

	inst = udmap_get_inst(msg->dev_id, trace_action);
	if (inst == NULL) {
		r = -EINVAL;
	}

	if (r == SUCCESS) {
		if (inst->common_flow == NULL) {
			r = -EINVAL;
			trace_action |= TRACE_RM_ACTION_FAIL;
			rm_trace_sub(trace_action,
				     TRACE_RM_SUB_ACTION_UDMA_FLOW_DELEGATE_SUPPORT,
				     0U);
		}
	}

	if (r == SUCCESS) {
		r = udmap_check_flow_index(inst, msg->hdr.host,
					   msg->flow_index, SFALSE,
					   NULL,
					   trace_action);
	}

	if ((r == SUCCESS) &&
	    (rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_DELEGATE_HOST_VALID) ==
	     STRUE)) {
		r = osal_core_verify_host_id(msg->delegated_host);
		if (r == SUCCESS) {
			delegate_host = msg->delegated_host;
		} else {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}
		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_DELEGATE_HOST,
			     msg->delegated_host);
	}

	if ((r == SUCCESS) &&
	    (rm_core_param_is_valid(msg->valid_params,
				    TISCI_MSG_VALUE_RM_UDMAP_FLOW_DELEGATE_CLEAR_VALID) ==
	     STRUE)) {
		if (msg->clear == TISCI_MSG_VALUE_RM_UDMAP_FLOW_DELEGATE_CLEAR) {
			delegate_host = HOST_ID_NONE;
		} else {
			trace_action |= TRACE_RM_ACTION_FAIL;
		}
		rm_trace_sub(trace_action,
			     TRACE_RM_SUB_ACTION_UDMA_FLOW_DELEGATE_CLEAR,
			     msg->clear);
	}

	if ((r == SUCCESS) &&
	    ((rm_core_param_is_valid(msg->valid_params,
				     TISCI_MSG_VALUE_RM_UDMAP_FLOW_DELEGATE_HOST_VALID) ==
	      STRUE) ||
	     (rm_core_param_is_valid(msg->valid_params,
				     TISCI_MSG_VALUE_RM_UDMAP_FLOW_DELEGATE_CLEAR_VALID) ==
	      STRUE))) {
		/*
		 * Program delegate host only if delegated_host or clear is set
		 * as valid in the message
		 */
		inst->common_flow->delegates[msg->flow_index - inst->common_flow->start] = delegate_host;
	}

	return r;
}

sbool rm_udmap_is_managed_resasg_utype(u16 utype)
{
	sbool r = SFALSE;
	u16 i, j;

	for (i = 0u; i < udmap_inst_count; i++) {
		for (j = 0u; j < udmap_inst[i].n_tx_ch_type; j++) {
			if (utype == udmap_inst[i].tx_ch_types[j].utype) {
				r = STRUE;
			}
		}

		for (j = 0u; (j < udmap_inst[i].n_rx_ch_type) && (r == SFALSE); j++) {
			if (utype == udmap_inst[i].rx_ch_types[j].utype) {
				r = STRUE;
			}

			if (r == SFALSE) {
				if (utype ==
				    udmap_inst[i].rx_ch_types[j].flow_utype) {
					r = STRUE;
				}
			}
		}

		for (j = 0U; (j < udmap_inst[i].n_bc_ch) && (r == SFALSE); j++) {
			if (utype == udmap_inst[i].bc_ch_types[j].utype) {
				r = STRUE;
			}
		}

		if (r == SFALSE) {
			if (udmap_inst[i].common_flow != NULL) {
				if (utype == udmap_inst[i].common_flow->utype) {
					r = STRUE;
				}
			}
		}

		if (r == SFALSE) {
			if (utype == udmap_inst[i].invalid_flow_oes_utype) {
				r = STRUE;
			}
		}

		if (r == SFALSE) {
			if (utype == udmap_inst[i].gcfg_utype) {
				r = STRUE;
			}
		}

		if (r == STRUE) {
			break;
		}
	}

	return r;
}

s32 rm_udmap_init(void)
{
	s32 r = SUCCESS;
	u8 i;
	mapped_addr_t maddr;
	u32 utc_ctrl_reg;

	for (i = 0U; i < udmap_inst_count; i++) {
		if ((rm_core_validate_devgrp(udmap_inst[i].id, udmap_inst[i].devgrp) ==
		     SUCCESS) &&
		    (udmap_inst[i].initialized == SFALSE)) {
			if ((r == SUCCESS) &&
			    (udmap_inst[i].dru_ch0_dst_thread_offset > 0U)) {
				/*
				 * Configure GCFG UTC_CTRL utc_chan_start to DRU ch0
				 * PSI-L dst thread offset
				 */
				maddr = rm_core_map_region(udmap_inst[i].gcfg->base);
				utc_ctrl_reg = rm_fmk(UDMAP_GCFG_UTC_CTRL_UTC_CH_START_SHIFT,
						      UDMAP_GCFG_UTC_CTRL_UTC_CH_START_MASK,
						      udmap_inst[i].dru_ch0_dst_thread_offset);
				r = writel_verified(utc_ctrl_reg, maddr + UDMAP_GCFG_UTC_CTRL);
				rm_core_unmap_region();
			}

#ifdef CONFIG_RM_IRQ
			if (r == SUCCESS) {
				udmap_inst[i].oes_handler.oes_get = udmap_get_ch_evt;
				udmap_inst[i].oes_handler.oes_set = udmap_set_ch_evt;

				r = rm_irq_oes_src_register(udmap_inst[i].id,
							    &udmap_inst[i].oes_handler);
			}
#endif

			if (r != SUCCESS) {
				break;
			} else {
				udmap_inst[i].initialized = STRUE;
			}
		}
	}

	if (r != SUCCESS) {
		rm_trace_sub((TRACE_RM_ACTION_UDMAP_INIT |
			      TRACE_RM_ACTION_FAIL), 0U, 0U);
	}

	return r;
}

s32 rm_udmap_deinit(devgrp_t devgrp, sbool rom_deinit)
{
	s32 r = -EFAIL;
	u8 i, j;

	for (i = 0U; i < udmap_inst_count; i++) {
		if ((rm_core_validate_devgrp(udmap_inst[i].id, udmap_inst[i].devgrp) ==
		     SUCCESS) &&
		    (udmap_inst[i].initialized == STRUE) &&
		    (udmap_inst[i].devgrp == devgrp)) {
			udmap_inst[i].initialized = SFALSE;

			if (rom_deinit == STRUE) {
				/* Since ROM will be using these resources again, reset the ROM usage flags,
				 * so that way these resources are able to be cleared again during RM init.
				 */
				for (j = 0U; j < (udmap_inst[i].n_rom_usage); j++) {
					udmap_inst[i].rom_usage[j].cleared = SFALSE;
				}
			}

			r = SUCCESS;
		}
	}

	return r;
}
