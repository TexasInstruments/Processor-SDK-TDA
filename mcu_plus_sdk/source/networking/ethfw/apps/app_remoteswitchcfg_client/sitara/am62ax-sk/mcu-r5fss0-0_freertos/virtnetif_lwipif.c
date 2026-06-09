/*
 *
 * Copyright (c) 2024-25 Texas Instruments Incorporated
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
#include <cslr_gtc.h>
#include <ethremotecfg/client/include/cpsw_proxy.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

#include "ti_enet_config.h"

#include <enet.h>
#include <include/per/cpsw.h>
#include <utils/include/enet_apputils.h>
#include <utils/include/enet_appmemutils_cfg.h>
#include <utils/include/enet_appmemutils.h>

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

#include <lwipif2enet_appif.h>
#include <lwip2lwipif.h>
#include <custom_pbuf.h>
#include "lwip2enet.h"

#if defined(ETHAPP_ENABLE_IPERF_SERVER)
#include "examples/lwiperf/lwiperf_example.h"
#endif

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
#include <enet/core/lwipific_tap/inc/netif_ic.h>
#include <enet/core/lwipific_tap/inc/lwip2enet_ic.h>
#include <enet/core/lwipific_tap/inc/lwip2lwipif_ic.h>
#endif

#if defined(ENABLE_MAC_ONLY_PORTS)
#define CPSW_REMOTE_APP_REMOTE_NETIF_MAX      (2U)
#else
#define CPSW_REMOTE_APP_REMOTE_NETIF_MAX      (1U)
#endif

#define CPSW_REMOTE_APP_PACKET_POLL_PERIOD_US (1000U)
#define CPSW_REMOTE_APP_GTC_PUSHEVT_BIT_SEL   (30U)

#define ETHAPP_HWRECOVERY_TASK_PRI              (2U)

/* Mailbox used to pass virtual netifs to be closed for recovery */
#define ETHAPP_VIRTNETIF_MBX_MSGSIZE            (64U)
#define ETHAPP_VIRTNETIF_MBX_MSGCOUNT           (4U)
#if defined(SAFERTOS)
#define ETHAPP_VIRTNETIF_MBX_SIZE               ((ETHAPP_VIRTNETIF_MBX_MSGSIZE * \
                                                  ETHAPP_VIRTNETIF_MBX_MSGCOUNT) + \
                                                 safertosapiQUEUE_OVERHEAD_BYTES)
#else
#define ETHAPP_VIRTNETIF_MBX_SIZE               (ETHAPP_VIRTNETIF_MBX_MSGSIZE * \
                                                 ETHAPP_VIRTNETIF_MBX_MSGCOUNT)
#endif

#if defined(SAFERTOS)
#define ETHAPP_MONITOR_TASK_STACKSIZE              (16U * 1024U)
#define ETHAPP_MONITOR_TASK_STACKALIGN             ETHAPP_LWIP_TASK_STACKSIZE
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

/* Ethernet Netif IDs */
#define ETHAPP_ETHNETIF_0               (0U)
#define ETHAPP_ETHNETIF_1               (1U)
#define ETHAPP_ETHNETIF_MAX             (2U)

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
static uint32_t netif_ic_state[IC_ETH_MAX_VIRTUAL_IF] =
{
    IC_ETH_IF_MCU2_0_MCU2_1,
    IC_ETH_IF_MCU2_1_MCU2_0,
    IC_ETH_IF_MCU2_0_A72
};

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

#define RTOS_CLIENT_ENETLWIP_PACKET_POLL_PERIOD_US (1000U)

/*! \brief RX packet task stack size */
#define RTOS_CLIENT_LWIPIF_RX_PACKET_TASK_STACK    (2048U)

/*! \brief TX packet task stack size */
#define RTOS_CLIENT_LWIPIF_TX_PACKET_TASK_STACK    (2048U)

/*! \brief Links status poll task stack size */
#if (_DEBUG_ == 1)
#define RTOS_CLIENT_LWIPIF_POLL_TASK_STACK         (3072U)
#else
#define RTOS_CLIENT_LWIPIF_POLL_TASK_STACK         (2048U)
#endif

#define RTOS_CLIENT_LWIPIF_NUM_RX_PACKET_TASKS     (1U)

#define RTOS_CLIENT_LWIPIF_NUM_TX_PACKET_TASKS     (1U)

#define RTOS_CLIENT_RX_CHANNELID_COUNT             (1U)

#define RTOS_CLIENT_TX_CHANNELID_COUNT             (1U)

#define RTOS_CLIENT_ENETLWIP_PACKET_POLL_PERIOD_US (1000U)

#define RTOS_CLIENT_ENETLWIP_APP_POLL_PERIOD       (500U)

#define OS_TASKPRIHIGH                             (8U)

#define RTOS_CLIENT_LWIPIF_RX_PACKET_TASK_PRI      (OS_TASKPRIHIGH)

#define RTOS_CLIENT_LWIPIF_TX_PACKET_TASK_PRI      (OS_TASKPRIHIGH)

#define RTOS_CLIENT_LWIP_POLL_TASK_PRI             (OS_TASKPRIHIGH - 1U)


typedef void* LwipifEnetApp_Handle;

/* Handle to the Application interface for the LwIPIf Layer */
LwipifEnetApp_Handle hLwipIfApp = NULL;

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

    /*! Max TX packet size per priority. */
    uint32_t txMtu;

    /*! Max RX packet size. */
    uint32_t hostPortRxMtu;

    /* Enet DMA transmit channel. */
    EnetDma_TxChHandle hTxChannel;

    /*! TX channel peer id. */
    uint32_t txChNum;

    /*! Enet DMA receive channel. */
    EnetDma_RxChHandle hRxFlow;

    /*! UDMA flow index for flow used. */
    uint32_t rxFlowStartIdx;

    /*! UDMA Flow index for flow used. */
    uint32_t rxFlowIdx;

    /* lwIP network interface */
    struct netif netif;

    /* DHCP network interface */
    struct dhcp dhcpNetif;

    /* Whether this netif should be set as the default netif */
    bool isDfltNetif;

    /* Whether the initialization of this netif is complete or not */
    bool isInitDone;

} CpswRemoteApp_VirtNetif;

typedef struct CpswRemoteApp_VirtNetifObj_s
{
    /* Virtual network interface data */
    CpswRemoteApp_VirtNetif virtNetif[CPSW_REMOTE_APP_REMOTE_NETIF_MAX];

    /* Enet peripheral type */
    Enet_Type enetType;

    /* Enet peripheral instance id */
    uint32_t instId;

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
    EthFwOsal_MailboxHandle hMbx;

    /* Mailbox buffer for storing all the command messages */
    uint8_t mbxBuf[ETHAPP_VIRTNETIF_MBX_SIZE] __attribute__ ((aligned(32)));
#endif
} CpswRemoteApp_VirtNetifObj;

typedef struct CpswRemoteApp_HwRecoveryMsg_s
{
    /* Virtual network interface data */
    CpswRemoteApp_VirtNetif *virtNetif;

    /* Notify type */
    uint32_t notifyType;
} CpswRemoteApp_HwRecoveryMsg;

typedef struct LwipifEnetApp_RxTaskInfo_s
{
    TaskP_Object      task;
    uint8_t stack[RTOS_CLIENT_LWIPIF_RX_PACKET_TASK_STACK] __attribute__ ((aligned(sizeof(long long))));
    SemaphoreP_Object sem;
    struct netif *netif;
    /*
     * Handle to counting shutdown semaphore, which all subtasks created in the
     * open function must post before the close operation can complete.
     */
    SemaphoreP_Object shutDownSemObj;
    /** Boolean to indicate shutDownFlag status of translation layer.*/
    volatile bool shutDownFlag;
} LwipifEnetApp_RxTaskInfo;

typedef struct LwipifEnetApp_TxTaskInfo_s
{
    TaskP_Object      task;
    uint8_t stack[RTOS_CLIENT_LWIPIF_TX_PACKET_TASK_STACK] __attribute__ ((aligned(sizeof(long long))));
    SemaphoreP_Object sem;
    struct netif *netif;
    /*
     * Handle to counting shutdown semaphore, which all subtasks created in the
     * open function must post before the close operation can complete.
     */
    SemaphoreP_Object shutDownSemObj;
    /** Boolean to indicate shutDownFlag status of translation layer.*/
    volatile bool shutDownFlag;
} LwipifEnetApp_TxTaskInfo;

typedef struct LwipifEnetApp_PollTaskInfo_s
{
    TaskP_Object      task;
    uint8_t stack[RTOS_CLIENT_LWIPIF_POLL_TASK_STACK] __attribute__ ((aligned(sizeof(long long))));
    SemaphoreP_Object sem;
    struct netif *netif;
    /*
     * Handle to counting shutdown semaphore, which all subtasks created in the
     * open function must post before the close operation can complete.
     */
    SemaphoreP_Object shutDownSemObj;
    /** Boolean to indicate shutDownFlag status of translation layer.*/
    volatile bool shutDownFlag;

    /*
     * Clock handle for triggering the packet Rx notify
     */
    ClockP_Object pollLinkClkObj;
} LwipifEnetApp_PollTaskInfo;


typedef struct LwipifEnetApp_TaskInfo_s
{
    LwipifEnetApp_RxTaskInfo rxTask[RTOS_CLIENT_LWIPIF_NUM_RX_PACKET_TASKS];
    LwipifEnetApp_TxTaskInfo txTask[RTOS_CLIENT_LWIPIF_NUM_TX_PACKET_TASKS];
    LwipifEnetApp_PollTaskInfo pollTask;
} LwipifEnetApp_TaskInfo;

typedef struct LwipifEnetApp_Object_s
{
    LwipifEnetApp_TaskInfo task;
    /*
    * Clock handle for triggering the packet Rx notify
    */
    ClockP_Object pollLinkClkObj;
} LwipifEnetApp_Object;

/* Link status on these ports will be used to determine link up on virtual switch port */
static Enet_MacPort gRemoteAppMacPorts[] =
{
    ENET_MAC_PORT_1,
};

#if (CPSW_REMOTE_APP_REMOTE_NETIF_MAX >= 2)  && defined(ENABLE_MAC_ONLY_PORTS)
/* Link status on these ports will be used to determine link up on virtual MAC port */
static Enet_MacPort gRemoteApp_virtualMacPorts[] =
{
    ENET_MAC_PORT_2,
};
#endif

CpswRemoteApp_VirtNetifObj gVirtNetifObj =
{
    .enetType = ENET_CPSW_3G,
    .instId   = 0U,
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
            .isInitDone  = BFALSE,
        },
#if (CPSW_REMOTE_APP_REMOTE_NETIF_MAX >= 2) && defined(ENABLE_MAC_ONLY_PORTS)
        {
            .hCpswProxy  = NULL,
            .virtPort    = ETHREMOTECFG_MAC_PORT_2,
            .macPorts    = gRemoteApp_virtualMacPorts,
            .numMacPorts = ENET_ARRAYSIZE(gRemoteApp_virtualMacPorts),
            .isDfltNetif = BFALSE,
            .isInitDone  = BFALSE,
        },
#endif
    },
};

/* For Cpdma Rx scatter-gather implies that #rxpkts = #rxbuffers = #rxpbufs
   For Udma's static Rx scatter-gather, #rxbuffers = #rxpbufs = 4 * #rxpkts */
#if UDMA_STATIC_RX_SG_ENABLE
LWIP_MEMPOOL_DECLARE(RX_POOL, ENET_SYSCFG_TOTAL_NUM_RX_PKT * 4, sizeof(Rx_CustomPbuf), "Rx Custom Pbuf pool");
/* These must be sufficient for total number of rx pbufs and tx packets */
pbufNode gFreePbufArr[ENET_SYSCFG_TOTAL_NUM_RX_PKT * 5];
#else
LWIP_MEMPOOL_DECLARE(RX_POOL, ENET_SYSCFG_TOTAL_NUM_RX_PKT, sizeof(Rx_CustomPbuf), "Rx Custom Pbuf pool");
/* These must be sufficient for total number of rx pbufs and tx packets */
pbufNode gFreePbufArr[ENET_SYSCFG_TOTAL_NUM_RX_PKT * 2];
#endif

static LwipifEnetApp_Object gLwipifEnetAppObj;

Udma_DrvObject gUdmaDrvObj;

static int32_t CpswRemoteApp_openUdma(void);

static void CpswRemoteApp_openCpswProxy(CpswRemoteApp_VirtNetif *virtNetif);

void EthApp_initLwip(void *arg);

static void EthApp_initNetif(CpswRemoteApp_VirtNetif *virtNetif);

static void EthApp_virtNetifStatusCb(struct netif *netif);

static void EthApp_lwipNetifStatusCb(struct netif *netif);

void CpswRemoteApp_initVirtNetif(Enet_Type enetType, uint32_t instId);

void EthApp_initLwip(void *arg);

#if defined(ETHFW_MONITOR_SUPPORT)
static void EthApp_openDma(struct netif *netif);

static void EthApp_closeDma(struct netif *netif);

static void CpswRemoteApp_hwRecoveryTask(void *a0);

static void CpswRemoteApp_hwRecoveryNotify(uint32_t notifyType,
                                           void *notifyArg,
                                           void *cbArg);
#endif

static void EthApp_netifLinkChangeCb(struct netif *pNetif);

void LwipifEnetApp_startSchedule(LwipifEnetApp_Handle handle, struct netif *netif);

static void LwipifEnetApp_poll(void *arg);

static void LwipifEnetApp_postPollLink(ClockP_Object *clkObj, void *arg);

static void LwipifEnetApp_rxPacketTask(void *arg);

static void LwipifEnetApp_txPacketTask(void *arg);

void LwipifEnetApp_createTxPktHandlerTask(LwipifEnetApp_TxTaskInfo* pTxTaskInfo, struct netif *netif);

void LwipifEnetApp_createRxPktHandlerTask(LwipifEnetApp_RxTaskInfo* pRxTaskInfo, struct netif *netif);

LwipifEnetApp_Handle LwipifEnetApp_getHandle()
{
    return (LwipifEnetApp_Handle)&gLwipifEnetAppObj;
}

void CpswRemoteApp_initVirtNetif(Enet_Type enetType, uint32_t instId)
{
    EnetDma_initCfg dmaCfg;
#if defined(ETHFW_MONITOR_SUPPORT)
    EthFwOsal_MailboxParams mbxParams;
    EthFwOsal_TaskParams params;
#endif
    uint32_t i = 0U;
    int32_t status = ENET_SOK;

    /* Init UDMA LLD based on NAVSS instance */
    status = CpswRemoteApp_openUdma();
    if (status != UDMA_SOK)
    {
        EnetAppUtils_print("Failed to open UDMA LLD");
    }

    /* Initialize Enet memutils */
    status = EnetMem_init();
    if (status != ENET_SOK)
    {
        EnetAppUtils_print("Failed to init memutils");
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
            EnetAppUtils_print("Failed to init Enet LLD data path");
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
    EthFwOsal_initMailboxParam(&mbxParams);
    mbxParams.name    = (uint8_t *)"VirtNetif Mbox";
    mbxParams.size    = ETHAPP_VIRTNETIF_MBX_MSGSIZE;
    mbxParams.count   = ETHAPP_VIRTNETIF_MBX_MSGCOUNT;
    mbxParams.buf     = (void *)&gVirtNetifObj.mbxBuf[0U];
    mbxParams.bufsize = sizeof(gVirtNetifObj.mbxBuf);

    gVirtNetifObj.hMbx = EthFwOsal_createMailbox(&mbxParams);

    /* Step 8: Create HW recovery task */
    EthFwOsal_initTaskParam(&params);
    params.name      = "HW Recovery Task";
    params.priority  = ETHAPP_HWRECOVERY_TASK_PRI;
    params.arg0      = (void *)&gVirtNetifObj;
    params.stack     = &gEthAppRecoveryStackBuf[0];
    params.stacksize = sizeof(gEthAppRecoveryStackBuf);

    EthFwOsal_createTask(&CpswRemoteApp_hwRecoveryTask, &params);
#endif
}

static int32_t CpswRemoteApp_openUdma(void)
{
    Udma_InitPrms initPrms;
    Udma_DrvHandle hUdmaDrv = &gUdmaDrvObj;
    uint32_t instId = UDMA_INST_ID_PKTDMA_0;
    int32_t status;

    /* Initialize the UDMA driver based on NAVSS instance */
    UdmaInitPrms_init(instId, &initPrms);

    status = Udma_init(hUdmaDrv, &initPrms);
    if (status != UDMA_SOK)
    {
        EnetAppUtils_print("Failed init UDMA driver");
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
    pTxChPrms->useGlobalEvt = BFALSE;
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
        EnetAppUtils_print("Failed to open CpswProxy for virtPortId %u",
                       virtNetif->virtPort);
        EnetAppUtils_assert(hProxy != NULL);
    }
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

    ClockP_sleep(1);

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

    hLwipIfApp = LwipifEnetApp_getHandle();

    if(CpswProxy_isSwitchPort(virtNetif->virtPort))
    {
#if ETHAPP_LWIP_USE_DHCP
        EnetAppUtils_print("Starting lwIP, local interface IP is dhcp-enabled\r\n");
#else /* ETHAPP_LWIP_USE_DHCP */
        ETHFW_CLIENT_GW_SWITCH_PORT(&gw);
        ETHFW_CLIENT_IPADDR_SWITCH_PORT(&ipaddr);
        ETHFW_CLIENT_NETMASK_SWITCH_PORT(&netmask);
        EnetAppUtils_print("Starting lwIP, local interface IP is %s\r\n", ip4addr_ntoa(&ipaddr));
#endif /* ETHAPP_LWIP_USE_DHCP */

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
        /* Create Enet LLD ethernet interface */
        netif_add(netif, NULL, NULL, NULL, NULL, LWIPIF_LWIP_init, tcpip_input);
#if LWIP_CHECKSUM_CTRL_PER_NETIF
        NETIF_SET_CHECKSUM_CTRL(netif, chksumFlags);
#endif

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

        LWIPIF_LWIP_start(gVirtNetifObj.enetType, gVirtNetifObj.instId, netif, ETHAPP_ETHNETIF_0);
        netif_set_link_callback(netif, EthApp_netifLinkChangeCb);

    if (virtNetif->isDfltNetif)
    {
        netif_set_default(netif);
    }
#if LWIP_CHECKSUM_CTRL_PER_NETIF
        NETIF_SET_CHECKSUM_CTRL(netif, chksumFlags);
#endif
#endif

        LwipifEnetApp_startSchedule(hLwipIfApp, netif);

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
        EnetAppUtils_print("Starting lwIP, local interface IP is dhcp-enabled\r\n");
#else /* ETHAPP_LWIP_USE_DHCP */
        ETHFW_CLIENT_GW_MAC_PORT(&gw);
        ETHFW_CLIENT_IPADDR_MAC_PORT(&ipaddr);
        ETHFW_CLIENT_NETMASK_MAC_PORT(&netmask);
        ETHFWTRACE_INFO("Starting lwIP, local interface IP is %s\r\n", ip4addr_ntoa(&ipaddr));
#endif /* ETHAPP_LWIP_USE_DHCP */

        netif_add(netif, &ipaddr, &netmask, &gw, NULL, LWIPIF_LWIP_init, tcpip_input);
        LWIPIF_LWIP_start(gVirtNetifObj.enetType, gVirtNetifObj.instId, netif, ETHAPP_ETHNETIF_1);

        netif_set_link_callback(netif, EthApp_netifLinkChangeCb);

        if (virtNetif->isDfltNetif)
        {
            netif_set_default(netif);
        }

#if LWIP_CHECKSUM_CTRL_PER_NETIF
        NETIF_SET_CHECKSUM_CTRL(netif, chksumFlags);
#endif

        netif_set_status_callback(netif, EthApp_virtNetifStatusCb);

        dhcp_set_struct(netif, &virtNetif->dhcpNetif);

        netif_set_up(netif);

#if ETHAPP_LWIP_USE_DHCP
        err = dhcp_start(netif);
        if(err != ERR_OK)
        {
            EnetAppUtils_print("Failed to start DHCP");
        }
#endif /* ETHAPP_LWIP_USE_DHCP */
    }

    virtNetif->isInitDone = BTRUE;
}

static void EthApp_virtNetifStatusCb(struct netif *netif)
{
    CpswRemoteApp_VirtNetif *virtNetif;

    virtNetif = container_of(netif, CpswRemoteApp_VirtNetif, netif);

    if (netif_is_up(netif))
    {
        const ip4_addr_t *ipAddr = netif_ip4_addr(netif);

        EnetAppUtils_print("Added interface '%c%c%d', IP is %s\r\n",
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
        EnetAppUtils_print("Removed interface '%c%c%d'", netif->name[0], netif->name[1], netif->num);
    }
}

static void EthApp_netifLinkChangeCb(struct netif *pNetif)
{
    if (netif_is_link_up(pNetif))
    {
        EnetAppUtils_print("[%d]Network Link UP Event\r\n", pNetif->num);
    }
    else
    {
        EnetAppUtils_print("[%d]Network Link DOWN Event\r\n", pNetif->num);
    }
}

static void EthApp_lwipNetifStatusCb(struct netif *netif)
{
    if (netif_is_up(netif))
    {
        const ip4_addr_t *ipAddr = netif_ip4_addr(netif);

        EnetAppUtils_print("Added interface '%c%c%d', IP is %s",
                        netif->name[0], netif->name[1], netif->num, ip4addr_ntoa(ipAddr));
    }
    else
    {
        EnetAppUtils_print("Removed interface '%c%c%d'", netif->name[0], netif->name[1], netif->num);
    }
}

static bool LwipifEnetAppCb_isPortLinked(Enet_Handle hEnet)
{
    bool isLinked = BFALSE;
    CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];

    isLinked = (isLinked || CpswProxy_isPhyLinked((CpswProxy_Handle)virtNetif->hCpswProxy));

    return isLinked;
}

void LwipifEnetAppCb_getEnetLwipIfInstInfo(Enet_Type enetType, uint32_t instId, LwipifEnetAppIf_GetEnetLwipIfInstInfo *outArgs)
{
    CpswRemoteApp_VirtNetif *virtNetif = NULL;
    uint32_t numTxCh;
    uint32_t numRxFlow;

    for (uint32_t i = 0; i < ENET_ARRAYSIZE(gVirtNetifObj.virtNetif); i++)
    {
        virtNetif = &gVirtNetifObj.virtNetif[i];

        CpswProxy_attach(virtNetif->hCpswProxy,
                         virtNetif->virtPort,
                         &outArgs->hostPortRxMtu,
                         &outArgs->txMtu[0],
                         &numTxCh,
                         &numRxFlow,
                         &virtNetif->features);

        EnetAppUtils_assert(numTxCh != 0U);
        EnetAppUtils_assert(numRxFlow != 0U);

        virtNetif->hostPortRxMtu = outArgs->hostPortRxMtu;
        virtNetif->txMtu = outArgs->txMtu[0];
    }

    outArgs->isPortLinkedFxn = &LwipifEnetAppCb_isPortLinked;
    outArgs->timerPeriodUs   = RTOS_CLIENT_ENETLWIP_PACKET_POLL_PERIOD_US;
    outArgs->pPbufInfo = gFreePbufArr;
    outArgs->pPbufInfoSize = sizeof(gFreePbufArr)/sizeof(pbufNode);
    LWIP_MEMPOOL_INIT(RX_POOL);
}

void LwipifEnetAppCb_getTxHandleInfo(LwipifEnetAppIf_GetTxHandleInArgs *inArgs,
                                     LwipifEnetAppIf_TxHandleInfo *outArgs)
{
	CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];
    EnetUdma_OpenTxChPrms cpswTxChCfg;
    EnetDma_Pkt *pPktInfo;
    uint32_t txPSILId;
    uint32_t i = 0U;
    uint32_t relChPriority = 0U;


    CpswProxy_allocTxCh(virtNetif->hCpswProxy,
                        &txPSILId,
                        relChPriority);

    /* Set configuration parameters */
    EnetUdma_initTxChParams(&cpswTxChCfg);

    CpswRemoteApp_setTxChPrms(&cpswTxChCfg,
                              txPSILId,
                              gVirtNetifObj.hUdmaDrv,
                              ENET_SYSCFG_TOTAL_NUM_TX_PKT,
                              inArgs->cbArg,
                              inArgs->notifyCb);

    outArgs->hTxChannel = EnetUdma_openTxCh(gVirtNetifObj.hEnetDma, &cpswTxChCfg);
    EnetAppUtils_assert(outArgs->hTxChannel != NULL);
    outArgs->txChNum = txPSILId;
    outArgs->numPackets = ENET_SYSCFG_TOTAL_NUM_TX_PKT;
    outArgs->disableEvent = true;

    for(uint32_t i = 0; i< ENET_ARRAYSIZE(gVirtNetifObj.virtNetif); i++)
    {
        gVirtNetifObj.virtNetif[i].hTxChannel = outArgs->hTxChannel;
        gVirtNetifObj.virtNetif[i].txChNum = txPSILId;
    }

    /* Initialize the DMA free queue */
    EnetQueue_initQ(inArgs->pktInfoQ);

    for (i = 0U; i < outArgs->numPackets; i++)
    {
        /* Initialize Pkt info Q from allocated memory */
        pPktInfo = EnetMem_allocEthPktInfoMem(&inArgs->cbArg,
                                              ENETDMA_CACHELINE_ALIGNMENT);

        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_FREEQ);
        EnetQueue_enq(inArgs->pktInfoQ, &pPktInfo->node);

    }
}

void LwipifEnetAppCb_getRxHandleInfo(LwipifEnetAppIf_GetRxHandleInArgs *inArgs,
                                     LwipifEnetAppIf_RxHandleInfo *outArgs)
{
	CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];
#if defined(ENABLE_MAC_ONLY_PORTS)
    CpswRemoteApp_VirtNetif *virtNetifMac = &gVirtNetifObj.virtNetif[1];
#endif
	EnetUdma_OpenRxFlowPrms cpswRxFlowCfg;
    uint32_t flowIdx = 0U;
    uint32_t i;
    EnetDma_Pkt *pPktInfo;
    Rx_CustomPbuf *cPbuf;
    bool useRingMon = false;
    uint32_t numCustomPbuf;
    uint32_t scatterSegments[] =
    {
        ENET_MEM_LARGE_POOL_PKT_SIZE  /* Keep this size aligned with R5F cacheline of 32B */
    };

    CpswProxy_allocRxFlow(virtNetif->hCpswProxy,
                          &virtNetif->rxFlowStartIdx,
                          &virtNetif->rxFlowIdx,
                          flowIdx);

#if defined(ENABLE_MAC_ONLY_PORTS)
    virtNetifMac->rxFlowStartIdx = virtNetif->rxFlowStartIdx;
    virtNetifMac->rxFlowIdx = virtNetif->rxFlowIdx;
#endif

    CpswProxy_allocMac(virtNetif->hCpswProxy,
                       virtNetif->macAddr);

    CpswProxy_registerDstMacRxFlow(virtNetif->hCpswProxy,
                                   virtNetif->rxFlowStartIdx,
                                   virtNetif->rxFlowIdx,
                                   virtNetif->macAddr);

#if defined(ENABLE_MAC_ONLY_PORTS)
    CpswProxy_allocMac(virtNetifMac->hCpswProxy,
                       virtNetifMac->macAddr);
    CpswProxy_registerDstMacRxFlow(virtNetifMac->hCpswProxy,
                                   virtNetifMac->rxFlowStartIdx,
                                   virtNetifMac->rxFlowIdx,
                                   virtNetifMac->macAddr);
#endif

    EnetUdma_initRxFlowParams(&cpswRxFlowCfg);

    CpswRemoteApp_setRxFlowPrms(&cpswRxFlowCfg,
                                virtNetif->rxFlowStartIdx,
                                virtNetif->rxFlowIdx,
                                gVirtNetifObj.hUdmaDrv,
                                ENET_SYSCFG_TOTAL_NUM_RX_PKT,
                                inArgs->cbArg,
                                inArgs->notifyCb,
                                virtNetif->hostPortRxMtu);

    outArgs->hRxFlow = EnetUdma_openRxFlow(gVirtNetifObj.hEnetDma, &cpswRxFlowCfg);
    EnetAppUtils_assert(outArgs->hRxFlow != NULL);
    outArgs->rxFlowStartIdx = virtNetif->rxFlowStartIdx;
    outArgs->rxFlowIdx = virtNetif->rxFlowIdx;
    outArgs->numPackets  = ENET_SYSCFG_TOTAL_NUM_RX_PKT;
    outArgs->disableEvent = !useRingMon;
    EnetUtils_copyMacAddr(&outArgs->macAddr[0U][0U], virtNetif->macAddr);
    EnetAppUtils_print("Host MAC address : ");
    EnetAppUtils_printMacAddr(&outArgs->macAddr[0U][0U]);
#if UDMA_STATIC_RX_SG_ENABLE
    numCustomPbuf = outArgs->numPackets * 4;
#else
    numCustomPbuf = outArgs->numPackets;
#endif

    virtNetif->hRxFlow = outArgs->hRxFlow;
#if defined(ENABLE_MAC_ONLY_PORTS)
    virtNetifMac->hRxFlow = outArgs->hRxFlow;
#endif

    /* Initialize the DMA free queue */
    EnetQueue_initQ(inArgs->pReadyRxPktQ);
    EnetQueue_initQ(inArgs->pFreeRxPktInfoQ);
    pbufQ_init(inArgs->pFreePbufInfoQ);

    for (i = 0U; i < outArgs->numPackets; i++)
    {

        pPktInfo = EnetMem_allocEthPkt(&inArgs->cbArg,
                           ENETDMA_CACHELINE_ALIGNMENT,
                           ENET_ARRAYSIZE(scatterSegments),
                           scatterSegments);
        EnetAppUtils_assert(pPktInfo != NULL);
        ENET_UTILS_SET_PKT_APP_STATE(&pPktInfo->pktState, ENET_PKTSTATE_APP_WITH_READYQ);

        /* Put all the filled pPktInfo into readyRxPktQ and submit to driver */
        EnetQueue_enq(inArgs->pReadyRxPktQ, &pPktInfo->node);
    }

    EnetQueue_verifyQCount(inArgs->pReadyRxPktQ);
    for (i = 0U; i < numCustomPbuf; i++)
    {
        /* Allocate the Custom Pbuf structures and put them in freePbufInfoQ */
        cPbuf = NULL;
        cPbuf = (Rx_CustomPbuf*)LWIP_MEMPOOL_ALLOC(RX_POOL);
        EnetAppUtils_assert(cPbuf != NULL);
        cPbuf->p.custom_free_function = custom_pbuf_free;
        cPbuf->customPbufArgs         = (Rx_CustomPbuf_Args)inArgs->cbArg;
        cPbuf->next                   = NULL;
        cPbuf->alivePbufCount         = 0U;
        cPbuf->orgBufLen              = 0U;
        cPbuf->orgBufPtr              = NULL;
        cPbuf->p.pbuf.flags          |= PBUF_FLAG_IS_CUSTOM;
        pbufQ_enQ(inArgs->pFreePbufInfoQ, &(cPbuf->p.pbuf));
    }
}

void EnetApp_getMacAddress(uint32_t enetRxDmaChId,
                           EnetApp_GetMacAddrOutArgs *outArgs)
{
    CpswRemoteApp_VirtNetif *virtNetif = NULL;
	outArgs->macAddressCnt = ENET_ARRAYSIZE(gVirtNetifObj.virtNetif);

    for (uint32_t i = 0; i < outArgs->macAddressCnt; i++)
    {
        virtNetif = &gVirtNetifObj.virtNetif[i];
        EnetUtils_copyMacAddr(outArgs->macAddr[i], &virtNetif->macAddr[0]);
    }
}

void LwipifEnetAppCb_releaseTxHandle(LwipifEnetAppIf_ReleaseTxHandleInfo *releaseInfo)
{
	CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    int32_t status;

    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetDma_disableTxEvent(virtNetif->hTxChannel);
    status = EnetUdma_closeTxCh(virtNetif->hTxChannel, &fqPktInfoQ, &cqPktInfoQ);
    EnetAppUtils_assert(ENET_SOK == status);

    CpswProxy_freeTxCh(virtNetif->hCpswProxy,
                       virtNetif->txChNum);
    releaseInfo->txFreePktCb(releaseInfo->txFreePktCbArg, &fqPktInfoQ, &cqPktInfoQ);
}

void LwipifEnetAppCb_releaseRxHandle(LwipifEnetAppIf_ReleaseRxHandleInfo *releaseInfo)
{
    CpswRemoteApp_VirtNetif *virtNetif = &gVirtNetifObj.virtNetif[0];
    EnetDma_PktQ fqPktInfoQ;
    EnetDma_PktQ cqPktInfoQ;
    int32_t status = ETHFW_SOK;

    /* Close RX channel */
    EnetQueue_initQ(&fqPktInfoQ);
    EnetQueue_initQ(&cqPktInfoQ);

    EnetDma_disableRxEvent(virtNetif->hRxFlow);

    CpswProxy_unregisterIPV4Addr(virtNetif->hCpswProxy, virtNetif->ipv4Addr);

    CpswProxy_unregisterDstMacRxFlow(virtNetif->hCpswProxy,
                                     virtNetif->rxFlowStartIdx,
                                     virtNetif->rxFlowIdx,
                                     virtNetif->macAddr);

    status = EnetUdma_closeRxFlow(virtNetif->hRxFlow,
                               &fqPktInfoQ,
                               &cqPktInfoQ);
    EnetAppUtils_assert(status == ENET_SOK);

    CpswProxy_freeMac(virtNetif->hCpswProxy,
                      virtNetif->macAddr);
    CpswProxy_freeRxFlow(virtNetif->hCpswProxy,
                         virtNetif->rxFlowStartIdx,
                         virtNetif->rxFlowIdx);

    releaseInfo->rxFreePktCb(releaseInfo->rxFreePktCbArg, &fqPktInfoQ, &cqPktInfoQ);
}

int32_t LwipifEnetApp_getNetifIdx(LwipifEnetApp_Handle handle, struct netif* netif)
{
    int32_t idx = 0;
    CpswRemoteApp_VirtNetif *virtNetif = NULL;

    for (idx = 0; idx < CPSW_REMOTE_APP_REMOTE_NETIF_MAX; idx++)
    {
        virtNetif = &gVirtNetifObj.virtNetif[idx];
        if (&virtNetif->netif == netif)
        {
            break;
        }
    }

     return idx;
}

void LwipifEnetApp_getRxChIDs(const Enet_Type enetType, const uint32_t instId, uint32_t netifIdx, uint32_t* pRxChIdCount, uint32_t rxChIdList[LWIPIF_MAX_RX_CHANNELS_PER_PHERIPHERAL])
{
#if defined(ENABLE_MAC_ONLY_PORTS)
    int32_t netif2ChMap[ENET_SYSCFG_NETIF_COUNT][LWIPIF_MAX_RX_CHANNELS_PER_PHERIPHERAL] = {{ENET_DMA_RX_CH0, -1,},{ENET_DMA_RX_CH0, -1,},};
#else
    int32_t netif2ChMap[ENET_SYSCFG_NETIF_COUNT][LWIPIF_MAX_RX_CHANNELS_PER_PHERIPHERAL] = {{ENET_DMA_RX_CH0, -1,},};
#endif
    uint32_t chCount;

    EnetAppUtils_assert(netifIdx < ENET_SYSCFG_NETIF_COUNT);

    for (chCount = 0U; (chCount < LWIPIF_MAX_RX_CHANNELS_PER_PHERIPHERAL) && (netif2ChMap[netifIdx][chCount] != -1); chCount++)
    {
        rxChIdList[chCount] = netif2ChMap[netifIdx][chCount];
    }

    /* verifiy the user params */
    switch (enetType)
    {
    case ENET_ICSSG_SWITCH:
    {
        EnetAppUtils_assert(chCount == 2);
        break;
    }
    case ENET_ICSSG_DUALMAC:
    case ENET_CPSW_3G:
    case ENET_CPSW_2G:
    {
        EnetAppUtils_assert(chCount == 1);
        break;
    }
    default:
    {
        EnetAppUtils_assert(false);
    }
    }
    *pRxChIdCount = chCount;
    return;
}

void LwipifEnetApp_getTxChIDs(const Enet_Type enetType, const uint32_t instId, uint32_t netifIdx, uint32_t* pTxChIdCount, uint32_t txChIdList[LWIPIF_MAX_TX_CHANNELS_PER_PHERIPHERAL])
{
#if defined(ENABLE_MAC_ONLY_PORTS)
    int32_t netif2ChMap[ENET_SYSCFG_NETIF_COUNT][LWIPIF_MAX_TX_CHANNELS_PER_PHERIPHERAL] = {{ENET_DMA_TX_CH0},{ENET_DMA_TX_CH0},};
#else
    int32_t netif2ChMap[ENET_SYSCFG_NETIF_COUNT][LWIPIF_MAX_TX_CHANNELS_PER_PHERIPHERAL] = {{ENET_DMA_TX_CH0},};
#endif
    uint32_t chCount;

    EnetAppUtils_assert(netifIdx < ENET_SYSCFG_NETIF_COUNT);

    for (chCount = 0U; (chCount < LWIPIF_MAX_TX_CHANNELS_PER_PHERIPHERAL) && (netif2ChMap[netifIdx][chCount] != -1); chCount++)
    {
        txChIdList[chCount] = netif2ChMap[netifIdx][chCount];
    }

    /* verify the user params */
    switch (enetType)
    {
        case ENET_ICSSG_SWITCH:
        case ENET_ICSSG_DUALMAC:
        case ENET_CPSW_3G:
        case ENET_CPSW_2G:
        {
            EnetAppUtils_assert(chCount == 1);
            break;
        }
        default:
        {
            EnetAppUtils_assert(false);
        }
    }

    *pTxChIdCount = chCount;
    return;
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
            EnetAppUtils_print("Virtual %s port %u: HWERROR notify received",
                            isMacPort ? "MAC" : "switch", portNum);
            EthFwOsal_postMailbox(gVirtNetifObj.hMbx,  (void *)&msgMbx, EthFwOsal_WAIT_FOREVER);
        }
        else if (notifyType == ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE)
        {
            msgMbx.virtNetif = virtNetif;
            msgMbx.notifyType = notifyType;
            EnetAppUtils_print("Virtual %s port %u: HWRECOVERY_COMPLETE notify received",
                    isMacPort ? "MAC" : "switch", portNum);
            EthFwOsal_postMailbox(gVirtNetifObj.hMbx, (void *)&msgMbx, EthFwOsal_WAIT_FOREVER);
        }
        else
        {
        	EnetAppUtils_print("Unexpected HW recovery notify Type");
        }
    }
    else
    {
        EnetAppUtils_print("Unexpected HW recovery notify callback argument");
    }
}

static void CpswRemoteApp_hwRecoveryTask(void *a0)
{
    CpswRemoteApp_HwRecoveryMsg msgMbx;
    volatile bool exitTask = BFALSE;
    bool isMacPort;
    uint32_t portNum;

    while (!exitTask)
    {
    	EthFwOsal_pendMailbox(gVirtNetifObj.hMbx,  (void *)&msgMbx, EthFwOsal_WAIT_FOREVER);

        isMacPort = CpswProxy_isMacPort(msgMbx.virtNetif->virtPort);
        portNum = CpswProxy_getPortNum(msgMbx.virtNetif->virtPort);

        if (msgMbx.notifyType == ETHREMOTECFG_NOTIFY_HWERROR)
        {
            EnetAppUtils_print("Virtual %s port %u: Initiating DMA channel teardown",
                            isMacPort ? "MAC" : "switch", portNum);
            EthApp_closeDma(&msgMbx.virtNetif->netif);

            /* Send teardown completion */
            EnetAppUtils_print("Virtual %s port %u: DMA teardown complete, notify ETHFW",
                            isMacPort ? "MAC" : "switch", portNum);
            CpswProxy_teardownCompletion(msgMbx.virtNetif->hCpswProxy);
        }
        else if (msgMbx.notifyType == ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE)
        {
        	EnetAppUtils_print("Virtual %s port %u: Opening back DMA channel",
                            isMacPort ? "MAC" : "switch", portNum);
            EthApp_openDma(&msgMbx.virtNetif->netif);
        }
        else
        {
            EnetAppUtils_print("Unexpected HW recovery notify Type");
        }
    }
}
#endif

/* ------------- Copied auto-generated code ------------------*/

static err_t LwipifEnetApp_createPollTask(LwipifEnetApp_PollTaskInfo* pPollTaskInfo)
{
    TaskP_Params params;
    int32_t status;
    ClockP_Params clkPrms;

    if (NULL != pPollTaskInfo)
    {
        /*Initialize semaphore to call synchronize the poll function with a timer*/
        status = SemaphoreP_constructBinary(&pPollTaskInfo->sem, 0U);
        EnetAppUtils_assert(status == SystemP_SUCCESS);

        /*Initialize semaphore to call synchronize the poll function with a timer*/
        status = SemaphoreP_constructBinary(&pPollTaskInfo->shutDownSemObj, 0U);
        EnetAppUtils_assert(status == SystemP_SUCCESS);

        /* Initialize the poll function as a thread */
        TaskP_Params_init(&params);
        params.name           = "Lwipif_Lwip_poll";
        params.priority       = RTOS_CLIENT_LWIP_POLL_TASK_PRI;
        params.stack          = pPollTaskInfo->stack;
        params.stackSize      = sizeof(pPollTaskInfo->stack);
        params.args           = pPollTaskInfo;
        params.taskMain       = &LwipifEnetApp_poll;

        status = TaskP_construct(&pPollTaskInfo->task, &params);
        EnetAppUtils_assert(status == SystemP_SUCCESS);

        ClockP_Params_init(&clkPrms);
        clkPrms.start     = 0;
        clkPrms.period    = RTOS_CLIENT_ENETLWIP_APP_POLL_PERIOD;
        clkPrms.args      = &pPollTaskInfo->sem; // make a proper semaphore structure for this.
        clkPrms.callback  = &LwipifEnetApp_postPollLink;
        clkPrms.timeout   = RTOS_CLIENT_ENETLWIP_APP_POLL_PERIOD;

        /* Creating timer and setting timer callback function*/
        status = ClockP_construct(&pPollTaskInfo->pollLinkClkObj, &clkPrms);
        if (status == SystemP_SUCCESS)
        {
            /* Set timer expiry time in OS ticks */
            ClockP_setTimeout(&pPollTaskInfo->pollLinkClkObj, RTOS_CLIENT_ENETLWIP_APP_POLL_PERIOD);
            ClockP_start(&pPollTaskInfo->pollLinkClkObj);
        }
        else
        {
            EnetAppUtils_assert(status == SystemP_SUCCESS);
        }

        /* Filter not defined */
        /* Inform the world that we are operational. */
        EnetAppUtils_print("[LWIPIF_LWIP] Enet has been started successfully\r\n");

        return ERR_OK;
    }
    else
    {
        return ERR_BUF;
    }
}

/*
* create a function called postEvent[i]. each event, each postfxn.
*/
static void LwipifEnetApp_postSemaphore(void *pArg)
{
    SemaphoreP_Object *pSem = (SemaphoreP_Object *) pArg;
    SemaphoreP_post(pSem);
}

void LwipifEnetApp_startSchedule(LwipifEnetApp_Handle handle, struct netif *netif
    )
{
    LwipifEnetApp_Object* pObj = (LwipifEnetApp_Object*) handle;
    uint32_t status = ENET_SOK;
    const uint32_t netifIdx = LwipifEnetApp_getNetifIdx(handle, netif);

    EnetAppUtils_assert(netifIdx < RTOS_CLIENT_LWIPIF_NUM_TX_PACKET_TASKS);
    EnetAppUtils_assert(netifIdx < RTOS_CLIENT_LWIPIF_NUM_RX_PACKET_TASKS);

    status = SemaphoreP_constructBinary(&pObj->task.txTask[netifIdx].shutDownSemObj, 0U);
    EnetAppUtils_assert(status == SystemP_SUCCESS);

    status = SemaphoreP_constructBinary(&pObj->task.rxTask[netifIdx].shutDownSemObj, 0U);
    EnetAppUtils_assert(status == SystemP_SUCCESS);

    status = SemaphoreP_constructBinary(&pObj->task.txTask[netifIdx].sem, 0U);
    EnetAppUtils_assert(status == SystemP_SUCCESS);

    status = SemaphoreP_constructBinary(&pObj->task.rxTask[netifIdx].sem, 0U);
    EnetAppUtils_assert(status == SystemP_SUCCESS);

    Enet_notify_t txNotify =
    {
            .cbFxn = &LwipifEnetApp_postSemaphore,
            .cbArg = &pObj->task.txTask[netifIdx].sem,
    };

    Enet_notify_t rxNotify =
    {
            .cbFxn = &LwipifEnetApp_postSemaphore,
            .cbArg = &pObj->task.rxTask[netifIdx].sem,
    };

    LWIPIF_LWIP_setNotifyCallbacks(netif, &rxNotify, &txNotify);

    /* Initialize Tx task*/
    pObj->task.txTask[netifIdx].shutDownFlag =false;
    LwipifEnetApp_createTxPktHandlerTask(&pObj->task.txTask[netifIdx], netif);

    /* Initialize Rx Task*/
    pObj->task.rxTask[netifIdx].shutDownFlag =false;
    LwipifEnetApp_createRxPktHandlerTask(&pObj->task.rxTask[netifIdx], netif);

    if (netifIdx == 0)
    {
        pObj->task.pollTask.shutDownFlag =false;
        pObj->task.pollTask.netif = netif; // link to the first in the list
        /* Initialize Polling task*/
        LwipifEnetApp_createPollTask(&pObj->task.pollTask);
    }

}

Lwip2Enet_RxMode_t LwipifEnetAppCb_getRxMode(Enet_Type enetType, uint32_t instId)
{
    Lwip2Enet_RxMode_t rxMode = Lwip2Enet_RxMode_SwitchSharedChannel;

    for (uint32_t i=0; i< ENET_ARRAYSIZE(gVirtNetifObj.virtNetif); i++)
    {
        if(gVirtNetifObj.virtNetif[i].isInitDone == BFALSE)
        {
            if(CpswProxy_isSwitchPort(gVirtNetifObj.virtNetif[i].virtPort))
            {
                rxMode = Lwip2Enet_RxMode_SwitchSharedChannel;
            }
            else
            {
                rxMode = Lwip2Enet_RxMode_MacSharedChannel;
            }
            break;
        }
    }
    return rxMode;
}

void LwipifEnetApp_createRxPktHandlerTask(LwipifEnetApp_RxTaskInfo* pRxTaskInfo, struct netif *netif)
{
    TaskP_Params params;
    int32_t status;
    pRxTaskInfo->netif = netif;

    /* Create RX packet task */
    TaskP_Params_init(&params);
    params.name      = "LwipifEnetApp_RxPacketTask";
    params.priority  = RTOS_CLIENT_LWIPIF_RX_PACKET_TASK_PRI;
    params.stack     = &pRxTaskInfo->stack[0];
    params.stackSize = sizeof(pRxTaskInfo->stack);
    params.args      = pRxTaskInfo;
    params.taskMain  = &LwipifEnetApp_rxPacketTask;

    status = TaskP_construct(&pRxTaskInfo->task, &params);
    EnetAppUtils_assert(status == SystemP_SUCCESS);
}

void LwipifEnetApp_createTxPktHandlerTask(LwipifEnetApp_TxTaskInfo* pTxTaskInfo, struct netif *netif)
{
    TaskP_Params params;
    int32_t status;

    pTxTaskInfo->netif = netif;
    /* Create TX packet task */
    TaskP_Params_init(&params);
    params.name = "LwipifEnetApp_TxPacketTask";
    params.priority       = RTOS_CLIENT_LWIPIF_TX_PACKET_TASK_PRI;
    params.stack          = &pTxTaskInfo->stack[0];
    params.stackSize      = sizeof(pTxTaskInfo->stack);
    params.args           = pTxTaskInfo;
    params.taskMain       = &LwipifEnetApp_txPacketTask;

    status = TaskP_construct(&pTxTaskInfo->task , &params);
    EnetAppUtils_assert(status == SystemP_SUCCESS);
}

static void LwipifEnetApp_rxPacketTask(void *arg)
{
    LwipifEnetApp_RxTaskInfo* pTaskInfo = (LwipifEnetApp_RxTaskInfo*) arg;

    while (!pTaskInfo->shutDownFlag)
    {
        /* Wait for the Rx ISR to notify us that packets are available with data */
        SemaphoreP_pend(&pTaskInfo->sem, SystemP_WAIT_FOREVER);
        if (pTaskInfo->shutDownFlag)
        {
            /* This translation layer is shutting down, don't give anything else to the stack */
            break;
        }

        LWIPIF_LWIP_rxPktHandler(pTaskInfo->netif);
    }

    /* We are shutting down, notify that we are done */
    SemaphoreP_post(&pTaskInfo->shutDownSemObj);
}

static void LwipifEnetApp_txPacketTask(void *arg)
{
    LwipifEnetApp_TxTaskInfo* pTaskInfo = (LwipifEnetApp_TxTaskInfo*) arg;

    while (!pTaskInfo->shutDownFlag)
    {
        /*
         * Wait for the Tx ISR to notify us that empty packets are available
         * that were used to send data
         */
        SemaphoreP_pend(&pTaskInfo->sem, SystemP_WAIT_FOREVER);
        LWIPIF_LWIP_txPktHandler(pTaskInfo->netif);
    }

    /* We are shutting down, notify that we are done */
    SemaphoreP_post(&pTaskInfo->shutDownSemObj);
}

static void LwipifEnetApp_poll(void *arg)
{
    /* Call the driver's periodic polling function */
    LwipifEnetApp_PollTaskInfo* pTaskInfo = (LwipifEnetApp_PollTaskInfo*)arg;

    while (!pTaskInfo->shutDownFlag)
    {
        SemaphoreP_pend(&pTaskInfo->sem, SystemP_WAIT_FOREVER);
        sys_lock_tcpip_core();
        for (uint32_t i = 0; i < CPSW_REMOTE_APP_REMOTE_NETIF_MAX; i++)
        {
            struct netif *netif = &gVirtNetifObj.virtNetif[i].netif;
            if (netif != NULL)
            {
                LWIPIF_LWIP_periodic_polling(netif);
            }
        }
        sys_unlock_tcpip_core();
    }
    SemaphoreP_post(&pTaskInfo->shutDownSemObj);
}

static void LwipifEnetApp_postPollLink(ClockP_Object *clkObj, void *arg)
{
    if (arg != NULL)
    {
        SemaphoreP_Object *hPollSem = (SemaphoreP_Object *) arg;
        SemaphoreP_post(hPollSem);
    }
}

void LwipifEnetApp_getProxyArpRxChIDs(const Enet_Type enetType, const uint32_t instId, uint32_t netifIdx, uint32_t* pRxChIdCount, uint32_t rxChIdList[LWIPIF_MAX_RX_CHANNELS_PER_PHERIPHERAL])
{
    /* For ETHFW use case, clients don't support reception for ARP packets, hence returns 0U */
    *pRxChIdCount = 0U;
}

void LwipifEnetApp_setupProxyArphandler(const Enet_Type enetType, const uint32_t instId, Lwip2Enet_RxHandle hRx)
{

    /* For ETHFW use case, clients don't support reception for ARP packets, hence returns NULL */
    hRx->handlePktFxn = NULL;
}
