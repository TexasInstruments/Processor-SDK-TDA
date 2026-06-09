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
#define ETHFWTRACE_MOD_ID 0x701
#define ETHFWTRACE_MOD_NAME "RTOS-App"

#if defined(__KLOCWORK__)
#include <stdlib.h>
#endif

#include <cslr_gtc.h>
#include <apps/ipc_cfg/app_ipc_rsctable.h>

#include <ethremotecfg/client/include/cpsw_proxy.h>
#include <ethremotecfg/client/include/ts_coupler_client.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>

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

#define System_printf printf
#define System_vprintf vprintf

/*! Heartbeat poll period for CPSW proxy. */
#define CPSW_REMOTE_APP_POLL_PERIOD_MS        (2000U)

/*! CPSW proxy command response timeout. */
#if defined(SOC_J722S)
#define CPSW_REMOTE_APP_CMD_TIMEOUT_MS        (2500U) //increasing the j722s timeout to 2.5s as the time range for mailbox_pend varies from ~0.5s-1.5s during ipc
#else
#define CPSW_REMOTE_APP_CMD_TIMEOUT_MS        (1000U)
#endif

#if defined(SOC_J784S4) || defined(SOC_J7200)
#define CPSW_REMOTE_APP_TIMESYNC_TIMER_IDX       (14U)
#elif defined(SOC_J721E)
/*
 * The interrupt events of Main domain's DM Timer instance 12  - 19
 * are directly connected to the MAIN Pulsar VIMs.
 * This translation matches to sysbios implementation, where id=0 is
 * for dmTimer12, id =1 is for dmTimer13, id = 2 is for dmTimer14 etc.
 */
#define CPSW_REMOTE_APP_TIMESYNC_TIMER_IDX       (3U)
#endif

#define CPSW_REMOTE_APP_TIMESYNC_TIMERPERIOD_MS  (1000U) //1 second

#define CPSW_REMOTE_APP_GTC_PUSHEVT_BIT_SEL      (27U) // ~1 second

#define VQ_TIMEOUT              (100)
#define VQ_BUF_SIZE             (2048)

#if defined(SOC_J721E)
#define IPC_VRING_MEM_SIZE                    (32U * 1024U * 1024U)
#elif defined(SOC_J7200)
#define IPC_VRING_MEM_SIZE                    (8U * 1024U * 1024U)
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
#define IPC_VRING_MEM_SIZE                    (48U * 1024U * 1024U)
#elif defined(SOC_AM62PX) || defined(SOC_AM62DX) || defined(SOC_AM62AX)  || defined(SOC_J722S)
#define IPC_VRING_MEM_SIZE                    (0x2000U)
#else
#error "Unsupported device"
#endif

#define CPSW_REMOTE_APP_IPC_RPC_MSG_SIZE      (496U + 32U)
#define CPSW_REMOTE_APP_IPC_NUM_RPMSG_BUFS    (256U)
#define CPSW_REMOTE_APP_IPC_RPMSG_OBJ_SIZE    (256U)
#define CPSW_REMOTE_APP_IPC_DATA_SIZE         (CPSW_REMOTE_APP_IPC_RPC_MSG_SIZE * \
                                               CPSW_REMOTE_APP_IPC_NUM_RPMSG_BUFS + \
                                               CPSW_REMOTE_APP_IPC_RPMSG_OBJ_SIZE)

#if defined(SAFERTOS)
#define ETHAPP_LWIP_TASK_STACKSIZE              (16U * 1024U)
#define ETHAPP_LWIP_TASK_STACKALIGN             ETHAPP_LWIP_TASK_STACKSIZE
#define ETHAPP_IPC_TASK_STACKALIGN              ETHAPP_IPC_TASK_STACKSIZE
#else
#define ETHAPP_LWIP_TASK_STACKSIZE              (16U * 1024U)
#define ETHAPP_LWIP_TASK_STACKALIGN             (32U)
#define ETHAPP_IPC_TASK_STACKALIGN              (8192U)
#endif

#define ETHAPP_IPC_TASK_STACKSIZE               (0x2000U)

static uint8_t gEthAppLwipStackBuf[ETHAPP_LWIP_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__((aligned(ETHAPP_LWIP_TASK_STACKALIGN)));

#if !defined(MCU_PLUS_SDK)
static uint8_t g_initTaskStackBuf[ETHAPP_IPC_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(ETHAPP_IPC_TASK_STACKALIGN)))
;

static uint8_t g_vdevMonStackBuf[ETHAPP_IPC_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(ETHAPP_IPC_TASK_STACKALIGN)))
;
#endif

static uint8_t ctrlTaskBuf[ETHAPP_IPC_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(ETHAPP_IPC_TASK_STACKALIGN)))
;

#if defined(SAFERTOS)
static sys_sem_t gEthApp_lwipMainSemObj;
#endif

static uint8_t sysVqBuf[VQ_BUF_SIZE]  __attribute__ ((section("ipc_data_buffer"), aligned(8)));
static uint8_t gCntrlBuf[CPSW_REMOTE_APP_IPC_DATA_SIZE] __attribute__ ((section("ipc_data_buffer"), aligned(8)));

static uint8_t g_vringMemBuf[IPC_VRING_MEM_SIZE] __attribute__ ((section(".bss:ipc_vring_mem"), aligned(8192)));

#if defined (ETHFW_RTOS_MCU3_0)
static uint32_t gRemoteProc[] =
{
#if defined(SOC_J721E)
    IPC_MPU1_0, IPC_MCU1_0, IPC_MCU1_1, IPC_MCU2_0,
    IPC_MCU2_1, IPC_MCU3_1, IPC_C66X_1, IPC_C66X_2,
    IPC_C7X_1,
#elif defined(SOC_J784S4)
    IPC_MPU1_0, IPC_MCU1_0, IPC_MCU1_1, IPC_MCU2_0,
    IPC_MCU2_1, IPC_MCU3_1, IPC_MCU4_0, IPC_MCU4_1,
    IPC_C7X_1,  IPC_C7X_2,  IPC_C7X_3,  IPC_C7X_4,
#elif defined(SOC_J742S2)
    IPC_MPU1_0, IPC_MCU1_0, IPC_MCU1_1, IPC_MCU2_0,
    IPC_MCU2_1, IPC_MCU3_1, IPC_MCU4_0, IPC_MCU4_1,
    IPC_C7X_1,  IPC_C7X_2,  IPC_C7X_3,
#endif
};
#else
static uint32_t gRemoteProc[] =
{
#if defined(SOC_J721E)
    IPC_MPU1_0, IPC_MCU1_0, IPC_MCU1_1, IPC_MCU2_0,
    IPC_MCU3_0, IPC_MCU3_1, IPC_C66X_1, IPC_C66X_2,
    IPC_C7X_1,
#elif defined(SOC_J7200)
    IPC_MPU1_0, IPC_MCU1_0, IPC_MCU1_1, IPC_MCU2_0,
#elif defined(SOC_J784S4)
    IPC_MPU1_0, IPC_MCU1_0, IPC_MCU1_1, IPC_MCU2_0,
    IPC_MCU3_0, IPC_MCU3_1, IPC_MCU4_0, IPC_MCU4_1,
    IPC_C7X_1,  IPC_C7X_2,  IPC_C7X_3,  IPC_C7X_4,
#elif defined(SOC_J742S2)
    IPC_MPU1_0, IPC_MCU1_0, IPC_MCU1_1, IPC_MCU2_0,
    IPC_MCU3_0, IPC_MCU3_1, IPC_MCU4_0, IPC_MCU4_1,
    IPC_C7X_1,  IPC_C7X_2,  IPC_C7X_3,
#elif defined(SOC_AM62PX) || defined(SOC_AM62DX) || defined(SOC_AM62AX)  || defined(SOC_J722S)
    IPC_MCU2_0,
#endif
};
#endif

static uint32_t gNumRemoteProc = sizeof(gRemoteProc) / sizeof(uint32_t);


typedef struct CpswRemoteApp_Obj_s
{
    /* Enet peripheral type */
    Enet_Type enetType;

    /* Enet peripheral instance id */
    uint32_t instId;

    /* Core id used for Enet LLD APIs */
    uint32_t coreId;
} CpswRemoteApp_Obj;

void appLogPrintf(const char *format, ...);

/* Trace configuration */
static EthFwTrace_Cfg gRemoteApp_traceCfg =
{
    .print        = appLogPrintf,
    .traceTsFunc  = NULL,
    .extTraceFunc = NULL,
};

CpswRemoteApp_Obj gRemoteAppObj =
{
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
    .enetType         = ENET_CPSW_9G,
    .instId           = 0U,
#elif defined(SOC_J7200)
    .enetType         = ENET_CPSW_5G,
    .instId           = 0U,
#elif defined(SOC_AM62PX) || defined(SOC_AM62DX) || defined(SOC_AM62AX)  || defined(SOC_J722S)
    .enetType         = ENET_CPSW_3G,
    .instId           = 0U,
#endif
};

static void EthApp_waitForDebugger(void);

static uint64_t CpswRemoteApp_virtToPhysFxn(const void *virtAddr,
                                            void *appData);

static void *CpswRemoteApp_physToVirtFxn(uint64_t phyAddr,
                                         void *appData);

static int32_t CpswRemoteApp_openEnet(void);

char *VerStr = "LWIP CPSW Example";

static void EthApp_lwipMain(void *a0);


void CpswRemoteApp_initTask(void* a0);

extern void CpswRemoteApp_initVirtNetif(Enet_Type enetType, uint32_t instId);

extern void EthApp_initLwip(void *arg);

void localAssert(bool cond)
{
#if defined(__KLOCWORK__)
    if (!cond)
    {
        abort();
    }
#endif
}

void appLogPrintf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    System_vprintf(format, args);
    va_end(args);
}

static void CpswRemoteApp_ipcPrint(const char *str)
{
    appLogPrintf("%s", str);
    return;
}

static void rpmsg_vdevMonitorFxn(void* arg0)
{
    int32_t status = ETHFW_SOK;

    /* Wait for Linux VDev ready... */
    EthFwIpc_isRemoteReady(IPC_MPU1_0, ETHFWOSAL_WAIT_FOREVER);

    /* Create the VRing for Linux if not created ... */
    if (status == ETHFW_SOK)
    {
        status = EthFwIpc_lateInit(IPC_MPU1_0);
        ETHFWTRACE_ERR_IF((status != ETHFW_SOK), status, "RPMessage_lateInit failed");
    }

    return;
}

static void CpswRemoteApp_hbStatus(EthRemoteCfg_ServerStatus serverStatus,
                                   void *cbArg)
{
    switch (serverStatus)
    {
        case ETHREMOTECFG_SERVERSTATUS_UNINIT:
            ETHFWTRACE_INFO("Server status: server has not been initialized");
            break;
        case ETHREMOTECFG_SERVERSTATUS_READY:
            ETHFWTRACE_VERBOSE("Server status: server is running normally");
            break;
        case ETHREMOTECFG_SERVERSTATUS_RECOVERY:
            ETHFWTRACE_INFO("Server status: Ethernet switch is under recovery");
            break;
        case ETHREMOTECFG_SERVERSTATUS_BAD:
            ETHFWTRACE_ERR(serverStatus, "Server status: Not responding, possibly in bad/crashed state");
            break;
        default:
            ETHFWTRACE_ERR(serverStatus, "Server status: Unexpected status: %d", (int32_t)serverStatus);
            break;
    }
}

void printDevInfo(EthRemoteCfg_DeviceData *ethDevData)
{
    char *tf[] = {"false", "true"};

    ETHFWTRACE_INFO("ETHFW Version:%2d.%2d.%2d",
                    ethDevData->fwVer.major,
                    ethDevData->fwVer.minor,
                    ethDevData->fwVer.rev);

    ETHFWTRACE_INFO("ETHFW Build Date (YYYY/MMM/DD):%c%c%c%c/%c%c%c/%c%c",
                    ethDevData->fwVer.year[0],
                    ethDevData->fwVer.year[1],
                    ethDevData->fwVer.year[2],
                    ethDevData->fwVer.year[3],
                    ethDevData->fwVer.month[0],
                    ethDevData->fwVer.month[1],
                    ethDevData->fwVer.month[2],
                    ethDevData->fwVer.date[0],
                    ethDevData->fwVer.date[1]);

    ETHFWTRACE_INFO("ETHFW Commit SHA:%c%c%c%c%c%c%c%c",
                    ethDevData->fwVer.commitHash[0],
                    ethDevData->fwVer.commitHash[1],
                    ethDevData->fwVer.commitHash[2],
                    ethDevData->fwVer.commitHash[3],
                    ethDevData->fwVer.commitHash[4],
                    ethDevData->fwVer.commitHash[5],
                    ethDevData->fwVer.commitHash[6],
                    ethDevData->fwVer.commitHash[7]);

    ETHFWTRACE_INFO("ETHFW PermissionFlag:0x%llx, UART Connected:%s,UART Id:%d",
                    ethDevData->permissionFlags,
                    tf[ethDevData->uartConnected],
                    ethDevData->uartId);
}

void CpswRemoteApp_initTask(void* a0)
{
    EthFwOsal_TaskParams params;
    uint32_t numProc = gNumRemoteProc;
    uint32_t selfProcId = EnetSoc_getCoreId();
    CpswProxy_initParams initParams;
    int32_t status;

#if defined(MCU_PLUS_SDK)
    EthApp_waitForDebugger();
    EthFwOsal_init();
#endif

    /* Initialize ETHFW Trace with INFO log level and higher */
    EthFwTrace_init(&gRemoteApp_traceCfg);
    EthFwTrace_setLevel(ETHFW_TRACE_INFO);

    /* Step 1: Initialize the IPC */
    status = EthFwIpc_init(selfProcId,
                           numProc,
                           &gRemoteProc[0],
                           &CpswRemoteApp_ipcPrint);
#if !defined(MCU_PLUS_SDK)
#if !defined(A72_QNX_OS)
    if (status == ETHFW_SOK)
    {
        status = Ipc_loadResourceTable(appGetIpcResourceTable());
    }
#else
    ETHFWTRACE_INFO("Skipping Ipc_loadResourceTable for QNX (core : %s)\r\n", Ipc_mpGetSelfName());
#endif
#endif

    if (status == ETHFW_SOK)
    {
        status = EthFwIpc_initVirtIO(numProc, &sysVqBuf, &g_vringMemBuf,
                                     IPC_VRING_MEM_SIZE);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to init virtIO");
    }

    if (status == ETHFW_SOK)
    {
        status = EthFwIpc_initRpmsg(&gCntrlBuf, &ctrlTaskBuf, selfProcId);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to init RPMsg");
    }

#if !defined(MCU_PLUS_SDK)
    /* Step 4: Create RPMessage monitor task */
    EthFwOsal_initTaskParams(&params);
    params.priority = 3;
    params.stack = &g_vdevMonStackBuf[0];
    params.stacksize = sizeof(g_vdevMonStackBuf);

    EthFwOsal_createTask(&rpmsg_vdevMonitorFxn, &params);
#endif

    /* Step 5: Start Cpsw Proxy */
    /* Initialize config to default values, this is NOT mandatory */
    CpswProxy_initConfig(&initParams);

    /* Update App specific params. */
    initParams.hbPeriodInMsecs = CPSW_REMOTE_APP_POLL_PERIOD_MS;

    /* Pass 0U for blocking ProxyClient forever for remote command response */
    initParams.cmdTimeoutInMsecs = CPSW_REMOTE_APP_CMD_TIMEOUT_MS;
    initParams.hbNotifyCb.cbFxn = CpswRemoteApp_hbStatus;

#if defined(ETHFW_MTS_SUPPORT)
    /* Update timesync specific callback arguments */
    initParams.tsNotifyCb.cbFxn = TsCouplerClient_HwPushNotifyFxn;
#endif

    CpswProxy_init(&initParams);

    /* Step 5a. Wait for remote_device to be initialized on the server side */
    do
    {
        status = CpswProxy_connect();
    }
    while (status != ETHFW_SOK);

    CpswRemoteApp_initVirtNetif(gRemoteAppObj.enetType, gRemoteAppObj.instId);

    /* Step 6: Initialize lwIP */
    EthFwOsal_initTaskParams(&params);
    params.priority  = DEFAULT_THREAD_PRIO;
    params.stack     = &gEthAppLwipStackBuf[0];
    params.stacksize = sizeof(gEthAppLwipStackBuf);
    params.name      = "lwIP main loop";
#if defined(SAFERTOS)
    params.userData  = &gEthApp_lwipMainSemObj;
#endif

    EthFwOsal_createTask(&EthApp_lwipMain, &params);

#if defined(ETHFW_MTS_SUPPORT)
    TsCouplerClient_initParams initTsClientParams;
    TsCouplerClient_Cfg tsClientCfg;

    memset(&tsClientCfg, 0, sizeof(tsClientCfg));

    initTsClientParams.timerType = TS_COUPLER_CLIENT_TIMER_TYPE_GPTIMER;
    TsCouplerClient_init(&initTsClientParams);

    /* To DO: ETHFW-3048 - There is tight couple of CPSW proxy handle and virtnetif_lwipif.c
     * which needs to be cleaned up, main should have handle of CPSW proxy instead
     * and should be the one giving it to virtnetif_lwipif.c or other sub modules on
     * client side. Delay is needed to get attach being done by virtnetif_lwipif.c
     * before calling CPSW proxy commands that requires valid handles. */
    EthFwOsal_sleepTask(2000U);

    /* Alloc the CPTS HW push instance */
    status = TsCouplerClient_allocHwPushInst(&tsClientCfg.hwPushNum);
    localAssert(status == ENET_SOK);

    tsClientCfg.timerIdx       = CPSW_REMOTE_APP_TIMESYNC_TIMER_IDX;
    tsClientCfg.periodinMs     = CPSW_REMOTE_APP_TIMESYNC_TIMERPERIOD_MS;

    /* App must fill the corresponding Timesync interrupt router input for the applicable timerType. */
    if (initTsClientParams.timerType == TS_COUPLER_CLIENT_TIMER_TYPE_GPTIMER)
    {
        tsClientCfg.tsRouterTntrId = CSLR_TIMESYNC_INTRTR0_IN_TIMER14_TIMER_PWM_0;
    }
    else
    {
        tsClientCfg.pushEvtVal     = CPSW_REMOTE_APP_GTC_PUSHEVT_BIT_SEL;
        tsClientCfg.tsRouterTntrId = CSLR_TIMESYNC_INTRTR0_IN_GTC0_GTC_PUSH_EVENT_0;
    }

    TsCouplerClient_start(&tsClientCfg);
#endif
}

#if !defined(MCU_PLUS_SDK)
int main(void)
{
    EthFwOsal_TaskHandle hRemoteAppTask;
    EthFwOsal_TaskParams taskParams;
    int32_t status;

    OS_init();

    /* Wait for debugger to attach (disabled by default) */
    EthApp_waitForDebugger();
    
    EthFwOsal_init();

    /* Init Enet LLD and open Enet DMA */
    status = CpswRemoteApp_openEnet();
    if (status != ENET_SOK)
    {
        ETHFWTRACE_ERR(status, "Failed to open Enet LLD");
        localAssert(status == ENET_SOK);
    }

    EthFwOsal_initTaskParams(&taskParams);
    taskParams.priority = 2;
    taskParams.stack = &g_initTaskStackBuf[0];
    taskParams.stacksize = sizeof(g_initTaskStackBuf);

    hRemoteAppTask = EthFwOsal_createTask(&CpswRemoteApp_initTask, &taskParams);

    if (NULL == hRemoteAppTask)
    {
        OS_stop();
    }

    OS_start();    /* does not return */

    return(0);
}
#endif

static void EthApp_waitForDebugger(void)
{
    /* Set ccsHaltFlag to 1 for halting core for CCS connection */
    volatile uint32_t ccsHaltFlag = 0U;

    while (ccsHaltFlag);
}

static uint64_t CpswRemoteApp_virtToPhysFxn(const void *virtAddr,
                                            void *appData)
{
    return ((uint64_t)virtAddr);
}

static void *CpswRemoteApp_physToVirtFxn(uint64_t phyAddr,
                                         void *appData)
{
#if defined(__aarch64__)
    uint64_t temp = phyAddr;
#else
    /* R5 is 32-bit machine, need to truncate to avoid void * typecast error */
    uint32_t temp = (uint32_t)phyAddr;
#endif

    return ((void *)temp);
}

static int32_t CpswRemoteApp_openEnet(void)
{
    EnetUtils_Cfg utilsPrms;
    int32_t status = ENET_SOK;

    /* Init Enet LLD utils */
    utilsPrms.print      = (Enet_Print)System_printf;
    utilsPrms.physToVirt = &CpswRemoteApp_physToVirtFxn;
    utilsPrms.virtToPhys = &CpswRemoteApp_virtToPhysFxn;
    Enet_init(&utilsPrms);

    gRemoteAppObj.coreId   = EnetSoc_getCoreId();

    return status;
}

static void EthApp_lwipMain(void *a0)
{
    err_t err;
    sys_sem_t initSem;

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

    EthFwOsal_exitTask();
}

/* ToDo: Handle DHCP start here */
