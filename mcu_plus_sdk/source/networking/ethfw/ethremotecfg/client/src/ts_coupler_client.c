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
#include <math.h>

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

#define PPMFACTOR                               (1000000U) 
#define MAX_PPM_CORRECTION                      (1000U)

/**
 * The actual clock to CPTS is not restricted to 1GHz but
 * the CPTS timestamp value is increamented by 1GHz without gPTP.
 * In presence of gPTP this Timestamp is corrected by PPM
 */
#define CPTS_TIMER_FREQ                         (1000000000U)

/**
 *  ALPHA is the Learning rate coefficient. It is used for adjusting ppm correction
 *  based on Last iteration errors.
 *  ALPHA = 0 means no learning from past
 *  ALPHA = 1.0 means fully use last ppm correction.
 */
#define ALPHA                                   (0.95)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

typedef struct TSCouplerClient_Obj_s
{
    TsCouplerClient_TimerType timerType;

    TsCouplerClient_Cfg tsCfg;

    uint32_t tupleIndex;
    
    TsCouplerClient_timeSyncTuple prevTuple;
    
    TsCouplerClient_timeSyncTuple currTuple;

    EthFwOsal_SemHandle hTimeSyncSem;

    TimerP_Handle hTimeSyncSysTimer;

    uint64_t syncEventCount;

    uint64_t syncPeriodTicks;

    double baseRate;

    double ppmRate;

} TSCouplerClient_Obj;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void TsCouplerClient_task(void* arg0);

static void TsCouplerClient_TmrIsr(void* arg);

static void TsCouplerClient_calculateRateAndOffset();

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
    EnetAppUtils_assert(cfg != NULL);
    memcpy(&gTSCoupCliObj.tsCfg, cfg, sizeof(TsCouplerClient_Cfg));

    uint32_t freqHz;
    TimerP_Params timerParams;
    TimerP_Status status;

    if (gTSCoupCliObj.timerType == TS_COUPLER_CLIENT_TIMER_TYPE_GTC)
    {
        freqHz = GTC_CLK_RATE_200_MHZ; 
        gTSCoupCliObj.baseRate = (double)CPTS_TIMER_FREQ/(double)freqHz;
        gTSCoupCliObj.ppmRate = 0;

        ETHFWTRACE_INFO("Base rate: %lf",gTSCoupCliObj.baseRate);

        GTC_init();

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

        gTSCoupCliObj.syncPeriodTicks = (uint64_t)(((double)(gTSCoupCliObj.tsCfg.periodinMs)/ 1000.0) * (freqHz));
        gTSCoupCliObj.baseRate = (double)CPTS_TIMER_FREQ/(double)freqHz;
        gTSCoupCliObj.ppmRate = 0;

        ETHFWTRACE_INFO("Base rate: %lf",gTSCoupCliObj.baseRate);

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

#if defined(SOC_J784S4) || defined(SOC_J7200)
        status= EthFw_TimerP_enablePWMTrigger(gTSCoupCliObj.tsCfg.timerIdx, TimerP_PWM_MATCH_VALUE_MAX, TimerP_PWMPulseMode_SINGLE_PULSE);
#elif defined(SOC_J721E)
        /* To Do: EthFw is maintaining a patch for TimerP to support PWM. This will get cleaned up in future release.
         * We have to handle the interrupt events mapping of Main domain's DM Timer instance 12  - 19 with
         * MAIN Pulsar VIMs locally here, TimerP layer handles relevant mapping internally. */
        status= EthFw_TimerP_enablePWMTrigger(gTSCoupCliObj.tsCfg.timerIdx + 12U, TimerP_PWM_MATCH_VALUE_MAX, TimerP_PWMPulseMode_SINGLE_PULSE);
#endif
        EnetAppUtils_assert(TimerP_OK == status);

        status = TimerP_start(gTSCoupCliObj.hTimeSyncSysTimer);
        EnetAppUtils_assert(TimerP_OK == status);
    }
    
    EthFwOsal_TaskParams taskParams;
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
}

static void TsCouplerClient_task(void* arg0)
{
    bool exitTask = BFALSE;
   
    while (!exitTask)
    {
        EthFwOsal_pendSemaphore(gTSCoupCliObj.hTimeSyncSem, ETHFWOSAL_WAIT_FOREVER);

        /* Calculate rate and offset from previous and current tuples */
        if(gTSCoupCliObj.syncEventCount >= 2)
        {
            TsCouplerClient_calculateRateAndOffset();
        }
    }
}

void TsCouplerClient_HwPushNotifyFxn(uint32_t notifyType,
                                     void *notifyArg,
                                     void *cbArg)
{
    TsCouplerClient_HwPushNotifyParams *params = (TsCouplerClient_HwPushNotifyParams *)notifyArg;
    TsCouplerClient_timeSyncTuple *tuple;
    TsCouplerClient_timeSyncTuple *prevTuple;

    tuple = &gTSCoupCliObj.currTuple;
    prevTuple = &gTSCoupCliObj.prevTuple;

    memcpy(prevTuple,tuple,sizeof(TsCouplerClient_timeSyncTuple));

    if (gTSCoupCliObj.timerType == TS_COUPLER_CLIENT_TIMER_TYPE_GTC)
    {
        if (gTSCoupCliObj.tupleIndex == 0U)
        {
            tuple->phcTime = params->timestamp;

            GTC_disable();
            tuple->systemTime = (uint64_t)(1ULL << gTSCoupCliObj.tsCfg.pushEvtVal);
            GTC_setCounter64(tuple->systemTime);
            GTC_enable();
        }
        else
        {
            tuple->phcTime = params->timestamp;
            /**
            * gTSCoupCliObj.tsCfg.pushEvtVal bit of GTC timer is used to generate CPTS Push event
            * but CPTS Push event is triggered only on the rising edge. 
            * Thus CPTS trigger and by extension this callback will occur every 2^(gTSCoupCliObj.tsCfg.pushEvtVal+1) ticks.
            */
            tuple->systemTime = (uint64_t)(gTSCoupCliObj.prevTuple.systemTime) +
                                (uint64_t)(1ULL << (gTSCoupCliObj.tsCfg.pushEvtVal+1));
        }
        gTSCoupCliObj.syncEventCount++;
    }
    else
    {
        tuple->phcTime = params->timestamp;
        tuple->systemTime = ((uint64_t)gTSCoupCliObj.syncEventCount * (uint64_t)gTSCoupCliObj.syncPeriodTicks);
    }

    gTSCoupCliObj.tupleIndex++;

    EthFwOsal_postSemaphore((EthFwOsal_SemHandle)gTSCoupCliObj.hTimeSyncSem);
}

static void TsCouplerClient_calculateRateAndOffset()
{
    int64_t timerDiff,cptsDiff,deltaTime;
    uint64_t estimatedPhcTime;
    double rate;

    /**
    *  CPTS time  -> T
    *  timer time -> t
    *  Central equation of time estimation is 
    *  T_(i+1) = T_i + (t_(i+1)-t_i)*(R+r*10^6)
    *  Where R is base rate which is fixed to (CPTS_freq/timer_freq)
    *  when gPTP is involved CPTS clock is PPM corrected
    *  r tries to capture that PPM correction part.
    *  value of r is changed based on the error.  
    */

    timerDiff =  gTSCoupCliObj.currTuple.systemTime - gTSCoupCliObj.prevTuple.systemTime;
    cptsDiff  =  gTSCoupCliObj.currTuple.phcTime - gTSCoupCliObj.prevTuple.phcTime;

    if( timerDiff < 0 || cptsDiff < 0)
    {
        EnetAppUtils_assert(false);
    }
    
    rate = gTSCoupCliObj.baseRate + (gTSCoupCliObj.ppmRate/(double)PPMFACTOR);

    if(!isfinite(rate))
    {
        EnetAppUtils_assert(false);
    }

    estimatedPhcTime = gTSCoupCliObj.prevTuple.phcTime + (uint64_t)(timerDiff*rate);

    deltaTime = gTSCoupCliObj.currTuple.phcTime - estimatedPhcTime;

    if(timerDiff != 0)
    {
        double currentPpmRate = (((double)cptsDiff - (double)timerDiff*gTSCoupCliObj.baseRate)*PPMFACTOR)/(double)timerDiff;
        /**
         * If following condition is false, that indecates CPTS timer value is "Set" in the beginning of gPTP.
         * Excluding this tuple instance in order to maintain stability of ppmRate 
         */
        if(fabs(currentPpmRate/gTSCoupCliObj.baseRate) < MAX_PPM_CORRECTION)
        {
            gTSCoupCliObj.ppmRate += (ALPHA*deltaTime*PPMFACTOR)/(timerDiff);
        }
        else
        {
            ETHFWTRACE_INFO("CPTS set detected");
        }
    }
    else
    {
        ETHFWTRACE_INFO("Zero Timer diff: indicates Timer ISR miss");
    }

#if defined(ETHFW_MTS_DEMO_TEST)
    if(gTSCoupCliObj.syncEventCount % 10 == 0)
    {
        
        ETHFWTRACE_INFO(" %lld Timer ticks: %lld CPTS timestamp: %lld Delta time: %5lld ns Estimated PPM %lf",\
                gTSCoupCliObj.syncEventCount ,gTSCoupCliObj.currTuple.systemTime, gTSCoupCliObj.currTuple.phcTime, (uint64_t)(deltaTime),(gTSCoupCliObj.ppmRate/gTSCoupCliObj.baseRate));
    }
#endif
}

uint64_t TsCouplerClient_getSynchronizedTime()
{
    uint64_t localTime = 0U, synchronizedTime = 0U;
    double rate = gTSCoupCliObj.baseRate + (gTSCoupCliObj.ppmRate/(double)PPMFACTOR);

    if(gTSCoupCliObj.timerType == TS_COUPLER_CLIENT_TIMER_TYPE_GTC)
    {
        localTime = GTC_readCounter64();
    }
    else
    {
        /**
         * Following loop is to prevent race condition with TsCouplerClient_TmrIsr.
         * as gTSCoupCliObj.syncEventCount is shared variable.
        */
        uint64_t overflow_count1, overflow_count2,current_timercount;
        do
        {
            overflow_count1 = gTSCoupCliObj.syncEventCount;
            current_timercount = (uint64_t)((TimerP_getCount(gTSCoupCliObj.hTimeSyncSysTimer) - TimerP_getReloadCount(gTSCoupCliObj.hTimeSyncSysTimer)) + 1);
            overflow_count2 = gTSCoupCliObj.syncEventCount;
        }
        while(overflow_count1 != overflow_count2);
        localTime = ((uint64_t)overflow_count2 * (uint64_t)gTSCoupCliObj.syncPeriodTicks);
        localTime += current_timercount;      
    }
    
    synchronizedTime = gTSCoupCliObj.currTuple.phcTime;
    synchronizedTime += (uint64_t)((double)(localTime - gTSCoupCliObj.currTuple.systemTime) * rate);

    return synchronizedTime;
}