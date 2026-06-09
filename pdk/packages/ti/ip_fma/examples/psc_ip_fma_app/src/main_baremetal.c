/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \brief Periodic Software Readback Example reads static configuration
 *         registers of Power and Sleep Controller (PSC) and
 *         reports match or mismatch between expected register values
 *         and actual (read) ones.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include "ip_fma_psc.h"
#include <ti/drv/uart/UART_stdio.h>
#include "ti/board/board.h"
#include <string.h>
/* CSL PSC include */
#include <ti/csl/csl_psc.h>
#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/csl_soc_psc.h>
#include <ti/csl/soc/j721s2/src/cslr_soc_baseaddress.h>
#endif
#if defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/csl_soc_psc.h>
#include <ti/csl/soc/j784s4/src/cslr_soc_baseaddress.h>
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/**< Number of periodic readbacks */
#define PERIODIC_CHECK_NUM                      ((uint8_t)1U)
/**< Delay time in ms */
#define DELAY_MS                                ((uint8_t)1000U)

/**< Timeout counter value for PSC state transitions */
#define TIMEOUT_COUNT 1000U

/**< PSC check type enumeration. */
typedef enum {
    PSC_CHECK_TYPE_POWER,  /**< Check power domain transition */
    PSC_CHECK_TYPE_MODULE_RESET /**< Check module local reset */
} PscCheckType;

typedef IpFma_Status (*PscApp_GetRegsFunc)(uint32_t base, void *regs);
typedef IpFma_Status (*PscApp_CompareRegsFunc)(const void *expected, const void *actual);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/** \brief Pointer to PSC0 register base. */
static CSL_PscRegs *gPscRegs = (CSL_PscRegs *)CSL_PSC0_BASE;

/** \brief Pointer to WKUP_PSC0 register base. */
static CSL_PscRegs *gPscMcuRegs = (CSL_PscRegs *)CSL_WKUP_PSC0_BASE;

#if defined(SOC_J784S4)
/** \brief Pointer to Bolt-On PSC register base. */
static CSL_PscRegs *gPscBoltonRegs = (CSL_PscRegs *)CSL_AM_BOLT_PSC_WRAP0_VBUS_BASE;

/** \brief Expected values for Bolton PSC registers. */
static IpFma_PscBoltonRegs gPscBoltonRegsExpValues;
#endif

/** \brief Expected values for PSC main registers. */
static IpFma_PscRegs gPscRegsExpValues;

/** \brief Expected values for PSC mcu registers. */
static IpFma_PscMcuRegs gPscMcuRegsExpValues;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 * \brief Initialize the PSC application.
 *
 * This function initializes the PSC application by configuring the board
 * for UART standard I/O and calling the Board_init function with the
 * appropriate configuration.
 *
 * \note The function relies on BOARD_INIT_UART_STDIO for board initialization.
 *
 * \return  status      BOARD_SOK on successful, else otherwise.
 */
static int32_t PscApp_Init();

/**
 * \brief Configure and enable the VPAC power domain and its associated module.
 *
 * This function checks the current state of the VPAC power domain. If it is OFF,
 * it enables the power domain, sets the module next state to ENABLE, starts the
 * power domain state transition, and waits for the transition to complete or 
 * until a timeout occurs.
 * After the transition, the function reports the final status via UART.
 *
 * \return IPFMA_OK on success, otherwise IPFMA_E_MISMATCH.
 */
static IpFma_Status PscApp_PowerCfg(void);

/**
 * \brief Perform a reset sequence for the VPAC module.
 *
 * This function executes the full reset procedure for the VPAC module using
 * PSC (Power and Sleep Controller) mechanisms. It enables reset isolation,
 * asserts and deasserts the module's local reset, and waits for each stage
 * to complete, with a timeout for each operation. If a timeout occurs during
 * any stage, the function reports it via UART and exits early.
 *
 * The function performs the following steps:
 *   - Enables module reset isolation.
 *   - Asserts the module's local reset and waits for completion (with timeout).
 *   - Deasserts the module's local reset and waits for completion (with timeout).
 *   - Disables reset isolation.
 *   - Reports the status of each stage and final reset completion via UART.
 *
 * \note This function blocks until all reset stages are completed or a timeout occurs.
 *
 * \return IPFMA_OK on success, otherwise IPFMA_E_MISMATCH.
 */
static IpFma_Status PscApp_ResetVPAC(void);

/**
 * \brief Perform a reset sequence for the MCU test module.
 *
 * This function executes the full reset procedure for the MCU test module using
 * PSC (Power and Sleep Controller) mechanisms. It enables reset isolation,
 * asserts and deasserts the module's local reset, and waits for each stage
 * to complete, with a timeout for each operation. If a timeout occurs during
 * any stage, the function reports it via UART and exits early.
 *
 * The function performs the following steps:
 *   - Enables module reset isolation.
 *   - Asserts the module's local reset and waits for completion (with timeout).
 *   - Deasserts the module's local reset and waits for completion (with timeout).
 *   - Disables reset isolation.
 *   - Reports the status of each stage and final reset completion via UART.
 *
 * \note This function blocks until all reset stages are completed or a timeout occurs.
 *
 * \return IPFMA_OK on success, otherwise IPFMA_E_MISMATCH.
 */
static IpFma_Status PscApp_ResetMCUTestModule(void);

#if defined(SOC_J784S4)
/**
 * \brief Perform a reset sequence for the DRU0 (Bolt-on PD) module.
 *
 * This function executes the full reset procedure for the DRU0 module using
 * PSC (Power and Sleep Controller) mechanisms. It enables reset isolation,
 * asserts and deasserts the module's local reset, and waits for each stage
 * to complete, with a timeout for each operation. If a timeout occurs during
 * any stage, the function reports it via UART and exits early.
 *
 * The function performs the following steps:
 *   - Enables module reset isolation.
 *   - Asserts the module's local reset and waits for completion (with timeout).
 *   - Deasserts the module's local reset and waits for completion (with timeout).
 *   - Disables reset isolation.
 *   - Reports the status of each stage and final reset completion via UART.
 *
 * \note This function blocks until all reset stages are completed or a timeout occurs.
 *
 * \return IPFMA_OK on success, otherwise IPFMA_E_MISMATCH.
 */
static IpFma_Status PscApp_ResetDRU0Module(void);
#endif

/**
 * \brief Waits for a PSC state transition or module reset to complete.
 *
 * This function polls the PSC (Power and Sleep Controller) to check if a 
 * state transition for the specified power domain or module reset operation 
 * has completed. It uses a software timeout to avoid blocking indefinitely 
 * if the transition or reset does not complete.
 * 
 * \param pPscRegs  Pointer to PSC register structure.
 *
 * \param domain    The PSC power domain or module ID (e.g., CSL_MAIN_PD_VPAC)
 *                  for which the state transition or reset is being monitored.
 * \param checkType The type of check to perform:
 *                  - PSC_CHECK_TYPE_POWER: Wait for a power domain transition.
 *                  - PSC_CHECK_TYPE_MODULE_RESET: Wait for a module local reset.
 *
 * \retval 0  State transition or reset completed successfully.
 * \retval -1 Timeout occurred before the transition or reset completed.
 */
static int32_t PscApp_WaitPsc(CSL_PscRegs *pPscRegs, uint32_t domain, PscCheckType checkType);

/**
 * \brief   Local Delay function
 *
 * \param   wait_in_ms Delay time in miliseconds
 * 
 * \retval  None.
 */
static void PscApp_Delay(uint32_t wait_in_ms);

/**
 * \brief Periodically checks PSC register values.
 *
 * This function performs a periodic software readback of PSC registers.
 * It reads the actual register values using the provided GetRegs function
 * and compares them against the expected register values using the provided
 * CompareRegs function.
 *
 * The function is generic and can be used for different PSC register types
 * (e.g. MAIN PSC, WKUP/MCU PSC) by supplying appropriate callback functions
 * and register structures.
 *
 * \param name          Name of the PSC instance, used for logging.
 * \param base          Base address of the PSC register block.
 * \param actualRegs    Input structure with actual values.
 * \param expectedRegs  Input structure with expected values.
 * \param getFunc       Callback function used to read PSC register values. 
 * \param compareFunc   Callback function used to compare expected and actual.
 *
 * \return              IPFMA_OK on success, otherwise IPFMA_E_MISMATCH.
 */
static IpFma_Status PscApp_registerCheck(const char *name, uint32_t base,
                                         void *actualRegs, const void *expectedRegs,
                                         PscApp_GetRegsFunc getFunc,
                                         PscApp_CompareRegsFunc compareFunc);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static IpFma_Status PscApp_PowerCfg(void)
{
    IpFma_Status status = IPFMA_E_MISMATCH;
    IpFma_PscRegs pscRegsActualValues;
    /* VPAC power domain is turned OFF by default. */
    /* Turn on the VPAC power domain */
    if (CSL_PSC_getPowerDomainState(gPscRegs, CSL_MAIN_PD_VPAC) != PSC_PDSTATE_ON)
    {
        /* Enable the domain */
        CSL_PSC_enablePowerDomain (gPscRegs, CSL_MAIN_PD_VPAC);
        /* Enable MDCTL */
        CSL_PSC_setModuleNextState (gPscRegs, CSL_MAIN_LPSC_VPAC, PSC_MODSTATE_ENABLE);
        /* Apply the domain */     
        CSL_PSC_startStateTransition (gPscRegs, CSL_MAIN_PD_VPAC);

        status = IpFma_Psc_GetRegs(CSL_PSC0_BASE, &gPscRegsExpValues);
        //memset(&gPscRegsExpValues, 0, sizeof(gPscRegsExpValues));
        UART_printf("Loading expected PSC register values (Power on phase)...\n");

        /* Wait until the transition process is completed */ 
        if (PscApp_WaitPsc(gPscRegs, CSL_MAIN_PD_VPAC, PSC_CHECK_TYPE_POWER) == 0)
        {
            if (IPFMA_OK == status)
            {
                status = PscApp_registerCheck("PSC_main",
                                               CSL_PSC0_BASE,
                                               &pscRegsActualValues,
                                               &gPscRegsExpValues,
                                               (PscApp_GetRegsFunc)IpFma_Psc_GetRegs,
                                               (PscApp_CompareRegsFunc)IpFma_Psc_CompareRegs);
            }
            /* Return PSC status */
            if ((CSL_PSC_getPowerDomainState(gPscRegs, CSL_MAIN_PD_VPAC) == PSC_PDSTATE_ON) &&
                (CSL_PSC_getModuleState (gPscRegs, CSL_MAIN_LPSC_VPAC) == PSC_MODSTATE_ENABLE))
            {
                UART_printf("VPAC Power ON.\r\n");
            }
            else
            {
                UART_printf("VPAC Power on failed.\r\n");
            }
        }
        else
        {
            UART_printf("VPAC Power on TIMEOUT!\r\n");
        }
    }
    else
    {
        UART_printf ("VPAC power domain is already enabled.\r\n");
    }
    return status;
}

static IpFma_Status PscApp_ResetVPAC(void)
{
    UART_printf("Starting VPAC module reset...\r\n");

    IpFma_Status status = IPFMA_E_MISMATCH;
    IpFma_Status retVal = IPFMA_OK;
    IpFma_PscRegs pscRegsActualValues;

    /* Enable reset isolation */
    CSL_PSC_enableModuleResetIsolation(gPscRegs, CSL_MAIN_LPSC_VPAC);
    
    /* Assert module local reset */
    CSL_PSC_setModuleLocalReset(gPscRegs, CSL_MAIN_LPSC_VPAC, PSC_MDLRST_ASSERTED);

    status = IpFma_Psc_GetRegs(CSL_PSC0_BASE, &gPscRegsExpValues);
    //memset(&gPscRegsExpValues, 0, sizeof(gPscRegsExpValues));
    UART_printf("Loading expected PSC register values (ASSERT phase)...\n");

    /* Wait until local reset is done */
    if (PscApp_WaitPsc(gPscRegs, CSL_MAIN_LPSC_VPAC, PSC_CHECK_TYPE_MODULE_RESET) == 0)
    {
        if (IPFMA_OK == status)
        {
            status = PscApp_registerCheck("PSC_main",
                                          CSL_PSC0_BASE,
                                          &pscRegsActualValues,
                                          &gPscRegsExpValues,
                                          (PscApp_GetRegsFunc)IpFma_Psc_GetRegs,
                                          (PscApp_CompareRegsFunc)IpFma_Psc_CompareRegs);
        }

        if (IPFMA_OK != status)
        {
            retVal = IPFMA_E_MISMATCH;
        }

        UART_printf("VPAC module local reset ASSERT complete.\r\n");
    }
    else
    {
        UART_printf("VPAC module local reset ASSERT TIMEOUT!\r\n");
        CSL_PSC_disableModuleResetIsolation(gPscRegs, CSL_MAIN_LPSC_VPAC);
        status = IPFMA_E_MISMATCH;
        retVal = IPFMA_E_MISMATCH;
    }
    
    /* Deassert module local reset */
    CSL_PSC_setModuleLocalReset(gPscRegs, CSL_MAIN_LPSC_VPAC, PSC_MDLRST_DEASSERTED);

    status = IpFma_Psc_GetRegs(CSL_PSC0_BASE, &gPscRegsExpValues);
    //memset(&gPscRegsExpValues, 0, sizeof(gPscRegsExpValues));
    UART_printf("Loading expected PSC register values (DEASSERT phase)...\n");
   
    /* Wait until reset deassertion completes */    
    if (PscApp_WaitPsc(gPscRegs, CSL_MAIN_LPSC_VPAC, PSC_CHECK_TYPE_MODULE_RESET) == 0)
    {
        if (IPFMA_OK == status)
        {
            status = PscApp_registerCheck("PSC_main",
                                          CSL_PSC0_BASE,
                                          &pscRegsActualValues,
                                          &gPscRegsExpValues,
                                          (PscApp_GetRegsFunc)IpFma_Psc_GetRegs,
                                          (PscApp_CompareRegsFunc)IpFma_Psc_CompareRegs);
        }

        if (IPFMA_OK != status)
        {
            retVal = IPFMA_E_MISMATCH;
        }

        UART_printf("VPAC module local reset DEASSERT complete.\r\n");
    }
    else
    {
        UART_printf("VPAC module local reset DEASSERT TIMEOUT!\r\n");
        CSL_PSC_disableModuleResetIsolation(gPscRegs, CSL_MAIN_LPSC_VPAC);
        status = IPFMA_E_MISMATCH;
        retVal = IPFMA_E_MISMATCH;
    }

    /* Disable reset isolation */
    CSL_PSC_disableModuleResetIsolation(gPscRegs, CSL_MAIN_LPSC_VPAC);

    UART_printf("VPAC module reset successfully completed.\r\n");

    return retVal;
}

static IpFma_Status PscApp_ResetMCUTestModule(void)
{
    UART_printf("Starting MCU test module reset...\r\n");

    IpFma_Status status = IPFMA_E_MISMATCH;
    IpFma_Status retVal = IPFMA_OK;
    IpFma_PscMcuRegs pscMcuRegsActualValues;

    /* Enable reset isolation */
    CSL_PSC_enableModuleResetIsolation(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST);
    
    /* Assert module local reset */
    CSL_PSC_setModuleLocalReset(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST, PSC_MDLRST_ASSERTED);

    status = IpFma_Psc_GetMcuRegs(CSL_WKUP_PSC0_BASE, &gPscMcuRegsExpValues);
    //memset(&gPscMcuRegsExpValues, 0, sizeof(gPscMcuRegsExpValues));
    UART_printf("Loading expected PSC register values (ASSERT phase)...\n");

    /* Wait until local reset is done */
    if (PscApp_WaitPsc(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST, PSC_CHECK_TYPE_MODULE_RESET) == 0)
    {
        if (IPFMA_OK == status)
        {
            status = PscApp_registerCheck("PSC_MCU",
                                          CSL_WKUP_PSC0_BASE,
                                          &pscMcuRegsActualValues,
                                          &gPscMcuRegsExpValues,
                                          (PscApp_GetRegsFunc)IpFma_Psc_GetMcuRegs,
                                          (PscApp_CompareRegsFunc)IpFma_Psc_CompareMcuRegs);
        }

        if (IPFMA_OK != status)
        {
            retVal = IPFMA_E_MISMATCH;
        }

        UART_printf("MCU test module local reset ASSERT complete.\r\n");
    }
    else
    {
        UART_printf("MCU test module local reset ASSERT TIMEOUT!\r\n");
        CSL_PSC_disableModuleResetIsolation(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST);
        status = IPFMA_E_MISMATCH;
        retVal = IPFMA_E_MISMATCH;
    }
    
    /* Deassert module local reset */
    CSL_PSC_setModuleLocalReset(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST, PSC_MDLRST_DEASSERTED);

    status = IpFma_Psc_GetMcuRegs(CSL_WKUP_PSC0_BASE, &gPscMcuRegsExpValues);
    //memset(&gPscMcuRegsExpValues, 0, sizeof(gPscMcuRegsExpValues));
    UART_printf("Loading expected PSC register values (DEASSERT phase)...\n");
   
    /* Wait until reset deassertion completes */    
    if (PscApp_WaitPsc(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST, PSC_CHECK_TYPE_MODULE_RESET) == 0)
    {
        if (IPFMA_OK == status)
        {
            status = PscApp_registerCheck("PSC_MCU",
                                          CSL_WKUP_PSC0_BASE,
                                          &pscMcuRegsActualValues,
                                          &gPscMcuRegsExpValues,
                                          (PscApp_GetRegsFunc)IpFma_Psc_GetMcuRegs,
                                          (PscApp_CompareRegsFunc)IpFma_Psc_CompareMcuRegs);
        }

        if (IPFMA_OK != status)
        {
            retVal = IPFMA_E_MISMATCH;
        }

        UART_printf("MCU test module local reset DEASSERT complete.\r\n");
    }
    else
    {
        UART_printf("MCU test module local reset DEASSERT TIMEOUT!\r\n");
        CSL_PSC_disableModuleResetIsolation(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST);
        status = IPFMA_E_MISMATCH;
        retVal = IPFMA_E_MISMATCH;
    }

    /* Disable reset isolation */
    CSL_PSC_disableModuleResetIsolation(gPscMcuRegs, CSL_WKUP_LPSC_MCU_TEST);

    UART_printf("MCU test module reset successfully completed.\r\n");

    return retVal;
}

#if defined(SOC_J784S4)
static IpFma_Status PscApp_ResetDRU0Module(void)
{
    UART_printf("Starting DRU0 module reset...\r\n");

    IpFma_Status status = IPFMA_E_MISMATCH;
    IpFma_Status retVal = IPFMA_OK;
    IpFma_PscBoltonRegs pscBoltonRegsActualValues;

    /* Enable reset isolation */
    CSL_PSC_enableModuleResetIsolation(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0);
    
    /* Assert module local reset */
    CSL_PSC_setModuleLocalReset(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0, PSC_MDLRST_ASSERTED);

    status = IpFma_Psc_GetBoltonRegs(CSL_AM_BOLT_PSC_WRAP0_VBUS_BASE, &gPscBoltonRegsExpValues);
    //memset(&gPscBoltonRegsExpValues, 0, sizeof(gPscBoltonRegsExpValues));
    UART_printf("Loading expected PSC register values (ASSERT phase)...\n");

    /* Wait until local reset is done */
    if (PscApp_WaitPsc(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0, PSC_CHECK_TYPE_MODULE_RESET) == 0)
    {
        if (IPFMA_OK == status)
        {
            status = PscApp_registerCheck("PSC_Bolt-on",
                                          CSL_AM_BOLT_PSC_WRAP0_VBUS_BASE,
                                          &pscBoltonRegsActualValues,
                                          &gPscBoltonRegsExpValues,
                                          (PscApp_GetRegsFunc)IpFma_Psc_GetBoltonRegs,
                                          (PscApp_CompareRegsFunc)IpFma_Psc_CompareBoltonRegs);
        }

        if (IPFMA_OK != status)
        {
            retVal = IPFMA_E_MISMATCH;
        }

        UART_printf("DRU0 module local reset ASSERT complete.\r\n");
    }
    else
    {
        UART_printf("DRU0 module local reset ASSERT TIMEOUT!\r\n");
        CSL_PSC_disableModuleResetIsolation(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0);
        status = IPFMA_E_MISMATCH;
        retVal = IPFMA_E_MISMATCH;
    }
    
    /* Deassert module local reset */
    CSL_PSC_setModuleLocalReset(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0, PSC_MDLRST_DEASSERTED);

    status = IpFma_Psc_GetBoltonRegs(CSL_AM_BOLT_PSC_WRAP0_VBUS_BASE, &gPscBoltonRegsExpValues);
    //memset(&gPscBoltonRegsExpValues, 0, sizeof(gPscBoltonRegsExpValues));
    UART_printf("Loading expected PSC register values (DEASSERT phase)...\n");
   
    /* Wait until reset deassertion completes */    
    if (PscApp_WaitPsc(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0, PSC_CHECK_TYPE_MODULE_RESET) == 0)
    {
        if (IPFMA_OK == status)
        {
            status = PscApp_registerCheck("PSC_Bolt-on",
                                          CSL_AM_BOLT_PSC_WRAP0_VBUS_BASE,
                                          &pscBoltonRegsActualValues,
                                          &gPscBoltonRegsExpValues,
                                          (PscApp_GetRegsFunc)IpFma_Psc_GetBoltonRegs,
                                          (PscApp_CompareRegsFunc)IpFma_Psc_CompareBoltonRegs);
        }

        if (IPFMA_OK != status)
        {
            retVal = IPFMA_E_MISMATCH;
        }

        UART_printf("DRU0 module local reset DEASSERT complete.\r\n");
    }
    else
    {
        UART_printf("DRU0 module local reset DEASSERT TIMEOUT!\r\n");
        CSL_PSC_disableModuleResetIsolation(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0);
        status = IPFMA_E_MISMATCH;
        retVal = IPFMA_E_MISMATCH;
    }

    /* Disable reset isolation */
    CSL_PSC_disableModuleResetIsolation(gPscBoltonRegs, CSL_MAIN_LPSC_DRU_0);

    UART_printf("DRU0 module reset successfully completed.\r\n");

    return retVal;
}
#endif

static IpFma_Status PscApp_registerCheck(const char *name, uint32_t base,
                                         void *actualRegs, const void *expectedRegs,
                                         PscApp_GetRegsFunc getFunc,
                                         PscApp_CompareRegsFunc compareFunc)
{
    UART_printf("%s register check starts...\r\n", name);

    uint8_t readback_iteration = 0;
    IpFma_Status status = IPFMA_OK;

    while (readback_iteration < PERIODIC_CHECK_NUM)
    {
        status = getFunc(base, actualRegs);
        if (IPFMA_OK != status)
        {
            UART_printf("%s GetRegs FAILED\r\n", name);
            break;
        }

        UART_printf("Comparing expected-actual %s registers...", name);
        status = compareFunc(expectedRegs, actualRegs);

        if (IPFMA_OK == status)
        {
            UART_printf("values MATCH!\r\n");
        }
        else
        {
            UART_printf("values MISMATCH!\r\n");
        }

        readback_iteration++;
        PscApp_Delay(DELAY_MS);
    }

    return status;
}

static int32_t PscApp_WaitPsc(CSL_PscRegs *pPscRegs, uint32_t domain, PscCheckType checkType)
{
    uint32_t count = TIMEOUT_COUNT;

    switch(checkType)
    {
        case PSC_CHECK_TYPE_POWER:
            while (!CSL_PSC_isStateTransitionDone(pPscRegs, domain))
            {
                if (count-- == 0)
                    return -1;
            }
            break;

        case PSC_CHECK_TYPE_MODULE_RESET:
            while (!CSL_PSC_isModuleLocalResetDone(pPscRegs, domain))
            {
                if (count-- == 0)
                    return -1;
            }
            break;

        default:
            return -1;
    }

    return 0;
}

/*
 * Application main
 */
int32_t main(void)
{
    int32_t retVal = -1;
    IpFma_Status statusPwr = IPFMA_E_MISMATCH;
    IpFma_Status statusReset = IPFMA_E_MISMATCH;
    IpFma_Status statusMcuReset = IPFMA_E_MISMATCH;
#if defined(SOC_J784S4)
    IpFma_Status statusDru0Reset = IPFMA_E_MISMATCH;
#endif

    retVal = PscApp_Init();

    if (BOARD_SOK == retVal)
    {
        UART_printf("Application START!\n");
        statusPwr = PscApp_PowerCfg();
        statusReset = PscApp_ResetVPAC();
        statusMcuReset = PscApp_ResetMCUTestModule();
#if defined(SOC_J784S4)
        statusDru0Reset = PscApp_ResetDRU0Module();
#endif

#if defined(SOC_J784S4)
        if ((IPFMA_OK == statusPwr) &&
            (IPFMA_OK == statusReset) &&
            (IPFMA_OK == statusMcuReset) &&
            (IPFMA_OK == statusDru0Reset))
#else
        if ((IPFMA_OK == statusPwr) &&
            (IPFMA_OK == statusReset) &&
            (IPFMA_OK == statusMcuReset))
#endif
        {
            UART_printf("Register check done ...\r\n");
            UART_printf("All tests have passed!!\n");

        }
        else
        {
            UART_printf("Register check fail ...\r\n");
            UART_printf("Some tests have failed!!\n");
        }
    }   
    return retVal;
}

static int32_t PscApp_Init()
{
    int32_t status = -1;
    Board_initCfg boardCfg;

    boardCfg = BOARD_INIT_UART_STDIO;
    status   = Board_init(boardCfg);

    return status;
}

static void PscApp_Delay(uint32_t wait_in_ms)
{
    while (wait_in_ms--) {
        asm("   NOP");
    }
}
