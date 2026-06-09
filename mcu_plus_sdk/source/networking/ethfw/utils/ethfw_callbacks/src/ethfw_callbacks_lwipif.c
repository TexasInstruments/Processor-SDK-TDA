/*
 *
 * Copyright (c) 2021-2024 Texas Instruments Incorporated
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

/**
 *  \file ethfw_callbacks_lwipif.c
 *
 *  \brief Default LwIP interface callbacks for Ethernet Firmware application.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x501

#include <enet.h>
#include <utils/include/enet_apputils.h>
#include <utils/include/enet_appmemutils_cfg.h>
#include <utils/include/enet_appmemutils.h>
#include <utils/include/enet_mcm.h>
#include <lwipif2enet_appif.h>

#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_callbacks/include/ethfw_callbacks_lwipif.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <ethremotecfg/server/include/ethfw_arp.h>
#if defined(ETHFW_VEPA_SUPPORT)
#include <ethremotecfg/server/include/ethfw_vepa.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*! Remote app packet poll period in milliseconds */
#define CPSW_REMOTE_APP_PACKET_POLL_PERIOD_US         (1000U)

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if defined(ETHFW_VEPA_SUPPORT)
static int32_t EthFwCallbacks_setupPacketDuplicationRoute(Enet_Handle hEnet,
                                                          uint32_t coreKey,
                                                          uint32_t coreId,
                                                          EnetMcm_HandleInfo *handleInfo,
                                                          LwipifEnetAppIf_RxConfig *rxCfg,
                                                          EnetDma_RxChHandle *hRxFlow,
                                                          uint32_t *rxFlowStartIdx,
                                                          uint32_t *flowIdx);

static void EthFwCallbacks_teardownPacketDuplicationRoute(Enet_Handle hEnet,
                                                          uint32_t coreKey,
                                                          uint32_t coreId,
                                                          EnetDma_RxChHandle hRxFlow,
                                                          uint32_t flowIdx,
                                                          Lwip2EnetAppIf_FreePktInfo *freePktInfo);

static bool EthFwCallbacks_handlePacketDuplicationRxPktFxn(struct netif *netif,
                                                           struct pbuf *pbuf);
#elif defined(ETHFW_PROXY_ARP_HANDLING)
static int32_t EthFwCallbacks_setupArpRoute(Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            EnetMcm_HandleInfo *handleInfo,
                                            LwipifEnetAppIf_RxConfig *arpRxCfg,
                                            EnetDma_RxChHandle *hRxFlow,
                                            uint32_t *rxFlowStartIdx,
                                            uint32_t *flowIdx);

static void EthFwCallbacks_teardownArpRoute(Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            EnetDma_RxChHandle hRxFlow,
                                            uint32_t rxFlowStartIdx,
                                            uint32_t flowIdx,
                                            Lwip2EnetAppIf_FreePktInfo *freePktInfo);

bool EthFwCallbacks_handleArpRxPktFxn(struct netif *netif,
                                      struct pbuf *pbuf);
#endif

static void EthFwCallbacks_handleRxErrPkt(struct netif *netif,
                                          struct pbuf *pbuf,
                                          uint32_t errCode);

/* ========================================================================== */
/*                          Extern variables                                  */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if !defined(MCU_PLUS_SDK)
/* CPSW error codes. Relevant when MAC port's CEF, CMF and/or CSF are enabled */
static const char *gEthFwCallbacks_errCodeInfo[] =
{
    [ENET_UDMA_CPSW_PKT_ERR_NONE]           = "No error",
    [ENET_UDMA_CPSW_PKT_ERR_CRC]            = "CRC error",
    [ENET_UDMA_CPSW_PKT_ERR_CODEALIGN]      = "Code/align error",
    [ENET_UDMA_CPSW_PKT_ERR_SHORT]          = "Short frame (no code/align/CRC error)",
    [ENET_UDMA_CPSW_PKT_ERR_FRAG_CRC]       = "Fragment with CRC error",
    [ENET_UDMA_CPSW_PKT_ERR_FRAG_CODEALIGN] = "Fragment with code/align error",
    [ENET_UDMA_CPSW_PKT_ERR_LONG]           = "Long frame",
    [ENET_UDMA_CPSW_PKT_ERR_JABBER_CRC]     = "Long with CRC error",
    [ENET_UDMA_CPSW_PKT_ERR_JABBER_ALIGN]   = "Long with code/align error",
    [ENET_UDMA_CPSW_PKT_ERR_MAC_CONTROL]    = "MAC control frame",
    [ENET_UDMA_CPSW_PKT_ERR_MACCONTROL_CRC] = "MAC control frame with CRC error",
    [ENET_UDMA_CPSW_PKT_ERR_MACCONTROL_CODEALIGN]   = "MAC control frame with code/align error",
    [ENET_UDMA_CPSW_PKT_ERR_MACCONTROL_SHORT_FRAG]  = "Short MAC control frame with CRC/code/align error",
    [ENET_UDMA_CPSW_PKT_ERR_MACCONTROL_LONG_JABBER] = "Long MAC control frame with CRC/code/align error",
    [ENET_UDMA_CPSW_PKT_ERR_RSV1]           = "Reserved",
    [ENET_UDMA_CPSW_PKT_ERR_RSV2]           = "Reserved",
};
#endif

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthFwCallbacks_lwipifCpswGetHandle(Enet_Type enetType,
                                        uint32_t instId,
                                        LwipifEnetAppIf_GetHandleInArgs *inArgs,
                                        LwipifEnetAppIf_GetHandleOutArgs *outArgs)
{
#if !defined(MCU_PLUS_SDK)
    LwipifEnetAppIf_RxHandleInfo *rxInfo;
    LwipifEnetAppIf_RxConfig *rxCfg;
    EnetMcm_CmdIf mcmCmdIf;
    EnetMcm_HandleInfo handleInfo;
    EnetPer_AttachCoreOutArgs attachInfo;
    EnetUdma_OpenTxChPrms cpswTxChCfg;
    EnetUdma_OpenRxFlowPrms cpswRxFlowCfg;
#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
    bool useRingMon = BTRUE;
    EnetUdma_UdmaRingPrms *pFqRingPrms;
#endif
    uint8_t *macAddr;
    uint32_t coreId = EnetSoc_getCoreId();
    bool useDefaultFlow = BTRUE;    /* Must handle the default flow */
#if defined(ETHFW_PROXY_ARP_HANDLING) || defined(ETHFW_VEPA_SUPPORT)
    int32_t status;
#endif

    /* Get MCM command interface */
    EnetMcm_getCmdIf(enetType, &mcmCmdIf);
    EnetAppUtils_assert(mcmCmdIf.hMboxCmd != NULL);
    EnetAppUtils_assert(mcmCmdIf.hMboxResponse != NULL);

    /* Get CPSW and UDMA driver handles */
    EnetMcm_acquireHandleInfo(&mcmCmdIf, &handleInfo);

    /* Attach this core, if not done already */
    EnetMcm_coreAttach(&mcmCmdIf, coreId, &attachInfo);

    /* Open TX channel */
    EnetUdma_initTxChParams(&cpswTxChCfg);
    cpswTxChCfg.hUdmaDrv            = handleInfo.hUdmaDrv;
    cpswTxChCfg.numTxPkts           = inArgs->txCfg.numPackets;
    cpswTxChCfg.cbArg               = inArgs->txCfg.cbArg;
    cpswTxChCfg.notifyCb            = inArgs->txCfg.notifyCb;
    cpswTxChCfg.useProxy            = BTRUE;
    cpswTxChCfg.disableCacheOpsFlag = BFALSE;

    EnetAppUtils_openTxCh(handleInfo.hEnet,
                          attachInfo.coreKey,
                          coreId,
                          &outArgs->txInfo.txChNum,
                          &outArgs->txInfo.hTxChannel,
                          &cpswTxChCfg);

    /* Open first RX channel/flow */
    rxCfg  = &inArgs->rxCfg[0U];
    rxInfo = &outArgs->rxInfo[0U];

    EnetUdma_initRxFlowParams(&cpswRxFlowCfg);
    cpswRxFlowCfg.notifyCb  = rxCfg->notifyCb;
    cpswRxFlowCfg.numRxPkts = rxCfg->numPackets;
    cpswRxFlowCfg.hUdmaDrv  = handleInfo.hUdmaDrv;
    cpswRxFlowCfg.cbArg     = rxCfg->cbArg;
    cpswRxFlowCfg.useProxy  = BTRUE;
#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
    /* Use ring monitor for the CQ ring of RX flow */
    pFqRingPrms                  = &cpswRxFlowCfg.udmaChPrms.fqRingPrms;
    pFqRingPrms->useRingMon      = useRingMon;
    pFqRingPrms->ringMonCfg.mode = TISCI_MSG_VALUE_RM_MON_MODE_THRESHOLD;
    /* Ring mon low threshold */
#if defined _DEBUG_
    /* In debug mode as CPU is processing lesser packets per event, keep threshold more */
    pFqRingPrms->ringMonCfg.data0 = (rxCfg->numPackets - 10U);
#else
    pFqRingPrms->ringMonCfg.data0 = (rxCfg->numPackets - 20U);
#endif
    /* Ring mon high threshold - to get only low  threshold event, setting high threshold as more than ring depth*/
    pFqRingPrms->ringMonCfg.data1 = rxCfg->numPackets;
#endif
    cpswRxFlowCfg.disableCacheOpsFlag = BFALSE;
    cpswRxFlowCfg.rxFlowMtu           = attachInfo.rxMtu;

    EnetAppUtils_openRxFlow(enetType,
                            handleInfo.hEnet,
                            attachInfo.coreKey,
                            coreId,
                            useDefaultFlow,
                            &rxInfo->rxFlowStartIdx,
                            &rxInfo->rxFlowIdx,
                            &rxInfo->macAddr[0U],
                            &rxInfo->hRxFlow,
                            &cpswRxFlowCfg);

    outArgs->coreId          = coreId;
    outArgs->coreKey         = attachInfo.coreKey;
    outArgs->handleArg       = (void *)handleInfo.hEnet;
    outArgs->hostPortRxMtu   = attachInfo.rxMtu;
    ENET_UTILS_ARRAY_COPY(outArgs->txMtu, attachInfo.txMtu);
    outArgs->hUdmaDrv        = handleInfo.hUdmaDrv;
    outArgs->print           = &EnetAppUtils_print;
#if !defined(MCU_PLUS_SDK)
    outArgs->isPortLinkedFxn = &EthFwCallbacks_isPortLinked;
#endif
#if defined(ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA)
    outArgs->txCsumOffloadEn = BFALSE;
    outArgs->rxCsumOffloadEn = BFALSE;
#else
    outArgs->txCsumOffloadEn = BTRUE;
    outArgs->rxCsumOffloadEn = BTRUE;
#endif

    rxInfo->disableEvent = BTRUE;
    outArgs->timerPeriodUs = CPSW_REMOTE_APP_PACKET_POLL_PERIOD_US;

    /* Let LwIP interface use optimized processing where TX packets are relinquished
     * in next TX submit call */
    outArgs->txInfo.disableEvent = BTRUE;

    outArgs->txInfo.txPortNum = ENET_MAC_PORT_INV;

#if defined(MCU_PLUS_SDK)
    macAddr = &rxInfo->macAddr[0U][0U];
#else
    macAddr = &rxInfo->macAddr[0U];
#endif

    ETHFWTRACE_INFO("Host MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
                    macAddr[0] & 0xFF, macAddr[1] & 0xFF, macAddr[2] & 0xFF,
                    macAddr[3] & 0xFF, macAddr[4] & 0xFF, macAddr[5] & 0xFF);

    rxInfo->handlePktFxn = NULL;

#if !defined(MCU_PLUS_SDK)
    rxInfo->handleErrPktFxn = EthFwCallbacks_handleRxErrPkt;
#endif

    /* Mode of packet operation should be either interrupt or polling 
     * based, not both. So ensure both Rx and Tx mode have either interrupt
     * enabled or disabled. based on this configuration, LwipIf layer
     * will enable one of these two modes. */
    EnetAppUtils_assert(outArgs->txInfo.disableEvent == outArgs->rxInfo[0U].disableEvent);

#if defined(ETHFW_VEPA_SUPPORT)
    /* Open second RX channel/flow for broadcast/multicast packet duplication */
    rxCfg  = &inArgs->rxCfg[1U];
    rxInfo = &outArgs->rxInfo[1U];

    rxInfo->disableEvent = BTRUE;

    status = EthFwCallbacks_setupPacketDuplicationRoute(handleInfo.hEnet,
                                                        attachInfo.coreKey,
                                                        outArgs->coreId,
                                                        &handleInfo,
                                                        rxCfg,
                                                        &rxInfo->hRxFlow,
                                                        &rxInfo->rxFlowStartIdx,
                                                        &rxInfo->rxFlowIdx);
    if (status != ENET_SOK)
    {
        /* Just print an error as EthFw and its clients will continue to run with
         * limited functionality */
        ETHFWTRACE_ERR(status, "Failed to setup route for packet duplication");
    }
    else
    {
        rxInfo->handlePktFxn = EthFwCallbacks_handlePacketDuplicationRxPktFxn;
    }
#elif defined(ETHFW_PROXY_ARP_HANDLING)
    /* Open second RX channel/flow for ARP */
    rxCfg  = &inArgs->rxCfg[1U];
    rxInfo = &outArgs->rxInfo[1U];

    rxInfo->disableEvent = BTRUE;

    status = EthFwCallbacks_setupArpRoute(handleInfo.hEnet,
                                          attachInfo.coreKey,
                                          outArgs->coreId,
                                          &handleInfo,
                                          rxCfg,
                                          &rxInfo->hRxFlow,
                                          &rxInfo->rxFlowStartIdx,
                                          &rxInfo->rxFlowIdx);
    if (status != ENET_SOK)
    {
        /* Just print an error as EthFw and its clients will continue to run with
         * limited functionality */
        ETHFWTRACE_ERR(status, "Failed to setup route for ARP request packets");
    }
    else
    {
        rxInfo->handlePktFxn = EthFwCallbacks_handleArpRxPktFxn;
    }
#endif

    /* Mode of packet operation should be either interrupt or polling 
     * based, not both. So ensure modes of both flows have either interrupt
     * enabled or disabled. based on this configuration, LwipIf layer
     * will enable one of these two modes. */
    EnetAppUtils_assert(outArgs->rxInfo[0U].disableEvent == outArgs->rxInfo[1U].disableEvent);

    rxInfo->handleErrPktFxn = EthFwCallbacks_handleRxErrPkt;
#else
    /* Sitara devices shouldn't call this function */
    EnetAppUtils_assert(BFALSE);
#endif
}

void EthFwCallbacks_lwipifCpswReleaseHandle(Enet_Type enetType,
                                            uint32_t instId,
                                            LwipifEnetAppIf_ReleaseHandleInfo *releaseInfo)
{
#if !defined(MCU_PLUS_SDK)
    LwipifEnetAppIf_RxHandleInfo *rxInfo;
    Lwip2EnetAppIf_FreePktInfo *freePktInfo;
    EnetMcm_CmdIf mcmCmdIf;
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    Enet_Handle hEnet = (Enet_Handle)releaseInfo->handleArg;
    bool useDefaultFlow = BTRUE;    /* Must handle the default flow */

    /* Get MCM command interface */
    EnetMcm_getCmdIf(enetType, &mcmCmdIf);
    EnetAppUtils_assert(mcmCmdIf.hMboxCmd != NULL);
    EnetAppUtils_assert(mcmCmdIf.hMboxResponse != NULL);

#if defined(ETHFW_VEPA_SUPPORT)
    /* Tear-down packet duplication route (RX flow + ALE classifier) */
    rxInfo = &releaseInfo->rxInfo[1U];
    freePktInfo = &releaseInfo->rxFreePkt[1U];

    EthFwCallbacks_teardownPacketDuplicationRoute(hEnet,
                                                  releaseInfo->coreKey,
                                                  releaseInfo->coreId,
                                                  rxInfo->hRxFlow,
                                                  rxInfo->rxFlowIdx,
                                                  freePktInfo);
#elif defined(ETHFW_PROXY_ARP_HANDLING)
    /* Tear-down ARP route (RX flow + ALE classifier) */
    rxInfo = &releaseInfo->rxInfo[1U];
    freePktInfo = &releaseInfo->rxFreePkt[1U];

    EthFwCallbacks_teardownArpRoute(hEnet,
                                    releaseInfo->coreKey,
                                    releaseInfo->coreId,
                                    rxInfo->hRxFlow,
                                    rxInfo->rxFlowStartIdx,
                                    rxInfo->rxFlowIdx,
                                    freePktInfo);
#endif

    /* Close TX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);
    EnetAppUtils_closeTxCh(hEnet,
                           releaseInfo->coreKey,
                           releaseInfo->coreId,
                           &fqPktInfoQ,
                           &cqPktInfoQ,
                           releaseInfo->txInfo.hTxChannel,
                           releaseInfo->txInfo.txChNum);
    releaseInfo->txFreePkt.cb(releaseInfo->txFreePkt.cbArg, &fqPktInfoQ, &cqPktInfoQ);

    /* Close first RX channel/flow */
    rxInfo = &releaseInfo->rxInfo[0U];
    freePktInfo = &releaseInfo->rxFreePkt[0U];

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);
    EnetAppUtils_closeRxFlow(enetType,
                             hEnet,
                             releaseInfo->coreKey,
                             releaseInfo->coreId,
                             useDefaultFlow,
                             &fqPktInfoQ,
                             &cqPktInfoQ,
                             rxInfo->rxFlowStartIdx,
                             rxInfo->rxFlowIdx,
                             rxInfo->macAddr,
                             rxInfo->hRxFlow);
    freePktInfo->cb(freePktInfo->cbArg, &fqPktInfoQ, &cqPktInfoQ);

    EnetMcm_coreDetach(&mcmCmdIf, releaseInfo->coreId, releaseInfo->coreKey);
    EnetMcm_releaseHandleInfo(&mcmCmdIf);
    EnetMcm_releaseCmdIf(enetType, &mcmCmdIf);
#else
    /* Sitara devices shouldn't call this function */
    EnetAppUtils_assert(BFALSE);
#endif
}

#if defined(ETHFW_MONITOR_SUPPORT)
void LwipifEnetAppCb_openDma(LwipifEnetAppIf_GetHandleInArgs *inArgs,
                             LwipifEnetAppIf_GetHandleOutArgs *outArgs)
{
    LwipifEnetAppIf_RxHandleInfo *rxInfo;
    LwipifEnetAppIf_RxConfig *rxCfg;
    EnetUdma_OpenTxChPrms cpswTxChCfg;
    EnetUdma_OpenRxFlowPrms cpswRxFlowCfg;
#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
    EnetUdma_UdmaRingPrms *pFqRingPrms;
    bool useRingMon = BTRUE;
#endif
    EnetDma_Handle hDma;
    int32_t status = ENET_SOK;
    uint32_t coreId = EnetSoc_getCoreId();
    Enet_Handle hEnet = (Enet_Handle)outArgs->handleArg;

#if defined(ETHFW_PROXY_ARP_HANDLING)
    const uint8_t bcastAddr[ENET_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CpswAle_SetPolicerEntryInArgs polInArgs;
    CpswAle_SetPolicerEntryOutArgs polOutArgs;
    Enet_IoctlPrms prms;
#endif

    EnetAppUtils_assert(hEnet != ENET_SOK);

    /* Open TX channel */
    EnetUdma_initTxChParams(&cpswTxChCfg);
    EnetAppUtils_setCommonTxChPrms(&cpswTxChCfg);

    cpswTxChCfg.hUdmaDrv            = outArgs->hUdmaDrv;
    cpswTxChCfg.numTxPkts           = inArgs->txCfg.numPackets;
    cpswTxChCfg.cbArg               = inArgs->txCfg.cbArg;
    cpswTxChCfg.notifyCb            = inArgs->txCfg.notifyCb;
    cpswTxChCfg.useProxy            = BTRUE;
    cpswTxChCfg.chNum               = outArgs->txInfo.txChNum;

    hDma = Enet_getDmaHandle(hEnet);
    EnetAppUtils_assert(hDma != NULL);

    outArgs->txInfo.hTxChannel = EnetUdma_openTxCh(hDma, &cpswTxChCfg);
    EnetAppUtils_assert(outArgs->txInfo.hTxChannel != NULL);

    /* Open first RX channel/flow */
    rxInfo = &outArgs->rxInfo[0U];
    rxCfg = &inArgs->rxCfg[0U];

    EnetUdma_initRxFlowParams(&cpswRxFlowCfg);
    EnetAppUtils_setCommonRxFlowPrms(&cpswRxFlowCfg);

    cpswRxFlowCfg.notifyCb  = rxCfg->notifyCb;
    cpswRxFlowCfg.numRxPkts = rxCfg->numPackets;
    cpswRxFlowCfg.hUdmaDrv  = outArgs->hUdmaDrv;;
    cpswRxFlowCfg.cbArg     = rxCfg->cbArg;
    cpswRxFlowCfg.useProxy  = BTRUE;
    cpswRxFlowCfg.startIdx  = rxInfo->rxFlowStartIdx;
    cpswRxFlowCfg.flowIdx   = rxInfo->rxFlowIdx;
    rxInfo->handlePktFxn    = NULL;


#if (UDMA_SOC_CFG_UDMAP_PRESENT == 1)
    /* Use ring monitor for the CQ ring of RX flow */
    pFqRingPrms = &cpswRxFlowCfg.udmaChPrms.fqRingPrms;
    pFqRingPrms->useRingMon = useRingMon;
    pFqRingPrms->ringMonCfg.mode = TISCI_MSG_VALUE_RM_MON_MODE_THRESHOLD;
    /* Ring mon low threshold */

#if defined _DEBUG_
    /* In debug mode as CPU is processing lesser packets per event, keep threshold more */
    pFqRingPrms->ringMonCfg.data0 = (rxCfg->numPackets - 10U);
#else
    pFqRingPrms->ringMonCfg.data0 = (rxCfg->numPackets - 20U);
#endif
    /* Ring mon high threshold - to get only low  threshold event, setting high threshold as more than ring depth*/
    pFqRingPrms->ringMonCfg.data1 = rxCfg->numPackets;
#endif

    rxInfo->hRxFlow = EnetUdma_openRxFlow(hDma, &cpswRxFlowCfg);
    EnetAppUtils_assert(rxInfo->hRxFlow != NULL);

    status = EnetAppUtils_regDfltRxFlow(hEnet,
                                        outArgs->coreKey,
                                        coreId,
                                        rxInfo->rxFlowStartIdx,
                                        rxInfo->rxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);

#if defined(ETHFW_VEPA_SUPPORT) || defined(ETHFW_PROXY_ARP_HANDLING)
    /* Open second RX flow */
    rxCfg  = &inArgs->rxCfg[1U];
    rxInfo = &outArgs->rxInfo[1U];

    EnetUdma_initRxFlowParams(&cpswRxFlowCfg);
    EnetAppUtils_setCommonRxFlowPrms(&cpswRxFlowCfg);

    cpswRxFlowCfg.notifyCb  = rxCfg->notifyCb;
    cpswRxFlowCfg.numRxPkts = rxCfg->numPackets;
    cpswRxFlowCfg.hUdmaDrv  = outArgs->hUdmaDrv;;
    cpswRxFlowCfg.cbArg     = rxCfg->cbArg;
    cpswRxFlowCfg.useProxy  = BTRUE;
    cpswRxFlowCfg.startIdx  = rxInfo->rxFlowStartIdx;
    cpswRxFlowCfg.flowIdx   = rxInfo->rxFlowIdx;

    rxInfo->hRxFlow = EnetUdma_openRxFlow(hDma, &cpswRxFlowCfg);
    EnetAppUtils_assert(rxInfo->hRxFlow != NULL);

#if defined(ETHFW_VEPA_SUPPORT)
    status = EthFwVepa_setPacketDuplicationFlowIdx(rxInfo->rxFlowIdx);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                      "Failed to set flow for packet duplication", status);

    rxInfo->handlePktFxn = EthFwCallbacks_handlePacketDuplicationRxPktFxn;
#elif defined(ETHFW_PROXY_ARP_HANDLING)
    /* Set policer for ARP EtherType + Broadcast address matching */
    if (status == ENET_SOK)
    {
        /* Set policer params for ARP EtherType matching */
        polInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;
        polInArgs.policerMatch.etherType = ETHTYPE_ARP;
        polInArgs.policerMatch.portIsTrunk = BFALSE;

        /* Set policer params for broadcast address matching
         * Note that policer IOCTL takes a port number not a port mask which is what
         * we really need (mask = CPSW_ALE_ALL_PORTS_MASK).  So, we would have to
         * amend the ALE broadcast entry with another IOCTL, but the EthFw library
         * creates such broadcast entry (see EthFw_setAleBcastEntry(), so we
         * intentionally won't do it here. */
        polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_MACDST;
        polInArgs.policerMatch.dstMacAddrInfo.portNum = CPSW_ALE_HOST_PORT_NUM;
        polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = 0U;
        EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], &bcastAddr[0]);

        polInArgs.threadIdEn = BTRUE;
        polInArgs.threadId   = rxInfo->rxFlowIdx;
        polInArgs.peakRateInBitsPerSec   = 0U;
        polInArgs.commitRateInBitsPerSec = 0U;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &polInArgs, &polOutArgs);

        ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_SET_POLICER, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to add ARP policer: %d\n", status);
    }
    rxInfo->handlePktFxn = EthFwCallbacks_handleArpRxPktFxn;
#endif
#endif
}

void LwipifEnetAppCb_closeDma(LwipifEnetAppIf_ReleaseHandleInfo *releaseInfo)
{
    Enet_Handle hEnet = (Enet_Handle)releaseInfo->handleArg;
    LwipifEnetAppIf_RxHandleInfo *rxInfo;
    Lwip2EnetAppIf_FreePktInfo *freePktInfo;
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    int32_t status = ENET_SOK;

#if defined(ETHFW_PROXY_ARP_HANDLING)
    const uint8_t bcastAddr[ENET_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CpswAle_DelPolicerEntryInArgs polInArgs;
    Enet_IoctlPrms prms;
#endif

    EnetAppUtils_assert(hEnet != ENET_SOK);

#if defined(ETHFW_VEPA_SUPPORT) || defined(ETHFW_PROXY_ARP_HANDLING)
    /* Close second RX channel/flow */
    freePktInfo = &releaseInfo->rxFreePkt[1U];
    rxInfo = &releaseInfo->rxInfo[1U];

#if defined(ETHFW_VEPA_SUPPORT)
    /* Clear the packet duplication flow */
    EthFwVepa_clearPacketDuplicationFlowIdx();
#elif defined(ETHFW_PROXY_ARP_HANDLING)
     /* Set policer params for ARP EtherType matching */
     polInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;
     polInArgs.policerMatch.etherType = ETHTYPE_ARP;
     polInArgs.policerMatch.portIsTrunk = BFALSE;

     /* Set policer params for broadcast address matching */
     polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_MACDST;
     polInArgs.policerMatch.dstMacAddrInfo.portNum = CPSW_ALE_HOST_PORT_NUM;
     polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = 0U;
     EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], &bcastAddr[0]);

     /* We didn't add broadcast entry for all ports, so we won't delete anything either */
     polInArgs.aleEntryMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;

     ENET_IOCTL_SET_IN_ARGS(&prms, &polInArgs);

     /* Delete ALE policer */
     ENET_IOCTL(hEnet, releaseInfo->coreId, CPSW_ALE_IOCTL_DEL_POLICER, &prms, status);
     ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to delete ARP policer");
#endif

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    status = EnetUdma_closeRxFlow(rxInfo->hRxFlow, &fqPktInfoQ, &cqPktInfoQ);
    EnetAppUtils_assert(status == ENET_SOK);

    freePktInfo->cb(freePktInfo->cbArg, &fqPktInfoQ, &cqPktInfoQ);
#endif

        /* Close TX channel */
        EnetQueue_initQ(&fqPktInfoQ);
        EnetQueue_initQ(&cqPktInfoQ);

        EnetDma_disableTxEvent(releaseInfo->txInfo.hTxChannel);
        status = EnetUdma_closeTxCh(releaseInfo->txInfo.hTxChannel, &fqPktInfoQ, &cqPktInfoQ);
        EnetAppUtils_assert(status == ENET_SOK);

        releaseInfo->txFreePkt.cb(releaseInfo->txFreePkt.cbArg, &fqPktInfoQ, &cqPktInfoQ);

    /* Close first RX channel/flow */
    freePktInfo = &releaseInfo->rxFreePkt[0U];
    rxInfo = &releaseInfo->rxInfo[0U];

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    status = EnetAppUtils_unregDfltRxFlow(hEnet,
                                          releaseInfo->coreKey,
                                          releaseInfo->coreId,
                                          rxInfo->rxFlowStartIdx,
                                          rxInfo->rxFlowIdx);
    EnetAppUtils_assert(status == ENET_SOK);

    status = EnetUdma_closeRxFlow(rxInfo->hRxFlow, &fqPktInfoQ, &cqPktInfoQ);
    EnetAppUtils_assert(status == ENET_SOK);

    freePktInfo->cb(freePktInfo->cbArg, &fqPktInfoQ, &cqPktInfoQ);
}
#endif

#if defined(ETHFW_VEPA_SUPPORT)
/* Setup flow for packet duplication */
static int32_t EthFwCallbacks_setupPacketDuplicationRoute(Enet_Handle hEnet,
                                                          uint32_t coreKey,
                                                          uint32_t coreId,
                                                          EnetMcm_HandleInfo *handleInfo,
                                                          LwipifEnetAppIf_RxConfig *rxCfg,
                                                          EnetDma_RxChHandle *hRxFlow,
                                                          uint32_t *rxFlowStartIdx,
                                                          uint32_t *flowIdx)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    EnetUdma_OpenRxFlowPrms rxFlowCfg;
    int32_t status = ENET_SOK;

    if (hDma == NULL)
    {
        status = ENET_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to get Enet DMA handle for packet duplication flow");
    }

    if (status == ENET_SOK)
    {
        status = EnetAppUtils_allocRxFlow(hEnet, coreKey, coreId, rxFlowStartIdx, flowIdx);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to alloc RX flow for packet duplication traffic");
    }

    /* Open RX Flow */
    if (status == ENET_SOK)
    {
        EnetUdma_initRxFlowParams(&rxFlowCfg);
        EnetAppUtils_setCommonRxFlowPrms(&rxFlowCfg);
        rxFlowCfg.startIdx  = *rxFlowStartIdx;
        rxFlowCfg.flowIdx   = *flowIdx;
        rxFlowCfg.notifyCb  = rxCfg->notifyCb;
        rxFlowCfg.cbArg     = rxCfg->cbArg;
        rxFlowCfg.numRxPkts = rxCfg->numPackets;
        rxFlowCfg.hUdmaDrv  = handleInfo->hUdmaDrv;
        rxFlowCfg.useProxy  = BTRUE;

        *hRxFlow = EnetUdma_openRxFlow(hDma, &rxFlowCfg);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to open RX flow for packet duplication traffic");
    }

    /* Set packet duplication flow id */
    if (status == ENET_SOK)
    {
        status = EthFwVepa_setPacketDuplicationFlowIdx(*flowIdx);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                          "Failed to set flow for packet duplication");
    }

    return status;
}

/* Remove flow for packet duplication */
static void EthFwCallbacks_teardownPacketDuplicationRoute(Enet_Handle hEnet,
                                                          uint32_t coreKey,
                                                          uint32_t coreId,
                                                          EnetDma_RxChHandle hRxFlow,
                                                          uint32_t flowIdx,
                                                          Lwip2EnetAppIf_FreePktInfo *freePktInfo)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    int32_t status;

    /* Clear the packet duplication flow */
    EthFwVepa_clearPacketDuplicationFlowIdx();

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    if (hRxFlow != NULL)
    {
        EnetDma_disableRxEvent(hRxFlow);

        /* Close RX flow */
        status = EnetUdma_closeRxFlow(hRxFlow, &fqPktInfoQ, &cqPktInfoQ);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to close RX flow for packet duplication traffic");

        /* Free RX flow only if channel was closed */
        if (status == ENET_SOK)
        {
            status = EnetAppUtils_freeRxFlow(hEnet, coreKey, coreId, flowIdx);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                              "Failed to free RX flow used for packet duplication traffic");

            freePktInfo->cb(freePktInfo->cbArg, &fqPktInfoQ, &cqPktInfoQ);
        }
    }
}

static bool EthFwCallbacks_handlePacketDuplicationRxPktFxn(struct netif *netif,
                                                           struct pbuf *pbuf)
{
    int32_t status = ETHFW_SOK;
    struct eth_hdr *ethHdr;
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_DEBUG)
    uint8_t *dstMac;
    uint8_t *srcMac;
#endif

#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_DEBUG)
    dstMac = &ethHdr->dest.addr[0];
    srcMac = &ethHdr->src.addr[0];

    ETHFWTRACE_DBG("Received packet for duplication, "
                    "dstMac %02x:%02x:%02x:%02x:%02x:%02x and "
                    "srcMac %02x:%02x:%02x:%02x:%02x:%02x",
                    dstMac[0] & 0xFF, dstMac[1] & 0xFF,
                    dstMac[2] & 0xFF, dstMac[3] & 0xFF,
                    dstMac[4] & 0xFF, dstMac[5] & 0xFF,
                    srcMac[0] & 0xFF, srcMac[1] & 0xFF,
                    srcMac[2] & 0xFF, srcMac[3] & 0xFF,
                    srcMac[4] & 0xFF, srcMac[5] & 0xFF);
#endif

    ethHdr = (struct eth_hdr *)(pbuf->payload);
    ETHFWTRACE_ERR_IF((EnetUtils_isMcastAddr(&ethHdr->dest.addr[0]) != BTRUE), ENET_EFAIL,
                     "Error: Unicast packet received on packet duplication flow");

    status = EthFwVepa_sendRaw(netif, pbuf, &ethHdr->src, &ethHdr->dest);
    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                     "Failed to send packet for packet duplication");

    /* Lwip stack should also consume the packet */
    /* Returning false implies that packet will be consumed by Lwip stack */
    return BFALSE;
}

#elif defined(ETHFW_PROXY_ARP_HANDLING)
static int32_t EthFwCallbacks_setupArpRoute(Enet_Handle hEnet,
                                     uint32_t coreKey,
                                     uint32_t coreId,
                                     EnetMcm_HandleInfo *handleInfo,
                                     LwipifEnetAppIf_RxConfig *arpRxCfg,
                                     EnetDma_RxChHandle *hRxFlow,
                                     uint32_t *rxFlowStartIdx,
                                     uint32_t *flowIdx)
{
    EnetDma_Handle hDma = Enet_getDmaHandle(hEnet);
    const uint8_t bcastAddr[ENET_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    EnetUdma_OpenRxFlowPrms rxFlowCfg;
    CpswAle_SetPolicerEntryInArgs polInArgs;
    CpswAle_SetPolicerEntryOutArgs polOutArgs;
    Enet_IoctlPrms prms;
    int32_t status = ENET_SOK;

    if (hDma == NULL)
    {
        status = ENET_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to get Enet DMA handle");
    }

    if (status == ENET_SOK)
    {
        status = EnetAppUtils_allocRxFlow(hEnet, coreKey, coreId, rxFlowStartIdx, flowIdx);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to alloc RX flow for ARP traffic");
    }

    /* Open RX Flow */
    if (status == ENET_SOK)
    {
        EnetUdma_initRxFlowParams(&rxFlowCfg);
        EnetAppUtils_setCommonRxFlowPrms(&rxFlowCfg);
        rxFlowCfg.startIdx  = *rxFlowStartIdx;
        rxFlowCfg.flowIdx   = *flowIdx;
        rxFlowCfg.notifyCb  = arpRxCfg->notifyCb;
        rxFlowCfg.cbArg     = arpRxCfg->cbArg;
        rxFlowCfg.numRxPkts = arpRxCfg->numPackets;
        rxFlowCfg.hUdmaDrv  = handleInfo->hUdmaDrv;
        rxFlowCfg.useProxy  = BTRUE;

        *hRxFlow = EnetUdma_openRxFlow(hDma, &rxFlowCfg);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to open RX flow for ARP traffic");
    }

    /* Set policer for ARP EtherType + Broadcast address matching */
    if (status == ENET_SOK)
    {
        /* Set policer params for ARP EtherType matching */
        polInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;
        polInArgs.policerMatch.etherType = ETHTYPE_ARP;
        polInArgs.policerMatch.portIsTrunk = BFALSE;

        /* Set policer params for broadcast address matching
         * Note that policer IOCTL takes a port number not a port mask which is what
         * we really need (mask = CPSW_ALE_ALL_PORTS_MASK).  So, we would have to
         * amend the ALE broadcast entry with another IOCTL, but the EthFw library
         * creates such broadcast entry (see EthFw_setAleBcastEntry(), so we
         * intentionally won't do it here. */
        polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_MACDST;
        polInArgs.policerMatch.dstMacAddrInfo.portNum = CPSW_ALE_HOST_PORT_NUM;
        polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = 0U;
        EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], &bcastAddr[0]);

        polInArgs.threadIdEn = BTRUE;
        polInArgs.threadId   = *flowIdx;
        polInArgs.peakRateInBitsPerSec   = 0U;
        polInArgs.commitRateInBitsPerSec = 0U;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &polInArgs, &polOutArgs);

        ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_SET_POLICER, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to add ARP policer");
    }

    return status;
}

static void EthFwCallbacks_teardownArpRoute(Enet_Handle hEnet,
                                            uint32_t coreKey,
                                            uint32_t coreId,
                                            EnetDma_RxChHandle hRxFlow,
                                            uint32_t rxFlowStartIdx,
                                            uint32_t flowIdx,
                                            Lwip2EnetAppIf_FreePktInfo *freePktInfo)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    const uint8_t bcastAddr[ENET_MAC_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    CpswAle_DelPolicerEntryInArgs polInArgs;
    Enet_IoctlPrms prms;
    int32_t status;

    /* Set policer params for ARP EtherType matching */
    polInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;
    polInArgs.policerMatch.etherType = ETHTYPE_ARP;
    polInArgs.policerMatch.portIsTrunk = BFALSE;

    /* Set policer params for broadcast address matching */
    polInArgs.policerMatch.policerMatchEnMask |= CPSW_ALE_POLICER_MATCH_MACDST;
    polInArgs.policerMatch.dstMacAddrInfo.portNum = CPSW_ALE_HOST_PORT_NUM;
    polInArgs.policerMatch.dstMacAddrInfo.addr.vlanId = 0U;
    EnetUtils_copyMacAddr(&polInArgs.policerMatch.dstMacAddrInfo.addr.addr[0], &bcastAddr[0]);

    /* We didn't add broadcast entry for all ports, so we won't delete anything either */
    polInArgs.aleEntryMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;

    ENET_IOCTL_SET_IN_ARGS(&prms, &polInArgs);

    /* Delete ALE policer */
    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_DEL_POLICER, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to delete ARP policer");

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    if (hRxFlow != NULL)
    {
        EnetDma_disableRxEvent(hRxFlow);

        /* Close RX flow */
        status = EnetUdma_closeRxFlow(hRxFlow, &fqPktInfoQ, &cqPktInfoQ);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to close RX flow used for ARP traffic");

        /* Free RX flow only if channel was closed */
        if (status == ENET_SOK)
        {
            status = EnetAppUtils_freeRxFlow(hEnet, coreKey, coreId, flowIdx);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                              "Failed to free RX flow used for ARP traffic");

            freePktInfo->cb(freePktInfo->cbArg, &fqPktInfoQ, &cqPktInfoQ);
        }
    }
}

bool EthFwCallbacks_handleArpRxPktFxn(struct netif *netif,
                                      struct pbuf *pbuf)
{
    struct eth_hdr *ethHdr;
    struct eth_vlan_hdr *vlanHdr;
    struct etharp_hdr *ethArpHdr;
    struct eth_addr hwAddr;
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_DEBUG)
    uint8_t *dstMac;
#endif
    ip4_addr_t srcIp;
    ip4_addr_t dstIp;
    uint8_t *payload;
    uint16_t vlanId = 0U;
    bool handled = BFALSE;
    int32_t status;

    payload = (uint8_t *)pbuf->payload;

    ethHdr = (struct eth_hdr *)payload;
    payload += SIZEOF_ETH_HDR;

    if (ethHdr->type == lwip_ntohs(ETHTYPE_VLAN))
    {
        vlanHdr = (struct eth_vlan_hdr *)payload;
        vlanId = VLAN_ID(vlanHdr);
        payload += SIZEOF_VLAN_HDR;
    }

    ethArpHdr = (struct etharp_hdr *)payload;

    IPADDR_WORDALIGNED_COPY_TO_IP4_ADDR_T(&srcIp, &ethArpHdr->sipaddr);
    IPADDR_WORDALIGNED_COPY_TO_IP4_ADDR_T(&dstIp, &ethArpHdr->dipaddr);

#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_DEBUG)
    dstMac = &ethHdr->dest.addr[0];

    ETHFWTRACE_DBG("Received ARP dstIp %s, dstMac %02x:%02x:%02x:%02x:%02x:%02x vlanId %u",
                   ip4addr_ntoa(&dstIp),
                   dstMac[0] & 0xFF, dstMac[1] & 0xFF, dstMac[2] & 0xFF,
                   dstMac[3] & 0xFF, dstMac[4] & 0xFF, dstMac[5] & 0xFF,
                   vlanId);
#endif

    status = EthFwArp_getHwAddr(&dstIp, &hwAddr, vlanId);
    if (status == ETHFW_SOK)
    {
        EthFwArp_sendRaw(netif,
                         &hwAddr,
                         &ethArpHdr->shwaddr,
                         &hwAddr,
                         &dstIp,
                         &ethArpHdr->shwaddr,
                         &srcIp,
                         vlanId,
                         ARP_REPLY);

#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_DEBUG)
        ETHFWTRACE_DBG("Sent ARP response");
#endif
        handled = BTRUE;
    }

    return handled;
}
#endif

#if !defined(MCU_PLUS_SDK)
const char *EthFwCallbacks_errPktCodeStr(uint32_t errCode)
{
    const char *str = "Unexpected error code";

    if (errCode <= (uint32_t)ENET_UDMA_CPSW_PKT_ERR_RSV2)
    {
        str = gEthFwCallbacks_errCodeInfo[errCode];
    }

    return str;
}

static void EthFwCallbacks_handleRxErrPkt(struct netif *netif,
                                          struct pbuf *pbuf,
                                          uint32_t errCode)
{
    if (errCode <= (uint32_t)ENET_UDMA_CPSW_PKT_ERR_RSV2)
    {
        ETHFWTRACE_ERR(ETHFW_EFAIL, "netif %c%c%u: %s (%u)",
                       netif->name[0], netif->name[1], netif->num,
                       gEthFwCallbacks_errCodeInfo[errCode], errCode);
    }
    else
    {
        ETHFWTRACE_ERR(ETHFW_EUNEXPECTED, "netif %c%c%u: unexpected error code %u",
                           netif->name[0], netif->name[1], netif->num, errCode);
    }
}
#endif
