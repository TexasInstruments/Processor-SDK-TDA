/*
 *  Copyright (C) 2024 Texas Instruments Incorporated
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
 *  A PArtcCULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This example configures a GPIO pin connected to an LED on the EVM in
 * output mode.
 * The application toggles the LED on/off using rtc timer.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <drivers/gpio.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <kernel/dpl/DebugP.h>
#include <drivers/rtc.h>
#include <kernel/dpl/SemaphoreP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define LED_BLINK_COUNT  (10U)

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                   External Function Declarations                           */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

SemaphoreP_Object gOnOffSemObj;

volatile uint32_t gBlinkCount;
uint32_t gpioBaseAddr, pinNum;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void rtc_led_blink(void *args)
{
    RTC_Handle rtcHandle = NULL;
    RTC_Time setTime;
    int32_t status = SystemP_SUCCESS;

    DebugP_log("[RTC LED Blink Test] Starting ...\r\n");

    /* Open RTC instance */
    gRTCHandle[CONFIG_RTC0] = RTC_open(CONFIG_RTC0, &gRTCParams[CONFIG_RTC0]);

    if(NULL == gRTCHandle[CONFIG_RTC0])
    {
        DebugP_logError("RTC open failed for instance %d !!!\r\n", CONFIG_RTC0);
        status = SystemP_FAILURE;
    }

    rtcHandle = gRTCHandle[CONFIG_RTC0];

    /* Get address after translation translate */
    gpioBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(GPIO_LED_BASE_ADDR);
    pinNum       = GPIO_LED_PIN;
    gBlinkCount = 1U;

    /* Set LED GPIO pin in output mode */
    GPIO_setDirMode(gpioBaseAddr, pinNum, GPIO_LED_DIR);
    /* Set LED GPIO pin HIGH */
    GPIO_pinWriteHigh(gpioBaseAddr, pinNum);

    /* Set the time (e.g., 7th November 2024, 14:45:10) */
    setTime.year = 2024U;
    setTime.month = 11U;
    setTime.day = 7U;
    setTime.hour = 14U;
    setTime.minute = 45U;
    setTime.second = 10U;
    status = RTC_setTime(rtcHandle, &setTime);

    if(status == SystemP_SUCCESS)
    {
        SemaphoreP_constructBinary(&gOnOffSemObj, 0U);

        DebugP_log("[RTC LED Blink Test] Started...\r\n");

        /* Wait until the LED is blinked specified number of times */
        while(gBlinkCount <= (LED_BLINK_COUNT/2U))
        {
            setTime.second += (2U);
            RTC_setOn_OffTimerEvent(rtcHandle, &setTime);

            /* Wait for the interrupt */
            SemaphoreP_pend(&gOnOffSemObj, SystemP_WAIT_FOREVER);

            /* Set LED GPIO pin LOW */
            GPIO_pinWriteLow(gpioBaseAddr, pinNum);

            DebugP_log("LED blinked successfully at time: %02d/%02d/%04d %02d:%02d:%02d\r\n",
                setTime.day, setTime.month, setTime.year,
                setTime.hour, setTime.minute, setTime.second);

            setTime.second += (2U);
            RTC_setOn_OffTimerEvent(rtcHandle, &setTime);

            /* Wait for the interrupt */
            SemaphoreP_pend(&gOnOffSemObj, SystemP_WAIT_FOREVER);

            /* Set LED GPIO pin HIGH */
            GPIO_pinWriteHigh(gpioBaseAddr, pinNum);

            DebugP_log("LED blinked successfully at time: %02d/%02d/%04d %02d:%02d:%02d\r\n",
                setTime.day, setTime.month, setTime.year,
                setTime.hour, setTime.minute, setTime.second);

            gBlinkCount++;
        }

        DebugP_log("RTC LED blink test passed!!\r\n");

        SemaphoreP_destruct(&gOnOffSemObj);
    }
    else
    {
        DebugP_log("RTC LED blink test failed!!\r\n");
    }

    /* Close RTC instance */
     if(gRTCHandle[CONFIG_RTC0] != NULL)
    {
        RTC_close(gRTCHandle[CONFIG_RTC0]);
        gRTCHandle[CONFIG_RTC0] = NULL;
    }
}

void rtc_onOffBlink(void)
{
    /* Post Semaphore */
    SemaphoreP_post(&gOnOffSemObj);
}