/*
 *  Copyright (c) Texas Instruments Incorporated 2018-2020
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
 *  \file ipc_soc.c
 *
 *  \brief File containing the IPC driver - soc specific implementation.
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <ti/drv/ipc/ipc.h>
#include <ti/drv/ipc/include/ipc_types.h>
#include <ti/drv/ipc/soc/ipc_soc.h>
#include <ti/drv/ipc/src/ipc_priv.h>
#include <ti/drv/ipc/src/ipc_mailbox.h>

#include <ti/csl/csl_navss_main.h>
#include <ti/drv/sciclient/sciclient.h>

#define NAVSS_INTRTR_INPUT_MAILBOX0_USER0   (436U)
#define NAVSS_INTRTR_INPUT_MAILBOX1_USER0   (432U)
#define NAVSS_INTRTR_INPUT_MAILBOX2_USER0   (428U)
#define NAVSS_INTRTR_INPUT_MAILBOX3_USER0   (424U)
#define NAVSS_INTRTR_INPUT_MAILBOX4_USER0   (420U)
#define NAVSS_INTRTR_INPUT_MAILBOX5_USER0   (416U)
#define NAVSS_INTRTR_INPUT_MAILBOX6_USER0   (412U)
#define NAVSS_INTRTR_INPUT_MAILBOX7_USER0   (408U)
#define NAVSS_INTRTR_INPUT_MAILBOX8_USER0   (404U)
#define NAVSS_INTRTR_INPUT_MAILBOX9_USER0   (400U)
#define NAVSS_INTRTR_INPUT_MAILBOX10_USER0  (396U)
#define NAVSS_INTRTR_INPUT_MAILBOX11_USER0  (392U)

/* Maxwell CSL does not have Mailbox base address */
#define CSL_NAVSS_MAIN_MAILBOX_REGS_0_BASE           (0x31F80000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_1_BASE           (0x31F81000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_2_BASE           (0x31F82000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_3_BASE           (0x31F83000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_4_BASE           (0x31F84000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_5_BASE           (0x31F85000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_6_BASE           (0x31F86000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_7_BASE           (0x31F87000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_8_BASE           (0x31F88000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_9_BASE           (0x31F89000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_10_BASE          (0x31F8A000U)
#define CSL_NAVSS_MAIN_MAILBOX_REGS_11_BASE          (0x31F8B000U)


/**
 * \brief Main NavSS512 - Mailbox input line
 */
static uint32_t g_Navss512MbInput[IPC_MAILBOX_CLUSTER_CNT] =
{
    NAVSS_INTRTR_INPUT_MAILBOX0_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX1_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX2_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX3_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX4_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX5_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX6_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX7_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX8_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX9_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX10_USER0,
    NAVSS_INTRTR_INPUT_MAILBOX11_USER0
};

/**
 *  \brief Processor IDs to name mapping for all processor in Jacinto7
 */
static Ipc_ProcInfo g_Ipc_mp_procInfo[IPC_MAX_PROCS] =
{
    {IPC_MPU1_0,      "mpu1_0"},      /**< ARM A53 - VM0 */
    {IPC_MCU1_0,      "mcu1_0"},      /**< ARM MCU  R5F - core0 */
    {IPC_MCU1_1,      "mcu1_1"},      /**< ARM MCU  R5F - core1 */
};

/* Mailbox Cluster Base Address */
static uintptr_t g_IPC_Mailbox_BaseAddr[IPC_MAILBOX_CLUSTER_CNT] =
{
    CSL_NAVSS_MAIN_MAILBOX_REGS_0_BASE,     /* Mailbox - cluster0   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_1_BASE,     /* Mailbox - cluster1   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_2_BASE,     /* Mailbox - cluster2   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_3_BASE,     /* Mailbox - cluster3   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_4_BASE,     /* Mailbox - cluster4   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_5_BASE,     /* Mailbox - cluster5   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_6_BASE,     /* Mailbox - cluster6   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_7_BASE,     /* Mailbox - cluster7   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_8_BASE,     /* Mailbox - cluster8   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_9_BASE,     /* Mailbox - cluster9   */
    CSL_NAVSS_MAIN_MAILBOX_REGS_10_BASE,    /* Mailbox - cluster10  */
    CSL_NAVSS_MAIN_MAILBOX_REGS_11_BASE,    /* Mailbox - cluster11  */
};

static Ipc_MailboxInfo   g_IPC_MailboxInfo[IPC_MAX_PROCS][IPC_MAX_PROCS] =
{
    /* Host Processor - A53-vm0	*/
    {
        { { 0xFFU, 0xFFU,  0U}, { 0xFFU, 0xFFU,  0U} },  /* Self - A53-vm0 */
        { {    0U,    0U,  0U}, {    0U,    0U,  1U} },  /* mcu-r5f0 */
        { {    1U,    0U,  0U}, {    1U,    0U,  1U} },  /* mcu-r5f1 */
    },
    /* Host Processor - mcu1_0 	*/
    {
        { {    0U,    1U,  1U }, {    0U,    1U,  0U} },  /* A53-vm0 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - mcu-r5f0 */
        { {    2U,    0U,  0U }, {    2U, 0xFFU,  1U} },  /* mcu-r5f1 */
    },
    /* Host Processor - mcu1_1 */
    {
        { {    1U,    1U,  1U }, {    1U,    1U,  0U} },  /* A53-vm0 */
        { {    2U,    1U,  1U }, {    2U, 0xFFU,  0U} },  /* mcu-r5f0 */
        { { 0xFFU, 0xFFU,  0U }, { 0xFFU, 0xFFU,  0U} },  /* Self - mcu-r5f1 */
    }
};

static int32_t Ipc_sciclientIrqTranslate(uint16_t coreId, uint32_t eventId,
        uint16_t *procIrq);

int32_t Ipc_getMailboxInfoTx(uint32_t selfId, uint32_t remoteId,
                 uint32_t *clusterId, uint32_t *userId, uint32_t *queueId)
{
    int32_t retVal = -1;

    if( (selfId < IPC_MAX_PROCS) &&
        (remoteId < IPC_MAX_PROCS))
    {
        Ipc_MailboxInfo   *pMailboxInfo = &g_IPC_MailboxInfo[selfId][remoteId];

        *clusterId = pMailboxInfo->tx.cluster;
        *userId    = pMailboxInfo->tx.user;
        *queueId   = pMailboxInfo->tx.fifo;
        retVal = 0;
    }

    return retVal;
}

int32_t Ipc_getMailboxInfoRx(uint32_t selfId, uint32_t remoteId,
                 uint32_t *clusterId, uint32_t *userId, uint32_t *queueId)
{
    int32_t retVal = -1;

    if( (selfId < IPC_MAX_PROCS) &&
        (remoteId < IPC_MAX_PROCS))
    {
        Ipc_MailboxInfo   *pMailboxInfo = &g_IPC_MailboxInfo[selfId][remoteId];

        *clusterId = pMailboxInfo->rx.cluster;
        *userId    = pMailboxInfo->rx.user;
        *queueId   = pMailboxInfo->rx.fifo;
        retVal = 0;
    }

    return retVal;

}

uintptr_t Ipc_getMailboxBaseAddr(uint32_t clusterId)
{
    uintptr_t baseAddr = 0x00000000U;

    if( clusterId < IPC_MAILBOX_CLUSTER_CNT)
    {
        baseAddr = g_IPC_Mailbox_BaseAddr[clusterId];
    }

    return baseAddr;
}

uint32_t Ipc_getNavss512MailboxInputIntr(int32_t clusterId, int32_t userId)
{
    uint32_t   mailboxIntrNum = 0;

    if( (clusterId != MAILBOX_CLUSTER_INVALID)  &&
        (clusterId < IPC_MAILBOX_CLUSTER_CNT)   &&
        (userId != MAILBOX_USER_INVALID)        &&
        (userId < IPC_MAILBOX_USER_CNT))
    {
        mailboxIntrNum = g_Navss512MbInput[clusterId] + userId;
    }
    return mailboxIntrNum;
}

int32_t Ipc_setCoreEventId(uint32_t selfId, Ipc_MbConfig* cfg, uint32_t intrCnt)
{
    int32_t    retVal          = IPC_SOK;
    uint32_t   outIntrBaseNum  = 0U;
    uint32_t   vimEventBaseNum = 0U;
    uint16_t   proc_irq        = 0U;

    /*
     * static variable to used to store the base for first
     * mailbox interrupt register. In subsequent call, it uses
     * the offset of intrCnt from base
     */
    static uint16_t   start    = 0U;
    static uint16_t   range    = 0U;
    uint16_t   offset   = 0;

    /* Get available CorePack IRQ number from DMSC */
    if( (start == 0U) && (range == 0U))
    {
        retVal = Ipc_getIntNumRange(selfId, &start, &range);
    }

    if((retVal == IPC_SOK) && (range >= 1U))
    {
        /* Allocate the last 5 interrupts for IPC. Note that the IR allocation is
         * static so this needs to be carefully set. Currently first interrupt is
         * used by UDMA and middle one's are used by other modules like CPSW9G so
         * we are using last 5 as a safe option.
         */
        if(range >= 5)
        {
            offset = 5;
        }
        else
        {
            offset = range;
        }
        vimEventBaseNum = (start + range) - offset;

        /* Translation must happen after this offset */
        retVal = Ipc_sciclientIrqTranslate((uint16_t)selfId, vimEventBaseNum,
                                           &proc_irq);
        if (CSL_PASS == retVal)
        {
            vimEventBaseNum = proc_irq;
        }

    }
    else
    {
        retVal = IPC_EFAIL;
    }

    switch(selfId)
    {
        case IPC_MPU1_0:
            outIntrBaseNum = NAVSS512_MPU1_0_OUTPUT_OFFSET;
            cfg->outputIntrNum = outIntrBaseNum + intrCnt;
            cfg->eventId       = vimEventBaseNum + intrCnt;
            break;
        case IPC_MCU1_0:
            outIntrBaseNum = NAVSS512_MCU1R5F0_OUTPUT_OFFSET;
            cfg->outputIntrNum = outIntrBaseNum + intrCnt*2;
            cfg->eventId       = vimEventBaseNum + intrCnt;
            break;
        case IPC_MCU1_1:
            outIntrBaseNum = NAVSS512_MCU1R5F1_OUTPUT_OFFSET;
            cfg->outputIntrNum = outIntrBaseNum + intrCnt*2;
            cfg->eventId       = vimEventBaseNum + intrCnt;
            break;
    }

    return retVal;
}


int32_t Ipc_getMailboxIntrRouterCfg(uint32_t selfId, uint32_t clusterId, uint32_t userId,
            Ipc_MbConfig* cfg, uint32_t cnt)
{
    int32_t    retVal         = IPC_SOK;
    uint32_t   mailboxIntrNum = 0;

    /* Get Navss512 input interrupt number for mailbox */
    mailboxIntrNum = Ipc_getNavss512MailboxInputIntr(clusterId, userId);

    cfg->inputIntrNum  = mailboxIntrNum;
    cfg->priority      = 1U;
    retVal = Ipc_setCoreEventId(selfId, cfg, cnt);

    return retVal;
}

const char* Ipc_getCoreName(uint32_t procId)
{
    if(procId < IPC_MAX_PROCS)
       return g_Ipc_mp_procInfo[procId].name;
    else
       return (const char*)NULL;
}

#include <ti/drv/ipc/src/ipc_osal.h>

int32_t Ipc_main2mcu_intRouter(Ipc_MbConfig *cfg)
{
    int32_t             retVal = IPC_SOK;
    uint32_t            input = 0, output = 0;
    CSL_IntrRouterCfg   irRegs;
    static uint32_t     cnt = 0;

#ifdef BUILD_MCU1_0
    input = MAIN2MCU0_INTR_ROUTER_INPUT_BASE;
    output = MAIN2MCU0_INTR_ROUTER_OUTPUT_BASE;
#endif

#ifdef BUILD_MCU1_1
    input  = MAIN2MCU1_INTR_ROUTER_INPUT_BASE;
    output = MAIN2MCU1_INTR_ROUTER_OUTPUT_BASE;
#endif

    input  += cnt;
    output += (cnt++ * 2);

#if 1
    SystemP_printf("M2M_LVL : input %d, output %d \n", input, output);
#endif

    irRegs.pIntrRouterRegs = (CSL_intr_router_cfgRegs *) CSL_MAIN2MCU_LVL_INTRTR0_CFG_BASE;
    irRegs.pIntdRegs       = (CSL_intr_router_intd_cfgRegs *) NULL;
    irRegs.numInputIntrs   = 512;
    irRegs.numOutputIntrs  = 512;
    CSL_intrRouterCfgMux(&irRegs, input, output);

    return retVal;
}

/* Indexed list of req type */
static const uint16_t req_type[] =
{
    /* NOTE: This list should match the Core index */
    TISCI_DEV_NAVSS0_INTR_ROUTER_0,
    TISCI_DEV_MAIN2MCU_LVL_INTRTR0,
    TISCI_DEV_MAIN2MCU_LVL_INTRTR0 
};

/* Indexed list of req subtype */
static const uint16_t req_subtype[] =
{
    /* NOTE: This list should match the Core index */
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT,
    TISCI_RESASG_SUBTYPE_IR_OUTPUT 
};

/* Indexed list of dst ids */
static const int32_t map_dst_id[] =
{
    TISCI_DEV_GIC0,
    TISCI_DEV_MCU_ARMSS0_CPU0,
    TISCI_DEV_MCU_ARMSS0_CPU1
};

/* Indexed list of src ids */
static const uint16_t map_src_id[] =
{
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER0,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER1,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER2,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER3,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER4,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER5,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER6,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER7,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER8,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER9,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER10,
    TISCI_DEV_NAVSS0_MAILBOX0_CLUSTER11
};

static int32_t Ipc_sciclientIrqTranslate(uint16_t coreId, uint32_t eventId,
        uint16_t *procIrq)
{
    return Sciclient_rmIrqTranslateIrOutput(req_type[coreId],
                                            (uint16_t)eventId,
                                            (uint16_t)map_dst_id[coreId],
                                            procIrq);
}

int32_t Ipc_sciclientIrqRelease(uint16_t coreId, uint32_t clusterId,
        uint32_t userId, uint32_t intNumber)
{
	int32_t                               retVal = IPC_SOK;
    struct tisci_msg_rm_irq_release_req   rmIrqRel;

    rmIrqRel.ia_id                  = 0U;
    rmIrqRel.vint                   = 0U;
    rmIrqRel.global_event           = 0U;
    rmIrqRel.vint_status_bit_index  = 0U;

    rmIrqRel.valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID |
                              TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID;
    rmIrqRel.src_id         = map_src_id[clusterId];
    rmIrqRel.src_index      = userId;
    rmIrqRel.dst_id         = (uint16_t)map_dst_id[coreId];
    rmIrqRel.dst_host_irq   = (uint16_t)intNumber;
    rmIrqRel.secondary_host = (uint8_t)TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;

    retVal = Sciclient_rmIrqRelease(
                 &rmIrqRel, IPC_SCICLIENT_TIMEOUT);

    return retVal;
}


int32_t Ipc_sciclientIrqSet(uint16_t coreId, uint32_t clusterId,
        uint32_t userId, uint32_t intNumber)
{
	int32_t                           retVal = IPC_SOK;
    struct tisci_msg_rm_irq_set_req   rmIrqReq;
    struct tisci_msg_rm_irq_set_resp  rmIrqResp;

    rmIrqReq.ia_id                  = 0U;
    rmIrqReq.vint                   = 0U;
    rmIrqReq.global_event           = 0U;
    rmIrqReq.vint_status_bit_index  = 0U;

    rmIrqReq.valid_params   = TISCI_MSG_VALUE_RM_DST_ID_VALID |
                              TISCI_MSG_VALUE_RM_DST_HOST_IRQ_VALID;
    rmIrqReq.src_id         = map_src_id[clusterId];
    rmIrqReq.src_index      = (uint16_t)userId;
    rmIrqReq.dst_id         = (uint16_t)map_dst_id[coreId];
    rmIrqReq.dst_host_irq   = (uint16_t)intNumber;
    rmIrqReq.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;

    /* Config event */
    retVal = Sciclient_rmIrqSet(
                 &rmIrqReq, &rmIrqResp, IPC_SCICLIENT_TIMEOUT);

    return retVal;
}

int32_t Ipc_getIntNumRange(uint32_t coreIndex,
        uint16_t *rangeStartP, uint16_t *rangeNumP)
{
    int32_t                                     retVal = IPC_SOK;
    struct tisci_msg_rm_get_resource_range_resp res = {0};
    struct tisci_msg_rm_get_resource_range_req  req = {0};

    req.type           = req_type[coreIndex];
    req.subtype        = (uint8_t)req_subtype[coreIndex];
    req.secondary_host = TISCI_MSG_VALUE_RM_UNUSED_SECONDARY_HOST;

    res.range_num = 0;
    res.range_start = 0;

    /* Get interrupt number range */
    retVal =  Sciclient_rmGetResourceRange(
                &req,
                &res,
                IPC_SCICLIENT_TIMEOUT);
    if (CSL_PASS != retVal || res.range_num == 0) {
        /* Try with HOST_ID_ALL */
        req.type           = req_type[coreIndex];
        req.subtype        = (uint8_t)req_subtype[coreIndex];
        req.secondary_host = TISCI_HOST_ID_ALL;

        retVal = Sciclient_rmGetResourceRange(
                &req,
                &res,
                IPC_SCICLIENT_TIMEOUT);
    }
    if (CSL_PASS == retVal)
    {
        *rangeStartP = res.range_start;
        *rangeNumP = res.range_num;
    }

    return retVal;
}

uint32_t Ipc_getCoreId(void)
{
    uint32_t selfId =  IPC_INVALID_PROCID;

#if defined(BUILD_MPU1_0)
    selfId = IPC_MPU1_0;
#elif defined(BUILD_MCU1_0)
    selfId = IPC_MCU1_0;
#elif defined(BUILD_MCU1_1)
    selfId = IPC_MCU1_1;
#else
#error "Unsupported core Id"
#endif

    return (selfId);
}

uint32_t Ipc_isCacheCoherent(void)
{
    uint32_t isCacheCoherent;

#if defined (BUILD_MPU1_0)
    isCacheCoherent = TRUE;
#else
    isCacheCoherent = FALSE;
#endif

    return (isCacheCoherent);
}
