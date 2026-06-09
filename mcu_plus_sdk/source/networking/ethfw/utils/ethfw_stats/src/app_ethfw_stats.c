/*
 *
 * Copyright (c) 2019-2024 Texas Instruments Incorporated
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

#include <utils/remote_service/include/app_remote_service.h>
#include <utils/ethfw_stats/include/app_ethfw_stats_osal.h>
#include <ti/drv/enet/examples/utils/include/enet_apputils.h>
#include <utils/console_io/include/app_log.h>
#include <utils/ethfw_abstract/ethfw_osal.h>
#include <utils/include/enet_mcm.h>

#define APP_ETHFW_STATS_POLL_PERIOD_MS           (500U)

#define APP_ETHFW_STATS_COLLECTOR_TASK_PRI       (10U)

#if defined(SAFERTOS)
#define APP_ETHFW_STATS_COLLECTOR_TASK_STACKSIZE  (16U * 1024U)
#define APP_ETHFW_STATS_COLLECTOR_TASK_STACKALIGN APP_ETHFW_STATS_COLLECTOR_TASK_STACKSIZE
#else
#define APP_ETHFW_STATS_COLLECTOR_TASK_STACKSIZE  (8192U)
#define APP_ETHFW_STATS_COLLECTOR_TASK_STACKALIGN (32U)
#endif

#define APP_ETHFW_STATS_BW_MULT                  (1000U / (APP_ETHFW_STATS_POLL_PERIOD_MS))
#define APP_ETHFW_STATS_BW_FRAMES_MULT           (20U)

typedef struct
{
    Enet_Type enetType;

    uint32_t instId;

    Enet_Handle hEnet;

    uint32_t coreId;

    uint32_t coreKey;

    app_ethfw_port_bandwidth_t ethfwPortBw;

    CpswStats_PortStats prevPortStats[APP_ETHFW_PORT_NUM_MAX];

    EthFwOsal_ClockHandle hStatsClock;

    EthFwOsal_SemHandle clockSem;

    EthFwOsal_TaskHandle hStatsCollectorTask;

    bool ethfwStatsUpdateEnable;

    bool ethfwStatsShutdown;

} app_ethfw_stats_obj_t;

static app_ethfw_stats_obj_t g_app_ethfw_stats_obj;
static uint8_t g_ethfwStatsCollectorStack[APP_ETHFW_STATS_COLLECTOR_TASK_STACKSIZE] __attribute__ ((aligned(APP_ETHFW_STATS_COLLECTOR_TASK_STACKALIGN)));

static void appEthfw_statsCollectorTask(void *arg0);
static void appEthfwStats_clockCb(void *arg);


static void appEthfwStats_clockCb(void *arg)
{
    /* Post semaphore to Stats collecting task */
    app_ethfw_stats_obj_t *obj = (app_ethfw_stats_obj_t *)arg;

    if (obj->ethfwStatsUpdateEnable == BTRUE)
    {
        EthFwOsal_postSemaphore(obj->clockSem);
    }
}

static int32_t appEthfw_createClock(app_ethfw_stats_obj_t *obj)
{
    int32_t status = ENET_SOK;
    EthFwOsal_ClockParams clkParams;

    EthFwOsal_initClockParams(&clkParams);
    clkParams.startMode = ETHFWCLOCK_STARTMODE_AUTO;
    clkParams.period    = APP_ETHFW_STATS_POLL_PERIOD_MS;
    clkParams.runMode   = ETHFWCLOCK_RUNMODE_CONTINUOUS;
    clkParams.arg       = (void *)obj;

    /* Creating clock and setting clock callback function*/
    obj->hStatsClock = EthFwOsal_createClock((void*) &appEthfwStats_clockCb,
                                      &clkParams);
    if (obj->hStatsClock == NULL)
    {
        status = ENET_EFAIL;
    }

    return status;
}

static void appEthfw_calculateBw(CpswStats_PortStats currPortStats,
                                 CpswStats_PortStats prevPortStats,
                                 uint64_t *tx_bandwidth,
                                 uint64_t *rx_bandwidth)
{
    uint32_t txOctetsDiff = 0U, rxOctetsDiff = 0U, txFramesDiff = 0U, rxFramesDiff = 0U;
    app_ethfw_stats_obj_t *obj = &g_app_ethfw_stats_obj;

    if (obj->enetType == ENET_CPSW_2G)
    {
        CpswStats_HostPort_2g *cpsw2gCurrPortStats = (CpswStats_HostPort_2g *)&currPortStats;
        CpswStats_HostPort_2g *cpsw2gPrevPortStats = (CpswStats_HostPort_2g *)&prevPortStats;

        txOctetsDiff = cpsw2gCurrPortStats->txOctets - cpsw2gPrevPortStats->txOctets;
        txFramesDiff = cpsw2gCurrPortStats->txGoodFrames - cpsw2gPrevPortStats->txGoodFrames;

        rxOctetsDiff = cpsw2gCurrPortStats->rxOctets - cpsw2gPrevPortStats->rxOctets;
        rxFramesDiff = cpsw2gCurrPortStats->rxGoodFrames - cpsw2gPrevPortStats->rxGoodFrames;
    }
    if  ((obj->enetType == ENET_CPSW_9G)||(obj->enetType == ENET_CPSW_5G))
    {
        CpswStats_HostPort_Ng *cpsw9gCurrPortStats = (CpswStats_HostPort_Ng *)&currPortStats;
        CpswStats_HostPort_Ng *cpsw9gPrevPortStats = (CpswStats_HostPort_Ng *)&prevPortStats;

        txOctetsDiff = cpsw9gCurrPortStats->txOctets - cpsw9gPrevPortStats->txOctets;
        txFramesDiff = cpsw9gCurrPortStats->txGoodFrames - cpsw9gPrevPortStats->txGoodFrames;

        rxOctetsDiff = cpsw9gCurrPortStats->rxOctets - cpsw9gPrevPortStats->rxOctets;
        rxFramesDiff = cpsw9gCurrPortStats->rxGoodFrames - cpsw9gPrevPortStats->rxGoodFrames;
    }

    *tx_bandwidth = ((txOctetsDiff + (txFramesDiff * APP_ETHFW_STATS_BW_FRAMES_MULT)) * (APP_ETHFW_STATS_BW_MULT));
    *rx_bandwidth = ((rxOctetsDiff + (rxFramesDiff * APP_ETHFW_STATS_BW_FRAMES_MULT)) * (APP_ETHFW_STATS_BW_MULT));
}

static void appEthfw_statsCollectorTask(void *arg0)
{
    Enet_IoctlPrms prms;
    CpswStats_PortStats currPortStats;
    app_ethfw_stats_obj_t *obj = (app_ethfw_stats_obj_t *)arg0;
    uint8_t i, portCnt = 0U;
    Enet_MacPort portNum;
    int32_t status = ENET_SOK;

    while (!obj->ethfwStatsShutdown)
    {
        EthFwOsal_pendSemaphore(obj->clockSem, ETHFWOSAL_WAIT_FOREVER);

        /* Set Host port status to true always*/
        obj->ethfwPortBw.isportenabled[0] = BTRUE;

        /* Collect host port statistics and calculate bandwidth*/
        ENET_IOCTL_SET_OUT_ARGS(&prms, &currPortStats);
        status = Enet_ioctl(obj->hEnet, obj->coreId, ENET_STATS_IOCTL_GET_HOSTPORT_STATS, &prms);
        if (status != ENET_SOK)
        {
            appLogPrintf("ETHFW STATS: Error in collecting host port statistics: %d\n", status);
        }
        else
        {
            appEthfw_calculateBw(currPortStats,
                                 obj->prevPortStats[0U],
                                 &obj->ethfwPortBw.tx_bandwidth[0U],
                                 &obj->ethfwPortBw.rx_bandwidth[0U]);
            obj->prevPortStats[0U] = currPortStats;

        }

        portCnt = Enet_getMacPortMax(obj->enetType, obj->instId);
        for (i = 0U, portNum = ENET_MAC_PORT_FIRST; i < portCnt ; i++, portNum++)
        {
            obj->ethfwPortBw.isportenabled[i+1] = EnetAppUtils_isPortLinkUp(obj->hEnet, obj->coreId, portNum);

            if (obj->ethfwPortBw.isportenabled[i+1] == BTRUE)
            {
                /* Collect MAC port statistics and calculate bandwidth */
                ENET_IOCTL_SET_INOUT_ARGS(&prms, &portNum, &currPortStats);
                status = Enet_ioctl(obj->hEnet, obj->coreId, ENET_STATS_IOCTL_GET_MACPORT_STATS, &prms);
                if (status != ENET_SOK)
                {
                    appLogPrintf("ETHFW STATS: Error in collecting host port statistics: %d\n", status);
                }
                else
                {
                    appEthfw_calculateBw(currPortStats,
                                         obj->prevPortStats[i+1],
                                         &obj->ethfwPortBw.tx_bandwidth[i+1],
                                         &obj->ethfwPortBw.rx_bandwidth[i+1]);
                    obj->prevPortStats[i+1] = currPortStats;
                }
            }
            else
            {
                obj->ethfwPortBw.tx_bandwidth[i+1] = 0U;
                obj->ethfwPortBw.tx_bandwidth[i+1] = 0U;
            }
        }
    }
}

int32_t appEthfwStatsInit(Enet_Type enetType, uint32_t instId)
{
    int32_t status = ENET_SOK;
    EthFwOsal_TaskParams params;
    EnetPer_AttachCoreOutArgs attachInfo;
    EnetMcm_HandleInfo handleInfo;
    EnetMcm_CmdIf cmdIf;
    app_ethfw_stats_obj_t *obj;

    obj = &g_app_ethfw_stats_obj;
    memset(obj, 0, sizeof(app_ethfw_stats_obj_t));

    obj->enetType = enetType;
    obj->instId   = instId;
    obj->coreId   = EnetSoc_getCoreId();

    EnetMcm_getCmdIf(obj->enetType, &cmdIf);
    EnetAppUtils_assert(cmdIf.hMboxCmd != NULL);
    EnetAppUtils_assert(cmdIf.hMboxResponse != NULL);

    EnetMcm_acquireHandleInfo(&cmdIf, &handleInfo);
    EnetMcm_coreAttach(&cmdIf, obj->coreId, &attachInfo);

    obj->hEnet = handleInfo.hEnet;
    obj->coreKey = attachInfo.coreKey;

    obj->clockSem = EthFwOsal_createSemaphore(1U);
    if(obj->clockSem==NULL)
    {
        appLogPrintf("ETHFW STATS: Unable to create clock semaphore\n");
        status = ENET_EFAIL;
    }

    if (status == ENET_SOK)
    {
        EthFwOsal_initTaskParams(&params);
        params.name      = "EthFw stats collector Task";
        params.priority  = APP_ETHFW_STATS_COLLECTOR_TASK_PRI;
        params.stack     = g_ethfwStatsCollectorStack;
        params.stacksize = APP_ETHFW_STATS_COLLECTOR_TASK_STACKSIZE;
        params.arg0      = (void *)obj;

        obj->hStatsCollectorTask = (void *)EthFwOsal_createTask(&appEthfw_statsCollectorTask,
                                                        &params);
        if (NULL == obj->hStatsCollectorTask)
        {
            appLogPrintf("ETHFW STATS: Unable to create Stats collector task\n");
            status = ENET_EFAIL;
        }
    }

    if (status == ENET_SOK)
    {
        status = appEthfw_createClock(obj);
        if (status != ENET_SOK)
        {
            appLogPrintf("ETHFW STATS: Unable to create clock\n");
        }
    }

    if(status == ENET_SOK)
    {
        /* Now enable Ethfw statistics calculation */
        obj->ethfwStatsUpdateEnable = BTRUE;
        obj->ethfwStatsShutdown = BFALSE;
    }

    EnetMcm_releaseCmdIf(obj->enetType, &cmdIf);

    return status;
}

static void appEthfwStatsResetBwCalc(app_ethfw_stats_obj_t *obj)
{
    memset(&obj->ethfwPortBw, 0U, sizeof(app_ethfw_port_bandwidth_t));
    memset(&obj->prevPortStats, 0U, sizeof(obj->prevPortStats));
}

int32_t appEthFwStatsHandler(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags)
{
    int32_t status = 0;
    app_ethfw_stats_obj_t *obj = &g_app_ethfw_stats_obj;

    switch(cmd)
    {
        case APP_ETHFW_STATS_CMD_RESET_BANDWIDTH:
            appEthfwStatsResetBwCalc(obj);
            break;
        case APP_ETHFW_STATS_CMD_GET_BANDWIDTH:
            if(prm_size == sizeof(app_ethfw_port_bandwidth_t))
            {
                app_ethfw_port_bandwidth_t *port_bw = (app_ethfw_port_bandwidth_t*)prm;

                *port_bw = obj->ethfwPortBw;
            }
            else
            {
                status = -1;
                appLogPrintf("ETHFW STATS: ERROR: Invalid parameter size (cmd = %08x, prm_size = %d B, expected prm_size = %d B\n",
                             cmd, prm_size, sizeof(app_ethfw_port_bandwidth_t));
            }
            break;
        default:
            status = -1;
            appLogPrintf("ETHFW STATS: ERROR: Invalid command (cmd = %08x, prm_size = %d B\n",
                         cmd, prm_size);
            break;
    }

    return status;
}

int32_t appEthfwStatsRemoteServiceInit(void)
{
    int32_t status;

    status = appRemoteServiceRegister(APP_ETHFW_STATS_SERVICE_NAME, appEthFwStatsHandler);
    if(status!=0)
    {
        appLogPrintf("ETHFW STATS: ERROR: Unable to register service \n");
    }
    return status;
}


static void appEthfwStatsDeleteStatsCollectorTask(app_ethfw_stats_obj_t *obj)
{
    uint32_t sleep_time = 16U;

    /* confirm task termination */
    while ( ! EthFwOsal_isTaskTerminated(obj->hStatsCollectorTask) )
    {
        EthFwOsal_sleepTaskinMsecs(sleep_time);
        sleep_time >>= 1U;
        if (sleep_time == 0U)
        {
            /* Force delete after timeout */
            break;
        }
    }
    EthFwOsal_deleteTask(&obj->hStatsCollectorTask);
}

void appEthfwStatsDeInit(void)
{
    app_ethfw_stats_obj_t *obj = &g_app_ethfw_stats_obj;

    appRemoteServiceUnRegister(APP_ETHFW_STATS_SERVICE_NAME);

    appEthfwStatsDeleteStatsCollectorTask(obj);

    /* Delete semaphore */
    EthFwOsal_deleteSemaphore(obj->clockSem);

    /* Stop and delete the clock */
    EthFwOsal_stopClock(obj->hStatsClock);
    EthFwOsal_deleteClock(obj->hStatsClock);

    obj->ethfwStatsUpdateEnable = BFALSE;
    obj->ethfwStatsShutdown = BTRUE;

    appLogPrintf("ETHFW STATS: Deinit ... Done !!!\n");
}

