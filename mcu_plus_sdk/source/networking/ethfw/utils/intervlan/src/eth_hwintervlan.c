/*
 *
 * Copyright (c) 2019-2025 Texas Instruments Incorporated
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
 * \file     eth_hwintervlan.c
 *
 * \brief    This file contains the hardware interVLAN utils implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <include/per/cpsw.h>
#include <utils/include/enet_apputils.h>

#if !defined(MCU_PLUS_SDK)
#include <ti/board/board.h>
#endif

#include <utils/intervlan/include/eth_hwintervlan.h>
#include <utils/console_io/include/app_log.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#if defined(SOC_J721E)
#define CPSW_TEST_INTERVLAN_INGRESS_PORT_NUM            (ENET_MAC_PORT_8)
#define CPSW_TEST_INTERVLAN_EGRESS_PORT_NUM             (ENET_MAC_PORT_3)
#elif defined(SOC_J7200)
#define CPSW_TEST_INTERVLAN_INGRESS_PORT_NUM            (ENET_MAC_PORT_2)
#define CPSW_TEST_INTERVLAN_EGRESS_PORT_NUM             (ENET_MAC_PORT_3)
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
#define CPSW_TEST_INTERVLAN_INGRESS_PORT_NUM            (ENET_MAC_PORT_5)
#define CPSW_TEST_INTERVLAN_EGRESS_PORT_NUM             (ENET_MAC_PORT_3)
#elif defined(SOC_AM62PX) || defined(SOC_AM62DX)  || defined(SOC_J722S)
#define CPSW_TEST_INTERVLAN_INGRESS_PORT_NUM            (ENET_MAC_PORT_1)
#define CPSW_TEST_INTERVLAN_EGRESS_PORT_NUM             (ENET_MAC_PORT_2)
#else
#error "Unsupported SoC"
#endif

#define CPSW_TEST_INTERVLAN_INGRESS_VLANID              (100)
#define CPSW_TEST_INTERVLAN_EGRESS_VLANID               (200)
#define CPSW_TEST_INTERVLAN_HOSTPORT_PVID               (1)
#define CPSW_TEST_INTERVLAN_MACPORT_PVID_BASE           (400)
#define CPSW_TEST_INTERVLAN_DEFAULT_SHORTIPG_THRESHOLD  (11)

#define CPSW_TEST_INTERVLAN_IPV6_HOP_LIMIT_OFFSET       (7)
#define CPSW_TEST_INTERVLAN_IPV4_TTL_OFFSET             (8)
#define CPSW_TEST_INTERVLAN_IPV6_ETHERTYPE              (0x86DD)
#define CPSW_TEST_INTERVLAN_IPV4_ETHERTYPE              (0x0800)
#define CPSW_TEST_IPV6_OCTET2ARRAY(x)                   0x00, x
#define CPSW_TEST_IPV6_HEXTET2ARRAY(x)                  ((x) & 0xFF00) >> 8, ((x) & 0xFF)
#define CPSW_TEST_IPV4_NXT_HDR_TCP                      (6U)
#define CPSW_TEST_IPV4_NXT_HDR_UDP                      (17U)
#define CPSW_TEST_IPV4_NXT_HDR_ICMP                     (1U)
#define CPSW_TEST_IPV4_NXT_HDR_IGMP                     (2U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

static int32_t CpswAppInterVlan_setInterVlanUniEgress(Enet_Handle hEnet,
                                                      CpswMacPort_InterVlanRouteId expectedAllocRouteId,
                                                      uint32_t *pNumRoutesUsed,
                                                      EnetCfgServer_InterVlanConfig *pInterVlanCfg);

static int32_t CpswAppInterVlan_setShortIPG(Enet_Handle hEnet);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static uint8_t testSrcIpv6Addr[16] = {CPSW_TEST_IPV6_HEXTET2ARRAY(0x2000),
                                      CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                      CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                      CPSW_TEST_IPV6_OCTET2ARRAY(0x1),
                                      CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                      CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                      CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                      CPSW_TEST_IPV6_OCTET2ARRAY(0x2)};
uint8_t testDstIpv6Addr[16] = {CPSW_TEST_IPV6_HEXTET2ARRAY(0x2000),
                               CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                               CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                               CPSW_TEST_IPV6_OCTET2ARRAY(0x2),
                               CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                               CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                               CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                               CPSW_TEST_IPV6_OCTET2ARRAY(0x2)};
uint8_t testDstIpv6Addr2[16] = {CPSW_TEST_IPV6_HEXTET2ARRAY(0x2000),
                                CPSW_TEST_IPV6_OCTET2ARRAY(0x1),
                                CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                CPSW_TEST_IPV6_OCTET2ARRAY(0x4),
                                CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                CPSW_TEST_IPV6_OCTET2ARRAY(0x2)};
uint8_t testDstIpv6AddrMcast[] = {CPSW_TEST_IPV6_HEXTET2ARRAY(0x2FFF),
                                  CPSW_TEST_IPV6_OCTET2ARRAY(0x1),
                                  CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                  CPSW_TEST_IPV6_OCTET2ARRAY(0x4),
                                  CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                  CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                  CPSW_TEST_IPV6_OCTET2ARRAY(0x0),
                                  CPSW_TEST_IPV6_OCTET2ARRAY(0x2)};

static uint8_t testHostMacAddr[] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x02};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthHwInterVlan_setOpenPrms(Cpsw_Cfg *pCpswCfg)
{
    Enet_MacPort i;

    /* pCpswCfg->aleCfg.policerGlobalCfg.policingEn should be true for interVLAN.
     * Set to false to exercise driver internal logic to auto enable policer when interVLAN API
     * is invoked */
    pCpswCfg->aleCfg.policerGlobalCfg.policingEn      = BTRUE;
    pCpswCfg->hostPortCfg.passPriorityTaggedUnchanged = BTRUE;

    pCpswCfg->aleCfg.modeFlags = CPSW_ALE_CFG_MODULE_EN;

    pCpswCfg->aleCfg.policerGlobalCfg.redDropEn    = BFALSE;
    pCpswCfg->aleCfg.policerGlobalCfg.yellowDropEn = BFALSE;
    pCpswCfg->aleCfg.policerGlobalCfg.policerNoMatchMode = CPSW_ALE_POLICER_NOMATCH_MODE_GREEN;

    pCpswCfg->aleCfg.vlanCfg.aleVlanAwareMode  = BTRUE;
    pCpswCfg->aleCfg.vlanCfg.cpswVlanAwareMode = BTRUE;
    pCpswCfg->aleCfg.vlanCfg.unknownVlanMemberListMask = 0U;

    pCpswCfg->aleCfg.nwSecCfg.hostOuiNoMatchDeny = BFALSE;
    pCpswCfg->aleCfg.nwSecCfg.vid0ModeEn         = BTRUE;
    pCpswCfg->aleCfg.nwSecCfg.ipPktCfg.dfltNxtHdrWhitelistEn = BTRUE;
    pCpswCfg->aleCfg.nwSecCfg.ipPktCfg.ipNxtHdrWhitelistCnt  = 2U;
    pCpswCfg->aleCfg.nwSecCfg.ipPktCfg.ipNxtHdrWhitelist[0]  = CPSW_TEST_IPV4_NXT_HDR_TCP;
    pCpswCfg->aleCfg.nwSecCfg.ipPktCfg.ipNxtHdrWhitelist[1]  = CPSW_TEST_IPV4_NXT_HDR_UDP;

    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].learningCfg.noLearn             = BFALSE;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].vlanCfg.dropUntagged            = BFALSE;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.unregMcastFloodMask     = 0U;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.regMcastFloodMask       = CPSW_ALE_ALL_PORTS_MASK;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.forceUntaggedEgressMask = CPSW_ALE_ALL_PORTS_MASK;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.noLearnMask             = 0U;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.vidIngressCheck         = BFALSE;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.limitIPNxtHdr           = BFALSE;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.disallowIPFrag          = BFALSE;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.vlanIdInfo.tagType      = ENET_VLAN_TAG_TYPE_INNER;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.vlanIdInfo.vlanId       = CPSW_TEST_INTERVLAN_HOSTPORT_PVID;
    pCpswCfg->aleCfg.portCfg[CPSW_ALE_HOST_PORT_NUM].pvidCfg.vlanMemberList          = CPSW_ALE_ALL_PORTS_MASK;

    for (i = ENET_MAC_PORT_FIRST; i < CPSW_ALE_NUM_MAC_PORTS; i++)
    {
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].learningCfg.noLearn             = BFALSE;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].vlanCfg.dropUntagged            = BFALSE;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.unregMcastFloodMask     = 0U;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.regMcastFloodMask       = CPSW_ALE_ALL_PORTS_MASK;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.forceUntaggedEgressMask = CPSW_ALE_ALL_PORTS_MASK;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.noLearnMask             = 0U;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.vidIngressCheck         = BFALSE;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.limitIPNxtHdr           = BFALSE;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.disallowIPFrag          = BFALSE;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.vlanIdInfo.tagType      = ENET_VLAN_TAG_TYPE_INNER;
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.vlanIdInfo.vlanId       = CPSW_TEST_INTERVLAN_MACPORT_PVID_BASE + ENET_MACPORT_NORM(i);
        pCpswCfg->aleCfg.portCfg[CPSW_ALE_MACPORT_TO_ALEPORT(i)].pvidCfg.vlanMemberList          = CPSW_ALE_ALL_PORTS_MASK;
    }

    pCpswCfg->hostPortCfg.vlanCfg.portPri = 7U;
    pCpswCfg->hostPortCfg.vlanCfg.portCfi = 0U;
    pCpswCfg->hostPortCfg.vlanCfg.portVID = CPSW_TEST_INTERVLAN_HOSTPORT_PVID;
    pCpswCfg->vlanCfg.vlanAware = BTRUE;
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

static int32_t CpswAppInterVlan_addUniEgressAleTableEntries(Enet_Handle hEnet,
                                                            EnetCfgServer_InterVlanConfig *pInterVlanCfg)
{
    int32_t status;
    Enet_IoctlPrms prms;
    uint32_t setUcastOutArgs;
    CpswAle_SetUcastEntryInArgs setUcastInArgs;

    memcpy(&setUcastInArgs.addr.addr[0U], testHostMacAddr,
           sizeof(setUcastInArgs.addr.addr));
    setUcastInArgs.addr.vlanId = 0U;
    setUcastInArgs.info.portNum = CPSW_ALE_HOST_PORT_NUM;
    setUcastInArgs.info.blocked = BFALSE;
    setUcastInArgs.info.secure  = BFALSE;
    setUcastInArgs.info.super   = BFALSE;
    setUcastInArgs.info.ageable = BFALSE;
    setUcastInArgs.info.trunk   = BFALSE;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, &setUcastInArgs, &setUcastOutArgs);

    ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_ALE_IOCTL_ADD_UCAST, &prms, status);
    if (status != ENET_SOK)
    {
        appLogPrintf("%s() failed CPSW_ALE_IOCTL_ADD_UCAST: %d\n", __func__, status);
    }

    if (status == ENET_SOK)
    {
        CpswAle_VlanEntryInfo inArgs;
        uint32_t outArgs;

        inArgs.vlanIdInfo.vlanId       = pInterVlanCfg->ingVlanId;
        inArgs.vlanIdInfo.tagType      = ENET_VLAN_TAG_TYPE_INNER;
        inArgs.vlanMemberList          = CpswAppInterVlan_getIngressVlanMembershipMask(pInterVlanCfg);
        inArgs.unregMcastFloodMask     = CpswAppInterVlan_getIngressVlanMembershipMask(pInterVlanCfg);
        inArgs.regMcastFloodMask       = CpswAppInterVlan_getIngressVlanMembershipMask(pInterVlanCfg);
        inArgs.forceUntaggedEgressMask = 0U;
        inArgs.noLearnMask             = 0U;
        inArgs.vidIngressCheck         = BFALSE;
        inArgs.limitIPNxtHdr           = BFALSE;
        inArgs.disallowIPFrag          = BFALSE;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_ALE_IOCTL_ADD_VLAN, &prms, status);
        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed ADD_VLAN ioctl failed: %d\n", __func__, status);
        }
    }

    if (status == ENET_SOK)
    {
        CpswAle_VlanEntryInfo inArgs;
        uint32_t outArgs;

        inArgs.vlanIdInfo.vlanId       = pInterVlanCfg->egrVlanId;
        inArgs.vlanIdInfo.tagType      = ENET_VLAN_TAG_TYPE_INNER;
        inArgs.vlanMemberList          = CpswAppInterVlan_getEgressVlanMembershipMask(pInterVlanCfg);
        inArgs.unregMcastFloodMask     = CpswAppInterVlan_getEgressVlanMembershipMask(pInterVlanCfg);
        inArgs.regMcastFloodMask       = CpswAppInterVlan_getEgressVlanMembershipMask(pInterVlanCfg);
        inArgs.forceUntaggedEgressMask = 0U;
        inArgs.noLearnMask             = 0U;
        inArgs.vidIngressCheck         = BFALSE;
        inArgs.limitIPNxtHdr           = BFALSE;
        inArgs.disallowIPFrag          = BFALSE;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_ALE_IOCTL_ADD_VLAN, &prms, status);
        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed ADD_VLAN ioctl failed: %d\n", __func__, status);
        }
    }

    return status;
}

static int32_t CpswAppInterVlan_setInterVlanUniEgress(Enet_Handle hEnet,
                                                      CpswMacPort_InterVlanRouteId expectedAllocRouteId,
                                                      uint32_t *pNumRoutesUsed,
                                                      EnetCfgServer_InterVlanConfig *pInterVlanCfg)
{
    int32_t status;
    Enet_IoctlPrms prms;
    Cpsw_SetInterVlanRouteUniEgressInArgs inArgs;
    Cpsw_SetInterVlanRouteUniEgressOutArgs outArgs;

    *pNumRoutesUsed = 0U;
    status = CpswAppInterVlan_addUniEgressAleTableEntries(hEnet, pInterVlanCfg);

    if (ENET_SOK == status)
    {
        /* Set to invalid id and confirm outArgs populated correctly after IOCTL
         * called
         */
        outArgs.egressPortRouteId = CPSW_MACPORT_INTERVLAN_ROUTEID_LAST;

        inArgs.inPktMatchCfg.packetMatchEnMask = 0U;

        inArgs.inPktMatchCfg.ingressPort = (Enet_MacPort)pInterVlanCfg->ingPortNum;
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_PORT;

        inArgs.inPktMatchCfg.dstIpInfo.ipAddrType                = CPSW_ALE_IPADDR_CLASSIFIER_IPV6;
        inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.numLSBIgnoreBits = 8U;
        memcpy(inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.ipv6Addr, testDstIpv6Addr, sizeof(inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.ipv6Addr));
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_IPDST;

        memcpy(inArgs.inPktMatchCfg.dstMacAddrInfo.addr.addr, testHostMacAddr, sizeof(inArgs.inPktMatchCfg.dstMacAddrInfo.addr.addr));
        inArgs.inPktMatchCfg.dstMacAddrInfo.addr.vlanId = 0U;
        inArgs.inPktMatchCfg.dstMacAddrInfo.portNum     = CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->egrPortNum);
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_MACDST;

        inArgs.inPktMatchCfg.srcIpInfo.ipAddrType                = CPSW_ALE_IPADDR_CLASSIFIER_IPV6;
        inArgs.inPktMatchCfg.srcIpInfo.ipv6Info.numLSBIgnoreBits = 8U;
        memcpy(inArgs.inPktMatchCfg.srcIpInfo.ipv6Info.ipv6Addr, testSrcIpv6Addr, sizeof(inArgs.inPktMatchCfg.srcIpInfo.ipv6Info.ipv6Addr));
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_IPSRC;

        memcpy(inArgs.inPktMatchCfg.srcMacAddrInfo.addr.addr, &pInterVlanCfg->srcMacAddr[0U], sizeof(inArgs.inPktMatchCfg.srcMacAddrInfo.addr.addr));
        inArgs.inPktMatchCfg.srcMacAddrInfo.addr.vlanId = 0U;
        inArgs.inPktMatchCfg.srcMacAddrInfo.portNum     = CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->ingPortNum);
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_MACSRC;

        inArgs.inPktMatchCfg.etherType = CPSW_TEST_INTERVLAN_IPV6_ETHERTYPE;
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_ETHERTYPE;

        inArgs.inPktMatchCfg.vlanId     = pInterVlanCfg->ingVlanId;
        inArgs.inPktMatchCfg.ttlCheckEn = BTRUE;

        inArgs.egressCfg.egressPort                       = (Enet_MacPort)pInterVlanCfg->egrPortNum;
        inArgs.egressCfg.outPktModCfg.decrementTTL        = BTRUE;
        inArgs.egressCfg.outPktModCfg.forceUntaggedEgress = BFALSE;
        inArgs.egressCfg.outPktModCfg.replaceDASA         = BTRUE;
        memcpy(inArgs.egressCfg.outPktModCfg.srcAddr, testHostMacAddr, sizeof(inArgs.egressCfg.outPktModCfg.srcAddr));
        memcpy(inArgs.egressCfg.outPktModCfg.dstAddr, &pInterVlanCfg->dstMacAddr[0U], sizeof(inArgs.egressCfg.outPktModCfg.dstAddr));
        inArgs.egressCfg.outPktModCfg.vlanId              = pInterVlanCfg->egrVlanId;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS, &prms, status);
        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS: %d\n", __func__, status);
        }
    }

    if (status == ENET_SOK)
    {
        *pNumRoutesUsed += 1U;
        // EnetAppUtils_assert(outArgs.egressPortRouteId == expectedAllocRouteId);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.port == CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->ingPortNum));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.portIsTrunk == BFALSE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpcode == (1 + (outArgs.egressPortRouteId - CPSW_MACPORT_INTERVLAN_ROUTEID_FIRST)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.ttlCheckEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.dstPortMask == (1 << CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->egrPortNum)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.policerMatchEnMask == (CPSW_ALE_POLICER_MATCH_PORT |
                                                                                       CPSW_ALE_POLICER_MATCH_MACSRC |
                                                                                       CPSW_ALE_POLICER_MATCH_MACDST |
                                                                                       CPSW_ALE_POLICER_MATCH_IVLAN |
                                                                                       CPSW_ALE_POLICER_MATCH_ETHERTYPE |
                                                                                       CPSW_ALE_POLICER_MATCH_IPSRC |
                                                                                       CPSW_ALE_POLICER_MATCH_IPDST));
    }

    if (status == ENET_SOK)
    {
        /* Add another route to send packet out on ingress port.
         * Only destIP changes. Rest of params remain same
         */
        inArgs.inPktMatchCfg.dstIpInfo.ipAddrType                = CPSW_ALE_IPADDR_CLASSIFIER_IPV6;
        inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.numLSBIgnoreBits = 8U;
        memcpy(inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.ipv6Addr, testDstIpv6Addr2, sizeof(inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.ipv6Addr));
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_IPDST;

        inArgs.egressCfg.egressPort = (Enet_MacPort)pInterVlanCfg->ingPortNum;

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS, &prms, status);

        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS: %d\n", __func__, status);
        }
    }

    if (status == ENET_SOK)
    {
        /* Do not increment numRoutesUsed as it is a different egress port */
        // EnetAppUtils_assert(outArgs.egressPortRouteId == expectedAllocRouteId);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.port == CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->ingPortNum));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.portIsTrunk == BFALSE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpcode == (1 + (outArgs.egressPortRouteId - CPSW_MACPORT_INTERVLAN_ROUTEID_FIRST)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.ttlCheckEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.dstPortMask == (1 << CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->ingPortNum)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.policerMatchEnMask == (CPSW_ALE_POLICER_MATCH_PORT |
                                                                                           CPSW_ALE_POLICER_MATCH_MACSRC |
                                                                                           CPSW_ALE_POLICER_MATCH_MACDST |
                                                                                           CPSW_ALE_POLICER_MATCH_IVLAN |
                                                                                           CPSW_ALE_POLICER_MATCH_ETHERTYPE |
                                                                                           CPSW_ALE_POLICER_MATCH_IPSRC |
                                                                                           CPSW_ALE_POLICER_MATCH_IPDST));
    }

    /* Add a route in opposite direction to enable bidirectional intervlan switching */
    if (ENET_SOK == status)
    {
        /* Set to invalid id and confirm outArgs populated correctly after IOCTL
         * called
         */
        outArgs.egressPortRouteId = CPSW_MACPORT_INTERVLAN_ROUTEID_LAST;

        inArgs.inPktMatchCfg.packetMatchEnMask = 0U;

        inArgs.inPktMatchCfg.ingressPort = (Enet_MacPort)pInterVlanCfg->egrPortNum;
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_PORT;

        inArgs.inPktMatchCfg.dstIpInfo.ipAddrType                = CPSW_ALE_IPADDR_CLASSIFIER_IPV6;
        inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.numLSBIgnoreBits = 8U;
        memcpy(inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.ipv6Addr, testSrcIpv6Addr, sizeof(inArgs.inPktMatchCfg.dstIpInfo.ipv6Info.ipv6Addr));
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_IPDST;

        memcpy(inArgs.inPktMatchCfg.dstMacAddrInfo.addr.addr, testHostMacAddr, sizeof(inArgs.inPktMatchCfg.dstMacAddrInfo.addr.addr));
        inArgs.inPktMatchCfg.dstMacAddrInfo.addr.vlanId = 0U;
        inArgs.inPktMatchCfg.dstMacAddrInfo.portNum = CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->egrPortNum);

        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_MACDST;

        inArgs.inPktMatchCfg.srcIpInfo.ipAddrType                = CPSW_ALE_IPADDR_CLASSIFIER_IPV6;
        inArgs.inPktMatchCfg.srcIpInfo.ipv6Info.numLSBIgnoreBits = 8U;
        memcpy(inArgs.inPktMatchCfg.srcIpInfo.ipv6Info.ipv6Addr, testDstIpv6Addr, sizeof(inArgs.inPktMatchCfg.srcIpInfo.ipv6Info.ipv6Addr));
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_IPSRC;

        memcpy(inArgs.inPktMatchCfg.srcMacAddrInfo.addr.addr, &pInterVlanCfg->dstMacAddr[0U], sizeof(inArgs.inPktMatchCfg.srcMacAddrInfo.addr.addr));
        inArgs.inPktMatchCfg.srcMacAddrInfo.addr.vlanId = 0U;
        inArgs.inPktMatchCfg.srcMacAddrInfo.portNum     = CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->egrPortNum);
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_MACSRC;

        inArgs.inPktMatchCfg.etherType = 0x86DD;
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_ETHERTYPE;

        inArgs.inPktMatchCfg.vlanId     = pInterVlanCfg->egrPortNum;
        inArgs.inPktMatchCfg.ttlCheckEn = BTRUE;

        inArgs.egressCfg.egressPort                       = (Enet_MacPort)pInterVlanCfg->ingPortNum;
        inArgs.egressCfg.outPktModCfg.decrementTTL        = BTRUE;
        inArgs.egressCfg.outPktModCfg.forceUntaggedEgress = BFALSE;
        inArgs.egressCfg.outPktModCfg.replaceDASA         = BTRUE;
        inArgs.egressCfg.outPktModCfg.vlanId              = pInterVlanCfg->ingVlanId;
        memcpy(inArgs.egressCfg.outPktModCfg.srcAddr, testHostMacAddr, sizeof(inArgs.egressCfg.outPktModCfg.srcAddr));
        memcpy(inArgs.egressCfg.outPktModCfg.dstAddr, &pInterVlanCfg->srcMacAddr[0U], sizeof(inArgs.egressCfg.outPktModCfg.dstAddr));

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS, &prms, status);
        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS: %d\n", __func__, status);
        }
    }

    if (status == ENET_SOK)
    {
        // EnetAppUtils_assert(outArgs.egressPortRouteId == (expectedAllocRouteId + 1));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.port == CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->egrPortNum));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.portIsTrunk == BFALSE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpcode == (1 + (outArgs.egressPortRouteId - CPSW_MACPORT_INTERVLAN_ROUTEID_FIRST)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.ttlCheckEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.dstPortMask == (1 << CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->ingPortNum)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.policerMatchEnMask == (CPSW_ALE_POLICER_MATCH_PORT |
                                                                                           CPSW_ALE_POLICER_MATCH_MACSRC |
                                                                                           CPSW_ALE_POLICER_MATCH_MACDST |
                                                                                           CPSW_ALE_POLICER_MATCH_IVLAN |
                                                                                           CPSW_ALE_POLICER_MATCH_ETHERTYPE |
                                                                                           CPSW_ALE_POLICER_MATCH_IPSRC |
                                                                                           CPSW_ALE_POLICER_MATCH_IPDST));
    }

    /* Add IPv4 intervlan switching route */
    if (ENET_SOK == status)
    {
        /* Set to invalid id and confirm outArgs populated correctly after IOCTL
         * called
         */
        outArgs.egressPortRouteId = CPSW_MACPORT_INTERVLAN_ROUTEID_LAST;

        inArgs.inPktMatchCfg.packetMatchEnMask = 0U;

        inArgs.inPktMatchCfg.srcIpInfo.ipAddrType                = CPSW_ALE_IPADDR_CLASSIFIER_IPV4;
        inArgs.inPktMatchCfg.srcIpInfo.ipv4Info.numLSBIgnoreBits = 0U;
        memcpy(inArgs.inPktMatchCfg.srcIpInfo.ipv4Info.ipv4Addr, &pInterVlanCfg->srcIpv4Addr[0U], sizeof(inArgs.inPktMatchCfg.srcIpInfo.ipv4Info.ipv4Addr));
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_IPSRC;

        inArgs.inPktMatchCfg.dstIpInfo.ipAddrType                = CPSW_ALE_IPADDR_CLASSIFIER_IPV4;
        inArgs.inPktMatchCfg.dstIpInfo.ipv4Info.numLSBIgnoreBits = 0U;
        memcpy(inArgs.inPktMatchCfg.dstIpInfo.ipv4Info.ipv4Addr, &pInterVlanCfg->dstIpv4Addr[0U], sizeof(inArgs.inPktMatchCfg.dstIpInfo.ipv4Info.ipv4Addr));
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_IPDST;

        memcpy(inArgs.inPktMatchCfg.dstMacAddrInfo.addr.addr, testHostMacAddr, sizeof(inArgs.inPktMatchCfg.dstMacAddrInfo.addr.addr));
        inArgs.inPktMatchCfg.dstMacAddrInfo.addr.vlanId = 0U;
        inArgs.inPktMatchCfg.dstMacAddrInfo.portNum = CPSW_ALE_HOST_PORT_NUM;

        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_MACDST;

        memcpy(inArgs.inPktMatchCfg.srcMacAddrInfo.addr.addr, &pInterVlanCfg->srcMacAddr[0U], sizeof(inArgs.inPktMatchCfg.srcMacAddrInfo.addr.addr));
        inArgs.inPktMatchCfg.srcMacAddrInfo.addr.vlanId = 0U;
        inArgs.inPktMatchCfg.srcMacAddrInfo.portNum     = CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->ingPortNum);
        inArgs.inPktMatchCfg.packetMatchEnMask |= CPSW_INTERVLAN_INGRESSPKT_MATCH_MACSRC;

        inArgs.inPktMatchCfg.vlanId = pInterVlanCfg->ingVlanId;
        inArgs.inPktMatchCfg.ttlCheckEn = BTRUE;

        inArgs.egressCfg.egressPort                       = (Enet_MacPort)pInterVlanCfg->egrPortNum;
        inArgs.egressCfg.outPktModCfg.decrementTTL        = BTRUE;
        inArgs.egressCfg.outPktModCfg.forceUntaggedEgress = BFALSE;
        inArgs.egressCfg.outPktModCfg.replaceDASA         = BTRUE;
        inArgs.egressCfg.outPktModCfg.vlanId              = pInterVlanCfg->egrVlanId;
        memcpy(inArgs.egressCfg.outPktModCfg.srcAddr, testHostMacAddr, sizeof(inArgs.egressCfg.outPktModCfg.srcAddr));
        memcpy(inArgs.egressCfg.outPktModCfg.dstAddr, &pInterVlanCfg->dstMacAddr[0U], sizeof(inArgs.egressCfg.outPktModCfg.dstAddr));

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &inArgs, &outArgs);

        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS, &prms, status);
        if (status != ENET_SOK)
        {
            appLogPrintf("%s() failed CPSW_PER_IOCTL_SET_INTERVLAN_ROUTE_UNI_EGRESS: %d\n", __func__, status);
        }
    }

    if (status == ENET_SOK)
    {
        *pNumRoutesUsed += 1;
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.egressOpcode == (1 + (outArgs.egressPortRouteId - CPSW_MACPORT_INTERVLAN_ROUTEID_FIRST)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.ttlCheckEn == BTRUE);
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.dstPortMask == (1 << CPSW_ALE_MACPORT_TO_ALEPORT(pInterVlanCfg->egrPortNum)));
        EnetAppUtils_assert(outArgs.ingressPacketClassifierInfo.policerMatchEnMask == (CPSW_ALE_POLICER_MATCH_MACSRC |
                                                                                           CPSW_ALE_POLICER_MATCH_MACDST |
                                                                                           CPSW_ALE_POLICER_MATCH_IVLAN |
                                                                                           CPSW_ALE_POLICER_MATCH_IPSRC |
                                                                                           CPSW_ALE_POLICER_MATCH_IPDST));
    }

    return status;
}

static int32_t CpswAppInterVlan_setShortIPG(Enet_Handle hEnet)
{
    Enet_IoctlPrms prms;
    Cpsw_SetTxShortIpgCfgInArgs setShortIPGInArgs;
    int32_t status;

    memset(&setShortIPGInArgs, 0, sizeof(setShortIPGInArgs));

    ENET_IOCTL_SET_IN_ARGS(&prms, &setShortIPGInArgs);
    setShortIPGInArgs.configureGapThresh = BFALSE;
    setShortIPGInArgs.numMacPorts = 2U;
    setShortIPGInArgs.portShortIpgCfg[0].macPort                       = CPSW_TEST_INTERVLAN_INGRESS_PORT_NUM;
    setShortIPGInArgs.portShortIpgCfg[0].shortIpgCfg.txShortGapEn      = BFALSE;
    setShortIPGInArgs.portShortIpgCfg[0].shortIpgCfg.txShortGapLimitEn = BFALSE;

    setShortIPGInArgs.portShortIpgCfg[1].macPort                       = CPSW_TEST_INTERVLAN_EGRESS_PORT_NUM;
    setShortIPGInArgs.portShortIpgCfg[1].shortIpgCfg.txShortGapEn      = BFALSE;
    setShortIPGInArgs.portShortIpgCfg[1].shortIpgCfg.txShortGapLimitEn = BFALSE;

    ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_PER_IOCTL_SET_SHORT_IPG_CFG, &prms, status);
    if (ENET_SOK == status)
    {
        Cpsw_TxShortIpgCfg getShortIPGOutArgs;

        ENET_IOCTL_SET_OUT_ARGS(&prms, &getShortIPGOutArgs);

        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_PER_IOCTL_GET_SHORT_IPG_CFG, &prms, status);
        if (ENET_SOK == status)
        {
            CpswMacPort_PortTxShortIpgCfg *ipgCfg;
            uint32_t i;

            EnetAppUtils_assert(getShortIPGOutArgs.ipgTriggerThreshBlkCnt == CPSW_TEST_INTERVLAN_DEFAULT_SHORTIPG_THRESHOLD);

            for (i = 0U; i < getShortIPGOutArgs.numMacPorts; i++)
            {
                ipgCfg = &getShortIPGOutArgs.portShortIpgCfg[i];
                if ((ipgCfg->macPort == CPSW_TEST_INTERVLAN_EGRESS_PORT_NUM) ||
                    (ipgCfg->macPort == CPSW_TEST_INTERVLAN_INGRESS_PORT_NUM))
                {
                    EnetAppUtils_assert(ipgCfg->shortIpgCfg.txShortGapEn == BFALSE);
                    EnetAppUtils_assert(ipgCfg->shortIpgCfg.txShortGapLimitEn == BFALSE);
                }
            }
        }
    }

    return status;
}

void EthHwInterVlan_setupRouting(Enet_Type enetType,
                                 uint32_t instId,
                                 EnetCfgServer_InterVlanConfig *pInterVlanCfg)
{
    int32_t status = ENET_SOK;
    uint32_t numRoutesAllocated = 0U;
    Enet_Handle hEnet;

    /* Get CPSW Handle */
    hEnet = Enet_getHandle(enetType, instId);

    status = CpswAppInterVlan_setShortIPG(hEnet);
    if (ENET_SOK == status)
    {
        status = CpswAppInterVlan_setInterVlanUniEgress(hEnet,
                                                        CPSW_MACPORT_INTERVLAN_ROUTEID_FIRST,
                                                        &numRoutesAllocated,
                                                        pInterVlanCfg);
    }

    EnetAppUtils_assert(status == ENET_SOK);
}
