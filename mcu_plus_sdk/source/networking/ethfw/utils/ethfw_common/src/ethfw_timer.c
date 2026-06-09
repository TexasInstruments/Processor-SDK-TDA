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
 * \file ethfw_timer.c
 *
 * \brief Ethernet Firmware timer functionality abstraction implementation.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <utils/ethfw_common/include/ethfw_timer.h>

#include <ti/csl/soc.h>
#include <ti/csl/csl_timer.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
static inline uintptr_t EthFw_TimerP_getTimerBaseAddr(uint32_t timerIdx)
{
    uintptr_t baseAddr = NULL;

    /* Only Timer14 -19 supports Timesync capability, temp check to pass valid timerIdx.
     * If timerIdx is not supported, return NULL. */
    if ((timerIdx >=14U) && (timerIdx <= 19U))
    {
        baseAddr = (uintptr_t)CSL_TIMER0_CFG_BASE + (0x10000U * timerIdx);
    }

    return baseAddr;
}

uint32_t EthFw_TimerP_setTimerMode(uint32_t timerIdx, uint32_t value)
{
    int32_t status = ETHFW_SOK;
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);
  
    if ((uintptr_t)0U != baseTimerAddr) 
    {
        status = TIMERGPOConfigure(baseTimerAddr, value);
    }

    return status;
}

uint32_t  EthFw_TimerP_getMatchVal(uint32_t timerIdx)
{
    uint32_t val = 0U;
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);

    if ((uintptr_t)0U != baseTimerAddr) 
    {
        (void)TIMERMatchValGet(baseTimerAddr, &val);
    }

    return val;

}

uint32_t  EthFw_TimerP_setCaptureMode(uint32_t timerIdx, uint32_t value)
{
    int32_t status = ETHFW_SOK;
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);

    if ((uintptr_t)0U != baseTimerAddr) 
    {
        status = TIMERCaptureModeEn(baseTimerAddr, value);
    }

    return status;
}

uint32_t  EthFw_TimerP_CaptureIntEn(uint32_t timerIdx, uint32_t value)
{
    int32_t status = ETHFW_SOK;
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);

    if ((uintptr_t)0U != baseTimerAddr) 
    {
        status = TIMERIntEnable(baseTimerAddr, value);
    }

    return status;
}

uint32_t  EthFw_TimerP_CaptureIntClear(uint32_t timerIdx, uint32_t value)
{
    int32_t status = ETHFW_SOK;
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);

    if ((uintptr_t)0U != baseTimerAddr) 
    {
        status = TIMERIntStatusClear(baseTimerAddr, value); 
    }

    return status;
}


uint32_t  EthFw_TimerP_readCapture1Time(uint32_t timerIdx, uint32_t *pReadTime)
{
    int32_t status = ETHFW_SOK;
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);

  if ((uintptr_t)0U != baseTimerAddr) 
  {
    status = TIMERCapture1Read(baseTimerAddr, pReadTime);
  }

  return status;
}

static uint32_t  EthFw_TimerP_setPWMMode(uintptr_t baseAddr, uint32_t triggerMode, uint32_t pulseType, uint32_t matchValue)
{  
    int32_t cslStatus = CSL_PASS;
    uint32_t retVal = ETHFW_SOK;

    if (TIMER_TCLR_TRG_MODE_OVERFLOW_MATCH == triggerMode)
    {
        cslStatus = TIMERCompareSet(baseAddr, matchValue);
    }

    if (cslStatus != CSL_PASS)
    {
        retVal = ETHFW_EFAIL;
    }

    if (retVal == ETHFW_SOK)
    {
        cslStatus = TIMERSetTriggerMode(baseAddr, triggerMode);

        if (cslStatus != CSL_PASS)
        {
            retVal = ETHFW_EFAIL;
        }
    }
    
    if (retVal == ETHFW_SOK)
    {
        cslStatus = TIMERSetPulseMode(baseAddr, pulseType);
    }

    if (cslStatus != CSL_PASS)
    {
        retVal = ETHFW_EFAIL;
    }

    return retVal;

}

uint32_t  EthFw_TimerP_enablePWMTrigger(uint32_t timerIdx, uint32_t matchValue, EthFw_TimerP_PWMPulseMode pwmPulseMode)
{
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);
    uint32_t retVal = ETHFW_SOK;
    uint32_t triggerMode;
    uint32_t pulseType;

    if (matchValue == TimerP_PWM_MATCH_VALUE_MAX)
    {
    triggerMode = TIMER_TCLR_TRG_MODE_OVERFLOW;
    }
    else {
        triggerMode = TIMER_TCLR_TRG_MODE_OVERFLOW_MATCH;
    }

    if (pwmPulseMode == TimerP_PWMPulseMode_SINGLE_PULSE)
    {
        pulseType = TIMER_TCLR_PT_MODE_PULSE;
    }
    else {
        pulseType = TIMER_TCLR_PT_MODE_TOGGLE;
    }

    retVal = EthFw_TimerP_setPWMMode(baseTimerAddr, triggerMode, pulseType,matchValue);

  return retVal;
}

uint32_t  EthFw_TimerP_disablePWMTrigger(uint32_t timerIdx)
{
    uintptr_t baseTimerAddr = EthFw_TimerP_getTimerBaseAddr(timerIdx);
    uint32_t retVal = ETHFW_SOK;

    retVal = EthFw_TimerP_setPWMMode(baseTimerAddr, TIMER_TCLR_TRG_MODE_NONE, TIMER_TCLR_PT_MODE_PULSE, TimerP_PWM_MATCH_VALUE_MAX);

  return retVal;
}
