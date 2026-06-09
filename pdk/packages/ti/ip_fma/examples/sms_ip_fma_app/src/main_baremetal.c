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
 *  \brief SMS CPU 7 TIFS firmware version check and (periodic)readback of SMS registers
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

#include <ti/board/src/devices/common/common.h>
#include <ti/drv/sciclient/sciclient.h>

#include <ip_fma_r5f.h>
#include <ip_fma_sms.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* SMS DMTIMER ID */
#define SMS_DMTIMER_ID_0        (0x00U)
#define SMS_DMTIMER_ID_1        (0x01U)
#define SMS_DMTIMER_ID_2        (0x02U)
#define SMS_DMTIMER_ID_3        (0x03U)

/* Timer period for test, set to 3 s  */
#define TEST_TIMER_PERIOD_3S    ((3000U) * 10U)
/* Converts miliseconds to microseconds for TimerP configuration */
#define MSEC_TO_USEC(x)         ((uint32_t)(x) * 1000)

/* Number of times to run the sms periodic readback check. */
#define SMS_PERIODIC_READBACK_CHECK_NUMBER_OF_RUNS     (5U)
/* Number of times to run the sms tifs check. */
#define SMS_TIFS_CHECK_NUMBER_OF_RUNS                  (3U)

/* Number of different sms register groups to check. */
#define SMS_READBACK_REG_GROUPS                        (7U)

#define DELAY_MS                                        ((uint32_t)1000U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Number of times the periodic readback check was successful. */
uint32_t gSmsPeriodicReadbackSuccessfulCount = 0U;

/* Periodic readback check enable flag. */
bool gRunPeriodicReadbackCheck = false;

/* Status of the tests. */
bool gSmsReadbackSuccessful = false;
bool gSmsTifsSuccessful = false;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t SmsApp_Init();
static void SmsApp_RegCompareStatus(IpFma_Status status, char* registerName, uint32_t* readbackSuccessfullCount);

void SmsApp_tifsFirmwareVersionPeriodicCheck(void);
void SmsApp_smsPeriodicReadback(IpFma_SmsDmtimerRegs* dmtimer0,
                                 IpFma_SmsDmtimerRegs* dmtimer1,
                                 IpFma_SmsDmtimerRegs* dmtimer2,
                                 IpFma_SmsDmtimerRegs* dmtimer3,
                                 IpFma_SmsRtiRegs* rti,
                                 IpFma_SmsRtiMmrRegs* rtiMmr,
                                 IpFma_SmsPwrMmrRegs* pwrMmr);
void SmsApp_smsReadback(void);

static TimerP_Handle SmsApp_StartPeriodicTimer(const uint32_t periodMs, TimerP_Fxn callback, void *arg);
static void SmsApp_RegsPeriodicReadbackActivate(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

void SmsApp_smsReadback(void)
{
    IpFma_Status status = IPFMA_OK;
    uint32_t readbackSuccessfulCount = 0U;

    IpFma_SmsDmtimerRegs dmtimer0, dmtimer1, dmtimer2, dmtimer3;
    IpFma_SmsRtiRegs rti;
    IpFma_SmsRtiMmrRegs rtiMmr;
    IpFma_SmsPwrMmrRegs pwrMmr;

    UART_printf("\n\n Get TIFS DMTIMER0 configuration.");
    status = IpFma_Sms_GetDmtimerRegs(&dmtimer0, SMS_DMTIMER_ID_0);
    if (IPFMA_OK == status)
    {
        status = IpFma_Sms_VerifyDmtimerRegs(&dmtimer0, SMS_DMTIMER_ID_0);
        SmsApp_RegCompareStatus(status, "DMTIMER0", &readbackSuccessfulCount);
    }

    UART_printf("\n\n Get TIFS DMTIMER1 configuration.");
    status = IpFma_Sms_GetDmtimerRegs(&dmtimer1, SMS_DMTIMER_ID_1);
    if (IPFMA_OK == status)
    {
        status = IpFma_Sms_VerifyDmtimerRegs(&dmtimer1, SMS_DMTIMER_ID_1);
        SmsApp_RegCompareStatus(status, "DMTIMER1", &readbackSuccessfulCount);
    }

    UART_printf("\n\n Get TIFS DMTIMER2 configuration.");
    status = IpFma_Sms_GetDmtimerRegs(&dmtimer2, SMS_DMTIMER_ID_2);
    if (IPFMA_OK == status)
    {
        status = IpFma_Sms_VerifyDmtimerRegs(&dmtimer2, SMS_DMTIMER_ID_2);
        SmsApp_RegCompareStatus(status, "DMTIMER2", &readbackSuccessfulCount);
    }

    UART_printf("\n\n Get TIFS DMTIMER3 configuration.");
    status = IpFma_Sms_GetDmtimerRegs(&dmtimer3, SMS_DMTIMER_ID_3);
    if (IPFMA_OK == status)
    {
        status = IpFma_Sms_VerifyDmtimerRegs(&dmtimer3, SMS_DMTIMER_ID_3);
        SmsApp_RegCompareStatus(status, "DMTIMER3", &readbackSuccessfulCount);
    }

    UART_printf("\n\n Get TIFS WDT RTI configuration.");
    status = IpFma_Sms_GetRtiRegs(&rti);
    if (IPFMA_OK == status)
    {
        status = IpFma_Sms_VerifyRtiRegs(&rti);
        SmsApp_RegCompareStatus(status, "TIFS WDT RTI", &readbackSuccessfulCount);
    }

    UART_printf("\n\n Get TIFS RTI MMR configuration.");
    status = IpFma_Sms_GetRtiMmrRegs(&rtiMmr);
    if (IPFMA_OK == status)
    {
        status = IpFma_Sms_VerifyRtiMmrRegs(&rtiMmr);
        SmsApp_RegCompareStatus(status, "TIFS RTI MMR", &readbackSuccessfulCount);
    }

    UART_printf("\n\n Get TIFS PWR MMR configuration.");
    status = IpFma_Sms_GetPwrMmrRegs(&pwrMmr);
    if (IPFMA_OK == status)
    {
        status = IpFma_Sms_VerifyPwrMmrRegs(&pwrMmr);
        SmsApp_RegCompareStatus(status, "TIFS PWM MMR", &readbackSuccessfulCount);
    }

    if (SMS_READBACK_REG_GROUPS == readbackSuccessfulCount)
    {
        UART_printf("\n Register readback is successful.");
        gSmsReadbackSuccessful = true;
    }
    else
    {
        UART_printf("\n ERROR: Register readback is not successful.");
    }
}

void SmsApp_smsPeriodicReadback(IpFma_SmsDmtimerRegs* dmtimer0,
                                 IpFma_SmsDmtimerRegs* dmtimer1,
                                 IpFma_SmsDmtimerRegs* dmtimer2,
                                 IpFma_SmsDmtimerRegs* dmtimer3,
                                 IpFma_SmsRtiRegs* rti,
                                 IpFma_SmsRtiMmrRegs* rtiMmr,
                                 IpFma_SmsPwrMmrRegs* pwrMmr)
{
    IpFma_Status status = IPFMA_OK;
    uint32_t readbackSuccessfulCount = 0U;

    /* Reset the enable periodic readback check flag. */
    gRunPeriodicReadbackCheck = false;

    status = IpFma_Sms_VerifyDmtimerRegs(dmtimer0, SMS_DMTIMER_ID_0);
    SmsApp_RegCompareStatus(status, "DMTIMER0", &readbackSuccessfulCount);

    status = IpFma_Sms_VerifyDmtimerRegs(dmtimer1, SMS_DMTIMER_ID_1);
    SmsApp_RegCompareStatus(status, "DMTIMER1", &readbackSuccessfulCount);

    status = IpFma_Sms_VerifyDmtimerRegs(dmtimer2, SMS_DMTIMER_ID_2);
    SmsApp_RegCompareStatus(status, "DMTIMER2", &readbackSuccessfulCount);

    status = IpFma_Sms_VerifyDmtimerRegs(dmtimer3, SMS_DMTIMER_ID_3);
    SmsApp_RegCompareStatus(status, "DMTIMER3", &readbackSuccessfulCount);

    status = IpFma_Sms_VerifyRtiRegs(rti);
    SmsApp_RegCompareStatus(status, "TIFS WDT RTI", &readbackSuccessfulCount);

    status = IpFma_Sms_VerifyRtiMmrRegs(rtiMmr);
    SmsApp_RegCompareStatus(status, "TIFS RTI MMR", &readbackSuccessfulCount);

    status = IpFma_Sms_VerifyPwrMmrRegs(pwrMmr);
    SmsApp_RegCompareStatus(status, "TIFS PWR MMR", &readbackSuccessfulCount);

    if (SMS_READBACK_REG_GROUPS == readbackSuccessfulCount)
    {
        UART_printf("\n Periodic readback successful.");
        gSmsPeriodicReadbackSuccessfulCount++;
    }
}

void SmsApp_tifsFirmwareVersionPeriodicCheck(void)
{
    IpFma_Status status = IPFMA_OK;
    int32_t status_tifs_check = IPFMA_OK;
    Sciclient_ConfigPrms_t config =
    {
        SCICLIENT_SERVICE_OPERATION_MODE_POLLED,
        NULL,
        0 /* isSecure = 0 un secured for all cores */
    };

    struct tisci_msg_version_req request;
    const Sciclient_ReqPrm_t reqPrm =
    {
        TISCI_MSG_VERSION,
        TISCI_MSG_FLAG_AOP,
        (uint8_t *) &request,
        sizeof(request),
        SCICLIENT_SERVICE_WAIT_FOREVER
    };

    struct tisci_msg_version_resp response;
    Sciclient_RespPrm_t respPrm =
    {
        0,
        (uint8_t *) &response,
        sizeof (response)
    };

    UART_printf("\n\n\n--------- Test TIFS firmware version ---------\n\n");

    status = Sciclient_init(&config);

    if (IPFMA_OK == status)
    {
        uint32_t tifsCheckRunCount = 0;
        while (tifsCheckRunCount < SMS_TIFS_CHECK_NUMBER_OF_RUNS)
        {
            status = Sciclient_service(&reqPrm, &respPrm);

            if (IPFMA_OK == status)
            {
                if (TISCI_MSG_FLAG_ACK == respPrm.flags)
                {
                    UART_printf("DMSC Firmware Version %s\n", (char *) response.str);
                    UART_printf("Firmware revision 0x%x\n", response.version);
                }
                else
                {
                    status_tifs_check = IPFMA_E_IO;
                    UART_printf("DMSC Firmware Get Version failed!\n");
                }
            }
            else
            {
                status_tifs_check = IPFMA_E_IO;
                UART_printf("DMSC Firmware Get Version failed!\n");
            }

            tifsCheckRunCount++;
        }

        status = Sciclient_deinit();
    }

    if (IPFMA_OK == status_tifs_check)
    {
        UART_printf("\nTIFS firmware version check PASSED!\n");
        gSmsTifsSuccessful = true;
    }
    else
    {
        UART_printf("\nTIFS firmware version check FAILED!\n");
    }
}

/**
 *  \brief Main function, runs all checks.
 *
 *  \return  Status of the program, 0 - OK, -10 - Error.
 */
int main(void)
{
    int8_t status = BOARD_FAIL;

    status = SmsApp_Init();
    if (status != BOARD_SOK)
    {
        return status;
    }

    UART_printf("\n\n\n ------------READBACK TESTS------------");
    SmsApp_smsReadback();

    UART_printf("\n\n\n ------------PERIODIC READBACK TESTS------------");
    bool periodicReadbackChecksSuccesful = false;
    /* Execute the sms periodic readback checks. */
    if (IPFMA_OK == status)
    {
        /* Start a periodic timer with callback function that does register check. */
        TimerP_Handle timer = SmsApp_StartPeriodicTimer(TEST_TIMER_PERIOD_3S,
                                                         (TimerP_Fxn)&SmsApp_RegsPeriodicReadbackActivate,
                                                         NULL);

        if (NULL == timer)
        {
            UART_printf("\nTIMER: Unable to start periodic timer.\n");
        }
        else
        {
            uint32_t periodicReadbackCheckRanCount = 0U;
            while (SMS_PERIODIC_READBACK_CHECK_NUMBER_OF_RUNS > periodicReadbackCheckRanCount)
            {
                if (true == gRunPeriodicReadbackCheck)
                {
                    IpFma_SmsDmtimerRegs dmtimer0;
                    IpFma_SmsDmtimerRegs dmtimer1;
                    IpFma_SmsDmtimerRegs dmtimer2;
                    IpFma_SmsDmtimerRegs dmtimer3;
                    IpFma_SmsRtiRegs rti;
                    IpFma_SmsRtiMmrRegs rtiMmr;
                    IpFma_SmsPwrMmrRegs pwrMmr;

                    if (IPFMA_OK != IpFma_Sms_GetDmtimerRegs(&dmtimer0, SMS_DMTIMER_ID_0))
                    {
                        status = IPFMA_E_IO;
                    }

                    if (IPFMA_OK != IpFma_Sms_GetDmtimerRegs(&dmtimer1, SMS_DMTIMER_ID_1))
                    {
                        status = IPFMA_E_IO;
                    }

                    if (IPFMA_OK != IpFma_Sms_GetDmtimerRegs(&dmtimer2, SMS_DMTIMER_ID_2))
                    {
                        status = IPFMA_E_IO;
                    }

                    if (IPFMA_OK != IpFma_Sms_GetDmtimerRegs(&dmtimer3, SMS_DMTIMER_ID_3))
                    {
                        status = IPFMA_E_IO;
                    }

                    if (IPFMA_OK != IpFma_Sms_GetRtiRegs(&rti))
                    {
                        status = IPFMA_E_IO;
                    }

                    if (IPFMA_OK != IpFma_Sms_GetRtiMmrRegs(&rtiMmr))
                    {
                        status = IPFMA_E_IO;
                    }

                    if (IPFMA_OK != IpFma_Sms_GetPwrMmrRegs(&pwrMmr))
                    {
                        status = IPFMA_E_IO;
                    }

                    SmsApp_smsPeriodicReadback(&dmtimer0,
                                                &dmtimer1,
                                                &dmtimer2,
                                                &dmtimer3,
                                                &rti,
                                                &rtiMmr,
                                                &pwrMmr);

                    periodicReadbackCheckRanCount++;
                }

                Board_delay(DELAY_MS);
            }

            if (SMS_PERIODIC_READBACK_CHECK_NUMBER_OF_RUNS == gSmsPeriodicReadbackSuccessfulCount)
            {
                UART_printf("\n Periodic readback tests have passed.");
                periodicReadbackChecksSuccesful = true;
            }
            else
            {
                UART_printf("\n SMS periodic readback check pass rate is: %u/%u", gSmsPeriodicReadbackSuccessfulCount,
                                                                                   SMS_PERIODIC_READBACK_CHECK_NUMBER_OF_RUNS);
                UART_printf("\n Some periodic readback tests have failed.");
            }
        }
    }
    UART_printf("\n\n\n ------------TIFS FIRMWARE VERSION PERIODIC CHECK------------");
    SmsApp_tifsFirmwareVersionPeriodicCheck();

    if ((true == periodicReadbackChecksSuccesful) && \
        (true == gSmsReadbackSuccessful) && \
        (true == gSmsTifsSuccessful))
    {
        UART_printf("\n All tests have passed.");
    }
    else
    {
        UART_printf("\n Some tests have failed.");
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
static int32_t SmsApp_Init()
{
    UART_printf("\n ... Init \n\n");

    int8_t status = -1;
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    return status;
}

/**
 * \brief Enables sms periodic readback check
 */
static void SmsApp_RegsPeriodicReadbackActivate(void)
{
    gRunPeriodicReadbackCheck = true;
}

/**
 * \brief Starts a periodic timer which enables a periodic readback to be executed
 */
static TimerP_Handle SmsApp_StartPeriodicTimer(const uint32_t periodMs, TimerP_Fxn callback, void *arg)
{
    TimerP_Params timerParams;
    TimerP_Handle timerHandle;

    if (NULL == callback)
    {
        UART_printf("\nTIMER: SmsApp_StartPeriodicTimer: NULL callback provided.\n");
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
        UART_printf("\nTIMER: Failed to create timer (period = %u μs).\n", timerParams.period);
        return NULL;
    }

    UART_printf("\nTIMER: Timer started successfully (period = %u μs).\n", timerParams.period);
    return timerHandle;
}

/**
 *  \brief Simple utility function that prints whether the registers were equal or not based
 *         on the comparison results.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param registerGroupName    [IN] Name of the register group.
 *  \param regCompareStatus     [IN] Comparison status of the registers.
 *
 *  \return None.
 */
static void SmsApp_RegCompareStatus(IpFma_Status regCompareStatus, char* registerGroupName, uint32_t* readbackSuccessfulCount)
{
    if (IPFMA_OK == regCompareStatus)
    {
        UART_printf("\nSuccess! The registers *%s* have not been modified(they match the golden reference).", registerGroupName);
        (*readbackSuccessfulCount)++;
    }
    else if (IPFMA_E_MISMATCH == regCompareStatus)
    {
        UART_printf("\nError! The registers *%s* have been modified(they do not match the golden reference).", registerGroupName);
    }
}
