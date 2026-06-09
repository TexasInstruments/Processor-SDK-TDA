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
 *  \brief R5f CPU3 MPU functionality test
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>
#include <ti/csl/arch/r5/interrupt.h>
#include <ti/csl/arch/r5/csl_arm_r5_mpu.h>

#include <ip_fma_r5f.h>
#include <ip_fma_r5f_cpu3.h>
#include <ti/ip_fma/examples/r5f/common/inc/ip_fma_r5f_app_utils.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* This address is set in the linker script. */
#define TEST_MPU_REGION_BASE_ADDRESS            (0xc1010000U)

/* Number of MPU examples in this application. */
#define TEST_MPU_EXAMPLES_COUNT                 (5U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static IpFma_R5fDataAbortExptnHandlerArgs gR5fDataAbortExptnHandlerArgs;
static IpFma_R5fPrefetchAbortExptnHandlerArgs gR5fPrefetchAbortExptnHandlerArgs;
static CSL_R5ExptnHandlers gR5fExptnHandlers;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t R5fApp_AddMpuTestRegion(void);
static void R5fApp_UpdateMpuExceptionHandlers(void);

__attribute__((noinline, section(".test_section")))
static void R5fApp_TestMpuRegionFunc();

static void R5fApp_PrintDataAbortExceptionStatus(uint32_t exceptionAddress, uint32_t readVal);
static void R5fApp_PrintPrefetchAbortExceptionStatus(uint32_t exceptionAddress);
static void R5fApp_PrintCommonExceptionStatus(uint32_t exceptionType, uint32_t exceptionAddress);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  This is an example of how to add and invoke MPU fault handlers.
 *  MPU is initialised when the board is initialized.
 */
int main(void)
{
    int32_t status = BOARD_FAIL;
    status = R5fApp_Init();
    if (status == BOARD_SOK)
    {
        R5fApp_UpdateMpuExceptionHandlers();
        status = R5fApp_AddMpuTestRegion();
        if (status != BOARD_SOK)
        {
            UART_printf("\nApp failed to initialize correctly!!!\n");
        }
        else
        {
            R5fApp_Run();
        }
    }

    return 0;
}

static void R5fApp_Run(void)
{
    /* Select the newly created MPU region 8(test region) to use for examples below. */
    IpFma_R5fMpuRegionRegs mpuRegionRegs;
    uint32_t regionNum = 8U;
    IpFma_R5f_write_rgnr(regionNum);
    IpFma_Status mpuRegionRegsReadStatus = IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);
    IpFma_R5f_PrintRegReadStatus(mpuRegionRegsReadStatus, "MPU region 0");

    /**
     * Example to invoke permission type data abort exception.
     */
    UART_printf("\n\nDATA ABORT(PERMISSION) EXCEPTION EXAMPLE");

    /* Clear access permission bits i.e. set the region as inaccessible. */
    uint32_t regionAccessControlRegister = mpuRegionRegs.racr;
    regionAccessControlRegister &= ~(0x7U << CSL_ARM_R5_MPU_REGION_AC_AP_SHIFT);
    IpFma_R5f_write_racr(regionAccessControlRegister);

    /* Create a random address from the MPU test region. */
    uint32_t inaccessibleMpuRegionOffset = 0x1024U;
    uint32_t inaccessibleMpuRegionAddress = mpuRegionRegs.rbar + inaccessibleMpuRegionOffset;
    volatile uint32_t* inaccessibleMpuRegionAddressPtr = (volatile uint32_t*) inaccessibleMpuRegionAddress;

    /* Read the random inaccessible address/invoke data abort exception. */
    uint32_t inaccessibleMpuRegionReadVal = 0U;
    inaccessibleMpuRegionReadVal = *inaccessibleMpuRegionAddressPtr;

    /* Reset MPU access control register. */
    IpFma_R5f_write_racr(mpuRegionRegs.racr);

    R5fApp_PrintDataAbortExceptionStatus(inaccessibleMpuRegionAddress, inaccessibleMpuRegionReadVal);


    /**
     * Example to invoke permission type prefetch abort exception.
     */
    UART_printf("\n\nPREFETCH ABORT(PERMISSION) EXCEPTION EXAMPLE");

    /* Set execute never bit to true i.e. set the region as non executable. */
    regionAccessControlRegister = mpuRegionRegs.racr;
    regionAccessControlRegister |= CSL_ARM_R5_MPU_REGION_AC_XN_MASK;
    IpFma_R5f_write_racr(regionAccessControlRegister);

    /* Execute the non executable function/invoke prefetch abort exception. */
    R5fApp_TestMpuRegionFunc();

    R5fApp_PrintPrefetchAbortExceptionStatus(((uint32_t) R5fApp_TestMpuRegionFunc & ~(0x1U)));


    /**
     * Example to invoke alignment type data abort exception.
     */
    UART_printf("\n\nDATA ABORT(ALIGNMENT) EXCEPTION EXAMPLE");

    /* Unaligned access is not prohibited by default. */
    IpFma_R5fCoreCtrlRegs coreCtrlRegs;
    IpFma_Status coreCtrlRegsReadStatus = IpFma_R5f_GetCoreCtrlRegs(&coreCtrlRegs);
    IpFma_R5f_PrintRegReadStatus(coreCtrlRegsReadStatus, "Core Control region");

    uint32_t systemControlRegisterModified = (coreCtrlRegs.sctlr | 0x2U);
    IpFma_R5f_write_sctlr(systemControlRegisterModified);

    /* Create a random unaligned address from the MPU test region. */
    uint32_t unalignedOffset = 0x1025U;
    uint32_t unalignedAddress = mpuRegionRegs.rbar + unalignedOffset;
    volatile uint32_t* unalignedAddressPtr = (volatile uint32_t*) unalignedAddress;

    /* Read the random unaligned address/invoke data abort exception */
    uint32_t unalignedReadVal = 0U;
    unalignedReadVal = (uint32_t) *unalignedAddressPtr;

    /* Reset system control register. */
    IpFma_R5f_write_sctlr(coreCtrlRegs.sctlr);

    R5fApp_PrintDataAbortExceptionStatus(unalignedAddress, unalignedReadVal);


    /**
     * Example to invoke background type data abort exception.
     */
    UART_printf("\n\nDATA ABORT(BACKGROUND) EXCEPTION EXAMPLE");

    /* Disable subregion 0 of test region 8. */
    regionNum = 8U;
    IpFma_R5f_write_rgnr(regionNum);
    mpuRegionRegsReadStatus = IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);
    IpFma_R5f_PrintRegReadStatus(mpuRegionRegsReadStatus, "MPU region 8");
    uint32_t regionSizeRegister = mpuRegionRegs.rser;
    regionSizeRegister |= (CSL_ARM_R5_MPU_SUB_REGION_0_DISABLE << CSL_ARM_R5_MPU_REGION_SZEN_SRD_SHIFT);
    IpFma_R5f_write_rser(regionSizeRegister);

    uint32_t disabledMpuRegionBaseAddress = mpuRegionRegs.rbar;

    /* Disable subregion 4 of region 4. */
    regionNum = 4U;
    IpFma_R5f_write_rgnr(regionNum);
    mpuRegionRegsReadStatus = IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);
    IpFma_R5f_PrintRegReadStatus(mpuRegionRegsReadStatus, "MPU region 4");
    regionSizeRegister = mpuRegionRegs.rser;
    regionSizeRegister |= (CSL_ARM_R5_MPU_SUB_REGION_4_DISABLE << CSL_ARM_R5_MPU_REGION_SZEN_SRD_SHIFT);
    IpFma_R5f_write_rser(regionSizeRegister);

    /* Disable subregion 6 of region 0. */
    regionNum = 0U;
    IpFma_R5f_write_rgnr(regionNum);
    mpuRegionRegsReadStatus = IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);
    IpFma_R5f_PrintRegReadStatus(mpuRegionRegsReadStatus, "MPU region 0");
    regionSizeRegister = mpuRegionRegs.rser;
    regionSizeRegister |= (CSL_ARM_R5_MPU_SUB_REGION_6_DISABLE << CSL_ARM_R5_MPU_REGION_SZEN_SRD_SHIFT);
    IpFma_R5f_write_rser(regionSizeRegister);

    /* Create a random address from the MPU test region 8 subregion 1. */
    uint32_t disabledMpuRegionOffset = 0x512U;
    uint32_t disabledMpuRegionAddress = disabledMpuRegionBaseAddress + disabledMpuRegionOffset;
    volatile uint32_t* disabledMpuRegionAddressPtr = (volatile uint32_t*) disabledMpuRegionAddress;

    /* Read the random address from uncovered address space/invoke data abort exception */
    uint32_t disabledMpuRegionReadVal = 0U;
    disabledMpuRegionReadVal = *disabledMpuRegionAddressPtr;

    R5fApp_PrintDataAbortExceptionStatus(disabledMpuRegionAddress, disabledMpuRegionReadVal);


    /**
     * Example to invoke background type prefetch abort exception.
     */
    UART_printf("\n\nPREFETCH ABORT(BACKGROUND) EXCEPTION EXAMPLE");

    /* Subregions were disabled in the previous example. */
    /* Execute the function from uncovered address space/invoke prefetch abort exception. */
    R5fApp_TestMpuRegionFunc();

    R5fApp_PrintPrefetchAbortExceptionStatus(((uint32_t) R5fApp_TestMpuRegionFunc) & ~(0x1U));


    /* END OF ALL EXCEPTION EXAMPLES */

    uint32_t exceptionInvocationCount = IpFma_R5f_GetExceptionInvocationCount();
    if (TEST_MPU_EXAMPLES_COUNT == exceptionInvocationCount)
    {
        UART_printf("\n\n All tests have passed!");
    }
    else
    {
        UART_printf("\n\n Some tests have failed!");
    }
}

static int32_t R5fApp_Init()
{
    UART_printf("\n ... Init \n\n");

    int32_t status = -1;
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    return status;
}

/**
 *  \brief Updates the data abort and prefetch abort exception handlers and their arguments.
 */
static void R5fApp_UpdateMpuExceptionHandlers(void)
{
    /* Initialise new exception handlers container. */
    Intc_InitExptnHandlers(&gR5fExptnHandlers);

    /* Modify the data abort handler function and args. */
    IpFma_R5f_UpdateDataAbortExptnHandler(&gR5fExptnHandlers, (exptnHandlerPtr) IpFma_R5f_DataAbortExptnHandler, &gR5fDataAbortExptnHandlerArgs);

    /* Modify the prefetch abort handler function and args. */
    IpFma_R5f_UpdatePrefetchAbortExptnHandler(&gR5fExptnHandlers, (exptnHandlerPtr) IpFma_R5f_PrefetchAbortExptnHandler, &gR5fPrefetchAbortExptnHandlerArgs);

    /* Register the updated handler exceptions and arguments. */
    Intc_RegisterExptnHandlers(&gR5fExptnHandlers);
}

/**
 *  \brief Function created in the .test_section memory space, used for testing MPU.
 *
 *  noinline attribute is added to prevent compiler from optimizing out the function call by
 *  copying the function contents into the main function.
 */
__attribute__((noinline, section(".test_section"))) static void R5fApp_TestMpuRegionFunc(void)
{
    UART_printf("\nMPU REGION TEST FUNCTION INVOKED AFTER EXCEPTION! (Indicates the exception cause has been resolved)");
}

/**
 *  \brief Creates a new MPU region for testing data abort and prefetch abort exceptions.
 */
static int32_t R5fApp_AddMpuTestRegion(void)
{
    int32_t status = BOARD_SOK;
    uint32_t regionNum = 0U;
    IpFma_R5fMpuRegionRegs mpuRegionRegs;
    IpFma_Status mpuRegionRegsReadStatus;

    /* Use MPU region 5 as template */
    regionNum = 5U;
    IpFma_R5f_write_rgnr(regionNum);
    mpuRegionRegsReadStatus = IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);
    IpFma_R5f_PrintRegReadStatus(mpuRegionRegsReadStatus, "MPU region 5");

    /* Create region 8 */
    mpuRegionRegs.rgnr = 8U;
    mpuRegionRegs.rbar = (uint32_t) TEST_MPU_REGION_BASE_ADDRESS;
    CSL_armR5MpuCfgRegion(mpuRegionRegs.rgnr, mpuRegionRegs.rbar, mpuRegionRegs.rser, mpuRegionRegs.racr);

    /* Check if the MPU region has been created successfully */
    regionNum = 8U;
    IpFma_R5f_write_rgnr(regionNum);
    mpuRegionRegsReadStatus = IpFma_R5f_GetMpuRegionRegs(&mpuRegionRegs);
    IpFma_R5f_PrintRegReadStatus(mpuRegionRegsReadStatus, "MPU region 8");

    if (TEST_MPU_REGION_BASE_ADDRESS == mpuRegionRegs.rbar)
    {
        UART_printf("\nSUCCESS: MPU region 8 has been created successfully");
    }
    else
    {
        UART_printf("\nERROR: MPU region 8 has not been created successfully");
        status = BOARD_FAIL;
    }

    return status;
}


/**
 *  \brief Simple utility function that prints data abort exception arguments.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param exceptionAddress     [IN] Address used to invoke the MPU exception.
 *  \param readVal              [IN] Value read from the address above.
 *
 *  \return None.
 */
static void R5fApp_PrintDataAbortExceptionStatus(uint32_t exceptionAddress, uint32_t readVal)
{
    UART_printf("\nData abort exception successfully triggered and resolved!");
    UART_printf("\nValue at address is: %u (This is uninitialized memory!).", readVal);
    UART_printf("\nAddress that was used for triggering the exception: 0x%x", exceptionAddress);
    R5fApp_PrintCommonExceptionStatus(gR5fDataAbortExptnHandlerArgs.type, gR5fDataAbortExptnHandlerArgs.address);

    if (FAULT_STATUS_CAUSE_READ == gR5fDataAbortExptnHandlerArgs.cause)
    {
        UART_printf("\nThe data abort was triggered by a READ operation.");
    }
    else if (FAULT_STATUS_CAUSE_WRITE == gR5fDataAbortExptnHandlerArgs.cause)
    {
        UART_printf("\nThe data abort was triggered by a WRITE operation.");
    }
    else
    {
        UART_printf("\nThe data abort was triggered by UNKNOWN.");
    }
}

/**
 *  \brief Simple utility function that prints prefetch abort exception arguments.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param exceptionAddress     [IN] Address used to invoke the MPU exception.
 *
 *  \return None.
 */
static void R5fApp_PrintPrefetchAbortExceptionStatus(uint32_t exceptionAddress)
{
    UART_printf("\nPrefetch abort exception successfully triggered and resolved!");
    UART_printf("\nAddress that was used for triggering the exception: 0x%x", exceptionAddress);
    R5fApp_PrintCommonExceptionStatus(gR5fPrefetchAbortExptnHandlerArgs.type, gR5fPrefetchAbortExptnHandlerArgs.address);
}

/**
 *  \brief Simple utility function that prints common abort exception arguments.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param exceptionType        [IN] Type of the exception.
 *  \param exceptionAddress     [IN] The address that triggered the MPU exception.
 *
 *  \return None.
 */
static void R5fApp_PrintCommonExceptionStatus(uint32_t exceptionType, uint32_t exceptionAddress)
{
    UART_printf("\nThe address that triggered the MPU exception is: 0x%x", exceptionAddress);

    if (FAULT_STATUS_TYPE_ALIGNMENT == exceptionType)
    {
        UART_printf("\nType of the fault is: ALIGNMENT");
    }
    else if (FAULT_STATUS_TYPE_BACKGROUND == exceptionType)
    {
        UART_printf("\nType of the fault is: BACKGROUND");
    }
    else if (FAULT_STATUS_TYPE_PERMISSION == exceptionType)
    {
        UART_printf("\nType of the fault is: PERMISSION");
    }
    else
    {
        UART_printf("\nType of the fault is: UNKOWN");
        UART_printf("\nType code the fault is: %x", exceptionType);
    }
}
