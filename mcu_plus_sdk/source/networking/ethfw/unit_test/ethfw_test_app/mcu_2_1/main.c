/*
 *
 * Copyright (c) 2024 Texas Instruments Incorporated
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
#define ETHFWTRACE_MOD_ID 0x801
#define ETHFWTRACE_MOD_NAME "Client-Test-App"

#if defined(__KLOCWORK__)
#include <stdlib.h>
#endif
#include <stdio.h>

#include <cslr_gtc.h>

#include <ethremotecfg/client/include/cpsw_proxy.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_abstract/ethfw_ipc.h>

#include <enet.h>
#include <include/per/cpsw.h>
#include <utils/include/enet_apputils.h>
#include <utils/include/enet_appmemutils_cfg.h>
#include <utils/include/enet_appmemutils.h>

#if !defined(MCU_PLUS_SDK)
#include <ti/drv/udma/udma.h>
#include <ti/drv/uart/UART_stdio.h>
#endif

#include <apps/ipc_cfg/app_ipc_rsctable.h>

#include <ethfw_test_cases.h>

#define System_printf printf
#define System_vprintf vprintf

#define VQ_TIMEOUT              (100)
#define VQ_BUF_SIZE             (2048)

#if defined(SOC_J721E)
#define IPC_VRING_MEM_SIZE                    (32U * 1024U * 1024U)
#elif defined(SOC_J7200)
#define IPC_VRING_MEM_SIZE                    (8U * 1024U * 1024U)
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
#define IPC_VRING_MEM_SIZE                    (48U * 1024U * 1024U)
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
#define ETHAPP_IPC_TASK_STACKALIGN              IPC_TASK_STACKSIZE 
#else
#define ETHAPP_IPC_TASK_STACKALIGN              (8192U)
#endif

static uint8_t g_initTaskStackBuf[IPC_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(ETHAPP_IPC_TASK_STACKALIGN)))
;

static uint8_t ctrlTaskBuf[IPC_TASK_STACKSIZE]
__attribute__ ((section(".bss:taskStackSection")))
__attribute__ ((aligned(ETHAPP_IPC_TASK_STACKALIGN)))
;

static uint8_t sysVqBuf[VQ_BUF_SIZE]  __attribute__ ((section("ipc_data_buffer"), aligned(8)));
static uint8_t gCntrlBuf[CPSW_REMOTE_APP_IPC_DATA_SIZE] __attribute__ ((section("ipc_data_buffer"), aligned(8)));

static uint8_t g_vringMemBuf[IPC_VRING_MEM_SIZE] __attribute__ ((section(".bss:ipc_vring_mem"), aligned(8192)));

static uint32_t selfProcId = IPC_MCU2_1;
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
#endif
};
static uint32_t gNumRemoteProc = sizeof(gRemoteProc) / sizeof(uint32_t);

#if defined(SOC_J721E) || defined(SOC_J7200)
#define BOARD_UART_INSTANCE                   (0U)
#elif defined(SOC_J784S4) || defined(SOC_J742S2)
#define BOARD_UART_INSTANCE                   (8U)
#else
#error "Unsupported device"
#endif

typedef struct CpswRemoteApp_Obj_s
{
    /* UDMA LLD object */
    struct Udma_DrvObj udmaDrvObj;

    /* UDMA LLD handle */
    Udma_DrvHandle hUdmaDrv;

    /* Enet peripheral type */
    Enet_Type enetType;

    /* Enet peripheral instance id */
    uint32_t instId;

    /* Enet LLD DMA handle */
    EnetDma_Handle hEnetDma;

    /* Core id used for Enet LLD APIs */
    uint32_t coreId;

    /* Whether to use default flow or not */
    bool useDefaultRxFlow;

    /* Whether to use extended attach remote command or not */
    bool useExtAttach;
} CpswRemoteApp_Obj;

void appLogPrintf(const char *format, ...);

/* Trace configuration */
static EthFwTrace_Cfg gRemoteApp_traceCfg =
{
#if defined(ENABLE_UART_LOG)
    .print        = UART_printf,
#else
    .print        = appLogPrintf,
#endif
    .traceTsFunc  = NULL,
    .extTraceFunc = NULL,
};

/* Link status on these ports will be used to determine link up on virtual switch port */
static Enet_MacPort gRemoteAppMacPorts[] =
{
    ENET_MAC_PORT_3,
};

CpswRemoteApp_Obj gRemoteAppObj =
{
    .hUdmaDrv         = NULL,
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
    .enetType         = ENET_CPSW_9G,
    .instId           = 0U,
#elif defined(SOC_J7200)
    .enetType         = ENET_CPSW_5G,
    .instId           = 0U,
#endif
    .hEnetDma         = NULL,
    .useDefaultRxFlow = BFALSE,
    .useExtAttach     = BFALSE,
};

CpswProxy_Handle gProxy;
CpswProxy_Config gProxyConfig;

static void EthApp_waitForDebugger(void);

static uint64_t CpswRemoteApp_virtToPhysFxn(const void *virtAddr,
                                            void *appData);

static void *CpswRemoteApp_physToVirtFxn(uint64_t phyAddr,
                                         void *appData);

static int32_t CpswRemoteApp_openUdma(void);

static int32_t CpswRemoteApp_openEnet(void);

// hack for release mode build fix TODO fix this
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
#if defined(ENABLE_UART_LOG)
    UART_printf("%s", str);
#else
    appLogPrintf("%s", str);
#endif
    return;
}

void setUp(void)
{
#if !defined(SAFERTOS)
    /* Do the setup when gProxy is Null*/
    if (gProxy == NULL)
    {
        int32_t status;
        CpswProxy_initParams initParams;

        /* Start Cpsw Proxy */
        memset(&initParams, 0, sizeof(initParams));
        CpswProxy_initConfig(&initParams);
        CpswProxy_init(&initParams);

        /* Wait for remote_device to be initialized on the server side */
        do
        {
            status = CpswProxy_connect();
        }
        while (status != IPC_SOK);

        gProxy = CpswProxy_open(&gProxyConfig);
        localAssert(gProxy != NULL);
    }
#endif
}

void tearDown(void)
{
#if !defined(SAFERTOS)
    if (gProxy != NULL)
    {
        CpswProxy_close(gProxy);
        CpswProxy_deinit();
        gProxy = NULL;
    }
#endif
}

static void CpswRemoteTestApp_initTask(void* a0)
{
    uint32_t numProc = gNumRemoteProc;
    Ipc_VirtIoParams vqParam;
    Ipc_InitPrms initPrms;
    RPMessage_Params cntrlParam;
    EthFwOsal_MailboxParams mbxParams;
    CpswProxy_initParams initParams;

    int32_t status;
    uint32_t i;

#if defined(ENABLE_UART_LOG)
    UART_stdioInit(BOARD_UART_INSTANCE);
#endif

    /* Initialize ETHFW Trace with INFO log level and higher */
    EthFwTrace_init(&gRemoteApp_traceCfg);
    EthFwTrace_setLevel(ETHFW_TRACE_INFO);

    /* Step 1: Initialize the IPC */
    status = EthFwIpc_init(selfProcId,
                           numProc,
                           &gRemoteProc[0],
                           &CpswRemoteApp_ipcPrint);

#if !defined(A72_QNX_OS)
    if (status == ETHFW_SOK)
    {
        status = Ipc_loadResourceTable(appGetIpcResourceTable());
    }
#else
    ETHFWTRACE_INFO("Skipping Ipc_loadResourceTable for QNX (core : %s)\r\n", Ipc_mpGetSelfName());
#endif

    if (status == ENET_SOK)
    {
        /* Step2 : Initialize Virtio */
        vqParam.vqObjBaseAddr = (void *)&sysVqBuf[0];
        vqParam.vqBufSize = numProc * Ipc_getVqObjMemoryRequiredPerCore();
        vqParam.vringBaseAddr = (void *)g_vringMemBuf;
        vqParam.vringBufSize = sizeof(g_vringMemBuf);
        vqParam.timeoutCnt = VQ_TIMEOUT;     /* Wait for counts */
        status = Ipc_initVirtIO(&vqParam);
    }

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

    /* Step 5: Start Cpsw Proxy */
    memset(&initParams, 0, sizeof(initParams));
    CpswProxy_initConfig(&initParams);
    CpswProxy_init(&initParams);

    /* Step 5a. Wait for remote_device to be initialized on the server side */
    do
    {
        status = CpswProxy_connect();
    }
    while (status != IPC_SOK);
    
    gProxyConfig.virtPort = ETHREMOTECFG_SWITCH_PORT_1;

    gProxy = CpswProxy_open(&gProxyConfig);
    localAssert(gProxy != NULL);

    /* SafeRTOS doesn't have support for EthFwOsal_deleteMailbox and teardown/setup functions
     * deletes and creates new mailbox as a result UT fails on SafeRTOS after 20
     * test cases (OSAL_SAFERTOS_CONFIGNUM_MAILBOX) . As a solution, 
     * Init and deinit only once for SafeRTOS per client */
#if !defined(SAFERTOS)
    EthFwUT_testSwitchConnection((void *)gProxy, (void *)&gProxyConfig);

    /* Do a setup to reset the test configurations before calling another test suite. */
    setUp();

    EthFwUT_testSwitchResources((void *)gProxy, (void *)&gProxyConfig);

    /* Update proxyConfig to MAC only configuration */
    gProxyConfig.virtPort = ETHREMOTECFG_MAC_PORT_4;

    /* Do a setup to reset the test configurations before calling another test suite. */
    setUp();

    EthFwUT_testMacConnection((void *)gProxy, (void *)&gProxyConfig);

    /* Do a setup to reset the test configurations before calling another test suite. */
    setUp();

    EthFwUT_testMacResources((void *)gProxy, (void *)&gProxyConfig);
#else

    EthFwUT_testSwitchConnection((void *)gProxy, (void *)&gProxyConfig);

    EthFwUT_testSwitchResources((void *)gProxy, (void *)&gProxyConfig);

    CpswProxy_deinit();
    CpswProxy_close(gProxy);

    /* Update proxyConfig to MAC only configuration */
    gProxyConfig.virtPort = ETHREMOTECFG_MAC_PORT_4;

    /* Start Cpsw Proxy */
    memset(&initParams, 0, sizeof(initParams));
    CpswProxy_initConfig(&initParams);
    CpswProxy_init(&initParams);

    /* Wait for remote_device to be initialized on the server side */
    do
    {
        status = CpswProxy_connect();
    }
    while (status != IPC_SOK);

    gProxy = CpswProxy_open(&gProxyConfig);
    localAssert(gProxy != NULL);

    EthFwUT_testMacConnection((void *)gProxy, (void *)&gProxyConfig);

    EthFwUT_testMacResources((void *)gProxy, (void *)&gProxyConfig);

    CpswProxy_deinit();
    CpswProxy_close(gProxy);
#endif
}

int main(void)
{
    EthFwOsal_TaskHandle task;
    EthFwOsal_TaskParams taskParams;
    int32_t status;

    OS_init();

    /* Wait for debugger to attach (disabled by default) */
    EthApp_waitForDebugger();

    /* Init UDMA LLD based on NAVSS instance */
    status = CpswRemoteApp_openUdma();
    if (status != UDMA_SOK)
    {
        ETHFWTRACE_ERR(status, "Failed to open UDMA LLD");
        localAssert(status == UDMA_SOK);
    }

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

    task = EthFwOsal_createTask(&CpswRemoteTestApp_initTask, &taskParams);

    if (NULL == task)
    {
        OS_stop();
    }

    OS_start();    /* does not return */

    return(0);
}

static void EthApp_waitForDebugger(void)
{
    /* Set ccsHaltFlag to 1 for halting core for CCS connection */
    volatile uint32_t ccsHaltFlag = 0U;

    while (ccsHaltFlag);
}

static int32_t CpswRemoteApp_openUdma(void)
{
    Udma_InitPrms initPrms;
    Udma_DrvHandle hUdmaDrv = &gRemoteAppObj.udmaDrvObj;
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

    gRemoteAppObj.hUdmaDrv = hUdmaDrv;

    return status;
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
    EnetDma_initCfg dmaCfg;
    EnetDma_Handle hEnetDma;
    int32_t status = ENET_SOK;

    /* Init Enet LLD utils */
    utilsPrms.print      = (Enet_Print)System_printf;
    utilsPrms.physToVirt = &CpswRemoteApp_physToVirtFxn;
    utilsPrms.virtToPhys = &CpswRemoteApp_virtToPhysFxn;
    Enet_init(&utilsPrms);

    /* Initialize Enet memutils */
    status = EnetMem_init();
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to init memutils");

    /* Initialize data path of Enet LLD */
    if (status == ENET_SOK)
    {
        EnetUdma_initDataPathParams(&dmaCfg);
        dmaCfg.hUdmaDrv = gRemoteAppObj.hUdmaDrv;

        hEnetDma = EnetUdma_initDataPath(gRemoteAppObj.enetType,
                                         gRemoteAppObj.instId,
                                         &dmaCfg);
        if (hEnetDma == NULL)
        {
            status = ENET_EFAIL;
            ETHFWTRACE_ERR(status, "Failed to init Enet LLD data path");
            EnetMem_deInit();
        }
        else
        {
            gRemoteAppObj.hEnetDma = hEnetDma;
            gRemoteAppObj.coreId   = EnetSoc_getCoreId();
        }
    }

    return status;
}
