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
 *  \brief R5f CPU5 Illegal operations and instruction trapping examples
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <math.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>
#include <ti/csl/arch/r5/interrupt.h>
#include <ti/csl/arch/r5/csl_arm_r5_mpu.h>

#include <ip_fma_r5f.h>
#include <ip_fma_r5f_cpu5.h>
#include <ti/ip_fma/examples/r5f/common/inc/ip_fma_r5f_app_utils.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* Number of different test examples in this application.*/
#define TEST_EXAMPLES_COUNT                 (8U)

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static IpFma_R5f_UndefInstrExptnHandlerArgs undefInstrExptnHandlerArgs;
static CSL_R5ExptnHandlers gR5fExptnHandlers;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static void R5fApp_UpdateExceptionHandlers(void);
static void R5fApp_triggerUndefInstr(void);

static void R5fApp_PrintUndefInstrExceptionStatus(const char* instrName);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  This is an example of how to add and invoke undefined instruction fault handler.
 *  Performing illegal operations will cause undefined instruction exceptions.
 *  e.g. division by zero
 */
int main(void)
{
    int32_t status = BOARD_FAIL;
    status = R5fApp_Init();
    if (status != BOARD_SOK)
    {
        UART_printf("R5F App(board) not initialized correctly!");
    }
    else
    {
        R5fApp_Run();
    }

    return 0;
}

static void R5fApp_Run(void)
{
    R5fApp_UpdateExceptionHandlers();

    UART_printf("\n\n ---------DIVISION BY ZERO EXAMPLE---------");
    /* Division by zero will not invoke an undefined exception by default. */
    IpFma_R5fCoreCtrlRegs coreCtrlRegs;
    IpFma_Status coreCtrlRegsReadStatus = IpFma_R5f_GetCoreCtrlRegs(&coreCtrlRegs);
    IpFma_R5f_PrintRegReadStatus(coreCtrlRegsReadStatus, "Core Control region");
    uint32_t systemControlRegisterModified = (coreCtrlRegs.sctlr | R5F_APP_SCTLR_REG_DZ_MASK);
    IpFma_R5f_write_sctlr(systemControlRegisterModified);

    uint32_t divisionResInt = 0U;
    uint32_t dividendInt = 15U;
    uint32_t divisorIntZero = 0U;

    /**
     *  Default division uses "__aeabi_xxxdiv" type wrappers, these wrappers will not invoke an
     *  undefined exception in case of a division by zero.
     *  Force the use of hardware division through assembly calls.
     */
    __asm__ volatile ("udiv %0, %1, %2"
                      : "=r"(divisionResInt)
                      : "r"(dividendInt), "r"(divisorIntZero));
    R5fApp_PrintUndefInstrExceptionStatus("UDIV");

    /* Disable division by zero again and call sdiv instruction. */
    IpFma_R5f_write_sctlr(systemControlRegisterModified);
    __asm__ volatile ("sdiv %0, %1, %2"
                      : "=r"(divisionResInt)
                      : "r"(dividendInt), "r"(divisorIntZero));
    R5fApp_PrintUndefInstrExceptionStatus("SDIV");

    /* Floating point overflow fault example */
    /* FPSCR contains status bits for floating point operations for overflow, underflow, division by zero and other operations. */
    uint32_t fpscrReg = 0U;
    __asm__ volatile("VMRS %0, fpscr" : "=r"(fpscrReg));
    UART_printf("\n FPSCR overflow bit status: %x", (fpscrReg & (0x1U << 2)));
    double multiplicandDouble = 1.0e300;
    double multiplierDouble = 1.0e300;
    double multiplicationDoubleRes = multiplicandDouble * multiplierDouble;
    if (isinf(multiplicationDoubleRes)) {
        // Overflow occurred: result is +inf or -inf
        UART_printf("\nOverflow error infinity ");
    }
    __asm__ volatile("VMRS %0, fpscr" : "=r"(fpscrReg));
    UART_printf("\n FPSCR overflow bit status after overflow occurred: %x", (fpscrReg & (0x1U << 2)) >> 2);

    /* Floating point division by zero fault example */
    __asm__ volatile("VMRS %0, fpscr" : "=r"(fpscrReg));
    UART_printf("\n FPSCR division by zero bit status: %x", (fpscrReg & (0x1U << 1)));
    float divisionResFloat = 0.0;
    float dividendFloat = 15.0;
    float divisorFloatZero = 0.0;
    divisionResFloat = dividendFloat / divisorFloatZero;
    if (isinf(divisionResFloat)) {
        // Division by zero occurred: result is +inf or -inf
        UART_printf("\n Division by zero error infinity");
    }
    __asm__ volatile("VMRS %0, fpscr" : "=r"(fpscrReg));
    UART_printf("\n FPSCR division by zero bit status after overflow occurred: %x", (fpscrReg & (0x1U << 1)) >> 1);


    UART_printf("\n\n ---------FPU ARITHMETIC OPERATIONS WITH FPU ACCESS DISABLED---------");
    /* Disable access to FPU. */
    uint32_t cpacr = IpFma_R5f_read_cpacr();
    uint32_t cpacrModified = cpacr & ~R5F_APP_CPACR_FPU_ACCESS_PERMISSION_MASK;

    __asm__ volatile("mcr	p15, #0x0, %0, c1, c0, #0x2" : : "r"(cpacrModified));
    Double floatValueOne = 15.5;
    R5fApp_PrintUndefInstrExceptionStatus("MOVE DOUBLE CONSTANT");

    __asm__ volatile("mcr	p15, #0x0, %0, c1, c0, #0x2" : : "r"(cpacrModified));
    Double floatValueTwo = floatValueOne;
    R5fApp_PrintUndefInstrExceptionStatus("MOVE/VLDR DOUBLE");

    /* Perform calculations using double-precision floating-point numbers. */
    /* Division */
    Double floatResult = 0;
    __asm__ volatile ("mcr p15, #0x0, %1, c1, c0, #0x2\n\t"
                      "vdiv.f64 %P0, %P2, %P3"
                      : "=w"(floatResult)
                      : "r"(cpacrModified), "w"(floatValueOne), "w"(floatValueTwo));
    UART_printf("\nDivision result: %f",floatResult);
    R5fApp_PrintUndefInstrExceptionStatus("DOUBLE DIVISION");

    /* Multiplication */
    __asm__ volatile ("mcr p15, #0x0, %1, c1, c0, #0x2\n\t"
                      "vmul.f64 %P0, %P2, %P3"
                      : "=w"(floatResult)
                      : "r"(cpacrModified), "w"(floatValueOne), "w"(floatValueTwo));
    UART_printf("\nMultiplication result: %f",floatResult);
    R5fApp_PrintUndefInstrExceptionStatus("DOUBLE MULTIPLICATION");

    /* Addition */
    __asm__ volatile ("mcr p15, #0x0, %1, c1, c0, #0x2\n\t"
                      "vadd.f64 %P0, %P2, %P3"
                      : "=w"(floatResult)
                      : "r"(cpacrModified), "w"(floatValueOne), "w"(floatValueTwo));
    UART_printf("\nAddition result: %f",floatResult);
    R5fApp_PrintUndefInstrExceptionStatus("DOUBLE ADDITION");


    UART_printf("\n\n ---------EXECUTING INSTRUCTION WITH INVALID ENCODING---------");
    /* Call undefined instruction */
    R5fApp_triggerUndefInstr();
    R5fApp_PrintUndefInstrExceptionStatus("UNKNOWN UNDEFINED INSTRUCTION");

    /* END OF ALL EXCEPTION EXAMPLES */

    uint32_t undefExceptionInvocationCount = IpFma_R5f_GetExceptionInvocationCount();
    if (TEST_EXAMPLES_COUNT == undefExceptionInvocationCount)
    {
        UART_printf("\n\n All tests have passed!");
    }
    else
    {
        UART_printf("\n\n Some tests have failed!");
    }
}

static int32_t R5fApp_Init(void)
{
    UART_printf("\n ... Init \n\n");

    int32_t status = -1;
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    return status;
}

/**
 *  \brief Updates the undefined instruction exception handler and its arguments.
 */
static void R5fApp_UpdateExceptionHandlers(void)
{
    /* Initialise new exception handlers container. */
    Intc_InitExptnHandlers(&gR5fExptnHandlers);

    /* Modify the undefined exception handler function and args. */
    IpFma_R5f_UpdateUndefInstrExptnHandler(&gR5fExptnHandlers,
                                           (exptnHandlerPtr) IpFma_R5f_UndefInstrExptnHandler,
                                           &undefInstrExptnHandlerArgs);

    /* Register the updated handler exceptions and arguments. */
    Intc_RegisterExptnHandlers(&gR5fExptnHandlers);
}

/**
 *  \brief Used to trigger an undefined instruction as a example showcase.
 */
static void R5fApp_triggerUndefInstr(void)
{
    /* Inline assembly with an unknown/invalid encoding */
    __asm__ volatile (".word 0xFFFFFFFF");
}

/**
 *  \brief Simple utility function that prints undefined instruction exception arguments.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param  instrName    [IN] Name of the instruction that caused the undefined instr exception.
 *
 *  \return None.
 *
 */
static void R5fApp_PrintUndefInstrExceptionStatus(const char* instrName)
{
    UART_printf("\n Address of the failed \"%s\" instruction: %x", instrName, undefInstrExptnHandlerArgs.address);
    UART_printf("\n Encoding of the \"%s\" instruction: %x", instrName, undefInstrExptnHandlerArgs.undefInstrEncoding);

    if (IPFMA_R5F_INSTRSET_ARM_MODE == undefInstrExptnHandlerArgs.instrSet)
    {
        UART_printf("\n Instruction was executed as part of the ARM instruction set.\n");
    }
    else
    {
        UART_printf("\n Instruction was executed as part of the THUMB instruction set.\n");
    }
}
