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
 *  \brief DDR parity checker on controller and PHY configuration registers
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/board/board.h>

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

#include <ti/csl/src/ip/rat/V0/csl_rat.h>
#include <ti/csl/src/ip/ecc_aggr/V1/cslr_ecc_aggr.h>
#include <ti/csl/src/ip/ecc_aggr/V1/csl_ecc_aggr.h>

#include <ti/csl/soc.h>
#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_intr_esm0.h>
#include <ti/board/src/j721s2_evm/include/board_ddr.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_intr_esm0.h>
#include <ti/board/src/j784s4_evm/include/board_ddr.h>
#endif

#include "esm.h"
#include <ti/ip_fma/inc/ip_fma_common.h>
#include <ddr_parity_protection.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define DDR17_INTERRUPT_WAIT_DELAY                  (10U)
#define DDR17_INTERRUPT_WAIT_TIMEOUT                (3000u)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

uint32_t gDdrCtlTestPassedCount = 0U;
uint32_t gDdrCtlTestCount = 0U;

uint32_t gDdrPhyPiTestPassedCount = 0U;
uint32_t gDdrPhyPiTestCount = 0U;

/* This variable is set by the interrupt handler to indicate that the interrupt has occurred. */
volatile Bool gDdrParityInterruptInvoked = FALSE;

/* Global variables used by the interrupt handler to verify the correct interrupt was invoked. */
uint32_t gParityInterruptId;
uint32_t gDdrBaseAddress;
IpFma_Ddr17CtlParityErrorSource gCtlGlobalErrorSource;
IpFma_Ddr17PhyParityErrorSource gPhyParityErrorSource;

/* DDR CTL controller interrupt IDs. */
uint32_t gDdrCtlModulesInterruptIds[] = {
        CSLR_ESM0_ESM_LVL_EVENT_DDR0_DDRSS_CONTROLLER_GLOBAL_ERROR_FATAL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR1_DDRSS_CONTROLLER_GLOBAL_ERROR_FATAL_0,
#if defined(SOC_J784S4)
        CSLR_ESM0_ESM_LVL_EVENT_DDR2_DDRSS_CONTROLLER_GLOBAL_ERROR_FATAL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR3_DDRSS_CONTROLLER_GLOBAL_ERROR_FATAL_0,
#endif
};

/* DDR PHY controller interrupt IDs. */
uint32_t gDdrPhyModulesInterruptIds[] = {
    CSLR_ESM0_ESM_LVL_EVENT_DDR0_DDRSS_HS_PHY_GLOBAL_ERROR_0,
    CSLR_ESM0_ESM_LVL_EVENT_DDR1_DDRSS_HS_PHY_GLOBAL_ERROR_0,
#if defined(SOC_J784S4)
    CSLR_ESM0_ESM_LVL_EVENT_DDR2_DDRSS_HS_PHY_GLOBAL_ERROR_0,
    CSLR_ESM0_ESM_LVL_EVENT_DDR3_DDRSS_HS_PHY_GLOBAL_ERROR_0,
#endif
};

/* Base addresses for DDR modules. */
uint32_t gDdrModulesBaseAddr[] = {
        DDRSS0_BASE_ADDRESS,
        DDRSS1_BASE_ADDRESS,
#if defined(SOC_J784S4)
        DDRSS2_BASE_ADDRESS,
        DDRSS3_BASE_ADDRESS
#endif
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t Ddr17App_Init(void);
static int32_t Ddr17App_EsmSetup(uint32_t esmInterruptId);

static void Ddr17App_testDdrCtlRegsParityProtection();
static void Ddr17App_testDdrPhyPiRegsParityProtection();

static void Ddr17App_InjectCtlParityErrorAndVerify(uint32_t baseAddress);
static void Ddr17App_CtlParityTest(uint32_t baseAddress, IpFma_Ddr17CtlParityErrorSource source);

static void Ddr17App_PhyInjectError(uint32_t* parityErrInjRegAddr, uint32_t errInjOffset);
static void Ddr17App_PhyParityTest(uint32_t* parityErrInjRegAddr, IpFma_Ddr17PhyParityErrorSource injectedParityErrorInfo, uint32_t errInjOffset);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 * \brief Waits for the interrupt to be invoked.
 *
 * \param   testPassCounter    Counter of passed tests
 */
static void Ddr17App_WaitForInterrupt(uint32_t* testPassCounter, uint32_t* testCounter)
{
    IpFma_Status status = IPFMA_OK;

    /* Increment total number of tests. */
    (*testCounter)++;

    /* Wait for parity error triggered interrupt */
    uint32_t timeOutCnt = 0U;
    do
    {
        Osal_delay(DDR17_INTERRUPT_WAIT_DELAY);
        timeOutCnt += DDR17_INTERRUPT_WAIT_DELAY;
        if (timeOutCnt > DDR17_INTERRUPT_WAIT_TIMEOUT)
        {
            status = IPFMA_E_IO;
            UART_printf("\r\n Timed out while waiting for the interrupt.");
            break;
        }
    } while (gDdrParityInterruptInvoked == FALSE);

    if (IPFMA_OK == status)
    {
        UART_printf("\n Injection successful, interrupt invoked and handled!");
        (*testPassCounter)++;
    }
    else
    {
        UART_printf("\n ERROR: Injection failed!");
    }
}

/**
 * \brief Injects parity error into a CTL register and waits for interrupt to be invoked.
 *
 * \param   baseAddress         Base address of the DDR module.
 */
static void Ddr17App_InjectCtlParityErrorAndVerify(uint32_t baseAddress)
{
    /* Flip a bit of a random CTL config register, in this example emif_ew_ctlcfg_DDRSS_CTL_361 (CALVL_BG_PAT_0) is used. */
    uint32_t randomCtlRegOffset = 0x5A4U;
    uint32_t* randomCtlReg = (uint32_t*) (baseAddress + randomCtlRegOffset);
    *randomCtlReg = *randomCtlReg ^ 0x100;  /* Flip bit 8 */

    Ddr17App_WaitForInterrupt(&gDdrCtlTestPassedCount, &gDdrCtlTestCount);

    /* Reverse the bit flip from above. */
    *randomCtlReg = *randomCtlReg ^ 0x100;  /* Flip bit 8 */

    /* Reset the parity interrupt check variable for next test. */
    gDdrParityInterruptInvoked = FALSE;
}

/**
 * \brief This function sets up ESM and registers an interrupt handler
 *
 * \param   esmInterruptId      Interrupt ID in the ESM
 *
 * \return  \ref CSL_PASS       In case of success,
 *          \ref CSL_EFAIL      Or other in case of failure.
 */
static int32_t Ddr17App_EsmSetup(uint32_t esmInterruptId)
{
    int32_t status = CSL_PASS;

    /* Setup ESM */
    CSL_esm_app_R5_cfg cfg;
    cfg.hi_pri_evt = esmInterruptId;
    cfg.lo_pri_evt = 0U;    // Not used.
    status = cslAppEsmSetup(&cfg);

    /* This variable is used in the interrupt handler to check if the correct interrupt was invoked. */
    gParityInterruptId = esmInterruptId;

    return status;
}

/**
 * \brief Runs a CTL register parity test.
 *
 * \param   baseAddress         Base address of the DDR module.
 * \param   source              DDR CTL parity error source type to inject.
 */
static void Ddr17App_CtlParityTest(uint32_t baseAddress, IpFma_Ddr17CtlParityErrorSource source)
{
    IpFma_Status status = IPFMA_OK;

    status = IpFma_Ddr17_EnableCtlParityProtectionInjection(baseAddress, source);
    if (IPFMA_OK == status)
    {
        gCtlGlobalErrorSource = source;
        Ddr17App_InjectCtlParityErrorAndVerify(baseAddress);
    }
}

/**
 * \brief Generic PHY and PI parity error injection.
 *
 * \param   parityErrInjRegAddr   Address of the error injection register.
 * \param   errInjOffset          Offset for the error injection bitfield of the \ref parityErrInjRegAddr.
 */
static void Ddr17App_PhyInjectError(uint32_t* parityErrInjRegAddr, uint32_t errInjOffset)
{
    uint32_t errorToInject = *parityErrInjRegAddr | (0x1 << errInjOffset);
    uint32_t errorToInjectClr = *parityErrInjRegAddr & ~(0x1 << errInjOffset);

    UART_printf("\n\n Interrupts occur on error injection and error injection clearing.");
    UART_printf("\n Error injected.");
    *parityErrInjRegAddr = errorToInject;
    Ddr17App_WaitForInterrupt(&gDdrPhyPiTestPassedCount, &gDdrPhyPiTestCount);
    UART_printf("\n Error cleared.");
    *parityErrInjRegAddr = errorToInjectClr;
    Ddr17App_WaitForInterrupt(&gDdrPhyPiTestPassedCount, &gDdrPhyPiTestCount);
}

/**
 * \brief PHY and PI parity error injection tests.
 *
 * \param   parityErrInjRegAddr         Address of the error injection register.
 * \param   injectedParityErrorInfo     Parity error source.
 * \param   errInjOffset                Offset for the error injection bitfield of the \ref parityErrInjRegAddr.
 */
static void Ddr17App_PhyParityTest(uint32_t* parityErrInjRegAddr,
                                   IpFma_Ddr17PhyParityErrorSource injectedParityErrorInfo,
                                   uint32_t errInjOffset)
{
    UART_printf("\n\n");

    /* Set correct error info to check in interrupt handler. */
    gPhyParityErrorSource = injectedParityErrorInfo;

    /* Special handling in case of injecting parity error to register interface signals for slices [0-3]. */
    if (injectedParityErrorInfo >= DDR_PHY_PARITY_ERR_BIT3)
    {
        uint32_t parityErrInjRegVal = *parityErrInjRegAddr;
        uint32_t errorToInject = 0U;
        uint32_t errorToInjectClr = 0U;

        UART_printf("\n\n Inject error on bit 0 - interrupt on read.");
        errorToInject = parityErrInjRegVal | (0x1 << errInjOffset);
        errorToInjectClr = parityErrInjRegVal & ~(0x1 << errInjOffset);
        UART_printf("\n Error injected.");
        *parityErrInjRegAddr = errorToInject;
        UART_printf("\n Reading from the register.");
        uint32_t triggerRegReadInterrupt = *parityErrInjRegAddr;
        (void) triggerRegReadInterrupt;
        Ddr17App_WaitForInterrupt(&gDdrPhyPiTestPassedCount, &gDdrPhyPiTestCount);
        UART_printf("\n Interrupt should have been invoked!");
        *parityErrInjRegAddr = errorToInjectClr;

        UART_printf("\n\n Inject error on bit 4 - interrupt on write.");
        errorToInject = parityErrInjRegVal | (0x1 << (errInjOffset + 4));
        errorToInjectClr = parityErrInjRegVal & ~(0x1 << (errInjOffset + 4));
        UART_printf("\n Error injected.");
        *parityErrInjRegAddr = errorToInject;
        UART_printf("\n Writing to the register(clearing the injection).");
        *parityErrInjRegAddr = errorToInjectClr;
        Ddr17App_WaitForInterrupt(&gDdrPhyPiTestPassedCount, &gDdrPhyPiTestCount);
        UART_printf("\n Interrupt should have been invoked!");

        UART_printf("\n\n Inject error on bit 8 - interrupt on regmask.");
        errorToInject = parityErrInjRegVal | (0x1 << (errInjOffset + 8));
        errorToInjectClr = parityErrInjRegVal & ~(0x1 << (errInjOffset + 8));
        UART_printf("\n Error injected.");
        *parityErrInjRegAddr = errorToInject;
        UART_printf("\n Writing to the register(clearing the injection).");
        *parityErrInjRegAddr = errorToInjectClr;
        Ddr17App_WaitForInterrupt(&gDdrPhyPiTestPassedCount, &gDdrPhyPiTestCount);
        UART_printf("\n Interrupt should have been invoked!");

        UART_printf("\n\n Inject error on bit 9 - interrupt on register address bus(reading or writing).");
        errorToInject = parityErrInjRegVal | (0x1 << (errInjOffset + 9));
        errorToInjectClr = parityErrInjRegVal & ~(0x1 << (errInjOffset + 9));
        UART_printf("\n Error injected.");
        *parityErrInjRegAddr = errorToInject;
        UART_printf("\n Reading from the register.");
        triggerRegReadInterrupt = *parityErrInjRegAddr;
        (void) triggerRegReadInterrupt;
        Ddr17App_WaitForInterrupt(&gDdrPhyPiTestPassedCount, &gDdrPhyPiTestCount);
        UART_printf("\n Writing to the register(clearing the injection).");
        *parityErrInjRegAddr = errorToInjectClr;
        Ddr17App_WaitForInterrupt(&gDdrPhyPiTestPassedCount, &gDdrPhyPiTestCount);
        UART_printf("\n Interrupt should have been invoked two times!");
    }
    else
    {
        /* Call generic error injection. */
        Ddr17App_PhyInjectError((uint32_t*) parityErrInjRegAddr, errInjOffset);
        Ddr17App_PhyInjectError((uint32_t*) parityErrInjRegAddr, errInjOffset + 4);
        Ddr17App_PhyInjectError((uint32_t*) parityErrInjRegAddr, errInjOffset + 8);
        Ddr17App_PhyInjectError((uint32_t*) parityErrInjRegAddr, errInjOffset + 9);
    }
}

/**
 *  \brief Test runner for CTL register parity tests.
 */
static void Ddr17App_testDdrCtlRegsParityProtection()
{
    int32_t status = CSL_PASS;

    for (uint32_t ddrModuleIndex = 0U; ddrModuleIndex < DDR_MODULE_COUNT; ddrModuleIndex++)
    {

        /* The base address is needed in the interrupt handler to verify the DDR interrupt details. */
        gDdrBaseAddress = gDdrModulesBaseAddr[ddrModuleIndex];

        /* Setup ESM and register interrupt handler. */
        status = Ddr17App_EsmSetup(gDdrCtlModulesInterruptIds[ddrModuleIndex]);
        if (CSL_PASS != status)
        {
            UART_printf("\n ESM setup failed.");
        }

        if (CSL_PASS == status)
        {
            /* Enable parity protection for the DDR module. */
            IpFma_Ddr17_EnableCtlParityProtection(gDdrModulesBaseAddr[ddrModuleIndex]);

            /* Run the tests. */
            /* Enables regport address/command parity error injection from the regport to the param block. */
            Ddr17App_CtlParityTest(gDdrModulesBaseAddr[ddrModuleIndex], ADDRESS_PARITY_ERROR);

            /* Enables regport write mask data parity error injection from the regport to the param block. */
            Ddr17App_CtlParityTest(gDdrModulesBaseAddr[ddrModuleIndex], WRITE_DATA_MASK_PARITY_ERROR);

            /* Enables parity checking on the param registers. */
            Ddr17App_CtlParityTest(gDdrModulesBaseAddr[ddrModuleIndex], PARAM_REGISTER_PARITY_ERROR);

            /* Enables regport read data parity checking from the param block to the regport. */
            Ddr17App_CtlParityTest(gDdrModulesBaseAddr[ddrModuleIndex], READ_DATA_PARITY_ERROR);

            /* Enables regport write data parity error injection from the regport to the param block. */
            Ddr17App_CtlParityTest(gDdrModulesBaseAddr[ddrModuleIndex], WRITE_DATA_PARITY_ERROR);
        }
    }
}

/**
 *  \brief Test runner for PHY and PI register parity tests.
 *
 *  PHY and PI registers use the same global error and parity error info registers,
 *  so they are grouped together in one test runner.
 */
static void Ddr17App_testDdrPhyPiRegsParityProtection()
{
    int32_t status = CSL_PASS;

    for (uint32_t ddrModuleIndex = 0U; ddrModuleIndex < DDR_MODULE_COUNT; ddrModuleIndex++)
    {
        /* Mask the PHY timeout bit. */
        uint32_t* globalErrorInfoAddr = (uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_GLOBAL_ERROR_INFO_REG_OFFSET);
        *globalErrorInfoAddr |= DDRSS_PHY_GLOBAL_ERROR_INFO_MASK_TIMEOUT_BIT;

        /* Enable parity error injection. */
        /* When enabled, a register write will never update any registers but instead inject a parity error to the register. */
        uint32_t* phyParityConfigRegAddr = (uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_1370_OFFSET);
        uint32_t phyParityConfigRegVal = *phyParityConfigRegAddr;
        *phyParityConfigRegAddr = phyParityConfigRegVal | DDRSS_PHY_REG_1370_PARITY_ERROR_INJECTION_EN;

        /* Setup ESM and register interrupt handler. */
        status = Ddr17App_EsmSetup(gDdrPhyModulesInterruptIds[ddrModuleIndex]);
        if (CSL_PASS != status)
        {
            UART_printf("\n ESM setup failed.");
        }

        /* The base address is needed in the interrupt handler to verify the DDR interrupt details. */
        gDdrBaseAddress = gDdrModulesBaseAddr[ddrModuleIndex];

        if (CSL_PASS == status)
        {
            /* Inject parity error to register interface signals for address slice 0. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_1060_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT2,
                                    DDRSS_PHY_REG_1060_PARITY_ERROR_INJECTION_OFFSET);
            /* Inject parity error to register interface signals for ac slice. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_1349_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT1,
                                    DDRSS_PHY_REG_1349_PARITY_ERROR_INJECTION_OFFSET);
            /* Injects parity error to register interface signals in param_split. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_1370_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT1,
                                    DDRSS_PHY_REG_1370_PARITY_ERROR_INJECTION_OFFSET);

            /* Inject parity error to register interface signals for slice 0. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_81_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT3,
                                    DDRSS_PHY_REG_81_PARITY_ERROR_INJECTION_OFFSET);
            /* Inject parity error to register interface signals for slice 1. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_337_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT4,
                                    DDRSS_PHY_REG_337_PARITY_ERROR_INJECTION_OFFSET);
            /* Inject parity error to register interface signals for slice 2. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_593_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT5,
                                    DDRSS_PHY_REG_593_PARITY_ERROR_INJECTION_OFFSET);
            /* Inject parity error to register interface signals for slice 3. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PHY_REG_849_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT6,
                                    DDRSS_PHY_REG_849_PARITY_ERROR_INJECTION_OFFSET);

            /* PI uses the same PHY global error and parity error info registers so it is grouped together with PHY. */
            Ddr17App_PhyParityTest((uint32_t*) (gDdrModulesBaseAddr[ddrModuleIndex] + DDRSS_PI_REG_299_OFFSET),
                                    DDR_PHY_PARITY_ERR_BIT1,
                                    DDRSS_PI_REG_299_PARITY_ERROR_INJECTION_OFFSET);
        }

        UART_printf("\n Disable error injection!");
        /* Disable parity error injection. */
        *phyParityConfigRegAddr = phyParityConfigRegVal & ~DDRSS_PHY_REG_1370_PARITY_ERROR_INJECTION_EN;
    }
}

/**
 * \brief Test entry point.
 */
int main(void)
{
    int32_t status = BOARD_FAIL;
    status = Ddr17App_Init();
    if (status != BOARD_SOK)
    {
        UART_printf("\n Board not initialized successfully! \n");
    }
    else
    {
        UART_printf("\n Board init complete \n");
    }

    Ddr17App_testDdrCtlRegsParityProtection();
    Ddr17App_testDdrPhyPiRegsParityProtection();

    if ((gDdrCtlTestCount == gDdrCtlTestPassedCount) && (gDdrPhyPiTestCount == gDdrPhyPiTestPassedCount))
    {
        UART_printf("\n All tests have passed!");
    }
    else
    {
        if (gDdrCtlTestCount != gDdrCtlTestPassedCount)
        {
            UART_printf("\n Some DDR CTL controller tests failed!");
        }
        if (gDdrPhyPiTestCount != gDdrPhyPiTestPassedCount)
        {
            UART_printf("\n Some DDR PHY and PI tests failed!");
        }
    }

    return 0;
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
static int32_t Ddr17App_Init(void)
{
    int32_t status = BOARD_FAIL;
    Board_initCfg boardCfg;

    boardCfg = BOARD_INIT_MODULE_CLOCK | BOARD_INIT_UART_STDIO | BOARD_INIT_PINMUX_CONFIG;
    status = Board_init(boardCfg);

    return status;
}
