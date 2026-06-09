/*
 *
 * Copyright (c) 2024-2025 Texas Instruments Incorporated
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

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x707
#define ETHFWTRACE_MOD_NAME "Virtnetif-App"

#if defined(__KLOCWORK__)
#include <stdlib.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <ti/csl/cslr_gtc.h>
#include <ethremotecfg/client/include/cpsw_proxy.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

#include <ti/drv/udma/udma.h>
#include <ti/drv/enet/enet.h>
#include <ti/drv/enet/include/per/cpsw.h>
#include <ti/drv/enet/examples/utils/include/enet_apputils.h>
#include <ti/drv/enet/examples/utils/include/enet_appmemutils_cfg.h>
#include <ti/drv/enet/examples/utils/include/enet_appmemutils.h>

/* lwIP core includes */
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/api.h"

#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"

/* lwIP netif includes */
#include "lwip/etharp.h"
#include "netif/ethernet.h"
#include "netif/bridgeif.h"

#include <ti/drv/enet/lwipif/inc/lwipif2enet_appif.h>
#include <ti/drv/enet/lwipif/inc/lwip2lwipif.h>

#if defined(ETHAPP_ENABLE_IPERF_SERVER)
#include "examples/lwiperf/lwiperf_example.h"
#endif

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
#include <ti/drv/enet/lwipific/inc/netif_ic.h>
#include <ti/drv/enet/lwipific/inc/lwip2enet_ic.h>
#include <ti/drv/enet/lwipific/inc/lwip2lwipif_ic.h>
#endif

#define System_printf printf
#define System_vprintf vprintf

#if defined(ENABLE_MAC_ONLY_PORTS)
#define CPSW_REMOTE_APP_REMOTE_NETIF_MAX      (2U)
#else
#define CPSW_REMOTE_APP_REMOTE_NETIF_MAX      (1U)
#endif

#define CPSW_REMOTE_APP_PACKET_POLL_PERIOD_US (1000U)

#define ETHAPP_HWRECOVERY_TASK_PRI              (2U)

#if defined(SAFERTOS)
#define ETHAPP_MONITOR_TASK_STACKSIZE              (16U * 1024U)
#define ETHAPP_MONITOR_TASK_STACKALIGN             ETHAPP_MONITOR_TASK_STACKSIZE
#else
#define ETHAPP_MONITOR_TASK_STACKSIZE              (16U * 1024U)
#define ETHAPP_MONITOR_TASK_STACKALIGN             (32U)
#endif

/* lwIP features that EthFw relies on */
#ifndef LWIP_IPV4
#error "LWIP_IPV4 is not enabled"
#endif
#ifndef LWIP_NETIF_STATUS_CALLBACK
#error "LWIP_NETIF_STATUS_CALLBACK is not enabled"
#endif
#ifndef LWIP_NETIF_LINK_CALLBACK
#error "LWIP_NETIF_LINK_CALLBACK is not enabled"
#endif

/* DHCP or static IP */
#define ETHAPP_LWIP_USE_DHCP                    (1)
#if !ETHAPP_LWIP_USE_DHCP
#define ETHFW_CLIENT_IPADDR_SWITCH_PORT(addr)   IP4_ADDR((addr), 192,168,1,201)
#define ETHFW_CLIENT_GW_SWITCH_PORT(addr)       IP4_ADDR((addr), 192,168,1,1)
#define ETHFW_CLIENT_NETMASK_SWITCH_PORT(addr)  IP4_ADDR((addr), 255,255,255,0)

#define ETHFW_CLIENT_IPADDR_MAC_PORT(addr)      IP4_ADDR((addr), 192,168,2,201)
#define ETHFW_CLIENT_GW_MAC_PORT(addr)          IP4_ADDR((addr), 192,168,2,1)
#define ETHFW_CLIENT_NETMASK_MAC_PORT(addr)     IP4_ADDR((addr), 255,255,255,0)
#endif

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
static struct netif netif_ic;
#if defined (ETHFW_RTOS_MCU2_1_SERVER)
static uint32_t netif_ic_state[IC_ETH_MAX_VIRTUAL_IF] =
{
    IC_ETH_IF_MCU2_0_MCU2_1,
    IC_ETH_IF_MCU2_1_MCU2_0,
    IC_ETH_IF_MCU2_0_A72,
    IC_ETH_IF_MCU2_0_MCU3_0,
    IC_ETH_IF_MCU3_0_MCU2_0,
    IC_ETH_IF_MCU2_1_A72,
    IC_ETH_IF_MCU2_1_MCU3_0,
    IC_ETH_IF_MCU3_0_MCU2_1
};
#else
static uint32_t netif_ic_state[IC_ETH_MAX_VIRTUAL_IF] =
{
    IC_ETH_IF_MCU2_0_MCU2_1,
    IC_ETH_IF_MCU2_1_MCU2_0,
    IC_ETH_IF_MCU2_0_A72,
    IC_ETH_IF_MCU2_0_MCU3_0,
    IC_ETH_IF_MCU3_0_MCU2_0
};
#endif
struct netif netif_bridge;
bridgeif_initdata_t bridge_initdata;

#define ETHAPP_LWIP_BRIDGE_MAX_PORTS (4)
#define ETHAPP_LWIP_BRIDGE_MAX_DYNAMIC_ENTRIES (32)
#define ETHAPP_LWIP_BRIDGE_MAX_STATIC_ENTRIES (8)
#endif /* ETHAPP_ENABLE_INTERCORE_ETH */

#if defined(ETHFW_MONITOR_SUPPORT)
static uint8_t gEthAppRecoveryStackBuf[ETHAPP_MONITOR_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__((aligned(ETHAPP_MONITOR_TASK_STACKSIZE)));
#endif

typedef struct CpswRemoteApp_VirtNetif_s
{
    /* CpswProxy handle used to communicate with EthFw */
    CpswProxy_Handle hCpswProxy;

    /* Virtual port id */
    EthRemoteCfg_VirtPort virtPort;

    /* Bitmask of supported features */
    uint32_t features;

    /* MAC ports used by this client app */
    Enet_MacPort *macPorts;

    /* Number of MAC ports */
    uint32_t numMacPorts;

    /* MAC address */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];

    /* IPv4 address */
    uint8_t ipv4Addr[ENET_IPv4_ADDR_LEN];

    /* lwIP network interface */
    struct netif netif;

    /* DHCP network interface */
    struct dhcp dhcpNetif;

    /* Whether this netif should be set as the default netif */
    bool isDfltNetif;

} CpswRemoteApp_VirtNetif;

typedef struct CpswRemoteApp_HwRecoveryMsg_s
{
    /* Virtual network interface data */
    CpswRemoteApp_VirtNetif *virtNetif;

    /* Notify type */
    uint32_t notifyType;
} CpswRemoteApp_HwRecoveryMsg;

/* Mailbox used to pass virtual netifs to be closed for recovery */
#define ETHAPP_VIRTNETIF_MBX_MSGSIZE            (sizeof(CpswRemoteApp_HwRecoveryMsg))
#define ETHAPP_VIRTNETIF_MBX_MSGCOUNT           (4U)
#if defined(SAFERTOS)
#define ETHAPP_VIRTNETIF_MBX_SIZE               ((ETHAPP_VIRTNETIF_MBX_MSGSIZE * \
                                                  ETHAPP_VIRTNETIF_MBX_MSGCOUNT) + \
                                                 safertosapiQUEUE_OVERHEAD_BYTES)
#else
#define ETHAPP_VIRTNETIF_MBX_SIZE               (ETHAPP_VIRTNETIF_MBX_MSGSIZE * \
                                                 ETHAPP_VIRTNETIF_MBX_MSGCOUNT)
#endif

typedef struct CpswRemoteApp_VirtNetifObj_s
{
    /* Virtual network interface data */
    CpswRemoteApp_VirtNetif virtNetif[CPSW_REMOTE_APP_REMOTE_NETIF_MAX];

    /* UDMA LLD handle */
    Udma_DrvHandle hUdmaDrv;

    /* Enet LLD DMA handle */
    EnetDma_Handle hEnetDma;

    /* Whether to use extended attach remote command or not */
    bool useExtAttach;

    /* Whether to use default flow or not */
    bool useDefaultRxFlow;

#if defined(ETHFW_MONITOR_SUPPORT)
    /* Mailbox of virtual netifs to be closed during CPSW recovery */
    MailboxP_Handle hMbx;

    /* Mailbox buffer for storing all the command messages */
    uint8_t mbxBuf[ETHAPP_VIRTNETIF_MBX_SIZE] __attribute__ ((aligned(32)));
#endif
} CpswRemoteApp_VirtNetifObj;

/* Link status on these ports will be used to determine link up on virtual switch port */
static Enet_MacPort gRemoteAppMacPorts[] =
{
    ENET_MAC_PORT_3,
};

#if (CPSW_REMOTE_APP_REMOTE_NETIF_MAX >= 2)  && defined(ENABLE_MAC_ONLY_PORTS)
/* Link status on these ports will be used to determine link up on virtual MAC port */
static Enet_MacPort gRemoteApp_virtualMacPorts[] =
{
    ENET_MAC_PORT_4,
};
#endif

CpswRemoteApp_VirtNetifObj gVirtNetifObj =
{
    .useDefaultRxFlow = BFALSE,
    .useExtAttach     = BFALSE,
    .virtNetif =
    {
        {
            .hCpswProxy  = NULL,
            .virtPort    = ETHREMOTECFG_SWITCH_PORT_1,
            .macPorts    = gRemoteAppMacPorts,
            .numMacPorts = ENET_ARRAYSIZE(gRemoteAppMacPorts),
            .isDfltNetif = BTRUE,
        },
#if (CPSW_REMOTE_APP_REMOTE_NETIF_MAX >= 2) && defined(ENABLE_MAC_ONLY_PORTS)
        {
            .hCpswProxy  = NULL,
            .virtPort    = ETHREMOTECFG_MAC_PORT_4,
            .macPorts    = gRemoteApp_virtualMacPorts,
            .numMacPorts = ENET_ARRAYSIZE(gRemoteApp_virtualMacPorts),
            .isDfltNetif = BFALSE,
        },
#endif
    },
};

/* ToDo: UDMA LLD object */
struct Udma_DrvObj gUdmaDrvObj;

static int32_t CpswRemoteApp_openUdma(void);

static void CpswRemoteApp_openCpswProxy(CpswRemoteApp_VirtNetif *virtNetif);

void EthApp_initLwip(void *arg);

static void EthApp_initNetif(CpswRemoteApp_VirtNetif *virtNetif);

static void EthApp_virtNetifStatusCb(struct netif *netif);

static void EthApp_lwipNetifStatusCb(struct netif *netif);

void CpswRemoteApp_initVirtNetif(Enet_Type enetType, uint32_t instId);

void EthApp_initLwip(void *arg);

int32_t CpswRemoteApp_registerRemoteTimer(CpswCpts_HwPush hwPushNum);

#if defined(ETHFW_MONITOR_SUPPORT)
static void EthApp_openDma(struct netif *netif);

static void EthApp_closeDma(struct netif *netif);

static void CpswRemoteApp_hwRecoveryTask(void *a0, void *a1);

static void CpswRemoteApp_hwRecoveryNotify(uint32_t notifyType,
                                           void *notifyArg,
                                           void *cbArg);
#endif

static inline Enet_MacPort CpswRemoteApp_getMacPort(EthRemoteCfg_VirtPort portId)
{
    Enet_MacPort macPort = ENET_MAC_PORT_INV;

    if (CpswProxy_isMacPort(portId))
    {
        macPort = ENET_MACPORT_DENORM(portId - ETHREMOTECFG_MAC_PORT_1);
    }

    return macPort;
}

void CpswRemoteApp_initVirtNetif(Enet_Type enetType, uint32_t instId)
{
    EnetDma_initCfg dmaCfg;
#if defined(ETHFW_MONITOR_SUPPORT)
    MailboxP_Params mbxParams;
    TaskP_Params params;
#endif
    uint32_t i = 0U;
    int32_t status = ENET_SOK;

    /* Init UDMA LLD based on NAVSS instance */
    status = CpswRemoteApp_openUdma();
    ETHFWTRACE_ERR_IF((status != UDMA_SOK), status, "Failed to open UDMA LLD");

    /* Initialize Enet memutils */
    if (status == ENET_SOK)
    {
        status = EnetMem_init();
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to init memutils");
    }

    /* Initialize data path of Enet LLD */
    if (status == ENET_SOK)
    {
        EnetUdma_initDataPathParams(&dmaCfg);
        dmaCfg.hUdmaDrv = gVirtNetifObj.hUdmaDrv;

        gVirtNetifObj.hEnetDma = EnetUdma_initDataPath(enetType,
                                         instId,
                                         &dmaCfg);
        if (gVirtNetifObj.hEnetDma == NULL)
        {
            status = ENET_EFAIL;
            ETHFWTRACE_ERR(status, "Failed to init Enet LLD data path");
            EnetMem_deInit();
        }
    }

    /* Step 5b. Open proxy clients for all virtual interfaces */
    for (i = 0U; i < ENET_ARRAYSIZE(gVirtNetifObj.virtNetif); i++)
    {
        CpswRemoteApp_openCpswProxy(&gVirtNetifObj.virtNetif[i]);
    }

#if defined(ETHFW_MONITOR_SUPPORT)
    /* Step 7: Create mailbox to pass virtual netifs to close during recovery */
    MailboxP_Params_init(&mbxParams);
    mbxParams.name    = (uint8_t *)"VirtNetif Mbox";
    mbxParams.size    = ETHAPP_VIRTNETIF_MBX_MSGSIZE;
    mbxParams.count   = ETHAPP_VIRTNETIF_MBX_MSGCOUNT;
    mbxParams.buf     = (void *)&gVirtNetifObj.mbxBuf[0U];
    mbxParams.bufsize = sizeof(gVirtNetifObj.mbxBuf);

    gVirtNetifObj.hMbx = MailboxP_create(&mbxParams);

    /* Step 8: Create HW recovery task */
    TaskP_Params_init(&params);
    params.name      = "HW Recovery Task";
    params.priority  = ETHAPP_HWRECOVERY_TASK_PRI;
    params.arg0      = (void *)&gVirtNetifObj;
    params.stack     = &gEthAppRecoveryStackBuf[0];
    params.stacksize = sizeof(gEthAppRecoveryStackBuf);

    TaskP_create(&CpswRemoteApp_hwRecoveryTask, &params);
#endif
}

static int32_t CpswRemoteApp_openUdma(void)
{
    Udma_InitPrms initPrms;
    Udma_DrvHandle hUdmaDrv = &gUdmaDrvObj;
    uint32_t instId = UDMA_INST_ID_MAIN_0;
    int32_t status;

    memset(hUdmaDrv, 0, sizeof(*hUdmaDrv));

    /* Initialize the UDMA driver based on NAVSS instance */
    UdmaInitPrms_init(instId, &initPrms);
    initPrms.printFxn = (Udma_PrintFxn) &System_printf;

    status = Udma_init(hUdmaDrv, &initPrms);
    if (status != UDMA_SOK)
    {
        ETHFWTRACE_ERR(status, "Failed init UDMA driver");
        hUdmaDrv = NULL;
    }

    gVirtNetifObj.hUdmaDrv = hUdmaDrv;

    return status;
}

static void CpswRemoteApp_setRxFlowPrms(EnetUdma_OpenRxFlowPrms *pRxFlowPrms,
                                      uint32_t rxStartFlowIdx,
                                      uint32_t rxFlowIdx,
                                      Udma_DrvHandle hUdmaDrv,
                                      uint32_t numRxPackets,
                                      void *cbArg,
                                      EnetDma_PktNotifyCb eventCb,
                                      uint32_t rxFlowMtu)
{
    pRxFlowPrms->startIdx = rxStartFlowIdx;
    pRxFlowPrms->flowIdx = rxFlowIdx;

    pRxFlowPrms->hUdmaDrv = hUdmaDrv;

    pRxFlowPrms->notifyCb = eventCb;

    pRxFlowPrms->numRxPkts = numRxPackets;

    pRxFlowPrms->disableCacheOpsFlag = BFALSE;
    pRxFlowPrms->cbArg = cbArg;
    pRxFlowPrms->useGlobalEvt = BTRUE;
    pRxFlowPrms->useProxy = BFALSE;
    pRxFlowPrms->rxFlowMtu = rxFlowMtu;
}

static void CpswRemoteApp_setTxChPrms(EnetUdma_OpenTxChPrms *pTxChPrms,
                                    uint32_t txChNum,
                                    Udma_DrvHandle hUdmaDrv,
                                    uint32_t numTxPackets,
                                    void *cbArg,
                                    EnetDma_PktNotifyCb eventCb)
{
    pTxChPrms->chNum = txChNum;
    pTxChPrms->hUdmaDrv = hUdmaDrv;

    pTxChPrms->numTxPkts = numTxPackets;
    pTxChPrms->disableCacheOpsFlag = BFALSE;

    pTxChPrms->cbArg = cbArg;
    pTxChPrms->notifyCb = eventCb;
    pTxChPrms->useGlobalEvt = BTRUE;
}

static void CpswRemoteApp_openCpswProxy(CpswRemoteApp_VirtNetif *virtNetif)
{
    CpswProxy_Config proxyConfig;
    CpswProxy_Handle hProxy;

    proxyConfig.virtPort = virtNetif->virtPort;

    hProxy = CpswProxy_open(&proxyConfig);
    if (hProxy != NULL)
    {
        virtNetif->hCpswProxy = hProxy;

#if defined(ETHFW_MONITOR_SUPPORT)
        CpswProxy_registerNotifyCb(virtNetif->hCpswProxy,
                                   ETHREMOTECFG_NOTIFY_HWERROR,
                                   CpswRemoteApp_hwRecoveryNotify,
                                   (void *)virtNetif);

        CpswProxy_registerNotifyCb(virtNetif->hCpswProxy,
                                   ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE,
                                   CpswRemoteApp_hwRecoveryNotify,
                                   (void *)virtNetif);
#endif
    }
    else
    {
        ETHFWTRACE_ERR(CPSWPROXY_EFAIL, "Failed to open CpswProxy for virtPortId %u",
                       virtNetif->virtPort);
        EnetAppUtils_assert(hProxy != NULL);
    }
}

int32_t TsCouplerClient_allocHwPushInst(uint32_t *hwPushNum)
{
    int32_t status = CPSWPROXY_SOK;
    CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];

    /* Send request to Ethfw to alloc HW push instance */
    status = CpswProxy_allocHwPushInst(virtNetif->hCpswProxy,
                                       hwPushNum);

    return status;
}

int32_t TsCouplerClient_registerRemoteTimer(uint32_t hwPushNum,
                                            uint32_t tsRouterTntrId)
{
    int32_t status = CPSWPROXY_SOK;
    CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];

    /* Send request to Ethfw to configure TSR */
    status = CpswProxy_registerRemoteTimer(virtNetif->hCpswProxy,
                                           tsRouterTntrId,
                                           hwPushNum);

    return status;
}

int32_t CpswRemoteApp_unregisterRemoteTimer(CpswCpts_HwPush hwPushNum)
{
    int32_t status = CPSWPROXY_SOK;
    CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];

    /* Send request to Ethfw to configure TSR */
    status = CpswProxy_unregisterRemoteTimer(virtNetif->hCpswProxy,
                                             hwPushNum);

    /* ToDo: Disable GTC ? */

    return status;
}

void EthApp_initLwip(void *arg)
{
    sys_sem_t *initSem;
    uint32_t i;

    LWIP_ASSERT("arg != NULL", arg != NULL);
    initSem = (sys_sem_t*)arg;

    /* init randomizer again (seed per thread) */
    srand((unsigned int)sys_now()/1000);

    /* init network interfaces */
    for (i = 0U; i < ENET_ARRAYSIZE(gVirtNetifObj.virtNetif); i++)
    {
        EthApp_initNetif(&gVirtNetifObj.virtNetif[i]);
    }

#if defined(ETHAPP_ENABLE_IPERF_SERVER)
    /* Enable iperf */
    lwiperf_example_init();
#endif

    sys_sem_signal(initSem);
}

static void EthApp_initNetif(CpswRemoteApp_VirtNetif *virtNetif)
{
    struct netif *netif = &virtNetif->netif;
    ip4_addr_t ipaddr, netmask, gw;
#if LWIP_CHECKSUM_CTRL_PER_NETIF
    uint32_t chksumFlags = NETIF_CHECKSUM_ENABLE_ALL;
/* Disable checksum in software if CPSW can do it (i.e. not impacted by errata) */
#if !defined(ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA)
    chksumFlags = ~(NETIF_CHECKSUM_GEN_UDP |
                    NETIF_CHECKSUM_GEN_TCP |
                    NETIF_CHECKSUM_CHECK_TCP |
                    NETIF_CHECKSUM_CHECK_UDP);
#endif
#endif
#if ETHAPP_LWIP_USE_DHCP
    err_t err;
#endif

    ip4_addr_set_zero(&gw);
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);

    if(CpswProxy_isSwitchPort(virtNetif->virtPort))
    {
#if ETHAPP_LWIP_USE_DHCP
        ETHFWTRACE_INFO("Starting lwIP, local interface IP is dhcp-enabled");
#else /* ETHAPP_LWIP_USE_DHCP */
        ETHFW_CLIENT_GW_SWITCH_PORT(&gw);
        ETHFW_CLIENT_IPADDR_SWITCH_PORT(&ipaddr);
        ETHFW_CLIENT_NETMASK_SWITCH_PORT(&netmask);
        ETHFWTRACE_INFO("Starting lwIP, local interface IP is %s", ip4addr_ntoa(&ipaddr));
#endif /* ETHAPP_LWIP_USE_DHCP */

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
        /* Create Enet LLD ethernet interface */
        netif_add(netif, NULL, NULL, NULL, NULL, LWIPIF_LWIP_init, tcpip_input);
#if LWIP_CHECKSUM_CTRL_PER_NETIF
        NETIF_SET_CHECKSUM_CTRL(netif, chksumFlags);
#endif

#if defined (ETHFW_RTOS_MCU2_1_SERVER)
        /* Create inter-core virtual ethernet interface: MCU3_0 <-> MCU2_1 */
        netif_add(&netif_ic, NULL, NULL, NULL,
                  (void*)&netif_ic_state[IC_ETH_IF_MCU3_0_MCU2_1],
                  LWIPIF_LWIP_IC_init, tcpip_input);
#else
#if defined (ETHFW_RTOS_MCU3_0)
        /* Create inter-core virtual ethernet interface: MCU3_0 <-> MCU2_0 */
        netif_add(&netif_ic, NULL, NULL, NULL,
                  (void*)&netif_ic_state[IC_ETH_IF_MCU3_0_MCU2_0],
                  LWIPIF_LWIP_IC_init, tcpip_input);
#else
        /* Create inter-core virtual ethernet interface: MCU2_1 <-> MCU2_0 */
        netif_add(&netif_ic, NULL, NULL, NULL,
                  (void*)&netif_ic_state[IC_ETH_IF_MCU2_1_MCU2_0],
                  LWIPIF_LWIP_IC_init, tcpip_input);
#endif
#endif

        /* Create bridge interface */
        bridge_initdata.max_ports = ETHAPP_LWIP_BRIDGE_MAX_PORTS;
        bridge_initdata.max_fdb_dynamic_entries = ETHAPP_LWIP_BRIDGE_MAX_DYNAMIC_ENTRIES;
        bridge_initdata.max_fdb_static_entries = ETHAPP_LWIP_BRIDGE_MAX_STATIC_ENTRIES;
        EnetUtils_copyMacAddr(&bridge_initdata.ethaddr.addr[0U], &virtNetif->macAddr[0U]);

        netif_add(&netif_bridge, &ipaddr, &netmask, &gw, &bridge_initdata, bridgeif_init, netif_input);

        /* Add all network interfaces to the bridge */
        bridgeif_add_port_with_opts(&netif_bridge, netif, BRIDGEIF_PORT_CPSW);
        bridgeif_add_port_with_opts(&netif_bridge, &netif_ic, BRIDGEIF_PORT_VIRTUAL);

        /* Set bridge interface as the default */
        netif_set_default(&netif_bridge);
        netif_set_status_callback(&netif_bridge, EthApp_lwipNetifStatusCb);

#if LWIP_CHECKSUM_CTRL_PER_NETIF
        NETIF_SET_CHECKSUM_CTRL(&netif_bridge, chksumFlags);
        NETIF_SET_CHECKSUM_CTRL(&netif_ic, chksumFlags);
#endif
#else
        netif_add(netif, &ipaddr, &netmask, &gw, NULL, LWIPIF_LWIP_init, tcpip_input);
#if LWIP_CHECKSUM_CTRL_PER_NETIF
        NETIF_SET_CHECKSUM_CTRL(netif, chksumFlags);
#endif

        if (virtNetif->isDfltNetif)
        {
            netif_set_default(netif);
        }
#endif
        netif_set_status_callback(netif, EthApp_virtNetifStatusCb);

        dhcp_set_struct(netif_default, &virtNetif->dhcpNetif);

        netif_set_up(netif);
#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
        netif_set_up(&netif_ic);
        netif_set_up(&netif_bridge);
#endif

#if ETHAPP_LWIP_USE_DHCP
        err = dhcp_start(netif_default);
        ETHFWTRACE_ERR_IF((err != ERR_OK), err, "Failed to start DHCP");
#endif /* ETHAPP_LWIP_USE_DHCP */
    }
    else
    {
#if ETHAPP_LWIP_USE_DHCP
        ETHFWTRACE_INFO("Starting lwIP, local interface IP is dhcp-enabled");
#else /* ETHAPP_LWIP_USE_DHCP */
        ETHFW_CLIENT_GW_MAC_PORT(&gw);
        ETHFW_CLIENT_IPADDR_MAC_PORT(&ipaddr);
        ETHFW_CLIENT_NETMASK_MAC_PORT(&netmask);
        ETHFWTRACE_INFO("Starting lwIP, local interface IP is %s", ip4addr_ntoa(&ipaddr));
#endif /* ETHAPP_LWIP_USE_DHCP */

        netif_add(netif, &ipaddr, &netmask, &gw, NULL, LWIPIF_LWIP_init, tcpip_input);
#if LWIP_CHECKSUM_CTRL_PER_NETIF
        NETIF_SET_CHECKSUM_CTRL(netif, chksumFlags);
#endif

        if (virtNetif->isDfltNetif)
        {
            netif_set_default(netif);
        }

        netif_set_status_callback(netif, EthApp_virtNetifStatusCb);

        dhcp_set_struct(netif, &virtNetif->dhcpNetif);

        netif_set_up(netif);

#if ETHAPP_LWIP_USE_DHCP
        err = dhcp_start(netif);
        ETHFWTRACE_ERR_IF((err != ERR_OK), err, "Failed to start DHCP");
#endif /* ETHAPP_LWIP_USE_DHCP */
    }
}

static void EthApp_virtNetifStatusCb(struct netif *netif)
{
    CpswRemoteApp_VirtNetif *virtNetif;

    virtNetif = container_of(netif, CpswRemoteApp_VirtNetif, netif);

    if (netif_is_up(netif))
    {
        const ip4_addr_t *ipAddr = netif_ip4_addr(netif);

        ETHFWTRACE_INFO("Added interface '%c%c%d', IP is %s",
                        netif->name[0], netif->name[1], netif->num, ip4addr_ntoa(ipAddr));

        if (ipAddr->addr != 0)
        {
            virtNetif->ipv4Addr[0] = ip4_addr1_val(*ipAddr);
            virtNetif->ipv4Addr[1] = ip4_addr2_val(*ipAddr);
            virtNetif->ipv4Addr[2] = ip4_addr3_val(*ipAddr);
            virtNetif->ipv4Addr[3] = ip4_addr4_val(*ipAddr);

            EnetAppUtils_assert(virtNetif->hCpswProxy != NULL);

            if (CpswProxy_isSwitchPort(virtNetif->virtPort))
            {
                CpswProxy_registerIPV4Addr(virtNetif->hCpswProxy,
                                           virtNetif->macAddr,
                                           virtNetif->ipv4Addr);
            }
        }
    }
    else
    {
        ETHFWTRACE_INFO("Removed interface '%c%c%d'", netif->name[0], netif->name[1], netif->num);
    }
}

static void EthApp_lwipNetifStatusCb(struct netif *netif)
{
    if (netif_is_up(netif))
    {
        const ip4_addr_t *ipAddr = netif_ip4_addr(netif);

        ETHFWTRACE_INFO("Added interface '%c%c%d', IP is %s",
                        netif->name[0], netif->name[1], netif->num, ip4addr_ntoa(ipAddr));
    }
    else
    {
        ETHFWTRACE_INFO("Removed interface '%c%c%d'", netif->name[0], netif->name[1], netif->num);
    }
}

static void CpswRemoteApp_openLwipRxCh(CpswProxy_Handle hProxy,
                                       Udma_DrvHandle hUdmaDrv,
                                       bool useDefaultFlow,
                                       uint32_t rxFlowStartIdx,
                                       uint32_t rxFlowIdx,
                                       uint8_t *macAddress,
                                       LwipifEnetAppIf_RxConfig *rxConfig,
                                       LwipifEnetAppIf_RxHandleInfo *rxHandleInfo,
                                       uint32_t rxFlowMtu,
                                       bool isSwitchPort)
{
    EnetUdma_OpenRxFlowPrms cpswRxFlowCfg;

    rxHandleInfo->rxFlowStartIdx = rxFlowStartIdx;
    rxHandleInfo->rxFlowIdx = rxFlowIdx;
    ENET_UTILS_ARRAY_COPY(rxHandleInfo->macAddr, macAddress);

    EnetUdma_initRxFlowParams(&cpswRxFlowCfg);

    CpswRemoteApp_setRxFlowPrms(&cpswRxFlowCfg,
                                rxHandleInfo->rxFlowStartIdx,
                                rxHandleInfo->rxFlowIdx,
                                hUdmaDrv,
                                rxConfig->numPackets,
                                rxConfig->cbArg,
                                rxConfig->notifyCb,
                                rxFlowMtu);

    rxHandleInfo->hRxFlow = EnetUdma_openRxFlow(gVirtNetifObj.hEnetDma, &cpswRxFlowCfg);
    EnetAppUtils_assert(rxHandleInfo->hRxFlow != NULL);

    if (useDefaultFlow && isSwitchPort)
    {
        CpswProxy_registerDefaultRxFlow(hProxy,
                                        rxHandleInfo->rxFlowStartIdx,
                                        rxHandleInfo->rxFlowIdx);
    }
    else
    {
        CpswProxy_registerDstMacRxFlow(hProxy,
                                       rxHandleInfo->rxFlowStartIdx,
                                       rxHandleInfo->rxFlowIdx,
                                       rxHandleInfo->macAddr);
    }
}

static void CpswRemoteApp_closeLwipRxCh(CpswProxy_Handle hProxy,
                                        Udma_DrvHandle hUdmaDrv,
                                        bool useDefaultFlow,
                                        uint8_t *ipV4Addr,
                                        LwipifEnetAppIf_RxHandleInfo *rxHandleInfo,
                                        void *freeFxnArg,
                                        LwipifEnetAppIf_FreePktCbFxn freeFxn,
                                        bool isSwitchPort)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    int32_t status;

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetDma_disableRxEvent(rxHandleInfo->hRxFlow);

    if (isSwitchPort)
    {
        CpswProxy_unregisterIPV4Addr(hProxy, ipV4Addr);
    }

    if (useDefaultFlow && isSwitchPort)
    {
        CpswProxy_unregisterDefaultRxFlow(hProxy,
                                          rxHandleInfo->rxFlowStartIdx,
                                          rxHandleInfo->rxFlowIdx);
    }
    else
    {
        CpswProxy_unregisterDstMacRxFlow(hProxy,
                                         rxHandleInfo->rxFlowStartIdx,
                                         rxHandleInfo->rxFlowIdx,
                                         rxHandleInfo->macAddr);
    }

    status = EnetUdma_closeRxFlow(rxHandleInfo->hRxFlow,
                               &fqPktInfoQ,
                               &cqPktInfoQ);
    EnetAppUtils_assert(status == ENET_SOK);
    CpswProxy_freeMac(hProxy,
                      rxHandleInfo->macAddr);
    CpswProxy_freeRxFlow(hProxy,
                         rxHandleInfo->rxFlowStartIdx,
                         rxHandleInfo->rxFlowIdx);
    freeFxn(freeFxnArg, &fqPktInfoQ, &cqPktInfoQ);
}

static void CpswRemoteApp_openLwipTxCh(Udma_DrvHandle hUdmaDrv,
                                       uint32_t txPSILId,
                                       LwipifEnetAppIf_TxConfig *txConfig,
                                       LwipifEnetAppIf_TxHandleInfo *txHandleInfo)
{
    EnetUdma_OpenTxChPrms cpswTxChCfg;

    txHandleInfo->txChNum = txPSILId;

    /* Set configuration parameters */
    EnetUdma_initTxChParams(&cpswTxChCfg);

    CpswRemoteApp_setTxChPrms(&cpswTxChCfg,
                              txHandleInfo->txChNum,
                              hUdmaDrv,
                              txConfig->numPackets,
                              txConfig->cbArg,
                              txConfig->notifyCb);

    txHandleInfo->hTxChannel = EnetUdma_openTxCh(gVirtNetifObj.hEnetDma, &cpswTxChCfg);
    EnetAppUtils_assert(NULL != txHandleInfo->hTxChannel);
}

static void CpswRemoteApp_closeLwipTxCh(CpswProxy_Handle hProxy,
                                        Udma_DrvHandle hUdmaDrv,
                                        LwipifEnetAppIf_TxHandleInfo *txHandleInfo,
                                        void *freeFxnArg,
                                        LwipifEnetAppIf_FreePktCbFxn freeFxn)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    int32_t status;

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetDma_disableTxEvent(txHandleInfo->hTxChannel);
    status = EnetUdma_closeTxCh(txHandleInfo->hTxChannel, &fqPktInfoQ, &cqPktInfoQ);
    EnetAppUtils_assert(ENET_SOK == status);

    CpswProxy_freeTxCh(hProxy,
                       txHandleInfo->txChNum);
    freeFxn(freeFxnArg, &fqPktInfoQ, &cqPktInfoQ);
}

static bool LwipifEnetAppCb_isPortLinked(struct netif *netif,
                                         void *handleArg)
{
    bool isLinked = BFALSE;

    isLinked = (isLinked || CpswProxy_isPhyLinked((CpswProxy_Handle)handleArg));

    return isLinked;
}

void LwipifEnetAppCb_getHandle(LwipifEnetAppIf_GetHandleInArgs *inArgs,
                               LwipifEnetAppIf_GetHandleOutArgs *outArgs)
{
    CpswRemoteApp_VirtNetif *virtNetif;
    uint32_t txPSILId;
    uint32_t rxStartFlowId;
    uint32_t rxFlowIdOffset;
    bool isSwitchPort;
    uint32_t numTxCh;
    uint32_t numRxFlow;
    uint32_t relChPriority = 0U;
    uint32_t flowIdx = 0U;

    virtNetif = container_of(inArgs->netif, CpswRemoteApp_VirtNetif, netif);
    EnetAppUtils_assert(virtNetif->hCpswProxy != NULL);

    isSwitchPort = CpswProxy_isSwitchPort(virtNetif->virtPort);

    outArgs->handleArg = (void *)(virtNetif->hCpswProxy);
    outArgs->coreId = EnetSoc_getCoreId();
    outArgs->hUdmaDrv = gVirtNetifObj.hUdmaDrv;
    outArgs->print = (Enet_Print) & printf;
    outArgs->isPortLinkedFxn = &LwipifEnetAppCb_isPortLinked;
    outArgs->txCsumOffloadEn = BTRUE;
    outArgs->rxCsumOffloadEn = BTRUE;
    outArgs->rxInfo[0U].disableEvent = BTRUE;
    outArgs->timerPeriodUs = CPSW_REMOTE_APP_PACKET_POLL_PERIOD_US;
    outArgs->txInfo.disableEvent = BTRUE;
    outArgs->txInfo.txPortNum = CpswRemoteApp_getMacPort(virtNetif->virtPort);

    if (gVirtNetifObj.useExtAttach)
    {
        CpswProxy_attachExtended(virtNetif->hCpswProxy,
                                 virtNetif->virtPort,
                                 &outArgs->hostPortRxMtu,
                                 &outArgs->txMtu[0U],
                                 &txPSILId,
                                 &rxStartFlowId,
                                 &rxFlowIdOffset,
                                 virtNetif->macAddr,
                                 &virtNetif->features);
    }
    else
    {
        CpswProxy_attach(virtNetif->hCpswProxy,
                         virtNetif->virtPort,
                         &outArgs->hostPortRxMtu,
                         &outArgs->txMtu[0],
                         &numTxCh,
                         &numRxFlow,
                         &virtNetif->features);
        ETHFWTRACE_ERR_IF((numTxCh == 0U), ETHFW_EFAIL, "Number of tx channel allocated cannot be 0U");
        ETHFWTRACE_ERR_IF((numRxFlow == 0U), ETHFW_EFAIL, "Number of rx flow allocated cannot be 0U");
        /* To demonstrate the QoS functionality we should allocate multiple channels
         * But Out of box we are only giving 1 tx channel for each virtual port for RTOS client
         * Suppose 3 channels are allocated (update ethfw server main.c virtual port config for the same),
         * to allocate first channel with low priority, second channel with medium
         * and third channel with highest priority give relative channel priority to
         * be 0U, 1U and 2U in order respectively.
         * It is important that the numTxCh allocated should be > 1 to test the functionality.
         * relChPriority range from 0U to numTxCh - 1U */
        CpswProxy_allocTxCh(virtNetif->hCpswProxy,
                            &txPSILId,
                            relChPriority);
        /* As we have not added multiple rx flow support for QoS on RTOS client side.
         * For now we are allocating only 1 rx flow (i.e. default flow).
         * But we have tested rx QoS by manually calling multiple allocRxFlow
         * i.e. with default flow and extended flow as well */
        CpswProxy_allocRxFlow(virtNetif->hCpswProxy,
                              &rxStartFlowId,
                              &rxFlowIdOffset,
                              flowIdx);
        CpswProxy_allocMac(virtNetif->hCpswProxy,
                           virtNetif->macAddr);
    }

    CpswRemoteApp_openLwipTxCh(outArgs->hUdmaDrv,
                               txPSILId,
                               &inArgs->txCfg,
                               &outArgs->txInfo);

    CpswRemoteApp_openLwipRxCh(virtNetif->hCpswProxy,
                               outArgs->hUdmaDrv,
                               gVirtNetifObj.useDefaultRxFlow,
                               rxStartFlowId,
                               rxFlowIdOffset,
                               virtNetif->macAddr,
                               &inArgs->rxCfg[0U],
                               &outArgs->rxInfo[0U],
                               outArgs->hostPortRxMtu,
                               isSwitchPort);
}

void LwipifEnetAppCb_releaseHandle(LwipifEnetAppIf_ReleaseHandleInfo *releaseInfo)
{
    CpswRemoteApp_VirtNetif *virtNetif;
    bool isSwitchPort;

    virtNetif = container_of(releaseInfo->netif, CpswRemoteApp_VirtNetif, netif);
    EnetAppUtils_assert(virtNetif->hCpswProxy != NULL);

    isSwitchPort = CpswProxy_isSwitchPort(virtNetif->virtPort);

    CpswRemoteApp_closeLwipTxCh(virtNetif->hCpswProxy,
                                releaseInfo->hUdmaDrv,
                                &releaseInfo->txInfo,
                                releaseInfo->txFreePkt.cbArg,
                                releaseInfo->txFreePkt.cb);

    CpswRemoteApp_closeLwipRxCh(virtNetif->hCpswProxy,
                                releaseInfo->hUdmaDrv,
                                gVirtNetifObj.useDefaultRxFlow,
                                virtNetif->ipv4Addr,
                                &releaseInfo->rxInfo[0U],
                                releaseInfo->rxFreePkt[0U].cbArg,
                                releaseInfo->rxFreePkt[0U].cb,
                                isSwitchPort);

    CpswProxy_detach(virtNetif->hCpswProxy);
}

#if defined(ETHFW_MONITOR_SUPPORT)
void LwipifEnetAppCb_openDma(LwipifEnetAppIf_GetHandleInArgs *inArgs,
                             LwipifEnetAppIf_GetHandleOutArgs *outArgs)
{
    EnetUdma_OpenTxChPrms cpswTxChCfg;
    EnetUdma_OpenRxFlowPrms cpswRxFlowCfg;
    LwipifEnetAppIf_RxHandleInfo *rxInfo;
    LwipifEnetAppIf_RxConfig *rxCfg;
    CpswRemoteApp_VirtNetif *virtNetif;

    virtNetif = container_of(inArgs->netif, CpswRemoteApp_VirtNetif, netif);
    EnetAppUtils_assert(virtNetif->hCpswProxy != NULL);

    /* Set configuration parameters */
    EnetUdma_initTxChParams(&cpswTxChCfg);
    EnetAppUtils_setCommonTxChPrms(&cpswTxChCfg);
    cpswTxChCfg.hUdmaDrv  = outArgs->hUdmaDrv;
    cpswTxChCfg.useProxy  = BTRUE;
    cpswTxChCfg.numTxPkts = inArgs->txCfg.numPackets;
    cpswTxChCfg.cbArg     = inArgs->txCfg.cbArg;
    cpswTxChCfg.notifyCb  = inArgs->txCfg.notifyCb;
    cpswTxChCfg.chNum     = outArgs->txInfo.txChNum;

    outArgs->txInfo.hTxChannel = EnetUdma_openTxCh(gVirtNetifObj.hEnetDma, &cpswTxChCfg);
    EnetAppUtils_assert(NULL != outArgs->txInfo.hTxChannel);

    rxInfo = &outArgs->rxInfo[0U];
    rxCfg = &inArgs->rxCfg[0U];

    EnetUdma_initRxFlowParams(&cpswRxFlowCfg);
    EnetAppUtils_setCommonRxFlowPrms(&cpswRxFlowCfg);

    cpswRxFlowCfg.notifyCb  = rxCfg->notifyCb;
    cpswRxFlowCfg.numRxPkts = rxCfg->numPackets;
    cpswRxFlowCfg.hUdmaDrv  = outArgs->hUdmaDrv;;
    cpswRxFlowCfg.cbArg     = rxCfg->cbArg;
    cpswRxFlowCfg.useGlobalEvt = BTRUE;
    cpswRxFlowCfg.useProxy  = BFALSE;
    cpswRxFlowCfg.rxFlowMtu = outArgs->hostPortRxMtu;
    cpswRxFlowCfg.startIdx  = rxInfo->rxFlowStartIdx;
    cpswRxFlowCfg.flowIdx   = rxInfo->rxFlowIdx;

    outArgs->rxInfo[0U].hRxFlow = EnetUdma_openRxFlow(gVirtNetifObj.hEnetDma, &cpswRxFlowCfg);
    EnetAppUtils_assert(outArgs->rxInfo[0U].hRxFlow != NULL);

    CpswProxy_registerDstMacRxFlow(virtNetif->hCpswProxy,
                                   rxInfo->rxFlowStartIdx,
                                   rxInfo->rxFlowIdx,
                                   rxInfo->macAddr);
}

void LwipifEnetAppCb_closeDma(LwipifEnetAppIf_ReleaseHandleInfo *releaseInfo)
{
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    int32_t status = ENET_SOK;
    CpswRemoteApp_VirtNetif *virtNetif;

    virtNetif = container_of(releaseInfo->netif, CpswRemoteApp_VirtNetif, netif);
    EnetAppUtils_assert(virtNetif->hCpswProxy != NULL);

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetAppUtils_assert(NULL != releaseInfo->txInfo.hTxChannel);
    EnetAppUtils_assert(NULL != releaseInfo->rxInfo[0U].hRxFlow);

    EnetDma_disableTxEvent(releaseInfo->txInfo.hTxChannel);
    status = EnetUdma_closeTxCh(releaseInfo->txInfo.hTxChannel, &fqPktInfoQ, &cqPktInfoQ);
    releaseInfo->txInfo.hTxChannel = NULL;

    releaseInfo->txFreePkt.cb(releaseInfo->txFreePkt.cbArg, &fqPktInfoQ, &cqPktInfoQ);

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetDma_disableRxEvent(releaseInfo->rxInfo[0U].hRxFlow);

    CpswProxy_unregisterDstMacRxFlow(virtNetif->hCpswProxy,
                                     releaseInfo->rxInfo[0U].rxFlowStartIdx,
                                     releaseInfo->rxInfo[0U].rxFlowIdx,
                                     releaseInfo->rxInfo[0U].macAddr);

    status = EnetUdma_closeRxFlow(releaseInfo->rxInfo[0U].hRxFlow,
                               &fqPktInfoQ,
                               &cqPktInfoQ);

    /* Put some delay for flow to close by Sciclient.
     * Temp change will be removed when gPTP_stop issue is resolved. */
    EthFwOsal_sleepTask(200U);
    releaseInfo->rxInfo[0U].hRxFlow = NULL;

    releaseInfo->rxFreePkt[0U].cb(releaseInfo->rxFreePkt[0U].cbArg, &fqPktInfoQ, &cqPktInfoQ);

}

static void EthApp_openDma(struct netif *netif)
{
    /* Open the lwIP DMA channels and issue link up to lwIP stack */
    LWIPIF_LWIP_openDma(netif);

    sys_lock_tcpip_core();
    netif_set_link_up(netif);
    sys_unlock_tcpip_core();
}

static void EthApp_closeDma(struct netif *netif)
{
    /* Issue link down to lwIP stack and close the lwIP DMA channels */
    sys_lock_tcpip_core();
    netif_set_link_down(netif);
    sys_unlock_tcpip_core();

    /* Close the lwIP DMA channels */
    LWIPIF_LWIP_closeDma(netif);
}

static void CpswRemoteApp_hwRecoveryNotify(uint32_t notifyType,
                                           void *notifyArg,
                                           void *cbArg)
{
    CpswRemoteApp_VirtNetif *virtNetif = (CpswRemoteApp_VirtNetif *)cbArg;
    CpswRemoteApp_HwRecoveryMsg msgMbx;
    bool isMacPort;
    uint32_t portNum;

    if (virtNetif != NULL)
    {
        isMacPort = CpswProxy_isMacPort(virtNetif->virtPort);
        portNum = CpswProxy_getPortNum(virtNetif->virtPort);

        if (notifyType == ETHREMOTECFG_NOTIFY_HWERROR)
        {
            msgMbx.virtNetif = virtNetif;
            msgMbx.notifyType = notifyType;
            ETHFWTRACE_INFO("Virtual %s port %u: HWERROR notify received",
                            isMacPort ? "MAC" : "switch", portNum);
        MailboxP_post(gVirtNetifObj.hMbx,  (void *)&msgMbx, MailboxP_WAIT_FOREVER);
        }
        else if (notifyType == ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE)
        {
            msgMbx.virtNetif = virtNetif;
            msgMbx.notifyType = notifyType;
            ETHFWTRACE_INFO("Virtual %s port %u: HWRECOVERY_COMPLETE notify received",
                    isMacPort ? "MAC" : "switch", portNum);
            MailboxP_post(gVirtNetifObj.hMbx, (void *)&msgMbx, MailboxP_WAIT_FOREVER);
        }
        else
        {
            ETHFWTRACE_ERR(CPSWPROXY_EUNEXPECTED, "Unexpected HW recovery notify Type");
        }
    }
    else
    {
        ETHFWTRACE_ERR(CPSWPROXY_EUNEXPECTED, "Unexpected HW recovery notify callback argument");
    }
}

static void CpswRemoteApp_hwRecoveryTask(void *a0,
                                         void *a1)
{
    CpswRemoteApp_HwRecoveryMsg msgMbx;
    volatile bool exitTask = BFALSE;
    bool isMacPort;
    uint32_t portNum;

    while (!exitTask)
    {
        MailboxP_pend(gVirtNetifObj.hMbx,  (void *)&msgMbx, MailboxP_WAIT_FOREVER);

        isMacPort = CpswProxy_isMacPort(msgMbx.virtNetif->virtPort);
        portNum = CpswProxy_getPortNum(msgMbx.virtNetif->virtPort);

        if (msgMbx.notifyType == ETHREMOTECFG_NOTIFY_HWERROR)
        {
            ETHFWTRACE_INFO("Virtual %s port %u: Initiating DMA channel teardown",
                            isMacPort ? "MAC" : "switch", portNum);
            EthApp_closeDma(&msgMbx.virtNetif->netif);

            /* Send teardown completion */
            ETHFWTRACE_INFO("Virtual %s port %u: DMA teardown complete, notify ETHFW",
                            isMacPort ? "MAC" : "switch", portNum);
            CpswProxy_teardownCompletion(msgMbx.virtNetif->hCpswProxy);
        }
        else if (msgMbx.notifyType == ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE)
        {
            ETHFWTRACE_INFO("Virtual %s port %u: Opening back DMA channel",
                            isMacPort ? "MAC" : "switch", portNum);
            EthApp_openDma(&msgMbx.virtNetif->netif);
        }
        else
        {
            ETHFWTRACE_ERR(CPSWPROXY_EUNEXPECTED, "Unexpected HW recovery notify Type");
        }
    }
}
#endif
