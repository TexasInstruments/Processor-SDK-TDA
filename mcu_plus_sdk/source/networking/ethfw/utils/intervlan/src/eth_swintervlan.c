/*
 *
 * Copyright (c) 2019-2024 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*!
 * \file     eth_swintervlan.c
 *
 * \brief    This file contains the software interVLAN utils implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */


#include <csl_cpswitch.h>
#include <utils/include/enet_appmemutils_cfg.h>
#include <utils/include/enet_appmemutils.h>
#include <utils/include/enet_apputils.h>
#include <utils/include/enet_mcm.h>

#include <utils/intervlan/include/eth_swintervlan.h>
#include <utils/console_io/include/app_log.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
/* Un comment the below macro to print the packet count on UART */
// #define APP_PRINTPKTCNT

#define CPSW_FRWD_APP_NUM_PKTS          (16U)
/* Time in ms for which app pends waiting for completion semaphore to be
 * posted. This is time blocked waiting for new Rx packet arrival
 */
#define RX_TX_COMPLETION_TIMEOUT        (1U)
#define PKT_HEADER_SIZE                 (64U)

#if defined(SAFERTOS)
#define APP_TSK_STACK_SIZE              (16U * 1024U)
#define APP_TSK_STACK_ALIGN             APP_TSK_STACK_SIZE
#else
#define APP_TSK_STACK_SIZE              (6U * 1024U)
#define APP_TSK_STACK_ALIGN             (32U)
#endif

#define APP_INTERVLAN_INGRESS_PORT_NUM  (ENET_MAC_PORT_3)
#define APP_INTERVLAN_EGRESS_PORT_NUM   (ENET_MAC_PORT_2)

#define APP_INTERVLAN_INGRESS_VLANID    (100)
#define APP_INTERVLAN_EGRESS_VLANID     (200)

#define APP_INTERVLAN_IPV4_ETHERTYPE    (0x0800)

#define AUTO_RECLAIM_TXCQ

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

typedef struct
{
    Enet_Type enetType;

    /* CPSW driver handle */
    Enet_Handle hEnet;

    Udma_DrvHandle hUdmaDrv;

    EnetDma_RxChHandle hIngRxFlow;

    uint32_t rxFlowStartIdx;

    uint32_t ingRxFlowIdx;

    EnetDma_PktQ rxReadyQ;

    EnetDma_TxChHandle hTxCh;

    uint32_t txChNum;

    /* Semaphore for signalling packet ready for processing*/
    EthFwOsal_SemHandle completionSem;

    uint8_t hostMacAddr[ENET_MAC_ADDR_LEN];

    uint32_t num_pkts;

    uint32_t coreId;

    uint32_t coreKey;

    bool isDefaultFlow;
} CpswApp_Obj;

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

static void      CpswApp_pktRxTx(void);

static int32_t   CpswApp_getRxTxHandle(void);

static uint32_t  CpswApp_receivePkts(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

CpswApp_Obj gCpswInterVlanAppObj =
{
    .hostMacAddr = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02},
};

static uint8_t testDstMacAddr[] = {0x00, 0x11, 0x02, 0x00, 0x00, 0x01};

/* Test application stack */
static uint8_t gAppTskStackMain[APP_TSK_STACK_SIZE]
 __attribute__((section(".bss:appStack")))
__attribute__ ((aligned(APP_TSK_STACK_ALIGN)));

static EthFwOsal_TaskHandle task;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
static void CpswApp_InterVlanRouting(void* a0)
{
    int32_t status;
    EnetDma_PktQ fqPktInfoQ, cqPktInfoQ;
    EnetMcm_HandleInfo handleInfo;
    EnetPer_AttachCoreOutArgs attachInfo;
    EnetMcm_CmdIf cmdIf;

    EnetMcm_getCmdIf(gCpswInterVlanAppObj.enetType, &cmdIf);
    EnetAppUtils_assert(cmdIf.hMboxCmd != NULL);
    EnetAppUtils_assert(cmdIf.hMboxResponse != NULL);

    gCpswInterVlanAppObj.coreId = EnetSoc_getCoreId();

    EnetMcm_acquireHandleInfo(&cmdIf, &handleInfo);
    EnetMcm_coreAttach(&cmdIf, gCpswInterVlanAppObj.coreId, &attachInfo);

    gCpswInterVlanAppObj.hEnet = handleInfo.hEnet;
    gCpswInterVlanAppObj.hUdmaDrv = handleInfo.hUdmaDrv;
    gCpswInterVlanAppObj.coreKey = attachInfo.coreKey;
    gCpswInterVlanAppObj.isDefaultFlow = BFALSE;

    if (gCpswInterVlanAppObj.hEnet == NULL)
    {
        appLogPrintf("Failed to open CPSW\n");
        EnetAppUtils_assert(gCpswInterVlanAppObj.hEnet == NULL);
    }

    if (gCpswInterVlanAppObj.hUdmaDrv == NULL)
    {
        appLogPrintf("Failed to Get UDMA Handle\n");
        EnetAppUtils_assert(gCpswInterVlanAppObj.hUdmaDrv == NULL);
    }

    gCpswInterVlanAppObj.completionSem = EthFwOsal_createSemaphore(0U);

    status = CpswApp_getRxTxHandle();
    appLogPrintf("Rx Flow for Software Inter-VLAN Routing is up\n");

    if (status == ENET_SOK)
    {
        CpswApp_pktRxTx();
    }

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    /* Close RX Flow */
    EnetAppUtils_closeRxFlow(gCpswInterVlanAppObj.enetType,
                             gCpswInterVlanAppObj.hEnet,
                             gCpswInterVlanAppObj.coreKey,
                             gCpswInterVlanAppObj.coreId,
                             BTRUE,
                             &fqPktInfoQ,
                             &cqPktInfoQ,
                             gCpswInterVlanAppObj.rxFlowStartIdx,
                             gCpswInterVlanAppObj.ingRxFlowIdx,
                             gCpswInterVlanAppObj.hostMacAddr,
                             gCpswInterVlanAppObj.hIngRxFlow);

    EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
    EnetAppUtils_freePktInfoQ(&cqPktInfoQ);

    /* Close TX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetAppUtils_closeTxCh(gCpswInterVlanAppObj.hEnet,
                           gCpswInterVlanAppObj.coreKey,
                           gCpswInterVlanAppObj.coreId,
                           &fqPktInfoQ,
                           &cqPktInfoQ,
                           gCpswInterVlanAppObj.hTxCh,
                           gCpswInterVlanAppObj.txChNum);

    EnetAppUtils_freePktInfoQ(&fqPktInfoQ);
    EnetAppUtils_freePktInfoQ(&cqPktInfoQ);

    EnetMcm_releaseHandleInfo(&cmdIf);
    EnetMcm_releaseCmdIf(gCpswInterVlanAppObj.enetType, &cmdIf);
}

void CpswApp_ingRxIsrFxn(void *appData)
{
    EnetDma_RxChHandle *rxFlow = (EnetDma_RxChHandle *)appData;

    /* In Rx completion handler, post the Semaphore for Forwarding task to handle
     * further processing. We also disable further Rx completion events notifications
     * here. Once all pending packets are handled, the Forwarding task will re-enable the
     * interrupt subsequently*/

    /*Step1 : disable any further completion event handling*/
    EnetDma_disableRxEvent(*rxFlow);

    /*Step2 : Post semaphore for signalling the Forwarding task */
    EthFwOsal_postSemaphore(gCpswInterVlanAppObj.completionSem);
}

void CpswApp_txIsrFxn(void *appData)
{
    EnetDma_TxChHandle *txChn = (EnetDma_TxChHandle *)appData;

    /*Step1 : Disable Tx completion notification callback. Tx completion will be handled as part
     * of Rx completion handling - this being a forwarding case*/
    EnetDma_disableTxEvent(*txChn);

    /*Step2 : Post semaphore for signalling the Forwarding task */
    EthFwOsal_postSemaphore(gCpswInterVlanAppObj.completionSem);
}

static void CpswApp_initRxReadyPktQ(void)
{
    EnetDma_PktQ rxFreeQ;
    EnetDma_PktQ rxReadyQ;
    int32_t status;
    uint32_t i;
    EnetDma_Pkt *pPktInfo;
    uint32_t scatterSegments[1];

    EnetQueue_initQ(&rxFreeQ);
    EnetQueue_initQ(&rxReadyQ);

    scatterSegments[0] = ENET_MEM_LARGE_POOL_PKT_SIZE;
    for (i = 0U; i < CPSW_FRWD_APP_NUM_PKTS; i++)
    {
        pPktInfo = EnetMem_allocEthPkt(&gCpswInterVlanAppObj,
                                       UDMA_CACHELINE_ALIGNMENT,
                                       ENET_ARRAYSIZE(scatterSegments),
                                       scatterSegments);
        EnetAppUtils_assert(pPktInfo != NULL);
        EnetQueue_enq(&rxFreeQ, &pPktInfo->node);
    }

    /* Retrieve any CPSW packets which are ready */
    status = EnetDma_retrieveRxPktQ(gCpswInterVlanAppObj.hIngRxFlow, &rxReadyQ);
    EnetAppUtils_assert(status == ENET_SOK);
    /* There should not be any packet with DMA during init */
    EnetAppUtils_assert(EnetQueue_getQCount(&rxReadyQ) == 0U);

    EnetDma_submitRxPktQ(gCpswInterVlanAppObj.hIngRxFlow,
                                 &rxFreeQ);

    /* Assert here as during init no. of DMA descriptors should be equal to
     * no. of free Ethernet buffers available with app */

    EnetAppUtils_assert(0U == EnetQueue_getQCount(&rxFreeQ));
}


static uint32_t CpswAppInterVlan_getIngressVlanMembershipMask(EnetCfgServer_InterVlanConfig *pInterVlanCfg)
{
    uint32_t memberShipMask;

    memberShipMask =
        (1 << CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->ingPortNum));
    memberShipMask |= CPSW_ALE_HOST_PORT_MASK;
    return memberShipMask;
}

static uint32_t CpswAppInterVlan_getEgressVlanMembershipMask(EnetCfgServer_InterVlanConfig *pInterVlanCfg)
{
    uint32_t memberShipMask;

    memberShipMask =
        (1 << CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->egrPortNum));
    memberShipMask |= CPSW_ALE_HOST_PORT_MASK;
    return memberShipMask;
}

static int32_t CpswApp_addAleEntries(EnetCfgServer_InterVlanConfig *pInterVlanCfg)
{
    int32_t status;
    Enet_IoctlPrms prms;
    uint32_t setUcastOutArgs;
    CpswAle_SetUcastEntryInArgs setUcastInArgs;

    /* Add ALE entry for GW/Router that enables Inter VLAN routing */
    memcpy(&setUcastInArgs.addr.addr[0U], gCpswInterVlanAppObj.hostMacAddr, sizeof(setUcastInArgs.addr.addr));
    setUcastInArgs.addr.vlanId  = 0U;
    setUcastInArgs.info.portNum = 0U;
    setUcastInArgs.info.blocked = BFALSE;
    setUcastInArgs.info.secure  = BFALSE;
    setUcastInArgs.info.super   = BFALSE;
    setUcastInArgs.info.ageable = BFALSE;
    setUcastInArgs.info.trunk   = BFALSE;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &setUcastInArgs, &setUcastOutArgs);

    ENET_IOCTL(gCpswInterVlanAppObj.hEnet,
               gCpswInterVlanAppObj.coreId,
               CPSW_ALE_IOCTL_ADD_UCAST,
               &prms, status);
    if (status != ENET_SOK)
    {
        appLogPrintf("%s() failed CPSW_ALE_IOCTL_ADD_UCAST: %d\n", __func__, status);
    }

    memcpy(&setUcastInArgs.addr.addr[0U], &pInterVlanCfg->srcMacAddr[0U],
           sizeof(setUcastInArgs.addr.addr));
    setUcastInArgs.addr.vlanId  = pInterVlanCfg->egrVlanId;
    setUcastInArgs.info.portNum = pInterVlanCfg->egrPortNum;
    setUcastInArgs.info.blocked = BFALSE;
    setUcastInArgs.info.secure  = BFALSE;
    setUcastInArgs.info.super   = BFALSE;
    setUcastInArgs.info.ageable = BFALSE;
    setUcastInArgs.info.trunk   = BFALSE;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &setUcastInArgs, &setUcastOutArgs);

    ENET_IOCTL(gCpswInterVlanAppObj.hEnet,
               gCpswInterVlanAppObj.coreId,
               CPSW_ALE_IOCTL_ADD_UCAST,
               &prms, status);

    if (status != ENET_SOK)
    {
        appLogPrintf("%s() failed CPSW_ALE_IOCTL_ADD_UCAST: %d\n", __func__, status);
    }

    memcpy(&setUcastInArgs.addr.addr[0U], &pInterVlanCfg->dstMacAddr[0U],
           sizeof(setUcastInArgs.addr.addr));
    setUcastInArgs.addr.vlanId  = pInterVlanCfg->ingVlanId;
    setUcastInArgs.info.portNum = pInterVlanCfg->ingPortNum;
    setUcastInArgs.info.blocked = BFALSE;
    setUcastInArgs.info.secure  = BFALSE;
    setUcastInArgs.info.super   = BFALSE;
    setUcastInArgs.info.ageable = BFALSE;
    setUcastInArgs.info.trunk   = BFALSE;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &setUcastInArgs, &setUcastOutArgs);

    ENET_IOCTL(gCpswInterVlanAppObj.hEnet,
               gCpswInterVlanAppObj.coreId,
               CPSW_ALE_IOCTL_ADD_UCAST,
               &prms, status);

    if (status != ENET_SOK)
    {
        appLogPrintf("%s() failed CPSW_ALE_IOCTL_ADD_UCAST: %d\n", __func__, status);
    }

    if (status == ENET_SOK)
    {
        CpswAle_VlanEntryInfo inArgs;
        uint32_t outArgs;

        memset(&inArgs, 0U, sizeof (CpswAle_VlanEntryInfo));
        inArgs.vlanIdInfo.vlanId   = pInterVlanCfg->ingVlanId;
        inArgs.vlanIdInfo.tagType  = ENET_VLAN_TAG_TYPE_INNER;
        inArgs.vlanMemberList      = CpswAppInterVlan_getIngressVlanMembershipMask(pInterVlanCfg);
        inArgs.unregMcastFloodMask = CpswAppInterVlan_getIngressVlanMembershipMask(pInterVlanCfg);
        inArgs.regMcastFloodMask   = CpswAppInterVlan_getIngressVlanMembershipMask(pInterVlanCfg);
        inArgs.forceUntaggedEgressMask = 0U;
        inArgs.noLearnMask         = 0U;
        inArgs.vidIngressCheck     = BFALSE;
        inArgs.limitIPNxtHdr       = BFALSE;
        inArgs.disallowIPFrag      = BFALSE;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(gCpswInterVlanAppObj.hEnet,
                   gCpswInterVlanAppObj.coreId,
                   CPSW_ALE_IOCTL_ADD_VLAN,
                   &prms, status);
        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed ADD_VLAN ioctl failed: %d\n", __func__, status);
        }
    }

    if (status == ENET_SOK)
    {
        CpswAle_VlanEntryInfo inArgs;
        uint32_t outArgs;

        inArgs.vlanIdInfo.vlanId = pInterVlanCfg->egrVlanId;
        inArgs.vlanIdInfo.tagType = ENET_VLAN_TAG_TYPE_INNER;
        inArgs.vlanMemberList = CpswAppInterVlan_getEgressVlanMembershipMask(pInterVlanCfg);
        inArgs.unregMcastFloodMask     = CpswAppInterVlan_getEgressVlanMembershipMask(pInterVlanCfg);
        inArgs.regMcastFloodMask       = CpswAppInterVlan_getEgressVlanMembershipMask(pInterVlanCfg);
        inArgs.forceUntaggedEgressMask = 0U;
        inArgs.noLearnMask             = 0U;
        inArgs.vidIngressCheck         = BFALSE;
        inArgs.limitIPNxtHdr           = BFALSE;
        inArgs.disallowIPFrag          = BFALSE;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(gCpswInterVlanAppObj.hEnet,
                   gCpswInterVlanAppObj.coreId,
                   CPSW_ALE_IOCTL_ADD_VLAN,
                   &prms, status);
        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed ADD_VLAN ioctl failed: %d\n", __func__, status);
        }
    }

    return status;
}

int32_t EthSwInterVlan_addClassifierEntries(EnetCfgServer_InterVlanConfig *pInterVlanCfg)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    CpswAle_SetPolicerEntryOutArgs setPolicerEntryOutArgs;
    CpswAle_SetPolicerEntryInArgs setPolicerEntryInArgs;

    EnetAppUtils_addHostPortEntry(gCpswInterVlanAppObj.hEnet,
                                  gCpswInterVlanAppObj.coreId,
                                  &gCpswInterVlanAppObj.hostMacAddr[0U]);

    /* Copy the Dst MAC address to use it in routing */
    memcpy(&testDstMacAddr[0U],
           &pInterVlanCfg->dstMacAddr[0U],
           ENET_MAC_ADDR_LEN);

    if (status == ENET_SOK)
    {
        /* Add Policer Entry for Ingress Flow */
        /*TODO: Adding MACPORT based classification is not working, need to debug further */
        setPolicerEntryInArgs.policerMatch.policerMatchEnMask = ( // CPSW_ALE_POLICER_MATCH_PORT |
                                                                     CPSW_ALE_POLICER_MATCH_MACSRC |
                                                                     CPSW_ALE_POLICER_MATCH_MACDST |
                                                                     CPSW_ALE_POLICER_MATCH_IVLAN |
                                                                     CPSW_ALE_POLICER_MATCH_ETHERTYPE |
                                                                     CPSW_ALE_POLICER_MATCH_IPSRC |
                                                                     CPSW_ALE_POLICER_MATCH_IPDST);

        setPolicerEntryInArgs.policerMatch.portNum     = pInterVlanCfg->ingPortNum;
        setPolicerEntryInArgs.policerMatch.portIsTrunk = BFALSE;

        setPolicerEntryInArgs.policerMatch.srcMacAddrInfo.portNum = pInterVlanCfg->ingPortNum;
        setPolicerEntryInArgs.policerMatch.dstMacAddrInfo.portNum = 0U;

        memcpy(&setPolicerEntryInArgs.policerMatch.srcMacAddrInfo.addr.addr[0U],
               &pInterVlanCfg->srcMacAddr[0U],
               ENET_MAC_ADDR_LEN);

        memcpy(&setPolicerEntryInArgs.policerMatch.dstMacAddrInfo.addr.addr[0U],
               &gCpswInterVlanAppObj.hostMacAddr[0U],
               ENET_MAC_ADDR_LEN);

        setPolicerEntryInArgs.policerMatch.srcMacAddrInfo.addr.vlanId = 0U;
        setPolicerEntryInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = 0U;

        setPolicerEntryInArgs.policerMatch.ivlanId   = pInterVlanCfg->ingVlanId;
        setPolicerEntryInArgs.policerMatch.etherType = APP_INTERVLAN_IPV4_ETHERTYPE;

        setPolicerEntryInArgs.policerMatch.srcIpInfo.ipAddrType = CPSW_ALE_IPADDR_CLASSIFIER_IPV4;
        setPolicerEntryInArgs.policerMatch.dstIpInfo.ipAddrType = CPSW_ALE_IPADDR_CLASSIFIER_IPV4;

        memcpy(&setPolicerEntryInArgs.policerMatch.srcIpInfo.ipv4Info.ipv4Addr[0U],
               &pInterVlanCfg->srcIpv4Addr[0U],
               sizeof(setPolicerEntryInArgs.policerMatch.srcIpInfo.ipv4Info.ipv4Addr));

        memcpy(&setPolicerEntryInArgs.policerMatch.dstIpInfo.ipv4Info.ipv4Addr[0U],
               &pInterVlanCfg->dstIpv4Addr[0U],
               sizeof(setPolicerEntryInArgs.policerMatch.dstIpInfo.ipv4Info.ipv4Addr));

        setPolicerEntryInArgs.policerMatch.srcIpInfo.ipv4Info.numLSBIgnoreBits = 0U;
        setPolicerEntryInArgs.policerMatch.dstIpInfo.ipv4Info.numLSBIgnoreBits = 0U;

        setPolicerEntryInArgs.threadIdEn = BTRUE;
        setPolicerEntryInArgs.threadId = gCpswInterVlanAppObj.ingRxFlowIdx;
        setPolicerEntryInArgs.peakRateInBitsPerSec   = 0U;
        setPolicerEntryInArgs.commitRateInBitsPerSec = 0U;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &setPolicerEntryInArgs, &setPolicerEntryOutArgs);

        ENET_IOCTL(gCpswInterVlanAppObj.hEnet,
                   gCpswInterVlanAppObj.coreId,
                   CPSW_ALE_IOCTL_SET_POLICER,
                   &prms, status);

        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed CPSW_ALE_IOCTL_SET_POLICER: %d\n", __func__, status);
        }
    }

    if (status == ENET_SOK)
    {
        status = CpswApp_addAleEntries(pInterVlanCfg);
        EnetAppUtils_assert(ENET_SOK == status);
    }

    return status;
}

static int32_t CpswApp_getRxTxHandle(void)
{
    int32_t status = ENET_SOK;
    EnetDma_Handle hDma;
    EnetUdma_OpenTxChPrms cpswTxChCfg;
    EnetUdma_OpenRxFlowPrms cpswRxFlowCfg;
    EnetUdma_UdmaRingPrms *pFqRingPrms = NULL;

    /* Open the CPSW RX flow for Ingress */
    {
        EnetUdma_initRxFlowParams(&cpswRxFlowCfg);
        EnetAppUtils_setCommonRxFlowPrms(&cpswRxFlowCfg);

        cpswRxFlowCfg.notifyCb            = &CpswApp_ingRxIsrFxn;
        cpswRxFlowCfg.numRxPkts           = CPSW_FRWD_APP_NUM_PKTS;
        cpswRxFlowCfg.hUdmaDrv            = gCpswInterVlanAppObj.hUdmaDrv;
        cpswRxFlowCfg.cbArg               = &gCpswInterVlanAppObj.hIngRxFlow;
        cpswRxFlowCfg.useProxy            = BTRUE;
        cpswRxFlowCfg.disableCacheOpsFlag = BTRUE;

        /* Use ring monitor for the CQ ring of RX flow */
#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
        pFqRingPrms = &cpswRxFlowCfg.udmaChPrms.fqRingPrms;
#endif
#ifdef AUTO_RECLAIM_TXCQ
        /* set ring mode to message as both TX channel (pushes to CQ) and RX flow
         * (pops from FQ) uses this ring */
        pFqRingPrms->mode = CSL_RINGACC_RING_MODE_MESSAGE;
#endif

        pFqRingPrms->useRingMon      = BFALSE;
        pFqRingPrms->ringMonCfg.mode = TISCI_MSG_VALUE_RM_MON_MODE_THRESHOLD;

        /* Ring mon low threshold */
#if defined _DEBUG_
        /* In debug mode as CPU is processing lesser packets per event, keep threshold more */
        pFqRingPrms->ringMonCfg.data0 = (CPSW_FRWD_APP_NUM_PKTS - 10U);
#else
        pFqRingPrms->ringMonCfg.data0 = (CPSW_FRWD_APP_NUM_PKTS - 20U);
#endif
        /* Ring mon high threshold - to get only low  threshold event, setting high threshold as more than ring depth*/
        pFqRingPrms->ringMonCfg.data1 = CPSW_FRWD_APP_NUM_PKTS;

        status = EnetAppUtils_allocRxFlow(gCpswInterVlanAppObj.hEnet,
                                          gCpswInterVlanAppObj.coreKey,
                                          gCpswInterVlanAppObj.coreId,
                                          &gCpswInterVlanAppObj.rxFlowStartIdx,
                                          &gCpswInterVlanAppObj.ingRxFlowIdx);
        EnetAppUtils_assert(status == ENET_SOK);

        cpswRxFlowCfg.startIdx = gCpswInterVlanAppObj.rxFlowStartIdx;
        cpswRxFlowCfg.flowIdx = gCpswInterVlanAppObj.ingRxFlowIdx;

        hDma = Enet_getDmaHandle(gCpswInterVlanAppObj.hEnet);
        EnetAppUtils_assert(hDma != NULL);

        gCpswInterVlanAppObj.hIngRxFlow = EnetUdma_openRxFlow(hDma, &cpswRxFlowCfg);
        if (gCpswInterVlanAppObj.hIngRxFlow == NULL)
        {
            EnetAppUtils_freeRxFlow(gCpswInterVlanAppObj.hEnet,
                                    gCpswInterVlanAppObj.coreKey,
                                    gCpswInterVlanAppObj.coreId,
                                    gCpswInterVlanAppObj.ingRxFlowIdx);
            EnetAppUtils_assert(BFALSE);
        }
    }

    /* Open the CPSW TX channel  */
    {
        EnetUdma_initTxChParams(&cpswTxChCfg);
        EnetAppUtils_setCommonTxChPrms(&cpswTxChCfg);

        cpswTxChCfg.hUdmaDrv            = gCpswInterVlanAppObj.hUdmaDrv;
        cpswTxChCfg.numTxPkts           = CPSW_FRWD_APP_NUM_PKTS;
        cpswTxChCfg.cbArg               = &gCpswInterVlanAppObj.hTxCh;
        cpswTxChCfg.notifyCb            = &CpswApp_txIsrFxn;
        cpswTxChCfg.useProxy            = BTRUE;
        cpswTxChCfg.disableCacheOpsFlag = BTRUE;

#ifdef AUTO_RECLAIM_TXCQ
        cpswTxChCfg.autoReclaimPrms.enableFlag   = BTRUE;
        cpswTxChCfg.autoReclaimPrms.hDmaDescPool = EnetUdma_getRxFlowDescPoolHandle(gCpswInterVlanAppObj.hIngRxFlow);
        cpswTxChCfg.autoReclaimPrms.hReclaimRing = EnetUdma_getRxFlowFqHandle(gCpswInterVlanAppObj.hIngRxFlow);

        /* Filter all protocol specific and extended info as we are using auto reclaim and this
         * information might corrupt RX flow */
        cpswTxChCfg.udmaTxChPrms.filterEinfo = TISCI_MSG_VALUE_RM_UDMAP_TX_CH_FILT_EINFO_DISABLED;
        cpswTxChCfg.udmaTxChPrms.filterPsWords = TISCI_MSG_VALUE_RM_UDMAP_TX_CH_FILT_PSWORDS_ENABLED;
        /* Notify callback should be NULL when auto recycle is enabled as the CQ event should be disabled */
        cpswTxChCfg.notifyCb = NULL;
#else
        cpswTxChCfg.notifyCb = &CpswApp_txIsrFxn;
#endif

        EnetAppUtils_openTxCh(gCpswInterVlanAppObj.hEnet,
                              gCpswInterVlanAppObj.coreKey,
                              gCpswInterVlanAppObj.coreId,
                              &gCpswInterVlanAppObj.txChNum,
                              &gCpswInterVlanAppObj.hTxCh,
                              &cpswTxChCfg);
        EnetAppUtils_assert(NULL != gCpswInterVlanAppObj.hTxCh);

#ifndef AUTO_RECLAIM_TXCQ
        status = EnetDma_enableTxEvent(gCpswInterVlanAppObj.hTxCh);
#endif

        if (status != ENET_SOK)
        {
            EnetAppUtils_freeTxCh(gCpswInterVlanAppObj.hEnet,
                                  gCpswInterVlanAppObj.coreKey,
                                  gCpswInterVlanAppObj.coreId,
                                  gCpswInterVlanAppObj.txChNum);
            EnetAppUtils_assert(BFALSE);
        }
    }

    if (status == ENET_SOK)
    {
        CpswApp_initRxReadyPktQ();
    }

    return status;
}

static void CpswApp_pktRxTx(void)
{
    int32_t status = ENET_SOK;
    EnetDma_PktQ txSubmitQ;
    EnetDma_PktQ rxFreeQ;
    EnetDma_Pkt *pktInfo;
    EthVlanFrame *frame;
    uint32_t rxReadyCnt;
    int32_t semStatus;
    uint32_t iterationCount = 0U;
    volatile bool testDone = BFALSE;
    EthFwOsal_TaskHandle hTask;

    /*  The packet handling loop is structured as described below
     *  The outer loop waits for semaphore notification from RX completion
     *  ISR. At this moment the Rx completion interrupt is disabled and we switch
     *  into processing loop, handling all packets received, Tx completed packets.
     *
     *  For each packet received, we perform header inspection, header modification and
     *  enqueue packet for transmission.
     *
     *  We then handle Tx packet completion - for each TX packets transmitted, when the buffer
     *  is reclaimed we then add it back to Rx Free Q (this implicitly acts SW flow control)
     */
    EnetQueue_initQ(&gCpswInterVlanAppObj.rxReadyQ);

    hTask = EthFwOsal_getTaskSelf();
    if (hTask != NULL)
    {
        EthFwOsal_setTaskPrio(hTask, 5);
    }
    else
    {
        status = ENET_EFAIL;
        appLogPrintf("Couldn't retrieve the cuurent task handle\n", status);
    }
    do
    {
        /*TODO: add a voluntary yield here for other tasks at same priority to run
         * if we are continuously handling a stream of packets*/

        /* rxReadyQ should be empty here as we would have processed and queued all packets from
         * last iteration. */
        EnetAppUtils_assert(EnetQueue_getQCount(&gCpswInterVlanAppObj.rxReadyQ) == 0);

        /* Initialize the Tx Submission, Rx Free SW Qs*/
        EnetQueue_initQ(&txSubmitQ);
        EnetQueue_initQ(&rxFreeQ);

        /* Get the packets received so far */
        rxReadyCnt = CpswApp_receivePkts();

        if (0U == rxReadyCnt)
        {
            /* If we get here, it means that we have processed all received packets.
             * Need to switch on interrupt notification for Rx pkts and move to
             * blocked state until new pkt reception is signaled
             */

            /* Re-enable the Rx completion notification from ISR here */
            EnetDma_enableRxEvent(gCpswInterVlanAppObj.hIngRxFlow);

#ifndef AUTO_RECLAIM_TXCQ
            /* Re-enable the Tx completion notification from ISR here */
            EnetDma_enableTxEvent(gCpswInterVlanAppObj.hTxCh);
#endif

            /* Pend on semaphore notification event from Rx Completion ISR */
            semStatus = EthFwOsal_pendSemaphore(gCpswInterVlanAppObj.completionSem, RX_TX_COMPLETION_TIMEOUT);
            if (semStatus != ETHFWOSAL_SUCCESS)
            {
                iterationCount++;
                if ((iterationCount & 0x7FFU) == 0U)
                {
#ifdef APP_PRINTPKTCNT
                    appLogPrintf("# pkts=%d\n", gCpswInterVlanAppObj.num_pkts);
#endif
                }
            }
        }
        else
        {
            gCpswInterVlanAppObj.num_pkts += rxReadyCnt;

            /* Consume the received packets and release them */
            pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gCpswInterVlanAppObj.rxReadyQ);

            /*Processing loop for received packets, we will perform Header inspection,
             * mangling and enqueue the same for Tx Processing */
            while (NULL != pktInfo)
            {
                /* Consume the received packets and release them */
                /* TODO: Invalidate for the header portion*/
                frame = (EthVlanFrame *)pktInfo->sgList.list[0].bufPtr;
                EthFwOsal_invCache((Ptr)frame, PKT_HEADER_SIZE);

                /* Step2: Modify SA, DA fields of Ethernet header*/

                /* Modify DASA
                 * Modify VLAN ID
                 * Modify TTL
                 */
                memcpy(frame->hdr.dstMac, testDstMacAddr, ENET_MAC_ADDR_LEN);
                memcpy(frame->hdr.srcMac, gCpswInterVlanAppObj.hostMacAddr, ENET_MAC_ADDR_LEN);
                status = EthFrame_changeVlanId(frame,
                                               APP_INTERVLAN_EGRESS_VLANID);
                if (status == ENET_SOK)
                {
                    /* Decrement TTL by 1U*/
                    EthFrame_decrementTTL(frame);

                    /* TODO: Flush the cache contents for header region */
                    EthFwOsal_wbInvCache((Ptr)frame, PKT_HEADER_SIZE);

                    /* Step3: Enq the modified frame for transmission */
                    EnetQueue_enq(&txSubmitQ, &pktInfo->node);

                    pktInfo = (EnetDma_Pkt *)EnetQueue_deq(&gCpswInterVlanAppObj.rxReadyQ);
                }
            } /* end of while loop*/

            /* Submit the list of Packets to be Tx to HW */
            status = EnetDma_submitTxPktQ(gCpswInterVlanAppObj.hTxCh,
                                                  &txSubmitQ);
            EnetAppUtils_assert(EnetQueue_getQCount(&txSubmitQ) == 0U);
        } /*end of else condition*/

#ifndef AUTO_RECLAIM_TXCQ
        /* Reclaim Transmitted packets and add the reclaimed buffers to Rx FreeQ */
        status = EnetDma_retrieveTxPktQ(gCpswInterVlanAppObj.hTxCh, &rxFreeQ);
        EnetDma_submitRxPktQ(gCpswInterVlanAppObj.hIngRxFlow,
                                     &rxFreeQ);
        EnetAppUtils_assert(EnetQueue_getQCount(&rxFreeQ) == 0U);
#endif
    }
    while (testDone != BTRUE);
}

static uint32_t CpswApp_receivePkts(void)
{
    int32_t status;
    uint32_t rxReadyCnt = 0U;

    /* we fetch all Rx ready pkts from HW Q and populate the list in SW Q
     * The SW Q would have been empty before this call, as we would have serviced all
     * pending Rx packets from SW Q before going back to check on the HW Q status*/

    /* Retrieve any CPSW packets which are ready */
    status = EnetDma_retrieveRxPktQ(gCpswInterVlanAppObj.hIngRxFlow, &gCpswInterVlanAppObj.rxReadyQ);
    if (status == ENET_SOK)
    {
        rxReadyCnt = EnetQueue_getQCount(&gCpswInterVlanAppObj.rxReadyQ);
    }
    else
    {
        appLogPrintf("%s() failed to retrieve pkts: %d\n", __func__, status);
    }

    return rxReadyCnt;
}

void EthSwInterVlan_setupRouting(Enet_Type enetType,
                                 uint32_t swInterVlanTaskPri)
{
    EthFwOsal_TaskParams params;

    gCpswInterVlanAppObj.enetType = enetType;

    /* Initialize the task params. Set the task priority higher than the
     * default priority (1) */
    EthFwOsal_initTaskParams(&params);
    params.priority = swInterVlanTaskPri;
    params.stack = gAppTskStackMain;
    params.stacksize = sizeof(gAppTskStackMain);
    params.name = "Eth SW InterVLAN Task";

    task = EthFwOsal_createTask(&CpswApp_InterVlanRouting, &params);
    if (task == NULL)
    {
        OS_stop();
    }
}

/* end of file */
