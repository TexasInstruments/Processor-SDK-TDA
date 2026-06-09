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
 * \file  ethfw_timer.h
 *
 * \brief Ethernet Firmware timer funtionality interface.
 */

#ifndef ETHFW_TIMER_H_
#define ETHFW_TIMER_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */


#include <stdint.h>
#include <utils/ethfw_common/include/ethfw_types.h>
#include <ti/csl/csl_timer.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \ingroup  ETHFW_UTILS
 * \defgroup ETHFW_UTILS_TIMER Ethernet Firmware Timer API
 *
 * @{
 */

/* ========================================================================== */
/*                                 Macros                                     */
/* ========================================================================== */

#define TimerP_PWM_MATCH_VALUE_MAX        (~0U)

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */
/*!
 * @brief Timer mode for PWM Pulse.
 */
typedef enum EthFw_TimerP_PWMPulseMode_e {
    TimerP_PWMPulseMode_SINGLE_PULSE,
    TimerP_PWMPulseMode_TOGGLE
}EthFw_TimerP_PWMPulseMode;

/* ========================================================================== */
/*                         Global Variables Declarations                      */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

uint32_t EthFw_TimerP_setTimerMode(uint32_t timerIdx, uint32_t value);

uint32_t EthFw_TimerP_getMatchVal(uint32_t timerIdx);

uint32_t EthFw_TimerP_setCaptureMode(uint32_t timerIdx, uint32_t value);

uint32_t EthFw_TimerP_CaptureIntEn(uint32_t timerIdx, uint32_t value);

uint32_t EthFw_TimerP_CaptureIntClear(uint32_t timerIdx, uint32_t value);

uint32_t EthFw_TimerP_setTimerMode(uint32_t timerIdx, uint32_t value);

uint32_t EthFw_TimerP_readCapture1Time(uint32_t timerIdx, uint32_t *pReadTime);

uint32_t EthFw_TimerP_getLastPWMCount(uint32_t timerIdx);

uint32_t EthFw_TimerP_enablePWMTrigger(uint32_t timerIdx, uint32_t matchValue, EthFw_TimerP_PWMPulseMode pwmPulseMode);

/* ========================================================================== */
/*                        Deprecated Function Declarations                    */
/* ========================================================================== */

/* None */

/*! @} */

#ifdef __cplusplus
}
#endif

#endif /* ETHFW_TIMER_H_ */

