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
 *  \brief RATE LIMIT 7 static registers readback example code
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>

#include <ip_fma_rl7.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t Rl7App_Init();
static void Rl7App_PrintRegStatus(IpFma_Status status, char* registerName);
static void Rl7App_PrintRegCompareStatus(IpFma_Status status, char* registerName);

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/**
 *  \brief Main function.
 *
 *  This is an example of how to read rate limit static configuration registers.
 *
 *  \return  Status of the program, 0 - OK, -10 - Error.
 */
int main(void)
{
    int8_t status = BOARD_FAIL;
    status = Rl7App_Init();
    if (status != BOARD_SOK) return status;

    /**
     *  Rate limit registers are contained/grouped in \c IpFma_Rl7Regs structure.
     *  There are two identical register sets: one handles reads, the other handles writes.
     *  Each has a corresponding read API.
     */

    IpFma_Rl7Regs rl7ReadRegs;
    IpFma_Status rl7ReadRegsStatus = IpFma_Rl7_GetReadRegs(&rl7ReadRegs);
    Rl7App_PrintRegStatus(rl7ReadRegsStatus, "Rl7 Read registers");

    IpFma_Rl7Regs rl7WriteRegs;
    IpFma_Status rl7WriteRegsStatus = IpFma_Rl7_GetWriteRegs(&rl7WriteRegs);
    Rl7App_PrintRegStatus(rl7WriteRegsStatus, "Rl7 Write registers");

    IpFma_Rl7Regs rl7ReadRegsDefault;
    rl7ReadRegsDefault.CTRL = RD_CTRL_RESET_VALUE;
    rl7ReadRegsDefault.CIR = RD_BW_CIR_RESET_VALUE;
    rl7ReadRegsDefault.PIR = RD_BW_PIR_RESET_VALUE;
    rl7ReadRegsDefault.BURST_OFFSET = RD_BW_BURST_OFFSET_RESET_VALUE;
    rl7ReadRegsDefault.TXN = RD_TXN_RESET_VALUE;

    IpFma_Rl7Regs rl7WriteRegsDefault;
    rl7WriteRegsDefault.CTRL = WR_CTRL_RESET_VALUE;
    rl7WriteRegsDefault.CIR = WR_BW_CIR_RESET_VALUE;
    rl7WriteRegsDefault.PIR = WR_BW_PIR_RESET_VALUE;
    rl7WriteRegsDefault.BURST_OFFSET = WR_BW_BURST_OFFSET_RESET_VALUE;
    rl7WriteRegsDefault.TXN = WR_TXN_RESET_VALUE;

    IpFma_Status rl7ReadRegsCompareStatus = IpFma_Rl7_CompareRegs(&rl7ReadRegs, &rl7ReadRegsDefault);
    Rl7App_PrintRegCompareStatus(rl7ReadRegsCompareStatus, "READ REGISTERS");

    IpFma_Status rl7WriteRegsCompareStatus = IpFma_Rl7_CompareRegs(&rl7WriteRegs, &rl7WriteRegsDefault);
    Rl7App_PrintRegCompareStatus(rl7WriteRegsCompareStatus, "WRITE REGISTERS");

    if ((IPFMA_OK == rl7ReadRegsCompareStatus) && (IPFMA_OK == rl7WriteRegsCompareStatus))
    {
        UART_printf("\nRegister check done ...!\r\n");
        UART_printf("All tests have passed!!\n");
    } 
    else
    {
        UART_printf("\nRegister check done ...!\r\n");
        UART_printf("Some tests have failed!!\n");
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
static int32_t Rl7App_Init()
{
    UART_printf("\n ... Init \n\n");

    int8_t status = -1;
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    return status;
}

/**
 *  \brief Simple utility function that prints whether the register read operation was successful or not.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param registerName  [IN] Name of the register.
 *  \param regReadStatus [IN] Value that the register is expected contain.
 *
 *  \return None.
 */
static void Rl7App_PrintRegStatus(IpFma_Status regReadStatus, char* registerName)
{
    if (IPFMA_OK == regReadStatus)
    {
        UART_printf("\nSuccess! The registers %s have been read successfully.", registerName);
    }
    else
    {
        UART_printf("\nError! The registers %s have not been read successfully.", registerName);
    }
}

/**
 *  \brief Simple utility function that prints whether the registers were equal or not based
 *         on the comparison results.
 *
 *  This utility is intended for configuration/readback diagnostics. It emits a
 *  human-readable pass/fail message using \c UART_printf.
 *
 *  \param registerName     [IN] Name of the register.
 *  \param regCompareStatus [IN] Value that the register is expected contain.
 *
 *  \return None.
 */
static void Rl7App_PrintRegCompareStatus(IpFma_Status regCompareStatus, char* registerName)
{
    if (IPFMA_OK == regCompareStatus)
    {
        UART_printf("\nSuccess! The registers *%s* have not been modified.", registerName);
    }
    else
    {
        UART_printf("\nError! The registers *%s* have been modified.", registerName);
    }
}
