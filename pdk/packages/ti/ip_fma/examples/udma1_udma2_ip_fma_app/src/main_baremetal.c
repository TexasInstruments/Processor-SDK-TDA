/*
 *  Copyright (c) Texas Instruments Incorporated 2026
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
 *  \brief Main file for baremetal build of the udma1_udma2_ip_fma_app
 *         example application.
 */

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */

#include <stdint.h>
#include <ti/board/board.h>
#include <ti/drv/uart/UART.h>
#include <ti/drv/uart/UART_stdio.h>

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

extern int32_t Udma1Udma2_MemcpyMain(void);

/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

/* None */

/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

int main(void)
{
    Board_initCfg           boardCfg;

    boardCfg = BOARD_INIT_MODULE_CLOCK  |
               BOARD_INIT_PINMUX_CONFIG |
               BOARD_INIT_UART_STDIO;
    Board_init(boardCfg);

#if defined(BOARD_ENABLE_DDR_REG_VERIFY)
    Board_STATUS status;

    UART_printf("DDR Register Check In Progress...\n");
    status = Board_init(BOARD_INIT_DDR_REG_VERIFY);
    if (status != BOARD_SOK)
    {
        if (status == BOARD_DDR_CTL_REG_CHECK_FAIL)
        {
            UART_printf("DDR Control Register Check Failed!!\n");
        }
        else if (status == BOARD_DDR_PHYINDEP_REG_CHECK_FAIL)
        {
            UART_printf("DDR PHY INDEP Register Check Failed!!\n");
        }
        else if (status == BOARD_DDR_PHY_REG_CHECK_FAIL)
        {
            UART_printf("DDR PHY Register Check Failed!!\n");
        }

        return -1;
    }
    else
    {
        UART_printf("DDR Register Check Successful!\n");
    }
#endif

    Udma1Udma2_MemcpyMain();

    return(0);
}
