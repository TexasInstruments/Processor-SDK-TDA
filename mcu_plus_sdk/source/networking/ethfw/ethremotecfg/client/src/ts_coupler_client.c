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

/*!
 * \file ts_coupler_client.c
 *
 * \brief Ethernet Firmware Timesync Coupler client implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
/* EthFwTrace id for this module, must be unique within ETHFW */
#define ETHFWTRACE_MOD_ID 0x202
#define ETHFWTRACE_MOD_NAME "TS_CouplerClient"

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include <enet.h>
#include <include/per/cpsw.h>
#include <ti/osal/soc/osal_soc.h>
#include <ti/drv/gtc/gtc.h>

#include <utils/include/enet_apputils.h>
#include <utils/ethfw_common/include/ethfw_trace.h>
#include <utils/ethfw_common/include/ethfw_timer.h>
#include <utils/ethfw_abstract/ethfw_osal.h>

#include <ethremotecfg/client/include/ts_coupler_client.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define TS_COUPER_CLIENT_TASK_STACKSIZE         (16U * 1024U)
#define TS_COUPER_CLIENT_TASK_STACKALIGN        (32U)
#define TS_COUPER_CLIENT_TASK_PRIORITY          (3U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct TSCouplerClient_Obj_s
{
    TsCouplerClient_TimerType timerType;

    TsCouplerClient_Cfg tsCfg;

    TsCouplerClient_timeSyncTupleTbl gTsTupleInfo;

    EthFwOsal_SemHandle hTimeSyncSem;

    TimerP_Handle hTimeSyncSysTimer;

    uint64_t syncEventCount;

    uint64_t syncPeriodTicks;

    double rate;

    double offset;
#if defined(ETHFW_MTS_DEMO_TEST)
    uint64_t timerIsrCtxLSyncTime;
#endif
} TSCouplerClient_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void TsCouplerClient_task(void* arg0);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static uint8_t gTsCouplerClientStackBuf[TS_COUPER_CLIENT_TASK_STACKSIZE] __attribute__ ((section(".bss:taskStackSection"))) __attribute__((aligned(TS_COUPER_CLIENT_TASK_STACKALIGN)));

TSCouplerClient_Obj gTSCoupCliObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void TsCouplerClient_init(TsCouplerClient_initParams *params)
{
    memset(&gTSCoupCliObj, 0, sizeof(TSCouplerClient_Obj));

    gTSCoupCliObj.timerType = params->timerType;

    gTSCoupCliObj.hTimeSyncSem = EthFwOsal_createSemaphore(0U);
    EnetAppUtils_assert(gTSCoupCliObj.hTimeSyncSem  != NULL);
}

void TsCouplerClient_start(TsCouplerClient_Cfg *cfg)
{
    EthFwOsal_TaskParams taskParams;

    EnetAppUtils_assert(cfg != NULL);

    memcpy(&gTSCoupCliObj.tsCfg, cfg, sizeof(TsCouplerClient_Cfg));

    EthFwOsal_initTaskParams(&taskParams);
    taskParams.priority  = TS_COUPER_CLIENT_TASK_PRIORITY;
    taskParams.stack     = &gTsCouplerClientStackBuf[0];
    taskParams.stacksize = sizeof(gTsCouplerClientStackBuf);
    taskParams.name      = "TS Client Thread";

    EthFwOsal_createTask(&TsCouplerClient_task, &taskParams);
}

static void TsCouplerClient_TmrIsr(void* arg)
{
    gTSCoupCliObj.syncEventCount++;
    TsCouplerClient_timeSyncTuple *tuple;

    tuple = &(gTSCoupCliObj.gTsTupleInfo.tuple[gTSCoupCliObj.gTsTupleInfo.tupleIndex % ENET_ARRAYSIZE(gTSCoupCliObj.gTsTupleInfo.tuple)]);

    tuple->systemTime = ((uint64_t)gTSCoupCliObj.syncEventCount * (uint64_t)gTSCoupCliObj.syncPeriodTicks) + 
                        (uint64_t)((TimerP_getCount(gTSCoupCliObj.hTimeSyncSysTimer) - TimerP_getReloadCount(gTSCoupCliObj.hTimeSyncSysTimer)) + 1);

#if defined(ETHFW_MTS_DEMO_TEST)
    gTSCoupCliObj.timerIsrCtxLSyncTime = TsCouplerClient_getSynchronizedTime(TS_COUPLER_CLIENT_TIMER_TYPE_GPTIMER);
#endif
}

static void TsCouplerClient_task(void* arg0)
{
    bool exitTask = BFALSE;
    uint32_t freqHz;
    TimerP_Params timerParams;
    TimerP_Status status;

    if (gTSCoupCliObj.timerType == TS_COUPLER_CLIENT_TIMER_TYPE_GTC)
    {
        GTC_disable();

        GTC_selectPushEvent(gTSCoupCliObj.tsCfg.pushEvtVal);

        /* Register to Remote Timer to start TimeSync. */
        status = TsCouplerClient_registerRemoteTimer(gTSCoupCliObj.tsCfg.hwPushNum, gTSCoupCliObj.tsCfg.tsRouterTntrId);
        EnetAppUtils_assert(CPSWPROXY_SOK == status);

        GTC_enable();
    }
    else
    {
        freqHz = TIMERP_TIMER_FREQ_LO;
        EnetAppUtils_assert(TIMERP_TIMER_FREQ_HI == 0);

        gTSCoupCliObj.syncPeriodTicks = ((float)(gTSCoupCliObj.tsCfg.periodinMs * 1000U)/ (1000U * 1000U)) * (freqHz);

        /* Register to Remote Timer to start TimeSync. */
        status = TsCouplerClient_registerRemoteTimer(gTSCoupCliObj.tsCfg.hwPushNum, gTSCoupCliObj.tsCfg.tsRouterTntrId);
        EnetAppUtils_assert(CPSWPROXY_SOK == status);

        TimerP_Params_init(&timerParams);
        timerParams.runMode    = TimerP_RunMode_CONTINUOUS;
        timerParams.startMode  = TimerP_StartMode_USER;
        timerParams.periodType = TimerP_PeriodType_MICROSECS;
        timerParams.period     = (gTSCoupCliObj.tsCfg.periodinMs) * 1000U; //TimerP period takes microseconds.
    
        gTSCoupCliObj.hTimeSyncSysTimer = TimerP_create(gTSCoupCliObj.tsCfg.timerIdx, (TimerP_Fxn)&TsCouplerClient_TmrIsr, &timerParams);
        EnetAppUtils_assert(NULL != gTSCoupCliObj.hTimeSyncSysTimer);

        status = TimerP_start(gTSCoupCliObj.hTimeSyncSysTimer);
        EnetAppUtils_assert(TimerP_OK == status);

#if defined(SOC_J784S4) || defined(SOC_J7200)
        status= EthFw_TimerP_enablePWMTrigger(gTSCoupCliObj.tsCfg.timerIdx, TimerP_PWM_MATCH_VALUE_MAX, TimerP_PWMPulseMode_SINGLE_PULSE);
#elif defined(SOC_J721E)
        /* To Do: EthFw is maintaining a patch for TimerP to support PWM. This will get cleaned up in future release.
         * We have to handle the interrupt events mapping of Main domain's DM Timer instance 12  - 19 with
         * MAIN Pulsar VIMs locally here, TimerP layer handles relevant mapping internally. */
        status= EthFw_TimerP_enablePWMTrigger(gTSCoupCliObj.tsCfg.timerIdx + 12U, TimerP_PWM_MATCH_VALUE_MAX, TimerP_PWMPulseMode_SINGLE_PULSE);
#endif
        EnetAppUtils_assert(TimerP_OK == status);
    }
    
    while (!exitTask)
    {
        EthFwOsal_pendSemaphore(gTSCoupCliObj.hTimeSyncSem, ETHFWOSAL_WAIT_FOREVER);


        /* Calculate rate and offset */
        TsCouplerClient_calculateRateAndOffset(gTSCoupCliObj.gTsTupleInfo.tuple, TS_COUPLER_CLIENT_TUPLES_MAX_TUPLES, &gTSCoupCliObj.rate, &gTSCoupCliObj.offset);
    }
}

void TsCouplerClient_HwPushNotifyFxn(uint32_t notifyType,
                                     void *notifyArg,
                                     void *cbArg)
{
    TsCouplerClient_HwPushNotifyParams *params = (TsCouplerClient_HwPushNotifyParams *)notifyArg;
    TsCouplerClient_timeSyncTuple *tuple;

    tuple = &(gTSCoupCliObj.gTsTupleInfo.tuple[gTSCoupCliObj.gTsTupleInfo.tupleIndex % ENET_ARRAYSIZE(gTSCoupCliObj.gTsTupleInfo.tuple)]);

    if (gTSCoupCliObj.timerType == TS_COUPLER_CLIENT_TIMER_TYPE_GTC)
    {
        if (gTSCoupCliObj.gTsTupleInfo.tupleIndex == 0U)
        {
            tuple->phcTime = params->timestamp;

            GTC_disable();
            tuple->systemTime = (uint64_t)(1U << gTSCoupCliObj.tsCfg.pushEvtVal);
            GTC_setCounter64(tuple->systemTime);
            GTC_enable();
        }
        else
        {
            tuple->phcTime = params->timestamp;
            tuple->systemTime = (uint64_t)(gTSCoupCliObj.gTsTupleInfo.tuple[gTSCoupCliObj.gTsTupleInfo.tupleIndex - 1U].systemTime) + 
                                (uint64_t)(1U << gTSCoupCliObj.tsCfg.pushEvtVal);
        }
    }
    else
    {
        tuple->phcTime = params->timestamp;

    }

    gTSCoupCliObj.gTsTupleInfo.tupleIndex++;

#if defined(ETHFW_MTS_DEMO_TEST)
    /* For MTS accuracy test, print the delta between local synchronized time and remote time every 10s. */
    if ((gTSCoupCliObj.syncEventCount % 10U == 0U) && (gTSCoupCliObj.timerIsrCtxLSyncTime != 0U))
    {
        uint64_t deltaTime = 0U;
        
        if (tuple->phcTime > gTSCoupCliObj.timerIsrCtxLSyncTime)
        {
            deltaTime = (uint64_t)(tuple->phcTime - gTSCoupCliObj.timerIsrCtxLSyncTime);
        }
        else
        {
            deltaTime = (uint64_t)(gTSCoupCliObj.timerIsrCtxLSyncTime - tuple->phcTime);
        }
        ETHFWTRACE_INFO("IsrPhcTime: %lld RemotePhcTime: %lld Delta time: %lld", (uint64_t)(gTSCoupCliObj.timerIsrCtxLSyncTime),(uint64_t)(tuple->phcTime), (uint64_t)(deltaTime));
    }
#endif

    EthFwOsal_postSemaphore((EthFwOsal_SemHandle)gTSCoupCliObj.hTimeSyncSem);
}

void TsCouplerClient_calculateRateAndOffset(TsCouplerClient_timeSyncTuple *entries,
                                            uint32_t numEntries,
                                            double *rate,
                                            double *offset)
{
    uint64_t prevSystemTime = 0U;
    uint64_t prevPhcTime = 0U;
    double temp1 = 0.0;
    double temp2 = 0.0;

    // Iterate through the entries to find the first two valid entries
    for (uint32_t i = 0; i < numEntries; i++) {
        if (entries[i].systemTime != 0 && entries[i].phcTime != 0) {
            if (prevSystemTime == 0 && prevPhcTime == 0) {
                prevSystemTime = entries[i].systemTime;
                prevPhcTime = entries[i].phcTime;
            } else {
                // Calculate rate and offset using CRF algorithm
                double systemTimeDiff = (double)(entries[i].systemTime - prevSystemTime);
                double phcTimeDiff = (double)(entries[i].phcTime - prevPhcTime);
                *rate = phcTimeDiff / systemTimeDiff;
                temp1 = (double)entries[i].systemTime * ((double)prevPhcTime / systemTimeDiff);
                temp2 = (double)prevSystemTime * ((double)entries[i].phcTime / systemTimeDiff);
                *offset = temp1 - temp2;

                // Update previous system time and PHC time
                prevSystemTime = entries[i].systemTime;
                prevPhcTime = entries[i].phcTime;
                break;
            }
        }
    }
}

uint64_t TsCouplerClient_getSynchronizedTime(TsCouplerClient_TimerType timerType)
{
    uint64_t localTime = 0U, synchronizedTime = 0U;

    if(timerType == TS_COUPLER_CLIENT_TIMER_TYPE_GTC)
    {
        localTime = GTC_readCounter64();
    }
    else
    {
        localTime = ((uint64_t)gTSCoupCliObj.syncEventCount * (uint64_t)gTSCoupCliObj.syncPeriodTicks) + 
                    (uint64_t)((TimerP_getCount(gTSCoupCliObj.hTimeSyncSysTimer) - TimerP_getReloadCount(gTSCoupCliObj.hTimeSyncSysTimer)) + 1);
    }

    synchronizedTime = (uint64_t)((double)localTime * gTSCoupCliObj.rate + gTSCoupCliObj.offset);

    return synchronizedTime;
}