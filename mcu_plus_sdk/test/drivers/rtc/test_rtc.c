/*
 * Copyright (C) 2024 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <unity.h>
#include <drivers/rtc.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/SemaphoreP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structures and Enums                               */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/* Test Cases */
static void test_rtc_set_get_time(void *args);
static void test_rtc_multiple_on_off_interrupts(void *args);
static void test_rtc_multiple_off_on_interrupts(void *args);

/* Callback Functions */
void TestRtc_onOffCallback(RTC_Handle handle);
void TestRtc_offOnCallback(RTC_Handle handle);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static SemaphoreP_Object gOnOffSemObj;
static SemaphoreP_Object gOffOnSemObj;

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void test_main(void *args)
{
    /* Open RTC instance */
    gRTCHandle[CONFIG_RTC0] = RTC_open(CONFIG_RTC0, &gRTCParams[CONFIG_RTC0]);

    /* Open drivers to open the UART driver for console */
    UNITY_BEGIN();

    RUN_TEST(test_rtc_set_get_time, 5822, NULL);
    RUN_TEST(test_rtc_multiple_on_off_interrupts, 5823, NULL);
    RUN_TEST(test_rtc_multiple_off_on_interrupts, 5824, NULL);

    UNITY_END();

    /* Close RTC instance */
    if(gRTCHandle[CONFIG_RTC0] != NULL)
    {
        RTC_close(gRTCHandle[CONFIG_RTC0]);
        gRTCHandle[CONFIG_RTC0] = NULL;
    }

    return;
}

/*
 * Unity framework required functions
 */
void setUp(void)
{
}

void tearDown(void)
{
}

/*
 * Testcases
 */
static void test_rtc_set_get_time(void* args)
{
    RTC_Handle rtcHandle = NULL;
    RTC_Time setTime, getTime;
    int32_t status = SystemP_SUCCESS;

    DebugP_log("RTC set and get time test starting...\r\n");

    /* Store handle for Local use */
    if(gRTCHandle[CONFIG_RTC0] != NULL)
    {
        rtcHandle = gRTCHandle[CONFIG_RTC0];
    }
    else {
        status = SystemP_FAILURE;
    }

    /* Set the current time (e.g., 5th November 2024, 14:42:10) */
    setTime.year = 2024U;
    setTime.month = 11U;
    setTime.day = 5U;
    setTime.hour = 14U;
    setTime.minute = 42U;
    setTime.second = 10U;
    status = RTC_setTime(rtcHandle, &setTime);
    TEST_ASSERT_EQUAL_INT32(SystemP_SUCCESS, status);

    /* Wait for 5 seconds */
    ClockP_sleep(5U);

    /* Retrieve the time */
    status = RTC_getTime(rtcHandle, &getTime);
    TEST_ASSERT_EQUAL_INT32(SystemP_SUCCESS, status);

    TEST_ASSERT_UINT32_WITHIN(1U,15U,getTime.second);
}

static void test_rtc_multiple_on_off_interrupts(void* args)
{
    RTC_Handle rtcHandle = NULL;
    RTC_Time setTime;
    int32_t status = SystemP_SUCCESS;
    uint32_t i;

    DebugP_log("RTC multiple on-off interrupt test starting...\r\n");

    /* Store handle for Local use */
    if(gRTCHandle[CONFIG_RTC0] != NULL)
    {
        rtcHandle = gRTCHandle[CONFIG_RTC0];
    }
    else {
        status = SystemP_FAILURE;
    }

    SemaphoreP_constructBinary(&gOnOffSemObj, 0U);

    /* Set the current time (e.g., 5th November 2024, 14:45:10) */
    setTime.year = 2024U;
    setTime.month = 11U;
    setTime.day = 5U;
    setTime.hour = 14U;
    setTime.minute = 45U;
    setTime.second = 10U;

    status = RTC_setTime(rtcHandle, &setTime);
    TEST_ASSERT_EQUAL_INT32(SystemP_SUCCESS, status);

    /* Set multiple On-Off interrupts */
    for(i=1U; i<=3U; i++)
    {
        setTime.second += (2U);
        RTC_setOn_OffTimerEvent(rtcHandle, &setTime);

        /* Wait for the interrupt */
        SemaphoreP_pend(&gOnOffSemObj, SystemP_WAIT_FOREVER);

        DebugP_log("On-Off Interrupt %d triggered successfully!\r\n", i);
    }
    TEST_ASSERT_EQUAL_UINT32(4U,i);
    SemaphoreP_destruct(&gOnOffSemObj);
}

static void test_rtc_multiple_off_on_interrupts(void* args)
{
    RTC_Handle rtcHandle = NULL;
    RTC_Time setTime;
    int32_t status = SystemP_SUCCESS;
    uint32_t i;

    DebugP_log("RTC multiple off-on interrupt test starting...\r\n");

    /* Store handle for Local use */
    if(gRTCHandle[CONFIG_RTC0] != NULL)
    {
        rtcHandle = gRTCHandle[CONFIG_RTC0];
    }
    else {
        status = SystemP_FAILURE;
    }

    SemaphoreP_constructBinary(&gOffOnSemObj, 0U);

    /* Set the current time (e.g., 7th November 2024, 14:45:10) */
    setTime.year = 2024U;
    setTime.month = 11U;
    setTime.day = 7U;
    setTime.hour = 14U;
    setTime.minute = 45U;
    setTime.second = 10U;

    status = RTC_setTime(rtcHandle, &setTime);
    TEST_ASSERT_EQUAL_INT32(SystemP_SUCCESS, status);

    /* Set multiple Off-On interrupts */
    for(i=1U; i<=3U; i++)
    {
        setTime.second += (2U);
        RTC_setOff_OnTimerEvent(rtcHandle, &setTime);

        /* Wait for the interrupt */
        SemaphoreP_pend(&gOffOnSemObj, SystemP_WAIT_FOREVER);

        DebugP_log("Off-On Interrupt %d triggered successfully!\r\n", i);
    }
    TEST_ASSERT_EQUAL_UINT32(4U,i);
    SemaphoreP_destruct(&gOffOnSemObj);
}

void TestRtc_onOffCallback(RTC_Handle handle)
{
    /* Post Semaphore */
    SemaphoreP_post(&gOnOffSemObj);
}

void TestRtc_offOnCallback(RTC_Handle handle)
{
    /* Post Semaphore */
    SemaphoreP_post(&gOffOnSemObj);
}