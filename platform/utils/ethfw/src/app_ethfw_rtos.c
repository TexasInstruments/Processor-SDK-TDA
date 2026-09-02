/*
 *
 * Copyright (c) 2020-2026 Texas Instruments Incorporated
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
 *  \file app_ethfw_rtos.c
 *
 *  \brief Ethernet Firmware application for Vision Apps framework.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include "app_ethfw_priv.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* lwIP task stack configuration */
#if defined(SAFERTOS)
#define ETHAPP_LWIP_TASK_STACKSIZE      (16U * 1024U)
#define ETHAPP_LWIP_TASK_STACKALIGN     ETHAPP_LWIP_TASK_STACKSIZE
#else
#define ETHAPP_LWIP_TASK_STACKSIZE      (4U * 1024U)
#define ETHAPP_LWIP_TASK_STACKALIGN     (32U)
#endif

static uint8_t gEthAppLwipStackBuf[ETHAPP_LWIP_TASK_STACKSIZE] __attribute__ ((section(".bss:taskStackSection"))) __attribute__((aligned(ETHAPP_LWIP_TASK_STACKALIGN)));

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
#define ETHAPP_LWIP_USE_DHCP            (1)
#if !ETHAPP_LWIP_USE_DHCP
#define ETHFW_SERVER_IPADDR(addr)       IP4_ADDR((addr), 192,168,1,200)
#define ETHFW_SERVER_GW(addr)           IP4_ADDR((addr), 192,168,1,1)
#define ETHFW_SERVER_NETMASK(addr)      IP4_ADDR((addr), 255,255,255,0)
#endif

/* BridgeIf configuration parameters */
#define ETHAPP_LWIP_BRIDGE_MAX_PORTS (4U)
#define ETHAPP_LWIP_BRIDGE_MAX_DYNAMIC_ENTRIES (32U)
#define ETHAPP_LWIP_BRIDGE_MAX_STATIC_ENTRIES (8U)

/* BridgeIf port IDs
 * Used for creating CoreID to Bridge PortId Map
 */
#define ETHAPP_BRIDGEIF_PORT1_ID        (1U)
#define ETHAPP_BRIDGEIF_PORT2_ID        (2U)
#define ETHAPP_BRIDGEIF_CPU_PORT_ID     BRIDGEIF_MAX_PORTS

/* Inter-core netif IDs */
#define ETHAPP_NETIF_IC_MCU2_0_MCU2_1_IDX   (0U)
#define ETHAPP_NETIF_IC_MCU2_0_A72_IDX      (1U)
#define ETHAPP_NETIF_IC_MAX_IDX             (2U)

/* Max length of shared mcast address list */
#define ETHAPP_MAX_SHARED_MCAST_ADDR        (8U)

/* Required size of the MAC address pool (specific to the TI EVM configuration):
 *  1 x MAC address for Ethernet Firmware
 *  2 x MAC address for mpu1_0 virtual switch and MAC-only ports (Linux, 1 for QNX)
 *  2 x MAC address for mcu2_1 virtual switch and MAC-only ports (RTOS)
 *  1 x MAC address for mcu2_1 virtual switch port (AUTOSAR) */
#define ETHAPP_MAC_ADDR_POOL_SIZE           (6U)

/* Ethernet Netif IDs */
#define ETHAPP_ETHNETIF_0               (0U)
#define ETHAPP_ETHNETIF_1               (1U)
#define ETHAPP_ETHNETIF_MAX             (2U)

/* Default port mask for shared multicast addresses: all ports in switch mode */
#if defined(SOC_J721E)
#define ETHAPP_DFLT_PORT_MASK                   (CPSW_ALE_HOST_PORT_MASK | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_2) | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_3) | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_5) | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_6) | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_7) | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_8))
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
#define ETHAPP_DFLT_PORT_MASK                   (CPSW_ALE_HOST_PORT_MASK | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_3) | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_5))
#elif defined(SOC_J722S)
#if defined(ENABLE_MAC_ONLY_PORTS)
#define ETHAPP_DFLT_PORT_MASK                   (CPSW_ALE_HOST_PORT_MASK | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_1))
#else
#define ETHAPP_DFLT_PORT_MASK                   (CPSW_ALE_HOST_PORT_MASK | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_1) | \
                                                 CPSW_ALE_MACPORT_TO_PORTMASK(ENET_MAC_PORT_2))
#endif
#endif

#define ETHAPP_DFLT_VIRT_PORT_MASK          (ETHFW_BIT(ETHREMOTECFG_SWITCH_PORT_0) | \
                                             ETHFW_BIT(ETHREMOTECFG_SWITCH_PORT_1) | \
                                             ETHFW_BIT(ETHREMOTECFG_SWITCH_PORT_2))

/* TimeSync router SYNC0_OUT source interrupt */
#define ETHAPP_PPS_TIMESYNC_INTR_SYNC0_OUT_PIN                       (34U)
/* TimeSync router SYNC1_OUT source interrupt */
#define ETHAPP_PPS_TIMESYNC_INTR_SYNC1_OUT_PIN                       (35U)
/* TimeSync router SYNC2_OUT source interrupt */
#define ETHAPP_PPS_TIMESYNC_INTR_SYNC2_OUT_PIN                       (36U)
/* TimeSync router SYNC3_OUT source interrupt */
#define ETHAPP_PPS_TIMESYNC_INTR_SYNC3_OUT_PIN                       (37U)
/* Function generator length value for a given ref clk in Hz (1kHz) */
#define ETHAPP_PPS_TIMESYNC_REF_CLK_HZ                               (1000)
/* GenF generator instance index value */
#define ETHAPP_PPS_TIMESYNC_GENF_INST_IDX                            (0U)

/* Configurations for when IET is enabled */
#if defined(ETHFW_IET_ENABLE)
#define     MIN_FRAG_SIZE                            (1)
#define     PREMPTIVE_TRAFFIC                        (1)
#define     EXPRESS_TRAFFIC                          (0)
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

#if defined(MCU_PLUS_SDK)
/* Handle to the Application interface for the LwIPIf Layer */
LwipifEnetApp_Handle hLwipIfApp = NULL;
#endif

static EthAppObj gEthAppObj =
{
#if defined(SOC_J722S)
    .enetType = ENET_CPSW_3G,
#else
    .enetType = ENET_CPSW_9G,
#endif
    .hEthFw = NULL,
    .instId   = 0U,
};

static Enet_MacPort gEthAppPorts[] =
{
#if defined(SOC_J721E)
    /* On J721E EVM to use all 8 ports simultaneously, we use below configuration
       RGMII Ports - 1,3,4,8. QSGMII ports - 2,5,6,7 */
    ENET_MAC_PORT_1, /* RGMII */
    ENET_MAC_PORT_3, /* RGMII */
    ENET_MAC_PORT_4, /* RGMII */
    ENET_MAC_PORT_8, /* RGMII */
#if defined(ENABLE_QSGMII_PORTS)
    ENET_MAC_PORT_2, /* QSGMII main */
    ENET_MAC_PORT_5, /* QSGMII sub */
    ENET_MAC_PORT_6, /* QSGMII sub */
    ENET_MAC_PORT_7, /* QSGMII sub */
#endif
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
    ENET_MAC_PORT_1, /* QSGMII main */
    ENET_MAC_PORT_3, /* QSGMII sub */
    ENET_MAC_PORT_4, /* QSGMII sub */
    ENET_MAC_PORT_5, /* QSGMII sub */
#elif defined(SOC_J722S)
    ENET_MAC_PORT_1,
    ENET_MAC_PORT_2,
#endif
};

#if defined(ETHFW_GPTP_SUPPORT)
static Enet_MacPort gEthAppSwitchPorts[]=
{
#if defined(SOC_J721E)
    ENET_MAC_PORT_3,
    ENET_MAC_PORT_8,
#if defined(ENABLE_QSGMII_PORTS)
    ENET_MAC_PORT_2,
    ENET_MAC_PORT_5,
#endif
#endif

#if defined(SOC_J784S4) || defined(SOC_J742S2)
    ENET_MAC_PORT_3,
    ENET_MAC_PORT_5,
#endif

#if defined(SOC_J722S)
    ENET_MAC_PORT_1,
#if !defined(ENABLE_MAC_ONLY_PORTS)
    ENET_MAC_PORT_2,
#endif
#endif
};
#endif

/* Custom policers which clients need to provide to add their own policers.
 * Make sure that size of this array is <= ETHFW_UTILS_NUM_CUSTOM_POLICERS */
#if defined(PDK)
static CpswAle_SetPolicerEntryInPartitionInArgs gEthApp_customPolicers[ETHFW_UTILS_NUM_CUSTOM_POLICERS] = {};
#endif

/* NOTE: 2 virtual ports should not have same tx channel allocated to them */
static EthFwVirtPort_VirtPortCfg gEthApp_virtPortCfg[] =
{
    {
        .remoteCoreId  = IPC_MPU1_0,
        .portId        = ETHREMOTECFG_SWITCH_PORT_0,
#if defined(ETHFW_EST_DEMO_SUPPORT) || defined(ETHFW_DEMO_SUPPORT) || defined (MCU_PLUS_SDK)
        .numTxCh       = 1U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_4
                         },
#else
         .numTxCh      = 2U,
         .txCh         = {
                            [0] = ENET_RM_TX_CH_4,
                            [1] = ENET_RM_TX_CH_7
                         },
#endif
        /* Number of rx flow for this virtual port */
        .numRxFlow     = 1U,
        /* To create custom policers on rx flows clients need to give flow information (i.e. numCustomPolicers and customPolicersInArgs)
         * for each allocated flow.
         * Map the customPolicersInArgs with global custom policer's (i.e. gEthApp_customPolicers) array.
         * For example if numRxFlow is 1 and we want to create 1 custom policer to match with 2'nd custom policer in global array do this:
         * .rxFlowsInfo = {
         *                  [0] = {
         *                           .numCustomPolicers    = 1U,
         *                           .customPolicersInArgs = {
         *                                                       [0] = &gEthApp_customPolicers[2U],
         *                                                   }
         *                         }
         *               },
         * It is important to note that number of custom policers per rx flow is <= ETHREMOTECFG_POLICER_PERFLOW */
        .numMacAddress = 1U,
        .clientIdMask  = ETHFW_BIT(ETHREMOTECFG_CLIENTID_LINUX) | ETHFW_BIT(ETHREMOTECFG_CLIENTID_QNX),
    },
#if defined (PDK)
    {
        .remoteCoreId  = IPC_MCU1_0,
        .portId        = ETHREMOTECFG_SWITCH_PORT_2,
        .numTxCh       = 1U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_5,
                         },
        .numRxFlow     = 1U,
        .numMacAddress = 1U,
        .clientIdMask  = ETHFW_BIT(ETHREMOTECFG_CLIENTID_AUTOSAR),
    },
#endif
    {
        /* Virtual switch port for Ethfw, using ETHREMOTECFG_SWITCH_PORT_LAST */
        .remoteCoreId  = IPC_MCU2_0,
        .portId        = ETHREMOTECFG_SWITCH_PORT_LAST,
#if defined(ETHFW_EST_DEMO_SUPPORT) || defined(ETHFW_DEMO_SUPPORT)
        .numTxCh       = 3U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_0,
                            [1] = ENET_RM_TX_CH_7,
                            [2] = ENET_RM_TX_CH_6
                         },
#else
        .numTxCh       = 2U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_0,
                            [1] = ENET_RM_TX_CH_6
                         },
#endif
        .numRxFlow     = 5U,
        .numMacAddress = 1U,
        .clientIdMask  = ETHFW_BIT(ETHREMOTECFG_CLIENTID_NONE),
    },
    {
        /* SWITCH_PORT_1 is used for both RTOS and Autosar client */
        .remoteCoreId  = IPC_MCU2_1,
        .portId        = ETHREMOTECFG_SWITCH_PORT_1,
        .numTxCh       = 1U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_1
                         },
        .numRxFlow     = 1U,
        .numMacAddress = 1U,
        .clientIdMask  = ETHFW_BIT(ETHREMOTECFG_CLIENTID_AUTOSAR) | ETHFW_BIT(ETHREMOTECFG_CLIENTID_RTOS),
    },
#if defined(ENABLE_MAC_ONLY_PORTS)
#if defined(PDK)
    {
        .remoteCoreId  = IPC_MPU1_0,
        .portId        = ETHREMOTECFG_MAC_PORT_1,
        .numTxCh       = 1U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_3
                         },
        .numRxFlow     = 1U,
        .numMacAddress = 1U,
        .clientIdMask  = ETHFW_BIT(ETHREMOTECFG_CLIENTID_LINUX) | ETHFW_BIT(ETHREMOTECFG_CLIENTID_QNX),
    },
#endif
    {
        .remoteCoreId  = IPC_MCU2_1,
#if defined (MCU_PLUS_SDK)
        .portId        = ETHREMOTECFG_MAC_PORT_2,
#elif defined(PDK)
        .portId        = ETHREMOTECFG_MAC_PORT_4,
#endif
        .numTxCh       = 1U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_2
                         },
        .numRxFlow     = 1U,
        .numMacAddress = 1U,
        .clientIdMask  = ETHFW_BIT(ETHREMOTECFG_CLIENTID_RTOS),
    },
#endif
};

#if defined(SAFERTOS)
static sys_sem_t gEthApp_lwipMainTaskSemObj;
#endif

static app_rtos_task_handle_t gLwipTaskHandle;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void EthApp_lwipMain(void *a0,
                            void *a1);

static void EthApp_initLwip(void *arg);

static void EthApp_initNetif(void);

static void EthApp_netifStatusCb(struct netif *netif);

static int32_t EthApp_initEthFw(void);


static void EthApp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                          bool isLinkUp,
                                          void *appArg);

#if defined (MCU_PLUS_SDK)
static void EthApp_netifLinkChangeCb(struct netif *pNetif);
#endif

#if defined(ETHFW_MONITOR_SUPPORT)
static void EthApp_closeDmaCb(void *arg);

static void EthApp_openDmaCb(void *arg);
#endif

#if defined(ETHFW_DEMO_SUPPORT)
static void EthApp_startSwInterVlan(char *recvBuff,
                                    char *sendBuff);

static void EthApp_startHwInterVlan(char *recvBuff,
                                    char *sendBuff);
#endif

#if defined(ETHFW_GPTP_SUPPORT)
static void EthApp_configPtpCb(void *arg);

static void EthApp_initPtp(void);
#endif

#if defined(ETHAPP_ENABLE_INTERCORE_ETH) || defined(ETHFW_VEPA_SUPPORT)
static void EthApp_filterAddMacSharedCb(const uint8_t *mac_address,
                                        uint8_t hostId);

static void EthApp_filterDelMacSharedCb(const uint8_t *mac_address,
                                        uint8_t hostId);

/* Shared multicast address table */
typedef struct
{
    /*! Shared Mcast address */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];
    /*! lwIP Bridge port mask */
    bridgeif_portmask_t portMask;
} EthApp_SharedMcastAddrTable;

/* Must not exceed ETHAPP_MAX_SHARED_MCAST_ADDR entries */
static EthFwMcast_McastCfg gEthApp_sharedMcastCfgTable[] =
{
    {
        /* MCast IP ADDR: 224.0.0.1 */
        .macAddr      = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x01},
        .portMask     = ETHAPP_DFLT_PORT_MASK,
        .virtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK,
    },
    {
        /* MCast IP ADDR: 224.0.0.251 */
        .macAddr      = {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB},
        .portMask     = ETHAPP_DFLT_PORT_MASK,
        .virtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK,
    },
    {
        /* MCast IP ADDR: 224.0.0.252 */
        .macAddr      = {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFC},
        .portMask     = ETHAPP_DFLT_PORT_MASK,
        .virtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK,
    },
    {
        .macAddr      = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01},
        .portMask     = ETHAPP_DFLT_PORT_MASK,
        .virtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK,
    },
    {
        .macAddr      = {0x33, 0x33, 0xFF, 0x1D, 0x92, 0xC2},
        .portMask     = ETHAPP_DFLT_PORT_MASK,
        .virtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK,
    },
    {
        .macAddr      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x00},
        .portMask     = ETHAPP_DFLT_PORT_MASK,
        .virtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK,
    },
    {
        .macAddr      = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x03},
        .portMask     = ETHAPP_DFLT_PORT_MASK,
        .virtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK,
    },
};
#endif

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
/* Array to store coreId to lwip bridge portId map */
static uint8_t gEthApp_lwipBridgePortIdMap[IPC_MAX_PROCS];

static bridgeif_portmask_t gEthApp_bridgePortMask[ARRAY_SIZE(gEthApp_sharedMcastCfgTable)];
#endif

/* List of multicast addresses reserved for EthFw. Currently, this list is populated
 * only with PTP related multicast addresses which are used by the test PTP stack
 * used by EthFw.
 * Note: Must not exceed ETHFW_RSVD_MCAST_LIST_LEN */
static EthFwMcast_RsvdMcast gEthApp_rsvdMcastCfgTable[] =
{
    /* PTP - Peer delay messages */
    {
        .macAddr  = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x0E},
        .vlanId   = 0U,
        .portMask = ETHAPP_DFLT_PORT_MASK,
    },
    /* PTP - Non peer delay messages */
    {
        .macAddr  = {0x01, 0x1b, 0x19, 0x00, 0x00, 0x00},
        .vlanId   = 0U,
        .portMask = ETHAPP_DFLT_PORT_MASK,
    },
};

static EthFwPortMirroring_Cfg gEthApp_portMirCfg = 
{
    .mirroringType = DISABLE_PORT_MIRRORING
};

#if defined(ETHFW_IET_ENABLE)
static EthFwIET_Config gEthApp_IETCfg = {

/* If enabled does IET verfication before enabling iet*/
    .mac_verify_enable = BTRUE, 
/* 0 -> Express Traffic and 1 -> Premptable Traffic*/
    .queueMode = 
    {
    EXPRESS_TRAFFIC,
    PREMPTIVE_TRAFFIC,
    EXPRESS_TRAFFIC,
    PREMPTIVE_TRAFFIC,
    EXPRESS_TRAFFIC,
    PREMPTIVE_TRAFFIC,
    EXPRESS_TRAFFIC,
    PREMPTIVE_TRAFFIC
    },
/* Set minimum fragment size */
 .minFragSize =  MIN_FRAG_SIZE,
};
#endif

static struct netif netif;
#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
static struct netif netif_ic[ETHAPP_NETIF_IC_MAX_IDX];

static uint32_t netif_ic_state[IC_ETH_MAX_VIRTUAL_IF] =
{
    IC_ETH_IF_MCU2_0_MCU2_1,
    IC_ETH_IF_MCU2_1_MCU2_0,
    IC_ETH_IF_MCU2_0_A72
};

static struct netif netif_bridge;
bridgeif_initdata_t bridge_initdata;
#endif /* ETHAPP_ENABLE_INTERCORE_ETH */

#if defined(ETHFW_VEPA_SUPPORT)
/* Private VLAN ids used in broadcast/multicast packets sent from ETHFW
 * to remote clients using multihost flow */
static uint32_t gEthApp_remoteClientPrivVlanIdMap[ETHREMOTECFG_SWITCH_PORT_LAST+1] = 
{
    [ETHREMOTECFG_SWITCH_PORT_0] = 1100U, /* Linux client */
    [ETHREMOTECFG_SWITCH_PORT_1] = 1200U, /* AUTOSAR or RTOS client */
    [ETHREMOTECFG_SWITCH_PORT_2] = 1300U, /* AUTOSAR client */
};
#endif

/* Trace configuration */
static EthFwTrace_Cfg gEthApp_traceCfg =
{
    .print        = appLogPrintf,
    .traceTsFunc  = NULL,
    .extTraceFunc = NULL,
};

/* Test VLAN config */
static EthFwVlan_VlanCfg gEthApp_vlanCfg[] =
{
    {
        .vlanId              = 1024U,
        .memberMask          = ETHAPP_DFLT_PORT_MASK,
        .regMcastFloodMask   = ETHAPP_DFLT_PORT_MASK,
        .unregMcastFloodMask = ETHAPP_DFLT_PORT_MASK,
        .virtMemberMask      = ETHAPP_DFLT_VIRT_PORT_MASK,
        .untagMask           = 0U,
    },
};

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void EthApp_ipcPrint(const char *str)
{
    appLogPrintf("%s", str);
    return;
}
/**
 * Following block of code is required for SITARA devices
 * EnetApp_updateCpswInitCfg is called from EthApp_driverOpen from ti_enet_open_close.c
 * On Ethfw server application corresponding functions are present in 
 * ${MCU_PLUS_SDK_PATH}/source/networking/ethfw/apps/app_remoteswitchcfg_server/sitara/app_cpswconfighandler.c
 */
#if defined(MCU_PLUS_SDK)
static void EnetApp_mdioLinkStatusChange(Cpsw_MdioLinkStateChangeInfo *info,
                                         void *appArg)
{
    if (info->linkChanged)
    {
        EnetAppUtils_print("Link Status Changed. PHY: 0x%x, state: %s\r\n",
                info->phyAddr,
                info->isLinked? "up" : "down");
    }
}

static void EnetApp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                           bool isLinkUp,
                                           void *appArg)
{
    EnetAppUtils_print("MAC Port %u: link %s\r\n",
                       ENET_MACPORT_ID(macPort), isLinkUp ? "up" : "down");
#if defined(ETHFW_GPTP_SUPPORT)
    cb_lld_notify_linkchange();
#endif
}

/* Called in generated/ti_enet_open_close. */
void EnetApp_updateCpswInitCfg(Enet_Type enetType, uint32_t instId, Cpsw_Cfg *cpswCfg)
{
#if defined (ENET_SOC_HOSTPORT_DMA_TYPE_CPDMA)
    EnetDma_Cfg * dmaCfg = (EnetDma_Cfg *)cpswCfg->dmaCfg;

    EnetAppUtils_assert(dmaCfg != NULL);
    EnetAppUtils_assert(EnetAppUtils_isDescCached() == false);
    dmaCfg->rxInterruptPerMSec = 8;
    dmaCfg->txInterruptPerMSec = 2;
#endif


#if (ENET_SYSCFG_ENABLE_MDIO_MANUALMODE == 1U)
    cpswCfg->mdioLinkStateChangeCb    = NULL;
    cpswCfg->mdioLinkStateChangeCbArg = NULL;
#else
    cpswCfg->mdioLinkStateChangeCb    = &EnetApp_mdioLinkStatusChange;
    cpswCfg->mdioLinkStateChangeCbArg = NULL;
#endif

#if defined(ETHFW_BOOT_TIME_PROFILING)
    /* Link-up timestamp */
    cpswCfg->portLinkStatusChangeCb    = &EthApp_portLinkStatusChangeCb;
    cpswCfg->portLinkStatusChangeCbArg = NULL;
#else
    cpswCfg->portLinkStatusChangeCb = &EnetApp_portLinkStatusChangeCb;
    cpswCfg->portLinkStatusChangeCbArg = NULL;
#endif
}
#endif


void appEthFwEarlyInit()
{
    app_rtos_semaphore_params_t semParams;

    /* Create semaphore used to synchronize EthFw and NDK init.
     * EthFw opens the CPSW driver which is required by NDK during NIMU
     * initialization, hence EthFw init must complete first.
     * Currently, there is no control over NDK initialization time and its
     * task runs right away after BIOS_start() hence causing a race
     * condition with EthFw init */
    appRtosSemaphoreParamsInit(&semParams);

    semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
    semParams.initValue = 0U;

    gEthAppObj.hInitSem = appRtosSemaphoreCreate(semParams);

    EthFwIpc_resetObj();
}

int32_t appEthFwInit()
{
    int32_t status = ETHAPP_OK;
    EnetUtils_Cfg *utilsPrms = NULL;
    uint32_t flags = 0U;

    appLogPrintf("ETHFW: Init ... !!!\n");

    gEthAppObj.coreId = EnetSoc_getCoreId();

    Enet_init(utilsPrms);
    EthFwOsal_init();

#if defined(ETHFW_GPTP_SUPPORT)
    app_rtos_semaphore_params_t  semParams;
#endif

    /* Initialize EthFw trace utils */
    if (status == ENET_SOK)
    {
        EthFwTrace_init(&gEthApp_traceCfg);
        EthFwTrace_setLevel(ETHFW_TRACE_INFO);
    }

    /* Board related initialization */
#if defined(SOC_J721E)
    flags |= ETHFW_BOARD_GESI_ENABLE;
#if defined(ENABLE_QSGMII_PORTS)
    flags |= ETHFW_BOARD_QENET_ENABLE;
#endif
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
    flags |= (ETHFW_BOARD_QENET_ENABLE | ETHFW_BOARD_SERDES_CONFIG);
#elif defined(SOC_J722S)
flags |= (ETHFW_BOARD_SERDES_CONFIG | ETHFW_BOARD_UART_ALLOWED | ETHFW_BOARD_ENET_BRIDGE_ENABLE) ;
#endif
    /* Board related initialization */
    status = EthFwBoard_init(flags);
    if (status != ENET_SOK)
    {
        appLogPrintf("ETHFW: Board initialization failed\n");
    }

#if defined(ETHFW_GPTP_SUPPORT)
    /* Create semaphore used to synchronize MAC alloc by lwIP which is required and
     * shared with the gPTP stack */
    if (status == ETHAPP_OK)
    {
        appRtosSemaphoreParamsInit(&semParams);
        semParams.mode = APP_RTOS_SEMAPHORE_MODE_BINARY;
        semParams.initValue  = 0;

        gEthAppObj.hHostMacAllocSem = appRtosSemaphoreCreate(semParams);

        if (gEthAppObj.hHostMacAllocSem == NULL)
        {
            appLogPrintf("ETHFW: failed to create hostport MAC addr semaphore\n");
            status = ETHAPP_ERROR;
        }
    }
#endif
    /* Initialize Ethernet Firmware */
    if (status == ETHAPP_OK)
    {
        status = EthApp_initEthFw();
    }

    /* Initialize lwIP */
    if (status == ENET_SOK)
    {
        app_rtos_task_params_t taskParams;

        appRtosTaskParamsInit(&taskParams);
        taskParams.priority  = DEFAULT_THREAD_PRIO;
        taskParams.stack     = &gEthAppLwipStackBuf[0];
        taskParams.stacksize = sizeof(gEthAppLwipStackBuf);
        taskParams.name      = "lwIP main loop";
        taskParams.taskfxn   = &EthApp_lwipMain;
#if defined(SAFERTOS)
        taskParams.userData  = &gEthApp_lwipMainTaskSemObj;
#endif

        gLwipTaskHandle = appRtosTaskCreate(&taskParams);
    }

    #if defined(SOC_J722S)
    
    uint32_t EthAppRemoteProc[] = { IPC_MCU2_1,};
    uint32_t selfProcId = EnetSoc_getCoreId();
    uint32_t numProc = ARRAY_SIZE(EthAppRemoteProc);
    status = EthFwIpc_init(selfProcId,
                           numProc,
                           &EthAppRemoteProc[0],
                           &EthApp_ipcPrint);
    #endif

    if (status == ETHAPP_OK)
    {
        appLogPrintf("ETHFW: Init ... DONE !!!\n");
    }
    else
    {
        appLogPrintf("ETHFW: Init ... ERROR !!!\n");
    }
#if defined(ETHFW_GPTP_SUPPORT)
    /* Initialize gPTP stack */
    EthApp_initPtp();
#endif
    return status;
}

int32_t appEthFwDeInit()
{
    int32_t status = 0;

    EthFw_deinit(gEthAppObj.hEthFw);

    return status;
}

int32_t appEthFwRemoteServerInit()
{
    int32_t status;

    appLogPrintf("ETHFW: Remove server Init ... !!!\n");

    /* Initialize the Remote Config server (CPSW Proxy Server) */
    status = EthFw_initRemoteConfig(gEthAppObj.hEthFw);
    if (status != ENET_SOK)
    {
        appLogPrintf("ETHFW: Remove server Init ... ERROR (%d) !!! \n", status);
    }
    else
    {
        appLogPrintf("ETHFW: Remove server Init ... DONE !!!\n");
    }

    return status;
}

#if defined(MCU_PLUS_SDK)
int32_t appEthFwLateAnnounce()
{
    int32_t status = EthFw_lateAnnounce(gEthAppObj.hEthFw, IPC_MPU1_0);
    if (status != ETHFW_SOK)
    {
        appLogPrintf("ETHFW: late announcement failed: %d\n", status);
    }
    else
    {
        appLogPrintf("ETHFW: late announcement successful\n");
    }
    return status;
}
#endif

void LwipifEnetAppCb_getHandle(LwipifEnetAppIf_GetHandleInArgs *inArgs,
                               LwipifEnetAppIf_GetHandleOutArgs *outArgs)
{
    /* Wait for EthFw to be initialized */
    appRtosSemaphorePend(gEthAppObj.hInitSem, APP_RTOS_SEMAPHORE_WAIT_FOREVER);

    EthFwCallbacks_lwipifCpswGetHandle(gEthAppObj.enetType, gEthAppObj.instId, inArgs, outArgs);

#if defined(PDK)
    /* Save host port MAC address */
    EnetUtils_copyMacAddr(&gEthAppObj.hostMacAddr[0U],
                          &outArgs->rxInfo[0U].macAddr[0U]);
#endif
#if defined(ETHFW_GPTP_SUPPORT)
    appRtosSemaphorePost(gEthAppObj.hHostMacAllocSem);
#endif
}

void LwipifEnetAppCb_releaseHandle(LwipifEnetAppIf_ReleaseHandleInfo *releaseInfo)
{
    EthFwCallbacks_lwipifCpswReleaseHandle(gEthAppObj.enetType, gEthAppObj.instId, releaseInfo);
}

void EthApp_portLinkStatusChangeCb(Enet_MacPort macPort,
                                          bool isLinkUp,
                                          void *appArg)
{
#if defined(ETHFW_GPTP_SUPPORT)
    cb_lld_notify_linkchange();
#endif
#if defined(ETHFW_IET_ENABLE)
    EthFwIET_notifyLinkChange(macPort,isLinkUp);
#endif
}

#if defined(MCU_PLUS_SDK)
void EthApp_getHostMacAddr(struct netif *netif)
{
    /* This function is used for sitara where there is no callback to main to fetch host mac addr */
    EnetUtils_copyMacAddr(&gEthAppObj.hostMacAddr[0U], &netif->hwaddr[0U]);

#if defined(ETHFW_GPTP_SUPPORT)
    appRtosSemaphorePost(gEthAppObj.hHostMacAllocSem);
#endif
}
#endif

static int32_t EthApp_initEthFw(void)
{
    EthFw_Version ver;
    EthFw_Config ethFwCfg;
    Cpsw_Cfg *cpswCfg = &ethFwCfg.cpswCfg;
    EnetDma_Cfg dmaCfg;
    EnetRm_MacAddressPool *pool = &cpswCfg->resCfg.macList;
    #if defined(ETHAPP_ENABLE_INTERCORE_ETH) || defined(ETHFW_VEPA_SUPPORT)
    EthFwMcast_SharedMcastCfg *sharedMcastCfg = &ethFwCfg.mcastCfg.sharedMcastCfg;
    #endif
    EthFwMcast_RsvdMcastCfg *rsvdMcastCfg = &ethFwCfg.mcastCfg.rsvdMcastCfg;
    EthFwTsn_PpsConfig ppsConfig;
    uint32_t poolSize;
    int32_t status = ETHAPP_OK;
    int32_t i;

    /* Set EthFw config params */
    EthFw_initConfigParams(gEthAppObj.enetType, gEthAppObj.instId, &ethFwCfg);

#if defined(PDK)
    dmaCfg.rxChInitPrms.dmaPriority = UDMA_DEFAULT_RX_CH_DMA_PRIORITY;
#endif    
    dmaCfg.hUdmaDrv = appUdmaGetObj();
    if (dmaCfg.hUdmaDrv == NULL)
    {
        appLogPrintf("ETHFW: ERROR: failed to open UDMA driver\n");
        status = -1;
    }
    cpswCfg->dmaCfg = &dmaCfg;

    /* Populate MAC address pool */
    poolSize = EnetUtils_min(ENET_ARRAYSIZE(pool->macAddress), ETHAPP_MAC_ADDR_POOL_SIZE);
    pool->numMacAddress = EthFwBoard_getMacAddrPool(pool->macAddress, poolSize);

    /* Set hardware port configuration parameters */
    ethFwCfg.ports = &gEthAppPorts[0];
    ethFwCfg.numPorts = ARRAY_SIZE(gEthAppPorts);
    ethFwCfg.setPortCfg = EthFwBoard_setPortCfg;

    /* Set virtual port configuration parameters */
    ethFwCfg.virtPortCfg  = &gEthApp_virtPortCfg[0];
    ethFwCfg.numVirtPorts = ARRAY_SIZE(gEthApp_virtPortCfg);
    ethFwCfg.isStaticTxChanAllocated = true;

    /* Set static VLAN configuration parameters */
    ethFwCfg.vlanCfg             = &gEthApp_vlanCfg[0];
    ethFwCfg.numStaticVlans      = ARRAY_SIZE(gEthApp_vlanCfg);
    ethFwCfg.defaultPortMask     = ETHAPP_DFLT_PORT_MASK;
    ethFwCfg.defaultVirtPortMask = ETHAPP_DFLT_VIRT_PORT_MASK;
    ethFwCfg.dVlanSwtFwdEn       = BTRUE;

    /* CPTS_RFT_CLK is sourced from MAIN_SYSCLK0 (500MHz) */
    cpswCfg->cptsCfg.cptsRftClkFreq = CPSW_CPTS_RFTCLK_FREQ_500MHZ;
    /* Enable clear GenF */
    cpswCfg->cptsCfg.tsGenfClrEn = BTRUE;

#if defined(ETHFW_MONITOR_SUPPORT)
    /* Save the Lwip Dma parametrers */
    ethFwCfg.monitorCfg.lwipDmaCbArg   = (void *)&netif;
    ethFwCfg.monitorCfg.closeLwipDmaCb = EthApp_closeDmaCb;
    ethFwCfg.monitorCfg.openLwipDmaCb  = EthApp_openDmaCb;
#endif

#if defined(ETHFW_GPTP_SUPPORT)
#if defined(MCU_PLUS_SDK)
    gEthAppObj.enablePPS = BFALSE;
#else
    gEthAppObj.enablePPS = BTRUE;
#endif
    if (gEthAppObj.enablePPS)
    {
#if defined(SOC_J721E) || defined (SOC_J7200)
        ethFwCfg.ppsConfig.tsrIn = CSLR_TIMESYNC_INTRTR0_IN_CPSW0_CPTS_GENF0_0;
        ethFwCfg.ppsConfig.tsrOut = ETHAPP_PPS_TIMESYNC_INTR_SYNC2_OUT_PIN;
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
        ethFwCfg.ppsConfig.tsrIn = CSLR_TIMESYNC_INTRTR0_IN_CPSW_9XUSSM0_CPTS_GENF0_0;
        ethFwCfg.ppsConfig.tsrOut = ETHAPP_PPS_TIMESYNC_INTR_SYNC3_OUT_PIN;
#elif defined(SOC_J722S)
        /*GENF is not enabled for Sitara and J722S Devices*/
#else
#error "TSN: Unsupported SoC"
#endif

        ethFwCfg.ppsConfig.ppsFreqHz = ETHAPP_PPS_TIMESYNC_REF_CLK_HZ;
        ethFwCfg.ppsConfig.genfIdx = ETHAPP_PPS_TIMESYNC_GENF_INST_IDX;
    }
    /* gPTP stack config parameters */
    ethFwCfg.configPtpCb    = EthApp_configPtpCb;
    ethFwCfg.configPtpCbArg = NULL;
#endif

#if defined(ETHFW_DEMO_SUPPORT)
    /* Overwrite config params with those for hardware interVLAN */
    EthHwInterVlan_setOpenPrms(&ethFwCfg.cpswCfg);
#endif

#if defined(ETHFW_BOOT_TIME_PROFILING)|| defined(ETHFW_GPTP_SUPPORT) || defined(ETHFW_IET_ENABLE)
    /* Link-up timestamp */
    cpswCfg->portLinkStatusChangeCb    = &EthApp_portLinkStatusChangeCb;
    cpswCfg->portLinkStatusChangeCbArg = &gEthAppObj;
#endif

#if defined(ETHAPP_ENABLE_INTERCORE_ETH) || defined(ETHFW_VEPA_SUPPORT)
    if (ARRAY_SIZE(gEthApp_sharedMcastCfgTable) > ETHAPP_MAX_SHARED_MCAST_ADDR)
    {
        appLogPrintf("ETHFW error: No. of shared mcast addr cannot exceed %d\n",
                    ETHAPP_MAX_SHARED_MCAST_ADDR);
        status = ETHAPP_ERROR;
    }
    else
    {
        sharedMcastCfg->mcastCfg = &gEthApp_sharedMcastCfgTable[0U];
        sharedMcastCfg->numMcast = ARRAY_SIZE(gEthApp_sharedMcastCfgTable);
        sharedMcastCfg->filterAddMacSharedCb = EthApp_filterAddMacSharedCb;
        sharedMcastCfg->filterDelMacSharedCb = EthApp_filterDelMacSharedCb;
    }
#endif

    if (status == ETHAPP_OK)
    {
        if (ARRAY_SIZE(gEthApp_rsvdMcastCfgTable) > ETHFW_RSVD_MCAST_LIST_LEN)
        {
            appLogPrintf("ETHFW error: No. of rsvd mcast addr cannot exceed %d\n",
                         ETHFW_RSVD_MCAST_LIST_LEN);
            status = ETHAPP_ERROR;
        }
        else
        {
            rsvdMcastCfg->mcastCfg = &gEthApp_rsvdMcastCfgTable[0U];
            rsvdMcastCfg->numMcast = ARRAY_SIZE(gEthApp_rsvdMcastCfgTable);
        }
    }

#if defined(ETHFW_VEPA_SUPPORT)
    memcpy(ethFwCfg.vepaCfg.privVlanId,
           gEthApp_remoteClientPrivVlanIdMap,
           sizeof(gEthApp_remoteClientPrivVlanIdMap));
#endif

#if defined(ETHFW_IET_ENABLE)
    ethFwCfg.ietCfg = gEthApp_IETCfg;
#endif

    ethFwCfg.portMirCfg = &gEthApp_portMirCfg;

    /* Initialize the EthFw */
    if (status == ETHAPP_OK)
    {
        gEthAppObj.hEthFw = EthFw_init(gEthAppObj.enetType, gEthAppObj.instId, &ethFwCfg);
        if (gEthAppObj.hEthFw == NULL)
        {
            appLogPrintf("ETHFW: failed to initialize the firmware\n");
            status = ETHAPP_ERROR;
        }
    }

    /* Get and print EthFw version */
    if (status == ETHAPP_OK)
    {
        EthFw_getVersion(gEthAppObj.hEthFw, &ver);
        appLogPrintf("\nETHFW Version   : %d.%02d.%02d\r\n", ver.major, ver.minor, ver.rev);
        appLogPrintf("ETHFW Build Date: %s %s, %s\r\n", ver.month, ver.date, ver.year);
        appLogPrintf("ETHFW Build Time: %s:%s:%s\r\n", ver.hour, ver.min, ver.sec);
        appLogPrintf("ETHFW Commit SHA: %s\r\n\n", ver.commitHash);
    }

    /* Post semaphore so that lwip or NIMU/NDK can continue with their initialization */
    if (status == ETHAPP_OK)
    {
        appRtosSemaphorePost(gEthAppObj.hInitSem);
    }

    return status;
}

/* lwIP callbacks (exact name required) */

bool EthFwCallbacks_isPortLinked(struct netif *netif,
                                 void *handleArg)
{
    bool linked = false;
    Enet_Handle hEnet = (Enet_Handle)handleArg;
    uint32_t i;

    /* Report port linked as long as any port owned by EthFw is up */
    for (i = 0U; (i < ARRAY_SIZE(gEthAppPorts)) && !linked; i++)
    {
        linked = EnetAppUtils_isPortLinkUp(hEnet,
                                           gEthAppObj.coreId,
                                           gEthAppPorts[i]);
    }

    return linked;
}

static void EthApp_lwipMain(void *a0,
                            void *a1)
{
    err_t err;
    sys_sem_t initSem;

    appUtilsTaskInit();

    /* initialize lwIP stack and network interfaces */
    err = sys_sem_new(&initSem, 0);
    LWIP_ASSERT("failed to create initSem", err == ERR_OK);
    LWIP_UNUSED_ARG(err);

    tcpip_init(EthApp_initLwip, &initSem);

    /* we have to wait for initialization to finish before
     * calling update_adapter()! */
    sys_sem_wait(&initSem);
    sys_sem_free(&initSem);

#if (LWIP_SOCKET || LWIP_NETCONN) && LWIP_NETCONN_SEM_PER_THREAD
    netconn_thread_init();
#endif

    appRtosTaskDelete(&gLwipTaskHandle);
}

static void EthApp_initLwip(void *arg)
{
    sys_sem_t *initSem;

    LWIP_ASSERT("arg != NULL", arg != NULL);
    initSem = (sys_sem_t*)arg;

    /* init randomizer again (seed per thread) */
    srand((unsigned int)sys_now()/1000);

    /* init network interfaces */
    EthApp_initNetif();

#if defined(ETHAPP_ENABLE_IPERF_SERVER)
    /* Enable iperf */
    lwiperf_example_init();
#endif

    sys_sem_signal(initSem);
}

static void EthApp_initNetif(void)
{
    ip4_addr_t ipaddr, netmask, gw;
    struct netif *pNetif = &netif;
#if LWIP_CHECKSUM_CTRL_PER_NETIF
    uint32_t chksumFlags = NETIF_CHECKSUM_ENABLE_ALL;
/* Disable checksum in software if CPSW can do it (i.e. not impacted by errata) */
#if !defined(ETHFW_CPSW_MULTIHOST_CHECKSUM_ERRATA)
    chksumFlags &= ~(NETIF_CHECKSUM_GEN_UDP |
                    NETIF_CHECKSUM_GEN_TCP |
                    NETIF_CHECKSUM_CHECK_TCP |
                    NETIF_CHECKSUM_CHECK_UDP);
#endif
#endif
#if ETHAPP_LWIP_USE_DHCP
    err_t err;
#endif
#if defined(MCU_PLUS_SDK)
    hLwipIfApp = LwipifEnetApp_getHandle();
    pNetif = LwipifEnetApp_getNetifFromId(hLwipIfApp, 0U);
    EnetAppUtils_assert(pNetif != NULL);
#endif

    ip4_addr_set_zero(&gw);
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);

#if ETHAPP_LWIP_USE_DHCP
    appLogPrintf("Starting lwIP, local interface IP is dhcp-enabled\r\n");
#else /* ETHAPP_LWIP_USE_DHCP */
    ETHFW_SERVER_GW(&gw);
    ETHFW_SERVER_IPADDR(&ipaddr);
    ETHFW_SERVER_NETMASK(&netmask);
    appLogPrintf("Starting lwIP, local interface IP is %s\r\n", ip4addr_ntoa(&ipaddr));
#endif /* ETHAPP_LWIP_USE_DHCP */

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
    /* Create Enet LLD ethernet interface */
    netif_add(pNetif, NULL, NULL, NULL, NULL, LWIPIF_LWIP_init, tcpip_input);
#if defined(MCU_PLUS_SDK)
    LWIPIF_LWIP_start(gEthAppObj.enetType, gEthAppObj.instId, pNetif, ETHAPP_ETHNETIF_0);
    netif_set_link_callback(pNetif, EthApp_netifLinkChangeCb);
    EthApp_getHostMacAddr(pNetif);
#endif
#if LWIP_CHECKSUM_CTRL_PER_NETIF
    NETIF_SET_CHECKSUM_CTRL(pNetif, chksumFlags);
#endif

    /* Create inter-core virtual ethernet interface: MCU2_0 <-> MCU2_1 */
    netif_add(&netif_ic[ETHAPP_NETIF_IC_MCU2_0_MCU2_1_IDX], NULL, NULL, NULL,
              (void*)&netif_ic_state[IC_ETH_IF_MCU2_0_MCU2_1],
              LWIPIF_LWIP_IC_init, tcpip_input);

    /* Create inter-core virtual ethernet interface: MCU2_0 <-> A72 */
    netif_add(&netif_ic[ETHAPP_NETIF_IC_MCU2_0_A72_IDX], NULL, NULL, NULL,
              (void*)&netif_ic_state[IC_ETH_IF_MCU2_0_A72],
              LWIPIF_LWIP_IC_init, tcpip_input);

    /* Create bridge interface */
    bridge_initdata.max_ports = ETHAPP_LWIP_BRIDGE_MAX_PORTS;
    bridge_initdata.max_fdb_dynamic_entries = ETHAPP_LWIP_BRIDGE_MAX_DYNAMIC_ENTRIES;
    bridge_initdata.max_fdb_static_entries = ETHAPP_LWIP_BRIDGE_MAX_STATIC_ENTRIES;
    EnetUtils_copyMacAddr(&bridge_initdata.ethaddr.addr[0U], &gEthAppObj.hostMacAddr[0U]);

#if (defined MCU_PLUS_SDK)
    pNetif->flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP;
    netif_ic[ETHAPP_NETIF_IC_MCU2_0_A72_IDX].flags |= NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP;
#endif

    netif_add(&netif_bridge, &ipaddr, &netmask, &gw, &bridge_initdata, bridgeif_init, netif_input);

    /* Add all netifs to the bridge and create coreId to bridge portId map */
    bridgeif_add_port(&netif_bridge, pNetif);
    gEthApp_lwipBridgePortIdMap[IPC_MCU2_0] = ETHAPP_BRIDGEIF_CPU_PORT_ID;

    bridgeif_add_port(&netif_bridge, &netif_ic[ETHAPP_NETIF_IC_MCU2_0_MCU2_1_IDX]);
    gEthApp_lwipBridgePortIdMap[IPC_MCU2_1] = ETHAPP_BRIDGEIF_PORT1_ID;

    bridgeif_add_port(&netif_bridge, &netif_ic[ETHAPP_NETIF_IC_MCU2_0_A72_IDX]);
    gEthApp_lwipBridgePortIdMap[IPC_MPU1_0] = ETHAPP_BRIDGEIF_PORT2_ID;

    /* Set bridge interface as the default */
    netif_set_default(&netif_bridge);

#if LWIP_CHECKSUM_CTRL_PER_NETIF
    NETIF_SET_CHECKSUM_CTRL(&netif_bridge, chksumFlags);
    NETIF_SET_CHECKSUM_CTRL(&netif_ic[ETHAPP_NETIF_IC_MCU2_0_MCU2_1_IDX], chksumFlags);
    NETIF_SET_CHECKSUM_CTRL(&netif_ic[ETHAPP_NETIF_IC_MCU2_0_A72_IDX], chksumFlags);
#endif
#else
    netif_add(pNetif, &ipaddr, &netmask, &gw, NULL, LWIPIF_LWIP_init, tcpip_input);
#if defined(MCU_PLUS_SDK)
    LWIPIF_LWIP_start(gEthAppObj.enetType, gEthAppObj.instId, pNetif, ETHAPP_ETHNETIF_0);
    netif_set_link_callback(pNetif, EthApp_netifLinkChangeCb);
    EthApp_getHostMacAddr(pNetif);
#endif
    netif_set_default(pNetif);
#if LWIP_CHECKSUM_CTRL_PER_NETIF
    NETIF_SET_CHECKSUM_CTRL(pNetif, chksumFlags);
#endif
#endif
#if defined (MCU_PLUS_SDK)
    LwipifEnetApp_startSchedule(hLwipIfApp, pNetif);
#endif

    netif_set_status_callback(netif_default, EthApp_netifStatusCb);

    dhcp_set_struct(netif_default, &gEthAppObj.dhcpNetif);

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
    netif_set_up(pNetif);
    netif_set_up(&netif_ic[ETHAPP_NETIF_IC_MCU2_0_MCU2_1_IDX]);
    netif_set_up(&netif_ic[ETHAPP_NETIF_IC_MCU2_0_A72_IDX]);
    netif_set_up(&netif_bridge);
#else
    netif_set_up(netif_default);
#endif

#if ETHAPP_LWIP_USE_DHCP
    err = dhcp_start(netif_default);
    if (err != ERR_OK)
    {
        appLogPrintf("Failed to start DHCP: %d\n", err);
    }
#endif /* ETHAPP_LWIP_USE_DHCP */
}

static void EthApp_netifStatusCb(struct netif *netif)
{
    int32_t status;

    if (netif_is_up(netif))
    {
        const ip4_addr_t *ipAddr = netif_ip4_addr(netif);

        appLogPrintf("Added interface '%c%c%d', IP is %s\r\n",
                     netif->name[0], netif->name[1], netif->num, ip4addr_ntoa(ipAddr));

        if (ipAddr->addr != 0)
        {
#if defined(ETHFW_DEMO_SUPPORT)
            /* Assign functions that are to be called based on actions in GUI.
             * These cannot be dynamically pushed to function pointer array, as the
             * index is used in GUI as command */
            EnetCfgServer_fxn_table[9] = &EthApp_startSwInterVlan;
            EnetCfgServer_fxn_table[10] = &EthApp_startHwInterVlan;

            /* Start Configuration server */
            status = EnetCfgServer_init(gEthAppObj.enetType, gEthAppObj.instId);
            EnetAppUtils_assert(ENET_SOK == status);

            /* Start the software-based interVLAN routing */
            EthSwInterVlan_setupRouting(gEthAppObj.enetType, ETH_SWINTERVLAN_TASK_PRI);
#endif
        }
    }
    else
    {
        appLogPrintf("Removed interface '%c%c%d'\n", netif->name[0], netif->name[1], netif->num);
    }
}

#if defined (MCU_PLUS_SDK)
static void EthApp_netifLinkChangeCb(struct netif *pNetif)
{
    if (netif_is_link_up(pNetif))
    {
        appLogPrintf("[%d]Network Link UP Event\r\n", pNetif->num);
    }
    else
    {
        appLogPrintf("[%d]Network Link DOWN Event\r\n", pNetif->num);
    }
    return;
}
#endif

#if defined(ETHFW_MONITOR_SUPPORT)
static void EthApp_closeDmaCb(void *arg)
{
    struct netif *netif = (struct netif *)arg;

    /* Issue link down to Lwip stack and close the Lwip DMA channels */
    sys_lock_tcpip_core();
    netif_set_link_down(netif);
    sys_unlock_tcpip_core();

    LWIPIF_LWIP_closeDma(netif);
}

static void EthApp_openDmaCb(void *arg)
{
    struct netif *netif = (struct netif *)arg;

    /* Open the lwip dma channels and issue link up to Lwip stack */
    LWIPIF_LWIP_openDma(netif);

    sys_lock_tcpip_core();
    netif_set_link_up(netif);
    sys_unlock_tcpip_core();
}
#endif

/* Functions called from Config server library based on selection from GUI */
#if defined(ETHFW_DEMO_SUPPORT)
static void EthApp_startSwInterVlan(char *recvBuff,
                                    char *sendBuff)
{
    EnetCfgServer_InterVlanConfig *pInterVlanCfg;
    int32_t status;

    if (recvBuff != NULL)
    {
        pInterVlanCfg = (EnetCfgServer_InterVlanConfig *)recvBuff;
        status = EthSwInterVlan_addClassifierEntries(pInterVlanCfg);
        EnetAppUtils_assert(ENET_SOK == status);
    }
}

static void EthApp_startHwInterVlan(char *recvBuff,
                                    char *sendBuff)
{
    EnetCfgServer_InterVlanConfig *pInterVlanCfg;

    if (recvBuff != NULL)
    {
        pInterVlanCfg = (EnetCfgServer_InterVlanConfig *)recvBuff;
        EthHwInterVlan_setupRouting(gEthAppObj.enetType, gEthAppObj.instId, pInterVlanCfg);
    }
}
#endif

#if defined(ETHFW_GPTP_SUPPORT)
int32_t EthFwTsn_lldCfgUpdateCb(cb_socket_lldcfg_update_t *update_cfg)
{
    if (update_cfg->proto == ETH_P_1588)
    {
        update_cfg->numRxChannels = 1U;
#if defined(MCU_PLUS_SDK)
        update_cfg->dmaTxChId = ENET_DMA_TX_CH_PTP;
        update_cfg->dmaRxChId[0] = ENET_DMA_RX_CH_PTP;
        update_cfg->nTxPkts = ENET_DMA_TX_CH_PTP_NUM_PKTS;
        update_cfg->nRxPkts[0] = ENET_DMA_RX_CH_PTP_NUM_PKTS;
        update_cfg->pktSize = ENET_MEM_LARGE_POOL_PKT_SIZE;
#endif
    }

    return 0;
}
static void EthApp_configPtpCb(void *arg)
{
    EthFwTsn_gPTPConfigArg *cbArgs = (EthFwTsn_gPTPConfigArg *)arg;
    int32_t useHwPhase = 1;
    uint32_t ppsFreqHz = 0U;
    uint32_t genfIdx = 0U;

    if (gEthAppObj.enablePPS)
    {
        ppsFreqHz = ETHAPP_PPS_TIMESYNC_REF_CLK_HZ;
        genfIdx = ETHAPP_PPS_TIMESYNC_GENF_INST_IDX;
    }

    /* Apply phase adjustment directly to the HW */
    gptpgcfg_set_item(cbArgs->inst, XL4_EXTMOD_XL4GPTP_USE_HW_PHASE_ADJUSTMENT,
                      YDBI_CONFIG, &useHwPhase, sizeof(useHwPhase));
    gptpgcfg_set_item(cbArgs->inst, XL4_EXTMOD_XL4GPTP_CONF_TILLD_PPS_REFCLK_HZ,
                      YDBI_CONFIG, &ppsFreqHz, sizeof(ppsFreqHz));
    gptpgcfg_set_item(cbArgs->inst, XL4_EXTMOD_XL4GPTP_CONF_TILLD_PPS_OUTIDX,
                      YDBI_CONFIG, &genfIdx, sizeof(genfIdx));
}

static void EthApp_initPtp(void)
{
    uint32_t portMask = 0U;
    uint8_t i;

    /* MAC port used for PTP */
    for (i = 0U; i < ENET_ARRAYSIZE(gEthAppSwitchPorts); i++)
    {
        portMask |= ENET_MACPORT_MASK(gEthAppSwitchPorts[i]);
    }

    /* Wait for host port MAC address to be allocated during lwIP getHandle */
    appRtosSemaphorePend(gEthAppObj.hHostMacAllocSem, APP_RTOS_SEMAPHORE_WAIT_FOREVER);

    cb_socket_set_lldcfg_update_cb(EthFwTsn_lldCfgUpdateCb);

    /* Due to limitation with the number of MAC addresses provisioned in TI EVMs,
     * ETHFW MAC address will be used by TSN stack for packets sent on any port.
     * If there were sufficient addresses in the pool, it's recommended to pass
     * NULL pointer so TSN stack allocates one MAC address for every port */
    EthFwTsn_initTimeSyncPtp(&gEthAppObj.hostMacAddr[0U], portMask);
}

#endif

#if defined(ETHAPP_ENABLE_INTERCORE_ETH)
/* Application callback function to handle addition of a shared mcast
 * address in the ALE */
static void EthApp_filterAddMacSharedCb(const uint8_t *mac_address,
                                        uint8_t hostId)
{
    uint8_t idx = 0;
    struct eth_addr ethaddr;
    bool matchFound = BFALSE;
    int32_t errVal = 0;

    /* Search the mac_address in the shared mcast addr table */
    for (idx = 0; idx < ARRAY_SIZE(gEthApp_sharedMcastCfgTable); idx++)
    {
        if (EnetUtils_cmpMacAddr(mac_address,
                    &gEthApp_sharedMcastCfgTable[idx].macAddr[0]))
        {
            matchFound = BTRUE;
            /* Read and update stored port mask */
            gEthApp_bridgePortMask[idx] |= ETHFW_BIT(gEthApp_lwipBridgePortIdMap[hostId]);

            /* Update bridge fdb entry for this mac_address */
            EnetUtils_copyMacAddr(&ethaddr.addr[0U], mac_address);

            /* There will be a delay between removing existing FDB entry
             * and adding the updated one. During this time, multicast
             * packets will be flooded to all the bridge ports
             */
            bridgeif_fdb_remove(&netif_bridge, &ethaddr);

            errVal = bridgeif_fdb_add(&netif_bridge,
                                      &ethaddr,
                                      gEthApp_bridgePortMask[idx]);

            if (errVal)
            {
                appLogPrintf("addMacSharedCb: bridgeif_fdb_add failed (%d)\n", errVal);
            }

            /* The array should have unique mcast addresses,
             * so no other match is expected
             */
            break;
        }
    }

    if (!matchFound)
    {
        appLogPrintf("addMacSharedCb: Address not found\n");
    }
}

/* Application callback function to handle deletion of a shared mcast
 * address from the ALE */
static void EthApp_filterDelMacSharedCb(const uint8_t *mac_address,
                                        uint8_t hostId)
{
    uint8_t idx = 0;
    bridgeif_portmask_t bridgePortMask;
    struct eth_addr ethaddr;
    bool matchFound = BFALSE;
    int32_t errVal = 0;

    /* Search the mac_address in the shared mcast addr table */
    for (idx = 0; idx < ARRAY_SIZE(gEthApp_sharedMcastCfgTable); idx++)
    {
        if (EnetUtils_cmpMacAddr(mac_address,
                    &gEthApp_sharedMcastCfgTable[idx].macAddr[0]))
        {
            matchFound = BTRUE;
            /* Read and update stored port mask */
            gEthApp_bridgePortMask[idx] &= ~ETHFW_BIT(gEthApp_lwipBridgePortIdMap[hostId]);

            /* Update bridge fdb entry for this mac_address */
            EnetUtils_copyMacAddr(&ethaddr.addr[0U], mac_address);

            /* There will be a delay between removing existing FDB entry
             * and adding the updated one. During this time, multicast
             * packets will be flodded to all the bridge ports
             */
            bridgeif_fdb_remove(&netif_bridge, &ethaddr);

            if (gEthApp_bridgePortMask[idx])
            {
                errVal = bridgeif_fdb_add(&netif_bridge,
                                          &ethaddr,
                                          gEthApp_bridgePortMask[idx]);
            }

            if (errVal)
            {
                appLogPrintf("delMacSharedCb: bridgeif_fdb_add failed (%d)\n", errVal);
            }

            /* The array should have unique mcast addresses,
             * so no other match is expected
             */
            break;
        }
    }

    if (!matchFound)
    {
        appLogPrintf("delMacSharedCb: Address not found\n");
    }
}
#endif

#if defined(ETHFW_VEPA_SUPPORT)
/* Application callback function to handle addition of a shared mcast
 * address in the ALE */
static void EthApp_filterAddMacSharedCb(const uint8_t *mac_address,
                                        uint8_t hostId)
{
    uint8_t idx = 0;
    bool matchFound = BFALSE;

    /* Search the mac_address in the shared mcast addr table */
    for (idx = 0; idx < ARRAY_SIZE(gEthApp_sharedMcastCfgTable); idx++)
    {
        if (EnetUtils_cmpMacAddr(mac_address,
                    &gEthApp_sharedMcastCfgTable[idx].macAddr[0]))
        {
            matchFound = BTRUE;
            appLogPrintf("filterAddMacSharedCb: Address found: %x:%x:%x:%x:%x:%x\n",
                            mac_address[0],
                            mac_address[1],
                            mac_address[2],
                            mac_address[3],
                            mac_address[4],
                            mac_address[5]);
            /* The array should have unique mcast addresses,
             * so no other match is expected */
            break;
        }
    }

    if (!matchFound)
    {
        appLogPrintf("addMacSharedCb: Address not found\n");
    }
}

/* Application callback function to handle deletion of a shared mcast
 * address from the ALE */
static void EthApp_filterDelMacSharedCb(const uint8_t *mac_address,
                                        uint8_t hostId)
{
    uint8_t idx = 0;
    bool matchFound = BFALSE;

    /* Search the mac_address in the shared mcast addr table */
    for (idx = 0; idx < ARRAY_SIZE(gEthApp_sharedMcastCfgTable); idx++)
    {
        if (EnetUtils_cmpMacAddr(mac_address,
                    &gEthApp_sharedMcastCfgTable[idx].macAddr[0]))
        {
            matchFound = BTRUE;
            appLogPrintf("filterDelMacSharedCb: Address found: %x:%x:%x:%x:%x:%x\n",
                            mac_address[0],
                            mac_address[1],
                            mac_address[2],
                            mac_address[3],
                            mac_address[4],
                            mac_address[5]);
            /* The array should have unique mcast addresses,
             * so no other match is expected */
            break;
        }
    }

    if (!matchFound)
    {
        appLogPrintf("EthApp_Vepa_filterDelMacSharedCb: Address not found\n");
    }
}
#endif
