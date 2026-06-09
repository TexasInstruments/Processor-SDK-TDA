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
 *  \brief    This example performs single-shot ADC conversions on
 *            two independent ADC instances connected to separate
 *            input channels and compares their results in software.
 *            The result comparison allows detection of inconsistencies
 *            between the ADC instances, enabling high diagnostic coverage of
 *            ADC functionality.
 **/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdint.h>
#include <stdio.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/board.h>
#include <ti/osal/osal.h>
#include <ti/drv/sciclient/sciclient.h>
#include <ti/csl/csl_adc.h>

#if defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_soc_baseaddress.h>
#elif defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_soc_baseaddress.h>
#endif

/* ========================================================================== */
/*                                Macros                                      */
/* ========================================================================== */
#if defined (SOC_J784S4)
/* Base address of ADC0 instance */
#define APP_ADC0_MODULE          (CSL_MCU_ADC12FCC0_ADC_BASE)
/* Base address of ADC1 instance */
#define APP_ADC1_MODULE          (CSL_MCU_ADC12FCC1_ADC_BASE)
#elif defined (SOC_J721S2)
/* Base address of ADC0 instance */
#define APP_ADC0_MODULE          (CSL_MCU_ADC0_BASE)
/* Base address of ADC1 instance */
#define APP_ADC1_MODULE          (CSL_MCU_ADC1_BASE)
#endif

/* Device ID used by System Controller to control ADC0 power and reset */
#define APP_ADC0_DEV_ID          (TISCI_DEV_MCU_ADC12FC_16FFC0)
/* Device ID used by System Controller to control ADC1 power and reset */
#define APP_ADC1_DEV_ID          (TISCI_DEV_MCU_ADC12FC_16FFC1)

/* Interrupt vector number for ADC0 */
#define APP_ADC0_INT_VEC         (CSLR_MCU_R5FSS0_CORE0_INTR_MCU_ADC12FCC0_GEN_LEVEL_0)
/* Interrupt vector number for ADC1 */
#define APP_ADC1_INT_VEC         (CSLR_MCU_R5FSS0_CORE0_INTR_MCU_ADC12FCC1_GEN_LEVEL_0)

/* ADC input clock divider value */
#define APP_ADC_DIV             (1U)
/* Reference voltage for ADC - mV */
#define APP_ADC_REF_VOLTAGE     (1800U)
/* Maximum ADC conversion range (12-bit resolution) */
#define APP_ADC_RANGE_MAX       (4096U)

/* Maximum allowed difference between ADC0 and ADC1 conversion results (LSB) */
#define APP_ADC_TOLERANCE_LSB   (10)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* Flags set by ADC interrupt service routines to indicate
 * end of conversion for each ADC instance */
volatile uint32_t gIsrFlag0 = 0U;
volatile uint32_t gIsrFlag1 = 0U;

/* ========================================================================== */
/*                 Internal Function Declarations                             */
/* ========================================================================== */

/**
 * \brief ADC0 interrupt service routine.
 *
 * This interrupt service routine handles interrupts generated by ADC0.
 * It reads and clears the ADC interrupt status, reports the interrupt
 * source for debug purposes, and signals completion of the conversion
 * sequence to the main application.
 *
 * \param handle  Interrupt handle (unused).
 */
static void AdcApp_ADC0IntrISR(void *handle);

/**
 * \brief ADC1 interrupt service routine.
 *
 * This interrupt service routine handles interrupts generated by ADC1.
 * It reads and clears the ADC interrupt status, reports the interrupt
 * source for debug purposes, and signals completion of the conversion
 * sequence to the main application.
 *
 * \param handle  Interrupt handle (unused).
 */
static void AdcApp_ADC1IntrISR(void *handle);

/**
 * \brief Provides a fixed delay for ADC stabilization.
 *
 * This function introduces a delay to allow ADC analog front-end
 * stabilization after power-up or configuration changes.
 */
static void AdcApp_Wait(void);

/**
 * \brief Initializes the specified ADC module.
 *
 * This function retrieves and prints the ADC revision information,
 * clears any pending interrupt status, powers up the ADC analog
 * front-end, performs internal calibration, and configures the ADC
 * clock divider.
 *
 * \param base  Base address of the ADC instance to be initialized.
 *
 * \return STW_SOK on successful initialization, or STW_EFAIL on failure.
 */
static int32_t AdcApp_Init(uint32_t base);

/**
 * \brief Starts ADC conversion on the specified ADC instance.
 *
 * This function checks that the ADC sequencer FSM (Finite State Machine)
 * is idle before initiating a single-shot conversion. Once the sequencer
 * is confirmed idle, the ADC conversion is started.
 *
 * \param base  Base address of the ADC instance to start conversion on.
 */
static void AdcApp_Start(uint32_t base);

/**
 * \brief Stops ADC conversion on the specified ADC instance.
 *
 * This function disables the configured ADC step and ensures that the ADC
 * sequencer FSM (Finite State Machine) is idle before and after stopping
 * the conversion. It safely stops the ADC to prevent ongoing conversions
 * or data corruption.
 *
 * \param base  Base address of the ADC instance to stop conversion on.
 */
static void AdcApp_Stop(uint32_t base);

/**
 * \brief Initializes the board and UART for console output.
 *
 * This function configures the board pinmux settings and initializes the
 * UART standard I/O interface. It enables printing debug and ADC data
 * to the console via UART.
 */
static void AdcApp_UartInit(void);

/**
 * \brief Prints a string to the UART console.
 *
 * \param str Pointer to the null-terminated string to print.
 */
static void AdcApp_Print(const char * str);

/**
 * \brief Prints an unsigned 32-bit integer in decimal format to UART.
 *
 * \param value The integer value to print.
 */
static void AdcApp_PrintNum(uint32_t value);

/**
 * \brief Prints an unsigned 32-bit integer in hexadecimal format to UART.
 *
 * \param value The integer value to print in hex format.
 */
static void AdcApp_PrintHexNum(uint32_t value);

/**
 * \brief Configures and registers an interrupt for the ADC module.
 *
 * This function sets up an interrupt vector and associates it with the
 * provided ISR (Interrupt Service Routine) using the OSAL interrupt API.
 *
 * \param intVec The interrupt vector number to register.
 * \param isr    Pointer to the ISR function to handle the interrupt.
 */
static void AdcApp_ConfigureInterrupt(uint32_t intVec, void (*isr)(uintptr_t));

/**
 * \brief Enables the specified ADC module.
 *
 * This function powers up and enables the ADC module using the SCICLIENT
 * power management API. The module is set to ON state with exclusive access
 * and reset isolation flags.
 *
 * \param devId The device ID of the ADC module to enable.
 */
static void AdcApp_ModuleEnable(uint32_t devId);

/**
 * \brief Disables the specified ADC module.
 *
 * This function powers down the ADC module using the SCICLIENT power management
 * API. The module is set to AUTO_OFF state with exclusive access and reset
 * isolation flags.
 *
 * \param devId The device ID of the ADC module to disable.
 */
static void AdcApp_ModuleDisable(uint32_t devId);

/**
 * \brief Returns the name of the ADC module based on its base address.
 *
 * This function identifies the ADC instance by comparing the provided base
 * address with known ADC module addresses and returns a human-readable name.
 *
 * \param base The base address of the ADC module.
 * 
 * \return A string representing the ADC module name ("ADC0", "ADC1", or "ADC?" if unknown).
 */
static const char *AdcApp_GetAdcName(uint32_t base);

/**
 * \brief Performs a single‑shot conversion on two ADC instances and 
 *        compares their results to detect any mismatch.
 * 
 * \return None.
 */
static void AdcApp_test_csl_adc_singleshot_test_app(void);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */
static void AdcApp_test_csl_adc_singleshot_test_app(void)
{
    int32_t         configStatus, adc0Value, adc1Value, testErrCount = 0;
    uint32_t        fifoData, fifoWordCnt, voltageLvl;
    adcStepConfig_t adc0Config, adc1Config;

    /* Initialize the UART Instance */
    AdcApp_UartInit();

    AdcApp_Print("\nStarting application...\n");
    
    AdcApp_ConfigureInterrupt(APP_ADC0_INT_VEC, (void (*)(uintptr_t))(&AdcApp_ADC0IntrISR));
    AdcApp_ConfigureInterrupt(APP_ADC1_INT_VEC, (void (*)(uintptr_t))(&AdcApp_ADC1IntrISR));

    /* Enable ADC module */    
    AdcApp_ModuleEnable(APP_ADC0_DEV_ID);
    AdcApp_ModuleEnable(APP_ADC1_DEV_ID);

    /* Initialize ADC module */
    configStatus = AdcApp_Init(APP_ADC0_MODULE);
    if (STW_SOK != configStatus)
    {
        AdcApp_Print("Error in ADC0 divider configuration.\n");
        testErrCount++;
    }

    configStatus = AdcApp_Init(APP_ADC1_MODULE);
    if (STW_SOK != configStatus)
    {
        AdcApp_Print("Error in ADC1 divider configuration.\n");
        testErrCount++;
    }

    /* Initialize ADC0 configuration params */
    adc0Config.mode             = ADC_OPERATION_MODE_SINGLE_SHOT;
    adc0Config.channel          = ADC_CHANNEL_4;
    adc0Config.openDelay        = 0x1U;
    adc0Config.sampleDelay      = 0U;
    adc0Config.rangeCheckEnable = 0U;
    adc0Config.averaging        = ADC_AVERAGING_16_SAMPLES;
    adc0Config.fifoNum          = ADC_FIFO_NUM_0;

    /* Initialize ADC0 configuration params */
    adc1Config.mode             = ADC_OPERATION_MODE_SINGLE_SHOT;
    adc1Config.channel          = ADC_CHANNEL_1;
    adc1Config.openDelay        = 0x1U;
    adc1Config.sampleDelay      = 0U;
    adc1Config.rangeCheckEnable = 0U;
    adc1Config.averaging        = ADC_AVERAGING_16_SAMPLES;
    adc1Config.fifoNum          = ADC_FIFO_NUM_1;

    /* Enable interrupts */
    ADCEnableIntr(APP_ADC0_MODULE, (ADC_INTR_SRC_END_OF_SEQUENCE));
    ADCEnableIntr(APP_ADC1_MODULE, (ADC_INTR_SRC_END_OF_SEQUENCE));

    ADCStepIdTagEnable(APP_ADC0_MODULE, TRUE);
    ADCStepIdTagEnable(APP_ADC1_MODULE, TRUE);

    configStatus = ADCSetCPUFIFOThresholdLevel(APP_ADC0_MODULE, ADC_FIFO_NUM_0, 40U);
    if (STW_SOK != configStatus)
    {
        AdcApp_Print("Error in ADC0 CPU threshold configuration.\n");
        testErrCount++;
    }

    configStatus = ADCSetCPUFIFOThresholdLevel(APP_ADC1_MODULE, ADC_FIFO_NUM_1, 40U);
    if (STW_SOK != configStatus)
    {
        AdcApp_Print("Error in ADC1 CPU threshold configuration.\n");
        testErrCount++;
    }

    configStatus = ADCSetStepParams(APP_ADC0_MODULE, ADC_STEP_1, &adc0Config);
    if (configStatus != STW_SOK)
    {
        AdcApp_Print("Error in ADC0 step configuration.\n");
        testErrCount++;
    }
    /* step enable */
    ADCStepEnable(APP_ADC0_MODULE, ADC_STEP_1, TRUE);

    configStatus = ADCSetStepParams(APP_ADC1_MODULE, ADC_STEP_1, &adc1Config);
    if (configStatus != STW_SOK)
    {
        AdcApp_Print("Error in ADC1 step configuration.\n");
        testErrCount++;
    }
    /* step enable */
    ADCStepEnable(APP_ADC1_MODULE, ADC_STEP_1, TRUE);

    /* Start conversion */
    gIsrFlag0 = 0U;
    gIsrFlag1 = 0U;
    AdcApp_Start(APP_ADC0_MODULE);
    AdcApp_Start(APP_ADC1_MODULE);
    while (0U == gIsrFlag0) { }
    while (0U == gIsrFlag1) { }

    /*Get FIFO 0 data */
    fifoWordCnt = ADCGetFIFOWordCount(APP_ADC0_MODULE, ADC_FIFO_NUM_0);

    fifoData = ADCGetFIFOData(APP_ADC0_MODULE, ADC_FIFO_NUM_0);
    fifoData = ((fifoData & ADC_FIFODATA_ADCDATA_MASK) >>
                ADC_FIFODATA_ADCDATA_SHIFT);
    adc0Value = fifoData;
    voltageLvl  = fifoData * (uint32_t) APP_ADC_REF_VOLTAGE;
    voltageLvl /= (uint32_t) APP_ADC_RANGE_MAX;

    AdcApp_Print("\n[ADC0] Samples in FIFO_0 : ");
    AdcApp_PrintNum((uint32_t)fifoWordCnt);
    AdcApp_Print("\n");

    AdcApp_Print("[ADC0] Raw Code          : ");
    AdcApp_PrintNum((uint32_t)adc0Value);
    AdcApp_Print("\n");

    AdcApp_Print("[ADC0] Voltage Level     : ");
    AdcApp_PrintNum((uint32_t)voltageLvl);
    AdcApp_Print(" mV\n\n");  

    /*Get FIFO_1 data */
    fifoWordCnt = ADCGetFIFOWordCount(APP_ADC1_MODULE, ADC_FIFO_NUM_1);

    fifoData = ADCGetFIFOData(APP_ADC1_MODULE, ADC_FIFO_NUM_1);
    fifoData = ((fifoData & ADC_FIFODATA_ADCDATA_MASK) >>
                ADC_FIFODATA_ADCDATA_SHIFT);
    adc1Value = fifoData;
    voltageLvl  = fifoData * (uint32_t) APP_ADC_REF_VOLTAGE;
    voltageLvl /= (uint32_t) APP_ADC_RANGE_MAX;

    AdcApp_Print("[ADC1] Samples in FIFO_1 : ");
    AdcApp_PrintNum((uint32_t)fifoWordCnt);
    AdcApp_Print("\n");

    AdcApp_Print("[ADC1] Raw Code          : ");
    AdcApp_PrintNum((uint32_t)adc1Value);
    AdcApp_Print("\n");

    AdcApp_Print("[ADC1] Voltage Level     : ");
    AdcApp_PrintNum((uint32_t)voltageLvl);
    AdcApp_Print(" mV\n\n");

    AdcApp_Print("Checking consistency between ADC0 and ADC1...\n");

    uint32_t diffCode;
    diffCode = (adc0Value > adc1Value) ?
                (adc0Value - adc1Value) :
                (adc1Value - adc0Value);


    AdcApp_Print("       ADC0 Raw Code      : "); AdcApp_PrintNum(adc0Value); AdcApp_Print("\n");
    AdcApp_Print("       ADC1 Raw Code      : "); AdcApp_PrintNum(adc1Value); AdcApp_Print("\n");
    AdcApp_Print("       -----------------------------------\n");
    AdcApp_Print("       Absolute Difference : "); AdcApp_PrintNum(diffCode); AdcApp_Print(" LSB\n");
    AdcApp_Print("       Allowed Tolerance   : "); AdcApp_PrintNum(APP_ADC_TOLERANCE_LSB); AdcApp_Print(" LSB\n");

    if (diffCode > APP_ADC_TOLERANCE_LSB)
    {
        AdcApp_Print("\n[FAIL] ADC mismatch exceeds tolerance!\n\n");
        testErrCount++;
    }
    else
    {
        AdcApp_Print("\n[PASS] ADC0 and ADC1 readings are consistent.\n\n");
    }

    AdcApp_Stop(APP_ADC0_MODULE);
    AdcApp_Stop(APP_ADC1_MODULE);
    /* Power down ADC */
    ADCPowerUp(APP_ADC0_MODULE, FALSE);
    ADCPowerUp(APP_ADC1_MODULE, FALSE);
    /* Disable ADC module */
    AdcApp_ModuleDisable(APP_ADC0_DEV_ID);
    AdcApp_ModuleDisable(APP_ADC1_DEV_ID);

    AdcApp_Print("\nApplication is completed.\n");

    if (testErrCount == 0)
    {
        AdcApp_Print("\n All tests have passed. \n");
    }
    else
    {
        AdcApp_Print("\n ADC Test failed. \n");
    }
}

/* ========================================================================== */
/*                 Internal Function Definitions                              */
/* ========================================================================== */

static void AdcApp_ADC0IntrISR(void *handle)
{
    uint32_t status;

    AdcApp_Print("\nIn ISR...\n");
    status = ADCGetIntrStatus(APP_ADC0_MODULE);
    ADCClearIntrStatus(APP_ADC0_MODULE, status);

    if (ADC_INTR_SRC_END_OF_SEQUENCE == (status & ADC_INTR_SRC_END_OF_SEQUENCE))
    {
        AdcApp_Print("End of sequence interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO0_THRESHOLD == (status & ADC_INTR_SRC_FIFO0_THRESHOLD))
    {
        AdcApp_Print("FIFO 0 threshold interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO0_OVERRUN == (status & ADC_INTR_SRC_FIFO0_OVERRUN))
    {
        AdcApp_Print("FIFO 0 overrun interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO0_UNDERFLOW == (status & ADC_INTR_SRC_FIFO0_UNDERFLOW))
    {
        AdcApp_Print("FIFO 0 underflow interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO1_THRESHOLD == (status & ADC_INTR_SRC_FIFO1_THRESHOLD))
    {
        AdcApp_Print("FIFO 1 threshold interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO1_OVERRUN == (status & ADC_INTR_SRC_FIFO1_OVERRUN))
    {
        AdcApp_Print("FIFO 1 overrun interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO1_UNDERFLOW == (status & ADC_INTR_SRC_FIFO1_UNDERFLOW))
    {
        AdcApp_Print("FIFO 1 underflow interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_OUT_OF_RANGE == (status & ADC_INTR_SRC_OUT_OF_RANGE))
    {
        AdcApp_Print("Out of range interrupt occurred.\n");
    }

    gIsrFlag0++;
    ADCWriteEOI(APP_ADC0_MODULE);
}

static void AdcApp_ADC1IntrISR(void *handle)
{
    uint32_t status;

    AdcApp_Print("\nIn ISR...\n");
    status = ADCGetIntrStatus(APP_ADC1_MODULE);
    ADCClearIntrStatus(APP_ADC1_MODULE, status);

    if (ADC_INTR_SRC_END_OF_SEQUENCE == (status & ADC_INTR_SRC_END_OF_SEQUENCE))
    {
        AdcApp_Print("End of sequence interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO0_THRESHOLD == (status & ADC_INTR_SRC_FIFO0_THRESHOLD))
    {
        AdcApp_Print("FIFO 0 threshold interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO0_OVERRUN == (status & ADC_INTR_SRC_FIFO0_OVERRUN))
    {
        AdcApp_Print("FIFO 0 overrun interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO0_UNDERFLOW == (status & ADC_INTR_SRC_FIFO0_UNDERFLOW))
    {
        AdcApp_Print("FIFO 0 underflow interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO1_THRESHOLD == (status & ADC_INTR_SRC_FIFO1_THRESHOLD))
    {
        AdcApp_Print("FIFO 1 threshold interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO1_OVERRUN == (status & ADC_INTR_SRC_FIFO1_OVERRUN))
    {
        AdcApp_Print("FIFO 1 overrun interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_FIFO1_UNDERFLOW == (status & ADC_INTR_SRC_FIFO1_UNDERFLOW))
    {
        AdcApp_Print("FIFO 1 underflow interrupt occurred.\n");
    }
    if (ADC_INTR_SRC_OUT_OF_RANGE == (status & ADC_INTR_SRC_OUT_OF_RANGE))
    {
        AdcApp_Print("Out of range interrupt occurred.\n");
    }

    gIsrFlag1++;
    ADCWriteEOI(APP_ADC1_MODULE);
}

static int32_t AdcApp_Init(uint32_t base)
{
    adcRevisionId_t revId;
    int32_t         configStatus = STW_EFAIL;
    const char     *adcName = AdcApp_GetAdcName(base);

    /* Get ADC information */
    ADCGetRevisionId(base, &revId);
    AdcApp_Print(adcName);
    AdcApp_Print(" Revision ID:\n");
    AdcApp_Print("Scheme            :"); AdcApp_PrintHexNum((uint32_t)revId.scheme); AdcApp_Print("\n");
    AdcApp_Print("Functional number :"); AdcApp_PrintHexNum((uint32_t)revId.func);   AdcApp_Print("\n");
    AdcApp_Print("RTL revision      :"); AdcApp_PrintHexNum((uint32_t)revId.rtlRev); AdcApp_Print("\n");
    AdcApp_Print("Major revision    :"); AdcApp_PrintHexNum((uint32_t)revId.major);  AdcApp_Print("\n");
    AdcApp_Print("Minor revision    :"); AdcApp_PrintHexNum((uint32_t)revId.minor);  AdcApp_Print("\n");
    AdcApp_Print("Custom revision   :"); AdcApp_PrintHexNum((uint32_t)revId.custom); AdcApp_Print("\n");

    /* Clear All interrupt status */
    ADCClearIntrStatus(base, ADC_INTR_STATUS_ALL);
    /* Power up AFE */
    ADCPowerUp(base, TRUE);
    /* Wait for >=4us (ovde 10ms za sigurno start-up okruženje) */
    AdcApp_Wait();
    /* Do the internal calibration */
    ADCInit(base, FALSE, 0U, 0U);
    /* Configure ADC divider*/
    configStatus = ADCSetClkDivider(base, APP_ADC_DIV);

    return configStatus;
}

static void AdcApp_Start(uint32_t base)
{
    adcSequencerStatus_t status;

    /* Check if FSM is idle */
    ADCGetSequencerStatus(base, &status);
    while ((ADC_ADCSTAT_FSM_BUSY_IDLE != status.fsmBusy) &&
           (ADC_ADCSTAT_STEP_ID_IDLE != status.stepId))
    {
        ADCGetSequencerStatus(base, &status);
    }
    /* Start ADC conversion */
    ADCStart(base, TRUE);
}

static void AdcApp_Stop(uint32_t base)
{
    adcSequencerStatus_t status;
    ADCStepEnable(base, ADC_STEP_1, FALSE);

    /* Wait for FSM to go IDLE */
    ADCGetSequencerStatus(base, &status);
    while ((ADC_ADCSTAT_FSM_BUSY_IDLE != status.fsmBusy) &&
           (ADC_ADCSTAT_STEP_ID_IDLE != status.stepId))
    {
        ADCGetSequencerStatus(base, &status);
    }
    /* Stop ADC */
    ADCStart(base, FALSE);
    /* Wait for FSM to go IDLE */
    ADCGetSequencerStatus(base, &status);
    while ((ADC_ADCSTAT_FSM_BUSY_IDLE != status.fsmBusy) &&
           (ADC_ADCSTAT_STEP_ID_IDLE != status.stepId))
    {
        ADCGetSequencerStatus(base, &status);
    }
}

static void AdcApp_Wait(void)
{
    Osal_delay(10U);
}

static void AdcApp_ConfigureInterrupt(uint32_t intVec, void (*isr)(uintptr_t))
{
    OsalRegisterIntrParams_t intrPrms;
    OsalInterruptRetCode_e osalRetVal;
    HwiP_Handle hwiHandle;

    Osal_RegisterInterrupt_initParams(&intrPrms);
    intrPrms.corepacConfig.arg          = (uintptr_t)0;
    intrPrms.corepacConfig.priority     = 1U;
    intrPrms.corepacConfig.corepacEventNum = 0U;

    intrPrms.corepacConfig.intVecNum = intVec;

    intrPrms.corepacConfig.isrRoutine   = isr;
    osalRetVal = Osal_RegisterInterrupt(&intrPrms, &hwiHandle);
    if(OSAL_INT_SUCCESS != osalRetVal)
    {
        AdcApp_Print("Error Could not register ISR !!!\n");
    }
}

static void AdcApp_ModuleEnable(uint32_t devId)
{
    /* Enable ADC module */
    Sciclient_pmSetModuleState(devId,
        TISCI_MSG_VALUE_DEVICE_SW_STATE_ON,
        TISCI_MSG_FLAG_AOP |
        TISCI_MSG_FLAG_DEVICE_EXCLUSIVE |
        TISCI_MSG_FLAG_DEVICE_RESET_ISO,
        SCICLIENT_SERVICE_WAIT_FOREVER);
}

static void AdcApp_ModuleDisable(uint32_t devId)
{
    /* Disable ADC module */
    Sciclient_pmSetModuleState(devId,
        TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF,
        TISCI_MSG_FLAG_AOP |
        TISCI_MSG_FLAG_DEVICE_EXCLUSIVE |
        TISCI_MSG_FLAG_DEVICE_RESET_ISO,
        SCICLIENT_SERVICE_WAIT_FOREVER);
}

static void AdcApp_UartInit(void)
{
    Board_initCfg boardCfg;
    Board_STATUS  boardStatus;

    boardCfg = BOARD_INIT_PINMUX_CONFIG |
               BOARD_INIT_UART_STDIO;
    boardStatus = Board_init(boardCfg);
    if (boardStatus != BOARD_SOK)
    {
        AdcApp_Print("[Error] Board init failed!!\n");
    }
}

static const char *AdcApp_GetAdcName(uint32_t base)
{
    if (base == APP_ADC0_MODULE)
    {
        return "ADC0";
    }
    else if (base == APP_ADC1_MODULE)
    {
        return "ADC1";
    }
    else
    {
        return "ADC?";
    }
}

static void AdcApp_Print(const char * str)
{
    UART_printf(str);
}

static void AdcApp_PrintNum(uint32_t value)
{
    UART_printf("%d", value);
}

static void AdcApp_PrintHexNum(uint32_t value)
{
    UART_printf("0x%x", value);
}

int main(void)
{
    (void) AdcApp_test_csl_adc_singleshot_test_app();
}

/********************************* End of file ******************************/