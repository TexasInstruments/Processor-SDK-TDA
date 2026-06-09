/*
 *  Copyright (c) 2026 Texas Instruments Incorporated
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
 *  \brief Test of DDR parity checker on controller and PHY configuration registers via ECC aggregators
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/board/board.h>

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

#include <ti/csl/src/ip/rat/V0/csl_rat.h>
#include <ti/csl/src/ip/ecc_aggr/V1/csl_ecc_aggr.h>
#include <ti/csl/src/ip/ecc_aggr/V1/cslr_ecc_aggr.h>

#include <ti/csl/soc.h>
#if defined(SOC_J721S2)
#include <ti/csl/soc/j721s2/src/cslr_intr_esm0.h>
#include <ti/board/src/j721s2_evm/include/board_ddr.h>
#elif defined(SOC_J784S4)
#include <ti/csl/soc/j784s4/src/cslr_intr_esm0.h>
#include <ti/board/src/j784s4_evm/include/board_ddr.h>
#endif

#include <esm.h>
#include <ti/ip_fma/inc/ip_fma_common.h>


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define DDR_PARITY_ECC_AGGR_RAT_REGION_INDEX        (0)

#define DDRT7_INTERRUPT_WAIT_DELAY                  (10U)
#define DDRT7_INTERRUPT_WAIT_TIMEOUT                (3000u)

#define DDRT7_ECC_AGGR_MMR_SIZE                     (0x400ULL)
#define DDRT7_ECC_AGGR_MAX_RAM_IDS                  (6U)
#define DDRT7_ECC_AGGR_NUM_CFG_CTL                  (2U)

/**
 *  BOARD J784S4 contains 4 DDR modules, board J721S2 contains 2 DDR modules.
 *  Each DDR module contains 2 ECC aggregators for testing(CTL and CFG).
 */
#if defined(SOC_J721S2)
#define DDRT7_J721S2_DDR_COUNT                      (2U)
#define DDRT7_MAX_TESTCASES                         (DDRT7_ECC_AGGR_MAX_RAM_IDS * DDRT7_J721S2_DDR_COUNT * DDRT7_ECC_AGGR_NUM_CFG_CTL)
#endif
#if defined(SOC_J784S4)
#define DDRT7_J784S4_DDR_COUNT                      (4U)
#define DDRT7_MAX_TESTCASES                         (DDRT7_ECC_AGGR_MAX_RAM_IDS * DDRT7_J784S4_DDR_COUNT * DDRT7_ECC_AGGR_NUM_CFG_CTL)
#endif

/**
 *  \brief A simple structure that holds ECC aggregator data for each tested aggregator RAM ID used in the test cases.
 */
typedef struct {
    uint64_t baseAddress;
    uint32_t parityInterruptId;
    uint32_t ramId;
    uint32_t groupId;
} DdrT7App_EccAggrData;


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* This variable is set by the interrupt handler to indicate that the interrupt has occurred. */
volatile Bool gDdrParityInterruptInvoked = FALSE;

/* Global variables used by the interrupt handler to verify the correct interrupt source. */
uint32_t gRamId;
uint32_t gParityInterruptId;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t DdrT7App_Init(void);

static void DdrT7App_TestDdrEccParityCheckersRunner(void);
static int32_t DdrT7App_TestDdrEccParityCheckers(DdrT7App_EccAggrData eccAggr);
static void DdrT7App_PopulateEccAggrData(DdrT7App_EccAggrData* eccAggrData);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  \brief Simple utility function that populates test case data.
 *
 *  \param      eccAggrData         Array that will be populated with test case data
 *
 */
static void DdrT7App_PopulateEccAggrData(DdrT7App_EccAggrData* eccAggrData)
{
    /* BOARD J784S4 contains 4 DDR modules, board J721S2 contains 2 DDR modules. */

    /* Group ID is randomly picked from the list of RAM IDs that have available PARITY checkers. */
    uint32_t groupIds[] = {
        /* DDR 0 */
        4, 0, 0, 11,  3,  3,      /* CTL */
        4, 0, 1,  6, 14, 12,      /* CFG */
        /* DDR 1 */
        4,  18, 248, 11, 8, 3,    /* CTL */
        15, 19,  22, 30, 1, 4,    /* CFG */
#if defined(SOC_J784S4)
        /* DDR 2 */
        11, 33, 13, 10,  3,  3,   /* CTL */
        17, 16, 22, 12, 15, 12,   /* CFG */
        /* DDR 3 */
        5, 12, 8, 11,  2, 3,      /* CTL */
        4,  5, 8,  6, 14, 4,      /* CFG */
#endif
    };

    uint32_t parityInterruptIds[] = {
        CSLR_ESM0_ESM_LVL_EVENT_DDR0_DDRSS_CTL_ECC_AGGR_UNCORR_ERR_LVL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR0_DDRSS_CFG_ECC_AGGR_UNCORR_ERR_LVL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR1_DDRSS_CTL_ECC_AGGR_UNCORR_ERR_LVL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR1_DDRSS_CFG_ECC_AGGR_UNCORR_ERR_LVL_0,
#if defined(SOC_J784S4)
        CSLR_ESM0_ESM_LVL_EVENT_DDR2_DDRSS_CTL_ECC_AGGR_UNCORR_ERR_LVL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR2_DDRSS_CFG_ECC_AGGR_UNCORR_ERR_LVL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR3_DDRSS_CTL_ECC_AGGR_UNCORR_ERR_LVL_0,
        CSLR_ESM0_ESM_LVL_EVENT_DDR3_DDRSS_CFG_ECC_AGGR_UNCORR_ERR_LVL_0,
#endif
    };

    uint64_t baseAddresses[] = {
#if defined(SOC_J721S2)
        CSL_COMPUTE_CLUSTER0_DDR0_0_ECC_AGGR_CTL_BASE,
        CSL_COMPUTE_CLUSTER0_DDR0_0_ECC_AGGR_CFG_BASE,
        CSL_COMPUTE_CLUSTER0_DDR1_1_ECC_AGGR_CTL_BASE,
        CSL_COMPUTE_CLUSTER0_DDR1_1_ECC_AGGR_CFG_BASE,
#endif
#if defined(SOC_J784S4)
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS0_ECC_AGGR_CTL_BASE,
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS0_ECC_AGGR_CFG_BASE,
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS1_ECC_AGGR_CTL_BASE,
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS1_ECC_AGGR_CFG_BASE,
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS2_ECC_AGGR_CTL_BASE,
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS2_ECC_AGGR_CFG_BASE,
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS3_ECC_AGGR_CTL_BASE,
        CSL_COMPUTE_CLUSTER0_VBUSP_DDRSS3_ECC_AGGR_CFG_BASE,
#endif
    };

    uint32_t eccAggrIncrement = 0U;
    uint32_t ramId = 0U;
    for (uint32_t testCase = 0U; testCase < DDRT7_MAX_TESTCASES; testCase++)
    {
        eccAggrData[testCase].ramId = ramId;
        eccAggrData[testCase].baseAddress = baseAddresses[eccAggrIncrement];
        eccAggrData[testCase].parityInterruptId = parityInterruptIds[eccAggrIncrement];
        eccAggrData[testCase].groupId = groupIds[testCase];

        ramId++;
        /* Each ECC aggregator contains 6 RAM IDs. */
        if (DDRT7_ECC_AGGR_MAX_RAM_IDS == ramId)
        {
            eccAggrIncrement++;
            ramId = 0U;
        }
    }
}

/**
 *  \brief Configures DDR ECC Parity by enabling corresponding interrupts.
 *
 *  \param      eccAggrRegs         ECC aggregator registers.
 *
 *  \return     \ref CSL_PASS on success,
 *              \ref CSL_EFAIL, \ref CSL_EBADARGS or other on fail,
 */
static int32_t DdrT7App_DdrEccParityConfig(CSL_ecc_aggrRegs *eccAggrRegs)
{
    int32_t status = CSL_PASS;

    /* Enable the parity ECC interrupts for the ECC endpoint */
    CSL_ecc_aggrEnableCtrl memParityCtrl;
    memParityCtrl.validCfg               = CSL_ECC_AGGR_VALID_PARITY_ERR;
    memParityCtrl.intrEnableParityErr    = TRUE;

    status = CSL_ecc_aggrIntrEnableCtrl(eccAggrRegs, &memParityCtrl);
    if (CSL_PASS != status)
    {
        UART_printf("\n ECC AGGR Parity Error Interrupt Enable failed.");
    }

    if (CSL_PASS == status)
    {
        /**
         * Enable the DED(Double Error Detection) ECC Interrupts.
         * Parity Errors are always Non Correctable errors so they are
         * reported as DED interrupt even though they are one bit.
         */
        status = CSL_ecc_aggrEnableIntrs(eccAggrRegs, CSL_ECC_AGGR_INTR_SRC_DOUBLE_BIT);
    }

    return (status);
}

/**
 *  \brief Parity ECC Test.
 *
 *  Does the following:
 *  Sets up RAT translation because the ECC aggregator base address exceeds 32 bits.
 *  Configures DDR ECC Parity by enabling specific interrupts.
 *  Configures ESM and registers DDR ECC interrupt handler in the ESM.
 *  Configures the selected ram id of the ECC aggregator to do error checking
 *  Then forces the error and handles the caused interrupt.
 *
 * \param       eccAggr         Test case data.
 *
 * \return      \ref CSL_PASS on success,
 *              \ref CSL_EFAIL or other on fail,
 */
static int32_t DdrT7App_TestDdrEccParityCheckers(DdrT7App_EccAggrData eccAggr)
{
    int32_t status = CSL_PASS;
    CSL_ecc_aggrRegs *pEccAggrRegs;
    gRamId = eccAggr.ramId;
    gParityInterruptId = eccAggr.parityInterruptId;
    /* Reset the parity interrupt check variable for next test. */
    gDdrParityInterruptInvoked = FALSE;

    /* Add RAT configuration to access address > 32bit address range */
    CSL_RatTranslationCfgInfo translationCfg;
    translationCfg.translatedAddress = eccAggr.baseAddress;
    translationCfg.sizeInBytes       = DDRT7_ECC_AGGR_MMR_SIZE;
    translationCfg.baseAddress       = (uint32_t) DDR_ECC_AGGR_REGION_LOCAL_BASE;

    /* Set up RAT translation */
    Bool ratstatus = FALSE;
    ratstatus = CSL_ratConfigRegionTranslation((CSL_ratRegs*) CSL_MCU_R5FSS0_RAT_CFG_BASE,
                                               DDR_PARITY_ECC_AGGR_RAT_REGION_INDEX,
                                               &translationCfg);
    status = ratstatus ? CSL_PASS : CSL_EFAIL;
    if (CSL_PASS != status)
    {
        UART_printf("\n DDR RAT translation failed.");
    }

    /* Config Parity Intr */
    if (CSL_PASS == status)
    {
        pEccAggrRegs = (CSL_ecc_aggrRegs*) DDR_ECC_AGGR_REGION_LOCAL_BASE;
        status = DdrT7App_DdrEccParityConfig(pEccAggrRegs);
        if (CSL_PASS != status)
        {
            UART_printf("\n DDR ECC AGGR Parity Config Enable failed.");
        }
    }

    /* ESM Setup */
    if (CSL_PASS == status)
    {
        CSL_esm_app_R5_cfg cfg;
        cfg.hi_pri_evt = eccAggr.parityInterruptId;
        cfg.lo_pri_evt = 0U;    // Not used in this test.
        status = cslAppEsmSetup(&cfg);
        if (CSL_PASS != status)
        {
            UART_printf("\n DDR ESM Config failed.");
        }
    }

    /* Configure the specified ECC endpoint RAM ID. */
    if (CSL_PASS == status)
    {
        bool error_check = TRUE;
        status = CSL_ecc_aggrConfigEDCInterconnect(pEccAggrRegs, eccAggr.ramId, error_check);
        if (CSL_PASS != status)
        {
            UART_printf("\n DDR ECC EDC endpoint configuration of the specified Interconnect RAM Id failed.");
        }
    }

    /* Inject error into the specified ECC endpoint RAM id(interconnect bus). */
    if (CSL_PASS == status)
    {
        /* Configure the error to inject into the interconnect bus. */
        CSL_Ecc_AggrEDCInterconnectErrorInfo        forceErr;
        forceErr.intrSrc    = CSL_ECC_AGGR_INTR_SRC_SINGLE_BIT;
        forceErr.eccGroup   = eccAggr.groupId;
        /* Test bit location for introducing single bit errors for parity checkers. */
        forceErr.eccBit1    = 2U;
        forceErr.eccBit2    = 2U;
        forceErr.bNextBit   = FALSE;
        forceErr.eccPattern = CSL_ECC_AGGR_INJECT_PATTERN_A;
        status = CSL_ecc_aggrForceEDCInterconnectError(pEccAggrRegs, eccAggr.ramId, &forceErr);
        if (CSL_PASS != status)
        {
            UART_printf("\n DDR ECC failed to inject error into the interconnect bus.");
        }
    }

    if (CSL_PASS == status)
    {
        UART_printf("\r\n Waiting for Parity Error Generated Interrupt ");
        /* Wait for parity error triggered interrupt */
        uint32_t timeOutCnt = 0U;
        do
        {
            Osal_delay(DDRT7_INTERRUPT_WAIT_DELAY);
            timeOutCnt += DDRT7_INTERRUPT_WAIT_DELAY;
            if (timeOutCnt > DDRT7_INTERRUPT_WAIT_TIMEOUT)
            {
                status = CSL_EFAIL;
                UART_printf("\r\n Timed out while waiting for the interrupt.");
                break;
            }
        } while (gDdrParityInterruptInvoked == FALSE);

        if (CSL_PASS == status)
        {
            UART_printf("\r\n Interrupt invoked.");
            /* Check if the injected parity error group is the same as for the invoked interrupt. */
            CSL_Ecc_AggrEDCInterconnectErrorStatusInfo  errStatusInfo;
            status = CSL_ecc_aggrGetEDCInterconnectErrorStatus(pEccAggrRegs, eccAggr.ramId,  &errStatusInfo);
            if (errStatusInfo.eccGroup != eccAggr.groupId)
            {
                status = CSL_EFAIL;
            }
            if (CSL_PASS != status)
            {
                UART_printf("\n Interrupt parity group does not match the injected error parity group.");
            }
        }
    }
    UART_printf("\r\n **** DDR Memory Parity Error Test Complete ****");

    return (status);
}

/**
*  \brief Runs DDR ECC parity checker test cases.
*
*  Does the following:
*  Creates and populates the ECC aggregator test case data array.
*  Iterates through all configured test cases.
*  Runs the DDR ECC parity checker test for each test case.
*  Tracks the number of passed tests.
*  Prints the result of each individual test.
*  Prints a final summary indicating whether all tests passed or some failed.
*
*  \return      None.
*/
static void DdrT7App_TestDdrEccParityCheckersRunner(void)
{
    /* Create and populate ECC aggregator test data */
    DdrT7App_EccAggrData eccAggrData[DDRT7_MAX_TESTCASES];
    DdrT7App_PopulateEccAggrData(eccAggrData);

    uint32_t passedTests = 0U;
    for (uint32_t testCase = 0U; testCase < DDRT7_MAX_TESTCASES; testCase++)
    {
        /* Run the test. */
        int32_t testStatus = DdrT7App_TestDdrEccParityCheckers(eccAggrData[testCase]);

        if (CSL_PASS == testStatus)
        {
            passedTests++;
            UART_printf("\r\n Test %u SUCCESSFUL", testCase + 1);
        }
        else
        {
            UART_printf("\r\n Test %u FAILED!", testCase + 1);
        }
    }

    if (DDRT7_MAX_TESTCASES == passedTests)
    {
        UART_printf("\r\n All tests have passed.");
    }
    else
    {
        UART_printf("\r\n ERROR: Some tests have failed.");

    }
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
static int32_t DdrT7App_Init(void)
{
    int32_t status = BOARD_FAIL;
    Board_initCfg boardCfg;

    boardCfg = BOARD_INIT_MODULE_CLOCK | BOARD_INIT_UART_STDIO | BOARD_INIT_PINMUX_CONFIG;
    status = Board_init(boardCfg);

    return status;
}

int main(void)
{
    int32_t status = BOARD_FAIL;
    status = DdrT7App_Init();
    if (status != BOARD_SOK)
    {
        UART_printf("\n Board not initialized successfully! \n");
    }
    else
    {
        UART_printf("\n Board init complete \n");
    }

    /* Run the DDR ECC aggregator parity test. */
    DdrT7App_TestDdrEccParityCheckersRunner();

    return 0;
}
