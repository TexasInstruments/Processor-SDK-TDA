/*
 *
 * Copyright (c) 2020-2024 Texas Instruments Incorporated
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
#define ETHFWTRACE_MOD_ID 0x201
#define ETHFWTRACE_MOD_NAME "CpswProxy"

#ifdef QNX_OS
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/slogcodes.h>
#include <sys/slog.h>
#else
#if defined(__KLOCWORK__)
#include <stdlib.h>
#endif
#endif

#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>

#include <ethremotecfg/client/include/cpsw_proxy.h>
#include <ethremotecfg/client/include/ts_coupler_client.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_common/include/ethfw_utils.h>

#if defined(SAFERTOS)
#include "SafeRTOS_API.h"
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Maximum number of supported CpswProxy clients */
#define CPSWPROXY_CLIENT_MAX                            (2U)

/* Command response mailbox */
#define CPSWPROXY_CMD_RES_MBX_MSGSIZE                   (sizeof(CpswProxy_Msg))
#define CPSWPROXY_CMD_RES_MBX_MSGCOUNT                  (4U)
#if defined(SAFERTOS)
#define CPSWPROXY_CMD_RES_MBX_SIZE                      ((CPSWPROXY_CMD_RES_MBX_MSGSIZE * \
                                                          CPSWPROXY_CMD_RES_MBX_MSGCOUNT) + \
                                                         safertosapiQUEUE_OVERHEAD_BYTES)
#else
#define CPSWPROXY_CMD_RES_MBX_SIZE                      (CPSWPROXY_CMD_RES_MBX_MSGSIZE * \
                                                         CPSWPROXY_CMD_RES_MBX_MSGCOUNT)
#endif

#define CPSWPROXY_LOCATE_TIMEOUT                        (10U)

/* Command service task name and priority */
#define CPSWPROXY_CMD_SERVICE_TASK_NAME                 "CpswProxy-CmdTask"
#ifndef QNX_OS
#define CPSWPROXY_CMD_SERVICE_TASK_PRI                  (2U)
#else
#define CPSWPROXY_CMD_SERVICE_TASK_PRI                  (22U)
#endif

/* Notify service task name and priority */
#define CPSWPROXY_NOTIFY_SERVICE_TASK_NAME              "CpswProxy-NotifyTask"
#ifndef QNX_OS
#define CPSWPROXY_NOTIFY_SERVICE_TASK_PRI               (4U)
#else
#define CPSWPROXY_NOTIFY_SERVICE_TASK_PRI               (22U)
#endif

#define CPSWPROXY_IPC_TASK_STACKALIGN                   (8192U)

/*! Heartbeat Task priority */
#ifndef QNX_OS
#define CPSWPROXY_HB_TASK_PRI                           (2U)
#else
#define CPSWPROXY_HB_TASK_PRI                           (22U)
#endif

/*! Heartbeat Task stack size and alignment */
#if defined(SAFERTOS)
#define CPSWPROXY_HB_TASK_STACK_SIZE                     (8U * 1024U)
#define CPSWPROXY_HB_TASK_STACK_ALIGN                    CPSWPROXY_HB_TASK_STACK_SIZE
#else
#define CPSWPROXY_HB_TASK_STACK_SIZE                     (8U * 1024U)
#define CPSWPROXY_HB_TASK_STACK_ALIGN                    (32U)
#endif

/*! Remote service task stack size */
#define CPSWPROXY_TASK_STACKSIZE                        (16U * 1024U)
#define CPSWPROXY_TASK_STACKALIGN                       CPSWPROXY_TASK_STACKSIZE

#define CPSWPROXY_CMD_SERVICE_END_POINT                 (36U)

#if defined(__KLOCWORK__)
#define CpswProxy_assert(cond)               do { if (!(cond)) abort(); } while (0)
#else
#define CpswProxy_assert(cond)                                   \
    (CpswProxy_assertLocal((bool) (cond), (const char *) # cond, \
                    (const char *) __FILE__, (int32_t) __LINE__))
#endif

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* Command request container */
typedef struct CpswProxy_Cmd_s
{
#ifndef QNX_OS
    /* Mailbox where command service will send response to */
    EthFwOsal_MailboxHandle hMbx;
#endif

    /* Command request header */
    EthRemoteCfg_ReqHdr *req;

    /* Command request length */
    uint16_t reqLen;

    /* Command response header */
    EthRemoteCfg_ResHdr *res;

    /* Command response length */
    uint16_t resLen;
} CpswProxy_Cmd;

/* Message container */
typedef struct CpswProxy_Msg_s
{
    /* Status */
    int32_t status;

    /* Actual message length */
    uint16_t len;

    /* Message buffer */
    uint64_t buf[ETHREMOTECFG_IPC_MSG_SIZE / sizeof(uint64_t)];
} CpswProxy_Msg;

/* Notify callback info */
typedef struct CpswProxy_NotifyCb_s
{
    /*! Callback function */
    CpswProxy_NotifyCbFxn cbFxn;

    /*! Hardware push notify arguments */
    void *cbArg;
} CpswProxy_NotifyCb;

/* Notify service */
typedef struct CpswProxy_NotifyService_s
{
    /* RPMessage handle for remote notify service */
    EthFwIpc_RpmsgHandle hRpMsg;

    /* Local endpoint for notify service in proxy client */
    uint32_t localEndpt;

    /* Master core id where Cpsw Proxy Server runs */
    uint32_t masterCoreId;

    /* Endpoint associated with Cpsw Proxy Server */
    uint32_t masterEndpt;

    /* Task handle for remote notify service */
    EthFwOsal_TaskHandle hTask;

    /* Whether notify task should exit */
    bool exitTask;

    /* Buffer to store received messages for remote notify service */
    uint8_t rpMsgBuf[ETHREMOTECFG_IPC_DATA_SIZE] __attribute__ ((aligned(8192)));

    /* Notify service task stack buffer */
    uint8_t taskStack[CPSWPROXY_TASK_STACKSIZE]
            __attribute__ ((aligned(CPSWPROXY_TASK_STACKALIGN)));
} CpswProxy_NotifyService;

/* Command service */
typedef struct CpswProxy_CmdService_s
{
    /* RPMessage handle */
    EthFwIpc_RpmsgHandle hRpMsg;

    /* Local endpoint */
    uint32_t localEndpt;

    /* Master core id where Cpsw Proxy Server runs */
    uint32_t masterCoreId;

    /* Endpoint associated with Cpsw Proxy Server */
    uint32_t masterEndpt;

    /* Mutex handle used to protect the order in the cmd mailbox */
    EthFwOsal_MutexHandle hMutex;

    /* Task handle for message handling */
    EthFwOsal_TaskHandle hTask;

    /* Whether command handler task should exit */
    bool exitTask;

#ifndef QNX_OS
    /* Mailbox used to receive command responses from service task */
    EthFwOsal_MailboxHandle hMbx;

    /* Mailbox buffer for storing all the response message */
    uint8_t mbxBuf[CPSWPROXY_CMD_RES_MBX_SIZE] __attribute__ ((aligned(32)));
#else
    /* Channel id */
    int chid;

    /* Connection id */
    int coid;
#endif

    /* RPMessage buffer */
    uint8_t rpMsgBuf[ETHREMOTECFG_IPC_DATA_SIZE] __attribute__ ((aligned(1024)));

    /* Task stack */
    uint8_t taskStackBuf[CPSWPROXY_TASK_STACKSIZE]
            __attribute__ ((aligned(CPSWPROXY_TASK_STACKALIGN)));
} CpswProxy_CmdService;

/* Client - shares the same IPC channel with other clients */
typedef struct CpswProxy_ClientObj_s
{
    /* Whether client object is already in used by an app */
    bool inUse;

    /* Saved configuration params */
    CpswProxy_Config cfg;

    /* Token used after attaching to ETHFW */
    uint32_t token;

    /* Sequential request id number */
    uint32_t reqId;

    /* Features supported by related virtual port */
    uint32_t features;

    /* Notiy callbacks */
    CpswProxy_NotifyCb notifyCb[ETHREMOTECFG_NOTIFY_TYPE_COUNT];
} CpswProxy_ClientObj;

/* Cpsw Proxy - services multiple clients */
typedef struct CpswProxy_Obj_s
{
    /* Mutex handle used to protect get/free CpswProxy_ClientObjs */
    EthFwOsal_MutexHandle hMutex;

    /*! CPSW Proxy Heartbeat period in milliseconds */
    uint32_t hbPeriodInMsecs;

    /*! Clock handle for Heartbeat Task */
    EthFwOsal_ClockHandle hHeartbeatClock;

    /*! Semaphore handle for Heartbeat Task */
    EthFwOsal_SemHandle hHeartbeatSem;

    /*! Task handle for Heartbeat Task */
    EthFwOsal_TaskHandle hHeartbeatTask;

    /* Enable Heartbeat for CPSW Proxy */
    bool heartBeatEn;

    /* Heartbeat Task stack buffer */
    uint8_t hbTaskStackBuf[CPSWPROXY_HB_TASK_STACK_SIZE] __attribute__ ((aligned(CPSWPROXY_HB_TASK_STACK_ALIGN)));

    /* Notify callbacks for CPSW Proxy Heartbeat Event */
    CpswProxy_HeartbeatCb hbNotifyCb;

    /* Enable TimeSync for CPSW Proxy */
    bool timeSyncEn;

    /* Notify callbacks for CPSW Proxy TimeSync Event */
    CpswProxy_TimeSyncCb tsNotifyCb;

    /* Array of client objects. Size of this array determines the number of virtual ports
     * that can be used by this core */
    CpswProxy_ClientObj clientObj[CPSWPROXY_CLIENT_MAX];

    /* Command service */
    CpswProxy_CmdService cmdSvc;

    /* Notify service, currently supports only timestamp reception */
    CpswProxy_NotifyService notifySvc;

    /*! Command service timeout in milliseconds */
    uint32_t cmdTimeoutInMsecs;
} CpswProxy_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static CpswProxy_Handle CpswProxy_getHandle(uint32_t token);

static CpswProxy_Handle CpswProxy_allocHandle(void);

static void CpswProxy_freeHandle(CpswProxy_Handle hProxy);

static void CpswProxy_instanceInit(CpswProxy_Handle hProxy,
                                   const CpswProxy_Config *cfg);

static void CpswProxy_instanceDeinit(CpswProxy_Handle hProxy);

static int32_t CpswProxy_initCmdSvc(CpswProxy_CmdService *svc);

static void CpswProxy_deinitCmdSvc(CpswProxy_CmdService *svc);

static void CpswProxy_cmdHandlerTask(void* arg0);

static int32_t CpswProxy_cmdHandler(CpswProxy_CmdService *svc,
                                    CpswProxy_Msg *msg);

static int32_t CpswProxy_initNotifySvc(CpswProxy_NotifyService *svc);

static void CpswProxy_deinitNotifySvc(CpswProxy_NotifyService *svc);

static void CpswProxy_notifySvcTask(void *arg0);

static int32_t CpswProxy_notifyHandler(EthRemoteCfg_NotifyHdr *hdr,
                                       uint16_t msgLen);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static CpswProxy_Obj gCpswProxy;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void CpswProxy_initConfig(CpswProxy_initParams *params)
{
    params->hbPeriodInMsecs   = CPSWPROXY_HB_DEFAULT_POLL_PERIOD_MS;
    params->cmdTimeoutInMsecs = CPSWPROXY_CMD_DEFAULT_TIMEOUT_MS;
    params->hbNotifyCb.cbArg  = NULL;
    params->hbNotifyCb.cbFxn  = NULL;
    params->tsNotifyCb.cbArg  = NULL;
    params->tsNotifyCb.cbFxn  = NULL;
}

int32_t CpswProxy_init(const CpswProxy_initParams *params)
{
    int32_t status = CPSWPROXY_SOK;

    memset(&gCpswProxy, 0, sizeof(gCpswProxy));

    if ((params->hbPeriodInMsecs != 0U) && (params->hbNotifyCb.cbFxn != NULL))
    {
        gCpswProxy.heartBeatEn = BTRUE;
        gCpswProxy.hbPeriodInMsecs = params->hbPeriodInMsecs;
        gCpswProxy.hbNotifyCb.cbArg = params->hbNotifyCb.cbArg;
        gCpswProxy.hbNotifyCb.cbFxn = params->hbNotifyCb.cbFxn;
    }
    else
    {
        gCpswProxy.heartBeatEn = BFALSE;
    }

    if (params->tsNotifyCb.cbFxn != NULL)
    {
        gCpswProxy.timeSyncEn = BTRUE;
        gCpswProxy.tsNotifyCb.cbArg = params->tsNotifyCb.cbArg;
        gCpswProxy.tsNotifyCb.cbFxn = params->tsNotifyCb.cbFxn;
    }
    else
    {
        gCpswProxy.timeSyncEn = BFALSE;
    }

    /* Passing a value of 0 for cmdTimeoutInMsecs means ProxyClient blocks forever for remote command response */
    if (params->cmdTimeoutInMsecs == 0U)
    {
        gCpswProxy.cmdTimeoutInMsecs = ETHFWOSAL_WAIT_FOREVER;
    }
    else
    {
        gCpswProxy.cmdTimeoutInMsecs = params->cmdTimeoutInMsecs;
    }

    gCpswProxy.hMutex = EthFwOsal_createMutex();
    if (gCpswProxy.hMutex == NULL)
    {
        status = CPSWPROXY_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to create mutex");
        EthFw_assert(gCpswProxy.hMutex != NULL);
    }

    /* Initialize command service (talks to "ti.ethfw.ethdevice") */
    if (status == CPSWPROXY_SOK)
    {
        status = CpswProxy_initCmdSvc(&gCpswProxy.cmdSvc);
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                          "Failed to initialize command service");
    }

    /* Initialize command service (talks to "ti.ethfw.ethdevice") */
    if (status == CPSWPROXY_SOK)
    {
        status = CpswProxy_initNotifySvc(&gCpswProxy.notifySvc);
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                          "Failed to initialize command service");
    }

    ETHFWTRACE_INFO_IF((status == CPSWPROXY_SOK),
                       "Local cmd endpt %u, notify endpt %u",
                       gCpswProxy.cmdSvc.localEndpt,
                       gCpswProxy.notifySvc.localEndpt);

    return status;
}

void CpswProxy_deinit(void)
{
    gCpswProxy.heartBeatEn = BFALSE;

    if (gCpswProxy.hHeartbeatTask != NULL)
    {
        EthFwOsal_deleteTask(&gCpswProxy.hHeartbeatTask);
        gCpswProxy.hHeartbeatTask = NULL;

        /* Stop and delete the clock */
        EthFwOsal_stopClock(gCpswProxy.hHeartbeatClock);
        EthFwOsal_deleteClock(gCpswProxy.hHeartbeatClock);

        /* Delete semaphore */
        EthFwOsal_deleteSemaphore(gCpswProxy.hHeartbeatSem);
    }

    /* Deinitialize command service */
    CpswProxy_deinitCmdSvc(&gCpswProxy.cmdSvc);

    /* Deinitialize notify service */
    CpswProxy_deinitNotifySvc(&gCpswProxy.notifySvc);

    if (gCpswProxy.hMutex != NULL)
    {
        EthFwOsal_deleteMutex(gCpswProxy.hMutex);
    }
}

static void CpswProxy_ClockCb(void *arg)
{
    /* Post semaphore to Heartbeat Task */
    EthFwOsal_postSemaphore(gCpswProxy.hHeartbeatSem);
}

static void CpswProxy_HeartbeatTask(void *a0)
{
    CpswProxy_HeartbeatCbFxn cbFxn;
    void *cbArg = NULL;
    EthRemoteCfg_ServerStatus serverStatus;

    cbFxn = gCpswProxy.hbNotifyCb.cbFxn;
    cbArg = gCpswProxy.hbNotifyCb.cbArg;

    while (gCpswProxy.heartBeatEn)
    {
        EthFwOsal_pendSemaphore(gCpswProxy.hHeartbeatSem, ETHFWOSAL_WAIT_FOREVER);

        if (gCpswProxy.clientObj[0U].inUse == BTRUE)
        {
            CpswProxy_getServerStatus(&gCpswProxy.clientObj[0U], &serverStatus);
            cbFxn(serverStatus, cbArg);
        }
    }

}

int32_t CpswProxy_connect(void)
{
    CpswProxy_CmdService *cmdSvc = &gCpswProxy.cmdSvc;
    CpswProxy_NotifyService *notifySvc = &gCpswProxy.notifySvc;
    EthFwOsal_ClockParams clkParams;
    EthFwOsal_TaskParams params;
    int32_t status = CPSWPROXY_SOK;

    /* Wait for ETHFW's command service to be active */
    status = EthFwIpc_getRemoteEndPt(RPMESSAGE_ANY,
                                     ETHREMOTECFG_FRAMEWORK_SERVICE_NAME,
                                     &cmdSvc->masterCoreId,
                                     &cmdSvc->masterEndpt,
                                     CPSWPROXY_LOCATE_TIMEOUT);
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                      "Failed to get ETHFW command service endpt");

    /* Wait for ETHFW's notify service to be active on the same remote core */
    if (status == CPSWPROXY_SOK)
    {
        status = EthFwIpc_getRemoteEndPt(cmdSvc->masterCoreId,
                                         ETHREMOTECFG_REMOTE_NOTIFY_SERVICE,
                                         &notifySvc->masterCoreId,
                                         &notifySvc->masterEndpt,
                                         ETHFWIPC_WAIT_FOREVER);
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                          "Failed to get ETHFW command service endpt");
    }

    ETHFWTRACE_INFO_IF((status == CPSWPROXY_SOK),
                       "ETHFW services found at core %u endpts %u (%s) and %u (%s)",
                       cmdSvc->masterCoreId,
                       cmdSvc->masterEndpt, ETHREMOTECFG_FRAMEWORK_SERVICE_NAME,
                       notifySvc->masterEndpt, ETHREMOTECFG_REMOTE_NOTIFY_SERVICE);

    if (gCpswProxy.heartBeatEn == BTRUE)
    {
        /* Start the CPSW Proxy Heartbeat task*/
        if (status == CPSWPROXY_SOK)
        {
            gCpswProxy.hHeartbeatSem = EthFwOsal_createSemaphore(1U);

            if (gCpswProxy.hHeartbeatSem == NULL)
            {
                status = CPSWPROXY_EALLOC;
                ETHFWTRACE_ERR(status, "Unable to create Heartbeat clock semaphore");
                EthFw_assert(BFALSE);
            }
        }

        if (status == CPSWPROXY_SOK)
        {
            /* Create Heartbeat Task to detect EthFw's failure. */
            EthFwOsal_initTaskParams(&params);
            params.priority  = CPSWPROXY_HB_TASK_PRI;
            params.stack     = &gCpswProxy.hbTaskStackBuf[0];
            params.stacksize = sizeof(gCpswProxy.hbTaskStackBuf);
            params.name      = "CPSW Proxy Heartbeat Task";

            gCpswProxy.hHeartbeatTask = EthFwOsal_createTask(&CpswProxy_HeartbeatTask, &params);

            if (gCpswProxy.hHeartbeatTask == NULL)
            {
                status = CPSWPROXY_EALLOC;
                ETHFWTRACE_ERR(status, "Unable to create Heartbeat task");
                EthFw_assert(BFALSE);
            }
        }

        if (status == CPSWPROXY_SOK)
        {
            EthFwOsal_initClockParams(&clkParams);
            clkParams.startMode = ETHFWCLOCK_STARTMODE_AUTO;
            clkParams.period    = gCpswProxy.hbPeriodInMsecs;
            clkParams.runMode   = ETHFWCLOCK_RUNMODE_CONTINUOUS;

            /* Creating clock and setting clock callback function */
            gCpswProxy.hHeartbeatClock = EthFwOsal_createClock((void*)&CpswProxy_ClockCb, &clkParams);
            if (gCpswProxy.hHeartbeatClock == NULL)
            {
                status = CPSWPROXY_EALLOC;
                ETHFWTRACE_ERR(status, "Unable to create Heartbeat clock");
                EthFw_assert(BFALSE);
            }
        }
    }

    return status;
}

CpswProxy_Handle CpswProxy_open(const CpswProxy_Config *cfg)
{
    CpswProxy_Handle hProxy;

    /* Get a handle to a free CpswProxy object, if any */
    hProxy = CpswProxy_allocHandle();
    if (hProxy == NULL)
    {
        ETHFWTRACE_ERR(CPSWPROXY_EALLOC, "Failed to allocate client object");
    }
    else
    {
        /* Create CpswProxy's mailboxes, init cmd and notify tasks */
        CpswProxy_instanceInit(hProxy, cfg);
    }

    return hProxy;
}

void CpswProxy_close(CpswProxy_Handle hProxy)
{
    /* Delete CpswProxy's mailboxes, close tasks */
    CpswProxy_instanceDeinit(hProxy);

    /* Release handle to CpswProxy object */
    CpswProxy_freeHandle(hProxy);
}

int32_t CpswProxy_attach(CpswProxy_Handle hProxy,
                         EthRemoteCfg_VirtPort virtPort,
                         uint32_t *rxMtu,
                         uint32_t *txMtu,
                         uint32_t *pNumTxCh,
                         uint32_t *pNumRxFlow,
                         uint32_t *features)
{
    EthRemoteCfg_AttachReq req;
    EthRemoteCfg_AttachRes res;
    int32_t status;

    ETHFWTRACE_INFO("ATTACH | C2S | virtPort=%u", virtPort);

    req.virtPort = virtPort;

    memset(&res, 0, sizeof(EthRemoteCfg_AttachRes));

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_ATTACH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send ATTACH cmd");

    if (status == CPSWPROXY_SOK)
    {
        hProxy->token    = res.hdr.common.token;
        hProxy->features = res.features;

        *features        = res.features;
        *pNumTxCh        = res.numTxCh;
        *pNumRxFlow      = res.numRxFlow;

        *rxMtu = res.rxMtu;
        *txMtu = res.txMtu;
    }

    ETHFWTRACE_INFO("ATTACH | S2C | token=%d rxMtu=%u features=%x numTxCh=%u numRxFlow=%u status=%d",
                    (int32_t)res.hdr.common.token, res.rxMtu, res.features, res.numTxCh, res.numRxFlow, status);

    return status;
}

int32_t CpswProxy_attachExtended(CpswProxy_Handle hProxy,
                                 EthRemoteCfg_VirtPort virtPort,
                                 uint32_t *rxMtu,
                                 uint32_t *txMtu,
                                 uint32_t *txPSILThreadId,
                                 uint32_t *rxFlowIdxBase,
                                 uint32_t *rxFlowIdxOffset,
                                 uint8_t *macAddr,
                                 uint32_t *features)
{
    EthRemoteCfg_AttachReq req;
    EthRemoteCfg_AttachExtRes res;
    int32_t status;

    ETHFWTRACE_INFO("ATTACH_EXT | C2S | virtPort=%u", virtPort);

    req.virtPort = virtPort;

    memset(&res, 0, sizeof(EthRemoteCfg_AttachExtRes));

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_ATTACH_EXT,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send ATTACH_EXT cmd");

    if (status == CPSWPROXY_SOK)
    {
        hProxy->token    = res.hdr.common.token;
        hProxy->features = res.features;

        *features = res.features;
        *rxMtu = res.rxMtu;
        *txMtu = res.txMtu;

        *txPSILThreadId  = res.txPsilDstId;
        *rxFlowIdxBase   = res.rxFlowIdxBase;
        *rxFlowIdxOffset = res.rxFlowIdxOffset;

        memcpy(macAddr, res.macAddr, ETHREMOTECFG_MACADDRLEN);
    }

    ETHFWTRACE_INFO("ATTACH_EXT | S2C | token=%d rxMtu=%u features=%x flow=%u,%u "
                    "rxPsil=0x%x txPsil=0x%x macAddr=%02x:%02x:%02x:%02x:%02x:%02x",
                    (int32_t)res.hdr.common.token, res.rxMtu, res.features,
                    res.rxFlowIdxBase, res.rxFlowIdxOffset,
                    res.rxPsilSrcId, res.txPsilDstId,
                    res.macAddr[0U], res.macAddr[1U], res.macAddr[2U],
                    res.macAddr[3U], res.macAddr[4U], res.macAddr[5U]);

    return status;
}

int32_t CpswProxy_detach(CpswProxy_Handle hProxy)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("DETACH | C2S | token=%d", (int32_t)hProxy->token);

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DETACH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DETACH cmd");

    ETHFWTRACE_INFO("DETACH | S2C | status=%d", status);

    return status;
}

int32_t CpswProxy_allocTxCh(CpswProxy_Handle hProxy,
                            uint32_t *txPSILThreadId,
                            uint32_t chRelPriority)
{
    EthRemoteCfg_AllocTxReq req;
    EthRemoteCfg_AllocTxRes res;
    int32_t status;

    ETHFWTRACE_INFO("ALLOC_TX | C2S | token=%d chRelPri=%u", (int32_t)hProxy->token, chRelPriority);

    req.chRelPriority = chRelPriority;

    memset(&res, 0, sizeof(EthRemoteCfg_AllocTxRes));

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_ALLOC_TX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send ALLOC_TX cmd");

    *txPSILThreadId  = res.txPsilDstId;

    ETHFWTRACE_INFO("ALLOC_TX | S2C | token=%d txPsil=0x%x chRelPri=%u status=%d",
                    (int32_t)hProxy->token, res.txPsilDstId, chRelPriority ,status);

    return status;
}

int32_t CpswProxy_freeTxCh(CpswProxy_Handle hProxy,
                           uint32_t txChNum)
{
    EthRemoteCfg_FreeTxReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("FREE_TX | C2S | token=%d txPsil=0x%x", (int32_t)hProxy->token, txChNum);

    req.txPsilDstId = txChNum;

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_FREE_TX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send FREE_TX cmd");

    ETHFWTRACE_INFO("FREE_TX | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_allocRxFlow(CpswProxy_Handle hProxy,
                              uint32_t *rxFlowIdxBase,
                              uint32_t *rxFlowIdxOffset,
                              uint32_t flowIdx)
{
    EthRemoteCfg_AllocRxReq req;
    EthRemoteCfg_AllocRxRes res;
    int32_t status;

    ETHFWTRACE_INFO("ALLOC_RX | C2S | token=%d", (int32_t)hProxy->token);

    memset(&res, 0, sizeof(EthRemoteCfg_AllocRxRes));

    req.flowIdx = flowIdx;

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_ALLOC_RX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send ALLOC_RX cmd");

    *rxFlowIdxBase   = res.rxFlowIdxBase;
    *rxFlowIdxOffset = res.rxFlowIdxOffset;

    ETHFWTRACE_INFO("ALLOC_RX | S2C | token=%d flow=%u,%u rxPsil=0x%x status=%d",
                    (int32_t)hProxy->token, res.rxFlowIdxBase, res.rxFlowIdxOffset,
                    res.rxPsilSrcId, status);

    return status;
}

int32_t CpswProxy_freeRxFlow(CpswProxy_Handle hProxy,
                             uint32_t rxFlowIdxBase,
                             uint32_t rxFlowIdxOffset)
{
    EthRemoteCfg_FreeRxReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("FREE_RX | C2S | token=%d flow=%u,%u",
                    (int32_t)hProxy->token, rxFlowIdxBase, rxFlowIdxOffset);

    req.rxFlowIdxBase   = rxFlowIdxBase;
    req.rxFlowIdxOffset = rxFlowIdxOffset;

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_FREE_RX,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send FREE_RX cmd");

    ETHFWTRACE_INFO("FREE_RX | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_allocMac(CpswProxy_Handle hProxy,
                           uint8_t *macAddr)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_AllocMacRes res;
    int32_t status;

    ETHFWTRACE_INFO("ALLOC_MAC | C2S | token=%d", (int32_t)hProxy->token);

    memset(&res, 0, sizeof(EthRemoteCfg_AllocMacRes));

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_ALLOC_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send ALLOC_MAC cmd");

    memcpy(macAddr, res.macAddr, ETHREMOTECFG_MACADDRLEN);

    ETHFWTRACE_INFO("ALLOC_MAC | S2C | token=%d macAddr=%02x:%02x:%02x:%02x:%02x:%02x status=%d",
                    (int32_t)hProxy->token,
                    res.macAddr[0U], res.macAddr[1U], res.macAddr[2U],
                    res.macAddr[3U], res.macAddr[4U], res.macAddr[5U],
                    status);

    return status;
}

int32_t CpswProxy_freeMac(CpswProxy_Handle hProxy,
                          const uint8_t *macAddr)
{
    EthRemoteCfg_FreeMacReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("FREE_MAC | C2S | token=%d macAddr=%02x:%02x:%02x:%02x:%02x:%02x",
                    (int32_t)hProxy->token,
                    macAddr[0U], macAddr[1U], macAddr[2U],
                    macAddr[3U], macAddr[4U], macAddr[5U]);

    memcpy(req.macAddr, macAddr, ETHREMOTECFG_MACADDRLEN);

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_FREE_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send FREE_MAC cmd");

    ETHFWTRACE_INFO("FREE_MAC | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_registerDefaultRxFlow(CpswProxy_Handle hProxy,
                                        uint32_t flowIdxBase,
                                        uint32_t flowIdxOffset)
{
    EthRemoteCfg_RxDefaultFlowRegisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("SET_RX_DEFAULTFLOW | C2S | token=%d flowIdx=%u,%u",
                    (int32_t)hProxy->token, flowIdxBase, flowIdxOffset);

    req.flowIdxBase   = flowIdxBase;
    req.flowIdxOffset = flowIdxOffset;

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_SET_RX_DEFAULTFLOW,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send SET_RX_DEFAULTFLOW cmd");

    ETHFWTRACE_INFO("SET_RX_DEFAULTFLOW | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_unregisterDefaultRxFlow(CpswProxy_Handle hProxy,
                                          uint32_t flowIdxBase,
                                          uint32_t flowIdxOffset)
{
    EthRemoteCfg_RxDefaultFlowRegisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("DEL_RX_DEFAULTFLOW | C2S | token=%d flowIdx=%u,%u",
                    (int32_t)hProxy->token, flowIdxBase, flowIdxOffset);

    req.flowIdxBase   = flowIdxBase;
    req.flowIdxOffset = flowIdxOffset;

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DEL_RX_DEFAULTFLOW,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DEL_RX_DEFAULTFLOW cmd");

    ETHFWTRACE_INFO("DEL_RX_DEFAULTFLOW | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_registerDstMacRxFlow(CpswProxy_Handle hProxy,
                                       uint32_t flowIdxBase,
                                       uint32_t flowIdxOffset,
                                       const uint8_t *macAddr)
{
    EthRemoteCfg_MacAddrRxFlowReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("REGISTER_MAC | C2S | token=%d flowIdx=%u,%u",
                    (int32_t)hProxy->token, flowIdxBase, flowIdxOffset);

    req.flowIdxBase   = flowIdxBase;
    req.flowIdxOffset = flowIdxOffset;
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_REGISTER_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send REGISTER_MAC cmd");

    ETHFWTRACE_INFO("REGISTER_MAC | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_unregisterDstMacRxFlow(CpswProxy_Handle hProxy,
                                         uint32_t flowIdxBase,
                                         uint32_t flowIdxOffset,
                                         const uint8_t *macAddr)
{
    EthRemoteCfg_MacAddrRxFlowReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("DEREGISTER_MAC | C2S | token=%d flowIdx=%u,%u",
                    (int32_t)hProxy->token, flowIdxBase, flowIdxOffset);

    req.flowIdxBase   = flowIdxBase;
    req.flowIdxOffset = flowIdxOffset;
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DEREGISTER_MAC,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DEREGISTER_MAC cmd");

    ETHFWTRACE_INFO("DEREGISTER_MAC | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_registerIPV4Addr(CpswProxy_Handle hProxy,
                                   uint8_t *macAddr,
                                   uint8_t *ipv4Addr)
{
    EthRemoteCfg_IPv4AddrRegisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("REGISTER_IPv4 | C2S | token=%d ipAddr=%u.%u.%u.%u macAdd=%02x:%02x:%02x:%02x:%02x:%02x",
                    (int32_t)hProxy->token,
                    ipv4Addr[0U], ipv4Addr[1U], ipv4Addr[2U], ipv4Addr[3U],
                    macAddr[0U], macAddr[1U], macAddr[2U],
                    macAddr[3U], macAddr[4U], macAddr[5U]);

    memcpy(req.ipAddr, ipv4Addr, ETHREMOTECFG_IPV4ADDRLEN);
    memcpy(req.macAddr, macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_REGISTER_IPv4,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send REGISTER_IPv4 cmd");

    ETHFWTRACE_INFO("REGISTER_IPv4 | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_unregisterIPV4Addr(CpswProxy_Handle hProxy,
                                     uint8_t *ipv4Addr)
{
    EthRemoteCfg_IPv4AddrDeregisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("UNREGISTER_IPv4 | C2S | token=%d ipAddr=%u.%u.%u.%u",
                    (int32_t)hProxy->token,
                    ipv4Addr[0U], ipv4Addr[1U], ipv4Addr[2U], ipv4Addr[3U]);

    memcpy(req.ipAddr, ipv4Addr, ETHREMOTECFG_IPV4ADDRLEN);

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DEREGISTER_IPv4,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DEREGISTER_IPv4 cmd");

    ETHFWTRACE_INFO("UNREGISTER_IPv4 | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

bool CpswProxy_isPhyLinked(CpswProxy_Handle hProxy)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_PortLinkStatusRes res;
    bool isLinked;
    int32_t status;

    ETHFWTRACE_VERBOSE("PORT_LINK_STATUS | C2S | token=%d", (int32_t)hProxy->token);

    memset(&res, 0, sizeof(EthRemoteCfg_PortLinkStatusRes));

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_PORT_LINK_STATUS,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send PORT_LINK_STATUS cmd");

    ETHFWTRACE_VERBOSE("PORT_LINK_STATUS | S2C | token=%d speed=%u duplex=%s linked=%s status=%d",
                       (int32_t)hProxy->token, res.speed, res.duplexity ? "half" : "full",
                       res.isLinked ? "yes":"no", status);

    isLinked = (status == CPSWPROXY_SOK) ? res.isLinked : BFALSE;

    return isLinked;
}

int32_t CpswProxy_teardownCompletion(CpswProxy_Handle hProxy)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("TEARDOWN_COMPLETION | C2S | token=%d", (int32_t)hProxy->token);

    /* Notify server side that this client has tear-down its DMA channels and is ready
     * for Ethernet device recovery to continue */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_TEARDOWN_COMPLETION,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send TEARDOWN_COMPLETION cmd");

    ETHFWTRACE_INFO("TEARDOWN_COMPLETION | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_registerEthertypeRxFlow(CpswProxy_Handle hProxy,
                                          uint32_t flowIdxBase,
                                          uint32_t flowIdxOffset,
                                          uint16_t etherType)
{
    EthRemoteCfg_MatchEthertypeAddReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("REGISTER_MATCH_ETHERTYPE | C2S | token=%d etherType=%x flowIdx=%u,%u",
                    (int32_t)hProxy->token, etherType, flowIdxBase, flowIdxOffset);

    req.ethertype     = etherType;
    req.flowIdxBase   = flowIdxBase;
    req.flowIdxOffset = flowIdxOffset;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_REGISTER_MATCH_ETHTYPE,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send REGISTER_MATCH_ETHERTYPE cmd");

    ETHFWTRACE_INFO("REGISTER_MATCH_ETHERTYPE | S2C | token=%d status=%d",
                    (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_unregisterEthertypeRxFlow(CpswProxy_Handle hProxy,
                                            uint16_t etherType)
{
    EthRemoteCfg_MatchEthertypeDelReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("DEREGISTER_MATCH_ETHERTYPE | C2S | token=%d ethType=%x",
                    (int32_t)hProxy->token, etherType);

    req.ethertype = etherType;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DEREGISTER_MATCH_ETHTYPE,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DEREGISTER_MATCH_ETHERTYPE cmd");

    ETHFWTRACE_INFO("DEREGISTER_MATCH_ETHERTYPE | S2C | token=%d status=%d",
                    (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_registerRemoteTimer(CpswProxy_Handle hProxy,
                                      uint8_t timerId,
                                      uint8_t hwPushNum)
{
    EthRemoteCfg_RemoteTimerRegisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("REGISTER_REMOTE_TIMER | C2S | token=%d timerId=%u hwPushNum=%u",
                    (int32_t)hProxy->token, timerId, hwPushNum);

    req.hwPushNum = hwPushNum;
    req.timerId   = timerId;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_REGISTER_REMOTE_TIMER,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                      "Failed to send REGISTER_REMOTE_TIMER cmd");

    ETHFWTRACE_INFO("REGISTER_REMOTE_TIMER | S2C | token=%d status=%d",
                    (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_unregisterRemoteTimer(CpswProxy_Handle hProxy,
                                        uint8_t hwPushNum)
{
    EthRemoteCfg_RemoteTimerDeregisterReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("DEREGISTER_REMOTE_TIMER | C2S | token=%d hwPushNum=%u",
                    (int32_t)hProxy->token, hwPushNum);

    req.hwPushNum = hwPushNum;

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DEREGISTER_REMOTE_TIMER,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DEREGISTER_REMOTE_TIMER cmd");

    ETHFWTRACE_INFO("DEREGISTER_REMOTE_TIMER | S2C | token=%d status=%d",
                    (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_joinVlan(CpswProxy_Handle hProxy,
                           uint32_t flowIdxBase,
                           uint32_t flowIdxOffset,
                           const uint8_t *macAddr,
                           uint16_t vlanId)
{
    EthRemoteCfg_VlanJoinReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("JOIN_VLAN | C2S | token=%d vlanId=%u macAdd=%x:%x:%x:%x:%x:%x flowIdx=%u,%u",
                    (int32_t)hProxy->token, vlanId,
                    macAddr[0U], macAddr[1U], macAddr[2U],
                    macAddr[3U], macAddr[4U], macAddr[5U],
                    flowIdxBase, flowIdxOffset);

    req.vlanId        = vlanId;
    req.flowIdxBase   = flowIdxBase;
    req.flowIdxOffset = flowIdxOffset;
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_JOIN_VLAN,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send JOIN_VLAN cmd");

    ETHFWTRACE_INFO("JOIN_VLAN | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_leaveVlan(CpswProxy_Handle hProxy,
                            uint32_t flowIdxBase,
                            uint32_t flowIdxOffset,
                            const uint8_t *macAddr,
                            uint16_t vlanId)
{
    EthRemoteCfg_VlanLeaveReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("LEAVE_VLAN | C2S | token=%d vlanId=%u macAdd=%x:%x:%x:%x:%x:%x flowIdx=%u,%u",
                    (int32_t)hProxy->token, vlanId,
                    macAddr[0U], macAddr[1U], macAddr[2U],
                    macAddr[3U], macAddr[4U], macAddr[5U],
                    flowIdxBase, flowIdxOffset);

    req.vlanId        = vlanId;
    req.flowIdxBase   = flowIdxBase;
    req.flowIdxOffset = flowIdxOffset;
    memcpy(&req.macAddr[0U], macAddr, ETHREMOTECFG_MACADDRLEN);

    /* Send request to server and wait for response */
    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_LEAVE_VLAN,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send LEAVE_VLAN cmd");

    ETHFWTRACE_INFO("LEAVE_VLAN | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_filterAddMac(CpswProxy_Handle hProxy,
                            uint32_t flowIdxBase,
                            uint32_t flowIdxOffset,
                            const uint8_t *macAddr,
                            uint16_t vlanId)
{
    EthRemoteCfg_FilterMacAddReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status = CPSWPROXY_SOK;

    ETHFWTRACE_INFO("ADD_FILTER_MAC | C2S | token=%d macAdd=%x:%x:%x:%x:%x:%x vlanId=%u flowIdx=%u,%u",
                    (int32_t)hProxy->token,
                    macAddr[0U], macAddr[1U], macAddr[2U],
                    macAddr[3U], macAddr[4U], macAddr[5U],
                    vlanId, flowIdxBase, flowIdxOffset);

    if (!EthFwUtils_isMcastAddr(macAddr))
    {
        status = CPSWPROXY_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "MAC addr is not multicast");
    }

    if (status == CPSWPROXY_SOK)
    {
        /* Request specific params */
        req.vlanId        = vlanId;
        req.flowIdxBase   = flowIdxBase;
        req.flowIdxOffset = flowIdxOffset;
        memcpy(req.macAddr, macAddr, ETHREMOTECFG_MACADDRLEN);

        /* Send request to server and wait for response */
        status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_ADD_FILTER_MAC,
                                   &req.hdr, sizeof(req),
                                   &res.hdr, sizeof(res));
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send ADD_FILTER_MAC cmd");
    }

    ETHFWTRACE_INFO("ADD_FILTER_MAC | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_filterDelMac(CpswProxy_Handle hProxy,
                               const uint8_t *macAddr,
                               uint16_t vlanId)
{
    EthRemoteCfg_FilterMacDelReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status = CPSWPROXY_SOK;

    ETHFWTRACE_INFO("DEL_FILTER_MAC | C2S | token=%d macAdd=%x:%x:%x:%x:%x:%x vlanId=%u",
                    (int32_t)hProxy->token,
                    macAddr[0U], macAddr[1U], macAddr[2U],
                    macAddr[3U], macAddr[4U], macAddr[5U],
                    vlanId);

    if (!EthFwUtils_isMcastAddr(macAddr))
    {
        status = CPSWPROXY_EINVALIDPARAMS;
        ETHFWTRACE_ERR(status, "MAC addr is not multicast");
    }

    if (status == CPSWPROXY_SOK)
    {
        /* Request specific params */
        req.vlanId  = vlanId;
        memcpy(req.macAddr, macAddr, ETHREMOTECFG_MACADDRLEN);

        /* Send request to server and wait for response */
        status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DEL_FILTER_MAC,
                                   &req.hdr, sizeof(req),
                                   &res.hdr, sizeof(res));
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DEL_FILTER_MAC cmd");
    }

    ETHFWTRACE_INFO("DEL_FILTER_MAC | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_dumpStats(CpswProxy_Handle hProxy)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("DUMP | C2S | token=%d", (int32_t)hProxy->token);

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_DUMP,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send DUMP cmd");

    ETHFWTRACE_INFO("DUMP | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_getServerStatus(CpswProxy_Handle hProxy,
                                  EthRemoteCfg_ServerStatus *serverStatus)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_ServerStatusRes res;
    int32_t status;

    memset(&res, 0, sizeof(EthRemoteCfg_ServerStatusRes));
    ETHFWTRACE_VERBOSE("GET_SERVER_STATUS | C2S | token=%d", (int32_t)hProxy->token);

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_GET_SERVER_STATUS,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send GET_SERVER_STATUS cmd");

    if (status == CPSWPROXY_ETIMEOUT)
    {
        *serverStatus = ETHREMOTECFG_SERVERSTATUS_BAD;
    }
    else
    {
        *serverStatus = res.status;
    }

    ETHFWTRACE_VERBOSE("GET_SERVER_STATUS | S2C | token=%d status=%d serverStatus=%d", (int32_t)hProxy->token, status, (int32_t)*serverStatus);

    return status;
}

int32_t CpswProxy_allocHwPushInst(CpswProxy_Handle hProxy,
                                  uint32_t *hwPushNum)
{
    EthRemoteCfg_CommonReq req;
    EthRemoteCfg_AllocHwPushRes res;
    int32_t status;

    ETHFWTRACE_INFO("ALLOC_CPTS_HW_PUSH | C2S | token=%d", (int32_t)hProxy->token);

    memset(&res, 0, sizeof(EthRemoteCfg_AllocHwPushRes));

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_ALLOC_CPTS_HW_PUSH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send ALLOC_CPTS_HW_PUSH cmd");

    if (status == CPSWPROXY_SOK)
    {
        *hwPushNum = res.hwPushNum;
    }

    ETHFWTRACE_INFO("ALLOC_CPTS_HW_PUSH | S2C | token=%d hwPushNum=%d status=%d", (int32_t)hProxy->token, (uint32_t)res.hwPushNum, status);

    return status;
}

int32_t CpswProxy_freeHwPushInst(CpswProxy_Handle hProxy,
                                 uint32_t hwPushNum)
{
    EthRemoteCfg_FreeHwPushReq req;
    EthRemoteCfg_StatusRes res;
    int32_t status;

    ETHFWTRACE_INFO("FREE_CPTS_HW_PUSH | C2S | token=%d", (int32_t)hProxy->token);

    memset(&res, 0, sizeof(EthRemoteCfg_StatusRes));

    req.hwPushNum = hwPushNum;

    status = CpswProxy_sendCmd(hProxy, ETHREMOTECFG_CMD_FREE_CPTS_HW_PUSH,
                               &req.hdr, sizeof(req),
                               &res.hdr, sizeof(res));
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Failed to send FREE_CPTS_HW_PUSH cmd");

    ETHFWTRACE_INFO("FREE_CPTS_HW_PUSH | S2C | token=%d status=%d", (int32_t)hProxy->token, status);

    return status;
}

int32_t CpswProxy_registerNotifyCb(CpswProxy_Handle hProxy,
                                   uint32_t notifyType,
                                   CpswProxy_NotifyCbFxn cbFxn,
                                   void *cbArg)
{
    int32_t status = CPSWPROXY_EINVALIDPARAMS;

    if (notifyType < ETHREMOTECFG_NOTIFY_TYPE_COUNT)
    {
        hProxy->notifyCb[notifyType].cbFxn = cbFxn;
        hProxy->notifyCb[notifyType].cbArg = cbArg;
        status = CPSWPROXY_SOK;
    }

    return status;
}

int32_t CpswProxy_unregisterNotifyCb(CpswProxy_Handle hProxy,
                                     uint32_t notifyType)
{
    int32_t status = CPSWPROXY_EINVALIDPARAMS;

    if (notifyType < ETHREMOTECFG_NOTIFY_TYPE_COUNT)
    {
        hProxy->notifyCb[notifyType].cbFxn = NULL;
        hProxy->notifyCb[notifyType].cbArg = NULL;
        status = CPSWPROXY_SOK;
    }

    return status;
}

/* -------------------------- Client Obj Mgmt ------------------------------- */

static CpswProxy_Handle CpswProxy_getHandle(uint32_t token)
{
    CpswProxy_Handle hProxy = NULL;
    CpswProxy_Handle hProxyClient;
    uint32_t i;

    EthFwOsal_lockMutex(gCpswProxy.hMutex);

    for (i = 0U; i < ETHFW_ARRAYSIZE(gCpswProxy.clientObj); i++)
    {
        hProxyClient = &gCpswProxy.clientObj[i];

        if ((hProxyClient->inUse) &&
            (hProxyClient->token == token))
        {
            hProxy = hProxyClient;
            break;
        }
    }

    EthFwOsal_unlockMutex(gCpswProxy.hMutex);

    return hProxy;
}

static CpswProxy_Handle CpswProxy_allocHandle(void)
{
    CpswProxy_Handle hProxy = NULL;
    uint32_t i;

    EthFwOsal_lockMutex(gCpswProxy.hMutex);

    for (i = 0U; i < ETHFW_ARRAYSIZE(gCpswProxy.clientObj); i++)
    {
        if (!gCpswProxy.clientObj[i].inUse)
        {
            hProxy = &gCpswProxy.clientObj[i];
            hProxy->inUse = BTRUE;
            break;
        }
    }

    EthFwOsal_unlockMutex(gCpswProxy.hMutex);

    return hProxy;
}

static void CpswProxy_freeHandle(CpswProxy_Handle hProxy)
{
    EthFwOsal_lockMutex(gCpswProxy.hMutex);
    memset(hProxy, 0, sizeof(*hProxy));
    EthFwOsal_unlockMutex(gCpswProxy.hMutex);
}

static void CpswProxy_instanceInit(CpswProxy_Handle hProxy,
                                   const CpswProxy_Config *cfg)
{
    hProxy->cfg      = *cfg;
    hProxy->features = 0U;
    hProxy->reqId    = 0U;
    hProxy->token    = ETHREMOTECFG_TOKEN_NONE;
}

static void CpswProxy_instanceDeinit(CpswProxy_Handle hProxy)
{
    memset(&hProxy->cfg, 0, sizeof(hProxy->cfg));
    hProxy->features = 0U;
    hProxy->reqId    = 0U;
}

/* -------------------------- Command Service -------------------------------- */

static int32_t CpswProxy_initCmdSvc(CpswProxy_CmdService *svc)
{
#ifndef QNX_OS
    EthFwOsal_MailboxParams mbxParams;
#endif
    EthFwOsal_TaskParams taskParams;
    EthFwIpc_RpmsgCreateParams rpmsgParams;
    int32_t status = CPSWPROXY_SOK;

    svc->hMutex = EthFwOsal_createMutex();
    if (svc->hMutex == NULL)
    {
        status = CPSWPROXY_EALLOC;
        ETHFWTRACE_ERR(status, "Failed to create mutex");
        EthFw_assert(svc->hMutex != NULL);
    }

#ifndef QNX_OS
    if (status == CPSWPROXY_SOK)
    {
        /* Create command mailbox */
        EthFwOsal_initMailboxParams(&mbxParams);
        mbxParams.name    = (uint8_t *)"CmdSvc.CmdResMbx";
        mbxParams.size    = CPSWPROXY_CMD_RES_MBX_MSGSIZE;
        mbxParams.count   = CPSWPROXY_CMD_RES_MBX_MSGCOUNT;
        mbxParams.buf     = (void *)&svc->mbxBuf[0U];
        mbxParams.bufsize = sizeof(svc->mbxBuf);

        svc->hMbx = EthFwOsal_createMailbox(&mbxParams);
        if (svc->hMbx == NULL)
        {
            status = CPSWPROXY_EALLOC;
            ETHFWTRACE_ERR(status, "Failed to create command mailbox");
            EthFw_assert(svc->hMbx != NULL);
        }
    }
#else
    /* Create communication channel */
    if (status == CPSWPROXY_SOK)
    {
        svc->chid = ChannelCreate(0);
        if (svc->chid == -1)
        {
            status = CPSWPROXY_EALLOC;
            ETHFWTRACE_ERR(status, "Failed to create communication channel");
        }
    }

    /* Establish connection with channel */
    if (status == CPSWPROXY_SOK)
    {
        svc->coid = ConnectAttach(ND_LOCAL_NODE, 0, svc->chid, _NTO_SIDE_CHANNEL, 0);
        if (svc->coid == -1)
        {
            status = CPSWPROXY_EALLOC;
            ETHFWTRACE_ERR(status, "Failed to establish communication with channel %u", svc->chid);
        }
    }
#endif

    /* Create command RPMessage endpoint */
    if (status == CPSWPROXY_SOK)
    {
        EthFwIpc_initRpmsgParams(&rpmsgParams);
        rpmsgParams.buf            = &svc->rpMsgBuf[0U];
        rpmsgParams.bufSize        = sizeof(svc->rpMsgBuf);
        rpmsgParams.requestedEndpt = CPSWPROXY_CMD_SERVICE_END_POINT;

        svc->hRpMsg = EthFwIpc_createRpmsg(&rpmsgParams);
        if (svc->hRpMsg == NULL)
        {
            status = CPSWPROXY_EFAIL;
            ETHFWTRACE_ERR(status, "Failed to create command service endpt");
        }
        else
        {
            svc->localEndpt = rpmsgParams.requestedEndpt;
        }
    }

    /* Create command handler task */
    if (status == CPSWPROXY_SOK)
    {
        EthFwOsal_initTaskParams(&taskParams);
        taskParams.name      = CPSWPROXY_CMD_SERVICE_TASK_NAME;
        taskParams.priority  = CPSWPROXY_CMD_SERVICE_TASK_PRI;
        taskParams.arg0      = (void *)svc;
        taskParams.stack     = &svc->taskStackBuf[0U];
        taskParams.stacksize = sizeof(svc->taskStackBuf);

        svc->hTask = EthFwOsal_createTask(&CpswProxy_cmdHandlerTask, &taskParams);
        if (svc->hTask == NULL)
        {
            status = CPSWPROXY_EFAIL;
            ETHFWTRACE_ERR(status, "Failed to create command handler task");
            EthFw_assert(svc->hTask != NULL);
        }
    }

    return status;
}

static void CpswProxy_deinitCmdSvc(CpswProxy_CmdService *svc)
{
    int32_t status;

    ETHFWTRACE_DBG("Deinitializing command service");

    /* Stop task */
    svc->exitTask = BTRUE;

    /* Delete task */
    if (svc->hTask != NULL)
    {
        /* Unblock service thread if blocked on recv() */
        if (svc->hRpMsg != NULL)
        {
            EthFwIpc_unblockRpmsg(svc->hRpMsg);
        }

        /* Wait for task termination */
        while (EthFwOsal_isTaskTerminated(svc->hTask) != 1)
        {
            EthFwOsal_sleepTask(10);
        }

        ETHFWTRACE_DBG("Deleted cmd service task");
        EthFwOsal_deleteTask(&svc->hTask);
    }

    /* Delete RPMessage */
    if (svc->hRpMsg != NULL)
    {
        status = EthFwIpc_deleteRpmsg(svc->hRpMsg);
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                          "Failed to delete endpt %u", svc->localEndpt);
        svc->hRpMsg = NULL;
    }

#ifndef QNX_OS
    /* Delete command request mailbox */
    if (svc->hMbx != NULL)
    {
        EthFwOsal_deleteMailbox(svc->hMbx);
        svc->hMbx = NULL;
    }
#else
    /* Disconnect and destroy channel */
    ConnectDetach(svc->coid);
    ChannelDestroy(svc->chid);
#endif

    if (svc->hMutex != NULL)
    {
        EthFwOsal_deleteMutex(svc->hMutex);
        svc->hMutex = NULL;
    }

    ETHFWTRACE_DBG("Command service has been deinitialized");
}

int32_t CpswProxy_sendCmd(CpswProxy_Handle hProxy,
                          uint32_t reqType,
                          EthRemoteCfg_ReqHdr *req,
                          uint16_t reqLen,
                          EthRemoteCfg_ResHdr *res,
                          uint16_t resLen)
{
#ifndef QNX_OS
    int32_t mbxStatus = CPSWPROXY_SOK;
#endif
    CpswProxy_Msg msg;
    EthRemoteCfg_ResHdr *hdr = (EthRemoteCfg_ResHdr *)&msg.buf;
    uint32_t reqId;
    int32_t status = CPSWPROXY_SOK;

    EthFw_assert(hProxy != NULL);

    EthFwOsal_lockMutex(gCpswProxy.cmdSvc.hMutex);

    reqId = hProxy->reqId++;

    /* Populate command request header */
    req->common.msgType  = ETHREMOTECFG_MSGTYPE_REQUEST;
    req->common.clientId = ETHREMOTECFG_CLIENTID_RTOS;
    req->common.token    = hProxy->token;
    req->reqType         = reqType;
    req->reqId           = reqId;

    ETHFWTRACE_DBG_IF((reqType != ETHREMOTECFG_CMD_PORT_LINK_STATUS),
                      "C2S | msgType=%u token=%d clientId=%d reqType=%u reqId=%u",
                      req->common.msgType,
                      (int32_t)req->common.token,
                      req->common.clientId,
                      req->reqType,
                      req->reqId);

    /* Send command request to ETHFW */
    status = EthFwIpc_sendRpmsg(gCpswProxy.cmdSvc.hRpMsg,
                                gCpswProxy.cmdSvc.masterCoreId,
                                gCpswProxy.cmdSvc.masterEndpt,
                                gCpswProxy.cmdSvc.localEndpt,
                                (void *)req,
                                reqLen);
    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                      "Failed to send command %u request", reqType);

#ifndef QNX_OS
    /* Wait for service's response */
    if (status == CPSWPROXY_SOK)
    {
        do {
            mbxStatus = EthFwOsal_pendMailbox(gCpswProxy.cmdSvc.hMbx, (void *)&msg, gCpswProxy.cmdTimeoutInMsecs);
            if(mbxStatus == ETHFWOSAL_TIMEOUT)
            {
                status = CPSWPROXY_ETIMEOUT;
                ETHFWTRACE_ERR(status, "CMD %u response timed out from client's queue", reqType);
            }
            else if (mbxStatus != ETHFWOSAL_SUCCESS)
            {
                status = CPSWPROXY_EFAIL;
                ETHFWTRACE_ERR(status, "Failed to get cmd %u response from client's queue", reqType);
            }
            else
            {
                ETHFWTRACE_WARN_IF((hdr->resId < reqId),
                                   "Dropping late response to previous cmd (got %u, exp %u)", hdr->resId, reqId);
            }
        } while((hdr->resId < reqId));
    }
#else
    /* Send message through channel */
    if (status == CPSWPROXY_SOK)
    {
        status = MsgSend(gCpswProxy.cmdSvc.coid, &reqType, sizeof(reqType), &msg, sizeof(msg));
        ETHFWTRACE_ERR_IF((status == -1), status,
                          "Failed to send cmd %u message to channel", reqType);
    }
#endif

    /* Check command response length and copy to caller's buffer */
    if (status == CPSWPROXY_SOK)
    {
        if (msg.len != resLen)
        {
            status = CPSWPROXY_EUNEXPECTED;
            ETHFWTRACE_ERR(status, "Command %u response length mismatch (exp %u, got %u)",
                           reqType, resLen, msg.len);
        }
        else
        {
            /* Copy response payload into caller's buffer */
            if (status == CPSWPROXY_SOK)
            {
                memcpy(res, &msg.buf[0], msg.len);
            }
        }
    }

    /* Check if transaction id matches for request and response */
    if ((status == CPSWPROXY_SOK) &&
        (hdr->resId > reqId))
    {
        status = CPSWPROXY_EUNEXPECTED;
        ETHFWTRACE_ERR(status, "Got wrong resId (exp: %u, got: %u)", reqId, hdr->resId);
    }

    /* Return status from the command itself */
    if (status == CPSWPROXY_SOK)
    {
        status = hdr->status;
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status, "Cmd %u failed on remote side", reqType);
    }

    EthFwOsal_unlockMutex(gCpswProxy.cmdSvc.hMutex);

    return status;
}

static void CpswProxy_cmdHandlerTask(void *arg0)
{
    CpswProxy_CmdService *svc = (CpswProxy_CmdService *)arg0;
    CpswProxy_Msg msg;
    EthRemoteCfg_MsgHdr *common = (EthRemoteCfg_MsgHdr *)&msg.buf;
    uint32_t remoteProcId;
    uint32_t remoteEndpt;
    uint16_t len;
    int32_t status = CPSWPROXY_SOK;

    while (!svc->exitTask)
    {
        /* Wait for new messages from ETHFW */
        status = EthFwIpc_recvRpmsg(svc->hRpMsg,
                                    (void *)msg.buf,
                                    &len,
                                    &remoteEndpt,
                                    &remoteProcId,
                                    ETHFWIPC_WAIT_FOREVER);
        if (status != CPSWPROXY_SOK)
        {
            ETHFWTRACE_ERR(status, "Failed to receive IPC message");
        }
        else
        {
            if ((svc->masterCoreId != remoteProcId) ||
                (svc->masterEndpt != remoteEndpt))
            {
                status = CPSWPROXY_EUNEXPECTED;
                ETHFWTRACE_ERR(status, "Unexpected source core/endpt (exp %u.%u, got %u.%u)",
                               svc->masterCoreId, svc->masterEndpt,
                               remoteProcId, remoteEndpt);
            }
        }

        /* Call command response or notification handler */
        if (status == CPSWPROXY_SOK)
        {
            msg.status = status;
            msg.len = len;

            switch (common->msgType)
            {
                case ETHREMOTECFG_MSGTYPE_RESPONSE:
                {
#if (ETHFW_CFG_TRACE_LEVEL >= ETHFW_CFG_TRACE_LEVEL_DEBUG)
                    EthRemoteCfg_ResHdr *resHdr = (EthRemoteCfg_ResHdr *)msg.buf;
#endif

                    ETHFWTRACE_DBG_IF((resHdr->resType != ETHREMOTECFG_CMD_PORT_LINK_STATUS),
                                      "S2C | msgType=%u token=%d clientId=%u resType=%u resId=%u "
                                      "status=%d len=%u (%u.%u->%u.%u)",
                                      resHdr->common.msgType,
                                      (int32_t)resHdr->common.token,
                                      resHdr->common.clientId,
                                      resHdr->resType,
                                      resHdr->resId,
                                      resHdr->status,
                                      len,
                                      remoteProcId, remoteEndpt,
                                      Ipc_getCoreId(), svc->localEndpt);

                    status = CpswProxy_cmdHandler(svc, &msg);
                    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                                      "Failed to handle command");
                    break;
                }
                case ETHREMOTECFG_MSGTYPE_NOTIFY:
                {
                    EthRemoteCfg_NotifyHdr *notifyHdr = (EthRemoteCfg_NotifyHdr *)msg.buf;

                    ETHFWTRACE_DBG("S2C | msgType=%u token=%d clientId=%u notifyType=%u len=%u (%u.%u->%u.%u)",
                                   notifyHdr->common.msgType,
                                   (int32_t)notifyHdr->common.token,
                                   notifyHdr->common.clientId,
                                   notifyHdr->notifyType,
                                   len,
                                   remoteProcId, remoteEndpt,
                                   Ipc_getCoreId(), svc->localEndpt);

                    status = CpswProxy_notifyHandler(notifyHdr, len);
                    ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                                      "Failed to handle notification");
                    break;
                }
                default:
                {
                    status = CPSWPROXY_EUNEXPECTED;
                    ETHFWTRACE_ERR(status, "Invalid message type %u received %u.%u",
                                   common->msgType, remoteProcId, remoteEndpt);
                    break;
                }
            }
        }
    }
}

static int32_t CpswProxy_cmdHandler(CpswProxy_CmdService *svc,
                                    CpswProxy_Msg *msg)
{
    EthRemoteCfg_ResHdr *hdr = (EthRemoteCfg_ResHdr *)msg->buf;
#ifndef QNX_OS
    int32_t mbxStatus = CPSWPROXY_SOK;
#else
    int rcvid;
    uint32_t reqType;
#endif
    int32_t status = CPSWPROXY_SOK;

    /* Send command response back to caller */
#ifndef QNX_OS
    mbxStatus = EthFwOsal_postMailbox(svc->hMbx, (void *)msg, ETHFWOSAL_WAIT_FOREVER);
    if (mbxStatus != CPSWPROXY_SOK)
    {
        status = CPSWPROXY_EUNEXPECTED;
        ETHFWTRACE_ERR(status, "Failed to send cmd %u response via mailbox", hdr->resType);
    }
#else
    rcvid = MsgReceive(svc->chid, &reqType, sizeof(reqType), NULL);
    if (rcvid == -1)
    {
        status = CPSWPROXY_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to get cmd %u response from client's queue", reqType);
    }

    ETHFWTRACE_WARN_IF((reqType != hdr->resType), "Got unexpected response for cmd %u, exp %u",
                       hdr->resType, reqType);

    if (status == CPSWPROXY_SOK)
    {
        status = MsgReply(rcvid, EOK, msg, sizeof(*msg));
        ETHFWTRACE_ERR_IF((status == -1), status,
                          "Failed to send reply through channel, rcvid %u", rcvid);
    }
#endif

    return status;
}

/* -------------------------- Notify Service -------------------------------- */

static int32_t CpswProxy_initNotifySvc(CpswProxy_NotifyService *svc)
{
    EthFwIpc_RpmsgCreateParams rpmsgParams;
    EthFwOsal_TaskParams taskParams;
    int32_t status = CPSWPROXY_SOK;

    /* Create RPMessage to be used for S2C notification */
    EthFwIpc_initRpmsgParams(&rpmsgParams);
    rpmsgParams.requestedEndpt = ETHREMOTECFG_NOTIFY_SERVICE_ENDPT_ID;
    rpmsgParams.buf            = &svc->rpMsgBuf[0U];
    rpmsgParams.bufSize        = sizeof(svc->rpMsgBuf);
    rpmsgParams.numBufs        = ETHREMOTECFG_IPC_NUM_MSG_BUFS;

    svc->hRpMsg = EthFwIpc_createRpmsg(&rpmsgParams);
    if (svc->hRpMsg == NULL)
    {
        status = CPSWPROXY_EFAIL;
        ETHFWTRACE_ERR(status, "Failed to create endpt %u", rpmsgParams.requestedEndpt);
    }
    else
    {
        svc->localEndpt = rpmsgParams.requestedEndpt;
    }

    /* Create notify handler task */
    if (status == CPSWPROXY_SOK)
    {
        EthFwOsal_initTaskParams(&taskParams);
        taskParams.name      = CPSWPROXY_NOTIFY_SERVICE_TASK_NAME;
        taskParams.priority  = CPSWPROXY_NOTIFY_SERVICE_TASK_PRI;
        taskParams.arg0      = (void *)svc;
        taskParams.stack     = &svc->taskStack[0U];
        taskParams.stacksize = sizeof(svc->taskStack);

        svc->hTask = EthFwOsal_createTask(&CpswProxy_notifySvcTask, &taskParams);
        if (svc->hTask == NULL)
        {
            status = CPSWPROXY_EFAIL;
            ETHFWTRACE_ERR(status, "Failed to create notify handler task");
            EthFw_assert(svc->hTask != NULL);
        }
    }

    return status;
}

static void CpswProxy_deinitNotifySvc(CpswProxy_NotifyService *svc)
{
    int32_t status;

    ETHFWTRACE_DBG("Deinitializing notify service");

    /* Stop task */
    svc->exitTask = BTRUE;

    /* Delete task */
    if (svc->hTask != NULL)
    {
        /* Unblock service thread if blocked on recv() */
        if (svc->hRpMsg != NULL)
        {
            EthFwIpc_unblockRpmsg(svc->hRpMsg);
        }

        /* Wait for task termination */
        while (EthFwOsal_isTaskTerminated(svc->hTask) != 1)
        {
            EthFwOsal_sleepTask(10);
        }

        ETHFWTRACE_DBG("Deleted cmd service task");
        EthFwOsal_deleteTask(&svc->hTask);
    }

    /* Delete RPMessage */
    if (svc->hRpMsg != NULL)
    {
        status = EthFwIpc_deleteRpmsg(svc->hRpMsg);
        ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                          "Failed to delete endpt %u", svc->localEndpt);
        svc->hRpMsg = NULL;
    }

    ETHFWTRACE_DBG("Notify service has been deinitialized");
}

static void CpswProxy_notifySvcTask(void *arg0)
{
    CpswProxy_NotifyService *svc = (CpswProxy_NotifyService *)arg0;
    uint64_t msgBuf[ETHREMOTECFG_IPC_MSG_SIZE / sizeof(uint64_t)];
    EthRemoteCfg_NotifyHdr *notifyHdr = (EthRemoteCfg_NotifyHdr *)msgBuf;
    uint32_t remoteProcId;
    uint32_t remoteEndpt;
    uint16_t len;
    int32_t status = CPSWPROXY_SOK;

    while (!svc->exitTask)
    {
        /* Wait for new messages from ETHFW */
        status = EthFwIpc_recvRpmsg(svc->hRpMsg,
                                    (void *)msgBuf,
                                    &len,
                                    &remoteEndpt,
                                    &remoteProcId,
                                    ETHFWIPC_WAIT_FOREVER);
        if (status != CPSWPROXY_SOK)
        {
            ETHFWTRACE_ERR(status, "Failed to receive IPC message");
        }
        else
        {
            if ((svc->masterCoreId != remoteProcId) ||
                (svc->masterEndpt != remoteEndpt))
            {
                status = CPSWPROXY_EUNEXPECTED;
                ETHFWTRACE_ERR(status, "Unexpected source core/endpt (exp %u.%u, got %u.%u)",
                               svc->masterCoreId, svc->masterEndpt,
                               remoteProcId, remoteEndpt);
            }
        }

        /* Call notification handler */
        if (status == CPSWPROXY_SOK)
        {
            ETHFWTRACE_DBG("S2C | msgType=%u token=%d clientId=%u notifyType=%u len=%u (%u.%u->%u.%u)",
                           notifyHdr->common.msgType,
                           (int32_t)notifyHdr->common.token,
                           notifyHdr->common.clientId,
                           notifyHdr->notifyType,
                           len,
                           remoteProcId, remoteEndpt,
                           Ipc_getCoreId(), svc->localEndpt);

            status = CpswProxy_notifyHandler(notifyHdr, len);
            ETHFWTRACE_ERR_IF((status != CPSWPROXY_SOK), status,
                              "Failed to handle notification");
        }
    }
}

static int32_t CpswProxy_notifyHandler(EthRemoteCfg_NotifyHdr *hdr,
                                       uint16_t msgLen)
{
    CpswProxy_Handle hProxy = NULL;
    TsCouplerClient_HwPushNotifyParams hwPushParams;
    CpswProxy_NotifyCbFxn cbFxn;
    void *notifyArg = NULL;
    void *cbArg = NULL;
    uint32_t notifyType = hdr->notifyType;
    int32_t status = CPSWPROXY_SOK;

    hProxy = CpswProxy_getHandle(hdr->common.token);

    /* Process received notification */
    /* Check if this is a virtual netif based notify message */
    if (hProxy != NULL)
    {
        switch (notifyType)
        {
            case ETHREMOTECFG_NOTIFY_HWERROR:
            case ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE:
            {
                /* No notify specific params */
                notifyArg = NULL;
                break;
            }
            default:
            {
                status = CPSWPROXY_EBADARGS;
                /* One might get this error with notify type: ETHREMOTECFG_NOTIFY_HWPUSH
                 * when client gets a HW push notify before both the virtual ports attach - Fixme
                 */
                ETHFWTRACE_ERR(status, "Unknown notify type %u", notifyType);
                break;
            }
        }

        /* Call app's notify callback if one is registered */
        if (status == CPSWPROXY_SOK)
        {
            cbFxn = hProxy->notifyCb[notifyType].cbFxn;
            cbArg = hProxy->notifyCb[notifyType].cbArg;
            if (cbFxn != NULL)
            {
                cbFxn(notifyType, notifyArg, cbArg);
            }
        }
    }
    /* Check if this is a core based notify message */
    else
    {
        switch (notifyType)
        {
            case ETHREMOTECFG_NOTIFY_HWPUSH:
            {
                EthRemoteCfg_NotifyServiceHwPushMsg *hwPushMsg = (EthRemoteCfg_NotifyServiceHwPushMsg *)hdr;

                /* Check notify size */
                if (msgLen != sizeof(EthRemoteCfg_NotifyServiceHwPushMsg))
                {
                    status = CPSWPROXY_EINVALIDPARAMS;
                    ETHFWTRACE_ERR(status, "Notify %u param size mismatch (exp %u, got %u)",
                                   notifyType, sizeof(EthRemoteCfg_NotifyServiceHwPushMsg), msgLen);
                }
                else
                {
                    /* Populate HW push specific callback parameters */
                    hwPushParams.hwPushNum = hwPushMsg->hwPushNum;
                    hwPushParams.timestamp = hwPushMsg->timeStamp;
                    notifyArg = &hwPushParams;
                }
                break;
            }
            default:
            {
                status = CPSWPROXY_EBADARGS;
                ETHFWTRACE_ERR(status, "Unknown notify type %u", notifyType);
                break;
            }
        }
        /* Call app's notify callback if one is registered */
        if (status == CPSWPROXY_SOK)
        {
            cbFxn = gCpswProxy.tsNotifyCb.cbFxn;
            cbArg = gCpswProxy.tsNotifyCb.cbArg;
            if (cbFxn != NULL)
            {
                cbFxn(notifyType, notifyArg, cbArg);
            }
        }
    }

    return status;
}
