/*
 * Copyright (c) 2026 Texas Instruments Incorporated
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
 *  \file main_baremetal.c
 *
 *  \brief SAUL6 - SA3UL static registers readback example code
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>

#include <ip_fma_common.h>
#include <ip_fma_sa3ul_soc.h>
#include <ip_fma_sa3ul.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Timer period for test, set to 3 s  */
#define TEST_TIMER_PERIOD_3S    ((3000U) * 10U)
/* Converts miliseconds to microseconds for TimerP configuration */
#define MSEC_TO_USEC(x)         ((uint32_t)(x) * 1000)

/* Number of times to run the sa3ul periodic readback check. */
#define SA3UL_PERIODIC_CHECK_NUMBER_OF_RUNS         (5U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Number of times the periodic readback check was successful. */
uint32_t gSa3ulRegsPeriodicCheckSuccessfulCount = 0U;

/* Enable periodic readback check flag. */
bool gRunSa3ulRegsPeriodicCheck = false;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t Saul6App_Init(void);
static void Saul6App_Sa3ulRegsPeriodicReadback(const uintptr_t* goldenRefRegData);
static void Saul6App_Sa3ulRegsPeriodicReadbackActivate(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 * \brief Enables sa3ul periodic readback check
 */
static void Saul6App_Sa3ulRegsPeriodicReadbackActivate(void)
{
    gRunSa3ulRegsPeriodicCheck = true;
}

/**
 * \brief Reads sa3ul registers from memory and compares to a saved "golden reference" copy
 *
 * \param goldenRefRegData      Register configuration copy to compare against
 */
void Saul6App_Sa3ulRegsPeriodicReadback(const uintptr_t* goldenRefRegData)
{
    IpFma_Status status = IPFMA_OK;

    /* Reset the enable periodic readback check flag. */
    gRunSa3ulRegsPeriodicCheck = false;

    if (IPFMA_OK == status)
    {
        status = IpFma_Sa3ul_VerifyStaticRegsCfg(goldenRefRegData);

        if (IPFMA_OK == status)
    	{
    	    UART_printf("SUCCESS: SA3UL registers MATCH with golden reference.\r\n\n");
            gSa3ulRegsPeriodicCheckSuccessfulCount++;
    	}
    	else if (IPFMA_E_MISMATCH == status)
    	{
    	    UART_printf("ERROR: SA3UL registers MISMATCH against golden reference!!\r\n\n");
        }
    }
    else
    {
        UART_printf("ERROR: Failed to read SA3UL registers - invalid parameters!\r\n");
    }
}

static TimerP_Handle Saul6App_StartPeriodicTimer(const uint32_t periodMs, TimerP_Fxn callback, void *arg)
{
    TimerP_Params timerParams;
    TimerP_Handle timerHandle;

    if (NULL == callback)
    {
        UART_printf("TIMER: Saul6App_StartPeriodicTimer: NULL callback provided.\n");
        return NULL;
    }

    (void)TimerP_Params_init(&timerParams);
    timerParams.runMode = TimerP_RunMode_CONTINUOUS;
    timerParams.startMode = TimerP_StartMode_AUTO;
    timerParams.periodType = TimerP_PeriodType_MICROSECS;
    timerParams.period = ((MSEC_TO_USEC(periodMs)));
    timerParams.arg = (void *)arg;

    timerHandle = TimerP_create(TimerP_ANY, callback, &timerParams);
    if (NULL == timerHandle)
    {
        UART_printf("TIMER: Failed to create timer (period = %u μs).\n", timerParams.period);
        return NULL;
    }

    UART_printf("TIMER: Timer started successfully (period = %u μs).\n", timerParams.period);
    return timerHandle;
}


/**
 *  \brief Main function.
 *
 *  This is an example of how to read and compare SA3UL static configuration registers.
 *
 *  \return  Status of the program, 0 - OK, -10 - Error.
 */
int main(void)
{
    int32_t status = BOARD_FAIL;
    status = Saul6App_Init();
    if (status != BOARD_SOK) return status;

    uintptr_t goldenRefRegData[IP_FMA_SA3UL_STATIC_REGDUMP_BUFFER_SIZE];
    status = IpFma_Sa3ul_GetStaticRegsCfg(goldenRefRegData);

    if (IPFMA_OK == status)
    {
        /* Start a periodic timer with callback function that does register check. */
        TimerP_Handle timer = Saul6App_StartPeriodicTimer(TEST_TIMER_PERIOD_3S,
                                                        (TimerP_Fxn)&Saul6App_Sa3ulRegsPeriodicReadbackActivate,
                                                        NULL);

        if (NULL == timer)
        {
            UART_printf("\nTIMER: Unable to start periodic timer.\n");
        }
        else
        {
            uint32_t sa3ulRegsPeriodicCheckRanCount = 0U;
            while (SA3UL_PERIODIC_CHECK_NUMBER_OF_RUNS > sa3ulRegsPeriodicCheckRanCount)
            {
                if (true == gRunSa3ulRegsPeriodicCheck)
                {
                    Saul6App_Sa3ulRegsPeriodicReadback(goldenRefRegData);
                    sa3ulRegsPeriodicCheckRanCount++;
                }
            }

            if (SA3UL_PERIODIC_CHECK_NUMBER_OF_RUNS == gSa3ulRegsPeriodicCheckSuccessfulCount)
            {
                UART_printf("\nAll tests have passed.");
            }
            else
            {
                UART_printf("\n SA3UL periodic readback check pass rate is: %u/%u", gSa3ulRegsPeriodicCheckSuccessfulCount,
                                                                        SA3UL_PERIODIC_CHECK_NUMBER_OF_RUNS);
                UART_printf("\nSome tests have failed.");
            }
        }
    }

    return status;
}

/**
 *  \brief Initializes the application by setting the configuration.
 *
 *  This is a initialization function that sets the board configuration.
 *  It configures the board by enabling UART module needed to print log
 *  messages to the user via UART console. This is used so the tests can
 *  print results back to us.
 *
 *  \return  BOARD_SOK in case of success or appropriate error code.
 */
static int32_t Saul6App_Init(void)
{
    UART_printf("\n ... Init \n\n");

    int32_t status = BOARD_FAIL;
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    return status;
}
