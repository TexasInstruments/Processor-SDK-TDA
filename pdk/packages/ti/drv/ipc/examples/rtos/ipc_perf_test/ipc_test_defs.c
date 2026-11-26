/*
 *  Copyright (c) Texas Instruments Incorporated 2019
 *  All rights reserved.
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

/**
 *  \file ipc_test_defs.c
 *
 *  \brief definitions of the testcases
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ti/drv/ipc/ipc.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

#include "ipc_test_defs.h"

#define  RUN_CNT                10000U
#define  MAX_TEST_RESULT_CNT    64U

static Ipc_Testcase gTestCases[] =
{
#if defined(SOC_J721E)
    {  1U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU1_0, RUN_CNT, 0U },
    {  2U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU3_0, RUN_CNT, 0U },
    {  3U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C66X_1, RUN_CNT, 0U },
    {  4U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C7X_1,  RUN_CNT, 0U },

    { 11U, IPC_PERF_TEST, IPC_MCU3_0, IPC_MCU1_0, RUN_CNT, 0U },
    { 12U, IPC_PERF_TEST, IPC_MCU3_0, IPC_MCU2_0, RUN_CNT, 0U },
    { 13U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C66X_1, RUN_CNT, 0U },
    { 14U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C7X_1,  RUN_CNT, 0U },

    { 21U, IPC_PERF_TEST, IPC_C66X_1, IPC_MCU1_0, RUN_CNT, 0U },
    { 22U, IPC_PERF_TEST, IPC_C66X_1, IPC_MCU2_0, RUN_CNT, 0U },
    { 23U, IPC_PERF_TEST, IPC_C66X_1, IPC_MCU3_0, RUN_CNT, 0U },
    { 24U, IPC_PERF_TEST, IPC_C66X_1, IPC_C7X_1,  RUN_CNT, 0U },

    { 31U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU1_0, RUN_CNT, 0U },
    { 32U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU2_0, RUN_CNT, 0U },
    { 33U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU3_0, RUN_CNT, 0U },
    { 34U, IPC_PERF_TEST, IPC_C7X_1,  IPC_C66X_1, RUN_CNT, 0U },
#elif defined (SOC_J721S2)
    {  1U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU1_0, RUN_CNT, 0U },
    {  2U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU3_0, RUN_CNT, 0U },
    {  3U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C7X_1,  RUN_CNT, 0U },
    {  4U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C7X_2,  RUN_CNT, 0U },

    { 11U, IPC_PERF_TEST, IPC_MCU3_0, IPC_MCU1_0, RUN_CNT, 0U },
    { 12U, IPC_PERF_TEST, IPC_MCU3_0, IPC_MCU2_0, RUN_CNT, 0U },
    { 13U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C7X_1,  RUN_CNT, 0U },
    { 14U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C7X_2,  RUN_CNT, 0U },

    { 21U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU1_0, RUN_CNT, 0U },
    { 22U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU2_0, RUN_CNT, 0U },
    { 23U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU3_0, RUN_CNT, 0U },
    { 24U, IPC_PERF_TEST, IPC_C7X_1,  IPC_C7X_2,  RUN_CNT, 0U },

    { 31U, IPC_PERF_TEST, IPC_C7X_2,  IPC_MCU1_0, RUN_CNT, 0U },
    { 32U, IPC_PERF_TEST, IPC_C7X_2,  IPC_MCU2_0, RUN_CNT, 0U },
    { 33U, IPC_PERF_TEST, IPC_C7X_2,  IPC_MCU3_0, RUN_CNT, 0U },
    { 34U, IPC_PERF_TEST, IPC_C7X_2,  IPC_C7X_1,  RUN_CNT, 0U },
#elif defined (SOC_J7200)
    {  1U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU1_0, RUN_CNT, 0U },
#elif defined (SOC_J784S4)
    {  1U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU1_0, RUN_CNT, 0U },
    {  2U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU3_0, RUN_CNT, 0U },
    {  3U, IPC_PERF_TEST, IPC_MCU2_0, IPC_MCU4_0, RUN_CNT, 0U },
    {  4U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C7X_1,  RUN_CNT, 0U },
    {  5U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C7X_2,  RUN_CNT, 0U },
    {  6U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C7X_3,  RUN_CNT, 0U },
    {  7U, IPC_PERF_TEST, IPC_MCU2_0, IPC_C7X_4,  RUN_CNT, 0U },

    { 11U, IPC_PERF_TEST, IPC_MCU3_0, IPC_MCU1_0, RUN_CNT, 0U },
    { 12U, IPC_PERF_TEST, IPC_MCU3_0, IPC_MCU2_0, RUN_CNT, 0U },
    { 13U, IPC_PERF_TEST, IPC_MCU3_0, IPC_MCU4_0, RUN_CNT, 0U },
    { 14U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C7X_1,  RUN_CNT, 0U },
    { 15U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C7X_2,  RUN_CNT, 0U },
    { 16U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C7X_3,  RUN_CNT, 0U },
    { 17U, IPC_PERF_TEST, IPC_MCU3_0, IPC_C7X_4,  RUN_CNT, 0U },

    { 21U, IPC_PERF_TEST, IPC_MCU4_0, IPC_MCU1_0, RUN_CNT, 0U },
    { 22U, IPC_PERF_TEST, IPC_MCU4_0, IPC_MCU2_0, RUN_CNT, 0U },
    { 23U, IPC_PERF_TEST, IPC_MCU4_0, IPC_MCU3_0, RUN_CNT, 0U },
    { 24U, IPC_PERF_TEST, IPC_MCU4_0, IPC_C7X_1,  RUN_CNT, 0U },
    { 25U, IPC_PERF_TEST, IPC_MCU4_0, IPC_C7X_2,  RUN_CNT, 0U },
    { 26U, IPC_PERF_TEST, IPC_MCU4_0, IPC_C7X_3,  RUN_CNT, 0U },
    { 27U, IPC_PERF_TEST, IPC_MCU4_0, IPC_C7X_4,  RUN_CNT, 0U },

    { 31U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU1_0, RUN_CNT, 0U },
    { 32U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU2_0, RUN_CNT, 0U },
    { 33U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU3_0, RUN_CNT, 0U },
    { 34U, IPC_PERF_TEST, IPC_C7X_1,  IPC_MCU4_0, RUN_CNT, 0U },
    { 35U, IPC_PERF_TEST, IPC_C7X_1,  IPC_C7X_2,  RUN_CNT, 0U },
    { 36U, IPC_PERF_TEST, IPC_C7X_1,  IPC_C7X_3,  RUN_CNT, 0U },
    { 37U, IPC_PERF_TEST, IPC_C7X_1,  IPC_C7X_4,  RUN_CNT, 0U },

    { 41U, IPC_PERF_TEST, IPC_C7X_2,  IPC_MCU1_0, RUN_CNT, 0U },
    { 42U, IPC_PERF_TEST, IPC_C7X_2,  IPC_MCU2_0, RUN_CNT, 0U },
    { 43U, IPC_PERF_TEST, IPC_C7X_2,  IPC_MCU3_0, RUN_CNT, 0U },
    { 44U, IPC_PERF_TEST, IPC_C7X_2,  IPC_MCU4_0, RUN_CNT, 0U },
    { 45U, IPC_PERF_TEST, IPC_C7X_2,  IPC_C7X_1,  RUN_CNT, 0U },
    { 46U, IPC_PERF_TEST, IPC_C7X_2,  IPC_C7X_3,  RUN_CNT, 0U },
    { 47U, IPC_PERF_TEST, IPC_C7X_2,  IPC_C7X_4,  RUN_CNT, 0U },

    { 51U, IPC_PERF_TEST, IPC_C7X_3,  IPC_MCU1_0, RUN_CNT, 0U },
    { 52U, IPC_PERF_TEST, IPC_C7X_3,  IPC_MCU2_0, RUN_CNT, 0U },
    { 53U, IPC_PERF_TEST, IPC_C7X_3,  IPC_MCU3_0, RUN_CNT, 0U },
    { 54U, IPC_PERF_TEST, IPC_C7X_3,  IPC_MCU4_0, RUN_CNT, 0U },
    { 55U, IPC_PERF_TEST, IPC_C7X_3,  IPC_C7X_1,  RUN_CNT, 0U },
    { 56U, IPC_PERF_TEST, IPC_C7X_3,  IPC_C7X_2,  RUN_CNT, 0U },
    { 57U, IPC_PERF_TEST, IPC_C7X_3,  IPC_C7X_4,  RUN_CNT, 0U },

    { 61U, IPC_PERF_TEST, IPC_C7X_4,  IPC_MCU1_0, RUN_CNT, 0U },
    { 62U, IPC_PERF_TEST, IPC_C7X_4,  IPC_MCU2_0, RUN_CNT, 0U },
    { 63U, IPC_PERF_TEST, IPC_C7X_4,  IPC_MCU3_0, RUN_CNT, 0U },
    { 64U, IPC_PERF_TEST, IPC_C7X_4,  IPC_MCU4_0, RUN_CNT, 0U },
    { 65U, IPC_PERF_TEST, IPC_C7X_4,  IPC_C7X_1,  RUN_CNT, 0U },
    { 66U, IPC_PERF_TEST, IPC_C7X_4,  IPC_C7X_2,  RUN_CNT, 0U },
    { 67U, IPC_PERF_TEST, IPC_C7X_4,  IPC_C7X_3,  RUN_CNT, 0U },
#endif
};

static uint32_t gTestCnt = sizeof(gTestCases)/sizeof(Ipc_Testcase);
static Ipc_TestPerfResult  gPerfTestResult[MAX_TEST_RESULT_CNT];
static uint32_t            gPerfTestResCnt  = 0U;

Ipc_Testcase* Ipc_getTestcase(uint32_t testIndex)
{
    Ipc_Testcase* tstcase = NULL;

    if(testIndex < gTestCnt)
    {
        tstcase = &gTestCases[testIndex];
    }

    return tstcase;
}

void Ipc_addPerfTestResult(uint32_t testId, uint32_t testTime)
{
    if(gPerfTestResCnt < MAX_TEST_RESULT_CNT)
    {
        gPerfTestResult[gPerfTestResCnt].testId   = testId;
        gPerfTestResult[gPerfTestResCnt].testTime = testTime;
        gPerfTestResCnt++;
    }
}

void Ipc_addPerfTestResults(uint8_t *buf, uint32_t bufSize)
{
    uint8_t             *p = buf;
    uint32_t             bufLeft = bufSize;
    Ipc_TestPerfResult  *data;

    while( (bufLeft >= sizeof(Ipc_TestPerfResult)) &&
           (gPerfTestResCnt < MAX_TEST_RESULT_CNT-1))
    {
        data = (Ipc_TestPerfResult*)p;
        gPerfTestResult[gPerfTestResCnt].testId   = data->testId;
        gPerfTestResult[gPerfTestResCnt].testTime = data->testTime;
        gPerfTestResCnt++;
        bufLeft -= sizeof(Ipc_TestPerfResult);
        p += sizeof(Ipc_TestPerfResult);
    }
}


uint32_t Ipc_copyRestResult(uint8_t *buf, uint32_t bufSize)
{
    uint32_t szReqrd = 0U;
    uint32_t szData  = 0U;

    if(NULL != buf)
    {
        szReqrd = sizeof(Ipc_TestPerfResult) * gPerfTestResCnt;
        if(szReqrd < bufSize)
        {
            memcpy(buf, (const void *)gPerfTestResult, szReqrd);
            szData = szReqrd;
        }
    }
    return szData;
}

uint32_t Ipc_hasPerfTestResult(void)
{
    uint32_t hasTestResult = TRUE;
    if(0U == gPerfTestResCnt)
    {
        hasTestResult = FALSE;
    }

    return hasTestResult;
}

void Ipc_getCoreIdForTestcase(uint32_t testId, uint32_t *hostCore, uint32_t *remCore)
{
    uint32_t  i;

    for( i = 0; i < gTestCnt; i++)
    {
        if(gTestCases[i].testId == testId)
        {
            *hostCore = gTestCases[i].hostCore;
            *remCore  = gTestCases[i].slaveCore;
            break;
        }
    }
}

void Ipc_printPerfTestReport()
{
    uint32_t       i;
    uint32_t       hostCore = 0;
    uint32_t       remCore  = 0;
    uint32_t       prevHost = 0;

    UART_printf("======================================================\n");
    UART_printf(" IPC/LLD Performance Test Report \n");
    UART_printf("------------------------------------------------------\n");

    for(i = 0; i < gPerfTestResCnt; i++)
    {
        prevHost = hostCore;
        Ipc_getCoreIdForTestcase(gPerfTestResult[i].testId, &hostCore, &remCore);

        if(prevHost != hostCore)
        {
             UART_printf("------------------------------------------------------\n");
        }

        UART_printf("Host: %s, Remote: %s, Round-trip time : %d us\n",
                Ipc_mpGetName(hostCore), Ipc_mpGetName(remCore),
                gPerfTestResult[i].testTime);
    }

    UART_printf("======================================================\n\n");

}
