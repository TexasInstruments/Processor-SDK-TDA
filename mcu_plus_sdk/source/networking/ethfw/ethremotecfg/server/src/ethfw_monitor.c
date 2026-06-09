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

/*!
 *  \file ethfw_monitor.c
 *
 *  \brief Ethernet Firmware monitor.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x107

#include <stdint.h>

/* Enet LLD header files */
#include <utils/include/enet_mcm.h>
#include <utils/include/enet_apputils.h>

/* EthFw header files */
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <ethremotecfg/server/include/ethfw_monitor.h>
#include "cpsw_proxy_server.h"

#if defined(ETHFW_PROXY_ARP_HANDLING)
#include "ethfw_arp_priv.h"
#endif

#if defined(ETHFW_VEPA_SUPPORT)
#include "ethfw_vepa_priv.h"
#endif

#if defined(ETHFW_GPTP_SUPPORT)
#include <ethremotecfg/server/include/ethfw_tsn.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/*! Monitor Task priority */
#define ETHFW_MON_TASK_PRIORITY                       (2U)

/*! Monitor Task stack size and alignment */
#if defined(SAFERTOS)
#define ETHFW_MON_TASK_STACK_SIZE                     (16U * 1024U)
#define ETHFW_MON_TASK_STACK_ALIGN                    ETHFW_MON_TASK_STACK_SIZE
#else
#define ETHFW_MON_TASK_STACK_SIZE                     (16U * 1024U)
#define ETHFW_MON_TASK_STACK_ALIGN                    (32U)
#endif

/*! Monitor task polling period */
#define ETHFW_MON_TASK_POLL_PERIOD_MS                 (100U)

/*! Remote client idle status check period */
#define ETHFW_MON_HWRECOVERY_IDLE_CHECK_PERIOD_MS     (100U)

/*! Number of client idle check retries before printing number of
 * clients that have idled.  Each retry iteration takes
 * ETHFW_MON_HWRECOVERY_IDLE_CHECK_PERIOD_MS. */
#define ETHFW_MON_HWRECOVERY_RETRY_LOG_ITER           (10U)

/*! Value of seconds in nanoseconds. Useful for calculations */
#define ETHFW_TIME_SEC_TO_NS                          (1000000000U)

/*! Max number of CPSW MAC ports supported */
#if defined(SOC_J721E) || defined(SOC_J784S4) || defined(SOC_J784S4)
#define ETHFW_MON_MAC_PORT_MAX                        (8U)
#else
#define ETHFW_MON_MAC_PORT_MAX                        (4U)
#endif

/*!
 * \brief EthFw Monitor object.
 */
typedef struct EthFwMon_Obj_s
{
     /* Core Id */
     uint32_t coreId;

     /* Enet instance type */
     Enet_Type enetType;

     /* Enet Handle */
     Enet_Handle hEnet;

     /* instance Id */
     uint32_t instId;

     /*! Number of MAC ports owned by EthFw, that is, the size of
      *  EthFw_Config::ports array */
     uint32_t numPorts;

    /* Whether recovery is enabled or not */
    bool recoveryEn;

    /* Monitor and recovery callbacks */
    EthFwMon_Cfg monitor;

    /* Saved statistics counters */
    EthFwMon_Stats monStats[ETHFW_MON_MAC_PORT_MAX + 1U];

    /* CPSW statistics block */
    CpswStats_PortStats cpswStats;

    /*! Clock handle for Monitor Task */
    EthFwOsal_ClockHandle hMonitorClock;

    /*! Semaphore handle for Monitor Task */
    EthFwOsal_SemHandle hMonitorSem;

    /*! Task handle for Monitor Task */
    EthFwOsal_TaskHandle hMonitorTask;

    /* To run Monitor Task */
    bool monitorTaskRun;

    /* Monitor Task stack buffer */
    uint8_t monTaskStackBuf[ETHFW_MON_TASK_STACK_SIZE] __attribute__ ((aligned(ETHFW_MON_TASK_STACK_ALIGN)));

    /* Multiclient Manager (MCM) handle */
    EnetMcm_CmdIf mcmCmdIf;
} EthFwMon_Obj;



/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void EthFwMon_Task(void *a0);

static int32_t EthFwMon_resetHandler(void);

/* ========================================================================== */
/*                          Extern variables                                  */
/* ========================================================================== */

/* FIXME: Updating the status for EthFw needs to be done in a cleaner way (without extern). */
extern void EthFw_setStatus(EthRemoteCfg_ServerStatus serverStatus);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static EthFwMon_Obj gEthFwMonObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void EthFwMon_ClockCb(void *arg)
{
    /* Post semaphore to Monitor Task */
    EthFwOsal_postSemaphore(gEthFwMonObj.hMonitorSem);
}

void EthFwMon_initCfg(EthFwMon_Cfg *monCfg)
{
    monCfg->periodInMsecs     = ETHFW_MON_TASK_POLL_PERIOD_MS;
    monCfg->openLwipDmaCb     = NULL;
    monCfg->closeLwipDmaCb    = NULL;
    monCfg->lwipDmaCbArg      = NULL;
    monCfg->statsMonHostEvtCb = NULL;
    monCfg->statsMonMacEvtCb  = NULL;
    monCfg->statsMonCbArg     = NULL;
}

int32_t EthFwMon_init(const EthFwMon_Cfg *monCfg,
                      Enet_Type enetType,
                      uint32_t instId,
                      uint32_t numPorts)
{
    int32_t status = ENET_SOK;

    gEthFwMonObj.monitor    = *monCfg;
    gEthFwMonObj.enetType   = enetType;
    gEthFwMonObj.instId     = instId;
    gEthFwMonObj.coreId     = EnetSoc_getCoreId();
    gEthFwMonObj.numPorts   = numPorts;
    gEthFwMonObj.recoveryEn = ((monCfg->openLwipDmaCb != NULL) &&
                              (monCfg->closeLwipDmaCb != NULL));
    ETHFWTRACE_INFO_IF(!gEthFwMonObj.recoveryEn, "CPSW recovery is not enabled");

    return status;
}

int32_t EthFwMon_startTask(void)
{
    EthFwOsal_ClockParams clkParams;
    EthFwOsal_TaskParams params;
    int32_t status = ETHFW_SOK;

    gEthFwMonObj.hMonitorSem = EthFwOsal_createSemaphore(1U);

    if (gEthFwMonObj.hMonitorSem == NULL)
    {
        status = ETHFW_EALLOC;
        ETHFWTRACE_ERR(ETHFW_EALLOC, "Unable to create monitor clock semaphore");
        EnetAppUtils_assert(BFALSE);
    }

    if (status == ETHFW_SOK)
    {
        /* Create Monitor Task to monitor and detect EthFw's failure. */
        EthFwOsal_initTaskParams(&params);
        params.priority  = ETHFW_MON_TASK_PRIORITY;
        params.stack     = &gEthFwMonObj.monTaskStackBuf[0];
        params.stacksize = sizeof(gEthFwMonObj.monTaskStackBuf);
        params.name      = "ETHFW Monitor Task";
        gEthFwMonObj.monitorTaskRun = BTRUE;

        gEthFwMonObj.hMonitorTask = EthFwOsal_createTask(&EthFwMon_Task, &params);

        if (gEthFwMonObj.hMonitorTask == NULL)
        {
            status = ETHFW_EALLOC;
            ETHFWTRACE_ERR(ETHFW_EALLOC, "Unable to create monitor task");
            EnetAppUtils_assert(BFALSE);
        }
    }

    if (status == ETHFW_SOK)
    {
        EthFwOsal_initClockParams(&clkParams);
        clkParams.startMode = ETHFWCLOCK_STARTMODE_AUTO;
        clkParams.period    = gEthFwMonObj.monitor.periodInMsecs;
        clkParams.runMode   = ETHFWCLOCK_RUNMODE_CONTINUOUS;

        /* Creating clock and setting clock callback function */
        gEthFwMonObj.hMonitorClock = EthFwOsal_createClock((void*)&EthFwMon_ClockCb, &clkParams);
        if (gEthFwMonObj.hMonitorClock == NULL)
        {
            status = ETHFW_EALLOC;
            ETHFWTRACE_ERR(ETHFW_EALLOC, "Unable to create monitor clock");
            EnetAppUtils_assert(BFALSE);
        }
    }
    return status;
}

void EthFwMon_stopTask(void)
{
    gEthFwMonObj.monitorTaskRun = BFALSE;

    if (gEthFwMonObj.hMonitorTask != NULL)
    {
        EthFwOsal_deleteTask(&gEthFwMonObj.hMonitorTask);
        gEthFwMonObj.hMonitorTask = NULL;
    }

    /* Delete semaphore */
    EthFwOsal_deleteSemaphore(gEthFwMonObj.hMonitorSem);

    /* Stop and delete the clock */
    EthFwOsal_stopClock(gEthFwMonObj.hMonitorClock);
    EthFwOsal_deleteClock(gEthFwMonObj.hMonitorClock);
}

static void EthFwMon_saveStats(void)
{
    const CpswStats_HostPort_Ng *hostStats = (const CpswStats_HostPort_Ng *)&gEthFwMonObj.cpswStats;
    const CpswStats_MacPort_Ng *macStats = (const CpswStats_MacPort_Ng *)&gEthFwMonObj.cpswStats;
    EthFwMon_Stats *monStats;
    Enet_IoctlPrms prms;
    Enet_MacPort macPort;
    uint32_t portNum;
    uint32_t i;
    uint32_t j;
    int32_t status = ETHFW_SOK;

    /* Get host port stats counters */
    monStats = &gEthFwMonObj.monStats[0U];
    ENET_IOCTL_SET_OUT_ARGS(&prms, &gEthFwMonObj.cpswStats);
    ENET_IOCTL(gEthFwMonObj.hEnet, gEthFwMonObj.coreId, ENET_STATS_IOCTL_GET_HOSTPORT_STATS, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to get host port stats");

    if (status == ENET_SOK)
    {
        monStats->rxBottomOfFifoDrop = hostStats->rxBottomOfFifoDrop;
        monStats->rxTopOfFifoDrop    = hostStats->rxTopOfFifoDrop;
        for (j = 0U; j < ENET_PRI_NUM; j++)
        {
            monStats->txPriDrop[j] = hostStats->txPriDrop[j];
        }
    }

    /* Get MAC port stats counters */
    for (i = 0U; i < gEthFwMonObj.numPorts; i++)
    {
        macPort = ENET_MACPORT_DENORM(i);
        portNum = ENET_MACPORT_NORM(macPort);
        monStats = &gEthFwMonObj.monStats[portNum + 1U];

        ENET_IOCTL_SET_INOUT_ARGS(&prms, &macPort, &gEthFwMonObj.cpswStats);
        ENET_IOCTL(gEthFwMonObj.hEnet, gEthFwMonObj.coreId, ENET_STATS_IOCTL_GET_MACPORT_STATS, &prms, status);
        ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                          "Failed to get MAC port %u stats", ENET_MACPORT_ID(macPort));

        if (status == ENET_SOK)
        {
            monStats->rxBottomOfFifoDrop = macStats->rxBottomOfFifoDrop;
            monStats->rxTopOfFifoDrop    = macStats->rxTopOfFifoDrop;
            for (j = 0U; j < ENET_PRI_NUM; j++)
            {
                monStats->txPriDrop[j] = macStats->txPriDrop[j];
            }
        }
    }
}

static bool EthFwMon_analyzeHostStats(void)
{
    const CpswStats_HostPort_Ng *stats = (const CpswStats_HostPort_Ng *)&gEthFwMonObj.cpswStats;
    EthFwMon_Stats *monStats;
    EthFwMon_Stats diffStats;
    Enet_IoctlPrms prms;
    bool needsRecovery = BFALSE;
    uint32_t evt = 0U;
    uint32_t i;
    int32_t status = ETHFW_SOK;

    monStats = &gEthFwMonObj.monStats[0U];

    /* Get host port stats counters */
    ENET_IOCTL_SET_OUT_ARGS(&prms, &gEthFwMonObj.cpswStats);
    ENET_IOCTL(gEthFwMonObj.hEnet, gEthFwMonObj.coreId, ENET_STATS_IOCTL_GET_HOSTPORT_STATS, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Failed to get host port stats");

    /* Check stats counters we are monitoring */
    if (status == ENET_SOK)
    {
        /* Monitor port statistics to detect and CPSW peripheral failure.
         * Current EthFw Monitor task looks for rxBottomOfFifoDrop value.
         * if rxBottomOfFifoDrop > 0, the CPSW has gone into unrecoverable state,
         * so resetting the Enet Peripheral. */
        if (stats->rxBottomOfFifoDrop > monStats->rxBottomOfFifoDrop)
        {
            evt |= ETHFW_STATSMON_RXBOTTOMOFFIFODROP;
            needsRecovery = BTRUE;
        }

        if (stats->rxTopOfFifoDrop > monStats->rxTopOfFifoDrop)
        {
            evt |= ETHFW_STATSMON_RXTOPOFFIFODROP;
        }

        for (i = 0U; i < ENET_PRI_NUM; i++)
        {
            if (stats->txPriDrop[i] > monStats->txPriDrop[i])
            {
                evt |= ETHFW_STATSMON_TXPRIDROP;
                break;
            }
        }
    }

    /* Call application callback if one has been provided */
    if ((status == ETHFW_SOK) &&
        (evt != 0U) &&
        (gEthFwMonObj.monitor.statsMonHostEvtCb != NULL))
    {
        diffStats.rxBottomOfFifoDrop = stats->rxBottomOfFifoDrop - monStats->rxBottomOfFifoDrop;
        diffStats.rxTopOfFifoDrop    = stats->rxTopOfFifoDrop - monStats->rxTopOfFifoDrop;
        for (i = 0U; i < ENET_PRI_NUM; i++)
        {
            diffStats.txPriDrop[i] = stats->txPriDrop[i] - monStats->txPriDrop[i];
        }

        gEthFwMonObj.monitor.statsMonHostEvtCb(evt, &diffStats, stats,
                                            gEthFwMonObj.monitor.statsMonCbArg);

        monStats->rxBottomOfFifoDrop = stats->rxBottomOfFifoDrop;
        monStats->rxTopOfFifoDrop    = stats->rxTopOfFifoDrop;
        memcpy(monStats->txPriDrop, stats->txPriDrop, sizeof(monStats->txPriDrop));
    }

    return needsRecovery;
}

static bool EthFwMon_analyzePortStats(Enet_MacPort macPort)
{
    const CpswStats_MacPort_Ng *stats = (const CpswStats_MacPort_Ng *)&gEthFwMonObj.cpswStats;
    EthFwMon_Stats *monStats;
    EthFwMon_Stats diffStats;
    Enet_IoctlPrms prms;
    bool needsRecovery = BFALSE;
    uint32_t evt = 0U;
    uint32_t portNum = ENET_MACPORT_NORM(macPort);
    uint32_t i;
    int32_t status = ETHFW_SOK;

    monStats = &gEthFwMonObj.monStats[portNum + 1U];

    /* Get MAC port stats counters */
    ENET_IOCTL_SET_INOUT_ARGS(&prms, &macPort, &gEthFwMonObj.cpswStats);
    ENET_IOCTL(gEthFwMonObj.hEnet, gEthFwMonObj.coreId, ENET_STATS_IOCTL_GET_MACPORT_STATS, &prms, status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status,
                      "Failed to get MAC port %u stats", ENET_MACPORT_ID(macPort));

    /* Check stats counters we are monitoring */
    if (status == ENET_SOK)
    {
        /* Monitor port statistics to detect and CPSW peripheral failure.
         * Current EthFw Monitor task looks for rxBottomOfFifoDrop value.
         * if rxBottomOfFifoDrop > 0, the CPSW has gone into unrecoverable state,
         * so resetting the Enet Peripheral. */
        if (stats->rxBottomOfFifoDrop > monStats->rxBottomOfFifoDrop)
        {
            evt |= ETHFW_STATSMON_RXBOTTOMOFFIFODROP;
            needsRecovery = BTRUE;
        }

        if (stats->rxTopOfFifoDrop > monStats->rxTopOfFifoDrop)
        {
            evt |= ETHFW_STATSMON_RXTOPOFFIFODROP;
        }

        for (i = 0U; i < ENET_PRI_NUM; i++)
        {
            if (stats->txPriDrop[i] > monStats->txPriDrop[i])
            {
                evt |= ETHFW_STATSMON_TXPRIDROP;
                break;
            }
        }
    }

    /* Call application callback if one has been provided */
    if ((status == ETHFW_SOK) &&
        (evt != 0U) &&
        (gEthFwMonObj.monitor.statsMonMacEvtCb != NULL))
    {
        diffStats.rxBottomOfFifoDrop = stats->rxBottomOfFifoDrop - monStats->rxBottomOfFifoDrop;
        diffStats.rxTopOfFifoDrop    = stats->rxTopOfFifoDrop - monStats->rxTopOfFifoDrop;
        for (i = 0U; i < ENET_PRI_NUM; i++)
        {
            diffStats.txPriDrop[i] = stats->txPriDrop[i] - monStats->txPriDrop[i];
        }

        gEthFwMonObj.monitor.statsMonMacEvtCb(macPort, evt, &diffStats, stats,
                                               gEthFwMonObj.monitor.statsMonCbArg);

        monStats->rxBottomOfFifoDrop = stats->rxBottomOfFifoDrop;
        monStats->rxTopOfFifoDrop    = stats->rxTopOfFifoDrop;
        memcpy(monStats->txPriDrop, stats->txPriDrop, sizeof(monStats->txPriDrop));
    }

    return needsRecovery;
}

static void EthFwMon_Task(void *a0)
{
    Enet_MacPort macPort;
    Enet_MacPort recoveryMacPort = ENET_MAC_PORT_INV;
    bool needsRecovery = BFALSE;
    uint32_t i;
    int32_t status = ETHFW_SOK;
    bool isTeardownComplete = BFALSE;
    bool isrecoveryPortLinked = BFALSE;
    bool noPendingReq = BFALSE;
    uint32_t numTotalClients  = 0U;
    uint32_t numActiveClients  = 0U;
    uint32_t numIdleClients = 0U;
    uint32_t teardownLoopCnt = 0U;

    /* Get MCM command interface */
    EnetMcm_getCmdIf(gEthFwMonObj.enetType, &gEthFwMonObj.mcmCmdIf);
    EnetAppUtils_assert(gEthFwMonObj.mcmCmdIf.hMboxCmd != NULL);
    EnetAppUtils_assert(gEthFwMonObj.mcmCmdIf.hMboxResponse != NULL);

    gEthFwMonObj.hEnet = Enet_getHandle(gEthFwMonObj.enetType, gEthFwMonObj.instId);

    while (gEthFwMonObj.monitorTaskRun)
    {
        EthFwOsal_pendSemaphore(gEthFwMonObj.hMonitorSem, ETHFWOSAL_WAIT_FOREVER);

        needsRecovery = EthFwMon_analyzeHostStats();

        for (i = 0U; (i < gEthFwMonObj.numPorts) && !needsRecovery; i++)
        {
            macPort = ENET_MACPORT_DENORM(i);
            if (EnetAppUtils_isPortLinkUp(gEthFwMonObj.hEnet, gEthFwMonObj.coreId, macPort) == BTRUE)
            {
                needsRecovery = EthFwMon_analyzePortStats(macPort);
                if (needsRecovery)
                {
                    recoveryMacPort = macPort;

                    /* CPSW is in an unrecoverable state (MAC port is suspected of being locked up or not responsive),
                    hence moving ETHFW to BAD state. */
                    EthFw_setStatus(ETHREMOTECFG_SERVERSTATUS_BAD);
                }
            }
        }

        if (needsRecovery && gEthFwMonObj.recoveryEn)
        {
            /* Stop the clock during reset recovery handling */
            EthFwOsal_stopClock(gEthFwMonObj.hMonitorClock);

            /* Stop periodic ticks to Mcm */
            EnetMcm_stopPeriodicTick(&gEthFwMonObj.mcmCmdIf);

            /* Get the clients status */
            CpswProxyServer_getIdleClientCnt(&numTotalClients,&numIdleClients);

            ETHFWTRACE_INFO("%u clients attached to be reset", numTotalClients);

            if (numTotalClients != 0U)
            {
                /* Notify the clients about the HW error */
                CpswProxyServer_bcastNotify(ETHREMOTECFG_NOTIFY_HWERROR);

                /* Wait for clients to complete their DMA teardown */
                while (!isTeardownComplete)
                {
                    /* get the client status */
                    CpswProxyServer_getIdleClientCnt(&numActiveClients,&numIdleClients);

                    if (numIdleClients == numTotalClients)
                    {
                        isTeardownComplete = BTRUE;
                    }

                    EthFwOsal_sleepTask(ETHFW_MON_HWRECOVERY_IDLE_CHECK_PERIOD_MS);

                    teardownLoopCnt++;
                    if ((teardownLoopCnt % ETHFW_MON_HWRECOVERY_RETRY_LOG_ITER) == 0U)
                    {
                        ETHFWTRACE_INFO("%u of %u clients have completed DMA tear-down",
                                        numIdleClients, numActiveClients);
                        teardownLoopCnt = 0U;
                    }
                }

                /* Wait untill all ARP table entries are free. */
                while (!noPendingReq)
                {
#if defined(ETHFW_PROXY_ARP_HANDLING)
                    if (EthFwArp_getUseCnt() == 0U)
                    {
                        noPendingReq = BTRUE;
                    }
#endif

#if defined(ETHFW_VEPA_SUPPORT)
                    if (EthFwVepa_getUseCnt() == 0U)
                    {
                        noPendingReq = BTRUE;
                    }
#endif
                    EthFwOsal_sleepTask(ETHFW_MON_HWRECOVERY_IDLE_CHECK_PERIOD_MS);
                }
            }

            /* Recovery will happen now. */
            ETHFWTRACE_INFO("CPSW recovery is about to take place");

            /* Set the status of EthFw to ETHREMOTECFG_SERVERSTATUS_RECOVERY before calling the EthFw reset handler. */
            EthFw_setStatus(ETHREMOTECFG_SERVERSTATUS_RECOVERY);

            /* Call the EthFw reset handler */
            status = EthFwMon_resetHandler();

            if (status != ETHFW_SOK)
            {
                ETHFWTRACE_ERR(status, "Failed to recover CPSW");
                EnetAppUtils_assert(status == ETHFW_SOK);
            }
            else
            {
                /* Send a notification to clients for HW recovery completion only when Port link is up*/
                while(!isrecoveryPortLinked)
                {
                    if (EnetAppUtils_isPortLinkUp(gEthFwMonObj.hEnet, gEthFwMonObj.coreId, recoveryMacPort) == BTRUE)
                    {
                        isrecoveryPortLinked = BTRUE;
                    }

                    EthFwOsal_sleepTask(ETHFW_MON_HWRECOVERY_IDLE_CHECK_PERIOD_MS);
                }

#if defined(ETHFW_GPTP_SUPPORT)
                /* start the gPTP module */
                EthFwTsn_restartTsnModule(ETHFWTSN_GPTP_TASK_IDX);
#endif

                /* Notify the clients about the HW error recovery completion */
                CpswProxyServer_bcastNotify(ETHREMOTECFG_NOTIFY_HWRECOVERY_COMPLETE);
            }

            /* Set teardown flag to false for next iteration */
            isTeardownComplete = BFALSE;
            isrecoveryPortLinked = BFALSE;
            noPendingReq = BFALSE;
            /* CPSW recovery is complete, ETHFW can be moved back to READY state. */
            EthFw_setStatus(ETHREMOTECFG_SERVERSTATUS_READY);
            EthFwOsal_startClock(gEthFwMonObj.hMonitorClock);
        }
    }
}

uint64_t EthFwMon_getCurrentTime(uint32_t *nanoSeconds,
                                 uint64_t *seconds)
{
    int32_t status = ENET_SOK;
    Enet_IoctlPrms prms;
    uint64_t tsVal = 0U;

    /* Software Time stamp Push event */
    ENET_IOCTL_SET_OUT_ARGS(&prms, &tsVal);
    ENET_IOCTL(gEthFwMonObj.hEnet,
               gEthFwMonObj.coreId,
               ENET_TIMESYNC_IOCTL_GET_CURRENT_TIMESTAMP,
               &prms,
               status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Error in getting the current CPTS time");

    *nanoSeconds = (uint32_t)(tsVal % (uint64_t)ETHFW_TIME_SEC_TO_NS);
    *seconds = tsVal / (uint64_t)ETHFW_TIME_SEC_TO_NS;

    tsVal = (uint64_t)(((uint64_t)*seconds * (uint64_t)ETHFW_TIME_SEC_TO_NS) + *nanoSeconds);

    return tsVal;
}

void EthFwMon_setCurrentTime(uint64_t *time)
{
    int32_t status = ETHFW_SOK;
    Enet_IoctlPrms prms;

    /* Update the CPTS time */
    ENET_IOCTL_SET_IN_ARGS(&prms, time);

    ENET_IOCTL(gEthFwMonObj.hEnet,
               gEthFwMonObj.coreId,
               ENET_TIMESYNC_IOCTL_SET_TIMESTAMP,
               &prms,
               status);
    ETHFWTRACE_ERR_IF((status != ENET_SOK), status, "Error in setting the updated CPTS time");
}

static int32_t EthFwMon_resetHandler(void)
{
    uint32_t nanoSeconds = 0U;
    uint64_t seconds = 0LLU;
    uint64_t preResetTime;
    uint64_t postResetTime;
    uint64_t currentTime;
    uint64_t updatedTime;
    int32_t status = ETHFW_SOK;

    /* Get current CPTS time */
    currentTime = EthFwMon_getCurrentTime(&nanoSeconds, &seconds);

    /* Get OS time before triggering a reset */
    preResetTime = EthFwOsal_getTimeInUsecs();

    /* Close MAC Ports */
    status = EnetMcm_closeMacPorts(&gEthFwMonObj.mcmCmdIf);
    EnetAppUtils_assert(status == ENET_SOK);

    /* Call App callback to close the Lwip Dma channels */
    gEthFwMonObj.monitor.closeLwipDmaCb(gEthFwMonObj.monitor.lwipDmaCbArg);

#if defined(ETHFW_GPTP_SUPPORT)
    /* Stop the gPTP module */
    EthFwTsn_stopModule(ETHFWTSN_GPTP_TASK_IDX);
#endif

    /* Save the context */
    EnetMcm_saveCtxt(&gEthFwMonObj.mcmCmdIf);

    /* Reset the CPSW */
    EnetAppUtils_turnCpswOff();
    EnetAppUtils_delayInUsec(5000U);
    EnetAppUtils_turnCpswOn();

    /* Clear local stats counters */
    memset(gEthFwMonObj.monStats, 0, sizeof(gEthFwMonObj.monStats));

    /* Workaround: CPSW software stats are currently not cleared during save/restore context.
     * In order to prevent that the recovery mechanism runs in infinite loop, save the last
     * software stats so they become the starting point going forward */
    EthFwMon_saveStats();

    /* Restore the context */
    status = EnetMcm_restoreCtxt(&gEthFwMonObj.mcmCmdIf);
    EnetAppUtils_assert(status == ENET_SOK);

    /* Call App callback to open the Lwip Dma channels */
    gEthFwMonObj.monitor.openLwipDmaCb(gEthFwMonObj.monitor.lwipDmaCbArg);

    /* Open MAC Ports */
    status = EnetMcm_openMacPorts(&gEthFwMonObj.mcmCmdIf);
    EnetAppUtils_assert(status == ENET_SOK);

    /* Get OS time post reset is done */
    postResetTime = EthFwOsal_getTimeInUsecs();

    /* Calculate the updated time( current time + (time taken by during reset)*1000U (convert to nanoseconds))
    * in nanoseconds to be set into CPTS */
    updatedTime = currentTime + (postResetTime - preResetTime)*1000U;

    /* Update CPTS time with the time taken by reset recovery */
    EthFwMon_setCurrentTime(&updatedTime);

    /* Start periodic ticks to Mcm */
    EnetMcm_startPeriodicTick(&gEthFwMonObj.mcmCmdIf);

    return status;
}



