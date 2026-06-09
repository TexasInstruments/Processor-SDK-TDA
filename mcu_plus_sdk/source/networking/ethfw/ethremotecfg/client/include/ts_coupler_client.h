/*
 *  Copyright (c) Texas Instruments Incorporated 2025
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * \file  ts_coupler_client.h
 *
 * \brief Ethernet Firmware Timesync Coupler client interface.
 */

#ifndef TS_COUPLER_CLIENT_H_
#define TS_COUPLER_CLIENT_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */


#include <stdint.h>

#include <enet.h>
#include <include/per/cpsw.h>
#include <ethremotecfg/client/include/cpsw_proxy.h>

#ifdef __cplusplus
extern "C" {
#endif


/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

#define TS_COUPLER_CLIENT_TUPLES_MAX_TUPLES          (100U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */
typedef enum TsCouplerClient_TimerTypes_e
{
    TS_COUPLER_CLIENT_TIMER_TYPE_GPTIMER = 0,
    TS_COUPLER_CLIENT_TIMER_TYPE_GTC     = 1,
} TsCouplerClient_TimerType;

typedef struct  TsCouplerClient_initParams_s
{
    /*! Timer Type */
    TsCouplerClient_TimerType timerType;
} TsCouplerClient_initParams;

typedef struct  tsCouplerClient_Cfg_s
{
    uint32_t hwPushNum;

    uint32_t timerIdx;
    
    uint32_t periodinMs;

    uint32_t pushEvtVal;

    uint32_t tsRouterTntrId;
} TsCouplerClient_Cfg;

typedef struct TsCouplerClient_timeSyncTuple_s
{
    uint64_t systemTime;
    uint64_t phcTime;
} TsCouplerClient_timeSyncTuple;

typedef struct TsCouplerClient_timeSyncTupleTbl_s
{
   uint32_t tupleIndex;
   TsCouplerClient_timeSyncTuple tuple[TS_COUPLER_CLIENT_TUPLES_MAX_TUPLES];
}TsCouplerClient_timeSyncTupleTbl;

/*!
 * \brief CPTS HW push notify params.
 *
 * Parameters passed in \ref ETHREMOTECFG_NOTIFY_HWPUSH notification.
 */
typedef struct TsCouplerClient_HwPushNotifyParams_s
{
    /* CPTS HW push number */
    uint8_t hwPushNum;

    /* CPTS timestamp */
    uint64_t timestamp;
} TsCouplerClient_HwPushNotifyParams;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/*!
 * \brief Timesync Coupler Client inititalization.
 *
 * Initializes the Timesync Coupler Client.
 *
 * \param params    Init configuration params.
 * 
 */
void TsCouplerClient_init(TsCouplerClient_initParams *params);

void TsCouplerClient_start(TsCouplerClient_Cfg *cfg);

void TsCouplerClient_HwPushNotifyFxn(uint32_t notifyType,
                                     void *notifyArg,
                                     void *cbArg);

void TsCouplerClient_calculateRateAndOffset(TsCouplerClient_timeSyncTuple *entries,
                                            uint32_t numEntries,
                                            double *rate,
                                            double *offset);


int32_t TsCouplerClient_allocHwPushInst(uint32_t *hwPushNum);

int32_t TsCouplerClient_registerRemoteTimer(uint32_t hwPushNum,
                                            uint32_t tsRouterTntrId);


uint64_t TsCouplerClient_getSynchronizedTime(TsCouplerClient_TimerType timerType);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */


#ifdef __cplusplus
}
#endif

#endif /* TS_COUPLER_CLIENT_H_ */

