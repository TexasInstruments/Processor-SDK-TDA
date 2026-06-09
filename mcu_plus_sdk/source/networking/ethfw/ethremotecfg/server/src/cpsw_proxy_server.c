/*
 *
 * Copyright (c) 2020-2025 Texas Instruments Incorporated
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
/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x106

/* Enet utils header files */
#include <enet.h>
#include <include/per/cpsw.h>
#include <utils/include/enet_apputils.h>
#include <utils/include/enet_apprm.h>
#include <utils/include/enet_mcm.h>

/* EthFw utils header files */
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_common/include/ethfw_utils.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>
#include "cpsw_proxy_server.h"
#include "ethfw_mcast_priv.h"
#include "ethfw_arp_priv.h"
#include "ethfw_vlan_priv.h"
#if defined(ETHFW_VEPA_SUPPORT)
#include "ethfw_vepa_priv.h"
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define CPSWPROXY_CPSW9G_HWPUSH_BASE                     (26U)
#define CPSWPROXY_CPTS_HWPUSH_EVENTS_OR_MASK             (0xFFU)

#define CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_NAME            ("ASRETHDEVICE")
#define CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_PRIORITY        (2U)

#if defined(SAFERTOS)
#define CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_STACK           (16U * 1024U)
#define CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_ALIGN           CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_STACK
#else
#define CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_STACK           (0x4000U)
#define CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_ALIGN           (32U)
#endif

#define CPSWPROXY_AUTOSAR_ETHDRIVER_MSG_SIZE             (496U + 32U)
#define CPSWPROXY_AUTOSAR_ETHDRIVER_NUM_RPMSG_BUFS       (256U)
#define CPSWPROXY_AUTOSAR_ETHDRIVER_RPMSG_OBJ_SIZE       (256U)
#define CPSWPROXY_AUTOSAR_ETHDRIVER_DATA_SIZE            (CPSWPROXY_AUTOSAR_ETHDRIVER_MSG_SIZE * \
                                                          CPSWPROXY_AUTOSAR_ETHDRIVER_NUM_RPMSG_BUFS + \
                                                          CPSWPROXY_AUTOSAR_ETHDRIVER_RPMSG_OBJ_SIZE)

#define CPSWPROXY_ETH_CLIENT_TASK_NAME                   ("ETHREMOTEDEVICE")
#define CPSWPROXY_ETH_CLIENT_TASK_PRIORITY               (2U)

#if defined(SAFERTOS)
#define CPSWPROXY_ETH_CLIENT_TASK_STACK                  (16U * 1024U)
#define CPSWPROXY_ETH_CLIENT_TASK_ALIGN                  CPSWPROXY_ETH_CLIENT_TASK_STACK
#else
#define CPSWPROXY_ETH_CLIENT_TASK_STACK                  (0x4000U)
#define CPSWPROXY_ETH_CLIENT_TASK_ALIGN                  (32U)
#endif

#define CPSWPROXY_IPC_TASK_STACKALIGN                    (8192U)

#define ENET_COREKEY_CONVERT_MAGIC_NUM                   (0U)
#define CPSWPROXY_VIRTPORT_2_TOKEN(virtPort)             (((virtPort) * 100U) + ENET_COREKEY_CONVERT_MAGIC_NUM)
#define CPSWPROXY_TOKEN_2_VIRTPORT(token)                (((token) - ENET_COREKEY_CONVERT_MAGIC_NUM) / 100U)

/*! Remote notify service data size */
#define CPSWPROXY_NOTIFY_SERVICE_DATA_SIZE               ETHREMOTECFG_IPC_DATA_SIZE

/*! Remote notify service task stack size */
#define CPSWPROXY_NOTIFY_SERVICE_SERVER_TASK_STACKSIZE   (16U * 1024U)
#define CPSWPROXY_NOTIFY_SERVICE_SERVER_TASK_STACKALIGN  CPSWPROXY_NOTIFY_SERVICE_SERVER_TASK_STACKSIZE

/*! Remote notify service task name */
#define CPSWPROXY_NOTIFY_SERVICE_TASK_NAME               ("NOTIFY_SERVICE_TASK")
#define CPSWPROXY_NOTIFY_SERVICE_TASK_PRIORITY           (4U)

#define CPSWPROXY_PRINT_STATS_NONZERO(str, val)          ETHFWTRACE_INFO_IF(((val) != 0ULL), str, val)
#define CPSWPROXY_PRINT_STATS_IDX_NONZERO(str, idx, val) ETHFWTRACE_INFO_IF(((val) != 0ULL), str, idx, val)

#define CPSWPROXY_CLIENT_PRIMARY_REL_FLOW_IDX            (0U)
#define CPSWPROXY_CLIENT_INVALID_FLOW_IDX_OFFSET         (0xFFU)

/* Compile time check for error value consistency with ETHFW (and Enet LLD) */
#define CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(x)      ETHFW_UTILS_COMPILETIME_ASSERT(ETHFW_##x == ETHREMOTECFG_##x)
#define CPSWPROXY_ERR_CHECK(cond, err, val)              \
    do {                                                 \
        if (cond)                                        \
        {                                                \
          err = val;                                     \
        }                                                \
    } while (0)

#define CPSWPROXY_NOTIFY_SERVICE_ENDPT_ID                (24U)
#define CPSWPROXY_REMOTE_SERVICE_ENDPT_ID                (34U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/*
 * Client object
 */
typedef struct CpswProxyServer_ClientObj_s
{
    /* Whether client object is grabbed by a remote client */
    bool inUse;

    /* Token given to the remote client */
    uint32_t token;

    /* Virtual port id */
    EthRemoteCfg_VirtPort virtPort;

    /* Remote core id */
    uint32_t coreId;

    /* Client Id */
    uint32_t clientId;

    /* Features supported by related virtual port */
    uint32_t features;

    /* Allocated flow index base */
    uint32_t flowIdxBase;

    /* Allocated flow index offset */
    uint32_t flowIdxOffset;

    /* TX PSIL peer destination thread id */
    uint32_t psilDstId;

    /* Allocated MAC address */
    uint8_t macAddr[ENET_MAC_ADDR_LEN];

    /* End point of respective remote client */
    uint32_t remoteEp;

    /* If the client object has finished its teardown and became idle */
    bool isIdle;

#if defined(ETHFW_VEPA_SUPPORT)
    /* Vlan check count - register vlan once per virtual client for VEPA */
    uint32_t vlanRefCnt;
#endif
} CpswProxyServer_ClientObj;

/*
 * Remote Core Object, whose index points to the core Id of the remote core
 */
typedef struct CpswProxyServer_RemoteCoreObj_s
{
    /* Enet RM Reference Cnt per remote core */
    int32_t rmRefCnt;
    /* Enet LLD attachInfo, unique per coreId */
    EnetPer_AttachCoreOutArgs attachInfo;
    /* Client object for storing all the data associated with the client */
    CpswProxyServer_ClientObj clientObj[CPSWPROXYSERVER_VIRTPORT_PER_CLIENT_MAX];
} CpswProxyServer_RemoteCoreObj;

/*
 * Client object handle
 */
typedef CpswProxyServer_ClientObj *CpswProxyServer_ClientHandle;

typedef struct CpswProxyServer_AsrServiceObj_s
{
    /* Task handle for AUTOSAR IPC communication */
    EthFwOsal_TaskHandle         hAutosarEthTsk;
    /* RPMessage handle for AUTOSAR IPC communication */
    EthFwIpc_RpmsgHandle         hAutosarEthRpMsgEp;
    /* Processor Id of AUTOSAR client */
    uint32_t                     coreId;
    /* Local endpoint for the AUTOSAR client */
    uint32_t                     localEp;
    /* virtual port allocated for the AUTOSAR client, cuurently supports one virtPort */
    EthRemoteCfg_VirtPort        virtPort;
} CpswProxyServer_AsrServiceObj;

typedef struct CpswProxyServer_NotifyServiceObj_s
{
    Enet_Type                    notifyServiceCpswType;
    EthFwOsal_TaskHandle         hNotifyServiceTsk;
    EthFwOsal_EventHandle        hHwPushNotifyServiceEvent;
    uint32_t                     hwPushNotifyEventId[CPSW_CPTS_HWPUSH_COUNT_MAX];
    EthFwIpc_RpmsgHandle         hNotifyServicRpMsgEp;
    uint32_t                     hwPush2CoreIdMap[CPSW_CPTS_HWPUSH_COUNT_MAX];
    uint32_t                     dstProcMask;
    uint32_t                     localEp;
    uint32_t                     remoteEp;
} CpswProxyServer_NotifyServiceObj;

typedef struct CpswProxyServer_ClientServiceObj_s
{
    EthFwOsal_TaskHandle         hClientServiceTsk;
    EthFwIpc_RpmsgHandle         hClientServicRpMsgEp;
    EthFwOsal_SemHandle          rpmsgStartSem;
    uint32_t                     localEp;
} CpswProxyServer_ClientServiceObj;

typedef struct CpswProxyServer_Obj_s
{
    /* Mutex handle used to protect get/free CpswProxy_ClientObjs */
    EthFwOsal_MutexHandle hMutex;
    /* enetType of the server object */
    Enet_Type enetType;
    /* coreId of the master core (Server) */
    uint32_t masterCoreId;
    /* Instance Id of the CPSW server object */
    uint32_t instId;
    /* set to true when proxy server has been initialized */
    bool initDone;
    /* callback which populates Ethernet Firmware device data */
    CpswProxyServer_InitEthfwDeviceDataCb initEthfwDeviceDataCb;
    /* Callback to retrieve Mcm cmd handle of ETHFW */
    CpswProxyServer_GetMcmCmdIfCb         getMcmCmdIfCb;
    /* Callback for handling C2S notify message */
    CpswProxyServer_NotifyCb              notifyCb;
    /* Object containing the data required to communicate with AUTOSAR client */
    CpswProxyServer_AsrServiceObj         ethDrvObj[CPSWPROXYSERVER_AUTOSAR_REMOTE_CLIENT_MAX];
    /* Object contaning the data required to communicate with other remote clients */
    CpswProxyServer_ClientServiceObj      clientServiceObj;
    /* Object for notify service required to communicate with remote clients */
    CpswProxyServer_NotifyServiceObj      notifyServiceObj;
    /* ALE port mask for all supported MAC ports */
    uint32_t alePortMask;
    /* ALE port mask for MAC only ports */
    uint32_t aleMacOnlyPortMask;
    /* ALE port mask for switch only ports */
    uint32_t aleSwitchOnlyPortMask;
    /* Default VLAN id to be used for MAC ports configured in MAC-only mode */
    uint16_t dfltVlanIdMacOnlyPorts;
    /* Default VLAN id to be used for MAC ports configured in switch mode (non MAC-only) */
    uint16_t dfltVlanIdSwitchPorts;
    /* Enet Mcm Cmd handle */
    EnetMcm_CmdIf  *hMcmCmdIf;
    /* Handle to Enet LLD, unique per CPSW instance */
    Enet_Handle hEnet;
    /* Remote core object for holding all core specific information */
    CpswProxyServer_RemoteCoreObj coreObj[IPC_MAX_PROCS];
    /* set to true when checksum offload is enabled */
    bool csumOffloadEn;
    /* Virtual port configurations */
    EthFwVirtPort_VirtPortCfg virtPortCfg[CPSWPROXYSERVER_REMOTE_CLIENT_VIRTPORT_MAX];
    /* Number of remote virtual ports that remotes cores can attach */
    uint32_t numVirtPorts;
    /* Number of remote virtual ports that AUTOSAR can attach */
    uint32_t numAsrVirtPorts;
} CpswProxyServer_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
static int32_t CpswProxyServer_initAutosarEthDeviceEp(CpswProxyServer_Obj * hServer,
                                                      CpswProxyServer_Config_t * cfg,
                                                      uint32_t clientInst);

static void CpswProxyServer_autosarEthDriverTaskFxn(void* arg0);

static void CpswProxyServer_remoteClientEthDriverTaskFxn(void* arg0);

static int32_t CpswProxyServer_validateStartIdx(Enet_Handle hEnet,
                                                uint32_t hostId,
                                                uint32_t rxFlowStartId);

static int32_t CpswProxyServer_initRemoteClientEthDeviceEp(CpswProxyServer_Obj * hServer,
                                                           CpswProxyServer_Config_t * cfg);

static int32_t CpswProxyServer_regMacPortFlow(Enet_Handle hEnet,
                                              uint32_t coreKey,
                                              uint32_t remoteCoreId,
                                              Enet_MacPort macPort,
                                              uint8_t *macAddr,
                                              uint32_t flowStartIdx,
                                              uint32_t flowIdx);

static int32_t CpswProxyServer_unregMacPortFlow(Enet_Handle hEnet,
                                                uint32_t coreKey,
                                                uint32_t remoteCoreId,
                                                Enet_MacPort macPort,
                                                uint8_t *macAddr,
                                                uint32_t flowStartIdx,
                                                uint32_t flowIdx);

static void CpswProxyServer_hwPushNotifyFxn(void *arg, CpswCpts_HwPush hwPushNum);

static void CpswProxyServer_notifyServiceTaskFxn(void* arg0);

static int32_t CpswProxyServer_initNotifyServiceEp(CpswProxyServer_Obj * hServer,
                                                   CpswProxyServer_Config_t * cfg);

static int32_t CpswProxyServer_sendNotify(CpswProxyServer_ClientHandle hClient,
                                          uint32_t notifyId);

static uint32_t CpswProxyServer_getAbsTxChNumber(uint32_t chRelPriority,
                                                 EthRemoteCfg_VirtPort virtPort);

static int32_t CpswProxyServer_getClientTxChRxFlowNum(EthRemoteCfg_VirtPort virtPort,
                                                      uint32_t *pNumTxCh,
                                                      uint32_t *pNumRxFlow);

static int32_t CpswProxyServer_createCustomPolicer(Enet_Handle hEnet,
                                                   uint32_t coreId,
                                                   CpswAle_SetPolicerEntryInPartitionInArgs *customPolInArgs);

static int32_t CpswProxyServer_deleteCustomPolicer(Enet_Handle hEnet,
                                                   uint32_t coreId,
                                                   CpswAle_SetPolicerEntryInPartitionInArgs *customPolInArgs);

static int32_t CpswProxyServer_getRxFlowInfo(EthRemoteCfg_VirtPort virtPort,
                                             uint32_t relFlowIdx,
                                             EthFwQos_RxFlowInfo **rxFlowInfo);

/* Returns relative flow index from rx flows assigned to this virtual port */
static int32_t CpswProxyServer_getRelFlowIdx(EthRemoteCfg_VirtPort virtPort,
                                             uint32_t *relFlowIdx,
                                             uint32_t rxFlowIdxOffset);

/* Stores the flowIdxOffset assigned to a specific flow in virtual port configuration */
static void CpswProxyServer_saveRxFlowOffset(EthRemoteCfg_VirtPort virtPort,
                                             uint32_t relFlowIdx,
                                             uint32_t rxFlowIdxOffset);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/**< Buffer to store received messages. 256 messages of 512 bytes +
        space for book-keeping */
static uint8_t g_CpswProxyServerAutosarRpmsgBuf[CPSWPROXYSERVER_AUTOSAR_REMOTE_CLIENT_MAX][CPSWPROXY_AUTOSAR_ETHDRIVER_DATA_SIZE]  __attribute__ ((aligned(CPSWPROXY_IPC_TASK_STACKALIGN)));

/**< StackBuffer for different tasks */
static uint8_t gCpswProxyServer_autosarEthDriverTaskStackBuf[CPSWPROXYSERVER_AUTOSAR_REMOTE_CLIENT_MAX][CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_STACK] __attribute__ ((aligned(CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_ALIGN)));

static uint8_t gCpswProxyServerRpmsgbuf[ETHREMOTECFG_IPC_DATA_SIZE] __attribute__ ((aligned(1024)));

static uint8_t gCpswProxyServer_remoteClientEthDriverTaskStackBuf[CPSWPROXY_ETH_CLIENT_TASK_STACK] __attribute__ ((aligned(CPSWPROXY_IPC_TASK_STACKALIGN)));

/**< Buffer to store received messages. 256 messages of 512 bytes +
        space for book-keeping */
static uint8_t g_CpswProxyServerNotifyServiceRpmsgBuf[CPSWPROXY_NOTIFY_SERVICE_DATA_SIZE]  __attribute__ ((aligned(8192)));

static uint8_t gCpswProxyServer_notifyServiceTaskStackBuf[CPSWPROXY_NOTIFY_SERVICE_SERVER_TASK_STACKSIZE] __attribute__ ((aligned(CPSWPROXY_NOTIFY_SERVICE_SERVER_TASK_STACKALIGN)));

extern EthRemoteCfg_ServerStatus EthFw_getStatus(void);
/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void CpswProxyServer_compileTimeChecks(void)
{
    /* Verify that ETHREMOTECFG error types are consistent with ETHFW error types */
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(SOK);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(SINPROGRESS);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EFAIL);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EBADARGS);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EINVALIDPARAMS);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(ETIMEOUT);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EALLOC);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EUNEXPECTED);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EBUSY);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EALREADYOPEN);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(EPERM);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(ENOTSUPPORTED);
    CPSWPROXY_COMPILETIME_ETHREMOTECFG_CHECK(ENOTFOUND);
}

static CpswProxyServer_Obj *CpswProxyServer_initHandle(void)
{
    static CpswProxyServer_Obj gProxyServerObj =
    {
        .initEthfwDeviceDataCb = NULL,
        .getMcmCmdIfCb         = NULL,
        .initDone              = BFALSE,
    };

    return (&gProxyServerObj);
}

static int32_t CpswProxyServer_getHandle(CpswProxyServer_Obj **hServer)
{
    int32_t status = ETHREMOTECFG_SOK;

    /* Check that server itself is ready */
    *hServer = CpswProxyServer_initHandle();
    if (*hServer == NULL)
    {
        status = ETHREMOTECFG_EFAIL;
        ETHFWTRACE_ERR(status, "ETHFW server has not been created");
    }
    else if ((*hServer)->initDone != BTRUE)
    {
        status = ETHREMOTECFG_EBUSY;
        ETHFWTRACE_ERR(status, "ETHFW server is not ready");
    }
    else
    {
        status = ETHREMOTECFG_SOK;
    }

    return status;
}

static CpswProxyServer_ClientHandle CpswProxyServer_allocClient(uint32_t remoteProcId,
                                                                uint32_t remoteEndPt,
                                                                uint32_t virtPort)
{
    CpswProxyServer_Obj *hServer = NULL;
    CpswProxyServer_ClientHandle hClient = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t i;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    EthFwOsal_lockMutex(hServer->hMutex);

    for (i = 0U; i < ENET_ARRAYSIZE(hServer->coreObj[remoteProcId].clientObj); i++)
    {
        hClient = &hServer->coreObj[remoteProcId].clientObj[i];
        if (!hClient->inUse)
        {
            hClient->inUse = BTRUE;
            hClient->virtPort  = virtPort;
            hClient->token = ETHREMOTECFG_TOKEN_NONE;
            hClient->remoteEp = remoteEndPt;
            hClient->isIdle = BFALSE;
#if defined(ETHFW_VEPA_SUPPORT)
            hClient->vlanRefCnt = 0U;
#endif
            break;
        }
        else
        {
            hClient = NULL;
        }
    }

    EthFwOsal_unlockMutex(hServer->hMutex);

    if (hClient == NULL)
    {
        ETHFWTRACE_ERR(ETHREMOTECFG_EALLOC, "Failed to allocate client");
    }

    return hClient;
}

static void CpswProxyServer_freeClient(CpswProxyServer_ClientHandle hClient)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        EthFwOsal_lockMutex(hServer->hMutex);
        memset(hClient, 0, sizeof(*hClient));
        hClient->inUse = BFALSE;
        hClient->token = ETHREMOTECFG_TOKEN_NONE;
        EthFwOsal_unlockMutex(hServer->hMutex);
    }
}

static CpswProxyServer_ClientHandle CpswProxyServer_getClient(uint32_t remoteProcId,
                                                              uint32_t token)
{
    CpswProxyServer_Obj *hServer = NULL;
    CpswProxyServer_ClientHandle hClient = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    bool isFound = BFALSE;
    uint32_t i;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    EthFwOsal_lockMutex(hServer->hMutex);

    for (i = 0U; i < ENET_ARRAYSIZE(hServer->coreObj[remoteProcId].clientObj); i++)
    {
        hClient = &hServer->coreObj[remoteProcId].clientObj[i];

        if (hClient->inUse && (hClient->token == token))
        {
            isFound = BTRUE;
            break;
        }
    }

    EthFwOsal_unlockMutex(hServer->hMutex);

    if(!isFound)
    {
        hClient = NULL;
        ETHFWTRACE_ERR(ETHREMOTECFG_EBADARGS, "Failed to get a client handle");
    }

    return hClient;
}

int32_t CpswProxyServer_getIdleClientCnt(uint32_t *attachedClients,
                                         uint32_t *idleClients)
{
    CpswProxyServer_Obj *hServer = NULL;
    CpswProxyServer_ClientHandle hClient = NULL;
    int32_t status = ETHFW_SOK;
    *attachedClients = 0U;
    *idleClients = 0U;
    uint32_t i;
    uint32_t j;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < ENET_ARRAYSIZE(hServer->coreObj); i++)
    {
        for (j = 0U; j < ENET_ARRAYSIZE(hServer->coreObj[i].clientObj); j++)
        {
            hClient = &hServer->coreObj[i].clientObj[j];

            if (hClient->inUse && (hClient->token != ETHREMOTECFG_TOKEN_NONE))
            {
                (*attachedClients)++;

                if (hClient->isIdle)
                {
                    (*idleClients)++;
                }
            }
        }
    }
    return status;
}

static int32_t CpswProxyServer_sendNotify(CpswProxyServer_ClientHandle hClient,
                                          uint32_t notifyId)
{
    CpswProxyServer_Obj *hServer = NULL;
    EthRemoteCfg_CommonNotify notifyMsg;
    EthFwIpc_RpmsgHandle handle = NULL;
    uint32_t srcEndPt = 0U;
    uint32_t clientInst;
    int32_t status = ETHFW_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    notifyMsg.hdr.common.msgType = ETHREMOTECFG_MSGTYPE_NOTIFY;
    notifyMsg.hdr.common.clientId = hClient->clientId;
    notifyMsg.hdr.common.token = hClient->token;
    notifyMsg.hdr.notifyType = notifyId;

    if (hClient->clientId == ETHREMOTECFG_CLIENTID_AUTOSAR)
    {
        for (clientInst = 0U; clientInst < CPSWPROXYSERVER_AUTOSAR_REMOTE_CLIENT_MAX; clientInst++)
        {
            if( hServer->ethDrvObj[clientInst].coreId == hClient->coreId)
            {
                handle = hServer->ethDrvObj[clientInst].hAutosarEthRpMsgEp;
                srcEndPt = hServer->ethDrvObj[clientInst].localEp;
                break;
            }
        }
    }
    else
    {
        handle = hServer->clientServiceObj.hClientServicRpMsgEp;
        srcEndPt = hServer->clientServiceObj.localEp;
    }

    if (handle == NULL)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Couldn't find core %u client handle", hClient->coreId);
    }

    if (status == ETHFW_SOK)
    {
        ETHFWTRACE_INFO("NOTIFY | S2C | core=%u endpt=%u token=%d notifyType=%u",
                        hClient->coreId,
                        hClient->remoteEp,
                        (int32_t)notifyMsg.hdr.common.token,
                        notifyId);

        ETHFWTRACE_DBG("S2C | msgType=%u token=%d clientId=%u notifyType=%u len=%u (%u.%u->%u.%u)",
                       notifyMsg.hdr.common.msgType,
                       (int32_t)notifyMsg.hdr.common.token,
                       notifyMsg.hdr.common.clientId,
                       notifyMsg.hdr.notifyType,
                       sizeof(notifyMsg),
                       EnetSoc_getCoreId(), srcEndPt,
                       hClient->coreId, hClient->remoteEp);

        status = EthFwIpc_sendRpmsg(handle,
                                    hClient->coreId,
                                    hClient->remoteEp,
                                    srcEndPt,
                                    &notifyMsg,
                                    sizeof(notifyMsg));
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to send notify msg via IPC");
    }

    return status;
}

int32_t CpswProxyServer_bcastNotify(uint32_t notifyId)
{
    CpswProxyServer_Obj *hServer = NULL;
    CpswProxyServer_ClientHandle hClient = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t i;
    uint32_t j;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < ENET_ARRAYSIZE(hServer->coreObj); i++)
    {
        for (j = 0U; j < ENET_ARRAYSIZE(hServer->coreObj[i].clientObj); j++)
        {
            hClient = &hServer->coreObj[i].clientObj[j];

            if (hClient->inUse)
            {
                /* Set isIdle flag to false for next iteration of recovery */
                hClient->isIdle = BFALSE;
                status = CpswProxyServer_sendNotify(hClient, notifyId);
                ETHFWTRACE_ERR_IF((ETHFW_SOK != status), status,
                                  "Send Notification failed for coreId: %u", i);
            }
            EthFwOsal_sleepTask(50);
        }
    }

    return status;
}

static int32_t CpswProxyServer_getPortMask(uint32_t clientId,
                                           uint32_t hostId,
                                           uint32_t *switchPortMask,
                                           uint32_t *macPortMask)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    bool isSwitchPort = BFALSE;
    bool isFound = BFALSE;
    uint32_t i;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < hServer->numVirtPorts; i++)
    {
        isSwitchPort = EthFwVirtPort_isSwitchPort(hServer->virtPortCfg[i].portId);
        if ((hServer->virtPortCfg[i].remoteCoreId == hostId) &&
            ETHFW_IS_BIT_SET(hServer->virtPortCfg[i].clientIdMask, clientId))
        {
            if (isSwitchPort)
            {
                *switchPortMask = *switchPortMask | ENET_MACPORT_MASK(hServer->virtPortCfg[i].portId);
            }
            else
            {
                *macPortMask = *macPortMask | ENET_MACPORT_MASK(hServer->virtPortCfg[i].portId);
            }
            isFound = BTRUE;
        }
    }
    if (!isFound)
    {
        status = ETHREMOTECFG_EBADARGS;
        ETHFWTRACE_ERR(status, "No port mask found for clientId: %u and coreId: %u",
                       clientId, hostId);
    }
    return status;
}

static int32_t CpswProxyServer_VirtPortAllocCb(uint32_t clientId,
                                               uint32_t hostId,
                                               uint32_t *switchPortMask,
                                               uint32_t *macPortMask)
{
    int32_t status = ETHREMOTECFG_SOK;

    if (clientId <= ETHREMOTECFG_CLIENTID_LAST)
    {
        *switchPortMask = 0U;
        *macPortMask = 0U;
        status = CpswProxyServer_getPortMask(clientId, hostId, switchPortMask, macPortMask);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                           "Failed to get port mask for clientId: %u", clientId);
    }
    else
    {
        status = ETHREMOTECFG_EBADARGS;
        ETHFWTRACE_ERR(status, "Invalid client id %u", clientId);
    }

    return status;
}


static int32_t CpswProxyServer_attachHandlerCb(CpswProxyServer_ClientHandle hClient,
                                               uint32_t hostId,
                                               uint32_t portId,
                                               uint32_t *pRxMtu,
                                               uint32_t *pTxMtu,
                                               uint32_t *pFeatures,
                                               uint32_t *pNumTxCh,
                                               uint32_t *pNumRxFlow)
{
    CpswProxyServer_Obj *hServer = NULL;
    EnetPer_AttachCoreOutArgs attachInfo;
    EthRemoteCfg_VirtPort virtPort = ETHREMOTECFG_VIRTPORT_DENORM(portId);
    bool isMacPort = EthFwVirtPort_isMacPort(virtPort);
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t i;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        if (hServer->coreObj[hostId].rmRefCnt == 0U)
        {
            EnetMcm_coreAttach(hServer->hMcmCmdIf, hostId, &attachInfo);
            memcpy(&hServer->coreObj[hostId].attachInfo, &attachInfo, sizeof(attachInfo));
        }

        hServer->coreObj[hostId].rmRefCnt++;

        attachInfo = hServer->coreObj[hostId].attachInfo;
        *pRxMtu = attachInfo.rxMtu;

        /* Check if all MTUs in attachInfo.txMtu[] are of same size, then update *pTxMtu with first value
        attachInfo.txMtu[]. */
        for (i = 0U; i < ETHFW_ARRAYSIZE(attachInfo.txMtu); i++)
        {
            if (ETHREMOTECFG_SOK == status)
            {
                if (memcmp(&attachInfo.txMtu[i], &attachInfo.txMtu[0U], sizeof(attachInfo.txMtu[0U])) != 0)
                {
                    status = ETHREMOTECFG_EFAIL;
                    ETHFWTRACE_ERR(status, "All MTUs in attachInfo.txMtu[] are not of same size.");
                }
            }
        }

        if (ETHREMOTECFG_SOK == status)
        {
            *pTxMtu = attachInfo.txMtu[0U];

            *pFeatures = 0U;
            if (hServer->csumOffloadEn)
            {
                *pFeatures |= ETHREMOTECFG_FEATURE_TXCSUM;
            }
            if (!isMacPort)
            {
                *pFeatures |= ETHREMOTECFG_FEATURE_MC_FILTER;
            }

            /* If number of tx channels and rx flows allocated are required by the client
            * i.e. called not called from CpswProxyServer_attachExtHandlerCb attach */
            if (pNumTxCh != NULL && pNumRxFlow != NULL)
            {
                status = CpswProxyServer_getClientTxChRxFlowNum(virtPort, pNumTxCh, pNumRxFlow);
                ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                "Failed to get tx and rx channels for virtual port %d", virtPort);
            }

            /* Save parameters in client object */
            hClient->token     = CPSWPROXY_VIRTPORT_2_TOKEN(virtPort);
            hClient->coreId    = hostId;
            hClient->features  = *pFeatures;
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Attach failed for coreId: %u", hostId);
    }

    return status;
}


static int32_t CpswProxyServer_attachExtHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                  uint32_t hostId,
                                                  uint32_t portId,
                                                  uint32_t *pRxMtu,
                                                  uint32_t *pTxMtu,
                                                  uint32_t *pFeatures,
                                                  uint32_t *pRxFlowIdxBase,
                                                  uint32_t *pRxFlowIdxOffset,
                                                  uint32_t *pTxPsilDstId,
                                                  uint32_t *pRxPsilSrcId,
                                                  uint8_t *macAddr)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t coreKey;

    /* Actual attach operation */
    status = CpswProxyServer_attachHandlerCb(hClient, hostId, portId, pRxMtu, pTxMtu, pFeatures, NULL, NULL);

    if (ETHREMOTECFG_SOK == status)
    {
        status = CpswProxyServer_getHandle(&hServer);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");
    }

    /* Allocate RX flow */
    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        status = EnetAppUtils_allocRxFlow(hServer->hEnet,
                                          coreKey,
                                          hostId,
                                          pRxFlowIdxBase,
                                          pRxFlowIdxOffset);
        if (ENET_SOK == status)
        {
            status = CpswProxyServer_validateStartIdx(hServer->hEnet, hostId, *pRxFlowIdxBase);
            ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                               "RX flow start index validation failed");

            if(ETHREMOTECFG_SOK == status)
            {
                CpswProxyServer_saveRxFlowOffset(hClient->virtPort, CPSWPROXY_CLIENT_PRIMARY_REL_FLOW_IDX, *pRxFlowIdxOffset);
            }
        }
    }

    /* Allocate TX channel */
    if (ENET_SOK == status)
    {
        status = EnetAppUtils_allocTxCh(hServer->hEnet,
                                        coreKey,
                                        hostId,
                                        pTxPsilDstId);
    }

    /* Allocate MAC address */
    if (ENET_SOK == status)
    {
        status = EnetAppUtils_allocMac(hServer->hEnet,
                                       coreKey,
                                       hostId,
                                       macAddr);
    }

    if (ENET_SOK == status)
    {
        /* RX PSIL thread id */
        *pRxPsilSrcId = EnetSoc_getRxChPeerId(hServer->enetType, hServer->instId, 0U);

        /* Save parameters in client object */
        hClient->flowIdxBase   = *pRxFlowIdxBase;
        hClient->flowIdxOffset = *pRxFlowIdxOffset;
        hClient->psilDstId     = *pTxPsilDstId;
        EnetUtils_copyMacAddr(&hClient->macAddr[0U], macAddr);
    }

    ETHFWTRACE_ERR_IF((ENET_SOK != status), status, "Attach Ext failed for coreId: %u", hostId);

    return status;
}

static int32_t CpswProxyServer_allocTxHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                uint32_t hostId,
                                                uint32_t *pTxPsilDstId,
                                                uint32_t chRelPriority)
{
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t coreKey;
    uint32_t txChNum;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;
        /* Get absolute tx channel number from relative tx channel */
        txChNum = CpswProxyServer_getAbsTxChNumber(chRelPriority, hClient->virtPort);
        if (txChNum == ENET_RM_TXCHNUM_INVALID)
        {
            status = ETHREMOTECFG_EBADARGS;
            ETHFWTRACE_ERR(status, "Absolute TX channel number: %d is invalid",
                           ENET_RM_TXCHNUM_INVALID);
        }
        else
        {
            status = EnetAppUtils_allocAbsTxCh(hServer->hEnet,
                                              coreKey,
                                              hostId,
                                              pTxPsilDstId,
                                              txChNum);
        }

        if (ENET_SOK == status)
        {
            hClient->psilDstId = *pTxPsilDstId;
        }
        else
        {
            ETHFWTRACE_ERR(status, "Failed to alloc TX channel");
        }
    }

    return status;
}

static int32_t CpswProxyServer_allocRxHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                uint32_t hostId,
                                                uint32_t *pRxFlowIdxBase,
                                                uint32_t *pRxFlowIdxOffset,
                                                uint32_t *pRxPsilSrcId,
                                                uint32_t relFlowIdx)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t coreKey;
    EthFwQos_RxFlowInfo *rxFlowInfo;
    uint32_t i;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;
        *pRxPsilSrcId = EnetSoc_getRxChPeerId(hServer->enetType, hServer->instId, 0U);

        status = EnetAppUtils_allocRxFlow(hServer->hEnet,
                                            coreKey,
                                            hostId,
                                            pRxFlowIdxBase,
                                            pRxFlowIdxOffset);

        if (ENET_SOK == status)
        {
            status = CpswProxyServer_validateStartIdx(hServer->hEnet, hostId, *pRxFlowIdxBase);
            ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                               "RX flow start index validation failed");
        }
        else
        {
            ETHFWTRACE_ERR(status, "Failed to alloc RX channel");
        }

        if (status == ENET_SOK)
        {
            hClient->flowIdxBase   = *pRxFlowIdxBase;
            hClient->flowIdxOffset = *pRxFlowIdxOffset;

            CpswProxyServer_saveRxFlowOffset(hClient->virtPort, relFlowIdx, *pRxFlowIdxOffset);
            status = CpswProxyServer_getRxFlowInfo(hClient->virtPort, relFlowIdx, &rxFlowInfo);
            ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                               "Failed to get virtual port %u rx flow %u info",
                               hClient->virtPort,
                               pRxFlowIdxOffset);
            if (status == ETHREMOTECFG_SOK)
            {
                /* Flow has been allocated, create custom policer for it (if any) */
                for (i = 0U; i < rxFlowInfo->numCustomPolicers; i++)
                {
                    rxFlowInfo->customPolicersInArgs[i]->threadId = *pRxFlowIdxOffset;
                    status = CpswProxyServer_createCustomPolicer(hServer->hEnet, hostId, rxFlowInfo->customPolicersInArgs[i]);
                    ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                                       "Failed to create custom policer for virtual port %u flow %u",
                                       hClient->virtPort,
                                       *pRxFlowIdxOffset);
                }
            }
        }
    }

    return status;
}

static int32_t CpswProxyServer_allocMacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                 uint32_t hostId,
                                                 uint8_t *macAddr)
{
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t coreKey;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        status = EnetAppUtils_allocMac(hServer->hEnet,
                                       coreKey,
                                       hostId,
                                       macAddr);
        if (ENET_SOK == status)
        {
            EnetUtils_copyMacAddr(&hClient->macAddr[0U], macAddr);
        }
        else
        {
            ETHFWTRACE_ERR(status, "Failed to alloc MAC Address");
        }
    }

    return status;
}

static int32_t CpswProxyServer_detachHandlerCb(CpswProxyServer_ClientHandle hClient,
                                               uint32_t hostId)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t coreKey;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        /* Detach from MCM */
        if (hServer->hMcmCmdIf != NULL)
        {
            hServer->coreObj[hostId].rmRefCnt--;
            if (hServer->coreObj[hostId].rmRefCnt == 0U)
            {
                EnetMcm_coreDetach(hServer->hMcmCmdIf, hostId, coreKey);
            }
        }
        else
        {
            status = ETHREMOTECFG_EINVALIDPARAMS;
            ETHFWTRACE_ERR(status, "Cannot detach as MCM handle is NULL");
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to detach the coreId: %u", hostId);
    }

    return status;
}

static int32_t CpswProxyServer_freeTxHandlerCb(CpswProxyServer_ClientHandle hClient,
                                               uint32_t hostId,
                                               uint32_t pTxPsilDstId)
{
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t coreKey;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        status = EnetAppUtils_freeTxCh(hServer->hEnet,
                                       coreKey,
                                       hostId,
                                       pTxPsilDstId);

        if (ENET_SOK == status)
        {
            hClient->psilDstId = 0U;
        }
        else
        {
            ETHFWTRACE_ERR(status, "Failed to free Tx channel with Id: %u", pTxPsilDstId);
        }
    }

    return status;
}

static int32_t CpswProxyServer_freeRxHandlerCb(CpswProxyServer_ClientHandle hClient,
                                               uint32_t hostId,
                                               uint32_t rxFlowIdxBase,
                                               uint32_t rxFlowIdxOffset)
{
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t coreKey;
    uint32_t relFlowIdx = 0U;
    EthFwQos_RxFlowInfo *rxFlowInfo;
    uint32_t i;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (status == ETHREMOTECFG_SOK)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        status = CpswProxyServer_getRelFlowIdx(hClient->virtPort, &relFlowIdx, rxFlowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "Failed to get relative flow index for virtual port %u flow %u",
                            hClient->virtPort,
                            rxFlowIdxOffset);
        if (ETHREMOTECFG_SOK == status)
        {
            status = CpswProxyServer_getRxFlowInfo(hClient->virtPort, relFlowIdx, &rxFlowInfo);
            ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                               "Failed to get virtual port %u rx flow %u info",
                                hClient->virtPort,
                                rxFlowIdxOffset);
            /* Remove custom policers for this flow (if any) */
            for (i = 0U; i < rxFlowInfo->numCustomPolicers; i++)
            {
                status = CpswProxyServer_deleteCustomPolicer(hServer->hEnet, hostId, rxFlowInfo->customPolicersInArgs[i]);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                                   "Failed to delete custom policer for virtual port %u flow %u",
                                    hClient->virtPort,
                                    rxFlowIdxOffset);
            }
            /* Remove flowIdxOffset assigned to specific flow from virtual port config */
            CpswProxyServer_saveRxFlowOffset(hClient->virtPort, relFlowIdx, CPSWPROXY_CLIENT_INVALID_FLOW_IDX_OFFSET);
        }
    }

    if (ETHREMOTECFG_SOK == status)
    {
        status = CpswProxyServer_validateStartIdx(hServer->hEnet, hostId, rxFlowIdxBase);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "RX flow start index validation failed");

        if(ETHREMOTECFG_SOK == status)
        {
            status = EnetAppUtils_freeRxFlow(hServer->hEnet,
                                             coreKey,
                                             hostId,
                                             rxFlowIdxOffset);
        }

        if (ENET_SOK == status)
        {
            hClient->flowIdxBase   = 0U;
            hClient->flowIdxOffset = 0U;
        }
        else
        {
            ETHFWTRACE_ERR(status, "Failed to free Rx flow with base: %u,"
                           "offset: %u", rxFlowIdxBase, rxFlowIdxOffset);
        }
    }

    return status;
}

static int32_t CpswProxyServer_freeMacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                uint32_t hostId,
                                                uint8_t *macAddr)
{
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t coreKey;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        status = EnetAppUtils_freeMac(hServer->hEnet,
                                      coreKey,
                                      hostId,
                                      macAddr);

        if (ENET_SOK ==status)
        {
            memcpy(&hClient->macAddr[0U], 0U, ENET_MAC_ADDR_LEN);
        }
        else
        {
            ETHFWTRACE_ERR(status, "Failed to free the macAddr=%02x:%02x:%02x:%02x:%02x:%02x",
                           macAddr[0U], macAddr[1U], macAddr[2U],
                           macAddr[3U], macAddr[4U], macAddr[5U]);
        }
    }

    return status;
}

static int32_t CpswProxyServer_registerMacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                    uint32_t hostId,
                                                    uint8_t *macAddr,
                                                    uint32_t flowIdxBase,
                                                    uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    Enet_MacPort macPort;
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t coreKey;
    uint32_t relFlowIdx;
#if defined(ETHFW_VEPA_SUPPORT)
    struct eth_addr hwAddr;
#endif

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;
        /* Make sure that CpswProxyServer_registerMacHandlerCb is requested on primary flow */
        status = CpswProxyServer_getRelFlowIdx(hClient->virtPort, &relFlowIdx, flowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "Failed to get relative flow index for virtual port %u flow %u",
                           hClient->virtPort,
                           flowIdxOffset);
        if (ETHREMOTECFG_SOK == status)
        {
            if(relFlowIdx != CPSWPROXY_CLIENT_PRIMARY_REL_FLOW_IDX)
            {
                status = ETHREMOTECFG_ENOTSUPPORTED;
                ETHFWTRACE_ERR(status, "Failed to register MAC addr on extended flow, supported on primary flow only");
            }
        }
    }

    if (ETHREMOTECFG_SOK == status)
    {
        status = CpswProxyServer_validateStartIdx(hServer->hEnet, hostId, flowIdxBase);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "RX flow start index validation failed");

        if (ETHREMOTECFG_SOK == status)
        {
            isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
            if (isSwitchPort)
            {
                /* Setup MAC address based classifier to the requested RX flow */
                status = EnetAppUtils_regDstMacRxFlow(hServer->hEnet,
                                                        coreKey,
                                                        hostId,
                                                        flowIdxBase,
                                                        flowIdxOffset,
                                                        macAddr);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to setup MAC addr based route");

    #if defined(ETHFW_VEPA_SUPPORT)
                if ((hClient->vlanRefCnt == 0U) && (status == ENET_SOK))
                {
                    SMEMCPY(&hwAddr, macAddr, ETH_HWADDR_LEN);
                    status = EthFwVepa_registerClient(hServer->hEnet, hostId, flowIdxOffset,
                                                        hServer->dfltVlanIdSwitchPorts,
                                                        hClient->virtPort,
                                                        &hwAddr);
                    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                        "Failed to register client core %u "
                                        "macAddr %02x:%02x:%02x:%02x:%02x:%02x into VEPA table",
                                        hostId,
                                        macAddr[0], macAddr[1], macAddr[2],
                                        macAddr[3], macAddr[4], macAddr[5]);
                }
                if (status == ETHFW_SOK)
                {
                    hClient->vlanRefCnt++;
                }
    #endif
            }
            else
            {
                macPort = EthFwVirtPort_getMacPort(hClient->virtPort);

                /* Setup MAC port based classifier to the requested RX flow */
                status = CpswProxyServer_regMacPortFlow(hServer->hEnet,
                                                        coreKey,
                                                        hostId,
                                                        macPort,
                                                        macAddr,
                                                        flowIdxBase,
                                                        flowIdxOffset);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to setup MAC port based route");
            }
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to register macAddr=%02x:%02x:%02x:%02x:%02x:%02x",
                        macAddr[0U], macAddr[1U], macAddr[2U],
                        macAddr[3U], macAddr[4U], macAddr[5U]);
    }

    return status;
}

static int32_t CpswProxyServer_unregisterMacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                      uint32_t hostId,
                                                      uint8_t *macAddr,
                                                      uint32_t flowIdxBase,
                                                      uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    Enet_MacPort macPort;
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t coreKey;
    uint32_t relFlowIdx;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;
        /* Make sure that CpswProxyServer_unregisterMacHandlerCb is requested on primary flow */
        status = CpswProxyServer_getRelFlowIdx(hClient->virtPort, &relFlowIdx, flowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "Failed to get relative flow index for virtual port %u flow %u",
                           hClient->virtPort,
                           flowIdxOffset);
        if (ETHREMOTECFG_SOK == status)
        {
            if(relFlowIdx != CPSWPROXY_CLIENT_PRIMARY_REL_FLOW_IDX)
            {
                status = ETHREMOTECFG_ENOTSUPPORTED;
                ETHFWTRACE_ERR(status, "Failed to unregister MAC addr on extended flow, supported on primary flow only");
            }
        }
    }

    if (ETHREMOTECFG_SOK == status)
    {
        status = CpswProxyServer_validateStartIdx(hServer->hEnet, hostId, flowIdxBase);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "RX flow start index validation failed");

        if(ETHREMOTECFG_SOK == status)
        {
            isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
            if (isSwitchPort)
            {
                /* Teardown MAC address based classifier */
                status = EnetAppUtils_unregDstMacRxFlow(hServer->hEnet,
                                                        coreKey,
                                                        hostId,
                                                        flowIdxBase,
                                                        flowIdxOffset,
                                                        macAddr);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to teardown MAC addr based route");

    #if defined(ETHFW_VEPA_SUPPORT)
                if (status == ENET_SOK)
                {
                    hClient->vlanRefCnt--;
                }
                if ((hClient->vlanRefCnt == 0U) && (status == ENET_SOK))
                {
                    status = EthFwVepa_unregisterClient(hServer->hEnet, hostId, flowIdxOffset,
                                                        hServer->dfltVlanIdSwitchPorts,
                                                        hClient->virtPort);
                    ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                    "Failed to unregister client core %u "
                                    "macAddr %02x:%02x:%02x:%02x:%02x:%02x into VEPA table",
                                    hostId,
                                    macAddr[0], macAddr[1], macAddr[2],
                                    macAddr[3], macAddr[4], macAddr[5]);
                }
    #endif
            }
            else
            {
                macPort = EthFwVirtPort_getMacPort(hClient->virtPort);

                /* Teardown MAC port based classifier */
                status = CpswProxyServer_unregMacPortFlow(hServer->hEnet,
                                                        coreKey,
                                                        hostId,
                                                        macPort,
                                                        macAddr,
                                                        flowIdxBase,
                                                        flowIdxOffset);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to teardown MAC port based route");
            }
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to de-register macAddr=%02x:%02x:%02x:%02x:%02x:%02x",
                        macAddr[0U], macAddr[1U], macAddr[2U],
                        macAddr[3U], macAddr[4U], macAddr[5U]);
    }

    return status;
}

static int32_t CpswProxyServer_registerIPv4MacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                        uint32_t hostId,
                                                        uint8_t *macAddr,
                                                        uint8_t *ipAddr)
{
    CpswProxyServer_Obj *hServer = NULL;
#if (defined(FREERTOS) || defined(SAFERTOS)) && defined(ETHFW_PROXY_ARP_HANDLING)
    uint16_t vlanId = 0U;
    ip4_addr_t ip4Addr;
    struct eth_addr hwAddr;
#endif
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
        if (isSwitchPort)
        {
            /* Add IPv4:MAC address to ETHFW ARP table */
#if (defined(FREERTOS) || defined(SAFERTOS)) && defined(ETHFW_PROXY_ARP_HANDLING)
            IP4_ADDR(&ip4Addr, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
            SMEMCPY(&hwAddr, macAddr, ETH_HWADDR_LEN);

            status = EthFwArp_addAddr(&ip4Addr, &hwAddr, vlanId);
            if (status != ETHFW_SOK)
            {
                status = ETHREMOTECFG_EFAIL;
                ETHFWTRACE_ERR(status, "Failed to add ARP entry");
            }
            else
            {
                EthFwArp_printTable();
            }
#endif
        }
        else
        {
            /* ETHFW ARP table is supported only on virtual switch ports.
            * Virtual MAC ports don't needed proxy ARP because all traffic is already
            * forwarded to the remote client */
            status = ETHREMOTECFG_ENOTSUPPORTED;
            ETHFWTRACE_ERR(status, "IPv4:MAC registration is not supported on virtual MAC ports");
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to register ipAddr=%u.%u.%u.%u",
                       ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
    }

    return status;
}

static int32_t CpswProxyServer_deregisterIPv4MacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                          uint32_t hostId,
                                                          uint8_t *ipAddr)
{
    CpswProxyServer_Obj *hServer = NULL;
#if (defined(FREERTOS) || defined(SAFERTOS)) && defined(ETHFW_PROXY_ARP_HANDLING)
    ip4_addr_t ip4Addr;
    uint16_t vlanId = 0U;
#endif
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
        if (isSwitchPort)
        {
            /* Remove IP address from ETHFW proxy ARP table */
#if (defined(FREERTOS) || defined(SAFERTOS)) && defined(ETHFW_PROXY_ARP_HANDLING)
            IP4_ADDR(&ip4Addr, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);

            status = EthFwArp_delAddr(&ip4Addr, vlanId);
            if (status != ETHFW_SOK)
            {
                status = ETHREMOTECFG_EFAIL;
                ETHFWTRACE_ERR(status, "Failed to remove ARP entry");
            }
            else
            {
                EthFwArp_printTable();
            }
#endif
        }
        else
        {
            /* ETHFW ARP table is supported only on virtual switch ports */
            status = ETHREMOTECFG_ENOTSUPPORTED;
            ETHFWTRACE_ERR(status, "IPv4:MAC deregistration is not supported on virtual MAC ports");
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to de-register ipAddr=%u.%u.%u.%u",
                       ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
    }

    return status;
}

static int32_t CpswProxyServer_vlanJoinHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                 uint32_t coreId,
                                                 uint16_t vlanId,
                                                 const uint8_t *macAddr,
                                                 uint32_t flowIdxBase,
                                                 uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        status = EthFwVlan_join(hServer->hEnet,
                            hClient->virtPort,
                            vlanId,
                            macAddr,
                            flowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                          "Failed to join VLAN %u", vlanId);
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to join the vlan");
    }

    return status;
}

static int32_t CpswProxyServer_vlanLeaveHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                  uint32_t coreId,
                                                  uint16_t vlanId,
                                                  const uint8_t *macAddr,
                                                  uint32_t flowIdxBase,
                                                  uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        status = EthFwVlan_leave(hServer->hEnet,
                             hClient->virtPort,
                             vlanId,
                             macAddr,
                             flowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                      "Failed to leave VLAN %u", vlanId);
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to leave the vlan");
    }

    return status;
}

static int32_t CpswProxyServer_validateStartIdx(Enet_Handle hEnet,
                                                uint32_t hostId,
                                                uint32_t rxFlowStartId)
{
    uint32_t p0FlowIdOffset;
    int32_t status = ETHREMOTECFG_SOK;

    p0FlowIdOffset = EnetAppUtils_getStartFlowIdx(hEnet, hostId);

    if(rxFlowStartId != p0FlowIdOffset)
    {
        status = ETHREMOTECFG_EBADARGS;
        ETHFWTRACE_ERR(status, "Invalid rxStartIdx: %d", rxFlowStartId);
    }

    return status;
}

static int32_t CpswProxyServer_regMacPortFlow(Enet_Handle hEnet,
                                              uint32_t coreKey,
                                              uint32_t remoteCoreId,
                                              Enet_MacPort macPort,
                                              uint8_t *macAddr,
                                              uint32_t flowStartIdx,
                                              uint32_t flowIdx)
{
    CpswAle_SetUcastEntryInArgs ucastInArgs;
    CpswAle_SetPolicerEntryInPartitionInArgs polInArgs;
    CpswAle_SetPolicerEntryOutArgs polOutArgs;
    Enet_IoctlPrms prms;
    uint32_t entryIdx;
    int32_t status = ETHREMOTECFG_SOK;

    if (EnetUtils_isMcastAddr(macAddr))
    {
        status = ETHREMOTECFG_ENOTSUPPORTED;
        ETHFWTRACE_ERR(status, "Port %u: mcast not supported", ENET_MACPORT_ID(macPort));
    }

    /* Add unicast address */
    if (status == ETHREMOTECFG_SOK)
    {
        ucastInArgs.addr.vlanId  = 0U;
        ucastInArgs.info.portNum = CPSW_ALE_HOST_PORT_NUM;
        ucastInArgs.info.blocked = BFALSE;
        ucastInArgs.info.secure  = BTRUE;
        ucastInArgs.info.super   = BFALSE;
        ucastInArgs.info.ageable = BFALSE;
        ucastInArgs.info.trunk   = BFALSE;
        EnetUtils_copyMacAddr(&ucastInArgs.addr.addr[0U], macAddr);

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &ucastInArgs, &entryIdx);

        ENET_IOCTL(hEnet, remoteCoreId, CPSW_ALE_IOCTL_ADD_UCAST, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Port %u: failed to add ucast entry", ENET_MACPORT_ID(macPort));
    }

    /* Setup policer with "port" as match criteria */
    if (status == ENET_SOK)
    {
        memset(&polInArgs, 0, sizeof(polInArgs));

        polInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_PORT;
        polInArgs.policerMatch.portNum   = CPSW_ALE_MACPORT_TO_ALEPORT(macPort);
        polInArgs.threadIdEn             = BTRUE;
        polInArgs.threadId               = flowIdx;
        polInArgs.peakRateInBitsPerSec   = 0;
        polInArgs.commitRateInBitsPerSec = 0;
        polInArgs.policerPartLevel       = CPSW_ALE_POLICER_PARTITION_LEVEL_3;
        ENET_IOCTL_SET_INOUT_ARGS(&prms, &polInArgs, &polOutArgs);

        ENET_IOCTL(hEnet, remoteCoreId, CPSW_ALE_IOCTL_SET_POLICER_IN_PARTITION, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to set port %u policer", ENET_MACPORT_ID(macPort));
    }

    return status;
}

static int32_t CpswProxyServer_unregMacPortFlow(Enet_Handle hEnet,
                                                uint32_t coreKey,
                                                uint32_t remoteCoreId,
                                                Enet_MacPort macPort,
                                                uint8_t *macAddr,
                                                uint32_t flowStartIdx,
                                                uint32_t flowIdx)
{
    CpswAle_DelPolicerEntryInArgs delPolInArgs;
    CpswAle_PolicerMatchParams polMatch;
    CpswAle_PolicerEntryOutArgs polOutArgs;
    CpswAle_MacAddrInfo macAddrInfo;
    Enet_IoctlPrms prms;
    int32_t status = ETHREMOTECFG_SOK;

    if (EnetUtils_isMcastAddr(macAddr))
    {
        status = ETHREMOTECFG_ENOTSUPPORTED;
        ETHFWTRACE_ERR(status, "Port %u: mcast not supported", ENET_MACPORT_ID(macPort));
    }

    /* Remove policer with "port" match criteria */
    if (status == ETHREMOTECFG_SOK)
    {
        memset(&polMatch, 0, sizeof(polMatch));
        polMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_PORT;
        polMatch.portNum = CPSW_ALE_MACPORT_TO_ALEPORT(macPort);

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &polMatch, &polOutArgs);

        ENET_IOCTL(hEnet, remoteCoreId, CPSW_ALE_IOCTL_GET_POLICER, &prms, status);
        if (status == ENET_SOK)
        {
            if ((polOutArgs.threadIdEn == BTRUE) &&
                (polOutArgs.threadId == flowIdx))
            {
                status = ENET_SOK;
            }
            else
            {
                status = ENET_EINVALIDPARAMS;
                ETHFWTRACE_ERR(status, "Invalid policer thread cfg (threadIdEn=%u threadId=%u)",
                               polOutArgs.threadIdEn, polOutArgs.threadId);
            }
        }

        if (status == ENET_SOK)
        {
            memset(&delPolInArgs, 0, sizeof(delPolInArgs));
            delPolInArgs.policerMatch = polMatch;

            ENET_IOCTL_SET_IN_ARGS(&prms, &delPolInArgs);

            ENET_IOCTL(hEnet, remoteCoreId, CPSW_ALE_IOCTL_DEL_POLICER, &prms, status);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                              "Invalid port %u policer flow %d", ENET_MACPORT_ID(macPort), flowIdx);
        }
    }

    /* Remove unicast address */
    if (status == ETHREMOTECFG_SOK)
    {
        macAddrInfo.vlanId = 0U;
        EnetUtils_copyMacAddr(&macAddrInfo.addr[0U], macAddr);

        ENET_IOCTL_SET_IN_ARGS(&prms, &macAddrInfo);

        ENET_IOCTL(hEnet, remoteCoreId, CPSW_ALE_IOCTL_REMOVE_ADDR, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Port %u: failed to remove ucast entry", ENET_MACPORT_ID(macPort));
    }

    return status;
}

static int32_t CpswProxyServer_registerRxDefaultHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                          uint32_t hostId,
                                                          uint32_t flowIdxBase,
                                                          uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t coreKey;
    uint32_t relFlowIdx;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;
        /* Make sure that CpswProxyServer_registerRxDefaultHandlerCb is requested on primary flow */
        status = CpswProxyServer_getRelFlowIdx(hClient->virtPort, &relFlowIdx, flowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "Failed to get relative flow index for virtual port %u flow %u",
                           hClient->virtPort,
                           flowIdxOffset);
        if (ETHREMOTECFG_SOK == status)
        {
            if(relFlowIdx != CPSWPROXY_CLIENT_PRIMARY_REL_FLOW_IDX)
            {
                status = ETHREMOTECFG_ENOTSUPPORTED;
                ETHFWTRACE_ERR(status, "Failed to register rx default flow on extended flow, supported on primary flow only");
            }
        }
    }

    if (ETHREMOTECFG_SOK == status)
    {
        status = CpswProxyServer_validateStartIdx(hServer->hEnet, hostId, flowIdxBase);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "RX flow start index validation failed");

        if (ETHREMOTECFG_SOK == status)
        {
            isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
            if (isSwitchPort)
            {
                status = EnetAppUtils_regDfltRxFlow(hServer->hEnet,
                                                    coreKey,
                                                    hostId,
                                                    flowIdxBase,
                                                    flowIdxOffset);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to register default flow");
            }
            else
            {
                /* RX default flow is supported only on virtual switch ports */
                status = ETHREMOTECFG_ENOTSUPPORTED;
                ETHFWTRACE_ERR(status, "Default flow setting is not supported on virtual MAC ports");
            }
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to register default flow for Rx flowId"
                       "base:%u, offset:%u", flowIdxBase, flowIdxOffset);
    }

    return status;
}

static int32_t CpswProxyServer_deregisterRxDefaultHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                            uint32_t hostId,
                                                            uint32_t flowIdxBase,
                                                            uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t coreKey;
    uint32_t relFlowIdx;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;
        /* Make sure that CpswProxyServer_deregisterRxDefaultHandlerCb is requested on primary flow */
        status = CpswProxyServer_getRelFlowIdx(hClient->virtPort, &relFlowIdx, flowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "Failed to get relative flow index for virtual port %u flow %u",
                           hClient->virtPort,
                           flowIdxOffset);
        if (ETHREMOTECFG_SOK == status)
        {
            if(relFlowIdx != CPSWPROXY_CLIENT_PRIMARY_REL_FLOW_IDX)
            {
                status = ETHREMOTECFG_ENOTSUPPORTED;
                ETHFWTRACE_ERR(status, "Failed to deregister rx default flow on extended flow, supported on primary flow only");
            }
        }
    }

    if (ETHREMOTECFG_SOK == status)
    {
        status = CpswProxyServer_validateStartIdx(hServer->hEnet, hostId, flowIdxBase);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "RX flow start index validation failed");

        if (ETHREMOTECFG_SOK == status)
        {
            isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
            if (isSwitchPort)
            {
                status = EnetAppUtils_unregDfltRxFlow(hServer->hEnet,
                                                      coreKey,
                                                      hostId,
                                                      flowIdxBase,
                                                      flowIdxOffset);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to register default flow");
            }
            else
            {
                /* RX default flow is supported only on virtual switch ports */
                status = ETHREMOTECFG_ENOTSUPPORTED;
                ETHFWTRACE_ERR(status, "Default flow setting is not supported on virtual MAC ports");
            }
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to de-register default flow for Rx flowId"
                       "base:%u, offset:%u", flowIdxBase, flowIdxOffset);
    }

    return status;
}

static void CpswProxyServer_printHostPortStats(CpswStats_HostPort_Ng *st)
{
    uint_fast32_t i;

#if !defined(MCU_PLUS_SDK)
    CPSWPROXY_PRINT_STATS_NONZERO("  rxGoodFrames            = %llu", st->rxGoodFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxBcastFrames           = %llu", st->rxBcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxMcastFrames           = %llu", st->rxMcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxCrcErrors             = %llu", st->rxCrcErrors);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxOversizedFrames       = %llu", st->rxOversizedFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxUndersizedFrames      = %llu", st->rxUndersizedFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxFragments             = %llu", st->rxFragments);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleDrop                 = %llu", st->aleDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleOverrunDrop          = %llu", st->aleOverrunDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxOctets                = %llu", st->rxOctets);
    CPSWPROXY_PRINT_STATS_NONZERO("  txGoodFrames            = %llu", st->txGoodFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txBcastFrames           = %llu", st->txBcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txMcastFrames           = %llu", st->txMcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txOctets                = %llu", st->txOctets);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames64          = %llu", st->octetsFrames64);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames65to127     = %llu", st->octetsFrames65to127);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames128to255    = %llu", st->octetsFrames128to255);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames256to511    = %llu", st->octetsFrames256to511);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames512to1023   = %llu", st->octetsFrames512to1023);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames1024        = %llu", st->octetsFrames1024);
    CPSWPROXY_PRINT_STATS_NONZERO("  netOctets               = %llu", st->netOctets);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxBottomOfFifoDrop      = %llu", st->rxBottomOfFifoDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  portMaskDrop            = %llu", st->portMaskDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxTopOfFifoDrop         = %llu", st->rxTopOfFifoDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleRateLimitDrop        = %llu", st->aleRateLimitDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleVidIngressDrop       = %llu", st->aleVidIngressDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleDAEqSADrop           = %llu", st->aleDAEqSADrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleBlockDrop            = %llu", st->aleBlockDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleSecureDrop           = %llu", st->aleSecureDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleAuthDrop             = %llu", st->aleAuthDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownUcast         = %llu", st->aleUnknownUcast);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownUcastBcnt     = %llu", st->aleUnknownUcastBcnt);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownMcast         = %llu", st->aleUnknownMcast);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownMcastBcnt     = %llu", st->aleUnknownMcastBcnt);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownBcast         = %llu", st->aleUnknownBcast);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownBcastBcnt     = %llu", st->aleUnknownBcastBcnt);
    CPSWPROXY_PRINT_STATS_NONZERO("  alePolicyMatch          = %llu", st->alePolicyMatch);
    CPSWPROXY_PRINT_STATS_NONZERO("  alePolicyMatchRed       = %llu", st->alePolicyMatchRed);
    CPSWPROXY_PRINT_STATS_NONZERO("  alePolicyMatchYellow    = %llu", st->alePolicyMatchYellow);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleMultSADrop           = %llu", st->aleMultSADrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleDualVlanDrop         = %llu", st->aleDualVlanDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleLenErrorDrop         = %llu", st->aleLenErrorDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleIpNextHdrDrop        = %llu", st->aleIpNextHdrDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleIPv4FragDrop         = %llu", st->aleIPv4FragDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxAssemblyErr        = %llu", st->ietRxAssemblyErr);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxAssemblyOk         = %llu", st->ietRxAssemblyOk);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxSmdError           = %llu", st->ietRxSmdError);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxFrag               = %llu", st->ietRxFrag);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietTxHold               = %llu", st->ietTxHold);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietTxFrag               = %llu", st->ietTxFrag);
    CPSWPROXY_PRINT_STATS_NONZERO("  txMemProtectError       = %llu", st->txMemProtectError);
#endif

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPri); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPri[%u]                = %llu", i, st->txPri[i]);
    }

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPriBcnt); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPriBcnt[%u]            = %llu", i, st->txPriBcnt[i]);
    }

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPriDrop); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPriDrop[%u]            = %llu", i, st->txPriDrop[i]);
    }

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPriDropBcnt); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPriDropBcnt[%u]        = %llu", i, st->txPriDropBcnt[i]);
    }
}

static void CpswProxyServer_printMacPortStats(CpswStats_MacPort_Ng *st)
{
    uint_fast32_t i;

    CPSWPROXY_PRINT_STATS_NONZERO("  rxGoodFrames            = %llu", st->rxGoodFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxBcastFrames           = %llu", st->rxBcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxMcastFrames           = %llu", st->rxMcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxPauseFrames           = %llu", st->rxPauseFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxCrcErrors             = %llu", st->rxCrcErrors);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxAlignCodeErrors       = %llu", st->rxAlignCodeErrors);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxOversizedFrames       = %llu", st->rxOversizedFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxJabberFrames          = %llu", st->rxJabberFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxUndersizedFrames      = %llu", st->rxUndersizedFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxFragments             = %llu", st->rxFragments);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleDrop                 = %llu", st->aleDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleOverrunDrop          = %llu", st->aleOverrunDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxOctets                = %llu", st->rxOctets);
    CPSWPROXY_PRINT_STATS_NONZERO("  txGoodFrames            = %llu", st->txGoodFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txBcastFrames           = %llu", st->txBcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txMcastFrames           = %llu", st->txMcastFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txPauseFrames           = %llu", st->txPauseFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txDeferredFrames        = %llu", st->txDeferredFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txCollisionFrames       = %llu", st->txCollisionFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txSingleCollFrames      = %llu", st->txSingleCollFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txMultipleCollFrames    = %llu", st->txMultipleCollFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txExcessiveCollFrames   = %llu", st->txExcessiveCollFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  txLateCollFrames        = %llu", st->txLateCollFrames);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxIPGError              = %llu", st->rxIPGError);
    CPSWPROXY_PRINT_STATS_NONZERO("  txCarrierSenseErrors    = %llu", st->txCarrierSenseErrors);
    CPSWPROXY_PRINT_STATS_NONZERO("  txOctets                = %llu", st->txOctets);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames64          = %llu", st->octetsFrames64);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames65to127     = %llu", st->octetsFrames65to127);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames128to255    = %llu", st->octetsFrames128to255);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames256to511    = %llu", st->octetsFrames256to511);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames512to1023   = %llu", st->octetsFrames512to1023);
    CPSWPROXY_PRINT_STATS_NONZERO("  octetsFrames1024        = %llu", st->octetsFrames1024);
    CPSWPROXY_PRINT_STATS_NONZERO("  netOctets               = %llu", st->netOctets);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxBottomOfFifoDrop      = %llu", st->rxBottomOfFifoDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  portMaskDrop            = %llu", st->portMaskDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  rxTopOfFifoDrop         = %llu", st->rxTopOfFifoDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleRateLimitDrop        = %llu", st->aleRateLimitDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleVidIngressDrop       = %llu", st->aleVidIngressDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleDAEqSADrop           = %llu", st->aleDAEqSADrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleBlockDrop            = %llu", st->aleBlockDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleSecureDrop           = %llu", st->aleSecureDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleAuthDrop             = %llu", st->aleAuthDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownUcast         = %llu", st->aleUnknownUcast);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownUcastBcnt     = %llu", st->aleUnknownUcastBcnt);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownMcast         = %llu", st->aleUnknownMcast);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownMcastBcnt     = %llu", st->aleUnknownMcastBcnt);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownBcast         = %llu", st->aleUnknownBcast);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleUnknownBcastBcnt     = %llu", st->aleUnknownBcastBcnt);
    CPSWPROXY_PRINT_STATS_NONZERO("  alePolicyMatch          = %llu", st->alePolicyMatch);
    CPSWPROXY_PRINT_STATS_NONZERO("  alePolicyMatchRed       = %llu", st->alePolicyMatchRed);
    CPSWPROXY_PRINT_STATS_NONZERO("  alePolicyMatchYellow    = %llu", st->alePolicyMatchYellow);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleMultSADrop           = %llu", st->aleMultSADrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleDualVlanDrop         = %llu", st->aleDualVlanDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleLenErrorDrop         = %llu", st->aleLenErrorDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleIpNextHdrDrop        = %llu", st->aleIpNextHdrDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  aleIPv4FragDrop         = %llu", st->aleIPv4FragDrop);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxAssemblyErr        = %llu", st->ietRxAssemblyErr);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxAssemblyOk         = %llu", st->ietRxAssemblyOk);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxSmdError           = %llu", st->ietRxSmdError);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietRxFrag               = %llu", st->ietRxFrag);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietTxHold               = %llu", st->ietTxHold);
    CPSWPROXY_PRINT_STATS_NONZERO("  ietTxFrag               = %llu", st->ietTxFrag);
    CPSWPROXY_PRINT_STATS_NONZERO("  txMemProtectError       = %llu", st->txMemProtectError);

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPri); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPri[%u]                = %llu", i, st->txPri[i]);
    }

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPriBcnt); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPriBcnt[%u]            = %llu", i, st->txPriBcnt[i]);
    }

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPriDrop); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPriDrop[%u]            = %llu", i, st->txPriDrop[i]);
    }

    for (i = 0U; i < ENET_ARRAYSIZE(st->txPriDropBcnt); i++)
    {
        CPSWPROXY_PRINT_STATS_IDX_NONZERO("  txPriDropBcnt[%u]        = %llu", i, st->txPriDropBcnt[i]);
    }
}

static void CpswProxyServer_printStats(Enet_Handle hEnet,
                                       Enet_Type enetType,
                                       uint32_t instId,
                                       uint32_t coreId)
{
    Enet_IoctlPrms prms;
    Enet_MacPort portNum;
    CpswStats_PortStats portStats;
    int32_t status = ENET_SOK;
    uint32_t i;

    ENET_IOCTL_SET_OUT_ARGS(&prms, &portStats);
    ENET_IOCTL(hEnet, coreId, ENET_STATS_IOCTL_GET_HOSTPORT_STATS, &prms, status);
    if (status == ENET_SOK)
    {
        ETHFWTRACE_INFO("");
        ETHFWTRACE_INFO(" Host Port Statistics");
        ETHFWTRACE_INFO("-----------------------------------------");
        switch (enetType)
        {
            case ENET_CPSW_2G:
            {
                CpswStats_HostPort_2g *st;

                st = (CpswStats_HostPort_2g *)&portStats;
                EnetAppUtils_printHostPortStats2G(st);
                break;
            }

            case ENET_CPSW_3G:
            case ENET_CPSW_5G:
            case ENET_CPSW_9G:
            {
                CpswStats_HostPort_Ng *st;

                st = (CpswStats_HostPort_Ng *)&portStats;
                CpswProxyServer_printHostPortStats(st);
                break;
            }

            default:
            {
                status = ETHREMOTECFG_EINVALIDPARAMS;
                ETHFWTRACE_ERR(status, "Invalid enetType: %d", enetType);
                break;
            }
        }

        ETHFWTRACE_INFO("");
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to get host stats");
    }

    if (status == ENET_SOK)
    {
        for (i = 0, portNum = ENET_MAC_PORT_FIRST; i < Enet_getMacPortMax(enetType, instId); i++, portNum++)
        {
            ENET_IOCTL_SET_INOUT_ARGS(&prms, &portNum, &portStats);
            ENET_IOCTL(hEnet, coreId, ENET_STATS_IOCTL_GET_MACPORT_STATS, &prms, status);
            if (status == ENET_SOK)
            {
                ETHFWTRACE_INFO("");
                ETHFWTRACE_INFO(" External Port %d Statistics", ENET_MACPORT_ID(portNum));
                ETHFWTRACE_INFO("-----------------------------------------");
                switch (enetType)
                {
                    case ENET_CPSW_2G:
                    {
                        CpswStats_MacPort_2g *st;

                        st = (CpswStats_MacPort_2g *)&portStats;
                        EnetAppUtils_printMacPortStats2G(st);
                        break;
                    }

                    case ENET_CPSW_3G:
                    case ENET_CPSW_5G:
                    case ENET_CPSW_9G:
                    {
                        CpswStats_MacPort_Ng *st;

                        st = (CpswStats_MacPort_Ng *)&portStats;
                        CpswProxyServer_printMacPortStats(st);
                        break;
                    }

                    default:
                    {
                        status = ETHREMOTECFG_EINVALIDPARAMS;
                        ETHFWTRACE_ERR(status, "Invalid enetType: %d", enetType);
                        break;
                    }
                }

                ETHFWTRACE_INFO("");
            }
            else
            {
                ETHFWTRACE_ERR(status, "Failed to get MAC %u stats", ENET_MACPORT_ID(portNum));
            }
        }
    }
}

static int32_t CpswProxyServer_isLinkUpCb(CpswProxyServer_ClientHandle hClient,
                                          uint32_t hostId,
                                          bool *isLinked,
                                          uint32_t *speed,
                                          uint32_t *duplex)
{
    CpswProxyServer_Obj *hServer = NULL;
    Enet_IoctlPrms prms;
    EnetPhy_GenericInArgs phyInArgs;
    EnetMacPort_LinkCfg phyOutArgs;
    bool isMacPort;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        *speed  = ENET_SPEED_10MBIT;
        *duplex = ENET_DUPLEX_HALF;

        isMacPort = EthFwVirtPort_isMacPort(hClient->virtPort);
        if (isMacPort)
        {
            phyInArgs.macPort = EthFwVirtPort_getMacPort(hClient->virtPort);

            ENET_IOCTL_SET_INOUT_ARGS(&prms, &phyInArgs, isLinked);

            memset(&phyOutArgs, 0, sizeof(phyOutArgs));

            ENET_IOCTL(hServer->hEnet, hostId, ENET_PHY_IOCTL_IS_LINKED, &prms, status);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                            "Failed to get port %u link status", ENET_MACPORT_ID(phyInArgs.macPort));

            //FIXME: link speed/duplex IOCTL has to be PHY agnostics (i.e. MAC-to-MAC mode).
            if (*isLinked)
            {
                ENET_IOCTL_SET_INOUT_ARGS(&prms, &phyInArgs, &phyOutArgs);

                ENET_IOCTL(hServer->hEnet, hostId, ENET_PHY_IOCTL_GET_LINK_MODE, &prms, status);
                ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                                "Failed to get port %u link params", ENET_MACPORT_ID(phyInArgs.macPort));

                if (status == ENET_SOK)
                {
                    *speed  = phyOutArgs.speed;
                    *duplex = phyOutArgs.duplexity;
                }
            }
        }
        else
        {
            *isLinked = BTRUE;
            *speed    = ENET_SPEED_1GBIT;
            *duplex   = ENET_DUPLEX_FULL;
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to get the Link status");
    }

    return status;
}

static int32_t CpswProxyServer_registerEthertypeHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                          uint32_t hostId,
                                                          uint16_t etherType,
                                                          uint32_t flowIdxBase,
                                                          uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    Enet_IoctlPrms prms;
    CpswAle_SetPolicerEntryInArgs setPolicerInArgs;
    CpswAle_SetPolicerEntryOutArgs setPolicerOutArgs;
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
        if (isSwitchPort)
        {
            memset(&setPolicerInArgs, 0, sizeof(setPolicerInArgs));
            setPolicerInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;
            setPolicerInArgs.policerMatch.etherType = etherType;
            setPolicerInArgs.threadIdEn = BTRUE;
            setPolicerInArgs.threadId   = flowIdxOffset;
            setPolicerInArgs.peakRateInBitsPerSec   = 0U;
            setPolicerInArgs.commitRateInBitsPerSec = 0U;

            ENET_IOCTL_SET_INOUT_ARGS(&prms, &setPolicerInArgs, &setPolicerOutArgs);

            ENET_IOCTL(hServer->hEnet, hostId, CPSW_ALE_IOCTL_SET_POLICER, &prms, status);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                            "Failed to setup EtherType based route");
        }
        else
        {
            status = ETHREMOTECFG_ENOTSUPPORTED;
            ETHFWTRACE_ERR(status, "EtherType route is not supported on virtual MAC ports");
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to register Ethertype: %u",etherType);
    }

    return status;
}

static int32_t CpswProxyServer_deregisterEthertypeHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                            uint32_t hostId,
                                                            uint16_t etherType)
{
    CpswProxyServer_Obj *hServer = NULL;
    Enet_IoctlPrms prms;
    CpswAle_DelPolicerEntryInArgs delPolicerInArgs;
    bool isSwitchPort;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        isSwitchPort = EthFwVirtPort_isSwitchPort(hClient->virtPort);
        if (!isSwitchPort)
        {
            memset(&delPolicerInArgs, 0, sizeof(delPolicerInArgs));
            delPolicerInArgs.policerMatch.policerMatchEnMask = CPSW_ALE_POLICER_MATCH_ETHERTYPE;
            delPolicerInArgs.policerMatch.etherType = etherType;
            delPolicerInArgs.aleEntryMask = CPSW_ALE_POLICER_TABLEENTRY_DELETE_ETHERTYPE;

            ENET_IOCTL_SET_IN_ARGS(&prms, &delPolicerInArgs);

            ENET_IOCTL(hServer->hEnet, hostId, CPSW_ALE_IOCTL_DEL_POLICER, &prms, status);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                            "Failed to teardown EtherType based route");
        }
        else
        {
            status = ETHREMOTECFG_ENOTSUPPORTED;
            ETHFWTRACE_ERR(status, "EtherType route is not supported on virtual MAC ports");
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to de-register Ethertype: %u",etherType);
    }

    return status;
}

static int32_t CpswProxyServer_registerRemoteTimerHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                            uint32_t hostId,
                                                            uint8_t timerId,
                                                            uint8_t hwPushNum)
{
    Enet_IoctlPrms prms;
    CpswCpts_RegisterHwPushCbInArgs hwPushCbInArgs;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t hwPushNorm = CPSW_CPTS_HWPUSH_NORM((CpswCpts_HwPush)hwPushNum);
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");
    CPSWPROXY_ERR_CHECK((hClient != NULL), status, ETHREMOTECFG_EFAIL);

    if (hwPushNum > CPSW_CPTS_HWPUSH_COUNT_MAX)
    {
        status = ENET_EBADARGS;
        ETHFWTRACE_ERR(status, "Invalid HW push num %u", hwPushNum);
    }

    if (ETHREMOTECFG_SOK == status)
    {
        hServer->notifyServiceObj.dstProcMask |= ENET_BIT(hostId);

        /* Register hardware push callback */
        if (status == ETHREMOTECFG_SOK)
        {
            hwPushCbInArgs.hwPushNum = (CpswCpts_HwPush)hwPushNum;
            hwPushCbInArgs.hwPushNotifyCb = CpswProxyServer_hwPushNotifyFxn;
            hwPushCbInArgs.hwPushNotifyCbArg = (void *)hServer;

            ENET_IOCTL_SET_IN_ARGS(&prms, &hwPushCbInArgs);

            ENET_IOCTL(hServer->hEnet, hostId, CPSW_CPTS_IOCTL_REGISTER_HWPUSH_CALLBACK, &prms, status);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                            "Failed to register CPTS HW Push callback");
        }

        /* Configure timesync router */
        if (status == ENET_SOK)
        {
            status = EnetAppUtils_setTimeSyncRouter(hServer->enetType,
                                                    hServer->instId,
                                                    timerId,
                                                    hwPushNorm + CPSWPROXY_CPSW9G_HWPUSH_BASE);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to set TSR");
        }

        if (status == ENET_SOK)
        {
            hServer->notifyServiceObj.hwPush2CoreIdMap[hwPushNorm] = hostId;
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to register remote timer");
    }

    return status;
}

static int32_t CpswProxyServer_unregisterRemoteTimerHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                              uint32_t hostId,
                                                              uint8_t hwPushNum)
{
    int32_t status = ETHREMOTECFG_SOK;
    Enet_IoctlPrms prms;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t hwPushNorm = CPSW_CPTS_HWPUSH_NORM((CpswCpts_HwPush)hwPushNum);

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");
    CPSWPROXY_ERR_CHECK((hClient != NULL), status, ETHREMOTECFG_EFAIL);

    if (hwPushNum >= CPSW_CPTS_HWPUSH_COUNT_MAX)
    {
        status = ENET_EBADARGS;
        ETHFWTRACE_ERR(status, "Invalid HW push num %u", hwPushNum);
    }

    if (ETHREMOTECFG_SOK == status)
    {
        hServer->notifyServiceObj.dstProcMask &= ~ENET_BIT(hostId);

        /* Unregister hardware push callback */
        if (status == ENET_SOK)
        {
            hwPushNum = (CpswCpts_HwPush)hwPushNum;
            ENET_IOCTL_SET_IN_ARGS(&prms, &hwPushNum);
            ENET_IOCTL(hServer->hEnet, hostId, CPSW_CPTS_IOCTL_UNREGISTER_HWPUSH_CALLBACK, &prms, status);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                            "Failed to deregister CPTS HW push callback");
        }

        /* Clear timesync router configuration for hardware push,
        * Note: This assumes input signal is stopped */
        if (status == ENET_SOK)
        {
            status = EnetAppUtils_setTimeSyncRouter(hServer->enetType,
                                                    hServer->instId,
                                                    0U,
                                                    hwPushNorm + CPSWPROXY_CPSW9G_HWPUSH_BASE);
            ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to set TSR");
        }

        if (status == ENET_SOK)
        {
            /* Use IPC_MAX_PROCS as invalid core id */
            hServer->notifyServiceObj.hwPush2CoreIdMap[hwPushNorm] = IPC_MAX_PROCS;
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, " Failed to de-register remote timer");
    }

    return status;
}

static int32_t CpswProxyServer_filterAddMacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                     uint32_t hostId,
                                                     uint8_t *macAddr,
                                                     uint16_t vlanId,
                                                     uint32_t flowIdxBase,
                                                     uint32_t flowIdxOffset)
{
    CpswProxyServer_Obj *hServer = NULL;
    uint16_t hwVlanId = vlanId;
    int32_t status = ETHREMOTECFG_SOK;
    uint32_t relFlowIdx;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (status == ETHREMOTECFG_SOK)
    {
        /* Make sure that CpswProxyServer_filterAddMacHandlerCb is requested on primary flow */
        status = CpswProxyServer_getRelFlowIdx(hClient->virtPort, &relFlowIdx, flowIdxOffset);
        ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status,
                           "Failed to get relative flow index for virtual port %u flow %u",
                           hClient->virtPort,
                           flowIdxOffset);
        if (ETHREMOTECFG_SOK == status)
        {
            if(relFlowIdx != CPSWPROXY_CLIENT_PRIMARY_REL_FLOW_IDX)
            {
                status = ETHREMOTECFG_ENOTSUPPORTED;
                ETHFWTRACE_ERR(status, "Failed to add filter MAC on extended flow, supported on primary flow only");
            }
        }
    }

    if (!EnetUtils_isMcastAddr(macAddr))
    {
        status = ETHREMOTECFG_EBADARGS;
        ETHFWTRACE_ERR(status, "Addr is not multicast, cannot add/delete to filter");
    }

    /* Check if client is part of the VLAN */
    if (status == ETHREMOTECFG_SOK)
    {
        /* If client is not in a VLAN, it will pass VLAN id 0 or VLAN_USE_DFLT,
         * but we need to use the actual default VLAN id */
        if ((vlanId == 0U) || (vlanId == ETHREMOTECFG_ETHSWITCH_VLAN_USE_DFLT))
        {
            vlanId = 0U;
            hwVlanId = hServer->dfltVlanIdSwitchPorts;
        }
        else
        {
            /* Check if the client has joined the VLAN */
            if (!EthFwVlan_isInVlan(hClient->virtPort, vlanId))
            {
                status = ETHREMOTECFG_EBADARGS;
                ETHFWTRACE_ERR(status, "Virtual port %u is not part of VLAN %u", hClient->virtPort, vlanId);
            }
        }
    }

    /* Add multicast (shared or exclusive), reject reserved ones */
    if (status == ETHREMOTECFG_SOK)
    {
        status = EthFwMcast_filterAddMac(hClient->virtPort, hServer->hEnet,
                                         macAddr, vlanId, hwVlanId, flowIdxOffset, hostId);
        if (status != ETHFW_SOK)
        {
            ETHFWTRACE_ERR(status, "Failed to add multicast");
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to add MacAddr:%02x:%02x:%02x:%02x:%02x:%02x to"
                       "the multicast filter", macAddr[0U], macAddr[1U], macAddr[2U],
                       macAddr[3U], macAddr[4U], macAddr[5U]);
    }

    return status;
}

static int32_t CpswProxyServer_filterDelMacHandlerCb(CpswProxyServer_ClientHandle hClient,
                                                     uint32_t hostId,
                                                     uint8_t *macAddr,
                                                     uint16_t vlanId)
{
    CpswProxyServer_Obj *hServer = NULL;
    uint16_t hwVlanId = vlanId;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (!EnetUtils_isMcastAddr(macAddr))
    {
        status = ETHREMOTECFG_EBADARGS;
        ETHFWTRACE_ERR(status, "Addr is not multicast, cannot add/delete to filter");
    }

    /* Check if client is part of the VLAN */
    if (status == ETHREMOTECFG_SOK)
    {
        /* If client is not in a VLAN, it will pass VLAN id 0 or VLAN_USE_DFLT,
         * but we need to use the actual default VLAN id */
        if ((vlanId == 0U) || (vlanId == ETHREMOTECFG_ETHSWITCH_VLAN_USE_DFLT))
        {
            vlanId = 0U;
            hwVlanId = hServer->dfltVlanIdSwitchPorts;
        }
        else
        {
            /* Check if the client has joined the VLAN */
            if (!EthFwVlan_isInVlan(hClient->virtPort, vlanId))
            {
                status = ETHREMOTECFG_EBADARGS;
                ETHFWTRACE_ERR(status, "Virtual port %u is not part of VLAN %u",
                               hClient->virtPort, vlanId);
            }
        }
    }

    /* Delete multicast (shared or exclusive), reject reserved ones */
    if (status == ETHREMOTECFG_SOK)
    {
        status = EthFwMcast_filterDelMac(hClient->virtPort, hServer->hEnet,
                                         macAddr, vlanId, hwVlanId, hostId);
        if (status != ETHFW_SOK)
        {
            ETHFWTRACE_ERR(status, "Failed to remove multicast");
        }
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to delete MacAddr:%02x:%02x:%02x:%02x:%02x:%02x to"
                       "the multicast filter", macAddr[0U], macAddr[1U], macAddr[2U],
                       macAddr[3U], macAddr[4U], macAddr[5U]);
    }

    return status;
}

int32_t CpswProxyServer_init(CpswProxyServer_Config_t *cfg)
{
    CpswProxyServer_Obj *hServer;
    EnetMcm_CmdIf *hMcmCmdIf;
    Enet_IoctlPrms prms;
    EnetMcm_HandleInfo handleInfo;
    bool csumEnable;
    int32_t i;
    int32_t status = ETHFW_SOK;

    CpswProxyServer_compileTimeChecks();

    hServer = CpswProxyServer_initHandle();
    EnetAppUtils_assert((hServer != NULL) && (hServer->initDone == BFALSE));

    hServer->enetType = cfg->enetType;
    hServer->instId = cfg->instId;
    hServer->masterCoreId = EnetSoc_getCoreId();
    memset(&hServer->coreObj, 0, sizeof(hServer->coreObj));

    hServer->dfltVlanIdMacOnlyPorts = cfg->dfltVlanIdMacOnlyPorts;
    hServer->dfltVlanIdSwitchPorts  = cfg->dfltVlanIdSwitchPorts;

    hServer->alePortMask = cfg->enabledPortMask;
    hServer->aleMacOnlyPortMask = cfg->macOnlyPortMask;

    hServer->getMcmCmdIfCb = cfg->getMcmCmdIfCb;
    hServer->initEthfwDeviceDataCb = cfg->initEthfwDeviceDataCb;

    if ((hServer->aleMacOnlyPortMask & hServer->alePortMask) !=
        hServer->aleMacOnlyPortMask)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "MAC ports required for virtual MAC ports are not enabled");
        EnetAppUtils_assert(status == ENET_SOK);
    }

    hServer->aleSwitchOnlyPortMask = (hServer->alePortMask &
                                      ~hServer->aleMacOnlyPortMask);

    /* Get MCM cmd handle */
    EnetAppUtils_assert(hServer->getMcmCmdIfCb != NULL);
    hServer->getMcmCmdIfCb(hServer->enetType, &hMcmCmdIf);
    EnetAppUtils_assert(hMcmCmdIf != NULL);
    EnetAppUtils_assert(hMcmCmdIf->hMboxCmd != NULL);
    EnetAppUtils_assert(hMcmCmdIf->hMboxResponse != NULL);
    hServer->hMcmCmdIf = hMcmCmdIf;

    /* Connect to MCM on client's behalf (using its hostId) */
    EnetMcm_acquireHandleInfo(hMcmCmdIf, &handleInfo);
    hServer->hEnet = handleInfo.hEnet;

    ENET_IOCTL_SET_OUT_ARGS(&prms, &csumEnable);
    ENET_IOCTL(handleInfo.hEnet, hServer->masterCoreId,
               ENET_HOSTPORT_IS_CSUM_OFFLOAD_ENABLED, &prms, status);
    if (ENET_SOK == status)
    {
        hServer->csumOffloadEn = csumEnable;
    }
    else
    {
        status = ETHREMOTECFG_EFAIL;
        ETHFWTRACE_ERR(status, "IOCTL: Failed to get checksum offload enable");
    }

    if (status == ETHFW_SOK)
    {
        hServer->hMutex = EthFwOsal_createMutex();

        hServer->clientServiceObj.rpmsgStartSem = EthFwOsal_createSemaphore(0U);
        EnetAppUtils_assert(hServer->clientServiceObj.rpmsgStartSem != NULL);

        hServer->numVirtPorts = cfg->numVirtPorts;
        hServer->numAsrVirtPorts = 0U;
        for (i = 0U; i < hServer->numVirtPorts; i++)
        {
            hServer->virtPortCfg[i] = cfg->virtPortCfg[i];
        }

        ETHFWTRACE_INFO("Virtual port configuration:");

        EnetAppUtils_assert(cfg->numVirtPorts <= ENET_ARRAYSIZE(cfg->virtPortCfg));

        for (i = 0U; i < cfg->numVirtPorts; i++)
        {
            if (ETHFW_IS_BIT_SET(hServer->virtPortCfg[i].clientIdMask, ETHREMOTECFG_CLIENTID_AUTOSAR))
            {
                hServer->ethDrvObj[hServer->numAsrVirtPorts].localEp = hServer->virtPortCfg[i].endPointId;
                hServer->ethDrvObj[hServer->numAsrVirtPorts].coreId = hServer->virtPortCfg[i].remoteCoreId;
                hServer->ethDrvObj[hServer->numAsrVirtPorts].virtPort = hServer->virtPortCfg[i].portId;
                status = CpswProxyServer_initAutosarEthDeviceEp(hServer, cfg, hServer->numAsrVirtPorts);
                EnetAppUtils_assert(status == ETHFW_SOK);
                hServer->numAsrVirtPorts++;
            }
        }

        EnetAppUtils_assert(hServer->numAsrVirtPorts <= ENET_ARRAYSIZE(hServer->ethDrvObj));
        EnetAppUtils_assert(hServer->numAsrVirtPorts <= ENET_ARRAYSIZE(g_CpswProxyServerAutosarRpmsgBuf));
        EnetAppUtils_assert(hServer->numAsrVirtPorts <= ENET_ARRAYSIZE(gCpswProxyServer_autosarEthDriverTaskStackBuf));

        status = CpswProxyServer_initRemoteClientEthDeviceEp(hServer, cfg);
        EnetAppUtils_assert(status == ETHFW_SOK);

        status = CpswProxyServer_initNotifyServiceEp(hServer, cfg);
        EnetAppUtils_assert(status == ETHFW_SOK);

        hServer->initDone = BTRUE;
    }

    #if defined(ETHFW_RTOS_MCU2_1_SERVER)
    ETHFWTRACE_INFO("CpswProxyServer: initialization %s (core: mcu2_1)",
                    (status == ETHFW_SOK) ? "completed" : "failed");
    #else
    ETHFWTRACE_INFO("CpswProxyServer: initialization %s (core: mcu2_0)",
                    (status == ETHFW_SOK) ? "completed" : "failed");
    #endif

    return status;
}

static int32_t CpswProxyServer_dumpStatsCb(CpswProxyServer_ClientHandle hClient,
                                           uint32_t hostId)
{
#if defined(MCU_PLUS_SDK)
    int32_t status = ETHREMOTECFG_ENOTSUPPORTED;
#else
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;
    Enet_IoctlPrms prms;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        ENET_IOCTL_SET_NO_ARGS(&prms);
        ENET_IOCTL(hServer->hEnet, hostId, CPSW_ALE_IOCTL_DUMP_TABLE, &prms, status);

    }
    if (ETHREMOTECFG_SOK == status)
    {
        ENET_IOCTL(hServer->hEnet, hostId, CPSW_ALE_IOCTL_DUMP_POLICER_ENTRIES, &prms, status);
    }
    if (ETHREMOTECFG_SOK == status)
    {
        CpswProxyServer_printStats(hServer->hEnet, hServer->enetType, hServer->instId, hostId);
    }
    else
    {
        ETHFWTRACE_ERR(status, "Failed to dump stats for coreId:%u", hostId);
    }
#endif

    return status;
}

static int32_t CpswProxyServer_allocHwPushCb(CpswProxyServer_ClientHandle hClient,
                                             uint32_t hostId,
                                             uint32_t *hwPushNum)
{
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t coreKey;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        status = EnetAppUtils_allocHwPushInst(hServer->hEnet,
                                              coreKey,
                                              hostId,
                                              hwPushNum);
        if (ENET_SOK != status)
        {
            ETHFWTRACE_ERR(status, "Failed to alloc HW Push Instance");
        }
    }

    return status;
}

static int32_t CpswProxyServer_freeHwPushCb(CpswProxyServer_ClientHandle hClient,
                                            uint32_t hostId,
                                            uint32_t hwPushNum)
{
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;
    uint32_t coreKey;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    if (ETHREMOTECFG_SOK == status)
    {
        coreKey = hServer->coreObj[hostId].attachInfo.coreKey;

        status = EnetAppUtils_freeHwPushInst(hServer->hEnet,
                                              coreKey,
                                              hostId,
                                              hwPushNum);
        if (ENET_SOK != status)
        {
            ETHFWTRACE_ERR(status, "Failed to free HW Push Instance");
        }
    }

    return status;
}


static void CpswProxyServer_clientRequestHandler(EthFwIpc_RpmsgHandle hMsgHandle,
                                                 const void *reqBuf,
                                                 uint8_t clientId,
                                                 uint32_t remoteProcId,
                                                 uint32_t remoteEndPt,
                                                 uint32_t localEp)
{
    uint64_t resBuf[ETHREMOTECFG_IPC_MSG_SIZE / sizeof(uint64_t)];
    CpswProxyServer_ClientHandle hClient = NULL;
    EthRemoteCfg_ReqHdr *reqHdr = (EthRemoteCfg_ReqHdr *)reqBuf;  /* FIXME - make it const */
    EthRemoteCfg_ResHdr *resHdr = (EthRemoteCfg_ResHdr *)resBuf;
    uint16_t resLen = 0U;
    int32_t rtnVal = ENET_SOK;
    int32_t status = ENET_SOK;
    uint32_t token;

    token = reqHdr->common.token;

    switch (reqHdr->reqType)
    {
        case ETHREMOTECFG_CMD_VIRT_PORT_INFO:
        {
            EthRemoteCfg_OfferVirtPortRes *res = (EthRemoteCfg_OfferVirtPortRes *)resBuf;

            ETHFWTRACE_INFO("VIRT_PORT_INFO | C2S | core=%u endpt=%u",
                            remoteProcId, remoteEndPt);

            status = CpswProxyServer_VirtPortAllocCb(clientId,
                                                     remoteProcId,
                                                     &res->switchPortMask,
                                                     &res->macPortMask);
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                              "Failed to get virtual port allocation");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("VIRT_PORT_INFO | S2C | switchPortMask=%x macPortMask=%x",
                            res->switchPortMask, res->macPortMask);
            break;
        }
        case ETHREMOTECFG_CMD_ATTACH:
        {
            EthRemoteCfg_AttachReq *req = (EthRemoteCfg_AttachReq *)reqBuf;
            EthRemoteCfg_AttachRes *res = (EthRemoteCfg_AttachRes *)resBuf;

            ETHFWTRACE_INFO("ATTACH | C2S | core=%u endpt=%u virtPort=%u",
                            remoteProcId, remoteEndPt, req->virtPort);

            /* Allocate a client object */
            hClient = CpswProxyServer_allocClient(remoteProcId, remoteEndPt, req->virtPort);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EALLOC);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_attachHandlerCb(hClient,
                                                         remoteProcId,
                                                         req->virtPort,
                                                         &res->rxMtu,
                                                         &res->txMtu,
                                                         &res->features,
                                                         &res->numTxCh,
                                                         &res->numRxFlow);
                token = hClient->token;
                hClient->clientId = clientId;

                ETHFWTRACE_INFO("ATTACH | S2C | token=%d rxMtu=%u features=%x",
                                 (int32_t)token, res->rxMtu, res->features);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                              "Failed to attach virtual port %u core %u",
                              req->virtPort, remoteProcId);

            resLen = sizeof(EthRemoteCfg_AttachRes);
            break;
        }
        case ETHREMOTECFG_CMD_ATTACH_EXT:
        {
            EthRemoteCfg_AttachReq *req = (EthRemoteCfg_AttachReq *)reqBuf;
            EthRemoteCfg_AttachExtRes *res = (EthRemoteCfg_AttachExtRes *)resBuf;

            ETHFWTRACE_INFO("ATTACH_EXT | C2S | core=%u endpt=%u virtPort=%u",
                            remoteProcId, remoteEndPt, req->virtPort);

            /* Allocate a client object */
            hClient = CpswProxyServer_allocClient(remoteProcId, remoteEndPt, req->virtPort);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EALLOC);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_attachExtHandlerCb(hClient,
                                                            remoteProcId,
                                                            req->virtPort,
                                                            &res->rxMtu,
                                                            &res->txMtu,
                                                            &res->features,
                                                            &res->rxFlowIdxBase,
                                                            &res->rxFlowIdxOffset,
                                                            &res->txPsilDstId,
                                                            &res->rxPsilSrcId,
                                                            &res->macAddr[0U]);
                token = hClient->token;
                hClient->clientId = clientId;

                ETHFWTRACE_INFO("ATTACH_EXT | S2C | token=%d rxMtu=%u features=%x flow=%u,%u "
                                "rxPsil=0x%x txPsil=0x%x macAddr=%02x:%02x:%02x:%02x:%02x:%02x",
                                (int32_t)token, res->rxMtu, res->features,
                                res->rxFlowIdxBase, res->rxFlowIdxOffset,
                                res->rxPsilSrcId, res->txPsilDstId,
                                res->macAddr[0U], res->macAddr[1U], res->macAddr[2U],
                                res->macAddr[3U], res->macAddr[4U], res->macAddr[5U]);
            }
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                              "Failed to attach (ext) virtual port %u core %u",
                              req->virtPort, remoteProcId);

            resLen = sizeof(EthRemoteCfg_AttachExtRes);
            break;
        }
        case ETHREMOTECFG_CMD_DETACH:
        {
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DETACH | C2S | core=%u endpt=%u token=%d",
                            remoteProcId, remoteEndPt, (int32_t)token);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);

            /* Raise error for invalid detach request */
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EINVALIDPARAMS);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_detachHandlerCb(hClient,
                                                         remoteProcId);

                ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status,
                                    "Failed to detach virtual port %u core %u",
                                    hClient->virtPort, remoteProcId);

                CpswProxyServer_freeClient(hClient);
            }

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DETACH | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_ALLOC_TX:
        {
            EthRemoteCfg_AllocTxReq *req = (EthRemoteCfg_AllocTxReq *)reqBuf;
            EthRemoteCfg_AllocTxRes *res = (EthRemoteCfg_AllocTxRes *)resBuf;

            ETHFWTRACE_INFO("ALLOC_TX | C2S | core=%u endpt=%u token=%d",
                            remoteProcId, remoteEndPt, (int32_t)token);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_allocTxHandlerCb(hClient,
                                                          remoteProcId,
                                                          &res->txPsilDstId,
                                                          req->chRelPriority);

                ETHFWTRACE_INFO("ALLOC_TX | S2C | txPsil=0x%x status=%d",
                                res->txPsilDstId, status);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to alloc TX channel");

            resLen = sizeof(*res);

#if defined(ENABLE_MAC_ONLY_PORTS) && defined(MCU_PLUS_SDK)
            if (ETHREMOTECFG_SOK == status)
            {
                hClient = CpswProxyServer_getClient(remoteProcId, CPSWPROXY_VIRTPORT_2_TOKEN(ETHREMOTECFG_MAC_PORT_2));
                CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

                if (ETHREMOTECFG_SOK == status)
                {
                    hClient->psilDstId = res->txPsilDstId;
                }
            }
#endif
            break;
        }
        case ETHREMOTECFG_CMD_ALLOC_RX:
        {
            EthRemoteCfg_AllocRxReq *req = (EthRemoteCfg_AllocRxReq *)reqBuf;
            EthRemoteCfg_AllocRxRes *res = (EthRemoteCfg_AllocRxRes *)resBuf;

            ETHFWTRACE_INFO("ALLOC_RX | C2S | core=%u endpt=%u token=%d",
                            remoteProcId, remoteEndPt, (int32_t)token);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_allocRxHandlerCb(hClient,
                                                          remoteProcId,
                                                          &res->rxFlowIdxBase,
                                                          &res->rxFlowIdxOffset,
                                                          &res->rxPsilSrcId,
                                                          req->flowIdx);

                ETHFWTRACE_INFO("ALLOC_RX | S2C | flow=%u,%u rxPsil=0x%x status=%d",
                                res->rxFlowIdxBase, res->rxFlowIdxOffset, res->rxPsilSrcId, status);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to alloc RX flow");

            resLen = sizeof(*res);

#if defined(ENABLE_MAC_ONLY_PORTS) && defined(MCU_PLUS_SDK)
            if (ETHREMOTECFG_SOK == status)
            {
                hClient = CpswProxyServer_getClient(remoteProcId, CPSWPROXY_VIRTPORT_2_TOKEN(ETHREMOTECFG_MAC_PORT_2));
                CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

                if (ETHREMOTECFG_SOK == status)
                {
                    hClient->flowIdxBase   = res->rxFlowIdxBase;
                    hClient->flowIdxOffset = res->rxFlowIdxOffset;

                    CpswProxyServer_saveRxFlowOffset(hClient->virtPort, req->flowIdx, res->rxFlowIdxOffset);
                }
            }
#endif
            break;
        }
        case ETHREMOTECFG_CMD_ALLOC_MAC:
        {
            EthRemoteCfg_AllocMacRes *res = (EthRemoteCfg_AllocMacRes *)resBuf;

            ETHFWTRACE_INFO("ALLOC_MAC | C2S | core=%u endpt=%u token=%d",
                            remoteProcId, remoteEndPt, (int32_t)token);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_allocMacHandlerCb(hClient,
                                                          remoteProcId,
                                                          &res->macAddr[0U]);

                ETHFWTRACE_INFO("ALLOC_MAC | S2C | macAddr=%02x:%02x:%02x:%02x:%02x:%02x status=%d",
                                res->macAddr[0U], res->macAddr[1U], res->macAddr[2U],
                                res->macAddr[3U], res->macAddr[4U], res->macAddr[5U],
                                status);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to alloc MAC addr");

            resLen = sizeof(*res);
            break;
        }
        case ETHREMOTECFG_CMD_FREE_TX:
        {
            EthRemoteCfg_FreeTxReq *req = (EthRemoteCfg_FreeTxReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("FREE_TX | C2S | core=%u endpt=%u token=%d txPsil=0x%x",
                            remoteProcId, remoteEndPt, (int32_t)token, req->txPsilDstId);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_freeTxHandlerCb(hClient,
                                                         remoteProcId,
                                                         req->txPsilDstId);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to free TX channel");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("FREE_TX | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_FREE_RX:
        {
            EthRemoteCfg_FreeRxReq *req = (EthRemoteCfg_FreeRxReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("FREE_RX | C2S | core=%u endpt=%u token=%d flowidx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->rxFlowIdxBase, req->rxFlowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_freeRxHandlerCb(hClient,
                                                         remoteProcId,
                                                         req->rxFlowIdxBase,
                                                         req->rxFlowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to free RX flow");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("FREE_RX | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_FREE_MAC:
        {
            EthRemoteCfg_FreeMacReq *req = (EthRemoteCfg_FreeMacReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("FREE_MAC | C2S | core=%u endpt=%u token=%d "
                            "macAdd=%02x:%02x:%02x:%02x:%02x:%02x",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U]);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_freeMacHandlerCb(hClient,
                                                         remoteProcId,
                                                         req->macAddr);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to free MAC addr");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("FREE_MAC | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_REGISTER_MAC:
        {
            EthRemoteCfg_MacAddrRxFlowReq *req = (EthRemoteCfg_MacAddrRxFlowReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("REGISTER_MAC | C2S | core=%u endpt=%u token=%d "
                            "macAdd=%02x:%02x:%02x:%02x:%02x:%02x flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U],
                            req->flowIdxBase, req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_registerMacHandlerCb(hClient,
                                                              remoteProcId,
                                                              req->macAddr,
                                                              req->flowIdxBase,
                                                              req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to register MAC addr");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("REGISTER_MAC | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_DEREGISTER_MAC:
        {
            EthRemoteCfg_MacAddrRxFlowReq *req = (EthRemoteCfg_MacAddrRxFlowReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DEREGISTER_MAC | C2S | core=%u endpt=%u token=%d "
                            "macAdd=%02x:%02x:%02x:%02x:%02x:%02x flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U],
                            req->flowIdxBase, req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_unregisterMacHandlerCb(hClient,
                                                                remoteProcId,
                                                                req->macAddr,
                                                                req->flowIdxBase,
                                                                req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to unregister MAC addr");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DEREGISTER_MAC | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_REGISTER_IPv4:
        {
            EthRemoteCfg_IPv4AddrRegisterReq *req = (EthRemoteCfg_IPv4AddrRegisterReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("REGISTER_IPv4 | C2S | core=%u endpt=%u token=%d "
                            "ipAddr=%u.%u.%u.%u macAdd=%02x:%02x:%02x:%02x:%02x:%02x",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->ipAddr[0U], req->ipAddr[1U], req->ipAddr[2U], req->ipAddr[3U],
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U]);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_registerIPv4MacHandlerCb(hClient,
                                                                  remoteProcId,
                                                                  req->macAddr,
                                                                  req->ipAddr);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to register IPv4 addr");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("REGISTER_IPv4 | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_DEREGISTER_IPv4:
        {
            EthRemoteCfg_IPv4AddrDeregisterReq *req = (EthRemoteCfg_IPv4AddrDeregisterReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DEREGISTER_IPv4 | C2S | core=%u endpt=%u token=%d ipAddr=%u.%u.%u.%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->ipAddr[0U], req->ipAddr[1U], req->ipAddr[2U], req->ipAddr[3U]);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_deregisterIPv4MacHandlerCb(hClient,
                                                                    remoteProcId,
                                                                    req->ipAddr);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to deregister IPv4 addr");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DEREGISTER_IPv4 | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_JOIN_VLAN:
        {
            EthRemoteCfg_VlanJoinReq *req = (EthRemoteCfg_VlanJoinReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("JOIN_VLAN | C2S | core=%u endpt=%u token=%d vlanId=%u "
                            "macAdd=%02x:%02x:%02x:%02x:%02x:%02x flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token, req->vlanId,
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U],
                            req->flowIdxBase, req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_vlanJoinHandlerCb(hClient,
                                                           remoteProcId,
                                                           req->vlanId,
                                                           req->macAddr,
                                                           req->flowIdxBase,
                                                           req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to join VLAN %u", req->vlanId);

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("JOIN_VLAN | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_LEAVE_VLAN:
        {
            EthRemoteCfg_VlanLeaveReq *req = (EthRemoteCfg_VlanLeaveReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("LEAVE_VLAN | C2S | core=%u endpt=%u token=%d vlanId=%u "
                            "macAdd=%02x:%02x:%02x:%02x:%02x:%02x flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token, req->vlanId,
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U],
                            req->flowIdxBase, req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_vlanLeaveHandlerCb(hClient,
                                                            remoteProcId,
                                                            req->vlanId,
                                                            req->macAddr,
                                                            req->flowIdxBase,
                                                            req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to leave VLAN %u", req->vlanId);

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("LEAVE_VLAN | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_SET_RX_DEFAULTFLOW:
        {
            EthRemoteCfg_RxDefaultFlowRegisterReq *req = (EthRemoteCfg_RxDefaultFlowRegisterReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("SET_RX_DEFAULTFLOW | C2S | core=%u endpt=%u token=%d flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->flowIdxBase, req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_registerRxDefaultHandlerCb(hClient,
                                                                    remoteProcId,
                                                                    req->flowIdxBase,
                                                                    req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to register RX default flow");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("SET_RX_DEFAULTFLOW | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_DEL_RX_DEFAULTFLOW:
        {
            EthRemoteCfg_RxDefaultFlowRegisterReq *req = (EthRemoteCfg_RxDefaultFlowRegisterReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DEL_RX_DEFAULTFLOW | C2S | core=%u endpt=%u token=%d flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->flowIdxBase, req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_deregisterRxDefaultHandlerCb(hClient,
                                                                      remoteProcId,
                                                                      req->flowIdxBase,
                                                                      req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to deregister RX default flow");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DEL_RX_DEFAULTFLOW | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_REGISTER_MATCH_ETHTYPE:
        {
            EthRemoteCfg_MatchEthertypeAddReq *req = (EthRemoteCfg_MatchEthertypeAddReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("REGISTER_MATCH_ETHTYPE | C2S | core=%u endpt=%u token=%d "
                            "ethType=%x flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->ethertype, req->flowIdxBase, req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_registerEthertypeHandlerCb(hClient,
                                                                    remoteProcId,
                                                                    req->ethertype,
                                                                    req->flowIdxBase,
                                                                    req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to register EthType flow");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("REGISTER_MATCH_ETHTYPE | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_DEREGISTER_MATCH_ETHTYPE:
        {
            EthRemoteCfg_MatchEthertypeDelReq *req = (EthRemoteCfg_MatchEthertypeDelReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DEREGISTER_MATCH_ETHTYPE | C2S | core=%u endpt=%u token=%d ethType=%x",
                            remoteProcId, remoteEndPt, (int32_t)token, req->ethertype);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_deregisterEthertypeHandlerCb(hClient,
                                                                      remoteProcId,
                                                                      req->ethertype);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to deregister EthType flow");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DEREGISTER_MATCH_ETHTYPE | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_ADD_FILTER_MAC:
        {
            EthRemoteCfg_FilterMacAddReq *req = (EthRemoteCfg_FilterMacAddReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("ADD_FILTER_MAC | C2S | core=%u endpt=%u token=%d "
                            "macAdd=%02x:%02x:%02x:%02x:%02x:%02x vlanId=%u flowIdx=%u,%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U],
                            req->vlanId, req->flowIdxBase,req->flowIdxOffset);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_filterAddMacHandlerCb(hClient,
                                                               remoteProcId,
                                                               req->macAddr,
                                                               req->vlanId,
                                                               req->flowIdxBase,
                                                               req->flowIdxOffset);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to add mcast to filter");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("ADD_FILTER_MAC | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_DEL_FILTER_MAC:
        {
            EthRemoteCfg_FilterMacDelReq *req = (EthRemoteCfg_FilterMacDelReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DEL_FILTER_MAC | C2S | core=%u endpt=%u token=%d "
                            "macAdd=%02x:%02x:%02x:%02x:%02x:%02x vlanId=%u",
                            remoteProcId, remoteEndPt, (int32_t)token,
                            req->macAddr[0U], req->macAddr[1U], req->macAddr[2U],
                            req->macAddr[3U], req->macAddr[4U], req->macAddr[5U],
                            req->vlanId);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_filterDelMacHandlerCb(hClient,
                                                               remoteProcId,
                                                               req->macAddr,
                                                               req->vlanId);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to delete mcast from filter");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DEL_FILTER_MAC | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_PORT_LINK_STATUS:
        {
            EthRemoteCfg_PortLinkStatusRes *res = (EthRemoteCfg_PortLinkStatusRes *)resBuf;

            ETHFWTRACE_VERBOSE("PORT_LINK_STATUS | C2S | core=%u endpt=%u token=%d",
                               remoteProcId, remoteEndPt, (int32_t)token);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_isLinkUpCb(hClient,
                                                    remoteProcId,
                                                    &res->isLinked,
                                                    &res->speed,
                                                    &res->duplexity);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to get port link params");

            resLen = sizeof(*res);

            ETHFWTRACE_VERBOSE("PORT_LINK_STATUS | S2C | isLinked=%u speed=%u duplex=%u status=%d",
                               res->isLinked, res->speed, res->duplexity, status);
            break;
        }
        case ETHREMOTECFG_CMD_REGISTER_REMOTE_TIMER:
        {
            EthRemoteCfg_RemoteTimerRegisterReq *req = (EthRemoteCfg_RemoteTimerRegisterReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("REGISTER_REMOTE_TIMER | C2S | core=%u endpt=%u hwPushNum=%u timerId=%u",
                            remoteProcId, remoteEndPt, req->hwPushNum, req->timerId);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_registerRemoteTimerHandlerCb(NULL,
                                                                      remoteProcId,
                                                                      req->timerId,
                                                                      req->hwPushNum);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to register remote timer");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("REGISTER_REMOTE_TIMER | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_DEREGISTER_REMOTE_TIMER:
        {
            EthRemoteCfg_RemoteTimerDeregisterReq *req = (EthRemoteCfg_RemoteTimerDeregisterReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DEREGISTER_REMOTE_TIMER | C2S | core=%u endpt=%u hwPushNum=%u",
                            remoteProcId, remoteEndPt, req->hwPushNum);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_unregisterRemoteTimerHandlerCb(NULL,
                                                                        remoteProcId,
                                                                        req->hwPushNum);
            }

            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to unregister remote timer");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DEREGISTER_REMOTE_TIMER | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_GET_SERVER_STATUS:
        {
            EthRemoteCfg_ServerStatusRes *res = (EthRemoteCfg_ServerStatusRes *)resBuf;
            EthRemoteCfg_ServerStatus serverStatus;

            ETHFWTRACE_VERBOSE("GET_SERVER_STATUS | C2S | core=%u endpt=%u",
                            remoteProcId, remoteEndPt);

            /* Get the status of EthFw. */
            serverStatus = EthFw_getStatus();

            res->status = serverStatus;
            resLen = sizeof(*res);

            ETHFWTRACE_VERBOSE("GET_SERVER_STATUS | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_TEARDOWN_COMPLETION:
        {
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;
            status = ETHREMOTECFG_SOK;

            ETHFWTRACE_INFO("TEARDOWN_COMPLETION | C2S | core=%u endpt=%u token=%d",
                            remoteProcId, remoteEndPt, (int32_t)token);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                hClient->isIdle = BTRUE;
            }
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to set client as idle");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("TEARDOWN_COMPLETION | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_DUMP:
        {
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("DUMP | C2S | core=%u endpt=%u",
                            remoteProcId, remoteEndPt);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_dumpStatsCb(hClient, remoteProcId);
            }
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to dump stats");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("DUMP | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_ALLOC_CPTS_HW_PUSH:
        {
            EthRemoteCfg_AllocHwPushRes *res = (EthRemoteCfg_AllocHwPushRes *)resBuf;

            ETHFWTRACE_INFO("ALLOC_CPTS_HW_PUSH | C2S | core=%u endpt=%u",
                            remoteProcId, remoteEndPt);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_allocHwPushCb(hClient, remoteProcId, &res->hwPushNum);
            }
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to allocate HW push instance");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("ALLOC_CPTS_HW_PUSH | S2C | status=%d", status);
            break;
        }
        case ETHREMOTECFG_CMD_FREE_CPTS_HW_PUSH:
        {
            EthRemoteCfg_FreeHwPushReq *req = (EthRemoteCfg_FreeHwPushReq *)reqBuf;
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            ETHFWTRACE_INFO("FREE_CPTS_HW_PUSH | C2S | core=%u endpt=%u",
                            remoteProcId, remoteEndPt);

            /* Get client object for token */
            hClient = CpswProxyServer_getClient(remoteProcId, token);
            CPSWPROXY_ERR_CHECK((hClient == NULL), status, ETHREMOTECFG_EFAIL);

            if (ETHREMOTECFG_SOK == status)
            {
                status = CpswProxyServer_freeHwPushCb(hClient, remoteProcId, req->hwPushNum);
            }
            ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "Failed to free HW push instance");

            resLen = sizeof(*res);

            ETHFWTRACE_INFO("FREE_CPTS_HW_PUSH | S2C | status=%d", status);
            break;
        }
        default:
        {
            EthRemoteCfg_StatusRes *res = (EthRemoteCfg_StatusRes *)resBuf;

            status = ETHREMOTECFG_EFAIL;
            resLen = sizeof(*res);
            ETHFWTRACE_ERR(status, "Unknown cmd %u", reqHdr->reqType);
            break;
        }
    }

    resHdr->common.msgType  = ETHREMOTECFG_MSGTYPE_RESPONSE;
    resHdr->common.token    = token;
    resHdr->common.clientId = clientId;
    resHdr->resType         = reqHdr->reqType;
    resHdr->resId           = reqHdr->reqId;
    resHdr->status          = status;

    ETHFWTRACE_DBG_IF((resHdr->resType != ETHREMOTECFG_CMD_PORT_LINK_STATUS),
                      "S2C | msgType=%u token=%d clientId=%u resId=%u status=%d len=%u (%u.%u->%u.%u)",
                      resHdr->common.msgType,
                      (int32_t)resHdr->common.token,
                      resHdr->common.clientId,
                      resHdr->resId,
                      resHdr->status,
                      resLen,
                      EnetSoc_getCoreId(), localEp,
                      remoteProcId, remoteEndPt);

    ETHFWTRACE_VERBOSE_IF((resHdr->resType == ETHREMOTECFG_CMD_PORT_LINK_STATUS),
                          "S2C | msgType=%u token=%d clientId=%u resId=%u status=%d len=%u (%u.%u->%u.%u)",
                          resHdr->common.msgType,
                          (int32_t)resHdr->common.token,
                          resHdr->common.clientId,
                          resHdr->resId,
                          resHdr->status,
                          resLen,
                          EnetSoc_getCoreId(), localEp,
                          remoteProcId, remoteEndPt);

    rtnVal = EthFwIpc_sendRpmsg(hMsgHandle, remoteProcId, remoteEndPt, localEp, &resBuf, resLen);
    ETHFWTRACE_ERR_IF((rtnVal != ETHFW_SOK), rtnVal, "Failed to send msg via IPC");
}

static int32_t CpswProxyServer_initRemoteClientEthDeviceEp(CpswProxyServer_Obj *hServer,
                                                           CpswProxyServer_Config_t * cfg)
{
    EthFwOsal_TaskParams taskParams;
    int32_t status = ETHFW_SOK;
    EthFwIpc_RpmsgCreateParams comParams;

    /* Initialize the param and set memory for HeapMemory for control task */
    EthFwIpc_initRpmsgParams(&comParams);
    comParams.buf = gCpswProxyServerRpmsgbuf;
    comParams.bufSize = sizeof(gCpswProxyServerRpmsgbuf);
    comParams.numBufs = ETHREMOTECFG_IPC_NUM_MSG_BUFS;
    comParams.requestedEndpt = CPSWPROXY_REMOTE_SERVICE_ENDPT_ID;

    hServer->clientServiceObj.hClientServicRpMsgEp = EthFwIpc_createRpmsg(&comParams);
    if (hServer->clientServiceObj.hClientServicRpMsgEp == NULL)
    {
        status = ETHFW_EFAIL;
        ETHFWTRACE_ERR(status, "Could not create communication channel for endpoint %d",
                       comParams.requestedEndpt);
    }
    else
    {
        EthFwOsal_postSemaphore(hServer->clientServiceObj.rpmsgStartSem);
    }

    if (ETHFW_SOK == status)
    {
        hServer->clientServiceObj.localEp = comParams.requestedEndpt;
    }

    if (ETHFW_SOK == status)
    {
        /* Initialize the task params */
        EthFwOsal_initTaskParams(&taskParams);
        taskParams.name         = CPSWPROXY_ETH_CLIENT_TASK_NAME;
        taskParams.priority     = CPSWPROXY_ETH_CLIENT_TASK_PRIORITY;
        taskParams.arg0         = (void*) hServer;
        taskParams.stack        = &gCpswProxyServer_remoteClientEthDriverTaskStackBuf[0];
        taskParams.stacksize    = CPSWPROXY_ETH_CLIENT_TASK_STACK;

        hServer->clientServiceObj.hClientServiceTsk = EthFwOsal_createTask(&CpswProxyServer_remoteClientEthDriverTaskFxn, &taskParams);
        if(hServer->clientServiceObj.hClientServiceTsk == NULL)
        {
            status = ETHFW_EFAIL;
            ETHFWTRACE_ERR(status, "Could not create task for endpoint %d", comParams.requestedEndpt);
        }
    }

    return status;
}

static void CpswProxyServer_remoteClientEthDriverTaskFxn(void* arg0)
{
    CpswProxyServer_Obj *hServer = (CpswProxyServer_Obj *)arg0;
    int32_t rtnVal = ETHFW_SOK;
    uint32_t remoteProcId;
    uint32_t remoteEndPt;
    uint16_t len;
    uint64_t msgBuf[ETHREMOTECFG_IPC_MSG_SIZE / sizeof(uint64_t)];
    bool exitTask;
    uint32_t localEp = hServer->clientServiceObj.localEp;
    EthFwIpc_RpmsgHandle hClientServicRpMsgEp = hServer->clientServiceObj.hClientServicRpMsgEp;

    rtnVal = EthFwIpc_announceAll(localEp, ETHREMOTECFG_FRAMEWORK_SERVICE_NAME);
    if (ETHFW_SOK != rtnVal)
    {
        ETHFWTRACE_ERR(rtnVal, "Couldn't announce endpoint for remote clients");
    }
    else
    {
        exitTask = BFALSE;

        while (!exitTask)
        {
            rtnVal = EthFwIpc_recvRpmsg(hClientServicRpMsgEp,
                                        (void *)msgBuf,
                                        &len,
                                        &remoteEndPt,
                                        &remoteProcId,
                                        ETHFWIPC_WAIT_FOREVER);
            if (ETHFW_SOK == rtnVal)
            {
                EthRemoteCfg_MsgHdr *hdr;
                uint8_t clientId;

                EnetAppUtils_assert(len <= sizeof(msgBuf));

                hdr = (EthRemoteCfg_MsgHdr *)msgBuf;
                clientId = hdr->clientId;

                if (hdr->msgType == ETHREMOTECFG_MSGTYPE_REQUEST)
                {
                    CpswProxyServer_clientRequestHandler(hClientServicRpMsgEp,
                                                         &msgBuf,
                                                         clientId,
                                                         remoteProcId,
                                                         remoteEndPt,
                                                         localEp);
                }
                else if (hdr->msgType == ETHREMOTECFG_MSGTYPE_NOTIFY)
                {
                    ETHFWTRACE_ERR(ETHREMOTECFG_EFAIL, "Received NOTIFY message: Client-Server communication supports only requests");
                }
                else
                {
                    ETHFWTRACE_ERR(ETHREMOTECFG_EFAIL, "Received RESPONSE message: Client-Server communication supports only requests");
                }
            }
            else
            {
                ETHFWTRACE_ERR(rtnVal, "Failed to receive msg via IPC");
            }
        }
    }
}

static void CpswProxyServer_hwPushNotifyFxn(void *arg, CpswCpts_HwPush hwPushNum)
{
    if (arg != NULL)
    {
        CpswProxyServer_Obj *hServer = (CpswProxyServer_Obj *)arg;

        /* Post Event */
        EthFwOsal_postEvent(hServer->notifyServiceObj.hHwPushNotifyServiceEvent,
                            hServer->notifyServiceObj.hwPushNotifyEventId[CPSW_CPTS_HWPUSH_NORM(hwPushNum)]);
    }
}

static int32_t CpswProxyServer_initAutosarEthDeviceEp(CpswProxyServer_Obj *hServer,
                                                      CpswProxyServer_Config_t * cfg,
                                                      uint32_t clientInst)
{
    EthFwOsal_TaskParams taskParams;
    int32_t retVal = ETHFW_SOK;
    EthFwIpc_RpmsgCreateParams comChParam;

    EthFwIpc_initRpmsgParams(&comChParam);
    comChParam.numBufs = CPSWPROXY_AUTOSAR_ETHDRIVER_NUM_RPMSG_BUFS;
    comChParam.buf = g_CpswProxyServerAutosarRpmsgBuf[clientInst];
    comChParam.bufSize = sizeof(g_CpswProxyServerAutosarRpmsgBuf[clientInst]);
    comChParam.requestedEndpt = hServer->ethDrvObj[clientInst].localEp;

    hServer->ethDrvObj[clientInst].hAutosarEthRpMsgEp = EthFwIpc_createRpmsg(&comChParam);
    if (hServer->ethDrvObj[clientInst].hAutosarEthRpMsgEp == NULL)
    {
        retVal = ETHFW_EFAIL;
        ETHFWTRACE_ERR(retVal, "Could not create communication channel for endpoint %d",
                       comChParam.requestedEndpt);
    }
    else
    {
        hServer->ethDrvObj[clientInst].localEp = comChParam.requestedEndpt;
    }

    if (ETHFW_SOK == retVal)
    {
        /* Initialize the task params */
        EthFwOsal_initTaskParams(&taskParams);
        taskParams.name         = CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_NAME;
        taskParams.priority     = CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_PRIORITY;
        taskParams.arg0         = (void*) clientInst;
        taskParams.stack        = &gCpswProxyServer_autosarEthDriverTaskStackBuf[clientInst][0];
        taskParams.stacksize    = CPSWPROXY_AUTOSAR_ETHDRIVER_TASK_STACK;

        hServer->ethDrvObj[clientInst].hAutosarEthTsk = EthFwOsal_createTask(&CpswProxyServer_autosarEthDriverTaskFxn, &taskParams);
        if(hServer->ethDrvObj[clientInst].hAutosarEthTsk == NULL)
        {
            retVal = ETHFW_EFAIL;
            ETHFWTRACE_ERR(retVal, "Could not create task for endpoint %d", comChParam.requestedEndpt);
        }
    }

    return retVal;
}

static int32_t CpswProxyServer_initNotifyServiceEp(CpswProxyServer_Obj * hServer, CpswProxyServer_Config_t * cfg)
{
    EthFwOsal_TaskParams taskParams;
    int32_t retVal = ETHFW_SOK;
    EthFwIpc_RpmsgCreateParams comChParam;
    EthFwOsal_EventParams eventParams;
    uint8_t i = 0;

    hServer->notifyServiceObj.notifyServiceCpswType = hServer->enetType;
    hServer->notifyServiceObj.dstProcMask = 0U;

    for (i = 0U; i < CPSW_CPTS_HWPUSH_COUNT_MAX; i++)
    {
        hServer->notifyServiceObj.hwPush2CoreIdMap[i] = IPC_MAX_PROCS;
    }

    EthFwIpc_initRpmsgParams(&comChParam);
    comChParam.numBufs = ETHREMOTECFG_IPC_NUM_MSG_BUFS;
    comChParam.buf = g_CpswProxyServerNotifyServiceRpmsgBuf;
    comChParam.bufSize = sizeof(g_CpswProxyServerNotifyServiceRpmsgBuf);
    comChParam.requestedEndpt = CPSWPROXY_NOTIFY_SERVICE_ENDPT_ID;

    hServer->notifyServiceObj.hNotifyServicRpMsgEp = EthFwIpc_createRpmsg(&comChParam);

    if (hServer->notifyServiceObj.hNotifyServicRpMsgEp == NULL)
    {
        retVal = ETHFW_EFAIL;
        ETHFWTRACE_ERR(retVal, "Could not create communication channel");
    }
    else
    {
        hServer->notifyServiceObj.localEp = comChParam.requestedEndpt;
    }

    /* Announce service */
    if (ETHFW_SOK == retVal)
    {
        retVal = EthFwIpc_announceAll(hServer->notifyServiceObj.localEp,
                                      ETHREMOTECFG_REMOTE_NOTIFY_SERVICE);
        ETHFWTRACE_ERR_IF((retVal != ETHFW_SOK), retVal, "Failed to annount notify server");
    }

    /* Create Event to notify task */
    if (ETHFW_SOK == retVal)
    {
        EthFwOsal_initEventParams(&eventParams);

        for (i = 0U; i < CPSW_CPTS_HWPUSH_COUNT_MAX; i++)
        {
            hServer->notifyServiceObj.hwPushNotifyEventId[i] = (1U << i);
        }

        hServer->notifyServiceObj.hHwPushNotifyServiceEvent = EthFwOsal_createEvent(&eventParams);

        if (hServer->notifyServiceObj.hHwPushNotifyServiceEvent == NULL)
        {
            retVal = ETHFW_EFAIL;
            ETHFWTRACE_ERR(retVal, "Could not create an event");
        }
    }

    if (ETHFW_SOK == retVal)
    {
        /* Initialize the task params */
        EthFwOsal_initTaskParams(&taskParams);
        taskParams.name         = CPSWPROXY_NOTIFY_SERVICE_TASK_NAME;
        taskParams.priority     = CPSWPROXY_NOTIFY_SERVICE_TASK_PRIORITY;
        taskParams.arg0         = (void*) hServer;
        taskParams.stack        = &gCpswProxyServer_notifyServiceTaskStackBuf[0];
        taskParams.stacksize    = CPSWPROXY_NOTIFY_SERVICE_SERVER_TASK_STACKSIZE;
        hServer->notifyServiceObj.hNotifyServiceTsk = EthFwOsal_createTask(&CpswProxyServer_notifyServiceTaskFxn, &taskParams);
        if(hServer->notifyServiceObj.hNotifyServiceTsk == NULL)
        {
            retVal = ETHFW_EFAIL;
            ETHFWTRACE_ERR(retVal, "Could not create a task");
        }
    }

    return retVal;
}

static void CpswProxyServer_notifyServiceTaskFxn(void* arg0)
{
    int32_t rtnVal = ENET_SOK;
    Enet_Handle hEnet = NULL;
    EnetMcm_HandleInfo handleInfo;
    CpswProxyServer_Obj * hServer = (CpswProxyServer_Obj *)arg0;
    uint64_t msgBuffer[(ETHREMOTECFG_IPC_MSG_SIZE / sizeof(uint64_t))];
    volatile bool exitTask = BFALSE;
    Enet_IoctlPrms prms;
    CpswCpts_Event lookupEventInArgs;
    CpswCpts_Event lookupEventOutArgs;
    uint32_t i = 0U;
    uint32_t events = 0U;
    uint32_t remoteCoreId;

    /* Get MCM cmd handle */
    if (hServer->hMcmCmdIf != NULL)
    {
        /* Connect to MCM on client's behalf (using its hostId) */
        EnetMcm_acquireHandleInfo(hServer->hMcmCmdIf, &handleInfo);
        hEnet = handleInfo.hEnet;
    }
    else
    {
        rtnVal = ETHREMOTECFG_EINVALIDPARAMS;
        ETHFWTRACE_ERR(rtnVal, "Cannot get lookup events as MCM handle is NULL");
    }

    while (!exitTask)
    {
        /*Wait 1ms for hardware push event, then move on to other events*/
        events = EthFwOsal_waitEvent(hServer->notifyServiceObj.hHwPushNotifyServiceEvent,
                                     CPSWPROXY_CPTS_HWPUSH_EVENTS_OR_MASK,
                                     ETHFWEVENT_WAITMODE_ANY,
                                     1U);

        /* Lookup for timestamp if it is a hardware push notification*/
        if (events)
        {
            if ((rtnVal == ENET_SOK) && (NULL != hEnet))
            {
                for (i = 0U; i < CPSW_CPTS_HWPUSH_COUNT_MAX; i++)
                {
                    if(ENET_IS_BIT_SET(events, i))
                    {
                        EthRemoteCfg_NotifyServiceHwPushMsg *hwPushMsg = (EthRemoteCfg_NotifyServiceHwPushMsg *)msgBuffer;

                        remoteCoreId = hServer->notifyServiceObj.hwPush2CoreIdMap[i];

                        memset(hwPushMsg, 0, sizeof(*hwPushMsg));
                        hwPushMsg->hdr.common.token = ETHREMOTECFG_TOKEN_NONE;
                        hwPushMsg->hdr.notifyType = ETHREMOTECFG_NOTIFY_HWPUSH;
                        hwPushMsg->hwPushNum = i + 1U;

                        lookupEventInArgs.eventType = CPSW_CPTS_EVENTTYPE_HW_TS_PUSH;
                        lookupEventInArgs.hwPushNum = (CpswCpts_HwPush) hwPushMsg->hwPushNum;
                        lookupEventInArgs.portNum = 0U;
                        lookupEventInArgs.seqId = 0U;
                        lookupEventInArgs.domain  = 0U;

                        ENET_IOCTL_SET_INOUT_ARGS(&prms, &lookupEventInArgs, &lookupEventOutArgs);
                        ENET_IOCTL(hEnet, EnetSoc_getCoreId(), CPSW_CPTS_IOCTL_LOOKUP_EVENT, &prms, rtnVal);
                        if (rtnVal == ENET_SOK)
                        {
                            if ((remoteCoreId != IPC_MAX_PROCS) &&
                                ENET_IS_BIT_SET(hServer->notifyServiceObj.dstProcMask, remoteCoreId))
                            {
                                hwPushMsg->timeStamp = lookupEventOutArgs.tsVal;

                                ETHFWTRACE_DBG("NOTIFY | S2C | core=%u endpt=%u token=%d notifyType=%u",
                                               remoteCoreId,
                                               ETHREMOTECFG_NOTIFY_SERVICE_ENDPT_ID,
                                               (int32_t)hwPushMsg->hdr.common.token,
                                               hwPushMsg->hdr.notifyType);

                                ETHFWTRACE_DBG("S2C | msgType=%u token=%d clientId=%u notifyType=%u len=%u (%u.%u->%u.%u)",
                                               hwPushMsg->hdr.common.msgType,
                                               (int32_t)hwPushMsg->hdr.common.token,
                                               hwPushMsg->hdr.common.clientId,
                                               hwPushMsg->hdr.notifyType,
                                               sizeof(*hwPushMsg),
                                               EnetSoc_getCoreId(), hServer->notifyServiceObj.localEp,
                                               remoteCoreId, ETHREMOTECFG_NOTIFY_SERVICE_ENDPT_ID);

                                rtnVal = EthFwIpc_sendRpmsg(hServer->notifyServiceObj.hNotifyServicRpMsgEp,
                                                            remoteCoreId,
                                                            ETHREMOTECFG_NOTIFY_SERVICE_ENDPT_ID,
                                                            hServer->notifyServiceObj.localEp,
                                                            hwPushMsg,
                                                            sizeof(*hwPushMsg));
                                ETHFWTRACE_ERR_IF((rtnVal != ENET_SOK), rtnVal, "Failed to send tstamp notification");
                            }
                        }
                        else
                        {
                            ETHFWTRACE_ERR(rtnVal, "Failed to get CPTS event info");
                        }
                    }
                }
            }
        }
    }
}


static void CpswProxyServer_autosarEthDriverTaskFxn(void* arg0)
{
    int32_t rtnVal = ENET_SOK;
    uint32_t remoteProcId, remoteEndPt;
    CpswProxyServer_Obj *hServer = CpswProxyServer_initHandle();
    uint32_t clientNum = (uint32_t)arg0;
    uint32_t remoteProc, remoteEp;
    uint16_t len;
    uint64_t msgBuf[CPSWPROXY_AUTOSAR_ETHDRIVER_MSG_SIZE / sizeof(uint64_t)];
    uint32_t localdstProc = hServer->ethDrvObj[clientNum].coreId;
    uint32_t localEp = hServer->ethDrvObj[clientNum].localEp;
    EthFwIpc_RpmsgHandle hAutosarEthRpMsgEp = hServer->ethDrvObj[clientNum].hAutosarEthRpMsgEp;
    EthRemoteCfg_DeviceData deviceData;

    /* Wait for Remote EP to active */
    rtnVal = EthFwIpc_getRemoteEndPt(localdstProc,
                                     ETHREMOTECFG_AUTOSAR_REMOTE_SERVICE_NAME,
                                     &remoteProcId,
                                     &remoteEndPt,
                                     ETHFWIPC_WAIT_FOREVER);

    if (ENET_SOK != rtnVal)
    {
        ETHFWTRACE_ERR(rtnVal, "Remote AUTOSAR Ethernet Device locate failed");
    }
    else
    {
        bool exitTask = BFALSE;

        EnetAppUtils_assert(hServer->initEthfwDeviceDataCb != NULL);
        hServer->initEthfwDeviceDataCb(&deviceData);

        deviceData.hdr.notifyType = ETHREMOTECFG_NOTIFY_FWINFO;
        deviceData.hdr.common.clientId = ETHREMOTECFG_CLIENTID_NONE;
        deviceData.hdr.common.msgType = ETHREMOTECFG_MSGTYPE_NOTIFY;
        deviceData.hdr.common.token = ETHREMOTECFG_TOKEN_NONE;

        /* Send the EthFw Device Data to AUTOSAR EthDriver on location of endpoint */
        rtnVal = EthFwIpc_sendRpmsg(hAutosarEthRpMsgEp,
                                    remoteProcId,
                                    remoteEndPt,
                                    localEp,
                                    (Ptr)&deviceData,
                                    sizeof(deviceData));
        EnetAppUtils_assert(ENET_SOK == rtnVal);

        while (!exitTask)
        {
            rtnVal = EthFwIpc_recvRpmsg(hAutosarEthRpMsgEp,
                                        (Ptr)msgBuf,
                                        &len,
                                        &remoteEp,
                                        &remoteProc,
                                        ETHFWIPC_WAIT_FOREVER);
            if (ENET_SOK == rtnVal)
            {
                EthRemoteCfg_MsgHdr *hdr;
                uint8_t clientId;

                EnetAppUtils_assert(len <= sizeof(msgBuf));
                EnetAppUtils_assert(remoteEp == remoteEndPt);
                EnetAppUtils_assert(remoteProcId == remoteProc);

                hdr = (EthRemoteCfg_MsgHdr *)msgBuf;
                clientId = hdr->clientId;

                if (hdr->msgType == ETHREMOTECFG_MSGTYPE_REQUEST)
                {
                    CpswProxyServer_clientRequestHandler(hAutosarEthRpMsgEp, &msgBuf, clientId, remoteProc, remoteEp, localEp);
                }

                else if ( hdr->msgType == ETHREMOTECFG_MSGTYPE_NOTIFY)
                {
                    ETHFWTRACE_ERR(ETHREMOTECFG_EFAIL, "Received NOTIFY message: Client-Server communication supports only requests");
                }
                else
                {
                    ETHFWTRACE_ERR(ETHREMOTECFG_EFAIL, "Received RESPONSE message: Client-Server communication supports only requests");
                }
            }
            else
            {
                ETHFWTRACE_ERR(rtnVal, "Failed to receive msg via IPC");
            }
        }
    }
}

int32_t CpswProxyServer_lateAnnounce(uint32_t procId)
{
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");
    EthFwOsal_pendSemaphore(hServer->clientServiceObj.rpmsgStartSem, ETHFWOSAL_WAIT_FOREVER);

    status = EthFwIpc_announce(procId, hServer->clientServiceObj.localEp, ETHREMOTECFG_FRAMEWORK_SERVICE_NAME);
    ETHFWTRACE_INFO_IF((status == ETHFW_SOK), "Announce Endpoint Service to HLOS");

    return status;
}

static uint32_t CpswProxyServer_getAbsTxChNumber(uint32_t chRelPriority,
                                                 EthRemoteCfg_VirtPort virtPort)
{
    uint32_t i;
    uint32_t txChNum = ENET_RM_TXCHNUM_INVALID;
    int32_t status = ETHREMOTECFG_SOK;
    CpswProxyServer_Obj *hServer = NULL;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < hServer->numVirtPorts; i++)
    {
        if (hServer->virtPortCfg[i].portId == virtPort)
        {
            if (chRelPriority < hServer->virtPortCfg[i].numTxCh)
            {
                txChNum = hServer->virtPortCfg[i].txCh[chRelPriority];
            }
            else
            {
                ETHFWTRACE_ERR(ETHFW_EFAIL, "Failed to get absolute tx channel number, invalid relative channel id %u passed",
                               chRelPriority);
            }
            break;
        }
    }

    return txChNum;
}

static int32_t CpswProxyServer_getClientTxChRxFlowNum(EthRemoteCfg_VirtPort virtPort,
                                                       uint32_t *pNumTxCh,
                                                       uint32_t *pNumRxFlow)
{
    uint32_t i;
    int32_t status = ETHFW_EFAIL;
    CpswProxyServer_Obj *hServer = NULL;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < hServer->numVirtPorts; i++)
    {
        if (hServer->virtPortCfg[i].portId == virtPort)
        {
            *pNumTxCh = hServer->virtPortCfg[i].numTxCh;
            *pNumRxFlow = hServer->virtPortCfg[i].numRxFlow;
            status = ETHFW_SOK;
            break;
        }
    }

    return status;
}

static int32_t CpswProxyServer_createCustomPolicer(Enet_Handle hEnet,
                                                   uint32_t coreId,
                                                   CpswAle_SetPolicerEntryInPartitionInArgs *customPolInArgs)
{
    CpswAle_SetPolicerEntryOutArgs polOutArgs;
    Enet_IoctlPrms prms;
    int32_t status = ETHFW_EFAIL;

    ENET_IOCTL_SET_INOUT_ARGS(&prms, customPolInArgs, &polOutArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_SET_POLICER_IN_PARTITION, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                      "Failed to create custom policer for core %u",
                       coreId);

    return status;
}

static int32_t CpswProxyServer_deleteCustomPolicer(Enet_Handle hEnet,
                                                   uint32_t coreId,
                                                   CpswAle_SetPolicerEntryInPartitionInArgs *customPolInArgs)
{
    CpswAle_DelPolicerEntryInArgs polInArgs;
    Enet_IoctlPrms prms;
    int32_t status = ETHFW_EFAIL;

    polInArgs.policerMatch = customPolInArgs->policerMatch;
    /* Remove ALE policer and entry from ALE as well */
    polInArgs.aleEntryMask = CPSW_ALE_POLICER_TABLEENTRY_DELETE_ALL;
    ENET_IOCTL_SET_IN_ARGS(&prms, &polInArgs);

    ENET_IOCTL(hEnet, coreId, CPSW_ALE_IOCTL_DEL_POLICER, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                      "Failed to create custom policer for core %u",
                       coreId);

    return status;
}

static int32_t CpswProxyServer_getRxFlowInfo(EthRemoteCfg_VirtPort virtPort,
                                             uint32_t relFlowIdx,
                                             EthFwQos_RxFlowInfo **rxFlowInfo)
{
    uint32_t i;
    int32_t status = ETHREMOTECFG_EFAIL;
    CpswProxyServer_Obj *hServer =  NULL;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < hServer->numVirtPorts; i++)
    {
        if (hServer->virtPortCfg[i].portId == virtPort)
        {
            if (relFlowIdx < hServer->virtPortCfg[i].numRxFlow)
            {
                *rxFlowInfo = &hServer->virtPortCfg[i].rxFlowsInfo[relFlowIdx];
                status = ETHREMOTECFG_SOK;
            }
            else
            {
                status = ETHREMOTECFG_EBADARGS;
                ETHFWTRACE_ERR(status,
                               "Invalid rel flow index %u passed to virtual port %u",
                               relFlowIdx,
                               virtPort);
            }
            break;
        }
    }

    return status;
}

static int32_t CpswProxyServer_getRelFlowIdx(EthRemoteCfg_VirtPort virtPort,
                                             uint32_t *relFlowIdx,
                                             uint32_t rxFlowIdxOffset)
{
    uint32_t i;
    uint32_t j;
    int32_t status = ETHREMOTECFG_EFAIL;
    CpswProxyServer_Obj *hServer = NULL;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < hServer->numVirtPorts; i++)
    {
        if (hServer->virtPortCfg[i].portId == virtPort)
        {
            for (j = 0U; j < ENET_CFG_RX_FLOWS_NUM; j++)
            {
                if (hServer->virtPortCfg[i].rxFlowsInfo[j].rxFlowIdxOffset == rxFlowIdxOffset)
                {
                    *relFlowIdx = j;
                    status = ETHREMOTECFG_SOK;
                    break;
                }
            }
            break;
        }
    }

    return status;
}

static void CpswProxyServer_saveRxFlowOffset(EthRemoteCfg_VirtPort virtPort,
                                             uint32_t relFlowIdx,
                                             uint32_t rxFlowIdxOffset)
{
    uint32_t i;
    CpswProxyServer_Obj *hServer = NULL;
    int32_t status = ETHREMOTECFG_SOK;

    status = CpswProxyServer_getHandle(&hServer);
    ETHFWTRACE_ERR_IF((status != ETHREMOTECFG_SOK), status, "Failed to get server handle");

    for (i = 0U; i < hServer->numVirtPorts; i++)
    {
        if (hServer->virtPortCfg[i].portId == virtPort)
        {
            hServer->virtPortCfg[i].rxFlowsInfo[relFlowIdx].rxFlowIdxOffset = rxFlowIdxOffset;
            break;
        }
    }
}
