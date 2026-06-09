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
 *  \brief SAUL5 - SA3UL dynamic registers readback example code
 *
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>
#include <ti/board/src/devices/common/common.h>

#include <ip_fma_common.h>
#include <ip_fma_sa3ul.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

static int32_t Saul5App_Init();
static void Saul5App_Sa3ulDynamicRegReadback();

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void Saul5App_Sa3ulDynamicRegReadback()
{
    IpFma_Status status = IPFMA_OK;

    uintptr_t goldenRefRegData[IP_FMA_SA3UL_DYNAMIC_REGDUMP_BUFFER_SIZE];
    status = IpFma_Sa3ul_GetDynamicRegsCfg(goldenRefRegData);

    if (IPFMA_OK == status)
    {
        status = IpFma_Sa3ul_VerifyDynamicRegsCfg(goldenRefRegData);

        if (IPFMA_OK == status)
        {
            UART_printf("\nSA3UL registers read successfully.");
            UART_printf("\nSUCCESS: All tests have passed.");
        }
        else
        {
            UART_printf("\n ERROR: Mismatch detected!");
            UART_printf("\n Some tests have failed!");
        }
    }
    else
    {
        UART_printf("\n ERROR: SA3UL registers were NOT read successfully!");
        UART_printf("\n Some tests have failed!");
    }
}

/**
 *  \brief Main function.
 *
 *  This is an example of how to read SA3UL dynamic configuration registers.
 *
 *  \return  Status of the program, 0 - OK, -10 - Error.
 */
int main(void)
{
    int32_t status = BOARD_FAIL;
    status = Saul5App_Init();
    if (status != BOARD_SOK) return status;

    Saul5App_Sa3ulDynamicRegReadback();

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
static int32_t Saul5App_Init()
{
    UART_printf("\n ... Init \n\n");

    int32_t status = BOARD_FAIL;
    Board_initCfg boardCfg;
    boardCfg = BOARD_INIT_UART_STDIO;
    status = Board_init(boardCfg);

    return status;
}
