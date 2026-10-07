/*
 *   Copyright (c) Texas Instruments Incorporated 2026
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
 *
 */

 /**
 *  \file     main_baremetal.c
 *
 *  \brief    This example application demonstrates comparison of 2 clock
 *            sources.
 *
 *  \details  Different clock sources are provided to Counter 1 (Test Clock)
 *            and Counter 0 (Reference Clock). The application configures the
 *            DCC module to operate in single-shot mode and generate an
 *            interrupt when Counter 1 reaches 0. When Counter 0 along with
 *            the valid counter and Counter 1 reach 0 at the same time, a
 *            completion interrupt is generated, indicating that no clock
 *            drift was observed. In addition, the application performs
 *            periodic software readback of static DCC configuration
 *            registers and reports match or mismatch between expected
 *            register values and actual (read) values for validation and
 *            diagnostic purposes.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <drivers/hw_include/csl_types.h>
#include <sdl/dcc/v0/sdl_dcc.h>
#include <drivers/hw_include/j722s/cslr_intr_mcu_r5fss0_core0.h>

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/SystemP.h>
#include <kernel/dpl/ClockP.h>

#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include <ip_fma_clk.h>
#include <dpl_interface.h>

/* ========================================================================== */
/*                                Macros                                      */
/* ========================================================================== */

/**< Example Common display string */
#define APP_DCC_STR                     "DCC Config Registers Periodic Check"
/**< Instance of DCC. While changing the instance, ensure update clock sources*/
#define APP_DCC_MODULE_INST             (SDL_DCC_INST_MCU_DCC0)
/**< One Shot mode, Stop counting when Counter 1, reaches 0. */
#define APP_DCC_MODE                    (SDL_DCC_MODE_SINGLE_SHOT_2)
/**< Maximum value that can be held in the COUNT0 register (ref clock) */
#define APP_DCC_SRC0_MAX_VAL            (0xFFFFFU)
/**< Maximum value that can be held in the VALID0 register (ref clock) */
#define APP_DCC_SRC0_VALID_MAX_VAL      (0x0FFFFU)
/**< Maximum value that can be held in the COUNT1 register (test clock) */
#define APP_DCC_SRC1_MAX_VAL            (0xFFFFFU)
/* Defines that control the clock inputs to DCC and allowed variance */
#define APP_DCC_REF_CLOCK_SRC_0         (SDL_DCC_CLK0_SRC_CLOCK0_0)
#define APP_DCC_TEST_CLOCK_SRC_1        (SDL_DCC_CLK1_SRC_CLOCK1)
/**< Allowed drift in percentage (+/-) */
#define APP_DCC_TEST_CLOCK_SRC_1_DRIFT  (5U)
/**< 25 MHz for HFOSC0 */
#define APP_DCC_REF_CLOCK_FREQ_IN_KHZ   (25000U)
/**< Expected test clock frequency in KHz */
#define APP_DCC_TEST_CLOCK_FREQ_IN_KHZ  (200000U)
/**< Number of periodic readbacks */
#define PERIODIC_CHECK_NUM                        ((uint8_t)10U)
/**< Delay time in ms */
#define DELAY_MS                                  ((uint8_t)1000U)
/**< Fixed reference frequency (in KHz) used internally by the DCC
 *   seed/tolerance-window error-margin formula (asyncErr calculation).
 *   Matches TI's own dcc_uc1.c reference value. */
#define APP_DCC_SYSCLK_FREQ                                            (200000U)
/**< Fixed digitization error margin (in DCC clock cycles) inherent to
 *   the DCC comparator's internal measurement resolution. Added to
 *   asyncErr to form dccErr, the base error budget before drift
 *   tolerance is applied. */
#define APP_DCC_DIGITIZATION_ERR                                          (8.0f)
/**< Main oscillator setup/settle time, used only as a divisor when
 *   the caller-requested drift percentage exceeds 100% and a minimum
 *   safe drift must be derived instead. */
#define APP_DCC_MOSC_SETUP_TIME                                       1048575.0f
/**< Lower bound (in percent) clamped onto the requested drift
 *   tolerance - prevents an unrealistically tight window that the
 *   the DCC error budget (dccErr) could never actually satisfy. */
#define APP_DCC_MIN_DRIFT                                                   0.2f
/**< Upper bound (in percent) clamped onto the requested drift
 *   tolerance - prevents an excessively loose window that would make
 *   the comparison nearly meaningless. */
#define APP_DCC_MAX_DRIFT                                                  48.0f

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/**< Flag used to indicate occurrence of the completion interrupt */
volatile uint32_t isrFlag = 0U;

/**< HWI object for DCC done interrupt */
static HwiP_Object gDccHwiObject;

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/**
 * \brief   Prints a string to the debug log
 *
 * \param   str     String to print
 *
 * \retval  None.
 */
static void Clk_AppPrint(char * str);

/**
 * \brief   This function returns clock frequencies
 *
 * \param   dccInst     Instance of DCC
 * \param   clkSrc0     Clock source for counter 0
 * \param   clkSrc1     Clock source for counter 1
 * \param   clk0Freq    Clock frequency for counter 0 in KHz
 * \param   clk1Freq    Clock frequency for counter 1 in KHz
 *
 * \retval  status      Negative number in case of errors
 */
static int32_t ClkApp_GetClkfreqKHz(SDL_DCC_Inst dccInst,
                                    uint32_t clkSrc0, uint32_t clkSrc1,
                                    uint32_t *clk0Freq, uint32_t *clk1Freq);

/**
 * \brief   This function returns least integral ratio for given clocks.
 *
 * \param   refClkFreq      Reference clock frequency in KHz.
 * \param   testClkFreq     Test clock frequency in KHz.
 * \param   refClkRatioNum  Reference clock ratio number.
 * \param   testClkRatioNum Test clock ratio number.
 *
 * \retval  None.
 */
static void ClkApp_GetClkRatio(uint32_t  refClkFreq,
                              uint32_t  testClkFreq,
                              uint32_t *refClkRatioNum,
                              uint32_t *testClkRatioNum);

/**
 * \brief   This function returns seed value for COUNT1.
 *
 * \param   refClkFreq      Reference clock frequency in KHz.
 * \param   testClkFreq     Test clock frequency in KHz.
 * \param   refClkRatioNum  Reference clock ratio number.
 * \param   testClkRatioNum Test clock ratio number.
 * \param   drfitPer        Allowed drift in test clock in percentage.
 * \param   configParams    DCC configuration parameters.
 *                          Refer struct #SDL_DCC_Config.
 *
 * \retval  None.
 */

static void ClkApp_SetSeedVals(uint32_t        refClkFreq,
                              uint32_t        testClkFreq,
                              uint32_t        refClkRatioNum,
                              uint32_t        testClkRatioNum,
                              uint32_t        drfitPer,
                              SDL_DCC_Config *configParams);

/**
 * \brief   This function register ISR for a given instance of DCC
 *
 * \param   dccInst     Instance of DCC
 *
 * \retval  CSL_PASS on successful interrupt handler registration.
 */
static int32_t ClkApp_RegisterIsr(SDL_DCC_Inst dccInst);

/**
 * \brief   ISR for done interrupt, set a global flag
 *
 * \retval  None
 */
static void ClkApp_DoneIntrISR(void *args);

/**
 * \brief   This function introduces a delay in milliseconds.
 *
 * \param   wait_in_ms   Number of milliseconds to wait.
 *
 * \retval  None.
 */

static void ClkApp_Delay(uint32_t wait_in_ms);

/**
 * \brief   Executes a single‑shot DCC (Dual Clock Comparator) test sequence and
 *          performs periodic software readback of DCC registers.
 *
 * \retval  None.
 */
static void ClkApp_TestSdlDccSingleshotmodeApp(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void ClkApp_TestSdlDccSingleshotmodeApp(void)
{
    int32_t status;
    uint32_t clk0Freq, clk1Freq, refClkRatioNum, testClkRatioNum;
    SDL_DCC_Config configParams;

    Clk_AppPrint("\r\n" APP_DCC_STR ": Start\r\n");

    /* Steps
    1. Determine the clock frequencies for the sources
    2. Figure out the seed values for successful completion
    3. Configure DCC instance
    4. Register ISR and configure interrupts for normal completion
    5. Enable DCC
    6. Wait for normal completion or error interrupt and
       perform periodic software readback of DCC registers
    */

    /* Ensure a clean state before configuring - clear any stale error/done
       latch left over from a previous run */
    SDL_DCC_disable(APP_DCC_MODULE_INST);
    SDL_DCC_clearIntr(APP_DCC_MODULE_INST, SDL_DCC_INTERRUPT_ERR);
    SDL_DCC_clearIntr(APP_DCC_MODULE_INST, SDL_DCC_INTERRUPT_DONE);

    /* Step 1 Determine the clock frequencies for the sources */
    status = ClkApp_GetClkfreqKHz(APP_DCC_MODULE_INST,
                                   APP_DCC_REF_CLOCK_SRC_0,
                                   APP_DCC_TEST_CLOCK_SRC_1,
                                   &clk0Freq, &clk1Freq);

    if (CSL_PASS == status)
    {
        /* Step 2 Figure out the seed values for successful completion */
        ClkApp_GetClkRatio(clk0Freq,
                            clk1Freq,
                            &refClkRatioNum,
                            &testClkRatioNum);

        configParams.mode    = APP_DCC_MODE;
        configParams.clk0Src = APP_DCC_REF_CLOCK_SRC_0;
        configParams.clk1Src = APP_DCC_TEST_CLOCK_SRC_1;

        /* Get the seed values for given clock selections and allowed drift */
        ClkApp_SetSeedVals(clk0Freq,
                            clk1Freq,
                            refClkRatioNum,
                            testClkRatioNum,
                            APP_DCC_TEST_CLOCK_SRC_1_DRIFT,
                            &configParams);

        /* Step 3 Configure DCC instance */
        status = SDL_DCC_configure(APP_DCC_MODULE_INST, &configParams);

        if (SDL_PASS == status)
        {
            status = SDL_DCC_verifyConfig(APP_DCC_MODULE_INST, &configParams);
        }
        else
        {
            status = SDL_EFAIL;
        }

        if (status == SDL_PASS)
        {
            /* Step 4 Register ISR and configure interrupts for normal completion */
            status = ClkApp_RegisterIsr(APP_DCC_MODULE_INST);

            if (CSL_PASS == status)
            {
                /* Clear the interrupt flag, completion interrupt will pend
                   on this */
                isrFlag = 0U;

                Clk_AppPrint(APP_DCC_STR ": DCC configured \r\n");
                SDL_DCC_enableIntr(APP_DCC_MODULE_INST, SDL_DCC_INTERRUPT_ERR);
                SDL_DCC_enableIntr(APP_DCC_MODULE_INST, SDL_DCC_INTERRUPT_DONE);

                Clk_AppPrint(APP_DCC_STR ": Enabling DCC and waiting for "
                           "completion interrupt \r\n");

                Clk_AppPrint(APP_DCC_STR ": Register check starts...\r\n");
                /* Get expected values from DCC registers for MCU_DCC0 instance */
                IpFma_DccRegs dccRegsExpValues;
                IpFma_Status statusRb = IPFMA_OK;
                statusRb = IpFma_Clk_GetDccRegs(APP_DCC_MODULE_INST, &dccRegsExpValues);
                
                if (IPFMA_OK == statusRb)
                {
                    Clk_AppPrint(APP_DCC_STR ": Loading expected DCC register values...\r\n");

                    /* Step 5 Enable DCC */
                    SDL_DCC_enable(APP_DCC_MODULE_INST);

                    SDL_DCC_Status dccStatus;
                    memset(&dccStatus, 0, sizeof(dccStatus));

                    /* Step 6 Wait for completion interrupt or error flag, performing
                    periodic register readback/compare on every pass while waiting */
                    while ((0U == isrFlag) && (!dccStatus.errIntr) && (!dccStatus.doneIntr) )
                    {
                        SDL_DCC_getStatus(APP_DCC_MODULE_INST, &dccStatus);

                        int8_t i = PERIODIC_CHECK_NUM;
                        while (i > 0)
                        {
                            /* Read the actual values from registers */
                            IpFma_DccRegs dccRegsActualValues;
                            statusRb = IpFma_Clk_GetDccRegs(APP_DCC_MODULE_INST, &dccRegsActualValues);

                            if (IPFMA_OK == statusRb)
                            {
                                /* Compare expected and actual values periodically */
                                Clk_AppPrint(APP_DCC_STR ": Comparing expected-actual DCC register values..." "\r\n");
                                statusRb = IpFma_Clk_CompareDccRegs(&dccRegsExpValues, &dccRegsActualValues);

                                if (IPFMA_OK == statusRb)
                                {
                                    Clk_AppPrint(APP_DCC_STR ": values MATCH!\r\n");
                                }
                                else
                                {
                                    Clk_AppPrint(APP_DCC_STR ": values MISMATCH!\r\n");
                                }
                                i--;
                            }
                            ClkApp_Delay(DELAY_MS);
                        }
                    }

                    /* Refresh status once after the loop exits - dccStatus may be stale
                     * if isrFlag fired while the inner delay loop was running */
                    SDL_DCC_getStatus(APP_DCC_MODULE_INST, &dccStatus);

                    /* Ensure no error - reported once, after the wait loop exits */
                    if (dccStatus.errIntr)
                    {
                        Clk_AppPrint(APP_DCC_STR ": Error : DCC Generated error interrupt \r\n");
                        Clk_AppPrint(APP_DCC_STR ": Error interrupt is not expected \r\n");
                    }
                    else if (statusRb == IPFMA_OK)
                    {
                        Clk_AppPrint(APP_DCC_STR ": DCC Generated completion interrupt \r\n");
                        Clk_AppPrint(APP_DCC_STR ": No Clock Drift was observed \r\n");
                        Clk_AppPrint(APP_DCC_STR ": All tests have passed. \r\n");
                    }
                    else if (statusRb == IPFMA_E_PARAM)
                    {
                        Clk_AppPrint(APP_DCC_STR ": Error : Invalid parameter in DCC register read \r\n");
                    }
                    else if (statusRb == IPFMA_E_IO)
                    {
                        Clk_AppPrint(APP_DCC_STR ": Error : Hardware read error in DCC register read \r\n");
                    }
                    else
                    {
                        Clk_AppPrint(APP_DCC_STR ": DCC register check failed!! \r\n");
                    }
                }
                else
                {
                    Clk_AppPrint(APP_DCC_STR ": Error : Could not capture expected DCC register values!!!\r\n");
                }
            }
        }
        else
        {
            Clk_AppPrint(APP_DCC_STR ": Some/All Tests have failed. \r\n");
        }
    }
    else
    {
        Clk_AppPrint(APP_DCC_STR ": Error : Could not derive clock "
                   "frequency!!!\r\n");
    }

    Clk_AppPrint(APP_DCC_STR ": Completes!!!\r\n");

    return;
}

/* ========================================================================== */
/*                 Internal Function Definitions                              */
/* ========================================================================== */

static void Clk_AppPrint(char * str)
{
    DebugP_log(str);
}

static int32_t ClkApp_GetClkfreqKHz(SDL_DCC_Inst dccInst,
                                        uint32_t clkSrc0, uint32_t clkSrc1,
                                        uint32_t *clk0Freq, uint32_t *clk1Freq)
{
    int32_t retVal = CSL_EFAIL;
    switch (dccInst)
    {
        case SDL_DCC_INST_MCU_DCC0 :
            if (APP_DCC_REF_CLOCK_SRC_0 == clkSrc0)
            {
                /* 25 MHz */
                *clk0Freq = APP_DCC_REF_CLOCK_FREQ_IN_KHZ;
                retVal = CSL_PASS;
            }
            if ((APP_DCC_TEST_CLOCK_SRC_1 == clkSrc1) &&
                (CSL_PASS == retVal))
            {
                /* 200MHz */
                *clk1Freq = APP_DCC_TEST_CLOCK_FREQ_IN_KHZ;
            }
            else
            {
                retVal = CSL_EFAIL;
            }
            if (CSL_PASS != retVal)
            {
                DebugP_log(APP_DCC_STR ": Error : Selected clock sources is"
                        " not supported in this example !!!\r\n");
            }
        break;

        default :
            DebugP_log(APP_DCC_STR ": Error : DCC Instance not supported in"
                        " this example !!!\r\n");
        break;
    }
    return (retVal);
}

static void ClkApp_GetClkRatio(uint32_t  refClkFreq,
                              uint32_t  testClkFreq,
                              uint32_t *refClkRatioNum,
                              uint32_t *testClkRatioNum)
{
    uint32_t loopCnt, hcf = 1U;

    for (loopCnt = 1;
         (loopCnt <= refClkFreq) || (loopCnt <= testClkFreq);
         loopCnt++)
    {
        if ((refClkFreq % loopCnt == 0) && (testClkFreq % loopCnt == 0))
        {
            hcf = loopCnt;
        }
    }
    *refClkRatioNum  = (refClkFreq / hcf);
    *testClkRatioNum = (testClkFreq / hcf);
}

static void ClkApp_SetSeedVals(uint32_t        refClkFreq,
                               uint32_t        testClkFreq,
                               uint32_t        refClkRatioNum,
                               uint32_t        testClkRatioNum,
                               uint32_t        drift,
                               SDL_DCC_Config *configParams)
{
    float asyncErr, dccErr, window, freqErr, totErr, driftPer;

    /* Calculate asyncErr depending on higher frequency */
    if (refClkFreq > testClkFreq)
    {
        asyncErr = 2.0f * ((float)refClkRatioNum/(float)testClkRatioNum) + 2.0f * ((float)APP_DCC_SYSCLK_FREQ/(float)refClkFreq);
    }
    else
    {
        asyncErr = 2.0f + 2.0f * ((float)APP_DCC_SYSCLK_FREQ/(float)refClkFreq);
    }

    /* Calculate seed values */
    dccErr = asyncErr + APP_DCC_DIGITIZATION_ERR;

    if (100U < drift)
    {
        /* Drift greater than 100 */
        DebugP_log(APP_DCC_STR ": Drift set is greater than 100%\r\n");
        DebugP_log(APP_DCC_STR ": Application will try to run with minimum allowed drift\r\n");

        driftPer = (100.0f * dccErr * (float)testClkRatioNum) / ((float)refClkRatioNum * APP_DCC_MOSC_SETUP_TIME);
    }
    else
    {
        driftPer = (float)drift;
    }

    if (driftPer < APP_DCC_MIN_DRIFT)
    {
        driftPer = APP_DCC_MIN_DRIFT;
    }
    else if (driftPer > APP_DCC_MAX_DRIFT)
    {
        DebugP_log(APP_DCC_STR ": Error - bad clock frequencies, setting driftPer to 48%\r\n");
        driftPer = APP_DCC_MAX_DRIFT;
    }

    window = dccErr / (0.01f * driftPer);
    freqErr = window * (driftPer / 100.0f);
    totErr = dccErr + freqErr;
    configParams->clk0Seed = (uint32_t)(window - totErr);
    configParams->clk1Seed = (uint32_t)(window * ((float)testClkRatioNum / (float)refClkRatioNum));
    configParams->clk0ValidSeed = (uint32_t)(2.0f * totErr);
    /* Seed values exceed range */
    if (APP_DCC_SRC0_MAX_VAL < configParams->clk0Seed)
    {
        DebugP_log(APP_DCC_STR ": Warning - Clk 0 seed is set higher than max value. Reducing to max value.\r\n");
        configParams->clk0Seed = APP_DCC_SRC0_MAX_VAL;
    }
    if (APP_DCC_SRC0_VALID_MAX_VAL < configParams->clk0ValidSeed)
    {
        DebugP_log(APP_DCC_STR ": Warning - Valid seed is set higher than max value. Reducing to max value.\r\n");
        configParams->clk0ValidSeed = APP_DCC_SRC0_VALID_MAX_VAL;
    }
    DebugP_log(APP_DCC_STR ": Seed values calculation done.\r\n");
}

static int32_t ClkApp_RegisterIsr(SDL_DCC_Inst dccInst)
{
    int32_t retVal = SystemP_FAILURE;
    HwiP_Params hwiPrms;

    switch (dccInst)
    {
        case SDL_DCC_INST_MCU_DCC0 :
            HwiP_Params_init(&hwiPrms);
            hwiPrms.intNum   = CSLR_MCU_R5FSS0_CORE0_CPU0_INTR_MCU_DCC0_INTR_DONE_LEVEL_0;
            hwiPrms.callback = &ClkApp_DoneIntrISR;
            hwiPrms.args     = NULL;
            hwiPrms.priority = 1U;
            retVal = HwiP_construct(&gDccHwiObject, &hwiPrms);
            if (SystemP_SUCCESS != retVal)
            {
                retVal = CSL_EFAIL;
                DebugP_log(APP_DCC_STR ": Error Could not register ISR !!! \r\n");
            }
            else
            {
                retVal = CSL_PASS;
            }
        break;

        default :
            Clk_AppPrint(APP_DCC_STR ": Error : DCC Instance not supported in"
                        " this example !!! \r\n");
        break;
    }
    return (retVal);
}

static void ClkApp_DoneIntrISR(void *args)
{
    SDL_DCC_clearIntr(APP_DCC_MODULE_INST, SDL_DCC_INTERRUPT_DONE);
    isrFlag  = 1U;
}

static void ClkApp_Delay(uint32_t wait_in_ms)
{
    ClockP_usleep(wait_in_ms * 1000U);
}

int main(void)
{
    int32_t status = SystemP_SUCCESS;

    System_init();
    Board_init();

    Drivers_open();
    status = Board_driversOpen();
    DebugP_assert(status == SystemP_SUCCESS);

    status = SDL_TEST_dplInit();
    DebugP_assert(status == SDL_PASS);

    (void) ClkApp_TestSdlDccSingleshotmodeApp();

    Board_driversClose();
    Drivers_close();

    Board_deinit();
    System_deinit();

    return(0);
}

/********************************* End of file ******************************/
