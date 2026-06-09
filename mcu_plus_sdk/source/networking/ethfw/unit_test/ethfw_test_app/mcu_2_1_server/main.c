/*
 *
 * Copyright (c) 2025 Texas Instruments Incorporated
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
 *  \file main.c
 *
 *  \brief Main file for Ethernet Firmware server test application.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x901
#define ETHFWTRACE_MOD_NAME "Server-Test-App"

#include <stdio.h>
#include <stdint.h>

/* Enet LLD Driver Header files */
#include <enet.h>
#include <utils/include/enet_apputils.h>

/* EthFw header files */
#include <apps/ipc_cfg/app_ipc_rsctable.h>

#if defined(ETHFW_GPTP_SUPPORT)
/* Timesync header files */
#include <tsn_buildconf/jacinto_buildconf.h>
#include <tsn_gptp/gptpconf/gptpgcfg.h>
#include <tsn_gptp/gptpconf/xl4-extmod-xl4gptp.h>
#include <ethremotecfg/server/include/ethfw_tsn.h>
#endif

/* EthFw utils header files */
#include <utils/remote_service/include/app_remote_service.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/ethfw_stats/include/app_ethfw_stats_osal.h>
#include <utils/board/include/ethfw_board_utils.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

#include <ethremotecfg/protocol/ethremotecfg.h>
#include <ethremotecfg/server/include/ethfw.h>

#include <ethfw_test_cases.h>

#define System_printf Ipc_Trace_printf
#define System_vprintf Ipc_Trace_vprintf

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define ETHAPP_OK                               (0)

#define ETHAPP_ERROR                            (-1)

#define ETHAPP_TRACEBUF_FLUSH_PERIOD_IN_MSEC    (500U)

#define IPC_TRACEBUF_SIZE                       (0x80000U)

#if defined(SAFERTOS)
#define ETHFW_UT_TASK_STACKSIZE            (16U * 1024U)
#define ETHFW_UT_TASK_STACKALIGN           (ETHFW_UT_TASK_STACKSIZE)
#else
#define ETHFW_UT_TASK_STACKSIZE            (16U * 1024U)
#define ETHFW_UT_TASK_STACKALIGN           (32U)
#endif

#define ETHAPP_MAC_ADDR_POOL_SIZE               (6U)
#define ARRAY_SIZE(x)                           (sizeof((x)) / sizeof(x[0U]))

static uint8_t gEthTestAppStackBuf[ETHFW_UT_TASK_STACKSIZE] __attribute__ ((section(".bss:taskStackSection"))) __attribute__ ((aligned(ETHFW_UT_TASK_STACKALIGN)));

typedef struct
{
    /* Core Id */
    uint32_t coreId;

    /* Enet instance type */
    Enet_Type enetType;

    /* Enet instance id */
    uint32_t instId;

    /* Ethernet Firmware handle */
    EthFw_Handle hEthFw;

    /* Ethernet Firmware configuration params */
    EthFw_Config hEthFwCfg;

    /* UDMA driver handle */
    Udma_DrvHandle hUdmaDrv;

    /* UDMA configuration */
    EnetDma_Cfg dmaCfg;

    /* Host MAC address */
    uint8_t hostMacAddr[ENET_MAC_ADDR_LEN];

    /* IPC trace buffer address */
    uint8_t *traceBufAddr;

    /* IPC trace buffer size */
    uint32_t traceBufSize;

    /* Timestamp of last IPC trace buffer flush */
    uint64_t traceBufLastFlushTicksInUsecs;
} EthTestAppObj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

void appLogPrintf(const char *format, ...);

static void EthApp_waitForDebugger(void);

EthTestAppObj gEthTestAppObj =
{
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J742S2)
    .enetType = ENET_CPSW_9G,
    .instId   = 0U,
#endif
    .hEthFw = NULL,
    .hUdmaDrv = NULL,
};

/* Custom policers which clients need to provide to add their own policers.
 * Make sure that size of this array is <= ETHFW_UTILS_NUM_CUSTOM_POLICERS */
static CpswAle_SetPolicerEntryInPartitionInArgs gEthApp_customPolicers[ETHFW_UTILS_NUM_CUSTOM_POLICERS] = {};


Enet_MacPort gEthAppPorts[] =
{
#if defined(SOC_J721E)
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
#endif

#if defined(SOC_J784S4) || defined(SOC_J742S2)
    ENET_MAC_PORT_1, /* QSGMII main */
    ENET_MAC_PORT_3, /* QSGMII sub */
    ENET_MAC_PORT_4, /* QSGMII sub */
    ENET_MAC_PORT_5, /* QSGMII sub */
#endif
};

/* NOTE: 2 virtual ports should not have same tx channel allocated to them */
EthFwVirtPort_VirtPortCfg gEthApp_virtPortCfg[] =
{
    {
        .remoteCoreId  = IPC_MPU1_0,
        .portId        = ETHREMOTECFG_SWITCH_PORT_0,
         .numTxCh      = 2U,
         .txCh         = {
                            [0] = ENET_RM_TX_CH_4,
                            [1] = ENET_RM_TX_CH_7
                         },
        .numRxFlow     = 1U,
        .numMacAddress = 1U,
        .clientIdMask  = ETHFW_BIT(ETHREMOTECFG_CLIENTID_LINUX) | ETHFW_BIT(ETHREMOTECFG_CLIENTID_QNX),
    },
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
    {
        /* Virtual switch port for Ethfw, using ETHREMOTECFG_SWITCH_PORT_LAST */
        .remoteCoreId  = IPC_MCU2_0,
        .portId        = ETHREMOTECFG_SWITCH_PORT_LAST,
        .numTxCh       = 2U,
        .txCh          = {
                            [0] = ENET_RM_TX_CH_0,
                            [1] = ENET_RM_TX_CH_6
                         },
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
    {
        .remoteCoreId  = IPC_MCU2_1,
        .portId        = ETHREMOTECFG_MAC_PORT_4,
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

EthFwPortMirroring_Cfg gEthApp_portMirCfg = 
{
    .mirroringType = DISABLE_PORT_MIRRORING
};

void appLogPrintf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    System_vprintf(format, args);
    EnetAppUtils_vprint(format, args);
    va_end(args);
}

static int32_t EthApp_boardInit(void)
{
    uint32_t flags = 0U;
    int32_t status;

#if defined(SOC_J721E)
    flags |= (ETHFW_BOARD_GESI_ENABLE | ETHFW_BOARD_UART_ALLOWED);
#if defined(ETHFW_CCS)
    flags |= (ETHFW_BOARD_SERDES_CONFIG | ETHFW_BOARD_I2C_ALLOWED | ETHFW_BOARD_GPIO_ALLOWED);
#endif
#if defined(ENABLE_QSGMII_PORTS)
    flags |= ETHFW_BOARD_QENET_ENABLE;
#endif
#endif

#if defined(SOC_J784S4) || defined(SOC_J742S2)
    flags |= (ETHFW_BOARD_SERDES_CONFIG | ETHFW_BOARD_QENET_ENABLE | ETHFW_BOARD_UART_ALLOWED);
#if defined(ETHFW_CCS)
    flags |= ETHFW_BOARD_I2C_ALLOWED;
#endif
#endif

    /* Board related initialization */
    status = EthFwBoard_init(flags);

    return status;
}

/* Trace configuration */
static EthFwTrace_Cfg gEthApp_traceCfg =
{
    .print        = appLogPrintf,
    .traceTsFunc  = NULL,
    .extTraceFunc = NULL,
};

void setUp(void)
{
    Cpsw_Cfg *cpswCfg = &gEthTestAppObj.hEthFwCfg.cpswCfg;

    EnetRm_MacAddressPool *pool = &cpswCfg->resCfg.macList;
    uint32_t poolSize;

    /* Init EthFw config params */
    EthFw_initConfigParams(gEthTestAppObj.enetType, gEthTestAppObj.instId, &gEthTestAppObj.hEthFwCfg);

    /* Set UDMA handle to Enet LLD config */
    gEthTestAppObj.dmaCfg.hUdmaDrv = gEthTestAppObj.hUdmaDrv;
    gEthTestAppObj.dmaCfg.rxChInitPrms.dmaPriority = UDMA_DEFAULT_RX_CH_DMA_PRIORITY;
    cpswCfg->dmaCfg = &gEthTestAppObj.dmaCfg;

    /* Populate MAC address pool */
    poolSize = EnetUtils_min(ENET_ARRAYSIZE(pool->macAddress), ETHAPP_MAC_ADDR_POOL_SIZE);
    pool->numMacAddress = EthFwBoard_getMacAddrPool(pool->macAddress, poolSize);

    /* Set hardware port configuration parameters */
    gEthTestAppObj.hEthFwCfg.ports    = &gEthAppPorts[0];
    gEthTestAppObj.hEthFwCfg.numPorts = ARRAY_SIZE(gEthAppPorts);
    gEthTestAppObj.hEthFwCfg.setPortCfg = EthFwBoard_setPortCfg;

    /* Set virtual port configuration parameters */
    gEthTestAppObj.hEthFwCfg.virtPortCfg  = &gEthApp_virtPortCfg[0];
    gEthTestAppObj.hEthFwCfg.numVirtPorts = ARRAY_SIZE(gEthApp_virtPortCfg);
    gEthTestAppObj.hEthFwCfg.isStaticTxChanAllocated = BTRUE;
    gEthTestAppObj.hEthFwCfg.portMirCfg = &gEthApp_portMirCfg;

    /* CPTS_RFT_CLK is sourced from MAIN_SYSCLK0 (500MHz) */
    cpswCfg->cptsCfg.cptsRftClkFreq = CPSW_CPTS_RFTCLK_FREQ_500MHZ;
}

void tearDown(void)
{
    memset(&gEthTestAppObj.hEthFwCfg, 0, sizeof(EthFw_Config)); 
}

static void EthFwTest_initTask(void* a0)
{
    EthFw_Version ver;
    int32_t status = ETHAPP_OK;

    /* Initialize EthFw trace utils */
    EthFwTrace_init(&gEthApp_traceCfg);
    EthFwTrace_setLevel(ETHFW_TRACE_INFO);

    /* Board initialization: Serdes, GPIOs, pinmux, etc */
    status = EthApp_boardInit();
    if (status != ENET_SOK)
    {
        appLogPrintf("ETHFW: Board initialization failed\n");
    }

    /* Print EthFw banner */
    if (status == ETHAPP_OK)
    {
        appLogPrintf("=======================================================\n");
        appLogPrintf("            EthFw Test App                             \n");
        appLogPrintf("=======================================================\n");

        /* Open UDMA driver */
        gEthTestAppObj.hUdmaDrv = EnetAppUtils_udmaOpen(gEthTestAppObj.enetType, NULL);
        if (gEthTestAppObj.hUdmaDrv == NULL)
        {
            appLogPrintf("ETHFW: failed to open UDMA driver\n");
            status = ETHAPP_ERROR;
        }
    }

    EthFwUT_testEthRemoteCfg((void *)&gEthTestAppObj.hEthFwCfg);
}

int main(void)
{
    EthFwOsal_TaskHandle task;
    EthFwOsal_TaskParams taskParams;
    int32_t status;

    OS_init();

    /* Wait for debugger to attach (disabled by default) */
    EthApp_waitForDebugger();

    gEthTestAppObj.coreId = EnetSoc_getCoreId();

    /* Create initialization task */
    EthFwOsal_initTaskParams(&taskParams);
    taskParams.priority = 2;
    taskParams.stack = &gEthTestAppStackBuf[0];
    taskParams.stacksize = sizeof(gEthTestAppStackBuf);
    taskParams.name = "Test App Init Task";

    task = EthFwOsal_createTask(&EthFwTest_initTask, &taskParams);
    if (NULL == task)
    {
        OS_stop();
    }

    /* Does not return */
    OS_start();

    return(0);
}

static void EthApp_waitForDebugger(void)
{
    /* Set ccsHaltFlag to 1 for halting core for CCS connection */
    volatile uint32_t ccsHaltFlag = 0U;

    while (ccsHaltFlag);
}
